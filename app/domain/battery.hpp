#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
namespace perivolt {
struct Level {
    std::string kind = "unavailable";
    double percent = 0; // meaningful ONLY for reported_percent
    std::string detail = "not_reported"; // qualitative value or diagnostic reason
};
struct Device {
    std::string id, identity_confidence = "unconfirmed", parent_device_id = "", slot = "";
};
struct Observation {
    std::string device, component, source, source_epoch;
    std::uint64_t generation = 0;
    int payload_version = 1;
    Level level;
    std::string charging = "unknown", connection = "unknown";
    std::optional<std::int64_t> observed_ms;
    std::string monotonic_epoch;
    std::string id = "", endpoint = "";
    std::optional<std::int64_t> charging_observed_ms = std::nullopt, connection_observed_ms = std::nullopt;
};
struct AlertConfig {
    int threshold = 20;
    int hysteresis = 5;
    bool enabled = true, permission = true, ratified = false;
};
struct State {
    std::string device, component;
    bool present = false;
    std::string identity_confidence = "unconfirmed", endpoint_id = "", parent_device_id = "", slot = "";
    std::vector<std::string> rejected_ids;
    Level level;
    std::string charging = "unknown", connection = "unknown";
    std::string level_freshness = "unknown", charging_freshness = "unknown", connection_freshness = "unknown";
    bool alert_eligible = false;
    std::string suppression = "no_current_level";
};
struct Notification {
    bool requested = false;
    std::string blocked_reason;
    bool rearm_applied = false;
};
// First increment: one source per canonical device/component; multi-source is rejected.
// Caller provides trusted monotonic epoch/time. No wall clock or OS calls.
class Battery {
public:
    explicit Battery(std::int64_t ttl_ms);
    void configure(AlertConfig config);
    void set_threshold(int threshold);
    bool observe(Observation observation);
    void register_device(Device device);
    void source_permission(const std::string& source, const std::string& state);
    bool remove(const std::string& source, const std::string& device,
                const std::string& component, std::uint64_t generation,
                const std::string& source_epoch = "");
    void change_source_epoch(const std::string& source, const std::string& old_epoch,
                             const std::string& new_epoch);
    void sleep(std::int64_t now_ms);
    void resume(std::int64_t now_ms);
    State project(const std::string& device, const std::string& component,
                  std::int64_t now_ms, const std::string& epoch) const;
    Notification evaluate(const State& state);
    void delivery(const State& state, const std::string& outcome);
    bool episode(const State& state) const;
    void restore_episode(const std::string& device, const std::string& component, bool notified);
private:
    using Key = std::pair<std::string, std::string>;
    struct Episode { bool notified = false, pending = false; };
    struct Lifecycle {
        std::string source, epoch;
        std::uint64_t generation = 0;
        bool removed = false, sleep_invalidated = false;
        std::vector<std::string> rejected;
        std::uint64_t permission_revision = 0;
    };
    struct SourcePermission { std::string state = "unknown"; std::uint64_t revision = 0; };
    std::map<std::string, SourcePermission> permissions_;
    bool sleeping_ = false;
    std::optional<std::int64_t> sleep_cutoff_;
    std::map<std::string, Device> devices_;
    std::map<std::string, std::string> source_epochs_;
    std::map<Key, Lifecycle> lifecycle_;
    std::int64_t ttl_;
    AlertConfig config_;
    std::map<Key, Observation> observations_;
    std::map<Key, Episode> episodes_;
};
}
