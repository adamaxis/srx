#pragma once

#include "shadowrun/genesis_bus.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace shadowrun {

namespace genesis {
constexpr GenesisAddress kIoVersion = 0xA10001;
constexpr GenesisAddress kControllerData = 0xA10003;
constexpr GenesisAddress kControllerData2 = 0xA10005;
constexpr GenesisAddress kControllerControl = 0xA10009;
constexpr GenesisAddress kControllerControl2 = 0xA1000B;
constexpr GenesisAddress kZ80BusRequest = 0xA11100;
constexpr GenesisAddress kZ80Ram = 0xA00000;
constexpr GenesisAddress kZ80CommandIndex = 0xA00036;
constexpr GenesisAddress kZ80CommandRing = 0xA01B40;
constexpr GenesisAddress kVdpData = 0xC00000;
constexpr GenesisAddress kVdpControl = 0xC00004;
constexpr GenesisAddress kVdpStatusByte = 0xC00005;
}

struct RegionProbe {
    std::uint8_t raw_io_value = 0;
    std::uint8_t region_index = 0;
};

// Native result of the reset display block around ROM 0x0330. The renderer or
// host window can consume this directly; the GenesisBus adapter below exists
// only for source verification tests.
struct StartupDisplayPlan {
    std::array<std::uint16_t, 5> register_commands{};
    std::uint32_t clear_vram_command = 0;
    std::uint16_t clear_vram_word = 0;
    std::uint32_t text_vram_command = 0;
};

// Native result of ROM 0x0436. The original's A0 pointer and VDP writes become
// ordinary host data: one destination command and one glyph-index sequence.
struct StartupTextTransfer {
    std::uint32_t vdp_command = 0;
    std::vector<std::uint16_t> glyph_indices;
};

RegionProbe decode_region_value(std::uint8_t raw_io_value);
StartupDisplayPlan make_startup_display_plan();
StartupTextTransfer encode_startup_text(std::uint8_t column,
                                        std::uint8_t row,
                                        std::string_view zero_terminated_text);

class ShadowrunBoot {
public:
    explicit ShadowrunBoot(GenesisBus& bus) : bus_(bus) {}

    RegionProbe read_region() const;

    // ROM address 0x330 writes the early VDP register sequence. The values are
    // kept as the actual command words observed in the image, including the
    // register numbers encoded in their upper byte.
    void initialise_display() const;

    // ROM address 0x436. The original receives a column, row, and a zero-ended
    // encoded string; it converts each byte from the ROM's space-based glyph
    // encoding and streams the resulting tile indices to VRAM.
    void write_startup_text(std::uint8_t column, std::uint8_t row,
                            std::string_view zero_terminated_text) const;

private:
    GenesisBus& bus_;
};

} // namespace shadowrun
