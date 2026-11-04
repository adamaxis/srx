#pragma once

#include "shadowrun/core/genesis_bus.hpp"
#include "shadowrun/core/runtime.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun {

constexpr GenesisAddress kResourceStreamStaging = 0x00FF9700;

// ROM 0x1E2E0/0x1E2EC. These are the small resource-queue leaves used by
// several entity paths. They consume the resource pointer already staged at
// FFDD2C and leave the actual VDP transfer to the later queue consumer.
void queue_resource_load_0x1E2E0(GenesisBus& bus);
void queue_resource_load_0x1E2EC(GenesisBus& bus);

// The 12-byte records consumed by the first A6 list in ROM 0x1406. The low
// bit of raw_source selects 0x1530 versus 0x0F5C; the cleared value is passed
// as A3. The remaining longwords are loaded into D1/D2, and D0 receives the
// low word of D1 before the callee is entered.
struct PrimaryResourceStream {
    std::uint32_t raw_source;
    GenesisAddress source;
    std::uint32_t d1_long;
    std::uint32_t d2_long;
    std::uint16_t d0_word;
    GenesisAddress staging_address;
};

// The 14-byte records consumed by the second A6 list in ROM 0x1406. The
// source bit at 0x10 selects 0x1728; otherwise 0x1232 is called. Register
// names are retained because their semantic meanings are not proven.
struct SecondaryResourceStream {
    std::uint32_t raw_source;
    GenesisAddress source_before_bit_clear;
    GenesisAddress source_after_bit_clear;
    std::uint32_t d4_long;
    std::uint16_t d7_word;
    std::uint16_t d1_word;
    std::uint16_t d2_word;
    std::uint16_t d3_word;
    std::uint16_t d5_word;
    bool bit16_was_set;
};

using PrimaryResourceRoutine =
    std::function<void(GenesisBus&, const PrimaryResourceStream&)>;
using SecondaryResourceRoutine =
    std::function<void(GenesisBus&, const SecondaryResourceStream&)>;

struct ResourceSetupHooks {
    PrimaryResourceRoutine routine_0x0F5C;
    PrimaryResourceRoutine routine_0x1530;
    SecondaryResourceRoutine routine_0x1232;
    SecondaryResourceRoutine routine_0x1728;
    FrameWaitRoutine frame_wait;
};

struct ResourceSetupResult {
    std::size_t primary_streams;
    std::size_t secondary_streams;
    GenesisAddress secondary_list_end;
    std::uint8_t flags_after;
    std::size_t frame_waits;
};

// ROM 0x1406. setup_block is the A6 input: two longword source pointers,
// four VDP setup words, then the two sentinel-terminated resource lists.
// The four stream callees remain injectable hooks; reviewed default
// translations are supplied for 0x1232, 0x1728, 0x0F5C/0x0F98, and 0x1530.
ResourceSetupResult run_resource_setup(
    GenesisBus& bus,
    GenesisAddress setup_block,
    const ResourceSetupHooks& hooks = {});

} // namespace shadowrun
