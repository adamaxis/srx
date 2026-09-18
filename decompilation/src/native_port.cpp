#include "shadowrun/native_port.hpp"

#include "shadowrun/entity_projection.hpp"
#include "shadowrun/graphics.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace shadowrun::native {

namespace {

using Glyph = std::array<std::uint8_t, 7>;

Glyph glyph_for(char character) {
    if (character >= 'a' && character <= 'z') {
        character = static_cast<char>(character - ('a' - 'A'));
    }
    switch (character) {
    case 'A': return {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    case 'B': return {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    case 'C':
    case 'c': return {0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F};
    case 'D': return {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
    case 'E': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    case 'F': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    case 'G': return {0x0F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0F};
    case 'H': return {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    case 'I': return {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F};
    case 'J': return {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E};
    case 'K': return {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    case 'L': return {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    case 'M': return {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
    case 'N': return {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    case 'O': return {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    case 'P': return {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    case 'Q': return {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
    case 'R': return {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    case 'S': return {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    case 'T': return {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    case 'U': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    case 'V': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
    case 'W': return {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
    case 'X': return {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
    case 'Y': return {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    case 'Z': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
    case '0': return {0x0E, 0x13, 0x15, 0x15, 0x15, 0x19, 0x0E};
    case '1': return {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    case '2': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    case '3': return {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
    case '4': return {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    case '5': return {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
    case '6': return {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    case '7': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    case '8': return {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    case '9': return {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E};
    case '+': return {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00};
    case '-': return {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
    case ':': return {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00};
    case '.': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};
    case '/': return {0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10};
    case '?': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04};
    case '!': return {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04};
    default: return {0, 0, 0, 0, 0, 0, 0};
    }
}

} // namespace

SoftwareScreen::SoftwareScreen(std::uint16_t width, std::uint16_t height)
    : width_(width), height_(height),
      pixels_(static_cast<std::size_t>(width) * height, 0) {}

void SoftwareScreen::begin_frame() {}

std::uint32_t SoftwareScreen::pack(Rgba8 color) {
    return (static_cast<std::uint32_t>(color.red) << 16U) |
           (static_cast<std::uint32_t>(color.green) << 8U) |
           static_cast<std::uint32_t>(color.blue);
}

void SoftwareScreen::clear(Rgba8 color) {
    std::fill(pixels_.begin(), pixels_.end(), pack(color));
}

void SoftwareScreen::fill_rect(std::int32_t x, std::int32_t y,
                               std::int32_t width, std::int32_t height,
                               Rgba8 color) {
    const auto left = std::max<std::int32_t>(x, 0);
    const auto top = std::max<std::int32_t>(y, 0);
    const auto right = std::min<std::int32_t>(x + width, width_);
    const auto bottom = std::min<std::int32_t>(y + height, height_);
    if (left >= right || top >= bottom) {
        return;
    }

    const auto packed = pack(color);
    for (auto row = top; row < bottom; ++row) {
        const auto begin = pixels_.begin() +
            static_cast<std::size_t>(row) * width_ + left;
        std::fill(begin, begin + (right - left), packed);
    }
}

void SoftwareScreen::blit_rgba(std::int32_t x, std::int32_t y,
                               std::uint16_t width, std::uint16_t height,
                               const std::vector<Rgba8>& pixels) {
    if (pixels.size() != static_cast<std::size_t>(width) * height) {
        return;
    }
    for (std::int32_t source_y = 0; source_y < height; ++source_y) {
        for (std::int32_t source_x = 0; source_x < width; ++source_x) {
            const auto target_x = x + source_x;
            const auto target_y = y + source_y;
            if (target_x < 0 || target_y < 0 || target_x >= width_ ||
                target_y >= height_) {
                continue;
            }
            const auto& source = pixels[static_cast<std::size_t>(source_y) *
                                         width + source_x];
            if (source.alpha == 0) {
                continue;
            }
            pixels_[static_cast<std::size_t>(target_y) * width_ + target_x] =
                pack(source);
        }
    }
}

void SoftwareScreen::write_glyph(std::int32_t x, std::int32_t y,
                                 char character, std::uint8_t scale,
                                 std::uint32_t packed_color) {
    const auto glyph = glyph_for(character);
    for (std::int32_t row = 0; row < 7; ++row) {
        for (std::int32_t column = 0; column < 5; ++column) {
            if ((glyph[static_cast<std::size_t>(row)] &
                 (1U << (4U - static_cast<unsigned>(column)))) == 0) {
                continue;
            }
            fill_rect(x + column * scale, y + row * scale,
                      scale, scale,
                      Rgba8{
                          static_cast<std::uint8_t>(packed_color >> 16U),
                          static_cast<std::uint8_t>(packed_color >> 8U),
                          static_cast<std::uint8_t>(packed_color), 255});
        }
    }
}

void SoftwareScreen::write_text(std::int32_t x, std::int32_t y,
                                std::string_view text, std::uint8_t scale,
                                Rgba8 color) {
    if (scale == 0) {
        return;
    }
    const auto packed = pack(color);
    auto cursor = x;
    for (const auto character : text) {
        if (character == '\n') {
            cursor = x;
            y += 8 * scale;
            continue;
        }
        write_glyph(cursor, y, character, scale, packed);
        cursor += 6 * scale;
    }
}

void SoftwareScreen::present() {
    ++presented_frames_;
}

std::uint32_t SoftwareScreen::pixel(std::int32_t x, std::int32_t y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return 0;
    }
    return pixels_[static_cast<std::size_t>(y) * width_ + x];
}

namespace {

std::uint16_t fixed_world_coordinate(std::int32_t pixels) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(pixels) << 4U);
}

constexpr std::int32_t kGenesisVdpSpriteOrigin = 0x80;
constexpr std::int32_t kFieldViewportWidth = kScreenWidth;
constexpr std::int32_t kFieldViewportHeight = 168;
constexpr std::int32_t kFieldScreenOriginY = 36;
// ROM 0x5A62 accepts VDP X values strictly below 0x1AE. After removing the
// 0x80 sprite origin, this is the last host pixel at which the record remains
// in the direct projection window.
constexpr std::int32_t kFieldEntityMaxHostX = 0x1AE -
                                                kGenesisVdpSpriteOrigin - 1;

// The opening is not a menu page carousel.  New Game enters ROM 0x59C5E,
// which runs the three timed presentation routines 0x59D7C, 0x5AA96, and
// 0x5AD1A.  These groups are the text writes those routines make through
// C0DA.  Their source line order is retained in NativeIntroData; the native
// renderer only replaces the VDP destination with a host pixel position.
struct IntroStoryGroup {
    std::uint64_t start_frame;
    std::size_t first_line;
    std::size_t line_count;
    std::int32_t top_y;
};

constexpr std::array<IntroStoryGroup, 5> kIntroStoryGroups{{
    // 0x59D7C's first 0xA0-vblank presentation.
    {0x000U, 0U, 5U, 72},
    // 0x59D7C's second loop.  The 0xADFE/0xAE3A writes at countdown 0x164
    // add the three ambush aftermath lines before 0x5AA96 begins.
    {0x0A0U, 5U, 5U, 72},
    // 0x5AA96 writes the tri-vid paragraph before its 0x114-frame loop.
    {0x290U, 10U, 3U, 80},
    // 0x5AD1A calls 0x5A9E2 during setup, before its 0x528-frame loop.
    {0x3A4U, 13U, 3U, 80},
    // 0x5AD1A calls 0x5A96A when its countdown reaches 0x1E0.
    {0x6ECU, 16U, 6U, 64},
}};

constexpr std::uint64_t kIntroDuration = 0x8CCU;

const IntroStoryGroup& intro_story_group(std::uint64_t frame) {
    const auto group = std::find_if(
        kIntroStoryGroups.rbegin(), kIntroStoryGroups.rend(),
        [frame](const IntroStoryGroup& candidate) {
            return frame >= candidate.start_frame;
        });
    return group == kIntroStoryGroups.rend() ? kIntroStoryGroups.front()
                                             : *group;
}

// ROM 0x391A walks entity records after the current record. A candidate must
// be active (bit 0), not suppressed (bit 6), and reach the selection exit
// with bit 4 clear. These are record-state predicates, not UI guesses. The
// record type at +0x3E controls wrap-around when the current record is type 2;
// it does not make that third startup record ineligible.
bool runner_switch_candidate(const NativeFieldRecord& record) {
    return (record.flags_10 & 0x01U) != 0 &&
           (record.flags_10 & 0x40U) == 0 &&
           (record.flags_10 & 0x10U) == 0;
}

// ROM 0x1170C selects these labels from the 17-byte strings beginning at
// 0xD1D28. The first two selector values intentionally resolve to the same
// Inventory label; their secondary row is handled by FF F0D1 in 0x11506.
constexpr std::array<std::string_view, 7> kFieldMenuLabels{{
    "    Inventory    ",
    "    Inventory    ",
    "      Clips      ",
    "Attributes/Skills",
    "    Cyberware    ",
    "      Magic      ",
    "Pocket Secretary ",
}};

// ROM 0x12B8E..0x12BBA chooses the literal strings at 0xD29C4 (ammo),
// 0xD29CA (uses), and 0xD29D0 (rtng). Codes 0x15..0x17 and 0x29..0x2B
// fall through to item-specific handlers; armor >=0x2C uses the separate
// 0x11352 armor writer.
std::string_view source_item_detail_label(std::uint8_t item_code) {
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

bool field_menu_uses_alt_item_palette(const NativeFieldEntity& entity) {
    // ROM 0x3602 selects the command-screen icon palette from the active
    // record's +0x56, +0x57, and +0x9B bytes. Keep the branch structure in
    // the native boundary instead of choosing a host color per item.
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

// ROM 0x11B58 enters this four-option clip submenu with selector two. The
// 0x118C6 helper keeps the selector in the inclusive range 2..5.
constexpr std::array<std::string_view, 4> kClipsMenuLabels{{
    "Increase Clips",
    "Decrease Clips",
    "Trade Clips",
    "Change Recipient",
}};

} // namespace

namespace {

std::uint8_t source_inventory_phase_for_code(
    const NativeFieldEntity& entity, std::uint8_t item_code) {
    for (std::size_t slot = 0; slot < entity.inventory_item_codes.size();
         ++slot) {
        if (entity.inventory_item_codes[slot] == item_code) {
            return entity.inventory_phase_bytes[slot];
        }
    }
    return 0;
}

unsigned source_command_average(const NativeFieldEntity& entity) {
    // 0x55ED6: (+0x78 + +0x7B + +0x7C) / 2, then the phase byte for item
    // 0x28 if that special item is present in one of the eight slots.
    const auto average =
        (static_cast<unsigned>(entity.attribute_values[1]) +
         static_cast<unsigned>(entity.attribute_values[4]) +
         static_cast<unsigned>(entity.attribute_values[5])) /
        2U;
    return average + source_inventory_phase_for_code(entity, 0x28U);
}

unsigned source_posture_value(unsigned base, std::uint8_t posture,
                              bool defense) {
    // 0x55F20: D1 * (+0x4D * 0x19) / 0x64. D7 >= 0 returns the penalty
    // (attack); D7 < 0 returns the post-penalty value (defense).
    const auto penalty =
        base * static_cast<unsigned>(posture) * 0x19U / 0x64U;
    return defense ? base - std::min(base, penalty) : penalty;
}

unsigned source_merge_skill(unsigned current, unsigned skill) {
    // 0x12C74's 12CC2 tail: when skill is below the current value it is
    // doubled; otherwise it is added to the current value. Zero becomes one.
    if (skill < current) {
        current = skill;
    }
    current += skill;
    return current == 0U ? 1U : current;
}

std::uint8_t source_bar_index(unsigned value) {
    // 0x12E7A decrements the selected row and clamps negative values to the
    // first source strip. The extracted table contains 23 rows.
    return static_cast<std::uint8_t>(
        std::min<unsigned>(22U, value == 0U ? 0U : value - 1U));
}

std::uint8_t source_item_factor(
    const std::array<NativeInventoryItem, kNativeInventoryItemCount>& items,
    std::uint8_t code) {
    if (code == 0U || code > items.size()) {
        return 0;
    }
    return items[code - 1U].source_combat_factor;
}

unsigned source_store_ammunition_units(
    unsigned rounds,
    const std::array<NativeInventoryItem, kNativeInventoryItemCount>& items,
    std::uint8_t code) {
    // 0x125FA converts +0x4E into the common source unit before a weapon is
    // changed. The original rounds upward whenever the quotient is nonzero.
    const auto factor = source_item_factor(items, code);
    if (factor == 0U) {
        return rounds * 100U + (rounds == 0U ? 0U : 1U);
    }
    const auto quotient = rounds * 100U / factor;
    return quotient + (quotient == 0U ? 0U : 1U);
}

std::uint8_t source_load_ammunition(
    unsigned units,
    const std::array<NativeInventoryItem, kNativeInventoryItemCount>& items,
    std::uint8_t code) {
    // 0x12256..0x12284 converts the common unit into the new weapon's
    // rounds and rounds at a remainder of at least 50.
    const auto factor = source_item_factor(items, code);
    if (factor == 0U) {
        return static_cast<std::uint8_t>(units & 0xFFU);
    }
    const auto scaled = units * factor;
    auto rounds = scaled / 100U;
    if (scaled % 100U >= 50U) {
        ++rounds;
    }
    // The source stores the result with MOVE.B, so retain its low-byte
    // truncation instead of applying a host saturation rule.
    return static_cast<std::uint8_t>(rounds & 0xFFU);
}

} // namespace

NativeCommandBarIndices compute_command_bar_indices(
    const NativeFieldEntity& entity,
    const std::array<NativeInventoryItem, kNativeInventoryItemCount>&,
    std::uint8_t posture, std::uint8_t command_selector,
    std::uint8_t command_row) {
    NativeCommandBarIndices result{};
    const auto base = source_command_average(entity);

    // 0x12CEE is called with D0=0 for attack and D0=-1 for defense by
    // 0x11ABE. The startup/normal command overview takes the selector >= 2
    // path; the native shell only calls this helper for that same overview.
    const auto attack_value = [&]() {
        auto value = source_posture_value(base, posture, false);
        const auto active_item = entity.active_item_code;
        unsigned skill = 0;
        if (entity.display_state == 0x7F) {
            // 0x12DD2: no equipped weapon uses +0x85.
            skill = entity.skill_values[5];
        } else if (entity.display_state >= 0) {
            if (active_item < 0x0DU) {
                // 0x12D60 -> 0x12C74 selects +0x81/+0x82, +0x83, or
                // +0x84 according to the active item code.
                skill = entity.skill_values[1];
                if (active_item < 0x08U) {
                    skill = std::max<unsigned>(skill, entity.skill_values[2]);
                } else if (active_item < 0x0BU) {
                    skill = std::max<unsigned>(skill, entity.skill_values[3]);
                } else {
                    skill = std::max<unsigned>(skill, entity.skill_values[4]);
                }
            } else if (active_item < 0x10U) {
                // 0x12D46: item codes 0x0D..0x0F use +0x87 directly.
                skill = entity.skill_values[6];
            } else if (active_item == 0x17U) {
                skill = entity.skill_values[5];
            } else {
                // 0x12E98 returns without writing an attack strip for the
                // remaining item classes.
                return 0U;
            }
        } else {
            // The negative +0x56 branch (0x12D7C/0x12FA4) is a separate
            // special-item calculation. It is not used by the extracted
            // startup records; keep its unsupported result explicit rather
            // than substituting the old guessed formula.
            return 0U;
        }
        return source_merge_skill(value, skill);
    }();
    result.attack = source_bar_index(attack_value);

    // 0x12E12 only writes the defense strip for an empty/low item selector or
    // the special item 0x17. This is the selector read performed by 0x12148,
    // not the currently highlighted inventory cell's display code.
    const auto selected_slot = static_cast<std::size_t>(command_row) * 2U +
                               command_selector;
    const auto selected_item = selected_slot < entity.inventory_item_codes.size()
        ? entity.inventory_item_codes[selected_slot]
        : 0U;
    if (selected_item >= 0x10U && selected_item != 0x17U) {
        result.defense = 0U;
        return result;
    }
    auto defense_value = source_posture_value(base, posture, true);
    // 0x12E3E adds +0x77 before the cyberware branch.
    defense_value += entity.attribute_values[0];
    // 0x12E52..0x12E66 starts at 9 and examines cyberware bits 9..11,
    // subtracting eight for an installed flag and incrementing otherwise.
    unsigned cyberware_adjustment = 9U;
    for (unsigned bit = 9U; bit <= 11U; ++bit) {
        if ((entity.cyberware_flags & (1U << bit)) != 0U) {
            cyberware_adjustment -= 8U;
        } else {
            ++cyberware_adjustment;
        }
    }
    defense_value += cyberware_adjustment;
    result.defense = source_bar_index(defense_value);
    return result;
}

NativePortShell::NativePortShell(std::string title)
    : title_(std::move(title)) {}

std::string_view NativePortShell::field_active_item_detail_label() const {
    return source_item_detail_label(field_entity_.active_item_code);
}

NativePortShell::NativePortShell(
    std::string title, const std::vector<std::uint8_t>& rom)
    : NativePortShell(
          std::move(title), graphics::decode_genesis_native_assets(rom)) {}

NativePortShell::NativePortShell(std::string title, NativePortAssets assets)
    : title_(std::move(title)), scene_(NativeScene::Title) {
    title_width_ = assets.title_scene.width;
    title_height_ = assets.title_scene.height;
    title_pixels_ = std::move(assets.title_scene.pixels);
    title_background_ = std::move(assets.title_background);
    title_logo_plane_ = std::move(assets.title_logo_plane);
    title_text_ = std::move(assets.title_text);
    title_sprite_frames_ = std::move(assets.title_sprite_frames);
    title_animation_frames_ = std::move(assets.title_animation_frames);
    field_menu_background_width_ = assets.field_menu_background.width;
    field_menu_background_height_ = assets.field_menu_background.height;
    field_menu_background_pixels_ = std::move(assets.field_menu_background.pixels);
    field_menu_inventory_boxes_width_ = assets.field_menu_inventory_boxes.width;
    field_menu_inventory_boxes_height_ = assets.field_menu_inventory_boxes.height;
    field_menu_inventory_boxes_pixels_ =
        std::move(assets.field_menu_inventory_boxes.pixels);
    field_menu_font_width_ = assets.field_menu_font.width;
    field_menu_font_height_ = assets.field_menu_font.height;
    field_menu_font_pixels_ = std::move(assets.field_menu_font.pixels);
    startup_font_ = std::move(assets.startup_font);
    startup_cursor_ = std::move(assets.startup_cursor);
    field_menu_runner_portraits_ = std::move(assets.field_menu_runner_portraits);
    field_menu_selected_cells_ = std::move(assets.field_menu_selected_cells);
    field_menu_cursor_ = std::move(assets.field_menu_cursor);
    field_menu_item_art_ = std::move(assets.field_menu_item_art);
    field_menu_inventory_icons_ = std::move(assets.field_menu_inventory_icons);
    field_menu_runner_names_ = std::move(assets.field_menu_runner_names);
    field_menu_status_bars_ = std::move(assets.field_menu_status_bars);
    field_menu_status_names_ = std::move(assets.field_menu_status_names);
    field_menu_damage_bars_ = std::move(assets.field_menu_damage_bars);
    field_menu_drain_bars_ = std::move(assets.field_menu_drain_bars);
    field_menu_attack_bars_ = std::move(assets.field_menu_attack_bars);
    field_menu_defense_bars_ = std::move(assets.field_menu_defense_bars);
    field_menu_power_labels_ = std::move(assets.field_menu_power_labels);
    field_menu_armor_labels_ = std::move(assets.field_menu_armor_labels);
    intro_ = std::move(assets.intro);
    entity_frames_ = std::move(assets.entity_frames);
    field_entity_animations_ = std::move(assets.field_entity_animations);
    field_direction_table_ = assets.field_direction_table;
    inventory_items_ = std::move(assets.inventory_items);
    cyberware_names_ = std::move(assets.cyberware_names);
    attribute_skill_names_ = std::move(assets.attribute_skill_names);
    protagonist_archetype_records_ =
        std::move(assets.protagonist_archetype_records);
    field_initial_records_ = assets.initial_field_records;
    field_records_ = field_initial_records_;
    field_terrain_ = std::move(assets.field_terrain);
    field_map_width_ = assets.field_map.width;
    field_map_height_ = assets.field_map.height;
    field_map_pixels_ = std::move(assets.field_map.pixels);
    field_areas_ = std::move(assets.field_areas);
    field_event_mode_table_ = assets.field_event_mode_table;

    // These values are the host form of ROM 0x3084/0x2B34's mode and record
    // initialization. Keep the camera and controlled record separate: the
    // mode camera remains a top-left world origin, not a viewport center.
    field_initial_world_x_ = static_cast<std::int32_t>(
        assets.initial_field_records[0].position.world_x_fixed >> 4U);
    field_initial_world_y_ = static_cast<std::int32_t>(
        assets.initial_field_records[0].position.world_y_fixed >> 4U);
    field_initial_world_x_fixed_ =
        assets.initial_field_records[0].position.world_x_fixed;
    field_initial_world_y_fixed_ =
        assets.initial_field_records[0].position.world_y_fixed;
    field_initial_camera_x_ = static_cast<std::int32_t>(
        assets.initial_camera_x_fixed >> 4U);
    field_initial_camera_y_ = static_cast<std::int32_t>(
        assets.initial_camera_y_fixed >> 4U);
    field_world_x_ = field_initial_world_x_;
    field_world_y_ = field_initial_world_y_;
    field_camera_x_ = field_initial_camera_x_;
    field_camera_y_ = field_initial_camera_y_;
    field_entity_records_ = std::move(assets.field_entity_records);
    field_entity_ = std::move(assets.entity_template);
    // Packs written before the per-runner extraction have only the first
    // entity template. Keep those packs runnable while preferring the full
    // source-record array when it is present.
    if (field_entity_records_[0].kind == 0U && field_entity_.kind != 0U) {
        field_entity_records_[0] = field_entity_;
    }
    if (field_entity_records_[0].kind != 0U) {
        field_entity_ = field_entity_records_[0];
    }
    field_initial_entity_records_ = field_entity_records_;
    field_entity_.record_index = 0;
    field_entity_.record_type = field_records_[0].record_type;
    field_entity_.flags_10 = field_records_[0].flags_10;
    field_entity_.flags_11 = field_records_[0].flags_11;
    field_initial_equipped_armor_ = field_entity_.equipped_armor_index;
    for (std::size_t index = 0; index < field_equipped_armor_by_record_.size();
         ++index) {
        field_equipped_armor_by_record_[index] =
            field_initial_entity_records_[index].kind != 0U
                ? field_initial_entity_records_[index].equipped_armor_index
                : field_initial_equipped_armor_;
    }
    field_entity_.world_x_fixed =
        assets.initial_field_records[0].position.world_x_fixed;
    field_entity_.world_y_fixed =
        assets.initial_field_records[0].position.world_y_fixed;
    select_field_entity_idle(0);
    refresh_field_view();
    refresh_field_entity_projection();
    marker_x_ = field_entity_screen_x();
    marker_y_ = field_entity_screen_y();
    refresh_field_collision();
}

void NativePortShell::apply_protagonist_archetype() {
    if (selected_archetype_ >= protagonist_archetype_records_.size()) {
        selected_archetype_ = 0;
    }
    const auto preserved_position = field_entity_records_[0].world_x_fixed != 0 ||
            field_entity_records_[0].world_y_fixed != 0
        ? NativeFieldRecordPosition{
              field_entity_records_[0].world_x_fixed,
              field_entity_records_[0].world_y_fixed}
        : field_initial_records_[0].position;
    field_entity_records_[0] = protagonist_archetype_records_[selected_archetype_];
    field_entity_records_[0].record_index = 0;
    field_entity_records_[0].world_x_fixed = preserved_position.world_x_fixed;
    field_entity_records_[0].world_y_fixed = preserved_position.world_y_fixed;
    field_entity_records_[0].record_type = field_initial_records_[0].record_type;
    field_entity_records_[0].flags_10 = field_initial_records_[0].flags_10;
    field_entity_records_[0].flags_11 = field_initial_records_[0].flags_11;
    field_initial_entity_records_[0] = field_entity_records_[0];
    field_initial_equipped_armor_ =
        field_entity_records_[0].equipped_armor_index;
    field_equipped_armor_by_record_[0] = field_initial_equipped_armor_;
    if (active_field_record_ == 0) {
        field_entity_ = field_entity_records_[0];
    }
}

void NativePortShell::reset_field_state() {
    field_records_ = field_initial_records_;
    field_entity_records_ = field_initial_entity_records_;
    active_field_record_ = 0;
    field_menu_posture_active_ = false;
    field_initial_world_x_ = static_cast<std::int32_t>(
        field_initial_records_[0].position.world_x_fixed >> 4U);
    field_initial_world_y_ = static_cast<std::int32_t>(
        field_initial_records_[0].position.world_y_fixed >> 4U);
    field_initial_world_x_fixed_ =
        field_initial_records_[0].position.world_x_fixed;
    field_initial_world_y_fixed_ =
        field_initial_records_[0].position.world_y_fixed;
    field_world_x_ = field_initial_world_x_;
    field_world_y_ = field_initial_world_y_;
    field_camera_x_ = field_initial_camera_x_;
    field_camera_y_ = field_initial_camera_y_;
    if (field_entity_records_[0].kind != 0U) {
        field_entity_ = field_entity_records_[0];
    }
    field_entity_.record_index = 0;
    field_entity_.record_type = field_records_[0].record_type;
    field_entity_.flags_10 = field_records_[0].flags_10;
    field_entity_.flags_11 = field_records_[0].flags_11;
    field_entity_.world_x_fixed = field_initial_world_x_fixed_;
    field_entity_.world_y_fixed = field_initial_world_y_fixed_;
    for (std::size_t index = 0; index < field_equipped_armor_by_record_.size();
         ++index) {
        field_equipped_armor_by_record_[index] =
            field_initial_entity_records_[index].kind != 0U
                ? field_initial_entity_records_[index].equipped_armor_index
                : field_initial_equipped_armor_;
    }
    field_entity_.equipped_armor_index = field_initial_equipped_armor_;
    field_entity_.frame_index = 0;
    field_entity_.state_selector = 0;
    field_entity_.state_index = 0;
    field_entity_.animation_accumulator = 0;
    select_field_entity_idle(0);
    refresh_field_view();
    refresh_field_entity_projection();
    marker_x_ = field_entity_screen_x();
    marker_y_ = field_entity_screen_y();
    move_rejected_ = false;
    refresh_field_collision();
}

bool NativePortShell::load_field_area(std::uint16_t mode_id) {
    const auto area = std::find_if(
        field_areas_.begin(), field_areas_.end(),
        [mode_id](const NativeFieldArea& candidate) {
            return candidate.mode_id == mode_id;
        });
    if (area == field_areas_.end() || area->map.pixels.empty()) {
        return false;
    }

    field_mode_ = mode_id;
    field_map_width_ = area->map.width;
    field_map_height_ = area->map.height;
    field_map_pixels_ = area->map.pixels;
    field_terrain_ = area->terrain;
    field_initial_records_ = area->initial_field_records;
    field_records_ = field_initial_records_;
    field_initial_world_x_ = static_cast<std::int32_t>(
        field_initial_records_[0].position.world_x_fixed >> 4U);
    field_initial_world_y_ = static_cast<std::int32_t>(
        field_initial_records_[0].position.world_y_fixed >> 4U);
    field_initial_world_x_fixed_ =
        field_initial_records_[0].position.world_x_fixed;
    field_initial_world_y_fixed_ =
        field_initial_records_[0].position.world_y_fixed;
    field_initial_camera_x_ = static_cast<std::int32_t>(
        area->initial_camera_x_fixed >> 4U);
    field_initial_camera_y_ = static_cast<std::int32_t>(
        area->initial_camera_y_fixed >> 4U);
    field_world_x_ = field_initial_world_x_;
    field_world_y_ = field_initial_world_y_;
    field_camera_x_ = field_initial_camera_x_;
    field_camera_y_ = field_initial_camera_y_;
    field_event_mode_table_ = area->event_mode_table;
    active_field_record_ = 0;
    if (field_entity_records_[0].kind != 0U) {
        field_entity_ = field_entity_records_[0];
    }
    field_entity_.record_index = 0;
    field_entity_.record_type = field_records_[0].record_type;
    field_entity_.flags_10 = field_records_[0].flags_10;
    field_entity_.flags_11 = field_records_[0].flags_11;
    field_entity_.world_x_fixed = field_initial_world_x_fixed_;
    field_entity_.world_y_fixed = field_initial_world_y_fixed_;
    field_entity_.frame_index = 0;
    field_entity_.state_selector = 0;
    field_entity_.state_index = 0;
    field_entity_.animation_accumulator = 0;
    select_field_entity_idle(0);
    move_rejected_ = false;
    refresh_field_view();
    refresh_field_entity_projection();
    marker_x_ = field_entity_screen_x();
    marker_y_ = field_entity_screen_y();
    refresh_field_collision();
    return true;
}

void NativePortShell::update_start_options(const InputState& input) {
    if (intro_.start_options.empty()) {
        return;
    }
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step && input.is_held(Button::Up) &&
        !input.is_held(Button::Down)) {
        start_options_selection_ = start_options_selection_ == 0
            ? intro_.start_options.size() - 1U
            : start_options_selection_ - 1U;
    } else if (vertical_step && input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        start_options_selection_ =
            (start_options_selection_ + 1U) % intro_.start_options.size();
    }

    if (!input.was_pressed(Button::A) && !input.was_pressed(Button::B)) {
        return;
    }
    if (start_options_selection_ == 0U) {
        archetype_selection_ = 0;
        start_options_load_rejected_ = false;
        scene_ = NativeScene::ArchetypeMenu;
        return;
    }

    // ROM 0x59172 enters the two-slot screen. The original does not jump
    // directly to an error here; it draws the slots, waits for a slot action,
    // and only then displays the source no-save message when the SRAM check
    // fails.
    save_load_selection_ = 0;
    save_load_message_visible_ = false;
    save_load_message_success_ = false;
    start_options_load_rejected_ = false;
    scene_ = NativeScene::SaveLoad;
}

void NativePortShell::update_save_load(const InputState& input) {
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step && input.is_held(Button::Up) &&
        !input.is_held(Button::Down)) {
        if (save_load_selection_ != 0U) {
            --save_load_selection_;
        }
    } else if (vertical_step && input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        if (save_load_selection_ < 1U) {
            ++save_load_selection_;
        }
    }

    if (input.was_pressed(Button::Start) || input.was_pressed(Button::B)) {
        scene_ = NativeScene::StartOptions;
        save_load_message_visible_ = false;
        return;
    }
    if (!input.was_pressed(Button::A)) {
        return;
    }

    // The native port has not yet recovered a persistent SRAM format. Keep
    // the ROM's visible result for an empty slot instead of treating Load as
    // New Game. The message is drawn from 0x1B6527 at C492.
    save_load_message_visible_ = true;
    save_load_message_success_ = false;
}

void NativePortShell::update_archetype_menu(const InputState& input) {
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step && input.is_held(Button::Up) &&
        !input.is_held(Button::Down)) {
        archetype_selection_ = archetype_selection_ == 0
            ? kNativeArchetypeChoiceCount - 1U
            : archetype_selection_ - 1U;
    } else if (vertical_step && input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        archetype_selection_ =
            (archetype_selection_ + 1U) % kNativeArchetypeChoiceCount;
    }

    if (input.was_pressed(Button::Start)) {
        scene_ = NativeScene::StartOptions;
        return;
    }
    if (!input.was_pressed(Button::A) && !input.was_pressed(Button::B)) {
        return;
    }
    selected_archetype_ = archetype_selection_;
    apply_protagonist_archetype();
    // New Game leaves FFE108 clear.  The main routine therefore calls
    // 0x59C5E after 0x59248/0x42B0, while a successful Load Game sets
    // FFE108 and skips the opening.  Enter the source-timed opening here;
    // this is the native equivalent of that branch, not an extra menu page.
    intro_line_start_ = 0;
    intro_frame_ = 0;
    scene_ = intro_.story_lines.empty() ? NativeScene::Field
                                        : NativeScene::Intro;
    if (scene_ == NativeScene::Field && !field_map_pixels_.empty()) {
        reset_field_state();
    }
}

void NativePortShell::update_intro(const InputState& input) {
    // 0x59C5E's presentation loops abort on the same controller edge that
    // the Genesis input sampler exposes.  Keep that useful skip behavior,
    // but allow the untouched source timeline to play through as well.
    if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
        input.was_pressed(Button::Start)) {
        scene_ = NativeScene::Field;
        if (!field_map_pixels_.empty()) {
            reset_field_state();
        }
        return;
    }

    if (intro_frame_ < kIntroDuration) {
        ++intro_frame_;
    }
    const auto& group = intro_story_group(intro_frame_);
    intro_line_start_ = group.first_line;
    if (intro_frame_ >= kIntroDuration) {
        scene_ = NativeScene::Field;
        if (!field_map_pixels_.empty()) {
            reset_field_state();
        }
    }
}

void NativePortShell::open_stoker_motel_dialog() {
    // The C0 doorway resolves to mode 0x14. The source interaction callback
    // queued at ROM 0x20E48 uses the following text resources at ROM
    // 0x1167EB..0x116A79; keep the original paragraphs intact and let the host
    // writer wrap them to the Genesis dialogue width.
    field_dialog_lines_ = {
        "<The battered sign reads Stoker's Coffin Motel.",
        "Motels like these are cheap, each room only large enough to fit a small bed.",
        "Dumps like these tend to attract the dregs of humanity.>",
        "<Stoker squints at you with his one good eye.",
        "So whadda' ya' want?",
        "I got a room for 20$, up front.",
        "You interested?",
    };
    field_dialog_line_index_ = 0;
    scene_ = NativeScene::FieldDialog;
}

void NativePortShell::update_field_dialog(const InputState& input) {
    if (!input.was_pressed(Button::A) && !input.was_pressed(Button::B) &&
        !input.was_pressed(Button::Start)) {
        return;
    }
    if (field_dialog_line_index_ + 1U < field_dialog_lines_.size()) {
        ++field_dialog_line_index_;
        return;
    }
    field_dialog_lines_.clear();
    field_dialog_line_index_ = 0;
    scene_ = NativeScene::Field;
    field_entity_walking_ = false;
    select_field_entity_idle(field_entity_direction_);
    refresh_field_entity_projection();
}

void NativePortShell::activate_field_record(std::size_t record_index) {
    if (record_index >= field_records_.size() ||
        record_index == active_field_record_) {
        return;
    }

    // ROM 0x391A changes the controlled record, but does not discard the
    // record that was just left. Retain its current fixed-point position in
    // the native party state before selecting the next proven slot.
    field_records_[active_field_record_] = NativeFieldRecord{
        {field_entity_.world_x_fixed, field_entity_.world_y_fixed},
        field_entity_.record_type,
        field_entity_.flags_10,
        field_entity_.flags_11,
    };
    field_equipped_armor_by_record_[active_field_record_] =
        field_entity_.equipped_armor_index;
    field_entity_records_[active_field_record_] = field_entity_;
    active_field_record_ = record_index;
    if (field_entity_records_[record_index].kind != 0U) {
        field_entity_ = field_entity_records_[record_index];
    }
    field_entity_.record_index = static_cast<std::uint8_t>(record_index);
    field_entity_.world_x_fixed =
        field_records_[record_index].position.world_x_fixed;
    field_entity_.world_y_fixed =
        field_records_[record_index].position.world_y_fixed;
    field_entity_.record_type = field_records_[record_index].record_type;
    field_entity_.flags_10 = field_records_[record_index].flags_10;
    field_entity_.flags_11 = field_records_[record_index].flags_11;
    field_entity_.equipped_armor_index =
        field_equipped_armor_by_record_[record_index];
    field_world_x_ = static_cast<std::int32_t>(
        field_entity_.world_x_fixed >> 4U);
    field_world_y_ = static_cast<std::int32_t>(
        field_entity_.world_y_fixed >> 4U);
    field_entity_.frame_index = 0;
    field_entity_.state_selector = 0;
    field_entity_.state_index = 0;
    field_entity_.animation_accumulator = 0;
    select_field_entity_idle(0);
    refresh_field_view();
    refresh_field_entity_projection();
    marker_x_ = field_entity_screen_x();
    marker_y_ = field_entity_screen_y();
    move_rejected_ = false;
    refresh_field_collision();
}

std::size_t NativePortShell::next_available_field_record() const {
    if (field_records_.empty()) {
        return active_field_record_;
    }
    for (std::size_t step = 1; step <= field_records_.size(); ++step) {
        const auto candidate =
            (active_field_record_ + step) % field_records_.size();
        if (runner_switch_candidate(field_records_[candidate])) {
            return candidate;
        }
    }
    // 0x391A can walk to its terminal record without finding a usable
    // runner. The observed menu behavior is a sound-only no-op: retain the
    // current controlled record and all of its position/state.
    return active_field_record_;
}

void NativePortShell::refresh_field_view() {
    if (field_map_pixels_.empty() || field_map_width_ == 0 ||
        field_map_height_ == 0) {
        return;
    }
    const auto max_viewport_x = static_cast<std::int32_t>(
        field_map_width_) - kFieldViewportWidth;
    const auto max_viewport_y = static_cast<std::int32_t>(
        field_map_height_) - kFieldViewportHeight;
    const auto viewport_x = std::clamp(
        field_camera_x_, 0, max_viewport_x);
    const auto viewport_y = std::clamp(
        field_camera_y_, 0, max_viewport_y);
    field_view_width_ = static_cast<std::uint16_t>(kFieldViewportWidth);
    field_view_height_ = static_cast<std::uint16_t>(kFieldViewportHeight);
    field_view_pixels_.resize(static_cast<std::size_t>(
        kFieldViewportWidth * kFieldViewportHeight));
    for (std::int32_t y = 0; y < kFieldViewportHeight; ++y) {
        for (std::int32_t x = 0; x < kFieldViewportWidth; ++x) {
            field_view_pixels_[static_cast<std::size_t>(y) * kFieldViewportWidth +
                               x] = field_map_pixels_[
                static_cast<std::size_t>(viewport_y + y) * field_map_width_ +
                viewport_x + x];
        }
    }
}

void NativePortShell::refresh_field_entity_projection() {
    if (entity_frames_.empty() || field_map_pixels_.empty()) {
        field_entity_.visible = false;
        field_entity_.screen_x = 0;
        field_entity_.screen_y = 0;
        return;
    }

    // The field cursor is the native control anchor. The small world offsets
    // place an entity's feet at that anchor; projection itself is the
    // reviewed ROM 0x5A62 fixed-point transform.
    const shadowrun::EntityProjectionInput entity{
        field_entity_.world_x_fixed,
        field_entity_.world_y_fixed,
        field_entity_.projection_word_08,
    };
    const auto result = shadowrun::project_entity_field(
        entity, fixed_world_coordinate(field_camera_x_),
        fixed_world_coordinate(field_camera_y_));
    field_entity_.vdp_screen_x = result.screen_x;
    field_entity_.vdp_screen_y = result.screen_y;
    // Genesis sprite coordinates are encoded relative to 0x80. The native
    // screen writer consumes ordinary top-left-origin pixels, so this is the
    // explicit hardware-boundary adaptation of ROM 0x5A62's result.
    field_entity_.screen_x = static_cast<std::uint16_t>(
        result.screen_x - kGenesisVdpSpriteOrigin);
    field_entity_.screen_y = static_cast<std::uint16_t>(
        result.screen_y - kGenesisVdpSpriteOrigin);
    // ROM 0x36FC uses +0x56/+0x57 while choosing the separately rendered
    // equipped-item sprite and palette. It does not remove the runner body
    // when +0x56 becomes 0x7F (no weapon). Keep item display state separate
    // from the projected body visibility so unequipping cannot replace the
    // character with the native fallback marker.
    field_entity_.visible = result.visible;
}

void NativePortShell::refresh_field_collision() {
    if (field_terrain_.statuses.empty()) {
        field_collision_ = {};
        return;
    }
    field_collision_ = terrain::probe(
        field_terrain_, field_entity_.world_x_fixed,
        field_entity_.world_y_fixed, true);
}

void NativePortShell::update_field_entity_direction(
    std::int32_t delta_x, std::int32_t delta_y) {
    const auto has_animation_frames = std::any_of(
        field_entity_animations_.begin(), field_entity_animations_.end(),
        [](const NativeFieldAnimation& animation) {
            return !animation.frames.empty();
        });
    if ((delta_x == 0 && delta_y == 0) ||
        (entity_frames_.empty() && !has_animation_frames)) {
        return;
    }

    // ROM 0xBABC probes the eight signed vectors at 0xC002. Their ordinal is
    // also the byte-addressed state-table selector consumed by 0x600E, so
    // match the host input vector against the extracted table rather than
    // duplicating its directional order in the frontend.
    const auto sign = [](std::int32_t value) {
        return value < 0 ? -1 : value > 0 ? 1 : 0;
    };
    const auto desired_x = sign(delta_x);
    const auto desired_y = sign(delta_y);
    std::size_t direction = 0;
    for (std::size_t index = 0; index < field_direction_table_.size();
         ++index) {
        const auto& vector = field_direction_table_[index];
        if (sign(vector[0]) == desired_x && sign(vector[1]) == desired_y) {
            direction = index;
            break;
        }
    }
    field_entity_direction_ = direction;
}

void NativePortShell::select_field_entity_idle(std::size_t direction) {
    field_entity_direction_ = direction % field_direction_table_.size();
    field_entity_walking_ = false;
    field_entity_.frame_index = entity_frames_.empty()
        ? 0U
        : field_entity_direction_ % entity_frames_.size();
    // The first kind-3 pointer table at 0x6378 contains the eight one-frame
    // directional states. Its startup selector is zero; the direction is
    // represented by the frame chosen from that table, not by a walking
    // selector such as 0x60 or 0x68.
    field_entity_.state_selector = 0;
    field_entity_.state_index = 0;
    field_entity_.animation_accumulator = 0;
}

void NativePortShell::select_field_entity_animation(std::size_t direction) {
    field_entity_direction_ = direction % field_direction_table_.size();
    field_entity_walking_ = true;
    if (direction < field_entity_animations_.size() &&
        !field_entity_animations_[direction].frames.empty()) {
        const auto& animation = field_entity_animations_[direction];
        field_entity_.frame_index = 0;
        field_entity_.state_selector = animation.state_selector;
        field_entity_.state_index = 0;
        field_entity_.animation_accumulator = 0;
        // ROM 0x600E resolves the active state list and writes its timing
        // classification to record +0x18. For Joshua's kind-3 walking lists
        // the source timing word is 0x76F0, which classifies to 0x0010. The
        // neighboring 0x0300 word is list metadata, not the frame cadence.
        field_entity_.animation_multiplier =
            std::max<std::uint16_t>(animation.timing_multiplier, 1U);
        if (field_entity_.animation_threshold == 0U) {
            field_entity_.animation_threshold = 0x1000U;
        }
        field_entity_.animation_descriptor = 0;
        return;
    }
    if (entity_frames_.empty()) {
        field_entity_.frame_index = 0;
        field_entity_.state_selector = static_cast<std::uint16_t>(
            direction * 4U);
        field_entity_.state_index = 0;
        field_entity_.animation_accumulator = 0;
        return;
    }
    // Packs written before version 10 only have the old one-frame-per-
    // direction table. Keep that data usable while the current extractor
    // supplies the complete walking lists.
    field_entity_.frame_index = direction % entity_frames_.size();
    field_entity_.state_selector = static_cast<std::uint16_t>(direction * 4U);
    field_entity_.state_index = 0;
    field_entity_.animation_accumulator = 0;
}

void NativePortShell::update_field_entity_animation(bool movement_attempted) {
    if (!movement_attempted || !field_entity_walking_) {
        return;
    }
    const auto animation = std::find_if(
        field_entity_animations_.begin(), field_entity_animations_.end(),
        [this](const NativeFieldAnimation& candidate) {
            return candidate.state_selector == field_entity_.state_selector;
        });
    if (animation == field_entity_animations_.end() ||
        animation->frames.size() < 2U) {
        return;
    }

    // This is the direct host form of ROM 0x5FDC: +0x18 advances +0x1A and
    // a frame changes only when +0x1C is reached. For the kind-3 walking
    // lists the source values are timing_multiplier=0x0010 and
    // threshold=0x1000. Keep the 16-bit accumulator and threshold behavior
    // of 0x5FDC instead of converting the source cadence to a host delay.
    const auto threshold = std::max<std::uint16_t>(
        field_entity_.animation_threshold, 1U);
    const auto multiplier = std::max<std::uint16_t>(
        field_entity_.animation_multiplier, 1U);
    const auto accumulated = static_cast<std::uint32_t>(
        field_entity_.animation_accumulator) + multiplier;
    if (accumulated < threshold) {
        field_entity_.animation_accumulator =
            static_cast<std::uint16_t>(accumulated);
        return;
    }
    field_entity_.animation_accumulator = static_cast<std::uint16_t>(
        accumulated - threshold);
    field_entity_.state_index = static_cast<std::uint16_t>(
        (field_entity_.state_index + 1U) % animation->frames.size());
    field_entity_.frame_index = field_entity_.state_index;
}

void NativePortShell::open_field_menu() {
    if (field_map_pixels_.empty()) {
        return;
    }

    // ROM 0x10F50 initializes FFF0D0 to six and FFF0D1 to zero before
    // entering the command-screen loop. Opening the native state preserves
    // the field record, camera, and collision result instead of restarting
    // the map.
    scene_ = NativeScene::FieldMenu;
    field_menu_selection_ = 6;
    field_menu_row_ = 0;
    field_menu_posture_active_ = false;
    field_last_command_selection_ = 0;
    field_last_command_row_ = 0;
    field_command_message_frames_ = 0;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

bool NativePortShell::menu_direction_step(const InputState& input,
                                          Button negative, Button positive,
                                          bool horizontal) {
    const auto negative_held = input.is_held(negative);
    const auto positive_held = input.is_held(positive);
    const auto direction_mask = negative_held && !positive_held
        ? button_mask(negative)
        : positive_held && !negative_held ? button_mask(positive) : 0U;
    auto& repeat_frames = horizontal ? menu_horizontal_repeat_frames_
                                     : menu_vertical_repeat_frames_;
    auto& repeat_mask = horizontal ? menu_horizontal_repeat_mask_
                                   : menu_vertical_repeat_mask_;
    if (direction_mask == 0U) {
        repeat_frames = 0;
        repeat_mask = 0;
        return false;
    }
    if (direction_mask != repeat_mask) {
        repeat_mask = direction_mask;
        repeat_frames = 0;
        return true;
    }
    ++repeat_frames;
    // The ROM's command loop samples input once per VBlank and only changes
    // the selector on its input gate. Adapt that gate to the host update
    // cadence: one edge, a short initial delay, then a deliberate repeat.
    constexpr std::uint16_t kInitialDelay = 12U;
    constexpr std::uint16_t kRepeatPeriod = 4U;
    return repeat_frames >= kInitialDelay &&
           ((repeat_frames - kInitialDelay) % kRepeatPeriod) == 0U;
}

void NativePortShell::write_field_menu_text(ScreenWriter& screen,
                                            std::int32_t x, std::int32_t y,
                                            std::string_view text) const {
    if (field_menu_font_pixels_.size() !=
        static_cast<std::size_t>(field_menu_font_width_) *
            field_menu_font_height_ ||
        field_menu_font_width_ < 8U || field_menu_font_height_ < 8U) {
        screen.write_text(x, y, text, 1, {0xE8, 0xF4, 0xFF, 0xFF});
        return;
    }
    auto cursor = x;
    for (const auto character : text) {
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph_index = static_cast<std::size_t>(
            code - 0x20U);
        const auto glyph_column = glyph_index % 16U;
        const auto glyph_row = glyph_index / 16U;
        if ((glyph_row + 1U) * 8U > field_menu_font_height_) {
            cursor += 8;
            continue;
        }
        std::vector<Rgba8> glyph(64U);
        auto glyph_has_ink = false;
        for (std::size_t row = 0; row < 8U; ++row) {
            for (std::size_t column = 0; column < 8U; ++column) {
                glyph[row * 8U + column] = field_menu_font_pixels_[
                    (glyph_row * 8U + row) * field_menu_font_width_ +
                    glyph_column * 8U + column];
                glyph_has_ink = glyph_has_ink ||
                    glyph[row * 8U + column].alpha != 0U;
            }
        }
        if (glyph_has_ink) {
            screen.blit_rgba(cursor, y, 8, 8, glyph);
        } else if (character != ' ') {
            // Keep the extracted tile as the first choice. If a host asset
            // pack was produced before the command-font atlas was decoded,
            // retain the source text destination instead of silently losing
            // every command label.
            screen.write_text(cursor, y, std::string(1, character), 1,
                              {0xA0, 0xE8, 0xF8, 0xFF});
        }
        cursor += 8;
    }
}

void NativePortShell::write_startup_text(ScreenWriter& screen,
                                         std::int32_t x, std::int32_t y,
                                         std::string_view text) const {
    if (startup_font_.pixels.size() !=
        static_cast<std::size_t>(startup_font_.width) * startup_font_.height ||
        startup_font_.width < 8U || startup_font_.height < 8U) {
        screen.write_text(x, y, text, 1, {0xFF, 0xFF, 0xFF, 0xFF});
        return;
    }
    auto cursor = x;
    for (const auto character : text) {
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph_index = static_cast<std::size_t>(code - 0x20U);
        const auto glyph_column = glyph_index % 16U;
        const auto glyph_row = glyph_index / 16U;
        if ((glyph_row + 1U) * 8U > startup_font_.height) {
            cursor += 8;
            continue;
        }
        std::vector<Rgba8> glyph(64U);
        for (std::size_t row = 0; row < 8U; ++row) {
            for (std::size_t column = 0; column < 8U; ++column) {
                glyph[row * 8U + column] = startup_font_.pixels[
                    (glyph_row * 8U + row) * startup_font_.width +
                    glyph_column * 8U + column];
            }
        }
        screen.blit_rgba(cursor, y, 8U, 8U, glyph);
        cursor += 8;
    }
}

void NativePortShell::render_startup_background(ScreenWriter& screen) const {
    // 0x59092/0x59172/0x59248 enter their own cleared startup planes. The
    // title circuit plane is not retained behind these menus; blitting it
    // here made the create-character screen visibly unlike the source.
    screen.clear({0x00, 0x00, 0x00, 0xFF});
}

void NativePortShell::update_field_menu(const InputState& input) {
    // ROM 0x119CE replaces the command selector with a dedicated posture
    // loop. Left/right mutate record +0x4D through 0x11AE4; they do not move
    // the command selector while that loop is active.
    if (field_menu_posture_active_) {
        if (input.was_pressed(Button::Start)) {
            field_menu_posture_active_ = false;
            scene_ = NativeScene::Field;
            return;
        }
        if (input.was_pressed(Button::B)) {
            field_menu_posture_active_ = false;
            return;
        }
        // Posture is a focus inside the command overview, not a modal screen.
        // The source returns to the command selector when Down is pressed.
        if (input.was_pressed(Button::Down)) {
            field_menu_posture_active_ = false;
            menu_vertical_repeat_frames_ = 0;
            menu_vertical_repeat_mask_ = 0;
            return;
        }
        const auto horizontal_step =
            menu_direction_step(input, Button::Left, Button::Right, true);
        if (horizontal_step && input.is_held(Button::Left) &&
            !input.is_held(Button::Right)) {
            if (field_entity_.menu_status_index != 0U) {
                --field_entity_.menu_status_index;
            }
        } else if (horizontal_step && input.is_held(Button::Right) &&
                   !input.is_held(Button::Left)) {
            if (field_entity_.menu_status_index + 1U <
                field_menu_status_bars_.size()) {
                ++field_entity_.menu_status_index;
            }
        }
        // ROM 0x11ABE redraws the command overview through 0x12CEE after it
        // mutates +0x4D. 12CEE does not apply a generic posture multiplier to
        // the existing bars: it recomputes the source attack/defense branch
        // from the equipped item, skills, cyberware, and item selector.
        const auto posture = static_cast<std::uint8_t>(std::min<std::size_t>(
            field_entity_.menu_status_index,
            kNativeMenuStatusCount - 1U));
        const auto bars = compute_command_bar_indices(
            field_entity_, inventory_items_, posture,
            static_cast<std::uint8_t>(field_menu_selection_),
            static_cast<std::uint8_t>(field_menu_row_));
        field_entity_.attack_bar_index = bars.attack;
        field_entity_.defense_bar_index = bars.defense;
        field_entity_.attack_bar_indices_by_posture[posture] = bars.attack;
        field_entity_.defense_bar_indices_by_posture[posture] = bars.defense;
        field_entity_records_[active_field_record_] = field_entity_;
        return;
    }

    // 0x114A0 changes FFF0D0 for left/right. It wraps the seven native
    // selector values with the same 6 -> 0 and 0 -> 6 behavior.
    const auto cycle_selection = [this](std::int32_t delta) {
        constexpr auto count = kFieldMenuLabels.size();
        const auto current = static_cast<std::int32_t>(field_menu_selection_);
        const auto next = (current + delta + static_cast<std::int32_t>(count)) %
                          static_cast<std::int32_t>(count);
        field_menu_selection_ = static_cast<std::size_t>(next);
    };
    const auto horizontal_step =
        menu_direction_step(input, Button::Left, Button::Right, true);
    if (horizontal_step &&
        input.is_held(Button::Left) && !input.is_held(Button::Right)) {
        cycle_selection(-1);
    } else if (horizontal_step &&
               input.is_held(Button::Right) &&
               !input.is_held(Button::Left)) {
        cycle_selection(1);
    }

    // 0x11506 sends selectors 2..6 into the posture loop. Only the two
    // Inventory columns own the four-row secondary selector.
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (field_menu_selection_ >= 2U && vertical_step &&
        input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        field_menu_posture_active_ = true;
        menu_horizontal_repeat_frames_ = 0;
        menu_horizontal_repeat_mask_ = 0;
    } else if (field_menu_selection_ < 2U && vertical_step &&
        input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        if (field_menu_row_ != 0U) {
            --field_menu_row_;
        }
    } else if (field_menu_selection_ < 2U && vertical_step &&
               input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        if (field_menu_row_ < 3U) {
            ++field_menu_row_;
        }
    }

    // Both A (bit 14) and B (bit 12) enter the same 0x10E4E dispatch path in
    // the reviewed ROM. Keep the selected command explicit until its typed
    // subsystem replaces this dispatch record.
    if (input.was_pressed(Button::A) || input.was_pressed(Button::B)) {
        field_last_command_selection_ = field_menu_selection_;
        field_last_command_row_ = field_menu_row_;
        if (field_menu_selection_ < 2U) {
            // ROM 0x12148 supplies the item byte to 0x12168. The armor branch
            // at 0x1219C/0x121B0 toggles record +0x76 and stores the
            // one-based armor ordinal in the selected slot's +0x6E byte.
            apply_field_inventory_action();
        } else if (field_menu_selection_ == 2U) {
            open_clips_menu();
        } else if (field_menu_selection_ == 4U) {
            open_cyberware_menu();
        } else if (field_menu_selection_ == 3U) {
            open_attributes_menu();
        }
        field_command_message_frames_ = 30;
        ++field_command_count_;
    }

    // C enters 0x391A, switches the controlled field record, and restores
    // FFF0D0 to six before returning to the command loop.
    if (input.was_pressed(Button::C) && !field_records_.empty()) {
        ++runner_switch_attempts_;
        const auto previous_record = active_field_record_;
        activate_field_record(next_available_field_record());
        if (active_field_record_ == previous_record) {
            ++runner_switch_rejections_;
        }
        field_menu_selection_ = 6;
        field_menu_row_ = 0;
        ++action_count_;
    }

    // The Start edge exits the command-screen loop. It does not reset the
    // selected field record or its camera.
    if (input.was_pressed(Button::Start)) {
        scene_ = NativeScene::Field;
        field_command_message_frames_ = 0;
    }

    if (field_command_message_frames_ != 0) {
        --field_command_message_frames_;
    }
}

void NativePortShell::apply_field_inventory_action() {
    const auto slot = field_menu_row_ * 2U + field_menu_selection_;
    if (slot >= field_entity_.inventory_item_codes.size()) {
        return;
    }
    const auto item_code = field_entity_.inventory_item_codes[slot];
    // Empty +0x66 cells are not actionable inventory records. The source
    // dispatch returns through its no-item path; they do not become a weapon
    // with item code zero and must not disturb +0x4E ammunition.
    if (item_code == 0U) {
        return;
    }
    // ROM 0x1219C enters this branch for armor item codes 0x2C and above;
    // 0x121B4 stores code-0x2B in +0x76 and the selected +0x6E byte. A second
    // activation clears both, matching the ROM toggle without pretending the
    // unrelated weapon/medical handlers are already ported.
    if (item_code >= 0x2CU) {
        // ROM 0x1219C is the armor branch. It deliberately tests the
        // currently equipped armor field rather than treating possession as
        // equipment: an equipped armor value clears first; an empty value
        // equips the selected armor ordinal and writes that ordinal into the
        // selected slot's +0x6E phase byte.
        if (field_entity_.equipped_armor_index != 0U) {
            field_entity_.equipped_armor_index = 0;
            field_entity_.inventory_phase_bytes[slot] = 0;
        } else {
            field_entity_.equipped_armor_index =
                static_cast<std::uint8_t>(item_code - 0x2BU);
            field_entity_.inventory_phase_bytes[slot] =
                field_entity_.equipped_armor_index;
        }
    } else if (item_code == 0x13U || item_code == 0x14U ||
               item_code == 0x16U ||
               item_code >= 0x18U) {
        // ROM 0x121C0 dispatches these item classes to their own use/error
        // path. It does not equip them as a weapon. Those handlers are not
        // silently replaced with a native mutation here.
        return;
    } else {
        // ROM 0x121E0..0x1228C is the weapon/gear equip path. +0x56 stores
        // the selected inventory slot and +0x57 stores the equipped item;
        // the eight +0x66 bytes are merely possession slots.
        const auto old_slot = field_entity_.display_state;
        const auto old_item = field_entity_.active_item_code;
        auto source_units = static_cast<unsigned>(field_entity_.ammo_rounds);
        if (old_item != 0U && old_item < 0x0DU) {
            source_units = source_store_ammunition_units(
                source_units, inventory_items_, old_item);
        }
        field_entity_.ammo_rounds = 0;
        if (old_slot >= 0 && old_slot != 0x7F &&
            static_cast<std::size_t>(old_slot) == slot) {
            // Selecting the equipped cell again sets +0x56=0x7F and clears
            // +0x57 in the ROM. The record is no longer weapon-equipped.
            field_entity_.display_state = 0x7F;
            field_entity_.display_substate = 0;
            field_entity_.active_item_code = 0;
            field_entity_.ammo_rounds =
                static_cast<std::uint8_t>(source_units & 0xFFU);
        } else {
            field_entity_.display_state = static_cast<std::int8_t>(slot);
            field_entity_.display_substate =
                static_cast<std::int8_t>(item_code);
            field_entity_.active_item_code = item_code;
            if (item_code >= 0x0DU) {
                // The source carries raw +0x4E through non-ammunition gear.
                field_entity_.ammo_rounds =
                    static_cast<std::uint8_t>(source_units & 0xFFU);
            } else {
                field_entity_.ammo_rounds = source_load_ammunition(
                    source_units, inventory_items_, item_code);
            }
        }
        const auto bars = compute_command_bar_indices(
            field_entity_, inventory_items_, field_entity_.menu_status_index,
            static_cast<std::uint8_t>(field_menu_selection_),
            static_cast<std::uint8_t>(field_menu_row_));
        field_entity_.attack_bar_index = bars.attack;
        field_entity_.defense_bar_index = bars.defense;
        field_entity_.attack_bar_indices_by_posture[
            field_entity_.menu_status_index] = bars.attack;
        field_entity_.defense_bar_indices_by_posture[
            field_entity_.menu_status_index] = bars.defense;
        refresh_field_entity_projection();
    }
    field_equipped_armor_by_record_[active_field_record_] =
        field_entity_.equipped_armor_index;
    field_entity_records_[active_field_record_] = field_entity_;
}

void NativePortShell::open_clips_menu() {
    // ROM 0x11B58 saves the field selector, then starts its own selector at
    // two before drawing Increase Clips. Keep the parent FieldMenu selection
    // intact so Start returns to the same command entry.
    scene_ = NativeScene::ClipsMenu;
    clips_menu_selection_ = 2;
    clips_last_action_selection_ = 2;
    clips_action_message_frames_ = 0;
    clips_out_of_ammo_ = false;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::update_clips_menu(const InputState& input) {
    // ROM 0x118C6 applies the left/right delta and wraps 2 -> 5 and 5 -> 2.
    const auto horizontal_step =
        menu_direction_step(input, Button::Left, Button::Right, true);
    if (horizontal_step &&
        input.is_held(Button::Left) && !input.is_held(Button::Right)) {
        clips_menu_selection_ = clips_menu_selection_ == 2U
            ? 5U : clips_menu_selection_ - 1U;
    } else if (horizontal_step &&
               input.is_held(Button::Right) &&
               !input.is_held(Button::Left)) {
        clips_menu_selection_ = clips_menu_selection_ == 5U
            ? 2U : clips_menu_selection_ + 1U;
    }

    // 0x12062 gates every A/B/C action on the active record's +0x4F clip
    // count. The selected reload/trade handlers are kept as a typed dispatch
    // record until their shared quantity-transfer path is ported.
    if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
        input.was_pressed(Button::C)) {
        clips_last_action_selection_ = clips_menu_selection_;
        clips_out_of_ammo_ = field_entity_.ammo_clips == 0;
        clips_action_message_frames_ = 30;
        if (!clips_out_of_ammo_) {
            ++clips_action_count_;
        }
    }

    // The ROM's Start edge restores the saved top-level selector and returns
    // to the command-screen loop. It does not reset the active field record.
    if (input.was_pressed(Button::Start)) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 2;
        field_command_message_frames_ = 0;
        clips_action_message_frames_ = 0;
    }

    if (clips_action_message_frames_ != 0) {
        --clips_action_message_frames_;
    }
}

void NativePortShell::open_cyberware_menu() {
    // ROM 0x11950 is a display-only handler. It reads +0x90, emits one line
    // for each installed bit, then waits for the controller before returning
    // to the command screen. Keep the active record and its field state intact.
    scene_ = NativeScene::CyberwareMenu;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::update_cyberware_menu(const InputState& input) {
    // The original handler has no selection or mutation loop. Start is the
    // only exit edge at this boundary; arrows and action buttons are ignored.
    if (input.was_pressed(Button::Start)) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 4;
        field_command_message_frames_ = 0;
    }
}

void NativePortShell::open_attributes_menu() {
    // ROM 0xD01C initializes FFF0D0 to zero before entering the attribute /
    // skill display loop. FFF0D1 is the ten-row cursor used by that loop.
    scene_ = NativeScene::AttributesMenu;
    attributes_menu_column_ = 0;
    attributes_menu_row_ = 0;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::update_attributes_menu(const InputState& input) {
    // ROM 0xD27C keeps the first selector in the two-column range 0..1.
    const auto horizontal_step =
        menu_direction_step(input, Button::Left, Button::Right, true);
    if (horizontal_step &&
        input.is_held(Button::Left) && !input.is_held(Button::Right)) {
        attributes_menu_column_ = 0;
    } else if (horizontal_step &&
               input.is_held(Button::Right) &&
               !input.is_held(Button::Left)) {
        attributes_menu_column_ = 1;
    }
    // ROM 0xD2DA/0xD302 clamps the secondary selector to rows 0..9. The
    // native view uses the same two-column, ten-row presentation boundary.
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step &&
        input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        if (attributes_menu_row_ != 0) {
            --attributes_menu_row_;
        }
    } else if (vertical_step &&
               input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        if (attributes_menu_row_ < 9U) {
            ++attributes_menu_row_;
        }
    }

    // The ROM's A/B/C update/distribution branches mutate unresolved points
    // and are deliberately not guessed here. Start is the proven display
    // loop exit and returns to the same top-level command selector.
    if (input.was_pressed(Button::Start)) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 3;
        field_command_message_frames_ = 0;
    }
}

void NativePortShell::update(const InputState& input) {
    input_ = input;

    if (scene_ == NativeScene::Title) {
        // ROM 0x0BF2 records Start as a pressed high-byte input and the
        // 0x1A9E loop exits when FFDDC2 is set. An early edge finishes the
        // title presentation; it does not jump directly to the game field.
        if (input.was_pressed(Button::Start)) {
            if (!title_skip_requested_) {
                title_skip_requested_ = true;
                constexpr std::uint64_t kTitleSettledLoop = 0xE10U;
                frame_number_ = std::max(frame_number_,
                                         kTitleSettledLoop + 1U);
            } else {
                // ROM 0x59092 enters the startup-options block. New Game
                // later enters 0x59C5E; Load Game skips it only after a valid
                // save sets FFE108.
                scene_ = has_startup_menus() ? NativeScene::StartOptions
                                             : NativeScene::Field;
                if (scene_ == NativeScene::Field) {
                    if (!field_map_pixels_.empty()) {
                        reset_field_state();
                    } else {
                        marker_x_ = kScreenWidth / 2;
                        marker_y_ = kScreenHeight / 2;
                    }
                }
            }
        }
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::StartOptions) {
        update_start_options(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::SaveLoad) {
        update_save_load(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::ArchetypeMenu) {
        update_archetype_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::Intro) {
        update_intro(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::FieldDialog) {
        update_field_dialog(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::FieldMenu) {
        update_field_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::ClipsMenu) {
        update_clips_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::CyberwareMenu) {
        update_cyberware_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::AttributesMenu) {
        update_attributes_menu(input);
        ++frame_number_;
        return;
    }

    constexpr std::int32_t kSpeed = 2;
    auto delta_x = 0;
    auto delta_y = 0;
    if (input.is_held(Button::Up)) {
        delta_y -= kSpeed;
    }
    if (input.is_held(Button::Down)) {
        delta_y += kSpeed;
    }
    if (input.is_held(Button::Left)) {
        delta_x -= kSpeed;
    }
    if (input.is_held(Button::Right)) {
        delta_x += kSpeed;
    }
    const auto attempted_movement = delta_x != 0 || delta_y != 0;
    const auto requested_delta_x = delta_x;
    const auto requested_delta_y = delta_y;
    if (input.was_pressed(Button::Start)) {
        action_flash_frames_ = 0;
        if (!field_map_pixels_.empty()) {
            open_field_menu();
            ++frame_number_;
            return;
        } else {
            marker_x_ = kScreenWidth / 2;
            marker_y_ = kScreenHeight / 2;
        }
    }
    if (input.was_pressed(Button::A)) {
        marker_color_ = {0x00, 0xD8, 0xFF, 0xFF};
        action_flash_frames_ = 8;
        ++action_count_;
    }
    if (input.was_pressed(Button::B)) {
        marker_color_ = {0xFF, 0x40, 0x70, 0xFF};
        action_flash_frames_ = 8;
        ++action_count_;
    }
    move_rejected_ = false;
    if (!field_map_pixels_.empty() &&
        (delta_x != 0 || delta_y != 0)) {
        const auto candidate_x = static_cast<std::uint16_t>(
            static_cast<std::int32_t>(field_entity_.world_x_fixed) +
            delta_x * 16);
        const auto candidate_y = static_cast<std::uint16_t>(
            static_cast<std::int32_t>(field_entity_.world_y_fixed) +
            delta_y * 16);
        const auto candidate_collision = terrain::probe(
            field_terrain_, candidate_x, candidate_y, true);
        field_collision_ = candidate_collision;
        if (candidate_collision.blocked) {
            move_rejected_ = true;
            // ROM 0xB7B8 retains the raw terrain byte even though 0xE7C8
            // returns 0x7F for the blocked move. The active descriptor's
            // +0x4E reaction-mode table is consumed by the later 0x167AE
            // transition path after 0xB7B8 copies the low five status bits
            // into FFFBF9. The host combines those already-decoded steps at
            // the native boundary.
            const auto event_index = static_cast<std::size_t>(
                candidate_collision.raw_status & 0x1FU);
            // ROM 0xE7C8/0xE8A0 chooses the highest-status cell from the
            // four-corner footprint and 0xB7B8 retains that selected raw
            // byte. It does not replace a wall corner with a second center
            // probe. That shortcut sent the Coffin Motel doorway through a
            // neighboring event cell and could select mode 0x1A instead of
            // the source event-0 mode 0x14 destination.
            if ((candidate_collision.raw_status & 0xE0U) == 0xC0U &&
                event_index < field_event_mode_table_.size()) {
                const auto source_mode = field_mode_;
                const auto destination = field_event_mode_table_[event_index];
                if (destination != 0U && destination != 0x80U &&
                    load_field_area(destination)) {
                    ++field_area_transition_count_;
                    action_flash_frames_ = 0;
                    delta_x = 0;
                    delta_y = 0;
                    // This is the reviewed Redmond event tuple: mode 2,
                    // raw C0/event 0, reaction table destination 0x14. The
                    // source does not turn this into the unrelated mode-26
                    // security interior; its 0x20E48 callback enters the
                    // motel NPC/dialogue state after the area load.
                    if (source_mode == 2U && field_mode_ == 0x14U &&
                        event_index == 0U &&
                        candidate_collision.raw_status == 0xC0U) {
                        open_stoker_motel_dialog();
                    }
                } else {
                    delta_x = 0;
                    delta_y = 0;
                }
            } else {
                delta_x = 0;
                delta_y = 0;
            }
        } else {
            field_entity_.world_x_fixed = candidate_x;
            field_entity_.world_y_fixed = candidate_y;
            field_world_x_ = static_cast<std::int32_t>(candidate_x >> 4U);
            field_world_y_ = static_cast<std::int32_t>(candidate_y >> 4U);
        }
    } else if (field_map_pixels_.empty()) {
        marker_x_ += delta_x;
        marker_y_ += delta_y;
        marker_x_ = std::clamp(marker_x_, 8,
                               static_cast<std::int32_t>(kScreenWidth) - 16);
        marker_y_ = std::clamp(marker_y_, 48,
                               static_cast<std::int32_t>(kScreenHeight) - 16);
    }
    const auto movement_accepted =
        !field_map_pixels_.empty() && attempted_movement &&
        !move_rejected_ && (delta_x != 0 || delta_y != 0);
    if (attempted_movement) {
        update_field_entity_direction(requested_delta_x, requested_delta_y);
        if (movement_accepted) {
            const auto animation = std::find_if(
                field_entity_animations_.begin(),
                field_entity_animations_.end(),
                [this](const NativeFieldAnimation& candidate) {
                    return candidate.state_selector ==
                               static_cast<std::uint16_t>(
                                   0x60U + field_entity_direction_ * 4U);
                });
            const auto direction_changed =
                field_entity_.state_selector !=
                static_cast<std::uint16_t>(0x60U +
                                           field_entity_direction_ * 4U);
            if (!field_entity_walking_ || direction_changed) {
                select_field_entity_animation(field_entity_direction_);
            } else if (animation == field_entity_animations_.end()) {
                field_entity_walking_ = false;
                select_field_entity_idle(field_entity_direction_);
            }
        } else {
            select_field_entity_idle(field_entity_direction_);
        }
    } else if (field_entity_walking_) {
        // The ROM's movement state clears when no directional input reaches
        // the accepted-step path. Return to the directional idle table rather
        // than leaving the last walking frame onscreen.
        select_field_entity_idle(field_entity_direction_);
    }
    update_field_entity_animation(movement_accepted);
    if (!field_map_pixels_.empty()) {
        // The direct 0x5A62 result is a VDP coordinate. Keep the decoded map
        // in the same top-left world space and shift the camera only after
        // the entity would leave the native viewport.
        const auto entity_x = static_cast<std::int32_t>(
            field_entity_.world_x_fixed >> 4U);
        const auto entity_y = static_cast<std::int32_t>(
            field_entity_.world_y_fixed >> 4U);
        const auto screen_x = entity_x - field_camera_x_;
        const auto screen_y = entity_y - field_camera_y_;
        if (screen_x < 0) {
            field_camera_x_ = entity_x;
        } else if (screen_x > kFieldEntityMaxHostX) {
            field_camera_x_ = entity_x - kFieldEntityMaxHostX;
        }
        if (screen_y < 0) {
            field_camera_y_ = entity_y;
        } else if (screen_y > kFieldViewportHeight) {
            field_camera_y_ = entity_y - kFieldViewportHeight;
        }
        refresh_field_view();
        refresh_field_entity_projection();
        marker_x_ = field_entity_screen_x();
        marker_y_ = field_entity_screen_y();
        if (!attempted_movement) {
            refresh_field_collision();
        }
        field_records_[active_field_record_] = NativeFieldRecord{
            {field_entity_.world_x_fixed, field_entity_.world_y_fixed},
            field_entity_.record_type,
            field_entity_.flags_10,
            field_entity_.flags_11,
        };
        field_entity_records_[active_field_record_] = field_entity_;
    }
    if (input.was_pressed(Button::C) && !field_map_pixels_.empty()) {
        // The main field loop dispatches raw 0x2000 to 0x391A after its
        // ordinary entity pass. Raw 0x2000 is the Genesis C edge; on a
        // three-record startup party this selects the next controlled record
        // in the same order as the ROM loop.
        ++runner_switch_attempts_;
        const auto previous_record = active_field_record_;
        activate_field_record(next_available_field_record());
        if (active_field_record_ == previous_record) {
            ++runner_switch_rejections_;
        }
        ++action_count_;
    }
    if (action_flash_frames_ != 0) {
        --action_flash_frames_;
    }
    ++frame_number_;
}

void NativePortShell::render(ScreenWriter& screen) const {
    screen.begin_frame();
    if (scene_ == NativeScene::Title && !title_pixels_.empty()) {
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        // ROM 0x1E4E reveals one logo column on each title-loop pass after
        // the initial 0x5A-frame delay. The same loop calls 0x1C6A only while
        // that reveal is active; the SAT origin advances four pixels per call
        // and its six lists advance every four calls. Keep those source state
        // machines separate on the host instead of treating the sprite as a
        // continuously moving full-screen layer.
        if (!title_background_.pixels.empty()) {
            screen.blit_rgba(0, 0, title_background_.width,
                             title_background_.height,
                             title_background_.pixels);
            constexpr std::uint64_t kTitleRevealStart = 0x5AU;
            constexpr std::size_t kTitleRevealColumns = 38U;
            // frame_number_ is the number of completed host update calls. A
            // freshly constructed shell has performed no title-loop pass yet;
            // after the first update, source loop index zero has completed.
            const auto has_title_loop = frame_number_ != 0U;
            const auto title_loop = has_title_loop ? frame_number_ - 1U : 0U;
            auto visible_columns = std::size_t{0};
            if (has_title_loop && title_loop >= kTitleRevealStart) {
                visible_columns = std::min<std::size_t>(
                    kTitleRevealColumns,
                    1U + static_cast<std::size_t>(
                             title_loop - kTitleRevealStart));
            }
            if (visible_columns != 0U && !title_logo_plane_.pixels.empty()) {
                auto logo = title_logo_plane_.pixels;
                const auto first_visible_x = std::size_t{8U};
                const auto last_visible_x = std::min<std::size_t>(
                    title_logo_plane_.width,
                    first_visible_x + visible_columns * 8U);
                for (std::size_t y = 0; y < title_logo_plane_.height; ++y) {
                    for (std::size_t x = 0; x < title_logo_plane_.width; ++x) {
                        if (x < first_visible_x || x >= last_visible_x) {
                            logo[y * title_logo_plane_.width + x].alpha = 0;
                        }
                    }
                }
                screen.blit_rgba(0, 0, title_logo_plane_.width,
                                 title_logo_plane_.height, logo);
            }
            // 0x1C6A is called once for each of the first 38 reveal writes.
            // Its first call increments DD3E before selecting a list, so the
            // visible sequence starts at pointer-table entry 1. The SAT
            // origin is already $009C in the extracted layer (the initial
            // source value is $0098 and 0x1C6A adds four before 0x1378).
            if (!title_sprite_frames_.empty() && visible_columns != 0U &&
                visible_columns < kTitleRevealColumns) {
                const auto sprite_call_count = visible_columns;
                const auto phase = static_cast<std::size_t>(
                    (1U + (sprite_call_count - 1U) / 4U) %
                    title_sprite_frames_.size());
                const auto& sprite = title_sprite_frames_[phase];
                // Translate by the number of completed source sprite calls,
                // not by host frames. Once the logo is complete the source
                // stops calling 0x1C6A and the last SAT image remains stable.
                auto translated = std::vector<Rgba8>(
                    sprite.pixels.size(), Rgba8{0, 0, 0, 0});
                const auto delta = static_cast<std::int32_t>(
                    (sprite_call_count - 1U) * 4U);
                for (std::size_t y = 0; y < sprite.height; ++y) {
                    for (std::size_t x = 0; x < sprite.width; ++x) {
                        const auto source_index =
                            y * static_cast<std::size_t>(sprite.width) + x;
                        const auto target_x = static_cast<std::int32_t>(x) +
                                              delta;
                        if (target_x < 0 || target_x >= sprite.width) {
                            continue;
                        }
                        translated[y * static_cast<std::size_t>(sprite.width) +
                                   static_cast<std::size_t>(target_x)] =
                            sprite.pixels[source_index];
                    }
                }
                screen.blit_rgba(0, 0, sprite.width, sprite.height,
                                 translated);
            }
            if (!title_text_.pixels.empty()) {
                // 0x1DCA does not write PRESS START until the title loop has
                // reached its late countdown, while 0x1DF6 writes the
                // copyright block after the logo reveal. Keep these text
                // writes as a separate transparent layer so the native title
                // follows the source timing instead of showing a settled
                // screenshot on frame zero.
                auto blit_title_rows = [&screen, this](std::size_t first_row,
                                                        std::size_t row_count) {
                    auto text = title_text_.pixels;
                    const auto first_pixel =
                        first_row * 8U * title_text_.width;
                    const auto last_pixel = std::min<std::size_t>(
                        text.size(), (first_row + row_count) * 8U *
                                         title_text_.width);
                    for (std::size_t pixel = 0; pixel < text.size(); ++pixel) {
                        if (pixel < first_pixel || pixel >= last_pixel) {
                            text[pixel].alpha = 0;
                        }
                    }
                    screen.blit_rgba(0, 0, title_text_.width,
                                     title_text_.height, text);
                };
                // 0x1C1E starts DD5C at 0x5A when the final logo column sets
                // DD5A=1. It writes the copyright block when that countdown
                // crosses below zero, after the 38 reveal passes.
                constexpr auto kTitleCopyrightLoop =
                    kTitleRevealStart + kTitleRevealColumns + 0x5AU;
                if (has_title_loop &&
                    title_loop >= kTitleCopyrightLoop) {
                    blit_title_rows(22U, 4U);
                }
                // 0x1B72 selects 0x1DCA (PRESS START) for the first ten
                // values of D7&$3F; 0x1DE0 writes the blank companion line
                // for the other 54 values. D7 starts at $E10 and is decremented
                // once per title loop.
                const auto title_timer = static_cast<std::uint64_t>(
                    0xE10U - std::min<std::uint64_t>(title_loop, 0xE10U));
                if (has_title_loop && (title_timer & 0x3FU) < 10U) {
                    blit_title_rows(21U, 1U);
                }
            }
        } else if (!title_animation_frames_.empty()) {
            // Compatibility path for version-17 packs.
            constexpr std::uint64_t kTitleSpriteStartDelay = 0x5AU;
            constexpr std::uint64_t kTitleSpriteFrameCadence = 4U;
            const auto phase = frame_number_ < kTitleSpriteStartDelay
                ? 0U
                : static_cast<std::size_t>(
                      ((frame_number_ - kTitleSpriteStartDelay) /
                       kTitleSpriteFrameCadence) %
                      title_animation_frames_.size());
            const auto& title_frame = title_animation_frames_[phase];
            screen.blit_rgba(0, 0, title_frame.width, title_frame.height,
                             title_frame.pixels);
        } else {
            screen.blit_rgba(0, 0, title_width_, title_height_, title_pixels_);
        }
    } else if (scene_ == NativeScene::StartOptions) {
        render_startup_background(screen);
        for (const auto& line : intro_.start_options_layout) {
            write_startup_text(
                screen, 104, 56 + static_cast<std::int32_t>(line.row) * 8,
                line.text);
        }
        if (!startup_cursor_.pixels.empty()) {
            screen.blit_rgba(
                104, 80 + static_cast<std::int32_t>(start_options_selection_) *
                              16,
                startup_cursor_.width, startup_cursor_.height,
                startup_cursor_.pixels);
        }
        screen.present();
        return;
    } else if (scene_ == NativeScene::SaveLoad) {
        render_startup_background(screen);
        if (save_load_message_visible_) {
            const auto& layout = save_load_message_success_
                ? intro_.save_success_layout : intro_.no_save_layout;
            for (const auto& line : layout) {
                write_startup_text(
                    screen, 72, 72 + static_cast<std::int32_t>(line.row) * 8,
                    line.text);
            }
        } else {
            for (const auto& line : intro_.save_load_layout) {
                write_startup_text(
                    screen, 152, 112 + static_cast<std::int32_t>(line.row) * 8,
                    line.text);
            }
            if (!startup_cursor_.pixels.empty()) {
                screen.blit_rgba(
                    136, 112 + static_cast<std::int32_t>(
                                   save_load_selection_) * 16,
                    startup_cursor_.width, startup_cursor_.height,
                    startup_cursor_.pixels);
            }
        }
        screen.present();
        return;
    } else if (scene_ == NativeScene::ArchetypeMenu) {
        render_startup_background(screen);
        for (const auto& line : intro_.archetype_layout) {
            write_startup_text(
                screen, 152, 56 + static_cast<std::int32_t>(line.row) * 8,
                line.text);
        }
        if (!startup_cursor_.pixels.empty()) {
            screen.blit_rgba(
                152, 88 + static_cast<std::int32_t>(archetype_selection_) * 16,
                startup_cursor_.width, startup_cursor_.height,
                startup_cursor_.pixels);
        }
        screen.present();
        return;
    } else if (scene_ == NativeScene::Intro) {
        // 0x59C5E begins by clearing the VDP and then lets the timed
        // presentation routines write the story through C0DA.  There is no
        // native header, boxed panel, or "next page" prompt in that path.
        // Keep the source's eight-pixel character grid and center each line
        // in the 40-column Genesis display area.
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        const auto& group = intro_story_group(intro_frame_);
        const auto line_end = std::min(
            group.first_line + group.line_count, intro_.story_lines.size());
        for (auto line = group.first_line; line < line_end; ++line) {
            const auto& text = intro_.story_lines[line];
            const auto text_width = static_cast<std::int32_t>(text.size() * 8U);
            const auto x = std::max<std::int32_t>(
                0, (static_cast<std::int32_t>(kScreenWidth) - text_width) / 2);
            const auto y = group.top_y + static_cast<std::int32_t>(
                line - group.first_line) * 16;
            write_startup_text(screen, x, y, text);
        }
        screen.present();
        return;
    } else if ((scene_ == NativeScene::Field ||
                scene_ == NativeScene::FieldDialog ||
                scene_ == NativeScene::FieldMenu ||
                scene_ == NativeScene::ClipsMenu ||
                scene_ == NativeScene::CyberwareMenu ||
                scene_ == NativeScene::AttributesMenu) &&
               !field_view_pixels_.empty()) {
        // The field background is the ROM's mode-2 map translated into a host
        // RGBA viewport. NativeFieldEntity is composited after the terrain,
        // matching the ROM's entity/object ordering at this boundary.
        screen.clear({0x04, 0x08, 0x14, 0xFF});
        screen.fill_rect(0, 0, kScreenWidth, 34,
                         {0x12, 0x20, 0x38, 0xFF});
        screen.fill_rect(0, 34, kScreenWidth, 2,
                         {0x00, 0xA8, 0xC8, 0xFF});
        screen.write_text(12, 8, "SHADOWRUN FIELD", 2,
                          {0xE8, 0xF4, 0xFF, 0xFF});
        screen.write_text(204, 10,
                          "MODE " + std::to_string(field_mode_), 1,
                          {0xA0, 0xE8, 0xF8, 0xFF});

        screen.blit_rgba(0, 36, field_view_width_, field_view_height_,
                         field_view_pixels_);

        const NativeEntityFrame* frame = nullptr;
        const auto animation = field_entity_walking_
            ? std::find_if(
                  field_entity_animations_.begin(),
                  field_entity_animations_.end(),
                  [this](const NativeFieldAnimation& candidate) {
                      return candidate.state_selector ==
                             field_entity_.state_selector;
                  })
            : field_entity_animations_.end();
        if (animation != field_entity_animations_.end() &&
            field_entity_.frame_index < animation->frames.size()) {
            frame = &animation->frames[field_entity_.frame_index];
        } else if (!entity_frames_.empty() &&
                   field_entity_.frame_index < entity_frames_.size()) {
            frame = &entity_frames_[field_entity_.frame_index];
        }
        if (field_entity_.visible && frame != nullptr) {
            const auto entity_x = static_cast<std::int32_t>(
                static_cast<std::int16_t>(field_entity_.screen_x));
            const auto entity_y = static_cast<std::int32_t>(
                static_cast<std::int16_t>(field_entity_.screen_y)) +
                kFieldScreenOriginY;
            screen.fill_rect(entity_x - 7, entity_y - 2, 14, 4,
                             {0x00, 0x00, 0x00, 0xB0});
            screen.blit_rgba(
                entity_x - frame->width / 2, entity_y - frame->height,
                frame->width, frame->height, frame->pixels);
        } else {
            // Keep the input seam visible if a ROM is supplied without the
            // reviewed entity descriptor.
            screen.fill_rect(marker_x_, marker_y_, 8, 12, marker_color_);
        }
        if (scene_ == NativeScene::FieldDialog) {
            // The source 0x20E48 callback presents a text interaction over
            // the loaded 0x14 motel state. Keep the field behind it so the
            // transition remains observable, then wrap the source paragraph
            // at the Genesis 40-column text width.
            screen.fill_rect(8, 92, 304, 104, {0x00, 0x00, 0x00, 0xFF});
            screen.fill_rect(8, 92, 304, 2, {0x00, 0xA8, 0xC8, 0xFF});
            screen.fill_rect(8, 194, 304, 2, {0x00, 0xA8, 0xC8, 0xFF});
            screen.fill_rect(8, 92, 2, 104, {0x00, 0xA8, 0xC8, 0xFF});
            screen.fill_rect(310, 92, 2, 104, {0x00, 0xA8, 0xC8, 0xFF});
            write_field_menu_text(screen, 16, 100,
                                  "STOKER'S COFFIN MOTEL");

            const auto& paragraph = field_dialog_lines_[field_dialog_line_index_];
            std::vector<std::string> wrapped;
            std::string current;
            std::string word;
            const auto flush_word = [&]() {
                if (word.empty()) {
                    return;
                }
                if (!current.empty() && current.size() + 1U + word.size() >
                        34U) {
                    wrapped.push_back(current);
                    current.clear();
                }
                if (!current.empty()) {
                    current += ' ';
                }
                current += word;
                word.clear();
            };
            for (const auto character : paragraph) {
                if (character == ' ') {
                    flush_word();
                } else {
                    word += character;
                }
            }
            flush_word();
            if (!current.empty()) {
                wrapped.push_back(current);
            }
            for (std::size_t row = 0; row < wrapped.size() && row < 8U;
                 ++row) {
                write_field_menu_text(
                    screen, 16, 116 + static_cast<std::int32_t>(row) * 8,
                    wrapped[row]);
            }
            screen.write_text(222, 184, "A NEXT", 1,
                              {0xFF, 0xF0, 0x90, 0xFF});
        }
        if (action_flash_frames_ != 0) {
            const auto action_x = field_entity_.visible
                ? static_cast<std::int32_t>(
                      static_cast<std::int16_t>(field_entity_.screen_x))
                : marker_x_;
            const auto action_y = field_entity_.visible
                ? static_cast<std::int32_t>(
                      static_cast<std::int16_t>(field_entity_.screen_y)) +
                      kFieldScreenOriginY
                : marker_y_;
            screen.fill_rect(action_x - 4, action_y + 4, 16, 2,
                             {0xFF, 0xF0, 0x90, 0xFF});
            screen.fill_rect(action_x + 3, action_y - 8, 2, 24,
                             {0xFF, 0xF0, 0x90, 0xFF});
        }
        if (scene_ == NativeScene::FieldMenu) {
            // 0x10F50 replaces the field plane with the ROM's command-screen
            // base. The extracted image contains the actual panel geometry;
            // Inventory adds the separate 0x116C4 cell layer below.
            screen.clear({0x00, 0x00, 0x00, 0xFF});
            if (!field_menu_background_pixels_.empty()) {
                screen.blit_rgba(0, 0, field_menu_background_width_,
                                 field_menu_background_height_,
                                 field_menu_background_pixels_);
            }
            if (!field_menu_inventory_boxes_pixels_.empty()) {
                screen.blit_rgba(0, 0, field_menu_inventory_boxes_width_,
                                 field_menu_inventory_boxes_height_,
                                 field_menu_inventory_boxes_pixels_);
            }
            // ROM 0x110BC writes the active runner's 8x10 tile portrait to
            // Plane B at E104 (x=16, y=16), then 0x1111A writes its name to
            // Plane A at C0A0 (x=128, y=8). These are the fixed command-screen
            // destinations visible in the original left/center layout.
            if (active_field_record_ < field_menu_runner_portraits_.size()) {
                // The three portrait resources are Joshua's three source
                // archetype portraits. A party slot is not a portrait index.
                const auto portrait_index = active_field_record_ == 0
                    ? selected_archetype_ : active_field_record_;
                const auto& portrait =
                    field_menu_runner_portraits_[portrait_index];
                if (!portrait.pixels.empty()) {
                    screen.blit_rgba(16, 16, portrait.width, portrait.height,
                                     portrait.pixels);
                }
                const auto& name = field_menu_runner_names_[active_field_record_];
                if (!name.empty()) {
                    write_field_menu_text(screen, 128, 8, name);
                }
            }
            // 0x11928/0x11AE4 writes the active record's five-state status
            // strip at Plane B CAAC (x=176, y=168) and its matching text at
            // CB22 (x=136, y=176). Both resources are extracted from CF134,
            // CF384, and D1A05; no host status bar is invented here.
            const auto status_index = std::min<std::size_t>(
                field_entity_.menu_status_index,
                field_menu_status_bars_.size() - 1U);
            const auto& status_bar = field_menu_status_bars_[status_index];
            if (!status_bar.pixels.empty()) {
                screen.blit_rgba(176, 168, status_bar.width,
                                 status_bar.height, status_bar.pixels);
            }
            const auto& status_name = field_menu_status_names_[status_index];
            if (!status_name.empty()) {
                write_field_menu_text(screen, 136, 176, status_name);
            }
            const auto draw_item_detail = [this, &screen](
                                              std::uint8_t item_code,
                                              bool include_art) {
                if (item_code == 0U || item_code > kNativeInventoryItemCount) {
                    return;
                }
                const auto item_index = static_cast<std::size_t>(item_code - 1U);
                const auto& item_art = field_menu_item_art_[item_index];
                // ROM 0x12A8C places artwork at C1A4 only for the selected
                // inventory item. The overview's 0x11260 path calls the
                // shared 0x12EAE text/stat writer without this art pass.
                if (include_art && !item_art.pixels.empty()) {
                    screen.blit_rgba(144, 24, item_art.width,
                                     item_art.height, item_art.pixels);
                }
                const auto& item = inventory_items_[item_index];
                if (!item.name.empty()) {
                    write_field_menu_text(screen, 128, 64, item.name);
                }
                if (!item.category.empty()) {
                    write_field_menu_text(screen, 128, 72, item.category);
                }
                // 0x115F2 calls 0x12E9E only for item codes below 0x10.
                // Medkit (0x10), patches, tools, spells, and armor do not
                // use the weapon Damage/Power detail writer. 0x12A8C still
                // writes its source Ammo/Uses/Rtng label and phase-table
                // value for categories that have one.
                const auto is_weapon_detail = item_code < 0x10U;
                if (is_weapon_detail &&
                    item.damage_table_index < field_menu_damage_bars_.size()) {
                    write_field_menu_text(screen, 112, 80, "Damage");
                    const auto& damage_bar =
                        field_menu_damage_bars_[item.damage_table_index];
                    if (!damage_bar.pixels.empty()) {
                        screen.blit_rgba(168, 80, damage_bar.width,
                                         damage_bar.height, damage_bar.pixels);
                    }
                }
                if (is_weapon_detail && item.power_table_index != 0U) {
                    write_field_menu_text(screen, 112, 88, "Power");
                    const auto label_index = static_cast<std::size_t>(
                        item.power_table_index - 1U);
                    if (label_index < field_menu_power_labels_.size()) {
                        write_field_menu_text(
                            screen, 168, 88,
                            field_menu_power_labels_[label_index]);
                    }
                }
                const auto detail_label = source_item_detail_label(item_code);
                if (!detail_label.empty()) {
                    write_field_menu_text(screen, 112, 80, detail_label);
                    // The source value is a two-tile entry selected from
                    // 0x6AC46 by the selected slot's +0x6E byte. Until that
                    // tile table is promoted into the native asset pack,
                    // expose the exact source selector rather than faking a
                    // weapon damage statistic for medical/utility items.
                    const auto phase = source_inventory_phase_for_code(
                        field_entity_, item_code);
                    write_field_menu_text(screen, 168, 80,
                                          std::to_string(phase));
                }
            };
            if (field_menu_selection_ >= 2U) {
                // ROM 0x111E8 redraws this overview whenever FFF112 is set.
                // These destinations are the literal C1A0/C1AC/C2A0/C2B4/
                // C334/C420 family used by that path; the source labels and
                // record bytes are now kept together instead of showing an
                // empty center panel.
                const auto race = field_entity_.race_index == 7U
                    ? std::string_view("Human") : std::string_view("Human");
                // The live record stores the source class index, not the
                // startup selector index: 1=Gator Shaman, 2=Samurai,
                // 3=Decker in the three 0x7291A templates.
                const auto archetype = [&]() -> std::string {
                    switch (field_entity_.archetype_index) {
                    case 1U:
                        return intro_.archetypes[2] + " " + intro_.archetypes[3];
                    case 2U:
                        return intro_.archetypes[0];
                    case 3U:
                        return intro_.archetypes[1];
                    default:
                        return "Unknown";
                    }
                }();
                write_field_menu_text(
                    screen, 128, 24,
                    std::string(race) + " " + archetype);
                write_field_menu_text(screen, 128, 40, "Physical");
                write_field_menu_text(screen, 128, 48, "Mental");
                write_field_menu_text(
                    screen, 208, 40,
                    std::to_string(std::min<unsigned>(100U,
                        static_cast<unsigned>(field_entity_.physical_value) *
                            10U)) + "%");
                write_field_menu_text(
                    screen, 208, 48,
                    std::to_string(std::min<unsigned>(100U,
                        static_cast<unsigned>(field_entity_.mental_value) *
                            10U)) + "%");
                draw_item_detail(field_entity_.active_item_code, false);
                write_field_menu_text(screen, 112, 104, "Atk");
                write_field_menu_text(screen, 112, 112, "Def");
                // 0x11ABE -> 0x12CEE writes the long attack/defense strips
                // from CF3D4/CF610 at the source C6A4/C724 destinations.
                if (field_entity_.attack_bar_index <
                        field_menu_attack_bars_.size()) {
                    const auto& bar = field_menu_attack_bars_[
                        field_entity_.attack_bar_index];
                    if (!bar.pixels.empty()) {
                        screen.blit_rgba(144, 104, bar.width, bar.height,
                                         bar.pixels);
                    }
                }
                if (field_entity_.defense_bar_index <
                        field_menu_defense_bars_.size()) {
                    const auto& bar = field_menu_defense_bars_[
                        field_entity_.defense_bar_index];
                    if (!bar.pixels.empty()) {
                        screen.blit_rgba(144, 112, bar.width, bar.height,
                                         bar.pixels);
                    }
                }
                // 0x11352 emits the selected two-row armor record at C820,
                // then the fixed Wpns/Guns pair at C922. The source rows are
                // deliberately kept at their own destinations: the status
                // strip begins at CAAC and must not be overwritten by menu
                // labels.
                const auto armor_index = field_entity_.equipped_armor_index;
                if (armor_index != 0U &&
                    armor_index <= field_menu_armor_labels_.size()) {
                    const auto& armor =
                        field_menu_armor_labels_[armor_index - 1U];
                    for (std::size_t row = 0; row < armor.size(); ++row) {
                        if (!armor[row].empty()) {
                            write_field_menu_text(
                                screen, 128, 128 + static_cast<std::int32_t>(
                                    row * 8U), armor[row]);
                        }
                    }
                } else {
                    // 0x1142E uses D1F45/C8A6 for the no-armor fallback.
                    write_field_menu_text(screen, 152, 136, "No Armor");
                }
                write_field_menu_text(screen, 136, 144, "Wpns");
                write_field_menu_text(screen, 136, 152, "Guns");
                if (field_entity_.weapons_bar_index <
                        field_menu_damage_bars_.size()) {
                    const auto& bar = field_menu_damage_bars_[
                        field_entity_.weapons_bar_index];
                    if (!bar.pixels.empty()) {
                        screen.blit_rgba(176, 144, bar.width, bar.height,
                                         bar.pixels);
                    }
                }
                if (field_entity_.guns_bar_index <
                        field_menu_drain_bars_.size()) {
                    const auto& bar = field_menu_drain_bars_[
                        field_entity_.guns_bar_index];
                    if (!bar.pixels.empty()) {
                        screen.blit_rgba(176, 152, bar.width, bar.height,
                                         bar.pixels);
                    }
                }
            }
            // ROM 0x11136 starts at Plane A C784 and advances by 0x0A for
            // each 3x2 icon writer. The eight slots are two columns by four
            // rows at the exact name-table coordinates below. Empty item
            // bytes leave their extracted box black; a nonzero byte indexes
            // the 50-record icon table at D4540.
            constexpr std::array<std::array<std::int32_t, 2>,
                                 kNativeInventorySlotCount>
                kInventoryIconPositions{{
                    {{16, 120}}, {{56, 120}},
                    {{16, 144}}, {{56, 144}},
                    {{16, 168}}, {{56, 168}},
                    {{16, 192}}, {{56, 192}},
                }};
            const auto icon_palette_variant =
                field_menu_uses_alt_item_palette(field_entity_) ? 1U : 0U;
            const auto selected_slot = field_menu_row_ * 2U +
                                       field_menu_selection_;
            const auto selected_code = field_menu_selection_ < 2U &&
                    selected_slot < field_entity_.inventory_item_codes.size()
                ? field_entity_.inventory_item_codes[selected_slot]
                : 0U;
            if (field_menu_selection_ < 2U) {
                // ROM 0x11592 emits D29F6 before 0x11624 redraws the cell
                // layers. The 0x80 sprite origin converts its first
                // coordinate pair to (12,116), then the source's 0x28/0x18
                // selector strides move it with the selected slot.
                if (!field_menu_cursor_.pixels.empty()) {
                    screen.blit_rgba(
                        12 + static_cast<std::int32_t>(
                                 field_menu_selection_ * 40U),
                        116 + static_cast<std::int32_t>(field_menu_row_ * 24U),
                        field_menu_cursor_.width, field_menu_cursor_.height,
                        field_menu_cursor_.pixels);
                }
            }
            // 0x11624 draws the light cell border only for source equipment:
            // first +0x56 (weapon slot), then the inventory cell containing
            // +0x76+0x2B (armor). It is not the cursor and must not surround
            // every merely selected inventory cell.
            const auto draw_equipment_border = [this, &screen](std::size_t slot) {
                if (slot >= kNativeInventorySlotCount) {
                    return;
                }
                const auto overlay_index = ((slot / 2U) & 1U) == 0U ? 0U : 1U;
                const auto& overlay = field_menu_selected_cells_[overlay_index];
                if (!overlay.pixels.empty()) {
                    screen.blit_rgba(
                        8 + static_cast<std::int32_t>((slot % 2U) * 40U),
                        112 + static_cast<std::int32_t>((slot / 2U) * 24U),
                        overlay.width, overlay.height, overlay.pixels);
                }
            };
            if (field_entity_.display_state >= 0 &&
                field_entity_.display_state != 0x7F) {
                draw_equipment_border(
                    static_cast<std::size_t>(field_entity_.display_state));
            }
            if (field_entity_.equipped_armor_index != 0U) {
                const auto armor_code = static_cast<std::uint8_t>(
                    0x2BU + field_entity_.equipped_armor_index);
                for (std::size_t slot = 0;
                     slot < field_entity_.inventory_item_codes.size(); ++slot) {
                    if (field_entity_.inventory_item_codes[slot] == armor_code) {
                        draw_equipment_border(slot);
                        break;
                    }
                }
            }
            for (std::size_t slot = 0; slot <
                 field_entity_.inventory_item_codes.size(); ++slot) {
                const auto item_code =
                    field_entity_.inventory_item_codes[slot];
                if (item_code == 0U ||
                    item_code > kNativeInventoryItemCount) {
                    continue;
                }
                const auto& icon = field_menu_inventory_icons_[item_code - 1U]
                    [icon_palette_variant];
                if (!icon.pixels.empty()) {
                    screen.blit_rgba(kInventoryIconPositions[slot][0],
                                     kInventoryIconPositions[slot][1],
                                     icon.width, icon.height, icon.pixels);
                }
            }
            if (field_menu_selection_ < 2U) {
                draw_item_detail(selected_code, true);
            }
            // ROM 0x1170C emits only the currently selected command at
            // plane-B address EC9C (x=112, y=200). The other command names
            // are not a vertical checklist on this screen.
            write_field_menu_text(screen, 112, 200,
                                  kFieldMenuLabels[field_menu_selection_]);
        } else if (scene_ == NativeScene::ClipsMenu) {
            screen.fill_rect(24, 44, 272, 144,
                             {0x02, 0x08, 0x18, 0xF0});
            screen.fill_rect(24, 44, 272, 2,
                             {0x00, 0xA8, 0xC8, 0xFF});
            screen.write_text(36, 52, "CLIPS", 2,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            screen.write_text(166, 54,
                              "AVAILABLE " +
                                  std::to_string(field_entity_.ammo_clips),
                              1, {0xA0, 0xE8, 0xF8, 0xFF});
            screen.write_text(166, 66,
                              "IN WEAPON " +
                                  std::to_string(field_entity_.ammo_rounds),
                              1, {0xA0, 0xE8, 0xF8, 0xFF});
            for (std::size_t index = 0; index < kClipsMenuLabels.size();
                 ++index) {
                const auto selector = index + 2U;
                const auto y = 82 + static_cast<std::int32_t>(index) * 16;
                const auto selected = selector == clips_menu_selection_;
                if (selected) {
                    screen.fill_rect(32, y - 2, 256, 13,
                                     {0x00, 0x68, 0x88, 0xFF});
                }
                screen.write_text(40, y, kClipsMenuLabels[index], 1,
                                  selected
                                      ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                                      : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
            }
            screen.write_text(36, 152,
                              "LEFT/RIGHT SELECT  Z/X/C ACTION", 1,
                              {0x90, 0xB8, 0xC8, 0xFF});
            if (clips_action_message_frames_ != 0) {
                screen.write_text(176, 152,
                                  clips_out_of_ammo_ ? "NO CLIPS" : "QUEUED",
                                  1, {0xF0, 0xC0, 0x50, 0xFF});
            }
            screen.fill_rect(0, 204, kScreenWidth, 20,
                             {0x02, 0x05, 0x0C, 0xFF});
            screen.write_text(8, 208, "CLIPS COMMANDS  ENTER RETURN", 1,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            screen.write_text(8, 216,
                              "SELECTOR " +
                                  std::to_string(clips_menu_selection_) +
                                  "  ACTIONS " +
                                  std::to_string(clips_action_count_),
                              1, {0x90, 0xB8, 0xC8, 0xFF});
        } else if (scene_ == NativeScene::CyberwareMenu) {
            screen.fill_rect(24, 44, 272, 144,
                             {0x02, 0x08, 0x18, 0xF0});
            screen.fill_rect(24, 44, 272, 2,
                             {0x00, 0xA8, 0xC8, 0xFF});
            screen.write_text(36, 52, "CYBERWARE", 2,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            for (std::size_t index = 0; index < cyberware_names_.size();
                 ++index) {
                const auto installed =
                    (field_entity_.cyberware_flags &
                     static_cast<std::uint16_t>(1U << index)) != 0;
                const auto& name = cyberware_names_[index];
                const auto line = std::string(installed ? "[ON] " : "[--] ") +
                                  (name.empty() ? "UNKNOWN" : name);
                screen.write_text(
                    36, 70 + static_cast<std::int32_t>(index) * 7, line, 1,
                    installed
                        ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                        : Rgba8{0xA8, 0xB8, 0xC8, 0xFF});
            }
            screen.write_text(36, 181, "START RETURN", 1,
                              {0x90, 0xB8, 0xC8, 0xFF});
        } else if (scene_ == NativeScene::AttributesMenu) {
            screen.fill_rect(24, 44, 272, 144,
                             {0x02, 0x08, 0x18, 0xF0});
            screen.fill_rect(24, 44, 272, 2,
                             {0x00, 0xA8, 0xC8, 0xFF});
            screen.write_text(36, 52, "ATTRIBUTES/SKILLS", 2,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            for (std::size_t column = 0; column < 2U; ++column) {
                for (std::size_t row = 0; row < 10U; ++row) {
                    const auto index = column * 10U + row;
                    const auto selected = column == attributes_menu_column_ &&
                                          row == attributes_menu_row_;
                    const auto x = column == 0U ? 32 : 164;
                    const auto y = 70 + static_cast<std::int32_t>(row) * 10;
                    if (selected) {
                        screen.fill_rect(x, y - 2, 124, 10,
                                         {0x00, 0x68, 0x88, 0xFF});
                    }
                    const auto value = index < kNativeAttributeCount
                        ? field_entity_.attribute_values[index]
                        : field_entity_.skill_values[index -
                                                      kNativeAttributeCount];
                    const auto& name = attribute_skill_names_[index];
                    const auto line = (name.empty() ? "UNKNOWN" : name) +
                                      " " + std::to_string(value);
                    screen.write_text(
                        x + 6, y, line, 1,
                        selected
                            ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                            : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
                }
            }
            screen.write_text(36, 181, "ARROWS SELECT  START RETURN", 1,
                              {0x90, 0xB8, 0xC8, 0xFF});
        } else {
            screen.fill_rect(0, 204, kScreenWidth, 20,
                             {0x02, 0x05, 0x0C, 0xFF});
            screen.write_text(8, 208, "ARROWS MOVE ENTITY  C SWITCH RUNNER", 1,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            screen.write_text(8, 216, "ENTER COMMANDS  Z/X PENDING", 1,
                              {0x90, 0xB8, 0xC8, 0xFF});
        }
        screen.write_text(226, 216, std::to_string(action_count_), 1,
                          {0xF0, 0xC0, 0x50, 0xFF});
    } else {
        screen.clear({0x06, 0x08, 0x18, 0xFF});
        screen.fill_rect(0, 0, kScreenWidth, 34, {0x16, 0x24, 0x48, 0xFF});
        screen.fill_rect(0, 34, kScreenWidth, 2, {0x00, 0xA8, 0xC8, 0xFF});
        screen.write_text(18, 10, title_, 2, {0xE8, 0xF4, 0xFF, 0xFF});
        screen.write_text(18, 52, "NATIVE C++ PORT BOOTSTRAP", 1,
                          {0x7C, 0xD8, 0xF0, 0xFF});
        screen.write_text(18, 66, "ARROWS MOVE  Z X C TEST BUTTONS", 1,
                          {0xA0, 0xA8, 0xC0, 0xFF});
        screen.write_text(18, 78, "ENTER RESETS THE MARKER", 1,
                          {0xA0, 0xA8, 0xC0, 0xFF});
    }
    if (scene_ == NativeScene::Bootstrap) {
        screen.fill_rect(marker_x_, marker_y_, 8, 8, marker_color_);
        screen.fill_rect(marker_x_ + 2, marker_y_ + 2, 4, 4,
                         {0xFF, 0xFF, 0xFF, 0xFF});
        screen.write_text(18, 204, "FRAME", 1, {0x78, 0x88, 0xA8, 0xFF});
        screen.write_text(60, 204, std::to_string(frame_number_), 1,
                          {0xD0, 0xD8, 0xF0, 0xFF});
    }
    screen.present();
}

} // namespace shadowrun::native
