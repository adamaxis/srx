#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/menus/startup_menu_conversion.hpp"
#include "shadowrun_ghidra/title_intro/boot_sequence_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::integration {

// This is the conversion-side application boundary. It owns the ordering
// between already-converted startup/menu contracts; it is not native game
// state and it is not linked into shadowrun_port.
enum class StartupIntegrationPhase {
    not_started,
    title_ready,
    start_options,
    save_load,
    joshua_archetype,
    field_ready,
    failed,
};

enum class StartupIntegrationStartResult {
    assets_missing,
    callback_unresolved,
    title_ready,
};

enum class StartupIntegrationEvent {
    invalid_phase,
    entered_start_options,
    entered_save_load,
    entered_joshua_archetype,
    waiting,
    selection_moved,
    no_save_data,
    save_loaded,
    field_ready,
    callback_unresolved,
};

const char* startup_integration_phase_name(StartupIntegrationPhase phase);

struct StartupIntegrationCallbacks {
    // 0x0236 -> 0x073A display handoff.
    std::function<void()> initialize_display;
    VdpReadyCallbacks vdp_ready;

    // 0x5D746 -> 0x1D7A title handoff.
    PreTitleSequenceCallbacks pre_title;
    TitleSetupCallbacks title_setup;

    // 0x59092 -> 0x59316 startup menu tree.
    StartOptionsMenuCallbacks start_options;
    SaveLoadMenuCallbacks save_load;
    JoshuaArchetypeMenuCallbacks joshua_archetype;

    // Observation and the final handoff remain host boundaries. They do not
    // replace the converted source routines with invented behavior.
    std::function<void(StartupIntegrationPhase)> phase_changed;
    std::function<void()> field_ready;
};

struct StartupIntegrationState {
    AssetCatalogLoadResult asset_catalog;
    AssetSetReport required_boot_assets;

    BootStartupState boot;
    PreTitleSequenceState pre_title;
    VblankFrameState frame;
    TitleSetupState title_setup;

    StartOptionsMenuState start_options;
    SaveLoadMenuState save_load;
    JoshuaArchetypeMenuState joshua_archetype;

    StartupIntegrationPhase phase = StartupIntegrationPhase::not_started;
    bool pre_title_complete = false;
    std::vector<std::string> event_log;
};

// Loads and verifies the direct asset catalog, then composes the existing
// boot, pre-title, and title-setup conversions in their source-derived order.
// A pre-title callback gap is propagated as callback_unresolved instead of
// being hidden by the void callback shape of run_boot_startup().
StartupIntegrationStartResult start_startup_integration(
    const std::filesystem::path& asset_root,
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks);

// The title loop's Start edge is a separate source boundary from boot/title
// setup. Entering this menu explicitly keeps that boundary visible while
// connecting it to the converted New Game/Load Game routine.
StartupIntegrationEvent enter_start_options_integration(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks);

// Dispatch one sampled startup-menu controller byte through the currently
// active converted menu. New Game reaches Joshua's Samurai/Decker/Gator
// Shaman selector; Load Game reaches the two-slot save screen; successful
// completion of either path publishes the field-ready handoff.
StartupIntegrationEvent advance_startup_integration(
    StartupIntegrationState& state,
    std::uint8_t sampled_controller_byte,
    const StartupIntegrationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion::integration
