#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0x03032: identify the scene IDs that use the alternate field profile.
// The source accepts 6..9 and 0x2C..0x31, inclusive.
bool scene_uses_alternate_profile(std::int16_t scene_id);

} // namespace shadowrun::ghidra_conversion
