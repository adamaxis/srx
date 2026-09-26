#include "shadowrun_ghidra/core/controller_port_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint8_t read_controller_port(
    const ControllerPortReadCallbacks& callbacks) {
    if (!callbacks.begin_bus_access || !callbacks.read_phase ||
        !callbacks.end_bus_access) {
        return 0U;
    }

    callbacks.begin_bus_access();
    const auto first_phase = callbacks.read_phase(0x00U);
    const auto second_phase = callbacks.read_phase(0x40U);
    const auto packed = static_cast<std::uint8_t>(
        second_phase & 0x3FU | (first_phase & 0x30U) << 2U);
    callbacks.end_bus_access();
    return static_cast<std::uint8_t>(~packed);
}

} // namespace shadowrun::ghidra_conversion
