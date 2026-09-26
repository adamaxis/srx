#pragma once

#include "shadowrun_ghidra/title_intro/title_intro_stage_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_intro_transfer_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroSourceStreamRequest {
    std::uint32_t asset = 0;
    std::uint32_t decoded_bytes = 0;
};

struct TitleIntroHudSpriteStreamRequest {
    std::uint32_t asset = 0;
    std::uint16_t destination_offset = 0;
    std::uint16_t sprite_word_count = 0;
    std::uint16_t row_count = 0;
    std::uint16_t palette_flags = 0;
    std::uint16_t attribute_flags = 0;
};

struct TitleIntroOrchestratorAssets {
    std::array<std::uint32_t, kTitleIntroStageTextCount> text_assets{};
    std::uint32_t first_graphics_asset = 0;
    std::uint32_t second_graphics_asset = 0;
    std::uint32_t palette_asset = 0;
    std::uint32_t hud_sprite_asset = 0;
};

struct TitleIntroOrchestratorState {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t loop_frame = 0;
    TitleIntroStageFrameState frame_state{};
    TitleIntroStageTimedState timed_state{};
    TitleIntroStageProjectionState projection_state{};
};

struct TitleIntroOrchestratorCallbacks {
    std::function<void()> reset_upload_state;
    std::function<void(const TitleIntroDisplayClearRequest&)>
        clear_display_buffer;
    std::function<void()> clear_plane_a;
    std::function<void()> clear_plane_b;
    std::function<void(const TitleIntroSourceStreamRequest&)>
        decode_and_upload;
    std::function<void()> load_title_font;
    std::function<void(const TitleIntroStageTextRequest&)> render_text;
    std::function<void()> wait_for_vblank;
    std::function<void(std::uint32_t)> copy_target_palette;
    std::function<void(const TitleIntroFillRowRequest&)> write_fill_row;
    std::function<void(const TitleIntroHudSpriteStreamRequest&)>
        render_hud_sprites;
    std::function<void()> advance_palette_fade;
    std::function<void()> sample_controller;
    std::function<void()> latch_controller_events;
    std::function<void(std::uint16_t)> service_step;
    std::function<void(const TitleIntroStageSpriteRequest&)> draw_sprite;
    std::function<bool()> stop_requested;
};

enum class TitleIntroOrchestratorStopReason {
    source_complete,
    external_stop,
};

// ROM 0x5AA96: prepare the secondary intro resources, run the source-ordered
// stage setup, then service 0x115 frames of stage animation and color steps.
TitleIntroOrchestratorStopReason run_title_intro_orchestrator(
    TitleIntroOrchestratorState& state,
    const TitleIntroOrchestratorAssets& assets,
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>& timed_phases,
    std::uint32_t base_sprite_descriptor,
    std::uint32_t projection_sprite_descriptor,
    const TitleIntroOrchestratorCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
