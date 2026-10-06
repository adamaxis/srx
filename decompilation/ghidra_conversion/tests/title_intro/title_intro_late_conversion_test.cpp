#include "shadowrun_ghidra/title_intro/title_intro_late_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<std::uint32_t, kTitleIntroThreeTextCount> assets{{
        0x1000U, 0x1001U, 0x1002U}};
    std::vector<TitleIntroLateTextRenderRequest> text;
    const auto collect_text = [&text](const TitleIntroLateTextRenderRequest& r) {
        text.push_back(r);
    };
    render_title_intro_middle_text_batch(assets, collect_text);
    if (text.size() != 3U || text[0].destination_word != 0x2186U ||
        text[2].text_asset != 0x1002U || text[0].palette_flags != 0xE000U) {
        return 1;
    }
    text.clear();
    render_title_intro_center_text_batch(0x1100U, collect_text);
    if (text.size() != 3U || text[1].text_asset != 0x1100U ||
        text[2].destination_word != 0x2386U) {
        return 2;
    }
    text.clear();
    render_title_intro_late_text_batch(assets, collect_text);
    if (text.size() != 3U || text[0].destination_word != 0x2106U ||
        text[2].destination_word != 0x2306U) {
        return 3;
    }
    text.clear();
    render_title_intro_scroll_text_batch(0x1200U, collect_text);
    if (text.size() != 5U || text[0].destination_word != 0x2106U ||
        text[4].destination_word != 0x2306U ||
        text[0].palette_flags != 0xA000U) {
        return 4;
    }

    TitleIntroScrollState scroll{-0x190, false, 0U, 0U};
    TitleIntroScrollFillRequest fill{};
    advance_title_intro_scroll(scroll, [&fill](const auto& request) {
        fill = request;
    });
    if (!scroll.transition_fill_started || fill.destination_word != 0x074AU ||
        fill.words_per_row != 5U || fill.row_count != 8U ||
        scroll.x_offset != -0x192) {
        return 5;
    }
    scroll.x_offset = -0x1B0;
    advance_title_intro_scroll_fade(scroll, [] {});
    if (scroll.fade_counter != 1U || scroll.dirty_flags != 0x0CU) {
        return 6;
    }

    TitleIntroTripletState triplet{{1U, 2U, 3U}, false};
    if (advance_title_intro_triplet(triplet, 7U) != 7U ||
        advance_title_intro_triplet(triplet, 8U) != 1U ||
        triplet.values != std::array<std::uint16_t, 3>{{2U, 3U, 1U}} ||
        !triplet.dirty) {
        return 7;
    }

    TitleIntroPulseState pulse{-0x1B8, 10U, 0U, 0U, 0U};
    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount> descriptors{{
        0x2000U, 0x2001U, 0x2002U, 0x2003U, 0x2004U, 0x2005U}};
    std::vector<TitleIntroSpriteRequest> sprites;
    advance_title_intro_pulse(pulse, descriptors,
                               [&sprites](const auto& request) {
                                   sprites.push_back(request);
                               });
    if (sprites.size() != 1U || sprites[0].sprite_descriptor != 0x2000U ||
        sprites[0].x != 0x0044U || sprites[0].y != 0x00EFU ||
        pulse.next_event_frame != 16U) {
        return 8;
    }
    for (std::size_t phase = 1U; phase < descriptors.size(); ++phase) {
        pulse.current_frame = pulse.next_event_frame;
        sprites.clear();
        advance_title_intro_pulse(pulse, descriptors,
                                  [&sprites](const auto& request) {
                                      sprites.push_back(request);
                                  });
        if (sprites.size() != 1U ||
            sprites[0].sprite_descriptor != descriptors[phase] ||
            sprites[0].x != 0x0044U || sprites[0].y != 0x00EFU ||
            sprites[0].palette_flags != 0x2000U ||
            pulse.phase != (phase + 1U) % descriptors.size() ||
            pulse.next_event_frame != pulse.current_frame + 6U) {
            return 9;
        }
    }
    pulse.current_frame = pulse.next_event_frame;
    sprites.clear();
    advance_title_intro_pulse(pulse, descriptors,
                              [&sprites](const auto& request) {
                                  sprites.push_back(request);
                              });
    if (sprites.size() != 1U || sprites[0].sprite_descriptor != 0x2000U ||
        pulse.phase != 1U || pulse.next_event_frame != 52U) {
        return 10;
    }

    TitleIntroPulseState outside_window{-0x1BA, 10U, 0U, 0U, 0U};
    sprites.clear();
    advance_title_intro_pulse(outside_window, descriptors,
                              [&sprites](const auto& request) {
                                  sprites.push_back(request);
                              });
    if (!sprites.empty() || outside_window.phase != 0U ||
        outside_window.next_event_frame != 0U ||
        outside_window.sprite_descriptor != 0U) {
        return 11;
    }

    TitleIntroLandingState landing{};
    reset_title_intro_landing(landing);
    if (landing.x_accumulator != 0x7C00U ||
        landing.y_accumulator != 0xB700U || landing.phase != 0U) {
        return 12;
    }
    landing.current_frame = 0x226U;
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>
        phases{{{0x3000U, 0x00F1U, 0x0071U, 0x0024U},
                {0x3001U, 0x006EU, 0x0000U, 0x005AU},
                {0x3002U, 0x0071U, 0x0071U, 0x0048U},
                {0x3003U, 0x002AU, 0x0071U, 0x0012U},
                {0x3004U, 0x0000U, 0x0038U, 0x0024U},
                {0x3005U, 0x0063U, 0x0038U, 0x0012U},
                {0x3006U, 0x0000U, 0x0000U, 0x0040U}}};
    sprites.clear();
    advance_title_intro_landing(landing, phases, 0x4000U,
                                 [&sprites](const auto& request) {
                                     sprites.push_back(request);
                                 });
    if (sprites.size() != 1U || sprites[0].sprite_descriptor != 0x3000U ||
        sprites[0].x != 0x7CU || sprites[0].y != 0x00DFU ||
        landing.phase != 1U ||
        landing.next_event_frame != 0x24AU) {
        return 13;
    }
    landing.current_frame = 0x24AU;
    landing.y_accumulator = 0xE800U;
    sprites.clear();
    advance_title_intro_landing(landing, phases, 0x4000U,
                                 [&sprites](const auto& request) {
                                     sprites.push_back(request);
                                 });
    if (sprites.size() != 1U || sprites[0].sprite_descriptor != 0x4000U ||
        sprites[0].y != 0x0110U) {
        return 14;
    }

    TitleIntroEntityColorState colors{};
    set_title_intro_entity_color_words(colors, 2U, 0x0EEEU);
    step_title_intro_entity_color_words(colors, 2U);
    if (colors.words[2][0] != 0x0CCCU || !colors.dirty) {
        return 15;
    }
    return 0;
}
