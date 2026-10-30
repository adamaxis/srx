#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion::menus {

// Semantic section identity used by the command menu and its presentation
// adapter. The ROM's numeric selector is translated only at the legacy
// adapter boundary.
enum class CommandSection : std::uint8_t {
    inventory_left,
    inventory_right,
    clips,
    attributes_and_skills,
    cyberware,
    magic,
    pocket_secretary,
};

} // namespace shadowrun::ghidra_conversion::menus
