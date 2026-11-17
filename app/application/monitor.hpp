#pragma once
#include "../ui/presenter.hpp"
#include "../adapters/attackshark-x11/platform/macos/producer.hpp"
#include <future>
#include <map>
#include <mutex>
namespace perivolt::application {
struct SessionToken { std::uint64_t generation=0; bool opt_in=false; };
enum class SampleOrigin { session_observation, metadata, synthetic, cache, historical_import };
struct Sample {
    Device identity;
    std::string name;
    Observation observation;
    SampleOrigin origin=SampleOrigin::metadata;
};
// Thread-safe handoff only: no I/O, worker, hardware calls or alert evaluation.
// Native producer must supply actual monotonic observation time in snapshot's epoch.
class Monitor {
public:
    explicit Monitor(std::int64_t ttl_ms=30000);
    ~Monitor();
    SessionToken request_read(bool enabled);
    // Registry-only metadata, asynchronous on Apple; no HID or battery transport.
    void refresh_metadata();
    bool submit_metadata(const attackshark_x11::macos::NativeSample& sample);
    std::optional<attackshark_x11::macos::NativeSample> metadata() const;
    void cancel();
    void shutdown();
    void source_status(bool capability_available, ui::Permission permission);
    bool submit(SessionToken token, const Sample& sample);
    ui::Snapshot snapshot(std::int64_t now_ms, const std::string& monotonic_epoch) const;
private:
    struct Entry { std::string name, source, epoch; std::optional<std::int64_t> observed; };
    using Key=std::pair<std::string,std::string>;
    void clear_locked();
    mutable std::mutex mutex_;
    std::int64_t ttl_;
    Battery battery_;
    std::map<Key,Entry> entries_;
    ui::ReadControl control_;
    std::uint64_t generation_=0;
    bool closed_=false;
    std::optional<attackshark_x11::macos::NativeSample> metadata_;
    std::future<void> metadata_task_;
};
}
