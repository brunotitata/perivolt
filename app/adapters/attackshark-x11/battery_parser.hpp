#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace perivolt::attackshark_x11 {
enum class Charging { unknown };
enum class Reason { decoded, truncated, wrong_report, unsupported_model, wrong_event,
                    percent_out_of_range };

inline constexpr std::string_view protocol_source =
    "https://github.com/MiguelSanzBr/attackshark-x11-linux/blob/"
    "e13a99644dcf3538d8cc5908a07832318b49cf2d/docs/PROTOCOL.md#4-input-report-0x03-question-4";

struct BatteryResult {
    std::optional<std::uint8_t> percent;
    Charging charging = Charging::unknown;
    Reason reason = Reason::truncated;
    std::string_view provenance = protocol_source;
};

// Input includes report ID. Decodes only the documented five-byte prefix.
// No transport inference, discovery, I/O, device support or freshness claim.
[[nodiscard]] BatteryResult parse(std::span<const std::uint8_t> payload) noexcept;
} // namespace perivolt::attackshark_x11
