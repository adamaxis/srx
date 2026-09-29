#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroDisplayClearRequest {
    std::uint16_t transfer_command = 0x3000U;
    std::uint32_t packed_fill_word = 0x07FF07FFU;
    std::uint16_t long_write_count = 0x0400U;
};

// ROM 0x5B29A: clear the title-stage display buffer as one typed host plan.
void clear_title_intro_display_buffer(
    const std::function<void(const TitleIntroDisplayClearRequest&)>& clear);

struct TitleIntroFillRowRequest {
    std::uint16_t destination_offset = 0;
    std::uint16_t word_count = 0;
    std::uint16_t fill_word = 0;
    std::uint16_t row_index = 0;
};

struct TitleIntroWordFillPlan {
    std::uint16_t destination_offset = 0;
    std::uint16_t word_count = 0;
    std::uint16_t row_count = 0;
    std::uint16_t row_stride = 0x0080U;
    std::uint16_t fill_word = 0x07FFU;
};

// ROM 0x5B32C: emit each repeated fill row with its source destination step.
void fill_title_intro_word_block(
    const TitleIntroWordFillPlan& plan,
    const std::function<void(const TitleIntroFillRowRequest&)>& write_row);

constexpr std::size_t kWorldFrameServiceWordCount = 34U;

struct WorldFrameServiceState {
    std::array<std::uint16_t, kWorldFrameServiceWordCount> words{};
};

// ROM 0x5D260: clear 17 longs at FFF11E, then +0=0x11C0, +4=-0x180,
// +0x22=-1, +0x24=0xB4.
void reset_world_frame_service_state(WorldFrameServiceState& state);

constexpr std::array<std::array<std::int16_t, 2>, 8> kWorldSceneEffectHeadingOffsets{{
    {{2, 41}}, {{5, 41}}, {{4, 41}}, {{7, 41}},
    {{6, 41}}, {{7, 41}}, {{5, 41}}, {{5, 41}},
}};
constexpr std::array<std::uint32_t, 9> kWorldSceneEffectTimerALists{
    0x0001FEB38U, 0x0001FEB38U, 0x0001FEB38U, 0x0001FEB38U, 0x0001FEA52U,
    0x0001FEB38U, 0x0001FEA52U, 0x0001FEB38U, 0x0001FEA52U};

struct WorldSceneEffectSpriteBuild {
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::uint16_t flags = 0x4000U;
    std::uint32_t source = 0;
};

// ROM 0x5D5A2: +6 countdown reload 0x14, +0x16 wrap 0..8, then 0x1378.
WorldSceneEffectSpriteBuild advance_world_scene_effect_timer_a(
    WorldFrameServiceState& state);

} // namespace shadowrun::ghidra_conversion
