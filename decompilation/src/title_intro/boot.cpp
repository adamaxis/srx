#include "shadowrun/title_intro/boot.hpp"

#include <array>

namespace shadowrun {

RegionProbe decode_region_value(std::uint8_t raw_io_value) {
    return RegionProbe{
        raw_io_value,
        static_cast<std::uint8_t>((raw_io_value >> 6U) & 0x03U),
    };
}

StartupDisplayPlan make_startup_display_plan() {
    return StartupDisplayPlan{
        {0x8164, 0x8230, 0x8C81, 0x8F02, 0x9001},
        0xC0020000U,
        0x0EEEU,
        0x40000000U,
    };
}

StartupTextTransfer encode_startup_text(
    std::uint8_t column, std::uint8_t row,
    std::string_view text) {
    StartupTextTransfer result{};
    // 0x0436: (row << 23) | (column << 17) | 0x40000003.
    result.vdp_command = 0x40000003U |
        (static_cast<std::uint32_t>(row) << 23U) |
        (static_cast<std::uint32_t>(column) << 17U);
    for (const auto character : text) {
        if (character == '\0') {
            break;
        }
        const auto byte = static_cast<unsigned char>(character);
        result.glyph_indices.push_back(
            static_cast<std::uint16_t>(byte >= 0x20U ? byte - 0x20U : 0U));
    }
    return result;
}

RegionProbe ShadowrunBoot::read_region() const {
    return decode_region_value(bus_.read8(genesis::kIoVersion));
}

void ShadowrunBoot::initialise_display() const {
    const auto plan = make_startup_display_plan();
    for (const auto command : plan.register_commands) {
        bus_.write16(genesis::kVdpControl, command);
    }
    bus_.write32(genesis::kVdpControl, plan.clear_vram_command);
    bus_.write16(genesis::kVdpData, plan.clear_vram_word);
    bus_.write32(genesis::kVdpControl, plan.text_vram_command);
}

void ShadowrunBoot::write_startup_text(std::uint8_t column, std::uint8_t row,
                                       std::string_view text) const {
    const auto transfer = encode_startup_text(column, row, text);
    bus_.write32(genesis::kVdpControl, transfer.vdp_command);
    for (const auto glyph : transfer.glyph_indices) {
        bus_.write16(genesis::kVdpData, glyph);
    }
}

} // namespace shadowrun
