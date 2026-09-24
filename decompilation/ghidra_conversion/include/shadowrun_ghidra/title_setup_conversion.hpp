#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleSetupState {
    // ROM 0x1D7A clears these four title-owned words before rebuilding the
    // title scene. Their individual source meanings remain under analysis.
    std::array<std::uint16_t, 4> cleared_words{};
    std::uint8_t palette_flags = 0;
};

struct TitleSetupCallbacks {
    std::function<void()> prepare_scene_resources;
    std::function<void()> load_title_resources;
    std::function<void()> load_title_font;
    std::function<void(std::uint16_t frame_count)> advance_palette_frames;
};

// ROM 0x1D7A: reset title-owned state, perform the source setup sequence,
// publish palette bits 0, 2, and 3, then advance the palette for 17 frames.
// The resource and presentation operations are typed host boundaries; this
// module does not reproduce the Genesis VDP or ROM-addressed memory.
void setup_title_state(TitleSetupState& state,
                       const TitleSetupCallbacks& callbacks);

enum class TitleTextAsset {
    prompt_variant_a,
    prompt_variant_b,
    copyright_line_1,
    copyright_line_2,
    copyright_line_3,
    copyright_line_4,
};

struct TitleTextRenderRequest {
    TitleTextAsset asset;
    std::uint16_t destination_tile_base;
    std::uint16_t palette_flags;
};

using TitleTextDrawBoundary =
    std::function<void(const TitleTextRenderRequest& request)>;

// ROM 0x1DCA / 0x1DE0: select one title prompt record for the common text
// writer. ROM 0x1DF6 submits the four copyright records in order.
void draw_title_prompt_variant_a(const TitleTextDrawBoundary& draw_text);
void draw_title_prompt_variant_b(const TitleTextDrawBoundary& draw_text);
void draw_title_copyright(const TitleTextDrawBoundary& draw_text);

} // namespace shadowrun::ghidra_conversion
