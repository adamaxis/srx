#include "shadowrun/graphics.hpp"
#include "shadowrun/native_port.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
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
    press(shell, Button::Start);
    press(shell, Button::Start);
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
                ++compared;
                if (screen.pixel(cursor + static_cast<std::int32_t>(x),
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

    // These are the source-owned dynamic rectangles. Every other pixel must
    // still be the exact composition of the two 0x10F50/0x116C4 plane layers.
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
                inside(x, y, 112, 200, 144, 24);
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

void test_field_menu_visual_contract(const NativePortAssets& assets) {
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
            require_source_text(screen, assets.field_menu_font, 128, 64,
                                active_item.name, "active item name");
            require_source_text(screen, assets.field_menu_font, 128, 72,
                                active_item.category, "active item category");
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
            require_image_pixels(screen, assets.field_menu_command_focus[focus],
                                 focus_positions[focus][0],
                                 focus_positions[focus][1],
                                 "command-tab focus " + std::to_string(focus));
        }

        // Inventory selection uses the extracted 24x32 sprite. It is a
        // separate cursor from the equipment border and must land on slot 0.
        while (shell.field_menu_selection() != 0U) {
            press(shell, Button::Left);
        }
        shell.render(screen);
        require_image_pixels(screen, assets.field_menu_cursor, 12, 116,
                             "inventory cursor at slot 0");

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
            require_image_pixels(
                screen, assets.field_menu_item_art[code - 1U], 144, 24,
                "item detail art for archetype " + std::to_string(archetype) +
                    ", slot " + std::to_string(slot) + ", item " +
                    std::to_string(code));
            require_image_pixels(
                screen, assets.field_menu_cursor,
                12 + static_cast<std::int32_t>((slot & 1U) * 40U),
                116 + static_cast<std::int32_t>((slot / 2U) * 24U),
                "inventory cursor for slot " + std::to_string(slot));
        }
    }
}

} // namespace

int main() {
    try {
        const auto rom = read_rom();
        const auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
        test_field_menu_visual_contract(assets);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "menu visual regression: %s\n", error.what());
        return 1;
    }
}
