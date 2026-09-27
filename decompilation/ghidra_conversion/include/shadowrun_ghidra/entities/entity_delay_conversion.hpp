#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityDelayAttributes {
    // 0xD81C reads A0+0x78 with MOVE.B and contributes it to D1.
    std::uint8_t primary_delay_attribute = 0;
    // 0xD81C reads A0+0x7B with ADD.B, after which D1 is shifted once.
    std::uint8_t secondary_delay_attribute = 0;
    // 0xD7EA reads A0+0x90 with MOVE.W and scans source bits 12..14.
    std::uint16_t capability_flags = 0;
};

// ROM 0xD81C: clear D1, add the entity record bytes at A0+0x78 and A0+0x7B
// with byte arithmetic, then return the floor average in D1. The source adds
// them as bytes before shifting, so sums wrap at 256.
std::uint16_t average_entity_delay_attributes(
    std::uint8_t primary_delay_attribute,
    std::uint8_t secondary_delay_attribute);

// ROM 0xD7EA: load the entity capability word from A0+0x90, choose the first
// active capability bit in source positions 12..14, convert it to a 4/8/12
// modifier in D2, call 0xD81C for the wrapped attribute average in D1, then
// add the modifier, subtract one, and clamp the returned factor at zero.
std::uint16_t compute_entity_delay_factor(
    const EntityDelayAttributes& attributes);

enum class EntityDelayScale {
    Normal,
    Double,
    Half,
};

// ROM 0xABFC: decode the raw D0 mode word before applying its scaling rule.
// Zero is normal, two doubles the delay, and every other nonzero mode halves
// it.
EntityDelayScale decode_entity_delay_scale(std::uint16_t source_mode);

struct EntityDelayState {
    // Destination written by ROM 0xABFC at A0+0xDC.
    std::int16_t animation_delay = 0;
};

// ROM 0xABFC: preserve the caller's D1/D2, call 0xD7EA for the factor, turn
// it into 0x6A - factor*4, apply the D0 mode scaling, and clamp the signed
// result to 15 before writing the entity delay at A0+0xDC. The source's bit-1
// test at A0+0x10 has no divergent branch, so it has no native state effect.
void update_entity_animation_delay(
    EntityDelayState& state,
    const EntityDelayAttributes& attributes,
    EntityDelayScale scale);

} // namespace shadowrun::ghidra_conversion
