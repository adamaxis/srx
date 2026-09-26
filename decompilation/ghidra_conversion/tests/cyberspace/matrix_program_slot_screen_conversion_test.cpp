#include "shadowrun_ghidra/cyberspace/matrix_program_slot_screen_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    MatrixProgramSlotScreenState state{};
    state.ready_timer_active = true;
    std::vector<std::string> events;
    const auto callbacks = MatrixProgramSlotScreenCallbacks{
        [&] { events.emplace_back("prepare"); },
        [&] { events.emplace_back("vblank"); },
        [&] { events.emplace_back("latch"); },
        [&] { events.emplace_back("sample"); },
        [&] { events.emplace_back("history"); },
        [&](const std::uint8_t selection) {
            events.emplace_back(selection == 0U ? "yes-cursor" : "no-cursor");
        },
        [&] { events.emplace_back("apply"); },
        [&] { events.emplace_back("clear-timer"); },
        [&] { events.emplace_back("refresh-slot"); },
        [&] { events.emplace_back("panel-grid"); },
        [&] { events.emplace_back("panel"); },
    };

    // The source enters on No (one), moves up to Yes, and applies the
    // pending slot command before its unconditional Matrix redraw.
    const auto installed = run_matrix_program_slot_screen(
        state, {move_prompt_up,
                accept_prompt_a | return_to_matrix_from_prompt}, callbacks);
    if (!expect(installed.exit == MatrixProgramSlotScreenExit::installed_program &&
                installed.selection == 0U && installed.frames_processed == 2U &&
                events == std::vector<std::string>{
                    "prepare", "no-cursor", "vblank", "latch", "sample",
                    "history", "yes-cursor", "vblank", "latch", "sample",
                    "history", "apply", "panel-grid", "panel"})) {
        return 1;
    }

    // No is a non-mutating return even if the exit bit is simultaneously set;
    // activation is tested before bit 15 in the source.
    state = {};
    events.clear();
    const auto cancelled = run_matrix_program_slot_screen(
        state, {accept_prompt_b | return_to_matrix_from_prompt}, callbacks);
    if (!expect(cancelled.exit == MatrixProgramSlotScreenExit::cancelled_prompt &&
                cancelled.selection == 1U &&
                events == std::vector<std::string>{
                    "prepare", "no-cursor", "vblank", "latch", "sample",
                    "history", "panel-grid", "panel"})) {
        return 2;
    }

    // Yes with no ready timer follows the source's clear-and-refresh branch.
    state = {};
    state.prompt.selection = 0U;
    events.clear();
    const auto refreshed = run_matrix_program_slot_screen(
        state, {accept_prompt_c}, callbacks);
    if (!expect(refreshed.exit ==
                    MatrixProgramSlotScreenExit::refreshed_existing_slot &&
                events == std::vector<std::string>{
                    "prepare", "yes-cursor", "vblank", "latch", "sample",
                    "history", "clear-timer", "refresh-slot", "panel-grid",
                    "panel"})) {
        return 3;
    }

    // A bounded host input sequence does not invent a source exit.
    state = {};
    events.clear();
    const auto bounded = run_matrix_program_slot_screen(state, {0U}, callbacks);
    if (!expect(bounded.exit ==
                    MatrixProgramSlotScreenExit::frame_budget_exhausted &&
                bounded.frames_processed == 1U && events.size() == 6U)) {
        return 4;
    }

    return 0;
}
