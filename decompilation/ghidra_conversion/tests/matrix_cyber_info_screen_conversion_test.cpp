#include "shadowrun_ghidra/matrix_cyber_info_screen_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    CyberInfoScreenState state{};
    std::vector<std::string> events;
    const auto callbacks = CyberInfoScreenCallbacks{
        [&] { events.emplace_back("grid"); },
        [&] { events.emplace_back("title"); },
        [&] { events.emplace_back("vblank"); },
        [&] { events.emplace_back("latch"); },
        [&] { events.emplace_back("sample"); },
        [&] { events.emplace_back("history"); },
        [&](const CyberInfoPage page) {
            events.emplace_back(page == CyberInfoPage::deck_stats ? "deck-cursor"
                                                                    : "run-cursor");
        },
        [&] { events.emplace_back("deck"); },
        [&] { events.emplace_back("run"); },
        [&] { events.emplace_back("matrix"); },
    };

    // The initial cursor is Deck Stats.  Up wraps to Run Info, and page
    // activation is handled before the Matrix-return bit.
    const auto run_info = run_cyber_info_screen(
        state, {move_page_up, activate_page_a | return_to_matrix}, callbacks);
    if (!expect(run_info.exit == CyberInfoScreenExit::page_rendered &&
                run_info.page == CyberInfoPage::run_info &&
                run_info.frames_processed == 2U &&
                events == std::vector<std::string>{
                    "grid", "title", "deck-cursor", "vblank", "latch",
                    "sample", "history", "run-cursor", "vblank", "latch",
                    "sample", "history", "run"})) {
        return 1;
    }

    // Up wins over down, while a plain bit-15 event redraws Matrix without
    // rendering either CyberInfo page.
    state = {};
    events.clear();
    const auto returned = run_cyber_info_screen(
        state, {move_page_up | move_page_down, return_to_matrix}, callbacks);
    if (!expect(returned.exit == CyberInfoScreenExit::returned_to_matrix)) {
        return 2;
    }
    if (!expect(returned.page == CyberInfoPage::run_info &&
                events == std::vector<std::string>{
                    "grid", "title", "deck-cursor", "vblank", "latch",
                    "sample", "history", "run-cursor", "vblank", "latch",
                    "sample", "history", "matrix"})) {
        return 2;
    }

    // With no exit event, the bounded host sequence reports the boundary
    // instead of inventing an implicit return.
    state = {};
    events.clear();
    const auto bounded = run_cyber_info_screen(state, {0U, 0U}, callbacks);
    if (!expect(bounded.exit == CyberInfoScreenExit::frame_budget_exhausted &&
                bounded.page == CyberInfoPage::deck_stats &&
                bounded.frames_processed == 2U && events.size() == 11U)) {
        return 3;
    }

    return 0;
}
