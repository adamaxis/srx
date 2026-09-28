#pragma once

#include "shadowrun_ghidra/title_intro/title_intro_animation_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_final_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_late_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_motion_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_overlay_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_persistent_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_schedule_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_sequence_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_stage2_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_stage_loop_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_sweeps_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_timed_sprites_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion::integration {

// These are host resource identities, not ROM addresses. Their only job is
// to let the conversion-side presentation callback select the individually
// extracted asset group that corresponds to a source SAT list.
enum class OpeningIntroSpriteFamily : std::uint8_t {
    unavailable,
    trivid_left,
    trivid_right,
    trivid_timed,
    trivid_overlay,
    trivid_final_overlay,
    ambush_actor,
    ambush_muzzle,
    ambush_flash,
    ambush_body,
    projectile_b,
    projectile_c,
    projectile_d,
    michael,
    helicopter,
    aircraft_movement,
    aircraft_plane_a,
    background_patch,
};

struct OpeningIntroSpriteGroup {
    OpeningIntroSpriteFamily family = OpeningIntroSpriteFamily::unavailable;
    std::uint8_t variant = 0;
};

constexpr std::uint32_t kOpeningIntroSpriteHandleTag = 0xA5000000U;

std::uint32_t opening_intro_sprite_handle(
    OpeningIntroSpriteFamily family, std::uint8_t variant);
OpeningIntroSpriteGroup opening_intro_sprite_group(std::uint32_t handle);
std::string opening_intro_sprite_alias_prefix(
    OpeningIntroSpriteGroup group);

// The text records are the source strings at 0x5B7D8..0x5BA4C, represented as
// typed host records. The Qt frontend uses these records with the extracted
// source font; no runtime ROM text decoder is involved.
enum class OpeningIntroTextAsset : std::uint8_t {
    time,
    place,
    team_cover,
    team_runners,
    team_wilderness,
    without_warning,
    ambushed,
    massacre,
    captured,
    slain,
    tri_vid_recovered,
    national_news,
    brother_last_die,
    spending,
    vow,
    brother_death,
    landing,
    flag_cab,
    last_credstick,
    transaction,
    motel,
    redmond,
};

constexpr std::uint32_t kOpeningIntroTextHandleTag = 0xB6000000U;

std::uint32_t opening_intro_text_handle(OpeningIntroTextAsset asset);
OpeningIntroTextAsset opening_intro_text_asset(std::uint32_t handle);
std::string_view opening_intro_text(OpeningIntroTextAsset asset);

enum class OpeningIntroScene : std::uint8_t {
    seattle,
    wilderness,
    trivid,
    airport,
    cab_map,
};

enum class OpeningIntroStage : std::uint8_t {
    not_started,
    schedule,
    stage2_entry,
    stage2_first_loop,
    stage2_second_loop,
    orchestrator_entry,
    orchestrator_loop,
    stage_loop_entry,
    stage_loop,
    complete,
    interrupted,
};

const char* opening_intro_stage_name(OpeningIntroStage stage);
const char* opening_intro_scene_name(OpeningIntroScene scene);

struct OpeningIntroSpriteRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t palette_flags = 0;
};

struct OpeningIntroTextRequest {
    std::uint32_t text_asset = 0;
    std::uint16_t destination_word = 0;
    std::uint16_t palette_flags = 0;
};

struct OpeningIntroSourcePlan {
    std::vector<TitleIntroScheduleEntry> opening_schedule;
    std::vector<TitleIntroScheduleEntry> stage2_schedule;

    std::array<TitleIntroOverlayPhase, kTitleIntroOverlayPhaseCount>
        overlay_phases{};
    std::uint32_t final_overlay_descriptor = 0;
    std::array<TitleIntroSequencePhase, kTitleIntroSequencePhaseCount>
        sequence_phases{};
    std::array<TitleIntroPersistentPhase, kTitleIntroPersistentPhaseCount>
        persistent_phases{};
    std::array<TitleIntroMotionPhase, kTitleIntroMotionPhaseCount>
        motion_phases{};
    std::array<std::uint32_t, kTitleIntroDualSpriteCount> dual_sprites{};
    std::array<std::uint32_t, kTitleIntroTimedSpriteCount> timed_sprites{};
    std::array<TitleIntroTargetedSweepPhase,
                kTitleIntroTargetedSweepPhaseCount>
        targeted_sweep_phases{};
    std::array<TitleIntroLinearSweepPhase, kTitleIntroLinearSweepPhaseCount>
        linear_sweep_a_phases{};
    std::array<TitleIntroLinearSweepPhase, kTitleIntroLinearSweepPhaseCount>
        linear_sweep_b_phases{};
    std::array<TitleIntroFinalProjectionPhase,
                kTitleIntroFinalProjectionPhaseCount>
        final_projection_phases{};

    std::array<TitleIntroStageTimedPhase, kTitleIntroStageTimedPhaseCount>
        stage_timed_phases{};
    std::array<std::uint32_t, kTitleIntroPulsePhaseCount> pulse_descriptors{};
    std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>
        landing_phases{};

    TitleIntroOrchestratorAssets orchestrator_assets{};
};

// Source table values recovered from the 0x59C5E/0x59D7C/0x5AA96/0x5AD1A
// call chain. This is immutable conversion data, not a runtime ROM table.
const OpeningIntroSourcePlan& opening_intro_source_plan();

struct OpeningIntroCallbacks {
    // Source frame and mode-service boundaries.
    std::function<void()> wait_for_vblank;
    std::function<void()> sample_controller;
    std::function<void()> latch_controller_events;
    std::function<void()> prepare_mode_entry;
    std::function<void(std::uint16_t)> service_audio_command;
    std::function<void()> service_audio;
    std::function<void()> step_palette_transition;
    std::function<void()> enter_post_intro_mode;

    // Host presentation boundaries. These consume direct assets and never
    // expose VDP/name-table operations to the Qt runtime.
    std::function<void()> clear_presentation;
    std::function<void()> begin_frame;
    std::function<void(OpeningIntroStage)> stage_changed;
    std::function<void(OpeningIntroScene)> scene_changed;
    std::function<void(const OpeningIntroSpriteRequest&)> draw_sprite;
    std::function<void(const OpeningIntroTextRequest&)> render_text;
    std::function<void(std::string_view)> source_boundary;
    std::function<bool()> stop_requested;
};

struct OpeningIntroState {
    OpeningIntroStage stage = OpeningIntroStage::not_started;
    OpeningIntroScene scene = OpeningIntroScene::seattle;
    std::uint32_t source_frame = 0;
    std::uint16_t stage2_first_loop_frame = 0;
    std::uint16_t stage2_second_loop_frame = 0;
    std::uint16_t orchestrator_loop_frame = 0;
    std::uint16_t stage_loop_frame = 0;
    std::uint16_t scroll_accumulator = 0;
    bool source_stop_marker = false;

    TitleIntroScheduleState schedule{};
    TitleIntroStage2State stage2{};
    TitleIntroOrchestratorState orchestrator{};
    TitleIntroStageLoopState stage_loop{};

    TitleIntroAnimationState animation{};
    TitleIntroOverlayState overlay{};
    TitleIntroSequenceState sequence{};
    TitleIntroPersistentSequenceState persistent{};
    TitleIntroMotionState motion{};
    TitleIntroDualSpriteState dual{};
    TitleIntroTimedSpriteState timed{};
    TitleIntroTargetedSweepState targeted_sweep{};
    TitleIntroLinearSweepState linear_sweep_a{};
    TitleIntroLinearSweepState linear_sweep_b{};
    TitleIntroFinalProjectionState final_projection{};
};

enum class OpeningIntroStartResult {
    started,
    callback_unresolved,
};

enum class OpeningIntroAdvanceResult {
    frame_advanced,
    source_complete,
    interrupted,
    callback_unresolved,
};

// Incremental conversion-side counterpart of ROM 0x59C5E. It composes the
// already converted schedule, stage, animation, transfer, and text routines;
// it is deliberately separate from both the emulator and shadowrun_port.
OpeningIntroStartResult begin_opening_intro(
    OpeningIntroState& state, const OpeningIntroCallbacks& callbacks);

// Advance exactly one source VBlank pass. A caller may paint after every
// return, which preserves the original timing without a bitmap flipbook.
OpeningIntroAdvanceResult advance_opening_intro_frame(
    OpeningIntroState& state, const OpeningIntroCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion::integration
