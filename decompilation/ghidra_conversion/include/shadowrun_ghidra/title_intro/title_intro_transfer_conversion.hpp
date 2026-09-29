#pragma once

#include "shadowrun_ghidra/rendering/sprite_record_builder_conversion.hpp"

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

constexpr std::array<std::uint32_t, 10> kWorldSceneEffectTimerBLists{
    0x0001FEADEU, 0x0001FEB02U, 0x0001FEB26U, 0x0001FEB14U, 0x0001FEAF0U,
    0x0001FEAF0U, 0x0001FEB14U, 0x0001FEB26U, 0x0001FEB02U, 0x0001FEADEU};

// ROM 0x5D5FC: +0x1A reload 0xA, +0x1C wrap 0..9, D2-0x18.
WorldSceneEffectSpriteBuild advance_world_scene_effect_timer_b(
    WorldFrameServiceState& state);

constexpr std::array<std::uint32_t, 40> kWorldSceneEffectTimerCLists{
    0x0001FEC34U, 0x0001FEC02U, 0x0001FEBD0U, 0x0001FEB8EU, 0x00001D47CU,
    0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x00001D47CU,
    0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x0001FEB7AU,
    0x0001FEB7AU, 0x0001FEB66U, 0x0001FEB66U, 0x0001FEB52U, 0x0001FEB52U,
    0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x00001D47CU,
    0x00001D47CU, 0x00001D47CU, 0x0001FEB84U, 0x0001FEB84U, 0x0001FEB70U,
    0x0001FEB70U, 0x0001FEB5CU, 0x0001FEB5CU, 0x00001D47CU, 0x00001D47CU,
    0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x00001D47CU, 0x00001D47CU};

// ROM 0x5D65A: +0x20 reload 4, +0x1E wrap 0..0x27, D2+0x30 D1+0x12.
WorldSceneEffectSpriteBuild advance_world_scene_effect_timer_c(
    WorldFrameServiceState& state);

constexpr std::array<SpriteSourceRecord, 2> kTitleIntroSceneEntitySource{{
    {6, -15, 4, 0x04F6U},
    {1, -7, 8, 0x04F8U},
}};

// ROM 0x5A37C: A2=0x1DF1B2, D1=FFDD46, D2=FFDD48, D6=0x2000, jmp 0x1378.
std::vector<BuiltSpriteRecord> render_title_intro_scene_entities(
    std::int16_t y_offset,
    std::int16_t x_offset,
    std::uint8_t first_sequence);

} // namespace shadowrun::ghidra_conversion
