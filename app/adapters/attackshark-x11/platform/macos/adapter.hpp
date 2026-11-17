#pragma once
#include "../../battery_parser.hpp"
#include <array>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace perivolt::attackshark_x11::macos {
using WallTime = std::chrono::system_clock::time_point;
inline constexpr std::string_view metadata_source = "macos.ioregistry.x11.metadata.experimental";
enum class Freshness { unknown };
enum class Origin { metadata, synthetic, cache };
enum class Reason { consent_required, native_validation_required, target_unverified,
                    ambiguous_target, timeout, cancelled, io_error, malformed_report,
                    report_limit, synthetic_battery, cached };
struct Observation {
    std::optional<std::uint8_t> percent;
    Charging charging = Charging::unknown;
    Freshness freshness = Freshness::unknown;
    Origin origin = Origin::metadata;
    Reason reason = Reason::native_validation_required;
    std::string_view source = metadata_source;
    std::string_view protocol = protocol_source;
    WallTime received_at{};
    std::optional<WallTime> observed_at;
    bool live = false;
};
struct EndpointMetadata {
    std::optional<std::uint32_t> vendor, product, usage_page, usage, max_input_bytes;
    bool candidate() const noexcept {
        return vendor == 0x1d57 && product == 0xfa60 && usage_page == 0x0a && usage == 0
            && max_input_bytes == 5;
    }
};
struct Discovery {
    std::vector<EndpointMetadata> endpoints;
    bool complete = false;
    std::int32_t os_error = 0;
    WallTime received_at{};
};
// Defined only in discovery_macos.cpp. No report I/O is implemented.
[[nodiscard]] Discovery discover_metadata();
struct Consent { bool explicit_report_read = false; bool consumed = false; };
[[nodiscard]] inline Reason native_read_gate(const Consent& consent) noexcept {
    return consent.explicit_report_read && !consent.consumed ?
        Reason::native_validation_required : Reason::consent_required;
}
[[nodiscard]] inline Observation as_cache(Observation value, WallTime received_at) {
    value.origin = Origin::cache; // Preserve the actual source and protocol provenance.
    value.reason = Reason::cached; value.received_at = received_at;
    value.freshness = Freshness::unknown; value.live = false;
    return value;
}
enum class ReadStatus { complete, timeout, cancelled, error };
struct ReadResult { ReadStatus status; std::size_t bytes = 0; };
struct SessionResult { Observation observation; unsigned reports = 0; };
// Deliberately FAKE-ONLY. An Apple transport cannot be bound to this function
// without a separate implementation/review. Frame contract is canonical 5 bytes
// INCLUDING report ID; no claim about Apple's callback buffer framing.
template<class T> requires requires { requires T::synthetic_transport; }
SessionResult run_fake_session(T& transport, Consent& consent, const EndpointMetadata& endpoint,
                              bool unique, bool privacy_validated, WallTime received_at) {
    SessionResult result;
    auto& observation = result.observation;
    observation.origin = Origin::synthetic;
    observation.source = "macos.x11.boundary.fake";
    observation.received_at = received_at;
    if (!consent.explicit_report_read || consent.consumed) {
        observation.reason = Reason::consent_required; return result;
    }
    if (!endpoint.candidate()) { observation.reason=Reason::target_unverified; return result; }
    if (!unique) { observation.reason=Reason::ambiguous_target; return result; }
    if (!privacy_validated) { observation.reason=Reason::native_validation_required; return result; }
    consent.consumed = true;
    struct Close { T& t; ~Close(){t.cancel_close();} } close{transport};
    const auto start = transport.now_ms();
    // Reject impossible monotonic clock values instead of wrapping the deadline.
    if (start > UINT64_MAX-8000) { observation.reason=Reason::timeout; return result; }
    const auto deadline = start+8000;
    for (unsigned i=0;i<32;++i) {
        if (transport.cancelled()) { observation.reason=Reason::cancelled; return result; }
        if (transport.now_ms()>=deadline) { observation.reason=Reason::timeout; return result; }
        struct Scratch {
            std::array<std::uint8_t,5> bytes{};
            ~Scratch(){volatile auto* p=bytes.data();for(unsigned j=0;j<5;++j)p[j]=0;}
        } frame;
        const auto read = transport.read(std::span<std::uint8_t,5>{frame.bytes},deadline);
        if (transport.cancelled() || read.status==ReadStatus::cancelled) {
            observation.reason=Reason::cancelled; return result;
        }
        if (transport.now_ms()>=deadline || read.status==ReadStatus::timeout) {
            observation.reason=Reason::timeout; return result;
        }
        if (read.status!=ReadStatus::complete) { observation.reason=Reason::io_error; return result; }
        ++result.reports;
        if (read.bytes!=5 || frame.bytes[0]!=3 || frame.bytes[1]!=0x55) {
            observation.reason=Reason::malformed_report; return result;
        }
        const auto decoded = perivolt::attackshark_x11::parse(frame.bytes);
        if (decoded.percent) {
            observation.percent=decoded.percent; observation.reason=Reason::synthetic_battery;
            // Synthetic receipt does not establish a real hardware observation time.
            return result;
        }
    }
    observation.reason=Reason::report_limit;
    return result;
}
}
