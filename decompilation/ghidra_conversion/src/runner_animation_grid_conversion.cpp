#include "shadowrun_ghidra/runner_animation_grid_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

struct DirectionStep {
    RunnerAnimationGridDirection direction;
    std::int16_t delta;
    std::uint16_t blocked_bit;
    std::uint16_t reverse_bit;
};

constexpr std::array<DirectionStep, 4> kDirectionSteps{{
    {RunnerAnimationGridDirection::north, -14, 0x0200U, 0x0800U},
    {RunnerAnimationGridDirection::east, 1, 0x0400U, 0x1000U},
    {RunnerAnimationGridDirection::south, 14, 0x0800U, 0x0200U},
    {RunnerAnimationGridDirection::west, -1, 0x1000U, 0x0400U},
}};

bool in_grid(const std::int16_t cell) {
    return cell >= 0 &&
           cell < static_cast<std::int16_t>(kRunnerAnimationGridCellCount);
}

bool crosses_horizontal_edge(const std::int16_t from,
                             const std::int16_t to,
                             const RunnerAnimationGridDirection direction) {
    if (direction == RunnerAnimationGridDirection::east) {
        return from % static_cast<std::int16_t>(kRunnerAnimationGridWidth) ==
               static_cast<std::int16_t>(kRunnerAnimationGridWidth - 1U);
    }
    if (direction == RunnerAnimationGridDirection::west) {
        return from % static_cast<std::int16_t>(kRunnerAnimationGridWidth) == 0;
    }
    return false;
}

void mark(RunnerAnimationGridState& state,
          const std::int16_t cell,
          const std::function<void(std::int16_t)>& callback) {
    if (!in_grid(cell) || state.marked[static_cast<std::size_t>(cell)] != 0U) {
        return;
    }
    state.marked[static_cast<std::size_t>(cell)] = 0xFFU;
    ++state.marked_count;
    if (callback) {
        callback(cell);
    }
}

} // namespace

void expand_runner_animation_grid(
    RunnerAnimationGridState& state,
    const std::int16_t current_cell,
    const RunnerAnimationGridCallbacks& callbacks,
    const std::uint16_t recursion_guard) {
    if (!in_grid(current_cell)) {
        return;
    }

    const auto previous_guard = state.recursion_guard;
    state.recursion_guard = recursion_guard;
    for (const auto& step : kDirectionSteps) {
        if ((recursion_guard & step.blocked_bit) != 0U) {
            continue;
        }
        const auto next_cell = static_cast<std::int16_t>(current_cell + step.delta);
        if (!in_grid(next_cell) ||
            crosses_horizontal_edge(current_cell, next_cell, step.direction) ||
            state.marked[static_cast<std::size_t>(next_cell)] != 0U) {
            continue;
        }

        const auto decision = callbacks.classify_step
                                 ? callbacks.classify_step(current_cell, next_cell,
                                                           step.direction)
                                 : RunnerAnimationGridStep::blocked;
        if (decision == RunnerAnimationGridStep::blocked) {
            continue;
        }
        mark(state, next_cell, callbacks.mark_cell);
        if (decision == RunnerAnimationGridStep::recurse) {
            expand_runner_animation_grid(
                state, next_cell, callbacks, recursion_guard | step.reverse_bit);
        }
    }
    state.recursion_guard = previous_guard;
}

bool initialize_runner_animation_grid(
    RunnerAnimationGridState& state,
    const RunnerAnimationGridCallbacks& callbacks) {
    if (callbacks.select_primary_runner_record_table) {
        callbacks.select_primary_runner_record_table();
    }
    if (!callbacks.resolve_selected_position) {
        return false;
    }

    const auto position = callbacks.resolve_selected_position();
    if (position.x < 0 || position.y < 0 ||
        position.x >= static_cast<std::int8_t>(kRunnerAnimationGridWidth) ||
        position.y >= static_cast<std::int8_t>(kRunnerAnimationGridHeight)) {
        return false;
    }

    state = {};
    state.start_cell = static_cast<std::int16_t>(
        static_cast<std::int16_t>(position.y) *
            static_cast<std::int16_t>(kRunnerAnimationGridWidth) +
        static_cast<std::int16_t>(position.x));
    mark(state, state.start_cell, callbacks.mark_cell);
    expand_runner_animation_grid(state, state.start_cell, callbacks);
    return true;
}

} // namespace shadowrun::ghidra_conversion
