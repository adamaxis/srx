#include "shadowrun/rendering/graphics.hpp"
#include "shadowrun/core/rom_layout.hpp"
#include "shadowrun/rendering/asset_taxonomy.hpp"
#include "shadowrun/rendering/resource_manifest.hpp"
#include "field_mode_profile_import.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>
#include <zlib.h>

namespace {

std::uint64_t parse_number(const std::string& value) {
    std::size_t consumed = 0;
    const auto number = std::stoull(value, &consumed, 0);
    if (consumed != value.size()) {
        throw std::invalid_argument("invalid numeric argument: " + value);
    }
    return number;
}

std::uint16_t read_big_endian_word(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset) {
    if (offset > bytes.size() || bytes.size() - offset < 2U) {
        throw std::runtime_error("source word exceeds input image");
    }
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        static_cast<std::uint16_t>(bytes[offset + 1U]));
}

std::uint32_t read_big_endian_long(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset) {
    if (offset > bytes.size() || bytes.size() - offset < 4U) {
        throw std::runtime_error("source long exceeds input image");
    }
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3U]);
}

std::string read_multiline_rom_text(const std::vector<std::uint8_t>& rom,
                                    const std::size_t offset,
                                    const std::size_t length,
                                    const std::string_view label) {
    if (offset > rom.size() || length > rom.size() - offset) {
        throw std::runtime_error(std::string(label) + " exceeds the ROM image");
    }
    std::string text;
    text.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto byte = rom[offset + index];
        if (byte == 0xFFU) {
            break;
        }
        if (byte == 0x80U) {
            text.push_back('\n');
        } else if (byte >= 0x20U && byte <= 0x7EU) {
            text.push_back(static_cast<char>(byte));
        } else {
            text.push_back(' ');
        }
    }
    return text;
}

shadowrun::graphics::RgbaImage decode_field_service_cyberware_icon(
    const std::vector<std::uint8_t>& rom, const std::size_t category) {
    constexpr std::size_t kIconTableOffset = 0xD177CU;
    // Mode 7/result 1 uploads the eight cyberware illustrations after the
    // shared field-menu graphics. Their name-table words select tiles 0x44E-
    // 0x47B, which are populated by this distinct source block.
    constexpr std::size_t kCyberwarePatternOffset = 0xCF97CU;
    constexpr std::size_t kCyberwarePatternVramOffset = 0x89A0U;
    constexpr std::size_t kCyberwarePatternBytes = 0x5E0U;
    constexpr std::size_t kSetupPaletteOffset = 0xD29D6U;
    constexpr std::size_t kFieldPaletteTailOffset = 0x18AC56U;
    constexpr std::size_t kIconWidth = 24U;
    constexpr std::size_t kIconHeight = 16U;
    if (category >= 8U) {
        throw std::out_of_range("Genesis cyberware icon category is invalid");
    }

    std::vector<std::uint8_t> pattern_vram(0x10000U, 0U);
    if (kCyberwarePatternOffset > rom.size() ||
        rom.size() - kCyberwarePatternOffset < kCyberwarePatternBytes ||
        kCyberwarePatternVramOffset > pattern_vram.size() ||
        pattern_vram.size() - kCyberwarePatternVramOffset <
            kCyberwarePatternBytes) {
        throw std::out_of_range(
            "Genesis cyberware pattern upload is truncated");
    }
    std::copy_n(
        rom.begin() + static_cast<std::ptrdiff_t>(kCyberwarePatternOffset),
        kCyberwarePatternBytes,
        pattern_vram.begin() +
            static_cast<std::ptrdiff_t>(kCyberwarePatternVramOffset));

    auto palette = shadowrun::graphics::read_genesis_palette_set(
        rom, kSetupPaletteOffset);
    for (std::size_t index = 16U; index < palette.size(); ++index) {
        const auto color_offset = kFieldPaletteTailOffset + (index - 16U) * 2U;
        if (color_offset > rom.size() || rom.size() - color_offset < 2U) {
            throw std::out_of_range("Genesis field palette tail is truncated");
        }
        palette[index] = shadowrun::graphics::decode_genesis_cram_word(
            read_big_endian_word(rom, color_offset), index % 16U == 0U);
    }

    const auto icon_table = kIconTableOffset + category * 12U;
    if (icon_table > rom.size() || rom.size() - icon_table < 12U) {
        throw std::out_of_range("Genesis cyberware icon grid is truncated");
    }
    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kIconWidth),
        static_cast<std::uint16_t>(kIconHeight),
        std::vector<shadowrun::native::Rgba8>(
            kIconWidth * kIconHeight,
            shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t row = 0U; row < 2U; ++row) {
        for (std::size_t column = 0U; column < 3U; ++column) {
            const auto name_word = read_big_endian_word(
                rom, icon_table + (row * 3U + column) * 2U);
            const auto palette_line =
                static_cast<std::size_t>((name_word >> 13U) & 0x03U) | 1U;
            if ((name_word & 0x8000U) != 0U || palette_line != 1U) {
                throw std::runtime_error(
                    "Genesis cyberware icon has unsupported tile attributes");
            }
            const auto tile_index =
                static_cast<std::size_t>(name_word & 0x07FFU);
            if (tile_index * 32U + 32U > pattern_vram.size()) {
                throw std::out_of_range(
                    "Genesis cyberware icon references missing tile data");
            }
            const auto tile = shadowrun::graphics::decode_genesis_tile(
                pattern_vram.data() + tile_index * 32U, 32U);
            const bool hflip = (name_word & 0x0800U) != 0U;
            const bool vflip = (name_word & 0x1000U) != 0U;
            for (std::size_t y = 0U; y < 8U; ++y) {
                for (std::size_t x = 0U; x < 8U; ++x) {
                    const auto source_x = hflip ? 7U - x : x;
                    const auto source_y = vflip ? 7U - y : y;
                    const auto color_index =
                        tile.pixels[source_y * 8U + source_x];
                    if (color_index == 0U) {
                        continue;
                    }
                    auto color = palette[palette_line * 16U + color_index];
                    color.alpha = 255U;
                    image.pixels[(row * 8U + y) * kIconWidth +
                                 column * 8U + x] = color;
                }
            }
        }
    }
    return image;
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open ROM: " + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

std::vector<std::uint16_t> decode_hud_sprite_stream_for_asset(
    const std::vector<std::uint8_t>& rom,
    const std::size_t source_offset,
    const std::string_view label) {
    // Offline extraction of the 0x1728 name-word format. Runtime rendering
    // consumes the resulting full-screen bitmap; it never walks this stream.
    if (source_offset > rom.size() || rom.size() - source_offset < 3U) {
        throw std::runtime_error(std::string(label) + " is truncated");
    }
    const auto tile_base = read_big_endian_word(rom, source_offset);
    auto cursor = source_offset + 2U;
    std::vector<std::uint16_t> words;
    words.reserve(40U * 28U);

    const auto read_byte = [&]() {
        if (cursor >= rom.size()) {
            throw std::runtime_error(std::string(label) + " packet is truncated");
        }
        return rom[cursor++];
    };
    const auto append = [&words, tile_base](const std::uint16_t delta) {
        words.push_back(static_cast<std::uint16_t>(tile_base + delta));
    };

    for (;;) {
        const auto control = read_byte();
        if (control == 0U) {
            break;
        }
        const auto encoded_count = static_cast<unsigned>(control & 0x1FU);
        if (control >= 0xC1U) {
            // 0x1728's BGT loop emits N words, with zero encoding one word.
            const auto count = std::max(encoded_count, 1U);
            if ((control & 0x20U) == 0U) {
                for (unsigned index = 0U; index < count; ++index) {
                    append(read_byte());
                }
            } else {
                for (unsigned index = 0U; index < count; ++index) {
                    const auto delta = read_big_endian_word(rom, cursor);
                    cursor += 2U;
                    append(delta);
                }
            }
        } else {
            const auto step = control < 0x41U
                                  ? -1
                                  : (control < 0x81U ? 0 : 1);
            // Every step run enters the shared BGE loop at 0x1816 and emits
            // N+1 words. Only literal runs use the separate BGT loop.
            const auto count = encoded_count + 1U;
            std::uint16_t delta = 0U;
            if ((control & 0x20U) != 0U) {
                delta = read_big_endian_word(rom, cursor);
                cursor += 2U;
            } else {
                delta = read_byte();
            }
            for (unsigned index = 0U; index < count; ++index) {
                append(delta);
                delta = static_cast<std::uint16_t>(
                    static_cast<int>(delta) + step);
            }
        }
    }
    return words;
}

void write_rom_record(const std::vector<std::uint8_t>& rom,
                      const std::filesystem::path& root,
                      const std::string& relative_name,
                      const std::size_t source_offset,
                      const std::size_t byte_count) {
    if (source_offset > rom.size() || byte_count > rom.size() - source_offset) {
        throw std::runtime_error("ROM record exceeds input image: " +
                                 relative_name);
    }
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create ROM record: " + path.string());
    }
    output.write(reinterpret_cast<const char*>(rom.data() + source_offset),
                 static_cast<std::streamsize>(byte_count));
    if (!output) {
        throw std::runtime_error("cannot write ROM record: " + path.string());
    }
}

void write_host_record(const std::vector<std::uint8_t>& bytes,
                       const std::filesystem::path& root,
                       const std::string& relative_name) {
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create host record: " + path.string());
    }
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    if (!output) {
        throw std::runtime_error("cannot write host record: " + path.string());
    }
}

std::vector<std::pair<std::string, std::string>>
write_field_service_routes(
    const std::vector<std::uint8_t>& rom,
    const std::filesystem::path& root,
    const shadowrun::native::NativePortAssets& assets) {
    constexpr std::array<std::string_view, 14> kMagicSpellIds{{
        "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep",
        "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm",
        "rockskin", "heal_wounds", "confusion", "barrier"}};
    const auto path = root / "menu" / "field" / "service-routes.tsv";
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create field service route catalog: " +
                                 path.string());
    }
    output << "key\ttext\n";
    std::vector<std::pair<std::string, std::string>> script_text;
    const auto emit = [&output, &script_text](const std::string& key,
                                             const std::string& text) {
        if (text.find_first_of("\t\r\n") != std::string::npos) {
            throw std::runtime_error(
                "field service route text contains a TSV control character: " +
                key);
        }
        if (key.rfind("field-service.title-destination.", 0U) == 0U) {
            output << key << '\t' << text << '\n';
        } else {
            script_text.emplace_back(key, text);
        }
    };

    const auto service_titles =
        shadowrun::graphics::read_genesis_field_service_titles(rom);
    for (std::size_t index = 0; index < service_titles.size(); ++index) {
        emit("field-service.title-destination." + std::to_string(index),
             std::to_string(service_titles[index].destination_word));
    }
    const auto escape_field_text = [](std::string text) {
        std::size_t position = 0U;
        while ((position = text.find('\n', position)) !=
               std::string::npos) {
            text.replace(position, 1U, "\\n");
            position += 2U;
        }
        return text;
    };
    const auto cyberdeck_unavailable_text =
        shadowrun::graphics::read_genesis_cyberdeck_unavailable_text(rom);
    constexpr std::array<std::string_view, 3> kCyberdeckTextIds{{
        "no_deck_first", "no_deck_second", "look_for_deck"}};
    for (std::size_t index = 0;
         index < cyberdeck_unavailable_text.size(); ++index) {
        emit("pocket_secretary.cyberdeck.option." +
                 std::string(kCyberdeckTextIds[index]),
             cyberdeck_unavailable_text[index]);
    }
    const auto group_item_names =
        shadowrun::graphics::read_genesis_group_item_catalog(rom);
    constexpr std::array<std::string_view, 15> kGroupItemTextIds{{
        "prototype_cyberheart", "indian_lands_visa", "feathered_scale",
        "hell_hound_pelt", "gargoyle_horn", "gift_lady_gillian",
        "gift_red_buffalo_woman", "landmark_map", "map_overlay",
        "current_run_package", "security_pass", "weapon_permit",
        "eye_fiver_ear_cuff", "halloweener_patch", "ork_armband"}};
    for (std::size_t index = 0; index < group_item_names.size(); ++index) {
        emit("pocket_secretary.group_item.option." +
                 std::string(kGroupItemTextIds[index]),
             group_item_names[index]);
    }
    const auto group_items_empty_text =
        shadowrun::graphics::read_genesis_group_items_empty_text(rom);
    constexpr std::array<std::string_view, 2> kGroupItemsEmptyTextIds{{
        "first_line", "second_line"}};
    for (std::size_t index = 0; index < group_items_empty_text.size(); ++index) {
        emit("pocket_secretary.group_items.empty.option." +
                 std::string(kGroupItemsEmptyTextIds[index]),
             group_items_empty_text[index]);
    }
    const auto dismiss_runner_labels =
        shadowrun::graphics::read_genesis_dismiss_runner_labels(rom);
    constexpr std::array<std::string_view, 3> kDismissRunnerTextIds{{
        "dismiss", "who_question", "abort"}};
    for (std::size_t index = 0; index < dismiss_runner_labels.size(); ++index) {
        emit("pocket_secretary.dismiss.option." +
                 std::string(kDismissRunnerTextIds[index]),
             dismiss_runner_labels[index]);
    }
    emit("pocket_secretary.dismiss.text.option.separator",
         shadowrun::graphics::read_genesis_dismiss_runner_separator(rom));
    const auto notebook_tips_text =
        shadowrun::graphics::read_genesis_notebook_tips_text(rom);
    emit("pocket_secretary.notebook.tips.option.title",
         notebook_tips_text.title);
    emit("pocket_secretary.notebook.tips.option.separator",
         notebook_tips_text.separator);
    emit("pocket_secretary.notebook.tips.option.page_count",
         notebook_tips_text.page_count_label);
    emit("pocket_secretary.notebook.tips.option.page_number_clear",
         notebook_tips_text.page_number_clear);
    constexpr std::array<std::string_view, 6> kNotebookLabelKeys{{
        "pocket_secretary.notebook.options.title",
        "pocket_secretary.notebook.option.current_run",
        "pocket_secretary.notebook.option.tips_and_clues",
        "pocket_secretary.notebook.option.contacts",
        "pocket_secretary.notebook.option.mr_johnsons",
        "pocket_secretary.notebook.option.shadowrunners",
    }};
    for (std::size_t index = 0; index < assets.notebook_labels.size(); ++index) {
        emit(std::string(kNotebookLabelKeys[index]),
             assets.notebook_labels[index]);
    }
    constexpr std::array<std::string_view, 3> kNotebookNoRunTextIds{{
        "first_line", "second_line", "last_line"}};
    for (std::size_t index = 0; index < assets.notebook_no_run_lines.size();
         ++index) {
        emit("pocket_secretary.notebook.no_run.option." +
                 std::string(kNotebookNoRunTextIds[index]),
             assets.notebook_no_run_lines[index]);
    }
    constexpr std::array<std::string_view, 5> kMrJohnsonTextIds{{
        "gunderson", "julius_strouther", "mortimer_reed",
        "vigore_and_jarl", "caleb_brightmore"}};
    for (std::size_t index = 0;
         index < assets.notebook_mr_johnson_names.size(); ++index) {
        emit("pocket_secretary.notebook.mr_johnson.name.option." +
                 std::string(kMrJohnsonTextIds[index]),
             assets.notebook_mr_johnson_names[index]);
    }
    const auto mr_johnson_descriptions =
        shadowrun::graphics::read_genesis_notebook_mr_johnson_descriptions(
            rom);
    for (std::size_t index = 0; index < mr_johnson_descriptions.size();
         ++index) {
        emit("pocket_secretary.notebook.mr_johnson.detail.option." +
                 std::string(kMrJohnsonTextIds[index]),
             escape_field_text(mr_johnson_descriptions[index]));
    }
    const auto command_names =
        shadowrun::graphics::read_genesis_command_overview_names(rom);
    // The command-screen presenter selects these padded ROM rows by the
    // runner record's species/class bytes. Stable names preserve their source
    // order without exposing numeric text-table slots to the runtime.
    constexpr std::array<std::string_view, 5> kCommandOverviewRaceIds{{
        "human", "elf", "dwarf", "orc", "troll"}};
    for (std::size_t index = 0; index < command_names.races.size(); ++index) {
        emit("command_overview.race.option." +
                 std::string(kCommandOverviewRaceIds[index]),
             command_names.races[index]);
    }
    constexpr std::array<std::string_view, 4> kCommandOverviewClassIds{{
        "mage", "shaman", "samurai", "decker"}};
    for (std::size_t index = 0; index < command_names.classes.size(); ++index) {
        emit("command_overview.class.option." +
                 std::string(kCommandOverviewClassIds[index]),
             command_names.classes[index]);
    }
    constexpr std::array<std::string_view, 25> kAttributesSkillsTextIds{{
        "body", "quickness", "strength", "charisma", "intelligence",
        "willpower", "essence", "magic", "sorcery", "firearms", "pistols",
        "smgs", "shotguns", "melee_combat", "throwing", "computer",
        "biotech", "electronics", "reputation", "negotiation",
        "upgrade_header", "maximum_level", "mage_only",
        "insufficient_karma", "upgrade_prompt"}};
    const auto attribute_skill_labels =
        shadowrun::graphics::read_genesis_attribute_skill_labels(rom);
    for (std::size_t index = 0; index < attribute_skill_labels.size();
         ++index) {
        emit("attributes_skills.text.option." +
                 std::string(kAttributesSkillsTextIds[index]),
             attribute_skill_labels[index]);
    }
    constexpr std::array<std::string_view, 20>
        kAttributeSkillDescriptionIds{{
            "body", "quickness", "strength", "charisma", "intelligence",
            "willpower", "essence", "magic", "sorcery", "firearms",
            "pistols", "smgs", "shotguns", "melee_combat", "throwing",
            "computer", "biotech", "electronics", "reputation",
            "negotiation"}};
    // ROM 0xC1B70 is the 20-entry bottom-panel description pointer table.
    // Keep these strings in the offline importer so each menu runtime reads
    // stable host text without walking ROM pointers.
    for (std::size_t index = 0;
         index < kAttributeSkillDescriptionIds.size(); ++index) {
        const auto source = static_cast<std::size_t>(
            read_big_endian_long(rom, 0xC1B70U + index * 4U));
        const auto segments = shadowrun::graphics::
            read_genesis_dialogue_segments(rom, source);
        std::string description;
        for (std::size_t segment = 0; segment < segments.size(); ++segment) {
            if (segment != 0U) {
                description.push_back('\n');
            }
            description += segments[segment];
        }
        emit("attributes_skills.description." +
                 std::string(kAttributeSkillDescriptionIds[index]),
             escape_field_text(std::move(description)));
    }
    auto attributes_skills_text =
        shadowrun::graphics::read_genesis_attributes_skills_text(rom);
    // The source strings use 0x80 row breaks. The older fixed-width menu-text
    // helper stops at the first break, while ROM 0xD3D0/0xD408 sends the full
    // strings to the title writer. Keep both rows in the host text catalog.
    attributes_skills_text.mage_only_message =
        escape_field_text(read_multiline_rom_text(
            rom, 0xC2122U, 0x2CU,
            "Genesis Attributes/Skills mage-only text"));
    attributes_skills_text.insufficient_karma_message =
        escape_field_text(read_multiline_rom_text(
            rom, 0xC214EU, 0x2FU,
            "Genesis Attributes/Skills Karma text"));
    const std::array<std::string, 5> attributes_upgrade_text{{
        attributes_skills_text.upgrade_header,
        attributes_skills_text.maximum_level_message,
        attributes_skills_text.mage_only_message,
        attributes_skills_text.insufficient_karma_message,
        attributes_skills_text.upgrade_prompt}};
    for (std::size_t index = 0; index < attributes_upgrade_text.size();
         ++index) {
        emit("attributes_skills.text.option." +
                 std::string(kAttributesSkillsTextIds[
                     attribute_skill_labels.size() + index]),
             attributes_upgrade_text[index]);
    }
    const auto cyberware_text =
        shadowrun::graphics::read_genesis_cyberware_text(rom);
    const auto title_break = cyberware_text.title.find('\n');
    if (title_break == std::string::npos ||
        cyberware_text.title.find('\n', title_break + 1U) !=
            std::string::npos) {
        throw std::runtime_error(
            "field menu Cyberware title must contain exactly two source lines");
    }
    constexpr std::array<std::string_view, 18> kCyberwareTextIds{{
        "title", "separator", "empty", "datajack", "cyber_eyes",
        "hand_razors", "spurs", "smartlink", "muscle_replace_1",
        "muscle_replace_2", "muscle_replace_3", "muscle_replace_4",
        "dermal_plating_1", "dermal_plating_2", "dermal_plating_3",
        "wired_reflexes_1", "wired_reflexes_2", "wired_reflexes_3"}};
    std::array<std::string, 18> cyberware_ui_text{{
        cyberware_text.title.substr(0, title_break),
        cyberware_text.title.substr(title_break + 1U),
        cyberware_text.empty_message,
    }};
    for (std::size_t index = 0; index < cyberware_text.entries.size();
         ++index) {
        cyberware_ui_text[index + 3U] = cyberware_text.entries[index];
    }
    for (std::size_t index = 0; index < kCyberwareTextIds.size(); ++index) {
        emit("cyberware.text.option." +
                 std::string(kCyberwareTextIds[index]),
             cyberware_ui_text[index]);
    }
    // ROM 0xFBA6 passes C89C0 to the encoded-title writer as the fallback
    // heading. Keep the source's leading space for its original cell offset.
    const auto magic_text =
        shadowrun::graphics::read_genesis_magic_menu_text(rom);
    if (magic_text.spell_names.size() != kMagicSpellIds.size() ||
        magic_text.spell_descriptions.size() != kMagicSpellIds.size()) {
        throw std::runtime_error(
            "Magic spell text count no longer matches its stable IDs");
    }
    const auto escape_newlines = [](std::string value) {
        std::size_t position = 0U;
        while ((position = value.find('\n', position)) != std::string::npos) {
            value.replace(position, 1U, "\\n");
            position += 2U;
        }
        return value;
    };
    constexpr std::array<std::string_view, 7> kMagicPanelTextIds{{
        "defense_title", "force_posture_heading", "success_label",
        "force_label", "availability_label", "panel_metric_labels",
        "panel_force_posture_labels"}};
    const std::array<std::string, 7> magic_panel_text{{
        " Magical Defense", magic_text.set_force_and_posture,
        magic_text.success_label, magic_text.force_label,
        magic_text.availability_label,
        escape_newlines(magic_text.panel_metric_labels),
        escape_newlines(magic_text.force_posture_labels)}};
    for (std::size_t index = 0; index < kMagicPanelTextIds.size(); ++index) {
        emit("magic.panel.text.option." +
                 std::string(kMagicPanelTextIds[index]),
             magic_panel_text[index]);
    }
    for (std::size_t index = 0; index < magic_text.spell_names.size();
         ++index) {
        const auto spell_id = std::string(kMagicSpellIds[index]);
        emit("magic.spells.option." + spell_id,
             magic_text.spell_names[index]);
        emit("magic.spells.description." + spell_id,
             escape_newlines(magic_text.spell_descriptions[index]));
    }
    for (std::size_t index = 0; index < assets.intro.archetypes.size();
         ++index) {
        emit("archetype." + std::to_string(index),
             assets.intro.archetypes[index]);
    }
    for (std::size_t index = 0; index < assets.field_menu_runner_names.size();
         ++index) {
        emit("runner." + std::to_string(index),
             assets.field_menu_runner_names[index]);
    }
    constexpr std::array<std::string_view, 5> kCommandOverviewPostureIds{{
        "full_defense", "medium_defense", "neutral", "medium_offense",
        "full_offense"}};
    for (std::size_t index = 0; index < assets.field_menu_status_names.size();
         ++index) {
        emit("command_overview.posture.option." +
                 std::string(kCommandOverviewPostureIds[index]),
             assets.field_menu_status_names[index]);
    }
    const auto field_labels =
        shadowrun::graphics::read_genesis_field_menu_labels(rom);
    constexpr std::array<std::string_view, 11> kFieldMetricTextIds{{
        "physical", "mental", "attack", "defense", "damage", "drain",
        "none", "no_armor", "weapons", "guns", "power"}};
    const std::array<std::string, 11> field_metric_labels{{
        field_labels.physical, field_labels.mental, field_labels.attack,
        field_labels.defense, field_labels.damage, field_labels.drain,
        field_labels.none, field_labels.no_armor, field_labels.weapons,
        field_labels.guns, field_labels.power}};
    for (std::size_t index = 0; index < kFieldMetricTextIds.size(); ++index) {
        emit("ui.field_metric.option." +
                 std::string(kFieldMetricTextIds[index]),
             field_metric_labels[index]);
    }
    const auto action_text =
        shadowrun::graphics::read_genesis_field_menu_action_text(rom);
    constexpr std::array<std::string_view, 14> kItemUiTextIds{{
        "equipped_prefix", "equipped_suffix", "discard_prompt",
        "cannot_discard", "cannot_trade", "trade_recipient",
        "trade_full_inventory", "automatic_use", "first_aid_prompt",
        "first_aid_label", "first_aid_cancel", "first_aid_not_injured",
        "first_aid_slap_patch_used", "first_aid_medkit_used"}};
    const std::array<std::string, 14> item_ui_text{{
        action_text.item_equipped_prefix,
        action_text.item_equipped_suffix,
        action_text.discard_prompt,
        action_text.cannot_discard,
        action_text.cannot_trade,
        action_text.trade_recipient_label,
        escape_newlines(action_text.trade_full_inventory_message),
        escape_newlines(action_text.automatic_use_message),
        escape_newlines(action_text.first_aid_prompt),
        action_text.first_aid_label,
        action_text.first_aid_cancel,
        escape_newlines(action_text.first_aid_not_injured),
        action_text.first_aid_slap_patch_used,
        action_text.first_aid_medkit_used}};
    for (std::size_t index = 0; index < kItemUiTextIds.size(); ++index) {
        emit("items.text.option." + std::string(kItemUiTextIds[index]),
             item_ui_text[index]);
    }
    emit("ui.option.yes", action_text.choice_yes);
    emit("ui.option.no", action_text.choice_no);
    const auto clips_text =
        shadowrun::graphics::read_genesis_field_menu_clips_text(rom);
    constexpr std::array<std::string_view, 4> kClipsCommandTextKeys{{
        "clips.option.reload", "clips.option.trade",
        "clips.option.distribute", "clips.option.cancel"}};
    constexpr std::array<std::string_view, 4> kClipsTradeCommandTextKeys{{
        "clips.trade.option.increase_clips",
        "clips.trade.option.decrease_clips",
        "clips.trade.option.trade_clips",
        "clips.trade.option.change_recipient"}};
    for (std::size_t index = 0; index < clips_text.commands.size(); ++index) {
        emit(std::string(kClipsCommandTextKeys[index]),
             clips_text.commands[index]);
        emit(std::string(kClipsTradeCommandTextKeys[index]),
             clips_text.trade_commands[index]);
    }
    const auto emit_clips_text = [&emit](const std::string_view id,
                                         const std::string& text) {
        emit("clips.text.option." + std::string(id), text);
    };
    emit_clips_text("header_title", clips_text.title);
    emit_clips_text("header_separator", clips_text.separator);
    emit_clips_text("counter_label", clips_text.counter_label);
    constexpr std::array<std::string_view, 3> kTradePromptIds{{
        "trade_prompt_action", "trade_prompt_clips_to",
        "trade_prompt_recipient"}};
    for (std::size_t index = 0; index < clips_text.trade_prompt.size();
         ++index) {
        emit_clips_text(kTradePromptIds[index],
                        clips_text.trade_prompt[index]);
    }
    constexpr std::array<std::string_view, 2> kExchangeCompleteIds{{
        "exchange_complete_first", "exchange_complete_second"}};
    for (std::size_t index = 0; index < clips_text.exchange_complete.size();
         ++index) {
        emit_clips_text(kExchangeCompleteIds[index],
                        clips_text.exchange_complete[index]);
    }
    constexpr std::array<std::string_view, 3> kReloadPromptIds{{
        "reload_prompt_ammo_remaining", "reload_prompt_ammo_suffix",
        "reload_prompt_question"}};
    for (std::size_t index = 0; index < clips_text.reload_prompt.size();
         ++index) {
        emit_clips_text(kReloadPromptIds[index],
                        clips_text.reload_prompt[index]);
    }
    constexpr std::array<std::string_view, 2> kReloadSuccessIds{{
        "reload_success_first", "reload_success_second"}};
    for (std::size_t index = 0; index < clips_text.reload_success.size();
         ++index) {
        emit_clips_text(kReloadSuccessIds[index],
                        clips_text.reload_success[index]);
    }
    constexpr std::array<std::string_view, 2> kOutOfAmmoIds{{
        "out_of_ammo_name_suffix", "out_of_ammo_clips"}};
    for (std::size_t index = 0; index < clips_text.out_of_ammo.size();
         ++index) {
        emit_clips_text(kOutOfAmmoIds[index], clips_text.out_of_ammo[index]);
    }
    constexpr std::array<std::string_view, 3> kItemWithoutClipsIds{{
        "item_without_clips_first", "item_without_clips_second",
        "item_without_clips_third"}};
    for (std::size_t index = 0;
         index < clips_text.item_does_not_use_clips.size(); ++index) {
        emit_clips_text(kItemWithoutClipsIds[index],
                        clips_text.item_does_not_use_clips[index]);
    }
    constexpr std::array<std::string_view, 3> kNoPartyIds{{
        "no_party_first", "no_party_second", "no_party_third"}};
    for (std::size_t index = 0; index < clips_text.no_party_members.size();
         ++index) {
        emit_clips_text(kNoPartyIds[index], clips_text.no_party_members[index]);
    }
    // ROM 0x11C9C prints the shared Function Aborted message from 0xD1ED8.
    // 0x80 is the source line break and 0xFF ends the encoded title string.
    emit_clips_text(
        "function_aborted",
        escape_field_text(read_multiline_rom_text(
            rom, 0xD1ED8U, 18U, "Genesis field-menu Function Aborted text")));
    constexpr std::array<std::string_view, 27> kItemDisplayTextIds{{
        "power_meager", "power_minor", "power_very_low", "power_low",
        "power_very_light", "power_light", "power_medium",
        "power_medium_high", "power_high", "power_brutal", "power_severe",
        "power_massive", "power_incredible", "armor_vest_top",
        "armor_vest_bottom", "armor_clothing_top", "armor_clothing_bottom",
        "armor_jacket_top", "armor_jacket_bottom", "armor_plates_top",
        "armor_plates_bottom", "lined_duster_top", "lined_duster_bottom",
        "light_combat_armor_top", "light_combat_armor_bottom",
        "heavy_combat_armor_top", "heavy_combat_armor_bottom"}};
    std::size_t item_display_text_index = 0U;
    for (const auto& label : assets.field_menu_power_labels) {
        emit("items.display.option." +
                 std::string(kItemDisplayTextIds[item_display_text_index++]),
             label);
    }
    for (std::size_t armor = 0; armor < assets.field_menu_armor_labels.size();
         ++armor) {
        for (const auto& label : assets.field_menu_armor_labels[armor]) {
            emit("items.display.option." +
                     std::string(
                         kItemDisplayTextIds[item_display_text_index++]),
                 label);
        }
    }
    if (!output) {
        throw std::runtime_error("cannot write field service route catalog: " +
                                 path.string());
    }
    return script_text;
}

std::vector<std::pair<std::string, std::string>>
read_notebook_tips_script_text(
    const shadowrun::native::NativePortAssets& assets) {
    std::vector<std::pair<std::string, std::string>> script_text;
    script_text.reserve(assets.notebook_tip_text.size());
    for (std::size_t index = 0; index < assets.notebook_tip_text.size();
         ++index) {
        auto text = assets.notebook_tip_text[index];
        if (text.find_first_of("\t\r") != std::string::npos) {
            throw std::runtime_error(
                "Notebook tip contains an unsupported TSV control character: " +
                std::to_string(index));
        }
        std::size_t position = 0U;
        while ((position = text.find('\n', position)) != std::string::npos) {
            text.replace(position, 1U, "\\n");
            position += 2U;
        }
        script_text.emplace_back(
            "pocket_secretary.notebook.tips.option.tip_" +
                std::to_string(index + 1U),
                                 std::move(text));
    }
    return script_text;
}

void remove_obsolete_menu_text_assets(const std::filesystem::path& root) {
    // These generated TSVs were replaced by keyed Lua text. Remove them when
    // refreshing an existing asset directory so stale copies cannot linger.
    std::filesystem::remove(root / "menu" / "field" / "text.tsv");
    std::filesystem::remove(root / "menu" / "pocket-secretary" /
                            "notebook-tips.tsv");
}

class DialogueTextJsonReader final {
public:
    explicit DialogueTextJsonReader(const std::string_view source)
        : source_(source) {}

    std::vector<std::pair<std::string, std::string>> read() {
        skip_whitespace();
        expect('{');
        bool schema_found = false;
        bool texts_found = false;
        std::vector<std::pair<std::string, std::string>> texts;
        skip_whitespace();
        while (peek() != '}') {
            const auto key = read_string();
            skip_whitespace();
            expect(':');
            skip_whitespace();
            if (key == "schemaVersion") {
                if (schema_found || read_integer() != 1U) {
                    fail("unsupported or duplicate schemaVersion");
                }
                schema_found = true;
            } else if (key == "texts") {
                if (texts_found) {
                    fail("duplicate texts object");
                }
                texts = read_texts();
                texts_found = true;
            } else {
                fail("unexpected top-level property");
            }
            skip_whitespace();
            if (peek() == '}') {
                break;
            }
            expect(',');
            skip_whitespace();
        }
        expect('}');
        skip_whitespace();
        if (position_ != source_.size()) {
            fail("trailing data");
        }
        if (!schema_found || !texts_found) {
            fail("missing schemaVersion or texts property");
        }
        return texts;
    }

private:
    char peek() const {
        return position_ < source_.size() ? source_[position_] : '\0';
    }

    [[noreturn]] void fail(const std::string_view message) const {
        throw std::runtime_error(
            "invalid dialogue JSON at byte " +
            std::to_string(position_) + ": " + std::string(message));
    }

    void skip_whitespace() {
        while (position_ < source_.size()) {
            const auto character = source_[position_];
            if (character != ' ' && character != '\t' && character != '\r' &&
                character != '\n') {
                break;
            }
            ++position_;
        }
    }

    void expect(const char expected) {
        if (peek() != expected) {
            fail("unexpected character");
        }
        ++position_;
    }

    unsigned read_hex_quad() {
        if (position_ + 4U > source_.size()) {
            fail("incomplete Unicode escape");
        }
        unsigned value = 0U;
        for (std::size_t index = 0; index < 4U; ++index) {
            const auto character = source_[position_++];
            unsigned digit = 0U;
            if (character >= '0' && character <= '9') {
                digit = static_cast<unsigned>(character - '0');
            } else if (character >= 'a' && character <= 'f') {
                digit = static_cast<unsigned>(character - 'a') + 10U;
            } else if (character >= 'A' && character <= 'F') {
                digit = static_cast<unsigned>(character - 'A') + 10U;
            } else {
                fail("invalid Unicode escape");
            }
            value = (value << 4U) | digit;
        }
        return value;
    }

    static void append_utf8(std::string& output, const unsigned codepoint) {
        if (codepoint <= 0x7FU) {
            output.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7FFU) {
            output.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
            output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        } else if (codepoint <= 0xFFFFU) {
            output.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
            output.push_back(static_cast<char>(
                0x80U | ((codepoint >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        } else {
            output.push_back(static_cast<char>(0xF0U | (codepoint >> 18U)));
            output.push_back(static_cast<char>(
                0x80U | ((codepoint >> 12U) & 0x3FU)));
            output.push_back(static_cast<char>(
                0x80U | ((codepoint >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        }
    }

    std::string read_string() {
        expect('"');
        std::string result;
        while (position_ < source_.size()) {
            const auto character = source_[position_++];
            if (character == '"') {
                return result;
            }
            if (static_cast<unsigned char>(character) < 0x20U) {
                fail("unescaped control byte in string");
            }
            if (character != '\\') {
                result.push_back(character);
                continue;
            }
            if (position_ >= source_.size()) {
                fail("incomplete string escape");
            }
            const auto escaped = source_[position_++];
            switch (escaped) {
            case '"': result.push_back('"'); break;
            case '\\': result.push_back('\\'); break;
            case '/': result.push_back('/'); break;
            case 'b': result.push_back('\b'); break;
            case 'f': result.push_back('\f'); break;
            case 'n': result.push_back('\n'); break;
            case 'r': result.push_back('\r'); break;
            case 't': result.push_back('\t'); break;
            case 'u': {
                auto codepoint = read_hex_quad();
                if (codepoint >= 0xD800U && codepoint <= 0xDBFFU) {
                    if (position_ + 2U > source_.size() ||
                        source_[position_] != '\\' ||
                        source_[position_ + 1U] != 'u') {
                        fail("high surrogate has no low surrogate");
                    }
                    position_ += 2U;
                    const auto low = read_hex_quad();
                    if (low < 0xDC00U || low > 0xDFFFU) {
                        fail("invalid low surrogate");
                    }
                    codepoint = 0x10000U +
                                ((codepoint - 0xD800U) << 10U) +
                                (low - 0xDC00U);
                } else if (codepoint >= 0xDC00U && codepoint <= 0xDFFFU) {
                    fail("unexpected low surrogate");
                }
                append_utf8(result, codepoint);
                break;
            }
            default: fail("invalid string escape");
            }
        }
        fail("unterminated string");
    }

    unsigned read_integer() {
        const auto start = position_;
        while (peek() >= '0' && peek() <= '9') {
            ++position_;
        }
        if (position_ == start ||
            (position_ - start > 1U && source_[start] == '0')) {
            fail("invalid schema version");
        }
        unsigned value = 0U;
        for (auto index = start; index < position_; ++index) {
            value = value * 10U +
                    static_cast<unsigned>(source_[index] - '0');
        }
        return value;
    }

    std::vector<std::pair<std::string, std::string>> read_texts() {
        std::vector<std::pair<std::string, std::string>> result;
        std::unordered_set<std::string> keys;
        expect('{');
        skip_whitespace();
        if (peek() == '}') {
            ++position_;
            return result;
        }
        while (true) {
            auto key = read_string();
            skip_whitespace();
            expect(':');
            skip_whitespace();
            auto value = read_string();
            if (!keys.insert(key).second) {
                fail("duplicate text key");
            }
            result.emplace_back(std::move(key), std::move(value));
            if (result.size() > 4096U) {
                fail("text entry limit exceeded");
            }
            skip_whitespace();
            if (peek() == '}') {
                ++position_;
                return result;
            }
            expect(',');
            skip_whitespace();
        }
    }

    std::string_view source_;
    std::size_t position_ = 0U;
};

std::vector<std::pair<std::string, std::string>> read_dialogue_text_json(
    const std::filesystem::path& path) {
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        if (error) {
            throw std::runtime_error(
                "cannot inspect existing dialogue JSON: " + error.message());
        }
        return {};
    }
    if (!std::filesystem::is_regular_file(path, error) || error) {
        throw std::runtime_error("existing dialogue JSON is not a file: " +
                                 path.string());
    }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > 8U * 1024U * 1024U) {
        throw std::runtime_error(
            error ? "cannot read existing dialogue JSON size: " +
                        error.message()
                  : "existing dialogue JSON exceeds the byte limit");
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open existing dialogue JSON: " +
                                 path.string());
    }
    std::string source(static_cast<std::size_t>(size), '\0');
    if (!source.empty()) {
        input.read(source.data(), static_cast<std::streamsize>(source.size()));
    }
    if (!input || input.gcount() != static_cast<std::streamsize>(source.size())) {
        throw std::runtime_error("cannot read existing dialogue JSON: " +
                                 path.string());
    }
    return DialogueTextJsonReader(source).read();
}

void write_json_string(std::ostream& output, const std::string_view text) {
    constexpr char hex[] = "0123456789abcdef";
    output << '"';
    for (const auto character : text) {
        const auto byte = static_cast<unsigned char>(character);
        switch (byte) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (byte < 0x20U) {
                output << "\\u00" << hex[byte >> 4U] << hex[byte & 0x0FU];
            } else {
                output << character;
            }
            break;
        }
    }
    output << '"';
}

void write_dialogue_text_catalog(const std::vector<std::uint8_t>& rom,
                                 const std::filesystem::path& root,
                                 const shadowrun::native::NativePortAssets& assets,
                                 const std::vector<std::pair<std::string,
                                                             std::string>>&
                                     field_text) {
    const auto path = root / "dialogue" / "script-text.json";
    std::filesystem::create_directories(path.parent_path());
    std::map<std::string, std::string> generated_texts;
    const auto write_text_entry = [&generated_texts](
                                     const std::string_view key,
                                     const std::string_view text) {
        generated_texts[std::string(key)] = std::string(text);
    };

    const std::pair<const char*, std::size_t> records[]{
        // Stoker prompt pointers consumed by the source event/profile path.
        {"stoker.motel.arrival.title", 0x1167EBU},
        {"stoker.motel.arrival.more", 0x0FE5BFU},
        {"stoker.motel.manager.title", 0x116962U},
        {"stoker.motel.option.crash", 0x103A17U},
        {"stoker.motel.option.leave", 0x104FEBU},
        {"stoker.motel.street-ambush.title", 0x1168FCU},
        {"stoker.motel.room.title", 0x1169B8U},
        {"stoker.motel.option.pay", 0x103A7AU},
        {"stoker.motel.package-handoff.title", 0x1168A3U},
        {"stoker.motel.package-question.title", 0x112765U},
        {"stoker.motel.option.package", 0x103A55U},
        {"stoker.motel.option.debt-payment", 0x103B93U},
        {"stoker.motel.option.jump-house", 0x103BBFU},
        {"stoker.motel.leave.title", 0x0F4134U},
        {"stoker.motel.debt.title", 0x1169E9U},
        {"stoker.motel.room.paid.title", 0x0F4241U},
        {"stoker.motel.room.insufficient.title", 0x0F42C1U},
        {"stoker.motel.jump-house.title", 0x0F41A1U},
        // Scene-1/event-1 Tarislar City Inn title service and generic prompt.
        {"tarislar.city_inn.entry", 0x117CE2U},
        {"tarislar.city_inn.offer", 0x117DB6U},
        // Scene-3/event-1 Wylie's Gala Inn field-exit service.
        {"wylie.gala_inn.entry", 0x114CE1U},
        {"wylie.gala_inn.room", 0x114E66U},
        {"wylie.gala_inn.free_stay", 0x114EF5U},
        {"wylie.gala_inn.package_after", 0x113A22U},
        {"wylie.gala_inn.package_before", 0x114DC1U},
        {"wylie.gala_inn.option.crash", 0x103A17U},
        {"wylie.gala_inn.option.leave", 0x104FEBU},
        {"wylie.gala_inn.option.pay", 0x103A7AU},
        {"tarislar.city_inn.room", 0x117E58U},
        {"tarislar.city_inn.shale.0", 0x117EC5U},
        {"tarislar.city_inn.shale.1", 0x117F78U},
        {"tarislar.city_inn.shale.2", 0x118060U},
        {"tarislar.city_inn.shale.apartment", 0x0F5D09U},
        {"tarislar.city_inn.shale.last_run", 0x0F5DA6U},
        {"tarislar.city_inn.shale.caleb", 0x0F5EA7U},
        {"tarislar.city_inn.shale.errascoe", 0x0F5F4AU},
        {"tarislar.city_inn.shale.caleb_followup", 0x0F600DU},
        {"tarislar.city_inn.shale.refuses_recruitment", 0x0F60D6U},
        {"tarislar.city_inn.shale.attack", 0x0F617FU},
        {"tarislar.city_inn.frosty", 0x1180D2U},
        {"tarislar.city_inn.package.before", 0x113991U},
        {"tarislar.city_inn.package.after", 0x113A22U},
        {"tarislar.city_inn.leave", 0x0F4134U},
        {"tarislar.city_inn.option.crash", 0x103A17U},
        {"tarislar.city_inn.option.leave", 0x104FEBU},
        {"tarislar.city_inn.option.shale", 0x103C8DU},
        {"tarislar.city_inn.option.frosty", 0x103CB7U},
        {"tarislar.city_inn.option.pay", 0x103A7AU},
        {"tarislar.city_inn.option.decline", 0x103A13U},
        {"tarislar.city_inn.option.about_michael", 0x103CFCU},
        {"tarislar.city_inn.option.caleb", 0x103D53U},
        {"tarislar.city_inn.option.errascoe", 0x103D24U},
        {"tarislar.city_inn.option.recruit", 0x103C5BU},
        {"tarislar.city_inn.option.thanks", 0x104F7EU},
        // Scene-6/event-0 Owlfeather services and their source prompt labels.
        {"salish.owlfeather.option.lands", 0x103E57U},
        {"salish.owlfeather.option.council", 0x103E39U},
        {"salish.owlfeather.option.brother", 0x1040BEU},
        {"salish.owlfeather.option.rangers", 0x103E7BU},
        {"salish.owlfeather.option.villages", 0x103E8CU},
        {"salish.owlfeather.option.sinsearach", 0x103EB1U},
        {"salish.owlfeather.option.thanks", 0x104FC2U},
        {"salish.owlfeather.option.learned", 0x103EBDU},
        {"salish.owlfeather.option.like_who", 0x103EDBU},
        {"salish.owlfeather.option.simple_task", 0x103EECU},
        {"salish.owlfeather.option.later", 0x104FEBU},
        {"salish.owlfeather.option.freed", 0x1040EEU},
        {"salish.owlfeather.page.opening", 0x0F63CFU},
        {"salish.owlfeather.page.lands", 0x0F645BU},
        {"salish.owlfeather.page.rangers", 0x0F651EU},
        {"salish.owlfeather.page.villages", 0x0F65D1U},
        {"salish.owlfeather.page.sinsearach", 0x0F66ADU},
        {"salish.owlfeather.page.council", 0x0F675DU},
        {"salish.owlfeather.page.council_topics", 0x0F6809U},
        {"salish.owlfeather.page.brother", 0x0F68E0U},
        {"salish.owlfeather.page.brother_status", 0x0F6980U},
        {"salish.owlfeather.page.status_more", 0x0F6A3FU},
        {"salish.owlfeather.page.status_topic", 0x0F6AF5U},
        {"salish.owlfeather.page.simple_task", 0x0F6BA2U},
        {"salish.owlfeather.page.search_followup", 0x0F7190U},
        {"salish.owlfeather.page.task_result", 0x0F6CE8U},
        {"salish.owlfeather.page.return_offer", 0x0F6DB9U},
        {"salish.owlfeather.page.return_arrival", 0x0F6EB4U},
        {"salish.owlfeather.page.return_arrival_more", 0x0F6F21U},
        {"salish.owlfeather.page.return_reunion", 0x0F6FEEU},
        {"salish.owlfeather.page.return_closing", 0x0F70B7U},
        {"salish.red_buffalo.page.introduction", 0x0FB57CU},
        {"salish.red_buffalo.page.gift", 0x0FB5EBU},
        {"salish.red_buffalo.page.no_gift", 0x0FB647U},
        {"salish.red_buffalo.page.no_business_followup", 0x0FBD94U},
        {"salish.red_buffalo.page.no_business_response", 0x0FBEECU},
        {"tarislar.garden.apartments.page.guarded_entry", 0x0FBCC7U},
        {"tarislar.garden.apartments.page.guard_challenge", 0x0FBD5AU},
        {"tarislar.garden.apartments.option.no_business", 0x0FBD5AU},
        {"tarislar.garden.apartments.page.guard_leave", 0x0FBD94U},
        {"tarislar.garden.apartments.page.penthouse_entry", 0x0FBEECU},
        {"tarislar.garden.apartments.page.authorized_entry", 0x0FBE12U},
        {"salish.red_buffalo.page.topics", 0x0FBFCBU},
        {"salish.red_buffalo.page.michael", 0x0FC08BU},
        {"salish.red_buffalo.page.thon", 0x0FC166U},
        {"salish.red_buffalo.page.thon_more", 0x0FC2FDU},
        {"salish.red_buffalo.page.renraku", 0x0FC3ACU},
        {"salish.red_buffalo.page.renraku_more", 0x0FC477U},
        {"salish.red_buffalo.page.gillian", 0x0FC54EU},
        {"salish.red_buffalo.page.gillian_followup", 0x0FC628U},
        {"salish.red_buffalo.page.gillian_detail", 0x0FC6D6U},
        {"salish.red_buffalo.page.ito", 0x0FC757U},
        {"salish.red_buffalo.page.ito_detail", 0x0FC921U},
        {"salish.red_buffalo.page.branch_0", 0x0FC95AU},
        {"salish.red_buffalo.page.branch_1", 0x0FCE10U},
        {"salish.red_buffalo.page.branch_2", 0x0FCEAEU},
        {"salish.red_buffalo.page.branch_3", 0x0FCF20U},
        {"salish.red_buffalo.page.branch_4", 0x0FCFE4U},
        {"salish.red_buffalo.page.branch_5", 0x0FC9F3U},
        {"salish.red_buffalo.page.branch_6", 0x0FCA56U},
        {"salish.red_buffalo.page.branch_7", 0x0FCB09U},
        {"salish.red_buffalo.page.branch_8", 0x0FCBA7U},
        {"salish.red_buffalo.page.branch_9", 0x0FC222U},
        {"salish.red_buffalo.page.branch_10", 0x0FC841U},
        {"salish.red_buffalo.page.branch_11", 0x0FCC74U},
        {"salish.red_buffalo.page.branch_12", 0x0FCD6DU},
        {"salish.red_buffalo.page.branch_13", 0x0FD06EU},
        {"salish.red_buffalo.page.branch_14", 0x0FD130U},
        {"salish.red_buffalo.page.branch_15", 0x0FD200U},
        {"salish.red_buffalo.page.branch_16", 0x0F414EU},
        {"salish.red_buffalo.page.branch_17", 0x0F41A1U},
        {"salish.red_buffalo.page.branch_18", 0x0F4241U},
        {"salish.owlfeather.page.farewell_more", 0x0F7152U},
        {"salish.owlfeather.page.farewell", 0x0F4298U},
        // Scene-4 event 27 Spirit Eyes ritual and post-ritual answer pages.
        // These are imported as separate stable JSON text keys so graph
        // structure can select ROM pages without embedding dialogue copy.
        {"salish.spirit_eyes.page.first_contact", 0x0F7204U},
        {"salish.spirit_eyes.page.spirit_eyes_description", 0x0F72ABU},
        {"salish.spirit_eyes.page.welcome_question", 0x0F735EU},
        {"salish.spirit_eyes.page.welcome_response", 0x0F7401U},
        {"salish.spirit_eyes.page.ritual_question", 0x0F74DDU},
        {"salish.spirit_eyes.page.first_horn_request", 0x0F7579U},
        {"salish.spirit_eyes.page.ritual_setup", 0x0F78A3U},
        {"salish.spirit_eyes.option.brother_killer", 0x103F4AU},
        {"salish.spirit_eyes.option.gather", 0x103F7BU},
        {"salish.spirit_eyes.page.need_horn", 0x0F7ACBU},
        {"salish.spirit_eyes.page.second_item", 0x0F7641U},
        {"salish.spirit_eyes.page.need_pelt", 0x0F7B3AU},
        {"salish.spirit_eyes.page.final_item", 0x0F771DU},
        {"salish.spirit_eyes.page.need_scale", 0x0F7BC8U},
        {"salish.spirit_eyes.page.tasks_done", 0x0F77FEU},
        {"salish.spirit_eyes.page.post_ritual_opening", 0x0F7C79U},
        {"salish.spirit_eyes.page.post_ritual_warning", 0x0F7915U},
        {"salish.spirit_eyes.page.post_ritual_name", 0x0F798DU},
        {"salish.spirit_eyes.page.post_ritual_brother", 0x0F79F0U},
        {"salish.spirit_eyes.page.post_ritual_lust", 0x0F7A8BU},
        // Scene-47/event-25 Licourtrix dialogue records at 0xF4C44,
        // including both story-gated repeat pages and the A/B answer labels.
        {"salish.licourtrix.page.serpent_flight", 0x0F7DBFU},
        {"salish.licourtrix.page.empty_lair", 0x0F7E72U},
        {"salish.licourtrix.page.entering_antechamber", 0x0F7EF5U},
        {"salish.licourtrix.page.face_to_face", 0x0F7FC4U},
        {"salish.licourtrix.page.appreciation", 0x0F8087U},
        {"salish.licourtrix.page.intentions_prompt", 0x0F8115U},
        {"salish.licourtrix.page.lie_prompt", 0x0F8179U},
        {"salish.licourtrix.page.honesty_prompt", 0x0F81F7U},
        {"salish.licourtrix.page.task_prompt", 0x0F828FU},
        {"salish.licourtrix.page.life_debt", 0x0F82E8U},
        {"salish.licourtrix.page.protective_bond", 0x0F835CU},
        {"salish.licourtrix.page.farewell", 0x0F8410U},
        // Scene-47 mode-table activity opening page selected by 0x53EE2.
        {"salish.renraku.page.cave_ambush", 0x0F7D0CU},
        {"salish.licourtrix.option.neighborhood", 0x103F9FU},
        {"salish.licourtrix.option.scale", 0x103FC5U},
        {"salish.licourtrix.option.thanks", 0x104003U},
        {"salish.licourtrix.option.what", 0x104012U},
        {"salish.red_buffalo.page.branch_21", 0x0F8AB9U},
        {"salish.red_buffalo.page.branch_22", 0x0F8A88U},
        {"salish.red_buffalo.option.no_business", 0x0FBD5AU},
        // Scene-2/event-6 indexed title service 10 for Boris' Greenhouse.
        // Keep the text roots and labels as named host assets; the dialogue
        // graph itself is expressed in Lua from the traced source branches.
        {"boris.greenhouse.page.default", 0x0F8508U},
        {"boris.greenhouse.page.alternate_0", 0x0F8609U},
        {"boris.greenhouse.page.alternate_1", 0x0F8739U},
        {"boris.greenhouse.page.alternate_2", 0x0F8794U},
        {"boris.greenhouse.page.followup", 0x0F98FAU},
        {"boris.greenhouse.page.opening", 0x0F85B4U},
        {"boris.greenhouse.page.repeat", 0x0F8819U},
        {"boris.greenhouse.page.after_michael", 0x0F8697U},
        {"boris.greenhouse.page.story_prompt", 0x0F88F6U},
        {"boris.greenhouse.page.story_answer", 0x0F8947U},
        {"boris.greenhouse.page.information_prompt", 0x0F8AE8U},
        {"boris.greenhouse.page.paid_information", 0x0F8B83U},
        {"boris.greenhouse.page.paid_information_repeat", 0x0F8BE3U},
        {"boris.greenhouse.page.michael_unknown", 0x0F8C0EU},
        {"boris.greenhouse.page.michael_learned", 0x0F8CA2U},
        {"boris.greenhouse.page.mako_information", 0x0F8E83U},
        {"boris.greenhouse.page.michael_repeat", 0x0F8F3AU},
        {"boris.greenhouse.page.david_information", 0x0F8FA0U},
        {"boris.greenhouse.page.ilene_information", 0x0F91B2U},
        {"boris.greenhouse.page.thon_information", 0x0F9269U},
        {"boris.greenhouse.page.david_brother", 0x0F95E2U},
        {"boris.greenhouse.page.insufficient_funds", 0x0F8A88U},
        {"boris.greenhouse.option.story", 0x104035U},
        {"boris.greenhouse.option.information", 0x104051U},
        {"boris.greenhouse.option.later", 0x104FEBU},
        {"boris.greenhouse.option.accept", 0x103A7AU},
        {"boris.greenhouse.option.decline", 0x103C3BU},
        {"boris.greenhouse.option.michael", 0x1040BEU},
        {"boris.greenhouse.option.david", 0x103DECU},
        {"boris.greenhouse.option.ilene", 0x10411AU},
        {"boris.greenhouse.option.thon", 0x104142U},
        {"boris.greenhouse.option.brother", 0x103E19U},
        {"renraku.offices.page.entry", 0x1195CFU},
        {"renraku.offices.option", 0x11199CU},
        {"penumbra.fuchi.page.entry", 0x11C213U},
        {"penumbra.fuchi.page.followup", 0x11C2D2U},
        {"gates.undersound.page.entry", 0x113D46U},
        {"gates.undersound.page.followup", 0x113B47U},
        {"gates.undersound.page.result", 0x1131B5U},
        {"gates.undersound.page.closing", 0x113FE7U},
        // Shared output nodes returned by command 0x06's payment branches.
        {"shared.hotel.room.paid", 0x0F4241U},
        {"shared.hotel.room.insufficient", 0x0F42C1U},
        // Mode-2 field object prompts and outcomes selected by 0x20E48,
        // 0x20EC2, and their source mode-command handlers.
        {"field.maglock.door.title", 0x11C882U},
        {"field.maglock.option.more", 0x0FE5BFU},
        {"field.maglock.no-passkey", 0x11C8C8U},
        {"field.maglock.insufficient-passkey", 0x11C91CU},
        {"field.interaction.option.electronics", 0x11C984U},
        {"field.interaction.option.grenade", 0x11C9A1U},
        {"field.interaction.option.walk-away", 0x103B53U},
        {"field.maglock.passkey.result", 0x11C9E6U},
        {"field.maglock.electronics.no-skill", 0x11CA59U},
        {"field.maglock.electronics.success", 0x11CAB2U},
        {"field.maglock.electronics.failure", 0x11CAF6U},
        {"field.maglock.electronics.alert", 0x11CB5CU},
        {"field.maglock.grenade.unavailable", 0x11CBF5U},
        {"field.maglock.grenade.result", 0x11CC26U},
        {"field.maglock.already-unlocked", 0x0F4837U},
        {"field.security-terminal.title", 0x11CDBAU},
        {"field.security-terminal.failed", 0x11CDE9U},
        {"field.security-terminal.alarm-disabled", 0x11CE2AU},
        {"field.security-terminal.system-disabled", 0x11CE68U},
        {"field.security-terminal.files", 0x11CEC0U},
        {"field.security-terminal.floor.0", 0x11CEF4U},
        {"field.security-terminal.floor.1", 0x11CF02U},
        {"field.security-terminal.floor.2", 0x11CF11U},
        {"field.security-terminal.floor.3", 0x11CF1FU},
        {"field.security-terminal.floor.4", 0x11CF2EU},
        {"field.security-terminal.floor.5", 0x11CF3CU},
        // Redmond scene-2/event-1 mode-exit text. ROM 0x1F830 chooses the
        // description when FFF0D5 is clear, the payment text when positive,
        // and the package handoff when negative.
        {"field.mode.little_chiba.description", 0x11739EU},
        {"field.mode.little_chiba.package_handoff", 0x1176BBU},
        {"field.mode.little_chiba.package_payment", 0x117718U},
        // Scene-4/event-4 Council Island Hospital entry. The descriptor at
        // 0x110ECA selects the default copy and its two choices; alternate
        // results select the single-page roots at 0x110EE2/0x110EEA.
        {"council_island_hospital.entry.default", 0x11AAFAU},
        {"council_island_hospital.entry.package_handoff", 0x11ABB7U},
        {"council_island_hospital.entry.package_payment", 0x11AC2BU},
        {"council_island_hospital.option.services", 0x1044F2U},
        {"council_island_hospital.option.leave", 0x105101U},
        // Hospital mode 4/event 4 opens the full-screen menu at 0xC928.
        // Its title, separator, rows, and leave confirmation are ROM text;
        // the row pointers are the text records following the menu metadata.
        {"hospital.options.title", 0x0C150CU},
        {"hospital.options.separator", 0x0C151EU},
        {"hospital.options.medical_attention", 0x0C1534U},
        {"hospital.options.cyberware", 0x0C1545U},
        {"hospital.options.exit", 0x0C1552U},
        {"hospital.medical.row.100", 0x0C1564U},
        {"hospital.medical.row.90", 0x0C1578U},
        {"hospital.medical.row.70", 0x0C158CU},
        {"hospital.medical.row.40", 0x0C15A0U},
        {"hospital.medical.not_needed", 0x0C15B4U},
        {"hospital.medical.choose", 0x0C15D7U},
        {"hospital.medical.insufficient_funds", 0x0C15FBU},
        {"hospital.leave.prompt", 0x0C1629U},
        // Field-mode cyberspace entry uses these ROM pages when its deck,
        // MPCP, or datajack gate fails; 0xE1E6 presents the search copy.
        {"field.cyberspace.no_deck", 0xC21B5U},
        {"field.cyberspace.no_mpcp", 0xC2202U},
        {"field.cyberspace.no_datajack", 0xC2252U},
        {"field.cyberspace.search_progress", 0xC1978U},
        {"field.main_menu.passcodes.title", 0xC179BU},
        // Scene 2/event 7 indexed title service 22. ROM 0x545B6 passes this
        // three-record narrative to the shared field prompt runner.
        {"redmond.vigore-search.page-0", 0x117193U},
        {"redmond.vigore-search.page-1", 0x117266U},
        {"redmond.vigore-search.page-2", 0x117314U},
        // Scene-1/event-7 title service 21. These prompt labels are selected
        // by the Harlequin graph rooted at 0xF4DE0/0xF4DF4/0xF4E08.
        {"tarislar.garden.apartments.option.ask_about_michael", 0x1040BEU},
        {"tarislar.garden.apartments.option.who_is_thon", 0x10416DU},
        {"tarislar.garden.apartments.option.why_renraku", 0x104371U},
        {"tarislar.garden.apartments.option.ito_killed", 0x104494U},
        {"tarislar.garden.apartments.option.questions", 0x1047D4U},
        {"tarislar.garden.apartments.option.tomb_yes", 0x103A0EU},
        {"tarislar.garden.apartments.option.tomb_no", 0x103A13U},
        {"tarislar.garden.apartments.option.information", 0x104473U},
        // Scene-2/event-4 entry service and scene-13 Gunderson dialogue.
        {"jump-house.entry", 0x116C12U},
        {"gunderson.greeting", 0x106266U},
        {"gunderson.greeting.continuation", 0x106289U},
        {"gunderson.greeting.after_information", 0x106396U},
        {"gunderson.greeting.after_run_return", 0x105B52U},
        {"gunderson.option.information", 0x104051U},
        {"gunderson.option.work", 0x103A3AU},
        {"gunderson.option.leave", 0x104FEBU},
        {"gunderson.information.opening", 0x1063D9U},
        {"gunderson.information.contacts", 0x1064AAU},
        {"gunderson.information.after_return", 0x106533U},
        {"gunderson.information.option.questions", 0x1047D4U},
        {"gunderson.information.option.contacts", 0x104666U},
        {"gunderson.information.option.back", 0x104817U},
        {"gunderson.information.page.1", 0x1063E4U},
        {"gunderson.information.page.2", 0x106417U},
        {"gunderson.information.page.3", 0x106450U},
        {"gunderson.information.page.4", 0x106503U},
        {"gunderson.question.johnson", 0x104B81U},
        {"gunderson.answer.johnson", 0x106642U},
        {"gunderson.answer.johnson.more", 0x1066A0U},
        {"gunderson.work.opening", 0x106C74U},
        {"gunderson.work.page", 0x106CC5U},
        {"gunderson.work.followup", 0x10659BU},
        {"gunderson.contacts.introduction", 0x1069EDU},
        {"gunderson.contacts.decker_offer", 0x106AA8U},
        {"gunderson.contacts.option.accept", 0x103A7AU},
        {"gunderson.contacts.purchase", 0x106B64U},
        {"gunderson.question.answer", 0x10662BU},
        {"gunderson.question.followup", 0x106701U},
        {"gunderson.question.vigore.alternate_response", 0x1067AEU},
        {"gunderson.question.vigore.alternate_followup", 0x106878U},
        {"gunderson.question.option.local", 0x1047F4U},
        {"gunderson.question.prompt", 0x1069D5U},
        {"gunderson.question.option.vigore", 0x1045B0U},
        {"gunderson.question.option.errascoe", 0x103D24U},
        {"gunderson.question.vigore.response", 0x106CD2U},
        {"gunderson.question.errascoe.response", 0x106929U},
        // Scene 11 event-30 selects Mortimer Reed's dialogue profile and
        // opening descriptor through the mode-2 entry in 0x12CC50.
        {"big-rhino.booth.entry", 0x107D56U},
        {"big-rhino.mortimer.opening", 0x107DE0U},
        {"big-rhino.mortimer.repeat-booth-entry", 0x107E69U},
        {"big-rhino.mortimer.repeat-opening", 0x107F0FU},
        {"big-rhino.mortimer.conditional-opening", 0x105B7AU},
        {"big-rhino.mortimer.after-information", 0x107F61U},
        {"big-rhino.information.opening", 0x107FA9U},
        {"big-rhino.information.topics", 0x108049U},
        {"big-rhino.information.questions.prompt", 0x1080C3U},
        {"big-rhino.information.question.default", 0x106229U},
        {"big-rhino.information.question.julius", 0x10811FU},
        {"big-rhino.information.question.mako", 0x1081F1U},
        {"big-rhino.information.question.aragorn", 0x108565U},
        {"big-rhino.contacts.insufficient", 0x1082C9U},
        {"big-rhino.contacts.offer", 0x10838AU},
        {"big-rhino.contacts.purchased", 0x108457U},
        {"big-rhino.leave", 0x10854FU},
        {"big-rhino.option.information", 0x104051U},
        {"big-rhino.option.work", 0x103A3AU},
        {"big-rhino.option.leave", 0x104FEBU},
        {"big-rhino.information.option.questions", 0x1047D4U},
        {"big-rhino.information.option.contacts", 0x104666U},
        {"big-rhino.information.option.back", 0x104817U},
        {"big-rhino.contacts.option.accept", 0x103A7AU},
        {"big-rhino.question.option.another", 0x1047F4U},
        {"big-rhino.question.option.julius", 0x104636U},
        {"big-rhino.question.option.mako", 0x104B33U},
        {"big-rhino.question.option.aragorn", 0x103E22U},
        // Context 2's work callback at 0x215DA shows this page when no active
        // offer is being resumed, then indexes the context-2 run table.
        {"big-rhino.work.opening", 0x108501U},
        // Related restaurant and package text is imported for map inspection;
        // it is not asserted as the event-30 dialogue without a route trace.
        {"big-rhino.related.restaurant-description", 0x1154AEU},
        {"big-rhino.related.package-approach", 0x11555CU},
        {"big-rhino.related.maitre-d-offer", 0x1155B5U},
        {"big-rhino.related.package-payment", 0x1155E1U},
        // Scene 14 event 30 selects the Wanderer dialogue slot. These ROM
        // strings identify the first visit, the returning Johnson menu, the
        // conditional credit page, and the post-departure booth message.
        {"wanderer.vigore_jarl.first-impression", 0x106D7CU},
        {"wanderer.vigore_jarl.first-meeting", 0x106E5EU},
        {"wanderer.vigore_jarl.opening", 0x10702DU},
        {"wanderer.vigore_jarl.conditional-opening", 0x105BD9U},
        {"wanderer.vigore_jarl.booth-vacated", 0x10760CU},
        {"wanderer.vigore_jarl.booth-vacated.option.fixers", 0x10742EU},
        // Scene 12 event 30 selects Caleb Brightmore after the access gate.
        // These strings remain content records; dialogue.lua owns the menu
        // profile and the scene route TSV records the ROM state branches.
        {"icarus-descending.access.blocked", 0x108796U},
        {"icarus-descending.caleb.first-meeting", 0x10863EU},
        {"icarus-descending.caleb.first-meeting.continuation", 0x1086C6U},
        {"icarus-descending.caleb.opening", 0x108886U},
        {"icarus-descending.caleb.post-run-opening", 0x105C55U},
        {"icarus-descending.caleb.information.contacts-offer", 0x1088F8U},
        {"icarus-descending.caleb.work.opening", 0x109025U},
        {"icarus-descending.caleb.work.renraku-introduction", 0x1090E9U},
        {"icarus-descending.caleb.work.renraku-followup", 0x109174U},
        {"icarus-descending.caleb.work.recommended-run", 0x1092D3U},
        {"icarus-descending.caleb.work.matrix-requirement", 0x109222U},
        {"icarus-descending.caleb.leave", 0x10963FU},
        {"icarus-descending.caleb.transaction", 0x10959DU},
        {"icarus-descending.caleb.option.information", 0x104051U},
        {"icarus-descending.caleb.option.work", 0x103A3AU},
        {"icarus-descending.caleb.option.leave", 0x104FEBU},
        // Scene 18 event 30 selects Julius Strouther through the context-1
        // row at 0x12CC60. Preserve the separate first-visit page, repeat
        // roots, question branches, and paid Quinton contact flow.
        {"julius-strouther.booth-entry", 0x107695U},
        {"julius-strouther.first-meeting", 0x107727U},
        {"julius-strouther.repeat-greeting", 0x1077D4U},
        {"julius-strouther.after-information", 0x107806U},
        {"julius-strouther.information.opening", 0x107840U},
        {"julius-strouther.work.opening", 0x107A36U},
        {"julius-strouther.contacts.insufficient", 0x107A7DU},
        {"julius-strouther.question.caleb", 0x1078C0U},
        {"julius-strouther.question.mortimer", 0x107966U},
        {"julius-strouther.question.default", 0x106229U},
        {"julius-strouther.question.prompt", 0x107D37U},
        {"julius-strouther.contacts.opening", 0x107B37U},
        {"julius-strouther.contacts.offer", 0x107BE4U},
        {"julius-strouther.contacts.purchased", 0x107C5FU},
        {"julius-strouther.leave", 0x107D51U},
        {"julius-strouther.run-return", 0x105AB7U},
        {"julius-strouther.option.information", 0x104051U},
        {"julius-strouther.option.work", 0x103A3AU},
        {"julius-strouther.option.leave", 0x104FEBU},
        {"julius-strouther.information.option.questions", 0x1047D4U},
        {"julius-strouther.information.option.contacts", 0x104666U},
        {"julius-strouther.information.option.back", 0x104817U},
        {"julius-strouther.contacts.option.accept", 0x103A7AU},
        {"julius-strouther.question.option.another", 0x1047F4U},
        {"julius-strouther.question.option.about-caleb", 0x103D53U},
        {"julius-strouther.question.option.about-mortimer", 0x104605U},
        // Scene 19's event-30 fallback prompt at 0x20C78 uses this first
        // Scene 19's event-30 fallback prompt at 0x20C78 uses this first
        // narrative pointer. The remaining pointer slots are not presented
        // because the first choice pointer is the ROM's negative sentinel.
        {"jackals-lantern.old-man.opening", 0x0F3FBCU},
        // Vid-Phone choices and labels selected by 0xE4C8 through the
        // 13-entry contact-record pointer table at 0x109682. Each pointed-to
        // 12-byte record stores first/repeat stream pointers at +2/+6.
        {"vid_phone.option.services", 0x104862U},
        {"vid_phone.option.goodbye", 0x104889U},
        {"vid_phone.contact.page_next", 0x0C661DU},
        {"vid_phone.contact.page_previous", 0x0C662BU},
        {"vid_phone.contact.capt_quinton_jaymes.first_call", 0x1097FCU},
        {"vid_phone.contact.capt_quinton_jaymes.repeat_call", 0x1098A3U},
        {"vid_phone.contact.alesandro_hobbs.first_call", 0x10A15CU},
        {"vid_phone.contact.alesandro_hobbs.repeat_call", 0x10A1B9U},
        {"vid_phone.contact.sharkey.first_call", 0x10A8A8U},
        {"vid_phone.contact.sharkey.repeat_call", 0x10A8A8U},
        {"vid_phone.contact.kipp_david.first_call", 0x10AEE4U},
        {"vid_phone.contact.kipp_david.repeat_call", 0x10AF69U},
        {"vid_phone.contact.wilma_temmenhoff.first_call", 0x10B864U},
        {"vid_phone.contact.wilma_temmenhoff.repeat_call", 0x10B8CCU},
        {"vid_phone.contact.macklemann.first_call", 0x10C09EU},
        {"vid_phone.contact.macklemann.repeat_call", 0x10C09EU},
        {"vid_phone.contact.gregory_wilns.first_call", 0x10C928U},
        {"vid_phone.contact.gregory_wilns.repeat_call", 0x10C998U},
        {"vid_phone.goodbye", 0x1109A8U},
        // Gregory Wilns' 0x10C7FA contact descriptor points to this
        // four-node creature lesson and the two command-backed offers.
        {"vid_phone.gregory_wilns.services", 0x10C9B2U},
        {"vid_phone.gregory_wilns.creature_page_1", 0x10CB0AU},
        {"vid_phone.gregory_wilns.creature_page_2", 0x10CBEBU},
        {"vid_phone.gregory_wilns.creature_page_3", 0x10CCA8U},
        {"vid_phone.gregory_wilns.creature_page_4", 0x10CD8FU},
        {"vid_phone.gregory_wilns.discount_offer", 0x10CDDBU},
        {"vid_phone.gregory_wilns.discount_result", 0x10CE5FU},
        {"vid_phone.gregory_wilns.power_focus_offer", 0x10CED5U},
        {"vid_phone.gregory_wilns.power_focus_unavailable", 0x10CFBBU},
        {"vid_phone.gregory_wilns.power_focus_result", 0x10D020U},
        {"vid_phone.gregory_wilns.insufficient_balance", 0x10CA8BU},
        {"vid_phone.gregory_wilns.power_focus_ineligible", 0x1109B1U},
        {"vid_phone.gregory_wilns.option.paranormal_creatures", 0x1048FCU},
        {"vid_phone.gregory_wilns.option.magic_enhancement_device", 0x1048C8U},
        {"vid_phone.gregory_wilns.option.corporate_discount", 0x1048ABU},
        {"vid_phone.gregory_wilns.option.yes", 0x103A0EU},
        {"vid_phone.gregory_wilns.option.no", 0x103A13U},
        {"vid_phone.gregory_wilns.option.talk_later", 0x104FEBU},
        {"vid_phone.contact.mr_faradouchi.first_call", 0x10D106U},
        {"vid_phone.contact.mr_faradouchi.repeat_call", 0x10D170U},
        {"vid_phone.mr_faradouchi.unknown_caller", 0x10D223U},
        {"vid_phone.mr_faradouchi.mafia_offer", 0x10D2BBU},
        {"vid_phone.mr_faradouchi.yakuza_rejection", 0x10D346U},
        {"vid_phone.mr_faradouchi.pledge_accepted", 0x10D3D4U},
        {"vid_phone.contact.agira_tetsumi.first_call", 0x10D748U},
        {"vid_phone.contact.agira_tetsumi.repeat_call", 0x10D7B7U},
        {"vid_phone.agira_tetsumi.unknown_name", 0x10D862U},
        {"vid_phone.agira_tetsumi.yakuza_offer", 0x10D8BBU},
        {"vid_phone.agira_tetsumi.mafia_rejection", 0x10D98FU},
        {"vid_phone.agira_tetsumi.pledge_accepted", 0x10DDC3U},
        {"vid_phone.contact.henry_j_culver.first_call", 0x10DF12U},
        {"vid_phone.contact.henry_j_culver.repeat_call", 0x10DF92U},
        {"vid_phone.contact.max.first_call", 0x10E54EU},
        {"vid_phone.contact.max.repeat_call", 0x10E5C8U},
        {"vid_phone.contact.chillicut.first_call", 0x10EAF8U},
        {"vid_phone.contact.chillicut.repeat_call", 0x10EC09U},
        {"vid_phone.contact.professor_jefferson.first_call", 0x10F1EAU},
        {"vid_phone.contact.professor_jefferson.repeat_call", 0x10F257U},
        // Max's service graph at 0x10E400 has separate badge, hardware,
        // product-result, balance, and out-of-stock records.
        {"vid_phone.max.services", 0x10E606U},
        {"vid_phone.max.hardware_intro", 0x10E70AU},
        {"vid_phone.max.passkey_list", 0x10E7E5U},
        {"vid_phone.max.product_choices", 0x10E8A7U},
        {"vid_phone.max.badge_offer", 0x10E8C1U},
        {"vid_phone.max.badge_success", 0x10E944U},
        {"vid_phone.max.service_return", 0x10E5F0U},
        {"vid_phone.max.purchase_result", 0x10A601U},
        {"vid_phone.max.out_of_stock", 0x10A654U},
        {"vid_phone.max.insufficient_balance", 0x10E687U},
        {"vid_phone.max.goodbye", 0x10E9CEU},
        {"vid_phone.max.option.corporate_badge", 0x104C1BU},
        {"vid_phone.max.option.hardware", 0x1041A8U},
        {"vid_phone.max.option.smg", 0x1042EDU},
        {"vid_phone.max.option.passkey", 0x104277U},
        {"vid_phone.max.option.hk227", 0x1042FFU},
        {"vid_phone.max.option.ak97", 0x104314U},
        {"vid_phone.max.option.passkey_level_4", 0x104294U},
        {"vid_phone.max.option.passkey_level_5", 0x1042B7U},
        {"vid_phone.max.option.different_subject", 0x104817U},
        {"vid_phone.max.option.accept_badge", 0x103A7AU},
        // Professor Jefferson's contact descriptor at 0x10F0D4 selects this
        // two-page paranormal lesson and two independently paid services.
        {"vid_phone.professor_jefferson.services", 0x10F29FU},
        {"vid_phone.professor_jefferson.creature_page_1", 0x10F34DU},
        {"vid_phone.professor_jefferson.creature_page_2", 0x10F42EU},
        {"vid_phone.professor_jefferson.creature_page_3", 0x10F489U},
        {"vid_phone.professor_jefferson.club_information", 0x10F538U},
        {"vid_phone.professor_jefferson.club_offer", 0x10F5DFU},
        {"vid_phone.professor_jefferson.club_success", 0x10F63AU},
        {"vid_phone.professor_jefferson.student_offer", 0x10F684U},
        {"vid_phone.professor_jefferson.student_result", 0x10F73CU},
        {"vid_phone.professor_jefferson.goodbye", 0x10F799U},
        {"vid_phone.professor_jefferson.option.paranormal_creatures", 0x1048FCU},
        {"vid_phone.professor_jefferson.option.magic_club", 0x1048E3U},
        {"vid_phone.professor_jefferson.option.former_student", 0x104666U},
        {"vid_phone.professor_jefferson.option.yes", 0x103A0EU},
        {"vid_phone.professor_jefferson.option.no", 0x103A13U},
        // Henry J. Culver and Sgt. Chillicut share the same three service
        // prompts, but their source command IDs apply different prices.
        {"vid_phone.option.yes", 0x103A0EU},
        {"vid_phone.option.no", 0x103A13U},
        {"vid_phone.henry_j_culver.services", 0x10DFCAU},
        {"vid_phone.henry_j_culver.service_return", 0x10DFB6U},
        {"vid_phone.henry_j_culver.offer.delete_record", 0x10E138U},
        {"vid_phone.henry_j_culver.offer.hollywood_access", 0x10E1F6U},
        {"vid_phone.henry_j_culver.offer.weapon_permit", 0x10E336U},
        {"vid_phone.henry_j_culver.payment_insufficient", 0x10E09AU},
        {"vid_phone.henry_j_culver.record_deleted", 0x10E17CU},
        {"vid_phone.henry_j_culver.hollywood_access_granted", 0x10E2C0U},
        {"vid_phone.henry_j_culver.weapon_permit_granted", 0x10E3C1U},
        {"vid_phone.henry_j_culver.option.delete_record", 0x104BA0U},
        {"vid_phone.henry_j_culver.option.hollywood_access", 0x104AFCU},
        {"vid_phone.henry_j_culver.option.weapon_permit", 0x104BC9U},
        {"vid_phone.chillicut.first_call_details", 0x10EB5FU},
        {"vid_phone.chillicut.services", 0x10EC57U},
        {"vid_phone.chillicut.payment_insufficient", 0x10ECFFU},
        {"vid_phone.chillicut.offer.delete_record", 0x10ED7CU},
        {"vid_phone.chillicut.offer.hollywood_access", 0x10EE67U},
        {"vid_phone.chillicut.offer.weapon_permit", 0x10EF7BU},
        {"vid_phone.chillicut.record_deleted", 0x10EDDCU},
        {"vid_phone.chillicut.hollywood_access_granted", 0x10EF27U},
        {"vid_phone.chillicut.weapon_permit_granted", 0x10F059U},
        {"vid_phone.chillicut.option.delete_record", 0x104BA0U},
        {"vid_phone.chillicut.option.hollywood_access", 0x104AFCU},
        {"vid_phone.chillicut.option.weapon_permit", 0x104BC9U},
        // Captain Quinton's Services tree: information, equipment, and
        // contact offers selected by the 0x1096F4 prompt and command handlers.
        {"vid_phone.capt_quinton_jaymes.services", 0x109900U},
        {"vid_phone.capt_quinton_jaymes.hollywood_intro", 0x1099E7U},
        {"vid_phone.capt_quinton_jaymes.hollywood_topics", 0x109A7EU},
        {"vid_phone.capt_quinton_jaymes.weller_intro", 0x109B36U},
        {"vid_phone.capt_quinton_jaymes.weller_result", 0x109C0CU},
        {"vid_phone.capt_quinton_jaymes.firepower_intro", 0x109CF8U},
        {"vid_phone.capt_quinton_jaymes.firepower_items", 0x109DE3U},
        {"vid_phone.capt_quinton_jaymes.contacts_offer", 0x109E77U},
        {"vid_phone.capt_quinton_jaymes.purchase_result", 0x109E59U},
        {"vid_phone.capt_quinton_jaymes.sharkey_result", 0x109F1BU},
        {"vid_phone.capt_quinton_jaymes.purchase.insufficient", 0x109F9CU},
        {"vid_phone.capt_quinton_jaymes.purchase.out_of_stock", 0x109FE8U},
        // 0x10A0AC is the terminal prompt record; its title pointer is 10A7C4.
        {"vid_phone.capt_quinton_jaymes.services.disconnected", 0x10A7C4U},
        {"vid_phone.capt_quinton_jaymes.option.hollywood_information", 0x10499BU},
        {"vid_phone.capt_quinton_jaymes.option.firepower", 0x1041A8U},
        {"vid_phone.capt_quinton_jaymes.option.contacts", 0x104666U},
        {"vid_phone.capt_quinton_jaymes.option.more", 0x0FE5BFU},
        {"vid_phone.capt_quinton_jaymes.option.weller_prison_break", 0x109B00U},
        {"vid_phone.capt_quinton_jaymes.option.disconnect", 0x103B3CU},
        {"vid_phone.capt_quinton_jaymes.option.return_to_services", 0x104817U},
        {"vid_phone.capt_quinton_jaymes.option.frag_grenades", 0x104329U},
        {"vid_phone.capt_quinton_jaymes.option.hk227_s", 0x1042EDU},
        {"vid_phone.capt_quinton_jaymes.option.no_thanks", 0x103A13U},
        {"vid_phone.capt_quinton_jaymes.option.pay_for_rigger_contact", 0x103A7AU},
        {"vid_phone.capt_quinton_jaymes.option.decline_rigger_contact", 0x105101U},
        // Alesandro's service nodes are selected from his 0x10A080 record.
        // Commands 0x6B/0x6C are the two passkey levels; 0x6D introduces
        // Sgt. Macklemann. Content and option labels follow the record's
        // pointers and the handlers at 0x22934/0x2296E/0x229A8.
        {"vid_phone.alesandro_hobbs.services", 0x10A1EBU},
        {"vid_phone.alesandro_hobbs.corporation_information", 0x10A2E1U},
        {"vid_phone.alesandro_hobbs.security_patrols", 0x10A30DU},
        {"vid_phone.alesandro_hobbs.security_alerts", 0x10A38FU},
        {"vid_phone.alesandro_hobbs.security_technological", 0x10A3F5U},
        {"vid_phone.alesandro_hobbs.security_cameras", 0x10A4D8U},
        {"vid_phone.alesandro_hobbs.passkey_offer", 0x10A598U},
        {"vid_phone.alesandro_hobbs.purchase.insufficient", 0x10A28FU},
        {"vid_phone.alesandro_hobbs.purchase_result", 0x10A601U},
        {"vid_phone.alesandro_hobbs.purchase.out_of_stock", 0x10A654U},
        {"vid_phone.alesandro_hobbs.official_contact_offer", 0x10A689U},
        {"vid_phone.alesandro_hobbs.macklemann_result", 0x10A72DU},
        {"vid_phone.alesandro_hobbs.option.corporation_information", 0x104A62U},
        {"vid_phone.alesandro_hobbs.option.maglock_passkey", 0x104277U},
        {"vid_phone.alesandro_hobbs.option.seattle_official", 0x104D68U},
        {"vid_phone.alesandro_hobbs.option.second_security_type", 0x104A86U},
        {"vid_phone.alesandro_hobbs.option.security_procedures", 0x104AABU},
        {"vid_phone.alesandro_hobbs.option.avoid_detection", 0x104AD8U},
        {"vid_phone.alesandro_hobbs.option.pay_for_macklemann", 0x103A7AU},
        {"vid_phone.option.more", 0x0FE5BFU},
        // Sharkey's contact record at 0x10A7F2 contains three service
        // commands (0x6E/0x6F/0x70); keep its text pointers named so their
        // menu paths can be correlated with the ROM handlers.
        {"vid_phone.sharkey.services", 0x10A969U},
        {"vid_phone.sharkey.salish_passage", 0x10AA71U},
        {"vid_phone.sharkey.sinsearach_passage", 0x10AB51U},
        {"vid_phone.sharkey.smart_goggles_offer", 0x10ACA3U},
        {"vid_phone.sharkey.passage_result", 0x10ABCBU},
        {"vid_phone.sharkey.insufficient_funds", 0x10AA20U},
        {"vid_phone.sharkey.smart_goggles_result", 0x10AD62U},
        // Command 0x70 points at a small result record at 0x10E476; its first
        // longword is the actual inventory-full dialogue at 0x10A654.
        {"vid_phone.sharkey.smart_goggles_inventory_full", 0x10A654U},
        {"vid_phone.sharkey.option.pass_salish", 0x1049C2U},
        {"vid_phone.sharkey.option.pass_sinsearach", 0x104A06U},
        {"vid_phone.sharkey.option.smart_goggles", 0x1041C9U},
        {"vid_phone.sharkey.option.pay_for_passage", 0x103A7AU},
        {"vid_phone.sharkey.option.return_to_services", 0x104817U},
        {"vid_phone.sharkey.option.confirm_purchase", 0x103A0EU},
        {"vid_phone.sharkey.option.other_subject", 0x104FEBU},
        // Kipp David record 0x10AD8A. Commands 0x71-0x73 are 0x22A54,
        // 0x22A9E, and 0x22AF4. 0x71 writes the Excalibur bytes at FFFBBE
        // and does not set FF018F. Program buys check storage as rating 4
        // and store rating 6.
        {"vid_phone.kipp_david.intro", 0x10AFACU},
        {"vid_phone.kipp_david.services", 0x10B033U},
        {"vid_phone.kipp_david.matrix_intro", 0x10B236U},
        {"vid_phone.kipp_david.matrix_corps", 0x10B278U},
        {"vid_phone.kipp_david.matrix_result", 0x10B379U},
        {"vid_phone.kipp_david.program_intro", 0x10B468U},
        {"vid_phone.kipp_david.program_details", 0x10B540U},
        {"vid_phone.kipp_david.program_offer", 0x10B614U},
        {"vid_phone.kipp_david.deck_offer", 0x10B635U},
        {"vid_phone.kipp_david.insufficient_funds", 0x10B0F5U},
        {"vid_phone.kipp_david.disconnected", 0x10A7C4U},
        {"vid_phone.kipp_david.storage_full", 0x10BF30U},
        {"vid_phone.kipp_david.program_result", 0x10BEE3U},
        {"vid_phone.kipp_david.option.matrix", 0x104924U},
        {"vid_phone.kipp_david.option.programs", 0x10493EU},
        {"vid_phone.kipp_david.option.cyberdeck", 0x104200U},
        {"vid_phone.kipp_david.option.buy_rebound", 0x104969U},
        {"vid_phone.kipp_david.option.buy_degrade", 0x104982U},
        {"vid_phone.kipp_david.option.different_subject", 0x104817U},
        {"vid_phone.kipp_david.option.pay_for_deck", 0x103A7AU},
        // Wilma Temmenhoff record 0x10B71E. Commands 0x74/0x75 are
        // 0x22B4A and 0x22BA0. Both charge 5,000, size the download as
        // rating 3, and store rating 3. Result nodes send Services to
        // 0x10B75C and Goodbye to 0x10B778.
        {"vid_phone.wilma_temmenhoff.credstick", 0x10B924U},
        {"vid_phone.wilma_temmenhoff.services", 0x10BA03U},
        {"vid_phone.wilma_temmenhoff.matrix_intro", 0x10BADDU},
        {"vid_phone.wilma_temmenhoff.matrix_companies", 0x10BB68U},
        {"vid_phone.wilma_temmenhoff.matrix_sleaze", 0x10BC4DU},
        {"vid_phone.wilma_temmenhoff.matrix_result", 0x10BCB8U},
        {"vid_phone.wilma_temmenhoff.program_intro", 0x10BD48U},
        {"vid_phone.wilma_temmenhoff.program_degrade", 0x10BE03U},
        {"vid_phone.wilma_temmenhoff.program_offer", 0x10BE9DU},
        {"vid_phone.wilma_temmenhoff.insufficient_funds", 0x10BA82U},
        {"vid_phone.wilma_temmenhoff.storage_full", 0x10BF30U},
        {"vid_phone.wilma_temmenhoff.program_result", 0x10BEE3U},
        {"vid_phone.wilma_temmenhoff.later", 0x11031DU},
        // Sgt. Macklemann record 0x10BFAC. First and repeat both
        // enter 0x10BFD2. Command 0x76 is 0x22BF6: 5,000 nuyen, then
        // FFFC2C bit 28. Command 0x64 later clears that bit at the gate.
        {"vid_phone.macklemann.services", 0x10C12CU},
        {"vid_phone.macklemann.security_levels", 0x10C293U},
        {"vid_phone.macklemann.security_patrols", 0x10C36FU},
        {"vid_phone.macklemann.security_pass", 0x10C41DU},
        {"vid_phone.macklemann.avoid_invisibility", 0x10C4D2U},
        {"vid_phone.macklemann.avoid_cameras", 0x10C5B4U},
        {"vid_phone.macklemann.avoid_decker", 0x10C665U},
        {"vid_phone.macklemann.hollywood_offer", 0x10C6EDU},
        {"vid_phone.macklemann.insufficient_funds", 0x10C20AU},
        {"vid_phone.macklemann.disconnected", 0x10A7C4U},
        {"vid_phone.macklemann.gate", 0x10C799U},
        {"vid_phone.macklemann.option.hollywood", 0x104AFCU},
    };

    constexpr std::array<std::string_view, 7> kCommandOverviewTextKeys{{
        "field.command_overview.option.inventory_left",
        "field.command_overview.option.inventory_right",
        "field.command_overview.option.clips",
        "field.command_overview.option.attributes_skills",
        "field.command_overview.option.cyberware",
        "field.command_overview.option.magic",
        "field.command_overview.option.pocket_secretary"}};
    const auto command_labels =
        shadowrun::graphics::read_genesis_command_overview_labels(rom);
    for (std::size_t index = 0; index < command_labels.size(); ++index) {
        write_text_entry(kCommandOverviewTextKeys[index],
                         command_labels[index]);
    }
    // The ordinary map menu and its gated branches are read from ROM text
    // records. The resource-scene menu remains a separate two-choice screen.
    const auto field_menu =
        shadowrun::graphics::read_genesis_field_main_menu_text(rom);
    std::string no_contacts_text;
    for (std::size_t index = 0; index < field_menu.empty_phone_message.size();
         ++index) {
        if (index != 0U) {
            no_contacts_text.push_back('|');
        }
        no_contacts_text += field_menu.empty_phone_message[index];
    }
    const std::array<std::pair<std::string_view, const std::string*>, 13>
        kFieldMenuText{{
            {"field.main_menu.title", &field_menu.ordinary_title},
            {"field.main_menu.option.call_taxi", &field_menu.ordinary_choices[0]},
            {"field.main_menu.option.vid_phone", &field_menu.ordinary_choices[1]},
            {"field.main_menu.option.cyberspace", &field_menu.ordinary_choices[2]},
            {"field.main_menu.option.exit", &field_menu.ordinary_choices[3]},
            {"field.activity.main_menu.title", &field_menu.activity_title},
            {"field.activity.main_menu.option.cyberspace", &field_menu.activity_choices[0]},
            {"field.activity.main_menu.option.exit", &field_menu.activity_choices[1]},
            {"field.main_menu.cyberspace.title", &field_menu.cyberspace_title},
            {"field.main_menu.cyberspace.option.enter_passcode", &field_menu.cyberspace_choices[0]},
            {"field.main_menu.cyberspace.option.system_search", &field_menu.cyberspace_choices[1]},
            {"field.main_menu.cyberspace.option.exit", &field_menu.cyberspace_choices[2]},
            {"field.main_menu.vid_phone.no_contacts", &no_contacts_text},
        }};
    for (const auto& [key, value] : kFieldMenuText) {
        write_text_entry(key, *value);
    }
    for (const auto& [key, source_offset] : records) {
        std::vector<std::string> segments;
        try {
            segments = shadowrun::graphics::read_genesis_dialogue_segments(
                rom, source_offset);
        } catch (const std::exception& error) {
            std::ostringstream message;
            message << "dialogue text " << key << " at ROM +0x" << std::hex
                    << source_offset << ": " << error.what();
            throw std::runtime_error(message.str());
        }
        if (std::string_view(key).rfind("hospital.options.", 0U) == 0U &&
            segments.size() > 1U) {
            // The menu title, separator, and each option occupy consecutive
            // 0x80-delimited records terminated together by 0xFF. Emit only
            // the record selected by this pointer, not every later row.
            segments.resize(1U);
        }
        if (segments.empty()) {
            throw std::runtime_error("empty dialogue text: " +
                                     std::string(key));
        }
        std::string text;
        for (std::size_t segment_index = 0;
             segment_index < segments.size(); ++segment_index) {
            if (segment_index != 0U) {
                text.push_back('|');
            }
            text += segments[segment_index];
        }
        // The two upper security-terminal floor records are single-string
        // records in the source continuation; retain their first segment.
        if (key == "field.security-terminal.floor.4" ||
            key == "field.security-terminal.floor.5") {
            text = segments.front();
        }
        write_text_entry(key, text);
        // The ROM's 0xFE5BF continuation caption is shared by dialogue and
        // generic world-event prompts. Preserve the motel-specific alias for
        // existing speaker data, and emit the semantic shared UI key too.
        if (key == "stoker.motel.arrival.more") {
            write_text_entry("shared.more", text);
        }
    }
    // Enter Passcode labels are selected through the 12-pointer table at
    // 0xC19C2. Keep the label order stable for Lua's named passcode IDs while
    // resolving the displayed copy from each source pointer.
    constexpr std::array<std::string_view, 12> kPasscodeChoiceIds{{
        "run_passcode", "aztechnology", "club_penumbra",
        "seattle_general_hospital", "ares", "gates_undersound",
        "hollywood", "mitsuhama", "renraku", "fuchi", "lone_star",
        "ito_passcode"}};
    for (std::size_t index = 0; index < kPasscodeChoiceIds.size(); ++index) {
        const auto source = static_cast<std::size_t>(
            read_big_endian_long(rom, 0xC19C2U + index * 4U));
        const auto segments =
            shadowrun::graphics::read_genesis_dialogue_segments(rom, source);
        if (segments.empty()) {
            throw std::runtime_error(
                "empty field-menu passcode label in ROM pointer table");
        }
        std::string text;
        for (std::size_t segment = 0; segment < segments.size(); ++segment) {
            if (segment != 0U) {
                text.push_back('|');
            }
            text += segments[segment];
        }
        const auto key = "field.main_menu.passcodes.option." +
                         std::string(kPasscodeChoiceIds[index]);
        write_text_entry(key, text);
    }
    // The ROM stores both prices in one sentence at 0x10A5B9 and dispatches
    // level 3/5 as separate commands without separate captions. Split that
    // source offer into explicit host choices while retaining the ROM wording
    // and amounts.
    constexpr std::array<std::pair<std::string_view, std::string_view>, 2>
        kAlesandroPasskeyLevelChoices{{
            {"vid_phone.alesandro_hobbs.option.level_3_passkey",
             "Level 3 ($10,000)"},
            {"vid_phone.alesandro_hobbs.option.level_5_passkey",
             "Level 5 ($20,000)"},
        }};
    for (const auto& [key, text] : kAlesandroPasskeyLevelChoices) {
        write_text_entry(key, text);
    }
    // Recruited-runner biographies and the converted Ricky/Winston/Trent
    // topic trees are imported from their ROM stream starts. The source
    // descriptors at 0xFE848..0xFF8F0, 0xFE894..0xFE908,
    // 0xFEA8C..0xFEAF4, and 0xFEC78..0xFECE0 choose these records. Keep
    // every 0x80-delimited segment in the generated Lua value.
    struct RunnerDialogueRecord {
        std::string_view key;
        std::size_t source_offset;
        std::size_t expected_segments;
    };
    constexpr std::array<RunnerDialogueRecord, 46> kRunnerDialogueRecords{{
        {"runner.ricky.information.opening", 0x0FFCF4U, 3U},
        {"runner.ricky.information.continuation", 0x0FFD93U, 3U},
        {"runner.winston_marrs.information.opening", 0x1004A0U, 3U},
        {"runner.winston_marrs.information.continuation", 0x10058CU, 3U},
        {"runner.trent.information.opening", 0x100B59U, 4U},
        {"runner.trent.information.continuation", 0x100C27U, 1U},
        {"runner.petr_uvehr.information.opening", 0x101131U, 2U},
        {"runner.petr_uvehr.information.continuation", 0x1011E5U, 2U},
        {"runner.walking_bear.information.opening", 0x101FACU, 3U},
        {"runner.walking_bear.information.continuation", 0x10207EU, 2U},
        {"runner.phantom.information.opening", 0x1016EEU, 3U},
        {"runner.phantom.information.continuation", 0x10173CU, 3U},
        {"runner.two_fists.information.opening", 0x10254FU, 2U},
        {"runner.two_fists.information.continuation", 0x10261EU, 1U},
        {"runner.freya.information.opening", 0x10327CU, 2U},
        {"runner.freya.information.continuation", 0x103335U, 3U},
        {"runner.rianna.information.opening", 0x102B42U, 4U},
        {"runner.rianna.information.continuation", 0x102C18U, 3U},
        {"runner.stark.information.opening", 0x103730U, 1U},
        {"runner.stark.information.continuation", 0x1037C8U, 1U},
        {"runner.ricky.subject.title", 0x0FFE85U, 1U},
        {"runner.ricky.subject.option.shaman_magic", 0x104C9CU, 1U},
        {"runner.ricky.subject.option.local_area", 0x104CB8U, 1U},
        {"runner.ricky.subject.option.johnson", 0x105009U, 1U},
        {"runner.winston_marrs.subject.title", 0x10068AU, 1U},
        {"runner.winston_marrs.subject.option.halloweeners", 0x1047B4U, 1U},
        {"runner.winston_marrs.subject.option.lone_star", 0x104DF8U, 1U},
        {"runner.winston_marrs.subject.option.troll_origin", 0x105032U, 1U},
        {"runner.trent.subject.title", 0x100C82U, 1U},
        {"runner.trent.subject.option.mages", 0x104CD6U, 1U},
        {"runner.trent.subject.option.contacts", 0x104666U, 1U},
        {"runner.trent.subject.option.johnsons", 0x105009U, 1U},
        {"runner.ricky.subject.shaman_magic.page_0", 0x0FFE90U, 3U},
        {"runner.ricky.subject.shaman_magic.page_1", 0x0FFF24U, 4U},
        {"runner.ricky.subject.shaman_magic.page_2", 0x0FFFD6U, 3U},
        {"runner.ricky.subject.local_area.page_0", 0x100073U, 3U},
        {"runner.ricky.subject.local_area.page_1", 0x100117U, 2U},
        {"runner.ricky.subject.johnson_gunderson.page_0", 0x1001AFU, 3U},
        {"runner.winston_marrs.subject.halloweeners.page_0", 0x10069CU, 3U},
        {"runner.winston_marrs.subject.lone_star.page_0", 0x10078AU, 4U},
        {"runner.winston_marrs.subject.troll_origin.page_0", 0x100848U, 2U},
        {"runner.winston_marrs.subject.troll_origin.page_1", 0x1008DDU, 4U},
        {"runner.trent.subject.mages.page_0", 0x100C92U, 2U},
        {"runner.trent.subject.mages.page_1", 0x100D62U, 1U},
        {"runner.trent.subject.contacts.page_0", 0x100DBEU, 3U},
        {"runner.trent.subject.johnsons.page_0", 0x100EB2U, 3U},
    }};
    for (const auto& record : kRunnerDialogueRecords) {
        const auto segments =
            shadowrun::graphics::read_genesis_dialogue_segments(
                rom, record.source_offset);
        // The converted runner presenter models the non-empty text rows. In
        // these streams, repeated 0x80 controls separate adjacent ROM text
        // rows but do not add another presenter row, so retain the source
        // wording and validate against that typed view's row count.
        const auto text_row_count = static_cast<std::size_t>(std::count_if(
            segments.begin(), segments.end(),
            [](const std::string& segment) { return !segment.empty(); }));
        if (text_row_count != record.expected_segments) {
            throw std::runtime_error(
                "recruited-runner dialogue segment count changed for " +
                std::string(record.key));
        }
        std::string text;
        bool first_row = true;
        for (const auto& segment : segments) {
            if (segment.empty()) {
                continue;
            }
            if (!first_row) {
                text.push_back('|');
            }
            text += segment;
            first_row = false;
        }
        write_text_entry(record.key, text);
    }
    struct RunnerNameRecord {
        std::string_view key;
        std::string_view expected_name;
        std::size_t source_offset;
    };
    // The recruited runner names are the +0x58 fields in template records
    // 3..12 at 0x7291A. These are display names, so trim fixed-width padding
    // while preserving the ROM spelling and capitalization.
    constexpr std::array<RunnerNameRecord, 10> kRunnerNameRecords{{
        {"runner.ricky.name", "Ricky", 0x72C72U},
        {"runner.winston_marrs.name", "Winston Marrs", 0x72D72U},
        {"runner.trent.name", "Trent", 0x72E72U},
        {"runner.petr_uvehr.name", "Petr Uvehr", 0x72F72U},
        {"runner.walking_bear.name", "Walking Bear", 0x73072U},
        {"runner.phantom.name", "Phantom", 0x73172U},
        {"runner.two_fists.name", "Two Fists", 0x73272U},
        {"runner.freya.name", "Freya", 0x73372U},
        {"runner.rianna.name", "Rianna", 0x73472U},
        {"runner.stark.name", "Stark", 0x73572U},
    }};
    for (const auto& record : kRunnerNameRecords) {
        auto segments = shadowrun::graphics::read_genesis_dialogue_segments(
            rom, record.source_offset);
        if (segments.size() != 1U) {
            throw std::runtime_error(
                "recruited-runner display name is not one ROM string: " +
                std::string(record.key));
        }
        auto& name = segments.front();
        const auto first = name.find_first_not_of(' ');
        const auto last = name.find_last_not_of(' ');
        name = first == std::string::npos
                   ? std::string{}
                   : name.substr(first, last - first + 1U);
        if (name != record.expected_name) {
            throw std::runtime_error(
                "recruited-runner display name changed for " +
                std::string(record.key));
        }
        write_text_entry(record.key, name);
    }
    const auto hotel_text =
        shadowrun::graphics::read_genesis_hotel_options_text(rom);
    struct OpeningStoryTextRecord {
        std::string_view id;
        std::size_t source_offset;
        std::size_t byte_count;
    };
    // Each caption is a separate FF-terminated record in the ROM story text
    // stream at 0x5B7D8..0x5BA4B. Import the ROM bytes directly so copy never
    // has a second runtime C++ source of truth.
    constexpr std::array<OpeningStoryTextRecord, 22> kOpeningStoryText{{
        {"time", 0x5B7D8U, 27U},
        {"place", 0x5B7F4U, 24U},
        {"team_cover", 0x5B80DU, 35U},
        {"team_runners", 0x5B831U, 30U},
        {"team_wilderness", 0x5B850U, 29U},
        {"without_warning", 0x5B86EU, 25U},
        {"ambushed", 0x5B888U, 27U},
        {"massacre", 0x5B8A4U, 33U},
        {"captured", 0x5B8C6U, 33U},
        {"slain", 0x5B8E8U, 25U},
        {"tri_vid_recovered", 0x5B902U, 34U},
        {"national_news", 0x5B925U, 29U},
        {"brother_last_die", 0x5B943U, 29U},
        {"spending", 0x5B961U, 33U},
        {"vow", 0x5B983U, 31U},
        {"brother_death", 0x5B9A3U, 16U},
        {"landing", 0x5B9B4U, 27U},
        {"flag_cab", 0x5B9D0U, 30U},
        {"last_credstick", 0x5B9EFU, 24U},
        {"transaction", 0x5BA08U, 12U},
        {"motel", 0x5BA15U, 30U},
        {"redmond", 0x5BA34U, 23U},
    }};
    for (const auto& record : kOpeningStoryText) {
        if (record.source_offset > rom.size() ||
            record.byte_count >= rom.size() - record.source_offset ||
            rom[record.source_offset + record.byte_count] != 0xFFU) {
            throw std::runtime_error(
                "opening story text record is truncated or no longer FF-terminated: " +
                std::string(record.id));
        }
        std::string text;
        text.reserve(record.byte_count);
        for (std::size_t index = 0; index < record.byte_count; ++index) {
            const auto byte = rom[record.source_offset + index];
            if (byte < 0x20U || byte > 0x7EU) {
                throw std::runtime_error(
                    "opening story text record contains a control byte: " +
                    std::string(record.id));
            }
            text.push_back(static_cast<char>(byte));
        }
        write_text_entry("opening.story." + std::string(record.id), text);
    }
    // Startup layout text at ROM 0x1B6498, 0x1B64CF, 0x1B6518,
    // 0x1B6527, and 0x1B6577. Native C++ retains row placement and typed
    // controller state; Lua owns the rendered copy and ordered choice labels.
    write_text_entry("startup.options.title", " Start Options");
    write_text_entry("startup.options.separator",
                     std::string("[") + std::string(13U, '\\'));
    write_text_entry("startup.option.new_game", "  New Game");
    write_text_entry("startup.option.load_game", "  Load Game");
    write_text_entry("startup.archetype.heading.choose", "  Choose");
    write_text_entry("startup.archetype.heading.archetype", " Archetype");
    write_text_entry("startup.archetype.separator",
                     std::string("[") + std::string(10U, '\\'));
    write_text_entry("startup.option.samurai", "  Samurai");
    write_text_entry("startup.option.decker", "  Decker");
    write_text_entry("startup.option.gator_shaman", "  Gator");
    write_text_entry("startup.archetype.gator_shaman.suffix", "    Shaman");
    write_text_entry("startup.option.slot_one", "Game 1");
    write_text_entry("startup.option.slot_two", "Game 2");
    write_text_entry("startup.message.no_save.0", "Jack out and smell the");
    write_text_entry("startup.message.no_save.1", "   soycaf, chummer!");
    write_text_entry("startup.message.no_save.2", "   There's no game");
    write_text_entry("startup.message.no_save.3", "     saved here!");
    write_text_entry("startup.message.save_success.0",
                     " Successful download!");
    write_text_entry("startup.message.save_success.1",
                     "   Press any button");
    write_text_entry("startup.message.save_success.2",
                     "to run in the shadows.");
    // Generic Ares mode-text prompt paragraphs selected by ROM 0x1FA78.
    // These adjacent variable-length records end with 0x80/0xFF controls, so
    // decode their known visible spans rather than scanning into the next
    // record with the standalone dialogue-segment reader.
    const auto read_mode_prompt_paragraphs = [&rom](
        const std::initializer_list<std::pair<std::size_t, std::size_t>>
            records_to_read) {
        const auto read_record = [&rom](const std::size_t source_offset,
                                        const std::size_t byte_count) {
            if (source_offset > rom.size() ||
                byte_count > rom.size() - source_offset) {
                throw std::runtime_error(
                    "Ares mode-text prompt record exceeds the ROM");
            }
            std::string record;
            record.reserve(byte_count);
            for (std::size_t index = 0; index < byte_count; ++index) {
                const auto byte = rom[source_offset + index];
                if (byte < 0x20U || byte > 0x7EU) {
                    throw std::runtime_error(
                        "Ares mode-text prompt span contains a control byte");
                }
                record.push_back(static_cast<char>(byte));
            }
            return record;
        };
        std::string text;
        for (const auto& [offset, length] : records_to_read) {
            if (!text.empty()) {
                text += "\n\n";
            }
            text += read_record(offset, length);
        }
        return text;
    };
    write_text_entry(
        "field.mode.ares.store_intro",
        read_mode_prompt_paragraphs(
            {{0x118934U, 85U}, {0x11898BU, 67U}, {0x1189D0U, 33U}}));
    write_text_entry(
        "field.mode.ares.package_handoff",
        read_mode_prompt_paragraphs(
            {{0x1189F2U, 42U}, {0x118A1EU, 38U}, {0x118A46U, 37U}}));
    write_text_entry(
        "field.mode.ares.package_payment",
        read_mode_prompt_paragraphs(
            {{0x118A6CU, 54U}, {0x118AA4U, 80U}}));
    // ROM 0x1FA78 selects these three narrative variants for each of the
    // four Scene-5 storefront service records. The location's ordinary event
    // table byte is not the destination when this service selector matches.
    const auto read_mode_dialogue_record = [&rom](
        const std::size_t source_offset) {
        const auto segments =
            shadowrun::graphics::read_genesis_dialogue_segments(
                rom, source_offset);
        if (segments.empty()) {
            throw std::runtime_error(
                "field-mode dialogue record is empty at ROM +0x" +
                [&source_offset] {
                    std::ostringstream address;
                    address << std::hex << source_offset;
                    return address.str();
                }());
        }
        std::string text;
        for (std::size_t index = 0U; index < segments.size(); ++index) {
            if (index != 0U) {
                text.push_back('|');
            }
            text += segments[index];
        }
        return text;
    };
    struct RenrakuStorePromptRecords {
        std::string_view id;
        std::size_t intro;
        std::size_t handoff;
        std::size_t payment;
    };
    constexpr std::array<RenrakuStorePromptRecords, 4>
        kRenrakuStorePrompts{{
            {"wire_masters", 0x1198D2U, 0x11996EU, 0x1199E5U},
            {"merlins_lore", 0x119AD3U, 0x119B2CU, 0x119B96U},
            {"microtronics", 0x119C1FU, 0x119CC1U, 0x119D49U},
            {"weapons_world", 0x119DF3U, 0x119EA4U, 0x119F2BU},
        }};
    for (const auto& store : kRenrakuStorePrompts) {
        const auto prefix = "field.mode." + std::string(store.id) + ".";
        write_text_entry(prefix + "store_intro",
                         read_mode_dialogue_record(store.intro));
        write_text_entry(prefix + "package_handoff",
                         read_mode_dialogue_record(store.handoff));
        write_text_entry(prefix + "package_payment",
                         read_mode_dialogue_record(store.payment));
    }
    constexpr std::array<std::string_view, 12> kServiceTitleIds{{
        "weapons_world", "ares_weapon_emporium", "crime_mall",
        "medicine_lodge", "riannons", "merlins_lore", "microtronics",
        "wire_masters", "little_chiba", "dr_bobs",
        "council_island_hospital", "seattle_general_hospital"}};
    const auto service_titles =
        shadowrun::graphics::read_genesis_field_service_titles(rom);
    for (std::size_t index = 0; index < service_titles.size(); ++index) {
        write_text_entry("shop.service.option." +
                             std::string(kServiceTitleIds[index]),
                         service_titles[index].text);
    }
    constexpr std::array<std::string_view, 32> kShopActionHeadingIds{{
        "ares_weapons_display", "ares_weapons_available",
        "ares_armor_display", "ares_armor_available",
        "ares_other_items_display", "ares_other_items_equipment",
        "ares_weapon_accessories_display",
        "ares_weapon_accessories_accessories",
        "ares_firearm_accessories_select",
        "ares_firearm_accessories_to_accessorize", "ares_sell_item_from",
        "ares_sell_inventory", "combat_utilities_display",
        "combat_utilities", "defense_utilities_display", "defense_utilities",
        "mask_sense_utilities_display", "mask_sense_utilities",
        "cyberdecks_display", "cyberdecks", "cyberdeck_upgrades_display",
        "cyberdeck_upgrades", "cyberdeck_upgrades_second_display",
        "cyberdeck_upgrades_second", "magic_spells_display", "magic_spells",
        "magic_items_display", "magic_items", "magic_foci_display",
        "magic_foci", "sell_item_from_secondary",
        "sell_inventory_secondary"}};
    const auto service_action_titles =
        shadowrun::graphics::read_genesis_field_service_action_titles(rom);
    for (std::size_t index = 0; index < service_action_titles.size(); ++index) {
        const auto key = "shop.action_heading.option." +
                         std::string(kShopActionHeadingIds[index]);
        write_text_entry(key, service_action_titles[index]);
    }
    const auto shop_text =
        shadowrun::graphics::read_genesis_field_menu_action_text(rom);
    const auto universal_clips_text =
        shadowrun::graphics::read_genesis_field_service_universal_clips_text(
            rom);
    write_text_entry("shop.universal_clips.title",
                     universal_clips_text.title);
    write_text_entry("shop.universal_clips.description",
                     universal_clips_text.description);
    write_text_entry("shop.transaction.prompt",
                     shop_text.transaction_prompt);
    write_text_entry("shop.transaction.no_inventory_slot",
                     shop_text.no_inventory_slot_message);
    write_text_entry("shop.transaction.insufficient_funds",
                     shop_text.insufficient_funds_message);
    write_text_entry("shop.transaction.special_action_limit",
                     shop_text.special_action_limit_message);
    write_text_entry("shop.transaction.protected_sale",
                     shop_text.protected_sale_message);
    write_text_entry("shop.transaction.sale_quote_heading",
                     shop_text.sale_quote_heading);
    write_text_entry("shop.transaction.purchase_price_heading",
                     shop_text.purchase_price_heading);
    write_text_entry(
        "shop.cyberware.already_owned",
        read_multiline_rom_text(rom, 0x1AA787U, 0x40U,
                                "cyberware already-owned message"));
    write_text_entry(
        "shop.cyberware.spurs_conflict",
        read_multiline_rom_text(rom, 0x1AA7ADU, 0x40U,
                                "cyberware Spurs conflict message"));
    write_text_entry(
        "shop.cyberware.insufficient_essence",
        read_multiline_rom_text(rom, 0x1AA7E2U, 0x50U,
                                "cyberware Essence rejection message"));
    write_text_entry(
        "shop.cyberware.purchase_heading",
        read_multiline_rom_text(rom, 0x1AA76FU, 0x40U,
                                "cyberware purchase heading"));
    const auto essence_headings = read_multiline_rom_text(
        rom, 0x1AB517U, 0x40U, "cyberware Essence headings");
    const auto essence_heading_break = essence_headings.find('\n');
    if (essence_heading_break == std::string::npos) {
        throw std::runtime_error(
            "cyberware Essence headings lack their source line break");
    }
    write_text_entry("shop.cyberware.essence_heading",
                     essence_headings.substr(0U, essence_heading_break));
    write_text_entry("shop.cyberware.essence_cost_heading",
                     essence_headings.substr(essence_heading_break + 1U));
    constexpr std::array<std::string_view, 8> kCyberwareShopTextIds{{
        "datajack", "cyber_eyes", "hand_razors", "spurs", "smartlink",
        "muscle_replacement", "dermal_plating", "wired_reflexes"}};
    constexpr std::array<std::string_view, 8> kCyberwareDescriptionIds{{
        "datajack", "cyber_eyes", "hand_razors", "spurs", "smartlink",
        "muscle_replace", "dermal_plating", "wired_reflexes"}};
    for (std::size_t index = 0U; index < kCyberwareShopTextIds.size(); ++index) {
        const auto selection_name_offset = 0x1AAFA0U + index * 24U;
        const auto description_offset = read_big_endian_long(
            rom, 0x1AC8C0U + index * 4U);
        write_text_entry(
            "shop.cyberware.selection_name." +
                std::string(kCyberwareShopTextIds[index]),
            read_multiline_rom_text(rom, selection_name_offset, 24U,
                                    "cyberware selected-item name"));
        write_text_entry(
            "shop.cyberware.description." +
                std::string(kCyberwareDescriptionIds[index]),
            read_multiline_rom_text(rom, description_offset, 0x80U,
                                    "cyberware description"));
    }
    write_text_entry(
        "shop.store_closed",
        read_multiline_rom_text(rom, 0x1AA3B4U, 0x20U,
                                "MicroTronics store-closed message"));
    write_text_entry(
        "shop.progression.training_unavailable",
        read_multiline_rom_text(rom, 0x1AA42CU, 0x80U,
                                "field spell-training restriction"));
    write_text_entry(
        "shop.progression.maximum_level",
        read_multiline_rom_text(rom, 0x1AA85FU, 0x40U,
                                "field spell maximum-level message"));
    write_text_entry(
        "shop.progression.cost_heading",
        read_multiline_rom_text(rom, 0x1AA884U, 0x20U,
                                "field spell-training cost heading"));
    write_text_entry(
        "shop.progression.level_label",
        read_multiline_rom_text(rom, 0x1AA89AU, 0x10U,
                                "field spell progression level label"));
    const auto item_detail_labels =
        shadowrun::graphics::read_genesis_field_menu_item_detail_labels(rom);
    constexpr std::array<std::string_view, 3> kItemDetailLabelKeys{{
        "items.detail_label.ammo", "items.detail_label.uses",
        "items.detail_label.rating"}};
    for (std::size_t index = 0; index < item_detail_labels.size(); ++index) {
        write_text_entry(kItemDetailLabelKeys[index], item_detail_labels[index]);
    }
    for (std::size_t index = 0; index < assets.inventory_items.size(); ++index) {
        const auto code = std::to_string(index + 1U);
        write_text_entry("items.name." + code,
                         assets.inventory_items[index].name);
        write_text_entry("items.category." + code,
                         assets.inventory_items[index].category);
    }
    const auto item_detail_texts =
        shadowrun::graphics::read_genesis_field_action_detail_texts(rom);
    for (std::size_t index = 0; index < item_detail_texts.size(); ++index) {
        write_text_entry("items.action_detail." + std::to_string(index),
                         item_detail_texts[index]);
    }
    write_text_entry("hotel.options.title", hotel_text.title);
    write_text_entry("hotel.options.separator", hotel_text.separator);
    write_text_entry("hotel.option.karma_screen", hotel_text.options[0]);
    write_text_entry("hotel.option.check_out", hotel_text.options[1]);
    write_text_entry(
        "hotel.checkout.title",
        hotel_text.checkout_question[0] + "|" +
            hotel_text.checkout_question[1]);
    write_text_entry("hotel.option.checkout_yes",
                     hotel_text.checkout_options[0]);
    write_text_entry("hotel.option.checkout_no",
                     hotel_text.checkout_options[2]);
    write_text_entry("hotel.status.medical_attention_suffix",
                     hotel_text.medical_attention_suffix);
    for (std::size_t index = 2U;
         index < hotel_text.recovery_message.size();
         ++index) {
        write_text_entry("hotel.status.recovery." + std::to_string(index),
                         hotel_text.recovery_message[index]);
    }
    const auto pocket_secretary_text =
        shadowrun::graphics::read_genesis_pocket_secretary_text(rom);
    write_text_entry("pocket_secretary.root.title",
                     pocket_secretary_text.title);
    write_text_entry("pocket_secretary.root.text.option.separator",
                     pocket_secretary_text.separator);
    constexpr std::array<std::string_view, 5> kPocketSecretaryFieldChoiceIds{{
        "notebook", "group_items", "cyberdeck", "dismiss_runner",
        "save_load"}};
    for (std::size_t index = 0;
         index < kPocketSecretaryFieldChoiceIds.size(); ++index) {
        write_text_entry(
            "pocket_secretary.option." +
                std::string(kPocketSecretaryFieldChoiceIds[index]),
            pocket_secretary_text.options[index]);
    }
    for (const auto& [key, text] : field_text) {
        write_text_entry(key, text);
    }
    // Specialized importers and authored additions share this versioned JSON
    // catalog. New ROM-derived values replace old values by stable key while
    // keys owned by another importer remain available for the next pass.
    auto merged_texts = read_dialogue_text_json(path);
    // These pointers were previously given non-semantic or incorrect route
    // names. Keep only their source-derived dialogue owners in the catalog.
    merged_texts.erase(
        std::remove_if(merged_texts.begin(), merged_texts.end(),
                       [](const auto& entry) {
                           return entry.first ==
                                      "salish.red_buffalo.page.branch_19" ||
                                  entry.first ==
                                      "salish.red_buffalo.page.branch_20" ||
                                  entry.first ==
                                      "salish.spirit_eyes.option.neighborhood" ||
                                  entry.first ==
                                      "salish.spirit_eyes.option.scale" ||
                                  entry.first ==
                                      "salish.spirit_eyes.option.thanks" ||
                                  entry.first ==
                                      "salish.spirit_eyes.option.what";
                       }),
        merged_texts.end());
    for (const auto& [key, value] : generated_texts) {
        const auto existing = std::find_if(
            merged_texts.begin(), merged_texts.end(),
            [&key](const auto& entry) { return entry.first == key; });
        if (existing != merged_texts.end()) {
            existing->second = value;
            continue;
        }

        // Keep authored/imported catalog sections in their existing order.
        // Insert a new key after its current two-part namespace when possible
        // so a normal asset refresh does not alphabetize the entire document.
        const auto namespace_end = key.find('.', key.find('.') + 1U);
        const auto group = namespace_end == std::string::npos
                               ? key
                               : key.substr(0U, namespace_end);
        auto insertion = merged_texts.end();
        for (auto candidate = merged_texts.begin();
             candidate != merged_texts.end(); ++candidate) {
            const auto candidate_end = candidate->first.find(
                '.', candidate->first.find('.') + 1U);
            const auto candidate_group =
                candidate_end == std::string::npos
                    ? candidate->first
                    : candidate->first.substr(0U, candidate_end);
            if (candidate_group == group) {
                insertion = std::next(candidate);
            }
        }
        merged_texts.insert(insertion, {key, value});
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create dialogue JSON: " +
                                 path.string());
    }
    output << "{\n  \"schemaVersion\": 1,\n  \"texts\": {\n";
    std::size_t written = 0U;
    for (const auto& [key, value] : merged_texts) {
        output << "    ";
        write_json_string(output, key);
        output << ": ";
        write_json_string(output, value);
        if (++written < merged_texts.size()) {
            output << ',';
        }
        output << '\n';
    }
    output << "  }\n}\n";
    if (!output) {
        throw std::runtime_error("cannot write dialogue JSON: " +
                                 path.string());
    }
}

struct DecodedSourceByteStream {
    std::vector<std::uint8_t> bytes;
    std::size_t source_end = 0;
};

DecodedSourceByteStream decode_source_byte_stream_record(
    const std::vector<std::uint8_t>& rom, const std::size_t source_offset) {
    // ROM 0x01610 uses a DBF count: the leading word describes the number of
    // decrements, so a valid stream produces count + 1 output bytes. Literal,
    // repeat, and back-reference commands are kept here as an offline import
    // operation; no source decoder belongs in the native runtime.
    const auto read_byte = [&rom](const std::size_t offset) -> std::uint8_t {
        if (offset >= rom.size()) {
            throw std::runtime_error("source byte stream exceeds ROM");
        }
        return rom[offset];
    };
    const auto read_word = [&read_byte](const std::size_t offset) {
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(read_byte(offset)) << 8U) |
            static_cast<std::uint16_t>(read_byte(offset + 1U)));
    };

    std::size_t cursor = source_offset;
    int remaining = static_cast<int>(read_word(cursor++));
    cursor++;
    if (remaining < 0) {
        throw std::runtime_error("source byte stream count is invalid");
    }
    DecodedSourceByteStream result;
    result.bytes.reserve(static_cast<std::size_t>(remaining) + 1U);

    std::uint8_t flag_byte = 0;
    int flag_bits_remaining = 0;
    const auto emit = [&result](const std::uint8_t value) {
        result.bytes.push_back(value);
    };
    const auto finish_dbf = [&remaining]() {
        if (remaining == 0) {
            return true;
        }
        --remaining;
        return false;
    };

    while (true) {
        if (flag_bits_remaining == 0) {
            flag_byte = read_byte(cursor++);
            flag_bits_remaining = 8;
        }
        const bool compressed = (flag_byte & 0x80U) != 0U;
        flag_byte = static_cast<std::uint8_t>(flag_byte << 1U);
        --flag_bits_remaining;

        if (!compressed) {
            emit(read_byte(cursor++));
            if (finish_dbf()) {
                break;
            }
            continue;
        }

        const auto control = read_byte(cursor++);
        if ((control & 0x80U) == 0U) {
            const auto run_length = static_cast<int>((control & 0x7FU) + 2U);
            remaining -= run_length;
            if (remaining < 0) {
                throw std::runtime_error(
                    "source byte stream repeat exceeds declared output");
            }
            const auto value = read_byte(cursor++);
            for (int index = 0; index <= run_length; ++index) {
                emit(value);
            }
        } else {
            const auto distance_high = read_byte(cursor++);
            const auto distance = static_cast<std::size_t>(
                (((static_cast<std::uint16_t>(distance_high) << 3U) |
                  static_cast<std::uint16_t>((control >> 4U) & 0x07U)) &
                 0x07FFU));
            const auto run_length = static_cast<int>((control & 0x0FU) + 2U);
            remaining -= run_length;
            if (remaining < 0 || distance == 0U ||
                distance > result.bytes.size()) {
                throw std::runtime_error(
                    "source byte stream back-reference is invalid");
            }
            for (int index = 0; index <= run_length; ++index) {
                emit(result.bytes[result.bytes.size() - distance]);
            }
        }
        if (finish_dbf()) {
            break;
        }
    }

    result.source_end = cursor;
    return result;
}

std::vector<std::uint8_t> read_field_scene_metatile_record(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::GenesisFieldModeResources& resources) {
    if (resources.metatile_encoding ==
        shadowrun::graphics::GenesisMetatileEncoding::CompressedResource) {
        // The source pointer is an offline extraction input. The native
        // runtime receives the expanded bytes and never runs 0x01610.
        return decode_source_byte_stream_record(
                   rom, resources.metatile_stream_offset)
            .bytes;
    }

    const auto map_entry_count = static_cast<std::size_t>(
        resources.tilemap_width) * resources.tilemap_height;
    std::size_t required_metatile_count = 0U;
    const auto scan_tilemap = [&](const std::size_t tilemap_offset) {
        if (tilemap_offset > rom.size() ||
            map_entry_count > (rom.size() - tilemap_offset) / 2U) {
            throw std::runtime_error(
                "field scene tilemap exceeds input image");
        }
        for (std::size_t entry = 0; entry < map_entry_count; ++entry) {
            const auto name_word = read_big_endian_word(
                rom, tilemap_offset + entry * sizeof(std::uint16_t));
            required_metatile_count = std::max(
                required_metatile_count,
                static_cast<std::size_t>(name_word) + 1U);
        }
    };
    scan_tilemap(resources.primary_tilemap_offset);
    scan_tilemap(resources.secondary_tilemap_offset);

    const auto byte_count = required_metatile_count * 32U;
    if (resources.metatile_stream_offset > rom.size() ||
        byte_count > rom.size() - resources.metatile_stream_offset) {
        throw std::runtime_error(
            "field scene raw metatile table exceeds input image");
    }
    return std::vector<std::uint8_t>(
        rom.begin() + static_cast<std::ptrdiff_t>(
                          resources.metatile_stream_offset),
        rom.begin() + static_cast<std::ptrdiff_t>(
                          resources.metatile_stream_offset + byte_count));
}

std::vector<std::uint8_t> read_field_scene_collision_records(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::GenesisFieldModeResources& resources) {
    std::vector<std::uint8_t> records;
    auto cursor = resources.terrain_record_offset;
    for (;;) {
        const auto x_start = read_big_endian_word(rom, cursor);
        if ((x_start & 0x8000U) != 0U) {
            return records;
        }
        if (cursor > rom.size() || rom.size() - cursor < 10U) {
            throw std::runtime_error(
                "field scene terrain collision record is truncated");
        }
        records.insert(records.end(),
                       rom.begin() + static_cast<std::ptrdiff_t>(cursor),
                       rom.begin() + static_cast<std::ptrdiff_t>(cursor + 10U));
        cursor += 10U;
    }
}

std::string field_scene_record_directory(const std::uint16_t scene_id) {
    std::ostringstream result;
    result << "records/field/scenes/scene-" << std::setfill('0')
           << std::setw(2) << scene_id;
    return result.str();
}

void extract_field_session_record_assets(
    const std::vector<std::uint8_t>& rom,
    const std::filesystem::path& root) {
    // These are source records consumed by the converted field-session
    // bootstrap. They remain raw, individually named host records; the
    // conversion layer owns their typed interpretation.
    write_rom_record(rom, root,
                     "records/field/action-selection-profiles.bin",
                     0x166AD8U, 5U * 0x22U);
    write_rom_record(rom, root, "records/field/mode-selection-state.bin",
                     0x14C1F4U, 0x17U * sizeof(std::uint16_t));
    // 0x03084 preserves FFDDC4 in D0 after the 0x3032 classifier and indexes
    // all 56 entries at 0x3232. Materialize the complete 48-word rows once
    // so the conversion runtime can select a scene by typed id without
    // carrying a ROM pointer table.
    std::vector<std::uint8_t> scene_profiles;
    scene_profiles.reserve(shadowrun::rom::kFieldModeCount * 0x60U);
    std::vector<std::uint8_t> elevator_scene_groups;
    elevator_scene_groups.reserve(
        shadowrun::rom::kFieldModeCount * 6U);
    for (std::uint16_t scene_id = 0;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto table_entry =
            0x3232U + static_cast<std::size_t>(scene_id) * 4U;
        const auto profile_offset = static_cast<std::size_t>(
            read_big_endian_long(rom, table_entry));
        if (profile_offset > rom.size() ||
            rom.size() - profile_offset < 0x60U) {
            throw std::runtime_error(
                "field scene profile row exceeds input image");
        }
        scene_profiles.insert(
            scene_profiles.end(),
            rom.begin() + static_cast<std::ptrdiff_t>(profile_offset),
            rom.begin() +
                static_cast<std::ptrdiff_t>(profile_offset + 0x60U));

        // 0x58DE0 reads a scene group from the selected profile's +0x58
        // resource pointer: its first longword points to a word-counted list
        // of child scene IDs. Resolve the list now, preserving empty rows for
        // profiles without an elevator group so the runtime consumes only
        // typed native IDs.
        std::array<std::uint8_t, 6U> elevator_group{};
        const auto resource_bundle = static_cast<std::size_t>(
            read_big_endian_long(rom, profile_offset + 0x58U));
        if (resource_bundle != 0U && resource_bundle <= rom.size() &&
            rom.size() - resource_bundle >= sizeof(std::uint32_t)) {
            const auto group_offset = static_cast<std::size_t>(
                read_big_endian_long(rom, resource_bundle));
            if (group_offset != 0U && group_offset <= rom.size() &&
                rom.size() - group_offset >= sizeof(std::uint16_t)) {
                const auto count = read_big_endian_word(rom, group_offset);
                if (count >= 2U && count <= 5U &&
                    rom.size() - group_offset >=
                        (1U + static_cast<std::size_t>(count)) *
                            sizeof(std::uint16_t)) {
                    std::array<std::uint16_t, 5U> child_scenes{};
                    bool valid_group = true;
                    for (std::size_t child = 0; child < count; ++child) {
                        child_scenes[child] = read_big_endian_word(
                            rom, group_offset +
                                     (1U + child) * sizeof(std::uint16_t));
                        valid_group = valid_group &&
                            child_scenes[child] <
                                shadowrun::rom::kFieldModeCount;
                    }
                    if (valid_group) {
                        elevator_group[0] = static_cast<std::uint8_t>(count);
                        for (std::size_t child = 0; child < count; ++child) {
                            elevator_group[1U + child] =
                                static_cast<std::uint8_t>(child_scenes[child]);
                        }
                    }
                }
            }
        }
        elevator_scene_groups.insert(elevator_scene_groups.end(),
                                     elevator_group.begin(),
                                     elevator_group.end());

        // ROM 0x167AE resolves FFDE1E from the selected resource-scene
        // profile, dereferences the first bundle longword, and passes that
        // word to 0x0D3A before publishing FFE12C. Resolve this source chain
        // once into a named two-byte host value; the runtime never follows
        // the profile or bundle pointer.
        if (scene_id >= 0x15U && scene_id < 0x2CU) {
            const auto bundle_offset = static_cast<std::size_t>(
                read_big_endian_long(rom, profile_offset + 0x58U));
            if (bundle_offset == 0U || bundle_offset > rom.size() ||
                rom.size() - bundle_offset < 0x18U) {
                throw std::runtime_error(
                    "world activity scene has no complete resource bundle");
            }
            const auto bound_offset = static_cast<std::size_t>(
                read_big_endian_long(rom, bundle_offset));
            const auto bound = read_big_endian_word(rom, bound_offset);
            if (bound == 0U || bound > 6U) {
                throw std::runtime_error(
                    "world activity random-index bound changed");
            }
            const std::vector<std::uint8_t> bound_record{
                static_cast<std::uint8_t>(bound >> 8U),
                static_cast<std::uint8_t>(bound)};
            write_host_record(
                bound_record, root,
                field_scene_record_directory(scene_id) +
                    "/world-activity-random-index-bound.bin");
            const auto cursor_word = read_big_endian_word(
                rom, bundle_offset + 0x14U);
            const auto resource_index = read_big_endian_word(
                rom, bundle_offset + 0x16U);
            const std::vector<std::uint8_t> resource_cursor_record{
                static_cast<std::uint8_t>(cursor_word),
                static_cast<std::uint8_t>(resource_index >> 8U),
                static_cast<std::uint8_t>(resource_index)};
            write_host_record(
                resource_cursor_record, root,
                field_scene_record_directory(scene_id) +
                    "/world-activity-resource-cursor.bin");
        }
    }
    write_host_record(scene_profiles, root,
                      "records/field/scene-profiles.bin");
    write_host_record(elevator_scene_groups, root,
                      "records/field/elevator-scene-groups.bin");
    // ROM 0x0BABC loads the eight signed fixed-point movement vectors from
    // 0x0C002 before 0x0B77E probes the candidate position. Keep this as one
    // direct source record; the conversion loader owns its typed pairs.
    write_rom_record(
        rom, root, "records/field/entity-movement-direction-table.bin",
        0x0C002U, 8U * 4U);
    // ROM 0xBC36 selects one of four three-point collision-probe banks from
    // the signed FFEF10..12 sample bytes. Keep the complete 4 x 3 x 2-word
    // source table as one direct record; the conversion loader interprets the
    // big-endian signed offsets and the runtime never reads ROM addresses.
    write_rom_record(
        rom, root, "records/field/entity-collision-probe-offset-banks.bin",
        0x0BD34U, 4U * 3U * 2U * sizeof(std::uint16_t));
    // ROM 0x195C indexes this sixteen-word D-pad/heading table before the
    // movement heading is slewed and converted to a vector-table ordinal.
    write_rom_record(
        rom, root, "records/field/entity-controller-heading-table.bin",
        0x019D2U, 16U * sizeof(std::uint16_t));
    // ROM 0xBB86 selects one of four descriptors through the long-pointer
    // table at 0xBBF6. Each descriptor stores pointers to FFEF10..13 and two
    // fallback direction slots. Resolve those pointers once during offline
    // extraction into four six-byte semantic records:
    // primary sample index, secondary sample index, primary fallback, and
    // secondary fallback. The runtime never follows the source RAM pointers.
    const std::array<std::uint32_t, 4> route_sample_pointers{
        0xFFEF10U, 0xFFEF11U, 0xFFEF12U, 0xFFEF13U};
    std::vector<std::uint8_t> movement_route_table;
    movement_route_table.reserve(4U * 6U);
    for (std::size_t route_index = 0; route_index < 4U; ++route_index) {
        const auto descriptor_pointer = read_big_endian_long(
            rom, 0x0BBF6U + route_index * sizeof(std::uint32_t));
        if (descriptor_pointer < 0x0BC06U ||
            descriptor_pointer + 12U > rom.size()) {
            throw std::runtime_error(
                "field movement route descriptor pointer changed");
        }
        const auto primary_pointer =
            read_big_endian_long(rom, descriptor_pointer);
        const auto secondary_pointer =
            read_big_endian_long(rom, descriptor_pointer + 4U);
        const auto primary = std::find(
            route_sample_pointers.begin(), route_sample_pointers.end(),
            primary_pointer);
        const auto secondary = std::find(
            route_sample_pointers.begin(), route_sample_pointers.end(),
            secondary_pointer);
        if (primary == route_sample_pointers.end() ||
            secondary == route_sample_pointers.end()) {
            throw std::runtime_error(
                "field movement route sample pointer changed");
        }
        movement_route_table.push_back(static_cast<std::uint8_t>(
            std::distance(route_sample_pointers.begin(), primary)));
        movement_route_table.push_back(static_cast<std::uint8_t>(
            std::distance(route_sample_pointers.begin(), secondary)));
        const auto primary_fallback =
            read_big_endian_word(rom, descriptor_pointer + 8U);
        const auto secondary_fallback =
            read_big_endian_word(rom, descriptor_pointer + 10U);
        movement_route_table.push_back(
            static_cast<std::uint8_t>(primary_fallback >> 8U));
        movement_route_table.push_back(
            static_cast<std::uint8_t>(primary_fallback));
        movement_route_table.push_back(
            static_cast<std::uint8_t>(secondary_fallback >> 8U));
        movement_route_table.push_back(
            static_cast<std::uint8_t>(secondary_fallback));
    }
    write_host_record(movement_route_table, root,
                      "records/field/entity-movement-route-table.bin");
    // 0x03084/0x0313A can select every row in the 0x3232 scene table. Keep
    // the complete direct resource set available to the conversion layer:
    // fixed terrain work buffers, offline-expanded metatile bytes, the map
    // descriptor, collision records through the source sentinel, and the
    // optional event table. The runtime consumes these files by scene id and
    // never follows the source pointers or decodes 0x01610.
    std::filesystem::create_directories(root / "records/field/scenes");
    std::ofstream scene_index(
        root / "records/field/scenes/index.tsv", std::ios::binary);
    if (!scene_index) {
        throw std::runtime_error("cannot create field scene resource index");
    }
    scene_index
        << "scene_id\trecord_offset\tmetatile_stream\tmetatile_encoding"
           "\tlayer_a\tlayer_b\tterrain_stream\tdescriptor"
           "\tcollision_rectangles\tevent_table\n";
    for (std::uint16_t scene_id = 0;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(rom, scene_id);
        const auto directory = field_scene_record_directory(scene_id);
        const auto layer_byte_count =
            0xE10U * sizeof(std::uint16_t);
        const auto layer_a = directory + "/terrain-layer-a.bin";
        const auto layer_b = directory + "/terrain-layer-b.bin";
        const auto terrain_stream = directory + "/terrain-stream.bin";
        const auto descriptor = directory + "/descriptor.bin";
        const auto collision = directory + "/terrain-collision-rectangles.bin";
        const auto event = directory + "/event-table.bin";
        write_rom_record(rom, root, layer_a,
                         resources.primary_tilemap_offset, layer_byte_count);
        write_rom_record(rom, root, layer_b,
                         resources.secondary_tilemap_offset, layer_byte_count);
        write_host_record(read_field_scene_metatile_record(rom, resources),
                          root, terrain_stream);
        write_rom_record(rom, root, descriptor, resources.record_offset, 0x58U);
        write_host_record(read_field_scene_collision_records(rom, resources),
                          root, collision);
        if (resources.event_table_offset != 0U) {
            write_rom_record(rom, root, event, resources.event_table_offset, 32U);
        }
        scene_index << scene_id << "\t0x" << std::uppercase << std::hex
                    << resources.record_offset << "\t0x"
                    << resources.metatile_stream_offset << "\t"
                    << (resources.metatile_encoding ==
                                shadowrun::graphics::GenesisMetatileEncoding::
                                    RawNameWords
                            ? "raw-name-words"
                            : "compressed-resource")
                    << std::dec << "\t" << layer_a << "\t" << layer_b << "\t"
                    << terrain_stream << "\t" << descriptor << "\t" << collision;
        if (resources.event_table_offset != 0U) {
            scene_index << "\t" << event;
        }
        scene_index << '\n';
    }
}

struct CroppedAsset {
    shadowrun::graphics::RgbaImage image;
    std::int32_t origin_x = 0;
    std::int32_t origin_y = 0;
};

CroppedAsset crop_transparent_bounds(
    const shadowrun::graphics::RgbaImage& source) {
    if (source.width == 0 || source.height == 0 || source.pixels.empty()) {
        throw std::invalid_argument("cannot crop an empty asset");
    }
    std::int32_t left = source.width;
    std::int32_t top = source.height;
    std::int32_t right = -1;
    std::int32_t bottom = -1;
    for (std::int32_t y = 0; y < source.height; ++y) {
        for (std::int32_t x = 0; x < source.width; ++x) {
            const auto& pixel = source.pixels[
                static_cast<std::size_t>(y) * source.width + x];
            if (pixel.alpha == 0U) {
                continue;
            }
            left = std::min(left, x);
            top = std::min(top, y);
            right = std::max(right, x);
            bottom = std::max(bottom, y);
        }
    }
    if (right < left || bottom < top) {
        throw std::runtime_error("asset contains no opaque pixels");
    }
    CroppedAsset result;
    result.origin_x = left;
    result.origin_y = top;
    result.image.width = static_cast<std::uint16_t>(right - left + 1);
    result.image.height = static_cast<std::uint16_t>(bottom - top + 1);
    result.image.pixels.resize(static_cast<std::size_t>(result.image.width) *
                               result.image.height);
    for (std::int32_t y = top; y <= bottom; ++y) {
        for (std::int32_t x = left; x <= right; ++x) {
            result.image.pixels[
                static_cast<std::size_t>(y - top) * result.image.width +
                (x - left)] = source.pixels[
                    static_cast<std::size_t>(y) * source.width + x];
        }
    }
    return result;
}

CroppedAsset crop_source_region(
    const shadowrun::graphics::RgbaImage& source, const std::int32_t origin_x,
    const std::int32_t origin_y, const std::uint16_t width,
    const std::uint16_t height) {
    if (width == 0U || height == 0U || origin_x < 0 || origin_y < 0 ||
        origin_x + width > source.width || origin_y + height > source.height) {
        throw std::invalid_argument("source asset region exceeds image");
    }
    CroppedAsset result;
    result.origin_x = origin_x;
    result.origin_y = origin_y;
    result.image.width = width;
    result.image.height = height;
    result.image.pixels.resize(static_cast<std::size_t>(width) * height);
    for (std::int32_t y = 0; y < height; ++y) {
        for (std::int32_t x = 0; x < width; ++x) {
            result.image.pixels[static_cast<std::size_t>(y) * width + x] =
                source.pixels[static_cast<std::size_t>(origin_y + y) *
                                  source.width + origin_x + x];
        }
    }
    return result;
}

void write_scene_asset(const std::filesystem::path& root,
                       const std::string& relative_name,
                       const shadowrun::graphics::RgbaImage& source,
                       std::ofstream& manifest, bool crop) {
    const auto asset = crop ? crop_transparent_bounds(source)
                            : CroppedAsset{source, 0, 0};
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    shadowrun::graphics::write_bmp32(path.string(), asset.image);
    manifest << relative_name << '\t' << asset.origin_x << '\t'
             << asset.origin_y << '\t' << asset.image.width << '\t'
             << asset.image.height << '\n';
}

void write_circuit_backgrounds(
    const std::filesystem::path& root,
    const std::array<shadowrun::graphics::RgbaImage, 7>& phases,
    std::ofstream& manifest) {
    // Each decoder result is already the complete 320x224 background with
    // one ROM-selected circuit-light phase applied. Store those seven frames
    // directly so runtime animation is a small frame selection, not a
    // reconstruction from hundreds of cropped fragments.
    for (std::size_t phase_index = 0; phase_index < phases.size();
         ++phase_index) {
        write_scene_asset(root,
                          "title/circuit-background-" +
                              std::to_string(phase_index) + ".bmp",
                          phases[phase_index], manifest, false);
    }
}

void remove_legacy_circuit_fragments(const std::filesystem::path& root) {
    const auto title_root = root / "title";
    if (!std::filesystem::exists(title_root)) {
        return;
    }
    for (const auto& entry :
         std::filesystem::directory_iterator(title_root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto name = entry.path().filename().string();
        if (name.rfind("circuit-light-", 0) != 0 ||
            entry.path().extension() != ".bmp") {
            continue;
        }
        std::error_code error;
        std::filesystem::remove(entry.path(), error);
        if (error) {
            throw std::runtime_error(
                "cannot remove legacy circuit light fragment: " +
                entry.path().string());
        }
    }
}

void extract_title_boot_assets(const std::vector<std::uint8_t>& rom,
                               const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    std::ofstream manifest(root / "manifest.tsv", std::ios::binary);
    if (!manifest) {
        throw std::runtime_error("cannot create title/boot asset manifest");
    }
    manifest << "asset\tx\ty\twidth\theight\n";

    const auto title_background =
        shadowrun::graphics::decode_genesis_title_background(rom);
    write_scene_asset(root, "title/background.bmp", title_background,
                      manifest, false);
    const auto title_lights =
        shadowrun::graphics::decode_genesis_title_circuit_lights(rom);
    remove_legacy_circuit_fragments(root);
    write_circuit_backgrounds(root, title_lights, manifest);
    write_scene_asset(root, "title/logo.bmp",
                      shadowrun::graphics::decode_genesis_title_logo_plane(rom),
                      manifest, true);
    write_scene_asset(root, "title/text.bmp",
                      shadowrun::graphics::decode_genesis_title_text(rom),
                      manifest, true);
    const auto title_sprites =
        shadowrun::graphics::decode_genesis_title_sprite_frames(rom);
    for (std::size_t index = 0; index < title_sprites.size(); ++index) {
        write_scene_asset(root,
                          "title/sprite-" + std::to_string(index) + ".bmp",
                          title_sprites[index], manifest, true);
    }

    write_scene_asset(root, "boot/snow.bmp",
                      shadowrun::graphics::decode_genesis_boot_static(rom),
                      manifest, false);
    write_scene_asset(root, "boot/sega-background.bmp",
                      shadowrun::graphics::decode_genesis_boot_background(rom),
                      manifest, true);
    write_scene_asset(root, "boot/guy.bmp",
                      shadowrun::graphics::decode_genesis_boot_guy(rom),
                      manifest, true);
    const auto sega = shadowrun::graphics::decode_genesis_boot_sega_frames(rom);
    for (std::size_t index = 0; index < sega.size(); ++index) {
        write_scene_asset(root,
                          "boot/sega-lightning-" + std::to_string(index) +
                              ".bmp",
                          sega[index], manifest, true);
    }
    const auto strobe =
        shadowrun::graphics::decode_genesis_boot_logo_strobe(rom);
    for (std::size_t index = 0; index < strobe.size(); ++index) {
        write_scene_asset(root,
                          "boot/sega-strobe-" + std::to_string(index) +
                              ".bmp",
                          strobe[index], manifest, true);
    }
    write_scene_asset(root, "boot/fasa.bmp",
                      shadowrun::graphics::decode_genesis_boot_fasa(rom),
                      manifest, true);
}

shadowrun::graphics::RgbaImage to_rgba_image(
    const shadowrun::native::NativeImage& source) {
    return {source.width, source.height, source.pixels};
}

struct MatrixGraphicsUploadHistory {
    std::array<std::uint8_t, 0x800U> overlap{};
};

std::vector<std::uint8_t> decode_matrix_graphics_upload(
    const std::vector<std::uint8_t>& rom, const std::size_t source_offset,
    MatrixGraphicsUploadHistory& history) {
    // 0x177CC uploads D86E0 then D78AA through 0x1530 using the same
    // FFBF40 overlap buffer; 0x1826C uploads D9376 through that buffer next.
    // Keep this state only in the offline asset extractor. The runtime uses
    // the named images below and has no ROM resource decoder.
    if (source_offset > rom.size() || rom.size() - source_offset < 2U) {
        throw std::runtime_error(
            "Matrix graphics upload header is truncated");
    }
    const auto output_count = static_cast<std::size_t>(
        read_big_endian_word(rom, source_offset)) + 1U;
    if (output_count > 0x10000U) {
        throw std::runtime_error("Matrix graphics upload size is invalid");
    }
    std::size_t source = source_offset + 2U;
    std::size_t ring_index = 0U;
    std::size_t remaining = output_count;
    std::uint8_t control = 0U;
    unsigned control_bits = 0U;
    std::vector<std::uint8_t> output;
    output.reserve(output_count);

    const auto read_byte = [&]() {
        if (source >= rom.size()) {
            throw std::runtime_error(
                "Matrix graphics upload token is truncated");
        }
        return rom[source++];
    };
    const auto emit = [&](const std::uint8_t value) {
        history.overlap[ring_index] = value;
        ring_index = (ring_index + 1U) & 0x07FFU;
        output.push_back(value);
        --remaining;
    };

    while (remaining > 0U) {
        if (control_bits == 0U) {
            control = read_byte();
            control_bits = 8U;
        }
        const bool compressed = (control & 0x80U) != 0U;
        control = static_cast<std::uint8_t>(control << 1U);
        --control_bits;
        if (!compressed) {
            emit(read_byte());
            continue;
        }

        const auto descriptor = read_byte();
        const auto run_count = (descriptor & 0x80U) == 0U
            ? static_cast<std::size_t>(descriptor & 0x7FU) + 3U
            : static_cast<std::size_t>(descriptor & 0x0FU) + 3U;
        if (run_count > remaining) {
            throw std::runtime_error(
                "Matrix graphics upload run exceeds its declared output");
        }
        if ((descriptor & 0x80U) == 0U) {
            const auto value = read_byte();
            for (std::size_t index = 0U; index < run_count; ++index) {
                emit(value);
            }
            continue;
        }

        const auto offset_low = read_byte();
        const auto offset = static_cast<std::size_t>(
            ((static_cast<std::size_t>(offset_low) << 3U) |
             ((descriptor >> 4U) & 0x07U)) & 0x07FFU);
        auto copy_index = (ring_index - offset) & 0x07FFU;
        for (std::size_t index = 0U; index < run_count; ++index) {
            const auto value = history.overlap[copy_index];
            copy_index = (copy_index + 1U) & 0x07FFU;
            emit(value);
        }
    }
    return output;
}

shadowrun::graphics::RgbaImage compose_matrix_map_backdrop(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& patterns,
    const std::size_t pattern_base,
    const std::size_t pattern_count) {
    constexpr std::size_t kColumns = 40U;
    constexpr std::size_t kRows = 28U;
    constexpr std::size_t kTable = 0x15C4DCU;
    constexpr std::size_t kAtlasColumns = 16U;
    if (patterns.width != kAtlasColumns * 8U ||
        pattern_base > 0x7FFU || pattern_count == 0U) {
        throw std::runtime_error("Matrix backdrop atlas has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kColumns * 8U),
        static_cast<std::uint16_t>(kRows * 8U),
        std::vector<shadowrun::native::Rgba8>(
            kColumns * kRows * 64U,
            shadowrun::native::Rgba8{0U, 0U, 0U, 255U}),
    };
    for (std::size_t cell = 0U; cell < kColumns * kRows; ++cell) {
        const auto word = read_big_endian_word(rom, kTable + cell * 2U);
        const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
        if (pattern < pattern_base || pattern - pattern_base >= pattern_count ||
            ((word >> 13U) & 0x03U) != 0U) {
            throw std::runtime_error(
                "Matrix backdrop name table references an unexpected pattern");
        }
        const auto pattern_index = pattern - pattern_base;
        const auto source_x = (pattern_index % kAtlasColumns) * 8U;
        const auto source_y = (pattern_index / kAtlasColumns) * 8U;
        const auto target_x = (cell % kColumns) * 8U;
        const auto target_y = (cell / kColumns) * 8U;
        const bool flip_x = (word & 0x0800U) != 0U;
        const bool flip_y = (word & 0x1000U) != 0U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto from_x = source_x + (flip_x ? 7U - x : x);
                const auto from_y = source_y + (flip_y ? 7U - y : y);
                const auto& source = patterns.pixels[
                    from_y * static_cast<std::size_t>(patterns.width) +
                    from_x];
                if (source.alpha == 0U) {
                    continue;
                }
                image.pixels[(target_y + y) * kColumns * 8U + target_x + x] =
                    source;
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_plane_b_background(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& matrix_ui_patterns,
    const shadowrun::graphics::RgbaImage& field_menu_patterns) {
    // Matrix entry 0x181F8 writes D9C96 to Plane B at $E000 (40x22,
    // palette line 1). Its live colors are installed by 0x17F80 -> 0x566E2,
    // which copies sixteen words from 0xE2EBA into FFD89E. D9C96 emits the
    // full 40x22 Plane-B panel. The subsequent 0x17708 HUD pass writes D8316
    // at $EB00, row 22 (40x6, palette line 3); both streams fill every
    // visible name-table cell in their respective areas.
    // Both resource banks are loaded before the menu: D9376 at $8160 and
    // D78AA at $0580. This is a one-time host image composition.
    constexpr std::size_t kWidth = 320U;
    constexpr std::size_t kHeight = 224U;
    constexpr std::size_t kPlaneColumns = 40U;
    constexpr std::size_t kPlaneRows = 28U;
    constexpr std::size_t kMainPatternBase = 0x040BU;
    constexpr std::size_t kMainPatternCount = 199U;
    constexpr std::size_t kFooterPatternBase = 0x002CU;
    constexpr std::size_t kFooterPatternCount = 147U;
    constexpr std::size_t kFooterFirstRow = 22U;
    constexpr std::size_t kMainExpectedWords = 880U;
    constexpr std::size_t kFooterExpectedWords = 240U;
    if (matrix_ui_patterns.width != 128U ||
        matrix_ui_patterns.height != 104U ||
        field_menu_patterns.width != 128U ||
        field_menu_patterns.height != 80U) {
        throw std::runtime_error(
            "Matrix Cyberdeck Plane-B pattern atlases have invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kWidth),
        static_cast<std::uint16_t>(kHeight),
        std::vector<shadowrun::native::Rgba8>(
            kWidth * kHeight, shadowrun::native::Rgba8{0U, 0U, 0U, 255U}),
    };
    const auto draw_stream = [&image](
        const std::vector<std::uint16_t>& words,
        const std::size_t first_row,
        const shadowrun::graphics::RgbaImage& patterns,
        const std::size_t pattern_base,
        const std::size_t pattern_count,
        const std::string_view label) {
        if (words.size() > kPlaneColumns * (kPlaneRows - first_row)) {
            throw std::runtime_error(std::string(label) +
                                     " exceeds the 40-column Plane-B area");
        }
        for (std::size_t index = 0U; index < words.size(); ++index) {
            const auto word = static_cast<std::uint16_t>(words[index] |
                (label == "D9C96" ? 0x2000U : 0x6000U));
            const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
            if (pattern < pattern_base ||
                pattern - pattern_base >= pattern_count) {
                throw std::runtime_error(std::string(label) +
                                         " references a missing source tile");
            }
            const auto pattern_index = pattern - pattern_base;
            const auto source_x = (pattern_index % 16U) * 8U;
            const auto source_y = (pattern_index / 16U) * 8U;
            const auto target_x = (index % kPlaneColumns) * 8U;
            const auto target_y = (first_row + index / kPlaneColumns) * 8U;
            const bool flip_x = (word & 0x0800U) != 0U;
            const bool flip_y = (word & 0x1000U) != 0U;
            for (std::size_t y = 0U; y < 8U; ++y) {
                for (std::size_t x = 0U; x < 8U; ++x) {
                    const auto from_x = source_x + (flip_x ? 7U - x : x);
                    const auto from_y = source_y + (flip_y ? 7U - y : y);
                    const auto& pixel = patterns.pixels[
                        from_y * static_cast<std::size_t>(patterns.width) +
                        from_x];
                    if (pixel.alpha == 0U) {
                        continue;
                    }
                    image.pixels[(target_y + y) * kWidth + target_x + x] =
                        pixel;
                }
            }
        }
    };

    const auto main_words = decode_hud_sprite_stream_for_asset(
        rom, 0xD9C96U, "Matrix Plane-B stream D9C96");
    const auto footer_words = decode_hud_sprite_stream_for_asset(
        rom, 0xD8316U, "Matrix HUD footer stream D8316");
    if (main_words.size() != kMainExpectedWords ||
        footer_words.size() != kFooterExpectedWords) {
        throw std::runtime_error(
            "Matrix Cyberdeck Plane-B stream cell counts changed");
    }
    draw_stream(main_words, 0U, matrix_ui_patterns, kMainPatternBase,
                kMainPatternCount, "D9C96");
    draw_stream(footer_words, kFooterFirstRow, field_menu_patterns,
                kFooterPatternBase, kFooterPatternCount, "D8316");
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_panel(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas) {
    // ROM 0x1897C repeats 19 words from 0xD8454 for 18 rows at $C126.
    // Plane A uses 64 cells per row, and $C126 wraps to VRAM byte address
    // 0x0126, placing the right-side panel at pixel (152,16). The grid writer
    // ORs 0x6000 into each word, so these patterns use CRAM line 3.
    // ROM 0x177CC uploads 0xD78AA to VRAM $0580 (pattern $02C). These are
    // the patterns named by 0xD8454; 0xD9376 is the separate Matrix icon set
    // uploaded at $8160. This is a one-time host composition; the runtime
    // consumes the resulting image directly.
    constexpr std::size_t kPatternBase = 0x002CU;
    constexpr std::size_t kPatternCount = 147U;
    constexpr std::size_t kRowTable = 0xD8454U;
    constexpr std::size_t kColumns = 19U;
    constexpr std::size_t kRows = 18U;
    constexpr std::size_t kOriginX = 152U;
    constexpr std::size_t kOriginY = 16U;
    if (pattern_atlas.width != 128U || pattern_atlas.height != 80U) {
        throw std::runtime_error("Matrix panel pattern atlas has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        320U, 224U,
        std::vector<shadowrun::native::Rgba8>(
            320U * 224U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t row = 0U; row < kRows; ++row) {
        for (std::size_t column = 0U; column < kColumns; ++column) {
            const auto word = read_big_endian_word(
                rom, kRowTable + column * 2U);
            const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
            if (pattern < kPatternBase ||
                pattern - kPatternBase >= kPatternCount) {
                throw std::runtime_error(
                    "Matrix panel row references an unimported pattern");
            }
            const bool flip_x = (word & 0x0800U) != 0U;
            const bool flip_y = (word & 0x1000U) != 0U;
            const auto pattern_index = pattern - kPatternBase;
            const auto source_x = (pattern_index % 16U) * 8U;
            const auto source_y = (pattern_index / 16U) * 8U;
            const auto target_x = kOriginX + column * 8U;
            const auto target_y = kOriginY + row * 8U;
            for (std::size_t y = 0U; y < 8U; ++y) {
                for (std::size_t x = 0U; x < 8U; ++x) {
                    const auto from_x = source_x + (flip_x ? 7U - x : x);
                    const auto from_y = source_y + (flip_y ? 7U - y : y);
                    const auto& source = pattern_atlas.pixels[
                        from_y * static_cast<std::size_t>(pattern_atlas.width) +
                        from_x];
                    if (source.alpha == 0U) {
                        continue;
                    }
                    image.pixels[(target_y + y) * 320U + target_x + x] =
                        source;
                }
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_runner_headers(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas) {
    // ROM 0x1A16A writes five source tiles from D9ED4 at $C4A6 and five from
    // D9EDE at $C526. These are Plane-A detail rows at (152,72) and (152,80).
    // Their 0x0439.. tile IDs refer to the Matrix UI bank from D9376, loaded
    // at $8160, and use CRAM line 1. Decode them once into a named host image.
    constexpr std::size_t kPatternBase = 0x040BU;
    constexpr std::size_t kPatternCount = 199U;
    constexpr std::array<std::size_t, 2> kRows{0xD9ED4U, 0xD9EDEU};
    constexpr std::array<std::size_t, 2> kOriginY{72U, 80U};
    constexpr std::size_t kColumns = 5U;
    if (pattern_atlas.width != 128U || pattern_atlas.height != 104U) {
        throw std::runtime_error(
            "Matrix runner-detail pattern atlas has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        320U, 224U,
        std::vector<shadowrun::native::Rgba8>(
            320U * 224U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t row = 0U; row < kRows.size(); ++row) {
        for (std::size_t column = 0U; column < kColumns; ++column) {
            const auto word = static_cast<std::uint16_t>(
                read_big_endian_word(rom, kRows[row] + column * 2U) |
                0x2000U);
            const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
            if (pattern < kPatternBase ||
                pattern - kPatternBase >= kPatternCount) {
                throw std::runtime_error(
                    "Matrix runner-detail header references an unavailable "
                    "pattern at row " + std::to_string(row) + ", column " +
                    std::to_string(column) + ": " +
                    std::to_string(pattern));
            }
            const auto pattern_index = pattern - kPatternBase;
            const auto source_x = (pattern_index % 16U) * 8U;
            const auto source_y = (pattern_index / 16U) * 8U;
            const auto target_x = 152U + column * 8U;
            const auto target_y = kOriginY[row];
            const bool flip_x = (word & 0x0800U) != 0U;
            const bool flip_y = (word & 0x1000U) != 0U;
            for (std::size_t y = 0U; y < 8U; ++y) {
                for (std::size_t x = 0U; x < 8U; ++x) {
                    const auto from_x = source_x + (flip_x ? 7U - x : x);
                    const auto from_y = source_y + (flip_y ? 7U - y : y);
                    const auto& pixel = pattern_atlas.pixels[
                        from_y * static_cast<std::size_t>(pattern_atlas.width) +
                        from_x];
                    if (pixel.alpha == 0U) continue;
                    image.pixels[(target_y + y) * 320U + target_x + x] =
                        pixel;
                }
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_detail_strip(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas,
    const std::size_t entry) {
    constexpr std::size_t kTable = 0xD9EE8U;
    constexpr std::size_t kEntryCount = 16U;
    constexpr std::size_t kEntryStride = 0x0CU;
    constexpr std::size_t kPatternBase = 0x040BU;
    constexpr std::size_t kPatternCount = 199U;
    if (entry >= kEntryCount || pattern_atlas.width != 128U ||
        pattern_atlas.height != 104U) {
        throw std::runtime_error("Matrix deck detail strip source is invalid");
    }
    shadowrun::graphics::RgbaImage image{
        48U, 8U,
        std::vector<shadowrun::native::Rgba8>(
            48U * 8U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t column = 0U; column < 6U; ++column) {
        const auto word = read_big_endian_word(
            rom, kTable + entry * kEntryStride + column * 2U);
        const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
        if (pattern < kPatternBase || pattern - kPatternBase >= kPatternCount) {
            throw std::runtime_error(
                "Matrix deck detail strip references an unavailable pattern");
        }
        const auto pattern_index = pattern - kPatternBase;
        const auto source_x = (pattern_index % 16U) * 8U;
        const auto source_y = (pattern_index / 16U) * 8U;
        const bool flip_x = (word & 0x0800U) != 0U;
        const bool flip_y = (word & 0x1000U) != 0U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto from_x = source_x + (flip_x ? 7U - x : x);
                const auto from_y = source_y + (flip_y ? 7U - y : y);
                const auto& pixel = pattern_atlas.pixels[
                    from_y * static_cast<std::size_t>(pattern_atlas.width) +
                    from_x];
                if (pixel.alpha == 0U) continue;
                image.pixels[y * 48U + column * 8U + x] = pixel;
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_cursor(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas) {
    constexpr std::size_t kSpriteRecord = 0xE31E8U;
    constexpr std::size_t kPatternBase = 0x040BU;
    constexpr std::size_t kPatternCount = 199U;
    if (pattern_atlas.width != 128U || pattern_atlas.height != 104U) {
        throw std::runtime_error("Matrix deck cursor atlas is invalid");
    }
    const auto word = read_big_endian_word(rom, kSpriteRecord + 6U);
    const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
    if (pattern < kPatternBase || pattern - kPatternBase >= kPatternCount) {
        throw std::runtime_error("Matrix deck cursor pattern is unavailable");
    }
    const auto pattern_index = pattern - kPatternBase;
    const auto source_x = (pattern_index % 16U) * 8U;
    const auto source_y = (pattern_index / 16U) * 8U;
    shadowrun::graphics::RgbaImage image{
        8U, 8U,
        std::vector<shadowrun::native::Rgba8>(
            8U * 8U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    const bool flip_x = (word & 0x0800U) != 0U;
    const bool flip_y = (word & 0x1000U) != 0U;
    for (std::size_t y = 0U; y < 8U; ++y) {
        for (std::size_t x = 0U; x < 8U; ++x) {
            const auto from_x = source_x + (flip_x ? 7U - x : x);
            const auto from_y = source_y + (flip_y ? 7U - y : y);
            image.pixels[y * 8U + x] = pattern_atlas.pixels[
                from_y * static_cast<std::size_t>(pattern_atlas.width) +
                from_x];
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_program_icon(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas,
    const std::size_t icon) {
    constexpr std::size_t kIconTable = 0xD92B6U;
    constexpr std::size_t kIconCount = 16U;
    constexpr std::size_t kPatternBase = 0x00BFU;
    constexpr std::size_t kPatternCount = 139U;
    constexpr std::size_t kIconStride = 0x0CU;
    if (icon >= kIconCount || pattern_atlas.width != 128U ||
        pattern_atlas.height != 72U) {
        throw std::runtime_error(
            "Matrix program icon source has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        24U, 16U,
        std::vector<shadowrun::native::Rgba8>(
            24U * 16U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t cell = 0U; cell < 6U; ++cell) {
        const auto word = read_big_endian_word(
            rom, kIconTable + icon * kIconStride + cell * 2U);
        const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
        if (pattern < kPatternBase ||
            pattern - kPatternBase >= kPatternCount) {
            throw std::runtime_error(
                "Matrix program icon references an unavailable pattern");
        }
        const auto pattern_index = pattern - kPatternBase;
        const auto source_x = (pattern_index % 16U) * 8U;
        const auto source_y = (pattern_index / 16U) * 8U;
        const auto target_x = (cell % 3U) * 8U;
        const auto target_y = (cell / 3U) * 8U;
        const bool flip_x = (word & 0x0800U) != 0U;
        const bool flip_y = (word & 0x1000U) != 0U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto from_x = source_x + (flip_x ? 7U - x : x);
                const auto from_y = source_y + (flip_y ? 7U - y : y);
                const auto& source = pattern_atlas.pixels[
                    from_y * static_cast<std::size_t>(pattern_atlas.width) +
                    from_x];
                if (source.alpha == 0U) continue;
                image.pixels[(target_y + y) * 24U + target_x + x] = source;
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_empty_program_icon(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas) {
    // 0x17708 selects the six-word empty-slot record at 0xD92AA whenever a
    // signed installed-program ID is negative. This shares the same uploaded
    // D86E0 tile bank and palette line as the installed-program records.
    constexpr std::size_t kEmptyIconTable = 0xD92AAU;
    constexpr std::size_t kPatternBase = 0x00BFU;
    constexpr std::size_t kPatternCount = 139U;
    if (pattern_atlas.width != 128U || pattern_atlas.height != 72U) {
        throw std::runtime_error(
            "Matrix empty program icon atlas has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        24U, 16U,
        std::vector<shadowrun::native::Rgba8>(
            24U * 16U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t cell = 0U; cell < 6U; ++cell) {
        const auto word = read_big_endian_word(
            rom, kEmptyIconTable + cell * 2U);
        const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
        if (pattern < kPatternBase ||
            pattern - kPatternBase >= kPatternCount) {
            throw std::runtime_error(
                "Matrix empty icon references an unavailable pattern");
        }
        const auto pattern_index = pattern - kPatternBase;
        const auto source_x = (pattern_index % 16U) * 8U;
        const auto source_y = (pattern_index / 16U) * 8U;
        const auto target_x = (cell % 3U) * 8U;
        const auto target_y = (cell / 3U) * 8U;
        const bool flip_x = (word & 0x0800U) != 0U;
        const bool flip_y = (word & 0x1000U) != 0U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto from_x = source_x + (flip_x ? 7U - x : x);
                const auto from_y = source_y + (flip_y ? 7U - y : y);
                const auto& source = pattern_atlas.pixels[
                    from_y * static_cast<std::size_t>(pattern_atlas.width) +
                    from_x];
                if (source.alpha == 0U) continue;
                image.pixels[(target_y + y) * 24U + target_x + x] = source;
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_footer_slot_status(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas,
    const std::size_t status) {
    // 0x19856 selects three tiles from D84D6 + status*6, then writes them
    // with palette line 3. D78AA is uploaded at tile $02C by 0x177CC.
    constexpr std::size_t kStatusTable = 0xD84D6U;
    constexpr std::size_t kStatusCount = 7U;
    constexpr std::size_t kPatternBase = 0x002CU;
    constexpr std::size_t kPatternCount = 147U;
    if (status >= kStatusCount || pattern_atlas.width != 128U ||
        pattern_atlas.height != 80U) {
        throw std::runtime_error(
            "Matrix footer status source has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        24U, 8U,
        std::vector<shadowrun::native::Rgba8>(
            24U * 8U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t cell = 0U; cell < 3U; ++cell) {
        const auto word = read_big_endian_word(
            rom, kStatusTable + status * 6U + cell * 2U);
        const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
        if (pattern < kPatternBase ||
            pattern - kPatternBase >= kPatternCount) {
            throw std::runtime_error(
                "Matrix footer status references an unavailable pattern");
        }
        const auto pattern_index = pattern - kPatternBase;
        const auto source_x = (pattern_index % 16U) * 8U;
        const auto source_y = (pattern_index / 16U) * 8U;
        const auto target_x = cell * 8U;
        const bool flip_x = (word & 0x0800U) != 0U;
        const bool flip_y = (word & 0x1000U) != 0U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto from_x = source_x + (flip_x ? 7U - x : x);
                const auto from_y = source_y + (flip_y ? 7U - y : y);
                const auto& source = pattern_atlas.pixels[
                    from_y * static_cast<std::size_t>(pattern_atlas.width) +
                    from_x];
                if (source.alpha == 0U) continue;
                image.pixels[y * 24U + target_x + x] = source;
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_cyberdeck_selector(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& pattern_atlas,
    const std::size_t selected_row) {
    constexpr std::size_t kSelectorTable = 0xD9FA8U;
    constexpr std::size_t kPatternBase = 0x040BU;
    constexpr std::size_t kPatternCount = 199U;
    constexpr std::size_t kRowStride = 0x24U;
    constexpr std::size_t kDestinationX = 40U;
    constexpr std::size_t kDestinationY = 8U;
    if (selected_row >= 4U || pattern_atlas.width != 128U ||
        pattern_atlas.height != 104U) {
        throw std::runtime_error(
            "Matrix selector source has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        320U, 224U,
        std::vector<shadowrun::native::Rgba8>(
            320U * 224U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    // 0x189D8 writes the previous tab's inactive nine-cell strip and the
    // current tab's active strip when selection moves. The static D9C96 map
    // already contains all four inactive labels; this host layer contains
    // only the active strip so it cannot repaint the other three tabs.
    const auto target_y = kDestinationY + selected_row * 40U;
    const auto source_row = kSelectorTable + selected_row * kRowStride + 9U * 2U;
    for (std::size_t column = 0U; column < 9U; ++column) {
        const auto word = read_big_endian_word(
            rom, source_row + column * 2U);
        const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
        if (pattern < kPatternBase ||
            pattern - kPatternBase >= kPatternCount) {
            throw std::runtime_error(
                "Matrix selector references an unavailable pattern");
        }
        const auto pattern_index = pattern - kPatternBase;
        const auto source_x = (pattern_index % 16U) * 8U;
        const auto source_y = (pattern_index / 16U) * 8U;
        const auto target_x = kDestinationX + column * 8U;
        const bool flip_x = (word & 0x0800U) != 0U;
        const bool flip_y = (word & 0x1000U) != 0U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto from_x = source_x + (flip_x ? 7U - x : x);
                const auto from_y = source_y + (flip_y ? 7U - y : y);
                const auto& source = pattern_atlas.pixels[
                    from_y * static_cast<std::size_t>(pattern_atlas.width) +
                    from_x];
                if (source.alpha == 0U) continue;
                image.pixels[(target_y + y) * 320U + target_x + x] =
                    source;
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_node_action_background(
    const std::vector<std::uint8_t>& rom,
    const std::vector<std::uint8_t>& matrix_field_resource,
    const std::size_t profile,
    const std::size_t theme) {
    // 0x1745E loads the field-mode pattern stream at 0xD646A into VRAM
    // $8160 (pattern $00B). 0x54662 selects one of two six-grid layouts at
    // 0x166330/0x166368, loads the shared terminal patterns from 0x15FF30 to
    // VRAM $1C80, then writes the centered terminal pieces from
    // 0x1604D0/0x160538/0x160558/0x160578.
    // The selected node color indexes the palette pointer table at 0x166448;
    // the grid writer ORs $6000 into every tile word, selecting CRAM line 3.
    constexpr std::size_t kMatrixFieldPatternBase = 0x000BU;
    constexpr std::size_t kMatrixFieldPatternCount = 304U;
    constexpr std::size_t kMatrixScenePatternBase = 0x00E4U;
    constexpr std::size_t kMatrixScenePatternCount = 45U;
    constexpr std::size_t kNodeProfileTable = 0x166330U;
    constexpr std::size_t kNodeProfileStride = 0x38U;
    constexpr std::size_t kThemePointerTable = 0x166448U;
    constexpr std::size_t kScenePatternSource = 0x15FF30U;
    constexpr std::size_t kScenePatternBytes = 0x5A0U;
    constexpr std::array<std::array<std::size_t, 4>, 4> kTerminalGrids{{
        {{0x1604D0U, 0xC20EU, 26U, 2U}},
        {{0x160538U, 0xC310U, 1U, 16U}},
        {{0x160558U, 0xC33EU, 1U, 16U}},
        {{0x160578U, 0xCB10U, 24U, 2U}},
    }};
    if (profile > 4U || theme > 3U ||
        matrix_field_resource.size() != kMatrixFieldPatternCount * 32U) {
        throw std::runtime_error("Matrix node layout selection is invalid");
    }

    const auto palette_source = read_big_endian_long(
        rom, kThemePointerTable + theme * 4U);
    const auto palette =
        shadowrun::graphics::read_genesis_palette(rom, palette_source);
    const auto field_patterns = shadowrun::graphics::decode_tile_atlas(
        matrix_field_resource, 0U, kMatrixFieldPatternCount, palette, 16U, 0U);
    const auto scene_patterns = shadowrun::graphics::decode_tile_atlas(
        rom, kScenePatternSource, kMatrixScenePatternCount, palette, 16U, 0U);
    if (scene_patterns.width != 128U ||
        scene_patterns.height != 24U) {
        throw std::runtime_error("Matrix terminal pattern block changed size");
    }
    if (kScenePatternBytes != kMatrixScenePatternCount * 32U) {
        throw std::runtime_error("Matrix terminal source size is inconsistent");
    }

    const auto profile_address =
        kNodeProfileTable + profile * kNodeProfileStride;
    const auto extra_pattern_source = static_cast<std::size_t>(
        read_big_endian_long(rom, profile_address));
    const auto extra_pattern_bytes = static_cast<std::size_t>(
        read_big_endian_long(rom, profile_address + 4U));
    shadowrun::graphics::RgbaImage extra_patterns{};
    if (extra_pattern_bytes != 0U) {
        if (extra_pattern_bytes % 32U != 0U) {
            throw std::runtime_error(
                "Matrix node extra pattern block is not tile-aligned");
        }
        extra_patterns = shadowrun::graphics::decode_tile_atlas(
            rom, extra_pattern_source, extra_pattern_bytes / 32U, palette,
            16U, 0U);
    }

    shadowrun::graphics::RgbaImage image{
        320U, 224U,
        std::vector<shadowrun::native::Rgba8>(
            320U * 224U,
            shadowrun::native::Rgba8{0U, 0U, 0U, 255U}),
    };
    const auto draw_word = [&](const std::size_t cell_index,
                               const std::uint16_t raw_word) {
        const auto word = static_cast<std::uint16_t>(raw_word | 0x6000U);
        const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
        const shadowrun::graphics::RgbaImage* atlas = nullptr;
        std::size_t atlas_pattern = 0U;
        if (pattern >= kMatrixScenePatternBase &&
            pattern - kMatrixScenePatternBase <
                kMatrixScenePatternCount) {
            atlas = &scene_patterns;
            atlas_pattern = pattern - kMatrixScenePatternBase;
        } else if (!extra_patterns.pixels.empty() &&
                   pattern >= 0x0111U &&
                   pattern - 0x0111U < extra_pattern_bytes / 32U) {
            atlas = &extra_patterns;
            atlas_pattern = pattern - 0x0111U;
        } else if (pattern >= kMatrixFieldPatternBase &&
                   pattern - kMatrixFieldPatternBase <
                       kMatrixFieldPatternCount) {
            atlas = &field_patterns;
            atlas_pattern = pattern - kMatrixFieldPatternBase;
        } else if (pattern <= 0x000AU) {
            // Low reserved and transparent patterns are left as the black
            // background established when the source planes are cleared.
            return;
        } else {
            throw std::runtime_error(
                "Matrix node name table references an unavailable pattern");
        }

        const auto cell_x = cell_index % 64U;
        const auto cell_y = cell_index / 64U;
        if (cell_x >= 40U || cell_y >= 28U) return;
        const auto target_x = cell_x * 8U;
        const auto target_y = cell_y * 8U;
        const auto source_x = (atlas_pattern % 16U) * 8U;
        const auto source_y = (atlas_pattern / 16U) * 8U;
        const bool flip_x = (word & 0x0800U) != 0U;
        const bool flip_y = (word & 0x1000U) != 0U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto from_x = source_x + (flip_x ? 7U - x : x);
                const auto from_y = source_y + (flip_y ? 7U - y : y);
                const auto& source = atlas->pixels[
                    from_y * static_cast<std::size_t>(atlas->width) +
                    from_x];
                if (source.alpha == 0U) continue;
                image.pixels[(target_y + y) * 320U + target_x + x] = source;
            }
        }
    };
    const auto draw_grid = [&rom, &draw_word](const std::size_t source,
                                              const std::size_t destination,
                                              const std::size_t columns,
                                              const std::size_t rows) {
        // The profile tables target Plane B at $E000, while the terminal
        // tables target Plane A at $C000. Both are screen-local name-table
        // positions after subtracting their respective VDP plane bases.
        const auto plane_base = destination >= 0xE000U
                                    ? 0xE000U
                                    : 0xC000U;
        for (std::size_t row = 0U; row < rows; ++row) {
            const auto row_address =
                (destination - plane_base + row * 0x80U) & 0x3FFFU;
            for (std::size_t column = 0U; column < columns; ++column) {
                const auto source_word =
                    source + (row * columns + column) * 2U;
                const auto word = read_big_endian_word(rom, source_word);
                const auto address =
                    (row_address + column * 2U) & 0x3FFFU;
                draw_word(address / 2U, word);
            }
        }
    };

    for (std::size_t grid = 0U; grid < 6U; ++grid) {
        const auto record = profile_address + 8U + grid * 8U;
        const auto source = static_cast<std::size_t>(
            read_big_endian_long(rom, record));
        const auto destination = static_cast<std::size_t>(
            read_big_endian_word(rom, record + 4U));
        const auto columns = static_cast<std::size_t>(rom[record + 6U]) + 1U;
        const auto rows = static_cast<std::size_t>(rom[record + 7U]) + 1U;
        draw_grid(source, destination, columns, rows);
    }

    for (const auto& grid : kTerminalGrids) {
        draw_grid(grid[0], grid[1], grid[2], grid[3]);
    }
    return image;
}

template <typename Palette>
shadowrun::graphics::RgbaImage compose_matrix_node_action_cursor(
    const std::vector<std::uint8_t>& title_font_resource,
    const Palette& palette) {
    constexpr std::size_t kPatternBase = 0x05ABU;
    constexpr std::size_t kPatternCount = 0x55U;
    constexpr std::uint16_t kCursorWord = 0x85FFU;
    constexpr std::size_t kCursorPattern = kCursorWord & 0x07FFU;
    if (title_font_resource.size() != kPatternCount * 32U ||
        kCursorPattern < kPatternBase ||
        kCursorPattern - kPatternBase >= kPatternCount) {
        throw std::runtime_error(
            "Matrix node-action cursor source tile is unavailable");
    }

    // 0xC0C2 uploads 85 title-font patterns from 0xC1064 at VRAM $B560
    // (pattern $5AB). 0xC376 writes $85FF for the selected row, selecting
    // the final pattern ($5FF) with palette line 0. The security-color
    // setup at 0x566D8 supplies that line from the corresponding 0xE2D52
    // palette record. This is an offline tile extraction; Qt receives only
    // the named 8x8 image.
    const auto patterns = shadowrun::graphics::decode_tile_atlas(
        title_font_resource, 0U, kPatternCount, palette, 16U, 0U);
    if (patterns.width != 128U || patterns.height != 48U) {
        throw std::runtime_error(
            "Matrix node-action cursor atlas has invalid dimensions");
    }
    const auto pattern_index = kCursorPattern - kPatternBase;
    const auto source_x = (pattern_index % 16U) * 8U;
    const auto source_y = (pattern_index / 16U) * 8U;
    shadowrun::graphics::RgbaImage cursor{
        8U, 8U,
        std::vector<shadowrun::native::Rgba8>(
            64U, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t y = 0U; y < 8U; ++y) {
        for (std::size_t x = 0U; x < 8U; ++x) {
            cursor.pixels[y * 8U + x] = patterns.pixels[
                (source_y + y) * patterns.width + source_x + x];
        }
    }
    return cursor;
}

shadowrun::graphics::RgbaImage compose_matrix_traversal_background(
    const std::vector<std::uint8_t>& rom,
    const std::vector<std::uint8_t>& matrix_field_resource,
    const shadowrun::graphics::RgbaImage& matrix_menu_patterns,
    const std::size_t theme) {
    // 0x1745E uploads the Matrix menu patterns from 0xD78AA at $0580 and
    // the 304 field patterns from 0xD646A at $8160, then draws the world view
    // with the 0x1728 streams at 0xD806C/$E000 (40x13) and 0xD74F4/$E680
    // (40x9). 0x17708 draws the shared six-row footer from 0xD8316 at $EB00.
    // D806C and D8316 use the menu patterns at $02C with palette 0xE2E9A;
    // D74F4 uses the field patterns at $040B and the selected palette pointer
    // from 0x166448. This is an offline composition; the Qt runtime loads the
    // resulting image and never decodes the source streams.
    constexpr std::size_t kWidth = 320U;
    constexpr std::size_t kHeight = 224U;
    constexpr std::size_t kColumns = 40U;
    constexpr std::size_t kRows = 28U;
    constexpr std::size_t kFieldPatternBase = 0x040BU;
    constexpr std::size_t kFieldPatternCount = 304U;
    constexpr std::size_t kMenuPatternBase = 0x002CU;
    constexpr std::size_t kMenuPatternCount = 147U;
    constexpr std::size_t kPalettePointerTable = 0x166448U;
    constexpr std::array<std::size_t, 3> kStreamSources{
        0xD806CU, 0xD74F4U, 0xD8316U};
    constexpr std::array<std::size_t, 3> kStreamFirstRows{0U, 13U, 22U};
    constexpr std::array<std::size_t, 3> kExpectedWordCounts{
        520U, 360U, 240U};
    if (theme > 3U ||
        matrix_field_resource.size() != kFieldPatternCount * 32U ||
        matrix_menu_patterns.width != 128U ||
        matrix_menu_patterns.height != 80U) {
        throw std::runtime_error(
            "Matrix traversal background selection is invalid");
    }

    const auto palette_source = static_cast<std::size_t>(
        read_big_endian_long(rom, kPalettePointerTable + theme * 4U));
    const auto field_patterns = shadowrun::graphics::decode_tile_atlas(
        matrix_field_resource, 0U, kFieldPatternCount,
        shadowrun::graphics::read_genesis_palette(rom, palette_source),
        16U, 0U);
    if (field_patterns.width != 128U || field_patterns.height != 152U) {
        throw std::runtime_error(
            "Matrix traversal pattern atlas has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kWidth),
        static_cast<std::uint16_t>(kHeight),
        std::vector<shadowrun::native::Rgba8>(
            kWidth * kHeight,
            shadowrun::native::Rgba8{0U, 0U, 0U, 255U}),
    };
    constexpr std::array<std::size_t, 3> kPatternBases{
        kMenuPatternBase, kFieldPatternBase, kMenuPatternBase};
    constexpr std::array<std::size_t, 3> kPatternCounts{
        kMenuPatternCount, kFieldPatternCount, kMenuPatternCount};
    const std::array<const shadowrun::graphics::RgbaImage*, 3> atlases{
        &matrix_menu_patterns, &field_patterns, &matrix_menu_patterns};
    for (std::size_t stream = 0U; stream < kStreamSources.size(); ++stream) {
        const auto words = decode_hud_sprite_stream_for_asset(
            rom, kStreamSources[stream], "Matrix traversal background stream");
        if (words.size() != kExpectedWordCounts[stream] ||
            words.size() > kColumns * (kRows - kStreamFirstRows[stream])) {
            throw std::runtime_error(
                "Matrix traversal background stream cell count changed");
        }
        for (std::size_t index = 0U; index < words.size(); ++index) {
            const auto word = static_cast<std::uint16_t>(words[index] | 0x6000U);
            const auto pattern = static_cast<std::size_t>(word & 0x07FFU);
            if (pattern < kPatternBases[stream] ||
                pattern - kPatternBases[stream] >= kPatternCounts[stream]) {
                throw std::runtime_error(
                    "Matrix traversal stream references a missing tile");
            }
            const auto pattern_index = pattern - kPatternBases[stream];
            const auto source_x = (pattern_index % 16U) * 8U;
            const auto source_y = (pattern_index / 16U) * 8U;
            const auto target_x = (index % kColumns) * 8U;
            const auto target_y =
                (kStreamFirstRows[stream] + index / kColumns) * 8U;
            const bool flip_x = (word & 0x0800U) != 0U;
            const bool flip_y = (word & 0x1000U) != 0U;
            for (std::size_t y = 0U; y < 8U; ++y) {
                for (std::size_t x = 0U; x < 8U; ++x) {
                    const auto from_x = source_x + (flip_x ? 7U - x : x);
                    const auto from_y = source_y + (flip_y ? 7U - y : y);
                    const auto& source = atlases[stream]->pixels[
                        from_y * static_cast<std::size_t>(atlases[stream]->width) +
                        from_x];
                    if (source.alpha == 0U) continue;
                    image.pixels[(target_y + y) * kWidth + target_x + x] =
                        source;
                }
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_traversal_hud_frame(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& matrix_menu_patterns) {
    // 0x1789C calls 0x178C8 twice. Each batch writes five small Plane-A grids
    // from 0xD86C8; the second pass is displaced by 0x28 bytes to complete the
    // two top status frames. The ROM's grid writer adds 0x80 bytes per row.
    constexpr std::size_t kScreenWidth = 320U;
    constexpr std::size_t kScreenHeight = 224U;
    constexpr std::size_t kPatternBase = 0x002CU;
    constexpr std::size_t kPatternCount = 147U;
    constexpr std::size_t kSource = 0xD86C8U;
    struct HudGrid {
        std::size_t source_word_offset;
        std::uint16_t destination;
        std::size_t columns;
        std::size_t rows;
    };
    constexpr std::array<HudGrid, 5> kGrids{{
        {0U, 0xC004U, 1U, 1U},
        {1U, 0xC022U, 1U, 1U},
        {2U, 0xC204U, 2U, 2U},
        {6U, 0xC220U, 2U, 2U},
        {10U, 0xC292U, 2U, 1U},
    }};
    if (matrix_menu_patterns.width != 128U ||
        matrix_menu_patterns.height != 80U) {
        throw std::runtime_error(
            "Matrix traversal HUD pattern atlas has invalid dimensions");
    }

    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kScreenWidth),
        static_cast<std::uint16_t>(kScreenHeight),
        std::vector<shadowrun::native::Rgba8>(
            kScreenWidth * kScreenHeight,
            shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t batch = 0U; batch < 2U; ++batch) {
        const auto batch_offset = batch == 0U ? 0U : 0x28U;
        for (const auto& grid : kGrids) {
            const auto destination = static_cast<std::size_t>(
                grid.destination) + batch_offset;
            const auto plane_offset = destination - 0xC000U;
            for (std::size_t row = 0U; row < grid.rows; ++row) {
                for (std::size_t column = 0U; column < grid.columns;
                     ++column) {
                    const auto source_offset = kSource +
                        (grid.source_word_offset + row * grid.columns +
                         column) * 2U;
                    const auto word = static_cast<std::uint16_t>(
                        read_big_endian_word(rom, source_offset) | 0x6000U);
                    const auto pattern = static_cast<std::size_t>(
                        word & 0x07FFU);
                    if (pattern < kPatternBase ||
                        pattern - kPatternBase >= kPatternCount) {
                        throw std::runtime_error(
                            "Matrix traversal HUD frame references a missing tile");
                    }
                    const auto pattern_index = pattern - kPatternBase;
                    const auto source_x = (pattern_index % 16U) * 8U;
                    const auto source_y = (pattern_index / 16U) * 8U;
                    const auto name_cell =
                        (plane_offset + row * 0x80U + column * 2U) / 2U;
                    const auto cell_x = name_cell % 64U;
                    const auto cell_y = name_cell / 64U;
                    if (cell_x >= kScreenWidth / 8U ||
                        cell_y >= kScreenHeight / 8U) {
                        continue;
                    }
                    const auto target_x = cell_x * 8U;
                    const auto target_y = cell_y * 8U;
                    const bool flip_x = (word & 0x0800U) != 0U;
                    const bool flip_y = (word & 0x1000U) != 0U;
                    for (std::size_t y = 0U; y < 8U; ++y) {
                        for (std::size_t x = 0U; x < 8U; ++x) {
                            const auto from_x = source_x +
                                (flip_x ? 7U - x : x);
                            const auto from_y = source_y +
                                (flip_y ? 7U - y : y);
                            const auto& pixel = matrix_menu_patterns.pixels[
                                from_y * matrix_menu_patterns.width + from_x];
                            if (pixel.alpha == 0U) continue;
                            image.pixels[(target_y + y) * kScreenWidth +
                                         target_x + x] = pixel;
                        }
                    }
                }
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_traversal_runner_pose(
    const std::vector<std::uint8_t>& rom,
    const std::size_t primary_table,
    const std::size_t primary_phase,
    const std::size_t secondary_table,
    const std::size_t secondary_phase,
    const char* pose_name) {
    // 0x1C506 draws the primary list before the secondary list. Behavior E
    // supplies the steady traversal pose; behavior A's final phase supplies
    // the raised-arm action pose. Both use the fixed 0xDA038 bank at tile
    // 0x11B, and the action pose also references the 0xDAAB8 bank at tile
    // 0x16F. D1=FF158E=0x120 is SAT X and D2=FF1590=0x12F is SAT Y; 0x1378
    // adds the first/second list words to X/Y respectively.
    constexpr std::size_t kWidth = 320U;
    constexpr std::size_t kHeight = 224U;
    constexpr std::size_t kFixedPatternSource = 0xDA038U;
    constexpr std::size_t kFixedPatternBytes = 0x0A80U;
    constexpr std::size_t kFixedPatternFirstTile = 0x011BU;
    constexpr std::size_t kActionPatternFirstTile = 0x016FU;
    constexpr std::size_t kTilesPerRow = 16U;
    constexpr int kBaseX = 0x0120 - 128;
    constexpr int kBaseY = 0x012F - 128;
    if (kFixedPatternSource > rom.size() ||
        kFixedPatternBytes > rom.size() - kFixedPatternSource ||
        kFixedPatternBytes % 32U != 0U) {
        throw std::runtime_error(
            "Matrix traversal fixed runner pattern bank is truncated");
    }
    const std::vector<std::uint8_t> fixed_patterns(
        rom.begin() + static_cast<std::ptrdiff_t>(kFixedPatternSource),
        rom.begin() + static_cast<std::ptrdiff_t>(
            kFixedPatternSource + kFixedPatternBytes));
    const auto action_patterns =
        shadowrun::graphics::decode_genesis_resource_stream(rom, 0xDAAB8U);
    if (action_patterns.empty() || action_patterns.size() % 32U != 0U) {
        throw std::runtime_error(
            "Matrix traversal action runner pattern bank is invalid");
    }
    const auto palette = shadowrun::graphics::read_genesis_palette(
        rom, 0x0E2E9AU);
    const auto fixed_atlas = shadowrun::graphics::decode_tile_atlas(
        fixed_patterns, 0U, kFixedPatternBytes / 32U, palette,
        kTilesPerRow, 0U);
    const auto action_atlas = shadowrun::graphics::decode_tile_atlas(
        action_patterns, 0U, action_patterns.size() / 32U, palette,
        kTilesPerRow, 0U);
    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kWidth),
        static_cast<std::uint16_t>(kHeight),
        std::vector<shadowrun::native::Rgba8>(
            kWidth * kHeight,
            shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    const auto get_list = [&](const std::size_t table,
                              const std::size_t phase) {
        const auto maximum_phase = static_cast<std::size_t>(
            read_big_endian_word(rom, table));
        if (phase > maximum_phase) {
            throw std::runtime_error(
                std::string("Matrix traversal ") + pose_name +
                " phase exceeds its source list table");
        }
        return static_cast<std::size_t>(read_big_endian_long(
            rom, table + 2U + phase * 4U));
    };
    const auto draw_list = [&](std::size_t record) {
        for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
            if (record > rom.size() || rom.size() - record < 8U) {
                throw std::runtime_error(
                    std::string("Matrix traversal ") + pose_name +
                    " list is truncated");
            }
            const auto x_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record));
            if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                return;
            }
            const auto y_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record + 2U));
            const auto size_word = read_big_endian_word(rom, record + 4U);
            const auto attributes = read_big_endian_word(rom, record + 6U);
            if ((size_word & 0xFF00U) != 0U ||
                (attributes & 0xF000U) != 0x8000U) {
                throw std::runtime_error(
                    std::string("Matrix traversal ") + pose_name +
                    " record has invalid attributes");
            }
            const auto first_tile = static_cast<std::size_t>(
                attributes & 0x07FFU);
            const bool uses_action_bank =
                first_tile >= kActionPatternFirstTile;
            const auto pattern_first_tile = uses_action_bank
                ? kActionPatternFirstTile : kFixedPatternFirstTile;
            const auto pattern_tile_count = uses_action_bank
                ? action_patterns.size() / 32U
                : kFixedPatternBytes / 32U;
            const auto& atlas = uses_action_bank
                ? action_atlas : fixed_atlas;
            if (first_tile < pattern_first_tile) {
                throw std::runtime_error(
                    std::string("Matrix traversal ") + pose_name +
                    " record precedes its pattern bank");
            }
            const auto width_tiles = static_cast<std::size_t>(
                ((size_word >> 2U) & 0x03U) + 1U);
            const auto height_tiles = static_cast<std::size_t>(
                (size_word & 0x03U) + 1U);
            const auto tile_offset = first_tile - pattern_first_tile;
            if (tile_offset > pattern_tile_count ||
                width_tiles * height_tiles >
                    pattern_tile_count - tile_offset) {
                throw std::runtime_error(
                    std::string("Matrix traversal ") + pose_name +
                    " sprite exceeds its pattern bank");
            }
            const auto sprite_width = width_tiles * 8U;
            const auto sprite_height = height_tiles * 8U;
            const bool flip_x = (attributes & 0x0800U) != 0U;
            const bool flip_y = (attributes & 0x1000U) != 0U;
            for (std::size_t y = 0U; y < sprite_height; ++y) {
                for (std::size_t x = 0U; x < sprite_width; ++x) {
                    const auto source_x = flip_x ? sprite_width - 1U - x : x;
                    const auto source_y = flip_y ? sprite_height - 1U - y : y;
                    const auto tile = tile_offset +
                        (source_x / 8U) * height_tiles + source_y / 8U;
                    const auto atlas_x = (tile % kTilesPerRow) * 8U +
                                         source_x % 8U;
                    const auto atlas_y = (tile / kTilesPerRow) * 8U +
                                         source_y % 8U;
                    const auto pixel = atlas.pixels[
                        atlas_y * atlas.width + atlas_x];
                    const auto target_x = kBaseX + x_offset +
                                          static_cast<int>(x);
                    const auto target_y = kBaseY + y_offset +
                                          static_cast<int>(y);
                    if (pixel.alpha == 0U || target_x < 0 || target_y < 0 ||
                        target_x >= static_cast<int>(kWidth) ||
                        target_y >= static_cast<int>(kHeight)) {
                        continue;
                    }
                    image.pixels[static_cast<std::size_t>(target_y) * kWidth +
                                 static_cast<std::size_t>(target_x)] = pixel;
                }
            }
            record += 8U;
        }
        throw std::runtime_error(
            std::string("Matrix traversal ") + pose_name +
            " list has no terminator");
    };
    // 0x1C506 draws A2 (primary) before A4 (secondary).
    draw_list(get_list(primary_table, primary_phase));
    draw_list(get_list(secondary_table, secondary_phase));
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_traversal_idle_runner(
    const std::vector<std::uint8_t>& rom) {
    // 0x1ADE6 attaches descriptor E, whose tables are phase-zero only.
    return compose_matrix_traversal_runner_pose(
        rom, 0x1C34AU, 0U, 0x1C344U, 0U, "descriptor-E idle runner");
}

shadowrun::graphics::RgbaImage compose_matrix_traversal_world_sequence_runner(
    const std::vector<std::uint8_t>& rom) {
    // 0x1D280 starts the FFDF8C entry sequence with phase 0 / countdown 4.
    // 0x1B364 advances the four independent figure groups once per sequence
    // frame. On the 239-VBlank compact path, the sequence-completion branch
    // stops the final figure pass at phase 4. This is the wide runner retained
    // by the first Matrix traversal frame, not descriptor E's arms-down pose.
    struct FigureGroup {
        std::size_t pointer_table;
        std::size_t destination;
        std::uint16_t base_x;
        std::uint16_t base_y;
    };
    constexpr std::size_t kPhase = 4U;
    constexpr std::size_t kScreenWidth = 320U;
    constexpr std::size_t kScreenHeight = 224U;
    constexpr std::array<FigureGroup, 4> kGroups{{
        {0x1DAF4U, 0xF000U, 0x00F8U, 0x00E4U},
        {0x1DB08U, 0xF000U, 0x0148U, 0x00E4U},
        {0x0EA490U, 0xF800U, 0x00E2U, 0x0114U},
        {0x0EA4A4U, 0xF800U, 0x015EU, 0x0114U},
    }};
    auto figure = shadowrun::graphics::RgbaImage{
        static_cast<std::uint16_t>(kScreenWidth),
        static_cast<std::uint16_t>(kScreenHeight),
        std::vector<shadowrun::native::Rgba8>(
            kScreenWidth * kScreenHeight,
            shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    const auto palette = shadowrun::graphics::read_genesis_palette(
        rom, 0x0E2E9AU);
    const auto draw_sprite = [&] (
        shadowrun::graphics::RgbaImage& target,
        const shadowrun::graphics::RgbaImage& atlas,
        const std::size_t atlas_tile_count,
        const std::size_t atlas_tiles_per_row,
        const std::size_t sprite_tile_first,
        const std::size_t sprite_tile_bank_first,
        const std::uint8_t size,
        const std::uint16_t attributes,
        const int origin_x,
        const int origin_y) {
        if (sprite_tile_first < sprite_tile_bank_first ||
            atlas_tiles_per_row == 0U) {
            throw std::runtime_error(
                "Matrix entry runner sprite starts outside its tile bank");
        }
        const auto width_tiles = static_cast<std::size_t>(
            ((size >> 2U) & 0x03U) + 1U);
        const auto height_tiles = static_cast<std::size_t>(
            (size & 0x03U) + 1U);
        const auto tile_offset = sprite_tile_first - sprite_tile_bank_first;
        const auto sprite_tile_count = width_tiles * height_tiles;
        if (tile_offset > atlas_tile_count ||
            sprite_tile_count > atlas_tile_count - tile_offset) {
            throw std::runtime_error(
                "Matrix entry runner sprite exceeds its decoded tile bank");
        }
        const bool flip_x = (attributes & 0x0800U) != 0U;
        const bool flip_y = (attributes & 0x1000U) != 0U;
        const auto width = width_tiles * 8U;
        const auto height = height_tiles * 8U;
        for (std::size_t y = 0U; y < height; ++y) {
            for (std::size_t x = 0U; x < width; ++x) {
                const auto source_x = flip_x ? width - x - 1U : x;
                const auto source_y = flip_y ? height - y - 1U : y;
                const auto tile_index = tile_offset +
                    (source_x / 8U) * height_tiles + source_y / 8U;
                const auto atlas_x =
                    (tile_index % atlas_tiles_per_row) * 8U + source_x % 8U;
                const auto atlas_y =
                    (tile_index / atlas_tiles_per_row) * 8U + source_y % 8U;
                const auto source_pixel = atlas.pixels[
                    atlas_y * atlas.width + atlas_x];
                if (source_pixel.alpha == 0U) {
                    continue;
                }
                const auto target_x = origin_x + static_cast<int>(x);
                const auto target_y = origin_y + static_cast<int>(y);
                if (target_x < 0 || target_y < 0 ||
                    target_x >= static_cast<int>(kScreenWidth) ||
                    target_y >= static_cast<int>(kScreenHeight)) {
                    continue;
                }
                target.pixels[static_cast<std::size_t>(target_y) *
                                  kScreenWidth +
                              static_cast<std::size_t>(target_x)] = source_pixel;
            }
        }
    };
    // 0x1D2E6's world sequence contributes the centered body from its fixed
    // pair before 0x1B364 adds the four animated side groups below.
    constexpr std::size_t kFixedPatternSource = 0xDA038U;
    constexpr std::size_t kFixedPatternBytes = 0x0A80U;
    constexpr std::size_t kFixedPatternFirstTile = 0x011BU;
    constexpr std::size_t kFixedPatternTileCount =
        kFixedPatternBytes / 32U;
    constexpr std::size_t kFixedPatternTilesPerRow = 16U;
    const auto fixed_atlas = shadowrun::graphics::decode_tile_atlas(
        rom, kFixedPatternSource, kFixedPatternTileCount, palette,
        kFixedPatternTilesPerRow, 0U);
    constexpr std::array<std::size_t, 2> kFixedFigureLists{{
        0x1D428U, 0x1D452U,
    }};
    constexpr int kFixedFigureBaseX = 0x0120 - 128;
    constexpr int kFixedFigureBaseY = 0x012F - 128;
    for (const auto list : kFixedFigureLists) {
        auto record = list;
        bool terminated = false;
        for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
            if (record > rom.size() || rom.size() - record < 8U) {
                throw std::runtime_error(
                    "Matrix entry runner fixed sprite list is truncated");
            }
            const auto x_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record));
            if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                terminated = true;
                break;
            }
            const auto y_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record + 2U));
            const auto size_word = read_big_endian_word(rom, record + 4U);
            const auto attributes = static_cast<std::uint16_t>(
                read_big_endian_word(rom, record + 6U) | 0x6000U);
            if ((size_word & 0xFF00U) != 0U) {
                throw std::runtime_error(
                    "Matrix entry runner fixed sprite size is invalid");
            }
            const auto first_tile = static_cast<std::size_t>(
                attributes & 0x07FFU);
            draw_sprite(figure, fixed_atlas, kFixedPatternTileCount,
                        kFixedPatternTilesPerRow, first_tile,
                        kFixedPatternFirstTile,
                        static_cast<std::uint8_t>(size_word), attributes,
                        kFixedFigureBaseX + x_offset,
                        kFixedFigureBaseY + y_offset);
            record += 8U;
        }
        if (!terminated) {
            throw std::runtime_error(
                "Matrix entry runner fixed sprite list has no terminator");
        }
    }
    for (const auto& group : kGroups) {
        const auto table_entry = group.pointer_table + kPhase * 4U;
        if (table_entry > rom.size() || rom.size() - table_entry < 4U) {
            throw std::runtime_error(
                "Matrix entry runner phase table is truncated");
        }
        const auto descriptor = static_cast<std::size_t>(
            read_big_endian_long(rom, table_entry));
        if (descriptor > rom.size() || rom.size() - descriptor < 8U) {
            throw std::runtime_error(
                "Matrix entry runner descriptor is truncated");
        }
        const auto pattern_source = static_cast<std::size_t>(
            read_big_endian_long(rom, descriptor + 2U));
        const auto pattern_bytes = static_cast<std::size_t>(
            read_big_endian_word(rom, descriptor + 6U));
        if (pattern_bytes == 0U || pattern_bytes % 32U != 0U ||
            pattern_source > rom.size() ||
            pattern_bytes > rom.size() - pattern_source ||
            group.destination > 0x10000U ||
            pattern_bytes > 0x10000U - group.destination) {
            throw std::runtime_error(
                "Matrix entry runner pattern block is invalid");
        }
        std::vector<std::uint8_t> pattern_vram(0x10000U, 0U);
        std::copy_n(rom.begin() + static_cast<std::ptrdiff_t>(pattern_source),
                    pattern_bytes,
                    pattern_vram.begin() + static_cast<std::ptrdiff_t>(
                                               group.destination));
        const auto tile_count = pattern_bytes / 32U;
        const auto atlas = shadowrun::graphics::decode_tile_atlas(
            pattern_vram, group.destination, tile_count, palette,
            tile_count, 0U);

        auto record = descriptor + 8U;
        bool terminated = false;
        for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
            if (record > rom.size() || rom.size() - record < 8U) {
                throw std::runtime_error(
                    "Matrix entry runner sprite list is truncated");
            }
            const auto x_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record));
            if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                terminated = true;
                break;
            }
            const auto y_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record + 2U));
            const auto size_word = read_big_endian_word(rom, record + 4U);
            const auto attributes = read_big_endian_word(rom, record + 6U);
            if ((size_word & 0xFF00U) != 0U ||
                ((attributes & 0xF000U) != 0U &&
                 (attributes & 0xF000U) != 0x0800U)) {
                throw std::runtime_error(
                    "Matrix entry runner SAT entry has invalid flags");
            }
            const auto first_tile = static_cast<std::size_t>(
                attributes & 0x07FFU);
            const auto origin_x = static_cast<int>(group.base_x) +
                                  x_offset - 128;
            const auto origin_y = static_cast<int>(group.base_y) +
                                  y_offset - 128;
            draw_sprite(figure, atlas, tile_count, tile_count, first_tile,
                        0U, static_cast<std::uint8_t>(size_word), attributes,
                        origin_x, origin_y);
            record += 8U;
        }
        if (!terminated) {
            throw std::runtime_error(
                "Matrix entry runner sprite list has no terminator");
        }
    }
    return figure;
}

shadowrun::graphics::RgbaImage compose_matrix_traversal_action_runner_phase(
    const std::vector<std::uint8_t>& rom,
    const std::size_t phase) {
    // 0x1B5B8 attaches behavior A after a successful world-action selection.
    // 0x1C3FA advances both layer phases from zero to one after five source
    // calls; phase zero is the compact bent-arm pose and phase one is raised.
    return compose_matrix_traversal_runner_pose(
        rom, 0x1C33AU, phase, 0x1C330U, phase,
        phase == 0U ? "behavior-A initial runner"
                    : "behavior-A raised-arm runner");
}

shadowrun::graphics::RgbaImage compose_matrix_world_model(
    const std::vector<std::uint8_t>& rom,
    const std::size_t model,
    const std::size_t palette_source) {
    constexpr std::size_t kModelTable = 0x0E2EDAU;
    constexpr std::size_t kModelCount = 6U;
    constexpr std::size_t kModelRecordBytes = 12U;
    if (model >= kModelCount) {
        throw std::invalid_argument("Matrix world model index is invalid");
    }

    const auto record = kModelTable + model * kModelRecordBytes;
    const auto width = static_cast<std::size_t>(
        read_big_endian_word(rom, record));
    const auto height = static_cast<std::size_t>(
        read_big_endian_word(rom, record + 2U));
    const auto map_source = static_cast<std::size_t>(
        read_big_endian_long(rom, record + 4U));
    const auto pattern_source = static_cast<std::size_t>(
        read_big_endian_long(rom, record + 8U));
    if ((width != 112U || height != 64U) &&
        (width != 96U || height != 80U)) {
        throw std::runtime_error(
            "Matrix world model record has an unexpected pixel size");
    }

    const auto columns = width / 8U;
    const auto rows = height / 8U;
    const auto map_entries = columns * rows;
    const auto map_bytes = map_entries * 2U;
    if (map_source > rom.size() || map_bytes > rom.size() - map_source) {
        throw std::runtime_error("Matrix world model map exceeds the ROM");
    }
    std::vector<std::uint16_t> tile_map;
    tile_map.reserve(map_entries);
    std::size_t maximum_tile = 0U;
    for (std::size_t index = 0U; index < map_entries; ++index) {
        const auto tile = static_cast<std::size_t>(
            read_big_endian_word(rom, map_source + index * 2U));
        maximum_tile = std::max(maximum_tile, tile);
        tile_map.push_back(static_cast<std::uint16_t>(tile));
    }

    const auto tile_count = maximum_tile + 1U;
    const auto pattern_bytes = tile_count * 32U;
    if (pattern_source > rom.size() ||
        pattern_bytes > rom.size() - pattern_source) {
        throw std::runtime_error("Matrix world model patterns exceed the ROM");
    }
    const std::vector<std::uint8_t> patterns(
        rom.begin() + static_cast<std::ptrdiff_t>(pattern_source),
        rom.begin() + static_cast<std::ptrdiff_t>(
            pattern_source + pattern_bytes));
    const auto atlas = shadowrun::graphics::decode_tile_atlas(
        patterns, 0U, tile_count,
        shadowrun::graphics::read_genesis_palette(rom, palette_source),
        16U, 0U);

    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(width),
        static_cast<std::uint16_t>(height),
        std::vector<shadowrun::native::Rgba8>(
            width * height, shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t row = 0U; row < rows; ++row) {
        for (std::size_t column = 0U; column < columns; ++column) {
            const auto tile = static_cast<std::size_t>(
                tile_map[row * columns + column]);
            for (std::size_t pixel_y = 0U; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0U; pixel_x < 8U; ++pixel_x) {
                    const auto atlas_x = (tile % 16U) * 8U + pixel_x;
                    const auto atlas_y = (tile / 16U) * 8U + pixel_y;
                    image.pixels[(row * 8U + pixel_y) * width +
                                 column * 8U + pixel_x] = atlas.pixels[
                        atlas_y * atlas.width + atlas_x];
                }
            }
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_analyze_ring_phase(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::RgbaImage& action_atlas,
    const std::size_t action_tile_count,
    const std::size_t phase) {
    // The successful Analyze action enters 0x1BEBA's 60-tick profile
    // transition. It selects a four-phase list from 0x1C136 and builds it at
    // both hand anchors. This helper is only for offline extraction; the game
    // runtime consumes the resulting named phase images.
    constexpr std::size_t kWidth = 320U;
    constexpr std::size_t kHeight = 224U;
    constexpr std::size_t kListTable = 0x1C136U;
    constexpr std::size_t kPatternFirstTile = 0x016FU;
    constexpr std::size_t kTilesPerRow = 16U;
    constexpr std::array<int, 2> kOriginX{0x015E - 128, 0x00E0 - 128};
    constexpr int kOriginY = 0x00EE - 128;
    if (phase >= 4U || action_tile_count == 0U ||
        action_atlas.width != kTilesPerRow * 8U) {
        throw std::invalid_argument(
            "Matrix Analyze ring phase extraction inputs are invalid");
    }
    const auto list = static_cast<std::size_t>(read_big_endian_long(
        rom, kListTable + phase * 4U));
    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kWidth),
        static_cast<std::uint16_t>(kHeight),
        std::vector<shadowrun::native::Rgba8>(
            kWidth * kHeight,
            shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (const auto origin_x : kOriginX) {
        auto record = list;
        auto terminated = false;
        for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
            if (record > rom.size() || rom.size() - record < 8U) {
                throw std::runtime_error(
                    "Matrix Analyze ring sprite list is truncated");
            }
            const auto x_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record));
            if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                terminated = true;
                break;
            }
            const auto y_offset = static_cast<std::int16_t>(
                read_big_endian_word(rom, record + 2U));
            const auto size = read_big_endian_word(rom, record + 4U);
            const auto attributes = static_cast<std::uint16_t>(
                read_big_endian_word(rom, record + 6U) | 0x2000U);
            if ((size & 0xFF00U) != 0U ||
                (attributes & 0xE000U) != 0xA000U) {
                throw std::runtime_error(
                    "Matrix Analyze ring sprite record is invalid");
            }
            const auto first_tile = static_cast<std::size_t>(
                attributes & 0x07FFU);
            if (first_tile < kPatternFirstTile) {
                throw std::runtime_error(
                    "Matrix Analyze ring sprite precedes its pattern bank");
            }
            const auto width_tiles = static_cast<std::size_t>(
                ((size >> 2U) & 0x03U) + 1U);
            const auto height_tiles = static_cast<std::size_t>(
                (size & 0x03U) + 1U);
            const auto tile_offset = first_tile - kPatternFirstTile;
            const auto sprite_tile_count = width_tiles * height_tiles;
            if (tile_offset > action_tile_count ||
                sprite_tile_count > action_tile_count - tile_offset) {
                throw std::runtime_error(
                    "Matrix Analyze ring sprite exceeds its pattern bank");
            }
            const bool flip_x = (attributes & 0x0800U) != 0U;
            const bool flip_y = (attributes & 0x1000U) != 0U;
            const auto sprite_width = width_tiles * 8U;
            const auto sprite_height = height_tiles * 8U;
            for (std::size_t y = 0U; y < sprite_height; ++y) {
                for (std::size_t x = 0U; x < sprite_width; ++x) {
                    const auto source_x = flip_x ? sprite_width - 1U - x : x;
                    const auto source_y = flip_y ? sprite_height - 1U - y : y;
                    // Genesis sprite tiles advance down each tile column.
                    const auto tile = tile_offset +
                        (source_x / 8U) * height_tiles + source_y / 8U;
                    const auto atlas_x = (tile % kTilesPerRow) * 8U +
                                         source_x % 8U;
                    const auto atlas_y = (tile / kTilesPerRow) * 8U +
                                         source_y % 8U;
                    const auto& pixel = action_atlas.pixels[
                        atlas_y * action_atlas.width + atlas_x];
                    const auto target_x = origin_x + x_offset +
                                          static_cast<int>(x);
                    const auto target_y = kOriginY + y_offset +
                                          static_cast<int>(y);
                    if (pixel.alpha == 0U || target_x < 0 || target_y < 0 ||
                        target_x >= static_cast<int>(kWidth) ||
                        target_y >= static_cast<int>(kHeight)) {
                        continue;
                    }
                    image.pixels[static_cast<std::size_t>(target_y) * kWidth +
                                 static_cast<std::size_t>(target_x)] = pixel;
                }
            }
            record += 8U;
        }
        if (!terminated) {
            throw std::runtime_error(
                "Matrix Analyze ring sprite list has no terminator");
        }
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_world_effect_record_phase(
    const std::vector<std::uint8_t>& rom,
    const std::size_t mode_record,
    const std::size_t phase) {
    // Offline import for one record selected by 0x1A908/0x19B40. The
    // negative selector uses the separate default record at 0x1AA10.
    // 0x1AA40 builds its sprites at (0x120,0xD0) with tile base 0x694 and
    // palette attributes 0x2000, so the host image is anchored at (160,80)
    // and decoded with CRAM line 1 from 0xE2E5A.
    constexpr std::size_t kFrameWidth = 320U;
    constexpr std::size_t kFrameHeight = 224U;
    constexpr std::size_t kTilesPerRow = 16U;
    constexpr std::size_t kFirstTile = 0x0694U;
    constexpr int kOriginX = 0x0120 - 128;
    constexpr int kOriginY = 0x00D0 - 128;
    if (mode_record > rom.size() || rom.size() - mode_record < 6U) {
        throw std::runtime_error("Matrix world-effect mode record is truncated");
    }
    const auto phase_count = static_cast<std::size_t>(rom[mode_record + 1U]);
    const auto phase_table = static_cast<std::size_t>(
        read_big_endian_long(rom, mode_record + 2U));
    if (phase_count == 0U || phase >= phase_count ||
        phase_table > rom.size() ||
        phase_count > (rom.size() - phase_table) / 4U) {
        throw std::out_of_range("Matrix world-effect phase is invalid");
    }

    const auto descriptor = static_cast<std::size_t>(
        read_big_endian_long(rom, phase_table + phase * 4U));
    if (descriptor > rom.size() || rom.size() - descriptor < 8U) {
        throw std::runtime_error(
            "Matrix world-effect descriptor is truncated");
    }
    const auto pattern_source = static_cast<std::size_t>(
        read_big_endian_long(rom, descriptor + 2U));
    const auto pattern_bytes = static_cast<std::size_t>(
        read_big_endian_word(rom, descriptor + 6U));
    if (pattern_bytes == 0U || pattern_bytes % 32U != 0U ||
        pattern_source > rom.size() ||
        pattern_bytes > rom.size() - pattern_source) {
        throw std::runtime_error(
            "Matrix world-effect pattern bank is invalid");
    }
    const auto tile_count = pattern_bytes / 32U;
    const std::vector<std::uint8_t> pattern_data(
        rom.begin() + static_cast<std::ptrdiff_t>(pattern_source),
        rom.begin() + static_cast<std::ptrdiff_t>(
            pattern_source + pattern_bytes));
    const auto atlas = shadowrun::graphics::decode_tile_atlas(
        pattern_data, 0U, tile_count,
        shadowrun::graphics::read_genesis_palette(rom, 0x0E2E5AU),
        kTilesPerRow, 0U);

    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(kFrameWidth),
        static_cast<std::uint16_t>(kFrameHeight),
        std::vector<shadowrun::native::Rgba8>(
            kFrameWidth * kFrameHeight,
            shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    auto terminated = false;
    for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
        const auto record = descriptor + 8U + sprite * 8U;
        if (record > rom.size() || rom.size() - record < 8U) {
            throw std::runtime_error(
                "Matrix world-effect sprite list is truncated");
        }
        const auto x_offset = static_cast<std::int16_t>(
            read_big_endian_word(rom, record));
        if (x_offset == std::numeric_limits<std::int16_t>::min()) {
            terminated = true;
            break;
        }
        const auto y_offset = static_cast<std::int16_t>(
            read_big_endian_word(rom, record + 2U));
        const auto size_word = read_big_endian_word(rom, record + 4U);
        const auto tile_offset = static_cast<std::size_t>(
            read_big_endian_word(rom, record + 6U));
        if ((size_word & 0xFF00U) != 0U || tile_offset > tile_count) {
            throw std::runtime_error(
                "Matrix world-effect sprite record is invalid");
        }
        const auto width_tiles = static_cast<std::size_t>(
            ((size_word >> 2U) & 0x03U) + 1U);
        const auto height_tiles = static_cast<std::size_t>(
            (size_word & 0x03U) + 1U);
        const auto sprite_tile_count = width_tiles * height_tiles;
        if (sprite_tile_count > tile_count - tile_offset) {
            throw std::runtime_error(
                "Matrix world-effect sprite exceeds its pattern bank");
        }

        const auto sprite_width = width_tiles * 8U;
        const auto sprite_height = height_tiles * 8U;
        for (std::size_t local_y = 0U; local_y < sprite_height; ++local_y) {
            for (std::size_t local_x = 0U; local_x < sprite_width; ++local_x) {
                // Genesis multi-tile sprites advance vertically within each
                // tile column. Mode records do not request X/Y flips.
                const auto tile_index = tile_offset +
                    (local_x / 8U) * height_tiles + local_y / 8U;
                const auto atlas_x = (tile_index % kTilesPerRow) * 8U +
                                     local_x % 8U;
                const auto atlas_y = (tile_index / kTilesPerRow) * 8U +
                                     local_y % 8U;
                const auto pixel = atlas.pixels[
                    atlas_y * atlas.width + atlas_x];
                const auto target_x = kOriginX + x_offset +
                                      static_cast<int>(local_x);
                const auto target_y = kOriginY + y_offset +
                                      static_cast<int>(local_y);
                if (pixel.alpha == 0U || target_x < 0 || target_y < 0 ||
                    target_x >= static_cast<int>(kFrameWidth) ||
                    target_y >= static_cast<int>(kFrameHeight)) {
                    continue;
                }
                image.pixels[static_cast<std::size_t>(target_y) * kFrameWidth +
                             static_cast<std::size_t>(target_x)] = pixel;
            }
        }
    }
    if (!terminated) {
        throw std::runtime_error(
            "Matrix world-effect sprite list has no terminator");
    }
    return image;
}

shadowrun::graphics::RgbaImage compose_matrix_world_effect_mode_phase(
    const std::vector<std::uint8_t>& rom,
    const std::size_t mode,
    const std::size_t phase) {
    constexpr std::size_t kModeTable = 0x1A9CEU;
    constexpr std::size_t kModeCount = 9U;
    if (mode >= kModeCount || kModeTable + (mode + 1U) * 4U > rom.size()) {
        throw std::invalid_argument("Matrix world-effect mode is invalid");
    }
    const auto mode_record = static_cast<std::size_t>(
        read_big_endian_long(rom, kModeTable + mode * 4U));
    return compose_matrix_world_effect_record_phase(rom, mode_record, phase);
}

shadowrun::graphics::RgbaImage decode_matrix_map_font(
    const std::vector<std::uint8_t>& rom,
    const std::size_t matrix_palette_base) {
    // 0xC0DA resolves glyphs through 0xC1320 and writes the caller's
    // 0x2000 name-table attribute. Its glyph pixels therefore use palette
    // line 1 from the Matrix screen's 0x15FCB6 CRAM block, not the startup
    // palette used by the shared extracted font mask.
    constexpr std::size_t kStartupPaletteBase = 0x01FF0U;
    auto font = shadowrun::graphics::decode_genesis_startup_font(rom);
    const auto startup_palette =
        shadowrun::graphics::read_genesis_palette_set(
            rom, kStartupPaletteBase);
    const auto matrix_palette =
        shadowrun::graphics::read_genesis_palette_set(
            rom, matrix_palette_base);

    for (auto& pixel : font.pixels) {
        if (pixel.alpha == 0U) {
            continue;
        }
        bool matched = false;
        for (std::size_t color_index = 1U; color_index < 16U;
             ++color_index) {
            const auto& startup_color = startup_palette[16U + color_index];
            if (pixel.red != startup_color.red ||
                pixel.green != startup_color.green ||
                pixel.blue != startup_color.blue) {
                continue;
            }
            pixel = matrix_palette[16U + color_index];
            pixel.alpha = 255U;
            matched = true;
            break;
        }
        if (!matched) {
            throw std::runtime_error(
                "Matrix text glyph does not map to the startup palette");
        }
    }
    return font;
}

std::string format_resource_source(
    const shadowrun::graphics::GenesisGraphicsResource& resource) {
    std::ostringstream output;
    output << "table=0x" << std::hex << resource.table_offset << " index="
           << std::dec << resource.table_index << " sprite="
           << resource.sprite_index << " source=0x" << std::hex
           << resource.source_offset << " attr=0x" << resource.attr_or;
    return output.str();
}

void write_png32(const std::string& path,
                 const shadowrun::graphics::RgbaImage& image) {
    if (image.width == 0U || image.height == 0U || image.pixels.empty()) {
        throw std::runtime_error("cannot write an empty PNG image");
    }

    std::vector<std::uint8_t> scanlines;
    scanlines.reserve(static_cast<std::size_t>(image.height) *
                      (static_cast<std::size_t>(image.width) * 4U + 1U));
    for (std::size_t y = 0U; y < image.height; ++y) {
        scanlines.push_back(0U);
        for (std::size_t x = 0U; x < image.width; ++x) {
            const auto& pixel =
                image.pixels[y * static_cast<std::size_t>(image.width) + x];
            scanlines.push_back(pixel.red);
            scanlines.push_back(pixel.green);
            scanlines.push_back(pixel.blue);
            scanlines.push_back(pixel.alpha);
        }
    }

    uLongf compressed_size =
        compressBound(static_cast<uLong>(scanlines.size()));
    std::vector<std::uint8_t> compressed(compressed_size);
    const auto compression = compress2(
        compressed.data(), &compressed_size, scanlines.data(),
        static_cast<uLong>(scanlines.size()), Z_BEST_COMPRESSION);
    if (compression != Z_OK) {
        throw std::runtime_error("cannot compress PNG image rows");
    }
    compressed.resize(static_cast<std::size_t>(compressed_size));

    std::vector<std::uint8_t> png{
        0x89U, 'P', 'N', 'G', 0x0DU, 0x0AU, 0x1AU, 0x0AU};
    const auto append_big_endian = [&png](const std::uint32_t value) {
        png.push_back(static_cast<std::uint8_t>(value >> 24U));
        png.push_back(static_cast<std::uint8_t>(value >> 16U));
        png.push_back(static_cast<std::uint8_t>(value >> 8U));
        png.push_back(static_cast<std::uint8_t>(value));
    };
    const auto append_chunk = [&png, &append_big_endian](
                                  const std::array<std::uint8_t, 4>& type,
                                  const std::vector<std::uint8_t>& data) {
        append_big_endian(static_cast<std::uint32_t>(data.size()));
        const auto crc_begin = png.size();
        png.insert(png.end(), type.begin(), type.end());
        png.insert(png.end(), data.begin(), data.end());
        const auto checksum = crc32(
            crc32(0L, Z_NULL, 0), png.data() + crc_begin,
            static_cast<uInt>(png.size() - crc_begin));
        append_big_endian(static_cast<std::uint32_t>(checksum));
    };

    std::vector<std::uint8_t> header;
    header.reserve(13U);
    const auto append_header_word = [&header](const std::uint32_t value) {
        header.push_back(static_cast<std::uint8_t>(value >> 24U));
        header.push_back(static_cast<std::uint8_t>(value >> 16U));
        header.push_back(static_cast<std::uint8_t>(value >> 8U));
        header.push_back(static_cast<std::uint8_t>(value));
    };
    append_header_word(image.width);
    append_header_word(image.height);
    header.push_back(8U);
    header.push_back(6U);
    header.push_back(0U);
    header.push_back(0U);
    header.push_back(0U);
    append_chunk(std::array<std::uint8_t, 4>{{'I', 'H', 'D', 'R'}}, header);
    append_chunk(std::array<std::uint8_t, 4>{{'I', 'D', 'A', 'T'}}, compressed);
    append_chunk(std::array<std::uint8_t, 4>{{'I', 'E', 'N', 'D'}}, {});

    std::filesystem::create_directories(
        std::filesystem::path(path).parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create PNG image: " + path);
    }
    output.write(reinterpret_cast<const char*>(png.data()),
                 static_cast<std::streamsize>(png.size()));
    if (!output) {
        throw std::runtime_error("cannot finish PNG image: " + path);
    }
}

void write_catalog_asset(const std::filesystem::path& root,
                         const std::string& relative_name,
                         const shadowrun::graphics::RgbaImage& image,
                         const std::string& kind,
                         const std::string& provenance, bool crop,
                         std::ofstream& manifest, std::size_t& written,
                         std::size_t& missing, std::size_t& blank,
                         const std::int32_t source_origin_x = 0,
                         const std::int32_t source_origin_y = 0,
                         const std::uint16_t source_width = 0,
                         const std::uint16_t source_height = 0) {
    if (image.width == 0U || image.height == 0U || image.pixels.empty()) {
        ++missing;
        manifest << relative_name << '\t' << kind
                 << "\t0\t0\t0\t0\t" << provenance << "\tmissing\n";
        return;
    }
    auto status = "extracted";
    CroppedAsset asset;
    if (source_width != 0U && source_height != 0U) {
        asset = crop_source_region(image, source_origin_x, source_origin_y,
                                   source_width, source_height);
    } else if (crop) {
        try {
            asset = crop_transparent_bounds(image);
            // Descriptor origins locate the uncropped frame relative to its
            // world anchor. Cropping removes transparent pixels from the
            // image, so carry both offsets into the direct-asset catalog.
            asset.origin_x += source_origin_x;
            asset.origin_y += source_origin_y;
        } catch (const std::runtime_error& error) {
            if (std::string(error.what()) != "asset contains no opaque pixels") {
                throw;
            }
            // A state-specific source can legitimately be fully transparent
            // (for example, an empty initial HUD counter). Preserve it as an
            // individual transparent sprite instead of silently dropping the
            // resource or treating it as a decoder failure.
            asset = CroppedAsset{image, source_origin_x, source_origin_y};
            status = "extracted-empty";
            ++blank;
        }
    } else {
        asset = CroppedAsset{image, source_origin_x, source_origin_y};
    }
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    if (path.extension() == ".png") {
        write_png32(path.string(), asset.image);
    } else {
        shadowrun::graphics::write_bmp32(path.string(), asset.image);
    }
    manifest << relative_name << '\t' << kind << '\t' << asset.origin_x
             << '\t' << asset.origin_y << '\t' << asset.image.width << '\t'
             << asset.image.height << '\t' << provenance << '\t' << status
             << '\n';
    ++written;
}

struct ExtractedSpriteListFrame {
    shadowrun::graphics::RgbaImage image;
    std::int32_t origin_x = 0;
    std::int32_t origin_y = 0;
};

ExtractedSpriteListFrame decode_field_entity_sprite_list(
    const std::vector<std::uint8_t>& rom,
    const std::vector<std::uint8_t>& vram,
    const std::size_t sprite_list_offset,
    const std::array<shadowrun::native::Rgba8, 64>& palette,
    const std::uint16_t palette_attributes) {
    constexpr std::size_t kVramSize = 0x10000U;
    constexpr std::uint16_t kEndSpriteList = 0x8000U;
    if (vram.size() != kVramSize) {
        throw std::invalid_argument("field actor sprite VRAM has invalid size");
    }

    struct PositionedSprite {
        std::int32_t x = 0;
        std::int32_t y = 0;
        std::uint8_t size = 0;
        std::uint16_t attributes = 0;
    };
    std::vector<PositionedSprite> sprites;
    auto source = sprite_list_offset;
    auto terminated = false;
    for (;;) {
        const auto x_word = read_big_endian_word(rom, source);
        source += 2U;
        if (x_word == kEndSpriteList) {
            terminated = true;
            break;
        }
        if (source > rom.size() || rom.size() - source < 6U) {
            throw std::out_of_range(
                "field actor sprite-list record is truncated");
        }
        const auto y_word = read_big_endian_word(rom, source);
        const auto size_word = read_big_endian_word(rom, source + 2U);
        const auto attributes = read_big_endian_word(rom, source + 4U);
        source += 6U;
        if ((size_word & 0xFF00U) != 0U) {
            throw std::runtime_error(
                "field actor sprite size exceeds the source byte field");
        }
        sprites.push_back({
            static_cast<std::int16_t>(x_word),
            static_cast<std::int16_t>(y_word),
            static_cast<std::uint8_t>(size_word),
            static_cast<std::uint16_t>(attributes | palette_attributes),
        });
        if (sprites.size() > 80U) {
            throw std::runtime_error(
                "field actor sprite list exceeds the supported source bound");
        }
    }
    if (!terminated || sprites.empty()) {
        throw std::runtime_error(
            "field actor sprite list is empty or lacks its sentinel");
    }

    std::int32_t min_x = std::numeric_limits<std::int32_t>::max();
    std::int32_t min_y = std::numeric_limits<std::int32_t>::max();
    std::int32_t max_x = std::numeric_limits<std::int32_t>::min();
    std::int32_t max_y = std::numeric_limits<std::int32_t>::min();
    for (const auto& sprite : sprites) {
        const auto width = static_cast<std::int32_t>(
            ((sprite.size >> 2U) & 0x03U) + 1U) * 8;
        const auto height = static_cast<std::int32_t>(
            (sprite.size & 0x03U) + 1U) * 8;
        min_x = std::min(min_x, sprite.x);
        min_y = std::min(min_y, sprite.y);
        max_x = std::max(max_x, sprite.x + width);
        max_y = std::max(max_y, sprite.y + height);
    }
    if (max_x <= min_x || max_y <= min_y ||
        max_x - min_x > std::numeric_limits<std::uint16_t>::max() ||
        max_y - min_y > std::numeric_limits<std::uint16_t>::max()) {
        throw std::runtime_error("field actor sprite-list bounds are invalid");
    }

    shadowrun::graphics::RgbaImage image{
        static_cast<std::uint16_t>(max_x - min_x),
        static_cast<std::uint16_t>(max_y - min_y),
        std::vector<shadowrun::native::Rgba8>(
            static_cast<std::size_t>(max_x - min_x) *
                static_cast<std::size_t>(max_y - min_y),
            shadowrun::native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (const auto& sprite : sprites) {
        const auto hflip = (sprite.attributes & 0x0800U) != 0U;
        const auto vflip = (sprite.attributes & 0x1000U) != 0U;
        const auto palette_line = static_cast<std::size_t>(
            (sprite.attributes >> 13U) & 0x03U);
        const auto tile_base = static_cast<std::size_t>(
            sprite.attributes & 0x07FFU);
        const auto width_tiles = static_cast<std::size_t>(
            ((sprite.size >> 2U) & 0x03U) + 1U);
        const auto height_tiles = static_cast<std::size_t>(
            (sprite.size & 0x03U) + 1U);
        for (std::size_t tile_x = 0; tile_x < width_tiles; ++tile_x) {
            for (std::size_t tile_y = 0; tile_y < height_tiles; ++tile_y) {
                const auto source_tile_x =
                    hflip ? width_tiles - 1U - tile_x : tile_x;
                const auto source_tile_y =
                    vflip ? height_tiles - 1U - tile_y : tile_y;
                // Genesis sprite patterns advance down a column before the
                // next column. The source sprite-list renderer does the same.
                const auto tile_index = tile_base +
                    source_tile_x * height_tiles + source_tile_y;
                const auto tile_offset = tile_index * 32U;
                if (tile_offset > vram.size() ||
                    vram.size() - tile_offset < 32U) {
                    throw std::out_of_range(
                        "field actor sprite pattern lies outside VRAM");
                }
                const auto tile = shadowrun::graphics::decode_genesis_tile(
                    vram.data() + tile_offset, 32U);
                for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                    for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                        const auto color_index = tile.pixels[
                            (vflip ? 7U - pixel_y : pixel_y) * 8U +
                            (hflip ? 7U - pixel_x : pixel_x)];
                        if (color_index == 0U) {
                            continue;
                        }
                        const auto target_x = sprite.x - min_x +
                            static_cast<std::int32_t>(tile_x * 8U + pixel_x);
                        const auto target_y = sprite.y - min_y +
                            static_cast<std::int32_t>(tile_y * 8U + pixel_y);
                        image.pixels[
                            static_cast<std::size_t>(target_y) * image.width +
                            static_cast<std::size_t>(target_x)] =
                            palette[palette_line * 16U + color_index];
                    }
                }
            }
        }
    }
    return {std::move(image), min_x, min_y};
}

void append_title_boot_catalog(const std::filesystem::path& root,
                               std::ofstream& manifest) {
    std::ifstream source(root / "manifest.tsv", std::ios::binary);
    if (!source) {
        throw std::runtime_error("title/boot manifest was not generated");
    }
    std::string line;
    std::getline(source, line);
    while (std::getline(source, line)) {
        if (line.empty()) {
            continue;
        }
        const auto kind = line.find("background.bmp") != std::string::npos ||
                line.find("circuit-background-") != std::string::npos ||
                line.find("snow.bmp") != std::string::npos
            ? "background"
            : "sprite";
        manifest << line.substr(0, line.find('\t')) << '\t' << kind
                 << '\t' << line.substr(line.find('\t') + 1)
                 << '\t' << "ROM title/boot resource\textracted\n";
    }
}

void write_asset_readme(const std::filesystem::path& root) {
    std::ofstream readme(root / "README.md", std::ios::binary);
    if (!readme) {
        throw std::runtime_error("cannot create direct asset README");
    }
    readme << "# Direct Shadowrun assets\n\n"
           << "Generated from the US Genesis ROM by "
              "shadowrun_graphics_extract --all-assets.\n\n"
           << "The native and Ghidra-conversion runtimes consume these "
              "individually named BMP and PNG images, direct text records, "
              "native/state.bin, "
              "and resources.tsv directly. This tree deliberately contains "
              "semantic folders: menu, intro, field, and item categories. "
              "Static intro plates and independently extracted intro sprites "
              "remain separate; "
              "it does not contain full-screen flipbook samples or an aggregate "
              "serializer. All ROM-derived dialogue, field-menu labels, "
              "startup menu copy, feedback, and Notebook Tips are keyed in "
              "`dialogue/script-text.json`; `menu/field/service-routes.tsv` "
              "retains only "
              "numeric service destinations. Command labels, menu dialogue, "
              "service titles, shop category headings, item names/categories/details, "
              "startup labels/messages, and transaction copy are emitted in "
              "`dialogue/script-text.json`; `gameplay/scripts/shop.lua` binds "
              "the 12 service titles, 32 source-order action headings, Ares "
              "categories, and message keys, while "
              "`gameplay/scripts/items.lua` maps source item IDs to their "
              "name/category/detail keys, inventory/First Aid copy, weapon "
              "power grades, and armor display labels. "
              "`gameplay/scripts/attributes_skills.lua` maps the "
              "Attributes/Skills labels and upgrade copy to stable IDs. "
              "`gameplay/scripts/items.lua` also maps weapon power grades and "
              "armor labels to stable display IDs. "
              "The spell-capable Magic panel is an individually "
              "named RGBA overlay. Item art is grouped under items/weapons, "
              "items/first-aid, items/miscellaneous, items/magic, and items/armor. "
              "The shared field dialogue frames and A/B/C marker sprites are "
              "grouped under dialogue/panel, with speaker portraits under "
              "dialogue/portraits. ROM-derived Stoker, Gunderson, and field interaction "
              "text, Hotel menu and status text, Ares mode-event dialogue, "
              "Pocket Secretary menu and child-page text, field-service "
              "titles, shop "
              "text, and item descriptions are emitted as "
              "`dialogue/script-text.json` and consumed by the gameplay script "
              "runtime. `gameplay/scripts/hotel.lua`, "
              "`gameplay/scripts/pocket_secretary.lua`, and "
              "`gameplay/scripts/startup.lua` provide cursor-menu choice IDs; "
              "`gameplay/scripts/clips.lua` provides selector and text IDs; "
              "`gameplay/scripts/ui.lua` provides shared confirmation choices "
              "and field metric caption IDs. "
              "`gameplay/scripts/magic.lua` provides ROM-ordered spell and "
              "Magic panel text IDs. "
              "`gameplay/scripts/cyberware.lua` provides stable IDs for the "
              "Cyberware panel. `gameplay/scripts/command_overview.lua` maps "
              "race, class, and posture caption IDs. "
              "Separately traced cinematic resources belong under movies/<scene>; "
              "they must not be mixed into intro or field assets.\n\n"
           << "Audio is emitted here only as complete Windows-playable lossless "
              "FLAC files under\n"
              "`audio/`. The offline Z80/VGM extraction sources and intermediate "
              "event data\n"
              "remain outside this runtime asset tree. Verified repeating audio "
              "uses\n"
              "`audio/loop-points.tsv` for exact PCM frame boundaries. Use "
              "`resources.tsv` for stable aliases,\n"
              "such as `assets.audio_path(\"audio.intro.title.stage-1\")`; do "
              "not add raw ROM\n"
              "slices or split event fragments here. A missing loop-point entry "
              "is\n"
              "intentional: the native player must not guess a repeat boundary "
              "from an\n"
              "internal channel restart.\n\n"
           << "Startup/session records are imported separately under "
               "`records/startup/` by `shadowrun_ghidra_startup_record_extract` "
               "and listed in `resources.tsv`. Field startup records remain "
               "individually named under `records/field/`; the complete 56-row "
               "scene profile table is `records/field/scene-profiles.bin`, and "
               "the complete field resource set is grouped under "
               "`records/field/scenes/scene-00` through `scene-55`. Each scene "
               "has its fixed terrain layers, offline-expanded metatile bytes, "
               "descriptor, collision records, and optional event table listed "
               "by a stable alias in `resources.tsv`. The conversion loader "
               "interprets each into typed field state and host collision maps "
               "without making the runtime read ROM addresses. The source "
               "movement vector table at `0xC002` is likewise one named direct "
               "record consumed as typed directional offsets by the conversion "
               "layer. The `0xBD34` collision-probe offset banks are kept as "
               "one four-bank direct record beside it. The `0x19D2` "
               "controller-heading table is kept beside it "
               "as one sixteen-word direct record. The `0xBBF6` aligned-route "
               "table is kept as four six-byte semantic records after its "
               "FFEF10..13 sample pointers are resolved offline.\n";
}

void write_runtime_resource_manifest(const std::filesystem::path& root) {
    std::ifstream catalog(root / "catalog.tsv", std::ios::binary);
    if (!catalog) {
        throw std::runtime_error("cannot read generated asset catalog");
    }
    // Graphics extraction is an explicit import step, but it must not erase
    // independently imported runtime resources. Preserve existing state and
    // audio rows while regenerating the image aliases from catalog.tsv.
    std::vector<std::string> preserved_state_rows;
    std::vector<std::string> preserved_runtime_rows;
    std::ifstream prior_manifest(
        root / std::string(shadowrun::resources::kManifestFile),
        std::ios::binary);
    if (prior_manifest) {
        std::string prior_line;
        std::getline(prior_manifest, prior_line);
        while (std::getline(prior_manifest, prior_line)) {
            if (prior_line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::size_t start = 0;
            for (;;) {
                const auto end = prior_line.find('\t', start);
                fields.push_back(prior_line.substr(
                    start, end == std::string::npos ? std::string::npos
                                                     : end - start));
                if (end == std::string::npos) {
                    break;
                }
                start = end + 1U;
            }
            const auto legacy_field_scene_record =
                fields.size() == 5U &&
                (fields[0].rfind("records.field.mode.0.", 0U) == 0U ||
                 fields[0].rfind("records.field.mode.2.", 0U) == 0U);
            if (fields.size() == 5U && fields[0] != "state" &&
                fields[2] != "image" && !legacy_field_scene_record &&
                std::filesystem::is_regular_file(root / fields[1])) {
                if (fields[2] == "state") {
                    preserved_state_rows.push_back(prior_line);
                } else {
                    preserved_runtime_rows.push_back(prior_line);
                }
            }
        }
    }
    std::ofstream output(root / std::string(
                              shadowrun::resources::kManifestFile),
                         std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot create runtime resource manifest");
    }
    output << "alias\tpath\tkind\torigin_x\torigin_y\n"
           << "state\tnative/state.bin\tstate\t0\t0\n";
    std::unordered_set<std::string> aliases{"state"};
    const auto emit_preserved_rows = [&output, &aliases](
                                         const std::vector<std::string>& rows) {
        for (const auto& preserved : rows) {
            const auto separator = preserved.find('\t');
            if (separator == std::string::npos) {
                continue;
            }
            const auto alias = preserved.substr(0, separator);
            if (aliases.insert(alias).second) {
                output << preserved << '\n';
            }
        }
    };
    emit_preserved_rows(preserved_state_rows);
    const auto emit_generated_state_row = [&output, &aliases, &root](
                                              const std::string& alias,
                                              const std::string& relative) {
        if (!std::filesystem::is_regular_file(root / relative)) {
            throw std::runtime_error("generated source record is missing: " +
                                     std::string(relative));
        }
        if (aliases.insert(alias).second) {
            output << alias << '\t' << relative << "\tstate\t0\t0\n";
        }
    };
    emit_generated_state_row(
        "records.field.action.selection.profiles",
        "records/field/action-selection-profiles.bin");
    emit_generated_state_row(
        "records.field.mode.selection.state",
        "records/field/mode-selection-state.bin");
    emit_generated_state_row(
        "records.field.scene.profiles",
        "records/field/scene-profiles.bin");
    emit_generated_state_row(
        "records.field.elevator.scene.groups",
        "records/field/elevator-scene-groups.bin");
    emit_generated_state_row(
        "records.field.entity.movement.direction.table",
        "records/field/entity-movement-direction-table.bin");
    emit_generated_state_row(
        "records.field.entity.collision.probe.offset.banks",
        "records/field/entity-collision-probe-offset-banks.bin");
    emit_generated_state_row(
        "records.field.entity.controller.heading.table",
        "records/field/entity-controller-heading-table.bin");
    emit_generated_state_row(
        "records.field.entity.movement.route.table",
        "records/field/entity-movement-route-table.bin");
    for (std::uint16_t scene_id = 0;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto directory = field_scene_record_directory(scene_id);
        const auto emit_scene_record = [&](const std::string& suffix) {
            const auto relative = directory + "/" + suffix + ".bin";
            emit_generated_state_row(
                shadowrun::resources::alias_for_path(relative), relative);
        };
        emit_scene_record("terrain-layer-a");
        emit_scene_record("terrain-layer-b");
        emit_scene_record("terrain-stream");
        emit_scene_record("descriptor");
        emit_scene_record("terrain-collision-rectangles");
        if (scene_id >= 0x15U && scene_id < 0x2CU) {
            emit_scene_record("world-activity-random-index-bound");
            emit_scene_record("world-activity-resource-cursor");
        }
        const auto event_relative = directory + "/event-table.bin";
        if (std::filesystem::is_regular_file(root / event_relative)) {
            emit_generated_state_row(
                shadowrun::resources::alias_for_path(event_relative),
                event_relative);
        }
    }
    // Startup/session records are extracted by the separate conversion-side
    // record tool, but once present they must remain first-class manifest rows
    // even if --all-assets is rerun from the native graphics importer.
    const auto emit_optional_state_row = [&output, &aliases, &root](
                                             const char* alias,
                                             const char* relative) {
        if (std::filesystem::is_regular_file(root / relative) &&
            aliases.insert(alias).second) {
            output << alias << '\t' << relative << "\tstate\t0\t0\n";
        }
    };
    emit_optional_state_row(
        "records.startup.joshua.archetypes",
        "records/startup/joshua-archetypes.bin");
    emit_optional_state_row(
        "records.startup.recruited.runners",
        "records/startup/recruited-runner-records.bin");
    emit_optional_state_row(
        "records.startup.save.roster.header",
        "records/startup/save-roster-header.bin");
    const auto emit_generated_resource_row = [&output, &aliases, &root](
                                                const char* alias,
                                                const char* relative,
                                                const char* kind) {
        if (!std::filesystem::is_regular_file(root / relative)) {
            throw std::runtime_error("generated runtime resource is missing: " +
                                     std::string(relative));
        }
        if (aliases.insert(alias).second) {
            output << alias << '\t' << relative << '\t' << kind << "\t0\t0\n";
        }
    };
    emit_generated_resource_row(
        "menu.field.service.routes", "menu/field/service-routes.tsv", "text");
    emit_generated_resource_row(
        "field.entities.default.actors",
        "field/entities/default-actors.tsv", "state");
    emit_generated_resource_row(
        "field.entities.default.actor.waypoints",
        "field/entities/default-actor-waypoints.tsv", "state");
    emit_generated_resource_row(
        "field.entities.world.activity.actors",
        "field/entities/world-activity-actors.tsv", "state");
    emit_generated_resource_row(
        "field.entities.world.activity.encounter.actors",
        "field/entities/dynamic-world-activity-actors.tsv", "state");
    emit_generated_resource_row(
        "field.entities.actor.action.profiles",
        "field/entities/actor-action-profiles.tsv", "state");
    emit_generated_resource_row(
        "field.entities.actor.reaction.profiles",
        "field/entities/actor-reaction-profiles.tsv", "state");
    std::string line;
    std::getline(catalog, line);
    while (std::getline(catalog, line)) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> fields;
        std::size_t start = 0;
        for (;;) {
            const auto end = line.find('\t', start);
            fields.push_back(line.substr(
                start, end == std::string::npos ? std::string::npos
                                                 : end - start));
            if (end == std::string::npos) {
                break;
            }
            start = end + 1U;
        }
        if (fields.size() < 8U) {
            throw std::runtime_error("invalid generated asset catalog row: " +
                                     line);
        }
        const auto& relative = fields[0];
        if (relative.find("-gray-") != std::string::npos) {
            continue;
        }
        const auto alias = shadowrun::resources::alias_for_path(relative);
        if (!aliases.insert(alias).second) {
            throw std::runtime_error("duplicate generated resource alias: " +
                                     alias);
        }
        output << alias << '\t' << relative << "\timage\t" << fields[2]
               << '\t' << fields[3] << '\n';
    }
    emit_preserved_rows(preserved_runtime_rows);
}

void extract_all_assets(const std::vector<std::uint8_t>& rom,
                        const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    remove_obsolete_menu_text_assets(root);
    extract_title_boot_assets(rom, root);

    std::ofstream manifest(root / "catalog.tsv", std::ios::binary);
    if (!manifest) {
        throw std::runtime_error("cannot create all-resource catalog");
    }
    manifest << "asset\tkind\tx\ty\twidth\theight\tsource\tstatus\n";
    write_asset_readme(root);
    append_title_boot_catalog(root, manifest);
    std::size_t written = 0;
    std::size_t missing = 0;
    std::size_t blank = 0;
    const auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    auto field_text = write_field_service_routes(rom, root, assets);
    struct NotebookRecordTables {
        std::string_view id;
        std::size_t count;
        std::size_t name_table;
        std::size_t detail_table;
    };
    // ROM pointer tables back the notebook's 13 contacts and 10 recruited
    // runners. Import their names and biographies here so --all-assets is a
    // complete one-step extraction; the runtime consumes only keyed Lua text.
    constexpr std::array<NotebookRecordTables, 2> kNotebookRecordTables{{
        {"contact", 13U, 0xC66ACU, 0xC66E0U},
        {"shadowrunner", 10U, 0xC665CU, 0xC6684U},
    }};
    const auto read_notebook_text = [&rom](const std::size_t source) {
        const auto segments =
            shadowrun::graphics::read_genesis_dialogue_segments(rom, source);
        std::string text;
        for (std::size_t index = 0; index < segments.size(); ++index) {
            if (index != 0U) {
                text.push_back('\n');
            }
            text += segments[index];
        }
        if (text.empty()) {
            throw std::runtime_error("notebook ROM text is empty");
        }
        return text;
    };
    for (const auto& table : kNotebookRecordTables) {
        for (std::size_t index = 0; index < table.count; ++index) {
            const auto record_id = "record_" + std::to_string(index);
            const auto name_source = static_cast<std::size_t>(
                read_big_endian_long(rom, table.name_table + index * 4U));
            const auto detail_source = static_cast<std::size_t>(
                read_big_endian_long(rom, table.detail_table + index * 4U));
            field_text.emplace_back(
                "pocket_secretary.notebook." + std::string(table.id) +
                    ".name.option." + record_id,
                read_notebook_text(name_source));
            field_text.emplace_back(
                "pocket_secretary.notebook." + std::string(table.id) +
                    ".detail.option." + record_id,
                read_notebook_text(detail_source));
        }
    }
    field_text.emplace_back(
        "pocket_secretary.notebook.record_page.option.next",
        read_notebook_text(0xC661DU));
    field_text.emplace_back(
        "pocket_secretary.notebook.record_page.option.prior",
        read_notebook_text(0xC662BU));
    auto notebook_tips = read_notebook_tips_script_text(assets);
    field_text.insert(field_text.end(), notebook_tips.begin(),
                      notebook_tips.end());
    write_dialogue_text_catalog(rom, root, assets, field_text);
    const auto write = [&](const std::string& name,
                           const shadowrun::native::NativeImage& image,
                           const std::string& kind,
                           const std::string& source, bool crop) {
        write_catalog_asset(root, name, to_rgba_image(image), kind, source,
                            crop, manifest, written, missing, blank);
    };
    const auto write_frame = [&](const std::string& name,
                                 const shadowrun::native::NativeEntityFrame& frame,
                                 const std::string& kind,
                                 const std::string& source, bool crop) {
        write(name,
              shadowrun::native::NativeImage{frame.width, frame.height,
                                             frame.pixels},
              kind, source, crop);
    };
    const auto write_array = [&](const std::string& prefix,
                                 const auto& images, const std::string& kind,
                                 const std::string& source, bool crop) {
        for (std::size_t index = 0; index < images.size(); ++index) {
            write(prefix + std::to_string(index) + ".bmp", images[index], kind,
                  source + "[" + std::to_string(index) + "]", crop);
        }
    };
    const auto write_rgba_array = [&](const std::string& prefix,
                                      const auto& images,
                                      const std::string& kind,
                                      const std::string& source, bool crop) {
        for (std::size_t index = 0; index < images.size(); ++index) {
            write_catalog_asset(
                root, prefix + std::to_string(index) + ".bmp", images[index],
                kind, source + "[" + std::to_string(index) + "]", crop,
                manifest, written, missing, blank);
        }
    };

    // Matrix map visuals are imported as direct host images. 0x532BC draws
    // the 40x28 background name table (0x15C4DC) from the patterns at
    // 0x15BEAC; its four CRAM lines are copied from 0x15FCB6. The same tile
    // upload supplies patterns 0xBF-0xD8 referenced by the 14x10 cell tables
    // at 0x15FC0E/0x15FC6A.
    constexpr std::size_t kMatrixMapPatternBase = 0x00BFU;
    constexpr std::size_t kMatrixMapPatternCount = 0x29U;
    constexpr std::size_t kMatrixMapPaletteBase = 0x15FCB6U;
    const auto matrix_backdrop_patterns =
        shadowrun::graphics::decode_tile_atlas(
            rom, 0x15BEACU, kMatrixMapPatternCount,
            shadowrun::graphics::read_genesis_palette(rom,
                                                       kMatrixMapPaletteBase),
            16U, 0);
    const auto matrix_map_backdrop = compose_matrix_map_backdrop(
        rom, matrix_backdrop_patterns, kMatrixMapPatternBase,
        kMatrixMapPatternCount);
    write_catalog_asset(
        root, "cyberspace/matrix/system-map-background.bmp",
        matrix_map_backdrop, "background",
        "0x532BC/0x15C4DC name table; 0x15BEAC patterns; CRAM 0x15FCB6",
        false, manifest, written, missing, blank);
    write_catalog_asset(
        root, "cyberspace/matrix/system-map-font.bmp",
        decode_matrix_map_font(rom, kMatrixMapPaletteBase), "sprite",
        "0x532BC/0xC0DA/0xC1320 glyphs; attribute 0x2000; "
        "CRAM 0x15FCB6 palette line 1",
        true, manifest, written, missing, blank);

    constexpr std::size_t kMatrixUiPatternCount = 199U;
    constexpr std::size_t kMatrixMenuPatternCount = 147U;
    constexpr std::size_t kMatrixFieldMenuPatternCount = 139U;
    constexpr std::size_t kMatrixPaletteBase = 0x0E2E3AU;
    // Matrix entry calls 0x17F80 -> 0x566E2, which copies these sixteen
    // palette words into FFD89E (CRAM line 1) after the common palette at
    // 0xE2E3A has been loaded. Extract Matrix UI tiles with the live palette.
    constexpr std::size_t kMatrixLiveUiPalette = 0x0E2EBAU;
    constexpr std::size_t kMatrixPanelPaletteLine = 3U;
    MatrixGraphicsUploadHistory matrix_upload_history{};
    const auto matrix_field_menu_resource = decode_matrix_graphics_upload(
        rom, 0xD86E0U, matrix_upload_history);
    const auto matrix_menu_resource = decode_matrix_graphics_upload(
        rom, 0xD78AAU, matrix_upload_history);
    if (matrix_menu_resource.size() != kMatrixMenuPatternCount * 32U) {
        throw std::runtime_error(
            "Matrix menu graphics stream no longer contains 147 tiles");
    }
    const auto matrix_menu_patterns = shadowrun::graphics::decode_tile_atlas(
        matrix_menu_resource, 0U, kMatrixMenuPatternCount,
        shadowrun::graphics::read_genesis_palette(
            rom, kMatrixPaletteBase + kMatrixPanelPaletteLine * 0x20U),
        16U, 0U);
    const auto matrix_ui_resource = decode_matrix_graphics_upload(
        rom, 0xD9376U, matrix_upload_history);
    if (matrix_ui_resource.size() != kMatrixUiPatternCount * 32U) {
        throw std::runtime_error(
            "Matrix UI graphics stream no longer contains 199 tiles");
    }
    // C0C2 loads the title-font stream after the D86E0/D78AA/D9376 uploads.
    // It shares FFBF40's 0x800-byte overlap history, so keep that order in
    // this offline extraction pass as well.
    const auto matrix_title_font_resource = decode_matrix_graphics_upload(
        rom, 0xC1064U, matrix_upload_history);
    if (matrix_title_font_resource.size() != 0x55U * 32U) {
        throw std::runtime_error(
            "Matrix title-font stream no longer contains 85 tiles");
    }
    constexpr std::size_t kMatrixFieldPatternCount = 304U;
    const auto matrix_field_resource =
        shadowrun::graphics::decode_genesis_resource_stream(rom, 0xD646AU);
    if (matrix_field_resource.size() != kMatrixFieldPatternCount * 32U) {
        throw std::runtime_error(
            "Matrix field-mode graphics stream no longer contains 304 tiles");
    }
    for (std::size_t theme = 0U; theme < 4U; ++theme) {
        write_catalog_asset(
            root,
            "cyberspace/matrix/traversal-background-theme-" +
                std::to_string(theme + 1U) + ".bmp",
            compose_matrix_traversal_background(
                rom, matrix_field_resource, matrix_menu_patterns, theme),
            "background",
            "0x1745E field-mode presentation: 0xD646A patterns; 0xD806C "
            "and 0xD74F4 Plane-B streams; 0xD8316 shared footer; "
            "palette 0x166448[" + std::to_string(theme) + "]",
            false, manifest, written, missing, blank);
        write_catalog_asset(
            root, "cyberspace/matrix/node-action-cursor-color-" +
                      std::to_string(theme + 1U) + ".bmp",
            compose_matrix_node_action_cursor(
                matrix_title_font_resource,
                shadowrun::graphics::read_genesis_palette(
                    rom, 0xE2D52U + theme * 0x20U)),
            "sprite",
            "0xC0C2 loads 0xC1064 at VRAM $B560; 0xC376 writes the selected "
            "cursor tile $85FF (pattern $5FF); 0x566D8 color palette line 0 "
            "from 0xE2D52[" + std::to_string(theme) + "]",
            false, manifest, written, missing, blank);
    }
    write_catalog_asset(
        root, "cyberspace/matrix/traversal-hud-frame.png",
        compose_matrix_traversal_hud_frame(rom, matrix_menu_patterns),
        "sprite",
        "0x1789C/0x178C8 writes the two 5-grid Plane-A status-frame batches "
        "from 0xD86C8; 147 Matrix menu tiles from 0xD78AA; palette line 3 "
        "from 0xE2E9A",
        false, manifest, written, missing, blank);
    write_catalog_asset(
        root, "cyberspace/matrix/traversal-runner-idle.png",
        compose_matrix_traversal_idle_runner(rom), "sprite",
        "0x1ADE6 attaches descriptor E; 0x1C34A/0x1C344 phase-zero lists; "
        "0xDA038 patterns at tile 0x11B; 0x1C506 origin FF158E/FF1590; "
        "palette line 3 at 0xE2E9A",
        false, manifest, written, missing, blank);
    write_catalog_asset(
        root,
        "cyberspace/matrix/traversal-runner-world-sequence-phase-4.png",
        compose_matrix_traversal_world_sequence_runner(rom), "sprite",
        "0x1D280 initializes the FFDF8C sequence figure at phase 0/countdown "
        "4; 0x1B364 draws the four 1DAF4/1DB08/EA490/EA4A4 groups; the "
        "239-VBlank compact entry retains phase 4; palette line 3 at 0xE2E9A",
        false, manifest, written, missing, blank);
    write_catalog_asset(
        root, "cyberspace/matrix/traversal-runner-action-phase-0.png",
        compose_matrix_traversal_action_runner_phase(rom, 0U), "sprite",
        "0x1B5B8 behavior A phase 0/0 before 0x1C3FA advances both layers; "
        "0x1C33A/0x1C330 lists use 0xDA038 tile 0x11B and 0xDAAB8 tile "
        "0x16F; origin FF158E/FF1590; palette line 3 at 0xE2E9A",
        false, manifest, written, missing, blank);
    write_catalog_asset(
        root, "cyberspace/matrix/traversal-runner-action-raised.png",
        compose_matrix_traversal_action_runner_phase(rom, 1U), "sprite",
        "0x1B5B8 behavior A phase 1/1 after 0x1C3FA advances both layers; "
        "0x1C33A/0x1C330 lists use 0xDA038 tile 0x11B and 0xDAAB8 tile "
        "0x16F; origin FF158E/FF1590; palette line 3 at 0xE2E9A",
        false, manifest, written, missing, blank);
    const auto matrix_action_pattern_bytes =
        shadowrun::graphics::decode_genesis_resource_stream(rom, 0xDAAB8U);
    if (matrix_action_pattern_bytes.empty() ||
        matrix_action_pattern_bytes.size() % 32U != 0U) {
        throw std::runtime_error(
            "Matrix Analyze effect pattern bank is invalid");
    }
    const auto matrix_action_atlas =
        shadowrun::graphics::decode_tile_atlas(
            matrix_action_pattern_bytes, 0U,
            matrix_action_pattern_bytes.size() / 32U,
            shadowrun::graphics::read_genesis_palette(rom, 0x0E2E5AU),
            16U, 0U);
    for (std::size_t phase = 0U; phase < 4U; ++phase) {
        write_catalog_asset(
            root,
            "cyberspace/matrix/traversal-analyze-ring-phase-" +
                std::to_string(phase) + ".png",
            compose_matrix_analyze_ring_phase(
                rom, matrix_action_atlas,
                matrix_action_pattern_bytes.size() / 32U, phase),
            "sprite",
            "0x1BE82 Analyze success starts 0x1BEBA's 60-tick profile "
            "transition; phase ((-FFE00C >> 1) & 3) selects 0x1C136; "
            "0xDAAB8 tile bank starts at 0x16F; D6=0x2000 selects traversal "
            "CRAM line 1 at 0xE2E5A; screen anchors (222,110)/(96,110)",
            false, manifest, written, missing, blank);
    }
    constexpr std::size_t kWorldEffectModeTable = 0x1A9CEU;
    constexpr std::size_t kWorldEffectModeCount = 9U;
    for (std::size_t mode = 0U; mode < kWorldEffectModeCount; ++mode) {
        const auto mode_record = static_cast<std::size_t>(
            read_big_endian_long(rom, kWorldEffectModeTable + mode * 4U));
        if (mode_record > rom.size() || rom.size() - mode_record < 6U) {
            throw std::runtime_error(
                "Matrix world-effect mode record is truncated");
        }
        const auto cadence = rom[mode_record];
        const auto phase_count = static_cast<std::size_t>(
            rom[mode_record + 1U]);
        for (std::size_t phase = 0U; phase < phase_count; ++phase) {
            write_catalog_asset(
                root,
                "cyberspace/matrix/traversal-world-effect-mode-" +
                    std::to_string(mode) + "-phase-" +
                    std::to_string(phase) + ".png",
                compose_matrix_world_effect_mode_phase(rom, mode, phase),
                "sprite",
                "0x1A908/0x19B40 mode selection; 0x1A9CE[" +
                    std::to_string(mode) + "] phase " +
                    std::to_string(phase) + " from its ROM descriptor table; "
                    "0x1AA40 cadence " + std::to_string(cadence) +
                    " ticks; screen anchor (160,80), D6=0x2000, CRAM line 1 "
                    "at 0xE2E5A",
                false, manifest, written, missing, blank);
        }
    }
    constexpr std::size_t kWorldEffectDefaultRecord = 0x1AA10U;
    if (kWorldEffectDefaultRecord > rom.size() ||
        rom.size() - kWorldEffectDefaultRecord < 6U) {
        throw std::runtime_error(
            "Matrix world-effect default record is truncated");
    }
    const auto default_cadence = rom[kWorldEffectDefaultRecord];
    const auto default_phase_count = static_cast<std::size_t>(
        rom[kWorldEffectDefaultRecord + 1U]);
    for (std::size_t phase = 0U; phase < default_phase_count; ++phase) {
        write_catalog_asset(
            root,
            "cyberspace/matrix/traversal-world-effect-default-phase-" +
                std::to_string(phase) + ".png",
            compose_matrix_world_effect_record_phase(
                rom, kWorldEffectDefaultRecord, phase),
            "sprite",
            "0x1A908 negative selector fallback at 0x1AA10, phase " +
                std::to_string(phase) + "; 0x1AA40 cadence " +
                std::to_string(default_cadence) +
                " ticks; screen anchor (160,80), D6=0x2000, CRAM line 1 "
                "at 0xE2E5A",
            false, manifest, written, missing, blank);
    }
    // The world-model SAT attributes select palette line 0. After the base
    // four-line load, 0x19DF2 leaves D0 at zero for an unknown node and
    // selects FF1105 for a known node. Both paths use the five 0x20-byte
    // records rooted at E2D52; live Matrix security colors use ids 1..4.
    struct WorldModelPalette {
        std::uint8_t security_color;
        const char* source_label;
        std::size_t source;
    };
    constexpr std::array<WorldModelPalette, 5> kWorldModelPalettes{{
        {0U, "e2d52", 0x0E2D52U}, {1U, "e2d72", 0x0E2D72U},
        {2U, "e2d92", 0x0E2D92U}, {3U, "e2db2", 0x0E2DB2U},
        {4U, "e2dd2", 0x0E2DD2U},
    }};
    for (std::size_t model = 0U; model < 6U; ++model) {
        for (const auto& palette : kWorldModelPalettes) {
            const auto relative_name =
                "cyberspace/matrix/world-model-" +
                std::to_string(model) + "-security-color-" +
                std::to_string(palette.security_color) + ".png";
            write_catalog_asset(
                root, relative_name,
                compose_matrix_world_model(rom, model, palette.source),
                "sprite",
                "0xE2EDA record " + std::to_string(model) +
                    " supplies the pixel dimensions, tile map, and pattern "
                    "bank; 0x19DF2 selects security-color " +
                    std::to_string(palette.security_color) +
                    " from 0x" + palette.source_label,
                false, manifest, written, missing, blank);
        }
    }
    const auto matrix_ui_palette_one_patterns =
        shadowrun::graphics::decode_tile_atlas(
            matrix_ui_resource, 0U, kMatrixUiPatternCount,
            shadowrun::graphics::read_genesis_palette(
                rom, kMatrixLiveUiPalette),
            16U, 0U);
    write_catalog_asset(
        root, "cyberspace/matrix/cyberdeck-background.bmp",
        compose_matrix_cyberdeck_plane_b_background(
            rom, matrix_ui_palette_one_patterns, matrix_menu_patterns),
        "background",
        "0x181F8 writes D9C96 to Plane B at $E000 (palette line 1); "
        "0x17F80/0x566E2 replaces CRAM line 1 from 0xE2EBA; "
        "0x17708 writes D8316 at $EB00 (palette line 3); patterns from "
        "D9376 at $8160 and D78AA at $0580",
        false, manifest, written, missing, blank);
    write_catalog_asset(
        root, "cyberspace/matrix/cyberdeck-panel.bmp",
        compose_matrix_cyberdeck_panel(rom, matrix_menu_patterns),
        "background",
        "0x177CC uploads 0xD78AA to $0580; palette line 3 from 0xE2E3A; "
        "0x1882E/0x1897C repeat 19x18 tiles from 0xD8454 at $C126",
        false, manifest, written, missing, blank);
    write_catalog_asset(
        root, "cyberspace/matrix/cyberdeck-runner-detail-headers.bmp",
        compose_matrix_cyberdeck_runner_headers(
            rom, matrix_ui_palette_one_patterns),
        "background",
        "0x1A16A writes five-tile grids D9ED4/D9EDE at $C4A6/$C526 "
        "(152,72)/(152,80), live CRAM line 1 from 0xE2EBA",
        false, manifest, written, missing, blank);

    write_catalog_asset(
        root, "cyberspace/matrix/cyberdeck-cursor.bmp",
        compose_matrix_cyberdeck_cursor(rom,
                                        matrix_ui_palette_one_patterns),
        "sprite",
        "0x182C0 builds the cursor from 0xE31E8; tile 0x04C6 from 0xD9376 "
        "uses live CRAM line 1 from 0xE2EBA; screen origin is $0C,$14",
        false, manifest, written, missing, blank);

    for (std::size_t selected_row = 0U; selected_row < 4U; ++selected_row) {
        write_catalog_asset(
            root,
            "cyberspace/matrix/cyberdeck-selector-selected-row-" +
                std::to_string(selected_row) + ".bmp",
            compose_matrix_cyberdeck_selector(
                rom, matrix_ui_palette_one_patterns, selected_row),
            "background",
            "0x189D8 selects one of four rows from 0xD9FA8; pattern stream "
            "0xD9376 at $8160; live CRAM line 1 from 0xE2EBA",
            false, manifest, written, missing, blank);
    }

    // 0x196F2 rotates CRAM line 1 entries 8-11 every five field-loop frames.
    // Import each resulting phase once; the host chooses a named image from
    // the Lua menu phase and does not decode or simulate palette memory.
    for (std::size_t phase = 1U; phase < 4U; ++phase) {
        auto phase_palette = shadowrun::graphics::read_genesis_palette(
            rom, kMatrixLiveUiPalette);
        std::rotate(phase_palette.begin() + 8,
                    phase_palette.begin() + 8 +
                        static_cast<std::ptrdiff_t>(phase),
                    phase_palette.begin() + 12);
        const auto phase_patterns = shadowrun::graphics::decode_tile_atlas(
            matrix_ui_resource, 0U, kMatrixUiPatternCount, phase_palette,
            16U, 0U);
        const auto palette_provenance =
            "0x196F2 phase " + std::to_string(phase) +
            " rotates CRAM line 1 entries 8-11 from 0xE2EBA; "
            "five-frame cadence; source images composed offline";
        write_catalog_asset(
            root, "cyberspace/matrix/cyberdeck-background-phase-" +
                      std::to_string(phase) + ".bmp",
            compose_matrix_cyberdeck_plane_b_background(
                rom, phase_patterns, matrix_menu_patterns),
            "background", palette_provenance, false, manifest, written,
            missing, blank);
        write_catalog_asset(
            root, "cyberspace/matrix/cyberdeck-cursor-phase-" +
                      std::to_string(phase) + ".bmp",
            compose_matrix_cyberdeck_cursor(rom, phase_patterns), "sprite",
            palette_provenance + "; cursor source 0xE31E8", false,
            manifest, written, missing, blank);
        for (std::size_t selected_row = 0U; selected_row < 4U;
             ++selected_row) {
            write_catalog_asset(
                root, "cyberspace/matrix/cyberdeck-selector-selected-row-" +
                          std::to_string(selected_row) + "-phase-" +
                          std::to_string(phase) + ".bmp",
                compose_matrix_cyberdeck_selector(
                    rom, phase_patterns, selected_row),
                "background", palette_provenance +
                                  "; selected row data 0xD9FA8",
                false, manifest, written, missing, blank);
        }
    }

    if (matrix_field_menu_resource.size() !=
        kMatrixFieldMenuPatternCount * 32U) {
        throw std::runtime_error(
            "Matrix deck icon stream no longer contains 139 tiles");
    }
    const auto matrix_field_menu_patterns =
        shadowrun::graphics::decode_tile_atlas(
            matrix_field_menu_resource, 0U, kMatrixFieldMenuPatternCount,
            shadowrun::graphics::read_genesis_palette(
                rom, kMatrixPaletteBase + kMatrixPanelPaletteLine * 0x20U),
            16U, 0U);
    write_catalog_asset(
        root, "cyberspace/matrix/cyberdeck-program-icon-empty.bmp",
        compose_matrix_cyberdeck_empty_program_icon(
            rom, matrix_field_menu_patterns),
        "sprite",
        "0x17708 selects the six-word empty-slot record at 0xD92AA; "
        "pattern stream 0xD86E0 at $17E0; palette line 3 from 0xE2E3A",
        false, manifest, written, missing, blank);
    for (std::size_t icon = 0U; icon < 16U; ++icon) {
        write_catalog_asset(
            root, "cyberspace/matrix/program-icon-" +
                      std::to_string(icon) + ".bmp",
            compose_matrix_cyberdeck_program_icon(
                rom, matrix_field_menu_patterns, icon),
            "sprite",
            "0x187AA reads 3x2 records from 0xD92B6; pattern stream "
            "0xD86E0 at $17E0; palette line 3 from 0xE2E3A",
            false, manifest, written, missing, blank);
        write_catalog_asset(
            root, "cyberspace/matrix/detail-strip-" +
                      std::to_string(icon) + ".bmp",
            compose_matrix_cyberdeck_detail_strip(
                rom, matrix_ui_palette_one_patterns, icon),
            "sprite",
            "0x18886 selects one six-tile row from 0xD9EE8; pattern stream "
            "0xD9376 at $8160; live CRAM line 1 from 0xE2EBA",
            false, manifest, written, missing, blank);
    }
    for (std::size_t status = 0U; status < 7U; ++status) {
        write_catalog_asset(
            root, "cyberspace/matrix/cyberdeck-footer-slot-status-" +
                      std::to_string(status) + ".bmp",
            compose_matrix_cyberdeck_footer_slot_status(
                rom, matrix_menu_patterns, status),
            "sprite",
            "0x19856 selects three words from 0xD84D6 + status*6; "
            "pattern stream 0xD78AA at $0580; palette line 3 from 0xE2E3A",
            false, manifest, written, missing, blank);
    }

    for (std::size_t profile = 0U; profile < 5U; ++profile) {
        for (std::size_t theme = 0U; theme < 4U; ++theme) {
            std::ostringstream provenance;
            provenance << "0x54662 node layout 0x166330/0x166368 profile "
                       << profile << "; field-mode tiles 0xD646A and "
                       << "terminal tiles 0x15FF30; "
                       << "terminal grids 0x1604D0/0x160538/0x160558/0x160578; "
                       << "palette 0x166448[" << theme << "]";
            write_catalog_asset(
                root,
                "cyberspace/matrix/node-background-profile-" +
                    std::to_string(profile) + "-color-" +
                    std::to_string(theme + 1U) + ".bmp",
                compose_matrix_node_action_background(
                    rom, matrix_field_resource, profile, theme),
                "background", provenance.str(), false, manifest, written,
                missing, blank);
        }
    }

    for (std::size_t palette_line = 1U; palette_line <= 3U;
         ++palette_line) {
        const auto palette_offset =
            kMatrixMapPaletteBase + palette_line * 0x20U;
        const auto map_tiles = shadowrun::graphics::decode_tile_atlas(
            rom, 0x15BEACU, 26U,
            shadowrun::graphics::read_genesis_palette(rom, palette_offset),
            16U, 0);
        std::ostringstream provenance;
        provenance << "0x532BC/0x15BEAC patterns 0xBF-0xD8; map table "
                   << "0x15FC0E/0x15FC6A; palette 0x" << std::hex
                   << palette_offset;
        write_catalog_asset(
            root, "cyberspace/matrix/map-tiles-palette-" +
                      std::to_string(palette_line) + ".bmp",
            map_tiles, "sprite", provenance.str(),
            false, manifest, written, missing, blank);
    }

    // Startup and opening source layers.
    write_array("title/animation-", assets.title_animation_frames, "background",
                "native title source composition", false);
    write("menu/startup/background.bmp", assets.startup_background, "background",
          "0x10B5A/0xCDCE6", false);
    write("menu/startup/font.bmp", assets.startup_font, "sprite", "0xC1320", true);
    write("menu/startup/cursor.bmp", assets.startup_cursor, "sprite", "0xD29F6", true);
    write("intro/aircraft-plane-a.bmp", assets.intro_aircraft_plane_a, "sprite",
          "0x5B528/0x5B7D8", true);
    write("intro/trivid-background.bmp", assets.intro_trivid_background,
          "background", "0x5AA96", false);
    const auto intro_backgrounds =
        shadowrun::graphics::decode_genesis_intro_backgrounds(rom);
    for (std::size_t index = 0; index < intro_backgrounds.size(); ++index) {
        write_catalog_asset(root, "intro/background-" + std::to_string(index) +
                                  ".bmp",
                            intro_backgrounds[index], "background", "0x59C5E",
                            false, manifest, written, missing, blank);
    }
    const auto intro_sprites =
        shadowrun::graphics::decode_genesis_intro_sprite_resources(rom);
    for (std::size_t index = 0; index < intro_sprites.size(); ++index) {
        const auto& resource = intro_sprites[index];
        write_catalog_asset(
            root, "intro/sprites/" + resource.role + "-" +
                      std::to_string(resource.table_index) + "-" +
                      std::to_string(resource.sprite_index) + ".bmp",
            resource.image, "sprite",
            format_resource_source(resource),
            true, manifest, written, missing, blank, resource.source_origin_x,
            resource.source_origin_y, resource.source_width,
            resource.source_height);
    }
    const auto intro_patches =
        shadowrun::graphics::decode_genesis_intro_ambush_background_resources(
            rom);
    for (std::size_t index = 0; index < intro_patches.size(); ++index) {
        const auto& resource = intro_patches[index];
        write_catalog_asset(
            root, "intro/background-patch-" + std::to_string(index) + ".bmp",
            resource.image, "background",
            format_resource_source(resource),
            true, manifest, written, missing, blank);
    }
    write("intro/cab-map.bmp", assets.cab_map, "background", "0x134D2/0xD2A2E",
          false);
    // The cab selection outline is the 0xD4446 eight-tile border stamped by
    // 0x13726; it is a live composition per destination, not part of the
    // map background.
    {
        const auto cursor_tiles =
            shadowrun::graphics::decode_genesis_cab_cursor_tiles(rom);
        constexpr std::array<std::string_view, 8> kCursorNames{{
            "top-left", "top-middle", "top-right", "left", "right",
            "bottom-left", "bottom-middle", "bottom-right"}};
        for (std::size_t index = 0; index < cursor_tiles.tiles.size();
             ++index) {
            const auto& tile = cursor_tiles.tiles[index];
            // Keep the full 8x8 tile cell; the border compositor places
            // these on the 8-pixel grid and a cropped sliver would carry an
            // origin the grid drawing does not apply.
            write_frame("intro/cab/cursor-" +
                            std::string(kCursorNames[index]) + ".bmp",
                        shadowrun::native::NativeEntityFrame{
                            tile.width, tile.height, tile.pixels},
                        "sprite", "0xD4446/0x13726", false);
        }
    }

    // 0x1F65A selects profile 0 for Stoker's Coffin Motel. The portrait is
    // written by the shared 7x9 resource stream at 0x122420 after its
    // profile-specific pattern upload at 0x1287C6. Keep this as one direct
    // sprite asset; the Qt conversion owns only its screen placement.
    write_catalog_asset(
        root, "dialogue/portraits/stoker-motel.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121FA0U),
        "sprite", "0x1F65A/0x121FA0/0x1287C6/0x122420", false, manifest,
        written, missing, blank);
    // Scene 13's mode-0 entry in 0x12CC50 selects the 0x121E80 speaker
    // profile for the event-30 Jump House conversation. Keep this separate
    // from Stoker's field-exit profile: it is Gunderson's distinct ROM art.
    write_catalog_asset(
        root, "dialogue/portraits/gunderson-jump-house.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121E80U),
        "sprite", "0x12CC50/0x121E80/0x20990", false, manifest, written,
        missing, blank);
    // Scene 14's row 3 in the 0x12CC50 table selects the portrait at
    // 0x121F28 for the Wanderer conversation. The bodyguard and Vigore are
    // dialogue portrait art; neither is a default field actor on map 14.
    write_catalog_asset(
        root, "dialogue/portraits/vigore-jarl.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121F28U),
        "sprite", "0x12CC80/0x121F28/0x20990", false, manifest, written,
        missing, blank);
    // Scene 11's mode-2 entry uses profile 7 at 0x121FF4 for the Big Rhino
    // booth dialogue. This is portrait art, not a field actor sprite.
    write_catalog_asset(
        root, "dialogue/portraits/mortimer-reed.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121FF4U),
        "sprite", "0x12CC70/0x121FF4/0x20990", false, manifest, written,
        missing, blank);
    // Scene 12's mode-0 entry in the 0x12CC90 dialogue row points to Caleb
    // Brightmore's profile at 0x121E98. It is dialogue portrait art, not a
    // standing map actor.
    write_catalog_asset(
        root, "dialogue/portraits/caleb-brightmore.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121E98U),
        "sprite", "0x12CC90/0x121E98/0x20990", false, manifest, written,
        missing, blank);
    // Scene 19's non-special event-30 path at 0x20990 loads this portrait
    // profile with prompt record 0xF3FAC. The ROM narrative calls the person
    // an old man but does not give him a name.
    write_catalog_asset(
        root, "dialogue/portraits/jackals-lantern-old-man.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121E5CU),
        "sprite", "0x20990/0x20C78/0x121E5C/0xF3FAC", false, manifest,
        written, missing, blank);
    // The scene-2/event-4 indexed title service uses a different profile for
    // the Jump House arrival description. Keep its face as separate host art;
    // it is not the later booth-conversation portrait.
    write_catalog_asset(
        root, "dialogue/portraits/jump-house-entry.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121EBCU),
        "sprite", "0x20484/0x121EBC/0x110D1E", false, manifest, written,
        missing, blank);
    // Scene-6/event-0 services 2-4 use profile 0x121E14 for David Owlfeather.
    write_catalog_asset(
        root, "dialogue/portraits/david-owlfeather.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121E14U),
        "sprite", "0x53B78/0x53BCC/0x53CEA/0x121E14", false, manifest,
        written, missing, blank);
    // Scene-6/event-1 service 18 uses the separate Red Buffalo Woman profile.
    write_catalog_asset(
        root, "dialogue/portraits/red-buffalo-woman.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121F04U),
        "sprite", "0x54322/0x121F04", false, manifest, written, missing,
        blank);
    // Scene-4 event 27 at 0x53D5A uses profile 0x121EF8 for the Spirit Eyes
    // conversation. The separate Scene-47 continuation at 0x53F3E uses
    // profile 0x121EA4 for the feathered serpent Licourtrix.
    write_catalog_asset(
        root, "dialogue/portraits/spirit-eyes.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121EF8U),
        "sprite", "0x53D5A/0x121EF8/0xF4BD8", false, manifest, written,
        missing, blank);
    write_catalog_asset(
        root, "dialogue/portraits/licourtrix.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121EA4U),
        "sprite", "0x53F3E/0x121EA4/0xF4C44", false, manifest, written,
        missing, blank);
    // Scene-5/event-6 title service 12 loads profile 0x121E2C with the
    // Renraku Offices entry root at 0x110A2A.
    write_catalog_asset(
        root, "dialogue/portraits/renraku-offices-entry.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121E2CU),
        "sprite", "0x540B8/0x121E2C/0x110A2A", false, manifest, written,
        missing, blank);
    // Scene-3/event-2 title service 14 loads profile 0x122090 with the
    // Fuchi cyber-heart entry root at 0x11B7F4.
    write_catalog_asset(
        root, "dialogue/portraits/fuchi-offices-entry.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x122090U),
        "sprite", "0x5419A/0x122090/0x11B7F4", false, manifest, written,
        missing, blank);
    // Scene-0/event-3 title service 15 loads profile 0x121FC4 with the
    // Ogami departure prompt rooted at 0x1111DE.
    write_catalog_asset(
        root, "dialogue/portraits/gates-undersound-entry.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121FC4U),
        "sprite", "0x5425A/0x121FC4/0x1111DE", false, manifest, written,
        missing, blank);
    // Scene-2/event-6 title service 10 loads Boris' profile at 0x121F58.
    // Keep the 56x72 portrait as a direct host asset for the Lua dialogue.
    write_catalog_asset(
        root, "dialogue/portraits/boris-greenhouse.bmp",
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x121F58U),
        "sprite", "0x53F8C/0x121F58/0x1F246", false, manifest, written,
        missing, blank);
    struct VidPhonePortraitRecord {
        std::string_view file_name;
        std::size_t contact_index;
    };
    constexpr std::array<VidPhonePortraitRecord, 13> kVidPhonePortraits{{
        {"capt-quinton-jaymes", 0U},
        {"alesandro-hobbs", 1U},
        {"sharkey", 2U},
        {"kipp-david", 3U},
        {"wilma-temmenhoff", 4U},
        {"macklemann", 5U},
        {"gregory-wilns", 6U},
        {"mr-faradouchi", 7U},
        {"agira-tetsumi", 8U},
        {"henry-j-culver", 9U},
        {"max", 10U},
        {"chillicut", 11U},
        {"professor-jefferson", 12U},
    }};
    constexpr std::size_t kVidPhoneContactPointerTable = 0x109682U;
    constexpr std::size_t kVidPhoneProfilePointerInRecord = 10U;
    constexpr std::size_t kVidPhoneProfilePointerSize = 4U;
    constexpr std::size_t kVidPhoneProfilePointerEnd =
        kVidPhoneProfilePointerInRecord + kVidPhoneProfilePointerSize;
    for (const auto& portrait : kVidPhonePortraits) {
        const auto record_offset = static_cast<std::size_t>(
            read_big_endian_long(
                rom, kVidPhoneContactPointerTable + portrait.contact_index * 4U));
        if (record_offset > rom.size() ||
            rom.size() - record_offset < kVidPhoneProfilePointerEnd) {
            throw std::runtime_error(
                "Vid-Phone contact record exceeds the ROM image");
        }
        const auto profile_offset = static_cast<std::size_t>(
            read_big_endian_long(
                rom, record_offset + kVidPhoneProfilePointerInRecord));
        write_catalog_asset(
            root, "dialogue/portraits/vid-phone-" +
                      std::string(portrait.file_name) + ".bmp",
            shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
                rom, profile_offset),
            "sprite", "0x109682/contact-record-pointer-" +
                          std::to_string(portrait.contact_index) +
                          "/record+0x0A/0x1F246",
            false, manifest, written, missing, blank);
    }
    write_catalog_asset(
        root, "dialogue/panel/frame.bmp",
        shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
            rom, false),
        "background", "0x10984/0x10918/0xCAB0C/0xCECD8-0xCEFA4", false,
        manifest, written, missing, blank);
    write_catalog_asset(
        root, "dialogue/panel/frame-alternate-left-panel.bmp",
        shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
            rom, true),
        "background", "0x10984/0x10918/0xCAB0C/0xCEFF0 (FFF0AF)", false,
        manifest, written, missing, blank);
    constexpr std::array<std::string_view, 3> kDialogueButtonFiles{{
        "dialogue/panel/button-a.bmp",
        "dialogue/panel/button-b.bmp",
        "dialogue/panel/button-c.bmp",
    }};
    const auto dialogue_buttons =
        shadowrun::graphics::decode_genesis_field_dialogue_option_markers(rom);
    for (std::size_t index = 0; index < dialogue_buttons.size(); ++index) {
        write_catalog_asset(
            root, std::string(kDialogueButtonFiles[index]),
            dialogue_buttons[index], "sprite",
            "0x1F3C4/0x1232/0xCF11C-0xCF12C", false,
            manifest, written, missing, blank);
    }

    // Field HUD and menu source layers.
    write("field/hud/background.bmp", assets.field_hud, "background",
          "0x334C/0x39F2", false);
    const auto ares_mode_sprite =
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x1220C0U);
    write("field/mode-entry/ares-sprite.bmp",
          shadowrun::native::NativeImage{ares_mode_sprite.width,
                                         ares_mode_sprite.height,
                                         ares_mode_sprite.pixels},
          "sprite", "0x1FA78/0x1F246/0x1F292/0x1220C0", true);
    const auto renraku_arcology_mode_sprite =
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x1220A8U);
    write("field/mode-entry/renraku-arcology-sprite.bmp",
          shadowrun::native::NativeImage{
              renraku_arcology_mode_sprite.width,
              renraku_arcology_mode_sprite.height,
              renraku_arcology_mode_sprite.pixels},
          "sprite", "0x1FA78/0x1F246/0x1F292/0x1220A8", true);
    const auto little_chiba_mode_sprite =
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x12200CU);
    write("field/mode-entry/little-chiba-sprite.bmp",
          shadowrun::native::NativeImage{little_chiba_mode_sprite.width,
                                         little_chiba_mode_sprite.height,
                                         little_chiba_mode_sprite.pixels},
          "sprite", "0x1F830/0x1F246/0x1F292/0x12200C", true);
    const auto renraku_mode_sprite =
        shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
            rom, 0x12206CU);
    write("field/mode-entry/renraku-strike-team-sprite.bmp",
          shadowrun::native::NativeImage{renraku_mode_sprite.width,
                                         renraku_mode_sprite.height,
                                         renraku_mode_sprite.pixels},
          "sprite", "0x53EE2/0x1F246/0x1F292/0x12206C", true);
    write_array("field/hud/face-", assets.field_hud_faces, "sprite", "0x3404",
                true);
    write_array("field/hud/item-", assets.field_hud_item_blocks, "sprite",
                "0x3404", true);
    write_array("field/hud/unarmed-item-",
                assets.field_hud_unarmed_item_blocks, "sprite",
                "0x3404/0xD1860-0xD1878", true);
    const auto field_hud_party =
        shadowrun::graphics::decode_genesis_field_hud_party_assets(rom);
    for (std::size_t family = 0;
         family < field_hud_party.portraits.size(); ++family) {
        for (std::size_t palette = 0;
             palette < field_hud_party.portraits[family].size(); ++palette) {
            write_catalog_asset(
                root,
                "field/hud/party/face-" + std::to_string(family) +
                    "-palette-" + std::to_string(palette) + ".bmp",
                field_hud_party.portraits[family][palette], "sprite",
                "0x3404/0x35D4/0x3602", false, manifest, written, missing,
                blank);
        }
    }
    for (std::size_t item = 0; item < field_hud_party.items.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party.items[item].size(); ++palette) {
            write_catalog_asset(
                root,
                "field/hud/party/item-" + std::to_string(item + 1U) +
                    "-palette-" + std::to_string(palette) + ".bmp",
                field_hud_party.items[item][palette], "sprite",
                "0x3404/0xD4540/0x35F0", false, manifest, written, missing,
                blank);
        }
    }
    for (std::size_t item = 0;
         item < field_hud_party.unarmed_items.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party.unarmed_items[item].size(); ++palette) {
            write_catalog_asset(
                root,
                "field/hud/party/unarmed-item-" + std::to_string(item) +
                    "-palette-" + std::to_string(palette) + ".bmp",
                field_hud_party.unarmed_items[item][palette], "sprite",
                "0x3404/0xD1860-0xD1878/0x35F0", false, manifest, written,
                missing, blank);
        }
    }
    for (std::size_t spell = 0;
         spell < assets.field_hud_magic_item_blocks.size(); ++spell) {
        for (std::size_t palette = 0;
             palette < assets.field_hud_magic_item_blocks[spell].size();
             ++palette) {
            const auto filename = "field/hud/magic-item-" +
                std::to_string(spell) + "-palette-" +
                std::to_string(palette) + ".bmp";
            write(filename,
                  assets.field_hud_magic_item_blocks[spell][palette],
                  "sprite", "0x3404/0xD4EA0/0x3602", true);
        }
    }
    write_array("field/hud/delay-meter-",
                assets.field_hud_delay_meter_counters, "sprite",
                "0x3674/0x6AB3E", true);
    for (std::size_t profile = 0;
         profile < assets.field_hud_behavior_meter_counters.size(); ++profile) {
        write_array(
            profile == 0U ? "field/hud/behavior-meter-primary-"
                          : "field/hud/behavior-meter-companion-",
            assets.field_hud_behavior_meter_counters[profile], "sprite",
            profile == 0U ? "0x37EC/0x6ABEE" : "0x37EC/0x6AB96", true);
    }
    write_array("field/hud/vertical-counter-", assets.field_hud_vertical_counters,
                "sprite", "0x35D4", true);
    write_array("field/hud/horizontal-counter-",
                assets.field_hud_horizontal_counters, "sprite", "0x35D4", true);
    write_array("field/hud/ammo-counter-", assets.field_hud_ammo_counters,
                "sprite", "0x36FC", true);
    write("menu/pocket-secretary/background.bmp",
          assets.pocket_secretary_background, "background", "0x10A7E", false);
    write_catalog_asset(
        root, "menu/pocket-secretary/grid.bmp",
        shadowrun::graphics::decode_genesis_pocket_secretary_grid(rom),
        "background", "0xC3E6/0x12C5A/0xC1320/0xC1064", false, manifest,
        written, missing, blank);
    write("menu/field/background.bmp", assets.field_menu_background, "background",
          "0x10F50", false);
    for (std::size_t frame = 0; frame < 17U; ++frame) {
        write_catalog_asset(
            root, "menu/field/elevator-selector-background-frame-" +
                      std::to_string(frame) + ".bmp",
            shadowrun::graphics::
                decode_genesis_field_elevator_selector_background(rom, frame),
            "background",
            "0x58DE0/0x566D8/0x1530/0x1728;0x5906E frame=" +
                std::to_string(frame),
            false, manifest, written, missing, blank);
    }
    for (std::size_t floor = 0; floor < 5U; ++floor) {
        write_catalog_asset(
            root, "menu/field/elevator-selector-current-floor-" +
                      std::to_string(floor) + ".bmp",
            shadowrun::graphics::
                decode_genesis_field_elevator_selector_floor_layer(
                    rom, floor, false),
            "overlay", "0x58FCC/0x1B6100",
            false, manifest, written, missing, blank);
        write_catalog_asset(
            root, "menu/field/elevator-selector-selected-floor-" +
                      std::to_string(floor) + ".bmp",
            shadowrun::graphics::
                decode_genesis_field_elevator_selector_floor_layer(
                    rom, floor, true),
            "overlay", "0x58FA8/0x1B60E8",
            false, manifest, written, missing, blank);
    }
    for (const bool previous_floor_input : {true, false}) {
        for (const bool active_phase : {true, false}) {
            const auto direction = previous_floor_input ? "previous" : "next";
            const auto phase = active_phase ? "active" : "rest";
            write_catalog_asset(
                root,
                "menu/field/elevator-selector-feedback-" +
                    std::string(direction) + "-" + std::string(phase) +
                    ".bmp",
                shadowrun::graphics::
                    decode_genesis_field_elevator_selector_feedback_layer(
                        rom, previous_floor_input, active_phase),
                "overlay",
                active_phase
                    ? (previous_floor_input
                           ? "0x58EBC/0x1B615C/0x1B6138/0xE8BA"
                           : "0x58EBC/0x1B615C/0x1B6130/0xE63A")
                    : (previous_floor_input
                           ? "0x58F1E/0x1B6140/0x1B6154/0xE8BA"
                           : "0x58F1E/0x1B6140/0x1B614C/0xE63A"),
                false, manifest, written, missing, blank);
        }
    }
    write_catalog_asset(
        root, "menu/field/service-background.bmp",
        shadowrun::graphics::decode_genesis_field_service_background(rom),
        "background", "0x10BC4/0x10918/0x1232", false, manifest, written,
        missing, blank);
    const auto service_selection_tabs =
        shadowrun::graphics::decode_genesis_field_service_selection_tabs(rom);
    for (std::size_t index = 0; index < service_selection_tabs.size(); ++index) {
        write_catalog_asset(
            root, "menu/field/service-tabs-selected-" +
                      std::to_string(index) + ".bmp",
            service_selection_tabs[index], "sprite",
            "0x58B5E/0x58BB6/0xCE17E/0xCE22E", false, manifest,
            written, missing, blank);
    }
    write_catalog_asset(
        root, "menu/field/service-universal-clips.bmp",
        shadowrun::graphics::decode_genesis_field_service_universal_clips_icon(
            rom),
        "sprite", "0x586F6/0xCE28E/0xD29D6/0x18AC56", false, manifest,
        written, missing, blank);
    constexpr std::array<std::string_view, 8> kCyberwareIconIds{{
        "datajack", "cyber_eyes", "hand_razors", "spurs", "smartlink",
        "muscle_replacement", "dermal_plating", "wired_reflexes"}};
    for (std::size_t index = 0U; index < kCyberwareIconIds.size(); ++index) {
        write_catalog_asset(
            root, "menu/field/service-cyberware-" +
                      std::string(kCyberwareIconIds[index]) + ".bmp",
            decode_field_service_cyberware_icon(rom, index), "sprite",
            "0x587DA/0x5895C/0x57EEE/0xD177C/0xCAB0C/0xD29D6/0x18AC56",
            false, manifest, written, missing, blank);
    }
    write("menu/field/action-background.bmp",
          assets.field_menu_action_background, "background", "0x117A0", false);
    write("menu/field/action-confirm-background.bmp",
          assets.field_menu_action_confirmation_background, "background",
          "0x127B0/0xCD3B6", false);
    write_catalog_asset(
        root, "menu/field/clips-background.bmp",
        shadowrun::graphics::decode_genesis_field_menu_clips_background(rom),
        "background", "0x11B58/0x11B98/0xCD33E", false, manifest, written,
        missing, blank);
    write_catalog_asset(
        root, "menu/field/clips-trade-background.bmp",
        shadowrun::graphics::decode_genesis_field_menu_clips_trade_background(
            rom),
        "background", "0x11E7E/0xCD37A", false, manifest, written, missing,
        blank);
    write("menu/field/attributes-background.bmp",
          assets.field_menu_attributes_background, "background", "0x10C96", false);
    write("menu/field/inventory-boxes.bmp", assets.field_menu_inventory_boxes,
          "background", "0x10F50/0x1232", false);
    write("menu/field/font.bmp", assets.field_menu_font, "sprite",
          "0xC1064 palette-line=3", true);
    write_catalog_asset(
        root, "menu/field/font-palette-0.bmp",
        shadowrun::graphics::decode_genesis_field_menu_font(rom, 0U),
        "sprite", "0xC1064/0x10276/0x102D4 palette-line=0", true,
        manifest, written, missing, blank);
    write_catalog_asset(
        root, "menu/field/magic-font-palette-0.bmp",
        shadowrun::graphics::decode_genesis_field_magic_menu_font(rom),
        "sprite", "0xFBA6/0xC8A60/0xC1064/0x10276 palette-line=0", true,
        manifest, written, missing, blank);
    write_catalog_asset(
        root, "menu/field/font-palette-2.bmp",
        shadowrun::graphics::decode_genesis_field_menu_font(rom, 2U),
        "sprite", "0xC1064/0xD29D6/0x18AC56 palette-line=2", true,
        manifest, written, missing, blank);
    write_array("menu/field/runner-portrait-", assets.field_menu_runner_portraits,
                "sprite", "0x7291A", true);
    std::vector<shadowrun::graphics::GenesisFieldEntityTemplate>
        runner_preview_records;
    runner_preview_records.reserve(13U);
    for (std::size_t index = 0; index < 3U; ++index) {
        runner_preview_records.push_back(
            shadowrun::graphics::read_genesis_field_entity_template(
                rom, 0x7291AU + index * 0x100U));
    }
    const auto recruited_preview_records =
        shadowrun::graphics::read_genesis_recruited_roster(rom);
    runner_preview_records.insert(runner_preview_records.end(),
                                  recruited_preview_records.begin(),
                                  recruited_preview_records.end());
    std::vector<std::pair<std::uint32_t, std::uint16_t>>
        emitted_preview_sources;
    for (const auto& record : runner_preview_records) {
        const auto source_key = std::pair{
            record.hud_portrait_table_pointer,
            record.hud_portrait_table_offset};
        if (std::find(emitted_preview_sources.begin(),
                      emitted_preview_sources.end(), source_key) !=
            emitted_preview_sources.end()) {
            continue;
        }
        emitted_preview_sources.push_back(source_key);
        const auto panels =
            shadowrun::graphics::decode_genesis_field_menu_dismiss_runner_preview_panels(
                rom, record);
        for (std::size_t palette_line = 0;
             palette_line < panels.size(); ++palette_line) {
            write_catalog_asset(
                root,
                "menu/field/runner-preview-table-" +
                    std::to_string(record.hud_portrait_table_pointer) +
                    "-offset-" +
                    std::to_string(record.hud_portrait_table_offset) +
                    "-palette-" + std::to_string(palette_line) + ".bmp",
                panels[palette_line], "sprite",
                "0x3404/0x6ACFA/0x110E8/0x110EC/0x03648", false, manifest,
                written, missing, blank);
        }
    }
    const auto random_grid_effects =
        shadowrun::graphics::decode_genesis_field_menu_random_grid_effects(
            rom);
    for (std::size_t index = 0; index < 3U; ++index) {
        write_catalog_asset(
            root,
            "menu/field/random-grid-fixed-" + std::to_string(index) +
                ".bmp",
            random_grid_effects[index], "background",
            "0xE660/0xC13E8/0xCE126/0xCE12E/0xCE136", false,
            manifest, written, missing, blank);
    }
    for (std::size_t index = 0; index < 4U; ++index) {
        write_catalog_asset(
            root,
            "menu/field/random-grid-indexed-" + std::to_string(index) +
                ".bmp",
            random_grid_effects[3U + index], "background",
            "0xE660/0xC13F8/0xCE13E", false, manifest, written, missing,
            blank);
    }
    write_array("menu/field/selected-cell-", assets.field_menu_selected_cells,
                "sprite", "0x11678", true);
    write_array("menu/field/command-focus-", assets.field_menu_command_focus,
                "sprite", "0x11454", true);
    write_array("menu/field/action-focus-", assets.field_menu_action_focus,
                "sprite", "0x117A4", true);
    write_array("menu/field/action-confirm-focus-",
                assets.field_menu_action_confirmation_focus, "sprite",
                "0x127FC/0xCD47A", true);
    write_array("menu/field/clips-focus-", assets.field_menu_clips_focus, "sprite",
                "0x11B58", true);
    write_array("menu/field/clips-trade-focus-", assets.field_menu_clips_trade_focus,
                "sprite", "0x11E7E", true);
    write("menu/field/posture-focus.bmp", assets.field_menu_posture_focus, "sprite",
          "0x119CE", true);
    write("menu/field/cursor.bmp", assets.field_menu_cursor, "sprite", "0x11592",
          true);
    write_catalog_asset(
        root, "menu/field/hotel-cursor.bmp",
        shadowrun::graphics::decode_genesis_hotel_options_cursor(rom),
        "sprite", "0xC0C2/0xC376/0xDA6A/0xC1064/0x5FF", true, manifest,
        written, missing, blank);
    write_catalog_asset(
        root, "menu/field/hotel-checkout-cursor.bmp",
        shadowrun::graphics::decode_genesis_hotel_options_cursor(rom, 2U),
        "sprite", "0xC0C2/0xC376/0xDA6A/0xC1064/0x5FF palette-line=2", true,
        manifest, written, missing, blank);
    for (std::size_t item = 0; item < assets.field_menu_item_art.size(); ++item) {
        const auto item_code = item + 1U;
        write(shadowrun::resources::item_detail_asset_path(item_code),
              assets.field_menu_item_art[item], "sprite",
              "0x12A8C/0xD4540[item=" + std::to_string(item_code) + "]", true);
    }
    for (std::size_t item = 0; item < assets.field_menu_inventory_icons.size();
         ++item) {
        for (std::size_t variant = 0;
             variant < assets.field_menu_inventory_icons[item].size(); ++variant) {
            write(shadowrun::resources::item_icon_asset_path(item + 1U, variant),
                  assets.field_menu_inventory_icons[item][variant], "sprite",
                  "0x35D4/0xD4540[item=" + std::to_string(item + 1U) +
                      "][variant=" + std::to_string(variant) + "]", true);
        }
    }
    write_array("menu/field/status-bar-", assets.field_menu_status_bars, "sprite",
                "0x11928", true);
    write_array("menu/field/damage-bar-", assets.field_menu_damage_bars, "sprite",
                "0x12F70", true);
    write_array("menu/field/drain-bar-", assets.field_menu_drain_bars, "sprite",
                "0x12FA4", true);
    write_array("menu/field/armor-summary-bar-",
                assets.field_menu_armor_summary_bars, "sprite",
                "0x11352/0xCF884", true);
    write_array("menu/field/detail-value-", assets.field_menu_detail_values, "sprite",
                "0x13000", true);
    write_array("menu/field/attack-bar-", assets.field_menu_attack_bars, "sprite",
                "0x12CEE/0xCF3D4", true);
    write_array("menu/field/defense-bar-", assets.field_menu_defense_bars, "sprite",
                "0x12CEE/0xCF610", true);
    write_catalog_asset(
        root, "menu/field/magic-panel-background.bmp",
        shadowrun::graphics::decode_genesis_field_magic_panel_background(rom),
        "background", "0xFBA6/0xC6714/0xC8108", false,
        manifest, written, missing, blank);
    write_rgba_array(
        "menu/field/magic-metric-bar-",
        shadowrun::graphics::decode_genesis_field_magic_metric_bars(rom),
        "sprite", "0x1055E/0xC8784/0xC8A60", false);
    write_rgba_array(
        "menu/field/magic-spell-icon-",
        shadowrun::graphics::decode_genesis_field_magic_spell_icons(rom),
        "sprite", "0xFBA6/0xFCB4/0x11190", false);
    write_rgba_array(
        "menu/field/magic-cursor-frame-",
        shadowrun::graphics::decode_genesis_field_magic_cursor_frames(rom),
        "sprite", "0xFDFE/0xC8B3E/0x1378", false);
    write_rgba_array(
        "menu/field/magic-posture-bar-",
        shadowrun::graphics::decode_genesis_field_magic_posture_bars(rom),
        "sprite", "0x107CE/0xC8600", false);
    write_rgba_array(
        "menu/field/magic-force-meter-tile-",
        shadowrun::graphics::decode_genesis_field_magic_force_meter_tiles(rom),
        "sprite", "0x10172/0xC85A2", false);
    write_rgba_array(
        "menu/field/magic-availability-bar-",
        shadowrun::graphics::decode_genesis_field_magic_availability_bars(rom),
        "sprite", "0x1088C/0xC86C0", false);
    write_rgba_array(
        "menu/field/magic-damage-bar-",
        shadowrun::graphics::decode_genesis_field_magic_damage_bars(rom),
        "sprite", "0x105E6/0xC86F8", false);

    // Field maps are backgrounds; entity and walking frames are sprites.
    // The mode-2 map is already the first entry in field_areas; emitting the
    // legacy field_map field as a second row would overwrite the same asset
    // path and make the catalog appear to contain a duplicate resource.
    for (const auto& area : assets.field_areas) {
        write("field/maps/mode-" + std::to_string(area.mode_id) + ".bmp",
              area.map, "background", "mode resource catalog", false);
        write("field/maps/mode-" + std::to_string(area.mode_id) +
                  "-priority-overlay.png",
              area.priority_overlay, "priority-overlay",
              "ROM field tile priority bits", false);
        write("field/maps/mode-" + std::to_string(area.mode_id) +
                  "-primary-plane.png",
              area.source_primary_plane, "map-source-plane",
              "ROM primary field tilemap plane", false);
        write("field/maps/mode-" + std::to_string(area.mode_id) +
                  "-secondary-plane.png",
              area.source_secondary_plane, "map-source-plane",
              "ROM secondary field tilemap plane", false);
    }

    // Keep each rendered map tied to the descriptor that selected it. This is
    // separate from catalog.tsv: that file describes host images, while this
    // index audits the complete ROM table used by 0x3074.
    std::ofstream map_index(root / "field/maps/index.tsv", std::ios::binary);
    if (!map_index) {
        throw std::runtime_error("cannot create field map resource index");
    }
    map_index
        << "mode_id\ttable_entry\trecord_offset\tcamera_x_fixed"
           "\tcamera_y_fixed\tbound_x\tbound_y\tmin_x\tmin_y"
           "\tterrain_record\tsetup_block\tpalette\tpattern\tpattern_bytes"
           "\tmetatile_stream\tprimary_tilemap\tsecondary_tilemap"
           "\ttilemap_width\ttilemap_height\tmetatile_encoding"
           "\tevent_table\tobject_pattern\tobject_vram"
           "\tobject_pattern_bytes\tasset\timage_width\timage_height"
           "\tpriority_overlay_asset\tprimary_plane_asset"
           "\tsecondary_plane_asset\n";
    const auto write_hex = [&map_index](std::size_t value) {
        map_index << "0x" << std::uppercase << std::hex << value << std::dec;
    };
    for (std::uint16_t mode_id = 0;
         mode_id < shadowrun::rom::kFieldModeCount; ++mode_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(rom, mode_id);
        const auto area = std::find_if(
            assets.field_areas.begin(), assets.field_areas.end(),
            [mode_id](const auto& candidate) { return candidate.mode_id == mode_id; });
        if (area == assets.field_areas.end()) {
            throw std::runtime_error("field map index is missing mode " +
                                     std::to_string(mode_id));
        }
        map_index << mode_id << '\t';
        write_hex(shadowrun::rom::kFieldModePointerTable +
                  static_cast<std::size_t>(mode_id) * 4U);
        map_index << '\t';
        write_hex(resources.record_offset);
        map_index << '\t' << resources.initial_camera_x_fixed << '\t'
                  << resources.initial_camera_y_fixed << '\t'
                  << resources.terrain_x_exclusive_upper << '\t'
                  << resources.terrain_y_exclusive_upper << '\t'
                  << resources.terrain_x_inclusive_lower << '\t'
                  << resources.terrain_y_inclusive_lower << '\t';
        write_hex(resources.terrain_record_offset);
        map_index << '\t';
        write_hex(resources.setup_block_offset);
        map_index << '\t';
        write_hex(resources.palette_offset);
        map_index << '\t';
        write_hex(resources.pattern_data_offset);
        map_index << '\t' << resources.pattern_byte_count << '\t';
        write_hex(resources.metatile_stream_offset);
        map_index << '\t';
        write_hex(resources.primary_tilemap_offset);
        map_index << '\t';
        write_hex(resources.secondary_tilemap_offset);
        map_index << '\t' << resources.tilemap_width << '\t'
                  << resources.tilemap_height << '\t'
                  << (resources.metatile_encoding ==
                              shadowrun::graphics::GenesisMetatileEncoding::RawNameWords
                          ? "raw-name-words"
                          : "compressed-resource")
                  << '\t';
        write_hex(resources.event_table_offset);
        map_index << '\t';
        write_hex(resources.object_pattern_data_offset);
        map_index << '\t';
        write_hex(resources.object_pattern_vram_offset);
        map_index << '\t' << resources.object_pattern_byte_count
                  << "\tfield/maps/mode-" << mode_id << ".bmp\t"
                  << area->map.width << '\t' << area->map.height
                  << "\tfield/maps/mode-" << mode_id
                  << "-priority-overlay.png\tfield/maps/mode-" << mode_id
                  << "-primary-plane.png\tfield/maps/mode-" << mode_id
                  << "-secondary-plane.png\n";
    }
    map_index.flush();

    // ROM 0x3084 copies the selected scene descriptor into RAM. Its dword at
    // byte offset 0x2A becomes FFDDF0, the profile-selected default actor-list
    // pointer consumed by 0x43B6. Import every descriptor/list pair offline;
    // the viewer can then distinguish empty lists from maps not yet profiled.
    struct DefaultActorSceneList {
        std::uint16_t scene_id;
        std::uint32_t profile_offset;
        std::uint32_t list_pointer_address;
        std::size_t list_offset;
        std::size_t actor_count = 0;
        std::string actor_types;
    };
    struct DefaultActorRecord {
        std::uint16_t scene_id;
        std::uint16_t source_order;
        std::uint16_t entity_kind;
        std::string actor_name;
        std::string profile_id;
        std::string sprite_stem;
        std::string action_kind;
        std::uint8_t initial_action_code;
        std::uint16_t world_x;
        std::uint16_t world_y;
        std::uint16_t initial_heading;
        std::uint8_t palette_line;
        std::uint8_t physical_meter;
        std::uint32_t template_offset;
        std::uint32_t position_offset;
        std::size_t list_offset;
        std::string overrides;
    };
    std::vector<DefaultActorSceneList> default_actor_lists;
    default_actor_lists.reserve(shadowrun::rom::kFieldModeCount);
    for (std::uint16_t scene_id = 0U;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(rom, scene_id);
        const auto list_pointer_address =
            static_cast<std::uint32_t>(resources.record_offset + 0x2AU);
        const auto list_offset = read_big_endian_long(rom, list_pointer_address);
        default_actor_lists.push_back(DefaultActorSceneList{
            scene_id,
            static_cast<std::uint32_t>(resources.record_offset),
            list_pointer_address,
            list_offset,
        });
    }
    const auto format_offset = [](const std::uint32_t value) {
        std::ostringstream text;
        text << "0x" << std::uppercase << std::hex << value;
        return text.str();
    };
    const auto format_byte = [](const std::uint32_t value) {
        std::ostringstream text;
        text << std::nouppercase << std::hex << std::setw(2)
             << std::setfill('0') << value;
        return text.str();
    };
    const auto read_actor_name = [&rom](const std::size_t template_offset) {
        std::string name;
        for (std::size_t index = 0U; index < 13U; ++index) {
            const auto character = rom[template_offset + 0x0AU + index];
            if (character == 0U) {
                break;
            }
            if (character >= 0x20U && character <= 0x7EU) {
                name.push_back(static_cast<char>(character));
            }
        }
        while (!name.empty() && name.back() == ' ') {
            name.pop_back();
        }
        return name;
    };
    const auto make_sprite_stem = [](const std::string_view name) {
        std::string stem;
        bool separator = false;
        for (const auto character : name) {
            const auto ascii = static_cast<unsigned char>(character);
            if ((ascii >= 'a' && ascii <= 'z') ||
                (ascii >= 'A' && ascii <= 'Z') ||
                (ascii >= '0' && ascii <= '9')) {
                if (separator && !stem.empty()) {
                    stem.push_back('-');
                }
                stem.push_back(static_cast<char>(
                    ascii >= 'A' && ascii <= 'Z' ? ascii - 'A' + 'a' : ascii));
                separator = false;
            } else {
                separator = true;
            }
        }
        return stem;
    };
    const auto make_actor_profile_id = [&format_byte](
        const std::vector<std::uint8_t>& source_rom,
        const std::size_t template_offset, const std::uint16_t entity_kind,
        const std::string_view action_kind,
        const std::uint8_t action_code) {
        std::string slug;
        bool previous_was_separator = false;
        for (std::size_t index = 0; index < 13U; ++index) {
            auto character = source_rom[template_offset + 0x0AU + index];
            if (character >= 'A' && character <= 'Z') {
                character = static_cast<std::uint8_t>(character - 'A' + 'a');
            }
            if ((character >= 'a' && character <= 'z') ||
                (character >= '0' && character <= '9')) {
                slug.push_back(static_cast<char>(character));
                previous_was_separator = false;
            } else if (!slug.empty() && !previous_was_separator) {
                slug.push_back('_');
                previous_was_separator = true;
            }
        }
        while (!slug.empty() && slug.back() == '_') {
            slug.pop_back();
        }
        if (slug.empty()) {
            throw std::runtime_error(
                "default actor template has no script-safe profile name");
        }
        return slug + "_k" + format_byte(entity_kind) + "_" +
               std::string(action_kind) + format_byte(action_code);
    };
    std::vector<DefaultActorRecord> default_actor_records;
    for (auto& source_list : default_actor_lists) {
        auto cursor = source_list.list_offset;
        const auto first_record = default_actor_records.size();
        std::map<std::string, std::size_t> type_counts;
        for (std::uint16_t source_order = 0U; source_order < 32U;
             ++source_order) {
            const auto tagged_template =
                read_big_endian_long(rom, cursor);
            if (tagged_template == 0U) {
                break;
            }
            const auto template_offset = tagged_template & 0x7FFFFFFFU;
            const auto position_offset =
                read_big_endian_long(rom, cursor + 4U);
            // 0x44BC copies through template byte +0x49. Byte +0x4C is not
            // part of this template; the next compact template starts nearby.
            if (template_offset > rom.size() ||
                rom.size() - template_offset < 0x4AU) {
                throw std::runtime_error(
                    "default field actor template pointer exceeds ROM");
            }
            const auto entity_kind = read_big_endian_word(rom, template_offset);
            const auto actor_name = read_actor_name(template_offset);
            if (actor_name.empty()) {
                throw std::runtime_error(
                    "default field actor template has no display name");
            }
            const auto action_table_selector = rom[template_offset + 8U];
            // ROM 0x44BC copies template byte +9 into live entity +0x57.
            // This is the one-based active item/action code used by the
            // entity behavior and item-row selectors; it is not an AI guess.
            const auto initial_action_code = rom[template_offset + 9U];
            const auto action_kind = initial_action_code == 0U
                ? std::string("none")
                : ((action_table_selector & 0x80U) != 0U
                       ? std::string("spell")
                       : std::string("item"));
            const auto profile_id = make_actor_profile_id(
                rom, template_offset, entity_kind, action_kind,
                initial_action_code);
            // The actor-name slug is not a unique visual key. Keep the ROM
            // action profile ID as the asset stem, matching the conditional
            // actor and encounter extractors below.
            const auto sprite_stem = profile_id;
            const auto palette_line = static_cast<std::uint8_t>(
                (read_big_endian_word(rom, template_offset + 2U) >> 13U) &
                0x03U);
            DefaultActorRecord record{
                source_list.scene_id,
                source_order,
                entity_kind,
                actor_name,
                profile_id,
                sprite_stem,
                action_kind,
                initial_action_code,
                read_big_endian_word(rom, position_offset),
                read_big_endian_word(rom, position_offset + 2U),
                read_big_endian_word(rom, 0x7361AU + 0x2EU),
                palette_line,
                // ROM 0x44BC maps template +0x04 to live entity +0x4C.
                rom[template_offset + 0x04U],
                template_offset,
                position_offset,
                source_list.list_offset,
                "-",
            };
            ++type_counts[actor_name];

            auto next_record = cursor + 8U;
            if (read_big_endian_long(rom, next_record) == 2U) {
                auto pair_offset = next_record + 4U;
                std::ostringstream overrides;
                bool first_override = true;
                for (;;) {
                    const auto pair = read_big_endian_word(rom, pair_offset);
                    if (pair == 0U) {
                        pair_offset += 2U;
                        break;
                    }
                    if (!first_override) {
                        overrides << ',';
                    }
                    const auto byte_offset =
                        static_cast<std::uint8_t>(pair >> 8U);
                    const auto byte_value =
                        static_cast<std::uint8_t>(pair & 0x00FFU);
                    overrides << format_offset(byte_offset) << '='
                              << format_offset(byte_value);
                    if (byte_offset == 0x2EU) {
                        record.initial_heading = static_cast<std::uint16_t>(
                            (static_cast<std::uint16_t>(byte_value) << 8U) |
                            (record.initial_heading & 0x00FFU));
                    } else if (byte_offset == 0x2FU) {
                        record.initial_heading = static_cast<std::uint16_t>(
                            (record.initial_heading & 0xFF00U) | byte_value);
                    } else if (byte_offset == 0x4CU) {
                        record.physical_meter = byte_value;
                    }
                    first_override = false;
                    pair_offset += 2U;
                }
                if (!first_override) {
                    record.overrides = overrides.str();
                }
                next_record = pair_offset;
            }
            default_actor_records.push_back(std::move(record));
            cursor = next_record;
        }
        if (read_big_endian_long(rom, cursor) != 0U) {
            throw std::runtime_error(
                "profile-selected default actor list has no null sentinel");
        }
        source_list.actor_count = default_actor_records.size() - first_record;
        std::ostringstream type_summary;
        bool first_type = true;
        for (const auto& [name, count] : type_counts) {
            if (!first_type) {
                type_summary << ", ";
            }
            type_summary << count << " x " << name;
            first_type = false;
        }
        source_list.actor_types = type_summary.str();
    }

    {
        const auto actor_table_path = root / "field/entities/default-actors.tsv";
        std::filesystem::create_directories(actor_table_path.parent_path());
        std::ofstream actor_table(actor_table_path, std::ios::binary);
        if (!actor_table) {
            throw std::runtime_error(
                "cannot create profile-selected default actor table");
        }
        actor_table << "scene\tsource_order\tentity_kind\tprofile_id"
                       "\tactor_name\tsprite_stem"
                       "\taction_kind\tinitial_action_code"
                       "\tworld_x\tworld_y"
                       "\tinitial_heading\tpalette_line\ttemplate_offset\tposition_offset"
                       "\tlist_offset\tordered_overrides\tphysical_meter\n";
        for (const auto& actor : default_actor_records) {
            actor_table << actor.scene_id << '\t' << actor.source_order << '\t'
                        << actor.entity_kind << '\t' << actor.profile_id
                        << '\t' << actor.actor_name << '\t' << actor.sprite_stem
                        << '\t' << actor.action_kind << '\t'
                        << static_cast<unsigned>(actor.initial_action_code)
                        << '\t' << actor.world_x << '\t'
                        << actor.world_y << '\t'
                        << actor.initial_heading << '\t'
                        << static_cast<unsigned>(actor.palette_line) << '\t'
                        << format_offset(actor.template_offset) << '\t'
                        << format_offset(actor.position_offset) << '\t'
                        << format_offset(static_cast<std::uint32_t>(
                               actor.list_offset))
                        << '\t' << actor.overrides << '\t'
                        << static_cast<unsigned>(actor.physical_meter) << '\n';
        }
        if (!actor_table) {
            throw std::runtime_error(
                "cannot write profile-selected default actor table");
        }
    }

    {
        const auto list_table_path =
            root / "field/entities/default-actor-lists.tsv";
        std::filesystem::create_directories(list_table_path.parent_path());
        std::ofstream list_table(list_table_path, std::ios::binary);
        if (!list_table) {
            throw std::runtime_error(
                "cannot create profile-selected actor-list index");
        }
        list_table << "scene\tprofile_offset\tlist_pointer_address"
                      "\tlist_offset\tactor_count\tactor_types\n";
        for (const auto& list : default_actor_lists) {
            list_table << list.scene_id << '\t'
                       << format_offset(list.profile_offset) << '\t'
                       << format_offset(list.list_pointer_address) << '\t'
                       << format_offset(static_cast<std::uint32_t>(list.list_offset))
                       << '\t' << list.actor_count << '\t'
                       << (list.actor_types.empty() ? "-" : list.actor_types)
                       << '\n';
        }
        if (!list_table) {
            throw std::runtime_error(
                "cannot write profile-selected actor-list index");
        }
    }

    // Actor position pointers reference a zero-word-terminated sequence of
    // 12.4 world coordinates. Preserve the complete ordered waypoint lists
    // as named offline data; the runtime and map viewer never chase ROM
    // pointers. A zero X word terminates one list, as in the source walker.
    {
        const auto waypoint_table_path =
            root / "field/entities/default-actor-waypoints.tsv";
        std::filesystem::create_directories(waypoint_table_path.parent_path());
        std::ofstream waypoint_table(waypoint_table_path, std::ios::binary);
        if (!waypoint_table) {
            throw std::runtime_error(
                "cannot create profile-selected actor waypoint table");
        }
        waypoint_table
            << "scene\tsource_order\twaypoint_order\tactor_name"
               "\tposition_list_address\tworld_x\tworld_y"
               "\tterminator_rule\n";
        for (const auto& actor : default_actor_records) {
            bool found_terminator = false;
            for (std::uint16_t waypoint_order = 0U; waypoint_order < 64U;
                 ++waypoint_order) {
                const auto point_offset =
                    static_cast<std::size_t>(actor.position_offset) +
                    static_cast<std::size_t>(waypoint_order) * 4U;
                if (point_offset > rom.size() ||
                    rom.size() - point_offset < 2U) {
                    break;
                }
                const auto world_x = read_big_endian_word(rom, point_offset);
                if (world_x == 0U) {
                    found_terminator = true;
                    break;
                }
                if (rom.size() - point_offset < 4U) {
                    break;
                }
                const auto world_y =
                    read_big_endian_word(rom, point_offset + 2U);
                waypoint_table << actor.scene_id << '\t'
                               << actor.source_order << '\t'
                               << waypoint_order << '\t' << actor.actor_name
                               << '\t' << format_offset(actor.position_offset)
                               << '\t' << world_x << '\t' << world_y
                               << "\tzero_x_word\n";
            }
            if (!found_terminator) {
                throw std::runtime_error(
                    "profile-selected actor waypoint list has no zero-X terminator");
            }
        }
        if (!waypoint_table) {
            throw std::runtime_error(
                "cannot write profile-selected actor waypoint table");
        }
    }

    struct WorldActivityActorList {
        std::uint16_t scene_id;
        std::int16_t script_index;
        std::uint32_t list_offset;
        std::string list_id;
        std::string activation_rule;
        std::int8_t world_effect_active_gate = -1;
        std::int8_t pending_world_effect_gate = -1;
    };
    struct WorldActivityActorTemplateFacts {
        std::uint8_t source_spawn_state_flags = 0U;
        std::uint8_t source_interaction_flags = 0U;
        std::uint8_t physical_meter = 0U;
        std::uint8_t delay_primary_attribute = 0U;
        std::uint8_t delay_secondary_attribute = 0U;
        std::uint16_t delay_capability_flags = 0U;
        std::uint8_t mental_meter = 0U;
        std::uint16_t cyberware_flags = 0U;
        std::uint8_t posture = 0U;
        std::uint8_t attribute_77 = 0U;
        std::uint8_t attribute_78 = 0U;
        std::uint8_t attribute_79 = 0U;
        std::uint8_t attribute_7b = 0U;
        std::uint8_t attribute_7c = 0U;
        std::uint8_t attribute_85 = 0U;
        std::uint8_t item18_base = 0U;
        std::uint8_t armor_index = 0U;
        std::int8_t race_or_state = 0;
        std::uint8_t class_code = 0U;
        std::uint8_t modifier_flags_df = 0U;
        std::uint16_t profile_index_a0 = 0U;
        std::array<std::uint8_t, 8> item_codes{};
        std::array<std::uint8_t, 8> item_phases{};
        std::array<std::uint8_t, 10> action_phase_extension{};
        std::array<std::uint8_t, 4> action_skills{};
        std::uint8_t auxiliary_roll_bonus = 0U;
    };
    struct WorldActivityActorRecord {
        std::uint16_t scene_id;
        std::int16_t script_index;
        std::string list_id;
        std::uint32_t list_offset;
        std::uint16_t source_order;
        std::uint32_t source_record_offset;
        std::uint32_t tagged_template;
        std::uint32_t template_offset;
        std::uint32_t position_offset;
        std::uint16_t entity_kind;
        std::string actor_name;
        std::string profile_id;
        std::string sprite_stem;
        std::uint8_t action_code;
        std::uint8_t source_action_slot;
        std::uint8_t source_action_ammo;
        std::uint8_t source_action_counter;
        std::string action_kind;
        std::uint8_t palette_line;
        std::uint16_t world_x;
        std::uint16_t world_y;
        std::uint16_t heading;
        std::string overrides;
        std::string activation_rule;
        std::int8_t world_effect_active_gate = -1;
        std::int8_t pending_world_effect_gate = -1;
        WorldActivityActorTemplateFacts template_facts;
    };
    const std::array<WorldActivityActorList, 14> kWorldActivityActorLists{{
        {0x0AU, 0, 0x17018U, "scene_10_event_6_script",
         "0x167AE: FFFBF9=6 and neither runner marker is 3"},
        {0x0AU, 1, 0x17034U, "scene_10_default_script",
         "0x167AE: FFFBF9!=6 and neither runner marker is 5"},
        {0x0BU, 1, 0x17050U, "scene_11_script",
         "0x167AE: neither runner marker is 2"},
        {0x0DU, 3, 0x1706CU, "scene_13_ilene_gate_script",
         "0x167AE: FFFC1E bit 19 (ILENE_MET) set and neither runner marker is 7"},
        {0x0EU, 4, 0x17088U, "scene_14_script",
         "0x167AE: neither runner marker is 4"},
        {0x0FU, 5, 0x170A2U, "scene_15_script",
         "0x167AE: FFFC1E bit 29 set and neither runner marker is 10"},
        {0x10U, 6, 0x170BCU, "scene_16_script",
         "0x167AE: neither runner marker is 6"},
        {0x11U, 7, 0x170D8U, "scene_17_script",
         "0x167AE: neither runner marker is 8"},
        {0x12U, 8, 0x170F4U, "scene_18_script",
         "0x167AE: neither runner marker is 9"},
        {0x13U, 9, 0x16FFCU, "scene_19_script",
         "0x167AE: neither runner marker is 1"},
        {0x14U, -1, 0xA8CCU, "scene_20_ilene_pending_list",
         "0xA800: FFE180 clear and FFE182 set", 0, 1},
        {0x2FU, -1, 0x1E552U, "mode_13_entry",
         "0x2CE0 mode 0x13 transition, with FFFC1E bit 8 clear"},
        {0x2FU, -1, 0x1E5C4U, "pending_list_1e5c4",
         "0x43B6 pending-list continuation for pointer 0x1E5C4"},
        {0x2FU, -1, 0x1E628U, "pending_list_1e628",
         "0x43B6 pending-list continuation for pointer 0x1E628"},
    }};
    const auto read_world_activity_actor_template_facts = [&rom](
        const std::size_t template_offset) {
        WorldActivityActorTemplateFacts facts{};
        facts.source_spawn_state_flags = rom[template_offset + 0x9EU];
        facts.source_interaction_flags = rom[
            shadowrun::rom::kPostObjectEntityTemplate + 0x11U];
        facts.physical_meter = rom[template_offset + 0x04U];
        facts.posture = rom[template_offset + 0x05U];
        facts.delay_primary_attribute = rom[template_offset + 0x2AU];
        facts.delay_secondary_attribute = rom[template_offset + 0x2DU];
        facts.delay_capability_flags = read_big_endian_word(
            rom, template_offset + 0x44U);
        facts.mental_meter = rom[template_offset + 0x43U];
        facts.cyberware_flags = facts.delay_capability_flags;
        facts.attribute_77 = rom[template_offset + 0x29U];
        facts.attribute_78 = rom[template_offset + 0x2AU];
        facts.attribute_79 = rom[template_offset + 0x2BU];
        facts.attribute_7b = rom[template_offset + 0x2DU];
        facts.attribute_7c = rom[template_offset + 0x2EU];
        facts.attribute_85 = rom[template_offset + 0x37U];
        facts.item18_base = rom[template_offset + 0x32U];
        facts.armor_index = rom[template_offset + 0x28U];
        facts.race_or_state = static_cast<std::int8_t>(
            rom[template_offset + 0x41U]);
        facts.class_code = rom[template_offset + 0x42U];
        facts.modifier_flags_df = rom[
            shadowrun::rom::kPostObjectEntityTemplate + 0xDFU];
        facts.profile_index_a0 = read_big_endian_word(
            rom, template_offset + 0x46U);
        for (std::size_t index = 0U; index < facts.item_codes.size(); ++index) {
            facts.item_codes[index] = rom[template_offset + 0x18U + index];
            facts.item_phases[index] = rom[template_offset + 0x20U + index];
        }
        for (std::size_t index = 0U;
             index < facts.action_phase_extension.size(); ++index) {
            facts.action_phase_extension[index] =
                rom[template_offset + 0x28U + index];
        }
        for (std::size_t index = 0U; index < facts.action_skills.size(); ++index) {
            facts.action_skills[index] = rom[template_offset + 0x33U + index];
        }
        facts.auxiliary_roll_bonus = rom[template_offset + 0x49U];
        return facts;
    };
    std::vector<WorldActivityActorRecord> world_activity_actor_records;
    for (const auto& list : kWorldActivityActorLists) {
        auto cursor = static_cast<std::size_t>(list.list_offset);
        bool found_terminator = false;
        for (std::uint16_t source_order = 0U; source_order < 32U;
             ++source_order) {
            const auto source_record_offset = cursor;
            const auto tagged_template = read_big_endian_long(rom, cursor);
            if (tagged_template == 0U) {
                found_terminator = true;
                break;
            }
            const auto template_offset = tagged_template & 0x7FFFFFFFU;
            const auto position_offset = read_big_endian_long(rom, cursor + 4U);
            if (template_offset > rom.size() ||
                rom.size() - template_offset < 0xA0U ||
                position_offset > rom.size() ||
                rom.size() - position_offset < 4U) {
                throw std::runtime_error(
                    "world-activity actor record points outside ROM");
            }
            auto template_facts =
                read_world_activity_actor_template_facts(template_offset);
            const auto actor_name = read_actor_name(template_offset);
            const auto entity_kind = read_big_endian_word(rom, template_offset);
            const auto palette_line = static_cast<std::uint8_t>(
                (read_big_endian_word(rom, template_offset + 2U) >> 13U) &
                0x03U);
            const auto action_selector = rom[template_offset + 8U];
            const auto action_code = rom[template_offset + 9U];
            const auto action_kind = action_code == 0U
                ? std::string("none")
                : ((action_selector & 0x80U) != 0U
                       ? std::string("spell")
                       : std::string("item"));
            const auto profile_id = make_actor_profile_id(
                rom, template_offset, entity_kind, action_kind, action_code);
            auto heading = read_big_endian_word(rom, 0x7361AU + 0x2EU);
            auto next_record = cursor + 8U;
            std::ostringstream overrides;
            bool first_override = true;
            if (read_big_endian_long(rom, next_record) == 2U) {
                auto pair_offset = next_record + 4U;
                for (;;) {
                    const auto pair = read_big_endian_word(rom, pair_offset);
                    if (pair == 0U) {
                        pair_offset += 2U;
                        break;
                    }
                    const auto byte_offset =
                        static_cast<std::uint8_t>(pair >> 8U);
                    const auto byte_value =
                        static_cast<std::uint8_t>(pair & 0x00FFU);
                    if (!first_override) {
                        overrides << ',';
                    }
                    overrides << format_offset(byte_offset) << '='
                              << format_offset(byte_value);
                    if (byte_offset == 0x2EU) {
                        heading = static_cast<std::uint16_t>(
                            (static_cast<std::uint16_t>(byte_value) << 8U) |
                            (heading & 0x00FFU));
                    } else if (byte_offset == 0x2FU) {
                        heading = static_cast<std::uint16_t>(
                            (heading & 0xFF00U) | byte_value);
                    } else if (byte_offset == 0x11U) {
                        template_facts.source_interaction_flags = byte_value;
                    } else if (byte_offset == 0x9EU) {
                        template_facts.source_spawn_state_flags = byte_value;
                    }
                    first_override = false;
                    pair_offset += 2U;
                }
                next_record = pair_offset;
            }
            const auto sprite_stem = entity_kind == 7U
                ? "actor-kind-7-palette-" + std::to_string(palette_line)
                : entity_kind == 5U
                    ? std::string("corp-guard")
                    : entity_kind == 11U
                        ? std::string("pedestrian")
                        : entity_kind == 8U
                            ? std::string("yakuza-weenr")
                            : make_sprite_stem(actor_name);
            world_activity_actor_records.push_back(WorldActivityActorRecord{
                list.scene_id,
                list.script_index,
                list.list_id,
                list.list_offset,
                source_order,
                static_cast<std::uint32_t>(source_record_offset),
                tagged_template,
                static_cast<std::uint32_t>(template_offset),
                position_offset,
                entity_kind,
                actor_name,
                profile_id,
                sprite_stem,
                action_code,
                action_selector,
                rom[template_offset + 6U],
                rom[template_offset + 7U],
                action_kind,
                palette_line,
                read_big_endian_word(rom, position_offset),
                read_big_endian_word(rom, position_offset + 2U),
                heading,
                first_override ? "-" : overrides.str(),
                list.activation_rule,
                list.world_effect_active_gate,
                list.pending_world_effect_gate,
                template_facts,
            });
            cursor = next_record;
        }
        if (!found_terminator) {
            throw std::runtime_error(
                "world-activity actor list has no null sentinel");
        }
    }
    // Actor template names are not unique animation-profile keys. The ROM
    // reuses names such as "lone11" with different entity-kind/palette
    // records; those resolve through distinct 0xC042/0xC0A2 selector tables.
    // Keep the decoded actor_name as source evidence and qualify only the
    // extracted asset namespace when one name maps to multiple visual kinds.
    std::map<std::string,
             std::set<std::pair<std::uint16_t, std::uint8_t>>>
        world_activity_sprite_variants;
    for (const auto& actor : default_actor_records) {
        world_activity_sprite_variants[actor.sprite_stem].insert(
            {actor.entity_kind, actor.palette_line});
    }
    for (const auto& actor : world_activity_actor_records) {
        world_activity_sprite_variants[actor.sprite_stem].insert(
            {actor.entity_kind, actor.palette_line});
    }
    for (auto& actor : world_activity_actor_records) {
        const auto variants =
            world_activity_sprite_variants.find(actor.sprite_stem);
        if (variants != world_activity_sprite_variants.end() &&
            variants->second.size() > 1U) {
            actor.sprite_stem =
                "actor-kind-" + std::to_string(actor.entity_kind) +
                "-palette-" + std::to_string(actor.palette_line);
        }
    }
    {
        const auto table_path = root / "field/entities/world-activity-actors.tsv";
        std::filesystem::create_directories(table_path.parent_path());
        std::ofstream table(table_path, std::ios::binary);
        if (!table) {
            throw std::runtime_error(
                "cannot create source world-activity actor table");
        }
        table << "scene\tlist_id\tlist_offset\tsource_order"
                    "\tscript_index"
                 "\tsource_record_offset\ttagged_template"
                 "\ttemplate_offset\tposition_offset\tentity_kind"
                 "\tactor_name\tprofile_id\tsprite_stem\taction_kind"
                 "\taction_code\tsource_action_slot"
                 "\tsource_action_ammo\tsource_action_counter"
                 "\tpalette_line\tworld_x\tworld_y\tinitial_heading"
                 "\tordered_overrides\tactivation_rule"
                 "\tworld_effect_active_gate\tpending_world_effect_gate"
                 "\tsource_spawn_state_flags\tsource_interaction_flags"
                 "\tphysical_meter\tdelay_primary_attribute"
                 "\tdelay_secondary_attribute\tdelay_capability_flags"
                 "\tcombat_mental_meter\tcombat_cyberware_flags"
                 "\tcombat_posture\tcombat_attribute_77"
                 "\tcombat_attribute_78\tcombat_attribute_79"
                 "\tcombat_attribute_7b\tcombat_attribute_7c"
                 "\tcombat_attribute_85\tcombat_item18_base"
                 "\tcombat_armor_index\tcombat_race_or_state"
                 "\tcombat_class_code\tcombat_modifier_flags_df"
                 "\tcombat_profile_index_a0"
                 "\tcombat_item_code_0\tcombat_item_code_1"
                 "\tcombat_item_code_2\tcombat_item_code_3"
                 "\tcombat_item_code_4\tcombat_item_code_5"
                 "\tcombat_item_code_6\tcombat_item_code_7"
                 "\tcombat_item_phase_0\tcombat_item_phase_1"
                 "\tcombat_item_phase_2\tcombat_item_phase_3"
                 "\tcombat_item_phase_4\tcombat_item_phase_5"
                 "\tcombat_item_phase_6\tcombat_item_phase_7"
                 "\tcombat_action_phase_76\tcombat_action_phase_77"
                 "\tcombat_action_phase_78\tcombat_action_phase_79"
                 "\tcombat_action_phase_7a\tcombat_action_phase_7b"
                 "\tcombat_action_phase_7c\tcombat_action_phase_7d"
                 "\tcombat_action_phase_7e\tcombat_action_phase_7f"
                 "\tcombat_action_skill_81\tcombat_action_skill_82"
                 "\tcombat_action_skill_83\tcombat_action_skill_84"
                 "\tcombat_auxiliary_roll_bonus\n";
        for (const auto& actor : world_activity_actor_records) {
            table << actor.scene_id << '\t' << actor.list_id << '\t'
                  << format_offset(actor.list_offset) << '\t'
                  << actor.source_order << '\t'
                  << actor.script_index << '\t'
                  << format_offset(actor.source_record_offset) << '\t'
                  << format_offset(actor.tagged_template) << '\t'
                  << format_offset(actor.template_offset) << '\t'
                  << format_offset(actor.position_offset) << '\t'
                  << actor.entity_kind << '\t' << actor.actor_name << '\t'
                  << actor.profile_id << '\t' << actor.sprite_stem << '\t'
                  << actor.action_kind << '\t'
                  << static_cast<unsigned>(actor.action_code) << '\t'
                  << static_cast<unsigned>(actor.source_action_slot) << '\t'
                  << static_cast<unsigned>(actor.source_action_ammo) << '\t'
                  << static_cast<unsigned>(actor.source_action_counter) << '\t'
                  << static_cast<unsigned>(actor.palette_line) << '\t'
                  << actor.world_x << '\t' << actor.world_y << '\t'
                  << actor.heading << '\t' << actor.overrides << '\t'
                  << actor.activation_rule << '\t'
                  << static_cast<int>(actor.world_effect_active_gate) << '\t'
                  << static_cast<int>(actor.pending_world_effect_gate) << '\t'
                  << static_cast<unsigned>(
                         actor.template_facts.source_spawn_state_flags) << '\t'
                  << static_cast<unsigned>(
                         actor.template_facts.source_interaction_flags) << '\t'
                  << static_cast<unsigned>(actor.template_facts.physical_meter)
                  << '\t'
                  << static_cast<unsigned>(
                         actor.template_facts.delay_primary_attribute) << '\t'
                  << static_cast<unsigned>(
                         actor.template_facts.delay_secondary_attribute) << '\t'
                  << actor.template_facts.delay_capability_flags << '\t'
                  << static_cast<unsigned>(actor.template_facts.mental_meter)
                  << '\t' << actor.template_facts.cyberware_flags << '\t'
                  << static_cast<unsigned>(actor.template_facts.posture) << '\t'
                  << static_cast<unsigned>(actor.template_facts.attribute_77)
                  << '\t'
                  << static_cast<unsigned>(actor.template_facts.attribute_78)
                  << '\t'
                  << static_cast<unsigned>(actor.template_facts.attribute_79)
                  << '\t'
                  << static_cast<unsigned>(actor.template_facts.attribute_7b)
                  << '\t'
                  << static_cast<unsigned>(actor.template_facts.attribute_7c)
                  << '\t'
                  << static_cast<unsigned>(actor.template_facts.attribute_85)
                  << '\t'
                  << static_cast<unsigned>(actor.template_facts.item18_base)
                  << '\t'
                  << static_cast<unsigned>(actor.template_facts.armor_index)
                  << '\t'
                  << static_cast<unsigned>(static_cast<std::uint8_t>(
                         actor.template_facts.race_or_state)) << '\t'
                  << static_cast<unsigned>(actor.template_facts.class_code)
                  << '\t'
                  << static_cast<unsigned>(
                         actor.template_facts.modifier_flags_df) << '\t'
                  << actor.template_facts.profile_index_a0;
            for (const auto value : actor.template_facts.item_codes) {
                table << '\t' << static_cast<unsigned>(value);
            }
            for (const auto value : actor.template_facts.item_phases) {
                table << '\t' << static_cast<unsigned>(value);
            }
            for (const auto value :
                 actor.template_facts.action_phase_extension) {
                table << '\t' << static_cast<unsigned>(value);
            }
            for (const auto value : actor.template_facts.action_skills) {
                table << '\t' << static_cast<unsigned>(value);
            }
            table << '\t' << static_cast<unsigned>(
                         actor.template_facts.auxiliary_roll_bonus)
                  << '\n';
        }
        if (!table) {
            throw std::runtime_error(
                "cannot write source world-activity actor table");
        }
    }

    struct DefaultActorKindSource {
        std::uint16_t entity_kind;
        std::uint8_t palette_line;
        std::string actor_name;
        std::string sprite_stem;
        std::uint32_t template_offset;
    };
    std::map<std::pair<std::uint16_t, std::uint8_t>, DefaultActorKindSource>
        actor_visual_sources;
    for (const auto& actor : default_actor_records) {
        const auto key = std::make_pair(actor.entity_kind, actor.palette_line);
        const auto [entry, inserted] = actor_visual_sources.try_emplace(
            key, DefaultActorKindSource{
                     actor.entity_kind, actor.palette_line, actor.actor_name,
                     actor.sprite_stem, actor.template_offset});
        if (!inserted && entry->second.actor_name != actor.actor_name) {
            throw std::runtime_error(
                "one ROM actor kind/palette resolves to multiple names");
        }
    }
    for (const auto& actor : world_activity_actor_records) {
        const auto key = std::make_pair(actor.entity_kind, actor.palette_line);
        if (actor_visual_sources.find(key) != actor_visual_sources.end()) {
            continue;
        }
        actor_visual_sources.emplace(
            key, DefaultActorKindSource{
                     actor.entity_kind, actor.palette_line,
                     "ROM kind " + std::to_string(actor.entity_kind),
                     actor.sprite_stem, actor.template_offset});
    }
    struct DynamicWorldActorRecord {
        std::uint16_t scene_id;
        std::uint8_t context_group;
        std::uint8_t random_sum;
        std::uint16_t source_order;
        std::uint32_t pointer_cell_offset;
        std::uint32_t sequence_list_offset;
        std::uint32_t tagged_template;
        std::uint32_t template_offset;
        std::uint16_t entity_kind;
        std::uint8_t palette_line;
        std::string actor_name;
        std::string profile_id;
        std::uint8_t action_code;
        std::uint8_t source_action_slot;
        std::uint8_t source_action_ammo;
        std::uint8_t source_action_counter;
        std::string action_kind;
        std::uint8_t source_interaction_flags;
        std::uint8_t physical_meter;
        std::uint8_t delay_primary_attribute;
        std::uint8_t delay_secondary_attribute;
        std::uint16_t delay_capability_flags;
        std::uint8_t mental_meter;
        std::uint16_t cyberware_flags;
        std::uint8_t posture;
        std::uint8_t attribute_77;
        std::uint8_t attribute_78;
        std::uint8_t attribute_79;
        std::uint8_t attribute_7b;
        std::uint8_t attribute_7c;
        std::uint8_t attribute_85;
        std::uint8_t item18_base;
        std::uint8_t armor_index;
        std::int8_t race_or_state;
        std::uint8_t class_code;
        std::uint8_t modifier_flags_df;
        std::uint16_t profile_index_a0;
        std::array<std::uint8_t, 8> item_codes;
        std::array<std::uint8_t, 8> item_phases;
        std::array<std::uint8_t, 10> action_phase_extension;
        std::array<std::uint8_t, 4> action_skills;
        std::uint8_t auxiliary_roll_bonus;
        std::string sprite_stem;
        std::string ordered_overrides;
        std::uint8_t run_target_area = 0U;
        std::uint8_t run_target_event = 0U;
        std::uint16_t run_target_x = 0U;
        std::uint16_t run_target_y = 0U;
    };
    constexpr std::size_t kFieldContextGroupTableOffset = 0x3312U;
    constexpr std::size_t kWorldActivityEnableTableOffset = 0x1D70E4U;
    constexpr std::size_t kWorldActivitySequencePointerTableOffset = 0x1D7174U;
    constexpr std::size_t kWorldActivityColumns = 9U;
    constexpr std::size_t kWorldActivityGroups = 16U;
    constexpr std::size_t kFieldSceneCount = 56U;
    std::vector<DynamicWorldActorRecord> dynamic_world_actor_records;
    for (std::size_t scene_id = 0U; scene_id < kFieldSceneCount; ++scene_id) {
        const auto context_group = rom[kFieldContextGroupTableOffset + scene_id];
        if (context_group >= kWorldActivityGroups) {
            throw std::runtime_error(
                "field context group exceeds the world activity table");
        }
        for (std::size_t random_sum = 0U;
             random_sum < kWorldActivityColumns; ++random_sum) {
            const auto cell_index =
                static_cast<std::size_t>(context_group) *
                    kWorldActivityColumns +
                random_sum;
            const auto enable_offset =
                kWorldActivityEnableTableOffset + cell_index;
            const auto enabled = rom[enable_offset];
            if (enabled > 1U) {
                throw std::runtime_error(
                    "world activity enable table contains a non-boolean cell");
            }
            if (enabled != 0U) {
                continue;
            }

            const auto pointer_cell_offset = static_cast<std::uint32_t>(
                kWorldActivitySequencePointerTableOffset + cell_index * 4U);
            const auto sequence_list_offset =
                read_big_endian_long(rom, pointer_cell_offset);
            auto cursor = static_cast<std::size_t>(sequence_list_offset);
            bool found_terminator = false;
            for (std::uint16_t source_order = 0U; source_order < 32U;
                 ++source_order) {
                const auto record_offset = cursor;
                const auto tagged_template = read_big_endian_long(rom, cursor);
                if (tagged_template == 0U) {
                    found_terminator = true;
                    break;
                }
                const auto template_offset = tagged_template & 0x7FFFFFFFU;
                // 0x44BC consumes template bytes through +0x49. The old
                // +0x4C read crossed into the next compact actor template.
                if (template_offset > rom.size() ||
                    rom.size() - template_offset < 0x4AU) {
                    throw std::runtime_error(
                        "dynamic world actor template points outside the ROM");
                }
                const auto entity_kind =
                    read_big_endian_word(rom, template_offset);
                const auto palette_line = static_cast<std::uint8_t>(
                    (read_big_endian_word(rom, template_offset + 2U) >> 13U) &
                    0x03U);
                auto action_selector = rom[template_offset + 8U];
                auto action_code = rom[template_offset + 9U];
                auto source_action_ammo = rom[template_offset + 6U];
                auto source_action_counter = rom[template_offset + 7U];
                auto delay_primary_attribute = rom[template_offset + 0x2AU];
                auto delay_secondary_attribute = rom[template_offset + 0x2DU];
                auto delay_capability_flags = read_big_endian_word(
                    rom, template_offset + 0x44U);
                auto physical_meter = rom[template_offset + 0x04U];
                auto source_interaction_flags = rom[
                    shadowrun::rom::kPostObjectEntityTemplate + 0x11U];
                auto mental_meter = rom[template_offset + 0x43U];
                auto cyberware_flags = read_big_endian_word(
                    rom, template_offset + 0x44U);
                auto posture = rom[template_offset + 0x05U];
                std::array<std::uint8_t, 10> action_phase_extension{};
                for (std::size_t field = 0U;
                     field < action_phase_extension.size(); ++field) {
                    action_phase_extension[field] =
                        rom[template_offset + 0x28U + field];
                }
                std::array<std::uint8_t, 4> action_skills{};
                for (std::size_t skill = 0U;
                     skill < action_skills.size(); ++skill) {
                    action_skills[skill] =
                        rom[template_offset + 0x33U + skill];
                }
                auto attribute_77 = action_phase_extension[1];
                auto attribute_78 = action_phase_extension[2];
                auto attribute_79 = action_phase_extension[3];
                auto attribute_7b = action_phase_extension[5];
                auto attribute_7c = action_phase_extension[6];
                auto attribute_85 = rom[template_offset + 0x37U];
                auto item18_base = rom[template_offset + 0x32U];
                auto armor_index = action_phase_extension[0];
                auto auxiliary_roll_bonus = rom[template_offset + 0x49U];
                auto race_or_state = static_cast<std::int8_t>(
                    rom[template_offset + 0x41U]);
                auto class_code = rom[template_offset + 0x42U];
                auto modifier_flags_df = rom[
                    shadowrun::rom::kPostObjectEntityTemplate + 0xDFU];
                auto profile_index_a0 = read_big_endian_word(
                    rom, template_offset + 0x46U);
                std::array<std::uint8_t, 8> item_codes{};
                std::array<std::uint8_t, 8> item_phases{};
                for (std::size_t item = 0U; item < item_codes.size(); ++item) {
                    item_codes[item] = rom[template_offset + 0x18U + item];
                    item_phases[item] = rom[template_offset + 0x20U + item];
                }
                const auto actor_name = read_actor_name(template_offset);
                if (entity_kind > 15U || actor_name.empty()) {
                    throw std::runtime_error(
                        "dynamic world actor template has no readable type/name");
                }

                auto next_record = cursor + 4U;
                std::ostringstream overrides;
                bool first_override = true;
                if (read_big_endian_long(rom, next_record) == 2U) {
                    auto pair_offset = next_record + 4U;
                    for (;;) {
                        const auto pair =
                            read_big_endian_word(rom, pair_offset);
                        if (pair == 0U) {
                            pair_offset += 2U;
                            break;
                        }
                        const auto override_offset =
                            static_cast<std::uint8_t>(pair >> 8U);
                        const auto override_value =
                            static_cast<std::uint8_t>(pair & 0x00FFU);
                        if (override_offset == 0x11U) {
                            source_interaction_flags = override_value;
                        } else if (override_offset == 0x4CU) {
                            physical_meter = override_value;
                        } else if (override_offset == 0x4EU) {
                            source_action_ammo = override_value;
                        } else if (override_offset == 0x4FU) {
                            source_action_counter = override_value;
                        } else if (override_offset == 0x56U) {
                            action_selector = override_value;
                        } else if (override_offset == 0x57U) {
                            action_code = override_value;
                        }
                        if (override_offset == 0x4DU) {
                            posture = override_value;
                        } else if (override_offset >= 0x66U &&
                                   override_offset <= 0x6DU) {
                            item_codes[override_offset - 0x66U] =
                                override_value;
                        } else if (override_offset >= 0x6EU &&
                                   override_offset <= 0x75U) {
                            item_phases[override_offset - 0x6EU] =
                                override_value;
                        } else if (override_offset >= 0x76U &&
                                   override_offset <= 0x7FU) {
                            action_phase_extension[override_offset - 0x76U] =
                                override_value;
                            armor_index = action_phase_extension[0];
                            attribute_77 = action_phase_extension[1];
                            attribute_78 = action_phase_extension[2];
                            attribute_79 = action_phase_extension[3];
                            attribute_7b = action_phase_extension[5];
                            attribute_7c = action_phase_extension[6];
                        } else if (override_offset >= 0x81U &&
                                   override_offset <= 0x84U) {
                            action_skills[override_offset - 0x81U] =
                                override_value;
                        } else if (override_offset == 0x80U) {
                            item18_base = override_value;
                        } else if (override_offset == 0x85U) {
                            attribute_85 = override_value;
                        } else if (override_offset == 0x90U) {
                            cyberware_flags = static_cast<std::uint16_t>(
                                (static_cast<std::uint16_t>(override_value) << 8U) |
                                (cyberware_flags & 0x00FFU));
                        } else if (override_offset == 0x91U) {
                            cyberware_flags = static_cast<std::uint16_t>(
                                (cyberware_flags & 0xFF00U) |
                                override_value);
                        } else if (override_offset == 0x9AU) {
                            race_or_state = static_cast<std::int8_t>(
                                override_value);
                        } else if (override_offset == 0x9BU) {
                            class_code = override_value;
                        } else if (override_offset == 0x9CU) {
                            mental_meter = override_value;
                        } else if (override_offset == 0x9DU) {
                            auxiliary_roll_bonus = override_value;
                        } else if (override_offset == 0xA0U) {
                            profile_index_a0 = static_cast<std::uint16_t>(
                                (static_cast<std::uint16_t>(override_value) << 8U) |
                                (profile_index_a0 & 0x00FFU));
                        } else if (override_offset == 0xA1U) {
                            profile_index_a0 = static_cast<std::uint16_t>(
                                (profile_index_a0 & 0xFF00U) |
                                override_value);
                        } else if (override_offset == 0xDFU) {
                            modifier_flags_df = override_value;
                        }
                        // ROM 0x44BC copies these template bytes into the
                        // live +0x78, +0x7B, and +0x90 delay inputs. Ordered
                        // list overrides replace those live bytes after the
                        // template copy, so export the final typed values.
                        if (override_offset == 0x78U) {
                            delay_primary_attribute = override_value;
                        } else if (override_offset == 0x7BU) {
                            delay_secondary_attribute = override_value;
                        } else if (override_offset == 0x90U) {
                            delay_capability_flags = static_cast<std::uint16_t>(
                                (static_cast<std::uint16_t>(override_value) << 8U) |
                                (delay_capability_flags & 0x00FFU));
                        } else if (override_offset == 0x91U) {
                            delay_capability_flags = static_cast<std::uint16_t>(
                                (delay_capability_flags & 0xFF00U) |
                                override_value);
                        }
                        if (!first_override) {
                            overrides << ',';
                        }
                        overrides << format_offset(pair >> 8U) << '='
                                  << format_offset(pair & 0x00FFU);
                        first_override = false;
                        pair_offset += 2U;
                    }
                    next_record = pair_offset;
                }

                const auto action_kind = action_code == 0U
                    ? std::string("none")
                    : ((action_selector & 0x80U) != 0U
                           ? std::string("spell")
                           : std::string("item"));
                const auto profile_id = make_actor_profile_id(
                    rom, template_offset, entity_kind, action_kind,
                    action_code);

                const auto sprite_stem =
                    "actor-kind-" + std::to_string(entity_kind) +
                    "-palette-" + std::to_string(palette_line);
                dynamic_world_actor_records.push_back(DynamicWorldActorRecord{
                    static_cast<std::uint16_t>(scene_id), context_group,
                    static_cast<std::uint8_t>(random_sum), source_order,
                    static_cast<std::uint32_t>(pointer_cell_offset),
                    sequence_list_offset, tagged_template,
                    static_cast<std::uint32_t>(template_offset), entity_kind,
                    palette_line, actor_name, profile_id, action_code,
                    action_selector, source_action_ammo,
                    source_action_counter, action_kind, source_interaction_flags,
                    physical_meter, delay_primary_attribute,
                    delay_secondary_attribute, delay_capability_flags,
                    mental_meter, cyberware_flags, posture, attribute_77, attribute_78,
                    attribute_79, attribute_7b, attribute_7c,
                    attribute_85, item18_base, armor_index, race_or_state,
                    class_code, modifier_flags_df, profile_index_a0,
                    item_codes, item_phases, action_phase_extension,
                    action_skills, auxiliary_roll_bonus,
                    sprite_stem,
                    first_override ? "-" : overrides.str(),
                });
                const auto key = std::make_pair(entity_kind, palette_line);
                if (actor_visual_sources.find(key) == actor_visual_sources.end()) {
                    actor_visual_sources.emplace(
                        key, DefaultActorKindSource{
                                 entity_kind, palette_line,
                                 "ROM kind " + std::to_string(entity_kind),
                                 sprite_stem,
                                 static_cast<std::uint32_t>(template_offset)});
                }
                cursor = next_record;
                if (cursor <= record_offset) {
                    throw std::runtime_error(
                        "dynamic world actor list did not advance");
                }
            }
            if (!found_terminator) {
                throw std::runtime_error(
                    "dynamic world actor list has no null sentinel");
            }
        }
    }
    // ROM 0x1F086 installs one of these pointer-only actor lists for a
    // matching gang-contract target. Unlike the regular random-encounter
    // lists above, these have no per-record override tail; each actor copies
    // its raw ROM template. Keep them in the typed Lua content table with
    // explicit target area/event fields so regular encounter selection never
    // consumes these records.
    struct RunTargetGangActorList {
        std::uint8_t area_id;
        std::uint8_t event_id;
        std::uint32_t list_offset;
    };
    static constexpr std::array<RunTargetGangActorList, 3>
        kRunTargetGangActorLists{{
            {1U, 9U, 0x1D6D4AU},
            {2U, 9U, 0x1D6D5EU},
            {3U, 4U, 0x1D6D72U},
        }};
    // ROM 0x16A26 pairs each selected actor-template pointer with the
    // corresponding six-byte placement record beginning at 0x16A6A. Actor
    // service 0x142DC reads the first two words as world X/Y; the third word
    // links a further placement and is zero for these four-row target groups.
    constexpr std::size_t kRunTargetPlacementTableOffset = 0x16A6AU;
    for (const auto& list : kRunTargetGangActorLists) {
        auto cursor = static_cast<std::size_t>(list.list_offset);
        bool found_terminator = false;
        for (std::uint16_t source_order = 0U; source_order < 16U;
             ++source_order, cursor += 4U) {
            const auto tagged_template = read_big_endian_long(rom, cursor);
            if (tagged_template == 0U) {
                found_terminator = true;
                break;
            }
            const auto template_offset =
                static_cast<std::size_t>(tagged_template & 0x7FFFFFFFU);
            if (template_offset > rom.size() ||
                rom.size() - template_offset < 0x4AU) {
                throw std::runtime_error(
                    "run-target actor template points outside the ROM");
            }

            const auto entity_kind =
                read_big_endian_word(rom, template_offset);
            const auto palette_line = static_cast<std::uint8_t>(
                (read_big_endian_word(rom, template_offset + 2U) >> 13U) &
                0x03U);
            const auto actor_name = read_actor_name(template_offset);
            if (actor_name.empty()) {
                throw std::runtime_error(
                    "run-target actor template has no readable name");
            }
            const auto action_selector = rom[template_offset + 8U];
            const auto action_code = rom[template_offset + 9U];
            const auto action_kind = action_code == 0U
                ? std::string("none")
                : ((action_selector & 0x80U) != 0U
                       ? std::string("spell")
                       : std::string("item"));
            const auto profile_id = make_actor_profile_id(
                rom, template_offset, entity_kind, action_kind, action_code);
            std::array<std::uint8_t, 10> action_phase_extension{};
            for (std::size_t index = 0U;
                 index < action_phase_extension.size(); ++index) {
                action_phase_extension[index] =
                    rom[template_offset + 0x28U + index];
            }
            std::array<std::uint8_t, 4> action_skills{};
            for (std::size_t index = 0U; index < action_skills.size(); ++index) {
                action_skills[index] = rom[template_offset + 0x33U + index];
            }
            std::array<std::uint8_t, 8> item_codes{};
            std::array<std::uint8_t, 8> item_phases{};
            for (std::size_t index = 0U; index < item_codes.size(); ++index) {
                item_codes[index] = rom[template_offset + 0x18U + index];
                item_phases[index] = rom[template_offset + 0x20U + index];
            }
            const auto sprite_stem =
                "actor-kind-" + std::to_string(entity_kind) + "-palette-" +
                std::to_string(palette_line);
            const auto cyberware_flags = read_big_endian_word(
                rom, template_offset + 0x44U);
            const auto key = std::make_pair(entity_kind, palette_line);
            if (actor_visual_sources.find(key) == actor_visual_sources.end()) {
                actor_visual_sources.emplace(
                    key, DefaultActorKindSource{
                             entity_kind, palette_line,
                             "ROM kind " + std::to_string(entity_kind),
                             sprite_stem,
                             static_cast<std::uint32_t>(template_offset)});
            }

            DynamicWorldActorRecord actor{};
            actor.scene_id = list.area_id;
            actor.context_group = 0U;
            actor.random_sum = 0U;
            actor.source_order = source_order;
            actor.pointer_cell_offset = list.list_offset;
            actor.sequence_list_offset = list.list_offset;
            actor.tagged_template = tagged_template;
            actor.template_offset =
                static_cast<std::uint32_t>(template_offset);
            actor.entity_kind = entity_kind;
            actor.palette_line = palette_line;
            actor.actor_name = actor_name;
            actor.profile_id = profile_id;
            actor.action_code = action_code;
            actor.source_action_slot = action_selector;
            actor.source_action_ammo = rom[template_offset + 6U];
            actor.source_action_counter = rom[template_offset + 7U];
            actor.action_kind = action_kind;
            actor.source_interaction_flags = rom[
                shadowrun::rom::kPostObjectEntityTemplate + 0x11U];
            actor.physical_meter = rom[template_offset + 0x04U];
            actor.delay_primary_attribute = rom[template_offset + 0x2AU];
            actor.delay_secondary_attribute = rom[template_offset + 0x2DU];
            actor.delay_capability_flags = cyberware_flags;
            actor.mental_meter = rom[template_offset + 0x43U];
            actor.cyberware_flags = cyberware_flags;
            actor.posture = rom[template_offset + 0x05U];
            actor.attribute_77 = action_phase_extension[1];
            actor.attribute_78 = action_phase_extension[2];
            actor.attribute_79 = action_phase_extension[3];
            actor.attribute_7b = action_phase_extension[5];
            actor.attribute_7c = action_phase_extension[6];
            actor.attribute_85 = rom[template_offset + 0x37U];
            actor.item18_base = rom[template_offset + 0x32U];
            actor.armor_index = action_phase_extension[0];
            actor.race_or_state = static_cast<std::int8_t>(
                rom[template_offset + 0x41U]);
            actor.class_code = rom[template_offset + 0x42U];
            actor.modifier_flags_df = rom[
                shadowrun::rom::kPostObjectEntityTemplate + 0xDFU];
            actor.profile_index_a0 = read_big_endian_word(
                rom, template_offset + 0x46U);
            actor.item_codes = item_codes;
            actor.item_phases = item_phases;
            actor.action_phase_extension = action_phase_extension;
            actor.action_skills = action_skills;
            actor.auxiliary_roll_bonus = rom[template_offset + 0x49U];
            actor.sprite_stem = sprite_stem;
            actor.ordered_overrides = "-";
            actor.run_target_area = list.area_id;
            actor.run_target_event = list.event_id;
            const auto placement_offset =
                kRunTargetPlacementTableOffset +
                static_cast<std::size_t>(source_order) * 6U;
            if (placement_offset > rom.size() ||
                rom.size() - placement_offset < 6U) {
                throw std::runtime_error(
                    "run-target actor placement points outside the ROM");
            }
            actor.run_target_x =
                read_big_endian_word(rom, placement_offset);
            actor.run_target_y =
                read_big_endian_word(rom, placement_offset + 2U);
            dynamic_world_actor_records.push_back(std::move(actor));
        }
        if (!found_terminator) {
            throw std::runtime_error(
                "run-target actor list has no null sentinel");
        }
    }
    {
        const auto table_path =
            root / "field/entities/dynamic-world-activity-actors.tsv";
        std::filesystem::create_directories(table_path.parent_path());
        std::ofstream table(table_path, std::ios::binary);
        if (!table) {
            throw std::runtime_error(
                "cannot create dynamic world-activity actor table");
        }
        table << "scene\tcontext_group\trandom_sum\tselection_cell_offset"
                 "\tsequence_list_offset\tsource_order\tactor_name"
                 "\tprofile_id\taction_kind\taction_code"
                 "\tsource_interaction_flags"
                 "\tentity_kind\tpalette_line\tsprite_stem"
                 "\ttagged_template\ttemplate_offset\tordered_overrides"
                 "\tplacement_rule\tphysical_meter"
                 "\tdelay_primary_attribute\tdelay_secondary_attribute"
                 "\tdelay_capability_flags\tcombat_mental_meter"
                 "\tcombat_cyberware_flags"
                 "\tcombat_posture\tcombat_attribute_77"
                 "\tcombat_attribute_78\tcombat_attribute_79"
                 "\tcombat_attribute_7b\tcombat_attribute_7c"
                 "\tcombat_attribute_85\tcombat_item18_base"
                 "\tcombat_armor_index\tcombat_race_or_state"
                 "\tcombat_class_code\tcombat_modifier_flags_df"
                 "\tcombat_profile_index_a0";
        for (std::size_t item = 0U; item < 8U; ++item) {
            table << "\tcombat_item_code_" << item;
        }
        for (std::size_t item = 0U; item < 8U; ++item) {
            table << "\tcombat_item_phase_" << item;
        }
        static constexpr std::array<std::string_view, 10>
            kActionPhaseHeaders{
                "combat_action_phase_76", "combat_action_phase_77",
                "combat_action_phase_78", "combat_action_phase_79",
                "combat_action_phase_7a", "combat_action_phase_7b",
                "combat_action_phase_7c", "combat_action_phase_7d",
                "combat_action_phase_7e", "combat_action_phase_7f"};
        for (const auto header : kActionPhaseHeaders) {
            table << '\t' << header;
        }
        static constexpr std::array<std::string_view, 4>
            kActionSkillHeaders{"combat_action_skill_81",
                                "combat_action_skill_82",
                                "combat_action_skill_83",
                                "combat_action_skill_84"};
        for (const auto header : kActionSkillHeaders) {
            table << '\t' << header;
        }
        table << "\tcombat_auxiliary_roll_bonus"
                 "\tsource_action_slot\tsource_action_ammo"
                 "\tsource_action_counter"
                 "\trun_target_area\trun_target_event"
                 "\trun_target_x\trun_target_y";
        table << '\n';
        for (const auto& actor : dynamic_world_actor_records) {
            const auto key = std::make_pair(actor.entity_kind, actor.palette_line);
            const auto visual = actor_visual_sources.find(key);
            if (visual == actor_visual_sources.end()) {
                throw std::runtime_error(
                    "dynamic actor type has no extracted visual source");
            }
            table << actor.scene_id << '\t'
                  << static_cast<unsigned>(actor.context_group) << '\t'
                  << static_cast<unsigned>(actor.random_sum) << '\t'
                  << format_offset(actor.pointer_cell_offset) << '\t'
                  << format_offset(actor.sequence_list_offset) << '\t'
                  << actor.source_order << '\t' << actor.actor_name << '\t'
                  << actor.profile_id << '\t' << actor.action_kind << '\t'
                  << static_cast<unsigned>(actor.action_code) << '\t'
                  << static_cast<unsigned>(actor.source_interaction_flags)
                  << '\t'
                  << actor.entity_kind << '\t'
                  << static_cast<unsigned>(actor.palette_line) << '\t'
                  << visual->second.sprite_stem << '\t'
                  << format_offset(actor.tagged_template) << '\t'
                  << format_offset(actor.template_offset) << '\t'
                  << actor.ordered_overrides << '\t'
                  << (actor.run_target_area == 0U
                          ? "open screen-edge candidates based on the current viewport and terrain"
                          : "ROM pending-descriptor actor placement")
                  << '\t'
                  << static_cast<unsigned>(actor.physical_meter) << '\t'
                  << static_cast<unsigned>(actor.delay_primary_attribute)
                  << '\t'
                  << static_cast<unsigned>(actor.delay_secondary_attribute)
                  << '\t' << actor.delay_capability_flags << '\t'
                  << static_cast<unsigned>(actor.mental_meter) << '\t'
                  << actor.cyberware_flags << '\t'
                  << static_cast<unsigned>(actor.posture) << '\t'
                  << static_cast<unsigned>(actor.attribute_77) << '\t'
                  << static_cast<unsigned>(actor.attribute_78) << '\t'
                  << static_cast<unsigned>(actor.attribute_79) << '\t'
                  << static_cast<unsigned>(actor.attribute_7b) << '\t'
                  << static_cast<unsigned>(actor.attribute_7c) << '\t'
                  << static_cast<unsigned>(actor.attribute_85) << '\t'
                  << static_cast<unsigned>(actor.item18_base) << '\t'
                  << static_cast<unsigned>(actor.armor_index) << '\t'
                  << static_cast<int>(actor.race_or_state) << '\t'
                  << static_cast<unsigned>(actor.class_code) << '\t'
                  << static_cast<unsigned>(actor.modifier_flags_df) << '\t'
                  << actor.profile_index_a0;
            for (const auto value : actor.item_codes) {
                table << '\t' << static_cast<unsigned>(value);
            }
            for (const auto value : actor.item_phases) {
                table << '\t' << static_cast<unsigned>(value);
            }
            for (const auto value : actor.action_phase_extension) {
                table << '\t' << static_cast<unsigned>(value);
            }
            for (const auto value : actor.action_skills) {
                table << '\t' << static_cast<unsigned>(value);
            }
            table << '\t'
                  << static_cast<unsigned>(actor.auxiliary_roll_bonus);
            table << '\t'
                  << static_cast<unsigned>(actor.source_action_slot) << '\t'
                  << static_cast<unsigned>(actor.source_action_ammo) << '\t'
                  << static_cast<unsigned>(actor.source_action_counter) << '\t'
                  << static_cast<unsigned>(actor.run_target_area) << '\t'
                  << static_cast<unsigned>(actor.run_target_event) << '\t'
                  << actor.run_target_x << '\t' << actor.run_target_y;
            table << '\n';
        }
        if (!table) {
            throw std::runtime_error(
                "cannot write dynamic world-activity actor table");
        }
    }
    constexpr std::size_t kIdleSelectorTableOffset = 0xC052U;
    constexpr std::size_t kItemActionSelectorTableOffset = 0xC042U;
    constexpr std::size_t kHurtSelectorTableOffset = 0xC072U;
    constexpr std::size_t kSpellSelectorTableOffset = 0xC082U;
    constexpr std::size_t kEntrySelectorTableOffset = 0xC0B2U;
    constexpr std::size_t kActionSelectorTableOffset = 0xC0A2U;
    constexpr std::size_t kActorHurtAudioTableOffset = 0xB03EU;
    constexpr std::size_t kActorDownAudioTableOffset = 0xB05EU;
    constexpr std::uint16_t kActorAnimationPeriod = 0x1000U;
    constexpr std::size_t kThonPatternStreamOffset = 0x1EBDEEU;
    constexpr std::size_t kThonPatternVramOffset = 0x5920U;
    constexpr std::size_t kActorStatePointerTableOffset = 0x62ACU;
    // The map viewer overlays source-defined one-of-three pedestrian
    // candidates from the 0x1EDB88 table. Their heading overrides cover all
    // eight C052 directions. Import each source kind/palette found in the
    // profile-selected lists so every map actor uses its own ROM frame.
    std::array<bool, 8> used_actor_directions{};
    used_actor_directions.fill(true);
    for (const auto& actor : default_actor_records) {
        auto rotated = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(actor.initial_heading << 3U) |
            (actor.initial_heading >> 13U));
        if ((rotated & 0x8000U) != 0U) {
            rotated = static_cast<std::uint16_t>(rotated + 1U);
        }
        used_actor_directions[rotated & 0x0007U] = true;
    }
    std::ofstream action_profile_table(
        root / "field/entities/actor-action-profiles.tsv",
        std::ios::binary);
    if (!action_profile_table) {
        throw std::runtime_error(
            "cannot create field actor action profile table");
    }
    action_profile_table
        << "sprite_stem\tentity_kind\tframe_count\tframe_increment"
           "\tframe_threshold\taudio_event\n";
    std::map<std::string, std::array<std::uint16_t, 3>> action_profile_facts;
    std::ofstream item_action_profile_table(
        root / "field/entities/actor-item-action-profiles.tsv",
        std::ios::binary);
    if (!item_action_profile_table) {
        throw std::runtime_error(
            "cannot create field actor item-action profile table");
    }
    item_action_profile_table
        << "sprite_stem\tentity_kind\tframe_count\tframe_increment"
           "\tframe_threshold\n";
    std::map<std::string, std::array<std::uint16_t, 3>>
        item_action_profile_facts;
    std::ofstream reaction_profile_table(
        root / "field/entities/actor-reaction-profiles.tsv",
        std::ios::binary);
    if (!reaction_profile_table) {
        throw std::runtime_error(
            "cannot create field actor reaction profile table");
    }
    reaction_profile_table
        << "sprite_stem\tentity_kind\thurt_frame_count\thurt_increment"
           "\tframe_threshold\thurt_audio_event\tdown_audio_event\n";
    std::map<std::string, std::array<std::uint16_t, 6>> reaction_profile_facts;
    for (const auto& [kind_palette, kind_source] : actor_visual_sources) {
        (void)kind_palette;
        const auto asset_stem =
            "field/entities/actors/" + kind_source.sprite_stem;
        if (kind_source.entity_kind == 15U) {
            // This is a distinct ROM format: 0x600E resolves a one-frame
            // state list whose entry is a seven-record VDP sprite list, not
            // an 0x8002 bitmap descriptor. The mode-55 marker-2 path loads
            // the matching tile data via 0x5BA4C/0x1530 at VRAM 0x5920.
            if (kind_source.actor_name != "Thon" ||
                kind_source.palette_line != 1U) {
                throw std::runtime_error(
                    "ROM kind 15 no longer matches the profiled Thon template");
            }
            const auto template_kind = read_big_endian_word(
                rom, kind_source.template_offset);
            const auto template_palette_attributes = read_big_endian_word(
                rom, kind_source.template_offset + 2U);
            if (template_kind != kind_source.entity_kind ||
                ((template_palette_attributes >> 13U) & 0x03U) !=
                    kind_source.palette_line) {
                throw std::runtime_error(
                    "Thon template kind/palette disagrees with the actor census");
            }

            const auto state_table = static_cast<std::size_t>(
                read_big_endian_long(
                    rom, kActorStatePointerTableOffset +
                        static_cast<std::size_t>(kind_source.entity_kind) * 4U));
            std::size_t thon_sprite_list = 0U;
            for (std::size_t direction = 0; direction < 8U; ++direction) {
                const auto selector = read_big_endian_word(
                    rom, kIdleSelectorTableOffset + direction * 2U);
                const auto state_list = static_cast<std::size_t>(
                    read_big_endian_long(
                        rom, state_table + (selector & 0x7FFFU)));
                if (read_big_endian_word(rom, state_list) != 0x8001U) {
                    throw std::runtime_error(
                        "Thon idle selector no longer selects one source frame");
                }
                const auto sprite_list = static_cast<std::size_t>(
                    read_big_endian_long(rom, state_list + 4U));
                if (direction == 0U) {
                    thon_sprite_list = sprite_list;
                } else if (sprite_list != thon_sprite_list) {
                    throw std::runtime_error(
                        "Thon directional selectors no longer share one source sprite list");
                }
            }
            if (thon_sprite_list == 0U) {
                throw std::runtime_error("Thon source sprite list is null");
            }

            const auto resources =
                shadowrun::graphics::read_genesis_field_mode_resources(
                    rom, 55U);
            auto palette = shadowrun::graphics::read_genesis_palette_set(
                rom, resources.palette_offset);
            // 0x566C2 replaces the descriptor's remaining CRAM words with
            // the shared field palette. Preserve that source line selection.
            for (std::size_t index = 16U; index < palette.size(); ++index) {
                palette[index] = shadowrun::graphics::decode_genesis_cram_word(
                    read_big_endian_word(
                        rom, 0x18AC56U + (index - 16U) * 2U),
                    index % 16U == 0U);
            }
            const auto thon_vram =
                shadowrun::graphics::decode_genesis_1530_vram(
                    rom, kThonPatternStreamOffset, kThonPatternVramOffset);
            const auto thon_frame = decode_field_entity_sprite_list(
                rom, thon_vram, thon_sprite_list, palette,
                template_palette_attributes);
            write_catalog_asset(
                root, asset_stem + "-idle.bmp", thon_frame.image, "sprite",
                "ROM 0x62AC/0xC052/0x600E/0x622C/0x624C/0xA77A Thon "
                "seven-piece idle sprite list; 0x5BA4C/0x1530 loads "
                "0x1EBDEE at VRAM 0x5920; template palette line 1",
                true, manifest, written, missing, blank,
                thon_frame.origin_x, thon_frame.origin_y);
            continue;
        }
        // Static map inhabitants use the ordinary human actor kinds. The
        // conditional field-event sequence also references the remaining
        // ROM actor kinds (gang members and creatures), which share the same
        // source state-pointer/idle-selector path and need their own palette
        // art in the map viewer's event evidence.
        for (std::size_t direction = 0; direction < used_actor_directions.size();
             ++direction) {
            if (!used_actor_directions[direction]) {
                continue;
            }
            const auto idle_selector = read_big_endian_word(
                rom, kIdleSelectorTableOffset + direction * 2U);
            const auto idle_frame =
                shadowrun::graphics::decode_genesis_field_entity_state_frame(
                    rom, kind_source.entity_kind, kind_source.palette_line,
                    idle_selector);
            const auto idle_name = asset_stem +
                                   (direction == 0U
                                        ? std::string("-idle.bmp")
                                        : "-idle-dir-" +
                                              std::to_string(direction) +
                                              ".bmp");
            write_catalog_asset(
                root, idle_name, idle_frame.image, "sprite",
                "0xB45C/0xC052/0x600E actor=" + kind_source.actor_name +
                    " kind=" +
                    std::to_string(kind_source.entity_kind) + " template=" +
                    format_offset(kind_source.template_offset) +
                    " descriptor=" +
                    format_offset(static_cast<std::uint32_t>(
                        idle_frame.descriptor_offset)) +
                    " selector=" + format_offset(idle_selector) +
                    " palette-line=" +
                    std::to_string(kind_source.palette_line),
                true, manifest, written, missing, blank,
                idle_frame.origin_x, idle_frame.origin_y);

            // AF7C/AFAE install the directional C082 state and switch the
            // actor callback to 0xAFE0. That callback is a NOP, so the cast
            // pose holds the first C082 image until another actor action
            // replaces it. Export that source frame as an ordinary host
            // sprite with its descriptor origin for the Qt field renderer.
            const auto spell_selector = read_big_endian_word(
                rom, kSpellSelectorTableOffset + direction * 2U);
            const auto spell_frame =
                shadowrun::graphics::decode_genesis_field_entity_state_frame(
                    rom, kind_source.entity_kind, kind_source.palette_line,
                    spell_selector, 0U);
            const auto spell_name = asset_stem +
                (direction == 0U
                     ? std::string("-spell.bmp")
                     : "-spell-dir-" + std::to_string(direction) + ".bmp");
            write_catalog_asset(
                root, spell_name, spell_frame.image, "sprite",
                "0xAF7C/0xAFAE/0xC082/0xAFE0/0x600E actor=" +
                    kind_source.actor_name + " kind=" +
                    std::to_string(kind_source.entity_kind) + " template=" +
                    format_offset(kind_source.template_offset) +
                    " selector=" + format_offset(spell_selector) +
                    " frame=0 palette-line=" +
                    std::to_string(kind_source.palette_line),
                true, manifest, written, missing, blank,
                spell_frame.origin_x, spell_frame.origin_y);
            const auto kind_state_pointer = static_cast<std::size_t>(
                read_big_endian_long(
                    rom, 0x62ACU +
                             static_cast<std::size_t>(kind_source.entity_kind) *
                                 4U));

            // ROM 0xB39A selects one of the directional C042 item-action
            // lists after the projectile service. Keep these separate from
            // C0A2 target attacks: item actions use this pose family and its
            // own frame cadence.
            // The complete C042 family is emitted once for this actor kind.
            // It already iterates all eight directions, unlike the surrounding
            // C072/C0B2/C062/C0A2 exports that use the outer direction loop.
            if (direction == 0U) {
                std::optional<std::array<std::uint16_t, 3>> item_action_facts;
                for (std::size_t frame_direction = 0U;
                     frame_direction < 8U; ++frame_direction) {
                const auto item_action_selector = read_big_endian_word(
                    rom, kItemActionSelectorTableOffset +
                             frame_direction * 2U);
                const auto item_action_list = static_cast<std::size_t>(
                    read_big_endian_long(
                        rom, kind_state_pointer + item_action_selector));
                const auto item_action_marker =
                    read_big_endian_word(rom, item_action_list);
                const auto item_action_frame_count =
                    static_cast<std::uint16_t>(
                        item_action_marker & 0x7FFFU);
                const auto item_action_increment =
                    read_big_endian_word(rom, item_action_list + 2U);
                if ((item_action_marker & 0x8000U) == 0U ||
                    item_action_frame_count == 0U ||
                    item_action_frame_count > 8U ||
                    item_action_increment == 0U) {
                    throw std::runtime_error(
                        "dynamic actor C042 item-action list has an unsupported source layout");
                }
                const std::array<std::uint16_t, 3> profile_facts{
                    kind_source.entity_kind, item_action_frame_count,
                    item_action_increment};
                if (item_action_facts.has_value() &&
                    *item_action_facts != profile_facts) {
                    throw std::runtime_error(
                        "dynamic actor C042 item-action lists disagree by direction");
                }
                item_action_facts = profile_facts;
                if (frame_direction == 0U) {
                    const auto [profile, inserted] =
                        item_action_profile_facts.emplace(
                            kind_source.sprite_stem, profile_facts);
                    if (!inserted && profile->second != profile_facts) {
                        throw std::runtime_error(
                            "dynamic actor C042 profiles disagree by sprite stem");
                    }
                    if (inserted) {
                        item_action_profile_table
                            << kind_source.sprite_stem << '\t'
                            << kind_source.entity_kind << '\t'
                            << item_action_frame_count << '\t'
                            << item_action_increment << '\t' << 0x1000U
                            << '\n';
                    }
                }
                for (std::uint16_t frame_index = 0U;
                     frame_index < item_action_frame_count; ++frame_index) {
                    const auto frame =
                        shadowrun::graphics::decode_genesis_field_entity_state_frame(
                            rom, kind_source.entity_kind,
                            kind_source.palette_line, item_action_selector,
                            frame_index);
                    write_catalog_asset(
                        root, asset_stem + "-item-action-dir-" +
                                  std::to_string(frame_direction) +
                                  "-frame-" + std::to_string(frame_index) +
                                  ".bmp",
                        frame.image, "sprite",
                        "0xB39A/0xC042/0x600E kind=" +
                            std::to_string(kind_source.entity_kind) +
                            " template=" +
                            format_offset(kind_source.template_offset) +
                            " descriptor=" +
                            format_offset(static_cast<std::uint32_t>(
                                frame.descriptor_offset)) +
                            " selector=" +
                            format_offset(item_action_selector) +
                            " frame=" + std::to_string(frame_index) +
                            " increment=" +
                            std::to_string(item_action_increment) +
                            " palette-line=" +
                            std::to_string(kind_source.palette_line),
                        true, manifest, written, missing, blank,
                        frame.origin_x, frame.origin_y);
                }
                }
            }

            // ROM 0xAFF2 selects the directional C072 list when a target's
            // physical meter remains nonzero. 0x600E uses the list's marker
            // and increment; the entity record's +0x1C period is initialized
            // to 0x1000 by 0x15132. Preserve the kind-indexed B03E/B05E
            // sounds beside the extracted frames so Lua can run the full
            // source reaction after a live hit.
            const auto hurt_selector = read_big_endian_word(
                rom, kHurtSelectorTableOffset + direction * 2U);
            const auto hurt_list = static_cast<std::size_t>(
                read_big_endian_long(
                    rom, kind_state_pointer + hurt_selector));
            const auto hurt_marker = read_big_endian_word(rom, hurt_list);
            const auto hurt_frame_count = static_cast<std::uint16_t>(
                hurt_marker & 0x7FFFU);
            const auto hurt_increment =
                read_big_endian_word(rom, hurt_list + 2U);
            if ((hurt_marker & 0x8000U) == 0U ||
                hurt_frame_count == 0U || hurt_frame_count > 8U ||
                hurt_increment == 0U) {
                throw std::runtime_error(
                    "dynamic actor C072 hurt list has an unsupported source layout");
            }
            const auto hurt_audio_event = read_big_endian_word(
                rom, kActorHurtAudioTableOffset +
                         static_cast<std::size_t>(kind_source.entity_kind) * 2U);
            const auto down_audio_event = read_big_endian_word(
                rom, kActorDownAudioTableOffset +
                         static_cast<std::size_t>(kind_source.entity_kind) * 2U);
            const std::array<std::uint16_t, 6> reaction_facts{
                kind_source.entity_kind, hurt_frame_count, hurt_increment,
                kActorAnimationPeriod, hurt_audio_event, down_audio_event};
            const auto [reaction_profile, reaction_inserted] =
                reaction_profile_facts.emplace(kind_source.sprite_stem,
                                                reaction_facts);
            if (!reaction_inserted &&
                reaction_profile->second != reaction_facts) {
                throw std::runtime_error(
                    "dynamic actor C072 lists disagree by direction");
            }
            if (direction == 0U && reaction_inserted) {
                reaction_profile_table
                    << kind_source.sprite_stem << '\t'
                    << kind_source.entity_kind << '\t'
                    << hurt_frame_count << '\t' << hurt_increment << '\t'
                    << kActorAnimationPeriod << '\t' << hurt_audio_event
                    << '\t' << down_audio_event << '\n';
            }
            for (std::uint16_t frame_index = 0U;
                 frame_index < hurt_frame_count; ++frame_index) {
                const auto frame =
                    shadowrun::graphics::decode_genesis_field_entity_state_frame(
                        rom, kind_source.entity_kind,
                        kind_source.palette_line, hurt_selector, frame_index);
                write_catalog_asset(
                    root, asset_stem + "-hurt-dir-" +
                              std::to_string(direction) + "-frame-" +
                              std::to_string(frame_index) + ".bmp",
                    frame.image, "sprite",
                    "0xAFF2/0xC072/0x5FDC/0x600E kind=" +
                        std::to_string(kind_source.entity_kind) +
                        " template=" +
                        format_offset(kind_source.template_offset) +
                        " descriptor=" +
                        format_offset(static_cast<std::uint32_t>(
                            frame.descriptor_offset)) +
                        " selector=" + format_offset(hurt_selector) +
                        " frame=" + std::to_string(frame_index) +
                        " increment=" + std::to_string(hurt_increment) +
                        " palette-line=" +
                        std::to_string(kind_source.palette_line),
                    true, manifest, written, missing, blank,
                    frame.origin_x, frame.origin_y);
            }

            const auto entry_selector = read_big_endian_word(
                rom, kEntrySelectorTableOffset + direction * 2U);
            const auto entry_list = static_cast<std::size_t>(
                read_big_endian_long(rom, kind_state_pointer + entry_selector));
            const auto entry_marker = read_big_endian_word(rom, entry_list);
            const auto entry_frame_count =
                static_cast<std::uint16_t>(entry_marker & 0x7FFFU);
            if ((entry_marker & 0x8000U) == 0U ||
                entry_frame_count == 0U || entry_frame_count > 8U ||
                read_big_endian_word(rom, entry_list + 2U) != 0x0200U) {
                throw std::runtime_error(
                    "profile-selected actor entry animation has an "
                    "unsupported source frame list");
            }
            for (std::uint16_t frame_index = 0U;
                 frame_index < entry_frame_count;
                 ++frame_index) {
                const auto frame =
                    shadowrun::graphics::decode_genesis_field_entity_state_frame(
                        rom, kind_source.entity_kind,
                        kind_source.palette_line, entry_selector, frame_index);
                const auto entry_name =
                asset_stem + "-entry-dir-" +
                    std::to_string(direction) + "-frame-" +
                    std::to_string(frame_index) + ".bmp";
                write_catalog_asset(
                    root, entry_name, frame.image, "sprite",
                    "0xB330/0xC0B2/0x5FDC/0x600E kind=" +
                        std::to_string(kind_source.entity_kind) +
                        " template=" +
                        format_offset(kind_source.template_offset) +
                        " descriptor=" +
                        format_offset(static_cast<std::uint32_t>(
                            frame.descriptor_offset)) +
                        " selector=" + format_offset(entry_selector) +
                        " frame=" + std::to_string(frame_index) +
                        " palette-line=" +
                        std::to_string(kind_source.palette_line),
                    true, manifest, written, missing, blank, frame.origin_x,
                    frame.origin_y);
            }

            // Ordinary accepted actor movement selects the positive-marker
            // C062 list, separate from the finite C0B2 entry transition.
            // Extract its source frames as named RGBA host assets; runtime
            // Lua applies the C062 frame cadence from the extracted +0xCA
            // timing fact and the native renderer only presents the result.
            const auto walk_selector = read_big_endian_word(
                rom, 0xC062U + direction * 2U);
            const auto walk_list = static_cast<std::size_t>(
                read_big_endian_long(
                    rom, kind_state_pointer + walk_selector));
            const auto walk_marker = read_big_endian_word(rom, walk_list);
            const auto walk_frame_count = static_cast<std::uint16_t>(
                walk_marker & 0x7FFFU);
            if ((walk_marker & 0x8000U) != 0U || walk_frame_count == 0U ||
                walk_frame_count > 8U) {
                throw std::runtime_error(
                    "dynamic actor C062 list has an unsupported source layout");
            }
            for (std::uint16_t frame_index = 0U;
                 frame_index < walk_frame_count; ++frame_index) {
                const auto frame =
                    shadowrun::graphics::decode_genesis_field_entity_state_frame(
                        rom, kind_source.entity_kind,
                        kind_source.palette_line, walk_selector, frame_index);
                write_catalog_asset(
                    root, asset_stem + "-walk-dir-" +
                              std::to_string(direction) + "-frame-" +
                              std::to_string(frame_index) + ".bmp",
                    frame.image, "sprite",
                    "0xB6DE/0xC062/0x600E kind=" +
                        std::to_string(kind_source.entity_kind) +
                        " template=" +
                        format_offset(kind_source.template_offset) +
                        " descriptor=" +
                        format_offset(static_cast<std::uint32_t>(
                            frame.descriptor_offset)) +
                        " selector=" + format_offset(walk_selector) +
                        " frame=" + std::to_string(frame_index) +
                        " palette-line=" +
                        std::to_string(kind_source.palette_line),
                    true, manifest, written, missing, blank,
                    frame.origin_x, frame.origin_y);
            }

            // B1DC installs the directional C0A2 finite list for a target
            // action. The list's +2 word becomes the live animation
            // increment; actor initialization at 1514E supplies the
            // 0x1000 threshold. Keep these as named source facts so Lua can
            // advance the action heartbeat without a runtime ROM lookup.
            const auto action_selector = read_big_endian_word(
                rom, kActionSelectorTableOffset + direction * 2U);
            const auto action_list = static_cast<std::size_t>(
                read_big_endian_long(
                    rom, kind_state_pointer + action_selector));
            const auto action_marker = read_big_endian_word(rom, action_list);
            const auto action_frame_count = static_cast<std::uint16_t>(
                action_marker & 0x7FFFU);
            const auto action_frame_increment =
                read_big_endian_word(rom, action_list + 2U);
            if ((action_marker & 0x8000U) == 0U ||
                action_frame_count == 0U || action_frame_count > 8U ||
                action_frame_increment == 0U) {
                throw std::runtime_error(
                    "dynamic actor C0A2 list has an unsupported source layout");
            }
            const std::array<std::uint16_t, 3> profile_facts{
                kind_source.entity_kind, action_frame_count,
                action_frame_increment};
            const auto [profile, inserted] = action_profile_facts.emplace(
                kind_source.sprite_stem, profile_facts);
            if (!inserted && profile->second != profile_facts) {
                throw std::runtime_error(
                    "dynamic actor C0A2 lists disagree by direction");
            }
            if (direction == 0U && inserted) {
                action_profile_table
                    << kind_source.sprite_stem << '\t'
                    << kind_source.entity_kind << '\t'
                    << action_frame_count << '\t'
                    << action_frame_increment << '\t' << 0x1000U << '\t'
                    << (kind_source.entity_kind == 12U ? 0x55U : 0x4CU)
                    << '\n';
            }
            for (std::uint16_t frame_index = 0U;
                 frame_index < action_frame_count; ++frame_index) {
                const auto frame =
                    shadowrun::graphics::decode_genesis_field_entity_state_frame(
                        rom, kind_source.entity_kind,
                        kind_source.palette_line, action_selector, frame_index);
                write_catalog_asset(
                    root, asset_stem + "-action-dir-" +
                              std::to_string(direction) + "-frame-" +
                              std::to_string(frame_index) + ".bmp",
                    frame.image, "sprite",
                    "0xB1DC/0xC0A2/0x5FDC/0x600E kind=" +
                        std::to_string(kind_source.entity_kind) +
                        " template=" +
                        format_offset(kind_source.template_offset) +
                        " descriptor=" +
                        format_offset(static_cast<std::uint32_t>(
                            frame.descriptor_offset)) +
                        " selector=" + format_offset(action_selector) +
                        " frame=" + std::to_string(frame_index) +
                        " increment=" +
                        std::to_string(action_frame_increment) +
                        " palette-line=" +
                        std::to_string(kind_source.palette_line),
                    true, manifest, written, missing, blank,
                    frame.origin_x, frame.origin_y);
            }
        }
    }
    action_profile_table.close();
    if (!action_profile_table) {
        throw std::runtime_error(
            "cannot write field actor action profile table");
    }
    reaction_profile_table.close();
    if (!reaction_profile_table) {
        throw std::runtime_error(
            "cannot write field actor reaction profile table");
    }

    for (std::size_t index = 0; index < assets.entity_frames.size(); ++index) {
        write_frame("field/entities/idle-" + std::to_string(index) + ".bmp",
                    assets.entity_frames[index], "sprite", "0x5A62/0x5FDC",
                    true);
    }
    for (std::size_t direction = 0;
         direction < assets.field_entity_animations.size(); ++direction) {
        const auto& animation = assets.field_entity_animations[direction];
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            write_frame("field/entities/walk-" + std::to_string(direction) + "-" +
                            std::to_string(frame) + ".bmp",
                        animation.frames[frame], "sprite", "0x5A62/0x5FDC", true);
        }
    }
    const auto field_movement_animations =
        shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
            rom, 0x7291A, 0x62AC, 0xC062, 0x3EDC, 0x172D8A, 8);
    for (std::size_t direction = 0;
         direction < field_movement_animations.size(); ++direction) {
        const auto& animation = field_movement_animations[direction];
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            const auto& source = animation.frames[frame];
            write_frame(
                "field/entities/movement-" + std::to_string(direction) + "-" +
                    std::to_string(frame) + ".bmp",
                shadowrun::native::NativeEntityFrame{
                    source.width, source.height, source.pixels},
                "sprite", "0xB6DE/0xC062/0x600E", true);
        }
    }
    const auto field_attack_animations =
        shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
            rom, 0x7291A, 0x62AC, 0xC042, 0x3EDC, 0x172D8A, 8);
    for (std::size_t direction = 0;
         direction < field_attack_animations.size(); ++direction) {
        const auto& animation = field_attack_animations[direction];
        if (animation.frames.size() != 1U) {
            throw std::runtime_error(
                "source C042 attack pose did not resolve to one frame");
        }
        const auto& source = animation.frames.front();
        write_frame(
            "field/entities/attack-" + std::to_string(direction) + ".bmp",
            shadowrun::native::NativeEntityFrame{
                source.width, source.height, source.pixels},
            "sprite", "0xB39A/0xC042/0x600E", true);
    }

    // Weapon actions allocate a paired behavior/projectile record at
    // 0x1514E. The second record selects one of the 16 direction lists in
    // 0x62EC/0x62FC and 0x630C; export each descriptor assembly as its own
    // host image so the field runtime never needs to decode ROM lists.
    const auto field_spawn_animations =
        shadowrun::graphics::decode_genesis_field_spawn_animations(rom);
    for (const auto& animation : field_spawn_animations) {
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            const auto& source = animation.frames[frame];
            const auto relative_name =
                "field/weapons/projectile-" +
                std::to_string(animation.selector) + "-" +
                std::to_string(frame) + ".bmp";
            write_catalog_asset(
                root, relative_name, source.image, "sprite",
                "0x1514E/0x62EC/0x62FC/0x630C/0x600E/0x622C", true,
                manifest, written, missing, blank, source.origin_x,
                source.origin_y);
        }
    }

    // ROM 0x5C7E selects one of four target-marker tile/palette variants,
    // while 0x1406 loads the current scene's palette for the shared object
    // atlas. Bake a scene-specific 8x8 tile for each source action-code group;
    // Qt consumes these named assets without a runtime VRAM atlas.
    constexpr std::array<std::string_view, 4> kTargetMarkerGroups{{
        "action-0-2", "action-3-5", "action-6-8", "action-9-plus",
    }};
    for (std::uint16_t scene_id = 0U;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto marker_images =
            shadowrun::graphics::decode_genesis_field_target_marker_images(
                rom, scene_id);
        for (std::size_t variant = 0U;
             variant < marker_images.size(); ++variant) {
            const auto relative_name =
                "field/entities/target-marker/scene-" +
                std::to_string(scene_id) + "-" +
                std::string(kTargetMarkerGroups[variant]) + ".png";
            const auto source =
                "0x5C7E/0x5D92..0x5DD4; shared object tile " +
                (variant == 1U ? std::string{"0x5E5"}
                 : variant == 2U ? std::string{"0x5E6"}
                                 : std::string{"0x5E7"}) +
                "; scene " + std::to_string(scene_id) +
                " palette line " +
                (variant == 3U ? std::string{"3"} : std::string{"1"});
            write_catalog_asset(
                root, relative_name, marker_images[variant], "sprite", source,
                false, manifest, written, missing, blank);
        }
    }

    // The kind-0xC branch at 0x14C7C claims a shared effect slot. Its 0x4E86
    // render continuation selects one of four directional descriptor lists
    // through 0x4F42, then 0x13BE places their sprites using palette line 1.
    // The first service pass advances +0x03 to phase 1; phase 4 retires the
    // effect without drawing, so only phases 1..3 are live artwork. Bake the
    // current field palette into each scene's named frames so Qt consumes
    // direct assets and never reconstructs the ROM's temporary VRAM slots.
    constexpr std::size_t kCloseAttackHeadingTableOffset = 0x4F42U;
    constexpr std::size_t kCloseAttackPaletteOffset = 0x18AC56U;
    constexpr std::size_t kVramSize = 0x10000U;
    for (std::uint16_t scene_id = 0U;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(
                rom, scene_id);
        auto palette = shadowrun::graphics::read_genesis_palette_set(
            rom, resources.palette_offset);
        for (std::size_t index = 16U; index < palette.size(); ++index) {
            palette[index] = shadowrun::graphics::decode_genesis_cram_word(
                read_big_endian_word(
                    rom, kCloseAttackPaletteOffset + (index - 16U) * 2U),
                index % 16U == 0U);
        }

        for (std::uint16_t heading = 0U; heading < 8U; ++heading) {
            const auto phase_table = static_cast<std::size_t>(
                read_big_endian_long(
                    rom, kCloseAttackHeadingTableOffset + heading * 4U));
            for (std::uint16_t phase = 1U; phase <= 3U; ++phase) {
                const auto descriptor_offset = static_cast<std::size_t>(
                    read_big_endian_long(rom, phase_table + phase * 4U));
                if (read_big_endian_word(rom, descriptor_offset) != 0x8002U) {
                    throw std::runtime_error(
                        "Hell Hound close-attack frame has an unsupported descriptor");
                }
                const auto pattern_offset = static_cast<std::size_t>(
                    read_big_endian_long(rom, descriptor_offset + 2U));
                const auto pattern_size = static_cast<std::size_t>(
                    read_big_endian_word(rom, descriptor_offset + 6U));
                if (pattern_size == 0U || pattern_size > kVramSize ||
                    pattern_offset > rom.size() ||
                    pattern_size > rom.size() - pattern_offset) {
                    throw std::runtime_error(
                        "Hell Hound close-attack pattern bytes exceed the ROM or host VRAM");
                }
                std::vector<std::uint8_t> vram(kVramSize, 0U);
                std::copy_n(rom.begin() +
                                static_cast<std::ptrdiff_t>(pattern_offset),
                            pattern_size, vram.begin());
                const auto frame = decode_field_entity_sprite_list(
                    rom, vram, descriptor_offset + 8U, palette, 0x2000U);
                const auto relative_name =
                    "field/entities/effects/hell-hound-close-attack/scene-" +
                    std::to_string(scene_id) + "-heading-" +
                    std::to_string(heading) + "-phase-" +
                    std::to_string(phase) + ".bmp";
                const auto source =
                    "ROM 0x14C7C kind=0xC; 0x4E86/0x4F42 heading=" +
                    std::to_string(heading) + " phase=" +
                    std::to_string(phase) + " descriptor=" +
                    format_offset(static_cast<std::uint32_t>(
                        descriptor_offset)) +
                    " pattern=" +
                    format_offset(static_cast<std::uint32_t>(pattern_offset)) +
                    " bytes=" + std::to_string(pattern_size) +
                    "; 0x13BE palette line 1; scene palette plus 0x18AC56";
                write_catalog_asset(
                    root, relative_name, frame.image, "sprite", source, true,
                    manifest, written, missing, blank, frame.origin_x,
                    frame.origin_y);
            }
        }
    }

    // The four spell action handlers present in the encounter actor catalog
    // read the effect record from the target actor during 0x4658 rendering.
    // Their phase pointers are source data, but the Qt runtime consumes these
    // individually named frames and never reconstructs the temporary VRAM.
    struct SpellEffectSourceProfile {
        std::uint8_t action_code;
        std::string_view asset_id;
        std::size_t phase_table;
        std::uint8_t phase_count;
        std::uint16_t palette_attributes;
        std::string_view handler;
    };
    constexpr std::array<SpellEffectSourceProfile, 5> kSpellEffects{{
        {1U, "flame-dart", 0x58B6U, 4U, 0x2000U, "0x521E"},
        {2U, "flame-bolt", 0x589EU, 6U, 0x2000U, "0x5152"},
        {5U, "sleep", 0x5876U, 9U, 0x6000U, "0x52BE/0x5322"},
        {9U, "mana-blast", 0x512AU, 6U, 0x4000U, "0x4FE2"},
        {12U, "heal-wounds", 0x5756U, 16U, 0x6000U, "0x56DE"},
    }};
    for (std::uint16_t scene_id = 0U;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(
                rom, scene_id);
        const auto palette = shadowrun::graphics::read_genesis_palette_set(
            rom, resources.palette_offset);
        for (const auto& profile : kSpellEffects) {
            for (std::uint8_t phase = 0U;
                 phase < profile.phase_count; ++phase) {
                const auto descriptor_offset = static_cast<std::size_t>(
                    read_big_endian_long(
                        rom, profile.phase_table + phase * 4U));
                if (read_big_endian_word(rom, descriptor_offset) != 0x8002U) {
                    throw std::runtime_error(
                        "spell effect frame has an unsupported source descriptor");
                }
                const auto pattern_offset = static_cast<std::size_t>(
                    read_big_endian_long(rom, descriptor_offset + 2U));
                const auto pattern_size = static_cast<std::size_t>(
                    read_big_endian_word(rom, descriptor_offset + 6U));
                if (pattern_size == 0U || pattern_size > kVramSize ||
                    pattern_offset > rom.size() ||
                    pattern_size > rom.size() - pattern_offset) {
                    throw std::runtime_error(
                        "spell effect pattern exceeds the ROM or host VRAM");
                }
                std::vector<std::uint8_t> vram(kVramSize, 0U);
                std::copy_n(rom.begin() +
                                static_cast<std::ptrdiff_t>(pattern_offset),
                            pattern_size, vram.begin());
                const auto frame = decode_field_entity_sprite_list(
                    rom, vram, descriptor_offset + 8U, palette,
                    profile.palette_attributes);
                const auto relative_name =
                    "field/entities/effects/spell/" +
                    std::string(profile.asset_id) + "/scene-" +
                    std::to_string(scene_id) + "-phase-" +
                    std::to_string(phase) + ".bmp";
                const auto source =
                    "ROM 0x490A action=" +
                    std::to_string(profile.action_code) + " handler=" +
                    std::string(profile.handler) + " phase=" +
                    std::to_string(phase) + " descriptor=" +
                    format_offset(static_cast<std::uint32_t>(
                        descriptor_offset)) + " pattern=" +
                    format_offset(static_cast<std::uint32_t>(pattern_offset)) +
                    " bytes=" + std::to_string(pattern_size) +
                    "; palette line=" +
                    std::to_string(profile.palette_attributes >> 13U) +
                    " scene=" + std::to_string(scene_id);
                write_catalog_asset(
                    root, relative_name, frame.image, "sprite", source, true,
                    manifest, written, missing, blank, frame.origin_x,
                    frame.origin_y);
            }
        }
    }

    extract_field_session_record_assets(rom, root);
    manifest.flush();
    std::filesystem::create_directories(root / "native");
    shadowrun::graphics::write_native_asset_state(
        (root / "native" / "state.bin").string(), assets);
    write_runtime_resource_manifest(root);
    std::cout << "all graphics assets written to " << root << "\n"
              << "extracted=" << written << " blank=" << blank
              << " missing=" << missing << "\n";
    if (missing != 0U) {
        throw std::runtime_error(
            "asset catalog contains missing image resources; extraction coverage is incomplete");
    }
}

shadowrun::graphics::RgbaImage make_field_entity_state_sheet(
    const std::vector<shadowrun::graphics::RgbaImage>& frames) {
    constexpr std::uint16_t kCellWidth = 40;
    constexpr std::uint16_t kCellHeight = 40;
    constexpr std::size_t kColumns = 4;
    const auto rows = std::max<std::size_t>(
        1U, (frames.size() + kColumns - 1U) / kColumns);
    shadowrun::graphics::RgbaImage sheet{
        static_cast<std::uint16_t>(kCellWidth * kColumns),
        static_cast<std::uint16_t>(kCellHeight * rows),
        std::vector<shadowrun::native::Rgba8>(
            static_cast<std::size_t>(kCellWidth * kColumns) *
                (kCellHeight * rows),
            {0x04, 0x08, 0x14, 0xFF}),
    };
    for (std::size_t index = 0; index < frames.size(); ++index) {
        const auto& frame = frames[index];
        const auto cell_x = static_cast<std::int32_t>(
            (index % kColumns) * kCellWidth);
        const auto cell_y = static_cast<std::int32_t>(
            (index / kColumns) * kCellHeight);
        const auto origin_x = cell_x +
            (static_cast<std::int32_t>(kCellWidth) - frame.width) / 2;
        const auto origin_y = cell_y +
            (static_cast<std::int32_t>(kCellHeight) - frame.height) / 2;
        for (std::int32_t y = 0; y < frame.height; ++y) {
            for (std::int32_t x = 0; x < frame.width; ++x) {
                const auto& pixel = frame.pixels[
                    static_cast<std::size_t>(y) * frame.width + x];
                if (pixel.alpha == 0) {
                    continue;
                }
                const auto target_x = origin_x + x;
                const auto target_y = origin_y + y;
                sheet.pixels[static_cast<std::size_t>(target_y) * sheet.width +
                             target_x] = pixel;
            }
        }
    }
    return sheet;
}

std::vector<shadowrun::graphics::RgbaImage> flatten_field_entity_animations(
    const std::vector<shadowrun::graphics::GenesisFieldEntityAnimation>&
        animations) {
    std::vector<shadowrun::graphics::RgbaImage> frames;
    for (const auto& animation : animations) {
        for (const auto& frame : animation.frames) {
            frames.push_back(frame);
        }
    }
    return frames;
}

void print_field_event_cells(
    const shadowrun::terrain::Map& terrain,
    const std::array<std::uint8_t, 32>& event_mode_table) {
    std::size_t count = 0;
    for (std::size_t row = 0; row < shadowrun::terrain::Map::kHeight; ++row) {
        for (std::size_t column = 0;
             column < shadowrun::terrain::Map::kWidth; ++column) {
            const auto status = terrain.statuses[
                row * shadowrun::terrain::Map::kWidth + column];
            if ((status & 0xE0U) != 0xC0U) {
                continue;
            }
            const auto event_index = static_cast<std::size_t>(status & 0x1FU);
            const auto destination = event_index < event_mode_table.size()
                ? event_mode_table[event_index]
                : 0U;
            std::cout << "cell (" << column * 32U << "," << row * 32U
                      << ") raw=0x" << std::hex << static_cast<unsigned>(status)
                      << " event=" << std::dec << event_index
                      << " destination=" << destination << "\n";
            ++count;
        }
    }
    std::cout << "C0 event cells: " << count << "\n";
}

shadowrun::graphics::RgbaImage render_native_field(
    shadowrun::native::NativePortAssets assets) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    // Follow the reviewed title -> start options -> new game -> archetype
    // path. New Game enters 0x59C5E; skip that source-timed opening for this
    // field-only diagnostic, just as a user can with an action edge.
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::A);
    press(shadowrun::native::Button::A);
    if (shell.scene() == shadowrun::native::NativeScene::Intro) {
        press(shadowrun::native::Button::Start);
    }
    if (shell.scene() != shadowrun::native::NativeScene::Field) {
        throw std::runtime_error(
            "native field diagnostic did not reach the field scene");
    }
    shell.render(screen);

    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size()),
    };
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255,
        };
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_field_menu(
    shadowrun::native::NativePortAssets assets, bool inventory) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::A);
    press(shadowrun::native::Button::A);
    if (shell.scene() == shadowrun::native::NativeScene::Intro) {
        press(shadowrun::native::Button::Start);
    }
    if (shell.scene() != shadowrun::native::NativeScene::Field) {
        throw std::runtime_error(
            "native field-menu diagnostic did not reach the field scene");
    }
    press(shadowrun::native::Button::Start);
    if (shell.scene() != shadowrun::native::NativeScene::FieldMenu) {
        throw std::runtime_error(
            "native field-menu diagnostic did not open the command screen");
    }
    if (inventory) {
        const auto press_menu_edge = [&shell](shadowrun::native::Button button) {
            const auto mask = shadowrun::native::button_mask(button);
            shell.update(shadowrun::native::InputState{mask, mask});
            shell.update(shadowrun::native::InputState{});
        };
        // The ROM starts at Pocket Secretary (selector 6). Six left edges
        // select the first Inventory column, which exercises the real
        // selected-cell, item-art, and item-stat destinations.
        for (int step = 0; step < 6; ++step) {
            press_menu_edge(shadowrun::native::Button::Left);
        }
    }
    shell.render(screen);

    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size()),
    };
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255,
        };
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_startup_scene(
    shadowrun::native::NativePortAssets assets, const std::string& scene_name) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    if (scene_name == "start-options") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
    } else if (scene_name == "save-load" || scene_name == "no-save") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Down);
        press(shadowrun::native::Button::A);
        if (scene_name == "no-save") {
            press(shadowrun::native::Button::A);
        }
    } else if (scene_name == "archetype") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::A);
    } else if (scene_name == "intro") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::A);
        press(shadowrun::native::Button::A);
    } else {
        throw std::invalid_argument("unknown native startup scene");
    }
    shell.render(screen);
    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_title_frame(
    shadowrun::native::NativePortAssets assets, std::size_t frame) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    for (std::size_t index = 0; index < frame; ++index) {
        shell.update(shadowrun::native::InputState{});
    }
    shell.render(screen);
    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 5 && std::string(argv[3]) == "--dialogue-record") {
        try {
            const auto rom = read_file(argv[1]);
            const auto source_offset = static_cast<std::size_t>(
                parse_number(argv[4]));
            const auto segments =
                shadowrun::graphics::read_genesis_dialogue_segments(
                    rom, source_offset);
            for (std::size_t index = 0U; index < segments.size(); ++index) {
                if (index != 0U) {
                    std::cout << "|";
                }
                std::cout << segments[index];
            }
            std::cout << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-event-cells") {
        try {
            const auto rom = read_file(argv[1]);
            const auto assets =
                shadowrun::graphics::decode_genesis_native_assets(rom);
            print_field_event_cells(assets.field_terrain,
                                    assets.field_event_mode_table);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-event-cells") {
        try {
            const auto rom = read_file(argv[1]);
            const auto assets =
                shadowrun::graphics::decode_genesis_native_assets(rom);
            const auto mode_id = static_cast<std::uint16_t>(
                parse_number(argv[4]));
            const auto area = std::find_if(
                assets.field_areas.begin(), assets.field_areas.end(),
                [mode_id](const shadowrun::native::NativeFieldArea& candidate) {
                    return candidate.mode_id == mode_id;
                });
            if (area == assets.field_areas.end()) {
                throw std::out_of_range("field mode is not in the native catalog");
            }
            std::cout << "field mode " << mode_id << " event cells:\n";
            print_field_event_cells(area->terrain, area->event_mode_table);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--title") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_title_screen(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "startup scene: " << image.width << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-title-text") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_title_text(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native title text layer: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        (std::string(argv[3]) == "--native-start-options" ||
         std::string(argv[3]) == "--native-save-load" ||
         std::string(argv[3]) == "--native-no-save" ||
         std::string(argv[3]) == "--native-archetype" ||
         std::string(argv[3]) == "--native-intro")) {
        try {
            const auto rom = read_file(argv[1]);
            const auto name = std::string(argv[3]).substr(9);
            const auto image = render_native_startup_scene(
                shadowrun::graphics::decode_genesis_native_assets(rom), name);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native startup scene: " << name << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--native-title-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frame = static_cast<std::size_t>(
                parse_number(argv[4]));
            const auto image = render_native_title_frame(
                shadowrun::graphics::decode_genesis_native_assets(rom), frame);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native title frame: " << frame << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-stage") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto stages = shadowrun::graphics::decode_genesis_intro_stages(rom);
            if (index >= stages.size()) {
                throw std::out_of_range("intro stage is outside the ROM stage set");
            }
            shadowrun::graphics::write_bmp32(argv[2], stages[index]);
            std::cout << "intro stage " << index << ": " << stages[index].width
                      << "x" << stages[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-heli") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames = shadowrun::graphics::decode_genesis_intro_heli_frames(rom);
            if (index >= frames.size()) {
                throw std::out_of_range("intro helicopter frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro helicopter frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-aircraft-movement") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_aircraft_movement_frames(
                    rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro aircraft movement frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro aircraft movement frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--intro-aircraft-plane-a") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_intro_aircraft_plane_a(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "intro aircraft Plane-A: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-wilderness-pan") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_wilderness_pan_frames(
                    rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro wilderness pan frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro wilderness pan frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-ambush") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_ambush_frames(rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro ambush frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro ambush frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--cab-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_cab_map(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "cab map: " << image.width << "x" << image.height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--startup-base") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_startup_screen(
                rom, 0x1F310EU, 0x67DF6U, 0x1FF0U);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "startup base scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--title-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_title_animation_frames(rom);
            const auto frame_index = static_cast<std::size_t>(
                parse_number(argv[4]));
            if (frame_index >= frames.size()) {
                throw std::out_of_range("title frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[frame_index]);
            std::cout << "title animation frame " << frame_index << ": "
                      << frames[frame_index].width << "x"
                      << frames[frame_index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field(
                shadowrun::graphics::decode_genesis_native_assets(rom));
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field-menu") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field_menu(
                shadowrun::graphics::decode_genesis_native_assets(rom), false);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field menu scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field-menu-inventory") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field_menu(
                shadowrun::graphics::decode_genesis_native_assets(rom), true);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field inventory menu scene: " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-menu") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_menu_background(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field menu base: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        std::string(argv[3]) == "--field-service-background") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_service_background(
                    rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field service background: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-dialogue-panel") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
                    rom, false);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field dialogue frame: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-dialogue-portrait") {
        try {
            const auto rom = read_file(argv[1]);
            const auto profile = static_cast<std::size_t>(
                parse_number(argv[4]));
            const auto image =
                shadowrun::graphics::decode_genesis_field_dialogue_portrait(
                    rom, profile);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field dialogue portrait profile " << profile << ": "
                      << image.width << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 &&
        std::string(argv[3]) == "--field-mode-profile-sprite") {
        try {
            const auto rom = read_file(argv[1]);
            const auto profile = static_cast<std::size_t>(
                parse_number(argv[4]));
            const auto image =
                shadowrun::ghidra_conversion::rom_research::decode_mode_profile_sprite(
                    rom, profile);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field mode profile sprite " << profile << ": "
                      << image.width << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) ==
                         "--field-dialogue-panel-alternate") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
                    rom, true);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "alternate field dialogue frame: " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-dialogue-buttons") {
        try {
            const auto rom = read_file(argv[1]);
            constexpr std::array<std::string_view, 3> kButtonFiles{{
                "button-a.bmp", "button-b.bmp", "button-c.bmp",
            }};
            const auto output_directory = std::filesystem::path(argv[2]);
            std::filesystem::create_directories(output_directory);
            const auto buttons = shadowrun::graphics::
                decode_genesis_field_dialogue_option_markers(rom);
            for (std::size_t index = 0; index < buttons.size(); ++index) {
                shadowrun::graphics::write_bmp32(
                    (output_directory / kButtonFiles[index]).string(),
                    buttons[index]);
            }
            std::cout << "field dialogue button markers: " << buttons.size()
                      << " 16x16 images\n"
                      << "output directory: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--magic-panel") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_magic_panel_background(
                    rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "magic panel background: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-menu-inventory") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_field_menu_inventory_boxes(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field inventory boxes: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--mode2-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_mode2_field_map(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "mode-2 field map: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--mode-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto mode_id = static_cast<std::uint16_t>(
                parse_number(argv[4]));
            const auto resources =
                shadowrun::graphics::read_genesis_field_mode_resources(
                    rom, mode_id);
            const auto image = shadowrun::graphics::decode_genesis_field_mode_map(
                rom, resources);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field mode " << mode_id << ": " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--entity-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_entity_frame(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "entity frame: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-states") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity state sheet: " << sheet.width << "x"
                      << sheet.height << " (" << frames.size() << " states)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-walking") {
        try {
            const auto rom = read_file(argv[1]);
            const auto animations =
                shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
                    rom);
            const auto frames = flatten_field_entity_animations(animations);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity behavior-transition sheet: "
                      << sheet.width << "x" << sheet.height << " ("
                      << frames.size() << " frames)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-movement") {
        try {
            const auto rom = read_file(argv[1]);
            const auto animations =
                shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
                    rom, 0x7291A, 0x62AC, 0xC062, 0x3EDC, 0x172D8A, 8);
            const auto frames = flatten_field_entity_animations(animations);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity movement sheet: " << sheet.width << "x"
                      << sheet.height << " (" << frames.size() << " frames)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-entity-state") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            if (index >= frames.size()) {
                throw std::out_of_range("field entity state index is out of range");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "field entity state " << index << ": "
                      << frames[index].width << "x" << frames[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--title-boot-assets") {
        try {
            const auto rom = read_file(argv[1]);
            extract_title_boot_assets(rom, argv[2]);
            std::cout << "individual title/boot assets written to "
                      << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        std::string(argv[3]) == "--matrix-cyberdeck-selectors") {
        try {
            const auto rom = read_file(argv[1]);
            MatrixGraphicsUploadHistory history{};
            const auto field_menu = decode_matrix_graphics_upload(
                rom, 0xD86E0U, history);
            const auto menu = decode_matrix_graphics_upload(
                rom, 0xD78AAU, history);
            const auto matrix_ui = decode_matrix_graphics_upload(
                rom, 0xD9376U, history);
            if (field_menu.size() != 139U * 32U ||
                menu.size() != 147U * 32U ||
                matrix_ui.size() != 199U * 32U) {
                throw std::runtime_error(
                    "Matrix Cyberdeck source graphics changed size");
            }
            const auto patterns = shadowrun::graphics::decode_tile_atlas(
                matrix_ui, 0U, 199U,
                shadowrun::graphics::read_genesis_palette(
                    rom, 0x0E2EBAU),
                16U, 0U);
            const std::filesystem::path root(argv[2]);
            for (std::size_t row = 0U; row < 4U; ++row) {
                const auto path = root / "cyberspace" / "matrix" /
                    ("cyberdeck-selector-selected-row-" +
                     std::to_string(row) + ".bmp");
                std::filesystem::create_directories(path.parent_path());
                shadowrun::graphics::write_bmp32(
                    path.string(),
                    compose_matrix_cyberdeck_selector(rom, patterns, row));
            }
            std::cout << "four Cyberdeck selector assets written to "
                      << argv[2]
                      << " using live ROM palette line 1 at 0xE2EBA\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--all-assets") {
        try {
            const auto rom = read_file(argv[1]);
            extract_all_assets(rom, argv[2]);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--matrix-field-atlases") {
        try {
            const auto rom = read_file(argv[1]);
            constexpr std::size_t kPatternCount = 304U;
            const auto resource =
                shadowrun::graphics::decode_genesis_resource_stream(
                    rom, 0xD646AU);
            if (resource.size() != kPatternCount * 32U) {
                throw std::runtime_error(
                    "Matrix field-mode graphics stream changed size");
            }
            const std::filesystem::path root(argv[2]);
            std::filesystem::create_directories(root);
            for (std::size_t theme = 0U; theme < 4U; ++theme) {
                const auto palette_offset = static_cast<std::size_t>(
                    read_big_endian_long(rom, 0x166448U + theme * 4U));
                const auto atlas = shadowrun::graphics::decode_tile_atlas(
                    resource, 0U, kPatternCount,
                    shadowrun::graphics::read_genesis_palette(
                        rom, palette_offset),
                    16U, 0U);
                const auto path = root /
                    ("matrix-field-pattern-atlas-theme-" +
                     std::to_string(theme + 1U) + ".bmp");
                shadowrun::graphics::write_bmp32(path.string(), atlas);
            }
            std::cout << "four 304-tile Matrix field-mode atlases written to "
                      << argv[2]
                      << " using palette pointers at 0x166448\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        std::string(argv[3]) == "--matrix-world-models") {
        try {
            const auto rom = read_file(argv[1]);
            constexpr std::size_t kModelTable = 0x0E2EDAU;
            constexpr std::size_t kModelCount = 6U;
            constexpr std::size_t kModelRecordBytes = 12U;
            // World model SAT packets use palette line 0. Matrix field-mode
            // entry copies the standard four-line palette block from E2E3A.
            constexpr std::size_t kPaletteSource = 0x0E2E3AU;
            const auto palette =
                shadowrun::graphics::read_genesis_palette(rom,
                                                          kPaletteSource);
            const std::filesystem::path root(argv[2]);
            std::filesystem::create_directories(root);
            for (std::size_t model = 0U; model < kModelCount; ++model) {
                const auto record = kModelTable +
                    model * kModelRecordBytes;
                const auto width_pixels = static_cast<std::size_t>(
                    read_big_endian_word(rom, record));
                const auto height_pixels = static_cast<std::size_t>(
                    read_big_endian_word(rom, record + 2U));
                const auto map_source = static_cast<std::size_t>(
                    read_big_endian_long(rom, record + 4U));
                const auto pattern_source = static_cast<std::size_t>(
                    read_big_endian_long(rom, record + 8U));
                if (width_pixels == 0U || height_pixels == 0U ||
                    width_pixels % 8U != 0U || height_pixels % 8U != 0U) {
                    throw std::runtime_error(
                        "Matrix world-object model has non-tile dimensions");
                }
                const auto columns = width_pixels / 8U;
                const auto rows = height_pixels / 8U;
                const auto map_bytes = columns * rows * 2U;
                if (map_source > rom.size() ||
                    map_bytes > rom.size() - map_source) {
                    throw std::runtime_error(
                        "Matrix world-object model tile map exceeds the ROM");
                }
                std::size_t maximum_tile = 0U;
                std::vector<std::uint16_t> tile_map;
                tile_map.reserve(columns * rows);
                for (std::size_t index = 0U; index < columns * rows;
                     ++index) {
                    const auto tile = static_cast<std::size_t>(
                        read_big_endian_word(rom,
                                             map_source + index * 2U));
                    maximum_tile = std::max(maximum_tile, tile);
                    tile_map.push_back(static_cast<std::uint16_t>(tile));
                }
                const auto tile_count = maximum_tile + 1U;
                const auto pattern_bytes = tile_count * 32U;
                if (pattern_source > rom.size() ||
                    pattern_bytes > rom.size() - pattern_source) {
                    throw std::runtime_error(
                        "Matrix world-object model pattern bank exceeds the ROM");
                }
                const std::vector<std::uint8_t> patterns(
                    rom.begin() + static_cast<std::ptrdiff_t>(pattern_source),
                    rom.begin() + static_cast<std::ptrdiff_t>(
                        pattern_source + pattern_bytes));
                const auto atlas = shadowrun::graphics::decode_tile_atlas(
                    patterns, 0U, tile_count, palette, 16U, 0U);
                shadowrun::graphics::RgbaImage model_image{};
                model_image.width =
                    static_cast<std::uint16_t>(width_pixels);
                model_image.height =
                    static_cast<std::uint16_t>(height_pixels);
                model_image.pixels.assign(
                    width_pixels * height_pixels,
                    shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
                for (std::size_t row = 0U; row < rows; ++row) {
                    for (std::size_t column = 0U; column < columns;
                         ++column) {
                        const auto tile = static_cast<std::size_t>(
                            tile_map[row * columns + column]);
                        for (std::size_t pixel_y = 0U; pixel_y < 8U;
                             ++pixel_y) {
                            for (std::size_t pixel_x = 0U; pixel_x < 8U;
                                 ++pixel_x) {
                                const auto atlas_x = (tile % 16U) * 8U +
                                    pixel_x;
                                const auto atlas_y = (tile / 16U) * 8U +
                                    pixel_y;
                                model_image.pixels[
                                    (row * 8U + pixel_y) * width_pixels +
                                    column * 8U + pixel_x] = atlas.pixels[
                                    atlas_y * atlas.width + atlas_x];
                            }
                        }
                    }
                }
                const auto path = root /
                    ("matrix-world-model-" + std::to_string(model) +
                     ".png");
                write_png32(path.string(), model_image);
                const auto alternate_palette =
                    shadowrun::graphics::read_genesis_palette(
                        rom, kPaletteSource + 0x20U);
                const auto alternate_atlas =
                    shadowrun::graphics::decode_tile_atlas(
                        patterns, 0U, tile_count, alternate_palette, 16U, 0U);
                auto alternate_image = model_image;
                for (std::size_t row = 0U; row < rows; ++row) {
                    for (std::size_t column = 0U; column < columns;
                         ++column) {
                        const auto tile = static_cast<std::size_t>(
                            tile_map[row * columns + column]);
                        for (std::size_t pixel_y = 0U; pixel_y < 8U;
                             ++pixel_y) {
                            for (std::size_t pixel_x = 0U; pixel_x < 8U;
                                 ++pixel_x) {
                                const auto atlas_x = (tile % 16U) * 8U +
                                    pixel_x;
                                const auto atlas_y = (tile / 16U) * 8U +
                                    pixel_y;
                                alternate_image.pixels[
                                    (row * 8U + pixel_y) * width_pixels +
                                    column * 8U + pixel_x] =
                                    alternate_atlas.pixels[
                                        atlas_y * alternate_atlas.width +
                                        atlas_x];
                            }
                        }
                    }
                }
                const auto alternate_path = root /
                    ("matrix-world-model-" + std::to_string(model) +
                     "-alternate-palette.png");
                write_png32(alternate_path.string(), alternate_image);
                std::cout << "model " << model << " " << width_pixels << 'x'
                          << height_pixels << " map=0x" << std::hex
                          << map_source << " patterns=0x" << pattern_source
                          << " tiles=" << std::dec << tile_count << " -> "
                          << path.string() << "\n";
            }
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        std::string(argv[3]) == "--scene-5bd2c-sprite-lists") {
        try {
            const auto rom = read_file(argv[1]);
            // The field callback at 0x5BAC4 loads descriptor 0x5C966. Its
            // second pattern stream is the odd pointer 0x1F002F (stream data
            // at 0x1F002E), uploaded at VRAM byte address $8A80 (tile $454).
            // The callback's sprite lists at 0x1EF988 and 0x1EFAAA select
            // from this bank. Keep this source path separate from the still-
            // untraced live Matrix traversal sprite path.
            constexpr std::size_t kPatternStream = 0x1F002EU;
            constexpr std::size_t kPatternFirstTile = 0x0454U;
            constexpr std::size_t kPaletteLine1 = 0x5C9F2U;
            constexpr std::array<std::size_t, 2> kLists{{
                0x1EF988U, 0x1EFAAAU,
            }};
            constexpr std::array<const char*, 2> kNames{{
                "list-1ef988", "list-1efaaa",
            }};
            const auto pattern_bytes =
                shadowrun::graphics::decode_genesis_resource_stream(
                    rom, kPatternStream);
            if (pattern_bytes.empty() || pattern_bytes.size() % 32U != 0U) {
                throw std::runtime_error(
                    "0x5BD2C sprite pattern stream has an invalid tile length");
            }
            const auto tile_count = pattern_bytes.size() / 32U;
            const auto palette = shadowrun::graphics::read_genesis_palette(
                rom, kPaletteLine1);
            constexpr std::size_t kTilesPerRow = 16U;
            const auto atlas = shadowrun::graphics::decode_tile_atlas(
                pattern_bytes, 0U, tile_count, palette, kTilesPerRow, 0U);
            const std::filesystem::path root(argv[2]);
            std::filesystem::create_directories(root);
            constexpr std::size_t kWidth = 320U;
            constexpr std::size_t kHeight = 224U;
            for (std::size_t list_index = 0U;
                 list_index < kLists.size(); ++list_index) {
                shadowrun::graphics::RgbaImage image{};
                image.width = static_cast<std::uint16_t>(kWidth);
                image.height = static_cast<std::uint16_t>(kHeight);
                image.pixels.assign(
                    kWidth * kHeight,
                    shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
                auto record = kLists[list_index];
                bool terminated = false;
                for (std::size_t sprite_index = 0U;
                     sprite_index < 64U; ++sprite_index) {
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                            "0x5BD2C sprite list is truncated");
                    }
                    const auto x_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(rom, record + 4U);
                    const auto attributes = read_big_endian_word(
                        rom, record + 6U);
                    const auto tile = static_cast<std::size_t>(
                        attributes & 0x07FFU);
                    const auto width_tiles = static_cast<std::size_t>(
                        ((size >> 2U) & 0x03U) + 1U);
                    const auto height_tiles = static_cast<std::size_t>(
                        (size & 0x03U) + 1U);
                    if ((size & 0xFF00U) != 0U ||
                        tile < kPatternFirstTile ||
                        tile - kPatternFirstTile > tile_count ||
                        width_tiles * height_tiles >
                            tile_count - (tile - kPatternFirstTile)) {
                        throw std::runtime_error(
                            "0x5BD2C sprite exceeds its uploaded pattern bank");
                    }
                    const auto tile_offset = tile - kPatternFirstTile;
                    const bool flip_x = (attributes & 0x0800U) != 0U;
                    const bool flip_y = (attributes & 0x1000U) != 0U;
                    const auto sprite_width = width_tiles * 8U;
                    const auto sprite_height = height_tiles * 8U;
                    for (std::size_t local_y = 0U;
                         local_y < sprite_height; ++local_y) {
                        for (std::size_t local_x = 0U;
                             local_x < sprite_width; ++local_x) {
                            const auto source_x = flip_x
                                ? sprite_width - 1U - local_x
                                : local_x;
                            const auto source_y = flip_y
                                ? sprite_height - 1U - local_y
                                : local_y;
                            const auto tile_index = tile_offset +
                                (source_x / 8U) * height_tiles +
                                (source_y / 8U);
                            const auto atlas_x =
                                (tile_index % kTilesPerRow) * 8U +
                                (source_x % 8U);
                            const auto atlas_y =
                                (tile_index / kTilesPerRow) * 8U +
                                (source_y % 8U);
                            const auto source_pixel = atlas.pixels[
                                atlas_y * atlas.width + atlas_x];
                            if (source_pixel.alpha == 0U) {
                                continue;
                            }
                            const auto target_x = 160 + x_offset +
                                static_cast<int>(local_x);
                            const auto target_y = 112 + y_offset +
                                static_cast<int>(local_y);
                            if (target_x < 0 || target_y < 0 ||
                                target_x >= static_cast<int>(kWidth) ||
                                target_y >= static_cast<int>(kHeight)) {
                                continue;
                            }
                            image.pixels[
                                static_cast<std::size_t>(target_y) * kWidth +
                                static_cast<std::size_t>(target_x)] = source_pixel;
                        }
                    }
                    record += 8U;
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "0x5BD2C sprite list has no terminator");
                }
                const auto output = root /
                    (std::string("scene-5bd2c-") + kNames[list_index] +
                     "-centered.png");
                write_png32(output.string(), image);
                std::cout << kNames[list_index] << " sprites rendered from "
                          << tile_count << " decoded patterns -> "
                          << output.string() << "\n";
            }
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        std::string(argv[3]) == "--matrix-title-figure-atlases") {
        try {
            const auto rom = read_file(argv[1]);
            struct FigurePatternGroup {
                const char* name;
                std::size_t pointer_table;
                std::size_t destination;
                std::uint16_t base_x;
                std::uint16_t base_y;
            };
            // ROM 0x1D428/0x1D452 and 0x1DAF4/0x1DB08/0xEA490/0xEA4A4
            // belong to the FFDF8C world-sequence transition. The normal
            // traversal behavior descriptor C selects fixed lists through
            // two phase tables rooted at 0x1C2D2 and 0x1C2C8.
            constexpr std::array<FigurePatternGroup, 4> kGroups{{
                {"upper-left", 0x1DAF4U, 0xF000U, 0x00F8U, 0x00E4U},
                {"upper-right", 0x1DB08U, 0xF000U, 0x0148U, 0x00E4U},
                {"lower-left", 0x0EA490U, 0xF800U, 0x00E2U, 0x0114U},
                {"lower-right", 0x0EA4A4U, 0xF800U, 0x015EU, 0x0114U},
            }};
            const auto palette = shadowrun::graphics::read_genesis_palette(
                rom, 0x0E2E9AU);
            const std::filesystem::path root(argv[2]);
            std::filesystem::create_directories(root);
            constexpr std::size_t kFrameWidth = 320U;
            constexpr std::size_t kFrameHeight = 224U;
            std::array<shadowrun::graphics::RgbaImage, 5> figures{};
            for (auto& figure : figures) {
                figure.width = static_cast<std::uint16_t>(kFrameWidth);
                figure.height = static_cast<std::uint16_t>(kFrameHeight);
                figure.pixels.assign(kFrameWidth * kFrameHeight,
                                     shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
            }
            constexpr std::size_t kFixedFigurePatternOffset = 0xDA038U;
            constexpr std::size_t kFixedFigurePatternBytes = 0x0A80U;
            constexpr std::size_t kFixedFigureFirstTile = 0x011BU;
            constexpr std::size_t kFixedFigureTileCount =
                kFixedFigurePatternBytes / 32U;
            constexpr std::size_t kFixedFigureTilesPerRow = 16U;
            if (kFixedFigurePatternOffset > rom.size() ||
                kFixedFigurePatternBytes >
                    rom.size() - kFixedFigurePatternOffset) {
                throw std::runtime_error(
                    "Matrix fixed-figure pattern bank is truncated");
            }
            const std::vector<std::uint8_t> fixed_figure_patterns(
                rom.begin() + static_cast<std::ptrdiff_t>(
                    kFixedFigurePatternOffset),
                rom.begin() + static_cast<std::ptrdiff_t>(
                    kFixedFigurePatternOffset +
                    kFixedFigurePatternBytes));
            const auto fixed_figure_atlas =
                shadowrun::graphics::decode_tile_atlas(
                    fixed_figure_patterns, 0U, kFixedFigureTileCount,
                    palette, kFixedFigureTilesPerRow, 0U);
            const auto draw_figure_sprite = [&] (
                shadowrun::graphics::RgbaImage& figure,
                const shadowrun::graphics::RgbaImage& atlas,
                const std::size_t atlas_tile_count,
                const std::size_t atlas_tiles_per_row,
                const std::size_t sprite_tile_first,
                const std::size_t sprite_tile_bank_first,
                const std::uint8_t size,
                const std::uint16_t attributes,
                const int origin_x,
                const int origin_y) {
                if (sprite_tile_first < sprite_tile_bank_first ||
                    atlas_tiles_per_row == 0U) {
                    throw std::runtime_error(
                        "Matrix figure sprite starts outside its tile bank");
                }
                const auto width_tiles = static_cast<std::size_t>(
                    ((size >> 2U) & 0x03U) + 1U);
                const auto height_tiles = static_cast<std::size_t>(
                    (size & 0x03U) + 1U);
                const auto tile_offset = sprite_tile_first -
                    sprite_tile_bank_first;
                const auto sprite_tile_count = width_tiles * height_tiles;
                if (tile_offset > atlas_tile_count ||
                    sprite_tile_count > atlas_tile_count - tile_offset) {
                    throw std::runtime_error(
                        "Matrix figure sprite exceeds its decoded tile bank");
                }
                const bool flip_x = (attributes & 0x0800U) != 0U;
                const bool flip_y = (attributes & 0x1000U) != 0U;
                const auto sprite_width = width_tiles * 8U;
                const auto sprite_height = height_tiles * 8U;
                for (std::size_t local_y = 0U;
                     local_y < sprite_height; ++local_y) {
                    for (std::size_t local_x = 0U;
                         local_x < sprite_width; ++local_x) {
                        const auto source_x = flip_x
                            ? sprite_width - 1U - local_x
                            : local_x;
                        const auto source_y = flip_y
                            ? sprite_height - 1U - local_y
                            : local_y;
                        // Genesis sprite patterns advance vertically through
                        // each tile column before moving to the next column.
                        const auto tile_index = tile_offset +
                            (source_x / 8U) * height_tiles +
                            (source_y / 8U);
                        const auto atlas_x =
                            (tile_index % atlas_tiles_per_row) * 8U +
                            (source_x % 8U);
                        const auto atlas_y =
                            (tile_index / atlas_tiles_per_row) * 8U +
                            (source_y % 8U);
                        const auto source_pixel = atlas.pixels[
                            atlas_y * atlas.width + atlas_x];
                        if (source_pixel.alpha == 0U) {
                            continue;
                        }
                        const auto target_x = origin_x +
                            static_cast<int>(local_x);
                        const auto target_y = origin_y +
                            static_cast<int>(local_y);
                        if (target_x < 0 || target_y < 0 ||
                            target_x >= static_cast<int>(kFrameWidth) ||
                            target_y >= static_cast<int>(kFrameHeight)) {
                            continue;
                        }
                        figure.pixels[
                            static_cast<std::size_t>(target_y) *
                                kFrameWidth +
                            static_cast<std::size_t>(target_x)] =
                            source_pixel;
                    }
                }
            };
            constexpr std::array<std::size_t, 2> kFixedFigureLists{{
                0x1D428U, 0x1D452U,
            }};
            // 0x1C506 loads D1 from FF158E=0x120 and D2 from
            // FF1590=0x12F; 0x1378 writes D1 to SAT X and D2 to SAT Y.
            constexpr int kFixedFigureBaseX = 0x0120 - 128;
            constexpr int kFixedFigureBaseY = 0x012F - 128;
            for (auto& figure : figures) {
                for (const auto list : kFixedFigureLists) {
                    auto record = list;
                    bool terminated = false;
                    for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                        if (record > rom.size() || rom.size() - record < 8U) {
                            throw std::runtime_error(
                                "Matrix fixed-figure SAT list is truncated");
                        }
                        const auto x_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record));
                        if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                            terminated = true;
                            break;
                        }
                        const auto y_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record + 2U));
                        const auto size_word = read_big_endian_word(
                            rom, record + 4U);
                        if ((size_word & 0xFF00U) != 0U) {
                            throw std::runtime_error(
                                "Matrix fixed-figure size exceeds one byte");
                        }
                        const auto attributes = static_cast<std::uint16_t>(
                            read_big_endian_word(rom, record + 6U) |
                            0x6000U);
                        const auto tile = static_cast<std::size_t>(
                            attributes & 0x07FFU);
                        draw_figure_sprite(
                            figure, fixed_figure_atlas,
                            kFixedFigureTileCount, kFixedFigureTilesPerRow,
                            tile, kFixedFigureFirstTile,
                            static_cast<std::uint8_t>(size_word), attributes,
                            kFixedFigureBaseX + x_offset,
                            kFixedFigureBaseY + y_offset);
                        record += 8U;
                    }
                    if (!terminated) {
                        throw std::runtime_error(
                            "Matrix fixed-figure SAT list has no terminator");
                    }
                }
            }

            // Field entry copies the bootstrap tables at 0x1C2B4/0x1C2BE,
            // then 0x1B628 immediately attaches descriptor C, selecting the
            // phase tables at 0x1C2D2/0x1C2C8. Each table's first word is its
            // maximum phase index; following longwords are ROM sprite lists.
            // Rendering order is primary (FF158D) then secondary (FF158C).
            // 0x1C506 passes FF158E=0x120 as D1/X and FF1590=0x12F as D2/Y;
            // 0x1378 writes D1 to SAT X and D2 to SAT Y.
            constexpr std::array<std::size_t, 2> kLiveFigureTables{{
                0x1C2D2U, 0x1C2C8U,
            }};
            constexpr int kLiveFigureBaseX = 0x0120 - 128;
            constexpr int kLiveFigureBaseY = 0x012F - 128;
            // ROM 0x1C506 checks bit 1 in the copied 0x1C4DA flags, then
            // redraws A2/A4 at the selected Y offset from 0x1C5EC. The row
            // is FFE169-1 (clamped to three); the column is FFDF88 & 3.
            constexpr std::array<std::array<int, 4>, 4>
                kLiveFigureOverlayYOffsets{{
                    {{0x003C, 0x0000, 0x0000, 0x0000}},
                    {{0x003C, -0x003C, 0x0000, 0x0000}},
                    {{0x003C, -0x003C, 0x0078, 0x0000}},
                    {{0x003C, -0x003C, 0x0078, -0x0078}},
                }};
            const bool live_figure_overlay_enabled =
                (rom[0x1C4DAU + 0x12U] & 0x02U) != 0U;
            const auto live_overlay_directory =
                root / "matrix-live-runner-overlay-states";
            std::filesystem::create_directories(live_overlay_directory);
            const auto draw_live_figure_list = [&] (
                shadowrun::graphics::RgbaImage& pose,
                const std::size_t list,
                const int y_offset) {
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                    if (list > rom.size() ||
                        rom.size() - list < (sprite + 1U) * 8U) {
                        throw std::runtime_error(
                            "Matrix live-figure SAT list is truncated");
                    }
                    const auto record = list + sprite * 8U;
                    const auto x = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(
                        rom, record + 4U);
                    const auto attributes = static_cast<std::uint16_t>(
                        read_big_endian_word(rom, record + 6U) | 0x6000U);
                    const auto tile = static_cast<std::size_t>(
                        attributes & 0x07FFU);
                    if ((size & 0xFF00U) != 0U) {
                        throw std::runtime_error(
                            "Matrix live-figure size exceeds one byte");
                    }
                    draw_figure_sprite(
                        pose, fixed_figure_atlas, kFixedFigureTileCount,
                        kFixedFigureTilesPerRow, tile,
                        kFixedFigureFirstTile,
                        static_cast<std::uint8_t>(size), attributes,
                        kLiveFigureBaseX + x,
                        kLiveFigureBaseY + y + y_offset);
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix live-figure SAT list has no terminator");
                }
            };
            for (std::size_t primary_phase = 0U; primary_phase < 2U;
                 ++primary_phase) {
                for (std::size_t secondary_phase = 0U;
                     secondary_phase < 2U; ++secondary_phase) {
                    const std::array<std::size_t, 2> phases{{
                        primary_phase, secondary_phase,
                    }};
                    std::array<std::size_t, 2> lists{};
                    for (std::size_t layer = 0U; layer < 2U; ++layer) {
                        const auto table = kLiveFigureTables[layer];
                        const auto maximum_phase = static_cast<std::size_t>(
                            read_big_endian_word(rom, table));
                        if (phases[layer] > maximum_phase) {
                            throw std::runtime_error(
                                "Matrix live-figure phase exceeds its ROM table");
                        }
                        lists[layer] = static_cast<std::size_t>(
                            read_big_endian_long(
                                rom, table + 2U + phases[layer] * 4U));
                    }
                    for (std::size_t overlay_row = 0U; overlay_row < 4U;
                         ++overlay_row) {
                        for (std::size_t frame_column = 0U;
                             frame_column < 4U; ++frame_column) {
                            shadowrun::graphics::RgbaImage pose{};
                            pose.width =
                                static_cast<std::uint16_t>(kFrameWidth);
                            pose.height =
                                static_cast<std::uint16_t>(kFrameHeight);
                            pose.pixels.assign(
                                kFrameWidth * kFrameHeight,
                                shadowrun::native::Rgba8{
                                    0U, 0U, 0U, 0U});
                            for (std::size_t layer = 0U; layer < 2U;
                                 ++layer) {
                                draw_live_figure_list(pose, lists[layer], 0);
                                const auto offset =
                                    kLiveFigureOverlayYOffsets[overlay_row]
                                        [frame_column];
                                if (live_figure_overlay_enabled &&
                                    offset != 0) {
                                    draw_live_figure_list(
                                        pose, lists[layer], offset);
                                }
                            }
                            const auto filename =
                                "matrix-live-runner-pose-primary-" +
                                std::to_string(primary_phase) +
                                "-secondary-" +
                                std::to_string(secondary_phase) +
                                "-overlay-row-" +
                                std::to_string(overlay_row) + "-frame-" +
                                std::to_string(frame_column) + ".png";
                            const auto path = live_overlay_directory /
                                filename;
                            write_png32(path.string(), pose);
                            if (overlay_row == 0U && frame_column == 1U) {
                                const auto baseline_path = root /
                                    ("matrix-live-runner-pose-primary-" +
                                     std::to_string(primary_phase) +
                                     "-secondary-" +
                                     std::to_string(secondary_phase) +
                                     ".png");
                                write_png32(baseline_path.string(), pose);
                            }
                        }
                    }
                }
            }
            for (const auto& group : kGroups) {
                for (std::size_t phase = 0U; phase < figures.size(); ++phase) {
                    const auto descriptor = static_cast<std::size_t>(
                        read_big_endian_long(
                            rom, group.pointer_table + phase * 4U));
                    const auto pattern_source = static_cast<std::size_t>(
                        read_big_endian_long(rom, descriptor + 2U));
                    const auto pattern_byte_count = static_cast<std::size_t>(
                        read_big_endian_word(rom, descriptor + 6U));
                    // append_sprite_record queues this byte count for
                    // write_vdp_word_block_with_source_wrap (0x0F5C). Its
                    // D2 loop consumes 64-byte blocks, then trailing words
                    // and an optional final word. The first ROM block is
                    // 0xC0 bytes and ends exactly at its following descriptor.
                    const auto pattern_length = pattern_byte_count;
                    if (pattern_length == 0U || pattern_length % 32U != 0U ||
                        pattern_source > rom.size() ||
                        pattern_length > rom.size() - pattern_source ||
                        group.destination > 0x10000U ||
                        pattern_length > 0x10000U - group.destination) {
                        throw std::runtime_error(
                            "Matrix figure pattern block exceeds ROM or VRAM");
                    }
                    // append_sprite_record (ROM 0x61CC) queues this source
                    // pointer and byte length for a direct DMA to key << 5.
                    // The descriptor does not reference a 0x1530 stream.
                    std::vector<std::uint8_t> pattern_vram(0x10000U, 0U);
                    std::copy_n(rom.begin() +
                                    static_cast<std::ptrdiff_t>(pattern_source),
                                pattern_length,
                                pattern_vram.begin() +
                                    static_cast<std::ptrdiff_t>(
                                        group.destination));
                    const auto tile_count = pattern_length / 32U;
                    const auto atlas = shadowrun::graphics::decode_tile_atlas(
                        pattern_vram, group.destination, tile_count, palette,
                        tile_count, 0U);
                    const auto atlas_path = root /
                        (std::string("matrix-title-figure-") + group.name +
                         "-phase-" + std::to_string(phase) + ".bmp");
                    shadowrun::graphics::write_bmp32(
                        atlas_path.string(), atlas);

                    auto record = descriptor + 8U;
                    bool terminated = false;
                    for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                        if (record > rom.size() || rom.size() - record < 8U) {
                            throw std::runtime_error(
                                "Matrix figure SAT list is truncated");
                        }
                        const auto x_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record));
                        if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                            terminated = true;
                            break;
                        }
                        const auto y_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record + 2U));
                        const auto size = read_big_endian_word(rom, record + 4U);
                        const auto tile_and_flip = read_big_endian_word(
                            rom, record + 6U);
                        // 0x13BE writes the source byte into the high byte of
                        // the Genesis SAT size/link word. In that byte, bits
                        // 0-1 encode vertical size and bits 2-3 horizontal
                        // size.
                        const auto first_tile = static_cast<std::size_t>(
                            tile_and_flip & 0x07FFU);
                        if ((size & 0xFF00U) != 0U ||
                            (tile_and_flip & 0xF000U) != 0x0000U &&
                                (tile_and_flip & 0xF000U) != 0x0800U) {
                            throw std::runtime_error(
                                "Matrix figure SAT entry exceeds its pattern block");
                        }
                        auto& figure = figures[phase];
                        const auto origin_x =
                            static_cast<int>(group.base_x) + x_offset - 128;
                        const auto origin_y =
                            static_cast<int>(group.base_y) + y_offset - 128;
                        draw_figure_sprite(
                            figure, atlas, tile_count, tile_count, first_tile,
                            0U, static_cast<std::uint8_t>(size),
                            tile_and_flip, origin_x, origin_y);
                        record += 8U;
                    }
                    if (!terminated) {
                        throw std::runtime_error(
                            "Matrix figure SAT list has no terminator");
                    }
                    std::cout << group.name << " phase " << phase
                              << " pattern=0x" << std::hex << pattern_source
                              << " bytes=0x" << pattern_length
                              << " SAT=0x" << descriptor + 8U << std::dec
                              << " -> " << atlas_path.string() << "\n";
                }
            }
            for (std::size_t phase = 0U; phase < figures.size(); ++phase) {
                const auto path = root /
                    ("matrix-title-figure-composed-phase-" +
                     std::to_string(phase) + ".png");
                write_png32(path.string(), figures[phase]);
            }

            // ROM 0x1ADE6 attaches descriptor E (0x1B680), whose live lists
            // are at 0x1C34A/0x1C344 and use the 0xDA038 field-pattern bank.
            // Render its two active source lists independently from descriptor
            // C and the world-sequence reveal figure above.
            const auto make_transparent_figure = [&]() {
                shadowrun::graphics::RgbaImage figure{};
                figure.width = static_cast<std::uint16_t>(kFrameWidth);
                figure.height = static_cast<std::uint16_t>(kFrameHeight);
                figure.pixels.assign(
                    kFrameWidth * kFrameHeight,
                    shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
                return figure;
            };
            constexpr std::array<std::size_t, 2> kDescriptorEListTables{{
                0x1C34AU, 0x1C344U,
            }};
            constexpr std::size_t kDescriptorEFirstTile = 0x011BU;
            // These use the same D1/D2 origin as 0x1C506 above: X=FF158E,
            // Y=FF1590. Keeping memory-field order separate from screen-axis
            // order avoids swapping the centered runner by 15 pixels.
            constexpr int kDescriptorEBaseX = 0x0120U - 128;
            constexpr int kDescriptorEBaseY = 0x012FU - 128;
            const auto draw_descriptor_e_list = [&] (
                shadowrun::graphics::RgbaImage& figure,
                const std::size_t list) {
                auto record = list;
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                            "Matrix descriptor-E sprite list is truncated");
                    }
                    const auto x_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(rom, record + 4U);
                    const auto attributes = read_big_endian_word(
                        rom, record + 6U);
                    if ((size & 0xFF00U) != 0U ||
                        (attributes & 0xF000U) != 0x8000U) {
                        throw std::runtime_error(
                            "Matrix descriptor-E sprite has unexpected attributes");
                    }
                    const auto first_tile = static_cast<std::size_t>(
                        attributes & 0x07FFU);
                    draw_figure_sprite(
                        figure, fixed_figure_atlas, kFixedFigureTileCount,
                        kFixedFigureTilesPerRow, first_tile,
                        kDescriptorEFirstTile,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(attributes | 0x6000U),
                        kDescriptorEBaseX + x_offset,
                        kDescriptorEBaseY + y_offset);
                    record += 8U;
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix descriptor-E sprite list has no terminator");
                }
            };
            const auto descriptor_e_primary_max = static_cast<std::size_t>(
                read_big_endian_word(rom, kDescriptorEListTables[0]));
            const auto descriptor_e_secondary_max = static_cast<std::size_t>(
                read_big_endian_word(rom, kDescriptorEListTables[1]));
            auto descriptor_e_figure = make_transparent_figure();
            for (std::size_t primary_phase = 0U;
                 primary_phase <= descriptor_e_primary_max; ++primary_phase) {
                const auto primary_list = static_cast<std::size_t>(
                    read_big_endian_long(
                        rom, kDescriptorEListTables[0] + 2U +
                            primary_phase * 4U));
                for (std::size_t secondary_phase = 0U;
                     secondary_phase <= descriptor_e_secondary_max;
                     ++secondary_phase) {
                    const auto secondary_list = static_cast<std::size_t>(
                        read_big_endian_long(
                            rom, kDescriptorEListTables[1] + 2U +
                                secondary_phase * 4U));
                    auto figure = make_transparent_figure();
                    // 0x1C506 draws the primary A2 list before the secondary
                    // A4 list. Preserve that source layer order per frame.
                    draw_descriptor_e_list(figure, primary_list);
                    draw_descriptor_e_list(figure, secondary_list);
                    const auto path = root /
                        ("matrix-world-effect-descriptor-e-primary-" +
                         std::to_string(primary_phase) + "-secondary-" +
                         std::to_string(secondary_phase) + ".png");
                    write_png32(path.string(), figure);
                    if (primary_phase == 0U && secondary_phase == 0U) {
                        descriptor_e_figure = figure;
                        write_png32(
                            (root / "matrix-world-effect-descriptor-e.png")
                                .string(),
                            figure);
                    }
                }
            }
            const auto descriptor_e_path =
                root / "matrix-world-effect-descriptor-e.png";
            write_png32(descriptor_e_path.string(), descriptor_e_figure);

            // Descriptor D (0x1B654) instead selects the 0x1C31E/0x1C318
            // lists from the 0xDAAB8 upload at VRAM $2DE0 (tile $16F).
            const auto descriptor_d_patterns =
                shadowrun::graphics::decode_genesis_resource_stream(
                    rom, 0x0DAAB8U);
            if (descriptor_d_patterns.size() % 32U != 0U) {
                throw std::runtime_error(
                    "Matrix descriptor-D pattern stream is not tile aligned");
            }
            const auto descriptor_d_tile_count =
                descriptor_d_patterns.size() / 32U;
            const auto descriptor_d_atlas =
                shadowrun::graphics::decode_tile_atlas(
                    descriptor_d_patterns, 0U, descriptor_d_tile_count,
                    palette, 16U, 0U);
            constexpr std::size_t kDescriptorDFirstTile = 0x016FU;
            // The traversal loads the 4-line palette block from 0xE2E3A;
            // D6=0x2000 selects line 1 at 0xE2E5A. 0xE2EBA is a later
            // Cyberdeck-only replacement and must not color this effect.
            const auto hand_ring_palette =
                shadowrun::graphics::read_genesis_palette(rom, 0x0E2E5AU);
            const auto hand_ring_atlas =
                shadowrun::graphics::decode_tile_atlas(
                    descriptor_d_patterns, 0U, descriptor_d_tile_count,
                    hand_ring_palette, 16U, 0U);

            // Emit the source-selected phase images as an offline diagnostic.
            for (std::size_t phase = 0U; phase < 4U; ++phase) {
                const auto image = compose_matrix_analyze_ring_phase(
                    rom, hand_ring_atlas, descriptor_d_tile_count, phase);
                const auto list = read_big_endian_long(
                    rom, 0x1C136U + phase * 4U);
                const auto output = root /
                    ("matrix-world-effect-hand-ring-phase-" +
                     std::to_string(phase) + ".png");
                write_png32(output.string(), image);
                std::cout << "Matrix hand-ring phase " << phase
                          << " list=0x" << std::hex << list << std::dec
                          << " -> " << output.string() << "\n";
            }
            const auto descriptor_d_atlas_path =
                root / "matrix-world-effect-descriptor-d-pattern-atlas.png";
            write_png32(
                descriptor_d_atlas_path.string(), descriptor_d_atlas);

            // 0x1C61C mirrors paired lists from 0x1C6BC around the runner
            // while advancing the 32-entry 0x1C6D4 phase/offset table. The
            // resulting motifs land near the hands but do not by themselves
            // explain the large circular outlines in the capture.
            const auto draw_title_sprite_list = [&] (
                shadowrun::graphics::RgbaImage& image,
                std::size_t list,
                const int base_x,
                const int base_y,
                const std::uint16_t attribute_offset) {
                for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                    if (list > rom.size() || rom.size() - list < 2U) {
                        throw std::runtime_error(
                            "Matrix title-sweep sprite list is truncated");
                    }
                    const auto x = static_cast<std::int16_t>(
                        read_big_endian_word(rom, list));
                    list += 2U;
                    if (x == std::numeric_limits<std::int16_t>::min()) {
                        return list;
                    }
                    if (list > rom.size() || rom.size() - list < 6U) {
                        throw std::runtime_error(
                            "Matrix title-sweep sprite record is truncated");
                    }
                    const auto y = static_cast<std::int16_t>(
                        read_big_endian_word(rom, list));
                    const auto size = read_big_endian_word(rom, list + 2U);
                    const auto source_attributes = read_big_endian_word(
                        rom, list + 4U);
                    list += 6U;
                    const auto first_tile = static_cast<std::size_t>(
                        source_attributes & 0x07FFU);
                    if ((size & 0xFF00U) != 0U ||
                        (source_attributes & 0xF000U) != 0x8000U ||
                        first_tile < kDescriptorDFirstTile) {
                        throw std::runtime_error(
                            "Matrix title-sweep sprite has unexpected source data");
                    }
                    draw_figure_sprite(
                        image, descriptor_d_atlas, descriptor_d_tile_count,
                        16U, first_tile, kDescriptorDFirstTile,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(
                            source_attributes | attribute_offset),
                        base_x + x, base_y + y);
                }
                throw std::runtime_error(
                    "Matrix title-sweep sprite list has no terminator");
            };
            for (std::size_t phase = 0U; phase < 32U; ++phase) {
                const auto entry = 0x1C6D4U + phase * 4U;
                const auto list_index = static_cast<std::size_t>(rom[entry]);
                const auto offset_byte = rom[entry + 1U];
                const auto x_offset = static_cast<int>(
                    static_cast<std::int8_t>(offset_byte));
                if (list_index >= 6U) {
                    throw std::runtime_error(
                        "Matrix title-sweep phase selects an unknown list");
                }
                const auto first_list = static_cast<std::size_t>(
                    read_big_endian_long(
                        rom, 0x1C6BCU + list_index * 4U));
                auto image = make_transparent_figure();
                const auto first_end = draw_title_sprite_list(
                    image, first_list,
                    0x0120 + x_offset - 128, 0x012F - 128, 0x6000U);
                draw_title_sprite_list(
                    image, first_end,
                    0x0120 - x_offset - 128, 0x012F - 128, 0x6000U);
                write_png32(
                    (root / ("matrix-world-effect-action-sweep-phase-" +
                             std::to_string(phase) + ".png"))
                        .string(),
                    image);
            }

            // 0x1BA6E advances a four-phase burst at five-frame intervals.
            // Its first eight bytes select that phase's pattern upload;
            // the following records are positioned relative to FFE04A/C.
            // Use a labeled center anchor here until those live coordinates
            // are recovered from the activation caller.
            for (std::size_t phase = 0U; phase < 4U; ++phase) {
                const auto packet = static_cast<std::size_t>(
                    read_big_endian_long(rom, 0x1D970U + phase * 4U));
                const auto pattern_source = static_cast<std::size_t>(
                    read_big_endian_long(rom, packet + 2U));
                const auto expected_pattern_bytes = static_cast<std::size_t>(
                    read_big_endian_word(rom, packet + 6U));
                if (pattern_source > rom.size() ||
                    expected_pattern_bytes > rom.size() - pattern_source) {
                    throw std::runtime_error(
                        "Matrix title-burst pattern range is outside the ROM");
                }
                const std::vector<std::uint8_t> patterns(
                    rom.begin() + static_cast<std::ptrdiff_t>(pattern_source),
                    rom.begin() + static_cast<std::ptrdiff_t>(
                        pattern_source + expected_pattern_bytes));
                if (patterns.size() != expected_pattern_bytes ||
                    patterns.size() % 32U != 0U) {
                    throw std::runtime_error(
                        "Matrix title-burst raw pattern range has an unexpected size");
                }
                const auto pattern_count = patterns.size() / 32U;
                const auto atlas = shadowrun::graphics::decode_tile_atlas(
                    patterns, 0U, pattern_count, palette, 16U, 0U);
                auto image = make_transparent_figure();
                auto record = packet + 8U;
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                            "Matrix title-burst sprite list is truncated");
                    }
                    const auto x = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(
                        rom, record + 4U);
                    const auto source_attributes = read_big_endian_word(
                        rom, record + 6U);
                    const auto first_tile = static_cast<std::size_t>(
                        source_attributes & 0x07FFU);
                    if ((size & 0xFF00U) != 0U ||
                        first_tile >= pattern_count) {
                        throw std::runtime_error(
                            "Matrix title-burst sprite record is invalid");
                    }
                    draw_figure_sprite(
                        image, atlas, pattern_count, 16U,
                        first_tile, 0U,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(
                            source_attributes + 0x0780U | 0x6000U),
                        160 + x, 112 + y);
                    record += 8U;
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix title-burst sprite list has no terminator");
                }
                const auto output = root /
                    ("matrix-world-effect-title-burst-phase-" +
                     std::to_string(phase) +
                     "-centered-inspection-anchor.png");
                write_png32(output.string(), image);
                std::cout << "Matrix title-burst phase " << phase
                          << " raw pattern=0x" << std::hex << pattern_source
                          << "+0x" << expected_pattern_bytes << std::dec
                          << " center inspection anchor -> "
                          << output.string() << "\n";
            }

            // 0x1CB6A spawns the left/right action effects; 0x1D0DE selects
            // their three-frame direct sprite tables. Preserve these as
            // source-tile previews and as exact spawn-callsite placements.
            // The effects then move every source frame, so the placement
            // previews below are deliberately labeled as spawn positions.
            struct WorldEffectMotionTable {
                std::size_t source_variant;
                std::size_t pointer_table;
            };
            constexpr std::array<WorldEffectMotionTable, 3>
                kWorldEffectMotionTables{{
                    {0U, 0x1D134U},
                    {1U, 0x1D11CU},
                    {2U, 0x1D128U},
                }};
            const auto render_world_effect_motion_list = [&] (
                const std::size_t list,
                const int base_x,
                const int base_y,
                const std::uint16_t attribute_offset,
                const std::filesystem::path& output) {
                auto image = make_transparent_figure();
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 16U; ++sprite) {
                    const auto record = list + sprite * 8U;
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                            "Matrix world-effect motion sprite list is truncated");
                    }
                    const auto x = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(
                        rom, record + 4U);
                    const auto source_attributes = read_big_endian_word(
                        rom, record + 6U);
                    if ((size & 0xFF00U) != 0U ||
                        (source_attributes & 0xF000U) != 0x8000U) {
                        throw std::runtime_error(
                            "Matrix world-effect motion sprite has unexpected attributes");
                    }
                    const auto first_tile = static_cast<std::size_t>(
                        source_attributes & 0x07FFU);
                    if (first_tile < kDescriptorDFirstTile) {
                        throw std::runtime_error(
                            "Matrix world-effect motion sprite is outside the 0xDAAB8 bank");
                    }
                    draw_figure_sprite(
                        image, descriptor_d_atlas, descriptor_d_tile_count,
                        16U, first_tile, kDescriptorDFirstTile,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(
                            source_attributes | attribute_offset),
                        base_x + x, base_y + y);
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix world-effect motion sprite list has no terminator");
                }
                write_png32(output.string(), image);
            };
            for (const auto& table : kWorldEffectMotionTables) {
                for (std::size_t phase = 0U; phase < 3U; ++phase) {
                    const auto list = static_cast<std::size_t>(
                        read_big_endian_long(
                            rom, table.pointer_table + phase * 4U));
                    render_world_effect_motion_list(
                        list, 160, 120, 0x6000U,
                        root / ("matrix-world-effect-motion-variant-" +
                                std::to_string(table.source_variant) +
                                "-phase-" + std::to_string(phase) +
                                "-centered-inspection-anchor.png"));
                }
            }
            constexpr int kActionRunnerRawX = 0x0120;
            constexpr int kActionRunnerRawY = 0x012F;
            constexpr std::array<std::pair<const char*, int>, 2>
                kHandEffectSpawnOffsets{{
                    {"left", -0x20}, {"right", 0x20},
                }};
            const auto spawn_phase_zero = static_cast<std::size_t>(
                read_big_endian_long(rom, 0x1D134U));
            for (const auto& hand : kHandEffectSpawnOffsets) {
                const auto base_x = kActionRunnerRawX + hand.second - 128;
                const auto base_y = kActionRunnerRawY - 0x28 - 128;
                render_world_effect_motion_list(
                    spawn_phase_zero, base_x, base_y, 0x6000U,
                    root / (std::string("matrix-world-effect-motion-spawn-") +
                            hand.first + "-variant-0-phase-0.png"));
            }

            // 0x1BB6A draws these paired 3x3 title-noise sprites while the
            // action timer is in its 4..7 subphase. Include each SAT record's
            // signed local offsets in the preview; the sprites do not form
            // the large circular outlines visible in the capture.
            constexpr std::array<std::pair<const char*, std::size_t>, 2>
                kActionPairedSpriteLists{{
                    {"left", 0x1DAEAU},
                    {"right", 0x1DAE0U},
                }};
            for (const auto& sprite_pair : kActionPairedSpriteLists) {
                const auto raw_x = sprite_pair.first[0] == 'l'
                    ? 0x00E7 : 0x0158;
                auto image = make_transparent_figure();
                auto record = sprite_pair.second;
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 8U; ++sprite) {
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                        "Matrix paired title-noise list is truncated");
                    }
                    const auto x_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x_offset ==
                        std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(
                        rom, record + 4U);
                    const auto source_attributes = read_big_endian_word(
                        rom, record + 6U);
                    const auto first_tile = static_cast<std::size_t>(
                        source_attributes & 0x07FFU);
                    if ((size & 0xFF00U) != 0U ||
                        (source_attributes & 0xF000U) != 0x8000U ||
                        first_tile < kDescriptorDFirstTile) {
                        throw std::runtime_error(
                            "Matrix paired title-noise sprite has unexpected source data");
                    }
                    draw_figure_sprite(
                        image, descriptor_d_atlas, descriptor_d_tile_count,
                        16U, first_tile, kDescriptorDFirstTile,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(
                            source_attributes | 0x2000U),
                        raw_x - 128 + x_offset,
                        0x00FC - 128 + y_offset);
                    record += 8U;
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix paired title-noise list has no terminator");
                }
                const auto output = root /
                    (std::string("matrix-world-effect-title-noise-paired-") +
                     sprite_pair.first + ".png");
                write_png32(output.string(), image);
                std::cout << "Matrix paired title-noise list 0x" << std::hex
                          << sprite_pair.second << " source origin x=0x"
                          << raw_x << " y=0xfc"
                          << std::dec << " -> screen position x="
                          << raw_x - 128 + static_cast<std::int16_t>(
                                 read_big_endian_word(
                                     rom, sprite_pair.second))
                          << ", y=" << 0xFC - 128 + static_cast<std::int16_t>(
                                 read_big_endian_word(
                                     rom, sprite_pair.second + 2U))
                          << " -> " << output.string() << "\n";
            }
            constexpr std::array<std::size_t, 4>
                kTitleNoiseInlineLists{{
                    0x1DAB8U, 0x1DAC2U, 0x1DACCU, 0x1DAD6U,
                }};
            for (std::size_t variant = 0U;
                 variant < kTitleNoiseInlineLists.size(); ++variant) {
                auto image = make_transparent_figure();
                const auto list = kTitleNoiseInlineLists[variant];
                const auto size = read_big_endian_word(rom, list + 4U);
                const auto source_attributes = read_big_endian_word(
                    rom, list + 6U);
                const auto first_tile = static_cast<std::size_t>(
                    source_attributes & 0x07FFU);
                if ((size & 0xFF00U) != 0U ||
                    (source_attributes & 0xF000U) != 0x8000U ||
                    first_tile < kDescriptorDFirstTile) {
                    throw std::runtime_error(
                        "Matrix title-noise inline sprite has unexpected source data");
                }
                for (std::size_t sprite = 0U; sprite < 7U; ++sprite) {
                    const auto raw_x =
                        0x00D8 + static_cast<int>(sprite + 1U) * 0x10;
                    draw_figure_sprite(
                        image, descriptor_d_atlas, descriptor_d_tile_count,
                        16U, first_tile, kDescriptorDFirstTile,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(
                            source_attributes | 0x2000U),
                        raw_x - 128, 0x00EC - 128);
                }
                write_png32(
                    (root / ("matrix-world-effect-title-noise-row-variant-" +
                             std::to_string(variant) + ".png"))
                        .string(),
                    image);
            }

            // The action-selection pulse attaches behavior A (0x1B5B8),
            // while the ordinary world-effect loop uses behavior B
            // (0x1B612). Their lists share tiles from the fixed 0xDA038 bank
            // and may also select the 0xDAAB8 upload at tile 0x16F. Emit each
            // discrete primary/secondary frame pair so that pose identity is
            // not inferred from a merge of mutually exclusive phases.
            const auto draw_behavior_list = [&] (
                shadowrun::graphics::RgbaImage& figure,
                const std::size_t list) {
                auto record = list;
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                            "Matrix behavior sprite list is truncated");
                    }
                    const auto x = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(
                        rom, record + 4U);
                    const auto attributes = read_big_endian_word(
                        rom, record + 6U);
                    if ((size & 0xFF00U) != 0U ||
                        (attributes & 0xF000U) != 0x8000U) {
                        throw std::runtime_error(
                            "Matrix behavior sprite has unexpected attributes");
                    }
                    const auto first_tile = static_cast<std::size_t>(
                        attributes & 0x07FFU);
                    const bool uses_descriptor_d_bank =
                        first_tile >= kDescriptorDFirstTile;
                    draw_figure_sprite(
                        figure,
                        uses_descriptor_d_bank
                            ? descriptor_d_atlas
                            : fixed_figure_atlas,
                        uses_descriptor_d_bank
                            ? descriptor_d_tile_count
                            : kFixedFigureTileCount,
                        16U, first_tile,
                        uses_descriptor_d_bank
                            ? kDescriptorDFirstTile
                            : kDescriptorEFirstTile,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(attributes | 0x6000U),
                        kDescriptorEBaseX + x,
                        kDescriptorEBaseY + y);
                    record += 8U;
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix behavior sprite list has no terminator");
                }
            };
            const auto write_behavior_pose_pairs = [&] (
                const char* behavior_name,
                const std::size_t primary_table,
                const std::size_t secondary_table) {
                const auto primary_max = static_cast<std::size_t>(
                    read_big_endian_word(rom, primary_table));
                const auto secondary_max = static_cast<std::size_t>(
                    read_big_endian_word(rom, secondary_table));
                for (std::size_t primary_phase = 0U;
                     primary_phase <= primary_max; ++primary_phase) {
                    const auto primary_list = static_cast<std::size_t>(
                        read_big_endian_long(
                            rom, primary_table + 2U + primary_phase * 4U));
                    for (std::size_t secondary_phase = 0U;
                         secondary_phase <= secondary_max;
                         ++secondary_phase) {
                        const auto secondary_list = static_cast<std::size_t>(
                            read_big_endian_long(
                                rom, secondary_table + 2U +
                                    secondary_phase * 4U));
                        auto figure = make_transparent_figure();
                        draw_behavior_list(figure, primary_list);
                        draw_behavior_list(figure, secondary_list);
                        const auto path = root /
                            (std::string("matrix-world-effect-behavior-") +
                             behavior_name + "-primary-" +
                             std::to_string(primary_phase) + "-secondary-" +
                             std::to_string(secondary_phase) + ".png");
                        write_png32(path.string(), figure);
                    }
                }
            };
            write_behavior_pose_pairs("a", 0x1C33AU, 0x1C330U);
            write_behavior_pose_pairs("b", 0x1C29EU, 0x1C294U);
            shadowrun::graphics::RgbaImage descriptor_d_figure{};
            descriptor_d_figure.width =
                static_cast<std::uint16_t>(kFrameWidth);
            descriptor_d_figure.height =
                static_cast<std::uint16_t>(kFrameHeight);
            descriptor_d_figure.pixels.assign(
                kFrameWidth * kFrameHeight,
                shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
            constexpr std::array<std::size_t, 2> kDescriptorDListTables{{
                0x1C31EU, 0x1C318U,
            }};
            for (const auto table : kDescriptorDListTables) {
                const auto maximum_phase = static_cast<std::size_t>(
                    read_big_endian_word(rom, table));
                for (std::size_t phase = 0U; phase <= maximum_phase; ++phase) {
                    const auto list = static_cast<std::size_t>(
                        read_big_endian_long(rom, table + 2U + phase * 4U));
                    auto record = list;
                    bool terminated = false;
                    for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                        if (record > rom.size() ||
                            rom.size() - record < 8U) {
                            throw std::runtime_error(
                                "Matrix descriptor-D sprite list is truncated");
                        }
                        const auto x_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record));
                        if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                            terminated = true;
                            break;
                        }
                        const auto y_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record + 2U));
                        const auto size = read_big_endian_word(
                            rom, record + 4U);
                        const auto tile_and_flip = read_big_endian_word(
                            rom, record + 6U);
                        if ((size & 0xFF00U) != 0U ||
                            (tile_and_flip & 0xF000U) != 0x8000U) {
                            throw std::runtime_error(
                                "Matrix descriptor-D sprite has unexpected attributes");
                        }
                        const auto first_tile = static_cast<std::size_t>(
                            tile_and_flip & 0x07FFU);
                        draw_figure_sprite(
                            descriptor_d_figure, descriptor_d_atlas,
                            descriptor_d_tile_count, 16U, first_tile,
                            kDescriptorDFirstTile,
                            static_cast<std::uint8_t>(size),
                            static_cast<std::uint16_t>(tile_and_flip | 0x6000U),
                            kDescriptorEBaseX + x_offset,
                            kDescriptorEBaseY + y_offset);
                        std::cout << "descriptor D list 0x" << std::hex << list
                                  << " phase 0x" << phase << " sprite 0x"
                                  << sprite << " tile 0x" << first_tile
                                  << std::dec << "\n";
                        record += 8U;
                    }
                    if (!terminated) {
                        throw std::runtime_error(
                            "Matrix descriptor-D sprite list has no terminator");
                    }
                }
            }
            const auto descriptor_d_path =
                root / "matrix-world-effect-descriptor-d.png";
            write_png32(descriptor_d_path.string(), descriptor_d_figure);

            // 0x1B5FC installs a separate, single-phase behavior record. Its
            // tables select lists at 0x1D6CA and 0x1D47C from the 0xDAAB8
            // upload; retain its source identity without misnaming it as the
            // ordinary behavior B attached at 0x1B612.
            shadowrun::graphics::RgbaImage transition_1b5fc_figure{};
            transition_1b5fc_figure.width =
                static_cast<std::uint16_t>(kFrameWidth);
            transition_1b5fc_figure.height =
                static_cast<std::uint16_t>(kFrameHeight);
            transition_1b5fc_figure.pixels.assign(
                kFrameWidth * kFrameHeight,
                shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
            constexpr std::array<std::size_t, 2> kTransition1B5FCListTables{{
                0x1C2A8U, 0x1C2AEU,
            }};
            for (const auto table : kTransition1B5FCListTables) {
                const auto maximum_phase = static_cast<std::size_t>(
                    read_big_endian_word(rom, table));
                for (std::size_t phase = 0U; phase <= maximum_phase; ++phase) {
                    const auto list = static_cast<std::size_t>(
                        read_big_endian_long(rom, table + 2U + phase * 4U));
                    auto record = list;
                    bool terminated = false;
                    for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                        if (record > rom.size() || rom.size() - record < 8U) {
                            throw std::runtime_error(
                            "Matrix 0x1B5FC sprite list is truncated");
                        }
                        const auto x_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record));
                        if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                            terminated = true;
                            break;
                        }
                        const auto y_offset = static_cast<std::int16_t>(
                            read_big_endian_word(rom, record + 2U));
                        const auto size = read_big_endian_word(
                            rom, record + 4U);
                        const auto attributes = read_big_endian_word(
                            rom, record + 6U);
                        if ((size & 0xFF00U) != 0U ||
                            (attributes & 0xF000U) != 0x8000U) {
                            throw std::runtime_error(
                                "Matrix descriptor-B sprite has unexpected attributes");
                        }
                        const auto first_tile = static_cast<std::size_t>(
                            attributes & 0x07FFU);
                        draw_figure_sprite(
                            transition_1b5fc_figure, descriptor_d_atlas,
                            descriptor_d_tile_count, 16U, first_tile,
                            kDescriptorDFirstTile,
                            static_cast<std::uint8_t>(size),
                            static_cast<std::uint16_t>(attributes | 0x6000U),
                            kDescriptorEBaseX + x_offset,
                            kDescriptorEBaseY + y_offset);
                        record += 8U;
                    }
                    if (!terminated) {
                        throw std::runtime_error(
                            "Matrix 0x1B5FC sprite list has no terminator");
                    }
                    std::cout << "0x1B5FC transition list 0x" << std::hex << list
                              << " phase 0x" << phase << std::dec << "\n";
                }
            }
            const auto transition_1b5fc_path =
                root / "matrix-world-effect-transition-1b5fc.png";
            write_png32(transition_1b5fc_path.string(),
                        transition_1b5fc_figure);

            // 0x26972 contributes a second SAT list on every source scene
            // update. Its branch keys off the active behavior's primary list
            // table: 0x1C31E selects 0x14CD0E, 0x1C32A selects 0x14CD30, and
            // all other tables (including descriptor E's 0x1C34A) select
            // 0x14CCEC. The lists reference the 0x14C222 stream uploaded at
            // VRAM $8C40 (tile $462).
            const auto auxiliary_sprite_patterns =
                shadowrun::graphics::decode_genesis_resource_stream(
                    rom, 0x14C222U);
            if (auxiliary_sprite_patterns.size() % 32U != 0U) {
                throw std::runtime_error(
                    "Matrix auxiliary sprite stream is not tile aligned");
            }
            const auto auxiliary_sprite_tile_count =
                auxiliary_sprite_patterns.size() / 32U;
            const auto auxiliary_sprite_atlas =
                shadowrun::graphics::decode_tile_atlas(
                    auxiliary_sprite_patterns, 0U,
                    auxiliary_sprite_tile_count, palette, 16U, 0U);
            constexpr std::size_t kAuxiliarySpriteFirstTile = 0x0462U;
            constexpr std::array<std::pair<const char*, std::size_t>, 3>
                kAuxiliarySpriteLists{{
                    {"table-1c31e", 0x14CD0EU},
                    {"table-1c32a", 0x14CD30U},
                    {"default", 0x14CCECU},
                }};
            for (const auto& [name, list] : kAuxiliarySpriteLists) {
                shadowrun::graphics::RgbaImage auxiliary_figure{};
                auxiliary_figure.width =
                    static_cast<std::uint16_t>(kFrameWidth);
                auxiliary_figure.height =
                    static_cast<std::uint16_t>(kFrameHeight);
                auxiliary_figure.pixels.assign(
                    kFrameWidth * kFrameHeight,
                    shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
                auto record = list;
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                            "Matrix auxiliary sprite list is truncated");
                    }
                    const auto x_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size = read_big_endian_word(
                        rom, record + 4U);
                    const auto attributes = read_big_endian_word(
                        rom, record + 6U);
                    if ((size & 0xFF00U) != 0U ||
                        (attributes & 0xE000U) != 0x8000U) {
                        throw std::runtime_error(
                            "Matrix auxiliary sprite has unexpected source attributes");
                    }
                    const auto first_tile = static_cast<std::size_t>(
                        attributes & 0x07FFU);
                    draw_figure_sprite(
                        auxiliary_figure, auxiliary_sprite_atlas,
                        auxiliary_sprite_tile_count, 16U, first_tile,
                        kAuxiliarySpriteFirstTile,
                        static_cast<std::uint8_t>(size),
                        static_cast<std::uint16_t>(attributes | 0x6000U),
                        kDescriptorEBaseX + x_offset,
                        kDescriptorEBaseY + y_offset);
                    std::cout << "auxiliary sprite list 0x" << std::hex
                              << list << " sprite 0x" << sprite
                              << " tile 0x" << first_tile << std::dec << "\n";
                    record += 8U;
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix auxiliary sprite list has no terminator");
                }
                const auto auxiliary_path = root /
                    (std::string("matrix-world-effect-") + name +
                     "-sprites.png");
                write_png32(auxiliary_path.string(), auxiliary_figure);
                if (list == 0x14CCECU) {
                    auto combined = auxiliary_figure;
                    for (std::size_t pixel = 0U;
                         pixel < combined.pixels.size(); ++pixel) {
                        if (descriptor_e_figure.pixels[pixel].alpha != 0U) {
                            combined.pixels[pixel] =
                                descriptor_e_figure.pixels[pixel];
                        }
                    }
                    write_png32(
                        (root / "matrix-world-effect-descriptor-e-combined.png")
                            .string(),
                        combined);
                    for (std::size_t pixel = 0U;
                         pixel < combined.pixels.size(); ++pixel) {
                        if (transition_1b5fc_figure.pixels[pixel].alpha != 0U) {
                            combined.pixels[pixel] =
                                transition_1b5fc_figure.pixels[pixel];
                        }
                    }
                    write_png32(
                        (root / "matrix-world-effect-transition-1b5fc-combined.png")
                            .string(),
                        combined);
                }
            }
            // 0x1A908 chooses a mode record through 0x1A9CE. Its phase
            // descriptor carries the direct pattern transfer at +2/+6 and
            // the SAT list at +8. 0x1AA40 uses D1=0x120 (X), D2=0xD0 (Y),
            // D4=0x694, and D6=0x2000. The three separately timed overlays
            // use the same descriptor layout with tile base 0x7A8.
            constexpr std::size_t kWorldEffectModeTable = 0x1A9CEU;
            constexpr std::size_t kWorldEffectModeCount = 9U;
            // 0x1AA40 emits attributes with D6=0x2000 (palette line 1).
            // 0x176EA loads the four-line block from E2E3A, so this line
            // begins at E2E5A; the generic figure palette above is line 3.
            const auto world_effect_mode_palette =
                shadowrun::graphics::read_genesis_palette(rom, 0x0E2E5AU);
            const auto render_world_effect_descriptor = [&] (
                const std::size_t descriptor,
                const std::size_t first_tile,
                const int base_x,
                const int base_y,
                const std::filesystem::path& output) {
                const auto pattern_source = static_cast<std::size_t>(
                    read_big_endian_long(rom, descriptor + 2U));
                const auto pattern_bytes = static_cast<std::size_t>(
                    read_big_endian_word(rom, descriptor + 6U));
                if (pattern_bytes == 0U || pattern_bytes % 32U != 0U ||
                    pattern_source > rom.size() ||
                    pattern_bytes > rom.size() - pattern_source) {
                    throw std::runtime_error(
                        "Matrix world-effect descriptor has invalid pattern data");
                }
                const auto tile_count = pattern_bytes / 32U;
                const std::vector<std::uint8_t> pattern_data(
                    rom.begin() + static_cast<std::ptrdiff_t>(pattern_source),
                    rom.begin() + static_cast<std::ptrdiff_t>(
                        pattern_source + pattern_bytes));
                const auto atlas = shadowrun::graphics::decode_tile_atlas(
                    pattern_data, 0U, tile_count, world_effect_mode_palette,
                    16U, 0U);

                shadowrun::graphics::RgbaImage image{};
                image.width = static_cast<std::uint16_t>(kFrameWidth);
                image.height = static_cast<std::uint16_t>(kFrameHeight);
                image.pixels.assign(
                    kFrameWidth * kFrameHeight,
                    shadowrun::native::Rgba8{0U, 0U, 0U, 0U});
                bool terminated = false;
                for (std::size_t sprite = 0U; sprite < 32U; ++sprite) {
                    const auto record = descriptor + 8U + sprite * 8U;
                    if (record > rom.size() || rom.size() - record < 8U) {
                        throw std::runtime_error(
                            "Matrix world-effect SAT list is truncated");
                    }
                    const auto x_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record));
                    if (x_offset == std::numeric_limits<std::int16_t>::min()) {
                        terminated = true;
                        break;
                    }
                    const auto y_offset = static_cast<std::int16_t>(
                        read_big_endian_word(rom, record + 2U));
                    const auto size_word = read_big_endian_word(
                        rom, record + 4U);
                    if ((size_word & 0xFF00U) != 0U) {
                        throw std::runtime_error(
                            "Matrix world-effect sprite size exceeds one byte");
                    }
                    const auto tile_offset = static_cast<std::size_t>(
                        read_big_endian_word(rom, record + 6U));
                    if (tile_offset > tile_count) {
                        throw std::runtime_error(
                            "Matrix world-effect sprite starts beyond its art");
                    }
                    const auto attributes = static_cast<std::uint16_t>(
                        0x2000U | (first_tile + tile_offset));
                    draw_figure_sprite(
                        image, atlas, tile_count, 16U,
                        first_tile + tile_offset, first_tile,
                        static_cast<std::uint8_t>(size_word), attributes,
                        base_x + x_offset, base_y + y_offset);
                }
                if (!terminated) {
                    throw std::runtime_error(
                        "Matrix world-effect SAT list has no terminator");
                }
                write_png32(output.string(), image);
                std::cout << "Matrix world-effect sprite descriptor 0x"
                          << std::hex << descriptor << " art=0x"
                          << pattern_source << "+0x" << pattern_bytes
                          << std::dec << " -> " << output.string() << "\n";
            };

            for (std::size_t mode = 0U; mode < kWorldEffectModeCount; ++mode) {
                const auto mode_record = static_cast<std::size_t>(
                    read_big_endian_long(
                        rom, kWorldEffectModeTable + mode * 4U));
                const auto phase_count = static_cast<std::size_t>(
                    rom[mode_record + 1U]);
                const auto phase_table = static_cast<std::size_t>(
                    read_big_endian_long(rom, mode_record + 2U));
                if (phase_count == 0U || phase_count > 32U) {
                    throw std::runtime_error(
                        "Matrix world-effect mode has an invalid phase count");
                }
                for (std::size_t phase = 0U; phase < phase_count; ++phase) {
                    const auto descriptor = static_cast<std::size_t>(
                        read_big_endian_long(rom, phase_table + phase * 4U));
                    const auto output = root /
                        ("matrix-world-effect-mode-" +
                         std::to_string(mode) + "-phase-" +
                         std::to_string(phase) + ".png");
                    render_world_effect_descriptor(
                        descriptor, 0x0694U, 0x0120 - 128, 0x00D0 - 128,
                        output);
                }
            }

            struct OverlayTable {
                const char* name;
                std::size_t table;
                std::size_t phases;
                std::size_t first_tile;
                int initial_x;
                int initial_y;
                bool dynamic_x;
            };
            constexpr std::array<OverlayTable, 3> kOverlayTables{{
                {"primary", 0xF1BDEU, 26U, 0x07A8U,
                 (0x4800 >> 6) - 128, 0x00B9 - 128, false},
                {"secondary", 0xF1BCEU, 4U, 0x07A8U,
                 (0x47C0 >> 6) - 128, 0x00B9 - 128, false},
                {"background", 0xF1BBEU, 4U, 0x07A8U,
                 160, 0x00D0 - 128, true},
            }};
            for (const auto& overlay : kOverlayTables) {
                for (std::size_t phase = 0U; phase < overlay.phases; ++phase) {
                    const auto descriptor = static_cast<std::size_t>(
                        read_big_endian_long(
                            rom, overlay.table + phase * 4U));
                    const auto output = root /
                        (std::string("matrix-world-effect-") + overlay.name +
                         "-phase-" + std::to_string(phase) +
                         (overlay.dynamic_x
                              ? "-centered-inspection-anchor.png"
                              : "-initial.png"));
                    render_world_effect_descriptor(
                        descriptor, overlay.first_tile, overlay.initial_x,
                        overlay.initial_y, output);
                    if (overlay.dynamic_x && phase == 0U) {
                        std::cout << "  dynamic X from FFE0C6>>4; centered "
                                     "inspection anchor is screen x=160\n";
                    }
                }
            }
            std::cout << "descriptor D pattern bytes: "
                      << descriptor_d_patterns.size() << "\n"
                      << "auxiliary sprite pattern bytes: "
                      << auxiliary_sprite_patterns.size() << "\n"
                      << "descriptor E figure: "
                      << descriptor_e_path.string() << "\n"
                      << "descriptor D figure: "
                      << descriptor_d_path.string() << "\n"
                      << "0x1B5FC transition figure: "
                      << transition_1b5fc_path.string() << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc != 6 && argc != 7) {
        std::cerr << "usage: shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "<tile-offset> <tile-count> <palette-offset> [--resource]\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <ignored> "
                     "--dialogue-record <rom-offset>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> --title\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-field\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--intro-stage <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--intro-heli <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-aircraft-plane-a\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-wilderness-pan <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-ambush <index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--cab-map\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-menu\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-service-background\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-dialogue-panel\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-dialogue-portrait <profile-index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-mode-profile-sprite <rom-offset>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-dialogue-panel-alternate\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out-dir> "
                      "--field-dialogue-buttons\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--magic-panel\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-menu-inventory\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-field-menu-inventory\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <ignored> "
                     "--field-event-cells\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <ignored> "
                     "--field-event-cells <mode-id>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--mode2-map\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--mode-map <mode-id>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--entity-frame\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-states\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-walking\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-movement\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-state <index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <asset-dir> "
                     "--title-boot-assets\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <asset-dir> "
                     "--all-assets\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <asset-dir> "
                     "--matrix-cyberdeck-selectors\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out-dir> "
                     "--matrix-field-atlases\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out-dir> "
                     "--matrix-world-models\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out-dir> "
                     "--scene-5bd2c-sprite-lists\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out-dir> "
                     "--matrix-title-figure-atlases\n";
        std::cerr << "numeric offsets accept decimal or 0x-prefixed hex\n";
        return 2;
    }
    try {
        const auto rom = read_file(argv[1]);
        const auto tile_offset = static_cast<std::size_t>(parse_number(argv[3]));
        const auto tile_count = static_cast<std::size_t>(parse_number(argv[4]));
        const auto palette_offset = static_cast<std::size_t>(parse_number(argv[5]));
        const auto resource_stream = argc == 7 && std::string(argv[6]) == "--resource";
        if (argc == 7 && !resource_stream) {
            throw std::invalid_argument("unknown extractor mode");
        }
        const auto palette = shadowrun::graphics::read_genesis_palette(
            rom, palette_offset);
        const auto resource = resource_stream
            ? shadowrun::graphics::decode_genesis_resource_stream(rom, tile_offset)
            : std::vector<std::uint8_t>{};
        const auto& tile_source = resource_stream ? resource : rom;
        const auto image = shadowrun::graphics::decode_tile_atlas(
            tile_source, resource_stream ? 0U : tile_offset, tile_count, palette);
        shadowrun::graphics::write_bmp32(argv[2], image);
        std::cout << "tiles: " << tile_count << "\n"
                  << "image: " << image.width << "x" << image.height << "\n"
                  << (resource_stream ? "decoded bytes: " +
                                             std::to_string(resource.size()) +
                                             "\n" : "")
                  << "output: " << argv[2] << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
