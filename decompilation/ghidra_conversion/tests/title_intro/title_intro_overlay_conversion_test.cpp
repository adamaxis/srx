#include "shadowrun_ghidra/title_intro/title_intro_overlay_conversion.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroOverlaySchedule schedule{7U, 4U, 0x1234U};
    reset_title_intro_overlay_schedule(schedule);
    if (schedule.next_sprite_frame != 0U || schedule.phase_index != 0U ||
        schedule.scroll_accumulator != 0U) {
        return 1;
    }

    std::array<TitleIntroOverlayPhase, kTitleIntroOverlayPhaseCount> phases{};
    for (std::size_t index = 0; index < phases.size(); ++index) {
        phases[index] = {static_cast<std::uint32_t>(0x1000U + index),
                         static_cast<std::uint16_t>(index + 2U)};
    }
    TitleIntroOverlayState state{};
    state.current_frame = 0x0120U;
    state.animation_start_frame = 0;
    state.schedule.next_sprite_frame = 0x0120U;
    std::vector<std::uint16_t> audio;
    std::vector<TitleIntroSpriteStreamRequest> streams;
    std::vector<std::string> presentation_order;
    int headers = 0;

    const TitleIntroOverlayCallbacks callbacks{
        [&audio](const std::uint16_t event) { audio.push_back(event); },
        [&headers, &presentation_order] {
            ++headers;
            presentation_order.push_back("header");
        },
        [&presentation_order] {
            presentation_order.push_back("clear-left");
        },
        [&streams, &presentation_order](
            const TitleIntroSpriteStreamRequest& request) {
            streams.push_back(request);
            presentation_order.push_back("stream");
        },
    };

    if (advance_title_intro_overlay(state, phases, 0x9000U, callbacks) !=
            TitleIntroOverlayAdvanceResult::phase_rendered ||
        state.horizontal_scroll != 0U || !state.horizontal_scroll_written ||
        state.schedule.next_sprite_frame != 0x0122U ||
        state.schedule.phase_index != 1U || state.dirty_flags != 0x04U ||
        streams.size() != 1U || streams.front().sprite_descriptor != 0x1000U) {
        return 2;
    }

    state.current_frame = 0x0168U;
    presentation_order.clear();
    if (advance_title_intro_overlay(state, phases, 0x9000U, callbacks) !=
            TitleIntroOverlayAdvanceResult::final_overlay_rendered ||
        audio != std::vector<std::uint16_t>{0x2EU} || headers != 1 ||
        presentation_order !=
            std::vector<std::string>{"header", "clear-left", "stream"} ||
        streams.size() != 2U ||
        streams.back().sprite_descriptor != 0x9000U ||
        streams.back().destination_word != 0x0786U ||
        state.horizontal_scroll != 0U || !state.horizontal_scroll_written) {
        return 3;
    }

    state.current_frame = 0x0169U;
    if (advance_title_intro_overlay(state, phases, 0x9000U, callbacks) !=
        TitleIntroOverlayAdvanceResult::no_op) {
        return 4;
    }
    return 0;
}
