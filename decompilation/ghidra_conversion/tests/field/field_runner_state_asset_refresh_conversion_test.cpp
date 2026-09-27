#include "shadowrun_ghidra/field/field_runner_state_asset_refresh_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldRunnerRefreshState state{};
    state.runners[0] = {true, 0U, 0U, 11U};
    state.runners[1] = {true, 0U, 0U, 22U};
    state.compact_record_markers = {{2U, 1U}};
    state.effect_counters = {5U, 0x01U};

    std::vector<std::string> calls;
    const auto result = refresh_field_runner_state_and_assets(
        state, 22U,
        {[&] { calls.emplace_back("serialize"); },
         [&](const FieldRunnerSlot slot) {
             calls.emplace_back(slot == FieldRunnerSlot::primary ? "clear-primary"
                                                                  : "clear-secondary");
         },
         [&](const FieldRunnerSlot slot) {
             calls.emplace_back(slot == FieldRunnerSlot::primary ? "action-primary"
                                                                  : "action-secondary");
         },
         [&] { calls.emplace_back("assets"); },
         [&] { calls.emplace_back("animation"); }});

    if (calls != std::vector<std::string>{
                    "serialize", "clear-primary", "clear-secondary",
                    "action-secondary", "assets", "animation"} ||
        !result.processed[0] || !result.processed[1] ||
        result.counter_updates[0] != FieldRunnerCounterUpdate::updated ||
        result.counter_updates[1] != FieldRunnerCounterUpdate::updated ||
        state.compact_record_markers != std::array<std::uint8_t, 2>{{0xFFU,
                                                                      0xFFU}} ||
        state.effect_counters != std::vector<std::uint8_t>{3U, 0xFFU}) {
        return 1;
    }

    // The direct source gate is an AND. A nonzero +0x9C gate excludes the
    // runner even when the other two fields would allow it.
    FieldRunnerRefreshState gated{};
    gated.runners[0] = {true, 0U, 1U, 31U};
    gated.compact_record_markers[0] = 1U;
    gated.effect_counters = {9U};
    int cleared = 0;
    int assets = 0;
    const auto gated_result = refresh_field_runner_state_and_assets(
        gated, 31U,
        {{},
         [&](const FieldRunnerSlot) { ++cleared; },
         {},
         [&] { ++assets; },
         {}});
    if (gated_result.processed[0] || cleared != 0 || assets != 1 ||
        gated.compact_record_markers[0] != 1U ||
        gated.effect_counters[0] != 9U) {
        return 2;
    }

    FieldRunnerRefreshState missing_marker{};
    missing_marker.runners[0] = {true, 0U, 0U, 41U};
    missing_marker.compact_record_markers[0] = 0xFFU;
    int missing_clear = 0;
    const auto missing_result = refresh_field_runner_state_and_assets(
        missing_marker, 99U,
        {{},
         [&](const FieldRunnerSlot) { ++missing_clear; },
         {},
         {},
         {}});
    if (!missing_result.processed[0] ||
        missing_result.counter_updates[0] !=
            FieldRunnerCounterUpdate::marker_unavailable ||
        missing_clear != 1) {
        return 3;
    }

    return 0;
}
