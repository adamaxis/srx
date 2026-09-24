#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityDelayAttributes {
    std::uint8_t primary = 0;
    std::uint8_t secondary = 0;
    std::uint16_t capability_flags = 0;
};

// ROM 0xD81C: average the two byte-sized entity delay attributes. The source
// adds them as bytes before shifting, so sums wrap at 256.
std::uint16_t average_entity_delay_attributes(
    std::uint8_t primary,
    std::uint8_t secondary);

// ROM 0xD7EA: choose the first active capability bit in source positions
// 12..14, convert it to a 4/8/12 modifier, add it to the attribute average,
// subtract one, and clamp the factor at zero.
std::uint16_t compute_entity_delay_factor(
    const EntityDelayAttributes& attributes);

enum class EntityDelayScale {
    Normal,
    Double,
    Half,
};

// ROM 0xABFC: decode the raw mode word before applying its scaling rule.
EntityDelayScale decode_entity_delay_scale(std::uint16_t source_mode);

struct EntityDelayState {
    std::int16_t animation_delay = 0;
};

// ROM 0xABFC: turn the factor into the entity's animation delay using the
// source 0x6A base, mode scaling, and minimum delay of 15.
void update_entity_animation_delay(
    EntityDelayState& state,
    const EntityDelayAttributes& attributes,
    EntityDelayScale scale);

} // namespace shadowrun::ghidra_conversion
