#pragma once

#include "shadowrun/core/genesis_bus.hpp"
#include "shadowrun/core/runtime.hpp"

#include <cstddef>
#include <cstdint>

namespace shadowrun {

// The Z80 command queue at A01B40 is a 0x40-byte circular stream. These
// results expose the exact byte-ring mutation while leaving the bus request,
// shared RAM, and eventual Z80 execution to the hardware backend.
struct Z80CommandResult {
    bool command_sent;
    std::uint8_t index_before;
    std::uint8_t index_after;
    std::uint8_t command;
};

// Direct translation of ROM 0x52F3C. The CPU requests the Z80 bus, writes a
// 0xFF separator, the fixed 0x10 opcode, and the caller's command byte, then
// advances A00036 modulo 0x40 and releases the bus.
Z80CommandResult send_z80_command_0x52F3C(
    GenesisBus& bus,
    std::uint8_t command);

// Direct translation of the shared 0x52EA2 byte append primitive as a
// complete bus transaction. It writes one byte at the current A00036 cursor,
// advances modulo 0x40, and releases the Z80 bus.
Z80CommandResult send_z80_byte_0x52EA2(
    GenesisBus& bus,
    std::uint8_t command);

// Direct translation of the 0x52E96 prefix primitive used by 0x52FA0. It
// writes an 0xFF separator and one command byte, advancing the shared cursor
// by two positions.
Z80CommandResult send_z80_command_0x52E96(
    GenesisBus& bus,
    std::uint8_t command);

// Direct translation of ROM 0x530DE. FFFC0A gates the 0x52F3C transfer.
Z80CommandResult dispatch_z80_command_0x530DE(
    GenesisBus& bus,
    std::uint8_t command);

struct Z80SynchronizeResult {
    std::size_t ramp_commands;
    std::size_t frame_waits;
    std::uint8_t initial_index;
    std::uint8_t final_index;
};

// Direct translation of ROM 0x53126. The six 0x14-stepped bytes are appended
// through 0x5315E/0x52F6E, with the original three-pass 0x0C24 wait between
// iterations, followed by 0x52FA0's 0x16 prefix and 0x53156's zero byte.
Z80SynchronizeResult synchronize_z80_0x53126(
    GenesisBus& bus,
    const FrameWaitRoutine& frame_wait = {});

} // namespace shadowrun
