#pragma once

#include <cstdint>

namespace shadowrun {

using GenesisAddress = std::uint32_t;

// The original program talks directly to the Genesis memory map. Keeping the
// bus behind this interface lets the reconstruction run against an emulator,
// a trace recorder, or a deterministic unit-test double without pretending
// that a desktop pointer is a Genesis address.
class GenesisBus {
public:
    virtual ~GenesisBus() = default;

    virtual std::uint8_t read8(GenesisAddress address) = 0;
    virtual std::uint16_t read16(GenesisAddress address) = 0;
    virtual std::uint32_t read32(GenesisAddress address) = 0;

    virtual void write8(GenesisAddress address, std::uint8_t value) = 0;
    virtual void write16(GenesisAddress address, std::uint16_t value) = 0;
    virtual void write32(GenesisAddress address, std::uint32_t value) = 0;
};

} // namespace shadowrun
