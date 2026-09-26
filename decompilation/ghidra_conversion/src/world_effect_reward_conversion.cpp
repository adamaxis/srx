#include "shadowrun_ghidra/world_effect_reward_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

WorldRewardResult make_success(const WorldRewardOutcome outcome) {
    WorldRewardResult result{};
    result.outcome = outcome;
    result.source_status = 0;
    return result;
}

} // namespace

WorldRewardResult apply_world_effect_reward(
    const std::uint16_t command,
    WorldRewardRunnerState& runner,
    WorldRewardGlobalState& globals,
    const WorldRewardRandomBoundary& sample_random) {
    // 0x133A6 first tests D0's sign.  The second sign test is the sign after
    // LSL.W #1, which is exactly the original bit 14.  Keep the two command
    // families separate instead of treating the word as a host integer.
    if ((command & 0x8000U) != 0U) {
        if ((command & 0x4000U) != 0U) {
            auto result = make_success(
                WorldRewardOutcome::special_action_count_updated);

            // ADD.B uses the low command byte and wraps before the source's
            // signed-byte comparison against 0x15.
            runner.special_action_count = static_cast<std::uint8_t>(
                runner.special_action_count +
                static_cast<std::uint8_t>(command & 0x00FFU));
            if (static_cast<std::int8_t>(runner.special_action_count) >=
                static_cast<std::int8_t>(0x15)) {
                runner.special_action_count = 0x14U;
            }
            return result;
        }

        auto result = make_success(WorldRewardOutcome::funds_added);
        globals.funds += static_cast<std::uint32_t>(command & 0x0FFFU);
        return result;
    }

    if (command == 0x64U) {
        auto result = make_success(WorldRewardOutcome::status_flag_set);
        globals.status_flags |= (std::uint32_t{1U} << 4U);
        return result;
    }
    if (command == 0x65U) {
        auto result = make_success(WorldRewardOutcome::status_flag_set);
        globals.status_flags |= (std::uint32_t{1U} << 3U);
        return result;
    }

    for (std::size_t slot = 0; slot < kWorldRewardInventorySlotCount;
         ++slot) {
        if (runner.item_codes[slot] != 0U) {
            continue;
        }

        auto result = make_success(WorldRewardOutcome::inventory_added);
        result.inventory_slot = slot;
        result.item_code = static_cast<std::uint8_t>(command + 1U);
        runner.item_codes[slot] = result.item_code;
        runner.item_phases[slot] = 0U;

        if (command >= 0x0DU) {
            result.random_phase_requested = true;
            if (sample_random) {
                result.item_phase = static_cast<std::uint8_t>(
                    sample_random(5U) + 2U);
                result.random_phase_sampled = true;
                runner.item_phases[slot] = result.item_phase;
            }
        }
        return result;
    }

    WorldRewardResult result{};
    result.outcome = WorldRewardOutcome::inventory_full;
    result.source_status = -1;
    return result;
}

} // namespace shadowrun::ghidra_conversion
