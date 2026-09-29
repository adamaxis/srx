#include "shadowrun_ghidra/title_intro/title_effect_animation_batch_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

std::int16_t select_menu_entry(const std::int8_t entry_state,
                               const MenuEntryCallbacks& callbacks) {
    const auto hud_mode = callbacks.select_hud_mode ? callbacks.select_hud_mode() : 0U;
    if (entry_state == 0 && (hud_mode == 0U || hud_mode == 8U || hud_mode < 8U)) {
        call_once(callbacks.service_entry);
        if (entry_state < 0) {
            call_once(callbacks.prepare_offset);
            return -1;
        }
        const auto value = callbacks.read_entry_value ? callbacks.read_entry_value() : 0U;
        (void)value;
        return 0;
    }
    call_once(callbacks.set_selection);
    call_once(callbacks.render_text);
    call_once(callbacks.prepare_offset);
    return 0x45;
}

void refresh_menu_selection_after_clear(
    const bool has_pending_value,
    const std::function<void()>& clear_selected,
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text) {
    if (has_pending_value) {
        call_once(clear_selected);
    }
    call_once(set_selection);
    call_once(render_text);
}

void refresh_menu_selection_after_clear(
    const MenuSelectionRefreshState& state,
    const MenuSelectionRefreshExactCallbacks& callbacks) {
    if (state.pending_marker > 0) {
        call_once(callbacks.clear_selected_menu_record);
    }
    if (callbacks.set_hud_selection) {
        callbacks.set_hud_selection(9U,
                                    state.pending_marker > 0 ? 6U : 0x0AU);
    }
    call_once(callbacks.render_menu_text_followup);
}

void refresh_menu_selection_or_prepare(
    const bool has_pending_value,
    const std::function<void()>& prepare_offset,
    const std::function<void()>& clear_selected,
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text) {
    if (has_pending_value) {
        call_once(clear_selected);
    } else {
        call_once(prepare_offset);
    }
    call_once(set_selection);
    call_once(render_text);
}

void refresh_menu_selection_or_prepare(
    const MenuSelectionRefreshState& state,
    const MenuSelectionRefreshExactCallbacks& callbacks) {
    if (state.pending_marker > 0) {
        call_once(callbacks.clear_selected_menu_record);
    } else {
        call_once(callbacks.prepare_menu_offset);
    }
    if (callbacks.set_hud_selection) {
        callbacks.set_hud_selection(0x0AU,
                                    state.pending_marker > 0 ? 6U : 0x0AU);
    }
    call_once(callbacks.render_menu_text_followup);
}

void initialize_audio_frame(AudioFrameState& state,
                            const std::function<void()>& service_audio) {
    state.field_a = 0;
    state.field_b = 2;
    state.field_c = 1;
    call_once(service_audio);
}

bool advance_title_animation_cycle(
    TitleAnimationCycleState& state,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprites) {
    --state.countdown;
    if (state.countdown < 0) {
        state.countdown = 2;
        ++state.phase;
        if (state.phase > 2) {
            state.active = false;
            return false;
        }
    }
    call_once(append_sprite);
    call_once(build_sprites);
    return true;
}

void build_title_sprites_twice(const std::function<void()>& build_sprites) {
    call_once(build_sprites);
    call_once(build_sprites);
}

bool append_title_vertical_sprite(
    TitleVerticalBuildState& state,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprites) {
    call_once(append_sprite);
    state.y = static_cast<std::uint16_t>(state.y + 0x60U);
    if (state.y > 0x127FU) {
        return false;
    }
    call_once(build_sprites);
    return true;
}

void initialize_world_effect_seed(
    WorldEffectSeedState& state,
    const std::int16_t variant,
    const std::uint16_t source_y,
    const std::function<void()>& build_effect,
    const std::function<void()>& service_audio) {
    state.variant = variant;
    state.source_x = 0x10C0U;
    state.source_y = source_y;
    state.horizontal_velocity = 10;
    state.lifetime = -1;
    state.palette = 0x1E;
    state.active = true;
    call_once(service_audio);
    if (variant != 0) {
        state.horizontal_velocity = variant < 0 ? -8 : 8;
    }
    call_once(build_effect);
}

void reset_title_transition(TitleTransitionState& state) {
    state.position = 0;
    state.timer = 1;
    state.phase = 0;
    state.dirty = true;
}

void advance_title_phase_cycle(
    TitlePhaseCycleState& state,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprites) {
    --state.countdown;
    if (state.countdown < 0) {
        state.countdown = 3;
        ++state.phase;
        if (state.phase > 5) {
            state.phase = 0;
        }
    }
    call_once(append_sprite);
    call_once(build_sprites);
}

bool advance_title_jitter(
    TitleJitterState& state,
    const std::function<std::uint16_t()>& next_random_word,
    const std::function<void()>& write_word_stream,
    const std::function<void()>& reset_word_stream) {
    ++state.frame;
    if (state.frame >= 0x3CU || !next_random_word) {
        state.frame = 0;
        state.bounds.fill(0);
        call_once(reset_word_stream);
        return false;
    }
    for (int iteration = 0; iteration < 0x58; ++iteration) {
        const auto random_word = next_random_word();
        auto delta_x = static_cast<std::int16_t>(random_word & 0x1FU);
        auto delta_y = static_cast<std::int16_t>((random_word >> 8U) & 0x1FU);
        if ((random_word & 0x80U) != 0U) {
            delta_x = static_cast<std::int16_t>(-delta_x);
        }
        if ((random_word & 0x8000U) != 0U) {
            delta_y = static_cast<std::int16_t>(-delta_y);
        }
        state.bounds[0] = static_cast<std::int16_t>(state.bounds[0] + delta_x);
        state.bounds[1] = static_cast<std::int16_t>(state.bounds[1] + delta_x);
        state.bounds[2] = static_cast<std::int16_t>(state.bounds[2] + delta_y);
        state.bounds[3] = static_cast<std::int16_t>(state.bounds[3] + delta_y);
    }
    state.dirty_flags |= 0x0CU;
    call_once(write_word_stream);
    return true;
}

bool dispatch_title_phase_handler(
    const std::size_t phase,
    const std::vector<std::function<void()>>& handlers,
    const std::function<void()>& build_sprites) {
    if (phase >= handlers.size() || !handlers[phase]) {
        return false;
    }
    handlers[phase]();
    call_once(build_sprites);
    return true;
}

void copy_title_profile(const std::array<std::uint16_t, 7>& source,
                        std::array<std::uint16_t, 7>& destination,
                        std::uint32_t& dirty_flags) {
    destination = source;
    dirty_flags |= 1U;
}

bool advance_alternating_profile(
    AlternatingProfileState& state,
    const std::function<void()>& final_service) {
    --state.countdown;
    if (state.countdown == 0) {
        call_once(final_service);
        return false;
    }
    const auto& profile = (state.countdown & 1) != 0 ? state.odd_profile
                                                     : state.even_profile;
    copy_title_profile(profile, state.destination, state.dirty_flags);
    return true;
}

bool advance_audio_menu_countdown(
    AudioMenuCountdownState& state,
    const std::function<void()>& service_audio,
    const std::function<void()>& clear_ready_menu_slot) {
    --state.frame;
    if (state.frame >= 0) {
        return false;
    }
    state.frame = state.reload;
    if (state.remaining != 0U) {
        --state.remaining;
    }
    if (state.remaining != 0U) {
        return false;
    }
    call_once(service_audio);
    state.entity_flags = static_cast<std::uint8_t>(state.entity_flags & ~0x02U);
    call_once(clear_ready_menu_slot);
    return true;
}

void initialize_effect_motion_seed(
    WorldEffectSeedState& state,
    const bool alternate_direction,
    const std::function<void()>& service_audio) {
    state.source_x = 0x10C0U;
    state.source_y = 0x1020U;
    state.horizontal_velocity = alternate_direction ? -18 : -10;
    state.lifetime = -1;
    state.palette = 0x1E;
    state.active = true;
    call_once(service_audio);
}

void update_interaction_state(InteractionState& state,
                              const bool source_bit_40,
                              const bool global_ready,
                              const std::int8_t mode_marker,
                              const InteractionCallbacks& callbacks) {
    if (source_bit_40) {
        if (state.latch != 0U) {
            return;
        }
        state.latch = 0xFFU;
        state.countdown = 0x3CU;
        state.x = 0x1400;
        state.y = 0x1070;
        state.velocity_x = -8;
        state.velocity_y = -14;
        return;
    }
    if (!global_ready) {
        if (mode_marker < 0) {
            state.phase = 0;
            state.countdown = 0x102;
            state.x = 0x10C;
            state.y = 0x102;
            state.velocity_y = 3;
            call_once(callbacks.service_audio);
            return;
        }
        if (mode_marker != 0) {
            call_once(callbacks.initialize_primary);
            return;
        }
    }
    call_once(callbacks.advance_motion);
    if (state.motion_active) {
        state.y = static_cast<std::int16_t>(state.y - 6);
    }
}

void update_persistent_title_effect(
    const bool source_active,
    const std::uint8_t slot_flags,
    std::uint16_t& transition_phase,
    const std::function<void()>& prepare_effect,
    const std::function<void()>& advance_effect,
    const std::function<void()>& build_sprites,
    const std::function<void()>& reset_transition,
    const std::function<void()>& refresh_menu) {
    if (!source_active) {
        return;
    }
    call_once(prepare_effect);
    call_once(advance_effect);
    call_once(build_sprites);
    if (slot_flags == 0U) {
        transition_phase = 0;
        call_once(reset_transition);
        call_once(refresh_menu);
    }
}

void advance_title_effect_position(
    std::array<std::int16_t, 4>& positions,
    const std::function<void()>& build_sprites) {
    positions[0] = static_cast<std::int16_t>(positions[0] + 9);
    positions[1] = static_cast<std::int16_t>(positions[1] - 0x1A);
    positions[2] = static_cast<std::int16_t>(positions[2] - 0x0E);
    positions[3] = static_cast<std::int16_t>(positions[3] - 0x0E);
    call_once(build_sprites);
}

void build_title_sprites(const std::function<void()>& build_sprites) {
    call_once(build_sprites);
}

} // namespace shadowrun::ghidra_conversion
