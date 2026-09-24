#include "shadowrun_ghidra/title_intro_late_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

template <std::size_t Count>
void render_text_batch(
    const std::array<std::uint32_t, Count>& text_assets,
    const std::array<std::uint16_t, Count>& destinations,
    std::uint16_t palette_flags,
    const TitleIntroLateTextRenderBoundary& render_text) {
    if (!render_text) {
        return;
    }
    for (std::size_t index = 0; index < Count; ++index) {
        render_text({text_assets[index], destinations[index], palette_flags});
    }
}

} // namespace

void render_title_intro_middle_text_batch(
    const std::array<std::uint32_t, kTitleIntroThreeTextCount>& text_assets,
    const TitleIntroLateTextRenderBoundary& render_text) {
    constexpr std::array<std::uint16_t, kTitleIntroThreeTextCount>
        kDestinations{{0x2186U, 0x2286U, 0x2386U}};
    render_text_batch(text_assets, kDestinations, 0xE000U, render_text);
}

void render_title_intro_center_text_batch(
    std::uint32_t text_asset,
    const TitleIntroLateTextRenderBoundary& render_text) {
    if (!render_text) {
        return;
    }
    constexpr std::array<std::uint16_t, kTitleIntroThreeTextCount>
        kDestinations{{0x2186U, 0x2286U, 0x2386U}};
    for (const auto destination : kDestinations) {
        render_text({text_asset, destination, 0xE000U});
    }
}

void render_title_intro_late_text_batch(
    const std::array<std::uint32_t, kTitleIntroThreeTextCount>& text_assets,
    const TitleIntroLateTextRenderBoundary& render_text) {
    constexpr std::array<std::uint16_t, kTitleIntroThreeTextCount>
        kDestinations{{0x2106U, 0x2206U, 0x2306U}};
    render_text_batch(text_assets, kDestinations, 0xE000U, render_text);
}

void render_title_intro_scroll_text_batch(
    std::uint32_t text_asset,
    const TitleIntroLateTextRenderBoundary& render_text) {
    if (!render_text) {
        return;
    }
    constexpr std::array<std::uint16_t, kTitleIntroScrollTextCount>
        kDestinations{{0x2106U, 0x2186U, 0x2206U, 0x2286U, 0x2306U}};
    for (const auto destination : kDestinations) {
        render_text({text_asset, destination, 0xA000U});
    }
}

void reset_title_intro_scroll_fill(TitleIntroScrollState& state) {
    state.transition_fill_started = false;
}

void advance_title_intro_scroll(
    TitleIntroScrollState& state,
    const std::function<void(const TitleIntroScrollFillRequest&)>& fill_tiles) {
    if (state.x_offset < -0x200) {
        return;
    }
    if (state.x_offset <= -0x190 && !state.transition_fill_started) {
        state.transition_fill_started = true;
        if (fill_tiles) {
            fill_tiles({0x074AU, 5U, 8U, 0x0080U});
        }
    }
    state.x_offset = static_cast<std::int16_t>(state.x_offset - 2);
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x04U);
}

void advance_title_intro_scroll_fade(
    TitleIntroScrollState& state,
    const std::function<void()>& render_transition_text) {
    if (state.x_offset > -0x1B0) {
        return;
    }
    if (state.x_offset == -0x1B0 && render_transition_text) {
        render_transition_text();
    }
    if (state.fade_counter < 0x78U) {
        ++state.fade_counter;
    }
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x08U);
}

std::uint16_t advance_title_intro_triplet(TitleIntroTripletState& state,
                                          std::uint16_t current_frame) {
    const auto frame_phase = static_cast<std::uint16_t>(current_frame & 7U);
    if (frame_phase != 0U) {
        return frame_phase;
    }
    const auto first = state.values[0];
    state.values[0] = state.values[1];
    state.values[1] = state.values[2];
    state.values[2] = first;
    state.dirty = true;
    return first;
}

void advance_title_intro_pulse(
    TitleIntroPulseState& state,
    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount>& descriptors,
    const std::function<void(const TitleIntroSpriteRequest&)>& draw_sprite) {
    if (state.x_offset < -0x1B8) {
        return;
    }
    if (state.current_frame >= state.next_event_frame) {
        state.sprite_descriptor = descriptors[state.phase];
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + 6U);
        state.phase = static_cast<std::uint16_t>(
            (state.phase + 1U) % kTitleIntroPulsePhaseCount);
    }
    if (draw_sprite) {
        draw_sprite({state.sprite_descriptor,
                     static_cast<std::uint16_t>(state.x_offset + 0x01FC),
                     0x00EFU,
                     0x2000U});
    }
}

void reset_title_intro_landing(TitleIntroLandingState& state) {
    state.phase = 0;
    state.next_event_frame = 0;
    state.x_accumulator = 0x7C00U;
    state.y_accumulator = 0xB700U;
}

void advance_title_intro_landing(
    TitleIntroLandingState& state,
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>&
        phases,
    std::uint32_t special_landing_descriptor,
    const std::function<void(const TitleIntroSpriteRequest&)>& draw_sprite) {
    const auto elapsed = static_cast<std::uint16_t>(
        state.current_frame - state.animation_start_frame);
    if (elapsed <= 0x0225U) {
        return;
    }
    if (state.current_frame >= state.next_event_frame &&
        state.phase < kTitleIntroLandingPhaseCount) {
        const auto& phase = phases[state.phase];
        state.sprite_descriptor = phase.sprite_descriptor;
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + phase.delay_frames);
        state.phase = static_cast<std::uint16_t>(state.phase + 1U);
    }
    const auto& phase = phases[std::min<std::size_t>(
        state.phase == 0U ? 0U : state.phase - 1U,
        kTitleIntroLandingPhaseCount - 1U)];
    state.x_accumulator = static_cast<std::uint16_t>(
        state.x_accumulator + phase.x_delta);
    state.y_accumulator = static_cast<std::uint16_t>(
        state.y_accumulator + phase.y_delta);
    const auto screen_x = static_cast<std::uint16_t>(state.x_accumulator >> 8);
    const auto base_y = static_cast<std::uint16_t>(state.y_accumulator >> 8);
    const auto screen_y = static_cast<std::uint16_t>(base_y + 0x28U);
    const auto descriptor = (screen_y >= 0x0110U && screen_y < 0x011FU)
                                ? special_landing_descriptor
                                : state.sprite_descriptor;
    if (draw_sprite) {
        draw_sprite({descriptor, screen_x, screen_y, 0x2000U});
    }
}

void service_title_intro_animation(
    TitleIntroAnimationServiceState& service_state,
    TitleIntroPulseState& pulse_state,
    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount>&
        pulse_descriptors,
    TitleIntroLandingState& landing_state,
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>&
        landing_phases,
    std::uint32_t special_landing_descriptor,
    const std::function<void(const TitleIntroSpriteRequest&)>& draw_sprite) {
    service_state.scene_animation_enabled = true;
    advance_title_intro_pulse(pulse_state, pulse_descriptors, draw_sprite);
    advance_title_intro_landing(landing_state, landing_phases,
                                special_landing_descriptor, draw_sprite);
    service_state.entity_record_pending = true;
    service_state.dirty_flags = static_cast<std::uint8_t>(
        service_state.dirty_flags | 0x02U);
}

void set_title_intro_entity_color_words(TitleIntroEntityColorState& state,
                                        std::size_t entity_index,
                                        std::uint16_t value) {
    if (entity_index >= state.words.size()) {
        return;
    }
    state.words[entity_index].fill(value);
    state.dirty = true;
}

void step_title_intro_entity_color_words(TitleIntroEntityColorState& state,
                                         std::size_t entity_index) {
    if (entity_index >= state.words.size()) {
        return;
    }
    for (auto& word : state.words[entity_index]) {
        if ((word & 0x000EU) != 0U) {
            word = static_cast<std::uint16_t>(word - 0x0002U);
        }
        if ((word & 0x00E0U) != 0U) {
            word = static_cast<std::uint16_t>(word - 0x0020U);
        }
        if ((word & 0x0E00U) != 0U) {
            word = static_cast<std::uint16_t>(word - 0x0200U);
        }
    }
    state.dirty = true;
}

} // namespace shadowrun::ghidra_conversion
