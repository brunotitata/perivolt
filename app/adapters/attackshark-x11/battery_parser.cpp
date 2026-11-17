#include "battery_parser.hpp"

namespace perivolt::attackshark_x11 {
BatteryResult parse(std::span<const std::uint8_t> payload) noexcept {
    BatteryResult result;
    if (payload.size() < 5) return result;
    if (payload[0] != 0x03) {
        result.reason = Reason::wrong_report;
    } else if (payload[1] != 0x55) {
        result.reason = Reason::unsupported_model;
    } else if (payload[2] != 0x40) {
        result.reason = Reason::wrong_event;
    } else if (payload[4] > 100) {
        result.reason = Reason::percent_out_of_range;
    } else {
        result.percent = payload[4];
        result.reason = Reason::decoded;
    }
    return result;
}
}
