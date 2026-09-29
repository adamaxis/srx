#include "shadowrun_ghidra/title_intro/title_intro_transfer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void clear_title_intro_display_buffer(
    const std::function<void(const TitleIntroDisplayClearRequest&)>& clear) {
    if (clear) {
        clear({});
    }
}

void fill_title_intro_word_block(
    const TitleIntroWordFillPlan& plan,
    const std::function<void(const TitleIntroFillRowRequest&)>& write_row) {
    if (!write_row) {
        return;
    }
    auto destination = plan.destination_offset;
    for (std::uint16_t row = 0; row < plan.row_count; ++row) {
        write_row({destination, plan.word_count, plan.fill_word, row});
        destination = static_cast<std::uint16_t>(destination + plan.row_stride);
    }
}

void reset_world_frame_service_state(WorldFrameServiceState& state) {
    state.words.fill(0U);
    state.words[0] = 0x11C0U;
    state.words[2] = 0xFE80U;
    state.words[0x11] = 0xFFFFU;
    state.words[0x12] = 0x00B4U;
}

WorldSceneEffectSpriteBuild advance_world_scene_effect_timer_a(
    WorldFrameServiceState& state) {
    auto countdown = static_cast<std::int16_t>(state.words[3]);
    --countdown;
    if (countdown < 0) {
        countdown = 0x14;
        auto frame = static_cast<std::uint16_t>(state.words[0x0B] + 1U);
        if (frame >= 9U) {
            frame = 0U;
        }
        state.words[0x0B] = frame;
    }
    state.words[3] = static_cast<std::uint16_t>(countdown);

    WorldSceneEffectSpriteBuild build{};
    const auto heading =
        static_cast<std::size_t>(state.words[0x0C] %
                                 kWorldSceneEffectHeadingOffsets.size());
    const auto origin_x = static_cast<std::int16_t>(state.words[5]);
    const auto origin_y = static_cast<std::int16_t>(state.words[6]);
    build.x = static_cast<std::int16_t>(
        origin_x + kWorldSceneEffectHeadingOffsets[heading][0]);
    build.y = static_cast<std::int16_t>(
        origin_y + kWorldSceneEffectHeadingOffsets[heading][1]);
    build.flags = 0x4000U;
    const auto frame = static_cast<std::size_t>(state.words[0x0B]);
    if (frame < kWorldSceneEffectTimerALists.size()) {
        build.source = kWorldSceneEffectTimerALists[frame];
    }
    return build;
}

} // namespace shadowrun::ghidra_conversion
