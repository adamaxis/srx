#include "shadowrun_ghidra/scene_profile_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool scene_uses_alternate_profile(const std::int16_t scene_id) {
    return (scene_id >= 6 && scene_id <= 9) ||
           (scene_id >= 0x2C && scene_id <= 0x31);
}

} // namespace shadowrun::ghidra_conversion
