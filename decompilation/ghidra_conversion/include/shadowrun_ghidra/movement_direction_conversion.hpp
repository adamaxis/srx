#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct DominantMovementDirections {
    std::int8_t primary = 0;
    std::int8_t secondary = 0;
};

// ROM 0x148D2: choose the source direction slot for the dominant axis first,
// then the other axis. The four returned slots are the source's signed-delta
// choices: horizontal positive/negative are 6/2 and vertical positive/negative
// are 0/4.
DominantMovementDirections choose_dominant_axis_directions(
    std::int32_t origin_x,
    std::int32_t origin_y,
    std::int32_t target_x,
    std::int32_t target_y);

} // namespace shadowrun::ghidra_conversion
