#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// Accessory bit positions stored in an inventory item's phase byte. These
// meanings are established by the ROM purchase path at 0x57196-0x571AA.
enum class UsWeaponAccessory : std::uint8_t {
    smartgun_system = 0U,
    silencer = 1U,
    sound_suppressor = 2U,
    laser_sight = 3U,
    gas_vent_ii = 4U,
    gas_vent_iii = 5U,
};

constexpr std::uint8_t weapon_accessory_flag(
    const UsWeaponAccessory accessory) noexcept {
    return static_cast<std::uint8_t>(
        1U << static_cast<unsigned>(accessory));
}

} // namespace shadowrun::ghidra_conversion
