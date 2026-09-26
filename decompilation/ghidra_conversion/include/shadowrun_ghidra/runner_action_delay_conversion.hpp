#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// The values consumed by ROM 0x55F44. These are source record fields and
// menu/profile bytes expressed as ordinary host data; no emulated address is
// part of the native conversion boundary.
struct RunnerActionDelayInput {
    std::uint8_t active_record_attribute_a = 0; // FFFC04 + 0x78
    std::uint8_t active_record_attribute_b = 0; // FFFC04 + 0x7B
    std::uint8_t active_record_modifier = 0;    // FFFC04 + 0x88
    std::uint8_t profile_rounding_marker = 0;   // FFFBBE + 0x07
    std::uint8_t profile_floor = 0;              // FFFBBE + 0x00
    std::int8_t mode_result = 0;                 // FFF0B7
    std::int16_t request = 0;                    // D1 at 0x55F44 entry
};

struct RunnerActionDelayResult {
    std::uint16_t delay = 0; // D6 returned by 0x55F44
};

// ROM 0x55F44: combine the active runner's two delay attributes with its
// record/profile modifiers, then apply the source mode-result scaling. A
// zero request uses the normal/one-third path; a nonzero request selects the
// rounded/two-thirds path. The positive mode-result branch applies the
// profile-floor clamp for any nonzero request, while the other branches apply
// it for a negative request.
RunnerActionDelayResult calculate_runner_action_delay(
    const RunnerActionDelayInput& input);

} // namespace shadowrun::ghidra_conversion
