#include "shadowrun/core/memory_bus.hpp"

#include <stdexcept>
#include <utility>

namespace shadowrun {

GenesisMemoryBus::GenesisMemoryBus(std::vector<std::uint8_t> rom)
    : rom_(std::move(rom)) {
    if (rom_.size() > kRomEnd) {
        throw std::invalid_argument("Genesis ROM exceeds the 24-bit ROM window");
    }
}

bool GenesisMemoryBus::is_vdp(GenesisAddress address) const {
    return address == kVdpData || address == kVdpControl ||
           address == kVdpControl + 1U;
}

std::uint8_t GenesisMemoryBus::read_mapped8(GenesisAddress address) const {
    if (address < rom_.size()) {
        return rom_[address];
    }
    if (address >= kZ80RamStart && address < kZ80RamEnd) {
        return z80_ram_[address - kZ80RamStart];
    }
    if (address >= kRamStart && address < kRamEnd) {
        return ram_[address - kRamStart];
    }
    if (address >= kIoStart && address < kIoEnd) {
        return io_[address - kIoStart];
    }
    return 0;
}

std::uint8_t GenesisMemoryBus::read8(GenesisAddress address) {
    const auto mapped = address & 0x00FFFFFFU;
    if (mapped == kVdpControl) {
        return static_cast<std::uint8_t>(vdp_status_ >> 8U);
    }
    if (mapped == kVdpControl + 1U) {
        return static_cast<std::uint8_t>(vdp_status_);
    }
    return read_mapped8(mapped);
}

std::uint16_t GenesisMemoryBus::read16(GenesisAddress address) {
    const auto mapped = address & 0x00FFFFFFU;
    if (is_vdp(mapped)) {
        return mapped == kVdpControl ? vdp_status_ : 0;
    }
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(read_mapped8(mapped)) << 8U) |
        read_mapped8(mapped + 1U));
}

std::uint32_t GenesisMemoryBus::read32(GenesisAddress address) {
    const auto high = read16(address);
    const auto low = read16(address + 2U);
    return (static_cast<std::uint32_t>(high) << 16U) | low;
}

void GenesisMemoryBus::write_mapped8(GenesisAddress address, std::uint8_t value) {
    if (address >= kZ80RamStart && address < kZ80RamEnd) {
        z80_ram_[address - kZ80RamStart] = value;
    } else if (address >= kRamStart && address < kRamEnd) {
        ram_[address - kRamStart] = value;
    } else if (address >= kIoStart && address < kIoEnd) {
        io_[address - kIoStart] = value;
    }
    // Writes to ROM and unmapped space have no storage side effect.
}

void GenesisMemoryBus::record_port(unsigned width, GenesisAddress address,
                                   std::uint32_t value) {
    port_writes_.push_back(PortWrite{width, address, value});
}

void GenesisMemoryBus::write8(GenesisAddress address, std::uint8_t value) {
    const auto mapped = address & 0x00FFFFFFU;
    if (is_vdp(mapped)) {
        record_port(8, mapped, value);
        return;
    }
    write_mapped8(mapped, value);
}

void GenesisMemoryBus::write16(GenesisAddress address, std::uint16_t value) {
    const auto mapped = address & 0x00FFFFFFU;
    if (is_vdp(mapped)) {
        record_port(16, mapped, value);
        return;
    }
    write_mapped8(mapped, static_cast<std::uint8_t>(value >> 8U));
    write_mapped8(mapped + 1U, static_cast<std::uint8_t>(value));
}

void GenesisMemoryBus::write32(GenesisAddress address, std::uint32_t value) {
    const auto mapped = address & 0x00FFFFFFU;
    if (is_vdp(mapped)) {
        record_port(32, mapped, value);
        return;
    }
    write_mapped8(mapped, static_cast<std::uint8_t>(value >> 24U));
    write_mapped8(mapped + 1U, static_cast<std::uint8_t>(value >> 16U));
    write_mapped8(mapped + 2U, static_cast<std::uint8_t>(value >> 8U));
    write_mapped8(mapped + 3U, static_cast<std::uint8_t>(value));
}

void GenesisMemoryBus::set_io8(GenesisAddress address, std::uint8_t value) {
    const auto mapped = address & 0x00FFFFFFU;
    if (mapped < kIoStart || mapped >= kIoEnd) {
        throw std::invalid_argument("address is outside the Genesis I/O window");
    }
    io_[mapped - kIoStart] = value;
}

} // namespace shadowrun
