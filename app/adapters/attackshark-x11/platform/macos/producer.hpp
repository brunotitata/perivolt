#pragma once
#include "adapter.hpp"

namespace perivolt::attackshark_x11::macos {
// Caller owns generations. OFF/new activation invalidates older results.
struct SessionToken {
    std::uint64_t generation = 0;
    bool opt_in = false;
    bool operator==(const SessionToken&) const = default;
};
enum class MetadataPresence { unknown, absent, candidate, ambiguous };
struct NativeSample {
    SessionToken token;
    Observation observation;
    MetadataPresence presence = MetadataPresence::unknown;
    unsigned candidate_count = 0;
    unsigned family_endpoint_count = 0;
    bool enumeration_complete = false;
    std::int32_t os_error = 0;
    // Metadata does not establish identity, connection or measurement time.
    bool read_available = false;
    bool active = false;
};
[[nodiscard]] inline bool accepts(SessionToken current, SessionToken result) noexcept {
    return current.opt_in && current == result;
}
[[nodiscard]] inline NativeSample from_discovery(const Discovery& discovery, SessionToken token) {
    NativeSample sample;
    sample.token=token;
    sample.enumeration_complete=discovery.complete;
    sample.os_error=discovery.os_error;
    sample.observation.received_at=discovery.received_at;
    sample.observation.reason=native_read_gate({token.opt_in,false});
    for(const auto& endpoint:discovery.endpoints) {
        if(endpoint.vendor!=0x1d57 || endpoint.product!=0xfa60)continue;
        ++sample.family_endpoint_count;
        if(endpoint.candidate())++sample.candidate_count;
    }
    if(discovery.complete && discovery.os_error==0) {
        if(sample.family_endpoint_count==0) sample.presence=MetadataPresence::absent;
        else if(sample.candidate_count==1) sample.presence=MetadataPresence::candidate;
        else if(sample.candidate_count>1) sample.presence=MetadataPresence::ambiguous;
    }
    // Security gate is deliberately unconditional: no runtime switch enables I/O.
    // Token only records the request; neither consent nor metadata proves safety.
    return sample;
}
// Native macOS registry-only producer; no device open or report transport.
[[nodiscard]] NativeSample poll_native_metadata(SessionToken token = {});
}
