#include "shadowrun_ghidra/session/runner_selection_conversion.hpp"

#include <optional>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerSelectionScreenState state{};
    std::vector<std::string> order;
    std::vector<RunnerSelectionGridRequest> grids;
    std::vector<RunnerSpriteBuildRequest> sprites;
    int move_calls = 0;
    int published = 0;
    int hud_renders = 0;
    int menu_uploads = 0;
    std::vector<std::uint16_t> events{0x0800U, 0x4000U};
    std::size_t event_index = 0;

    const RunnerSelectionScreenCallbacks callbacks{
        [&] { order.emplace_back("clear"); },
        [&] { order.emplace_back("copy"); },
        [&] { order.emplace_back("upload"); },
        [&](const RunnerSelectionGridRequest& request) { grids.push_back(request); },
        [&] { order.emplace_back("header"); },
        [&] { order.emplace_back("animation"); },
        [&] { order.emplace_back("build-once"); },
        [&] { order.emplace_back("record-table"); },
        [&](const RunnerSpriteBuildRequest& request) { sprites.push_back(request); },
        [](const std::int16_t) {
            return std::optional<RunnerSpriteSource>{{0, 3, -2}};
        },
        [] {},
        [] {},
        [&] { return events[event_index++]; },
        [&](const std::int16_t direction) -> std::optional<std::int16_t> {
            ++move_calls;
            return direction > 0 ? std::optional<std::int16_t>{1}
                                 : std::nullopt;
        },
        [&] { ++published; },
        [&] { ++hud_renders; },
        [&] { ++menu_uploads; },
        [] { return 17; },
    };

    const auto result = run_runner_selection_screen(state, callbacks);
    if (!expect(result == RunnerSelectionResult::runner_selected &&
                state.background_dirty && state.sprite_records_dirty &&
                state.selection_published && published == 1 && hud_renders == 1 &&
                menu_uploads == 0 && move_calls == 1 &&
                state.active_runner == 1 &&
                order == std::vector<std::string>{"clear", "copy", "upload",
                                                  "header", "animation", "build-once",
                                                  "record-table", "build-once", "build-once"} &&
                grids.size() == 141U && grids.front().initial &&
                grids.back().row == 9U && grids.back().column == 13U &&
                sprites.size() == 2U && sprites.back().origin_x == 48 &&
                sprites.back().origin_y == -32 &&
                sprites.back().animation_seed == 0x2000U)) {
        return 1;
    }

    RunnerSelectionScreenState menu_state{};
    menu_state.alternate_presenter = false;
    std::vector<std::uint16_t> menu_events{0x8000U};
    std::size_t menu_event_index = 0;
    std::int32_t menu_result = 0;
    auto menu_callbacks = callbacks;
    menu_callbacks.sample_controller_events = [&] {
        return menu_events[menu_event_index++];
    };
    menu_callbacks.resolve_selected_runner_sprite_source = [](const std::int16_t) {
        return std::optional<RunnerSpriteSource>{{0, 0, 0}};
    };
    const auto menu_outcome = run_runner_selection_screen(
        menu_state, menu_callbacks, &menu_result);
    if (!expect(menu_outcome == RunnerSelectionResult::menu_returned &&
                menu_result == 17 && menu_uploads == 1)) {
        return 2;
    }

    RunnerSelectionScreenState alternate_state{};
    alternate_state.alternate_presenter = true;
    std::vector<std::uint16_t> alternate_events{0x8000U, 0x4000U};
    std::size_t alternate_event_index = 0;
    auto alternate_callbacks = callbacks;
    alternate_callbacks.sample_controller_events = [&] {
        return alternate_events[alternate_event_index++];
    };
    alternate_callbacks.resolve_selected_runner_sprite_source = [](const std::int16_t) {
        return std::optional<RunnerSpriteSource>{{0, 1, 1}};
    };
    if (!expect(run_runner_selection_screen(alternate_state, alternate_callbacks) ==
                RunnerSelectionResult::runner_selected &&
                alternate_state.selection_published)) {
        return 3;
    }

    return 0;
}
