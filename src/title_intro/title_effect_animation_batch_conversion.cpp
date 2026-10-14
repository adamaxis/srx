#include "shadowrun_ghidra/title_intro/title_effect_animation_batch_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::array<std::uint16_t, 5> kMatrixProgramResourcePositions{
    0x00D0U, 0x0100U, 0x0130U, 0x0160U, 0x0190U};
constexpr std::array<std::uint32_t, 4> kTitleWorldPhaseSpriteData{
    0x0001D72CU, 0x0001D740U, 0x0001D736U, 0x0001D740U};

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

bool advance_title_world_phase_handler(
    TitleWorldPhaseDispatchState& state,
    const TitleWorldPhaseDispatchCallbacks& callbacks) {
    const auto timing_gate = (state.timing_selector & 0x0003U) == 0U;
    switch (static_cast<TitleWorldPhaseHandler>(state.handler_index)) {
    case TitleWorldPhaseHandler::move_title_phase_to_center:
        if (!timing_gate) {
            return true;
        }
        state.position_x = static_cast<std::uint16_t>(state.position_x + 1U);
        if (state.position_x >= 0x00D0U) {
            state.handler_index = static_cast<std::uint16_t>(
                TitleWorldPhaseHandler::move_title_phase_to_left_edge);
        }
        return true;

    case TitleWorldPhaseHandler::move_title_phase_to_left_edge:
        if (!timing_gate) {
            return true;
        }
        state.position_x = static_cast<std::uint16_t>(state.position_x - 1U);
        if (state.position_x < 0x00B0U) {
            state.handler_index = static_cast<std::uint16_t>(
                TitleWorldPhaseHandler::move_title_phase_to_center);
        }
        return true;

    case TitleWorldPhaseHandler::move_title_phase_to_right_edge:
        state.position_x = static_cast<std::uint16_t>(state.position_x + 3U);
        if (state.position_x >= 0x0120U) {
            state.handler_index = static_cast<std::uint16_t>(
                TitleWorldPhaseHandler::advance_title_sprite_bank);
            if (callbacks.service_audio_command) {
                callbacks.service_audio_command(0x2AU);
            }
        }
        return true;

    case TitleWorldPhaseHandler::retreat_title_phase_to_left_edge:
        state.position_x = static_cast<std::uint16_t>(state.position_x - 3U);
        if (state.position_x < 0x00B0U) {
            state.handler_index = static_cast<std::uint16_t>(
                TitleWorldPhaseHandler::move_title_phase_to_center);
        }
        return true;

    case TitleWorldPhaseHandler::advance_title_sprite_bank:
        if (!timing_gate) {
            return true;
        }
        state.sprite_table_index = static_cast<std::uint16_t>(
            state.sprite_table_index + 1U);
        if (static_cast<std::int16_t>(state.sprite_table_index) >= 4) {
            state.sprite_table_index = 0U;
            state.handler_index = static_cast<std::uint16_t>(
                TitleWorldPhaseHandler::retreat_title_phase_to_left_edge);
        }
        return true;
    }
    return false;
}

} // namespace

std::optional<std::uint16_t> resolve_matrix_program_resource_position(
    const std::uint16_t selected_slot) {
    if (selected_slot >= kMatrixProgramResourcePositions.size()) {
        return std::nullopt;
    }
    return kMatrixProgramResourcePositions[selected_slot];
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

void build_title_sprites_twice(
    const std::function<void(const TitleSpriteBatchBuildRequest&)>&
        build_sprites) {
    if (!build_sprites) {
        return;
    }

    // 0x1A570: the first list is the 0xF1B1E title sprite group at origin
    // (0x12A, 0x100), followed by the 0xF1B28 group at (0x115, 0x100).
    // Both calls pass D6=0x2000 to the no-attribute-offset builder.
    build_sprites(TitleSpriteBatchBuildRequest{
        0x000F1B1EU, 0x012AU, 0x0100U, 0x2000U});
    build_sprites(TitleSpriteBatchBuildRequest{
        0x000F1B28U, 0x0115U, 0x0100U, 0x2000U});
}

bool append_title_vertical_sprite(
    TitleVerticalBuildState& state,
    const std::function<void(std::uint16_t sprite_key)>& append_sprite,
    const std::function<void(const TitleVerticalSpriteBuildRequest&)>&
        build_sprites) {
    if (append_sprite) {
        append_sprite(0x06E0U);
    }
    state.y = static_cast<std::uint16_t>(state.y + 0x60U);
    // CMP.W #0x1280 followed by BCS: the builder is reached only while the
    // wrapped unsigned source word is strictly below 0x1280.
    if (state.y >= 0x1280U) {
        return false;
    }
    if (build_sprites) {
        const auto table_index = static_cast<std::uint8_t>(
            ((state.render_flags >> 1U) & 0x0003U) + 8U);
        build_sprites(TitleVerticalSpriteBuildRequest{
            0x000F1AEEU,
            table_index,
            0x06E0U,
            static_cast<std::uint16_t>(state.x >> 4U),
            static_cast<std::uint16_t>(state.y >> 4U),
            0xA000U});
    }
    return true;
}

MatrixProgramTransitionInitializationAction
initialize_matrix_program_transition(
    MatrixProgramTransitionState& state,
    const std::uint16_t selected_slot,
    const std::uint16_t requested_mode,
    const MatrixProgramTransitionInitializationCallbacks& callbacks) {
    // 0x1B1C4 writes these words in source order before selecting the typed
    // resource position from 0x1B278.
    state.selected_slot = selected_slot;
    state.requested_mode = requested_mode;
    state.mode_request_pending = true;
    state.stage = 0U;
    state.frame_countdown = 0x000CU;

    const auto resource = callbacks.resolve_resource_token
                              ? callbacks.resolve_resource_token(selected_slot)
                              : resolve_matrix_program_resource_position(
                                    selected_slot);
    if (!resource.has_value()) {
        return MatrixProgramTransitionInitializationAction::resource_unresolved;
    }
    state.resource_token = *resource;
    call_once(callbacks.initialize_world_effect_mode);
    return MatrixProgramTransitionInitializationAction::initialized;
}

void reset_title_transition(TitleTransitionState& state) {
    state.world_effect_mode = 0U;
    state.title_phase_active = true;
    state.phase_countdown = 0U;
    state.phase = 0;
}

TitlePhaseCycleAction advance_title_phase_cycle(
    TitlePhaseCycleState& state,
    const TitlePhaseCycleCallbacks& callbacks) {
    --state.countdown;
    if (state.countdown < 0) {
        state.countdown = 3;
        ++state.phase;
        if (state.phase > 5) {
            state.phase = 0;
        }
    }
    call_once([&] {
        if (callbacks.append_sprite_record) {
            callbacks.append_sprite_record(0x06E0U);
        }
    });
    const auto sprite_data = callbacks.resolve_sprite_data
                                 ? callbacks.resolve_sprite_data(
                                       0x0001DB92U,
                                       static_cast<std::uint16_t>(state.phase))
                                 : std::nullopt;
    if (!sprite_data.has_value()) {
        return TitlePhaseCycleAction::sprite_data_unresolved;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitlePhaseSpriteBuildRequest{
            0x0001DB92U,
            static_cast<std::uint16_t>(state.phase),
            *sprite_data,
            0x06E0U,
            0x010DU,
            0x012EU,
            0x2000U});
    }
    return TitlePhaseCycleAction::rendered;
}

TitleJitterAction advance_title_jitter(
    TitleJitterState& state,
    const std::function<std::uint16_t()>& next_random_word,
    const std::function<void()>& write_word_stream,
    const std::function<void()>& reset_word_stream) {
    ++state.frame;
    if (state.frame >= 0x3CU) {
        state.scene_complete = 0U;
        call_once(reset_word_stream);
        state.dirty_flags = static_cast<std::uint8_t>(
            state.dirty_flags | 0x0CU);
        return TitleJitterAction::completed;
    }
    if (!next_random_word) {
        return TitleJitterAction::prng_unresolved;
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
    call_once(write_word_stream);
    state.dirty_flags = static_cast<std::uint8_t>(
        state.dirty_flags | 0x0CU);
    return TitleJitterAction::updated;
}

std::optional<std::uint32_t> resolve_title_world_phase_sprite_data(
    const std::uint16_t table_index) {
    if (table_index >= kTitleWorldPhaseSpriteData.size()) {
        return std::nullopt;
    }
    return kTitleWorldPhaseSpriteData[table_index];
}

TitleWorldPhaseDispatchAction dispatch_title_world_phase_service(
    TitleWorldPhaseDispatchState& state,
    const TitleWorldPhaseDispatchCallbacks& callbacks) {
    if (state.handler_index > static_cast<std::uint16_t>(
                                 TitleWorldPhaseHandler::advance_title_sprite_bank)) {
        return TitleWorldPhaseDispatchAction::handler_unresolved;
    }
    // ROM 0x1B892 invokes the selected 0x1BAE4 handler before loading the
    // build position and sprite-table entry for the common 0x1378 call.
    if (!advance_title_world_phase_handler(state, callbacks)) {
        return TitleWorldPhaseDispatchAction::handler_unresolved;
    }
    const auto sprite_data = callbacks.resolve_sprite_data
                                 ? callbacks.resolve_sprite_data(
                                       0x0001BAD4U,
                                       state.sprite_table_index)
                                 : resolve_title_world_phase_sprite_data(
                                       state.sprite_table_index);
    if (!sprite_data.has_value()) {
        return TitleWorldPhaseDispatchAction::sprite_data_unresolved;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleWorldPhaseSpriteBuildRequest{
            0x0001BAD4U,
            state.sprite_table_index,
            *sprite_data,
            state.position_x,
            state.position_y,
            0x6000U});
    }
    return TitleWorldPhaseDispatchAction::rendered;
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
    WorldEffectMotionSeedState& state,
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
