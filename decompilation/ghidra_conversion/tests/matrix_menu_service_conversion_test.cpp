#include "shadowrun_ghidra/matrix_menu_service_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

using namespace shadowrun::ghidra_conversion;

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    MatrixProgramSlotCleanupState cleanup{};
    cleanup.program_codes = {3U, 5U, 6U, 7U, 9U};
    cleanup.visual_states = {1U, 1U, 1U, 1U, 1U};
    cleanup.menu_flags = 0xFFU;
    cleanup.pending_status = 4U;
    cleanup.secondary_status = 4U;
    cleanup.effect_marker = 4U;
    std::vector<std::string> cleanup_events;
    if (!expect(clear_matrix_program_slot(
                    cleanup, 0U,
                    {[&] { cleanup_events.emplace_back("repeat"); },
                     [&] { cleanup_events.emplace_back("grid"); },
                     [&] { cleanup_events.emplace_back("rows"); }})) ||
        !expect(cleanup.program_codes[0] == 0xFFU &&
                cleanup.visual_states[0] == 6U &&
                cleanup.pending_status == 0U &&
                cleanup_events == std::vector<std::string>{
                                      "repeat", "grid", "rows"})) {
        return 1;
    }
    if (!expect(clear_matrix_program_slot(cleanup, 1U, {}) &&
                cleanup.secondary_status == 0U) ||
        !expect(clear_matrix_program_slot(cleanup, 2U, {}) &&
                (cleanup.menu_flags & 0x04U) == 0U) ||
        !expect(clear_matrix_program_slot(cleanup, 3U, {}) &&
                (cleanup.menu_flags & 0x02U) == 0U &&
                cleanup.effect_marker == 0U) ||
        !expect(!clear_matrix_program_slot(cleanup, 8U, {}))) {
        return 2;
    }

    MatrixProgramLoadingState loading{};
    loading.overlay_countdown = 0U;
    loading.loading_active = true;
    loading.progress_delay = -1;
    loading.visual_states = {5U, 6U, 6U, 6U, 6U};
    loading.slot_delays = {3, 0, 0, 0, 0};
    std::vector<std::string> loading_events;
    const auto completed = advance_matrix_program_loading(
        loading,
        {[&] { loading_events.emplace_back("rows"); },
         [&] { loading_events.emplace_back("overlay"); },
         [&](const std::size_t slot) {
             loading_events.emplace_back("grid" + std::to_string(slot));
         },
         [&](const std::size_t slot) {
             loading_events.emplace_back("select" + std::to_string(slot));
         },
         [&] { loading_events.emplace_back("text"); }});
    if (!expect(completed == MatrixProgramLoadingEvent::slot_completed &&
                loading.overlay_countdown == 0x28U &&
                loading.selected_slot == 0U &&
                loading.progress_delay == 3 &&
                loading_events == std::vector<std::string>{
                                      "rows", "overlay", "grid0",
                                      "select0", "text"})) {
        return 3;
    }
    loading.progress_delay = 2;
    loading.visual_states[0] = 1U;
    loading_events.clear();
    if (!expect(advance_matrix_program_loading(
                    loading,
                    {[&] { loading_events.emplace_back("rows"); },
                     [&] { loading_events.emplace_back("overlay"); },
                     [&](const std::size_t slot) {
                         loading_events.emplace_back(
                             "grid" + std::to_string(slot));
                     },
                     {},
                     {}}) == MatrixProgramLoadingEvent::none &&
                loading_events == std::vector<std::string>{"overlay"})) {
        return 4;
    }

    HudModeTransitionState transition{};
    transition.transition_gate = 0;
    std::vector<std::string> transition_events;
    if (!expect(update_hud_mode_transition(
                    transition, 0,
                    {[&] { transition_events.emplace_back("fade"); },
                     [&] { transition_events.emplace_back("world"); },
                     [&] { transition_events.emplace_back("text"); },
                     [&] { transition_events.emplace_back("vblank"); },
                     [&] { transition_events.emplace_back("controller"); },
                     [&](const std::int8_t) {
                         transition_events.emplace_back("audio");
                     }}) == HudModeTransitionEvent::rendered &&
                transition_events == std::vector<std::string>{"text"})) {
        return 5;
    }
    transition_events.clear();
    transition.transition_gate = 1;
    if (!expect(update_hud_mode_transition(
                    transition, 2,
                    {[&] { transition_events.emplace_back("fade"); },
                     [&] { transition_events.emplace_back("world"); },
                     [&] { transition_events.emplace_back("text"); },
                     [&] { transition_events.emplace_back("vblank"); },
                     [&] { transition_events.emplace_back("controller"); },
                     [&](const std::int8_t) {
                         transition_events.emplace_back("audio");
                     }}) == HudModeTransitionEvent::completed_transition &&
                transition_events == std::vector<std::string>{
                                      "fade", "world", "text", "vblank",
                                      "controller"})) {
        return 6;
    }
    transition_events.clear();
    transition.transition_gate = 0;
    transition.published_variant = 0;
    if (!expect(update_hud_mode_transition(
                    transition, -2,
                    {[&] { transition_events.emplace_back("fade"); },
                     [&] { transition_events.emplace_back("world"); },
                     [&] { transition_events.emplace_back("text"); },
                     [&] { transition_events.emplace_back("vblank"); },
                     [&] { transition_events.emplace_back("controller"); },
                     [&](const std::int8_t) {
                         transition_events.emplace_back("audio");
                     }}) == HudModeTransitionEvent::audio_published &&
                transition.mode_variant == -1 &&
                transition_events == std::vector<std::string>{"text", "audio"})) {
        return 7;
    }
    return 0;
}
