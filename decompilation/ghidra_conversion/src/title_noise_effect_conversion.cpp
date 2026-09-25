#include "shadowrun_ghidra/title_noise_effect_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_word_until_negative(std::uint16_t& value) {
    value = static_cast<std::uint16_t>(value - 1U);
    return (value & 0x8000U) != 0U;
}

std::uint32_t select_noise_table(std::uint16_t& random_word) {
    const auto rotate_right = [&]() {
        const bool carry = (random_word & 1U) != 0U;
        random_word = static_cast<std::uint16_t>(
            (random_word >> 1U) | (carry ? 0x8000U : 0U));
        return carry;
    };

    if (rotate_right()) {
        if (rotate_right()) {
            return 0x0001DAD6U;
        }
        return 0x0001DAC2U;
    }
    if (rotate_right()) {
        return 0x0001DAccU;
    }
    return 0x0001DAB8U;
}

bool build_sprite(const std::uint32_t table, const std::uint16_t x,
                  const std::uint16_t y,
                  const TitleNoiseEffectCallbacks& callbacks) {
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(table)
                           : std::nullopt;
    if (!token.has_value()) {
        return false;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleNoiseEffectSpriteBuild{
            table, x, y, 0x2000U, *token});
    }
    return true;
}

} // namespace

TitleNoiseEffectAction advance_title_noise_effect(
    TitleNoiseEffectState& state,
    const TitleNoiseEffectCallbacks& callbacks) {
    if ((state.behavior_flags & 0x04U) != 0U) {
        if (decrement_word_until_negative(state.countdown)) {
            state.behavior_flags = static_cast<std::uint8_t>(
                state.behavior_flags & ~0x05U);
            if (callbacks.attach_behavior_descriptor_e) {
                callbacks.attach_behavior_descriptor_e();
            }
            state.transition_state = 0U;
            state.completion_state = 1U;
            if (callbacks.refresh_menu_selection_after_clear) {
                callbacks.refresh_menu_selection_after_clear();
            }
            return TitleNoiseEffectAction::finalized;
        }

        if ((state.countdown & 0x0007U) < 4U) {
            return TitleNoiseEffectAction::updated;
        }
        if (!build_sprite(0x0001DAE0U, 0x0158U, 0x00FCU, callbacks) ||
            !build_sprite(0x0001DAEAU, 0x00E7U, 0x00FCU, callbacks)) {
            return TitleNoiseEffectAction::sprite_data_unresolved;
        }
        return TitleNoiseEffectAction::rendered;
    }

    if (decrement_word_until_negative(state.countdown)) {
        state.behavior_flags = static_cast<std::uint8_t>(
            state.behavior_flags | 0x04U);
        state.countdown = 0x003CU;
        return TitleNoiseEffectAction::updated;
    }
    if (!callbacks.advance_source_prng) {
        return TitleNoiseEffectAction::prng_unresolved;
    }
    const auto random_word = callbacks.advance_source_prng();
    if (!random_word.has_value()) {
        return TitleNoiseEffectAction::prng_unresolved;
    }
    state.random_word = *random_word;

    for (std::uint16_t index = 0; index < 7U; ++index) {
        const auto table = select_noise_table(state.random_word);
        if (!build_sprite(table, 0x00E8U, 0x00ECU, callbacks)) {
            return TitleNoiseEffectAction::sprite_data_unresolved;
        }
    }
    return TitleNoiseEffectAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
