#pragma once
#include "../domain/battery.hpp"
#include <optional>
#include <string>
#include <vector>

namespace perivolt::ui {
enum class Origin { session_observation, historical_import };
enum class Consent { off, requested, granted, denied };
enum class Permission { unknown, granted, denied };
struct ReadControl {
    Consent consent = Consent::off;
    Permission permission = Permission::unknown;
    bool capability_available = false;
    bool producer_connected = false;
    bool active = false;
};
struct DeviceReading {
    std::string name;
    State state;
    std::optional<std::int64_t> age_ms;
    Origin origin = Origin::session_observation;
    std::string source;
};
struct Snapshot {
    std::vector<DeviceReading> devices;
    ReadControl control;
};
struct DiscoverySummary {
    bool complete = false;
    std::int32_t os_error = 0;
    unsigned candidates = 0;
    unsigned family_endpoints = 0;
};
std::string present_discovery(bool supported, const std::optional<DiscoverySummary>& summary);
struct DeviceRow {
    std::string device, component, name, level, diagnostic, age, charging, connection;
    std::string level_freshness, charging_freshness, connection_freshness, source;
};
struct View {
    std::vector<DeviceRow> rows;
    std::string status, read_explanation;
    bool read_enabled = false, read_checked = false;
};
// Presentation only. No timers, I/O, alert evaluation or fabricated timestamps.
DeviceRow present_reading(const DeviceReading& reading);
View present(const Snapshot& snapshot, bool read_intent_connected);
// A completion is accepted once, only for the user's latest explicit file choice.
class HistoricalImport {
public:
    std::uint64_t begin();
    bool complete(std::uint64_t request, std::vector<DeviceReading> readings);
    const std::vector<DeviceReading>& readings() const { return readings_; }
private:
    std::uint64_t sequence_ = 0, pending_ = 0;
    std::vector<DeviceReading> readings_;
};
}
