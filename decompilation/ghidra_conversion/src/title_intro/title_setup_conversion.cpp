#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::uint16_t kTitlePaletteFlags = 0x2000U;

void submit(const TitleTextDrawBoundary& draw_text,
            const TitleTextAsset asset,
            const std::uint16_t destination_tile_base) {
    if (draw_text) {
        draw_text({asset, destination_tile_base, kTitlePaletteFlags});
    }
}

} // namespace

std::uint16_t advance_title_palette_fade(
    const TitlePaletteFadeCallbacks& callbacks) {
    constexpr std::uint16_t kFadePassCount = 17U;
    for (std::uint16_t pass = 0; pass < kFadePassCount; ++pass) {
        if (callbacks.wait_for_palette_frame) {
            callbacks.wait_for_palette_frame();
        }
        if (callbacks.step_palette_toward_target) {
            callbacks.step_palette_toward_target();
        }
        if (callbacks.mark_palette_dirty) {
            callbacks.mark_palette_dirty();
        }
    }
    return kFadePassCount;
}

void setup_title_state(TitleSetupState& state,
                       const TitleSetupCallbacks& callbacks) {
    state.cleared_words.fill(0U);

    if (callbacks.prepare_scene_resources) {
        callbacks.prepare_scene_resources();
    }
    if (callbacks.load_title_resources) {
        callbacks.load_title_resources();
    }
    if (callbacks.load_title_font) {
        callbacks.load_title_font();
    }

    state.palette_flags = static_cast<std::uint8_t>(
        state.palette_flags | 0x0DU);
    if (callbacks.advance_palette_frames) {
        callbacks.advance_palette_frames(17U);
    }
}

void draw_title_prompt_variant_a(const TitleTextDrawBoundary& draw_text) {
    submit(draw_text, TitleTextAsset::prompt_variant_a, 0xCA84U);
}

void draw_title_prompt_variant_b(const TitleTextDrawBoundary& draw_text) {
    submit(draw_text, TitleTextAsset::prompt_variant_b, 0xCA84U);
}

void draw_title_copyright(const TitleTextDrawBoundary& draw_text) {
    submit(draw_text, TitleTextAsset::copyright_line_1, 0xCB04U);
    submit(draw_text, TitleTextAsset::copyright_line_2, 0xCB84U);
    submit(draw_text, TitleTextAsset::copyright_line_3, 0xCC04U);
    submit(draw_text, TitleTextAsset::copyright_line_4, 0xCC84U);
}

} // namespace shadowrun::ghidra_conversion
