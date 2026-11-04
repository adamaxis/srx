#pragma once

#include "shadowrun/core/genesis_bus.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace shadowrun {

struct PortWrite {
    unsigned width = 0;
    GenesisAddress address = 0;
    std::uint32_t value = 0;
};

// A renderer-neutral Genesis address space for the reconstructed routines.
// ROM and RAM are big-endian; VDP writes are recorded for a later graphics
// backend instead of being interpreted here.
class GenesisMemoryBus final : public GenesisBus {
public:
    explicit GenesisMemoryBus(std::vector<std::uint8_t> rom);

    std::uint8_t read8(GenesisAddress address) override;
    std::uint16_t read16(GenesisAddress address) override;
    std::uint32_t read32(GenesisAddress address) override;

    void write8(GenesisAddress address, std::uint8_t value) override;
    void write16(GenesisAddress address, std::uint16_t value) override;
    void write32(GenesisAddress address, std::uint32_t value) override;

    void set_io8(GenesisAddress address, std::uint8_t value);
    void set_vdp_status(std::uint16_t status) { vdp_status_ = status; }
    const std::array<std::uint8_t, 0x10000>& ram() const { return ram_; }
    const std::array<std::uint8_t, 0x10000>& z80_ram() const {
        return z80_ram_;
    }
    const std::vector<PortWrite>& port_writes() const { return port_writes_; }
    void clear_port_writes() { port_writes_.clear(); }

private:
    static constexpr GenesisAddress kRomEnd = 0x200000;
    static constexpr GenesisAddress kZ80RamStart = 0xA00000;
    static constexpr GenesisAddress kZ80RamEnd = 0xA10000;
    static constexpr GenesisAddress kRamStart = 0xFF0000;
    static constexpr GenesisAddress kRamEnd = 0x1000000;
    static constexpr GenesisAddress kIoStart = 0xA10000;
    static constexpr GenesisAddress kIoEnd = 0xA10100;
    static constexpr GenesisAddress kVdpData = 0xC00000;
    static constexpr GenesisAddress kVdpControl = 0xC00004;

    bool is_vdp(GenesisAddress address) const;
    std::uint8_t read_mapped8(GenesisAddress address) const;
    void write_mapped8(GenesisAddress address, std::uint8_t value);
    void record_port(unsigned width, GenesisAddress address, std::uint32_t value);

    std::vector<std::uint8_t> rom_;
    std::array<std::uint8_t, 0x10000> z80_ram_{};
    std::array<std::uint8_t, 0x10000> ram_{};
    std::array<std::uint8_t, 0x100> io_{};
    std::uint16_t vdp_status_ = 0x0004;
    std::vector<PortWrite> port_writes_;
};

} // namespace shadowrun
