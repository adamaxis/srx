#include "shadowrun/boot.hpp"

#include <array>

namespace shadowrun {

RegionProbe ShadowrunBoot::read_region() const {
    const auto raw = bus_.read8(genesis::kIoVersion);
    return RegionProbe{raw, static_cast<std::uint8_t>((raw >> 6U) & 0x03U)};
}

void ShadowrunBoot::initialise_display() const {
    // 0x374..0x384 in the ROM. These are VDP register command words, not
    // ordinary palette or tile data. The following long control write and
    // VRAM write are the next two operations in the same reset path.
    constexpr std::array<std::uint16_t, 5> kInitialVdpRegisters{
        0x8164, 0x8230, 0x8C81, 0x8F02, 0x9001
    };
    for (const auto command : kInitialVdpRegisters) {
        bus_.write16(genesis::kVdpControl, command);
    }
    bus_.write32(genesis::kVdpControl, 0xC0020000U);
    bus_.write16(genesis::kVdpData, 0x0EEEU);
    bus_.write32(genesis::kVdpControl, 0x40000000U);
}

void ShadowrunBoot::write_startup_text(std::uint8_t column, std::uint8_t row,
                                       std::string_view text) const {
    // 0x436 constructs:
    //   0x40000003 | (row << 23) | (column << 17)
    // and sends it to the VDP control port. This is the Genesis VRAM write
    // command used by the startup/region text path.
    const auto command = 0x40000003U |
        (static_cast<std::uint32_t>(row) << 23U) |
        (static_cast<std::uint32_t>(column) << 17U);
    bus_.write32(genesis::kVdpControl, command);

    for (const auto character : text) {
        if (character == '\0') {
            break;
        }
        const auto encoded = static_cast<std::uint16_t>(
            static_cast<unsigned char>(character) >= 0x20U
                ? static_cast<unsigned char>(character) - 0x20U
                : 0U);
        bus_.write16(genesis::kVdpData, encoded);
    }
}

} // namespace shadowrun
