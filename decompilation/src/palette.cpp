#include "shadowrun/palette.hpp"

namespace shadowrun {

std::uint16_t PaletteAnimator::toward_black(std::uint16_t color) {
    auto component = static_cast<std::uint16_t>(color & 0x000EU);
    if (component != 0) {
        component = static_cast<std::uint16_t>(component - 0x0002U);
    }
    auto stepped = static_cast<std::uint16_t>((color & 0xEEE0U) | component);

    component = static_cast<std::uint16_t>(stepped & 0x00E0U);
    if (component != 0) {
        component = static_cast<std::uint16_t>(component - 0x0020U);
    }
    stepped = static_cast<std::uint16_t>((stepped & 0xEE0EU) | component);

    component = static_cast<std::uint16_t>(stepped & 0x0E00U);
    if (component != 0) {
        component = static_cast<std::uint16_t>(component - 0x0200U);
    }
    return static_cast<std::uint16_t>((stepped & 0xE0EEU) | component);
}

std::uint16_t PaletteAnimator::toward_target(std::uint16_t color,
                                             std::uint16_t target) {
    auto component = static_cast<std::uint16_t>(color & 0x000EU);
    if (component != (target & 0x000EU)) {
        component = static_cast<std::uint16_t>(component + 0x0002U);
    }
    auto stepped = static_cast<std::uint16_t>((color & 0xEEE0U) | component);

    component = static_cast<std::uint16_t>(stepped & 0x00E0U);
    if (component != (target & 0x00E0U)) {
        component = static_cast<std::uint16_t>(component + 0x0020U);
    }
    stepped = static_cast<std::uint16_t>((stepped & 0xEE0EU) | component);

    component = static_cast<std::uint16_t>(stepped & 0x0E00U);
    if (component != (target & 0x0E00U)) {
        component = static_cast<std::uint16_t>(component + 0x0200U);
    }
    return static_cast<std::uint16_t>((stepped & 0xE0EEU) | component);
}

void PaletteAnimator::step_toward_black() const {
    for (std::size_t index = 0; index < palette_ram::kEntryCount; ++index) {
        const auto address = palette_ram::kCurrent +
            static_cast<GenesisAddress>(index * sizeof(std::uint16_t));
        bus_.write16(address, toward_black(bus_.read16(address)));
    }
}

void PaletteAnimator::step_toward_target() const {
    for (std::size_t index = 0; index < palette_ram::kEntryCount; ++index) {
        const auto address = palette_ram::kCurrent +
            static_cast<GenesisAddress>(index * sizeof(std::uint16_t));
        const auto target_address = palette_ram::kTarget +
            static_cast<GenesisAddress>(index * sizeof(std::uint16_t));
        bus_.write16(address, toward_target(bus_.read16(address),
                                            bus_.read16(target_address)));
    }
}

void PaletteAnimator::copy_target_from(GenesisAddress source) const {
    for (std::size_t index = 0; index < palette_ram::kEntryCount / 2U;
         ++index) {
        const auto source_address = source +
            static_cast<GenesisAddress>(index * sizeof(std::uint32_t));
        const auto target_address = palette_ram::kTarget +
            static_cast<GenesisAddress>(index * sizeof(std::uint32_t));
        bus_.write32(target_address, bus_.read32(source_address));
    }
}

} // namespace shadowrun
