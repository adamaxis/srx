#include "shadowrun_ghidra/title_intro/title_intro_transfer_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroDisplayClearRequest clear_request{};
    clear_title_intro_display_buffer(
        [&clear_request](const auto& request) { clear_request = request; });
    if (clear_request.transfer_command != 0x3000U ||
        clear_request.packed_fill_word != 0x07FF07FFU ||
        clear_request.long_write_count != 0x0400U) {
        return 1;
    }

    std::vector<TitleIntroFillRowRequest> rows;
    fill_title_intro_word_block(
        {0x0786U, 20U, 13U, 0x0080U, 0x07FFU},
        [&rows](const auto& request) { rows.push_back(request); });
    if (rows.size() != 13U || rows.front().destination_offset != 0x0786U ||
        rows.back().destination_offset != 0x0D86U ||
        rows.front().word_count != 20U || rows.back().row_index != 12U ||
        rows.front().fill_word != 0x07FFU) {
        return 2;
    }

    WorldFrameServiceState frame{};
    frame.words[3] = 0x1111U;
    reset_world_frame_service_state(frame);
    if (frame.words[0] != 0x11C0U || frame.words[1] != 0U ||
        frame.words[2] != 0xFE80U || frame.words[3] != 0U ||
        frame.words[0x11] != 0xFFFFU || frame.words[0x12] != 0x00B4U) {
        return 3;
    }

    frame.words[1] = 1U;
    frame.words[0x0C] = 7U;
    advance_world_scene_effect_frame(frame);
    if (frame.words[1] != 0U || frame.words[0x0C] != 7U) {
        return 4;
    }
    advance_world_scene_effect_frame(frame);
    if (frame.words[1] != 0x0AU || frame.words[0x0C] != 0U) {
        return 5;
    }

    std::vector<int> phase_order;
    std::vector<WorldSceneEffectAudioCue> audio_cues;
    std::vector<WorldSceneEffectMetadataSource> metadata_sources;
    WorldSceneEffectPhaseCallbacks phase_callbacks{};
    phase_callbacks.play_audio = [&phase_order, &audio_cues](
                                     const WorldSceneEffectAudioCue cue) {
        phase_order.push_back(1);
        audio_cues.push_back(cue);
    };
    phase_callbacks.spawn_special_effect = [&phase_order] {
        phase_order.push_back(2);
    };
    phase_callbacks.copy_resource_metadata = [&phase_order, &metadata_sources](
                                                const WorldSceneEffectMetadataSource source) {
        phase_order.push_back(3);
        metadata_sources.push_back(source);
    };

    frame.words[0x12] = 0x14U;
    frame.scene_origin_x = 100;
    frame.scene_origin_y = 200;
    frame.dirty_flags = 0U;
    auto phase = advance_world_scene_effect_phase(frame, phase_callbacks);
    if (phase.action !=
            WorldSceneEffectPhaseAction::play_special_effect_audio ||
        phase_order != std::vector<int>{1, 2} || audio_cues.size() != 1U ||
        audio_cues[0] != WorldSceneEffectAudioCue::special_effect ||
        frame.scene_origin_x != 100 || frame.scene_origin_y != 200 ||
        frame.dirty_flags != 0x0DU) {
        return 6;
    }

    phase_order.clear();
    frame.words[0x12] = 0x16U;
    phase = advance_world_scene_effect_phase(frame, phase_callbacks);
    if (phase.action !=
            WorldSceneEffectPhaseAction::load_transition_resource_metadata ||
        phase_order != std::vector<int>{3} || metadata_sources.back() !=
            WorldSceneEffectMetadataSource::transition_blank_resource) {
        return 7;
    }

    phase_order.clear();
    frame.words[0x12] = 0x18U;
    frame.scene_origin_x = 100;
    frame.scene_origin_y = 200;
    phase = advance_world_scene_effect_phase(frame, phase_callbacks);
    if (phase.action !=
            WorldSceneEffectPhaseAction::shift_scene_origin_backward ||
        !phase_order.empty() || frame.scene_origin_x != 92 ||
        frame.scene_origin_y != 192) {
        return 8;
    }

    phase_order.clear();
    frame.words[0x12] = 0x19U;
    phase = advance_world_scene_effect_phase(frame, phase_callbacks);
    if (phase.action !=
            WorldSceneEffectPhaseAction::play_scene_transition_audio ||
        phase_order != std::vector<int>{1} || audio_cues.back() !=
            WorldSceneEffectAudioCue::scene_transition ||
        frame.scene_origin_x != 100 || frame.scene_origin_y != 200) {
        return 9;
    }

    phase_order.clear();
    frame.words[0x12] = 0x15U;
    phase = advance_world_scene_effect_phase(frame, phase_callbacks);
    if (phase.action !=
            WorldSceneEffectPhaseAction::load_default_resource_metadata ||
        phase_order != std::vector<int>{3} || metadata_sources.back() !=
            WorldSceneEffectMetadataSource::default_mode_resource) {
        return 10;
    }

    phase_order.clear();
    frame.words[0x12] = 0x33U;
    frame.dirty_flags = 0U;
    phase = advance_world_scene_effect_phase(frame, phase_callbacks);
    if (phase.action != WorldSceneEffectPhaseAction::no_change ||
        !phase_order.empty() || frame.dirty_flags != 0U) {
        return 11;
    }

    WorldSceneSecondaryEffectState secondary_effect{};
    phase_order.clear();
    WorldSceneSecondaryEffectCallbacks secondary_callbacks{};
    secondary_callbacks.apply_entity_secondary_effect = [&phase_order] {
        phase_order.push_back(4);
    };
    secondary_callbacks.prepare_auxiliary_behavior = [&phase_order] {
        phase_order.push_back(5);
    };
    spawn_world_scene_secondary_effect(secondary_effect, secondary_callbacks);
    if (secondary_effect.behavior_flags != 0x01U ||
        phase_order != std::vector<int>{4, 5}) {
        return 12;
    }

    frame.words[3] = 1U;
    frame.words[5] = 10U;
    frame.words[6] = 20U;
    frame.words[0x0B] = 8U;
    frame.words[0x0C] = 0U;
    auto sprite = advance_world_scene_effect_timer_a(frame);
    if (frame.words[3] != 0U || frame.words[0x0B] != 8U || sprite.x != 12 ||
        sprite.y != 61 || sprite.flags != 0x4000U ||
        sprite.source != 0x0001FEA52U) {
        return 13;
    }
    sprite = advance_world_scene_effect_timer_a(frame);
    if (frame.words[3] != 0x14U || frame.words[0x0B] != 0U ||
        sprite.source != 0x0001FEB38U) {
        return 14;
    }

    frame.words[0x0D] = 0U;
    frame.words[0x0E] = 9U;
    sprite = advance_world_scene_effect_timer_b(frame);
    if (frame.words[0x0D] != 0x0AU || frame.words[0x0E] != 0U ||
        sprite.y != 37 || sprite.source != 0x0001FEADEU) {
        return 15;
    }

    frame.words[0x10] = 0U;
    frame.words[0x0F] = 39U;
    sprite = advance_world_scene_effect_timer_c(frame);
    if (frame.words[0x10] != 4U || frame.words[0x0F] != 0U || sprite.x != 30 ||
        sprite.y != 109 || sprite.source != 0x0001FEC34U) {
        return 16;
    }

    const auto entities = render_title_intro_scene_entities(100, 200, 3U);
    if (entities.size() != 2U || entities[0].y != 106 || entities[0].x != 185 ||
        entities[0].size != 4U || entities[0].sequence != 3U ||
        entities[0].attributes != 0x24F6U || entities[1].y != 101 ||
        entities[1].x != 193 || entities[1].sequence != 4U ||
        entities[1].attributes != 0x24F8U) {
        return 17;
    }

    WorldFrameServiceState coordinator_frame{};
    reset_world_frame_service_state(coordinator_frame);
    WorldSceneEntityEffectRuntimeState coordinator_runtime{};
    WorldSceneEntityEffectCallbacks coordinator_callbacks{};
    std::vector<int> coordinator_order;
    std::vector<WorldSceneEffectPhaseAction> phase_actions;
    std::vector<WorldSceneEffectSpriteBuild> published_sprites;
    coordinator_callbacks.project_entity_screen_position =
        [&coordinator_order] { coordinator_order.push_back(1); };
    coordinator_callbacks.publish_sprite =
        [&coordinator_order, &published_sprites](const auto& sprite) {
            coordinator_order.push_back(2);
            published_sprites.push_back(sprite);
        };
    coordinator_callbacks.refresh_world_scene_effect_sprites =
        [&coordinator_order] { coordinator_order.push_back(3); };
    coordinator_callbacks.phase_callbacks.copy_resource_metadata =
        [&phase_actions](const WorldSceneEffectMetadataSource) {
            phase_actions.push_back(
                WorldSceneEffectPhaseAction::load_default_resource_metadata);
        };
    coordinator_callbacks.install_scene_resource_index_entries =
        [&coordinator_order] { coordinator_order.push_back(4); };
    coordinator_callbacks.begin_world_activity_if_idle =
        [&coordinator_order] { coordinator_order.push_back(5); };
    coordinator_callbacks.prepare_terminal_auxiliary_entity =
        [&coordinator_order] { coordinator_order.push_back(6); };
    coordinator_callbacks.apply_entity_secondary_effect =
        [&coordinator_order](WorldSceneEffectEntitySlot&) {
            coordinator_order.push_back(7);
        };
    coordinator_callbacks.prepare_entity_auxiliary_behavior =
        [&coordinator_order](WorldSceneEffectEntitySlot&) {
            coordinator_order.push_back(8);
        };

    coordinator_runtime.scene_mode = 1U;
    auto coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::no_change ||
        !coordinator_order.empty()) {
        return 18;
    }

    coordinator_runtime.scene_mode = 2U;
    coordinator_frame.words[0x11] = 0xFFFFU;
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::no_change) {
        return 19;
    }

    coordinator_order.clear();
    coordinator_frame.words[0x11] = 0U;
    coordinator_frame.words[0x12] = 1U;
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::advanced_resource_phase ||
        coordinator_frame.words[0x12] != 0U || phase_actions.size() != 1U) {
        return 20;
    }
    coordinator_frame.words[0x12] = 0U;
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::entered_active_resource_stage ||
        coordinator_frame.words[0x11] != 1U ||
        coordinator_frame.words[0x12] != 0x0100U ||
        coordinator_order != std::vector<int>{4}) {
        return 21;
    }

    coordinator_order.clear();
    coordinator_frame.words[2] = 0x0240U;
    coordinator_frame.words[1] = 0U;
    coordinator_frame.words[0x0C] = 7U;
    coordinator_frame.words[0x12] = 0U;
    published_sprites.clear();
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::entered_final_effect_stage ||
        coordinator_frame.words[2] != 0x0248U ||
        coordinator_frame.words[0x11] != 2U ||
        coordinator_frame.words[0x12] != 0x0028U ||
        published_sprites.size() != 2U ||
        coordinator_order != std::vector<int>{1, 2, 2, 3}) {
        return 22;
    }

    coordinator_order.clear();
    coordinator_frame.words[1] = 0x000AU;
    coordinator_frame.words[0x0C] = 0U;
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::rendered_active_effect ||
        coordinator_order != std::vector<int>{1, 2, 2, 2, 3}) {
        return 23;
    }

    coordinator_order.clear();
    coordinator_frame.words[1] = 0U;
    coordinator_frame.words[0x0C] = 2U;
    coordinator_runtime.candidate_entities[2].record_flags = 0x01U;
    coordinator_runtime.candidate_entities[2].secondary_effect_trigger = 7U;
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::selected_existing_entity_effect ||
        !coordinator_result.selected_entity_index.has_value() ||
        coordinator_result.selected_entity_index.value() != 2U ||
        coordinator_runtime.candidate_entities[2].behavior_flags != 0x01U ||
        coordinator_order != std::vector<int>{1, 2, 2, 2, 3, 7, 8}) {
        return 24;
    }

    coordinator_runtime.candidate_entities[2] = {};
    coordinator_order.clear();
    coordinator_frame.words[1] = 0U;
    coordinator_frame.words[0x0C] = 2U;
    coordinator_frame.words[0x12] = 1U;
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::spawned_terminal_auxiliary_effect ||
        !coordinator_runtime.terminal_auxiliary_marker ||
        coordinator_runtime.auxiliary_entity.behavior_flags != 0x01U ||
        coordinator_runtime.auxiliary_entity.secondary_effect_trigger != 4U ||
        coordinator_runtime.auxiliary_entity.secondary_effect_phase != 4U ||
        coordinator_runtime.auxiliary_entity.record_flags != 0x21U ||
        coordinator_order != std::vector<int>{1, 2, 2, 2, 3, 6, 7, 8}) {
        return 25;
    }

    coordinator_order.clear();
    coordinator_frame.words[1] = 0U;
    coordinator_frame.words[0x0C] = 2U;
    coordinator_frame.words[0x12] = 0U;
    coordinator_result = advance_world_scene_entity_effect(
        coordinator_frame, coordinator_runtime, coordinator_callbacks);
    if (coordinator_result.action !=
            WorldSceneEntityEffectAction::completed_world_activity ||
        coordinator_order != std::vector<int>{1, 2, 2, 2, 3, 5}) {
        return 26;
    }
    return 0;
}
