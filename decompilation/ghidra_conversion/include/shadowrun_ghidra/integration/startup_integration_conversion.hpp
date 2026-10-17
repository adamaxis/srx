#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/integration/opening_intro_conversion.hpp"
#include "shadowrun_ghidra/menus/startup_menu_conversion.hpp"
#include "shadowrun_ghidra/session/save_field_restore_conversion.hpp"
#include "shadowrun_ghidra/session/startup_record_assets_conversion.hpp"
#include "shadowrun_ghidra/title_intro/boot_sequence_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::integration {

// This is the conversion-side application boundary. It owns the ordering
// between already-converted startup/menu contracts; it is not native game
// state and it is not linked into shadowrun_port.
enum class StartupIntegrationPhase {
    not_started,
    pre_title,
    title_ready,
    start_options_shell,
    start_options,
    save_load,
    joshua_archetype,
    // ROM 0x07DA: after 0x59092 returns with an archetype, 0x131C fades the
    // menu to black (17 CRAM passes, two VBlanks each) before 0x59C5E starts
    // the opening and its 0x10 music.
    joshua_archetype_exit_fade,
    opening_intro,
    field_ready,
    failed,
};

inline constexpr std::uint8_t kArchetypeExitPalettePasses = 17U;
inline constexpr std::uint8_t kArchetypeExitVblanksPerPalettePass = 2U;

enum class StartupIntegrationStartResult {
    assets_missing,
    callback_unresolved,
    boot_in_progress,
    title_ready,
};

enum class StartupIntegrationEvent {
    invalid_phase,
    boot_frame_advanced,
    title_ready,
    entered_start_options,
    start_options_menu_revealed,
    entered_save_load,
    entered_joshua_archetype,
    joshua_archetype_exit_fade_started,
    joshua_archetype_exit_fade_advanced,
    opening_started,
    opening_frame_advanced,
    waiting,
    selection_moved,
    no_save_data,
    save_loaded,
    returned_to_start_options,
    field_ready,
    callback_unresolved,
};

const char* startup_integration_phase_name(StartupIntegrationPhase phase);

// The conversion layer accepts a host save image only after the menu's
// source-derived validation boundary reports it loaded. The image itself is
// supplied by the future host persistence implementation; its restoration is
// performed by restore_saved_field_state(), not by a new menu-side shortcut.
struct LoadedSaveRestoreInput {
    std::vector<std::uint8_t> source_work_buffer;
    bool world_effect_pending = false;
    std::uint8_t scene_profile_selector = 0;
};

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

    // 0x59C5E timed opening. This remains a conversion-side source
    // coordinator and is not part of the native-port runtime.
    OpeningIntroCallbacks opening_intro;

    std::function<std::optional<LoadedSaveRestoreInput>(SaveSlotChoice)>
        read_loaded_save_for_restore;
    SaveFieldRestoreCallbacks save_field_restore;

    // Observation and the final handoff remain host boundaries. They do not
    // replace the converted source routines with invented behavior.
    std::function<void(StartupIntegrationPhase)> phase_changed;
    std::function<void()> field_ready;
};

struct StartupIntegrationState {
    AssetCatalogLoadResult asset_catalog;
    AssetSetReport required_boot_assets;
    std::optional<StartupRecordAssets> startup_records;
    std::vector<std::string> startup_record_asset_errors;
    std::optional<SaveRosterBuffer> save_roster_buffer;
    std::optional<FieldSessionSourceAssets> field_session_source_assets;
    std::vector<std::string> field_session_source_asset_errors;
    std::optional<JoshuaArchetypeSelection> joshua_selection;
    std::optional<SaveFieldRestoreState> restored_field_state;
    std::optional<SaveFieldRestoreResult> save_field_restore_result;

    BootStartupState boot;
    PreTitleSequenceState pre_title;
    OpeningIntroState opening_intro;
    VblankFrameState frame;
    TitleSetupState title_setup;

    StartOptionsMenuState start_options;
    SaveLoadMenuState save_load;
    JoshuaArchetypeMenuState joshua_archetype;

    StartupIntegrationPhase phase = StartupIntegrationPhase::not_started;
    bool pre_title_complete = false;
    // 0x131C progress for the archetype-to-opening fade.
    std::uint8_t archetype_exit_fade_passes = 0U;
    std::uint8_t archetype_exit_fade_vblank = 0U;
    // The 0x59C5E New Game opening leaves the source FFE18B activity marker
    // set for the initial field route. This fact is handed to the field
    // runtime when the opening completes or exits on Start and returns to the
    // shared field setup; loading a save does not set it.
    bool new_game_opening_completed = false;
    // ROM 0x1A9E clears FFF101 at title-loop entry and 0x1BAC sets it to FF
    // only after the seven-edge sequence matches. Preserve that result across
    // the title-to-field handoff; field menu exposure remains a separate port.
    bool title_debug_menu_unlocked = false;
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

// Non-blocking counterpart used by the separate Qt conversion frontend. It
// performs reset/display/VBlank setup and the source 0x5D746 resource setup,
// then leaves the pre-title frame loop at a visible boundary. The caller must
// advance_startup_boot_frame() once per host frame before the title loop can
// begin.
StartupIntegrationStartResult begin_startup_integration(
    const std::filesystem::path& asset_root,
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks);

// ROM 0x075E's no-input title-timeout path returns to run_game_loop() after
// the title fade and audio service. Re-enter the existing pre-title startup
// flow while retaining the already verified host asset catalog.
StartupIntegrationStartResult restart_startup_attract_loop(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks);

// Advances the source pre-title loop by one VBlank-sized step and publishes
// title setup when the direct Sega/guy/FASA sequence reaches its handoff.
StartupIntegrationEvent advance_startup_boot_frame(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks);

// Advances the converted 0x59C5E opening by one source VBlank-sized pass.
// New Game enters this phase after the archetype record is initialized; it
// does not jump directly to a synthetic field screen.
StartupIntegrationEvent advance_startup_opening_frame(
    StartupIntegrationState& state,
    const StartupIntegrationCallbacks& callbacks);

// Advances the 0x07DA -> 0x131C archetype exit fade by one VBlank. After the
// 17th pass it enters 0x59C5E (begin_opening_intro) and returns
// opening_started; no controller input is sampled during the fade.
StartupIntegrationEvent advance_startup_archetype_exit_fade_frame(
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
