#include "battery.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace perivolt {
Battery::Battery(std::int64_t ttl_ms) : ttl_(ttl_ms) {
    if (ttl_ms < 0) throw std::invalid_argument("negative TTL");
}
void Battery::configure(AlertConfig config) {
    if (config.threshold < 1 || config.threshold > 99 || config.hysteresis < 0)
        throw std::invalid_argument("invalid alert config");
    config_ = config;
}
void Battery::set_threshold(int threshold) {
    auto config = config_; config.threshold = threshold; configure(config);
}
bool Battery::observe(Observation o) {
    const Key key{o.device, o.component};
    if (o.device.empty() || o.component.empty() || o.source.empty())
        throw std::invalid_argument("missing identity");
    auto life = lifecycle_.find(key);
    if (life != lifecycle_.end() && life->second.source != o.source)
        throw std::invalid_argument("multi-source not implemented");
    const auto epoch = source_epochs_.find(o.source);
    if (epoch != source_epochs_.end() && epoch->second != o.source_epoch) {
        if (life != lifecycle_.end() && !o.id.empty()) life->second.rejected.push_back(o.id);
        return false;
    }
    if (life != lifecycle_.end() && life->second.epoch == o.source_epoch &&
        o.generation <= life->second.generation) {
        if (!o.id.empty()) life->second.rejected.push_back(o.id);
        return false;
    }
    source_epochs_[o.source] = o.source_epoch;
    auto& entry = lifecycle_[key];
    entry.source = o.source; entry.epoch = o.source_epoch;
    entry.generation = o.generation; entry.removed = false;
    entry.permission_revision = permissions_[o.source].revision;
    entry.sleep_invalidated = sleeping_ || (sleep_cutoff_ && (!o.observed_ms || *o.observed_ms < *sleep_cutoff_));
    if (o.payload_version != 1) {
        o.level = {"invalid", 0, "unsupported_version"};
        o.charging = "unknown"; o.connection = "unknown";
    } else if (o.level.kind == "reported_percent") {
        const double value = o.level.percent;
        if (!std::isfinite(value)) o.level = {"invalid", 0, "non_finite"};
        else if (value < 0 || value > 100) o.level = {"invalid", 0, "out_of_range"};
        else if (std::floor(value) != value) o.level = {"invalid", 0, "malformed"};
    } else if (o.level.kind == "qualitative") {
        const auto& v = o.level.detail;
        if (v != "empty" && v != "low" && v != "medium" && v != "high" && v != "full")
            o.level = {"invalid", 0, "malformed"};
    } else if (o.level.kind != "invalid" && o.level.kind != "unavailable") {
        o.level = {"invalid", 0, "malformed"};
    }
    if (o.charging != "charging" && o.charging != "discharging" && o.charging != "full" && o.charging != "not_charging") o.charging = "unknown";
    if (o.connection != "connected" && o.connection != "disconnected") o.connection = "unknown";
    // Explicit denial is not a disconnection and cannot provide a usable level.
    // Preserve invalid diagnostics, especially unsupported_version (higher precedence).
    if (permissions_[o.source].state == "denied" && o.level.kind != "invalid" && o.level.kind != "unavailable") {
        o.level = {"unavailable", 0, "permission_denied"};
        o.charging = "unknown"; o.connection = "unknown";
    }
    observations_[key] = std::move(o);
    episodes_[key].pending = false;
    return true;
}
State Battery::project(const std::string& device, const std::string& component,
                       std::int64_t now_ms, const std::string& epoch) const {
    State s; s.device = device; s.component = component;
    const auto life = lifecycle_.find({device, component});
    if (life != lifecycle_.end()) {
        s.rejected_ids = life->second.rejected;
        if (life->second.removed) return s;
    }
    const auto it = observations_.find({device, component});
    if (it == observations_.end()) return s;
    const auto& o = it->second;
    s.endpoint_id = o.endpoint;
    if (const auto d = devices_.find(device); d != devices_.end()) {
        s.identity_confidence = d->second.identity_confidence;
        s.parent_device_id = d->second.parent_device_id; s.slot = d->second.slot;
    }
    s.present = true; s.level = o.level; s.charging = o.charging; s.connection = o.connection;
    if (life != lifecycle_.end() && life->second.sleep_invalidated) {
        s.suppression = "sleep_invalidated"; return s;
    }
    if (source_epochs_.at(o.source) != o.source_epoch) {
        s.suppression = "unknown_freshness"; return s;
    }
    const auto permission = permissions_.find(o.source);
    if (permission != permissions_.end() && life != lifecycle_.end() &&
        permission->second.revision != life->second.permission_revision) {
        if (permission->second.state == "denied") {
            s.level = {"unavailable", 0, "permission_denied"};
            s.suppression = "unavailable";
        } else s.suppression = "unknown_freshness";
        return s;
    }
    const auto freshness = [&](std::optional<std::int64_t> observed) -> std::string {
        if (!observed || *observed < 0 || now_ms < *observed || epoch != o.monotonic_epoch ||
            (sleep_cutoff_ && *observed < *sleep_cutoff_)) return "unknown";
        return now_ms - *observed <= ttl_ ? "fresh" : "stale";
    };
    s.level_freshness = freshness(o.observed_ms);
    s.charging_freshness = s.charging == "unknown" ? "unknown" : freshness(o.charging_observed_ms ? o.charging_observed_ms : o.observed_ms);
    s.connection_freshness = s.connection == "unknown" ? "unknown" : freshness(o.connection_observed_ms ? o.connection_observed_ms : o.observed_ms);
    if (s.connection == "disconnected") {
        s.level_freshness = s.charging_freshness = "stale";
        s.suppression = "disconnected";
    } else if (s.level.kind == "invalid") s.suppression = "invalid";
    else if (s.level_freshness == "unknown") s.suppression = "unknown_freshness";
    else if (s.level.kind == "unavailable") s.suppression = "unavailable";
    else if (s.level_freshness == "stale" || s.connection_freshness == "stale") s.suppression = "stale";
    else if (s.connection_freshness == "unknown") s.suppression = "unknown_freshness";
    else if (s.connection != "connected") s.suppression = "no_current_level";
    else if (s.level.kind == "qualitative") s.suppression = "qualitative_unmapped";
    else { s.alert_eligible = true; s.suppression.clear(); }
    return s;
}
void Battery::source_permission(const std::string& source, const std::string& state) {
    if (source.empty() || (state != "granted" && state != "denied")) throw std::invalid_argument("invalid source permission");
    auto& permission = permissions_[source];
    if (permission.state == state) return;
    permission.state = state; ++permission.revision;
    for (const auto& [key, entry] : lifecycle_) if (entry.source == source) episodes_[key].pending = false;
}
void Battery::register_device(Device device) {
    if (device.id.empty()) throw std::invalid_argument("empty device id");
    if (device.identity_confidence != "confirmed" && device.identity_confidence != "probable" && device.identity_confidence != "unconfirmed")
        throw std::invalid_argument("invalid identity confidence");
    const auto it = devices_.find(device.id);
    if (it != devices_.end()) {
        if (it->second.identity_confidence != device.identity_confidence || it->second.parent_device_id != device.parent_device_id || it->second.slot != device.slot)
            throw std::invalid_argument("identity change not implemented");
        return;
    }
    devices_.emplace(device.id, std::move(device));
}
bool Battery::remove(const std::string& source, const std::string& device,
                     const std::string& component, std::uint64_t generation,
                     const std::string& source_epoch) {
    if (source.empty() || device.empty() || component.empty()) throw std::invalid_argument("missing identity");
    auto epoch = source_epochs_.find(source);
    if (epoch == source_epochs_.end()) {
        if (source_epoch.empty()) throw std::invalid_argument("removal needs source epoch");
        epoch = source_epochs_.emplace(source, source_epoch).first;
    }
    if (!source_epoch.empty() && source_epoch != epoch->second) return false;
    const Key key{device, component};
    const auto it = lifecycle_.find(key);
    if (it != lifecycle_.end()) {
        if (it->second.source != source) throw std::invalid_argument("multi-source not implemented");
        if (it->second.epoch == epoch->second && generation <= it->second.generation) return false;
    }
    auto& e = lifecycle_[key];
    e.source = source; e.epoch = epoch->second; e.generation = generation; e.removed = true;
    episodes_[key].pending = false;
    return true;
}
void Battery::change_source_epoch(const std::string& source, const std::string& old_epoch, const std::string& new_epoch) {
    if (source.empty() || new_epoch.empty() || new_epoch == old_epoch) throw std::invalid_argument("invalid epoch transition");
    const auto it = source_epochs_.find(source);
    if (it == source_epochs_.end() || it->second != old_epoch) throw std::invalid_argument("unexpected old epoch");
    it->second = new_epoch;
    for (const auto& [key, e] : lifecycle_) if (e.source == source) episodes_[key].pending = false;
}
void Battery::sleep(std::int64_t now_ms) {
    if (now_ms < 0) throw std::invalid_argument("negative sleep time");
    sleeping_ = true; sleep_cutoff_ = now_ms;
    for (auto& [key, e] : lifecycle_) { e.sleep_invalidated = true; episodes_[key].pending = false; }
}
void Battery::resume(std::int64_t now_ms) {
    if (now_ms < 0 || (sleep_cutoff_ && now_ms < *sleep_cutoff_)) throw std::invalid_argument("invalid resume time");
    sleeping_ = false; sleep_cutoff_ = now_ms;
}
Notification Battery::evaluate(const State& s) {
    auto& e = episodes_[{s.device, s.component}];
    e.pending = false;
    Notification n;
    // Deterministic precedence matching F18/F20; no multi-source inference.
    if (!config_.ratified) n.blocked_reason = "policy_pending";
    else if (!s.alert_eligible) n.blocked_reason = "ineligible";
    else if (!config_.enabled) n.blocked_reason = "alerts_disabled";
    else if (!config_.permission) n.blocked_reason = "permission_denied";
    else {
        // Avoid overflow even when caller configures large nonnegative hysteresis.
        const int rearm = config_.threshold + std::min(config_.hysteresis, 100 - config_.threshold);
        if (s.level.percent >= rearm) {
            e.notified = false; n.rearm_applied = true; n.blocked_reason = "at_or_above_rearm";
        } else if (s.level.percent >= config_.threshold) {
            n.blocked_reason = s.level.percent == config_.threshold ? "at_or_above_threshold" : "hysteresis_not_reached";
        } else if (s.charging == "charging") n.blocked_reason = "charging";
        else if (e.notified) n.blocked_reason = "already_notified";
        else { n.requested = true; e.pending = true; }
    }
    return n;
}
void Battery::delivery(const State& s, const std::string& outcome) {
    auto& e = episodes_[{s.device, s.component}];
    if (e.pending && outcome == "delivered") e.notified = true;
    e.pending = false;
}
bool Battery::episode(const State& s) const {
    const auto it = episodes_.find({s.device, s.component});
    return it != episodes_.end() && it->second.notified;
}
void Battery::restore_episode(const std::string& device, const std::string& component, bool notified) {
    episodes_[{device, component}] = {notified, false};
}
}
