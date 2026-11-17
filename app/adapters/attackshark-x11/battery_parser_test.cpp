#include "battery_parser.hpp"

#include <array>
#include <iostream>

namespace x11 = perivolt::attackshark_x11;
int main() {
    int failures = 0;
    int checks = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::cerr << "FAIL: " << name << '\n'; }
    };
    // ALL frames below are SYNTHETIC fixtures, never captured hardware data.
    std::array<std::uint8_t, 5> frame{0x03, 0x55, 0x40, 0x01, 0};
    for (unsigned value = 0; value <= 255; ++value) {
        frame[4] = static_cast<std::uint8_t>(value);
        const auto r = x11::parse(frame);
        check(r.percent.has_value() == (value <= 100), "percent domain");
        check(r.reason == (value <= 100 ? x11::Reason::decoded :
                          x11::Reason::percent_out_of_range), "percent reason");
        if (value <= 100) check(r.percent == value, "exact percent including 0/100");
        check(r.charging == x11::Charging::unknown, "charging unknown");
        check(r.provenance == x11::protocol_source, "pinned provenance");
    }
    frame[4] = 50;
    for (std::size_t size = 0; size < frame.size(); ++size) {
        const auto r = x11::parse(std::span{frame}.first(size));
        check(!r.percent && r.reason == x11::Reason::truncated, "every truncated prefix");
    }
    for (unsigned state = 0; state <= 255; ++state) {
        frame[3] = static_cast<std::uint8_t>(state);
        const auto r = x11::parse(frame);
        check(r.percent == 50 && r.charging == x11::Charging::unknown,
              "state never invents charging or overrides percent");
    }
    frame = {0x02, 0x55, 0x40, 0x01, 50};
    auto r = x11::parse(frame);
    check(!r.percent && r.reason == x11::Reason::wrong_report, "wrong report");
    for (const auto model : {0x07, 0xBE, 0x00}) {
        frame = {0x03, static_cast<std::uint8_t>(model), 0x40, 0x01, 50};
        r = x11::parse(frame);
        check(!r.percent && r.reason == x11::Reason::unsupported_model, "other model");
    }
    for (const auto event : {0x10, 0x20, 0x50, 0x60, 0x00}) {
        frame = {0x03, 0x55, static_cast<std::uint8_t>(event), 0x01, 50};
        r = x11::parse(frame);
        check(!r.percent && r.reason == x11::Reason::wrong_event, "other event");
    }
    const std::array<std::uint8_t, 8> padded{0x03, 0x55, 0x40, 0x03, 100, 255, 255, 255};
    check(x11::parse(padded).percent == 100, "trailing bytes ignored by prefix contract");
    std::cout << "SYNTHETIC parser checks: " << checks << ", failures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
