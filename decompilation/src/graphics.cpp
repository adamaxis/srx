#include "shadowrun/graphics.hpp"
#include "shadowrun/rom_layout.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <limits>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <utility>

namespace shadowrun::graphics {

namespace {

void require_bytes(std::size_t available, std::size_t required,
                   const char* what) {
    if (required > available) {
        throw std::out_of_range(what);
    }
}

std::uint16_t read_rom_u16(const std::vector<std::uint8_t>& rom,
                           std::size_t offset, const char* what) {
    if (offset > rom.size() || rom.size() - offset < 2U) {
        throw std::out_of_range(what);
    }
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(rom[offset]) << 8U) |
        rom[offset + 1U]);
}

std::uint32_t read_rom_u32(const std::vector<std::uint8_t>& rom,
                           std::size_t offset, const char* what) {
    if (offset > rom.size() || rom.size() - offset < 4U) {
        throw std::out_of_range(what);
    }
    return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
           (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
           (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
           static_cast<std::uint32_t>(rom[offset + 3U]);
}

RgbaImage decode_genesis_intro_aircraft_plane_a_impl(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, native::kNativeIntroAircraftMovementFrameCount>
decode_genesis_intro_aircraft_movement_frames_impl(
    const std::vector<std::uint8_t>& rom);

struct GenesisIntroTrividLayers {
    RgbaImage background;
    RgbaImage slain;
    RgbaImage moving;
};
GenesisIntroTrividLayers decode_genesis_intro_trivid_layers(
    const std::vector<std::uint8_t>& rom);

void write_u16(std::ostream& output, std::uint16_t value) {
    output.put(static_cast<char>(value & 0xFFU));
    output.put(static_cast<char>((value >> 8U) & 0xFFU));
}

void write_u32(std::ostream& output, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        output.put(static_cast<char>((value >> shift) & 0xFFU));
    }
}

void write_native_image(std::ostream& output,
                        const native::NativeImage& image) {
    const auto expected = static_cast<std::size_t>(image.width) * image.height;
    if (image.width == 0 || image.height == 0 || image.pixels.size() != expected) {
        throw std::invalid_argument("invalid native asset image");
    }
    write_u16(output, image.width);
    write_u16(output, image.height);
    write_u32(output, static_cast<std::uint32_t>(image.pixels.size()));
    for (const auto& pixel : image.pixels) {
        output.put(static_cast<char>(pixel.red));
        output.put(static_cast<char>(pixel.green));
        output.put(static_cast<char>(pixel.blue));
        output.put(static_cast<char>(pixel.alpha));
    }
}

std::uint8_t read_pack_u8(std::istream& input, const char* what) {
    const auto value = input.get();
    if (value == std::char_traits<char>::eof()) {
        throw std::out_of_range(what);
    }
    return static_cast<std::uint8_t>(value);
}

std::uint16_t read_pack_u16(std::istream& input, const char* what) {
    const auto low = read_pack_u8(input, what);
    const auto high = read_pack_u8(input, what);
    return static_cast<std::uint16_t>(low | (high << 8U));
}

std::uint32_t read_pack_u32(std::istream& input, const char* what) {
    const auto b0 = read_pack_u8(input, what);
    const auto b1 = read_pack_u8(input, what);
    const auto b2 = read_pack_u8(input, what);
    const auto b3 = read_pack_u8(input, what);
    return static_cast<std::uint32_t>(b0) |
           (static_cast<std::uint32_t>(b1) << 8U) |
           (static_cast<std::uint32_t>(b2) << 16U) |
           (static_cast<std::uint32_t>(b3) << 24U);
}

void read_pack_bytes(std::istream& input, std::uint8_t* destination,
                     std::size_t size, const char* what) {
    if (size > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
        throw std::out_of_range(what);
    }
    input.read(reinterpret_cast<char*>(destination),
               static_cast<std::streamsize>(size));
    if (!input) {
        throw std::out_of_range(what);
    }
}

void write_native_entity_payload(std::ostream& output,
                                const native::NativeFieldEntity& entity) {
    write_u16(output, entity.projection_word_08);
    write_u16(output, entity.kind);
    write_u16(output, entity.record_type);
    output.put(static_cast<char>(entity.flags_10));
    output.put(static_cast<char>(entity.flags_11));
    output.put(static_cast<char>(entity.race_index));
    output.put(static_cast<char>(entity.archetype_index));
    output.put(static_cast<char>(entity.physical_value));
    output.put(static_cast<char>(entity.mental_value));
    output.put(static_cast<char>(entity.active_item_code));
    write_u32(output, entity.state_pointer);
    write_u16(output, entity.state_selector);
    write_u16(output, entity.animation_multiplier);
    write_u16(output, entity.animation_accumulator);
    write_u16(output, entity.animation_threshold);
    write_u16(output, entity.state_index);
    output.put(static_cast<char>(static_cast<std::uint8_t>(
        entity.display_state)));
    output.put(static_cast<char>(static_cast<std::uint8_t>(
        entity.display_substate)));
    output.put(static_cast<char>(entity.display_palette_state));
    output.put(static_cast<char>(entity.menu_status_index));
    output.put(static_cast<char>(entity.attack_bar_index));
    output.put(static_cast<char>(entity.defense_bar_index));
    for (const auto index : entity.attack_bar_indices_by_posture) {
        output.put(static_cast<char>(index));
    }
    for (const auto index : entity.defense_bar_indices_by_posture) {
        output.put(static_cast<char>(index));
    }
    output.put(static_cast<char>(entity.weapons_bar_index));
    output.put(static_cast<char>(entity.guns_bar_index));
    for (const auto code : entity.inventory_item_codes) {
        output.put(static_cast<char>(code));
    }
    for (const auto phase : entity.inventory_phase_bytes) {
        output.put(static_cast<char>(phase));
    }
    output.put(static_cast<char>(entity.equipped_armor_index));
    output.put(static_cast<char>(entity.ammo_rounds));
    output.put(static_cast<char>(entity.ammo_clips));
    write_u16(output, entity.cyberware_flags);
    for (const auto value : entity.attribute_values) {
        output.put(static_cast<char>(value));
    }
    for (const auto value : entity.skill_values) {
        output.put(static_cast<char>(value));
    }
    for (const auto value : entity.attribute_skill_auxiliary) {
        output.put(static_cast<char>(value));
    }
}

native::NativeFieldEntity read_native_entity_payload(std::istream& input,
                                                    bool has_posture_bars) {
    native::NativeFieldEntity entity;
    entity.projection_word_08 = read_pack_u16(
        input, "native entity payload is truncated");
    entity.kind = read_pack_u16(input, "native entity payload is truncated");
    entity.record_type = read_pack_u16(
        input, "native entity payload is truncated");
    entity.flags_10 = read_pack_u8(input, "native entity payload is truncated");
    entity.flags_11 = read_pack_u8(input, "native entity payload is truncated");
    entity.race_index = read_pack_u8(input, "native entity payload is truncated");
    entity.archetype_index = read_pack_u8(
        input, "native entity payload is truncated");
    entity.physical_value = read_pack_u8(
        input, "native entity payload is truncated");
    entity.mental_value = read_pack_u8(
        input, "native entity payload is truncated");
    entity.active_item_code = read_pack_u8(
        input, "native entity payload is truncated");
    entity.state_pointer = read_pack_u32(
        input, "native entity payload is truncated");
    entity.state_selector = read_pack_u16(
        input, "native entity payload is truncated");
    entity.animation_multiplier = read_pack_u16(
        input, "native entity payload is truncated");
    entity.animation_accumulator = read_pack_u16(
        input, "native entity payload is truncated");
    entity.animation_threshold = read_pack_u16(
        input, "native entity payload is truncated");
    entity.state_index = read_pack_u16(
        input, "native entity payload is truncated");
    entity.display_state = static_cast<std::int8_t>(read_pack_u8(
        input, "native entity payload is truncated"));
    entity.display_substate = static_cast<std::int8_t>(read_pack_u8(
        input, "native entity payload is truncated"));
    entity.display_palette_state = read_pack_u8(
        input, "native entity payload is truncated");
    entity.menu_status_index = read_pack_u8(
        input, "native entity payload is truncated");
    entity.attack_bar_index = read_pack_u8(
        input, "native entity payload is truncated");
    entity.defense_bar_index = read_pack_u8(
        input, "native entity payload is truncated");
    if (has_posture_bars) {
        for (auto& index : entity.attack_bar_indices_by_posture) {
            index = read_pack_u8(input, "native entity posture bars are truncated");
        }
        for (auto& index : entity.defense_bar_indices_by_posture) {
            index = read_pack_u8(input, "native entity posture bars are truncated");
        }
    } else {
        entity.attack_bar_indices_by_posture.fill(entity.attack_bar_index);
        entity.defense_bar_indices_by_posture.fill(entity.defense_bar_index);
    }
    entity.weapons_bar_index = read_pack_u8(
        input, "native entity payload is truncated");
    entity.guns_bar_index = read_pack_u8(
        input, "native entity payload is truncated");
    for (auto& code : entity.inventory_item_codes) {
        code = read_pack_u8(input, "native entity payload is truncated");
    }
    for (auto& phase : entity.inventory_phase_bytes) {
        phase = read_pack_u8(input, "native entity payload is truncated");
    }
    entity.equipped_armor_index = read_pack_u8(
        input, "native entity payload is truncated");
    entity.ammo_rounds = read_pack_u8(
        input, "native entity payload is truncated");
    entity.ammo_clips = read_pack_u8(
        input, "native entity payload is truncated");
    entity.cyberware_flags = read_pack_u16(
        input, "native entity payload is truncated");
    for (auto& value : entity.attribute_values) {
        value = read_pack_u8(input, "native entity payload is truncated");
    }
    for (auto& value : entity.skill_values) {
        value = read_pack_u8(input, "native entity payload is truncated");
    }
    for (auto& value : entity.attribute_skill_auxiliary) {
        value = read_pack_u8(input, "native entity payload is truncated");
    }
    return entity;
}

std::string read_genesis_fixed_text(const std::vector<std::uint8_t>& rom,
                                    std::size_t offset, std::size_t length,
                                    const char* what) {
    if (offset > rom.size() || rom.size() - offset < length) {
        throw std::out_of_range(what);
    }
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto byte = rom[offset + index];
        if (byte == 0xFFU) {
            break;
        }
        value.push_back(byte >= 0x20U && byte <= 0x7EU
                            ? static_cast<char>(byte)
                            : ' ');
    }
    const auto first = value.find_first_not_of(' ');
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(' ');
    return value.substr(first, last - first + 1U);
}

// C0DA consumes 0x80/0xFF-delimited source lines whose leading spaces are
// layout, not disposable padding. Keep this separate from the catalog reader
// above, which intentionally trims fixed-width names.
std::string read_genesis_padded_text(const std::vector<std::uint8_t>& rom,
                                     std::size_t offset, std::size_t length,
                                     const char* what) {
    if (offset > rom.size() || rom.size() - offset < length) {
        throw std::out_of_range(what);
    }
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto byte = rom[offset + index];
        if (byte == 0x80U || byte == 0xFFU) {
            break;
        }
        value.push_back(byte >= 0x20U && byte <= 0x7EU
                            ? static_cast<char>(byte)
                            : ' ');
    }
    return value;
}

std::string read_genesis_multiline_text(const std::vector<std::uint8_t>& rom,
                                        std::size_t offset,
                                        std::size_t length,
                                        const char* what) {
    if (offset > rom.size() || rom.size() - offset < length) {
        throw std::out_of_range(what);
    }
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto byte = rom[offset + index];
        if (byte == 0xFFU) {
            break;
        }
        if (byte == 0x80U) {
            value.push_back('\n');
        } else {
            value.push_back(byte >= 0x20U && byte <= 0x7EU
                                ? static_cast<char>(byte)
                                : ' ');
        }
    }
    return value;
}

std::string read_genesis_menu_text(const std::vector<std::uint8_t>& rom,
                                   std::size_t offset, std::size_t length,
                                   const char* what) {
    if (offset > rom.size() || rom.size() - offset < length) {
        throw std::out_of_range(what);
    }
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto byte = rom[offset + index];
        if (byte == 0x80U) {
            continue;
        }
        if (byte >= 0x20U && byte <= 0x7EU) {
            value.push_back(static_cast<char>(byte));
        }
    }
    const auto first = value.find_first_not_of(' ');
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(' ');
    return value.substr(first, last - first + 1U);
}

std::vector<native::NativeStartupTextLine> read_genesis_startup_layout(
    const std::vector<std::uint8_t>& rom, std::size_t start_offset,
    std::size_t end_offset, const char* what) {
    if (start_offset > end_offset || end_offset > rom.size()) {
        throw std::out_of_range(what);
    }
    std::vector<native::NativeStartupTextLine> result;
    std::string line;
    std::size_t row = 0;
    const auto append_line = [&result](std::size_t line_row,
                                       std::string value) {
        // 0x1B6498 / 0x1B64CF store the title underline as font tiles
        // '[' + a run of '\'. Capture t1005/t1065 shows that bar under the
        // title only; the selector is the triangle cursor, not a host bar.
        result.push_back({static_cast<std::uint8_t>(line_row),
                          std::move(value)});
    };
    for (auto offset = start_offset; offset < end_offset; ++offset) {
        const auto byte = rom[offset];
        if (byte == 0xFFU) {
            if (!line.empty()) {
                append_line(row, std::move(line));
            }
            return result;
        }
        if (byte == 0x80U) {
            if (!line.empty()) {
                append_line(row, std::move(line));
                line.clear();
            }
            ++row;
            continue;
        }
        if (byte < 0x20U || byte > 0x7EU) {
            throw std::runtime_error(what);
        }
        line.push_back(static_cast<char>(byte));
    }
    throw std::out_of_range(what);
}

void write_pack_string(std::ostream& output, const std::string& value,
                       const char* what) {
    if (value.size() > 0xFFFFU) {
        throw std::invalid_argument(what);
    }
    write_u16(output, static_cast<std::uint16_t>(value.size()));
    output.write(value.data(), static_cast<std::streamsize>(value.size()));
}

std::string read_pack_string(std::istream& input, const char* what) {
    const auto size = static_cast<std::size_t>(read_pack_u16(input, what));
    if (size > 1024U) {
        throw std::runtime_error("native asset pack string is too long");
    }
    std::string value(size, '\0');
    if (size != 0U) {
        input.read(value.data(), static_cast<std::streamsize>(size));
        if (!input) {
            throw std::out_of_range(what);
        }
    }
    return value;
}

native::NativeImage read_native_image(std::istream& input) {
    const auto width = read_pack_u16(input, "native asset image header is truncated");
    const auto height = read_pack_u16(input, "native asset image header is truncated");
    const auto pixel_count = static_cast<std::size_t>(read_pack_u32(
        input, "native asset image size is truncated"));
    if (width == 0 || height == 0 || width > 8192 || height > 8192 ||
        pixel_count != static_cast<std::size_t>(width) * height) {
        throw std::runtime_error("invalid native asset image dimensions");
    }
    native::NativeImage image{
        width, height,
        std::vector<native::Rgba8>(pixel_count),
    };
    for (auto& pixel : image.pixels) {
        pixel.red = read_pack_u8(input, "native asset RGBA data is truncated");
        pixel.green = read_pack_u8(input, "native asset RGBA data is truncated");
        pixel.blue = read_pack_u8(input, "native asset RGBA data is truncated");
        pixel.alpha = read_pack_u8(input, "native asset RGBA data is truncated");
    }
    return image;
}

} // namespace

GenesisFieldEntityTemplate read_genesis_field_entity_template(
    const std::vector<std::uint8_t>& rom, std::size_t record_offset) {
    if (record_offset > rom.size() || rom.size() - record_offset < 0x92U) {
        throw std::out_of_range(
            "Genesis field entity template state is truncated");
    }
    const auto flags = read_rom_u16(
        rom, record_offset + 0x10U,
        "Genesis field entity template flags are truncated");
    GenesisFieldEntityTemplate result;
    result.record_offset = record_offset;
    result.projection_word_08 = read_rom_u16(
        rom, record_offset + 0x08U,
        "Genesis field entity template projection is truncated");
    result.kind = read_rom_u16(
        rom, record_offset + 0x0EU,
        "Genesis field entity template kind is truncated");
    result.record_type = read_rom_u16(
        rom, record_offset + 0x3EU,
        "Genesis field entity record type is truncated");
    result.race_index = rom[record_offset + 0x00U];
    result.archetype_index = rom[record_offset + 0x9BU];
    result.physical_value = rom[record_offset + 0x4CU];
    result.mental_value = rom[record_offset + 0x9CU];
    result.active_item_code = rom[record_offset + 0x57U];
    result.flags_10 = static_cast<std::uint8_t>(flags >> 8U);
    result.flags_11 = static_cast<std::uint8_t>(flags);
    result.state_pointer = read_rom_u32(
        rom, record_offset + 0x12U,
        "Genesis field entity template state pointer is truncated");
    result.state_selector = read_rom_u16(
        rom, record_offset + 0x16U,
        "Genesis field entity template selector is truncated");
    result.timing_multiplier = read_rom_u16(
        rom, record_offset + 0x18U,
        "Genesis field entity template multiplier is truncated");
    result.timing_accumulator = read_rom_u16(
        rom, record_offset + 0x1AU,
        "Genesis field entity template accumulator is truncated");
    result.timing_threshold = read_rom_u16(
        rom, record_offset + 0x1CU,
        "Genesis field entity template threshold is truncated");
    result.state_index = read_rom_u16(
        rom, record_offset + 0x1EU,
        "Genesis field entity template state index is truncated");
    result.display_state = static_cast<std::int8_t>(rom[record_offset + 0x56U]);
    result.display_substate =
        static_cast<std::int8_t>(rom[record_offset + 0x57U]);
    result.display_palette_state = rom[record_offset + 0x9BU];
    result.menu_status_index = rom[record_offset + 0x4DU];
    for (std::size_t index = 0; index < result.inventory_item_codes.size();
         ++index) {
        result.inventory_item_codes[index] =
            rom[record_offset + 0x66U + index];
        result.inventory_phase_bytes[index] =
            rom[record_offset + 0x6EU + index];
    }
    result.equipped_armor_index = rom[record_offset + 0x76U];
    result.ammo_rounds = rom[record_offset + 0x4EU];
    result.ammo_clips = rom[record_offset + 0x4FU];
    result.cyberware_flags = read_rom_u16(
        rom, record_offset + 0x90U,
        "Genesis field entity cyberware flags are truncated");
    for (std::size_t index = 0; index < result.attribute_values.size();
         ++index) {
        result.attribute_values[index] =
            rom[record_offset + 0x77U + index];
    }
    for (std::size_t index = 0; index < result.skill_values.size();
         ++index) {
        const auto offset = index < 6U
            ? 0x80U + index
            : 0x87U + (index - 6U);
        result.skill_values[index] = rom[record_offset + offset];
    }
    constexpr std::array<std::size_t,
                         native::kNativeAttributeSkillAuxiliaryCount>
        auxiliary_offsets{{0x7FU, 0x86U, 0x8DU, 0x8EU}};
    for (std::size_t index = 0; index < auxiliary_offsets.size(); ++index) {
        result.attribute_skill_auxiliary[index] =
            rom[record_offset + auxiliary_offsets[index]];
    }
    return result;
}

std::array<native::NativeInventoryItem, native::kNativeInventoryItemCount>
read_genesis_inventory_catalog(const std::vector<std::uint8_t>& rom) {
    std::array<native::NativeInventoryItem, native::kNativeInventoryItemCount>
        result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        const auto record_offset =
            static_cast<std::size_t>(rom::kInventoryItemTable) +
            index * rom::kInventoryItemRecordStride;
        result[index].code = static_cast<std::uint8_t>(index + 1U);
        result[index].source_combat_factor = rom[record_offset + 0x24U];
        result[index].damage_table_index = rom[record_offset + 0x25U];
        result[index].power_table_index = rom[record_offset + 0x26U];
        result[index].name = read_genesis_fixed_text(
            rom, record_offset, 12U,
            "Genesis inventory item name is truncated");
        result[index].category = read_genesis_fixed_text(
            rom, record_offset + 0x0DU, 12U,
            "Genesis inventory item category is truncated");
    }
    return result;
}

std::array<std::string, native::kNativeCyberwareCount>
read_genesis_cyberware_catalog(const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeCyberwareCount> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        const auto pointer_offset =
            static_cast<std::size_t>(rom::kCyberwarePointerTable) +
            index * 4U;
        const auto string_offset = static_cast<std::size_t>(read_rom_u32(
            rom, pointer_offset,
            "Genesis cyberware pointer table is truncated"));
        result[index] = read_genesis_fixed_text(
            rom, string_offset, 64U,
            "Genesis cyberware name is truncated");
    }
    return result;
}

std::array<std::string, native::kNativeGroupItemCount>
read_genesis_group_item_catalog(const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeGroupItemCount> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        const auto pointer_offset =
            static_cast<std::size_t>(rom::kGroupItemPointerTable) +
            index * 4U;
        const auto string_offset = static_cast<std::size_t>(read_rom_u32(
            rom, pointer_offset,
            "Genesis group-item pointer table is truncated"));
        result[index] = read_genesis_fixed_text(
            rom, string_offset, 64U,
            "Genesis group-item name is truncated");
    }
    return result;
}

std::array<std::string, native::kNativeCyberdeckUnavailableLineCount>
read_genesis_cyberdeck_unavailable_text(
    const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeCyberdeckUnavailableLineCount>
        result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        result[index] = read_genesis_padded_text(
            rom, rom::kCyberdeckUnavailableText[index],
            rom::kCyberdeckUnavailableTextLengths[index],
            "Genesis cyberdeck-unavailable text is truncated");
    }
    return result;
}

std::array<std::string, native::kNativeDismissRunnerLabelCount>
read_genesis_dismiss_runner_labels(const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeDismissRunnerLabelCount> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        result[index] = read_genesis_padded_text(
            rom, rom::kDismissRunnerLabelText[index],
            rom::kDismissRunnerLabelTextLengths[index],
            "Genesis dismiss-runner label is truncated");
    }
    return result;
}

std::array<std::string, native::kNativeNotebookLabelCount>
read_genesis_notebook_labels(const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeNotebookLabelCount> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        result[index] = read_genesis_padded_text(
            rom, rom::kNotebookLabelText[index],
            rom::kNotebookLabelTextLengths[index],
            "Genesis Notebook label is truncated");
    }
    return result;
}

std::array<std::string, native::kNativeNotebookNoRunLineCount>
read_genesis_notebook_no_run_text(const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeNotebookNoRunLineCount> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        result[index] = read_genesis_padded_text(
            rom, rom::kNotebookNoRunText[index],
            rom::kNotebookNoRunTextLengths[index],
            "Genesis Notebook no-run text is truncated");
    }
    return result;
}

std::array<std::string, native::kNativeNotebookTipCount>
read_genesis_notebook_tip_catalog(const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeNotebookTipCount> result{};
    for (std::size_t group = 0; group < rom::kNotebookTipPointerTables.size();
         ++group) {
        for (std::size_t entry = 0;
             entry < rom::kNotebookTipTableEntryCount; ++entry) {
            const auto table_offset =
                static_cast<std::size_t>(rom::kNotebookTipPointerTables[group]) +
                entry * 4U;
            const auto string_offset = static_cast<std::size_t>(read_rom_u32(
                rom, table_offset,
                "Genesis Notebook tip pointer table is truncated"));
            result[group * rom::kNotebookTipTableEntryCount + entry] =
                read_genesis_multiline_text(
                    rom, string_offset, rom.size() - string_offset,
                    "Genesis Notebook tip text is truncated");
        }
    }
    return result;
}

std::array<std::string, native::kNativeNotebookMrJohnsonCount>
read_genesis_notebook_mr_johnson_names(
    const std::vector<std::uint8_t>& rom) {
    std::array<std::string, native::kNativeNotebookMrJohnsonCount> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        result[index] = read_genesis_padded_text(
            rom, rom::kNotebookMrJohnsonText[index],
            rom::kNotebookMrJohnsonTextLengths[index],
            "Genesis Mr. Johnson name is truncated");
    }
    return result;
}

std::array<std::string,
           native::kNativeAttributeCount + native::kNativeSkillCount>
read_genesis_attribute_skill_catalog(const std::vector<std::uint8_t>& rom) {
    std::array<std::string,
               native::kNativeAttributeCount + native::kNativeSkillCount>
        result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        const auto pointer_offset =
            static_cast<std::size_t>(rom::kAttributeSkillPointerTable) +
            index * 4U;
        const auto string_offset = static_cast<std::size_t>(read_rom_u32(
            rom, pointer_offset,
            "Genesis attribute/skill pointer table is truncated"));
        result[index] = read_genesis_fixed_text(
            rom, string_offset, 64U,
            "Genesis attribute/skill name is truncated");
    }
    return result;
}

native::NativeIntroData read_genesis_intro_data(
    const std::vector<std::uint8_t>& rom) {
    native::NativeIntroData result;
    result.start_options_title = read_genesis_menu_text(
        rom, rom::kStartupOptionsTitleText, 13U,
        "Genesis startup options title is truncated");
    result.start_options[0] = read_genesis_menu_text(
        rom, rom::kStartupNewGameText, 8U,
        "Genesis new-game label is truncated");
    result.start_options[1] = read_genesis_menu_text(
        rom, rom::kStartupLoadGameText, 9U,
        "Genesis load-game label is truncated");
    result.archetype_title = read_genesis_menu_text(
        rom, rom::kStartupArchetypeTitleText, 17U,
        "Genesis archetype title is truncated");
    result.archetypes[0] = read_genesis_menu_text(
        rom, rom::kStartupSamuraiText, 7U,
        "Genesis Samurai label is truncated");
    result.archetypes[1] = read_genesis_menu_text(
        rom, rom::kStartupDeckerText, 6U,
        "Genesis Decker label is truncated");
    result.archetypes[2] = read_genesis_menu_text(
        rom, rom::kStartupGatorText, 5U,
        "Genesis Gator label is truncated");
    result.archetypes[3] = read_genesis_menu_text(
        rom, rom::kStartupShamanText, 6U,
        "Genesis Shaman label is truncated");
    result.start_options_layout = read_genesis_startup_layout(
        rom, 0x1B6498U, 0x1B64CFU,
        "Genesis startup-options layout is truncated");
    result.archetype_layout = read_genesis_startup_layout(
        rom, 0x1B64CFU, 0x1B6518U,
        "Genesis archetype layout is truncated");
    result.save_load_layout = read_genesis_startup_layout(
        rom, 0x1B6518U, 0x1B6527U,
        "Genesis save/load layout is truncated");
    result.no_save_layout = read_genesis_startup_layout(
        rom, 0x1B6527U, 0x1B6577U,
        "Genesis no-save message is truncated");
    result.save_success_layout = read_genesis_startup_layout(
        rom, 0x1B6577U, 0x1B65B9U,
        "Genesis save-success message is truncated");

    if (rom::kIntroStoryTextEnd > rom.size() ||
        rom::kIntroStoryText > rom::kIntroStoryTextEnd) {
        throw std::out_of_range("Genesis opening story block is truncated");
    }
    auto offset = static_cast<std::size_t>(rom::kIntroStoryText);
    const auto end = static_cast<std::size_t>(rom::kIntroStoryTextEnd);
    while (offset < end) {
        std::string line;
        while (offset < end && rom[offset] != 0xFFU) {
            const auto byte = rom[offset++];
            if (byte < 0x20U || byte > 0x7EU) {
                throw std::runtime_error(
                    "Genesis opening story contains an unsupported text byte");
            }
            line.push_back(static_cast<char>(byte));
        }
        if (offset == end) {
            throw std::out_of_range(
                "Genesis opening story line terminator is truncated");
        }
        result.story_lines.push_back(std::move(line));
        ++offset;
    }
    return result;
}

std::array<RgbaImage, 23> decode_genesis_field_menu_stat_bars(
    const std::vector<std::uint8_t>& rom, std::size_t table_offset,
    const char* table_name);
void fill_grayscale_entity_assets(native::NativePortAssets& assets);

native::NativePortAssets decode_genesis_native_assets(
    const std::vector<std::uint8_t>& rom) {
    native::NativePortAssets assets;
    auto title = decode_genesis_title_screen(rom);
    assets.title_scene = {
        title.width, title.height, std::move(title.pixels)};
    const auto title_background = decode_genesis_title_background(rom);
    assets.title_background = {
        title_background.width, title_background.height,
        title_background.pixels};
    const auto title_lights = decode_genesis_title_circuit_lights(rom);
    for (std::size_t index = 0; index < title_lights.size(); ++index) {
        assets.title_circuit_lights[index] = {
            title_lights[index].width, title_lights[index].height,
            title_lights[index].pixels};
    }
    const auto title_logo_plane = decode_genesis_title_logo_plane(rom);
    assets.title_logo_plane = {
        title_logo_plane.width, title_logo_plane.height,
        title_logo_plane.pixels};
    const auto title_text = decode_genesis_title_text(rom);
    assets.title_text = {
        title_text.width, title_text.height, title_text.pixels};
    const auto title_sprite_frames = decode_genesis_title_sprite_frames(rom);
    for (std::size_t index = 0; index < title_sprite_frames.size(); ++index) {
        assets.title_sprite_frames[index] = {
            title_sprite_frames[index].width,
            title_sprite_frames[index].height,
            title_sprite_frames[index].pixels};
    }
    const auto title_frames = decode_genesis_title_animation_frames(rom);
    for (std::size_t index = 0; index < title_frames.size(); ++index) {
        assets.title_animation_frames[index] = {
            title_frames[index].width, title_frames[index].height,
            title_frames[index].pixels};
    }
    const auto boot_background = decode_genesis_boot_background(rom);
    assets.boot_background = {
        boot_background.width, boot_background.height, boot_background.pixels};
    const auto boot_static = decode_genesis_boot_static(rom);
    assets.boot_static = {
        boot_static.width, boot_static.height, boot_static.pixels};
    const auto boot_guy = decode_genesis_boot_guy(rom);
    assets.boot_guy = {boot_guy.width, boot_guy.height, boot_guy.pixels};
    const auto boot_sega = decode_genesis_boot_sega_frames(rom);
    for (std::size_t index = 0; index < boot_sega.size(); ++index) {
        assets.boot_sega_frames[index] = {
            boot_sega[index].width, boot_sega[index].height,
            boot_sega[index].pixels};
    }
    const auto boot_strobe = decode_genesis_boot_logo_strobe(rom);
    for (std::size_t index = 0; index < boot_strobe.size(); ++index) {
        assets.boot_logo_strobe[index] = {
            boot_strobe[index].width, boot_strobe[index].height,
            boot_strobe[index].pixels};
    }
    const auto boot_fasa = decode_genesis_boot_fasa(rom);
    assets.boot_fasa = {boot_fasa.width, boot_fasa.height, boot_fasa.pixels};
    const auto intro_stages = decode_genesis_intro_stages(rom);
    for (std::size_t index = 0; index < intro_stages.size(); ++index) {
        assets.intro_stages[index] = {
            intro_stages[index].width, intro_stages[index].height,
            intro_stages[index].pixels};
    }
    const auto intro_wilderness_pan =
        decode_genesis_intro_wilderness_pan_frames(rom);
    for (std::size_t index = 0; index < intro_wilderness_pan.size(); ++index) {
        assets.intro_wilderness_pan_frames[index] = {
            intro_wilderness_pan[index].width,
            intro_wilderness_pan[index].height,
            intro_wilderness_pan[index].pixels};
    }
    const auto intro_ambush = decode_genesis_intro_ambush_frames(rom);
    for (std::size_t index = 0; index < intro_ambush.size(); ++index) {
        assets.intro_ambush_frames[index] = {
            intro_ambush[index].width, intro_ambush[index].height,
            intro_ambush[index].pixels};
    }
    const auto intro_heli = decode_genesis_intro_heli_frames(rom);
    for (std::size_t index = 0; index < intro_heli.size(); ++index) {
        assets.intro_heli_frames[index] = {
            intro_heli[index].width, intro_heli[index].height,
            intro_heli[index].pixels};
    }
    const auto intro_aircraft_plane_a =
        decode_genesis_intro_aircraft_plane_a_impl(rom);
    assets.intro_aircraft_plane_a = {
        intro_aircraft_plane_a.width, intro_aircraft_plane_a.height,
        intro_aircraft_plane_a.pixels};
    const auto intro_aircraft_movement =
        decode_genesis_intro_aircraft_movement_frames_impl(rom);
    for (std::size_t index = 0; index < intro_aircraft_movement.size();
         ++index) {
        assets.intro_aircraft_movement_frames[index] = {
            intro_aircraft_movement[index].width,
            intro_aircraft_movement[index].height,
            intro_aircraft_movement[index].pixels};
    }
    const auto intro_trivid = decode_genesis_intro_trivid_layers(rom);
    assets.intro_trivid_background = {
        intro_trivid.background.width, intro_trivid.background.height,
        std::move(intro_trivid.background.pixels)};
    assets.intro_trivid_slain = {
        intro_trivid.slain.width, intro_trivid.slain.height,
        std::move(intro_trivid.slain.pixels)};
    assets.intro_trivid_moving = {
        intro_trivid.moving.width, intro_trivid.moving.height,
        std::move(intro_trivid.moving.pixels)};
    const auto cab_map = decode_genesis_cab_map(rom);
    assets.cab_map = {
        cab_map.width, cab_map.height, std::move(cab_map.pixels)};
    const auto field_hud = decode_genesis_field_hud(rom);
    assets.field_hud = {
        field_hud.width, field_hud.height, field_hud.pixels};
    const auto pocket_secretary_background =
        decode_genesis_pocket_secretary_background(rom);
    assets.pocket_secretary_background = {
        pocket_secretary_background.width, pocket_secretary_background.height,
        pocket_secretary_background.pixels};
    auto field_menu_background = decode_genesis_field_menu_background(rom);
    assets.field_menu_background = {
        field_menu_background.width, field_menu_background.height,
        std::move(field_menu_background.pixels)};
    const auto field_menu_action_background =
        decode_genesis_field_menu_action_background(rom);
    assets.field_menu_action_background = {
        field_menu_action_background.width,
        field_menu_action_background.height,
        field_menu_action_background.pixels};
    auto field_menu_attributes_background =
        decode_genesis_field_menu_attributes_background(rom);
    assets.field_menu_attributes_background = {
        field_menu_attributes_background.width,
        field_menu_attributes_background.height,
        std::move(field_menu_attributes_background.pixels)};
    auto startup_background = decode_genesis_startup_background(rom);
    assets.startup_background = {
        startup_background.width, startup_background.height,
        std::move(startup_background.pixels)};
    auto field_menu_inventory_boxes =
        decode_genesis_field_menu_inventory_boxes(rom);
    assets.field_menu_inventory_boxes = {
        field_menu_inventory_boxes.width, field_menu_inventory_boxes.height,
        std::move(field_menu_inventory_boxes.pixels)};
    auto field_menu_font = decode_genesis_field_menu_font(rom);
    assets.field_menu_font = {
        field_menu_font.width, field_menu_font.height,
        std::move(field_menu_font.pixels)};
    const auto startup_font = decode_genesis_startup_font(rom);
    assets.startup_font = {
        startup_font.width, startup_font.height, startup_font.pixels};
    const auto startup_cursor = decode_genesis_startup_cursor(rom);
    assets.startup_cursor = {
        startup_cursor.width, startup_cursor.height, startup_cursor.pixels};
    const auto runner_portraits =
        decode_genesis_field_menu_runner_portraits(rom);
    for (std::size_t index = 0; index < runner_portraits.size(); ++index) {
        assets.field_menu_runner_portraits[index] = {
            runner_portraits[index].width, runner_portraits[index].height,
            runner_portraits[index].pixels};
        assets.field_menu_runner_names[index] = read_genesis_menu_text(
            rom, 0x7291AU + index * 0x100U + 0x58U, 16U,
            "Genesis runner menu name is truncated");
    }
    const auto selected_cells =
        decode_genesis_field_menu_selected_cells(rom);
    for (std::size_t index = 0; index < selected_cells.size(); ++index) {
        assets.field_menu_selected_cells[index] = {
            selected_cells[index].width, selected_cells[index].height,
            selected_cells[index].pixels};
    }
    const auto command_focus =
        decode_genesis_field_menu_command_focus(rom);
    for (std::size_t index = 0; index < command_focus.size(); ++index) {
        assets.field_menu_command_focus[index] = {
            command_focus[index].width, command_focus[index].height,
            command_focus[index].pixels};
    }
    const auto action_focus =
        decode_genesis_field_menu_action_focus(rom);
    for (std::size_t index = 0; index < action_focus.size(); ++index) {
        assets.field_menu_action_focus[index] = {
            action_focus[index].width, action_focus[index].height,
            action_focus[index].pixels};
    }
    const auto clips_focus = decode_genesis_field_menu_clips_focus(rom);
    for (std::size_t index = 0; index < clips_focus.size(); ++index) {
        assets.field_menu_clips_focus[index] = {
            clips_focus[index].width, clips_focus[index].height,
            clips_focus[index].pixels};
    }
    const auto clips_trade_focus =
        decode_genesis_field_menu_clips_trade_focus(rom);
    for (std::size_t index = 0; index < clips_trade_focus.size(); ++index) {
        assets.field_menu_clips_trade_focus[index] = {
            clips_trade_focus[index].width, clips_trade_focus[index].height,
            clips_trade_focus[index].pixels};
    }
    const auto posture_focus = decode_genesis_field_menu_posture_focus(rom);
    assets.field_menu_posture_focus = {
        posture_focus.width, posture_focus.height, posture_focus.pixels};
    const auto field_menu_cursor = decode_genesis_field_menu_cursor(rom);
    assets.field_menu_cursor = {
        field_menu_cursor.width, field_menu_cursor.height,
        std::move(field_menu_cursor.pixels)};
    const auto item_art = decode_genesis_field_menu_item_art(rom);
    for (std::size_t item = 0; item < item_art.size(); ++item) {
        assets.field_menu_item_art[item] = {
            item_art[item].width, item_art[item].height,
            item_art[item].pixels};
    }
    const auto damage_bars = decode_genesis_field_menu_damage_bars(rom);
    for (std::size_t index = 0; index < damage_bars.size(); ++index) {
        assets.field_menu_damage_bars[index] = {
            damage_bars[index].width, damage_bars[index].height,
            damage_bars[index].pixels};
    }
    const auto drain_bars = decode_genesis_field_menu_drain_bars(rom);
    for (std::size_t index = 0; index < drain_bars.size(); ++index) {
        assets.field_menu_drain_bars[index] = {
            drain_bars[index].width, drain_bars[index].height,
            drain_bars[index].pixels};
    }
    const auto detail_values = decode_genesis_field_menu_detail_values(rom);
    for (std::size_t index = 0; index < detail_values.size(); ++index) {
        assets.field_menu_detail_values[index] = {
            detail_values[index].width, detail_values[index].height,
            detail_values[index].pixels};
    }
    const auto attack_bars = decode_genesis_field_menu_stat_bars(
        rom, 0xCF3D4U, "Genesis attack bar table is truncated");
    const auto defense_bars = decode_genesis_field_menu_stat_bars(
        rom, 0xCF610U, "Genesis defense bar table is truncated");
    for (std::size_t index = 0; index < attack_bars.size(); ++index) {
        assets.field_menu_attack_bars[index] = native::NativeImage{
            attack_bars[index].width, attack_bars[index].height,
            attack_bars[index].pixels};
        assets.field_menu_defense_bars[index] = native::NativeImage{
            defense_bars[index].width, defense_bars[index].height,
            defense_bars[index].pixels};
    }
    assets.field_menu_power_labels =
        decode_genesis_field_menu_power_labels(rom);
    for (std::size_t index = 0; index < assets.field_menu_armor_labels.size();
         ++index) {
        const auto layout = read_genesis_startup_layout(
            rom, 0xD4D50U + index * 0x30U,
            0xD4D50U + (index + 1U) * 0x30U,
            "Genesis armor label is truncated");
        for (const auto& line : layout) {
            if (line.row < assets.field_menu_armor_labels[index].size()) {
                assets.field_menu_armor_labels[index][line.row] = line.text;
            }
        }
    }
    const auto menu_status_bars =
        decode_genesis_field_menu_status_bars(rom);
    for (std::size_t index = 0; index < menu_status_bars.size(); ++index) {
        assets.field_menu_status_bars[index] = {
            menu_status_bars[index].width, menu_status_bars[index].height,
            menu_status_bars[index].pixels};
        assets.field_menu_status_names[index] = read_genesis_menu_text(
            rom, 0xD1A05U + index * 0x0DU, 0x0DU,
            "Genesis command-screen status name is truncated");
    }
    const auto inventory_icons =
        decode_genesis_field_menu_inventory_icons(rom);
    for (std::size_t item = 0; item < inventory_icons.size(); ++item) {
        for (std::size_t variant = 0; variant < 2U; ++variant) {
            assets.field_menu_inventory_icons[item][variant] = {
                inventory_icons[item][variant].width,
                inventory_icons[item][variant].height,
                inventory_icons[item][variant].pixels};
        }
    }
    assets.intro = read_genesis_intro_data(rom);

    const auto entity_frames = decode_genesis_field_entity_animation(rom);
    assets.entity_frames.reserve(entity_frames.size());
    for (const auto& frame : entity_frames) {
        assets.entity_frames.push_back(
            {frame.width, frame.height, frame.pixels});
    }
    const auto entity_animations =
        decode_genesis_field_entity_animation_sequences(rom);
    if (entity_animations.size() != assets.field_entity_animations.size()) {
        throw std::runtime_error(
            "Genesis field entity animation direction count is invalid");
    }
    for (std::size_t index = 0; index < entity_animations.size(); ++index) {
        const auto& source = entity_animations[index];
        auto& destination = assets.field_entity_animations[index];
        destination.state_selector = source.state_selector;
        destination.state_marker = source.state_marker;
        destination.state_auxiliary_word = source.state_auxiliary_word;
        destination.source_timing_value = source.source_timing_value;
        destination.timing_multiplier = source.timing_multiplier;
        destination.frames.reserve(source.frames.size());
        for (const auto& frame : source.frames) {
            destination.frames.push_back(
                {frame.width, frame.height, frame.pixels});
        }
    }

    const auto resources = read_genesis_mode2_field_resources(rom);
    auto field_map = decode_genesis_mode2_field_map(rom, resources);
    assets.field_map = {
        field_map.width, field_map.height, std::move(field_map.pixels)};
    assets.field_terrain = decode_genesis_mode2_terrain_map(rom, resources);
    for (std::size_t index = 0; index < assets.initial_field_records.size();
         ++index) {
        assets.initial_field_records[index] = {
            {
                resources.initial_field_records[index].world_x_fixed,
                resources.initial_field_records[index].world_y_fixed,
            },
        };
    }
    assets.initial_camera_x_fixed = resources.initial_camera_x_fixed;
    assets.initial_camera_y_fixed = resources.initial_camera_y_fixed;
    assets.field_event_mode_table = resources.event_mode_table;
    fill_grayscale_entity_assets(assets);

    // Keep the legacy mode-2 fields above for existing callers, while also
    // creating the host area catalog consumed by the native transition loop.
    // These are every non-empty destination named by the mode-2 Redmond
    // reaction-mode table: 0x14/0x1A/0x0D/0x13/0x34/0x32/0x33. The decoder has
    // reviewed resource boundaries for all seven, so keep the native area
    // catalog data-driven instead of making the field loop special-case only
    // the first doorway.
    native::NativeFieldArea mode2_area;
    mode2_area.mode_id = 2;
    mode2_area.map = assets.field_map;
    mode2_area.terrain = assets.field_terrain;
    mode2_area.initial_camera_x_fixed = assets.initial_camera_x_fixed;
    mode2_area.initial_camera_y_fixed = assets.initial_camera_y_fixed;
    mode2_area.event_mode_table = assets.field_event_mode_table;
    assets.field_areas.push_back(std::move(mode2_area));

    // The ROM mode pointer table contains 56 descriptors, not only the
    // handful reached by the opening field. Keep mode 2 first because its
    // event/terrain state is shared with the native shell, then preserve the
    // complete ROM order for every other map resource.
    for (std::uint16_t mode_id = 0; mode_id < rom::kFieldModeCount;
         ++mode_id) {
        if (mode_id == 2U) {
            continue;
        }
        try {
            const auto area_resources = read_genesis_field_mode_resources(
                rom, mode_id);
            const auto area_map = decode_genesis_field_mode_map(
                rom, area_resources);
            native::NativeFieldArea area;
            area.mode_id = mode_id;
            area.map = {area_map.width, area_map.height, area_map.pixels};
            area.terrain = decode_genesis_field_mode_terrain_map(
                rom, area_resources);
            area.initial_camera_x_fixed = area_resources.initial_camera_x_fixed;
            area.initial_camera_y_fixed = area_resources.initial_camera_y_fixed;
            area.event_mode_table = area_resources.event_mode_table;
            for (std::size_t index = 0;
                 index < area.initial_field_records.size(); ++index) {
                area.initial_field_records[index] = {
                    {
                        area_resources.initial_field_records[index].world_x_fixed,
                        area_resources.initial_field_records[index].world_y_fixed,
                    },
                    assets.initial_field_records[index].record_type,
                    assets.initial_field_records[index].flags_10,
                    assets.initial_field_records[index].flags_11,
                };
            }
            assets.field_areas.push_back(std::move(area));
        } catch (const std::exception& error) {
            throw std::runtime_error("Genesis field mode " +
                                     std::to_string(mode_id) + ": " +
                                     error.what());
        }
    }

    // 0x42B0 copies one of the first three records at 0x7291A into Joshua's
    // live record. Those records are Samurai, Decker, and Gator Shaman
    // templates; they are not three runners that C can cycle through.
    const auto copy_entity_data = [](const GenesisFieldEntityTemplate& source,
                                     native::NativeFieldEntity& destination) {
        destination.projection_word_08 = source.projection_word_08;
        destination.kind = source.kind;
        destination.record_type = source.record_type;
        destination.race_index = source.race_index;
        destination.archetype_index = source.archetype_index;
        destination.physical_value = source.physical_value;
        destination.mental_value = source.mental_value;
        destination.active_item_code = source.active_item_code;
        destination.flags_10 = source.flags_10;
        destination.flags_11 = source.flags_11;
        destination.state_pointer = source.state_pointer;
        destination.state_selector = source.state_selector;
        destination.animation_multiplier = source.timing_multiplier;
        destination.animation_accumulator = source.timing_accumulator;
        destination.animation_threshold = source.timing_threshold;
        destination.state_index = source.state_index;
        destination.display_state = source.display_state;
        destination.display_substate = source.display_substate;
        destination.display_palette_state = source.display_palette_state;
        destination.menu_status_index = source.menu_status_index;
        destination.inventory_item_codes = source.inventory_item_codes;
        destination.inventory_phase_bytes = source.inventory_phase_bytes;
        destination.equipped_armor_index = source.equipped_armor_index;
        destination.ammo_rounds = source.ammo_rounds;
        destination.ammo_clips = source.ammo_clips;
        destination.cyberware_flags = source.cyberware_flags;
        destination.attribute_values = source.attribute_values;
        destination.skill_values = source.skill_values;
        destination.attribute_skill_auxiliary = source.attribute_skill_auxiliary;
    };
    assets.inventory_items = read_genesis_inventory_catalog(rom);
    std::array<GenesisFieldEntityTemplate, 3> hud_records{};
    for (std::size_t index = 0;
         index < assets.protagonist_archetype_records.size(); ++index) {
        const auto archetype_source = read_genesis_field_entity_template(
            rom, 0x7291AU + index * 0x100U);
        hud_records[index] = archetype_source;
        auto& archetype = assets.protagonist_archetype_records[index];
        copy_entity_data(archetype_source, archetype);
        archetype.record_index = 0;
        archetype.cyberware_flags = read_rom_u16(
            rom, archetype_source.record_offset + 0x90U,
            "Genesis protagonist cyberware flags are truncated");
        for (std::size_t posture = 0;
             posture < archetype.attack_bar_indices_by_posture.size();
             ++posture) {
            const auto bars = native::compute_command_bar_indices(
                archetype, assets.inventory_items,
                static_cast<std::uint8_t>(posture), 6U, 0U);
            archetype.attack_bar_indices_by_posture[posture] = bars.attack;
            archetype.defense_bar_indices_by_posture[posture] = bars.defense;
        }
        archetype.attack_bar_index =
            archetype.attack_bar_indices_by_posture[
                archetype.menu_status_index];
        archetype.defense_bar_index =
            archetype.defense_bar_indices_by_posture[
                archetype.menu_status_index];
        const auto armor_index = archetype_source.equipped_armor_index;
        if (armor_index != 0U && armor_index <= 7U) {
            const auto armor_offset = 0xD4D50U +
                (static_cast<std::size_t>(armor_index) - 1U) * 0x30U;
            archetype.weapons_bar_index = rom[armor_offset + 0x26U];
            archetype.guns_bar_index = rom[armor_offset + 0x25U];
        }
    }
    const auto field_hud_layers = decode_genesis_field_hud_layers(
        rom, hud_records);
    for (std::size_t index = 0; index < hud_records.size(); ++index) {
        assets.field_hud_faces[index] = {
            field_hud_layers.faces[index].width,
            field_hud_layers.faces[index].height,
            field_hud_layers.faces[index].pixels};
        assets.field_hud_item_blocks[index] = {
            field_hud_layers.item_blocks[index].width,
            field_hud_layers.item_blocks[index].height,
            field_hud_layers.item_blocks[index].pixels};
        assets.field_hud_unarmed_item_blocks[index] = {
            field_hud_layers.unarmed_item_blocks[index].width,
            field_hud_layers.unarmed_item_blocks[index].height,
            field_hud_layers.unarmed_item_blocks[index].pixels};
        assets.field_hud_vertical_counters[index] = {
            field_hud_layers.vertical_counters[index].width,
            field_hud_layers.vertical_counters[index].height,
            field_hud_layers.vertical_counters[index].pixels};
        assets.field_hud_horizontal_counters[index] = {
            field_hud_layers.horizontal_counters[index].width,
            field_hud_layers.horizontal_counters[index].height,
            field_hud_layers.horizontal_counters[index].pixels};
    }
    for (std::size_t ammo = 0;
         ammo < field_hud_layers.ammo_counters.size(); ++ammo) {
        assets.field_hud_ammo_counters[ammo] = {
            field_hud_layers.ammo_counters[ammo].width,
            field_hud_layers.ammo_counters[ammo].height,
            field_hud_layers.ammo_counters[ammo].pixels};
    }
    assets.entity_template = assets.protagonist_archetype_records[0];
    // New Game starts with Joshua only. The remaining runtime slots are
    // intentionally blank until recruited roster records are decoded from
    // the source's 0x72C1A roster table.
    assets.field_entity_records.fill({});
    assets.field_entity_records[0] = assets.protagonist_archetype_records[0];
    assets.field_menu_runner_names = {{"Joshua", "", ""}};
    for (std::size_t index = 1; index < assets.initial_field_records.size();
         ++index) {
        assets.initial_field_records[index].flags_10 = 0;
        assets.initial_field_records[index].flags_11 = 0;
    }
    // 0x2B34 writes only the three position pairs into the live field
    // records. Keep those coordinates and record types, but do not turn the
    // two empty party slots into switchable runners.
    for (std::size_t index = 0; index < assets.initial_field_records.size();
         ++index) {
        auto& record = assets.initial_field_records[index];
        auto& entity = assets.field_entity_records[index];
        record.record_type = static_cast<std::uint16_t>(index);
        entity.record_index = static_cast<std::uint8_t>(index);
        entity.world_x_fixed = record.position.world_x_fixed;
        entity.world_y_fixed = record.position.world_y_fixed;
    }
    assets.initial_field_records[0].flags_10 =
        assets.protagonist_archetype_records[0].flags_10;
    assets.initial_field_records[0].flags_11 =
        assets.protagonist_archetype_records[0].flags_11;
    assets.field_areas.front().initial_field_records =
        assets.initial_field_records;
    assets.cyberware_names = read_genesis_cyberware_catalog(rom);
    assets.attribute_skill_names = read_genesis_attribute_skill_catalog(rom);
    assets.group_item_names = read_genesis_group_item_catalog(rom);
    assets.cyberdeck_unavailable_lines =
        read_genesis_cyberdeck_unavailable_text(rom);
    assets.dismiss_runner_labels = read_genesis_dismiss_runner_labels(rom);
    assets.notebook_labels = read_genesis_notebook_labels(rom);
    assets.notebook_no_run_lines = read_genesis_notebook_no_run_text(rom);
    // 0x59C48 is the New Game initializer's move.l #$1C7F,FFFC2C. F33E
    // exposes Contacts when that source gate is nonzero; the other two
    // Notebook gates are event-driven and start clear in a fresh game.
    assets.notebook_event_gates[0] = read_rom_u32(
        rom, rom::kInitialNotebookContactsFlags,
        "Genesis Notebook initial event gate is truncated") != 0U;
    assets.notebook_event_gates[1] = 0;
    assets.notebook_event_gates[2] = 0;
    assets.notebook_tip_text = read_genesis_notebook_tip_catalog(rom);
    assets.notebook_mr_johnson_names =
        read_genesis_notebook_mr_johnson_names(rom);

    for (std::size_t index = 0;
         index < assets.field_direction_table.size(); ++index) {
        const auto table_offset = rom::kEntityMovementDirectionTable + index * 4U;
        assets.field_direction_table[index] = {
            static_cast<std::int16_t>(read_rom_u16(
                rom, table_offset,
                "Genesis entity movement direction table is truncated")),
            static_cast<std::int16_t>(read_rom_u16(
                rom, table_offset + 2U,
                "Genesis entity movement direction table is truncated")),
        };
    }
    return assets;
}

void write_native_asset_pack(const std::string& path,
                             const native::NativePortAssets& assets) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot create native asset pack: " + path);
    }
    output.write("SRNA", 4);
    write_u16(output, 53);
    write_u16(output, 0);
    write_native_image(output, assets.title_scene);
    for (const auto& title_frame : assets.title_animation_frames) {
        write_native_image(output, title_frame);
    }
    write_native_image(output, assets.title_background);
    for (const auto& light : assets.title_circuit_lights) {
        write_native_image(output, light);
    }
    write_native_image(output, assets.title_logo_plane);
    write_native_image(output, assets.title_text);
    for (const auto& title_sprite : assets.title_sprite_frames) {
        write_native_image(output, title_sprite);
    }
    write_native_image(output, assets.field_menu_background);
    write_native_image(output, assets.field_menu_action_background);
    write_native_image(output, assets.field_menu_attributes_background);
    write_native_image(output, assets.field_menu_inventory_boxes);
    write_native_image(output, assets.field_menu_font);
    write_native_image(output, assets.startup_font);
    write_native_image(output, assets.startup_cursor);
    for (const auto& portrait : assets.field_menu_runner_portraits) {
        write_native_image(output, portrait);
    }
    for (const auto& item : assets.field_menu_inventory_icons) {
        for (const auto& variant : item) {
            write_native_image(output, variant);
        }
    }
    for (const auto& name : assets.field_menu_runner_names) {
        write_pack_string(output, name,
                          "native runner menu name is too long");
    }
    for (const auto& status_bar : assets.field_menu_status_bars) {
        write_native_image(output, status_bar);
    }
    for (const auto& name : assets.field_menu_status_names) {
        write_pack_string(output, name,
                          "native menu status name is too long");
    }
    for (const auto& selected_cell : assets.field_menu_selected_cells) {
        write_native_image(output, selected_cell);
    }
    for (const auto& command_focus : assets.field_menu_command_focus) {
        write_native_image(output, command_focus);
    }
    for (const auto& action_focus : assets.field_menu_action_focus) {
        write_native_image(output, action_focus);
    }
    for (const auto& focus : assets.field_menu_clips_focus) {
        write_native_image(output, focus);
    }
    for (const auto& focus : assets.field_menu_clips_trade_focus) {
        write_native_image(output, focus);
    }
    write_native_image(output, assets.field_menu_posture_focus);
    write_native_image(output, assets.field_menu_cursor);
    for (const auto& item_art : assets.field_menu_item_art) {
        write_native_image(output, item_art);
    }
    for (const auto& bar : assets.field_menu_damage_bars) {
        write_native_image(output, bar);
    }
    for (const auto& bar : assets.field_menu_drain_bars) {
        write_native_image(output, bar);
    }
    for (const auto& value : assets.field_menu_detail_values) {
        write_native_image(output, value);
    }
    for (const auto& bar : assets.field_menu_attack_bars) {
        write_native_image(output, bar);
    }
    for (const auto& bar : assets.field_menu_defense_bars) {
        write_native_image(output, bar);
    }
    for (const auto& label : assets.field_menu_power_labels) {
        write_pack_string(output, label,
                          "native power label is too long");
    }
    for (const auto& armor : assets.field_menu_armor_labels) {
        for (const auto& label : armor) {
            write_pack_string(output, label,
                              "native armor label is too long");
        }
    }
    write_native_image(output, assets.field_map);
    if (assets.entity_frames.size() > 256) {
        throw std::invalid_argument("too many native entity frames");
    }
    write_u32(output, static_cast<std::uint32_t>(assets.entity_frames.size()));
    for (const auto& frame : assets.entity_frames) {
        write_native_image(output, {frame.width, frame.height, frame.pixels});
    }

    const auto& terrain = assets.field_terrain;
    if (terrain.regions.size() != terrain.region_count) {
        throw std::invalid_argument("native terrain region count is inconsistent");
    }
    write_u16(output, terrain.x_exclusive_upper);
    write_u16(output, terrain.y_exclusive_upper);
    write_u16(output, terrain.x_inclusive_lower);
    write_u16(output, terrain.y_inclusive_lower);
    write_u32(output, static_cast<std::uint32_t>(terrain.regions.size()));
    for (const auto& region : terrain.regions) {
        write_u16(output, region.x_start);
        write_u16(output, region.x_end);
        write_u16(output, region.y_start);
        write_u16(output, region.y_end);
        output.put(static_cast<char>(region.status));
    }
    if (terrain.statuses.size() > 0xFFFFFFFFU) {
        throw std::invalid_argument("native terrain status grid is too large");
    }
    write_u32(output, static_cast<std::uint32_t>(terrain.statuses.size()));
    output.write(reinterpret_cast<const char*>(terrain.statuses.data()),
                 static_cast<std::streamsize>(terrain.statuses.size()));

    write_u16(output, assets.initial_camera_x_fixed);
    write_u16(output, assets.initial_camera_y_fixed);
    for (const auto& record : assets.initial_field_records) {
        write_u16(output, record.position.world_x_fixed);
        write_u16(output, record.position.world_y_fixed);
        write_u16(output, record.record_type);
        output.put(static_cast<char>(record.flags_10));
        output.put(static_cast<char>(record.flags_11));
    }

    const auto& entity = assets.entity_template;
    write_u16(output, entity.projection_word_08);
    write_u16(output, entity.kind);
    write_u16(output, entity.record_type);
    output.put(static_cast<char>(entity.race_index));
    output.put(static_cast<char>(entity.archetype_index));
    output.put(static_cast<char>(entity.physical_value));
    output.put(static_cast<char>(entity.mental_value));
    output.put(static_cast<char>(entity.active_item_code));
    output.put(static_cast<char>(entity.flags_10));
    output.put(static_cast<char>(entity.flags_11));
    write_u32(output, entity.state_pointer);
    write_u16(output, entity.state_selector);
    write_u16(output, entity.animation_multiplier);
    write_u16(output, entity.animation_accumulator);
    write_u16(output, entity.animation_threshold);
    write_u16(output, entity.state_index);
    output.put(static_cast<char>(
        static_cast<std::uint8_t>(entity.display_state)));
    output.put(static_cast<char>(
        static_cast<std::uint8_t>(entity.display_substate)));
    output.put(static_cast<char>(entity.display_palette_state));
    output.put(static_cast<char>(entity.menu_status_index));
    output.put(static_cast<char>(entity.attack_bar_index));
    output.put(static_cast<char>(entity.defense_bar_index));
    output.put(static_cast<char>(entity.weapons_bar_index));
    output.put(static_cast<char>(entity.guns_bar_index));
    for (const auto index : entity.attack_bar_indices_by_posture) {
        output.put(static_cast<char>(index));
    }
    for (const auto index : entity.defense_bar_indices_by_posture) {
        output.put(static_cast<char>(index));
    }
    for (const auto code : entity.inventory_item_codes) {
        output.put(static_cast<char>(code));
    }
    for (const auto phase : entity.inventory_phase_bytes) {
        output.put(static_cast<char>(phase));
    }
    output.put(static_cast<char>(entity.equipped_armor_index));
    output.put(static_cast<char>(entity.ammo_rounds));
    output.put(static_cast<char>(entity.ammo_clips));
    write_u16(output, entity.cyberware_flags);
    for (const auto value : entity.attribute_values) {
        output.put(static_cast<char>(value));
    }
    for (const auto value : entity.skill_values) {
        output.put(static_cast<char>(value));
    }
    for (const auto value : entity.attribute_skill_auxiliary) {
        output.put(static_cast<char>(value));
    }
    write_u16(output, static_cast<std::uint16_t>(
                           assets.protagonist_archetype_records.size()));
    for (const auto& archetype : assets.protagonist_archetype_records) {
        write_native_entity_payload(output, archetype);
    }
    write_u16(output, static_cast<std::uint16_t>(
                           assets.field_entity_records.size()));
    for (const auto& record_entity : assets.field_entity_records) {
        write_u16(output, record_entity.projection_word_08);
        write_u16(output, record_entity.kind);
        write_u16(output, record_entity.record_type);
        output.put(static_cast<char>(record_entity.flags_10));
        output.put(static_cast<char>(record_entity.flags_11));
        output.put(static_cast<char>(record_entity.race_index));
        output.put(static_cast<char>(record_entity.archetype_index));
        output.put(static_cast<char>(record_entity.physical_value));
        output.put(static_cast<char>(record_entity.mental_value));
        output.put(static_cast<char>(record_entity.active_item_code));
        write_u32(output, record_entity.state_pointer);
        write_u16(output, record_entity.state_selector);
        write_u16(output, record_entity.animation_multiplier);
        write_u16(output, record_entity.animation_accumulator);
        write_u16(output, record_entity.animation_threshold);
        write_u16(output, record_entity.state_index);
        output.put(static_cast<char>(
            static_cast<std::uint8_t>(record_entity.display_state)));
        output.put(static_cast<char>(
            static_cast<std::uint8_t>(record_entity.display_substate)));
        output.put(static_cast<char>(record_entity.display_palette_state));
        output.put(static_cast<char>(record_entity.menu_status_index));
        output.put(static_cast<char>(record_entity.attack_bar_index));
        output.put(static_cast<char>(record_entity.defense_bar_index));
        output.put(static_cast<char>(record_entity.weapons_bar_index));
        output.put(static_cast<char>(record_entity.guns_bar_index));
        for (const auto index : record_entity.attack_bar_indices_by_posture) {
            output.put(static_cast<char>(index));
        }
        for (const auto index : record_entity.defense_bar_indices_by_posture) {
            output.put(static_cast<char>(index));
        }
        for (const auto code : record_entity.inventory_item_codes) {
            output.put(static_cast<char>(code));
        }
        for (const auto phase : record_entity.inventory_phase_bytes) {
            output.put(static_cast<char>(phase));
        }
        output.put(static_cast<char>(record_entity.equipped_armor_index));
        output.put(static_cast<char>(record_entity.ammo_rounds));
        output.put(static_cast<char>(record_entity.ammo_clips));
        write_u16(output, record_entity.cyberware_flags);
        for (const auto value : record_entity.attribute_values) {
            output.put(static_cast<char>(value));
        }
        for (const auto value : record_entity.skill_values) {
            output.put(static_cast<char>(value));
        }
        for (const auto value : record_entity.attribute_skill_auxiliary) {
            output.put(static_cast<char>(value));
        }
    }
    write_u16(output, static_cast<std::uint16_t>(assets.inventory_items.size()));
    for (const auto& item : assets.inventory_items) {
        output.put(static_cast<char>(item.code));
        output.put(static_cast<char>(item.source_combat_factor));
        output.put(static_cast<char>(item.damage_table_index));
        output.put(static_cast<char>(item.power_table_index));
        write_pack_string(output, item.name,
                          "native inventory item name is too long");
        write_pack_string(output, item.category,
                          "native inventory item category is too long");
    }
    write_u16(output, static_cast<std::uint16_t>(
                           assets.cyberware_names.size()));
    for (const auto& name : assets.cyberware_names) {
        write_pack_string(output, name,
                          "native cyberware name is too long");
    }
    write_u16(output, static_cast<std::uint16_t>(
                           assets.attribute_skill_names.size()));
    for (const auto& name : assets.attribute_skill_names) {
        write_pack_string(output, name,
                          "native attribute/skill name is too long");
    }
    write_u16(output, static_cast<std::uint16_t>(
                           assets.group_item_names.size()));
    for (const auto& name : assets.group_item_names) {
        write_pack_string(output, name,
                          "native group-item name is too long");
    }
    for (const auto& line : assets.cyberdeck_unavailable_lines) {
        write_pack_string(output, line,
                          "native cyberdeck-unavailable text is too long");
    }
    for (const auto& label : assets.dismiss_runner_labels) {
        write_pack_string(output, label,
                          "native dismiss-runner label is too long");
    }
    for (const auto& label : assets.notebook_labels) {
        write_pack_string(output, label,
                          "native Notebook label is too long");
    }
    for (const auto& line : assets.notebook_no_run_lines) {
        write_pack_string(output, line,
                          "native Notebook no-run text is too long");
    }
    for (const auto gate : assets.notebook_event_gates) {
        output.put(static_cast<char>(gate));
    }
    for (const auto& tip : assets.notebook_tip_text) {
        write_pack_string(output, tip,
                          "native Notebook tip text is too long");
    }
    for (const auto flags : assets.notebook_tip_flags) {
        write_u32(output, flags);
    }
    for (const auto& name : assets.notebook_mr_johnson_names) {
        write_pack_string(output, name,
                          "native Mr. Johnson name is too long");
    }
    for (const auto& direction : assets.field_direction_table) {
        write_u16(output, static_cast<std::uint16_t>(direction[0]));
        write_u16(output, static_cast<std::uint16_t>(direction[1]));
    }
    const auto& intro = assets.intro;
    write_pack_string(output, intro.start_options_title,
                      "native startup options title is too long");
    for (const auto& option : intro.start_options) {
        write_pack_string(output, option,
                          "native startup option is too long");
    }
    write_pack_string(output, intro.archetype_title,
                      "native archetype title is too long");
    for (const auto& archetype : intro.archetypes) {
        write_pack_string(output, archetype,
                          "native archetype label is too long");
    }
    if (intro.story_lines.size() > 0xFFFFU) {
        throw std::invalid_argument("too many native intro story lines");
    }
    write_u16(output, static_cast<std::uint16_t>(intro.story_lines.size()));
    for (const auto& line : intro.story_lines) {
        write_pack_string(output, line, "native intro story line is too long");
    }
    const auto write_startup_layout = [&output](
                                           const std::vector<native::NativeStartupTextLine>&
                                               layout) {
        if (layout.size() > 0xFFFFU) {
            throw std::invalid_argument("too many native startup text lines");
        }
        write_u16(output, static_cast<std::uint16_t>(layout.size()));
        for (const auto& line : layout) {
            output.put(static_cast<char>(line.row));
            write_pack_string(output, line.text,
                              "native startup text line is too long");
        }
    };
    if (assets.intro.start_options_layout.size() > 0xFFFFU) {
        throw std::invalid_argument("too many native startup text lines");
    }
    write_startup_layout(intro.start_options_layout);
    write_startup_layout(intro.save_load_layout);
    write_startup_layout(intro.archetype_layout);
    write_startup_layout(intro.no_save_layout);
    write_startup_layout(intro.save_success_layout);
    // Version 9 appends the native field-area catalog so older packs remain
    // readable while new packs can cross ROM-derived building transitions.
    for (const auto mode : assets.field_event_mode_table) {
        output.put(static_cast<char>(mode));
    }
    if (assets.field_areas.size() > 0xFFFFU) {
        throw std::invalid_argument("too many native field areas");
    }
    write_u16(output, static_cast<std::uint16_t>(assets.field_areas.size()));
    for (const auto& area : assets.field_areas) {
        write_u16(output, area.mode_id);
        write_native_image(output, area.map);
        const auto& terrain = area.terrain;
        if (terrain.regions.size() != terrain.region_count) {
            throw std::invalid_argument(
                "native field area terrain region count is inconsistent");
        }
        write_u16(output, terrain.x_exclusive_upper);
        write_u16(output, terrain.y_exclusive_upper);
        write_u16(output, terrain.x_inclusive_lower);
        write_u16(output, terrain.y_inclusive_lower);
        write_u32(output, static_cast<std::uint32_t>(terrain.regions.size()));
        for (const auto& region : terrain.regions) {
            write_u16(output, region.x_start);
            write_u16(output, region.x_end);
            write_u16(output, region.y_start);
            write_u16(output, region.y_end);
            output.put(static_cast<char>(region.status));
        }
        if (terrain.statuses.size() > 0xFFFFFFFFU) {
            throw std::invalid_argument(
                "native field area terrain grid is too large");
        }
        write_u32(output, static_cast<std::uint32_t>(terrain.statuses.size()));
        output.write(reinterpret_cast<const char*>(terrain.statuses.data()),
                     static_cast<std::streamsize>(terrain.statuses.size()));
        write_u16(output, area.initial_camera_x_fixed);
        write_u16(output, area.initial_camera_y_fixed);
        for (const auto& record : area.initial_field_records) {
            write_u16(output, record.position.world_x_fixed);
            write_u16(output, record.position.world_y_fixed);
            write_u16(output, record.record_type);
            output.put(static_cast<char>(record.flags_10));
            output.put(static_cast<char>(record.flags_11));
        }
        for (const auto mode : area.event_mode_table) {
            output.put(static_cast<char>(mode));
        }
    }
    for (const auto& animation : assets.field_entity_animations) {
        write_u16(output, animation.state_selector);
        write_u16(output, animation.state_marker);
        write_u16(output, animation.state_auxiliary_word);
        write_u16(output, animation.source_timing_value);
        write_u16(output, animation.timing_multiplier);
        if (animation.frames.size() > 256U) {
            throw std::invalid_argument(
                "native field animation has too many frames");
        }
        write_u32(output, static_cast<std::uint32_t>(animation.frames.size()));
        for (const auto& frame : animation.frames) {
            write_native_image(output, {frame.width, frame.height, frame.pixels});
        }
    }
    write_native_image(output, assets.boot_background);
    write_native_image(output, assets.boot_guy);
    for (const auto& frame : assets.boot_sega_frames) {
        write_native_image(output, frame);
    }
    write_native_image(output, assets.boot_static);
    for (const auto& frame : assets.boot_logo_strobe) {
        write_native_image(output, frame);
    }
    for (const auto& stage : assets.intro_stages) {
        write_native_image(output, stage);
    }
    for (const auto& frame : assets.intro_wilderness_pan_frames) {
        write_native_image(output, frame);
    }
    for (const auto& frame : assets.intro_ambush_frames) {
        write_native_image(output, frame);
    }
    write_native_image(output, assets.boot_fasa);
    write_native_image(output, assets.startup_background);
    for (const auto& heli : assets.intro_heli_frames) {
        write_native_image(output, heli);
    }
    write_native_image(output, assets.intro_aircraft_plane_a);
    for (const auto& movement : assets.intro_aircraft_movement_frames) {
        write_native_image(output, movement);
    }
    write_native_image(output, assets.intro_trivid_background);
    write_native_image(output, assets.intro_trivid_slain);
    write_native_image(output, assets.intro_trivid_moving);
    write_native_image(output, assets.field_hud);
    for (const auto& face : assets.field_hud_faces) {
        write_native_image(output, face);
    }
    for (const auto& item : assets.field_hud_item_blocks) {
        write_native_image(output, item);
    }
    for (const auto& item : assets.field_hud_unarmed_item_blocks) {
        write_native_image(output, item);
    }
    for (const auto& counter : assets.field_hud_vertical_counters) {
        write_native_image(output, counter);
    }
    for (const auto& counter : assets.field_hud_horizontal_counters) {
        write_native_image(output, counter);
    }
    for (const auto& counter : assets.field_hud_ammo_counters) {
        write_native_image(output, counter);
    }
    write_native_image(output, assets.pocket_secretary_background);
    write_native_image(output, assets.cab_map);
    if (!output) {
        throw std::runtime_error("failed writing native asset pack: " + path);
    }
}

native::NativePortAssets read_native_asset_pack(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open native asset pack: " + path);
    }
    char magic[4]{};
    input.read(magic, sizeof(magic));
    const auto version = read_pack_u16(
        input, "native asset pack version is truncated");
    if (!input || std::string(magic, sizeof(magic)) != "SRNA" ||
            (version < 2U || version > 53U)) {
        throw std::runtime_error("invalid native asset pack header");
    }
    (void)read_pack_u16(input, "native asset pack header is truncated");

    native::NativePortAssets assets;
    assets.title_scene = read_native_image(input);
    if (version >= 17U) {
        for (auto& title_frame : assets.title_animation_frames) {
            title_frame = read_native_image(input);
        }
    }
    if (version >= 18U) {
        assets.title_background = read_native_image(input);
        if (version >= 40U) {
            for (auto& light : assets.title_circuit_lights) {
                light = read_native_image(input);
            }
        }
        assets.title_logo_plane = read_native_image(input);
        assets.title_text = read_native_image(input);
        for (auto& title_sprite : assets.title_sprite_frames) {
            title_sprite = read_native_image(input);
        }
    }
    if (version >= 11U) {
        assets.field_menu_background = read_native_image(input);
        if (version >= 52U) {
            assets.field_menu_action_background = read_native_image(input);
        }
        if (version >= 48U) {
            assets.field_menu_attributes_background = read_native_image(input);
        }
        assets.field_menu_inventory_boxes = read_native_image(input);
        assets.field_menu_font = read_native_image(input);
        if (version >= 18U) {
            assets.startup_font = read_native_image(input);
            assets.startup_cursor = read_native_image(input);
        }
        if (version >= 12U) {
            for (auto& portrait : assets.field_menu_runner_portraits) {
                portrait = read_native_image(input);
            }
            for (auto& item : assets.field_menu_inventory_icons) {
                for (auto& variant : item) {
                    variant = read_native_image(input);
                }
            }
            for (auto& name : assets.field_menu_runner_names) {
                name = read_pack_string(
                    input, "native runner menu name is truncated");
            }
            if (version >= 13U) {
                for (auto& status_bar : assets.field_menu_status_bars) {
                    status_bar = read_native_image(input);
                }
                for (auto& name : assets.field_menu_status_names) {
                    name = read_pack_string(
                        input, "native menu status name is truncated");
                }
            }
            if (version >= 14U) {
                for (auto& selected_cell : assets.field_menu_selected_cells) {
                    selected_cell = read_native_image(input);
                }
                if (version >= 35U) {
                    for (auto& command_focus : assets.field_menu_command_focus) {
                        command_focus = read_native_image(input);
                    }
                }
                if (version >= 52U) {
                    for (auto& action_focus : assets.field_menu_action_focus) {
                        action_focus = read_native_image(input);
                    }
                }
                if (version >= 47U) {
                    for (auto& focus : assets.field_menu_clips_focus) {
                        focus = read_native_image(input);
                    }
                    for (auto& focus : assets.field_menu_clips_trade_focus) {
                        focus = read_native_image(input);
                    }
                    assets.field_menu_posture_focus = read_native_image(input);
                }
                if (version >= 15U) {
                    assets.field_menu_cursor = read_native_image(input);
                }
                for (auto& item_art : assets.field_menu_item_art) {
                    item_art = read_native_image(input);
                }
                if (version >= 16U) {
                    for (auto& bar : assets.field_menu_damage_bars) {
                        bar = read_native_image(input);
                    }
                    for (auto& bar : assets.field_menu_drain_bars) {
                        bar = read_native_image(input);
                    }
                    if (version >= 26U) {
                        for (auto& value : assets.field_menu_detail_values) {
                            value = read_native_image(input);
                        }
                    }
                    if (version >= 21U) {
                        for (auto& bar : assets.field_menu_attack_bars) {
                            bar = read_native_image(input);
                        }
                        for (auto& bar : assets.field_menu_defense_bars) {
                            bar = read_native_image(input);
                        }
                    }
                    for (auto& label : assets.field_menu_power_labels) {
                        label = read_pack_string(
                            input, "native power label is truncated");
                    }
                    if (version >= 20U) {
                        for (auto& armor : assets.field_menu_armor_labels) {
                            for (auto& label : armor) {
                                label = read_pack_string(
                                    input, "native armor label is truncated");
                            }
                        }
                    }
                }
            }
        }
    }
    assets.field_map = read_native_image(input);
    const auto entity_count = static_cast<std::size_t>(read_pack_u32(
        input, "native entity frame count is truncated"));
    if (entity_count > 256) {
        throw std::runtime_error("native asset pack has too many entity frames");
    }
    assets.entity_frames.reserve(entity_count);
    for (std::size_t index = 0; index < entity_count; ++index) {
        const auto image = read_native_image(input);
        assets.entity_frames.push_back(
            {image.width, image.height, image.pixels});
    }

    auto& terrain = assets.field_terrain;
    terrain.x_exclusive_upper = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.y_exclusive_upper = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.x_inclusive_lower = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.y_inclusive_lower = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.region_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain region count is truncated"));
    if (terrain.region_count > 65536) {
        throw std::runtime_error("native asset pack has too many terrain regions");
    }
    terrain.regions.resize(terrain.region_count);
    for (auto& region : terrain.regions) {
        region.x_start = read_pack_u16(input, "native terrain region is truncated");
        region.x_end = read_pack_u16(input, "native terrain region is truncated");
        region.y_start = read_pack_u16(input, "native terrain region is truncated");
        region.y_end = read_pack_u16(input, "native terrain region is truncated");
        region.status = read_pack_u8(input, "native terrain region is truncated");
    }
    const auto status_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain status count is truncated"));
    if (status_count != terrain::Map::kWidth * terrain::Map::kHeight) {
        throw std::runtime_error("native asset pack has an invalid terrain grid");
    }
    terrain.statuses.resize(status_count);
    read_pack_bytes(input, terrain.statuses.data(), status_count,
                    "native terrain status grid is truncated");

    assets.initial_camera_x_fixed = read_pack_u16(
        input, "native field camera is truncated");
    assets.initial_camera_y_fixed = read_pack_u16(
        input, "native field camera is truncated");
    for (auto& record : assets.initial_field_records) {
        record.position.world_x_fixed = read_pack_u16(
            input, "native field record position is truncated");
        record.position.world_y_fixed = read_pack_u16(
            input, "native field record position is truncated");
        if (version >= 7U) {
            record.record_type = read_pack_u16(
                input, "native field record kind is truncated");
            record.flags_10 = read_pack_u8(
                input, "native field record flags are truncated");
            record.flags_11 = read_pack_u8(
                input, "native field record flags are truncated");
        }
    }

    auto& entity = assets.entity_template;
    entity.projection_word_08 = read_pack_u16(
        input, "native entity template is truncated");
    entity.kind = read_pack_u16(input, "native entity template is truncated");
    entity.record_type = version >= 8U
        ? read_pack_u16(input, "native entity record type is truncated")
        : 0;
    if (version >= 18U) {
        entity.race_index = read_pack_u8(
            input, "native entity race index is truncated");
        entity.archetype_index = read_pack_u8(
            input, "native entity archetype index is truncated");
        entity.physical_value = read_pack_u8(
            input, "native entity physical value is truncated");
        entity.mental_value = read_pack_u8(
            input, "native entity mental value is truncated");
        if (version >= 19U) {
            entity.active_item_code = read_pack_u8(
                input, "native entity active item is truncated");
        } else {
            entity.active_item_code = static_cast<std::uint8_t>(
                entity.display_substate);
        }
    }
    entity.flags_10 = read_pack_u8(input, "native entity template is truncated");
    entity.flags_11 = read_pack_u8(input, "native entity template is truncated");
    entity.state_pointer = read_pack_u32(
        input, "native entity template is truncated");
    entity.state_selector = read_pack_u16(
        input, "native entity template is truncated");
    entity.animation_multiplier = read_pack_u16(
        input, "native entity template is truncated");
    entity.animation_accumulator = read_pack_u16(
        input, "native entity template is truncated");
    entity.animation_threshold = read_pack_u16(
        input, "native entity template is truncated");
    entity.state_index = read_pack_u16(
        input, "native entity template is truncated");
    entity.display_state = static_cast<std::int8_t>(read_pack_u8(
        input, "native entity display state is truncated"));
    entity.display_substate = static_cast<std::int8_t>(read_pack_u8(
        input, "native entity display substate is truncated"));
    if (version >= 12U) {
        entity.display_palette_state = read_pack_u8(
            input, "native entity display palette state is truncated");
    }
    if (version >= 13U) {
        entity.menu_status_index = read_pack_u8(
            input, "native entity menu status index is truncated");
    }
    if (version >= 21U) {
        entity.attack_bar_index = read_pack_u8(
            input, "native entity attack bar index is truncated");
        entity.defense_bar_index = read_pack_u8(
            input, "native entity defense bar index is truncated");
        if (version >= 22U) {
            entity.weapons_bar_index = read_pack_u8(
                input, "native entity weapons bar index is truncated");
            entity.guns_bar_index = read_pack_u8(
                input, "native entity guns bar index is truncated");
        }
        if (version >= 24U) {
            for (auto& index : entity.attack_bar_indices_by_posture) {
                index = read_pack_u8(
                    input, "native entity posture attack bars are truncated");
            }
            for (auto& index : entity.defense_bar_indices_by_posture) {
                index = read_pack_u8(
                    input, "native entity posture defense bars are truncated");
            }
        } else {
            entity.attack_bar_indices_by_posture.fill(entity.attack_bar_index);
            entity.defense_bar_indices_by_posture.fill(entity.defense_bar_index);
        }
    }
    if (version < 7U) {
        for (auto& record : assets.initial_field_records) {
            record.record_type = entity.record_type;
            record.flags_10 = entity.flags_10;
            record.flags_11 = entity.flags_11;
        }
    }
    if (version >= 3U) {
        for (auto& code : entity.inventory_item_codes) {
            code = read_pack_u8(input, "native inventory state is truncated");
        }
        for (auto& phase : entity.inventory_phase_bytes) {
            phase = read_pack_u8(input, "native inventory state is truncated");
        }
        entity.equipped_armor_index = read_pack_u8(
            input, "native inventory state is truncated");
        if (version >= 4U) {
            entity.ammo_rounds = read_pack_u8(
                input, "native ammunition state is truncated");
            entity.ammo_clips = read_pack_u8(
                input, "native ammunition state is truncated");
        }
        if (version >= 5U) {
            entity.cyberware_flags = read_pack_u16(
                input, "native cyberware state is truncated");
        }
        if (version >= 6U) {
            for (auto& value : entity.attribute_values) {
                value = read_pack_u8(
                    input, "native attribute state is truncated");
            }
            for (auto& value : entity.skill_values) {
                value = read_pack_u8(
                    input, "native skill state is truncated");
            }
        for (auto& value : entity.attribute_skill_auxiliary) {
            value = read_pack_u8(
                    input, "native attribute/skill auxiliary state is truncated");
        }
        if (version >= 23U) {
            const auto archetype_count = static_cast<std::size_t>(
                read_pack_u16(input, "native archetype record count is truncated"));
            if (archetype_count != assets.protagonist_archetype_records.size()) {
                throw std::runtime_error(
                    "native asset pack has an invalid archetype record count");
            }
            for (auto& archetype : assets.protagonist_archetype_records) {
                archetype = read_native_entity_payload(input, version >= 24U);
            }
        }
        if (version >= 18U) {
            const auto record_count = static_cast<std::size_t>(read_pack_u16(
                input, "native runner record count is truncated"));
            if (record_count != assets.field_entity_records.size()) {
                throw std::runtime_error(
                    "native asset pack has an invalid runner record count");
            }
            for (auto& record_entity : assets.field_entity_records) {
                record_entity.projection_word_08 = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.kind = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.record_type = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.flags_10 = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.flags_11 = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.race_index = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.archetype_index = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.physical_value = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.mental_value = read_pack_u8(
                    input, "native runner record is truncated");
                if (version >= 19U) {
                    record_entity.active_item_code = read_pack_u8(
                        input, "native runner record is truncated");
                } else {
                    record_entity.active_item_code = static_cast<std::uint8_t>(
                        record_entity.display_substate);
                }
                record_entity.state_pointer = read_pack_u32(
                    input, "native runner record is truncated");
                record_entity.state_selector = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.animation_multiplier = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.animation_accumulator = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.animation_threshold = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.state_index = read_pack_u16(
                    input, "native runner record is truncated");
                record_entity.display_state = static_cast<std::int8_t>(
                    read_pack_u8(input, "native runner record is truncated"));
                record_entity.display_substate = static_cast<std::int8_t>(
                    read_pack_u8(input, "native runner record is truncated"));
                record_entity.display_palette_state = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.menu_status_index = read_pack_u8(
                    input, "native runner record is truncated");
                if (version >= 21U) {
                    record_entity.attack_bar_index = read_pack_u8(
                        input, "native runner attack bar index is truncated");
                    record_entity.defense_bar_index = read_pack_u8(
                        input, "native runner defense bar index is truncated");
                    if (version >= 22U) {
                        record_entity.weapons_bar_index = read_pack_u8(
                            input, "native runner weapons bar index is truncated");
                        record_entity.guns_bar_index = read_pack_u8(
                            input, "native runner guns bar index is truncated");
                    }
                    if (version >= 24U) {
                        for (auto& index :
                             record_entity.attack_bar_indices_by_posture) {
                            index = read_pack_u8(
                                input,
                                "native runner posture attack bars are truncated");
                        }
                        for (auto& index :
                             record_entity.defense_bar_indices_by_posture) {
                            index = read_pack_u8(
                                input,
                                "native runner posture defense bars are truncated");
                        }
                    } else {
                        record_entity.attack_bar_indices_by_posture.fill(
                            record_entity.attack_bar_index);
                        record_entity.defense_bar_indices_by_posture.fill(
                            record_entity.defense_bar_index);
                    }
                }
                for (auto& code : record_entity.inventory_item_codes) {
                    code = read_pack_u8(input, "native runner record is truncated");
                }
                for (auto& phase : record_entity.inventory_phase_bytes) {
                    phase = read_pack_u8(input, "native runner record is truncated");
                }
                record_entity.equipped_armor_index = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.ammo_rounds = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.ammo_clips = read_pack_u8(
                    input, "native runner record is truncated");
                record_entity.cyberware_flags = read_pack_u16(
                    input, "native runner record is truncated");
                for (auto& value : record_entity.attribute_values) {
                    value = read_pack_u8(input, "native runner record is truncated");
                }
                for (auto& value : record_entity.skill_values) {
                    value = read_pack_u8(input, "native runner record is truncated");
                }
                for (auto& value : record_entity.attribute_skill_auxiliary) {
                    value = read_pack_u8(input, "native runner record is truncated");
                }
            }
        }
        }
        if (version < 23U) {
            // Version 22 and earlier called the three Joshua archetype
            // templates "runner records". Preserve those packs by promoting
            // them into the new template array, then make only Joshua active.
            assets.protagonist_archetype_records = assets.field_entity_records;
            const auto joshua = assets.field_entity_records[0];
            assets.field_entity_records.fill({});
            assets.field_entity_records[0] = joshua;
            assets.field_menu_runner_names[1].clear();
            assets.field_menu_runner_names[2].clear();
            assets.initial_field_records[1].flags_10 = 0;
            assets.initial_field_records[1].flags_11 = 0;
            assets.initial_field_records[2].flags_10 = 0;
            assets.initial_field_records[2].flags_11 = 0;
        }
        const auto item_count = static_cast<std::size_t>(read_pack_u16(
            input, "native inventory catalog count is truncated"));
        if (item_count > assets.inventory_items.size()) {
            throw std::runtime_error("native asset pack has too many inventory items");
        }
        for (std::size_t index = 0; index < item_count; ++index) {
            auto& item = assets.inventory_items[index];
            item.code = read_pack_u8(
                input, "native inventory item code is truncated");
            if (version >= 25U) {
                item.source_combat_factor = read_pack_u8(
                    input, "native inventory source factor is truncated");
            }
            if (version >= 16U) {
                item.damage_table_index = read_pack_u8(
                    input, "native inventory damage index is truncated");
                item.power_table_index = read_pack_u8(
                    input, "native inventory power index is truncated");
            }
            item.name = read_pack_string(
                input, "native inventory item name is truncated");
            item.category = read_pack_string(
                input, "native inventory item category is truncated");
        }
        if (version >= 5U) {
            const auto cyberware_count = static_cast<std::size_t>(
                read_pack_u16(input, "native cyberware catalog count is truncated"));
            if (cyberware_count > assets.cyberware_names.size()) {
                throw std::runtime_error(
                    "native asset pack has too many cyberware names");
            }
            for (std::size_t index = 0; index < cyberware_count; ++index) {
                assets.cyberware_names[index] = read_pack_string(
                    input, "native cyberware name is truncated");
            }
            if (version >= 6U) {
                const auto attribute_skill_count = static_cast<std::size_t>(
                    read_pack_u16(input,
                                  "native attribute/skill catalog count is truncated"));
                if (attribute_skill_count > assets.attribute_skill_names.size()) {
                    throw std::runtime_error(
                        "native asset pack has too many attribute/skill names");
                }
                for (std::size_t index = 0;
                     index < attribute_skill_count; ++index) {
                    assets.attribute_skill_names[index] = read_pack_string(
                        input, "native attribute/skill name is truncated");
                }
            }
            if (version >= 27U) {
                const auto group_item_count = static_cast<std::size_t>(
                    read_pack_u16(input,
                                  "native group-item catalog count is truncated"));
                if (group_item_count > assets.group_item_names.size()) {
                    throw std::runtime_error(
                        "native asset pack has too many group-item names");
                }
                for (std::size_t index = 0; index < group_item_count; ++index) {
                    assets.group_item_names[index] = read_pack_string(
                        input, "native group-item name is truncated");
                }
                if (version >= 28U) {
                    for (auto& line : assets.cyberdeck_unavailable_lines) {
                        line = read_pack_string(
                            input,
                            "native cyberdeck-unavailable text is truncated");
                    }
                    if (version >= 29U) {
                        for (auto& label : assets.dismiss_runner_labels) {
                            label = read_pack_string(
                                input,
                                "native dismiss-runner label is truncated");
                        }
                        if (version >= 30U) {
                            for (auto& label : assets.notebook_labels) {
                                label = read_pack_string(
                                    input,
                                    "native Notebook label is truncated");
                            }
                            for (auto& line : assets.notebook_no_run_lines) {
                                line = read_pack_string(
                                    input,
                                    "native Notebook no-run text is truncated");
                            }
                            if (version >= 31U) {
                                for (auto& gate : assets.notebook_event_gates) {
                                    gate = read_pack_u8(
                                        input,
                                        "native Notebook event gates are truncated");
                                }
                                if (version >= 32U) {
                                    for (auto& tip : assets.notebook_tip_text) {
                                        tip = read_pack_string(
                                            input,
                                            "native Notebook tip text is truncated");
                                    }
                                    if (version >= 33U) {
                                        for (auto& flags :
                                             assets.notebook_tip_flags) {
                                            flags = read_pack_u32(
                                                input,
                                                "native Notebook tip flags are truncated");
                                        }
                                        if (version >= 34U) {
                                            for (auto& name :
                                                 assets.notebook_mr_johnson_names) {
                                                name = read_pack_string(
                                                    input,
                                                    "native Mr. Johnson name is truncated");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    for (auto& direction : assets.field_direction_table) {
        direction[0] = static_cast<std::int16_t>(read_pack_u16(
            input, "native direction table is truncated"));
        direction[1] = static_cast<std::int16_t>(read_pack_u16(
            input, "native direction table is truncated"));
    }
    if (version >= 8U) {
        auto& intro = assets.intro;
        intro.start_options_title = read_pack_string(
            input, "native startup options title is truncated");
        for (auto& option : intro.start_options) {
            option = read_pack_string(input,
                                      "native startup option is truncated");
        }
        intro.archetype_title = read_pack_string(
            input, "native archetype title is truncated");
        for (auto& archetype : intro.archetypes) {
            archetype = read_pack_string(
                input, "native archetype label is truncated");
        }
        const auto story_count = static_cast<std::size_t>(read_pack_u16(
            input, "native intro story count is truncated"));
        if (story_count > 1024U) {
            throw std::runtime_error("native asset pack has too many intro lines");
        }
        intro.story_lines.reserve(story_count);
        for (std::size_t index = 0; index < story_count; ++index) {
            intro.story_lines.push_back(read_pack_string(
                input, "native intro story line is truncated"));
        }
        if (version >= 18U) {
            const auto read_startup_layout = [&input]() {
                const auto count = static_cast<std::size_t>(read_pack_u16(
                    input, "native startup text line count is truncated"));
                if (count > 256U) {
                    throw std::runtime_error(
                        "native asset pack has too many startup text lines");
                }
                std::vector<native::NativeStartupTextLine> layout;
                layout.reserve(count);
                for (std::size_t index = 0; index < count; ++index) {
                    native::NativeStartupTextLine line;
                    line.row = read_pack_u8(
                        input, "native startup text line is truncated");
                    line.text = read_pack_string(
                        input, "native startup text line is truncated");
                    layout.push_back(std::move(line));
                }
                return layout;
            };
            intro.start_options_layout = read_startup_layout();
            intro.save_load_layout = read_startup_layout();
            intro.archetype_layout = read_startup_layout();
            intro.no_save_layout = read_startup_layout();
            intro.save_success_layout = read_startup_layout();
        }
    }
    if (version >= 9U) {
        for (auto& mode : assets.field_event_mode_table) {
            mode = read_pack_u8(input, "native field event table is truncated");
        }
        const auto area_count = static_cast<std::size_t>(read_pack_u16(
            input, "native field area count is truncated"));
        if (area_count > 56U) {
            throw std::runtime_error("native asset pack has too many field areas");
        }
        assets.field_areas.reserve(area_count);
        for (std::size_t index = 0; index < area_count; ++index) {
            native::NativeFieldArea area;
            area.mode_id = read_pack_u16(
                input, "native field area mode is truncated");
            area.map = read_native_image(input);
            auto& terrain = area.terrain;
            terrain.x_exclusive_upper = read_pack_u16(
                input, "native field area terrain bounds are truncated");
            terrain.y_exclusive_upper = read_pack_u16(
                input, "native field area terrain bounds are truncated");
            terrain.x_inclusive_lower = read_pack_u16(
                input, "native field area terrain bounds are truncated");
            terrain.y_inclusive_lower = read_pack_u16(
                input, "native field area terrain bounds are truncated");
            terrain.region_count = static_cast<std::size_t>(read_pack_u32(
                input, "native field area terrain region count is truncated"));
            if (terrain.region_count > 65536U) {
                throw std::runtime_error(
                    "native field area has too many terrain regions");
            }
            terrain.regions.resize(terrain.region_count);
            for (auto& region : terrain.regions) {
                region.x_start = read_pack_u16(
                    input, "native field area terrain region is truncated");
                region.x_end = read_pack_u16(
                    input, "native field area terrain region is truncated");
                region.y_start = read_pack_u16(
                    input, "native field area terrain region is truncated");
                region.y_end = read_pack_u16(
                    input, "native field area terrain region is truncated");
                region.status = read_pack_u8(
                    input, "native field area terrain region is truncated");
            }
            const auto status_count = static_cast<std::size_t>(read_pack_u32(
                input, "native field area terrain grid is truncated"));
            if (status_count != terrain::Map::kWidth * terrain::Map::kHeight) {
                throw std::runtime_error(
                    "native field area has an invalid terrain grid");
            }
            terrain.statuses.resize(status_count);
            read_pack_bytes(input, terrain.statuses.data(), status_count,
                            "native field area terrain grid is truncated");
            area.initial_camera_x_fixed = read_pack_u16(
                input, "native field area camera is truncated");
            area.initial_camera_y_fixed = read_pack_u16(
                input, "native field area camera is truncated");
            for (auto& record : area.initial_field_records) {
                record.position.world_x_fixed = read_pack_u16(
                    input, "native field area record position is truncated");
                record.position.world_y_fixed = read_pack_u16(
                    input, "native field area record position is truncated");
                record.record_type = read_pack_u16(
                    input, "native field area record type is truncated");
                record.flags_10 = read_pack_u8(
                    input, "native field area record flags are truncated");
                record.flags_11 = read_pack_u8(
                    input, "native field area record flags are truncated");
            }
            for (auto& mode : area.event_mode_table) {
                mode = read_pack_u8(
                    input, "native field area event table is truncated");
            }
            assets.field_areas.push_back(std::move(area));
        }
    }
    if (version >= 10U) {
        for (auto& animation : assets.field_entity_animations) {
            animation.state_selector = read_pack_u16(
                input, "native field animation selector is truncated");
            animation.state_marker = read_pack_u16(
                input, "native field animation marker is truncated");
            animation.state_auxiliary_word = read_pack_u16(
                input, "native field animation auxiliary word is truncated");
            animation.source_timing_value = read_pack_u16(
                input, "native field animation timing value is truncated");
            animation.timing_multiplier = read_pack_u16(
                input, "native field animation multiplier is truncated");
            const auto frame_count = static_cast<std::size_t>(read_pack_u32(
                input, "native field animation frame count is truncated"));
            if (frame_count > 256U) {
                throw std::runtime_error(
                    "native field animation has too many frames");
            }
            animation.frames.reserve(frame_count);
            for (std::size_t frame_index = 0; frame_index < frame_count;
                 ++frame_index) {
                const auto image = read_native_image(input);
                animation.frames.push_back(
                    {image.width, image.height, image.pixels});
            }
        }
    }
    if (version >= 36U) {
        assets.boot_background = read_native_image(input);
        assets.boot_guy = read_native_image(input);
        for (auto& frame : assets.boot_sega_frames) {
            frame = read_native_image(input);
        }
    }
    if (version >= 37U) {
        assets.boot_static = read_native_image(input);
        for (auto& frame : assets.boot_logo_strobe) {
            frame = read_native_image(input);
        }
    }
    if (version >= 38U) {
        for (auto& stage : assets.intro_stages) {
            stage = read_native_image(input);
        }
    }
    if (version >= 46U) {
        for (auto& frame : assets.intro_wilderness_pan_frames) {
            frame = read_native_image(input);
        }
    }
    if (version >= 45U) {
        for (auto& frame : assets.intro_ambush_frames) {
            frame = read_native_image(input);
        }
    }
    if (version >= 39U) {
        assets.boot_fasa = read_native_image(input);
        assets.startup_background = read_native_image(input);
    }
    if (version >= 40U) {
        for (auto& heli : assets.intro_heli_frames) {
            heli = read_native_image(input);
        }
    }
    if (version >= 49U) {
        assets.intro_aircraft_plane_a = read_native_image(input);
        for (auto& movement : assets.intro_aircraft_movement_frames) {
            movement = read_native_image(input);
        }
    }
    if (version >= 50U) {
        assets.intro_trivid_background = read_native_image(input);
        assets.intro_trivid_slain = read_native_image(input);
        assets.intro_trivid_moving = read_native_image(input);
    }
    if (version >= 41U) {
        assets.field_hud = read_native_image(input);
    }
    if (version >= 43U) {
        for (auto& face : assets.field_hud_faces) {
            face = read_native_image(input);
        }
        for (auto& item : assets.field_hud_item_blocks) {
            item = read_native_image(input);
        }
        if (version >= 53U) {
            for (auto& item : assets.field_hud_unarmed_item_blocks) {
                item = read_native_image(input);
            }
        }
        for (auto& counter : assets.field_hud_vertical_counters) {
            counter = read_native_image(input);
        }
        for (auto& counter : assets.field_hud_horizontal_counters) {
            counter = read_native_image(input);
        }
        if (version >= 51U) {
            for (auto& counter : assets.field_hud_ammo_counters) {
                counter = read_native_image(input);
            }
        }
    }
    if (version >= 42U) {
        assets.pocket_secretary_background = read_native_image(input);
    }
    if (version >= 44U) {
        assets.cab_map = read_native_image(input);
    }
    fill_grayscale_entity_assets(assets);
    return assets;
}

// Direct-directory state helpers are defined below the legacy pack reader so
// the old serializer remains easy to compare with its historical format.
void write_native_field_record_state(std::ostream& output,
                                     const native::NativeFieldRecord& record);
native::NativeFieldRecord read_native_field_record_state(std::istream& input);
void write_native_field_entity_state(
    std::ostream& output, const native::NativeFieldEntity& entity);
native::NativeFieldEntity read_native_field_entity_state(std::istream& input);
void write_native_terrain_state(std::ostream& output,
                                const terrain::Map& map);
terrain::Map read_native_terrain_state(std::istream& input);
void write_native_intro_state(std::ostream& output,
                              const native::NativeIntroData& intro);
native::NativeIntroData read_native_intro_state(std::istream& input);

void write_native_asset_state(const std::string& path,
                              const native::NativePortAssets& assets) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot create native asset state: " + path);
    }
    output.write("SRNS", 4);
    write_u16(output, 2);
    write_u16(output, 0);

    write_native_terrain_state(output, assets.field_terrain);
    write_u16(output, assets.initial_camera_x_fixed);
    write_u16(output, assets.initial_camera_y_fixed);
    for (const auto& record : assets.initial_field_records) {
        write_native_field_record_state(output, record);
    }
    write_native_field_entity_state(output, assets.entity_template);
    for (const auto& entity : assets.protagonist_archetype_records) {
        write_native_field_entity_state(output, entity);
    }
    for (const auto& entity : assets.field_entity_records) {
        write_native_field_entity_state(output, entity);
    }
    for (const auto& item : assets.inventory_items) {
        output.put(static_cast<char>(item.code));
        output.put(static_cast<char>(item.source_combat_factor));
        output.put(static_cast<char>(item.damage_table_index));
        output.put(static_cast<char>(item.power_table_index));
        write_pack_string(output, item.name,
                          "native state inventory item name is too long");
        write_pack_string(output, item.category,
                          "native state inventory item category is too long");
    }
    for (const auto& value : assets.field_menu_runner_names) {
        write_pack_string(output, value,
                          "native state runner menu name is too long");
    }
    for (const auto& value : assets.field_menu_status_names) {
        write_pack_string(output, value,
                          "native state menu status name is too long");
    }
    for (const auto& value : assets.field_menu_power_labels) {
        write_pack_string(output, value,
                          "native state power label is too long");
    }
    for (const auto& armor : assets.field_menu_armor_labels) {
        for (const auto& value : armor) {
            write_pack_string(output, value,
                              "native state armor label is too long");
        }
    }
    for (const auto& value : assets.cyberware_names) {
        write_pack_string(output, value,
                          "native state cyberware name is too long");
    }
    for (const auto& value : assets.attribute_skill_names) {
        write_pack_string(output, value,
                          "native state attribute/skill name is too long");
    }
    for (const auto& value : assets.group_item_names) {
        write_pack_string(output, value,
                          "native state group item name is too long");
    }
    for (const auto& value : assets.cyberdeck_unavailable_lines) {
        write_pack_string(output, value,
                          "native state cyberdeck text is too long");
    }
    for (const auto& value : assets.dismiss_runner_labels) {
        write_pack_string(output, value,
                          "native state dismiss runner label is too long");
    }
    for (const auto& value : assets.notebook_labels) {
        write_pack_string(output, value,
                          "native state Notebook label is too long");
    }
    for (const auto& value : assets.notebook_no_run_lines) {
        write_pack_string(output, value,
                          "native state Notebook line is too long");
    }
    for (const auto value : assets.notebook_event_gates) {
        output.put(static_cast<char>(value));
    }
    for (const auto& value : assets.notebook_tip_text) {
        write_pack_string(output, value,
                          "native state Notebook tip is too long");
    }
    for (const auto value : assets.notebook_tip_flags) {
        write_u32(output, value);
    }
    for (const auto& value : assets.notebook_mr_johnson_names) {
        write_pack_string(output, value,
                          "native state Mr. Johnson name is too long");
    }
    for (const auto direction : assets.field_direction_table) {
        write_u16(output, static_cast<std::uint16_t>(direction[0]));
        write_u16(output, static_cast<std::uint16_t>(direction[1]));
    }
    write_native_intro_state(output, assets.intro);
    for (const auto mode : assets.field_event_mode_table) {
        output.put(static_cast<char>(mode));
    }
    if (assets.field_areas.size() > 0xFFFFU) {
        throw std::invalid_argument("native state has too many field areas");
    }
    write_u16(output, static_cast<std::uint16_t>(assets.field_areas.size()));
    for (const auto& area : assets.field_areas) {
        write_u16(output, area.mode_id);
        write_native_terrain_state(output, area.terrain);
        write_u16(output, area.initial_camera_x_fixed);
        write_u16(output, area.initial_camera_y_fixed);
        for (const auto& record : area.initial_field_records) {
            write_native_field_record_state(output, record);
        }
        for (const auto mode : area.event_mode_table) {
            output.put(static_cast<char>(mode));
        }
    }
    for (const auto& animation : assets.field_entity_animations) {
        write_u16(output, animation.state_selector);
        write_u16(output, animation.state_marker);
        write_u16(output, animation.state_auxiliary_word);
        write_u16(output, animation.source_timing_value);
        write_u16(output, animation.timing_multiplier);
        if (animation.frames.size() > 0xFFFFFFFFU) {
            throw std::invalid_argument(
                "native state has too many entity animation frames");
        }
        write_u32(output, static_cast<std::uint32_t>(animation.frames.size()));
    }
    if (assets.entity_frames.size() > 0xFFFFFFFFU) {
        throw std::invalid_argument("native state has too many entity frames");
    }
    write_u32(output, static_cast<std::uint32_t>(assets.entity_frames.size()));
    if (!output) {
        throw std::runtime_error("failed writing native asset state: " + path);
    }
}

native::NativePortAssets read_native_asset_directory(const std::string& root) {
    const std::filesystem::path asset_root(root);
    const auto state_path = asset_root / "native" / "state.bin";
    std::ifstream input(state_path, std::ios::binary);
    if (!input) {
        throw std::runtime_error(
            "missing direct asset state: " + state_path.string() +
            "; run shadowrun_graphics_extract --all-assets");
    }
    char magic[4]{};
    input.read(magic, sizeof(magic));
    const auto version = read_pack_u16(
        input, "native asset state version is truncated");
    if (!input || std::string(magic, sizeof(magic)) != "SRNS" || version != 2U) {
        throw std::runtime_error("invalid direct native asset state header");
    }
    (void)read_pack_u16(input, "native asset state header is truncated");

    native::NativePortAssets assets;
    assets.field_terrain = read_native_terrain_state(input);
    assets.initial_camera_x_fixed = read_pack_u16(
        input, "native asset state camera is truncated");
    assets.initial_camera_y_fixed = read_pack_u16(
        input, "native asset state camera is truncated");
    for (auto& record : assets.initial_field_records) {
        record = read_native_field_record_state(input);
    }
    assets.entity_template = read_native_field_entity_state(input);
    for (auto& entity : assets.protagonist_archetype_records) {
        entity = read_native_field_entity_state(input);
    }
    for (auto& entity : assets.field_entity_records) {
        entity = read_native_field_entity_state(input);
    }
    for (auto& item : assets.inventory_items) {
        item.code = read_pack_u8(input, "native asset state item is truncated");
        item.source_combat_factor = read_pack_u8(
            input, "native asset state item is truncated");
        item.damage_table_index = read_pack_u8(
            input, "native asset state item is truncated");
        item.power_table_index = read_pack_u8(
            input, "native asset state item is truncated");
        item.name = read_pack_string(input,
                                     "native asset state item is truncated");
        item.category = read_pack_string(input,
                                         "native asset state item is truncated");
    }
    for (auto& value : assets.field_menu_runner_names) {
        value = read_pack_string(input,
                                 "native asset state runner name is truncated");
    }
    for (auto& value : assets.field_menu_status_names) {
        value = read_pack_string(
            input, "native asset state status name is truncated");
    }
    for (auto& value : assets.field_menu_power_labels) {
        value = read_pack_string(input,
                                 "native asset state power label is truncated");
    }
    for (auto& armor : assets.field_menu_armor_labels) {
        for (auto& value : armor) {
            value = read_pack_string(
                input, "native asset state armor label is truncated");
        }
    }
    for (auto& value : assets.cyberware_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.attribute_skill_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.group_item_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.cyberdeck_unavailable_lines) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.dismiss_runner_labels) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_labels) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_no_run_lines) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_event_gates) {
        value = read_pack_u8(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_tip_text) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_tip_flags) {
        value = read_pack_u32(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_mr_johnson_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& direction : assets.field_direction_table) {
        direction[0] = static_cast<std::int16_t>(read_pack_u16(
            input, "native asset state direction table is truncated"));
        direction[1] = static_cast<std::int16_t>(read_pack_u16(
            input, "native asset state direction table is truncated"));
    }
    assets.intro = read_native_intro_state(input);
    for (auto& mode : assets.field_event_mode_table) {
        mode = read_pack_u8(input, "native asset state event table is truncated");
    }
    const auto area_count = static_cast<std::size_t>(read_pack_u16(
        input, "native asset state area count is truncated"));
    if (area_count > 56U) {
        throw std::runtime_error("native asset state has too many field areas");
    }
    assets.field_areas.reserve(area_count);
    for (std::size_t index = 0; index < area_count; ++index) {
        native::NativeFieldArea area;
        area.mode_id = read_pack_u16(
            input, "native asset state area mode is truncated");
        area.terrain = read_native_terrain_state(input);
        area.initial_camera_x_fixed = read_pack_u16(
            input, "native asset state area camera is truncated");
        area.initial_camera_y_fixed = read_pack_u16(
            input, "native asset state area camera is truncated");
        for (auto& record : area.initial_field_records) {
            record = read_native_field_record_state(input);
        }
        for (auto& mode : area.event_mode_table) {
            mode = read_pack_u8(input, "native asset state area event table is truncated");
        }
        assets.field_areas.push_back(std::move(area));
    }
    for (auto& animation : assets.field_entity_animations) {
        animation.state_selector = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.state_marker = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.state_auxiliary_word = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.source_timing_value = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.timing_multiplier = read_pack_u16(
            input, "native asset state animation is truncated");
        const auto frame_count = static_cast<std::size_t>(read_pack_u32(
            input, "native asset state animation frame count is truncated"));
        if (frame_count > 256U) {
            throw std::runtime_error(
                "native asset state has too many animation frames");
        }
        animation.frames.resize(frame_count);
    }
    const auto entity_count = static_cast<std::size_t>(read_pack_u32(
        input, "native asset state entity frame count is truncated"));
    if (entity_count > 256U) {
        throw std::runtime_error("native asset state has too many entity frames");
    }
    assets.entity_frames.resize(entity_count);

    std::unordered_map<std::string, std::pair<std::int16_t, std::int16_t>>
        direct_asset_origins;
    const auto catalog_path = asset_root / "catalog.tsv";
    std::ifstream catalog(catalog_path, std::ios::binary);
    if (catalog) {
        std::string line;
        std::getline(catalog, line); // header
        while (std::getline(catalog, line)) {
            const auto first = line.find('\t');
            const auto second = first == std::string::npos
                ? std::string::npos : line.find('\t', first + 1U);
            const auto third = second == std::string::npos
                ? std::string::npos : line.find('\t', second + 1U);
            const auto fourth = third == std::string::npos
                ? std::string::npos : line.find('\t', third + 1U);
            if (first == std::string::npos || second == std::string::npos ||
                third == std::string::npos || fourth == std::string::npos) {
                continue;
            }
            const auto relative = line.substr(0, first);
            const auto origin_x = std::stoi(
                line.substr(second + 1U, third - second - 1U));
            const auto origin_y = std::stoi(
                line.substr(third + 1U, fourth - third - 1U));
            direct_asset_origins.emplace(
                relative,
                std::make_pair(static_cast<std::int16_t>(origin_x),
                               static_cast<std::int16_t>(origin_y)));
        }
    }

    const auto load = [&asset_root, &direct_asset_origins](
                          const std::string& relative) {
        const auto path = asset_root / relative;
        try {
            const auto image = read_bmp32(path.string());
            native::NativeImage result{image.width, image.height, image.pixels};
            if (const auto origin = direct_asset_origins.find(relative);
                origin != direct_asset_origins.end()) {
                result.origin_x = origin->second.first;
                result.origin_y = origin->second.second;
            }
            return result;
        } catch (const std::exception& error) {
            throw std::runtime_error("cannot load direct asset " +
                                     path.string() + ": " + error.what());
        }
    };
    assets.title_scene = load("title/background.bmp");
    assets.title_background = assets.title_scene;
    for (std::size_t index = 0; index < assets.title_circuit_lights.size(); ++index) {
        assets.title_circuit_lights[index] =
            load("title/circuit-background-" + std::to_string(index) + ".bmp");
    }
    assets.title_logo_plane = load("title/logo.bmp");
    assets.title_text = load("title/text.bmp");
    for (std::size_t index = 0; index < assets.title_sprite_frames.size(); ++index) {
        assets.title_sprite_frames[index] =
            load("title/sprite-" + std::to_string(index) + ".bmp");
        assets.title_animation_frames[index] =
            load("title/animation-" + std::to_string(index) + ".bmp");
    }
    assets.boot_static = load("boot/snow.bmp");
    assets.boot_background = load("boot/sega-background.bmp");
    assets.boot_guy = load("boot/guy.bmp");
    for (std::size_t index = 0; index < assets.boot_sega_frames.size(); ++index) {
        assets.boot_sega_frames[index] =
            load("boot/sega-lightning-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.boot_logo_strobe.size(); ++index) {
        assets.boot_logo_strobe[index] =
            load("boot/sega-strobe-" + std::to_string(index) + ".bmp");
    }
    assets.boot_fasa = load("boot/fasa.bmp");
    assets.startup_background = load("startup/background.bmp");
    assets.startup_font = load("startup/font.bmp");
    assets.startup_cursor = load("startup/cursor.bmp");
    for (std::size_t index = 0; index < assets.intro_stages.size(); ++index) {
        assets.intro_stages[index] =
            load("intro/stage-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.intro_wilderness_pan_frames.size(); ++index) {
        assets.intro_wilderness_pan_frames[index] =
            load("intro/wilderness-pan-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.intro_ambush_frames.size(); ++index) {
        assets.intro_ambush_frames[index] =
            load("intro/ambush-" + std::to_string(index) + ".bmp");
    }
    assets.intro_aircraft_plane_a = load("intro/aircraft-plane-a.bmp");
    for (std::size_t index = 0; index < assets.intro_heli_frames.size(); ++index) {
        assets.intro_heli_frames[index] =
            load("intro/helicopter-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.intro_aircraft_movement_frames.size(); ++index) {
        assets.intro_aircraft_movement_frames[index] =
            load("intro/aircraft-movement-" + std::to_string(index) + ".bmp");
    }
    assets.intro_trivid_background = load("intro/trivid-background.bmp");
    assets.intro_trivid_slain = load("intro/trivid-slain.bmp");
    assets.intro_trivid_moving = load("intro/trivid-moving.bmp");
    assets.cab_map = load("intro/cab-map.bmp");

    assets.field_hud = load("field/hud/background.bmp");
    for (std::size_t index = 0; index < assets.field_hud_faces.size(); ++index) {
        assets.field_hud_faces[index] = load(
            "field/hud/face-" + std::to_string(index) + ".bmp");
        assets.field_hud_item_blocks[index] = load(
            "field/hud/item-" + std::to_string(index) + ".bmp");
        assets.field_hud_unarmed_item_blocks[index] = load(
            "field/hud/unarmed-item-" + std::to_string(index) + ".bmp");
        assets.field_hud_vertical_counters[index] = load(
            "field/hud/vertical-counter-" + std::to_string(index) + ".bmp");
        assets.field_hud_horizontal_counters[index] = load(
            "field/hud/horizontal-counter-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_hud_ammo_counters.size(); ++index) {
        assets.field_hud_ammo_counters[index] = load(
            "field/hud/ammo-counter-" + std::to_string(index) + ".bmp");
    }
    assets.pocket_secretary_background =
        load("field/menu/pocket-secretary-background.bmp");
    assets.field_menu_background = load("field/menu/background.bmp");
    assets.field_menu_action_background =
        load("field/menu/action-background.bmp");
    assets.field_menu_attributes_background =
        load("field/menu/attributes-background.bmp");
    assets.field_menu_inventory_boxes = load("field/menu/inventory-boxes.bmp");
    assets.field_menu_font = load("field/menu/font.bmp");
    for (std::size_t index = 0; index < assets.field_menu_runner_portraits.size(); ++index) {
        assets.field_menu_runner_portraits[index] = load(
            "field/menu/runner-portrait-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_selected_cells.size(); ++index) {
        assets.field_menu_selected_cells[index] = load(
            "field/menu/selected-cell-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_command_focus.size(); ++index) {
        assets.field_menu_command_focus[index] = load(
            "field/menu/command-focus-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_action_focus.size(); ++index) {
        assets.field_menu_action_focus[index] = load(
            "field/menu/action-focus-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_clips_focus.size(); ++index) {
        assets.field_menu_clips_focus[index] = load(
            "field/menu/clips-focus-" + std::to_string(index) + ".bmp");
        assets.field_menu_clips_trade_focus[index] = load(
            "field/menu/clips-trade-focus-" + std::to_string(index) + ".bmp");
    }
    assets.field_menu_posture_focus = load("field/menu/posture-focus.bmp");
    assets.field_menu_cursor = load("field/menu/cursor.bmp");
    for (std::size_t index = 0; index < assets.field_menu_item_art.size(); ++index) {
        assets.field_menu_item_art[index] = load(
            "field/menu/item-art-" + std::to_string(index) + ".bmp");
        for (std::size_t variant = 0;
             variant < assets.field_menu_inventory_icons[index].size(); ++variant) {
            assets.field_menu_inventory_icons[index][variant] = load(
                "field/menu/inventory-icon-" + std::to_string(index) + "-" +
                std::to_string(variant) + ".bmp");
        }
    }
    for (std::size_t index = 0; index < assets.field_menu_status_bars.size(); ++index) {
        assets.field_menu_status_bars[index] = load(
            "field/menu/status-bar-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_damage_bars.size(); ++index) {
        assets.field_menu_damage_bars[index] = load(
            "field/menu/damage-bar-" + std::to_string(index) + ".bmp");
        assets.field_menu_drain_bars[index] = load(
            "field/menu/drain-bar-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_detail_values.size(); ++index) {
        assets.field_menu_detail_values[index] = load(
            "field/menu/detail-value-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_attack_bars.size(); ++index) {
        assets.field_menu_attack_bars[index] = load(
            "field/menu/attack-bar-" + std::to_string(index) + ".bmp");
        assets.field_menu_defense_bars[index] = load(
            "field/menu/defense-bar-" + std::to_string(index) + ".bmp");
    }

    assets.field_map = load("field/map/mode-2.bmp");
    for (auto& area : assets.field_areas) {
        area.map = load("field/map/mode-" + std::to_string(area.mode_id) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.entity_frames.size(); ++index) {
        const auto image = load("field/entities/idle-" + std::to_string(index) + ".bmp");
        assets.entity_frames[index] = {image.width, image.height, image.pixels};
    }
    for (std::size_t direction = 0;
         direction < assets.field_entity_animations.size(); ++direction) {
        auto& animation = assets.field_entity_animations[direction];
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            const auto image = load("field/entities/walk-" +
                                    std::to_string(direction) + "-" +
                                    std::to_string(frame) + ".bmp");
            animation.frames[frame] = {image.width, image.height, image.pixels};
        }
    }
    fill_grayscale_entity_assets(assets);
    return assets;
}

RgbaImage make_grayscale_image(const RgbaImage& color) {
    RgbaImage gray = color;
    for (auto& pixel : gray.pixels) {
        const auto luminance = static_cast<std::uint8_t>(
            (static_cast<unsigned>(pixel.red) * 77U +
             static_cast<unsigned>(pixel.green) * 150U +
             static_cast<unsigned>(pixel.blue) * 29U) /
            256U);
        pixel.red = luminance;
        pixel.green = luminance;
        pixel.blue = luminance;
    }
    return gray;
}

native::NativeEntityFrame grayscale_entity_frame(
    const native::NativeEntityFrame& color) {
    const RgbaImage gray = make_grayscale_image(
        {color.width, color.height, color.pixels});
    return {gray.width, gray.height, gray.pixels};
}

void fill_grayscale_entity_assets(native::NativePortAssets& assets) {
    assets.entity_frames_gray.clear();
    assets.entity_frames_gray.reserve(assets.entity_frames.size());
    for (const auto& frame : assets.entity_frames) {
        assets.entity_frames_gray.push_back(grayscale_entity_frame(frame));
    }
    for (std::size_t index = 0; index < assets.field_entity_animations.size();
         ++index) {
        const auto& color = assets.field_entity_animations[index];
        auto& gray = assets.field_entity_animations_gray[index];
        gray = color;
        gray.frames.clear();
        gray.frames.reserve(color.frames.size());
        for (const auto& frame : color.frames) {
            gray.frames.push_back(grayscale_entity_frame(frame));
        }
    }
}

native::Rgba8 decode_genesis_cram_word(std::uint16_t cram_word,
                                       bool transparent) {
    const auto expand = [](std::uint8_t value) {
        return static_cast<std::uint8_t>((value << 5U) |
                                         (value << 2U) |
                                         (value >> 1U));
    };
    return {
        expand(static_cast<std::uint8_t>((cram_word >> 1U) & 0x07U)),
        expand(static_cast<std::uint8_t>((cram_word >> 5U) & 0x07U)),
        expand(static_cast<std::uint8_t>((cram_word >> 9U) & 0x07U)),
        static_cast<std::uint8_t>(transparent ? 0 : 255),
    };
}

IndexedTile decode_genesis_tile(const std::uint8_t* source,
                                std::size_t source_size) {
    require_bytes(source_size, 32, "Genesis tile is truncated");
    IndexedTile tile;
    for (std::size_t row = 0; row < IndexedTile::kHeight; ++row) {
        for (std::size_t byte = 0; byte < 4U; ++byte) {
            const auto packed_pixels = source[row * 4U + byte];
            const auto pixel = row * IndexedTile::kWidth + byte * 2U;
            tile.pixels[pixel] = static_cast<std::uint8_t>(
                packed_pixels >> 4U);
            tile.pixels[pixel + 1U] = static_cast<std::uint8_t>(
                packed_pixels & 0x0FU);
        }
    }
    return tile;
}

struct GenesisResourceDecoderState {
    std::array<std::uint8_t, 0x800> ring{};
};

std::vector<std::uint8_t> decode_genesis_resource_stream_with_state(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset,
    GenesisResourceDecoderState& state) {
    if (source_offset > rom.size() || rom.size() - source_offset < 2U) {
        throw std::out_of_range("Genesis resource stream is truncated");
    }
    const auto initial_counter = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(rom[source_offset]) << 8U) |
        rom[source_offset + 1U]);
    auto source = source_offset + 2U;
    auto remaining = initial_counter;
    auto& ring = state.ring;
    std::uint16_t ring_index = 0;
    std::uint8_t control = 0;
    std::uint16_t control_bits_remaining = 1;
    std::vector<std::uint8_t> output;
    output.reserve(static_cast<std::size_t>(initial_counter) + 1U);

    const auto read_byte = [&]() {
        if (source >= rom.size()) {
            throw std::out_of_range("Genesis resource token is truncated");
        }
        return rom[source++];
    };
    const auto emit = [&](std::uint8_t value) {
        ring[ring_index] = value;
        ring_index = static_cast<std::uint16_t>((ring_index + 1U) & 0x07FFU);
        output.push_back(value);
        --remaining;
    };

    for (;;) {
        --control_bits_remaining;
        if (control_bits_remaining == 0) {
            control = read_byte();
            control_bits_remaining = 8;
        }
        const auto compressed = (control & 0x80U) != 0;
        control = static_cast<std::uint8_t>(control << 1U);

        if (!compressed) {
            emit(read_byte());
        } else {
            const auto descriptor = read_byte();
            if ((descriptor & 0x80U) == 0) {
                const auto run_count = static_cast<unsigned>(
                    (descriptor & 0x7FU) + 2U);
                const auto value = read_byte();
                for (unsigned index = 0; index <= run_count; ++index) {
                    emit(value);
                }
            } else {
                const auto offset_low = read_byte();
                const auto offset = static_cast<std::uint16_t>(
                    ((static_cast<std::uint16_t>(offset_low) << 3U) |
                     ((descriptor >> 4U) & 0x0007U)) & 0x07FFU);
                const auto run_count = static_cast<unsigned>(
                    (descriptor & 0x0FU) + 2U);
                auto copy_index = static_cast<std::uint16_t>(
                    (ring_index - offset) & 0x07FFU);
                for (unsigned index = 0; index <= run_count; ++index) {
                    const auto value = ring[copy_index];
                    copy_index = static_cast<std::uint16_t>(
                        (copy_index + 1U) & 0x07FFU);
                    emit(value);
                }
            }
        }
        if (remaining == 0xFFFFU) {
            break;
        }
    }
    return output;
}

std::vector<std::uint8_t> decode_genesis_resource_stream(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset) {
    GenesisResourceDecoderState state;
    return decode_genesis_resource_stream_with_state(rom, source_offset,
                                                     state);
}

void write_genesis_1530_vram_with_state(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset,
    std::size_t target_offset, std::vector<std::uint8_t>& vram,
    GenesisResourceDecoderState& state) {
    constexpr std::size_t kVramSize = 0x10000U;
    if (vram.size() != kVramSize) {
        throw std::invalid_argument("Genesis VDP buffer has invalid size");
    }
    const auto bytes = decode_genesis_resource_stream_with_state(
        rom, source_offset, state);
    const auto counter = read_rom_u16(
        rom, source_offset, "Genesis 0x1530 resource stream is truncated");
    // The decompressor emits bytes into the 0x800-byte overlap ring, but the
    // 0x1530 routine sends one VDP word for every pair of emitted bytes. The
    // DBF counter starts at the first word's low-byte phase, so an odd
    // counter shifts the first byte into D7's high lane and the following
    // even counter writes the completed word.
    if (target_offset > kVramSize ||
        bytes.size() > kVramSize - target_offset) {
        throw std::out_of_range("Genesis 0x1530 VDP upload is truncated");
    }
    std::uint16_t d7 = 0;
    auto d2 = counter;
    std::size_t word_index = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        // MOVE.B D7,(A1,D6.W) replaces only the low byte. When the counter
        // is odd the ROM shifts that low byte into the high byte before the
        // MOVE.W to VDP, leaving the other byte from the previous output.
        d7 = static_cast<std::uint16_t>((d7 & 0xFF00U) | bytes[index]);
        if ((d2 & 1U) != 0U) {
            d7 = static_cast<std::uint16_t>(d7 << 8U);
        } else {
            const auto destination = target_offset + word_index * 2U;
            vram[destination] = static_cast<std::uint8_t>(d7 >> 8U);
            vram[destination + 1U] = static_cast<std::uint8_t>(d7);
            ++word_index;
        }
        d2 = static_cast<std::uint16_t>(d2 - 1U);
    }
}

void write_genesis_1530_vram(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset,
    std::size_t target_offset, std::vector<std::uint8_t>& vram) {
    GenesisResourceDecoderState state;
    write_genesis_1530_vram_with_state(rom, source_offset, target_offset, vram,
                                       state);
}

std::vector<std::uint8_t> decode_genesis_1530_vram(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset,
    std::size_t target_offset) {
    std::vector<std::uint8_t> vram(0x10000U, 0);
    write_genesis_1530_vram(rom, source_offset, target_offset, vram);
    return vram;
}

void write_genesis_f5c_vram(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset,
    std::size_t target_offset, std::size_t byte_count,
    std::vector<std::uint8_t>& vram) {
    constexpr std::size_t kVramSize = 0x10000U;
    if (vram.size() != kVramSize) {
        throw std::invalid_argument("Genesis VDP buffer has invalid size");
    }
    if (source_offset > rom.size() ||
        rom.size() - source_offset < byte_count) {
        throw std::out_of_range("Genesis 0xF5C ROM copy is truncated");
    }
    if (target_offset > kVramSize ||
        byte_count > kVramSize - target_offset) {
        throw std::out_of_range("Genesis 0xF5C VDP upload is truncated");
    }
    std::copy(rom.begin() + static_cast<std::ptrdiff_t>(source_offset),
              rom.begin() + static_cast<std::ptrdiff_t>(source_offset +
                                                         byte_count),
              vram.begin() + static_cast<std::ptrdiff_t>(target_offset));
}

std::array<native::Rgba8, 16> read_genesis_palette(
    const std::vector<std::uint8_t>& rom, std::size_t byte_offset) {
    require_bytes(rom.size() - std::min(byte_offset, rom.size()), 32,
                  "Genesis palette is truncated");
    std::array<native::Rgba8, 16> palette{};
    for (std::size_t index = 0; index < palette.size(); ++index) {
        const auto offset = byte_offset + index * 2U;
        const auto word = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
        palette[index] = decode_genesis_cram_word(word, index == 0);
    }
    return palette;
}

std::array<native::Rgba8, 64> read_genesis_palette_set(
    const std::vector<std::uint8_t>& rom, std::size_t byte_offset) {
    require_bytes(rom.size() - std::min(byte_offset, rom.size()), 128,
                  "Genesis palette set is truncated");
    std::array<native::Rgba8, 64> palette{};
    for (std::size_t index = 0; index < palette.size(); ++index) {
        const auto offset = byte_offset + index * 2U;
        const auto word = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
        palette[index] = decode_genesis_cram_word(word, index % 16U == 0);
    }
    return palette;
}

namespace {

std::array<native::Rgba8, 64> read_genesis_field_palette_set(
    const std::vector<std::uint8_t>& rom, std::size_t first_line_offset) {
    // 0x1406 loads the setup palette's first line, then field entry 0x2C6C
    // calls 0x566C2, which replaces the remaining 48 words from 0x18AC56.
    auto palette = read_genesis_palette_set(rom, first_line_offset);
    require_bytes(rom.size() - std::min<std::size_t>(0x18AC56U, rom.size()),
                  48U * 2U, "Genesis field palette tail is truncated");
    for (std::size_t index = 16U; index < palette.size(); ++index) {
        palette[index] = decode_genesis_cram_word(
            read_rom_u16(rom, 0x18AC56U + (index - 16U) * 2U,
                         "Genesis field palette tail is truncated"),
            index % 16U == 0U);
    }
    return palette;
}

} // namespace

RgbaImage decode_genesis_field_menu_layer(
    const std::vector<std::uint8_t>& rom, bool inventory_boxes,
    bool startup_crt = false) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::size_t kNameTableColumns = 64U;
    constexpr std::size_t kNameTableRows = 32U;
    constexpr std::size_t kVisibleColumns = 40U;
    constexpr std::size_t kVisibleRows = 28U;
    constexpr std::size_t kNameTableRowStride = 0x80U;

    // ROM 0x10918 sends CAB0C through 0x1530 to VRAM at 0x1E20. Reproduce
    // the complete word writes, including D7's preserved byte, then decode
    // the resulting VRAM with the Genesis four-plane format.
    auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    // 0x10F50 finishes the shared pattern upload with the raw 0x200-byte
    // block at CF134 -> VRAM 5A80. The name-table blocks below reference
    // tiles 2D4..2E3 from that block; omitting it leaves the large center
    // status panel black even though the frame edges decode correctly.
    require_bytes(rom.size() - std::min<std::size_t>(0xCF134U, rom.size()),
                  0x200U, "Genesis field menu status patterns are truncated");
    std::copy_n(rom.begin() + 0xCF134U, 0x200U,
                pattern_vram.begin() + 0x5A80U);

    std::array<std::uint16_t, kNameTableColumns * kNameTableRows>
        name_table{};
    std::array<bool, kNameTableColumns * kNameTableRows> written{};
    const auto write_block = [&](std::size_t source_offset,
                                 std::uint16_t vdp_address,
                                 std::size_t width, std::size_t height,
                                 std::uint16_t attributes) {
        if (width == 0U || height == 0U ||
            source_offset > rom.size() ||
            width * height > (rom.size() - source_offset) / 2U) {
            throw std::out_of_range("Genesis field menu name-table block is truncated");
        }
        const auto table_byte_offset = static_cast<std::size_t>(vdp_address) -
            0xE000U;
        if (table_byte_offset % 2U != 0U ||
            table_byte_offset / 2U >= name_table.size()) {
            throw std::out_of_range("Genesis field menu name-table address is invalid");
        }
        const auto first_column = (table_byte_offset % kNameTableRowStride) / 2U;
        const auto first_row = table_byte_offset / kNameTableRowStride;
        if (first_column + width > kNameTableColumns ||
            first_row + height > kNameTableRows) {
            throw std::out_of_range("Genesis field menu name-table block exceeds VDP plane");
        }
        for (std::size_t row = 0; row < height; ++row) {
            for (std::size_t column = 0; column < width; ++column) {
                const auto source = source_offset + (row * width + column) * 2U;
                const auto word = static_cast<std::uint16_t>(
                    (static_cast<std::uint16_t>(rom[source]) << 8U) |
                    rom[source + 1U]);
                const auto table_index = (first_row + row) * kNameTableColumns +
                                         first_column + column;
                name_table[table_index] =
                    static_cast<std::uint16_t>(word | attributes);
                written[table_index] = true;
            }
        }
    };

    // These are the four literal 0x1232 calls made by 0x10F50. The source
    // blocks and the E000/EB98 destinations are the screen's actual Genesis
    // geometry, including the four 5x4 item cells used by Inventory.
    if (startup_crt) {
        // ROM 0x10B5A: centered CRT bezel used by Start Options / Load /
        // Choose Archetype. D4 is the signed VRAM word (-0x1FFA = $E006).
        write_block(0xCDCE6U, 0xE006U, 34U, 4U, 0);
        write_block(0xCDDF6U, 0xE206U, 5U, 17U, 0);
        write_block(0xCDEA0U, 0xE240U, 5U, 17U, 0);
        write_block(0xCDF4AU, 0xEA86U, 34U, 7U, 0);
    } else if (!inventory_boxes) {
        write_block(0xCD598U, 0xE000U, 12U, 28U, 0);
        write_block(0xCD838U, 0xE018U, 21U, 2U, 0);
        write_block(0xCD88CU, 0xE042U, 1U, 23U, 0);
        write_block(0xCD8BAU, 0xEB98U, 22U, 5U, 0);
        // The command entry point immediately follows the initializer with
        // 0x11928, 0x11624, and 0x111D0. These calls add the lower static
        // panel and the four 5x4 inventory cells before input is sampled.
        write_block(0xCD48AU, 0xEA18U, 21U, 3U, 0);
        write_block(0xCD570U, 0xE702U, 5U, 4U, 0);
        write_block(0xCD570U, 0xE70CU, 5U, 4U, 0);
        write_block(0xCD570U, 0xEA02U, 5U, 4U, 0);
        write_block(0xCD570U, 0xEA0CU, 5U, 4U, 0);
        write_block(0xCD2C6U, 0xEB9EU, 15U, 2U, 0);
    } else {
        // ROM 0x116C4 draws these four cells after the selector enters the
        // Inventory branch. Their destinations are the four literal VDP
        // addresses passed to 0x1232, not a host-invented checklist grid.
        write_block(0xCD570U, 0xE702U, 5U, 4U, 0);
        write_block(0xCD570U, 0xE70CU, 5U, 4U, 0);
        write_block(0xCD570U, 0xEA02U, 5U, 4U, 0);
        write_block(0xCD570U, 0xEA0CU, 5U, 4U, 0);
    }

    const auto palette_source = read_genesis_palette(rom, 0xD29D6U);
    RgbaImage image{
        static_cast<std::uint16_t>(kVisibleColumns * 8U),
        static_cast<std::uint16_t>(kVisibleRows * 8U),
        std::vector<native::Rgba8>(
            kVisibleColumns * kVisibleRows * 64U,
            inventory_boxes ? native::Rgba8{0, 0, 0, 0}
                            : native::Rgba8{0, 0, 0, 255}),
    };
    for (std::size_t row = 0; row < kVisibleRows; ++row) {
        for (std::size_t column = 0; column < kVisibleColumns; ++column) {
            const auto name_word =
                name_table[row * kNameTableColumns + column];
            if (inventory_boxes &&
                !written[row * kNameTableColumns + column]) {
                continue;
            }
            const auto tile_index = static_cast<std::size_t>(
                name_word & 0x07FFU);
            if (tile_index * 32U + 32U > pattern_vram.size()) {
                throw std::out_of_range("Genesis field menu tile index is truncated");
            }
            const auto tile = decode_genesis_tile(
                pattern_vram.data() + tile_index * 32U, 32U);
            const auto palette_line = static_cast<std::size_t>(
                (name_word >> 13U) & 0x03U);
            if (palette_line != 0U) {
                throw std::runtime_error(
                    "Genesis field menu base selects an unreviewed palette line");
            }
            const auto hflip = (name_word & 0x0800U) != 0;
            const auto vflip = (name_word & 0x1000U) != 0;
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                    const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                    const auto color_index = tile.pixels[
                        source_y * 8U + source_x];
                    auto color = palette_source[color_index];
                    // Plane pixels with palette index zero are the opaque
                    // black background; only Genesis sprites use index zero
                    // as transparent in this screen composition.
                    color.alpha = 255;
                    image.pixels[(row * 8U + pixel_y) * image.width +
                                 column * 8U + pixel_x] = color;
                }
            }
        }
    }
    return image;
}

RgbaImage decode_genesis_field_menu_background(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_field_menu_layer(rom, false);
}

RgbaImage decode_genesis_field_menu_attributes_background(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::size_t kNameTableColumns = 64U;
    constexpr std::size_t kNameTableRows = 32U;
    constexpr std::size_t kVisibleColumns = 40U;
    constexpr std::size_t kVisibleRows = 28U;
    constexpr std::size_t kNameTableRowStride = 0x80U;

    // 0xD01C reaches 0x10C96, whose seven 0x1232 calls paint a different
    // frame from the ordinary command-screen initializer at 0x10F50.
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    std::array<std::uint16_t, kNameTableColumns * kNameTableRows>
        name_table{};
    std::array<bool, kNameTableColumns * kNameTableRows> written{};
    const auto write_block = [&](std::size_t source_offset,
                                 std::uint16_t vdp_address,
                                 std::size_t width, std::size_t height) {
        if (width == 0U || height == 0U || source_offset > rom.size() ||
            width * height > (rom.size() - source_offset) / 2U) {
            throw std::out_of_range(
                "Genesis attributes name-table block is truncated");
        }
        const auto table_byte_offset = static_cast<std::size_t>(vdp_address) -
            0xE000U;
        if (table_byte_offset % 2U != 0U ||
            table_byte_offset / 2U >= name_table.size()) {
            throw std::out_of_range(
                "Genesis attributes name-table address is invalid");
        }
        const auto first_column =
            (table_byte_offset % kNameTableRowStride) / 2U;
        const auto first_row = table_byte_offset / kNameTableRowStride;
        if (first_column + width > kNameTableColumns ||
            first_row + height > kNameTableRows) {
            throw std::out_of_range(
                "Genesis attributes name-table block exceeds VDP plane");
        }
        for (std::size_t row = 0; row < height; ++row) {
            for (std::size_t column = 0; column < width; ++column) {
                const auto source = source_offset +
                    (row * width + column) * 2U;
                const auto word = static_cast<std::uint16_t>(
                    (static_cast<std::uint16_t>(rom[source]) << 8U) |
                    rom[source + 1U]);
                const auto table_index =
                    (first_row + row) * kNameTableColumns +
                    first_column + column;
                name_table[table_index] = word;
                written[table_index] = true;
            }
        }
    };

    // 0x10C96 literals, in VDP order. The first two blocks form the upper
    // panes, the next two are the side rails, and the last three are the
    // horizontal dividers surrounding the six skill rows.
    write_block(0xCE850U, 0xE000U, 20U, 7U);
    write_block(0xCE968U, 0xE028U, 20U, 7U);
    write_block(0xCEA80U, 0xE380U, 2U, 21U);
    write_block(0xCEAD4U, 0xE3CCU, 2U, 21U);
    write_block(0xCEB28U, 0xE584U, 36U, 2U);
    write_block(0xCEBB8U, 0xE984U, 36U, 2U);
    write_block(0xCEC48U, 0xED04U, 36U, 2U);

    const auto palette = read_genesis_palette(rom, 0xD29D6U);
    RgbaImage image{
        static_cast<std::uint16_t>(kVisibleColumns * 8U),
        static_cast<std::uint16_t>(kVisibleRows * 8U),
        std::vector<native::Rgba8>(
            kVisibleColumns * kVisibleRows * 64U,
            native::Rgba8{0, 0, 0, 255}),
    };
    for (std::size_t row = 0; row < kVisibleRows; ++row) {
        for (std::size_t column = 0; column < kVisibleColumns; ++column) {
            const auto table_index = row * kNameTableColumns + column;
            if (!written[table_index]) {
                continue;
            }
            const auto name_word = name_table[table_index];
            const auto tile_index = static_cast<std::size_t>(
                name_word & 0x07FFU);
            if (tile_index * 32U + 32U > pattern_vram.size()) {
                throw std::out_of_range(
                    "Genesis attributes name-table tile is truncated");
            }
            const auto tile = decode_genesis_tile(
                pattern_vram.data() + tile_index * 32U, 32U);
            const auto palette_line = static_cast<std::size_t>(
                (name_word >> 13U) & 0x03U);
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto source_x = (name_word & 0x0800U) != 0U
                        ? 7U - pixel_x : pixel_x;
                    const auto source_y = (name_word & 0x1000U) != 0U
                        ? 7U - pixel_y : pixel_y;
                    auto color = palette[palette_line * 16U +
                        tile.pixels[source_y * 8U + source_x]];
                    color.alpha = 255U;
                    image.pixels[(row * 8U + pixel_y) * image.width +
                                 column * 8U + pixel_x] = color;
                }
            }
        }
    }
    return image;
}

void write_native_field_record_state(std::ostream& output,
                                     const native::NativeFieldRecord& record) {
    write_u16(output, record.position.world_x_fixed);
    write_u16(output, record.position.world_y_fixed);
    write_u16(output, record.record_type);
    output.put(static_cast<char>(record.flags_10));
    output.put(static_cast<char>(record.flags_11));
}

native::NativeFieldRecord read_native_field_record_state(std::istream& input) {
    native::NativeFieldRecord record;
    record.position.world_x_fixed = read_pack_u16(
        input, "native field record state is truncated");
    record.position.world_y_fixed = read_pack_u16(
        input, "native field record state is truncated");
    record.record_type = read_pack_u16(
        input, "native field record state is truncated");
    record.flags_10 = read_pack_u8(
        input, "native field record state is truncated");
    record.flags_11 = read_pack_u8(
        input, "native field record state is truncated");
    return record;
}

void write_native_field_entity_state(
    std::ostream& output, const native::NativeFieldEntity& entity) {
    output.put(static_cast<char>(entity.record_index));
    write_u16(output, entity.world_x_fixed);
    write_u16(output, entity.world_y_fixed);
    write_u16(output, entity.projection_word_08);
    write_u16(output, entity.kind);
    write_u16(output, entity.record_type);
    output.put(static_cast<char>(entity.race_index));
    output.put(static_cast<char>(entity.archetype_index));
    output.put(static_cast<char>(entity.physical_value));
    output.put(static_cast<char>(entity.mental_value));
    output.put(static_cast<char>(entity.active_item_code));
    output.put(static_cast<char>(entity.flags_10));
    output.put(static_cast<char>(entity.flags_11));
    write_u32(output, entity.state_pointer);
    write_u16(output, entity.vdp_screen_x);
    write_u16(output, entity.vdp_screen_y);
    write_u16(output, entity.screen_x);
    write_u16(output, entity.screen_y);
    output.put(static_cast<char>(entity.visible ? 1U : 0U));
    if (entity.frame_index > 0xFFFFFFFFU) {
        throw std::invalid_argument("native entity frame index is too large");
    }
    write_u32(output, static_cast<std::uint32_t>(entity.frame_index));
    write_u16(output, entity.state_selector);
    write_u16(output, entity.state_index);
    output.put(static_cast<char>(static_cast<std::uint8_t>(
        entity.display_state)));
    output.put(static_cast<char>(static_cast<std::uint8_t>(
        entity.display_substate)));
    output.put(static_cast<char>(entity.display_palette_state));
    output.put(static_cast<char>(entity.menu_status_index));
    output.put(static_cast<char>(entity.attack_bar_index));
    output.put(static_cast<char>(entity.defense_bar_index));
    for (const auto index : entity.attack_bar_indices_by_posture) {
        output.put(static_cast<char>(index));
    }
    for (const auto index : entity.defense_bar_indices_by_posture) {
        output.put(static_cast<char>(index));
    }
    output.put(static_cast<char>(entity.weapons_bar_index));
    output.put(static_cast<char>(entity.guns_bar_index));
    write_u16(output, entity.animation_accumulator);
    write_u16(output, entity.animation_multiplier);
    write_u16(output, entity.animation_threshold);
    write_u32(output, entity.animation_descriptor);
    for (const auto code : entity.inventory_item_codes) {
        output.put(static_cast<char>(code));
    }
    for (const auto phase : entity.inventory_phase_bytes) {
        output.put(static_cast<char>(phase));
    }
    output.put(static_cast<char>(entity.equipped_armor_index));
    output.put(static_cast<char>(entity.ammo_rounds));
    output.put(static_cast<char>(entity.ammo_clips));
    write_u16(output, entity.cyberware_flags);
    for (const auto value : entity.attribute_values) {
        output.put(static_cast<char>(value));
    }
    for (const auto value : entity.skill_values) {
        output.put(static_cast<char>(value));
    }
    for (const auto value : entity.attribute_skill_auxiliary) {
        output.put(static_cast<char>(value));
    }
}

native::NativeFieldEntity read_native_field_entity_state(std::istream& input) {
    native::NativeFieldEntity entity;
    entity.record_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.world_x_fixed = read_pack_u16(
        input, "native field entity state is truncated");
    entity.world_y_fixed = read_pack_u16(
        input, "native field entity state is truncated");
    entity.projection_word_08 = read_pack_u16(
        input, "native field entity state is truncated");
    entity.kind = read_pack_u16(input, "native field entity state is truncated");
    entity.record_type = read_pack_u16(
        input, "native field entity state is truncated");
    entity.race_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.archetype_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.physical_value = read_pack_u8(
        input, "native field entity state is truncated");
    entity.mental_value = read_pack_u8(
        input, "native field entity state is truncated");
    entity.active_item_code = read_pack_u8(
        input, "native field entity state is truncated");
    entity.flags_10 = read_pack_u8(
        input, "native field entity state is truncated");
    entity.flags_11 = read_pack_u8(
        input, "native field entity state is truncated");
    entity.state_pointer = read_pack_u32(
        input, "native field entity state is truncated");
    entity.vdp_screen_x = read_pack_u16(
        input, "native field entity state is truncated");
    entity.vdp_screen_y = read_pack_u16(
        input, "native field entity state is truncated");
    entity.screen_x = read_pack_u16(
        input, "native field entity state is truncated");
    entity.screen_y = read_pack_u16(
        input, "native field entity state is truncated");
    entity.visible = read_pack_u8(
        input, "native field entity state is truncated") != 0U;
    entity.frame_index = read_pack_u32(
        input, "native field entity state is truncated");
    entity.state_selector = read_pack_u16(
        input, "native field entity state is truncated");
    entity.state_index = read_pack_u16(
        input, "native field entity state is truncated");
    entity.display_state = static_cast<std::int8_t>(read_pack_u8(
        input, "native field entity state is truncated"));
    entity.display_substate = static_cast<std::int8_t>(read_pack_u8(
        input, "native field entity state is truncated"));
    entity.display_palette_state = read_pack_u8(
        input, "native field entity state is truncated");
    entity.menu_status_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.attack_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.defense_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    for (auto& index : entity.attack_bar_indices_by_posture) {
        index = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& index : entity.defense_bar_indices_by_posture) {
        index = read_pack_u8(input, "native field entity state is truncated");
    }
    entity.weapons_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.guns_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.animation_accumulator = read_pack_u16(
        input, "native field entity state is truncated");
    entity.animation_multiplier = read_pack_u16(
        input, "native field entity state is truncated");
    entity.animation_threshold = read_pack_u16(
        input, "native field entity state is truncated");
    entity.animation_descriptor = read_pack_u32(
        input, "native field entity state is truncated");
    for (auto& code : entity.inventory_item_codes) {
        code = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& phase : entity.inventory_phase_bytes) {
        phase = read_pack_u8(input, "native field entity state is truncated");
    }
    entity.equipped_armor_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.ammo_rounds = read_pack_u8(
        input, "native field entity state is truncated");
    entity.ammo_clips = read_pack_u8(
        input, "native field entity state is truncated");
    entity.cyberware_flags = read_pack_u16(
        input, "native field entity state is truncated");
    for (auto& value : entity.attribute_values) {
        value = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& value : entity.skill_values) {
        value = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& value : entity.attribute_skill_auxiliary) {
        value = read_pack_u8(input, "native field entity state is truncated");
    }
    return entity;
}

void write_native_terrain_state(std::ostream& output,
                                const terrain::Map& map) {
    write_u16(output, map.x_exclusive_upper);
    write_u16(output, map.y_exclusive_upper);
    write_u16(output, map.x_inclusive_lower);
    write_u16(output, map.y_inclusive_lower);
    if (map.regions.size() > 0xFFFFFFFFU ||
        map.statuses.size() > 0xFFFFFFFFU) {
        throw std::invalid_argument("native terrain state is too large");
    }
    write_u32(output, static_cast<std::uint32_t>(map.regions.size()));
    for (const auto& region : map.regions) {
        write_u16(output, region.x_start);
        write_u16(output, region.x_end);
        write_u16(output, region.y_start);
        write_u16(output, region.y_end);
        output.put(static_cast<char>(region.status));
    }
    write_u32(output, static_cast<std::uint32_t>(map.statuses.size()));
    output.write(reinterpret_cast<const char*>(map.statuses.data()),
                 static_cast<std::streamsize>(map.statuses.size()));
}

terrain::Map read_native_terrain_state(std::istream& input) {
    terrain::Map map;
    map.x_exclusive_upper = read_pack_u16(
        input, "native terrain state is truncated");
    map.y_exclusive_upper = read_pack_u16(
        input, "native terrain state is truncated");
    map.x_inclusive_lower = read_pack_u16(
        input, "native terrain state is truncated");
    map.y_inclusive_lower = read_pack_u16(
        input, "native terrain state is truncated");
    map.region_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain state is truncated"));
    if (map.region_count > 65536U) {
        throw std::runtime_error("native terrain state has too many regions");
    }
    map.regions.resize(map.region_count);
    for (auto& region : map.regions) {
        region.x_start = read_pack_u16(input, "native terrain state is truncated");
        region.x_end = read_pack_u16(input, "native terrain state is truncated");
        region.y_start = read_pack_u16(input, "native terrain state is truncated");
        region.y_end = read_pack_u16(input, "native terrain state is truncated");
        region.status = read_pack_u8(input, "native terrain state is truncated");
    }
    const auto status_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain state is truncated"));
    if (status_count != terrain::Map::kWidth * terrain::Map::kHeight) {
        throw std::runtime_error("native terrain state has an invalid grid");
    }
    map.statuses.resize(status_count);
    read_pack_bytes(input, map.statuses.data(), status_count,
                    "native terrain state is truncated");
    return map;
}

void write_native_intro_state(std::ostream& output,
                              const native::NativeIntroData& intro) {
    write_pack_string(output, intro.start_options_title,
                      "native state startup title is too long");
    for (const auto& value : intro.start_options) {
        write_pack_string(output, value, "native state startup option is too long");
    }
    write_pack_string(output, intro.archetype_title,
                      "native state archetype title is too long");
    for (const auto& value : intro.archetypes) {
        write_pack_string(output, value, "native state archetype is too long");
    }
    if (intro.story_lines.size() > 0xFFFFFFFFU) {
        throw std::invalid_argument("native state has too many story lines");
    }
    write_u32(output, static_cast<std::uint32_t>(intro.story_lines.size()));
    for (const auto& value : intro.story_lines) {
        write_pack_string(output, value, "native state story line is too long");
    }
    const auto write_layout = [&output](
                                  const std::vector<native::NativeStartupTextLine>&
                                      layout) {
        if (layout.size() > 0xFFFFFFFFU) {
            throw std::invalid_argument("native state layout is too large");
        }
        write_u32(output, static_cast<std::uint32_t>(layout.size()));
        for (const auto& line : layout) {
            output.put(static_cast<char>(line.row));
            write_pack_string(output, line.text,
                              "native state layout line is too long");
        }
    };
    write_layout(intro.start_options_layout);
    write_layout(intro.save_load_layout);
    write_layout(intro.archetype_layout);
    write_layout(intro.no_save_layout);
    write_layout(intro.save_success_layout);
}

native::NativeIntroData read_native_intro_state(std::istream& input) {
    native::NativeIntroData intro;
    intro.start_options_title = read_pack_string(
        input, "native state startup title is truncated");
    for (auto& value : intro.start_options) {
        value = read_pack_string(input, "native state startup option is truncated");
    }
    intro.archetype_title = read_pack_string(
        input, "native state archetype title is truncated");
    for (auto& value : intro.archetypes) {
        value = read_pack_string(input, "native state archetype is truncated");
    }
    const auto story_count = static_cast<std::size_t>(read_pack_u32(
        input, "native state story count is truncated"));
    if (story_count > 4096U) {
        throw std::runtime_error("native state has too many story lines");
    }
    intro.story_lines.reserve(story_count);
    for (std::size_t index = 0; index < story_count; ++index) {
        intro.story_lines.push_back(read_pack_string(
            input, "native state story line is truncated"));
    }
    const auto read_layout = [&input]() {
        const auto count = static_cast<std::size_t>(read_pack_u32(
            input, "native state layout count is truncated"));
        if (count > 1024U) {
            throw std::runtime_error("native state has too many layout lines");
        }
        std::vector<native::NativeStartupTextLine> layout;
        layout.reserve(count);
        for (std::size_t index = 0; index < count; ++index) {
            native::NativeStartupTextLine line;
            line.row = read_pack_u8(input, "native state layout is truncated");
            line.text = read_pack_string(input,
                                         "native state layout is truncated");
            layout.push_back(std::move(line));
        }
        return layout;
    };
    intro.start_options_layout = read_layout();
    intro.save_load_layout = read_layout();
    intro.archetype_layout = read_layout();
    intro.no_save_layout = read_layout();
    intro.save_success_layout = read_layout();
    return intro;
}

RgbaImage decode_genesis_startup_background(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_field_menu_layer(rom, false, true);
}

RgbaImage decode_genesis_field_menu_inventory_boxes(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_field_menu_layer(rom, true);
}

namespace {

std::array<native::Rgba8, 64> read_genesis_command_palette_set(
    const std::vector<std::uint8_t>& rom);

} // namespace

RgbaImage decode_genesis_field_menu_font(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kFontStreamOffset = 0xC1064U;
    constexpr std::size_t kFontVramOffset = 0xB560U;
    constexpr std::size_t kCharacterCount = 0x60U;
    constexpr std::size_t kAtlasColumns = 16U;
    constexpr std::size_t kAtlasRows = 6U;

    const auto font_vram = decode_genesis_1530_vram(
        rom, kFontStreamOffset, kFontVramOffset);

    // 0x10D7C invokes 0x5670E before the command loop. The menu font is
    // written with palette-line three (0x6000), so it must use the same
    // command CRAM image as the other field-menu layers rather than the
    // startup palette still stored at 0x1FF0.
    const auto palette = read_genesis_command_palette_set(rom);
    RgbaImage atlas{
        static_cast<std::uint16_t>(kAtlasColumns * 8U),
        static_cast<std::uint16_t>(kAtlasRows * 8U),
        std::vector<native::Rgba8>(
            kAtlasColumns * kAtlasRows * 64U,
            native::Rgba8{0, 0, 0, 0}),
    };
    for (std::size_t character_index = 0;
         character_index < kCharacterCount; ++character_index) {
        // C0DA uses the same compact table index adjustment as the ROM text
        // writer: the four reserved codes between '=' and '>' are absent.
        auto table_index = character_index;
        if (table_index >= 0x3DU) {
            table_index -= 4U;
        }
        const auto table_entry = read_rom_u16(
            rom, 0xC1320U + table_index * 2U,
            "Genesis field menu font table is truncated");
        const auto tile_index = static_cast<std::size_t>(table_entry & 0x07FFU);
        if (tile_index * 32U + 32U > font_vram.size()) {
            throw std::out_of_range("Genesis field menu font tile is truncated");
        }
        const auto tile = decode_genesis_tile(
            font_vram.data() + tile_index * 32U, 32U);
        const auto atlas_x = (character_index % kAtlasColumns) * 8U;
        const auto atlas_y = (character_index / kAtlasColumns) * 8U;
        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                const auto color_index =
                    tile.pixels[pixel_y * 8U + pixel_x];
                auto color = palette[48U + color_index];
                color.alpha = color_index == 0U ? 0U : 255U;
                atlas.pixels[(atlas_y + pixel_y) * atlas.width +
                             atlas_x + pixel_x] = color;
            }
        }
    }
    return atlas;
}

namespace {

std::array<native::Rgba8, 64> read_genesis_command_palette_set(
    const std::vector<std::uint8_t>& rom) {
    std::array<native::Rgba8, 64> palette{};
    const auto base_palette = read_genesis_palette(rom, 0xD29D6U);
    std::copy(base_palette.begin(), base_palette.end(), palette.begin());
    // 0x10D7C calls 0x5670E after the command-screen setup. That routine
    // copies the remaining 48 CRAM words from 0x18AC56; D29D6 is only the
    // first 16 colors, not a contiguous four-line palette image.
    for (std::size_t index = 0; index < 48U; ++index) {
        palette[16U + index] = decode_genesis_cram_word(
            read_rom_u16(rom, 0x18AC56U + index * 2U,
                         "Genesis command palette is truncated"));
    }
    return palette;
}

RgbaImage decode_genesis_menu_tile_grid(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset,
    std::size_t source_tile_count, std::size_t table_offset,
    std::size_t table_tile_count, std::size_t columns, std::size_t rows,
    std::size_t palette_line, bool item_icon) {
    if (columns == 0U || rows == 0U ||
        table_tile_count != columns * rows ||
        source_offset > rom.size() ||
        source_tile_count > (rom.size() - source_offset) / 32U ||
        table_offset > rom.size() ||
        table_tile_count > (rom.size() - table_offset) / 2U ||
        palette_line >= 4U) {
        throw std::out_of_range("Genesis command-screen tile grid is truncated");
    }
    const auto palette = read_genesis_command_palette_set(rom);
    RgbaImage image{
        static_cast<std::uint16_t>(columns * 8U),
        static_cast<std::uint16_t>(rows * 8U),
        std::vector<native::Rgba8>(columns * rows * 64U,
                                   native::Rgba8{0, 0, 0, 255}),
    };
    for (std::size_t tile_position = 0;
         tile_position < table_tile_count; ++tile_position) {
        const auto source_word = read_rom_u16(
            rom, table_offset + tile_position * 2U,
            "Genesis command-screen tile table is truncated");
        const auto tile_index = static_cast<std::size_t>(
            source_word & 0x07FFU);
        if (tile_index >= source_tile_count) {
            throw std::out_of_range(
                "Genesis command-screen source tile index is truncated");
        }
        const auto tile = decode_genesis_tile(
            rom.data() + source_offset + tile_index * 32U, 32U);
        const auto hflip = (source_word & 0x0800U) != 0U;
        const auto vflip = (source_word & 0x1000U) != 0U;
        const auto tile_x = (tile_position % columns) * 8U;
        const auto tile_y = (tile_position / columns) * 8U;
        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                const auto color_index = tile.pixels[
                    source_y * 8U + source_x];
                auto color = palette[palette_line * 16U + color_index];
                color.alpha = 255;
                image.pixels[(tile_y + pixel_y) * image.width +
                             tile_x + pixel_x] = color;
            }
        }
    }
    static_cast<void>(item_icon);
    return image;
}

} // namespace

std::array<RgbaImage, native::kNativeMenuStatusCount>
decode_genesis_field_menu_status_bars(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::size_t kStatusPatternOffset = 0xCF134U;
    constexpr std::size_t kStatusPatternVramOffset = 0x5A80U;
    constexpr std::size_t kStatusTableOffset = 0xCF384U;
    constexpr std::size_t kStatusTableStride = 0x10U;
    constexpr std::size_t kStatusTileCount = 8U;

    auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    require_bytes(rom.size() - std::min(kStatusPatternOffset, rom.size()),
                  0x200U,
                  "Genesis command-screen status patterns are truncated");
    std::copy_n(rom.begin() + kStatusPatternOffset, 0x200U,
                pattern_vram.begin() + kStatusPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);

    std::array<RgbaImage, native::kNativeMenuStatusCount> bars{};
    for (std::size_t status = 0; status < bars.size(); ++status) {
        bars[status] = {
            static_cast<std::uint16_t>(kStatusTileCount * 8U), 8U,
            std::vector<native::Rgba8>(kStatusTileCount * 64U,
                                       native::Rgba8{0, 0, 0, 255})};
        for (std::size_t tile_position = 0;
             tile_position < kStatusTileCount; ++tile_position) {
            const auto table_entry = read_rom_u16(
                rom, kStatusTableOffset + status * kStatusTableStride +
                         tile_position * 2U,
                "Genesis command-screen status table is truncated");
            const auto tile_index = static_cast<std::size_t>(
                table_entry & 0x07FFU);
            if (tile_index * 32U + 32U > pattern_vram.size()) {
                throw std::out_of_range(
                    "Genesis command-screen status tile is truncated");
            }
            const auto tile = decode_genesis_tile(
                pattern_vram.data() + tile_index * 32U, 32U);
            const auto hflip = (table_entry & 0x0800U) != 0U;
            const auto vflip = (table_entry & 0x1000U) != 0U;
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                    const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                    const auto color_index = tile.pixels[
                        source_y * 8U + source_x];
                    auto color = palette[16U + color_index];
                    color.alpha = 255;
                    bars[status].pixels[pixel_y * bars[status].width +
                                        tile_position * 8U + pixel_x] = color;
                }
            }
        }
    }
    return bars;
}

std::array<RgbaImage, 3> decode_genesis_field_menu_runner_portraits(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kStartupEntityOffset = 0x7291AU;
    constexpr std::size_t kRunnerTileSource = 0x6ACFAU;
    // Each 0x7291A template has its own 80-word nametable at +0x50.
    constexpr std::size_t kSourceTileCount = 256U;
    constexpr std::size_t kPortraitTileCount = 80U;
    std::array<RgbaImage, 3> portraits{};
    for (std::size_t index = 0; index < portraits.size(); ++index) {
        const auto record_offset = kStartupEntityOffset + index * 0x100U;
        require_bytes(rom.size() - std::min(record_offset, rom.size()),
                      0x58U, "Genesis runner portrait record is truncated");
        // 0x7291A+0x50 is the 80-word nametable for that template. Bit 0 of
        // +0x10 is not a "use common table" switch: Samurai/Decker/Gator
        // Shaman all have flags $0F00 and distinct tables at 0x720FA /
        // 0x7223A / 0x7219A. Using the common table for all three made the
        // archetype portrait never change.
        const auto table_offset = static_cast<std::size_t>(read_rom_u32(
            rom, record_offset + 0x50U,
            "Genesis runner portrait table pointer is truncated"));
        const auto palette_word = read_rom_u16(
            rom, record_offset + 0x2CU,
            "Genesis runner portrait palette is truncated");
        const auto palette_line = static_cast<std::size_t>(
            (palette_word & 0x6000U) >> 13U);
        portraits[index] = decode_genesis_menu_tile_grid(
            rom, kRunnerTileSource, kSourceTileCount, table_offset,
            kPortraitTileCount,
            8U, 10U, palette_line, false);
    }
    return portraits;
}

namespace {

RgbaImage decode_genesis_menu_name_table_block(
    const std::vector<std::uint8_t>& rom,
    const std::vector<std::uint8_t>& pattern_vram,
    std::size_t source_offset, std::size_t width, std::size_t height,
    const std::array<native::Rgba8, 64>& palette,
    std::uint16_t attributes = 0U) {
    if (width == 0U || height == 0U || source_offset > rom.size() ||
        width * height > (rom.size() - source_offset) / 2U) {
        throw std::out_of_range(
            "Genesis command-screen name-table block is truncated");
    }
    RgbaImage image{
        static_cast<std::uint16_t>(width * 8U),
        static_cast<std::uint16_t>(height * 8U),
        std::vector<native::Rgba8>(width * height * 64U,
                                   native::Rgba8{0, 0, 0, 255})};
    for (std::size_t tile_position = 0;
         tile_position < width * height; ++tile_position) {
        const auto table_entry = static_cast<std::uint16_t>(read_rom_u16(
            rom, source_offset + tile_position * 2U,
            "Genesis command-screen name-table block is truncated") |
            attributes);
        const auto tile_index = static_cast<std::size_t>(
            table_entry & 0x07FFU);
        if (tile_index * 32U + 32U > pattern_vram.size()) {
            throw std::out_of_range(
                "Genesis command-screen name-table tile is truncated");
        }
        const auto tile = decode_genesis_tile(
            pattern_vram.data() + tile_index * 32U, 32U);
        const auto palette_line = static_cast<std::size_t>(
            (table_entry >> 13U) & 0x03U);
        const auto hflip = (table_entry & 0x0800U) != 0U;
        const auto vflip = (table_entry & 0x1000U) != 0U;
        const auto tile_x = (tile_position % width) * 8U;
        const auto tile_y = (tile_position / width) * 8U;
        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                auto color = palette[palette_line * 16U + tile.pixels[
                    source_y * 8U + source_x]];
                color.alpha = 255;
                image.pixels[(tile_y + pixel_y) * image.width +
                             tile_x + pixel_x] = color;
            }
        }
    }
    return image;
}

} // namespace

RgbaImage decode_genesis_field_menu_action_background(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);
    // 0x117A0 supplies A1=$CD302 to 0x111D0. That wrapper writes a 15x2
    // tile row at VDP $EB9E, which is the four-action-button presentation
    // plus the source's trailing blank cell.
    return decode_genesis_menu_name_table_block(
        rom, pattern_vram, 0xCD302U, 15U, 2U, palette);
}

std::array<RgbaImage, 2> decode_genesis_field_menu_selected_cells(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::array<std::size_t, 2> kOverlayOffsets{{0xCD548U,
                                                            0xCD520U}};
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);
    std::array<RgbaImage, 2> overlays{};
    for (std::size_t index = 0; index < overlays.size(); ++index) {
        // 0x11678 passes D7=3 and D1=4 to 0x1232: five columns by four
        // rows. The source selects CD548 for display-state pairs 0 and 2,
        // and CD520 for pairs 1 and 3.
        overlays[index] = decode_genesis_menu_name_table_block(
            rom, pattern_vram, kOverlayOffsets[index], 5U, 4U, palette);
    }
    return overlays;
}

std::array<RgbaImage, 5> decode_genesis_field_menu_command_focus(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    // 0x11454 receives A1=0xCD3F2 for the command-tab selector. It advances
    // by (selector - 2) * 8 bytes and writes a 2x2 block through 0x1232.
    constexpr std::size_t kFocusTableOffset = 0xCD3F2U;
    constexpr std::size_t kFocusCount = 5U;
    constexpr std::size_t kFocusWidth = 2U;
    constexpr std::size_t kFocusHeight = 2U;
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);

    std::array<RgbaImage, kFocusCount> focus{};
    for (std::size_t index = 0; index < focus.size(); ++index) {
        focus[index] = decode_genesis_menu_name_table_block(
            rom, pattern_vram,
            kFocusTableOffset + index * kFocusWidth * kFocusHeight * 2U,
            kFocusWidth, kFocusHeight, palette);
        // The 0x1232 writer uses color zero as an opaque name-table pixel for
        // this block. Preserve that exact source composition rather than
        // treating it as a transparent sprite overlay.
        for (auto& pixel : focus[index].pixels) {
            pixel.alpha = 255U;
        }
    }
    return focus;
}

namespace {

std::array<RgbaImage, 4> decode_genesis_field_menu_focus_table(
    const std::vector<std::uint8_t>& rom, std::size_t table_offset,
    const char* error_prefix) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::size_t kFocusCount = 4U;
    constexpr std::size_t kFocusWidth = 2U;
    constexpr std::size_t kFocusHeight = 2U;
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);
    std::array<RgbaImage, kFocusCount> focus{};
    for (std::size_t index = 0; index < focus.size(); ++index) {
        focus[index] = decode_genesis_menu_name_table_block(
            rom, pattern_vram,
            table_offset + index * kFocusWidth * kFocusHeight * 2U,
            kFocusWidth, kFocusHeight, palette);
        for (auto& pixel : focus[index].pixels) {
            pixel.alpha = 255U;
        }
    }
    static_cast<void>(error_prefix);
    return focus;
}

} // namespace

std::array<RgbaImage, 4> decode_genesis_field_menu_action_focus(
    const std::vector<std::uint8_t>& rom) {
    // 0x117A4 loads A1=$CD41A before calling 0x11454. Four 2x2 blocks
    // follow the same source-table format as the top-level command focus.
    return decode_genesis_field_menu_focus_table(rom, 0xCD41AU,
                                                 "Genesis action focus table");
}

std::array<RgbaImage, 4> decode_genesis_field_menu_clips_focus(
    const std::vector<std::uint8_t>& rom) {
    // 0x11B58 passes CD43A to 0x11454 for Reload/Trade/Distribute/Cancel.
    return decode_genesis_field_menu_focus_table(rom, 0xCD43AU,
                                                 "Genesis Clips focus table");
}

std::array<RgbaImage, 4> decode_genesis_field_menu_clips_trade_focus(
    const std::vector<std::uint8_t>& rom) {
    // 0x11E7E passes CD45A to the same writer for the nested trade actions.
    return decode_genesis_field_menu_focus_table(
        rom, 0xCD45AU, "Genesis Clips trade focus table");
}

RgbaImage decode_genesis_field_menu_posture_focus(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::size_t kFocusTableOffset = 0xCD514U;
    constexpr std::size_t kFocusWidth = 6U;
    constexpr std::size_t kFocusHeight = 1U;
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);
    auto focus = decode_genesis_menu_name_table_block(
        rom, pattern_vram, kFocusTableOffset, kFocusWidth, kFocusHeight,
        palette);
    for (auto& pixel : focus.pixels) {
        pixel.alpha = 255U;
    }
    return focus;
}

RgbaImage decode_genesis_field_menu_cursor(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::size_t kSpriteListOffset = 0xD29F6U;
    constexpr std::size_t kSpritePaletteMask = 0x2000U;
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    const auto source_size = read_rom_u16(
        rom, kSpriteListOffset + 4U,
        "Genesis command-screen cursor sprite is truncated");
    const auto source_tile = read_rom_u16(
        rom, kSpriteListOffset + 6U,
        "Genesis command-screen cursor tile is truncated");
    const auto size_link = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(source_size & 0x00FFU) << 8U | 0x0002U);
    const auto width_tiles = static_cast<std::size_t>(
        ((size_link >> 8U) & 0x03U) + 1U);
    const auto height_tiles = static_cast<std::size_t>(
        ((size_link >> 10U) & 0x03U) + 1U);
    const auto palette = read_genesis_command_palette_set(rom);
    RgbaImage image{
        static_cast<std::uint16_t>(width_tiles * 8U),
        static_cast<std::uint16_t>(height_tiles * 8U),
        std::vector<native::Rgba8>(width_tiles * height_tiles * 64U,
                                   native::Rgba8{0, 0, 0, 0})};
    const auto tile_attributes = static_cast<std::uint16_t>(
        source_tile | kSpritePaletteMask);
    const auto palette_line = static_cast<std::size_t>(
        (tile_attributes >> 13U) & 0x03U);
    const auto hflip = (tile_attributes & 0x0800U) != 0U;
    const auto vflip = (tile_attributes & 0x1000U) != 0U;
    for (std::size_t tile_y = 0; tile_y < height_tiles; ++tile_y) {
        for (std::size_t tile_x = 0; tile_x < width_tiles; ++tile_x) {
            const auto source_tile_x = hflip
                ? width_tiles - 1U - tile_x : tile_x;
            const auto source_tile_y = vflip
                ? height_tiles - 1U - tile_y : tile_y;
            const auto tile_index = static_cast<std::size_t>(source_tile) +
                source_tile_y * width_tiles + source_tile_x;
            if (tile_index * 32U + 32U > pattern_vram.size()) {
                throw std::out_of_range(
                    "Genesis command-screen cursor tile is truncated");
            }
            const auto tile = decode_genesis_tile(
                pattern_vram.data() + tile_index * 32U, 32U);
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                    const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                    const auto color_index = tile.pixels[
                        source_y * 8U + source_x];
                    auto color = palette[palette_line * 16U + color_index];
                    color.alpha = color_index == 0U ? 0U : 255U;
                    image.pixels[(tile_y * 8U + pixel_y) * image.width +
                                 tile_x * 8U + pixel_x] = color;
                }
            }
        }
    }
    return image;
}

std::array<RgbaImage, native::kNativeInventoryItemCount>
decode_genesis_field_menu_item_art(const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kItemTable = 0xD4540U;
    constexpr std::size_t kItemRecordStride = 0x30U;
    constexpr std::size_t kItemArtworkTableOffset = 0x1EU;
    constexpr std::size_t kItemArtworkWidth = 9U;
    constexpr std::size_t kItemArtworkHeight = 5U;
    constexpr std::size_t kArtworkPatternStreamOffset = 0xC8BB6U;
    constexpr std::size_t kArtworkPatternVramOffset = 0x5C80U;
    const auto standard_pattern_vram = decode_genesis_1530_vram(
        rom, kArtworkPatternStreamOffset, kArtworkPatternVramOffset);
    // ROM 0x12A54 selects the alternate item-art upload at 0xC9B7C when
    // the one-based item number reaches 13. The native catalog is
    // zero-based, so item-art-12 is the first resource in that bank.
    const auto alternate_pattern_vram = decode_genesis_1530_vram(
        rom, 0xC9B7CU, kArtworkPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);
    std::array<RgbaImage, native::kNativeInventoryItemCount> artwork{};
    for (std::size_t item_index = 0; item_index < artwork.size(); ++item_index) {
        const auto item_offset = kItemTable + item_index * kItemRecordStride;
        const auto table_offset = static_cast<std::size_t>(read_rom_u32(
            rom, item_offset + kItemArtworkTableOffset,
            "Genesis inventory artwork pointer is truncated"));
        if (table_offset == std::numeric_limits<std::uint32_t>::max()) {
            // The ROM uses a negative +0x1E pointer for catalog entries that
            // have no large detail image; 0x12A8C tests this before writing.
            artwork[item_index] = {
                static_cast<std::uint16_t>(kItemArtworkWidth * 8U),
                static_cast<std::uint16_t>(kItemArtworkHeight * 8U),
                std::vector<native::Rgba8>(
                    kItemArtworkWidth * kItemArtworkHeight * 64U,
                    native::Rgba8{0, 0, 0, 0})};
            continue;
        }
        const auto& pattern_vram = item_index < 12U
            ? standard_pattern_vram
            : alternate_pattern_vram;
        artwork[item_index] = decode_genesis_menu_name_table_block(
            rom, pattern_vram, table_offset, kItemArtworkWidth,
            kItemArtworkHeight, palette, 0x2000U);
    }
    return artwork;
}

namespace {

std::array<RgbaImage, 14> decode_genesis_field_menu_detail_bars(
    const std::vector<std::uint8_t>& rom, std::size_t table_offset,
    const char* table_name) {
    constexpr std::size_t kPatternStreamOffset = 0xC8BB6U;
    constexpr std::size_t kPatternVramOffset = 0x5C80U;
    constexpr std::size_t kBarCount = 14U;
    constexpr std::size_t kBarStride = 0x0EU;
    constexpr std::size_t kBarWidth = 7U;
    auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    // The command setup appends CF134 to VRAM at 5A80 immediately after the
    // CAB0C upload. CF84C's seven-tile strips use those appended tiles
    // (0x2DA..0x2E1), not the later 5C80 item-art upload.
    require_bytes(rom.size() - std::min<std::size_t>(0xCF134U, rom.size()),
                  0x200U, "Genesis command bar patterns are truncated");
    std::copy_n(rom.begin() + 0xCF134U, 0x200U,
                pattern_vram.begin() + 0x5A80U);
    const auto palette = read_genesis_command_palette_set(rom);
    std::array<RgbaImage, kBarCount> bars{};
    for (std::size_t index = 0; index < bars.size(); ++index) {
        bars[index] = decode_genesis_menu_name_table_block(
            rom, pattern_vram, table_offset + index * kBarStride,
            kBarWidth, 1U, palette, 0x2000U);
        if (bars[index].pixels.empty()) {
            throw std::runtime_error(table_name);
        }
    }
    return bars;
}

} // namespace

std::array<RgbaImage, 14> decode_genesis_field_menu_damage_bars(
    const std::vector<std::uint8_t>& rom) {
    // 0x12F70: D6*0x0E from CF84C, written at C52A with palette line one.
    return decode_genesis_field_menu_detail_bars(
        rom, 0xCF84CU, "Genesis damage bar table is truncated");
}

std::array<RgbaImage, 23> decode_genesis_field_menu_stat_bars(
    const std::vector<std::uint8_t>& rom, std::size_t table_offset,
    const char* table_name) {
    constexpr std::size_t kPatternStreamOffset = 0xCAB0CU;
    constexpr std::size_t kPatternVramOffset = 0x1E20U;
    constexpr std::size_t kBarCount = 23U;
    constexpr std::size_t kBarStride = 0x1AU;
    constexpr std::size_t kBarWidth = 13U;
    auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    // 0x10F50's final raw upload places CF134 at 5A80. The CF3D4/CF610
    // strips reference that block, so omitting it turns every source bar
    // into a black tile even though the name-word tables decode correctly.
    require_bytes(rom.size() - std::min<std::size_t>(0xCF134U, rom.size()),
                  0x200U, "Genesis command stat-bar patterns are truncated");
    std::copy_n(rom.begin() + 0xCF134U, 0x200U,
                pattern_vram.begin() + 0x5A80U);
    const auto palette = read_genesis_command_palette_set(rom);
    std::array<RgbaImage, kBarCount> bars{};
    for (std::size_t index = 0; index < bars.size(); ++index) {
        bars[index] = decode_genesis_menu_name_table_block(
            rom, pattern_vram, table_offset + index * kBarStride,
            kBarWidth, 1U, palette, 0x2000U);
        if (bars[index].pixels.empty()) {
            throw std::runtime_error(table_name);
        }
    }
    return bars;
}

std::array<RgbaImage, 14> decode_genesis_field_menu_drain_bars(
    const std::vector<std::uint8_t>& rom) {
    // 0x12FA4: D2*0x0E from CF8F0, written at C4AA with palette line one.
    return decode_genesis_field_menu_detail_bars(
        rom, 0xCF8F0U, "Genesis drain bar table is truncated");
}

std::array<RgbaImage, 256> decode_genesis_field_menu_detail_values(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPatternStreamOffset = 0xC8BB6U;
    constexpr std::size_t kPatternVramOffset = 0x5C80U;
    constexpr std::size_t kValueTableOffset = 0x6AC46U;
    constexpr std::size_t kEntryStride = 4U;

    // 0x12A8C uses the same command-screen pattern upload as the detail bars,
    // then sends exactly two name-table words with D7=1 and D1=0. The source
    // phase byte is an index, not a damage or percentage value.
    const auto pattern_vram = decode_genesis_1530_vram(
        rom, kPatternStreamOffset, kPatternVramOffset);
    const auto palette = read_genesis_command_palette_set(rom);
    std::array<RgbaImage, 256> values{};
    for (std::size_t index = 0; index < values.size(); ++index) {
        values[index] = decode_genesis_menu_name_table_block(
            rom, pattern_vram, kValueTableOffset + index * kEntryStride,
            2U, 1U, palette, 0x2000U);
    }
    return values;
}

std::array<std::string, 13> decode_genesis_field_menu_power_labels(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kTableOffset = 0xD1F5EU;
    constexpr std::size_t kEntryStride = 0x0CU;
    std::array<std::string, 13> labels{};
    for (std::size_t index = 0; index < labels.size(); ++index) {
        labels[index] = read_genesis_fixed_text(
            rom, kTableOffset + index * kEntryStride, kEntryStride,
            "Genesis power-label table is truncated");
    }
    return labels;
}

std::array<std::array<RgbaImage, 2>, native::kNativeInventoryItemCount>
decode_genesis_field_menu_inventory_icons(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kItemTable = 0xD4540U;
    constexpr std::size_t kItemRecordStride = 0x30U;
    constexpr std::size_t kIconTileSource = 0xCF97CU;
    // 0x11048 uploads CF97C through 0x0F5C as 0x1E00 bytes: 0xF0 packed
    // source tiles. The per-item six-word tables use entries through 0x9B.
    constexpr std::size_t kIconTileCount = 0xF0U;
    std::array<std::array<RgbaImage, 2>, native::kNativeInventoryItemCount>
        icons{};
    for (std::size_t item_index = 0; item_index < icons.size(); ++item_index) {
        const auto item_offset = kItemTable + item_index * kItemRecordStride;
        const auto icon_table = static_cast<std::size_t>(read_rom_u32(
            rom, item_offset + 0x1AU,
            "Genesis inventory icon pointer is truncated"));
        for (std::size_t palette_variant = 0; palette_variant < 2U;
             ++palette_variant) {
            icons[item_index][palette_variant] =
                decode_genesis_menu_tile_grid(
                    rom, kIconTileSource, kIconTileCount, icon_table, 6U,
                    3U, 2U, palette_variant == 0U ? 1U : 3U, true);
        }
    }
    return icons;
}

GenesisFieldModeResources read_genesis_field_mode_resources_at(
    const std::vector<std::uint8_t>& rom, std::size_t record_offset) {
    if (record_offset > rom.size() || rom.size() - record_offset < 0x58U) {
        throw std::out_of_range("Genesis field mode record is truncated");
    }

    const auto setup_block = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x34U,
        "Genesis mode-2 setup pointer is truncated"));
    if (setup_block > rom.size() || rom.size() - setup_block < 0x20U) {
        throw std::out_of_range("Genesis mode-2 setup block is truncated");
    }

    const auto width_bytes = static_cast<std::size_t>(read_rom_u16(
        rom, record_offset + 0x24U,
        "Genesis mode-2 tilemap stride is truncated"));
    if (width_bytes == 0 || (width_bytes & 1U) != 0) {
        throw std::runtime_error(
            "Genesis mode-2 tilemap stride is not word-aligned");
    }
    const auto tilemap_width = width_bytes / 2U;

    const auto primary_tilemap = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x14U,
        "Genesis mode-2 primary tilemap pointer is truncated"));
    const auto secondary_tilemap = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x1CU,
        "Genesis mode-2 secondary tilemap pointer is truncated"));
    if (primary_tilemap > rom.size() || secondary_tilemap > rom.size()) {
        throw std::out_of_range("Genesis mode-2 tilemaps are outside the ROM");
    }
    const auto lower_tilemap = std::min(primary_tilemap, secondary_tilemap);
    const auto upper_tilemap = std::max(primary_tilemap, secondary_tilemap);
    if (lower_tilemap == upper_tilemap ||
        (upper_tilemap - lower_tilemap) % 2U != 0) {
        throw std::runtime_error(
            "Genesis field mode tilemap pointers do not describe equal maps");
    }
    auto tilemap_entries = (upper_tilemap - lower_tilemap) / 2U;

    const auto metatile_stream = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x18U,
        "Genesis mode-2 metatile pointer is truncated")) &
        ~std::size_t{1};
    // Modes 6..9 store a separator row between their two 60-column layer
    // tables. The mode descriptors all share 0x141D20 as the following
    // name-word resource, so the pointer gap includes that one row even
    // though it is not part of either visible map.
    if (metatile_stream == 0x141D20U && tilemap_entries > tilemap_width) {
        tilemap_entries -= tilemap_width;
    }
    if (tilemap_entries == 0 || tilemap_entries % tilemap_width != 0) {
        throw std::runtime_error(
            "Genesis field mode tilemap size does not match its stride");
    }

    // The large field sets point at direct name-word tables. These include
    // the shared mode-6..9 table, mode 20, mode 26, and modes 44..54; the
    // remaining mode descriptors use the 0x1530 resource stream.
    const auto metatile_encoding =
        metatile_stream == 0x141D20U || metatile_stream == 0x155432U ||
        metatile_stream == 0x1A84B2U || metatile_stream == 0x1EB62EU ||
        metatile_stream == 0x1CD29AU
            ? GenesisMetatileEncoding::RawNameWords
            : GenesisMetatileEncoding::CompressedResource;

    GenesisFieldModeResources result{
        record_offset,
        read_rom_u16(rom, record_offset,
                     "Genesis mode-2 camera X is truncated"),
        read_rom_u16(rom, record_offset + 2U,
                     "Genesis mode-2 camera Y is truncated"),
        {{
            GenesisFieldRecordPosition{
                read_rom_u16(rom, record_offset + 0x42U,
                             "Genesis mode-2 first field-record X is truncated"),
                read_rom_u16(rom, record_offset + 0x44U,
                             "Genesis mode-2 first field-record Y is truncated")},
            GenesisFieldRecordPosition{
                read_rom_u16(rom, record_offset + 0x46U,
                             "Genesis mode-2 second field-record X is truncated"),
                read_rom_u16(rom, record_offset + 0x48U,
                             "Genesis mode-2 second field-record Y is truncated")},
            GenesisFieldRecordPosition{
                read_rom_u16(rom, record_offset + 0x4AU,
                             "Genesis mode-2 third field-record X is truncated"),
                read_rom_u16(rom, record_offset + 0x4CU,
                             "Genesis mode-2 third field-record Y is truncated")}
        }},
        read_rom_u16(rom, record_offset + 0x0CU,
                     "Genesis mode-2 terrain X bound is truncated"),
        read_rom_u16(rom, record_offset + 0x0EU,
                     "Genesis mode-2 terrain Y bound is truncated"),
        read_rom_u16(rom, record_offset + 0x10U,
                     "Genesis mode-2 terrain X minimum is truncated"),
        read_rom_u16(rom, record_offset + 0x12U,
                     "Genesis mode-2 terrain Y minimum is truncated"),
        static_cast<std::size_t>(read_rom_u32(
            rom, record_offset + 0x26U,
            "Genesis mode-2 terrain record pointer is truncated")),
        setup_block,
        static_cast<std::size_t>(read_rom_u32(
            rom, setup_block + 4U,
            "Genesis mode-2 palette pointer is truncated")),
        static_cast<std::size_t>(read_rom_u32(
            rom, setup_block + 0x10U,
            "Genesis mode-2 pattern pointer is truncated")) & ~std::size_t{1},
        static_cast<std::size_t>(read_rom_u32(
            rom, setup_block + 0x18U,
            "Genesis mode-2 pattern length is truncated")),
        metatile_stream,
        primary_tilemap,
        secondary_tilemap,
        tilemap_width,
        tilemap_entries / tilemap_width,
        metatile_encoding,
    };
    result.event_table_offset = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x4EU,
        "Genesis field mode event table pointer is truncated"));
    if (result.event_table_offset != 0) {
        if (result.event_table_offset > rom.size() ||
            rom.size() - result.event_table_offset <
                result.event_mode_table.size()) {
            throw std::out_of_range(
                "Genesis field mode event table is truncated");
        }
        std::copy_n(rom.begin() +
                        static_cast<std::ptrdiff_t>(result.event_table_offset),
                    result.event_mode_table.size(),
                    result.event_mode_table.begin());
    }
    return result;
}

GenesisMode2FieldResources read_genesis_mode2_field_resources(
    const std::vector<std::uint8_t>& rom, std::size_t record_offset) {
    return read_genesis_field_mode_resources_at(rom, record_offset);
}

GenesisFieldModeResources read_genesis_field_mode_resources(
    const std::vector<std::uint8_t>& rom, std::uint16_t mode_id) {
    if (mode_id >= rom::kFieldModeCount) {
        throw std::out_of_range(
            "Genesis field mode id is outside the mode table");
    }
    const auto table_entry = static_cast<std::size_t>(
        rom::kFieldModePointerTable) + static_cast<std::size_t>(mode_id) * 4U;
    const auto record_offset = static_cast<std::size_t>(read_rom_u32(
        rom, table_entry, "Genesis field mode pointer table is truncated"));
    return read_genesis_field_mode_resources_at(rom, record_offset);
}

GenesisTerrainMap decode_genesis_mode2_terrain_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources) {
    GenesisTerrainMap terrain{};
    terrain.x_exclusive_upper = resources.terrain_x_exclusive_upper;
    terrain.y_exclusive_upper = resources.terrain_y_exclusive_upper;
    terrain.x_inclusive_lower = resources.terrain_x_inclusive_lower;
    terrain.y_inclusive_lower = resources.terrain_y_inclusive_lower;
    terrain.statuses.assign(GenesisTerrainMap::kWidth *
                                GenesisTerrainMap::kHeight,
                            0);
    auto cursor = resources.terrain_record_offset;
    for (;;) {
        const auto x_start = read_rom_u16(
            rom, cursor, "Genesis mode-2 terrain record is truncated");
        if ((x_start & 0x8000U) != 0) {
            return terrain;
        }
        const auto x_end = read_rom_u16(
            rom, cursor + 2U,
            "Genesis mode-2 terrain X range is truncated");
        const auto y_start = read_rom_u16(
            rom, cursor + 4U,
            "Genesis mode-2 terrain Y range is truncated");
        const auto y_end = read_rom_u16(
            rom, cursor + 6U,
            "Genesis mode-2 terrain Y range is truncated");
        const auto status = read_rom_u16(
            rom, cursor + 8U,
            "Genesis mode-2 terrain status is truncated");
        const auto x_range_inverted = x_end < x_start;
        const auto y_range_inverted = y_end < (y_start & 0xFFE0U);
        terrain.regions.push_back({
            x_start, x_end, y_start, y_end,
            static_cast<std::uint8_t>(status)});

        // ROM 0xEE10 retains the five-word record and advances to the next
        // one even when either DBF-style range test has no iterations. Mode
        // 0x33 contains one such intentional empty Y range; do not invent a
        // rectangle or reject the remaining native area because of it.
        if (x_range_inverted || y_range_inverted) {
            ++terrain.region_count;
            cursor += 10U;
            continue;
        }

        auto y_value = static_cast<std::uint32_t>(y_start & 0xFFE0U);
        for (;;) {
            const auto row = y_value / 0x20U;
            if (row >= GenesisTerrainMap::kHeight) {
                throw std::out_of_range(
                    "Genesis mode-2 terrain row is outside the grid");
            }
            auto x_value = static_cast<std::uint32_t>(x_start);
            auto column = static_cast<std::size_t>(x_start >> 5U);
            for (;;) {
                if (column >= GenesisTerrainMap::kWidth) {
                    throw std::out_of_range(
                        "Genesis mode-2 terrain column is outside the grid");
                }
                terrain.statuses[row * GenesisTerrainMap::kWidth + column] =
                    static_cast<std::uint8_t>(status);
                if (x_value + 0x20U > x_end) {
                    break;
                }
                x_value += 0x20U;
                ++column;
            }
            if (y_value + 0x20U > y_end) {
                break;
            }
            y_value += 0x20U;
        }
        ++terrain.region_count;
        cursor += 10U;
    }
}

GenesisTerrainMap decode_genesis_field_mode_terrain_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisFieldModeResources& resources) {
    return decode_genesis_mode2_terrain_map(rom, resources);
}

RgbaImage decode_tile_atlas(
    const std::vector<std::uint8_t>& rom,
    std::size_t tile_byte_offset,
    std::size_t tile_count,
    const std::array<native::Rgba8, 16>& palette,
    std::size_t columns,
    int transparent_index) {
    if (columns == 0 || columns > std::numeric_limits<std::uint16_t>::max()) {
        throw std::invalid_argument("tile atlas column count is invalid");
    }
    if (tile_count > std::numeric_limits<std::uint16_t>::max() / 8U) {
        throw std::invalid_argument("tile atlas is too large");
    }
    const auto rows = (tile_count + columns - 1U) / columns;
    if (rows > std::numeric_limits<std::uint16_t>::max() / 8U) {
        throw std::invalid_argument("tile atlas is too large");
    }
    if (tile_byte_offset > rom.size() ||
        tile_count > (rom.size() - tile_byte_offset) / 32U) {
        throw std::out_of_range("tile atlas is truncated");
    }

    RgbaImage image{
        static_cast<std::uint16_t>(columns * 8U),
        static_cast<std::uint16_t>(rows * 8U),
        std::vector<native::Rgba8>(columns * rows * 64U),
    };
    const native::Rgba8 transparent{0, 0, 0, 0};
    for (std::size_t tile_index = 0; tile_index < tile_count; ++tile_index) {
        const auto tile = decode_genesis_tile(
            rom.data() + tile_byte_offset + tile_index * 32U, 32U);
        const auto tile_x = (tile_index % columns) * 8U;
        const auto tile_y = (tile_index / columns) * 8U;
        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                const auto index = tile.pixels[pixel_y * 8U + pixel_x];
                image.pixels[(tile_y + pixel_y) * image.width +
                             tile_x + pixel_x] =
                    index == transparent_index ? transparent : palette[index];
            }
        }
    }
    return image;
}

RgbaImage decode_genesis_mode2_field_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources) {
    return decode_genesis_mode2_field_map(
        rom, resources.pattern_data_offset, resources.pattern_byte_count,
        resources.metatile_stream_offset, resources.primary_tilemap_offset,
        resources.secondary_tilemap_offset, resources.tilemap_width,
        resources.tilemap_height, resources.palette_offset,
        resources.metatile_encoding);
}

RgbaImage decode_genesis_field_mode_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisFieldModeResources& resources) {
    return decode_genesis_mode2_field_map(rom, resources);
}

RgbaImage decode_genesis_mode2_field_map(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_data_offset,
    std::size_t pattern_byte_count,
    std::size_t metatile_stream_offset,
    std::size_t primary_tilemap_offset,
    std::size_t secondary_tilemap_offset,
    std::size_t tilemap_width,
    std::size_t tilemap_height,
    std::size_t palette_offset,
    GenesisMetatileEncoding metatile_encoding) {
    if (tilemap_width == 0 || tilemap_height == 0) {
        throw std::invalid_argument("Genesis field map dimensions are invalid");
    }
    if (tilemap_width > std::numeric_limits<std::size_t>::max() / 32U ||
        tilemap_height > std::numeric_limits<std::size_t>::max() / 32U) {
        throw std::invalid_argument("Genesis field map dimensions overflow");
    }
    const auto output_width = tilemap_width * 32U;
    const auto output_height = tilemap_height * 32U;
    if (output_width > std::numeric_limits<std::uint16_t>::max() ||
        output_height > std::numeric_limits<std::uint16_t>::max()) {
        throw std::invalid_argument("Genesis field map is too large");
    }
    if (tilemap_width > std::numeric_limits<std::size_t>::max() /
                           tilemap_height) {
        throw std::invalid_argument("Genesis field map dimensions overflow");
    }
    const auto map_entries = tilemap_width * tilemap_height;
    const auto require_map = [&](std::size_t offset, const char* what) {
        if (offset > rom.size() ||
            map_entries > (rom.size() - offset) / 2U) {
            throw std::out_of_range(what);
        }
    };
    require_map(primary_tilemap_offset, "Genesis primary field tilemap is truncated");
    require_map(secondary_tilemap_offset,
                "Genesis secondary field tilemap is truncated");

    if (pattern_data_offset > rom.size() ||
        pattern_byte_count > rom.size() - pattern_data_offset ||
        pattern_byte_count < 32U || pattern_byte_count % 32U != 0) {
        throw std::runtime_error(
            "Genesis field pattern upload is not tile-aligned");
    }
    const auto tile_count = pattern_byte_count / 32U;
    std::vector<std::uint8_t> metatile_resource;
    std::size_t metatile_count = 0;
    if (metatile_encoding == GenesisMetatileEncoding::CompressedResource) {
        metatile_resource = decode_genesis_resource_stream(
            rom, metatile_stream_offset);
        if (metatile_resource.empty() || metatile_resource.size() % 32U != 0) {
            throw std::runtime_error(
                "Genesis field metatiles are not stamp-aligned");
        }
        metatile_count = metatile_resource.size() / 32U;
    }
    std::vector<IndexedTile> tiles;
    tiles.reserve(tile_count);
    for (std::size_t tile_index = 0; tile_index < tile_count; ++tile_index) {
        // 0x1406 uploads this source directly to VRAM at 0x4020. Genesis
        // patterns are packed nibbles: the nibble expansion at 0x3E86 is
        // applied to entity sources, while the field stream is already in
        // that same four-pixels-per-byte layout.
        tiles.push_back(decode_genesis_tile(
            rom.data() + pattern_data_offset + tile_index * 32U, 32U));
    }

    const auto palette = read_genesis_field_palette_set(rom, palette_offset);
    const auto read_map_word = [&](std::size_t offset,
                                   std::size_t entry_index) {
        const auto word_offset = offset + entry_index * 2U;
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[word_offset]) << 8U) |
            rom[word_offset + 1U]);
    };
    const auto read_metatile_word = [&](std::size_t metatile_index,
                                        std::size_t word_index) {
        if (metatile_encoding == GenesisMetatileEncoding::RawNameWords) {
            const auto word_offset = metatile_stream_offset +
                                     (metatile_index * 16U + word_index) * 2U;
            if (word_offset > rom.size() || rom.size() - word_offset < 2U) {
                throw std::out_of_range(
                    "Genesis raw field metatile table is truncated");
            }
            return static_cast<std::uint16_t>(
                (static_cast<std::uint16_t>(rom[word_offset]) << 8U) |
                rom[word_offset + 1U]);
        }
        const auto word_offset = metatile_index * 32U + word_index * 2U;
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(metatile_resource[word_offset]) << 8U) |
            metatile_resource[word_offset + 1U]);
    };
    struct LayerPixel {
        native::Rgba8 color{0, 0, 0, 255};
        bool high_priority = false;
        bool visible = false;
    };
    std::vector<LayerPixel> primary_layer(output_width * output_height);
    std::vector<LayerPixel> secondary_layer(output_width * output_height);

    const auto render_layer = [&](std::size_t tilemap_offset,
                                  std::uint16_t priority_mask,
                                  std::vector<LayerPixel>& layer) {
        for (std::size_t stamp_y = 0; stamp_y < tilemap_height; ++stamp_y) {
            for (std::size_t stamp_x = 0; stamp_x < tilemap_width; ++stamp_x) {
                const auto metatile_index = static_cast<std::size_t>(
                    read_map_word(tilemap_offset,
                                  stamp_y * tilemap_width + stamp_x));
                if (metatile_encoding ==
                        GenesisMetatileEncoding::CompressedResource &&
                    metatile_index >= metatile_count) {
                    throw std::out_of_range(
                        "Genesis field metatile index is truncated (index " +
                        std::to_string(metatile_index) + ", count " +
                        std::to_string(metatile_count) + ")");
                }
                for (std::size_t tile_y = 0; tile_y < 4U; ++tile_y) {
                    for (std::size_t tile_x = 0; tile_x < 4U; ++tile_x) {
                        auto name_word = read_metatile_word(
                            metatile_index, tile_y * 4U + tile_x);
                        // ROM 0x3D80 writes Plane A with FFDF58 clear, then
                        // writes Plane B with FFDF58=8000. Its negative path
                        // uses EORI #8000 and its positive path uses OR, so
                        // the authored secondary name word is toggled rather
                        // than unconditionally forced high-priority.
                        if ((priority_mask & 0x8000U) != 0U) {
                            name_word = static_cast<std::uint16_t>(
                                name_word ^ priority_mask);
                        }
                        const auto pattern_word =
                            static_cast<std::int32_t>(name_word & 0x07FFU) -
                            0x0201;
                        if (pattern_word < 0 ||
                            static_cast<std::size_t>(pattern_word) >= tile_count) {
                            throw std::out_of_range(
                                "Genesis field pattern index is truncated (name " +
                                std::to_string(name_word) + ", relative " +
                                std::to_string(pattern_word) + ", tiles " +
                                std::to_string(tile_count) + ", metatile " +
                                std::to_string(metatile_index) + ", word " +
                                std::to_string(tile_y * 4U + tile_x) +
                                ", stream " +
                                std::to_string(metatile_stream_offset) +
                                ", map " + std::to_string(tilemap_offset) +
                                ")");
                        }
                        const auto& tile = tiles[
                            static_cast<std::size_t>(pattern_word)];
                        const auto palette_line = static_cast<std::size_t>(
                            (name_word >> 13U) & 0x03U);
                        const auto hflip = (name_word & 0x0800U) != 0;
                        const auto vflip = (name_word & 0x1000U) != 0;
                        const auto origin_x = stamp_x * 32U + tile_x * 8U;
                        const auto origin_y = stamp_y * 32U + tile_y * 8U;
                        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                                const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                                const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                                const auto color_index = tile.pixels[
                                    source_y * 8U + source_x];
                                auto color = palette[palette_line * 16U +
                                                      color_index];
                                // The second map is the authored overlay
                                // plane. Its zero pattern is the empty cell
                                // used by the game, so it must not erase the
                                // primary map after being translated to an
                                // RGBA surface. The priority bit still
                                // decides which authored nonzero tile wins.
                                if ((priority_mask & 0x8000U) != 0U &&
                                    color_index == 0U) {
                                    continue;
                                }
                                color.alpha = 255;
                                auto& destination = layer[
                                    (origin_y + pixel_y) * output_width +
                                    origin_x + pixel_x];
                                destination.color = color;
                                destination.high_priority =
                                    (name_word & 0x8000U) != 0U;
                                destination.visible = true;
                            }
                        }
                    }
                }
            }
        }
    };
    std::size_t required_metatile_count = 0;
    const auto scan_metatile_indices = [&](std::size_t tilemap_offset) {
        for (std::size_t entry = 0; entry < map_entries; ++entry) {
            required_metatile_count = std::max(
                required_metatile_count,
                static_cast<std::size_t>(read_map_word(tilemap_offset, entry)) +
                    1U);
        }
    };
    scan_metatile_indices(primary_tilemap_offset);
    scan_metatile_indices(secondary_tilemap_offset);
    if (metatile_encoding == GenesisMetatileEncoding::RawNameWords) {
        if (metatile_stream_offset > rom.size() ||
            required_metatile_count >
                (rom.size() - metatile_stream_offset) / 32U) {
            throw std::out_of_range(
                "Genesis raw field metatile table is truncated");
        }
    }
    render_layer(primary_tilemap_offset, 0U, primary_layer);
    render_layer(secondary_tilemap_offset, 0x8000U, secondary_layer);
    RgbaImage image{
        static_cast<std::uint16_t>(output_width),
        static_cast<std::uint16_t>(output_height),
        std::vector<native::Rgba8>(output_width * output_height),
    };
    for (std::size_t index = 0; index < image.pixels.size(); ++index) {
        // Genesis resolves a high-priority tile ahead of a low-priority tile;
        // with equal priority Plane A is the front plane. The reviewed ROM
        // forces the secondary map high, so its authored overlay wins unless
        // the primary map explicitly authored a high-priority tile too.
        const auto& primary = primary_layer[index];
        const auto& secondary = secondary_layer[index];
        if (!secondary.visible) {
            image.pixels[index] = primary.color;
        } else if (!primary.visible) {
            image.pixels[index] = secondary.color;
        } else {
            image.pixels[index] =
                primary.high_priority != secondary.high_priority
                    ? secondary.high_priority ? secondary.color
                                               : primary.color
                    : primary.color;
        }
    }
    return image;
}

RgbaImage decode_genesis_entity_frame(
    const std::vector<std::uint8_t>& rom,
    std::size_t descriptor_offset,
    std::size_t nibble_table_offset,
    std::size_t palette_offset,
    bool expand_packed_source,
    int palette_line,
    bool compose_field_palette) {
    require_bytes(rom.size() - std::min(descriptor_offset, rom.size()), 0x10U,
                  "Genesis entity frame descriptor is truncated");
    const auto read16 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2U, what);
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };
    const auto read32 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 4U, what);
        return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
               (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(rom[offset + 3U]);
    };

    if (read16(descriptor_offset, "Genesis entity frame marker is truncated") !=
        0x8002U) {
        throw std::runtime_error(
            "Genesis entity descriptor is not an 0x8002 frame");
    }
    const auto source = static_cast<std::size_t>(read32(
        descriptor_offset + 2U, "Genesis entity frame source is truncated"));
    const auto length = static_cast<std::size_t>(read16(
        descriptor_offset + 6U, "Genesis entity frame length is truncated"));
    const auto dimensions = read16(
        descriptor_offset + 0x0CU,
        "Genesis entity frame dimensions are truncated");
    const auto columns = static_cast<std::size_t>((dimensions >> 2U) + 1U);
    const auto rows = static_cast<std::size_t>((dimensions & 3U) + 1U);
    const auto expected_length = columns * rows * 32U;
    if (length != expected_length) {
        throw std::runtime_error(
            "Genesis entity frame length does not match its dimensions");
    }
    if (source > rom.size() || length > rom.size() - source) {
        throw std::out_of_range("Genesis entity frame source is truncated");
    }
    require_bytes(rom.size() - std::min(nibble_table_offset, rom.size()),
                  16U, "Genesis entity nibble table is truncated");

    // Bake the CRAM line whose USED indices carry runner chroma (red jacket,
    // skin) into RGBA. The source's 0x3E86 preprocessing is conditional on
    // record +0x11 bit 5; field Joshua records take the raw-byte path.
    const auto palette_set = compose_field_palette
        ? read_genesis_field_palette_set(rom, palette_offset)
        : read_genesis_palette_set(rom, palette_offset);
    std::vector<std::uint8_t> decoded(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto packed = rom[source + index];
        if (!expand_packed_source) {
            decoded[index] = packed;
            continue;
        }
        const auto low = rom[nibble_table_offset + (packed & 0x0FU)];
        const auto high = rom[nibble_table_offset + (packed >> 4U)];
        decoded[index] = static_cast<std::uint8_t>(low | (high << 4U));
    }

    RgbaImage image{
        static_cast<std::uint16_t>(columns * 8U),
        static_cast<std::uint16_t>(rows * 8U),
        std::vector<native::Rgba8>(columns * rows * 64U,
                                   native::Rgba8{0, 0, 0, 0}),
    };
    std::array<std::uint8_t, 16> used_index{};
    std::vector<std::uint8_t> indices(image.pixels.size(), 0);
    // 0x3EFE consumes the generated tile buffer column first, then row.
    for (std::size_t column = 0; column < columns; ++column) {
        for (std::size_t row = 0; row < rows; ++row) {
            const auto tile_index = column * rows + row;
            const auto tile = decode_genesis_tile(
                decoded.data() + tile_index * 32U, 32U);
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto color_index =
                        tile.pixels[pixel_y * 8U + pixel_x];
                    const auto dest =
                        (row * 8U + pixel_y) * image.width +
                        column * 8U + pixel_x;
                    indices[dest] = color_index;
                    if (color_index < used_index.size()) {
                        used_index[color_index] = 1;
                    }
                }
            }
        }
    }
    std::size_t sprite_line = 1U;
    if (palette_line >= 0) {
        if (palette_line >= 4) {
            throw std::out_of_range("Genesis entity palette line is invalid");
        }
        sprite_line = static_cast<std::size_t>(palette_line);
    } else {
        int best_chroma = -1;
        for (std::size_t line = 0; line < 4U; ++line) {
            int chroma = 0;
            for (std::size_t index = 1; index < 16U; ++index) {
                if (used_index[index] == 0) {
                    continue;
                }
                const auto& color = palette_set[line * 16U + index];
                const auto maxc = std::max({color.red, color.green, color.blue});
                const auto minc = std::min({color.red, color.green, color.blue});
                chroma += static_cast<int>(maxc - minc);
                if (static_cast<int>(color.red) >
                        static_cast<int>(color.green) + 20 &&
                    static_cast<int>(color.red) >
                        static_cast<int>(color.blue) + 20) {
                    chroma += 80;
                }
            }
            if (chroma > best_chroma) {
                best_chroma = chroma;
                sprite_line = line;
            }
        }
    }
    for (std::size_t dest = 0; dest < indices.size(); ++dest) {
        const auto color_index = indices[dest];
        auto color = palette_set[sprite_line * 16U + color_index];
        if (color_index == 0) {
            color.alpha = 0;
        }
        image.pixels[dest] = color;
    }
    return image;
}

std::vector<RgbaImage> decode_genesis_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t state_block_offset,
    std::size_t nibble_table_offset,
    std::size_t palette_offset) {
    require_bytes(rom.size() - std::min(state_block_offset, rom.size()), 4U,
                  "Genesis entity animation state block is truncated");
    const auto read16 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2U, what);
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };
    const auto read32 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 4U, what);
        return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
               (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(rom[offset + 3U]);
    };

    const auto state_marker = read16(
        state_block_offset, "Genesis entity animation marker is truncated");
    if ((state_marker & 0x8000U) == 0) {
        throw std::runtime_error(
            "Genesis entity animation state block has no terminal marker");
    }
    const auto frame_count = static_cast<std::size_t>(state_marker & 0x7FFFU);
    if (frame_count == 0 || frame_count >
        (rom.size() - std::min(state_block_offset, rom.size()) - 4U) / 4U) {
        throw std::runtime_error(
            "Genesis entity animation frame table is truncated");
    }

    std::vector<RgbaImage> frames;
    frames.reserve(frame_count);
    for (std::size_t index = 0; index < frame_count; ++index) {
        const auto descriptor = static_cast<std::size_t>(read32(
            state_block_offset + 4U + index * 4U,
            "Genesis entity animation descriptor is truncated"));
        frames.push_back(decode_genesis_entity_frame(
            rom, descriptor, nibble_table_offset, palette_offset));
    }
    return frames;
}

std::vector<RgbaImage> decode_genesis_field_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset,
    std::size_t entity_state_pointer_table_offset,
    std::size_t nibble_table_offset,
    std::size_t palette_offset,
    std::size_t state_count) {
    const auto read16 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2U, what);
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };
    const auto read32 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 4U, what);
        return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
               (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(rom[offset + 3U]);
    };

    const auto entity_kind = read16(
        entity_record_offset + 0x0EU,
        "Genesis field entity kind is truncated");
    const auto first_pointer = static_cast<std::size_t>(read32(
        entity_state_pointer_table_offset +
            static_cast<std::size_t>(entity_kind) * 4U,
        "Genesis field entity state pointer is truncated"));
    const auto field_palette_line = static_cast<int>(
        (read16(entity_record_offset + 0x2CU,
                "Genesis field entity palette attributes are truncated") >>
         13U) & 0x03U);
    const auto state_selector = static_cast<std::size_t>(read16(
        entity_record_offset + 0x16U,
        "Genesis field entity state selector is truncated") & 0x7FFFU);
    const auto selected_state_pointer = static_cast<std::size_t>(read32(
        first_pointer + state_selector,
        "Genesis field entity state list is truncated"));
    const auto record_state_index = read16(
        entity_record_offset + 0x1EU,
        "Genesis field entity state index is truncated");
    const auto selected_state_marker = read16(
        selected_state_pointer,
        "Genesis field entity state marker is truncated");
    if (static_cast<std::int16_t>(selected_state_marker) < 0) {
        throw std::runtime_error(
            "Genesis field entity starts in an unsupported terminal state");
    }

    if (state_count == 0 || state_count > 256) {
        throw std::runtime_error(
            "Genesis field entity state count is outside the supported range");
    }

    std::vector<RgbaImage> frames;
    frames.reserve(state_count);
    for (std::size_t index = 0; index < state_count; ++index) {
        // The outer table is a longword table at the address selected by the
        // kind. Its entries point to independent state lists; they are not
        // eight adjacent frames in one list. ROM 0x600E uses the record's
        // +0x1E index, resetting it to zero when it reaches a list's marker.
        const auto state_list = static_cast<std::size_t>(read32(
            first_pointer + index * 4U,
            "Genesis field entity state-pointer table is truncated"));
        const auto state_marker = read16(
            state_list, "Genesis field entity state-list marker is truncated");
        if (static_cast<std::int16_t>(state_marker) < 0) {
            throw std::runtime_error(
                "Genesis field entity state-pointer table terminates early");
        }
        const auto frame_index = record_state_index >= state_marker
            ? 0U : static_cast<std::size_t>(record_state_index);
        const auto descriptor = static_cast<std::size_t>(read32(
            state_list + 4U + frame_index * 4U,
            "Genesis field entity frame descriptor is truncated"));
        if (read16(descriptor,
                   "Genesis field entity descriptor is truncated") !=
            0x8002U) {
            throw std::runtime_error(
                "Genesis field entity does not resolve to an 0x8002 frame");
        }
        frames.push_back(decode_genesis_entity_frame(
            rom, descriptor, nibble_table_offset, palette_offset, false,
            field_palette_line, true));
    }
    return frames;
}

std::vector<GenesisFieldEntityAnimation>
decode_genesis_field_entity_animation_sequences(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset,
    std::size_t entity_state_pointer_table_offset,
    std::size_t animation_frame_table_offset,
    std::size_t nibble_table_offset,
    std::size_t palette_offset,
    std::size_t state_count) {
    const auto read16 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2U, what);
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };
    const auto read32 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 4U, what);
        return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
               (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(rom[offset + 3U]);
    };

    if (state_count == 0 || state_count > 256U) {
        throw std::invalid_argument(
            "Genesis field entity animation direction count is invalid");
    }
    const auto entity_kind = read16(
        entity_record_offset + 0x0EU,
        "Genesis field entity kind is truncated");
    const auto first_pointer = static_cast<std::size_t>(read32(
        entity_state_pointer_table_offset +
            static_cast<std::size_t>(entity_kind) * 4U,
        "Genesis field entity animation pointer is truncated"));
    const auto source_timing_value = read16(
        first_pointer + 0x00CAU,
        "Genesis field animation timing value is truncated");
    const auto field_palette_line = static_cast<int>(
        (read16(entity_record_offset + 0x2CU,
                "Genesis field entity palette attributes are truncated") >>
         13U) & 0x03U);

    std::vector<GenesisFieldEntityAnimation> animations;
    animations.reserve(state_count);
    for (std::size_t direction = 0; direction < state_count; ++direction) {
        GenesisFieldEntityAnimation animation;
        animation.state_selector = read16(
            animation_frame_table_offset + direction * 2U,
            "Genesis field animation selector table is truncated");
        const auto state_list = static_cast<std::size_t>(read32(
            first_pointer + animation.state_selector,
            "Genesis field animation state list is truncated"));
        animation.state_marker = read16(
            state_list, "Genesis field animation marker is truncated");
        if (static_cast<std::int16_t>(animation.state_marker) >= 0) {
            throw std::runtime_error(
                "Genesis field walking animation has no terminal marker");
        }
        const auto frame_count = static_cast<std::size_t>(
            animation.state_marker & 0x7FFFU);
        if (frame_count == 0 || frame_count > 256U) {
            throw std::runtime_error(
                "Genesis field walking animation frame count is invalid");
        }

        // The reviewed kind-3 lists are laid out as:
        //   0x8002, auxiliary word, descriptor pointer[0..1].
        // This is distinct from the positive-marker lists handled by the
        // legacy single-frame decoder above.
        animation.state_auxiliary_word = read16(
            state_list + 2U,
            "Genesis field animation auxiliary word is truncated");
        animation.source_timing_value = source_timing_value;
        // ROM 0x600E selects the walking cadence in the live record: the
        // source routine writes 0x0010 to +0x18 and 0x1C remains the 0x1000
        // threshold consumed by 0x5FDC. The 0x0300 word at state_list+2 is
        // list metadata, not the animation clock; treating it as the
        // multiplier made the native walker advance the wrong state.
        animation.timing_multiplier = 0x0010U;
        animation.frames.reserve(frame_count);
        for (std::size_t frame = 0; frame < frame_count; ++frame) {
            const auto descriptor = static_cast<std::size_t>(read32(
                state_list + 4U + frame * 4U,
                "Genesis field animation descriptor is truncated"));
            if (read16(descriptor,
                       "Genesis field animation descriptor header is truncated") !=
                0x8002U) {
                throw std::runtime_error(
                    "Genesis field walking animation descriptor is invalid");
            }
            animation.frames.push_back(decode_genesis_entity_frame(
                rom, descriptor, nibble_table_offset, palette_offset, false,
                field_palette_line, true));
        }
        animations.push_back(std::move(animation));
    }
    return animations;
}

namespace {

struct GenesisStartupPatternData {
    std::vector<std::uint8_t> pattern_vram;
    std::array<native::Rgba8, 64> palette{};
};

GenesisStartupPatternData decode_genesis_startup_pattern_data(
    const std::vector<std::uint8_t>& rom, std::size_t pattern_stream_offset,
    std::size_t palette_offset) {
    constexpr std::size_t kPatternVramSize = 0x10000;
    constexpr std::size_t kPatternVramOffset = 0x20;
    constexpr std::size_t kPatternStream2Offset = 0x67CB0;
    constexpr std::size_t kPatternStream2VramOffset = 0x4B20;
    constexpr std::size_t kPatternStream3Offset = 0x6875C;
    constexpr std::size_t kPatternStream3VramOffset = 0x5020;
    constexpr std::size_t kFontPatternStreamOffset = 0xC1064;
    // Keep the host VRAM byte placement used by the startup font/cursor
    // decoders. The source's E9C command packing and register-4 pattern base
    // are resolved separately when name words are mapped to host tiles.
    constexpr std::size_t kFontPatternVramOffset = 0xB560;

    GenesisStartupPatternData result{
        std::vector<std::uint8_t>(kPatternVramSize, 0),
        read_genesis_palette_set(rom, palette_offset),
    };
    const auto copy_pattern_stream = [&](std::size_t source_offset,
                                         std::size_t target_offset) {
        write_genesis_1530_vram(rom, source_offset, target_offset,
                                 result.pattern_vram);
    };
    copy_pattern_stream(pattern_stream_offset, kPatternVramOffset);
    copy_pattern_stream(kPatternStream2Offset, kPatternStream2VramOffset);
    copy_pattern_stream(kPatternStream3Offset, kPatternStream3VramOffset);
    copy_pattern_stream(kFontPatternStreamOffset, kFontPatternVramOffset);
    return result;
}

std::vector<std::uint8_t> read_genesis_startup_line(
    const std::vector<std::uint8_t>& rom, std::size_t offset,
    const char* what) {
    if (offset >= rom.size()) {
        throw std::out_of_range(what);
    }
    std::vector<std::uint8_t> line;
    for (auto source = offset; source < rom.size(); ++source) {
        const auto byte = rom[source];
        if (byte == 0xFFU) {
            return line;
        }
        line.push_back(byte);
    }
    throw std::out_of_range(what);
}

void draw_genesis_startup_line(
    const std::vector<std::uint8_t>& rom,
    const GenesisStartupPatternData& startup, RgbaImage& image,
    std::size_t source_offset, std::size_t tile_x, std::size_t tile_y,
    const char* what, bool transparent_zero = false) {
    constexpr std::size_t kCharacterTableOffset = 0xC1320;
    const auto line = read_genesis_startup_line(rom, source_offset, what);
    for (std::size_t index = 0; index < line.size(); ++index) {
        auto character = line[index];
        if (character < 0x20U || character > 0x7EU) {
            throw std::runtime_error(
                "Genesis startup line contains an unsupported text byte");
        }
        auto character_index = static_cast<std::size_t>(character - 0x20U);
        // C0DA compares the post-' ' index with $3D.  The four omitted
        // character slots are the codes immediately after '='; using $1D
        // here shifts every capital letter after 'M' onto the wrong tile.
        if (character_index >= 0x3DU) {
            character_index -= 4U;
        }
        const auto table_entry = read_rom_u16(
            rom, kCharacterTableOffset + character_index * 2U,
            "Genesis startup character table is truncated");
        const auto tile_index = static_cast<std::size_t>(table_entry & 0x07FFU);
        if (tile_index * 32U + 32U > startup.pattern_vram.size()) {
            throw std::out_of_range("Genesis startup character tile is truncated");
        }
        const auto tile = decode_genesis_tile(
            startup.pattern_vram.data() + tile_index * 32U, 32U);
        const auto palette_line = static_cast<std::size_t>(
            ((table_entry | 0x2000U) >> 13U) & 0x03U);
        const auto hflip = (table_entry & 0x0800U) != 0;
        const auto vflip = (table_entry & 0x1000U) != 0;
        const auto pixel_x_origin = (tile_x + index) * 8U;
        const auto pixel_y_origin = tile_y * 8U;
        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                const auto color_index = tile.pixels[source_y * 8U + source_x];
                const auto target_x = pixel_x_origin + pixel_x;
                const auto target_y = pixel_y_origin + pixel_y;
                if (target_x >= image.width || target_y >= image.height) {
                    throw std::out_of_range(
                        "Genesis startup text lies outside the screen");
                }
                if (transparent_zero && color_index == 0U) {
                    continue;
                }
                auto color = startup.palette[palette_line * 16U + color_index];
                if (transparent_zero) {
                    color.alpha = 255;
                }
                image.pixels[target_y * image.width + target_x] = color;
            }
        }
    }
}

void draw_genesis_startup_sprite_list(
    const std::vector<std::uint8_t>& rom,
    const GenesisStartupPatternData& startup, RgbaImage& image,
    std::size_t pointer_table_offset, std::size_t frame_index,
    std::int32_t object_x_base = 0x9C) {
    constexpr std::size_t kSpritePointerCount = 6;
    constexpr std::int32_t kSpriteYOrigin = 0x80;
    constexpr std::int32_t kObjectYBase = 0x110;
    constexpr std::uint16_t kSpritePaletteMask = 0x6000;

    if (frame_index >= kSpritePointerCount) {
        throw std::out_of_range("Genesis title sprite frame is out of range");
    }
    const auto list_offset = static_cast<std::size_t>(read_rom_u32(
        rom, pointer_table_offset + frame_index * 4U,
        "Genesis title sprite pointer table is truncated"));
    for (auto source = list_offset;;) {
        const auto source_x = read_rom_u16(
            rom, source, "Genesis title sprite list is truncated");
        source += 2U;
        if (source_x == 0x8000U) {
            return;
        }
        const auto source_y = read_rom_u16(
            rom, source, "Genesis title sprite list is truncated");
        const auto source_size = read_rom_u16(
            rom, source + 2U, "Genesis title sprite list is truncated");
        const auto source_tile = read_rom_u16(
            rom, source + 4U, "Genesis title sprite list is truncated");
        source += 6U;

        // 0x1378: SAT X = D1 + list X, SAT Y = D2 + list Y. VDP origin is
        // $80 on both axes. Size byte is Genesis width-in-bits-2..3 /
        // height-in-bits-0..1; sprite pattern numbers advance column-major.
        constexpr std::int32_t kSpriteXOrigin = 0x80;
        const auto screen_x =
            static_cast<std::int32_t>(static_cast<std::int16_t>(source_x)) +
            object_x_base - kSpriteXOrigin;
        const auto screen_y =
            static_cast<std::int32_t>(static_cast<std::int16_t>(source_y)) +
            kObjectYBase - kSpriteYOrigin;
        const auto size = static_cast<std::uint16_t>(source_size & 0x00FFU);
        const auto width_tiles =
            static_cast<std::size_t>(((size >> 2U) & 0x03U) + 1U);
        const auto height_tiles =
            static_cast<std::size_t>((size & 0x03U) + 1U);
        const auto tile_attributes = static_cast<std::uint16_t>(
            source_tile | kSpritePaletteMask);
        const auto tile_index = static_cast<std::size_t>(
            tile_attributes & 0x07FFU);
        const auto palette_line = static_cast<std::size_t>(
            (tile_attributes >> 13U) & 0x03U);
        const auto hflip = (tile_attributes & 0x0800U) != 0;
        const auto vflip = (tile_attributes & 0x1000U) != 0;
        for (std::size_t tile_y = 0; tile_y < height_tiles; ++tile_y) {
            for (std::size_t tile_x = 0; tile_x < width_tiles; ++tile_x) {
                const auto source_tile_x = hflip
                    ? width_tiles - 1U - tile_x : tile_x;
                const auto source_tile_y = vflip
                    ? height_tiles - 1U - tile_y : tile_y;
                const auto current_tile = tile_index +
                    source_tile_x * height_tiles + source_tile_y;
                if (current_tile * 32U + 32U > startup.pattern_vram.size()) {
                    throw std::out_of_range(
                        "Genesis title sprite tile is truncated");
                }
                const auto tile = decode_genesis_tile(
                    startup.pattern_vram.data() + current_tile * 32U, 32U);
                for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                    for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                        const auto source_pixel_x = hflip ? 7U - pixel_x : pixel_x;
                        const auto source_pixel_y = vflip ? 7U - pixel_y : pixel_y;
                        const auto color_index = tile.pixels[
                            source_pixel_y * 8U + source_pixel_x];
                        const auto target_x = screen_x +
                            static_cast<std::int32_t>(tile_x * 8U + pixel_x);
                        const auto target_y = screen_y +
                            static_cast<std::int32_t>(tile_y * 8U + pixel_y);
                        if (target_x < 0 || target_y < 0 ||
                            target_x >= image.width || target_y >= image.height) {
                            continue;
                        }
                        const auto& color = startup.palette[
                            palette_line * 16U + color_index];
                        // Genesis sprite palette index zero is transparent;
                        // the title SAT objects must reveal the logo-plane
                        // art underneath it instead of painting CRAM color 0
                        // across the 4x4 object.
                        if (color_index != 0U && color.alpha != 0) {
                            image.pixels[static_cast<std::size_t>(target_y) *
                                         image.width +
                                         static_cast<std::size_t>(target_x)] = color;
                        }
                    }
                }
            }
        }
    }
}

void draw_genesis_startup_logo_plane(
    const std::vector<std::uint8_t>& rom,
    const GenesisStartupPatternData& startup, RgbaImage& image,
    std::size_t source_offset, std::size_t column_count) {
    // 0x5C8C6 reads a word with post-increment and then adds $4A, so the
    // next source row begins $4C bytes later, not $4A bytes later.
    constexpr std::size_t kSourceRowStride = 0x4CU;
    // 0x1E84 loads D1=$12 and 0x5C8C6 uses DBF, so each reveal column
    // contains nineteen words. Using 38 here writes into the copyright rows
    // at CB04/CB84 and is the direct cause of the title text corruption.
    constexpr std::size_t kRowCount = 19U;
    constexpr std::size_t kDestinationColumn = 1U;
    constexpr std::size_t kDestinationRow = 2U;

    // ROM 0x1E4E advances the source by one word for each output column and
    // 0x5C8C6 advances it by $4C bytes for each of the 19 rows emitted by
    // its DBF loop. The destination starts at C102 (plane A column 1, row 2)
    // and advances by one name-table row per write. This is the wide title
    // mark that was missing when only the 0x1C6A sprite list was composed.
    if (column_count == 0U || source_offset > rom.size() ||
        kRowCount * kSourceRowStride > rom.size() - source_offset) {
        throw std::out_of_range("Genesis startup logo tilemap is truncated");
    }
    for (std::size_t column = 0; column < column_count; ++column) {
        for (std::size_t row = 0; row < kRowCount; ++row) {
            const auto offset = source_offset + column * 2U +
                                row * kSourceRowStride;
            if (offset + 2U > rom.size()) {
                throw std::out_of_range(
                    "Genesis startup logo tilemap column is truncated");
            }
            const auto entry = read_rom_u16(
                rom, offset, "Genesis startup logo tilemap word is truncated");
            const auto tile_index = static_cast<std::size_t>(entry & 0x07FFU);
            if (tile_index * 32U + 32U > startup.pattern_vram.size()) {
                throw std::out_of_range("Genesis startup logo tile is truncated");
            }
            const auto tile = decode_genesis_tile(
                startup.pattern_vram.data() + tile_index * 32U, 32U);
            const auto palette_line = static_cast<std::size_t>(
                (entry >> 13U) & 0x03U);
            const auto hflip = (entry & 0x0800U) != 0;
            const auto vflip = (entry & 0x1000U) != 0;
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                    const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                    const auto color_index = tile.pixels[
                        source_y * 8U + source_x];
                    const auto target_x = (kDestinationColumn + column) * 8U +
                                          pixel_x;
                    const auto target_y = (kDestinationRow + row) * 8U +
                                          pixel_y;
                    if (target_x >= image.width || target_y >= image.height) {
                        continue;
                    }
                    const auto& color = startup.palette[
                        palette_line * 16U + color_index];
                    // Plane-A color zero exposes the background plane. This
                    // matters once the title reveal is rendered as a
                    // transparent host layer instead of a precomposed frame.
                    if (color_index != 0U && color.alpha != 0U) {
                        image.pixels[target_y * image.width + target_x] = color;
                    }
                }
            }
        }
    }
}

} // namespace

namespace {

RgbaImage make_transparent_layer(std::uint16_t width,
                                 std::uint16_t height) {
    return {
        width, height,
        std::vector<native::Rgba8>(
            static_cast<std::size_t>(width) * height,
            native::Rgba8{0, 0, 0, 0}),
    };
}

RgbaImage make_transparent_title_layer() {
    return make_transparent_layer(native::kScreenWidth, native::kScreenHeight);
}

RgbaImage decode_genesis_title_text_layer(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kPromptTileX = 2U;
    constexpr std::size_t kPromptTileY = 21U;
    constexpr std::size_t kCopyrightTileX = 2U;
    constexpr std::size_t kCopyrightTileY = 22U;
    const auto startup = decode_genesis_startup_pattern_data(
        rom, 0x1F310EU, 0x1FF0U);
    auto image = make_transparent_title_layer();
    draw_genesis_startup_line(
        rom, startup, image, 0x1F81U, kPromptTileX, kPromptTileY,
        "Genesis PRESS START line is truncated", true);
    draw_genesis_startup_line(
        rom, startup, image, 0x1F00U, kCopyrightTileX, kCopyrightTileY,
        "Genesis startup copyright line is truncated", true);
    draw_genesis_startup_line(
        rom, startup, image, 0x1F24U, kCopyrightTileX, kCopyrightTileY + 1U,
        "Genesis startup trademark line is truncated", true);
    draw_genesis_startup_line(
        rom, startup, image, 0x1F48U, kCopyrightTileX, kCopyrightTileY + 2U,
        "Genesis startup trademark owner line is truncated", true);
    draw_genesis_startup_line(
        rom, startup, image, 0x1F68U, kCopyrightTileX, kCopyrightTileY + 3U,
        "Genesis startup Sega line is truncated", true);
    return image;
}

} // namespace

RgbaImage decode_genesis_title_background(
    const std::vector<std::uint8_t>& rom) {
    // The circuit field is a separate layer from the logo reveal. Keep it in
    // The 64-word circuit palette begins at 0x1C024; 0x1C064 is its third
    // line, not the palette-set base. The source 0x1D58 tilemap writer ORs
    // $4000 into each name word, selecting the green circuit line (line 2).
    // The separate 0x1FF0 palette belongs to the logo/text resources.
    return decode_genesis_startup_screen(
        rom, 0x1F310EU, 0x67DF6U, 0x1C024U, 0x4000U);
}

std::array<RgbaImage, native::kNativeTitleCircuitLightCount>
decode_genesis_title_circuit_lights(const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeTitleCircuitLightCount> frames{};
    for (std::size_t phase = 0; phase < frames.size(); ++phase) {
        frames[phase] = decode_genesis_startup_screen(
            rom, 0x1F310EU, 0x67DF6U, 0x1C024U, 0x4000U,
            static_cast<int>(phase));
    }
    return frames;
}

RgbaImage decode_genesis_title_logo_plane(
    const std::vector<std::uint8_t>& rom) {
    auto image = make_transparent_title_layer();
    const auto startup = decode_genesis_startup_pattern_data(
        rom, 0x1F310EU, 0x1FF0U);
    draw_genesis_startup_logo_plane(
        rom, startup, image, 0x2090U, 38U);
    return image;
}

RgbaImage decode_genesis_title_text(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_title_text_layer(rom);
}

std::array<RgbaImage, native::kNativeTitleAnimationFrameCount>
decode_genesis_title_sprite_frames(
    const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeTitleAnimationFrameCount> frames{};
    const auto startup = decode_genesis_startup_pattern_data(
        rom, 0x1F310EU, 0x1FF0U);
    for (std::size_t index = 0; index < frames.size(); ++index) {
        frames[index] = make_transparent_title_layer();
        draw_genesis_startup_sprite_list(
            rom, startup, frames[index], 0x1CE4U, index);
    }
    return frames;
}

RgbaImage decode_genesis_startup_font(
    const std::vector<std::uint8_t>& rom) {
    constexpr std::size_t kFontCharacterCount = 0x60U;
    constexpr std::size_t kAtlasColumns = 16U;
    constexpr std::size_t kAtlasRows = 6U;
    const auto startup = decode_genesis_startup_pattern_data(
        rom, 0x1F310EU, 0x1FF0U);
    RgbaImage atlas{
        static_cast<std::uint16_t>(kAtlasColumns * 8U),
        static_cast<std::uint16_t>(kAtlasRows * 8U),
        std::vector<native::Rgba8>(kAtlasColumns * kAtlasRows * 64U,
                                   native::Rgba8{0, 0, 0, 0}),
    };
    for (std::size_t character = 0; character < kFontCharacterCount;
         ++character) {
        auto table_index = character;
        if (table_index >= 0x3DU) {
            table_index -= 4U;
        }
        const auto entry = read_rom_u16(
            rom, 0xC1320U + table_index * 2U,
            "Genesis startup font table is truncated");
        const auto tile_index = static_cast<std::size_t>(entry & 0x07FFU);
        if (tile_index * 32U + 32U > startup.pattern_vram.size()) {
            throw std::out_of_range("Genesis startup font tile is truncated");
        }
        const auto tile = decode_genesis_tile(
            startup.pattern_vram.data() + tile_index * 32U, 32U);
        const auto atlas_x = (character % kAtlasColumns) * 8U;
        const auto atlas_y = (character / kAtlasColumns) * 8U;
        for (std::size_t y = 0; y < 8U; ++y) {
            for (std::size_t x = 0; x < 8U; ++x) {
                const auto color_index = tile.pixels[y * 8U + x];
                auto color = startup.palette[16U + color_index];
                color.alpha = color_index == 0U ? 0U : 255U;
                atlas.pixels[(atlas_y + y) * atlas.width + atlas_x + x] =
                    color;
            }
        }
    }
    return atlas;
}

RgbaImage decode_genesis_startup_cursor(
    const std::vector<std::uint8_t>& rom) {
    const auto startup = decode_genesis_startup_pattern_data(
        rom, 0x1F310EU, 0x1FF0U);
    // The startup menu uses the command-screen text palette loaded by
    // 0x10918/0x5670E, not the title-logo palette at 0x1FF0.  The source
    // cursor name word is written with the command text's third palette line;
    // using the title palette made the selector white even though the menu
    // text is green.
    const auto command_palette = read_genesis_command_palette_set(rom);
    constexpr std::size_t kCursorPaletteLine = 3U;
    constexpr std::uint16_t kCursorNameWord =
        static_cast<std::uint16_t>(-0x7A55) | 0x6000U;
    const auto tile_index = static_cast<std::size_t>(kCursorNameWord & 0x07FFU);
    if (tile_index * 32U + 32U > startup.pattern_vram.size()) {
        throw std::out_of_range("Genesis startup cursor tile is truncated");
    }
    const auto tile = decode_genesis_tile(
        startup.pattern_vram.data() + tile_index * 32U, 32U);
    RgbaImage image{
        8U, 8U, std::vector<native::Rgba8>(64U, native::Rgba8{0, 0, 0, 0})};
    auto has_ink = false;
    for (std::size_t y = 0; y < 8U; ++y) {
        for (std::size_t x = 0; x < 8U; ++x) {
            const auto color_index = tile.pixels[y * 8U + x];
            auto color = command_palette[kCursorPaletteLine * 16U + color_index];
            color.alpha = color_index == 0U ? 0U : 255U;
            has_ink = has_ink || color_index != 0U;
            image.pixels[y * 8U + x] = color;
        }
    }
    if (!has_ink) {
        // The source writes this name-table word from C3B2. Some ROM dumps
        // omit the late cursor tile from the 0xC1064 upload even though the
        // menu still references it. Preserve the source cursor position and
        // direction with the same one-tile host fallback rather than leaving
        // the selectable option visually unmarked.
        const auto color = [&command_palette]() {
            auto result = command_palette[kCursorPaletteLine * 16U + 11U];
            result.alpha = 255U;
            return result;
        }();
        constexpr std::array<std::uint8_t, 8> kArrow{{
            0b00001000U, 0b00001100U, 0b00001110U, 0b00001111U,
            0b00001110U, 0b00001100U, 0b00001000U, 0b00000000U}};
        for (std::size_t y = 0; y < kArrow.size(); ++y) {
            for (std::size_t x = 0; x < 8U; ++x) {
                if ((kArrow[y] & (1U << (7U - x))) != 0U) {
                    image.pixels[y * 8U + x] = color;
                }
            }
        }
    }
    return image;
}

RgbaImage decode_genesis_startup_screen(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_stream_offset,
    std::size_t tilemap_stream_offset,
    std::size_t palette_offset,
    std::uint16_t name_word_mask,
    int circuit_light_phase) {
    constexpr std::size_t kColumns = 40;
    constexpr std::size_t kRows = 28;
    constexpr std::size_t kMapRows = 32;
    auto startup = decode_genesis_startup_pattern_data(
        rom, pattern_stream_offset, palette_offset);
    if (circuit_light_phase >= 0) {
        // 0x2634: move.w #$C050,VDP_CTRL then eight data words from
        // 0x26F2 + (DF92<<1). CRAM byte $50 is palette index 40.
        constexpr std::size_t kLightTable = 0x26F2U;
        const auto phase = static_cast<std::size_t>(circuit_light_phase);
        for (std::size_t color = 0; color < 8U; ++color) {
            const auto word = read_rom_u16(
                rom, kLightTable + (phase + color) * 2U,
                "Genesis title circuit light table is truncated");
            const auto decoded = decode_genesis_cram_word(word, false);
            // 0x2634 writes CRAM $50, which is palette line 2, color 8.
            // Do not copy the live CRAM write into the other three palette
            // lines: those lines belong to the title logo/text and changing
            // them makes the extracted circuit frames visually drift.
            startup.palette[2U * 16U + 8U + color] = decoded;
        }
    }
    require_bytes(rom.size() - std::min(tilemap_stream_offset, rom.size()), 2,
                  "Genesis startup tilemap is truncated");

    const auto read16 = [&](std::size_t offset) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2,
                      "Genesis startup tilemap word is truncated");
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };

    const auto read8 = [&](std::size_t offset) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 1,
                      "Genesis startup tilemap byte is truncated");
        return rom[offset];
    };
    auto source = tilemap_stream_offset;
    const auto base_word = read16(source);
    source += 2U;

    // ROM 0x1D58/0x1728: D7=$27 (40-wide), D1=$1F (32-tall), D3=0, D5=40.
    std::vector<std::uint16_t> map(kColumns * kMapRows, 0);
    std::size_t generated_count = 0;
    std::size_t output_count = 0;
    const auto emit = [&](std::uint16_t value) {
        if (output_count >= map.size()) {
            throw std::out_of_range("Genesis startup tilemap exceeds its VDP grid");
        }
        map[output_count++] = static_cast<std::uint16_t>(value | name_word_mask);
    };
    const auto emit_run = [&](std::uint16_t value, int delta,
                              unsigned count) {
        // 0x1816 uses SUBQ.B / BGE, so a low-five-bit count of N writes
        // N+1 cells. Literal tokens at 0x178C use BGT and stay N (or 1
        // when N is 0). 0x1D58 clears D3, so every generated cell is
        // written. The extra cell is important: omitting it shifts every
        // subsequent compressed run left, which breaks the board's
        // horizontal trace/chip alignment even though the tile graphics are
        // individually decoded correctly.
        const auto run_length = count + 1U;
        for (unsigned index = 0; index < run_length; ++index) {
            ++generated_count;
            emit(value);
            value = static_cast<std::uint16_t>(value + delta);
        }
    };

    while (output_count < map.size()) {
        const auto token = read8(source++);
        if (token == 0) {
            break;
        }
        if (token > 0xC0U) {
            const auto count = static_cast<unsigned>(token & 0x1FU);
            const auto wide = (token & 0x20U) != 0;
            const auto run_length = std::max<unsigned>(count, 1U);
            for (unsigned index = 0; index < run_length; ++index) {
                std::uint16_t value = wide ? read16(source) :
                    static_cast<std::uint16_t>(read8(source));
                source += wide ? 2U : 1U;
                value = static_cast<std::uint16_t>(value + base_word);
                ++generated_count;
                emit(value);
            }
            continue;
        }

        const auto delta = token > 0x80U ? 1 : token > 0x40U ? 0 : -1;
        const auto count = static_cast<unsigned>(token & 0x1FU);
        const auto wide = (token & 0x20U) != 0;
        std::uint16_t value = wide ?
            static_cast<std::uint16_t>(read8(source++)) << 8U : 0;
        value = static_cast<std::uint16_t>(value | read8(source++));
        value = static_cast<std::uint16_t>(value + base_word);
        emit_run(value, delta, count);
    }

    if (output_count == 0) {
        throw std::runtime_error("Genesis startup tilemap is empty");
    }
    RgbaImage image{
        static_cast<std::uint16_t>(kColumns * 8U),
        static_cast<std::uint16_t>(kRows * 8U),
        std::vector<native::Rgba8>(
            kColumns * kRows * 64U, native::Rgba8{0, 0, 0, 255}),
    };
    for (std::size_t map_y = 0; map_y < kRows; ++map_y) {
        for (std::size_t map_x = 0; map_x < kColumns; ++map_x) {
            const auto map_index = map_y * kColumns + map_x;
            const auto entry = map[map_index];
            if (entry == 0) {
                continue;
            }
            const auto tile_index = static_cast<std::size_t>(entry & 0x07FFU);
            if (tile_index * 32U + 32U > startup.pattern_vram.size()) {
                throw std::out_of_range("Genesis startup tile index is truncated");
            }
            const auto tile = decode_genesis_tile(
                startup.pattern_vram.data() + tile_index * 32U, 32U);
            const auto palette_line = static_cast<std::size_t>(
                (entry >> 13U) & 0x03U);
            const auto hflip = (entry & 0x0800U) != 0;
            const auto vflip = (entry & 0x1000U) != 0;
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                    const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                    const auto color_index = tile.pixels[source_y * 8U +
                                                         source_x];
                    image.pixels[(map_y * 8U + pixel_y) * image.width +
                                 map_x * 8U + pixel_x] =
                        startup.palette[palette_line * 16U + color_index];
                }
            }
        }
    }
    return image;
}

namespace {

RgbaImage decode_genesis_title_frame(const std::vector<std::uint8_t>& rom,
                                     std::size_t frame_index) {
    constexpr std::size_t kPromptTileX = 2;
    constexpr std::size_t kPromptTileY = 21;
    constexpr std::size_t kCopyrightTileX = 2;
    constexpr std::size_t kCopyrightTileY = 22;

    auto image = decode_genesis_title_background(rom);
    // 0x1FF0 is the title logo/sprite/text resource palette. 0xD29D6 is
    // command-screen data, not palette data; passing it here made the
    // composed title frame use pointer/text bytes as colors.
    const auto startup = decode_genesis_startup_pattern_data(
        rom, 0x1F310EU, 0x1FF0U);
    draw_genesis_startup_logo_plane(rom, startup, image, 0x2090U, 38U);
    // The static 0x1D58 plane is the dark circuit field. The title mark is
    // emitted separately by 0x1C6A through 0x1378 into the sprite list; its
    // six source lists are animation phases.
    draw_genesis_startup_sprite_list(rom, startup, image, 0x1CE4U,
                                     frame_index);
    // ROM 0x1DCA writes the 0x1F81 line at VRAM 0xCA84: column 2,
    // row 21 of the 64-column name table. ROM 0x1DF6 writes the
    // copyright block at 0xCB04, 0xCB84, 0xCC04, and 0xCC84. The native
    // image composes all four fixed source lines even though the first ROM
    // update only calls this block after the title plane has settled.
    draw_genesis_startup_line(rom, startup, image, 0x1F81U,
                              kPromptTileX, kPromptTileY,
                              "Genesis PRESS START line is truncated", true);
    draw_genesis_startup_line(rom, startup, image, 0x1F00U,
                              kCopyrightTileX, kCopyrightTileY,
                              "Genesis startup copyright line is truncated",
                              true);
    draw_genesis_startup_line(rom, startup, image, 0x1F24U,
                              kCopyrightTileX, kCopyrightTileY + 1U,
                              "Genesis startup trademark line is truncated",
                              true);
    draw_genesis_startup_line(rom, startup, image, 0x1F48U,
                              kCopyrightTileX, kCopyrightTileY + 2U,
                              "Genesis startup trademark owner line is truncated",
                              true);
    draw_genesis_startup_line(rom, startup, image, 0x1F68U,
                              kCopyrightTileX, kCopyrightTileY + 3U,
                              "Genesis startup Sega line is truncated", true);
    return image;
}

} // namespace

RgbaImage decode_genesis_title_screen(const std::vector<std::uint8_t>& rom) {
    return decode_genesis_title_frame(rom, 0U);
}

std::array<RgbaImage, native::kNativeTitleAnimationFrameCount>
decode_genesis_title_animation_frames(
    const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeTitleAnimationFrameCount> frames{};
    for (std::size_t index = 0; index < frames.size(); ++index) {
        frames[index] = decode_genesis_title_frame(rom, index);
    }
    return frames;
}

namespace {

struct GenesisBootPatternData {
    std::vector<std::uint8_t> pattern_vram;
    std::array<native::Rgba8, 64> palette{};
};

GenesisBootPatternData decode_genesis_boot_pattern_data(
    const std::vector<std::uint8_t>& rom) {
    GenesisBootPatternData result{
        std::vector<std::uint8_t>(0x10000U, 0),
        read_genesis_palette_set(rom, 0x5DC66U),
    };
    // 0x5DB10 pattern list: even 0xF5C copy 0x1FEC46 -> VRAM $8000, $E20 bytes.
    write_genesis_f5c_vram(rom, 0x1FEC46U, 0x8000U, 0xE20U,
                           result.pattern_vram);
    // 0x176BA: uncompressed guy tiles at VRAM $2360, then 0x1530 at $2DE0.
    write_genesis_f5c_vram(rom, 0xDA038U, 0x2360U, 0xA80U,
                           result.pattern_vram);
    write_genesis_1530_vram(rom, 0xDAAB8U, 0x2DE0U, result.pattern_vram);
    // 0x5DA50 packed SEGA tiles: 0x1530 from 0x14C222 to VRAM $400.
    write_genesis_1530_vram(rom, 0x14C222U, 0x400U, result.pattern_vram);
    return result;
}

void blit_genesis_tile(
    RgbaImage& image, const GenesisBootPatternData& boot,
    std::size_t tile_index, std::size_t palette_line, bool hflip, bool vflip,
    std::int32_t dest_x, std::int32_t dest_y, bool transparent_zero) {
    if (tile_index * 32U + 32U > boot.pattern_vram.size()) {
        throw std::out_of_range("Genesis boot tile is truncated");
    }
    const auto tile = decode_genesis_tile(
        boot.pattern_vram.data() + tile_index * 32U, 32U);
    for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
        for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
            const auto source_x = hflip ? 7U - pixel_x : pixel_x;
            const auto source_y = vflip ? 7U - pixel_y : pixel_y;
            const auto color_index =
                tile.pixels[source_y * 8U + source_x];
            auto color =
                boot.palette[palette_line * 16U + color_index];
            if (transparent_zero && color_index == 0U) {
                continue;
            }
            color.alpha = 255;
            const auto target_x = dest_x + static_cast<std::int32_t>(pixel_x);
            const auto target_y = dest_y + static_cast<std::int32_t>(pixel_y);
            if (target_x < 0 || target_y < 0 ||
                target_x >= image.width || target_y >= image.height) {
                continue;
            }
            image.pixels[static_cast<std::size_t>(target_y) * image.width +
                         static_cast<std::size_t>(target_x)] = color;
        }
    }
}

void blit_genesis_name_table(
    const std::vector<std::uint8_t>& rom, const GenesisBootPatternData& boot,
    RgbaImage& image, std::size_t map_offset, std::size_t columns,
    std::size_t rows, std::uint32_t dest_vram, std::uint32_t plane_base,
    std::uint16_t attr_or, bool transparent_zero,
    std::int32_t dest_y_adjust = 0, bool wrap_horizontal = false,
    std::int32_t dest_x_adjust = 0) {
    const auto plane_offset = dest_vram - plane_base;
    const auto start_tile_x = (plane_offset % 0x80U) / 2U;
    const auto start_tile_y = plane_offset / 0x80U;
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t col = 0; col < columns; ++col) {
            const auto word = static_cast<std::uint16_t>(
                read_rom_u16(
                    rom, map_offset + (row * columns + col) * 2U,
                    "Genesis boot nametable is truncated") |
                attr_or);
            const auto tile_index = static_cast<std::size_t>(word & 0x07FFU);
            const auto palette_line =
                static_cast<std::size_t>((word >> 13U) & 0x03U);
            const auto hflip = (word & 0x0800U) != 0;
            const auto vflip = (word & 0x1000U) != 0;
            const auto screen_tile_x = wrap_horizontal
                ? (start_tile_x + col) % 64U : start_tile_x + col;
            blit_genesis_tile(
                image, boot, tile_index, palette_line, hflip, vflip,
                static_cast<std::int32_t>(screen_tile_x * 8U) + dest_x_adjust,
                static_cast<std::int32_t>((start_tile_y + row) * 8U) +
                    dest_y_adjust,
                transparent_zero);
        }
    }
}

void blit_genesis_sat_list(
    const std::vector<std::uint8_t>& rom, const GenesisBootPatternData& boot,
    RgbaImage& image, std::size_t list_offset, std::int32_t origin_x,
    std::int32_t origin_y, std::uint16_t attr_or) {
    constexpr std::int32_t kSatOrigin = 0x80;
    for (auto source = list_offset;;) {
        const auto source_x = read_rom_u16(
            rom, source, "Genesis boot sprite list is truncated");
        if (source_x == 0x8000U) {
            return;
        }
        const auto source_y = read_rom_u16(
            rom, source + 2U, "Genesis boot sprite list is truncated");
        const auto source_size = read_rom_u16(
            rom, source + 4U, "Genesis boot sprite list is truncated");
        const auto source_tile = read_rom_u16(
            rom, source + 6U, "Genesis boot sprite list is truncated");
        source += 8U;
        const auto size = static_cast<std::uint16_t>(source_size & 0x00FFU);
        const auto width_tiles =
            static_cast<std::size_t>(((size >> 2U) & 0x03U) + 1U);
        const auto height_tiles =
            static_cast<std::size_t>((size & 0x03U) + 1U);
        const auto tile_attributes =
            static_cast<std::uint16_t>(source_tile | attr_or);
        const auto tile_index =
            static_cast<std::size_t>(tile_attributes & 0x07FFU);
        const auto palette_line =
            static_cast<std::size_t>((tile_attributes >> 13U) & 0x03U);
        const auto hflip = (tile_attributes & 0x0800U) != 0;
        const auto vflip = (tile_attributes & 0x1000U) != 0;
        const auto screen_x = static_cast<std::int32_t>(
                                  static_cast<std::int16_t>(source_x)) +
                              origin_x - kSatOrigin;
        const auto screen_y =
            static_cast<std::int32_t>(static_cast<std::int16_t>(source_y)) +
            origin_y - kSatOrigin;
        for (std::size_t tile_x = 0; tile_x < width_tiles; ++tile_x) {
            for (std::size_t tile_y = 0; tile_y < height_tiles; ++tile_y) {
                const auto source_tile_x =
                    hflip ? width_tiles - 1U - tile_x : tile_x;
                const auto source_tile_y =
                    vflip ? height_tiles - 1U - tile_y : tile_y;
                const auto current_tile =
                    tile_index + source_tile_x * height_tiles + source_tile_y;
                blit_genesis_tile(
                    image, boot, current_tile, palette_line, hflip, vflip,
                    screen_x + static_cast<std::int32_t>(tile_x * 8U),
                    screen_y + static_cast<std::int32_t>(tile_y * 8U), true);
            }
        }
    }
}

void blit_genesis_1728_name_table(
    const std::vector<std::uint8_t>& rom, const GenesisBootPatternData& scene,
    RgbaImage& image, std::size_t map_offset, std::size_t columns,
    std::size_t rows, std::uint32_t dest_vram, std::uint32_t plane_base,
    std::uint16_t attr_or, bool transparent_zero,
    std::int32_t dest_y_adjust = 0, bool wrap_horizontal = false,
    std::int32_t dest_x_adjust = 0) {
    if (columns == 0U || rows == 0U) {
        throw std::invalid_argument("Genesis 0x1728 map dimensions are invalid");
    }
    const auto cell_count = columns * rows;
    std::vector<std::uint16_t> map(cell_count, 0);
    auto source = map_offset;
    const auto base_word = read_rom_u16(
        rom, source, "Genesis 0x1728 map is truncated");
    source += 2U;
    std::size_t generated_count = 0;
    std::size_t output_count = 0;
    const auto emit = [&](std::uint16_t value) {
        if (output_count >= map.size()) {
            return;
        }
        map[output_count++] = static_cast<std::uint16_t>(value | attr_or);
    };
    const auto emit_run = [&](std::uint16_t value, int delta,
                              unsigned count) {
        // 0x1816: SUBQ.B / BGE writes count+1 cells. 0x1406 clears D3.
        const auto run_length = count + 1U;
        for (unsigned index = 0; index < run_length; ++index) {
            ++generated_count;
            emit(value);
            value = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(value) +
                static_cast<std::uint16_t>(static_cast<std::int16_t>(delta)));
        }
    };
    while (output_count < map.size()) {
        if (source >= rom.size()) {
            throw std::out_of_range("Genesis 0x1728 map is truncated");
        }
        const auto token = rom[source++];
        if (token == 0) {
            break;
        }
        if (token > 0xC0U) {
            const auto count = static_cast<unsigned>(token & 0x1FU);
            const auto wide = (token & 0x20U) != 0;
            const auto run_length = std::max<unsigned>(count, 1U);
            for (unsigned index = 0; index < run_length; ++index) {
                std::uint16_t value = 0;
                if (wide) {
                    value = read_rom_u16(
                        rom, source, "Genesis 0x1728 map is truncated");
                    source += 2U;
                } else {
                    if (source >= rom.size()) {
                        throw std::out_of_range(
                            "Genesis 0x1728 map is truncated");
                    }
                    value = rom[source++];
                }
                value = static_cast<std::uint16_t>(value + base_word);
                ++generated_count;
                emit(value);
            }
            continue;
        }
        const auto delta = token > 0x80U ? 1 : token > 0x40U ? 0 : -1;
        const auto count = static_cast<unsigned>(token & 0x1FU);
        const auto wide = (token & 0x20U) != 0;
        std::uint16_t value = 0;
        if (wide) {
            if (source >= rom.size()) {
                throw std::out_of_range("Genesis 0x1728 map is truncated");
            }
            value = static_cast<std::uint16_t>(rom[source++]) << 8U;
        }
        if (source >= rom.size()) {
            throw std::out_of_range("Genesis 0x1728 map is truncated");
        }
        value = static_cast<std::uint16_t>(value | rom[source++]);
        value = static_cast<std::uint16_t>(value + base_word);
        emit_run(value, delta, count);
    }
    const auto plane_offset = dest_vram - plane_base;
    const auto start_tile_x = (plane_offset % 0x80U) / 2U;
    const auto start_tile_y = plane_offset / 0x80U;
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t col = 0; col < columns; ++col) {
            const auto word = map[row * columns + col];
            if (word == 0U && transparent_zero) {
                continue;
            }
            const auto tile_index = static_cast<std::size_t>(word & 0x07FFU);
            const auto palette_line =
                static_cast<std::size_t>((word >> 13U) & 0x03U);
            const auto hflip = (word & 0x0800U) != 0;
            const auto vflip = (word & 0x1000U) != 0;
            const auto screen_tile_x = wrap_horizontal
                ? (start_tile_x + col) % 64U : start_tile_x + col;
            blit_genesis_tile(
                image, scene, tile_index, palette_line, hflip, vflip,
                static_cast<std::int32_t>(screen_tile_x * 8U) + dest_x_adjust,
                static_cast<std::int32_t>((start_tile_y + row) * 8U) +
                    dest_y_adjust,
                transparent_zero);
        }
    }
}

std::uint32_t plane_base_for_dest(std::uint16_t plane_a, std::uint16_t plane_b,
                                  std::uint32_t dest_vram) {
    if (plane_b != 0U && dest_vram >= plane_b) {
        return plane_b;
    }
    return plane_a;
}

void load_genesis_1406_patterns(const std::vector<std::uint8_t>& rom,
                                std::size_t descriptor,
                                GenesisBootPatternData& scene,
                                std::size_t& map_list_offset) {
    // 0x1406 keeps the 0x1530 overlap buffer at $FF9700 alive while it
    // uploads each consecutive compressed pattern block. The aircraft block
    // at 0x1E8910 uses back-references into the preceding 0x1E6956 upload.
    GenesisResourceDecoderState decoder_state;
    auto offset = descriptor + 0x10U;
    while (true) {
        const auto source = read_rom_u32(
            rom, offset, "Genesis 0x1406 pattern list is truncated");
        offset += 4U;
        if (source == 0U) {
            break;
        }
        const auto dest = read_rom_u32(
            rom, offset, "Genesis 0x1406 pattern list is truncated");
        offset += 4U;
        const auto size = read_rom_u32(
            rom, offset, "Genesis 0x1406 pattern list is truncated");
        offset += 4U;
        if ((source & 1U) != 0U) {
            write_genesis_1530_vram_with_state(
                rom, static_cast<std::size_t>(source & ~1U),
                static_cast<std::size_t>(dest & 0xFFFFU), scene.pattern_vram,
                decoder_state);
        } else {
            write_genesis_f5c_vram(
                rom, static_cast<std::size_t>(source),
                static_cast<std::size_t>(dest),
                static_cast<std::size_t>(size), scene.pattern_vram);
        }
    }
    map_list_offset = offset;
}

void blit_genesis_1406_maps(const std::vector<std::uint8_t>& rom,
                            const GenesisBootPatternData& scene,
                            RgbaImage& image, std::size_t map_list_offset,
                            std::uint16_t plane_a, std::uint16_t plane_b,
                            std::int32_t plane_b_vscroll = 0,
                            std::int32_t plane_a_xscroll = 0,
                            bool draw_plane_a = true,
                            bool draw_plane_b = true) {
    struct MapEntry {
        std::uint32_t source;
        std::uint32_t dest;
        std::size_t columns;
        std::size_t rows;
        std::uint16_t attr;
        bool transparent;
    };
    std::vector<MapEntry> maps;
    auto offset = map_list_offset;
    auto first_map = true;
    while (true) {
        const auto source = read_rom_u32(
            rom, offset, "Genesis 0x1406 map list is truncated");
        offset += 4U;
        if (source == 0U) {
            break;
        }
        const auto dest = read_rom_u32(
            rom, offset, "Genesis 0x1406 map list is truncated");
        offset += 4U;
        const auto columns = static_cast<std::size_t>(
            read_rom_u16(rom, offset, "Genesis 0x1406 map list is truncated") +
            1U);
        offset += 2U;
        const auto rows = static_cast<std::size_t>(
            read_rom_u16(rom, offset, "Genesis 0x1406 map list is truncated") +
            1U);
        offset += 2U;
        const auto attr = read_rom_u16(
            rom, offset, "Genesis 0x1406 map list is truncated");
        offset += 2U;
        maps.push_back({source, dest, columns, rows, attr, !first_map});
        first_map = false;
    }
    // 0x1406 lists Plane A before Plane B, but Genesis draws Plane B first
    // and Plane A over it. The ending descriptor's 27x8 Plane-A map is the
    // vehicle; drawing the list in source order hid it under the skyline.
    for (auto map = maps.rbegin(); map != maps.rend(); ++map) {
        const auto plane = plane_base_for_dest(
            plane_a, plane_b, static_cast<std::uint32_t>(map->dest));
        if ((plane == plane_a && !draw_plane_a) ||
            (plane == plane_b && !draw_plane_b)) {
            continue;
        }
        const auto y_adjust = plane == plane_b ? -plane_b_vscroll : 0;
        // The source stores the signed H-scroll value in DD08.  A negative
        // value moves Plane A left on screen; do not negate it a second time
        // when converting the source register to a host blit offset.  The
        // wilderness pan relies on this sign so the body and its separately
        // drawn head travel toward the same attachment point.
        const auto x_adjust = plane == plane_a ? plane_a_xscroll : 0;
        if ((map->source & 1U) != 0U) {
            blit_genesis_1728_name_table(
                rom, scene, image,
                static_cast<std::size_t>(map->source & ~1U), map->columns,
                map->rows, map->dest, plane, map->attr, map->transparent,
                y_adjust, true, x_adjust);
        } else {
            blit_genesis_name_table(
                rom, scene, image, static_cast<std::size_t>(map->source),
                map->columns, map->rows, map->dest, plane, map->attr,
                map->transparent, y_adjust, true, x_adjust);
        }
    }
}

RgbaImage decode_genesis_1406_scene(const std::vector<std::uint8_t>& rom,
                                    std::size_t descriptor,
                                    GenesisBootPatternData* patterns = nullptr,
                                    std::int32_t plane_b_vscroll = 0,
                                    std::int32_t plane_a_xscroll = 0,
                                    bool draw_plane_a = true,
                                    bool draw_plane_b = true,
                                    bool transparent_background = false) {
    const auto vdp = static_cast<std::size_t>(read_rom_u32(
        rom, descriptor, "Genesis 0x1406 descriptor is truncated"));
    const auto palette = static_cast<std::size_t>(read_rom_u32(
        rom, descriptor + 4U, "Genesis 0x1406 descriptor is truncated"));
    const auto plane_a = read_rom_u16(
        rom, vdp, "Genesis 0x1406 VDP table is truncated");
    const auto plane_b = read_rom_u16(
        rom, vdp + 2U, "Genesis 0x1406 VDP table is truncated");
    GenesisBootPatternData scene{
        std::vector<std::uint8_t>(0x10000U, 0),
        read_genesis_palette_set(rom, palette),
    };
    std::size_t map_list_offset = 0;
    load_genesis_1406_patterns(rom, descriptor, scene, map_list_offset);
    auto image = make_transparent_title_layer();
    if (!transparent_background) {
        for (auto& pixel : image.pixels) {
            pixel = {0, 0, 0, 255};
        }
    }
    blit_genesis_1406_maps(rom, scene, image, map_list_offset, plane_a,
                           plane_b, plane_b_vscroll, plane_a_xscroll,
                           draw_plane_a, draw_plane_b);
    if (patterns != nullptr) {
        *patterns = std::move(scene);
    }
    return image;
}

GenesisBootPatternData decode_genesis_intro_trivid_patterns(
    const std::vector<std::uint8_t>& rom) {
    GenesisBootPatternData scene{
        std::vector<std::uint8_t>(0x10000U, 0),
        read_genesis_palette_set(rom, 0x5B632U),
    };
    // 0x5AAAE reuses the same $FFBF40 overlap buffer for all three uploads.
    GenesisResourceDecoderState decoder_state;
    write_genesis_1530_vram_with_state(
        rom, 0x1E3F38U, 0x3020U, scene.pattern_vram, decoder_state);
    write_genesis_1530_vram_with_state(
        rom, 0x1E62BEU, 0xC000U, scene.pattern_vram, decoder_state);
    write_genesis_1530_vram_with_state(
        rom, 0xC1064U, 0xB560U, scene.pattern_vram, decoder_state);
    return scene;
}

RgbaImage decode_genesis_intro_trivid_impl(
    const std::vector<std::uint8_t>& rom, bool include_sprites) {
    auto scene = decode_genesis_intro_trivid_patterns(rom);
    auto image = make_transparent_title_layer();
    // Keep unused name-table cells transparent. The native compositor applies
    // the source Plane-A priority relative to the moving SAT layer.
    blit_genesis_1728_name_table(rom, scene, image, 0x1E620EU, 40U, 14U,
                                 0x700U, 0U, 0x8000U, true);
    if (include_sprites) {
        blit_genesis_sat_list(rom, scene, image, 0x1DF1D6U, 0xC2, 0x13C,
                              0xC000U);
        blit_genesis_sat_list(rom, scene, image, 0x1DF238U, 0x60, 0x123,
                              0x6000U);
    }
    return image;
}

GenesisIntroTrividLayers decode_genesis_intro_trivid_layers(
    const std::vector<std::uint8_t>& rom) {
    auto scene = decode_genesis_intro_trivid_patterns(rom);
    GenesisIntroTrividLayers result{
        make_transparent_title_layer(),
        make_transparent_title_layer(),
        make_transparent_title_layer(),
    };
    // 0x5AA96 draws the 40x14 cockpit name table before entering the 0x114
    // pass loop. The SAT writes below are intentionally separate layers: the
    // source calls 0x5AC0E at a fixed origin for the slain runner and moves
    // 0x5AC36 by FF F186/FF F188 fixed-point deltas each pass.
    blit_genesis_1728_name_table(rom, scene, result.background, 0x1E620EU,
                                 40U, 14U, 0x700U, 0U, 0x8000U, true);
    blit_genesis_sat_list(rom, scene, result.slain, 0x1DF1D6U, 0xC2, 0x13C,
                          0xC000U);
    blit_genesis_sat_list(rom, scene, result.moving, 0x1DF238U, 0x60, 0x123,
                          0x6000U);
    return result;
}

RgbaImage decode_genesis_intro_trivid(const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_trivid_impl(rom, true);
}

std::array<RgbaImage, native::kNativeIntroWildernessPanFrameCount>
decode_genesis_intro_wilderness_pan_frames_impl(
    const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeIntroWildernessPanFrameCount> frames{};
    GenesisBootPatternData wilderness;
    for (std::size_t index = 0; index < frames.size(); ++index) {
        // 0x59D7C updates the two VDP scroll registers while the scene is
        // being panned.  The old extractor kept the authored plane at its
        // final zero-scroll position and moved only the two head SAT pieces;
        // that made the Indian's head visibly fly into a stationary body.
        // Rebuild the map with the source scroll values and then apply the
        // independent SAT coordinates below.
        const auto plane_a_xscroll = static_cast<std::int32_t>(
            -0x40 + std::min<std::size_t>(index, 0x40U));
        const auto plane_b_vscroll = static_cast<std::int32_t>(
            -0x20 + std::min<std::size_t>(index / 2U, 0x20U));
        frames[index] = decode_genesis_1406_scene(
            rom, 0x5B4A8U, &wilderness, plane_b_vscroll,
            plane_a_xscroll);
        // 0x59D7C writes the Indian actor through the SAT list at 0x1DF1B2.
        // The 0x1406 Plane-A plate contains his robe/arms, but not the head
        // sprite.  The source starts the actor at (0x138,0xF1), then updates
        // DD46 toward 0x178 on every pan pass and DD48 toward 0xD1 on every
        // other pass before drawing it again.  Replaying those two source
        // registers keeps the head attached to the body throughout the pan;
        // using the terminal (0x178,0xD1) position for every frame was the
        // defect that made the head appear to fly into the body.
        const auto actor_x = static_cast<std::int32_t>(
            0x138 + std::min<std::size_t>(index, 0x40U));
        const auto actor_y = static_cast<std::int32_t>(
            0xF1 - std::min<std::size_t>(index / 2U, 0x20U));
        blit_genesis_sat_list(rom, wilderness, frames[index], 0x1DF1B2U,
                              actor_x, actor_y, 0x2000U);
    }
    return frames;
}

[[maybe_unused]] std::array<RgbaImage, native::kNativeIntroAmbushLegacyFrameCount>
decode_genesis_intro_ambush_frames_sampled_legacy(
    const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeIntroAmbushLegacyFrameCount> frames{};
    GenesisBootPatternData wilderness;
    const auto base = decode_genesis_1406_scene(
        rom, 0x5B4A8U, &wilderness);

    // 0x5A1EA owns eight independent SAT update routines. Their tables are
    // source animation data, not a host approximation of the characters:
    // each list is decoded with the same 0x1378 coordinate/origin contract
    // used by the pre-pan wilderness plate.
    constexpr std::array<std::size_t, 4> kActorLists{{
        0x1DF1B2U, 0x1DF1C4U, 0x1DF1B2U, 0x1DF1C4U}};
    constexpr std::array<std::size_t, 4> kActorDelays{{
        0x3CU, 0x24U, 0x3CU, 0x18U}};
    constexpr std::array<std::size_t, 5> kMuzzleLists{{
        0x1DF0CCU, 0x1DF0D6U, 0x1DF0D6U, 0x1DF0D6U, 0x1DF0D6U}};
    constexpr std::array<std::size_t, 5> kMuzzleDelays{{
        0x3CU, 0x06U, 0x0CU, 0x12U, 0x3CU}};
    constexpr std::array<std::int32_t, 5> kMuzzleX{{
        0x0001, 0x011A, 0x0000, 0x0000, 0x0001}};
    constexpr std::array<std::int32_t, 5> kMuzzleVelocity{{
        0x0000, 0x0004, 0x0006, 0x0008, 0x0000}};
    constexpr std::array<std::size_t, 4> kLeftFlashLists{{
        0x1DF182U, 0x1DF18CU, 0x1DF19EU, 0x1DF1A8U}};
    constexpr std::array<std::size_t, 6> kLeftBodyLists{{
        0x1DEF28U, 0x1DEF32U, 0x1DEF8CU, 0x1DEFC6U, 0x1DF010U,
        0x1DF06AU}};
    constexpr std::array<std::size_t, 7> kProjectileListsA{{
        0x1DF0EAU, 0x1DF0FCU, 0x1DF0E0U, 0x1DF0E0U,
        0x1DF0FCU, 0x1DF0E0U, 0x1DF130U}};
    constexpr std::array<std::int32_t, 7> kProjectileAY{{
        0x00BB, 0x00A3, 0x0113, 0x00E3,
        0x00F8, 0x0105, 0x00BC}};
    constexpr std::array<std::int32_t, 7> kProjectileAThresholds{{
        0x0040, 0x0040, 0x0145, 0x0183,
        0x0040, 0x0104, 0x0040}};
    constexpr std::array<std::size_t, 7> kProjectileADelays{{
        0x2AU, 0x30U, 0x4EU, 0x30U,
        0x6AU, 0x2AU, 0x3CU}};
    constexpr std::array<std::size_t, 8> kProjectileListsB{{
        0x1DF170U, 0x1DF130U, 0x1DF11EU, 0x1DF170U,
        0x1DF0EAU, 0x1DF170U, 0x1DF0FCU, 0x1DF0E0U}};
    constexpr std::array<std::int32_t, 8> kProjectileBY{{
        0x00AC, 0x00E6, 0x00BB, 0x0116,
        0x00B1, 0x012A, 0x00AC, 0x0116}};
    constexpr std::array<std::size_t, 8> kProjectileBDelays{{
        0x30U, 0x3CU, 0x30U, 0x3CU,
        0x30U, 0x3CU, 0x30U, 0x3CU}};
    constexpr std::array<std::size_t, 8> kProjectileListsC{{
        0x1DF0EAU, 0x1DF170U, 0x1DF11EU, 0x1DF0FCU,
        0x1DF130U, 0x1DF0E0U, 0x1DF170U, 0x1DF11EU}};
    constexpr std::array<std::int32_t, 8> kProjectileCY{{
        0x0120, 0x00D7, 0x0094, 0x00A6,
        0x010C, 0x00E2, 0x00C4, 0x00F8}};
    constexpr std::array<std::size_t, 8> kProjectileCDelays{{
        0x36U, 0x48U, 0x36U, 0x48U,
        0x36U, 0x48U, 0x36U, 0x48U}};
    constexpr std::array<std::size_t, 3> kLastShotLists{{
        0x1DF142U, 0x1DF154U, 0x1DF166U}};
    constexpr std::array<std::size_t, 3> kLastShotDelays{{
        0x03U, 0x06U, 0x09U}};
    constexpr std::array<std::size_t, 4> kMapLists{{
        0x1E2EBAU, 0x1E2F04U, 0x1E2F6CU, 0x1E2EBAU}};
    constexpr std::array<std::size_t, 4> kMapDelays{{
        0x0CU, 0x18U, 0x3CU, 0x0CU}};

    const auto render_sample = [&](std::size_t sample_frame) {
        auto image = base;

        std::size_t actor_phase = 0;
        std::size_t actor_list = kActorLists.front();
        std::size_t actor_next = 0;
        bool actor_loaded = false;

        std::size_t muzzle_phase = 0;
        std::size_t muzzle_list = kMuzzleLists.front();
        std::size_t muzzle_next = 0;
        std::int32_t muzzle_x = 0;
        std::int32_t muzzle_velocity = 0;
        bool muzzle_loaded = false;

        std::size_t flash_phase = 0;
        std::size_t flash_next = 0;
        std::size_t flash_first = kLeftFlashLists.front();
        std::size_t flash_second = kLeftFlashLists[1];
        bool flash_loaded = false;

        std::size_t body_phase = kLeftBodyLists.size();
        std::size_t body_list = kLeftBodyLists.front();
        std::size_t body_next = 0;
        std::int32_t body_x = 0;
        std::int32_t body_y = 0;
        bool body_loaded = false;

        std::size_t projectile_a_phase = 0;
        std::size_t projectile_a_list = kProjectileListsA.front();
        std::size_t projectile_a_next = 0;
        std::int32_t projectile_a_x = 0;
        std::int32_t projectile_a_threshold = 0;
        std::int32_t projectile_a_y = 0;
        bool projectile_a_active = false;

        std::size_t projectile_b_phase = 0;
        std::size_t projectile_b_list = kProjectileListsB.front();
        std::size_t projectile_b_next = 0;
        std::int32_t projectile_b_x = 0;
        std::int32_t projectile_b_y = 0;
        bool projectile_b_active = false;

        std::size_t projectile_c_phase = 0;
        std::size_t projectile_c_list = kProjectileListsC.front();
        std::size_t projectile_c_next = 0;
        std::int32_t projectile_c_x = 0;
        std::int32_t projectile_c_y = 0;
        bool projectile_c_active = false;

        std::size_t last_shot_phase = 0;
        std::size_t last_shot_list = kLastShotLists.front();
        std::size_t last_shot_next = 0;
        std::int32_t last_shot_x = 0x193;
        bool last_shot_active = false;

        std::size_t map_phase = 0;
        std::size_t map_next = 0;
        std::size_t map_list = kMapLists.front();
        bool map_loaded = false;

        // FFF14E/FFF150/FFF152/FFF154 are shared by the map hit routine and
        // the actor routine. Both source paths call 0x5A582 and then the
        // common body draw in 0x5A59C.
        const auto start_body = [&](std::int32_t x, std::int32_t y) {
            body_phase = 0;
            body_next = 0;
            body_x = x;
            body_y = y;
            body_loaded = false;
        };

        // 0xFFF124 is a fixed-point scroll term updated by 0x59F8E only
        // during the late tail. The projectile routines do not consume it;
        // only the muzzle/projectile list at 0x5A3BE does.
        std::int32_t scroll_fixed = 0;

        for (std::size_t frame = 0; frame <= sample_frame; ++frame) {
            // 0x59F8E runs before the map and dynamic SAT passes.
            if (frame >= 0x120U && frame <= 0x168U) {
                scroll_fixed += 0xAA;
            }
            const auto scroll_x = scroll_fixed >> 8;

            // 0x5A0B2 first patches the 13x12 name-table block. Its first
            // four phases select map lists; the remaining phases only drive
            // the source timing/sound and leave the final patch in place.
            if (map_phase < 0x0CU && frame >= map_next) {
                if (map_phase < kMapLists.size()) {
                    map_list = kMapLists[map_phase];
                    map_next = frame + kMapDelays[map_phase];
                    map_loaded = true;
                } else {
                    map_next = frame + 9U;
                }
                ++map_phase;
                if (map_phase == 9U) {
                    start_body(0x13D, 0x139);
                }
            }

            image = base;
            if (map_loaded) {
                // 0x5A0B2 invokes 0x1728 with D4=$0826, D7=$0C and
                // D1=$0B. This is the source's hit/death tile patch.
                blit_genesis_1728_name_table(
                    rom, wilderness, image, map_list, 13U, 12U, 0x826U,
                    0x0000U, 0, false);
            }

            // 0x5A252: the top-right actor advances through four lists,
            // pauses, then runs its terminal timing before disappearing.
            bool draw_actor = actor_loaded && actor_phase < 0x0CU;
            if (actor_phase < 0x0CU && frame >= actor_next) {
                if (actor_phase < kActorLists.size()) {
                    actor_list = kActorLists[actor_phase];
                    actor_next = frame + kActorDelays[actor_phase];
                    ++actor_phase;
                    actor_loaded = true;
                    draw_actor = true;
                } else if (actor_phase == 4U) {
                    // The actor routine disables the flash pair at its
                    // phase-four transition.
                    flash_phase = 4U;
                    actor_next = frame + 9U;
                    ++actor_phase;
                    draw_actor = true;
                } else {
                    actor_next = frame + 9U;
                    ++actor_phase;
                    draw_actor = true;
                    if (actor_phase == 9U) {
                        start_body(0x182, 0x0E2);
                    }
                }
            }
            if (draw_actor) {
                blit_genesis_sat_list(rom, wilderness, image, actor_list,
                                      0x178, 0xD1, 0x2000U);
            }

            // 0x5A3BE: the muzzle sequence loops back to table entry one
            // until the relative counter reaches $168, then stops.
            bool draw_muzzle = muzzle_loaded && muzzle_phase < 5U;
            if (muzzle_phase < 5U && frame >= muzzle_next) {
                const auto index = muzzle_phase;
                muzzle_list = kMuzzleLists[index];
                if (kMuzzleX[index] != 0) {
                    muzzle_x = kMuzzleX[index];
                }
                muzzle_velocity = kMuzzleVelocity[index];
                muzzle_next = frame + kMuzzleDelays[index];
                ++muzzle_phase;
                if (muzzle_phase >= 5U) {
                    muzzle_phase = frame < 0x168U ? 1U : 5U;
                }
                muzzle_loaded = true;
                draw_muzzle = muzzle_phase < 5U;
            }
            if (draw_muzzle) {
                muzzle_x += muzzle_velocity;
                blit_genesis_sat_list(rom, wilderness, image, muzzle_list,
                                      muzzle_x + scroll_x, 0x133, 0x6000U);
            }

            // 0x5A4C2: two flashing SAT lists share a six-frame cadence.
            // 0x5A252 can set FFF146=$4, which suppresses this pair.
            bool draw_flash = flash_loaded && flash_phase < 4U;
            if (flash_phase < 4U && frame >= flash_next) {
                flash_first = kLeftFlashLists[flash_phase];
                flash_second = kLeftFlashLists[
                    (flash_phase + 1U) % kLeftFlashLists.size()];
                flash_next = frame + 6U;
                ++flash_phase;
                if (flash_phase >= 4U) {
                    flash_phase = 0U;
                }
                flash_loaded = true;
                draw_flash = true;
            }
            if (draw_flash) {
                blit_genesis_sat_list(rom, wilderness, image, flash_first,
                                      0x183, 0xBE, 0x6000U);
                blit_genesis_sat_list(rom, wilderness, image, flash_second,
                                      0x1AB, 0xD7, 0x6000U);
            }

            // 0x5A59C draws the left body/death sequence at the position set
            // by either 0x5A0B2 or the actor's phase-nine callback.
            bool draw_body = body_loaded && body_phase < kLeftBodyLists.size();
            if (body_phase < kLeftBodyLists.size() && frame >= body_next) {
                body_list = kLeftBodyLists[body_phase];
                body_next = frame + 8U;
                ++body_phase;
                body_loaded = true;
                draw_body = true;
            }
            if (draw_body) {
                blit_genesis_sat_list(rom, wilderness, image, body_list,
                                      body_x, body_y, 0x6000U);
            }

            // The three projectile routines all subtract before drawing,
            // but test the prior position against their table threshold.
            bool draw_projectile_a = false;
            const auto projectile_a_due = frame >= projectile_a_next;
            if (projectile_a_due) {
                if (projectile_a_phase < kProjectileListsA.size()) {
                    projectile_a_list = kProjectileListsA[projectile_a_phase];
                    projectile_a_y = kProjectileAY[projectile_a_phase];
                    projectile_a_threshold =
                        kProjectileAThresholds[projectile_a_phase];
                    projectile_a_x = 0x200;
                    projectile_a_next = frame +
                        kProjectileADelays[projectile_a_phase];
                    ++projectile_a_phase;
                    projectile_a_active = true;
                } else {
                    projectile_a_active = false;
                }
            }
            if (projectile_a_active &&
                (projectile_a_due || projectile_a_x >= projectile_a_threshold)) {
                projectile_a_x -= 9;
                draw_projectile_a = true;
            }
            if (draw_projectile_a) {
                blit_genesis_sat_list(
                    rom, wilderness, image, projectile_a_list,
                    projectile_a_x, projectile_a_y + 0x18, 0x6000U);
            }

            bool draw_projectile_b = false;
            const auto projectile_b_due = frame >= projectile_b_next;
            if (projectile_b_due) {
                if (projectile_b_phase < kProjectileListsB.size()) {
                    projectile_b_list = kProjectileListsB[projectile_b_phase];
                    projectile_b_y = kProjectileBY[projectile_b_phase];
                    projectile_b_x = 0x200;
                    projectile_b_next = frame +
                        kProjectileBDelays[projectile_b_phase];
                    ++projectile_b_phase;
                    projectile_b_active = true;
                } else {
                    projectile_b_active = false;
                }
            }
            if (projectile_b_active &&
                (projectile_b_due || projectile_b_x >= 0x40)) {
                projectile_b_x -= 8;
                draw_projectile_b = true;
            }
            if (draw_projectile_b) {
                blit_genesis_sat_list(
                    rom, wilderness, image, projectile_b_list,
                    projectile_b_x, projectile_b_y + 0x18, 0x6000U);
            }

            bool draw_projectile_c = false;
            const auto projectile_c_due = frame >= projectile_c_next;
            if (projectile_c_due) {
                if (projectile_c_phase < kProjectileListsC.size()) {
                    projectile_c_list = kProjectileListsC[projectile_c_phase];
                    projectile_c_y = kProjectileCY[projectile_c_phase];
                    projectile_c_x = 0x200;
                    projectile_c_next = frame +
                        kProjectileCDelays[projectile_c_phase];
                    ++projectile_c_phase;
                    projectile_c_active = true;
                } else {
                    projectile_c_active = false;
                }
            }
            if (projectile_c_active &&
                (projectile_c_due || projectile_c_x >= 0x40)) {
                projectile_c_x -= 0x0B;
                draw_projectile_c = true;
            }
            if (draw_projectile_c) {
                blit_genesis_sat_list(
                    rom, wilderness, image, projectile_c_list,
                    projectile_c_x, projectile_c_y + 0x18, 0x6000U);
            }

            // 0x5A8DE starts only after six relative frames, then exposes
            // three short SAT bursts while advancing FFF184 by four pixels.
            if (frame >= last_shot_next) {
                if (frame < 6U) {
                    last_shot_active = false;
                } else if (last_shot_phase < kLastShotLists.size()) {
                    last_shot_list = kLastShotLists[last_shot_phase];
                    last_shot_next = frame + kLastShotDelays[last_shot_phase];
                    ++last_shot_phase;
                    last_shot_active = true;
                } else {
                    last_shot_active = false;
                }
            }
            if (last_shot_active) {
                last_shot_x += 4;
                blit_genesis_sat_list(rom, wilderness, image,
                                      last_shot_list, last_shot_x, 0x13C,
                                      0x6000U);
            }
        }
        return image;
    };

    for (std::size_t index = 0;
         index < native::kNativeIntroAmbushLegacyFrameCount; ++index) {
        frames[index] = render_sample(
            std::min<std::size_t>(
                index * native::kNativeIntroAmbushFrameStep, 0x1F0U));
    }
    return frames;
}

std::array<RgbaImage, native::kNativeIntroAmbushFrameCount>
decode_genesis_intro_ambush_frames_impl(
    const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeIntroAmbushFrameCount> frames{};
    GenesisBootPatternData wilderness;
    // 0x5A0B2 writes Plane A at $0826, while the descriptor's two authored
    // maps provide the opening background. The runners are SAT resources,
    // not a crop from the authored Plane-A map: 0x5A252 installs the first
    // actor list at its source origin before the animated death lists run.
    // Keeping the map and SAT layers separate is required for the source
    // phase-four SAT clear and the later 0x5A582 body starts.
    // 0x59D7C leaves both authored name-table planes live when the second
    // 0x5Axxx loop begins.  Plane A contains Michael's standing source art;
    // dropping it here and rebuilding only Plane B loses him before the ROM
    // has performed any transition.  The later 0x5A0B2 patches and SAT
    // updates are applied over this complete live scene below.
    auto base = decode_genesis_1406_scene(rom, 0x5B4A8U, &wilderness);
    // The pan leaves the actor SAT slot live in VDP memory, but 0x5A1EA
    // immediately takes ownership of that slot. Its 0x5A252 pass writes the
    // four actor lists and then deliberately stops writing after the
    // terminal nine-tick passes. Keep the ambush base clean here: baking the
    // pan actor into every frame makes a slain body persist underneath the
    // source death/impact lists.

    const auto pointer = [&rom](std::size_t table, std::size_t index,
                                std::size_t stride) {
        return static_cast<std::size_t>(read_rom_u32(
            rom, table + index * stride,
            "Genesis intro animation pointer table is truncated"));
    };
    const auto word = [&rom](std::size_t table, std::size_t index,
                             std::size_t stride, std::size_t offset) {
        return static_cast<std::uint32_t>(read_rom_u16(
            rom, table + index * stride + offset,
            "Genesis intro animation timing table is truncated"));
    };

    std::size_t actor_phase = 0;
    std::size_t actor_list = 0x1DF1B2U;
    std::uint32_t actor_next = 0;
    bool actor_loaded = false;
    std::size_t muzzle_phase = 0;
    std::size_t muzzle_list = 0x1DF0CCU;
    std::int32_t muzzle_x = 0;
    std::int32_t muzzle_velocity = 0;
    std::uint32_t muzzle_next = 0;
    std::size_t flash_phase = 0;
    std::size_t flash_first = 0x1DF182U;
    std::size_t flash_second = 0x1DF18CU;
    std::uint32_t flash_next = 0;
    bool flash_suppressed = false;
    std::size_t body_phase = 6;
    std::size_t body_list = 0x1DEF28U;
    std::int32_t body_x = 0;
    std::int32_t body_y = 0;
    std::uint32_t body_next = 0;
    bool body_loaded = false;
    std::size_t projectile_b_phase = 0;
    std::size_t projectile_b_list = 0x1DF0EAU;
    std::int32_t projectile_b_x = 0;
    std::int32_t projectile_b_y = 0;
    std::int32_t projectile_b_threshold = 0;
    std::uint32_t projectile_b_next = 0;
    std::size_t projectile_c_phase = 0;
    std::size_t projectile_c_list = 0x1DF170U;
    std::int32_t projectile_c_x = 0;
    std::int32_t projectile_c_y = 0;
    std::uint32_t projectile_c_next = 0;
    std::size_t projectile_d_phase = 0;
    std::size_t projectile_d_list = 0x1DF0EAU;
    std::int32_t projectile_d_x = 0;
    std::int32_t projectile_d_y = 0;
    std::uint32_t projectile_d_next = 0;
    std::size_t last_shot_phase = 0;
    std::size_t last_shot_list = 0x1DF142U;
    std::int32_t last_shot_x = 0x193;
    std::uint32_t last_shot_next = 0;
    bool last_shot_active = false;
    bool central_body_removed = false;
    bool indian_body_removed = false;
    std::size_t map_phase = 0;
    std::array<std::size_t, 4> map_lists{};
    std::size_t map_loaded_count = 0;
    std::uint32_t map_next = 0;
    bool map_loaded = false;
    std::int32_t scroll_fixed = 0;

    const auto start_body = [&](std::int32_t x, std::int32_t y) {
        body_phase = 0;
        body_next = 0;
        body_x = x;
        body_y = y;
        body_loaded = false;
        if (x == 0x13D) {
            central_body_removed = true;
        } else if (x == 0x182) {
            indian_body_removed = true;
        }
    };

    // One iteration below is one execution of the ROM's DBRA loop. Every
    // pointer, delay, projectile velocity, and removal boundary comes from
    // the 0x5Axxx tables; no video sampling interval is involved.
    for (std::size_t frame = 0;
         frame < native::kNativeIntroAmbushFrameCount; ++frame) {
        const auto now = static_cast<std::uint32_t>(frame);
        if (now >= 0x120U && now <= 0x168U) {
            scroll_fixed += 0xAA;
        }
        const auto scroll_x = scroll_fixed >> 8;

        // 0x5A0B2: four 0x1728 map writes, then the source nine-tick
        // callbacks; phase 9 starts the left body/death sequence.
        if (map_phase < 0x0CU && now >= map_next) {
            if (map_phase < 4U) {
                const auto loaded_list = pointer(0x5A1D2U, map_phase, 6U);
                map_next = now + word(0x5A1D2U, map_phase, 6U, 4U);
                map_lists[map_loaded_count++] = loaded_list;
                map_loaded = true;
                ++map_phase;
            } else if (map_phase == 4U) {
                ++map_phase;
                map_next = now + 9U;
            } else {
                map_next = now + 9U;
                ++map_phase;
                if (map_phase == 9U) {
                    start_body(0x13D, 0x139);
                }
            }
        }

        auto image = base;

        if (map_loaded) {
            // 0x5A0B2 writes four successive 0x1728 patches into the live
            // name table.  The later patches do not replace the earlier
            // writes; rebuilding a frame from the base therefore requires
            // replaying every patch that has fired so far.  Keeping only the
            // last pointer leaves the actors baked into the base plate.
            for (std::size_t patch_index = 0;
                 patch_index < map_loaded_count; ++patch_index) {
                blit_genesis_1728_name_table(
                    rom, wilderness, image, map_lists[patch_index], 13U, 12U,
                    0x826U, 0x0000U, 0, false);
            }
        }

        // The authored Plane-A cells are replaced by the source body/death
        // passes at the two distinct 0x5A582 origins. Clear only those actor
        // windows; Michael's authored window deliberately remains live.
        const auto clear_region = [&image](std::int32_t left,
                                           std::int32_t top,
                                           std::int32_t width,
                                           std::int32_t height) {
            for (std::int32_t y = top; y < top + height; ++y) {
                for (std::int32_t x = left; x < left + width; ++x) {
                    if (x < 0 || y < 0 || x >= image.width ||
                        y >= image.height) {
                        continue;
                    }
                    image.pixels[static_cast<std::size_t>(y) * image.width +
                                 static_cast<std::size_t>(x)] =
                        {0, 0, 0, 255};
                }
            }
        };
        if (central_body_removed) {
            clear_region(112, 128, 160, 96);
        }

        // 0x5A252: four actor list records followed by eight nine-tick
        // terminal passes. The list is no longer written at phase $C.
        if (actor_phase < 0x0CU && now >= actor_next) {
            if (actor_phase < 4U) {
                actor_list = pointer(0x5A398U, actor_phase, 6U);
                actor_next = now + word(0x5A398U, actor_phase, 6U, 4U);
                ++actor_phase;
                actor_loaded = true;
            } else {
                if (actor_phase == 4U) {
                    flash_suppressed = true;
                }
                actor_next = now + 9U;
                ++actor_phase;
                if (actor_phase == 9U) {
                    start_body(0x182, 0x0E2);
                }
            }
        }
        // 0x5A252 writes the four actor SAT lists first.  At phase four the
        // ROM calls 0x5B2C2, which clears that SAT slot; phases five through
        // eight only apply 0x5B2E2's coordinate decay to the already-cleared
        // slot.  Re-blitting the last actor list through phase eleven leaves
        // the slain runners permanently visible, which is exactly the stale
        // body defect this source-timed renderer must avoid.
        if (actor_loaded && actor_phase < 4U) {
            blit_genesis_sat_list(rom, wilderness, image, actor_list,
                                  0x178, 0xD1, 0x2000U);
        }
        // 0x5A252 phase $C calls 0x5B32C with D4=$3B8, D7=$9,
        // D1=$8.  That is a 10x9 name-table clear at (224,56), which
        // removes the authored Indian layer after the SAT slot has already
        // been cleared and decayed.  Clearing the whole guessed actor box
        // at phase nine was both too early and too large.
        if (actor_phase >= 0x0CU) {
            clear_region(224, 56, 80, 72);
        }

        // 0x5A3BE: long pointer, delay, initial X, velocity. The fifth
        // record loops to phase one until relative time $168. The source
        // passes D2=$133 directly; there is no host +$18 Y adjustment.
        if (muzzle_phase < 5U && now >= muzzle_next) {
            const auto record = muzzle_phase;
            muzzle_list = pointer(0x5A482U, record, 10U);
            const auto initial_x = word(0x5A482U, record, 10U, 6U);
            if (initial_x != 0U) {
                muzzle_x = static_cast<std::int32_t>(initial_x);
            }
            muzzle_velocity = static_cast<std::int32_t>(static_cast<
                std::int16_t>(word(0x5A482U, record, 10U, 8U)));
            muzzle_next = now + word(0x5A482U, record, 10U, 4U);
            ++muzzle_phase;
            if (muzzle_phase >= 5U) {
                muzzle_phase = now < 0x168U ? 1U : 5U;
            }
        }
        if (!flash_suppressed && muzzle_phase < 5U) {
            muzzle_x += muzzle_velocity;
            blit_genesis_sat_list(rom, wilderness, image, muzzle_list,
                                  muzzle_x + scroll_x, 0x133, 0x6000U);
        }

        // 0x5A4C2: two SAT lists, each at a fixed origin, cycled every six
        // source passes until FFF146=$4 suppresses the pair.
        if (!flash_suppressed) {
            if (now >= flash_next) {
                flash_first = pointer(0x5A562U, flash_phase, 4U);
                flash_second = pointer(
                    0x5A562U, (flash_phase + 1U) % 4U, 4U);
                flash_next = now + 6U;
                flash_phase = (flash_phase + 1U) % 4U;
            }
            blit_genesis_sat_list(rom, wilderness, image, flash_first,
                                  0x183, 0xBE, 0x6000U);
            blit_genesis_sat_list(rom, wilderness, image, flash_second,
                                  0x1AB, 0xD7, 0x6000U);
        }

        // 0x5A59C redraws the currently loaded body list for this source
        // pass. That includes the final smoke list, but once FFF14E reaches
        // six the routine returns and must not repaint it forever.
        bool draw_body_this_pass = false;
        if (body_phase < 6U && now >= body_next) {
            body_list = pointer(0x5A602U, body_phase, 4U);
            body_next = now + 8U;
            ++body_phase;
            body_loaded = true;
            draw_body_this_pass = true;
        }
        if (body_loaded && body_phase < 6U) {
            draw_body_this_pass = true;
        }
        if (draw_body_this_pass) {
            blit_genesis_sat_list(rom, wilderness, image, body_list, body_x,
                                  body_y, 0x6000U);
        }

        // 0x5A628: seven records carry pointer/Y/threshold/delay. X is
        // decremented by nine every source pass while it remains >= threshold.
        if (projectile_b_phase < 7U && now >= projectile_b_next) {
            const auto record = projectile_b_phase;
            projectile_b_list = pointer(0x5A6C8U, record, 10U);
            projectile_b_y = static_cast<std::int32_t>(
                word(0x5A6C8U, record, 10U, 4U));
            projectile_b_threshold = static_cast<std::int32_t>(
                word(0x5A6C8U, record, 10U, 6U));
            projectile_b_next = now + word(0x5A6C8U, record, 10U, 8U);
            projectile_b_x = 0x200;
            ++projectile_b_phase;
        }
        if (projectile_b_phase != 0U && projectile_b_phase <= 7U &&
            projectile_b_x >= projectile_b_threshold) {
            projectile_b_x -= 9;
            blit_genesis_sat_list(rom, wilderness, image, projectile_b_list,
                                  projectile_b_x, projectile_b_y + 0x18,
                                  0x6000U);
        }

        // 0x5A71C and 0x5A7F8: eight pointer/Y/delay records with -8 and
        // -$B source motion respectively.
        if (projectile_c_phase < 8U && now >= projectile_c_next) {
            const auto record = projectile_c_phase;
            projectile_c_list = pointer(0x5A7AAU, record, 8U);
            projectile_c_y = static_cast<std::int32_t>(
                word(0x5A7AAU, record, 8U, 4U));
            projectile_c_next = now + word(0x5A7AAU, record, 8U, 6U);
            projectile_c_x = 0x200;
            ++projectile_c_phase;
        }
        if (projectile_c_phase != 0U && projectile_c_phase <= 8U &&
            projectile_c_x >= 0x40) {
            projectile_c_x -= 8;
            blit_genesis_sat_list(rom, wilderness, image, projectile_c_list,
                                  projectile_c_x, projectile_c_y + 0x18,
                                  0x6000U);
        }
        if (projectile_d_phase < 8U && now >= projectile_d_next) {
            const auto record = projectile_d_phase;
            projectile_d_list = pointer(0x5A888U, record, 8U);
            projectile_d_y = static_cast<std::int32_t>(
                word(0x5A888U, record, 8U, 4U));
            projectile_d_next = now + word(0x5A888U, record, 8U, 6U);
            projectile_d_x = 0x200;
            ++projectile_d_phase;
        }
        if (projectile_d_phase != 0U && projectile_d_phase <= 8U &&
            projectile_d_x >= 0x40) {
            projectile_d_x -= 0x0B;
            blit_genesis_sat_list(rom, wilderness, image, projectile_d_list,
                                  projectile_d_x, projectile_d_y + 0x18,
                                  0x6000U);
        }

        // 0x5A8DE: six-tick delay, then the three source Michael lists. The
        // x position advances by four on every pass and the third list is
        // removed when its nine-tick deadline expires.
        if (now < last_shot_next) {
            if (last_shot_active) {
                last_shot_x += 4;
                blit_genesis_sat_list(rom, wilderness, image, last_shot_list,
                                      last_shot_x, 0x13C, 0x6000U);
            }
        } else if (now >= 6U && last_shot_phase < 3U) {
            last_shot_list = pointer(0x5A958U, last_shot_phase, 6U);
            last_shot_next = now +
                word(0x5A958U, last_shot_phase, 6U, 4U);
            ++last_shot_phase;
            last_shot_active = true;
            last_shot_x += 4;
            blit_genesis_sat_list(rom, wilderness, image, last_shot_list,
                                  last_shot_x, 0x13C, 0x6000U);
        } else {
            last_shot_active = false;
        }

        frames[frame] = std::move(image);
    }
    return frames;
}

std::array<RgbaImage, native::kNativeIntroStageCount>
decode_genesis_intro_stages_impl(const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeIntroStageCount> stages{};
    stages[0] = decode_genesis_1406_scene(rom, 0x5B574U);
    GenesisBootPatternData wilderness;
    stages[1] = decode_genesis_1406_scene(rom, 0x5B4A8U, &wilderness);
    // 0x59D7C's pre-pan draw uses the wilderness actor SAT at 0x1DF1B2.
    // Keep this source layer separate from the later animated SAT calls.
    blit_genesis_sat_list(rom, wilderness, stages[1], 0x1DF1B2U, 0x178, 0xD1,
                          0x2000U);
    // 0x59D7C's pre-pan draw uses the wilderness actor SAT at 0x1DF1B2.
    // Keep this as the source plate; the later 0x5Axxx calls are animated
    // overlays and must not be baked into the still.
    stages[2] = decode_genesis_intro_trivid(rom);
    // The plane-A 27x8 vehicle is a transient foreground layer. Keep the
    // settled airport/city plate as plane B; the native intro renderer
    // selects the vehicle layer during its source-timed window.
    stages[3] = decode_genesis_1406_scene(rom, 0x5B528U, nullptr, 0x78, 0,
                                          false, true);
    return stages;
}

std::array<RgbaImage, native::kNativeIntroHeliFrameCount>
decode_genesis_intro_heli_frames_impl(const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeIntroHeliFrameCount> frames{};
    GenesisBootPatternData scene;
    // 0x1406 supplies the patterns and palette used by both the Plane-A map
    // and the adjacent 0x1378 SAT overlays. Do not retain its opaque scene;
    // each overlay is composed onto an independent transparent layer.
    (void)decode_genesis_1406_scene(rom, 0x5B528U, &scene, 0, 0, false,
                                    false, true);
    for (std::size_t index = 0; index < frames.size(); ++index) {
        frames[index] = make_transparent_layer(512U, native::kScreenHeight);
        const auto list = static_cast<std::size_t>(read_rom_u32(
            rom, 0x5B180U + index * 4U,
            "Genesis airport overlay pointer table is truncated"));
        // 0x5B100 passes D1=DD08+$1FC and D2=$EF. The extracted reference
        // layer is therefore translated by DD08 at runtime. The 512px canvas
        // is intentional: at DD08=0 the source SAT list extends past the
        // right edge of the 320px viewport, and clipping it during extraction
        // would make the craft's rotor/tail disappear before the native scroll
        // begins.
        blit_genesis_sat_list(rom, scene, frames[index], list, 0x1FC, 0xEF,
                              0x2000U);
    }
    return frames;
}

std::array<RgbaImage, native::kNativeIntroAircraftMovementFrameCount>
decode_genesis_intro_aircraft_movement_frames_impl(
    const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeIntroAircraftMovementFrameCount>
        frames{};
    GenesisBootPatternData scene;
    (void)decode_genesis_1406_scene(rom, 0x5B528U, &scene, 0, 0, false,
                                    false, true);
    // 0x5B1B6 switches between these two lists when the source Y band reaches
    // the middle strip. Both use the same 0x1378 origin contract.
    constexpr std::array<std::size_t,
                         native::kNativeIntroAircraftMovementFrameCount> lists{{
        0x1DF3F4U, 0x1DF3FEU}};
    for (std::size_t index = 0; index < frames.size(); ++index) {
        frames[index] = make_transparent_layer(320U, 224U);
        blit_genesis_sat_list(rom, scene, frames[index], lists[index],
                              0x90, 0x90, 0x2000U);
    }
    return frames;
}

RgbaImage decode_genesis_intro_aircraft_plane_a_impl(
    const std::vector<std::uint8_t>& rom) {
    // 0x5AD1A's first map is the 27x8 vehicle layer at Plane A $074A. Render
    // that map at a normalized origin here; NativePort restores the source
    // $074A origin (x=296, y=112) and applies DD08 on every source pass.
    GenesisBootPatternData scene;
    (void)decode_genesis_1406_scene(rom, 0x5B528U, &scene, 0, 0, false,
                                    false, true);
    auto image = make_transparent_title_layer();
    blit_genesis_1728_name_table(rom, scene, image, 0x1E87BAU, 27U, 8U,
                                 0U, 0U, 0x2000U, false, 0, true, 0);
    return image;
}

std::array<RgbaImage, native::kNativeIntroStageCount>
decode_genesis_intro_backgrounds_impl(const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeIntroStageCount> backgrounds{};
    backgrounds[0] = decode_genesis_1406_scene(rom, 0x5B574U);
    // The actor used during the wilderness pan is a separate SAT resource;
    // do not bake it into the reusable background plate.
    backgrounds[1] = decode_genesis_1406_scene(rom, 0x5B4A8U);
    backgrounds[2] = decode_genesis_intro_trivid_impl(rom, false);
    backgrounds[3] = decode_genesis_1406_scene(rom, 0x5B528U, nullptr, 0x78,
                                                0, false, true);
    return backgrounds;
}

std::vector<GenesisGraphicsResource>
decode_genesis_intro_sprite_resources_impl(
    const std::vector<std::uint8_t>& rom) {
    const auto trivid = decode_genesis_intro_trivid_patterns(rom);
    GenesisBootPatternData wilderness;
    (void)decode_genesis_1406_scene(rom, 0x5B4A8U, &wilderness, 0, 0, false,
                                    false, true);
    std::vector<GenesisGraphicsResource> resources;
    const auto add = [&](const GenesisBootPatternData& scene,
                         const std::string& role, std::size_t table,
                         std::size_t index, std::size_t source,
                         std::uint16_t attr_or) {
        GenesisGraphicsResource resource;
        resource.role = role;
        resource.table_offset = table;
        resource.table_index = index;
        resource.source_offset = source;
        resource.attr_or = attr_or;
        resource.image = make_transparent_layer(1024U, 512U);
        // The extra 0x80 canvas offset keeps signed SAT coordinates (including
        // the actor lists that begin above the visible origin) in bounds. The
        // extractor later crops this host-only staging canvas to the sprite.
        blit_genesis_sat_list(rom, scene, resource.image, source, 0x100, 0x100,
                              attr_or);
        resources.push_back(std::move(resource));
    };
    const auto add_table = [&](const GenesisBootPatternData& scene,
                               const std::string& role, std::size_t table,
                               std::size_t count, std::size_t stride,
                               std::uint16_t attr_or) {
        for (std::size_t index = 0; index < count; ++index) {
            const auto source = static_cast<std::size_t>(read_rom_u32(
                rom, table + index * stride,
                "Genesis intro sprite pointer table is truncated"));
            add(scene, role, table, index, source, attr_or);
        }
    };

    // The two tri-vid SAT lists are direct source lists, not pointer-table
    // records. Their palette attributes are the values used by 0x5AAAE.
    add(trivid, "trivid-left", 0x1DF1D6U, 0, 0x1DF1D6U, 0xC000U);
    add(trivid, "trivid-right", 0x1DF238U, 0, 0x1DF238U, 0x6000U);

    // These tables are the exact 0x5Axxx pointer selections used by the
    // ambush loop: actor, muzzle, flash, body/death, three projectile paths,
    // and the terminal Michael hit sequence.
    add_table(wilderness, "ambush-actor", 0x5A398U, 4, 6, 0x2000U);
    add_table(wilderness, "ambush-muzzle", 0x5A482U, 5, 10, 0x6000U);
    add_table(wilderness, "ambush-flash", 0x5A562U, 4, 4, 0x6000U);
    add_table(wilderness, "ambush-body", 0x5A602U, 6, 4, 0x6000U);
    add_table(wilderness, "ambush-projectile-b", 0x5A6C8U, 7, 10, 0x6000U);
    add_table(wilderness, "ambush-projectile-c", 0x5A7AAU, 8, 8, 0x6000U);
    add_table(wilderness, "ambush-projectile-d", 0x5A888U, 8, 8, 0x6000U);
    add_table(wilderness, "ambush-michael", 0x5A958U, 3, 6, 0x6000U);
    return resources;
}

std::vector<GenesisGraphicsResource>
decode_genesis_intro_ambush_background_resources_impl(
    const std::vector<std::uint8_t>& rom) {
    GenesisBootPatternData scene;
    (void)decode_genesis_1406_scene(rom, 0x5B4A8U, &scene, 0, 0, false,
                                    false, true);
    std::vector<GenesisGraphicsResource> resources;
    for (std::size_t index = 0; index < 4U; ++index) {
        const auto table = 0x5A1D2U;
        const auto source = static_cast<std::size_t>(read_rom_u32(
            rom, table + index * 6U,
            "Genesis intro background pointer table is truncated"));
        GenesisGraphicsResource resource;
        resource.role = "ambush-map-patch";
        resource.table_offset = table;
        resource.table_index = index;
        resource.source_offset = source;
        resource.image = make_transparent_title_layer();
        blit_genesis_1728_name_table(rom, scene, resource.image, source, 13U,
                                     12U, 0x826U, 0x0000U, 0, false);
        resources.push_back(std::move(resource));
    }
    return resources;
}

} // namespace

RgbaImage decode_genesis_boot_background(
    const std::vector<std::uint8_t>& rom) {
    auto image = make_transparent_title_layer();
    for (auto& pixel : image.pixels) {
        pixel = {0, 0, 0, 255};
    }
    const auto boot = decode_genesis_boot_pattern_data(rom);
    // 0x5DB10 tilemap 0 / 0x5DAA8: 12x4 at Plane B $E51A, source 0x1FFA66.
    // Index 0 stays transparent so the letters composite over plane A snow.
    blit_genesis_name_table(rom, boot, image, 0x1FFA66U, 12U, 4U, 0xE51AU,
                            0xE000U, 0, true);
    return image;
}

RgbaImage decode_genesis_boot_static(
    const std::vector<std::uint8_t>& rom) {
    auto image = make_transparent_title_layer();
    const auto boot = decode_genesis_boot_pattern_data(rom);
    // 0x5DB10 tilemap 1: 40x9 at Plane A $C400, source 0x1FFB66, attr $2000.
    // Index 0 is the snow holes; 0x5DAC4 HSCROLLS this plane only.
    blit_genesis_name_table(rom, boot, image, 0x1FFB66U, 40U, 9U, 0xC400U,
                            0xC000U, 0x2000U, true);
    return image;
}

RgbaImage decode_genesis_boot_guy(const std::vector<std::uint8_t>& rom) {
    auto image = make_transparent_title_layer();
    const auto boot = decode_genesis_boot_pattern_data(rom);
    constexpr std::int32_t kOriginX = native::kBootGuyOriginX;
    constexpr std::int32_t kOriginY = native::kBootGuyComposeY;
    constexpr std::uint16_t kGuyAttr = 0x6000;
    blit_genesis_sat_list(rom, boot, image, 0x1D4E4U, kOriginX, kOriginY,
                          kGuyAttr);
    blit_genesis_sat_list(rom, boot, image, 0x1D50EU, kOriginX, kOriginY,
                          kGuyAttr);
    return image;
}

std::array<RgbaImage, native::kNativeBootSegaPhaseCount>
decode_genesis_boot_sega_frames(const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeBootSegaPhaseCount> frames{};
    const auto boot = decode_genesis_boot_pattern_data(rom);
    constexpr std::uint32_t kLeftTable = 0x5DA20U;
    constexpr std::uint32_t kRightTable = 0x5DA38U;
    constexpr std::int32_t kLeftX = 0x158;
    constexpr std::int32_t kRightX = 0xD8;
    constexpr std::int32_t kSegaY = 0x120;
    constexpr std::uint16_t kSegaAttr = 0x2000;
    for (std::size_t index = 0; index < frames.size(); ++index) {
        frames[index] = make_transparent_title_layer();
        const auto left = static_cast<std::size_t>(
            read_rom_u32(rom, kLeftTable + index * 4U,
                         "Genesis boot SEGA pointer table is truncated"));
        const auto right = static_cast<std::size_t>(
            read_rom_u32(rom, kRightTable + index * 4U,
                         "Genesis boot SEGA pointer table is truncated"));
        blit_genesis_sat_list(rom, boot, frames[index], left, kLeftX, kSegaY,
                              kSegaAttr);
        blit_genesis_sat_list(rom, boot, frames[index], right, kRightX, kSegaY,
                              kSegaAttr);
    }
    return frames;
}

std::array<RgbaImage, native::kNativeBootLogoStrobeCount>
decode_genesis_boot_logo_strobe(const std::vector<std::uint8_t>& rom) {
    std::array<RgbaImage, native::kNativeBootLogoStrobeCount> frames{};
    const auto boot = decode_genesis_boot_pattern_data(rom);
    constexpr std::size_t kStrobeTable = 0x5DCE6U;
    for (std::size_t index = 0; index < frames.size(); ++index) {
        auto strobe = boot;
        // 0x5D8BA copies eight words from 0x5DCE6 + (DF92 << 1) into live
        // CRAM at FFD884 (line 0 colors 3..10).
        for (std::size_t color = 0; color < 8U; ++color) {
            const auto word = read_rom_u16(
                rom, kStrobeTable + (index + color) * 2U,
                "Genesis boot logo strobe table is truncated");
            strobe.palette[3U + color] = decode_genesis_cram_word(word, false);
        }
        frames[index] = make_transparent_title_layer();
        blit_genesis_name_table(rom, strobe, frames[index], 0x1FFA66U, 12U, 4U,
                                0xE51AU, 0xE000U, 0, true);
    }
    return frames;
}

RgbaImage decode_genesis_boot_fasa(const std::vector<std::uint8_t>& rom) {
    auto image = make_transparent_title_layer();
    const auto boot = decode_genesis_boot_pattern_data(rom);
    // 0x5D886: 16x5 at $E518 from 0x1FFAC6, same tiles as the SEGA 12x4.
    blit_genesis_name_table(rom, boot, image, 0x1FFAC6U, 16U, 5U, 0xE518U,
                            0xE000U, 0, true);
    return image;
}

std::array<RgbaImage, native::kNativeIntroStageCount>
decode_genesis_intro_stages(const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_stages_impl(rom);
}

std::array<RgbaImage, native::kNativeIntroStageCount>
decode_genesis_intro_backgrounds(const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_backgrounds_impl(rom);
}

std::vector<GenesisGraphicsResource>
decode_genesis_intro_sprite_resources(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_sprite_resources_impl(rom);
}

std::vector<GenesisGraphicsResource>
decode_genesis_intro_ambush_background_resources(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_ambush_background_resources_impl(rom);
}

std::array<RgbaImage, native::kNativeIntroHeliFrameCount>
decode_genesis_intro_heli_frames(const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_heli_frames_impl(rom);
}

std::array<RgbaImage, native::kNativeIntroAircraftMovementFrameCount>
decode_genesis_intro_aircraft_movement_frames(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_aircraft_movement_frames_impl(rom);
}

RgbaImage decode_genesis_intro_aircraft_plane_a(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_aircraft_plane_a_impl(rom);
}

std::array<RgbaImage, native::kNativeIntroWildernessPanFrameCount>
decode_genesis_intro_wilderness_pan_frames(
    const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_wilderness_pan_frames_impl(rom);
}

std::array<RgbaImage, native::kNativeIntroAmbushFrameCount>
decode_genesis_intro_ambush_frames(const std::vector<std::uint8_t>& rom) {
    return decode_genesis_intro_ambush_frames_impl(rom);
}

RgbaImage decode_genesis_cab_map(const std::vector<std::uint8_t>& rom) {
    // 0x134D2 follows 0x59D12's VDP setup. The map resource is not one of
    // the field-mode assets: it uploads its own pattern stream at $6420 and
    // then writes the 0xD4298 0x1728 name table at $2208. Treat the latter
    // as a plane-local address with the source screen's $2000 origin; that
    // gives the same 26x17 tile window that the VDP displays.
    GenesisBootPatternData scene{
        decode_genesis_1530_vram(rom, 0xD2A2EU, 0x6420U),
        read_genesis_palette_set(rom, 0x5B732U),
    };
    auto source_image = make_transparent_title_layer();
    for (auto& pixel : source_image.pixels) {
        pixel = {0, 0, 0, 255};
    }
    blit_genesis_1728_name_table(rom, scene, source_image, 0xD4298U, 26U,
                                 17U, 0x2208U, 0x2000U, 0, false);

    // 0x134D2 enters 0x13726 with FFE18B set during this opening path, so
    // FFF09C is forced to four and D445D[4] selects record zero. The visible
    // settled selection is the record's terminal zero-size outline slot
    // (D4464+8, D4536+8); reproduce its source tile list from D4446 rather
    // than drawing a host rectangle over the map.
    const auto draw_cursor_tile = [&](std::uint16_t address,
                                      std::uint16_t word) {
        const auto relative = static_cast<std::uint16_t>(
            address - 0xC000U);
        const auto tile_x = static_cast<std::int32_t>((relative % 0x80U) / 2U);
        const auto tile_y = static_cast<std::int32_t>(relative / 0x80U);
        const auto tile_attributes = word;
        blit_genesis_tile(
            source_image, scene,
            static_cast<std::size_t>(tile_attributes & 0x07FFU),
            static_cast<std::size_t>((tile_attributes >> 13U) & 0x03U),
            (tile_attributes & 0x0800U) != 0,
            (tile_attributes & 0x1000U) != 0, tile_x * 8, tile_y * 8, true);
    };
    const auto position = read_rom_u16(
        rom, 0xD4464U + 4U * 2U,
        "Genesis cab cursor record is truncated");
    const auto width = static_cast<unsigned>(rom[0xD4536U + 8U]);
    const auto height = static_cast<unsigned>(rom[0xD4536U + 9U]);
    const auto base = static_cast<std::uint16_t>(0xC208U + position);
    auto cursor_source = 0xD4446U;
        const auto top_left = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        draw_cursor_tile(base, top_left);
        const auto top_middle = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        for (unsigned count = 0; count <= width; ++count) {
            draw_cursor_tile(static_cast<std::uint16_t>(
                                 base + 2U * (count + 1U)),
                             top_middle);
        }
        const auto top_right = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        draw_cursor_tile(static_cast<std::uint16_t>(
                             base + 2U * (width + 2U)),
                         top_right);

        const auto left = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        for (unsigned count = 0; count <= height + 1U; ++count) {
            draw_cursor_tile(static_cast<std::uint16_t>(
                                 base + 0x80U * (count + 1U)),
                             left);
        }
        const auto right = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        for (unsigned count = 0; count <= height + 1U; ++count) {
            draw_cursor_tile(static_cast<std::uint16_t>(
                                 base + 0x80U * (count + 1U) +
                                     2U * (width + 2U)),
                             right);
        }

        const auto bottom_left = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        draw_cursor_tile(static_cast<std::uint16_t>(
                             base + 0x80U * (height + 2U)),
                         bottom_left);
        const auto bottom_middle = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        for (unsigned count = 0; count <= width; ++count) {
            draw_cursor_tile(static_cast<std::uint16_t>(
                                 base + 0x80U * (height + 2U) +
                                     2U * (count + 1U)),
                             bottom_middle);
        }
        const auto bottom_right = read_rom_u16(
            rom, cursor_source, "Genesis cab cursor tile list is truncated");
        cursor_source += 2U;
        draw_cursor_tile(static_cast<std::uint16_t>(
                             base + 0x80U * (height + 2U) +
                                 2U * (width + 2U)),
                         bottom_right);

    // The supplied gameplay capture presents this 208x136 VDP plate in the
    // 166x130 viewport at (62,30). Keep the source-derived tile decode above
    // intact and bake only that capture-space presentation transform here.
    constexpr std::int32_t kSourceX = 32;
    constexpr std::int32_t kSourceY = 32;
    constexpr std::int32_t kTargetX = 62;
    constexpr std::int32_t kTargetY = 30;
    constexpr std::int32_t kSourceWidth = 208;
    constexpr std::int32_t kSourceHeight = 136;
    constexpr std::int32_t kTargetWidth = 166;
    constexpr std::int32_t kTargetHeight = 130;
    auto image = make_transparent_title_layer();
    for (auto& pixel : image.pixels) {
        pixel = {0, 0, 0, 255};
    }
    for (std::int32_t y = 0; y < kTargetHeight; ++y) {
        const auto source_y = kSourceY +
            (y * kSourceHeight) / kTargetHeight;
        for (std::int32_t x = 0; x < kTargetWidth; ++x) {
            const auto source_x = kSourceX +
                (x * kSourceWidth) / kTargetWidth;
            image.pixels[static_cast<std::size_t>(kTargetY + y) * image.width +
                         static_cast<std::size_t>(kTargetX + x)] =
                source_image.pixels[static_cast<std::size_t>(source_y) *
                                        source_image.width +
                                    static_cast<std::size_t>(source_x)];
        }
    }
    return image;
}

RgbaImage decode_genesis_pocket_secretary_background(
    const std::vector<std::uint8_t>& rom) {
    // ROM 0x10A7E is the Pocket Secretary initializer. It is a separate
    // 0x1232 composition from the field command bezel: the 34x3 top edge is
    // written at $E000, the two 3x21 right-edge strips at $E180/$E1BE, and
    // the 34x4 bottom edge at $EC00. Preserve the actual plane geometry so
    // the native text/cursor layer can use the same window coordinates.
    GenesisBootPatternData scene{
        decode_genesis_1530_vram(rom, 0xCAB0CU, 0x1E20U),
        read_genesis_palette_set(rom, 0xD29D6U),
    };
    auto image = make_transparent_title_layer();
    for (auto& pixel : image.pixels) {
        pixel = {0, 0, 0, 255};
    }
    blit_genesis_name_table(rom, scene, image, 0xCDA0EU, 34U, 3U,
                            0xE000U, 0xE000U, 0, false);
    blit_genesis_name_table(rom, scene, image, 0xCDADAU, 3U, 21U,
                            0xE180U, 0xE000U, 0, false);
    blit_genesis_name_table(rom, scene, image, 0xCDB58U, 3U, 21U,
                            0xE1BEU, 0xE000U, 0, false);
    blit_genesis_name_table(rom, scene, image, 0xCDBD6U, 34U, 4U,
                            0xEC00U, 0xE000U, 0, false);
    return image;
}

RgbaImage decode_genesis_field_hud(const std::vector<std::uint8_t>& rom) {
    // 0x334C is the field-loop HUD refresh called after 0x1745E's plane
    // setup. Its 0x6A5BE pattern upload is followed by five raw 0x1232
    // writes. The source addresses are in the plane's $F000 alias; normalize
    // them to the visible $E000 plane first. The field presentation window
    // then shifts this 48px source strip left by 16 pixels, leaving the
    // rightmost 16px black in the 320px capture.
    GenesisBootPatternData scene{
        std::vector<std::uint8_t>(0x10000U, 0),
        read_genesis_command_palette_set(rom),
    };
    write_genesis_f5c_vram(rom, 0x6A5BEU, 0xDAC0U, 0x4C0U,
                           scene.pattern_vram);
    auto image = make_transparent_title_layer();
    blit_genesis_name_table(rom, scene, image, 0x6ACE2U, 6U, 2U,
                            0xF044U, 0xF000U, 0x6000U, true);
    blit_genesis_name_table(rom, scene, image, 0x6ACE2U, 6U, 2U,
                            0xFD44U, 0xF000U, 0x6000U, true);
    for (const auto dest : {0xF144U, 0xF544U, 0xF944U}) {
        blit_genesis_name_table(rom, scene, image, 0x6AA7EU, 6U, 8U,
                                dest, 0xF000U, 0x6000U, true);
    }
    constexpr std::int32_t kRomHudX = 272;
    constexpr std::int32_t kDisplayedHudX = native::kFieldHudX;
    RgbaImage shifted = make_transparent_title_layer();
    for (std::uint16_t y = 0; y < image.height; ++y) {
        for (std::uint16_t x = 0; x < image.width; ++x) {
            const auto source_x = static_cast<std::int32_t>(x);
            if (source_x < kRomHudX ||
                source_x >= kRomHudX + native::kFieldHudWidth) {
                continue;
            }
            const auto target_x = source_x - kRomHudX + kDisplayedHudX;
            if (target_x < 0 || target_x >= image.width) {
                continue;
            }
            shifted.pixels[static_cast<std::size_t>(y) * shifted.width +
                           static_cast<std::size_t>(target_x)] =
                image.pixels[static_cast<std::size_t>(y) * image.width + x];
        }
    }
    return shifted;
}

GenesisFieldHudLayers decode_genesis_field_hud_layers(
    const std::vector<std::uint8_t>& rom,
    const std::array<GenesisFieldEntityTemplate, 3>& records) {
    // 0x177CC is part of the field initializer and supplies the fixed counter
    // tiles before 0x39F2 stamps their name words.  0x3404 then appends each
    // runner's 16 portrait tiles and six item tiles at the exact VRAM offsets
    // consumed by 0x35D4/0x35F0.
    GenesisBootPatternData scene{
        std::vector<std::uint8_t>(0x10000U, 0),
        read_genesis_command_palette_set(rom),
    };
    // 0x334C's fixed counter upload is separate from the portrait/item
    // uploads below. Without 0x6A5BE at $DAC0 the name words in 0x6AB3E,
    // 0x6AB96, and 0x6AC46 resolve to empty tiles and every live value looks
    // like 00.
    write_genesis_f5c_vram(rom, 0x6A5BEU, 0xDAC0U, 0x4C0U,
                           scene.pattern_vram);
    write_genesis_1530_vram(rom, 0xD86E0U, 0x17E0U, scene.pattern_vram);
    write_genesis_1530_vram(rom, 0xD78AAU, 0x0580U, scene.pattern_vram);

    constexpr std::size_t kPortraitTileSource = 0x6ACFAU;
    constexpr std::size_t kItemTileSource = 0xCF97CU;
    constexpr std::size_t kItemTable = 0xD4540U;
    constexpr std::size_t kItemRecordStride = 0x30U;
    constexpr std::size_t kPortraitUploadBase = 0xD280U;
    constexpr std::size_t kPortraitUploadStride = 0x2C0U;
    constexpr std::size_t kPortraitTileCount = 16U;
    constexpr std::size_t kItemTileCount = 6U;

    const auto copy_rom_tile = [&rom, &scene](std::size_t source,
                                               std::size_t destination) {
        if (source > rom.size() || rom.size() - source < 32U ||
            destination > scene.pattern_vram.size() ||
            scene.pattern_vram.size() - destination < 32U) {
            throw std::out_of_range("Genesis field HUD tile upload is truncated");
        }
        std::copy_n(rom.begin() + static_cast<std::ptrdiff_t>(source), 32U,
                    scene.pattern_vram.begin() +
                        static_cast<std::ptrdiff_t>(destination));
    };

    for (std::size_t index = 0; index < records.size(); ++index) {
        const auto& record = records[index];
        if (record.record_offset == 0U || record.kind == 0U) {
            continue;
        }
        auto portrait_table = static_cast<std::size_t>(read_rom_u32(
            rom, record.record_offset + 0x50U,
            "Genesis field HUD portrait pointer is truncated"));
        // 0x3404 uses the live record's +0x54 word as a byte offset into the
        // portrait nametable.  This is why the field face is not simply the
        // command-screen portrait's top-left 3x2 crop.
        portrait_table += read_rom_u16(
            rom, record.record_offset + 0x54U,
            "Genesis field HUD portrait selector is truncated");
        const auto record_upload =
            kPortraitUploadBase + index * kPortraitUploadStride;
        for (std::size_t row = 0; row < 4U; ++row) {
            for (std::size_t column = 0; column < 4U; ++column) {
                // 0x3404 copies four words, then advances A2 by eight bytes;
                // the source portrait table is 8 tiles wide while the live
                // upload is the left 4x4 crop used by 0x35F0.
                const auto table_word = read_rom_u16(
                    rom, portrait_table + (row * 8U + column) * 2U,
                    "Genesis field HUD portrait table is truncated");
                const auto source_tile = static_cast<std::size_t>(
                    table_word & 0x07FFU);
                const auto upload_tile = row * 4U + column;
                copy_rom_tile(kPortraitTileSource + source_tile * 32U,
                              record_upload + upload_tile * 32U);
            }
        }

        const auto item_code = static_cast<std::size_t>(record.active_item_code);
        if (item_code != 0U && item_code <= native::kNativeInventoryItemCount) {
            const auto item_record =
                kItemTable + (item_code - 1U) * kItemRecordStride;
            const auto item_table = static_cast<std::size_t>(read_rom_u32(
                rom, item_record + 0x1AU,
                "Genesis field HUD item pointer is truncated"));
            const auto item_upload =
                record_upload + kPortraitTileCount * 32U;
            for (std::size_t tile = 0; tile < kItemTileCount; ++tile) {
                const auto table_word = read_rom_u16(
                    rom, item_table + tile * 2U,
                    "Genesis field HUD item table is truncated");
                const auto source_tile = static_cast<std::size_t>(
                    table_word & 0x07FFU);
                copy_rom_tile(kItemTileSource + source_tile * 32U,
                              item_upload + tile * 32U);
            }
        }
    }

    const auto palette_line_for_item = [&rom](const auto& record) {
        const auto palette_word = read_rom_u16(
            rom, record.record_offset + 0x2CU,
            "Genesis field HUD item palette is truncated");
        return palette_word == 0U
            ? std::size_t{1U}
            : static_cast<std::size_t>((palette_word & 0x6000U) >> 13U);
    };

    const auto make_tile_block = [&scene](std::size_t tile_base,
                                           std::size_t columns,
                                           std::size_t rows,
                                           std::size_t palette_line,
                                           std::size_t source_columns = 0U) {
        if (source_columns == 0U) {
            source_columns = columns;
        }
        RgbaImage image{
            static_cast<std::uint16_t>(columns * 8U),
            static_cast<std::uint16_t>(rows * 8U),
            std::vector<native::Rgba8>(columns * rows * 64U,
                                       native::Rgba8{0, 0, 0, 0})};
        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                blit_genesis_tile(
                    image, scene, tile_base + row * source_columns + column,
                    palette_line, false, false,
                    static_cast<std::int32_t>(column * 8U),
                    static_cast<std::int32_t>(row * 8U), true);
            }
        }
        return image;
    };

    const auto make_rom_tile_block = [&rom, &scene](std::size_t table_offset,
                                                     std::size_t palette_line) {
        constexpr std::size_t kItemTileSource = 0xCF97CU;
        RgbaImage image{
            24U, 16U,
            std::vector<native::Rgba8>(24U * 16U,
                                       native::Rgba8{0, 0, 0, 0})};
        for (std::size_t tile_index = 0; tile_index < 6U; ++tile_index) {
            const auto table_word = read_rom_u16(
                rom, table_offset + tile_index * 2U,
                "Genesis unarmed HUD item table is truncated");
            const auto source_tile = static_cast<std::size_t>(
                table_word & 0x07FFU);
            const auto source_offset = kItemTileSource + source_tile * 32U;
            if (source_offset > rom.size() || rom.size() - source_offset < 32U) {
                throw std::out_of_range(
                    "Genesis unarmed HUD item tile is truncated");
            }
            const auto tile = decode_genesis_tile(
                rom.data() + source_offset, 32U);
            const auto hflip = (table_word & 0x0800U) != 0U;
            const auto vflip = (table_word & 0x1000U) != 0U;
            const auto tile_x = (tile_index % 3U) * 8U;
            const auto tile_y = (tile_index / 3U) * 8U;
            for (std::size_t y = 0; y < 8U; ++y) {
                for (std::size_t x = 0; x < 8U; ++x) {
                    const auto source_x = hflip ? 7U - x : x;
                    const auto source_y = vflip ? 7U - y : y;
                    const auto color_index = tile.pixels[
                        source_y * 8U + source_x];
                    auto color = scene.palette[
                        palette_line * 16U + color_index];
                    color.alpha = color_index == 0U ? 0U : 255U;
                    image.pixels[(tile_y + y) * image.width + tile_x + x] =
                        color;
                }
            }
        }
        return image;
    };

    const auto crop = [](const RgbaImage& source, std::int32_t x,
                         std::int32_t y, std::uint16_t width,
                         std::uint16_t height) {
        RgbaImage result{
            width, height,
            std::vector<native::Rgba8>(static_cast<std::size_t>(width) * height,
                                       native::Rgba8{0, 0, 0, 0})};
        for (std::uint16_t row = 0; row < height; ++row) {
            for (std::uint16_t column = 0; column < width; ++column) {
                const auto source_x = x + static_cast<std::int32_t>(column);
                const auto source_y = y + static_cast<std::int32_t>(row);
                if (source_x < 0 || source_y < 0 ||
                    source_x >= static_cast<std::int32_t>(source.width) ||
                    source_y >= static_cast<std::int32_t>(source.height)) {
                    continue;
                }
                result.pixels[static_cast<std::size_t>(row) * width + column] =
                    source.pixels[static_cast<std::size_t>(source_y) *
                                      source.width +
                                  static_cast<std::size_t>(source_x)];
            }
        }
        return result;
    };

    GenesisFieldHudLayers result;
    for (std::size_t index = 0; index < records.size(); ++index) {
        const auto& record = records[index];
        if (record.record_offset == 0U || record.kind == 0U) {
            continue;
        }
        const auto portrait_base =
            (kPortraitUploadBase + index * kPortraitUploadStride) / 32U;
        // 0x35D4 writes the uploaded portrait as a 4x4 block at F1C6.  The
        // command screen's 8x10 portrait is a different destination and must
        // not be cropped into this field-HUD slot: doing that leaves the face
        // undersized and exposes the wrong source window.
        const auto portrait_palette_word = read_rom_u16(
            rom, record.record_offset + 0x2CU,
            "Genesis field HUD portrait palette is truncated");
        const auto portrait_palette_line = static_cast<std::size_t>(
            (portrait_palette_word & 0x6000U) >> 13U);
        result.faces[index] = make_tile_block(
            portrait_base, 4U, 4U, portrait_palette_line);

        const auto item_base = portrait_base + kPortraitTileCount;
        // 0x3404 uploads only six item tiles. 0x35D4's surrounding 4x4 VDP
        // writes leave the remaining source cells untouched; keeping only the
        // uploaded 3x2 item region avoids leaking the next runner's portrait
        // tiles into this slot in the host representation.
        result.item_blocks[index] = make_tile_block(
            item_base, 3U, 2U, palette_line_for_item(record));

        // 0x3404 selects the unarmed 3x2 tile list when +0x57 is zero. The
        // list is chosen by the source cyberware bits at +0x90; it is not a
        // blank placeholder and must remain a separate extracted resource.
        const auto unarmed_table = (record.flags_10 & 0x01U) == 0U
            ? 0x34F8U
            : (record.cyberware_flags & 0x0008U) != 0U
                ? 0xD1878U
                : (record.cyberware_flags & 0x0004U) != 0U
                    ? 0xD186CU : 0xD1860U;
        result.unarmed_item_blocks[index] = make_rom_tile_block(
            unarmed_table, palette_line_for_item(record));

        const auto vertical_progress = std::min<std::size_t>(
            static_cast<std::size_t>(record.physical_value), 10U);
        const auto vertical_source =
            0x6AB3EU + (record.flags_10 & 0x01U ? vertical_progress * 8U : 0U);
        auto vertical_canvas = make_transparent_title_layer();
        blit_genesis_name_table(rom, scene, vertical_canvas, vertical_source,
                                1U, 4U, 0xF1C4U + index * 0x400U,
                                0xF000U, 0x6000U, true);
        // 0x3674 writes this one-column physical bar at F1C4/F5C4/F9C4.
        // This canvas remains in the ROM plane's source coordinates. Keep
        // the crop at that source location even though the host overlay is
        // positioned at the right edge of the presentation.
        result.vertical_counters[index] = crop(
            vertical_canvas, 272,
            native::kFieldHudFaceY +
                static_cast<std::int32_t>(index * 64U),
            8U, 32U);

        const auto horizontal_progress = std::min<std::size_t>(
            static_cast<std::size_t>(record.mental_value), 10U);
        const auto horizontal_source =
            (index == 0U ? 0x6ABEEU : 0x6AB96U) + horizontal_progress * 8U;
        auto horizontal_canvas = make_transparent_title_layer();
        blit_genesis_name_table(rom, scene, horizontal_canvas,
                                horizontal_source, 4U, 1U,
                                0xF3C6U + index * 0x400U, 0xF000U,
                                0x6000U, true);
        result.horizontal_counters[index] = crop(
            horizontal_canvas, 280,
            native::kFieldHudHorizontalCounterY +
                static_cast<std::int32_t>(index * 64U),
            32U, 8U);
    }
    // 0x36FC indexes the two-tile ammunition table at 0x6AC46 and writes it
    // at F4C4/F8C4/ FCC4. Preserve the whole source table so loading, firing,
    // and switching weapons can select the correct live count without
    // inventing text or repainting the counter in a menu font.
    for (std::size_t ammo = 0; ammo < result.ammo_counters.size(); ++ammo) {
        auto ammo_canvas = make_transparent_title_layer();
        blit_genesis_name_table(
            rom, scene, ammo_canvas, 0x6AC46U + ammo * 4U, 2U, 1U,
            0xF4C4U, 0xF000U, 0x6000U, true);
        // F4C4 is the source name-table row below the portrait. This source
        // crop remains at its ROM y=72 even though the physical strip's
        // presentation coordinate is y=24.
        result.ammo_counters[ammo] = crop(
            ammo_canvas, 272,
            72, 16U, 8U);
    }
    return result;
}

RgbaImage software_screen_image(const native::SoftwareScreen& screen) {
    RgbaImage image{
        screen.width(), screen.height(),
        std::vector<native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

void write_bmp32(const std::string& path, const RgbaImage& image) {
    if (image.width == 0 || image.height == 0 ||
        image.pixels.size() != static_cast<std::size_t>(image.width) *
                                    image.height) {
        throw std::invalid_argument("invalid RGBA image dimensions");
    }
    const auto row_bytes = static_cast<std::uint32_t>(image.width) * 4U;
    const auto pixel_bytes = row_bytes * image.height;
    const auto file_offset = 14U + 40U;
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot open BMP output");
    }
    output.put('B');
    output.put('M');
    write_u32(output, file_offset + pixel_bytes);
    write_u16(output, 0);
    write_u16(output, 0);
    write_u32(output, file_offset);
    write_u32(output, 40);
    write_u32(output, image.width);
    write_u32(output, image.height);
    write_u16(output, 1);
    write_u16(output, 32);
    write_u32(output, 0);
    write_u32(output, pixel_bytes);
    write_u32(output, 2835);
    write_u32(output, 2835);
    write_u32(output, 0);
    write_u32(output, 0);

    for (std::size_t row = image.height; row-- > 0;) {
        for (std::size_t column = 0; column < image.width; ++column) {
            const auto pixel = image.pixels[row * image.width + column];
            output.put(static_cast<char>(pixel.blue));
            output.put(static_cast<char>(pixel.green));
            output.put(static_cast<char>(pixel.red));
            output.put(static_cast<char>(pixel.alpha));
        }
    }
    if (!output) {
        throw std::runtime_error("cannot write BMP output");
    }
}

RgbaImage read_bmp32(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open BMP input");
    }
    const auto read_u16 = [&input](const char* what) {
        const auto b0 = input.get();
        const auto b1 = input.get();
        if (b0 == std::char_traits<char>::eof() ||
            b1 == std::char_traits<char>::eof()) {
            throw std::out_of_range(what);
        }
        return static_cast<std::uint16_t>(
            static_cast<std::uint8_t>(b0) |
            (static_cast<std::uint16_t>(static_cast<std::uint8_t>(b1)) << 8U));
    };
    const auto read_u32 = [&input](const char* what) {
        std::uint32_t value = 0;
        for (unsigned shift = 0; shift < 32; shift += 8) {
            const auto byte = input.get();
            if (byte == std::char_traits<char>::eof()) {
                throw std::out_of_range(what);
            }
            value |= static_cast<std::uint32_t>(
                         static_cast<std::uint8_t>(byte))
                     << shift;
        }
        return value;
    };
    if (input.get() != 'B' || input.get() != 'M') {
        throw std::runtime_error("invalid BMP signature");
    }
    (void)read_u32("BMP header is truncated");
    (void)read_u16("BMP header is truncated");
    (void)read_u16("BMP header is truncated");
    const auto pixel_offset = read_u32("BMP header is truncated");
    const auto header_size = read_u32("BMP info header is truncated");
    if (header_size < 40U) {
        throw std::runtime_error("unsupported BMP info header");
    }
    const auto width = read_u32("BMP dimensions are truncated");
    const auto raw_height = read_u32("BMP dimensions are truncated");
    const auto planes = read_u16("BMP format is truncated");
    const auto bits = read_u16("BMP format is truncated");
    const auto compression = read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    if (width == 0U || raw_height == 0U || width > 8192U ||
        raw_height > 8192U || planes != 1U || bits != 32U ||
        compression != 0U) {
        throw std::runtime_error("unsupported BMP format");
    }
    if (header_size > 40U) {
        input.seekg(static_cast<std::streamoff>(header_size - 40U),
                    std::ios::cur);
    }
    if (!input) {
        throw std::out_of_range("BMP info header is truncated");
    }
    const auto height = static_cast<std::size_t>(raw_height);
    const auto row_bytes = static_cast<std::size_t>(width) * 4U;
    if (height > std::numeric_limits<std::size_t>::max() / row_bytes) {
        throw std::overflow_error("BMP dimensions overflow host size");
    }
    input.seekg(static_cast<std::streamoff>(pixel_offset), std::ios::beg);
    if (!input) {
        throw std::out_of_range("BMP pixel offset is outside the file");
    }
    RgbaImage image{static_cast<std::uint16_t>(width),
                    static_cast<std::uint16_t>(height),
                    std::vector<native::Rgba8>(
                        static_cast<std::size_t>(width) * height)};
    for (std::size_t source_row = height; source_row-- > 0;) {
        for (std::size_t column = 0; column < width; ++column) {
            const auto blue = input.get();
            const auto green = input.get();
            const auto red = input.get();
            const auto alpha = input.get();
            if (blue == std::char_traits<char>::eof() ||
                green == std::char_traits<char>::eof() ||
                red == std::char_traits<char>::eof() ||
                alpha == std::char_traits<char>::eof()) {
                throw std::out_of_range("BMP pixel data is truncated");
            }
            image.pixels[source_row * width + column] = {
                static_cast<std::uint8_t>(red),
                static_cast<std::uint8_t>(green),
                static_cast<std::uint8_t>(blue),
                static_cast<std::uint8_t>(alpha),
            };
        }
    }
    return image;
}

} // namespace shadowrun::graphics
