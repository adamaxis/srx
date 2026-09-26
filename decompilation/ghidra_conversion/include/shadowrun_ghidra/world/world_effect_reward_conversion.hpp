#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kWorldRewardInventorySlotCount = 8U;

// ROM 0x133A6 reads the active runner's +0x4F special-action counter and the
// eight item/phase pairs at +0x66/+0x6E.  The native model keeps those fields
// together without exposing the source record address.
struct WorldRewardRunnerState {
    std::array<std::uint8_t, kWorldRewardInventorySlotCount> item_codes{};
    std::array<std::uint8_t, kWorldRewardInventorySlotCount> item_phases{};
    std::uint8_t special_action_count = 0; // active runner +0x4F
};

// The negative-command resource and the two positive command flags are the
// global values touched by 0x133A6.
struct WorldRewardGlobalState {
    std::uint32_t funds = 0;         // source FFFBFE
    std::uint32_t status_flags = 0;  // source FFFC22
};

// This is the source's 0x0D3A boundary.  It returns a value in [0, bound) for
// the normal source PRNG; keeping it injectable leaves the conversion free of
// a Genesis random-number implementation.
using WorldRewardRandomBoundary =
    std::function<std::uint8_t(std::uint8_t bound)>;

enum class WorldRewardOutcome {
    special_action_count_updated,
    funds_added,
    status_flag_set,
    inventory_added,
    inventory_full,
};

struct WorldRewardResult {
    WorldRewardOutcome outcome = WorldRewardOutcome::funds_added;
    std::int16_t source_status = 0; // ROM D7: 0 success, -1 full inventory
    std::size_t inventory_slot = kWorldRewardInventorySlotCount;
    std::uint8_t item_code = 0;
    std::uint8_t item_phase = 0;
    bool random_phase_requested = false;
    bool random_phase_sampled = false;

    bool succeeded() const noexcept { return source_status >= 0; }
};

// ROM 0x133A6 applies the interaction/reward command supplied in D0.  The
// command protocol is source data, not a menu operation:
//   - 0xCxxx adjusts the active runner's special-action count;
//   - 0x8xxx adds the low twelve bits to funds;
//   - 0x64/0x65 set status bits 4/3;
//   - other positive commands insert item code command+1 and, for commands
//     >= 0x0D, assign a random phase in [2, 6].
// A full eight-slot inventory returns D7=-1 and leaves the record unchanged.
WorldRewardResult apply_world_effect_reward(
    std::uint16_t command,
    WorldRewardRunnerState& runner,
    WorldRewardGlobalState& globals,
    const WorldRewardRandomBoundary& sample_random = {});

} // namespace shadowrun::ghidra_conversion
