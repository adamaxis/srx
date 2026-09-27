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
    case StartupIntegrationPhase::title_ready:
        return "title_ready";
    case StartupIntegrationPhase::start_options:
        return "start_options";
    case StartupIntegrationPhase::save_load:
        return "save_load";
    case StartupIntegrationPhase::joshua_archetype:
        return "joshua_archetype";
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
    state = StartupIntegrationState{};
    state.asset_catalog = load_conversion_asset_catalog(asset_root);
    state.required_boot_assets = verify_conversion_asset_set(
        state.asset_catalog.catalog, required_boot_asset_aliases());
    append_event(state, "assets.verified");

    if (!state.asset_catalog.complete() ||
        !state.required_boot_assets.complete()) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        return StartupIntegrationStartResult::assets_missing;
    }

    PreTitleSequenceResult pre_title_result =
        PreTitleSequenceResult::callback_unresolved;
    BootStartupCallbacks boot_callbacks{};
    boot_callbacks.enter_stage = [&state](const BootStartupStage stage) {
        state.event_log.emplace_back(boot_stage_name(stage));
    };
    boot_callbacks.initialize_display = callbacks.initialize_display;
    boot_callbacks.vdp_ready = callbacks.vdp_ready;
    boot_callbacks.run_pre_title_sequence = [&] {
        pre_title_result = run_pre_title_sequence(
            state.pre_title, callbacks.pre_title);
        state.pre_title_complete =
            pre_title_result == PreTitleSequenceResult::source_complete;
        state.boot.vblank_count = state.frame.frame_count;
    };
    boot_callbacks.setup_title_state = [&] {
        setup_title_state(state.title_setup, callbacks.title_setup);
    };

    const auto boot_result = run_boot_startup(
        state.required_boot_assets, state.boot, boot_callbacks);
    if (boot_result != BootStartupResult::title_ready ||
        pre_title_result != PreTitleSequenceResult::source_complete) {
        set_phase(state, callbacks, StartupIntegrationPhase::failed);
        return boot_result == BootStartupResult::assets_missing
            ? StartupIntegrationStartResult::assets_missing
            : StartupIntegrationStartResult::callback_unresolved;
    }

    set_phase(state, callbacks, StartupIntegrationPhase::title_ready);
    return StartupIntegrationStartResult::title_ready;
}

StartupIntegrationEvent enter_start_options_integration(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks) {
    if (state.phase != StartupIntegrationPhase::title_ready) {
        return StartupIntegrationEvent::invalid_phase;
    }
    enter_start_options_menu(state.start_options, callbacks.start_options);
    set_phase(state, callbacks, StartupIntegrationPhase::start_options);
    return StartupIntegrationEvent::entered_start_options;
}

StartupIntegrationEvent advance_startup_integration(
    StartupIntegrationState& state,
    const std::uint8_t sampled_controller_byte,
    const StartupIntegrationCallbacks& callbacks) {
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
            set_phase(state, callbacks, StartupIntegrationPhase::field_ready);
            if (callbacks.field_ready) {
                callbacks.field_ready();
            }
            return StartupIntegrationEvent::field_ready;
        case JoshuaArchetypeMenuEvent::callback_unresolved:
            set_phase(state, callbacks, StartupIntegrationPhase::failed);
            return StartupIntegrationEvent::callback_unresolved;
        }
        return StartupIntegrationEvent::callback_unresolved;
    }
    case StartupIntegrationPhase::not_started:
    case StartupIntegrationPhase::title_ready:
    case StartupIntegrationPhase::field_ready:
    case StartupIntegrationPhase::failed:
        return StartupIntegrationEvent::invalid_phase;
    }
    return StartupIntegrationEvent::invalid_phase;
}

} // namespace shadowrun::ghidra_conversion::integration
