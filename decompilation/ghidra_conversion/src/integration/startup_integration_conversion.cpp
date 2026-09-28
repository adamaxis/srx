#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion::integration {
namespace {

const char* boot_stage_name(const BootStartupStage stage) {
    switch (stage) {
    case BootStartupStage::reset:
        return "boot.reset";
    case BootStartupStage::vdp_ready:
        return "boot.vdp_ready";
    case BootStartupStage::pre_title:
        return "boot.pre_title";
    case BootStartupStage::title_setup:
        return "boot.title_setup";
    case BootStartupStage::title_ready:
        return "boot.title_ready";
    case BootStartupStage::failed:
        return "boot.failed";
    }
    return "boot.unknown";
}

void set_phase(StartupIntegrationState& state,
               const StartupIntegrationCallbacks& callbacks,
               const StartupIntegrationPhase phase) {
    if (state.phase == phase) {
        return;
    }
    state.phase = phase;
    state.event_log.emplace_back(
        std::string("phase.") + startup_integration_phase_name(phase));
    if (callbacks.phase_changed) {
        callbacks.phase_changed(phase);
    }
}

void append_event(StartupIntegrationState& state, const char* event) {
    state.event_log.emplace_back(event);
}

void enter_boot_stage(StartupIntegrationState& state,
                      const BootStartupStage stage) {
    state.boot.stage = stage;
    state.event_log.emplace_back(boot_stage_name(stage));
}

bool has_vdp_ready_callbacks(const VdpReadyCallbacks& callbacks) {
    return static_cast<bool>(callbacks.warmup_delay) &&
           static_cast<bool>(callbacks.wait_until_ready) &&
           static_cast<bool>(callbacks.settle_delay);
}

StartupIntegrationStartResult load_startup_assets(
    const std::filesystem::path& asset_root,
    StartupIntegrationState& state) {
    state.asset_catalog = load_conversion_asset_catalog(asset_root);
    state.required_boot_assets = verify_conversion_asset_set(
        state.asset_catalog.catalog, required_boot_asset_aliases());
    append_event(state, "assets.verified");

    const auto startup_records =
        load_startup_record_assets(state.asset_catalog.catalog);
    state.startup_record_asset_errors = startup_records.errors;
    if (startup_records.complete()) {
        state.startup_records = *startup_records.assets;
        append_event(state, "startup_records.verified");
    } else {
        append_event(state, "startup_records.missing");
    }

    const auto field_session_source =
        load_field_session_source_assets(state.asset_catalog.catalog);
    state.field_session_source_asset_errors = field_session_source.errors;
    if (field_session_source.complete()) {
        state.field_session_source_assets = *field_session_source.assets;
        append_event(state, "field_session_source_records.verified");
    } else {
        append_event(state, "field_session_source_records.missing");
    }

    if (!state.asset_catalog.complete() ||
        !state.required_boot_assets.complete() ||
        !state.startup_records.has_value() ||
        !state.field_session_source_assets.has_value()) {
        return StartupIntegrationStartResult::assets_missing;
    }
    return StartupIntegrationStartResult::boot_in_progress;
}

StartupIntegrationEvent map_menu_event(const StartOptionsMenuEvent event) {
    switch (event) {
    case StartOptionsMenuEvent::waiting:
        return StartupIntegrationEvent::waiting;
    case StartOptionsMenuEvent::selection_moved:
        return StartupIntegrationEvent::selection_moved;
    case StartOptionsMenuEvent::enter_joshua_archetype_menu:
        return StartupIntegrationEvent::entered_joshua_archetype;
    case StartOptionsMenuEvent::enter_save_load_menu:
        return StartupIntegrationEvent::entered_save_load;
    }
    return StartupIntegrationEvent::callback_unresolved;
}

} // namespace

const char* startup_integration_phase_name(
    const StartupIntegrationPhase phase) {
    switch (phase) {
    case StartupIntegrationPhase::not_started:
        return "not_started";
    case StartupIntegrationPhase::pre_title:
        return "pre_title";
    case StartupIntegrationPhase::title_ready:
        return "title_ready";
    case StartupIntegrationPhase::start_options:
        return "start_options";
    case StartupIntegrationPhase::save_load:
        return "save_load";
    case StartupIntegrationPhase::joshua_archetype:
        return "joshua_archetype";
    case StartupIntegrationPhase::opening_intro:
        return "opening_intro";
    case StartupIntegrationPhase::field_ready:
        return "field_ready";
    case StartupIntegrationPhase::failed:
        return "failed";
    }
    return "unknown";
}

StartupIntegrationStartResult start_startup_integration(
    const std::filesystem::path& asset_root,
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks) {
    const auto begin_result =
        begin_startup_integration(asset_root, state, callbacks);
    if (begin_result == StartupIntegrationStartResult::assets_missing ||
        begin_result == StartupIntegrationStartResult::callback_unresolved) {
        return begin_result;
    }

    while (state.phase == StartupIntegrationPhase::pre_title) {
        if (advance_startup_boot_frame(state, callbacks) ==
            StartupIntegrationEvent::callback_unresolved) {
            return StartupIntegrationStartResult::callback_unresolved;
        }
    }
    if (state.phase != StartupIntegrationPhase::title_ready) {
        return StartupIntegrationStartResult::callback_unresolved;
    }
    return StartupIntegrationStartResult::title_ready;
}

StartupIntegrationStartResult begin_startup_integration(
    const std::filesystem::path& asset_root,
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks) {
    state = StartupIntegrationState{};
    const auto asset_result = load_startup_assets(asset_root, state);
    if (asset_result == StartupIntegrationStartResult::assets_missing) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        return StartupIntegrationStartResult::assets_missing;
    }
    if (!callbacks.initialize_display || !has_vdp_ready_callbacks(callbacks.vdp_ready) ||
        !callbacks.title_setup.prepare_scene_resources ||
        !callbacks.title_setup.load_title_resources ||
        !callbacks.title_setup.load_title_font ||
        !callbacks.title_setup.advance_palette_frames) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        enter_boot_stage(state, BootStartupStage::failed);
        return StartupIntegrationStartResult::callback_unresolved;
    }

    enter_boot_stage(state, BootStartupStage::reset);
    callbacks.initialize_display();
    enter_boot_stage(state, BootStartupStage::vdp_ready);
    wait_for_vdp_ready(callbacks.vdp_ready);
    state.boot.display_ready = true;

    enter_boot_stage(state, BootStartupStage::pre_title);
    if (begin_pre_title_sequence(state.pre_title, callbacks.pre_title) !=
        PreTitleSequenceResult::source_complete) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        enter_boot_stage(state, BootStartupStage::failed);
        return StartupIntegrationStartResult::callback_unresolved;
    }
    set_phase(state, callbacks, StartupIntegrationPhase::pre_title);
    return StartupIntegrationStartResult::boot_in_progress;
}

StartupIntegrationEvent advance_startup_boot_frame(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks) {
    if (state.phase != StartupIntegrationPhase::pre_title) {
        return StartupIntegrationEvent::invalid_phase;
    }
    const auto result = advance_pre_title_sequence_frame(
        state.pre_title, callbacks.pre_title);
    if (result == PreTitleSequenceResult::callback_unresolved) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        enter_boot_stage(state, BootStartupStage::failed);
        return StartupIntegrationEvent::callback_unresolved;
    }
    if (state.pre_title.presentation_stage !=
        PreTitleSequenceState::PresentationStage::complete) {
        return StartupIntegrationEvent::boot_frame_advanced;
    }

    state.pre_title_complete = true;
    state.boot.vblank_count = state.frame.frame_count;
    enter_boot_stage(state, BootStartupStage::title_setup);
    setup_title_state(state.title_setup, callbacks.title_setup);
    state.boot.title_ready = true;
    enter_boot_stage(state, BootStartupStage::title_ready);
    set_phase(state, callbacks, StartupIntegrationPhase::title_ready);
    return StartupIntegrationEvent::title_ready;
}

StartupIntegrationEvent advance_startup_opening_frame(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks) {
    if (state.phase != StartupIntegrationPhase::opening_intro) {
        return StartupIntegrationEvent::invalid_phase;
    }

    const auto result = advance_opening_intro_frame(
        state.opening_intro, callbacks.opening_intro);
    if (result == OpeningIntroAdvanceResult::callback_unresolved ||
        result == OpeningIntroAdvanceResult::interrupted) {
        append_event(state, "opening_intro.unresolved");
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        return StartupIntegrationEvent::callback_unresolved;
    }
    if (result == OpeningIntroAdvanceResult::source_complete) {
        append_event(state, "opening_intro.complete");
        set_phase(state, callbacks, StartupIntegrationPhase::field_ready);
        if (callbacks.field_ready) {
            callbacks.field_ready();
        }
        return StartupIntegrationEvent::field_ready;
    }
    return StartupIntegrationEvent::opening_frame_advanced;
}

StartupIntegrationEvent enter_start_options_integration(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks) {
    if (state.phase != StartupIntegrationPhase::title_ready) {
        return StartupIntegrationEvent::invalid_phase;
    }
    if (!state.startup_records.has_value()) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        return StartupIntegrationEvent::callback_unresolved;
    }

    // The main loop calls 0x59318 immediately before 0x59092. Keep that
    // source boundary in the integration layer rather than hiding it in a
    // menu callback: the menu only owns the New Game/Load Game choice.
    state.save_roster_buffer = initialize_save_roster_buffer(
        state.startup_records->save_roster_header,
        state.startup_records->save_roster_templates);
    if (!state.save_roster_buffer.has_value()) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        return StartupIntegrationEvent::callback_unresolved;
    }
    state.event_log.emplace_back("save_roster.initialized");
    enter_start_options_menu(state.start_options, callbacks.start_options);
    set_phase(state, callbacks, StartupIntegrationPhase::start_options);
    return StartupIntegrationEvent::entered_start_options;
}

StartupIntegrationEvent advance_startup_integration(
    StartupIntegrationState& state,
    const std::uint8_t sampled_controller_byte,
    const StartupIntegrationCallbacks& callbacks) {
    if (state.phase == StartupIntegrationPhase::opening_intro) {
        return advance_startup_opening_frame(state, callbacks);
    }

    switch (state.phase) {
    case StartupIntegrationPhase::start_options: {
        const auto menu_event = advance_start_options_menu(
            state.start_options, sampled_controller_byte,
            callbacks.start_options);
        const auto integration_event = map_menu_event(menu_event);
        if (integration_event ==
            StartupIntegrationEvent::entered_save_load) {
            enter_save_load_menu(state.save_load, callbacks.save_load);
            set_phase(state, callbacks, StartupIntegrationPhase::save_load);
        } else if (integration_event ==
                   StartupIntegrationEvent::entered_joshua_archetype) {
            enter_joshua_archetype_menu(
                state.joshua_archetype, callbacks.joshua_archetype);
            set_phase(state, callbacks,
                      StartupIntegrationPhase::joshua_archetype);
        }
        return integration_event;
    }
    case StartupIntegrationPhase::save_load: {
        const auto menu_event = advance_save_load_menu(
            state.save_load, sampled_controller_byte, callbacks.save_load);
        switch (menu_event) {
        case SaveLoadMenuEvent::waiting:
            return StartupIntegrationEvent::waiting;
        case SaveLoadMenuEvent::selection_moved:
            return StartupIntegrationEvent::selection_moved;
        case SaveLoadMenuEvent::no_save_data:
            return StartupIntegrationEvent::no_save_data;
        case SaveLoadMenuEvent::save_loaded:
            if (!state.startup_records.has_value() ||
                !state.save_roster_buffer.has_value() ||
                !callbacks.read_loaded_save_for_restore) {
                append_event(state, "save_field_restore.missing");
                set_phase(state, callbacks, StartupIntegrationPhase::failed);
                return StartupIntegrationEvent::callback_unresolved;
            }
            {
                const auto restore_input =
                    callbacks.read_loaded_save_for_restore(
                        state.save_load.selected);
                if (!restore_input.has_value()) {
                    append_event(state, "save_field_restore.missing");
                    set_phase(state, callbacks,
                              StartupIntegrationPhase::failed);
                    return StartupIntegrationEvent::callback_unresolved;
                }

                const std::vector<RunnerRosterTemplate> templates(
                    state.startup_records->save_roster_templates.begin(),
                    state.startup_records->save_roster_templates.end());
                state.restored_field_state.emplace();
                state.save_field_restore_result = restore_saved_field_state(
                    restore_input->source_work_buffer, templates,
                    state.save_roster_buffer->compact_records,
                    restore_input->world_effect_pending,
                    restore_input->scene_profile_selector,
                    *state.restored_field_state,
                    callbacks.save_field_restore);
                if (state.save_field_restore_result->action !=
                    SaveFieldRestoreAction::restored) {
                    append_event(state, "save_field_restore.failed");
                    set_phase(state, callbacks,
                              StartupIntegrationPhase::failed);
                    return StartupIntegrationEvent::callback_unresolved;
                }
            }
            append_event(state, "save_field_restore.restored");
            set_phase(state, callbacks, StartupIntegrationPhase::field_ready);
            if (callbacks.field_ready) {
                callbacks.field_ready();
            }
            return StartupIntegrationEvent::save_loaded;
        case SaveLoadMenuEvent::callback_unresolved:
            set_phase(state, callbacks, StartupIntegrationPhase::failed);
            return StartupIntegrationEvent::callback_unresolved;
        }
        return StartupIntegrationEvent::callback_unresolved;
    }
    case StartupIntegrationPhase::joshua_archetype: {
        const auto menu_event = advance_joshua_archetype_menu(
            state.joshua_archetype, sampled_controller_byte,
            callbacks.joshua_archetype);
        switch (menu_event) {
        case JoshuaArchetypeMenuEvent::waiting:
            return StartupIntegrationEvent::waiting;
        case JoshuaArchetypeMenuEvent::selection_moved:
            return StartupIntegrationEvent::selection_moved;
        case JoshuaArchetypeMenuEvent::joshua_archetype_initialized:
            if (!state.startup_records.has_value()) {
                set_phase(state, callbacks, StartupIntegrationPhase::failed);
                return StartupIntegrationEvent::callback_unresolved;
            }
            {
                const std::vector<JoshuaArchetypeTemplate> templates(
                    state.startup_records->joshua_archetypes.begin(),
                    state.startup_records->joshua_archetypes.end());
                state.joshua_selection = initialize_joshua_archetype_selection(
                    static_cast<std::uint16_t>(
                        state.joshua_archetype.selected),
                    templates);
            }
            if (!state.joshua_selection.has_value()) {
                set_phase(state, callbacks, StartupIntegrationPhase::failed);
                return StartupIntegrationEvent::callback_unresolved;
            }
            state.event_log.emplace_back("joshua_archetype.initialized");
            if (begin_opening_intro(state.opening_intro,
                                    callbacks.opening_intro) !=
                OpeningIntroStartResult::started) {
                append_event(state, "opening_intro.unresolved");
                set_phase(state, callbacks, StartupIntegrationPhase::failed);
                return StartupIntegrationEvent::callback_unresolved;
            }
            append_event(state, "opening_intro.started");
            set_phase(state, callbacks, StartupIntegrationPhase::opening_intro);
            return StartupIntegrationEvent::opening_started;
        case JoshuaArchetypeMenuEvent::callback_unresolved:
            set_phase(state, callbacks, StartupIntegrationPhase::failed);
            return StartupIntegrationEvent::callback_unresolved;
        }
        return StartupIntegrationEvent::callback_unresolved;
    }
    case StartupIntegrationPhase::not_started:
    case StartupIntegrationPhase::pre_title:
    case StartupIntegrationPhase::title_ready:
    case StartupIntegrationPhase::opening_intro:
    case StartupIntegrationPhase::field_ready:
    case StartupIntegrationPhase::failed:
        return StartupIntegrationEvent::invalid_phase;
    }
    return StartupIntegrationEvent::invalid_phase;
}

} // namespace shadowrun::ghidra_conversion::integration
