#include "shadowrun_ghidra/entity_delay_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint16_t average_entity_delay_attributes(
    const std::uint8_t primary,
    const std::uint8_t secondary) {
    const auto wrapped_sum = static_cast<std::uint8_t>(primary + secondary);
    return static_cast<std::uint16_t>(wrapped_sum >> 1U);
}

std::uint16_t compute_entity_delay_factor(
    const EntityDelayAttributes& attributes) {
    std::uint16_t modifier = 0;
    for (std::uint16_t bit = 12; bit <= 14; ++bit) {
        if ((attributes.capability_flags &
             static_cast<std::uint16_t>(1U << bit)) != 0U) {
            modifier = static_cast<std::uint16_t>((bit - 11U) * 4U);
            break;
        }
    }

    const auto average = average_entity_delay_attributes(
        attributes.primary, attributes.secondary);
    const auto adjusted = static_cast<std::int32_t>(average) + modifier - 1;
    return adjusted > 0 ? static_cast<std::uint16_t>(adjusted) : 0U;
}

EntityDelayScale decode_entity_delay_scale(const std::uint16_t source_mode) {
    if (source_mode == 0U) {
        return EntityDelayScale::Normal;
    }
    if (source_mode == 2U) {
        return EntityDelayScale::Double;
    }
    return EntityDelayScale::Half;
}

void update_entity_animation_delay(
    EntityDelayState& state,
    const EntityDelayAttributes& attributes,
    const EntityDelayScale scale) {
    const auto factor = compute_entity_delay_factor(attributes);
    auto delay = static_cast<std::int32_t>(0x6A) -
                 static_cast<std::int32_t>(factor) * 4;
    if (scale == EntityDelayScale::Double) {
        delay *= 2;
    } else if (scale == EntityDelayScale::Half) {
        delay >>= 1;
    }

    if (delay < 0x0F) {
        delay = 0x0F;
    }
    state.animation_delay = static_cast<std::int16_t>(delay);
}

} // namespace shadowrun::ghidra_conversion
