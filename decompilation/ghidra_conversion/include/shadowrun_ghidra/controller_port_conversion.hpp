#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ControllerPortReadCallbacks {
    std::function<void()> begin_bus_access;
    // The source samples the port once with data-select 0 and once with 0x40.
    std::function<std::uint8_t(std::uint8_t data_select)> read_phase;
    std::function<void()> end_bus_access;
};

// ROM 0x19F2: acquire the controller bus, read both controller phases, pack
// the six-button result, and release the bus. Host input supplies the phase
// reads; the source's register protocol is not a runtime dependency.
std::uint8_t read_controller_port(
    const ControllerPortReadCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
