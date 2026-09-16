#include "shadowrun/z80_transport.hpp"

#include "shadowrun/boot.hpp"
#include "shadowrun/session.hpp"

namespace shadowrun {

namespace {

void acquire_z80_bus(GenesisBus& bus) {
    bus.write16(genesis::kZ80BusRequest, 0x0100U);
    while ((bus.read8(genesis::kZ80BusRequest) & 1U) != 0) {
    }
}

void release_z80_bus(GenesisBus& bus) {
    bus.write16(genesis::kZ80BusRequest, 0);
}

Z80CommandResult append_z80_byte_locked(
    GenesisBus& bus,
    std::uint8_t command) {
    const auto index_before = bus.read8(genesis::kZ80CommandIndex);
    bus.write8(genesis::kZ80CommandRing + index_before, command);
    const auto index_after = static_cast<std::uint8_t>(
        (index_before + 1U) & 0x3FU);
    bus.write8(genesis::kZ80CommandIndex, index_after);
    return {true, index_before, index_after, command};
}

Z80CommandResult append_separator_command_locked(
    GenesisBus& bus,
    std::uint8_t command) {
    const auto index_before = bus.read8(genesis::kZ80CommandIndex);
    bus.write8(genesis::kZ80CommandRing + index_before, 0xFFU);
    const auto command_index = static_cast<std::uint8_t>(
        (index_before + 1U) & 0x3FU);
    bus.write8(genesis::kZ80CommandRing + command_index, command);
    const auto index_after = static_cast<std::uint8_t>(
        (command_index + 1U) & 0x3FU);
    bus.write8(genesis::kZ80CommandIndex, index_after);
    return {true, index_before, index_after, command};
}

} // namespace

Z80CommandResult send_z80_command_0x52F3C(
    GenesisBus& bus,
    std::uint8_t command) {
    // 0x52F3C first emits the fixed 0x10 command through 0x52E96, then
    // appends its caller-supplied D0 byte through 0x52EA2 while retaining the
    // same bus lock and A6 frame.
    acquire_z80_bus(bus);
    const auto result = append_separator_command_locked(bus, 0x10U);
    const auto byte_result = append_z80_byte_locked(bus, command);
    release_z80_bus(bus);
    return {true, result.index_before, byte_result.index_after, command};
}

Z80CommandResult send_z80_byte_0x52EA2(
    GenesisBus& bus,
    std::uint8_t command) {
    acquire_z80_bus(bus);
    const auto result = append_z80_byte_locked(bus, command);
    release_z80_bus(bus);
    return result;
}

Z80CommandResult send_z80_command_0x52E96(
    GenesisBus& bus,
    std::uint8_t command) {
    acquire_z80_bus(bus);
    const auto result = append_separator_command_locked(bus, command);
    release_z80_bus(bus);
    return result;
}

Z80CommandResult dispatch_z80_command_0x530DE(
    GenesisBus& bus,
    std::uint8_t command) {
    if (bus.read16(session_ram::kReadyFlag1) == 0) {
        return {false, 0, 0, command};
    }
    return send_z80_command_0x52F3C(bus, command);
}

Z80SynchronizeResult synchronize_z80_0x53126(
    GenesisBus& bus,
    const FrameWaitRoutine& frame_wait) {
    const auto initial_index = bus.read8(genesis::kZ80CommandIndex);
    std::size_t frame_waits = 0;
    std::uint8_t command = 0;
    for (std::size_t pass = 0; pass < 6U; ++pass) {
        command = static_cast<std::uint8_t>(command + 0x14U);
        send_z80_byte_0x52EA2(bus, command);
        frame_waits += ShadowrunRuntime(bus).service_frame_input(
            2U, frame_wait);
    }

    // 0x52FA0 and 0x53156 are both complete bus-owning calls in the ROM.
    send_z80_command_0x52E96(bus, 0x16U);
    send_z80_byte_0x52EA2(bus, 0);
    return {6U, frame_waits, initial_index,
            bus.read8(genesis::kZ80CommandIndex)};
}

} // namespace shadowrun
