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
    wilderness_overlay,
    wilderness_final_overlay,
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
    wilderness_map_left,
    wilderness_map_center,
    wilderness_map_right,
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

// Intro SAT lists are extracted by blit_genesis_sat_entry at dump origin
// 0x100. Live 0x1378 SAT is origin $80; host crop top-left is
// dump_origin + live_sat - 0x100.
constexpr int kIntroSatExtractOrigin = 0x100;

// Plane-A $074A is a 27x8 nametable (col 37, row 14), not a SAT cell.
constexpr int kAircraftPlaneNametableX = 0x128;
constexpr int kAircraftPlaneNametableY = 0x70;

struct OpeningIntroDrawPoint {
    int x = 0;
    int y = 0;
};

OpeningIntroDrawPoint opening_intro_sat_draw_point(
    std::uint16_t sat_x, std::uint16_t sat_y, int origin_x, int origin_y);
OpeningIntroDrawPoint opening_intro_aircraft_plane_draw_point(
    std::int16_t scroll_x);

// ROM 0x5B378: D4=0x40E, five 0x5B36E offsets, 0x5B364 D1/D2 pairs, inner
// dbf so tiles are D1+1 by D2+1. Plane B (seattle) stays; Plane A is the
// outline. Five passes × two vblanks.
constexpr std::uint16_t kSeattleOutlineVram = 0x040EU;
constexpr std::uint16_t kSeattleOutlineHoldFrames = 10U;

struct OpeningIntroOutlinePlan {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

OpeningIntroOutlinePlan opening_intro_seattle_outline_plan(
    std::uint8_t phase);

// The text records are the source strings at 0x5B7D8..0x5BA4C, represented as
// typed host records. The blank record at 0x5B7B2 is kept separately because
// 0x5AE3A writes it across five rows to clear the previous story page. The Qt
// frontend uses these records with the extracted source font; no runtime ROM
// text decoder is involved.
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
    blank,
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
    post_intro,
    complete,
    interrupted,
};

// ROM 0x134D2 FFE18B!=0: 0x13726 mode 4 (5*2 vblanks) then DBF #0x40, then
// 0x13726 mode 2 and DBF #0x60, before returning into 0x59C5E's field handoff.
constexpr std::uint16_t kPostIntroCabModeFourFrames = 0x41U;
constexpr std::uint16_t kPostIntroCabModeTwoFrames = 0x61U;
constexpr std::uint16_t kPostIntroCabShrinkFrames = 10U;
constexpr std::uint16_t kPostIntroCabTotalFrames =
    kPostIntroCabShrinkFrames + kPostIntroCabModeFourFrames +
    kPostIntroCabShrinkFrames + kPostIntroCabModeTwoFrames;

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
    // 0x5B4A8's three initial Plane-A name-table blocks. These remain
    // separate source layers; the runtime never consumes a composite
    // wilderness background plate.
    std::array<std::uint32_t, 3> wilderness_map_descriptors{};
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
    // 0x131C: prepare_title_resources drives the current palette toward
    // black before a new title resource set is installed. This is a source
    // presentation boundary, not a host palette implementation.
    std::function<void()> fade_palette_to_black;
    // 0x12F6: after the new target palette is copied, advance the current
    // palette toward that target for the source's initial reveal.
    std::function<void()> advance_palette_fade;
    std::function<void()> enter_post_intro_mode;

    // Host presentation boundaries. These consume direct assets and never
    // expose VDP/name-table operations to the Qt runtime.
    std::function<void()> clear_presentation;
    std::function<void()> begin_frame;
    std::function<void(OpeningIntroStage)> stage_changed;
    std::function<void(OpeningIntroScene)> scene_changed;
    std::function<void(const OpeningIntroSpriteRequest&)> draw_sprite;
    // ROM 0x5A252/0x5A0B2 fill the right initial map block at $03B8/$3B8
    // after the terminal fade sequence. This is a source map clear, not a
    // missing sprite asset.
    std::function<void()> clear_wilderness_right_map;
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
    std::uint16_t post_intro_frame = 0;
    std::uint8_t post_intro_mode = 4;
    std::uint8_t post_intro_phase = 0;
    bool post_intro_caption_visible = false;
    bool seattle_plane_cleared = false;
    std::uint16_t seattle_outline_frame = 0;
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
    // 0x5A0B2 fades entity 0's palette line and 0x5A252 fades entity 1's
    // palette line. Keeping these typed source words in the integration state
    // lets a host presentation reproduce the fade without recreating CRAM.
    TitleIntroEntityColorState entity_colors{};
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
