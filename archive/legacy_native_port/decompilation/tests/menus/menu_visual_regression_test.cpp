#include "shadowrun/rendering/graphics.hpp"
#include "shadowrun/field/native_port.hpp"

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using shadowrun::native::Button;
using shadowrun::native::InputState;
using shadowrun::native::NativeFieldEntity;
using shadowrun::native::NativeImage;
using shadowrun::native::NativePortAssets;
using shadowrun::native::NativePortShell;
using shadowrun::native::NativeScene;
using shadowrun::native::Rgba8;
using shadowrun::native::SoftwareScreen;
using shadowrun::native::button_mask;
using shadowrun::native::kScreenHeight;
using shadowrun::native::kScreenWidth;

std::vector<std::uint8_t> read_rom() {
    std::ifstream input(SHADOWRUN_TEST_ROM, std::ios::binary);
    if (!input) {
        throw std::runtime_error("unable to open the source Genesis ROM");
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void press(NativePortShell& shell, Button button) {
    const auto mask = button_mask(button);
    shell.update(InputState{mask, mask});
    shell.update(InputState{});
}

void enter_field(NativePortShell& shell, std::size_t archetype) {
    shell.finish_boot();
    press(shell, Button::Start);
    for (int frame = 0; frame < 48 && shell.scene() == NativeScene::Title;
         ++frame) {
        shell.update(InputState{});
    }
    if (shell.scene() != NativeScene::StartOptions) {
        throw std::runtime_error("title/start-options transition did not match source");
    }
    press(shell, Button::A);
    if (shell.scene() != NativeScene::ArchetypeMenu) {
        throw std::runtime_error("New Game did not open the archetype screen");
    }
    for (std::size_t index = 0; index < archetype; ++index) {
        press(shell, Button::Down);
    }
    press(shell, Button::A);
    if (shell.scene() != NativeScene::Intro) {
        throw std::runtime_error("archetype selection did not enter the source intro");
    }
    press(shell, Button::Start);
    if (shell.scene() != NativeScene::Field) {
        throw std::runtime_error("intro did not enter the field scene");
    }
    press(shell, Button::Start);
    if (shell.scene() != NativeScene::FieldMenu) {
        throw std::runtime_error("field command menu did not open");
    }
}

std::uint32_t pack(Rgba8 color) {
    return (static_cast<std::uint32_t>(color.red) << 16U) |
           (static_cast<std::uint32_t>(color.green) << 8U) |
           color.blue;
}

void draw_inventory_selector_outline(SoftwareScreen& screen,
                                    const std::size_t selection,
                                    const std::size_t row) {
    if (selection >= 2U || row >= 4U) {
        return;
    }
    // 0x11592 positions the 32x24 selection footprint; section-06 captures
    // show a single one-pixel warm-gold edge hugging the 24x16 item image.
    const auto x = 14 + static_cast<std::int32_t>(selection * 40U);
    const auto y = 118 + static_cast<std::int32_t>(row * 24U);
    constexpr std::int32_t kOutlineWidth = 28;
    constexpr std::int32_t kOutlineHeight = 20;
    constexpr Rgba8 kSelectorGold{0xC4, 0xBE, 0x27, 0xFF};
    screen.fill_rect(x, y, kOutlineWidth, 1, kSelectorGold);
    screen.fill_rect(x, y + kOutlineHeight - 1, kOutlineWidth, 1,
                     kSelectorGold);
    screen.fill_rect(x, y, 1, kOutlineHeight, kSelectorGold);
    screen.fill_rect(x + kOutlineWidth - 1, y, 1, kOutlineHeight,
                     kSelectorGold);
}

void require_inventory_selector_outline(const SoftwareScreen& screen,
                                        const std::size_t selection,
                                        const std::size_t row) {
    if (selection >= 2U || row >= 4U) {
        throw std::runtime_error("inventory selector is outside its 2x4 grid");
    }
    const auto x = 14 + static_cast<std::int32_t>(selection * 40U);
    const auto y = 118 + static_cast<std::int32_t>(row * 24U);
    constexpr std::int32_t kOutlineWidth = 28;
    constexpr std::int32_t kOutlineHeight = 20;
    const auto kGold = pack(Rgba8{0xC4, 0xBE, 0x27, 0xFF});
    for (std::int32_t offset = 0; offset < kOutlineWidth; ++offset) {
        if (screen.pixel(x + offset, y) != kGold ||
            screen.pixel(x + offset, y + kOutlineHeight - 1) != kGold) {
            throw std::runtime_error(
                "inventory selector top/bottom gold edge is missing");
        }
    }
    for (std::int32_t offset = 0; offset < kOutlineHeight; ++offset) {
        if (screen.pixel(x, y + offset) != kGold ||
            screen.pixel(x + kOutlineWidth - 1, y + offset) != kGold) {
            throw std::runtime_error(
                "inventory selector left/right gold edge is missing");
        }
    }
}

void write_source_text(SoftwareScreen& screen, const NativeImage& font,
                       std::int32_t destination_x,
                       std::int32_t destination_y,
                       std::string_view text) {
    auto cursor = destination_x;
    for (const auto character : text) {
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph_index = static_cast<std::size_t>(code - 0x20U);
        const auto glyph_x = (glyph_index % 16U) * 8U;
        const auto glyph_y = (glyph_index / 16U) * 8U;
        std::vector<Rgba8> glyph(64U);
        for (std::size_t y = 0; y < 8U; ++y) {
            for (std::size_t x = 0; x < 8U; ++x) {
                glyph[y * 8U + x] =
                    font.pixels[(glyph_y + y) * font.width + glyph_x + x];
            }
        }
        screen.blit_rgba(cursor, destination_y, 8, 8, glyph);
        cursor += 8;
    }
}

std::string_view source_item_detail_label_for_test(std::uint8_t item_code) {
    if (item_code < 0x0DU) {
        return "ammo";
    }
    if (item_code < 0x14U) {
        return "uses";
    }
    if (item_code == 0x14U ||
        (item_code >= 0x18U && item_code < 0x29U)) {
        return "rtng";
    }
    return {};
}

std::string center_source_fixed_field_for_test(std::string_view value,
                                               std::size_t width = 12U) {
    if (value.size() >= width) {
        return std::string(value);
    }
    const auto left_padding = (width - value.size()) / 2U;
    return std::string(left_padding, ' ') + std::string(value);
}

std::uint8_t source_phase_for_test(const NativeFieldEntity& record,
                                   std::uint8_t item_code) {
    for (std::size_t slot = 0; slot < record.inventory_item_codes.size();
         ++slot) {
        if (record.inventory_item_codes[slot] == item_code) {
            return record.inventory_phase_bytes[slot];
        }
    }
    return 0;
}

bool source_alt_icon_palette(const NativeFieldEntity& entity);

SoftwareScreen source_field_menu_reference(const NativePortAssets& assets,
                                           const NativeFieldEntity& record,
                                           std::size_t portrait_index,
                                           std::size_t selection,
                                           std::size_t row) {
    SoftwareScreen expected;
    expected.clear({0, 0, 0, 255});
    expected.blit_rgba(0, 0, assets.field_menu_background.width,
                       assets.field_menu_background.height,
                       assets.field_menu_background.pixels);
    expected.blit_rgba(0, 0, assets.field_menu_inventory_boxes.width,
                       assets.field_menu_inventory_boxes.height,
                       assets.field_menu_inventory_boxes.pixels);

    const auto write_text = [&](std::int32_t x, std::int32_t y,
                                std::string_view text) {
        write_source_text(expected, assets.field_menu_font, x, y, text);
    };
    const auto archetype = [&]() {
        switch (record.archetype_index) {
        case 1U:
            return assets.intro.archetypes[2] + " " + assets.intro.archetypes[3];
        case 2U:
            return assets.intro.archetypes[0];
        case 3U:
            return assets.intro.archetypes[1];
        default:
            return std::string("Unknown");
        }
    }();
    const auto draw_item_detail = [&](std::uint8_t item_code,
                                      bool include_art) {
        if (item_code == 0U || item_code > assets.inventory_items.size()) {
            return;
        }
        const auto item_index = static_cast<std::size_t>(item_code - 1U);
        const auto& item = assets.inventory_items[item_index];
        if (include_art) {
            expected.blit_rgba(144, 24,
                               assets.field_menu_item_art[item_index].width,
                               assets.field_menu_item_art[item_index].height,
                               assets.field_menu_item_art[item_index].pixels);
        }
        write_text(128, 64, center_source_fixed_field_for_test(item.name));
        write_text(128, 72,
                   center_source_fixed_field_for_test(item.category));
        const auto is_weapon_detail = item_code < 0x10U;
        if (is_weapon_detail &&
            item.damage_table_index < assets.field_menu_damage_bars.size()) {
            write_text(112, 80, "Damage");
            const auto& bar = assets.field_menu_damage_bars[
                item.damage_table_index];
            expected.blit_rgba(168, 80, bar.width, bar.height, bar.pixels);
        }
        if (is_weapon_detail && item.power_table_index != 0U) {
            write_text(112, 88, "Power");
            const auto label_index = static_cast<std::size_t>(
                item.power_table_index - 1U);
            if (label_index < assets.field_menu_power_labels.size()) {
                write_text(168, 88,
                           assets.field_menu_power_labels[label_index]);
            }
        }
        const auto detail_label = include_art
            ? source_item_detail_label_for_test(item_code)
            : std::string_view{};
        if (!detail_label.empty()) {
            write_text(112, 48, detail_label);
            const auto phase = source_phase_for_test(record, item_code);
            if (phase < assets.field_menu_detail_values.size()) {
                const auto& value = assets.field_menu_detail_values[phase];
                expected.blit_rgba(120, 56, value.width, value.height,
                                   value.pixels);
            }
        }
    };

    // The source field-menu compositor writes the active portrait, runner
    // name, and status strip before branching to the overview or item page.
    // They remain visible while an inventory item is selected; only the
    // overview detail block changes with the middle-button selection.
    expected.blit_rgba(
        16, 16, assets.field_menu_runner_portraits[portrait_index].width,
        assets.field_menu_runner_portraits[portrait_index].height,
        assets.field_menu_runner_portraits[portrait_index].pixels);
    write_text(128, 8, assets.field_menu_runner_names[0]);
    const auto status_index = std::min<std::size_t>(
        record.menu_status_index, assets.field_menu_status_bars.size() - 1U);
    const auto& status = assets.field_menu_status_bars[status_index];
    expected.blit_rgba(176, 168, status.width, status.height, status.pixels);
    write_text(136, 176, assets.field_menu_status_names[status_index]);

    if (selection >= 2U) {
        write_text(128, 24, "Human " + archetype);
        write_text(128, 40, "Physical");
        write_text(128, 48, "Mental");
        write_text(208, 40,
                   std::to_string(std::min<unsigned>(
                       100U, static_cast<unsigned>(record.physical_value) * 10U)) +
                       "%");
        write_text(208, 48,
                   std::to_string(std::min<unsigned>(
                       100U, static_cast<unsigned>(record.mental_value) * 10U)) +
                       "%");
        draw_item_detail(record.active_item_code, false);
        write_text(112, 104, "Atk");
        write_text(112, 112, "Def");
        if (record.attack_bar_index < assets.field_menu_attack_bars.size()) {
            const auto& bar = assets.field_menu_attack_bars[record.attack_bar_index];
            expected.blit_rgba(144, 104, bar.width, bar.height, bar.pixels);
        }
        if (record.defense_bar_index < assets.field_menu_defense_bars.size()) {
            const auto& bar = assets.field_menu_defense_bars[record.defense_bar_index];
            expected.blit_rgba(144, 112, bar.width, bar.height, bar.pixels);
        }
        if (record.equipped_armor_index != 0U &&
            record.equipped_armor_index <= assets.field_menu_armor_labels.size()) {
            const auto& armor = assets.field_menu_armor_labels[
                record.equipped_armor_index - 1U];
            for (std::size_t line = 0; line < armor.size(); ++line) {
                write_text(128, 128 + static_cast<std::int32_t>(line * 8U),
                           armor[line]);
            }
        } else {
            write_text(152, 136, "No Armor");
        }
        write_text(136, 144, "Wpns");
        write_text(136, 152, "Guns");
        if (record.equipped_armor_index != 0U &&
            record.equipped_armor_index <=
                assets.field_menu_armor_summary_modifiers.size()) {
            const auto& modifiers = assets.field_menu_armor_summary_modifiers[
                record.equipped_armor_index - 1U];
            if (modifiers.weapons < assets.field_menu_armor_summary_bars.size()) {
                const auto& bar = assets.field_menu_armor_summary_bars[
                    modifiers.weapons];
                expected.blit_rgba(176, 144, bar.width, bar.height, bar.pixels);
            }
            if (modifiers.guns < assets.field_menu_armor_summary_bars.size()) {
                const auto& bar = assets.field_menu_armor_summary_bars[
                    modifiers.guns];
                expected.blit_rgba(176, 152, bar.width, bar.height, bar.pixels);
            }
        }
    }

    constexpr std::array<std::array<std::int32_t, 2>, 8> icon_positions{{
        {{16, 120}}, {{56, 120}}, {{16, 144}}, {{56, 144}},
        {{16, 168}}, {{56, 168}}, {{16, 192}}, {{56, 192}},
    }};
    const auto variant = source_alt_icon_palette(record) ? 1U : 0U;
    const auto draw_equipment_border = [&](std::size_t slot) {
        if (slot >= record.inventory_item_codes.size()) {
            return;
        }
        const auto overlay_index = ((slot / 2U) & 1U) == 0U ? 0U : 1U;
        const auto& overlay = assets.field_menu_selected_cells[overlay_index];
        expected.blit_rgba(8 + static_cast<std::int32_t>((slot % 2U) * 40U),
                           112 + static_cast<std::int32_t>((slot / 2U) * 24U),
                           overlay.width, overlay.height, overlay.pixels);
    };
    if (record.display_state >= 0 && record.display_state != 0x7F) {
        draw_equipment_border(static_cast<std::size_t>(record.display_state));
    }
    if (record.equipped_armor_index != 0U) {
        const auto armor_code = static_cast<std::uint8_t>(
            0x2BU + record.equipped_armor_index);
        for (std::size_t slot = 0; slot < record.inventory_item_codes.size();
             ++slot) {
            if (record.inventory_item_codes[slot] == armor_code) {
                draw_equipment_border(slot);
                break;
            }
        }
    }
    // The source composition draws the equipment cell layer first and the
    // inventory icon tiles over it, matching the VDP plane order.
    for (std::size_t slot = 0; slot < record.inventory_item_codes.size();
         ++slot) {
        const auto code = record.inventory_item_codes[slot];
        if (code == 0U || code > assets.field_menu_inventory_icons.size()) {
            continue;
        }
        const auto& icon = assets.field_menu_inventory_icons[code - 1U][variant];
        expected.blit_rgba(icon_positions[slot][0], icon_positions[slot][1],
                           icon.width, icon.height, icon.pixels);
    }
    if (selection >= 2U && selection <= 6U) {
        const auto& focus = assets.field_menu_command_focus[selection - 2U];
        expected.blit_rgba(128 + static_cast<std::int32_t>((selection - 2U) * 24U),
                           184, focus.width, focus.height, focus.pixels);
    }
    if (selection < 2U) {
        draw_inventory_selector_outline(expected, selection, row);
        const auto slot = row * 2U + selection;
        if (slot < record.inventory_item_codes.size()) {
            const auto item_code = record.inventory_item_codes[slot];
            draw_item_detail(item_code, true);
            if (item_code >= 0x2CU && item_code <= 0x32U) {
                // 0x12A8C -> 0x11352 temporarily previews the selected
                // D4D50 armor row in the lower panel. The selected armor
                // replaces the equipped summary at these same coordinates;
                // no Wpns/Guns pair is drawn beside the upper item detail.
                const auto armor_row = static_cast<std::size_t>(
                    item_code - 0x2CU);
                if (armor_row < assets.field_menu_armor_labels.size()) {
                    const auto& armor = assets.field_menu_armor_labels[armor_row];
                    for (std::size_t line = 0; line < armor.size(); ++line) {
                        write_text(128,
                                   128 + static_cast<std::int32_t>(line * 8U),
                                   armor[line]);
                    }
                }
                write_text(136, 144, "Wpns");
                write_text(136, 152, "Guns");
                if (armor_row <
                    assets.field_menu_armor_summary_modifiers.size()) {
                    const auto& modifiers =
                        assets.field_menu_armor_summary_modifiers[armor_row];
                    if (modifiers.weapons <
                        assets.field_menu_armor_summary_bars.size()) {
                        const auto& bar = assets.field_menu_armor_summary_bars[
                            modifiers.weapons];
                        expected.blit_rgba(176, 144, bar.width, bar.height,
                                           bar.pixels);
                    }
                    if (modifiers.guns <
                        assets.field_menu_armor_summary_bars.size()) {
                        const auto& bar = assets.field_menu_armor_summary_bars[
                            modifiers.guns];
                        expected.blit_rgba(176, 152, bar.width, bar.height,
                                           bar.pixels);
                    }
                }
            }
        }
    }
    constexpr std::array<std::string_view, 7> labels{{
        "    Inventory    ", "    Inventory    ", "      Clips      ",
        "Attributes/Skills", "    Cyberware    ", "      Magic      ",
        "Pocket Secretary ",
    }};
    write_text(112, 200, labels[selection]);
    if (!assets.field_hud.pixels.empty()) {
        expected.blit_rgba(0, 0, assets.field_hud.width,
                           assets.field_hud.height, assets.field_hud.pixels);
    }
    return expected;
}

shadowrun::graphics::RgbaImage screen_image(const SoftwareScreen& screen) {
    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

void capture_and_compare(const SoftwareScreen& actual,
                         const SoftwareScreen& expected,
                         const std::filesystem::path& directory,
                         const std::string& label) {
    std::filesystem::create_directories(directory);
    const auto actual_path = directory / (label + "-actual.bmp");
    const auto expected_path = directory / (label + "-expected.bmp");
    const auto diff_path = directory / (label + "-diff.bmp");
    shadowrun::graphics::write_bmp32(actual_path.string(), screen_image(actual));
    shadowrun::graphics::write_bmp32(expected_path.string(), screen_image(expected));

    auto diff = screen_image(actual);
    std::size_t mismatches = 0;
    for (std::size_t index = 0; index < actual.pixels().size(); ++index) {
        const auto x = static_cast<std::int32_t>(
            index % static_cast<std::size_t>(kScreenWidth));
        if (x >= shadowrun::native::kFieldHudX) {
            diff.pixels[index] = {0, 0, 0, 255};
            continue;
        }
        if (actual.pixels()[index] == expected.pixels()[index]) {
            diff.pixels[index] = {0, 0, 0, 255};
        } else {
            diff.pixels[index] = {255, 0, 0, 255};
            ++mismatches;
        }
    }
    shadowrun::graphics::write_bmp32(diff_path.string(), diff);
    if (mismatches != 0U) {
        throw std::runtime_error(
            "full-frame visual mismatch for " + label + ": " +
            std::to_string(mismatches) + " pixels; see " +
            actual_path.string() + ", " + expected_path.string() +
            ", and " + diff_path.string());
    }
}

void write_case_manifest_row(std::ofstream& manifest,
                             const std::string& label,
                             std::string_view stage,
                             std::size_t archetype,
                             const NativePortShell& shell) {
    manifest << label << '\t'
             << stage << '\t'
             << archetype << '\t'
             << shell.field_menu_selection() << '\t'
             << shell.field_menu_row() << '\t'
             << static_cast<unsigned>(shell.field_active_item_code()) << '\t'
             << static_cast<int>(shell.field_equipped_weapon_slot()) << '\t'
             << static_cast<unsigned>(shell.field_equipped_armor_index()) << '\t'
             << static_cast<unsigned>(shell.field_posture_index()) << '\t'
             << static_cast<unsigned>(shell.field_attack_bar_index()) << '\t'
             << static_cast<unsigned>(shell.field_defense_bar_index()) << '\t';
    for (std::size_t slot = 0; slot < 8U; ++slot) {
        if (slot != 0U) {
            manifest << ',';
        }
        manifest << static_cast<unsigned>(shell.field_inventory_item_code(slot));
    }
    manifest << "\t" << label << "-actual.bmp\t"
             << label << "-expected.bmp\t"
             << label << "-diff.bmp\n";
    manifest.flush();
}

void require_image_pixels(const SoftwareScreen& screen, const NativeImage& image,
                          std::int32_t destination_x,
                          std::int32_t destination_y,
                          const std::string& label) {
    if (image.pixels.size() !=
        static_cast<std::size_t>(image.width) * image.height) {
        throw std::runtime_error(label + " has an invalid source image");
    }
    std::size_t compared = 0;
    std::size_t mismatches = 0;
    for (std::size_t y = 0; y < image.height; ++y) {
        for (std::size_t x = 0; x < image.width; ++x) {
            const auto& source = image.pixels[y * image.width + x];
            if (source.alpha == 0U) {
                continue;
            }
            ++compared;
            if (screen.pixel(destination_x + static_cast<std::int32_t>(x),
                             destination_y + static_cast<std::int32_t>(y)) !=
                pack(source)) {
                ++mismatches;
            }
        }
    }
    if (compared == 0U || mismatches != 0U) {
        throw std::runtime_error(
            label + " differs from its extracted Genesis pixels (" +
            std::to_string(mismatches) + "/" + std::to_string(compared) +
            " mismatched)");
    }
}

void require_source_text(const SoftwareScreen& screen, const NativeImage& font,
                         std::int32_t destination_x,
                         std::int32_t destination_y,
                         const std::string& text,
                         const std::string& label) {
    if (font.width < 128U || font.height < 48U) {
        throw std::runtime_error("field-menu font atlas is incomplete");
    }
    std::size_t compared = 0;
    std::size_t mismatches = 0;
    auto cursor = destination_x;
    for (const auto character : text) {
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph_index = static_cast<std::size_t>(code - 0x20U);
        const auto glyph_x = (glyph_index % 16U) * 8U;
        const auto glyph_y = (glyph_index / 16U) * 8U;
        for (std::size_t y = 0; y < 8U; ++y) {
            for (std::size_t x = 0; x < 8U; ++x) {
                const auto& source = font.pixels[(glyph_y + y) * font.width +
                                                 glyph_x + x];
                if (source.alpha == 0U) {
                    continue;
                }
                const auto dest_x = cursor + static_cast<std::int32_t>(x);
                if (dest_x >= shadowrun::native::kFieldHudX) {
                    continue;
                }
                ++compared;
                if (screen.pixel(dest_x,
                                 destination_y + static_cast<std::int32_t>(y)) !=
                    pack(source)) {
                    ++mismatches;
                }
            }
        }
        cursor += 8;
    }
    if (compared == 0U || mismatches != 0U) {
        throw std::runtime_error(
            label + " differs from the extracted source glyphs (" +
            std::to_string(mismatches) + "/" + std::to_string(compared) +
            " mismatched)");
    }
}

void require_edge_pixels(const SoftwareScreen& screen, const NativeImage& image,
                         std::int32_t destination_x,
                         std::int32_t destination_y,
                         const std::string& label) {
    std::size_t compared = 0;
    std::size_t mismatches = 0;
    for (std::size_t y = 0; y < image.height; ++y) {
        for (std::size_t x = 0; x < image.width; ++x) {
            if (x >= 3U && y >= 3U && x + 3U < image.width &&
                y + 3U < image.height) {
                continue;
            }
            const auto& source = image.pixels[y * image.width + x];
            if (source.alpha == 0U) {
                continue;
            }
            ++compared;
            if (screen.pixel(destination_x + static_cast<std::int32_t>(x),
                             destination_y + static_cast<std::int32_t>(y)) !=
                pack(source)) {
                ++mismatches;
            }
        }
    }
    if (compared == 0U || mismatches != 0U) {
        throw std::runtime_error(
            label + " differs from its extracted equipment border (" +
            std::to_string(mismatches) + "/" + std::to_string(compared) +
            " mismatched)");
    }
}

bool inside(std::int32_t x, std::int32_t y, std::int32_t left,
            std::int32_t top, std::int32_t width, std::int32_t height) {
    return x >= left && x < left + width && y >= top && y < top + height;
}

void require_static_menu_pixels(const SoftwareScreen& actual,
                                const NativePortAssets& assets) {
    SoftwareScreen expected;
    expected.clear({0, 0, 0, 255});
    expected.blit_rgba(0, 0, assets.field_menu_background.width,
                       assets.field_menu_background.height,
                       assets.field_menu_background.pixels);
    expected.blit_rgba(0, 0, assets.field_menu_inventory_boxes.width,
                       assets.field_menu_inventory_boxes.height,
                       assets.field_menu_inventory_boxes.pixels);
    if (!assets.field_hud.pixels.empty()) {
        expected.blit_rgba(0, 0, assets.field_hud.width,
                           assets.field_hud.height, assets.field_hud.pixels);
    }

    // These are the source-owned dynamic rectangles. Every other pixel must
    // still be the exact composition of the two 0x10F50/0x116C4 plane layers
    // plus the 64px field HUD column.
    std::size_t compared = 0;
    std::size_t mismatches = 0;
    std::string first_mismatch;
    for (std::int32_t y = 0; y < expected.height(); ++y) {
        for (std::int32_t x = 0; x < expected.width(); ++x) {
            const auto dynamic =
                inside(x, y, 16, 16, 64, 80) ||
                inside(x, y, 112, 0, 144, 184) ||
                inside(x, y, 144, 24, 128, 64) ||
                inside(x, y, 8, 112, 80, 112) ||
                inside(x, y, 128, 184, 112, 16) ||
                inside(x, y, 112, 200, 144, 24) ||
                inside(x, y, shadowrun::native::kFieldHudX, 0,
                       kScreenWidth - shadowrun::native::kFieldHudX,
                       kScreenHeight);
            if (dynamic) {
                continue;
            }
            ++compared;
            if (actual.pixel(x, y) != expected.pixel(x, y)) {
                ++mismatches;
                if (first_mismatch.empty()) {
                    first_mismatch = " at (" + std::to_string(x) + "," +
                        std::to_string(y) + ") actual=0x" +
                        std::to_string(actual.pixel(x, y)) + " expected=0x" +
                        std::to_string(expected.pixel(x, y));
                }
            }
        }
    }
    if (mismatches != 0U) {
        throw std::runtime_error(
            "field-menu static plane differs outside dynamic regions (" +
            std::to_string(mismatches) + "/" + std::to_string(compared) +
            " mismatched)" + first_mismatch);
    }
}

bool source_alt_icon_palette(const NativeFieldEntity& entity) {
    // Direct transcription of ROM 0x3602's +0x56/+0x57/+0x9B branch. The
    // expected image is chosen from the extracted source variants, never from
    // a host-generated icon or a guessed item category.
    if (entity.display_substate == 0) {
        return false;
    }
    if (entity.display_palette_state >= 2U) {
        return true;
    }
    if (entity.display_state >= 0) {
        return entity.display_substate < 15;
    }
    if (entity.display_substate < 6) {
        return false;
    }
    return true;
}

void require_equipment_borders(const SoftwareScreen& screen,
                               const NativePortAssets& assets,
                               const NativeFieldEntity& source_record) {
    constexpr std::array<std::array<std::int32_t, 2>, 8> positions{{
        {{8, 112}}, {{48, 112}}, {{8, 136}}, {{48, 136}},
        {{8, 160}}, {{48, 160}}, {{8, 184}}, {{48, 184}},
    }};
    const auto check_slot = [&](std::size_t slot, const std::string& reason) {
        const auto overlay_index = ((slot / 2U) & 1U) == 0U ? 0U : 1U;
        const auto& overlay = assets.field_menu_selected_cells[overlay_index];
        require_edge_pixels(screen, overlay, positions[slot][0], positions[slot][1],
                             reason);
    };

    if (source_record.display_state >= 0 &&
        source_record.display_state != 0x7F) {
        check_slot(static_cast<std::size_t>(source_record.display_state),
                   "equipped weapon border");
    }
    if (source_record.equipped_armor_index != 0U) {
        const auto armor_code = static_cast<std::uint8_t>(
            0x2BU + source_record.equipped_armor_index);
        for (std::size_t slot = 0; slot < source_record.inventory_item_codes.size();
             ++slot) {
            if (source_record.inventory_item_codes[slot] == armor_code) {
                check_slot(slot, "equipped armor border");
                break;
            }
        }
    }
}

void test_field_menu_visual_contract(
    const NativePortAssets& assets, const std::filesystem::path& artifact_dir) {
    std::filesystem::create_directories(artifact_dir);
    std::ofstream manifest(artifact_dir / "manifest.tsv",
                           std::ios::out | std::ios::trunc);
    if (!manifest) {
        throw std::runtime_error("could not create visual artifact manifest: " +
                                 (artifact_dir / "manifest.tsv").string());
    }
    manifest << "case\tstage\tarchetype\tselection\trow\tactive_item_code\t"
                "equipped_weapon_slot\tequipped_armor_index\tposture_index\t"
                "attack_bar_index\tdefense_bar_index\tinventory_item_codes\t"
                "actual_bmp\texpected_bmp\tdiff_bmp\n";

    constexpr std::array<std::array<std::int32_t, 2>, 8> icon_positions{{
        {{16, 120}}, {{56, 120}}, {{16, 144}}, {{56, 144}},
        {{16, 168}}, {{56, 168}}, {{16, 192}}, {{56, 192}},
    }};
    constexpr std::array<std::array<std::int32_t, 2>, 5> focus_positions{{
        {{128, 184}}, {{152, 184}}, {{176, 184}}, {{200, 184}},
        {{224, 184}},
    }};

    // Exercise every source startup archetype. This catches the exact class
    // of regression that lets a Samurai portrait/icon set leak into a Decker
    // or Shaman command screen.
    for (std::size_t archetype = 0; archetype < 3U; ++archetype) {
        NativePortShell shell("SHADOWRUN", assets);
        enter_field(shell, archetype);
        SoftwareScreen screen;
        shell.render(screen);
        require_static_menu_pixels(screen, assets);

        const auto& source_record = assets.protagonist_archetype_records[archetype];
        const auto overview_label =
            "archetype-" + std::to_string(archetype) + "-overview";
        write_case_manifest_row(manifest, overview_label, "overview", archetype,
                                shell);
        capture_and_compare(
            screen,
            source_field_menu_reference(assets, source_record, archetype,
                                        shell.field_menu_selection(),
                                        shell.field_menu_row()),
            artifact_dir, overview_label);
        require_image_pixels(screen, assets.field_menu_runner_portraits[archetype],
                             16, 16, "runner portrait for archetype " +
                                 std::to_string(archetype));
        require_source_text(screen, assets.field_menu_font, 128, 8,
                            assets.field_menu_runner_names[0],
                            "runner name");
        const auto source_class = [&]() {
            switch (source_record.archetype_index) {
            case 1U:
                return assets.intro.archetypes[2] + " " + assets.intro.archetypes[3];
            case 2U:
                return assets.intro.archetypes[0];
            case 3U:
                return assets.intro.archetypes[1];
            default:
                return std::string("Unknown");
            }
        }();
        require_source_text(screen, assets.field_menu_font, 128, 24,
                            "Human " + source_class, "runner class");
        require_source_text(screen, assets.field_menu_font, 128, 40,
                            "Physical", "physical label");
        require_source_text(screen, assets.field_menu_font, 128, 48,
                            "Mental", "mental label");
        require_source_text(screen, assets.field_menu_font, 112, 104,
                            "Atk", "attack label");
        require_source_text(screen, assets.field_menu_font, 112, 112,
                            "Def", "defense label");
        require_source_text(screen, assets.field_menu_font, 136, 144,
                            "Wpns", "weapons label");
        require_source_text(screen, assets.field_menu_font, 136, 152,
                            "Guns", "guns label");
        if (source_record.menu_status_index < assets.field_menu_status_names.size()) {
            require_source_text(
                screen, assets.field_menu_font, 136, 176,
                assets.field_menu_status_names[source_record.menu_status_index],
                "status label");
        }
        if (source_record.active_item_code != 0U &&
            source_record.active_item_code <= assets.inventory_items.size()) {
            const auto& active_item = assets.inventory_items[
                source_record.active_item_code - 1U];
            require_source_text(
                screen, assets.field_menu_font, 128, 64,
                center_source_fixed_field_for_test(active_item.name),
                "active item name");
            require_source_text(
                screen, assets.field_menu_font, 128, 72,
                center_source_fixed_field_for_test(active_item.category),
                "active item category");
        }
        if (shell.field_equipped_weapon_slot() != source_record.display_state ||
            shell.field_equipped_armor_index() != source_record.equipped_armor_index) {
            throw std::runtime_error(
                "field-menu equipment state diverged from the selected source record");
        }
        const auto variant = source_alt_icon_palette(source_record) ? 1U : 0U;
        for (std::size_t slot = 0; slot < source_record.inventory_item_codes.size();
             ++slot) {
            const auto code = source_record.inventory_item_codes[slot];
            if (code == 0U || code > assets.field_menu_inventory_icons.size()) {
                continue;
            }
            require_image_pixels(
                screen, assets.field_menu_inventory_icons[code - 1U][variant],
                icon_positions[slot][0], icon_positions[slot][1],
                "inventory icon for archetype " + std::to_string(archetype) +
                    ", slot " + std::to_string(slot) + ", item " +
                    std::to_string(code));
        }
        require_equipment_borders(screen, assets, source_record);

        // The five small middle buttons are source 2x2 tile blocks, not a
        // label-only focus indicator. Check every block in its real position.
        for (std::size_t focus = 0; focus < 5U; ++focus) {
            while (shell.field_menu_selection() != focus + 2U) {
                press(shell, Button::Left);
            }
            shell.render(screen);
            const auto focus_label =
                "archetype-" + std::to_string(archetype) + "-focus-" +
                std::to_string(focus);
            write_case_manifest_row(manifest, focus_label, "command-focus",
                                    archetype, shell);
            capture_and_compare(
                screen,
                source_field_menu_reference(
                    assets, source_record, archetype,
                    shell.field_menu_selection(), shell.field_menu_row()),
                artifact_dir, focus_label);
            require_image_pixels(screen, assets.field_menu_command_focus[focus],
                                 focus_positions[focus][0],
                                 focus_positions[focus][1],
                                 "command-tab focus " + std::to_string(focus));
        }

        // Inventory selection uses the thin warm-gold outline visible in the
        // independent section-06 frames, separate from equipment borders.
        while (shell.field_menu_selection() != 0U) {
            press(shell, Button::Left);
        }
        shell.render(screen);
        require_inventory_selector_outline(
            screen, shell.field_menu_selection(), shell.field_menu_row());

        // Each occupied inventory cell also opens the source item-detail
        // page. Verify the exact item art selected by the source item code,
        // not merely that some pixels appeared in the detail panel.
        for (std::size_t slot = 0; slot < source_record.inventory_item_codes.size();
             ++slot) {
            while (shell.field_menu_selection() != 0U) {
                press(shell, Button::Left);
            }
            while (shell.field_menu_row() != 0U) {
                press(shell, Button::Up);
            }
            for (std::size_t row = 0; row < slot / 2U; ++row) {
                press(shell, Button::Down);
            }
            if ((slot & 1U) != 0U) {
                press(shell, Button::Right);
            }
            const auto code = source_record.inventory_item_codes[slot];
            if (code == 0U) {
                continue;
            }
            shell.render(screen);
            const auto item_label =
                "archetype-" + std::to_string(archetype) + "-item-" +
                std::to_string(slot);
            write_case_manifest_row(manifest, item_label, "inventory-item",
                                    archetype, shell);
            capture_and_compare(
                screen,
                source_field_menu_reference(
                    assets, source_record, archetype,
                    shell.field_menu_selection(), shell.field_menu_row()),
                artifact_dir, item_label);
            const auto& item_art = assets.field_menu_item_art[code - 1U];
            const auto has_item_art = std::any_of(
                item_art.pixels.begin(), item_art.pixels.end(),
                [](const auto& pixel) { return pixel.alpha != 0U; });
            // The ROM has intentional no-art records (items 40-42). An
            // empty extracted art record means the detail panel should not
            // be checked for opaque pixels.
            if (has_item_art) {
                require_image_pixels(
                    screen, item_art, 144, 24,
                    "item detail art for archetype " +
                        std::to_string(archetype) + ", slot " +
                        std::to_string(slot) + ", item " +
                        std::to_string(code));
            }
        }
    }
}

} // namespace

int main() {
    try {
        const auto rom = read_rom();
        const auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
        test_field_menu_visual_contract(
            assets, std::filesystem::path(SHADOWRUN_VISUAL_ARTIFACT_DIR));
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "menu visual regression: %s\n", error.what());
        return 1;
    }
}
