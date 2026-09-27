#include "shadowrun_ghidra/field/mode_services_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    SceneModeInitializationState scene{};
    int prepared = 0;
    SceneModeInitializationCallbacks scene_callbacks{};
    scene_callbacks.prepare_mode_entry = [&] {
        ++prepared;
        return static_cast<std::uint8_t>(6U);
    };
    initialize_scene_mode_state(scene, scene_callbacks);
    if (prepared != 1 || scene.current_mode != 6U ||
        scene.scene_transition_value != 0U || scene.scene_mode_marker != 0xFFU ||
        scene.camera_state != 0xFFFFU || scene.special_mode_flag != 0U ||
        scene.special_mode_indicator != 1U) {
        return 1;
    }

    ModeExitState exit_state{7U};
    int exit_calls = 0;
    ModeExitCallbacks exit_callbacks{};
    exit_callbacks.service_mode_exit = [&] { ++exit_calls; };
    finalize_mode_exit(exit_state, exit_callbacks);
    if (exit_state.exit_service_marker != 0U || exit_calls != 1) {
        return 2;
    }

    ModeWorldRefreshState world{};
    std::vector<std::uint16_t> random_bounds;
    std::vector<std::uint16_t> random_values{0U, 0U, 2U, 1U};
    std::size_t random_index = 0;
    int grid_calls = 0;
    ModeWorldGridPlan submitted_plan{};
    ModeWorldRefreshCallbacks world_callbacks{};
    world_callbacks.sample_random = [&](const std::uint16_t bound) {
        random_bounds.push_back(bound);
        return random_values[random_index++];
    };
    world_callbacks.write_world_grid = [&](const ModeWorldGridPlan& plan) {
        ++grid_calls;
        submitted_plan = plan;
    };
    service_mode_world_state(world, world_callbacks);
    if (world.random_samples != 4U ||
        random_bounds != std::vector<std::uint16_t>{2U, 2U, 3U, 3U} ||
        !world.grid_write_requested || grid_calls != 1 ||
        submitted_plan.first_selection_nonzero ||
        submitted_plan.second_selection_nonzero ||
        submitted_plan.primary_source !=
            ModeWorldPrimarySource::first_zero_second_zero_table ||
        submitted_plan.primary_entry_index != 2U ||
        submitted_plan.variant_source !=
            ModeWorldVariantSource::fixed_three_variant_resources ||
        submitted_plan.variant_entry_index != 1U ||
        submitted_plan.variant_byte_offset != 4U ||
        submitted_plan.grid_row_count != 2U) {
        return 3;
    }

    random_bounds.clear();
    random_values = {1U, 1U, 2U, 3U};
    random_index = 0;
    service_mode_world_state(world, world_callbacks);
    if (random_bounds != std::vector<std::uint16_t>{2U, 2U, 3U, 4U} ||
        !submitted_plan.first_selection_nonzero ||
        !submitted_plan.second_selection_nonzero ||
        submitted_plan.primary_source !=
            ModeWorldPrimarySource::first_nonzero_second_nonzero_table ||
        submitted_plan.primary_entry_index != 2U ||
        submitted_plan.variant_source !=
            ModeWorldVariantSource::indexed_four_variant_resources ||
        submitted_plan.variant_entry_index != 3U ||
        submitted_plan.variant_byte_offset != 2U ||
        submitted_plan.grid_row_count != 1U) {
        return 4;
    }
    return 0;
}
