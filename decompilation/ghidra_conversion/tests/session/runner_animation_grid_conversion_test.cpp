#include "shadowrun_ghidra/session/runner_animation_grid_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerAnimationGridState state{};
    int table_selections = 0;
    std::vector<std::int16_t> marked;
    const auto callbacks = RunnerAnimationGridCallbacks{
        [&] { ++table_selections; },
        [] { return RunnerAnimationGridPosition{3, 2}; },
        [](const std::int16_t from,
           const std::int16_t to,
           const RunnerAnimationGridDirection direction) {
            if (direction == RunnerAnimationGridDirection::north && from == 31) {
                return RunnerAnimationGridStep::mark_only;
            }
            return to == 32 ? RunnerAnimationGridStep::blocked
                            : RunnerAnimationGridStep::recurse;
        },
        [&](const std::int16_t cell) { marked.push_back(cell); },
    };

    if (!expect(initialize_runner_animation_grid(state, callbacks) &&
                state.start_cell == 31 && state.marked_count > 1U &&
                state.marked[31] == 0xFFU && table_selections == 1 &&
                marked.front() == 31 && state.recursion_guard == 0U)) {
        return 1;
    }

    RunnerAnimationGridState invalid{};
    if (!expect(!initialize_runner_animation_grid(
                    invalid,
                    { {}, [] { return RunnerAnimationGridPosition{14, 0}; },
                      {}, {} }) &&
                invalid.start_cell == -1 && invalid.marked_count == 0U)) {
        return 2;
    }

    return 0;
}
