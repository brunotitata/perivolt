#include "monitor.hpp"
#include <limits>
#include <chrono>
#include <stdexcept>
namespace perivolt::application {
Monitor::Monitor(std::int64_t ttl_ms):ttl_(ttl_ms),battery_(ttl_ms) {
    control_.producer_connected=true;
}
Monitor::~Monitor() { shutdown(); }
void Monitor::clear_locked() {
    metadata_.reset();
    battery_=Battery(ttl_); entries_.clear(); control_.active=false;
}
SessionToken Monitor::request_read(bool enabled) {
    const std::lock_guard lock(mutex_);
    if (closed_) return {};
    // Every request is a new session. OFF->ON never accepts an earlier completion.
    if (generation_==std::numeric_limits<std::uint64_t>::max()) {
        closed_=true; clear_locked(); control_.consent=ui::Consent::off;
        control_.producer_connected=false; return {};
    }
    ++generation_; clear_locked();
    control_.consent=enabled ? ui::Consent::requested : ui::Consent::off;
    return {generation_,enabled};
}
void Monitor::cancel() { (void)request_read(false); }
void Monitor::shutdown() {
    {
        const std::lock_guard lock(mutex_);
        closed_=true; clear_locked(); control_.consent=ui::Consent::off;
        control_.producer_connected=false;
    }
    // Registry enumeration has no cancellable transport. Join outside state lock.
    // Late completion is discarded by closed_, never delivered into a destroyed UI.
    if (metadata_task_.valid()) metadata_task_.wait();
}
void Monitor::refresh_metadata() {
#ifdef __APPLE__
    const std::lock_guard lock(mutex_);
    if (closed_) return;
    if (metadata_task_.valid() && metadata_task_.wait_for(std::chrono::seconds(0))!=std::future_status::ready) return;
    const attackshark_x11::macos::SessionToken token{generation_,control_.consent==ui::Consent::requested || control_.consent==ui::Consent::granted};
    metadata_task_=std::async(std::launch::async,[this,token] {
        try { submit_metadata(attackshark_x11::macos::poll_native_metadata(token)); }
        catch (...) { // Failure cannot create an observation or refresh cached battery time.
            attackshark_x11::macos::NativeSample error; error.token=token; error.os_error=-1;
            submit_metadata(error);
        }
    });
#endif
}
bool Monitor::submit_metadata(const attackshark_x11::macos::NativeSample& sample) {
    const std::lock_guard lock(mutex_);
    const bool opt_in=control_.consent==ui::Consent::requested || control_.consent==ui::Consent::granted;
    if (closed_ || sample.token.generation!=generation_ || sample.token.opt_in!=opt_in) return false;
    // Current native producer has NO identity, connection time or battery capability.
    // Even a future/malformed metadata result with percent/live set cannot enter Battery.
    metadata_=sample;
    metadata_->observation.percent.reset(); metadata_->observation.observed_at.reset();
    metadata_->observation.live=false; metadata_->read_available=false; metadata_->active=false;
    control_.capability_available=false; control_.active=false;
    return true;
}
std::optional<attackshark_x11::macos::NativeSample> Monitor::metadata() const {
    const std::lock_guard lock(mutex_); return metadata_;
}
void Monitor::source_status(bool capability_available, ui::Permission permission) {
    const std::lock_guard lock(mutex_);
    if (closed_) return;
    control_.capability_available=capability_available; control_.permission=permission;
    if (!capability_available || permission!=ui::Permission::granted) {
        // Revocation invalidates pending callbacks, including after permission returns.
        if (generation_!=std::numeric_limits<std::uint64_t>::max()) ++generation_;
        else closed_=true;
        clear_locked();
        if (control_.consent!=ui::Consent::off)
            control_.consent=permission==ui::Permission::denied ? ui::Consent::denied : ui::Consent::requested;
    }
}
bool Monitor::submit(SessionToken token,const Sample& sample) {
    const std::lock_guard lock(mutex_);
    if (closed_ || !token.opt_in || token.generation!=generation_ ||
        control_.consent==ui::Consent::off || control_.consent==ui::Consent::denied ||
        !control_.capability_available || control_.permission!=ui::Permission::granted ||
        sample.origin!=SampleOrigin::session_observation) return false;
    const auto& o=sample.observation;
    if (sample.identity.id.empty() || sample.identity.id!=o.device || o.component.empty() ||
        o.source.empty() || o.source_epoch.empty()) return false;
    const Key key{o.device,o.component};
    const auto prior=entries_.find(key);
    if (prior!=entries_.end() && prior->second.source!=o.source) return false;
    // Identity conflicts and unsupported source changes are refused, never merged.
    try { battery_.register_device(sample.identity); }
    catch (const std::invalid_argument&) { return false; }
    if (!battery_.observe(o)) return false;
    entries_[key]={sample.name,o.source,o.monotonic_epoch,o.observed_ms};
    control_.consent=ui::Consent::granted;
    // A completed point observation is NOT an ongoing monitoring cycle.
    control_.active=false;
    return true;
}
ui::Snapshot Monitor::snapshot(std::int64_t now_ms,const std::string& epoch) const {
    const std::lock_guard lock(mutex_);
    ui::Snapshot result; result.control=control_;
    for (const auto& [key,entry]:entries_) {
        auto state=battery_.project(key.first,key.second,now_ms,epoch);
        if (!state.present) continue;
        std::optional<std::int64_t> age;
        if (entry.observed && *entry.observed>=0 && now_ms>=*entry.observed &&
            !epoch.empty() && epoch==entry.epoch) age=now_ms-*entry.observed;
        result.devices.push_back({entry.name,std::move(state),age,ui::Origin::session_observation,entry.source});
    }
    return result;
}
}

