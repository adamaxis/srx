#include "shadowrun_ghidra/title_setup_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> fade_order;
    if (!expect(advance_title_palette_fade(
                    {[&fade_order] { fade_order.push_back(1); },
                     [&fade_order] { fade_order.push_back(2); },
                     [&fade_order] { fade_order.push_back(3); }}) == 17U &&
                fade_order.size() == 51U && fade_order[0] == 1 &&
                fade_order[1] == 2 && fade_order[2] == 3 &&
                fade_order[48] == 1 && fade_order[49] == 2 &&
                fade_order[50] == 3)) {
        return 1;
    }

    TitleSetupState state{{{9U, 8U, 7U, 6U}}, 0x20U};
    std::vector<int> order;
    std::uint16_t palette_frames = 0U;
    setup_title_state(
        state,
        {
            [&order] { order.push_back(1); },
            [&order] { order.push_back(2); },
            [&order] { order.push_back(3); },
            [&order, &palette_frames](const std::uint16_t frames) {
                order.push_back(4);
                palette_frames = frames;
            },
        });
    if (!expect(state.cleared_words ==
                    std::array<std::uint16_t, 4>{0U, 0U, 0U, 0U} &&
                state.palette_flags == 0x2DU && order ==
                    std::vector<int>{1, 2, 3, 4} &&
                palette_frames == 17U)) {
        return 2;
    }

    std::vector<TitleTextRenderRequest> requests;
    const auto capture = [&requests](const TitleTextRenderRequest& request) {
        requests.push_back(request);
    };
    draw_title_prompt_variant_a(capture);
    draw_title_prompt_variant_b(capture);
    draw_title_copyright(capture);
    if (!expect(requests.size() == 6U &&
                requests[0].asset == TitleTextAsset::prompt_variant_a &&
                requests[0].destination_tile_base == 0xCA84U &&
                requests[1].asset == TitleTextAsset::prompt_variant_b &&
                requests[4].asset == TitleTextAsset::copyright_line_3 &&
                requests[4].destination_tile_base == 0xCC04U &&
                requests[5].asset == TitleTextAsset::copyright_line_4 &&
                requests[5].destination_tile_base == 0xCC84U &&
                requests[5].palette_flags == 0x2000U)) {
        return 3;
    }

    return 0;
}
