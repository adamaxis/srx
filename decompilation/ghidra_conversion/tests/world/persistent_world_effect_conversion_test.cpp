#include "shadowrun_ghidra/world/persistent_world_effect_conversion.hpp"

#include <array>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    PersistentWorldEffectState state{};
    state.record.active = 0xFFU;
    state.record.lifetime = 0x3CU;
    state.record.frame_delay = 1U;
    state.record.phase_delay = 1U;
    state.record.source_variant = 2U;
    state.record.animation_phase = 1U;
    state.record.position_x = static_cast<std::int16_t>(0x1230);
    state.record.position_y = static_cast<std::int16_t>(0xFFE0);
    state.record.ring_index = 2U;
    state.frame_counter = 0U;
    state.transition_state = 0x1234U;
    for (std::uint8_t i = 0; i < 6U; ++i) {
        state.slots[i].sprite_data = 0x100U + i;
        state.slots[i].x = i;
        state.slots[i].y = static_cast<std::uint16_t>(i + 10U);
    }
    std::array<std::uint32_t, 7> built{};
    int build_count = 0;
    int menu_calls = 0;
    PersistentWorldEffectCallbacks callbacks{};
    callbacks.resolve_easing = [](const std::uint8_t) {
        return static_cast<std::int16_t>(0);
    };
    callbacks.resolve_sprite_source =
        [](const std::uint8_t variant, const std::uint8_t phase)
        -> std::optional<std::uint32_t> {
        if (variant != 2U) {
            return std::nullopt;
        }
        return 0x4321U;
    };
    callbacks.build_sprite =
        [&](const std::uint32_t token, const std::uint16_t x,
            const std::uint16_t y, const std::uint16_t flags) {
        if (build_count < 7) {
            built[build_count] = token;
        }
        if (flags != 0x6000U || x > 0x1230U || y > 0x0FFEU) {
            build_count += 100;
        }
        ++build_count;
    };
    callbacks.run_menu_selection_variant = [&] { ++menu_calls; };
    if (advance_persistent_world_effect(state, callbacks) !=
            PersistentWorldEffectAction::rendered ||
        state.record.ring_index != 3U || state.slots[3].sprite_data != 0x4321U ||
        build_count != 7 || built[0] != 0x104U || built[1] != 0x105U ||
        built[2] != 0x100U || built[3] != 0x101U || built[4] != 0x102U ||
        built[5] != 0x4321U || built[6] != 0x104U || menu_calls != 0) {
        return 1;
    }

    state.record.lifetime = 0U;
    state.record.active = 0xFFU;
    state.frame_counter = 1U;
    build_count = 0;
    if (advance_persistent_world_effect(state, callbacks) !=
            PersistentWorldEffectAction::finalized ||
        state.record.active != 0U || state.transition_state != 0U ||
        menu_calls != 1) {
        return 2;
    }

    state = {};
    if (advance_persistent_world_effect(state, callbacks) !=
        PersistentWorldEffectAction::inactive) {
        return 3;
    }
    return 0;
}
