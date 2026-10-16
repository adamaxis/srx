#include "shadowrun_ghidra/world/world_effect_reward_conversion.hpp"

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldRewardRunnerState runner{};
    WorldRewardGlobalState globals{};

    runner.special_action_count = 3U;
    const auto special =
        apply_world_effect_reward(0xC005U, runner, globals);
    if (special.outcome !=
            WorldRewardOutcome::special_action_count_updated ||
        !special.succeeded() || runner.special_action_count != 8U) {
        return 1;
    }

    runner.special_action_count = 0x13U;
    apply_world_effect_reward(0xC003U, runner, globals);
    if (runner.special_action_count != 0x14U) {
        return 2;
    }

    // CMP.B/BMI uses the wrapped eight-bit subtraction.  These values
    // distinguish that source flag result from a host int8_t comparison.
    runner.special_action_count = 0x80U;
    apply_world_effect_reward(0xC000U, runner, globals);
    if (runner.special_action_count != 0x14U) {
        return 3;
    }
    runner.special_action_count = 0x95U;
    apply_world_effect_reward(0xC000U, runner, globals);
    if (runner.special_action_count != 0x95U) {
        return 4;
    }

    globals.funds = 100U;
    const auto funds =
        apply_world_effect_reward(0x800AU, runner, globals);
    if (funds.outcome != WorldRewardOutcome::funds_added ||
        globals.funds != 110U) {
        return 5;
    }

    const auto flag_a =
        apply_world_effect_reward(0x64U, runner, globals);
    const auto flag_b =
        apply_world_effect_reward(0x65U, runner, globals);
    if (flag_a.outcome != WorldRewardOutcome::status_flag_set ||
        flag_b.outcome != WorldRewardOutcome::status_flag_set ||
        (globals.status_flags & (std::uint32_t{1U} << 4U)) == 0U ||
        (globals.status_flags & (std::uint32_t{1U} << 3U)) == 0U) {
        return 6;
    }

    std::uint8_t random_calls = 0U;
    const auto ordinary =
        apply_world_effect_reward(
            0x0CU, runner, globals,
            [&](const std::uint8_t bound) {
                if (bound != 5U) {
                    random_calls = 0xFFU;
                    return static_cast<std::uint8_t>(0U);
                }
                ++random_calls;
                return static_cast<std::uint8_t>(0U);
            });
    if (ordinary.outcome != WorldRewardOutcome::inventory_added ||
        ordinary.inventory_slot != 0U || ordinary.item_code != 0x0DU ||
        !ordinary.random_phase_requested || !ordinary.random_phase_sampled ||
        ordinary.item_phase != 2U || runner.item_codes[0] != 0x0DU ||
        runner.item_phases[0] != 2U || random_calls != 1U) {
        return 7;
    }

    const auto randomized = apply_world_effect_reward(
        0x0DU, runner, globals,
        [](const std::uint8_t bound) {
            return bound == 5U ? static_cast<std::uint8_t>(3U)
                               : static_cast<std::uint8_t>(0U);
        });
    if (randomized.inventory_slot != 1U || randomized.item_code != 0x0EU ||
        !randomized.random_phase_requested ||
        !randomized.random_phase_sampled || randomized.item_phase != 5U ||
        runner.item_phases[1] != 5U) {
        return 8;
    }

    WorldRewardRunnerState full{};
    full.item_codes.fill(1U);
    const auto rejected =
        apply_world_effect_reward(0x01U, full, globals);
    if (rejected.outcome != WorldRewardOutcome::inventory_full ||
        rejected.source_status != -1 || rejected.succeeded()) {
        return 9;
    }

    WorldRewardRunnerState missing_random{};
    const auto unresolved =
        apply_world_effect_reward(0x0DU, missing_random, globals);
    if (!unresolved.random_phase_requested ||
        unresolved.random_phase_sampled || missing_random.item_codes[0] != 0x0EU ||
        missing_random.item_phases[0] != 0U) {
        return 10;
    }

    return 0;
}
