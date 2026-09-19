#include "shadowrun/native_port.hpp"

#include "shadowrun/entity_projection.hpp"
#include "shadowrun/graphics.hpp"

#include <algorithm>
#include <array>
#include <optional>
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
// Capture t1720: playfield is 256px; HUD is a separate 64px column.
constexpr std::int32_t kFieldViewportWidth = kFieldPlayfieldWidth;
constexpr std::int32_t kFieldViewportHeight = kScreenHeight;
constexpr std::int32_t kFieldScreenOriginY = 0;
constexpr std::uint64_t kCabMapFrames = 90;
// ROM 0x5A62 accepts VDP X values strictly below 0x1AE. After removing the
// 0x80 sprite origin, this is the last host pixel at which the record remains
// in the direct projection window.
constexpr std::int32_t kFieldEntityMaxHostX = 0x1AE -
                                                kGenesisVdpSpriteOrigin - 1;

// The five command tabs are part of the extracted base plane. The source
// redraws their focus block separately from the selected command label.
constexpr std::array<std::int32_t, 5> kFieldMenuCommandButtonX{{
    128, 152, 176, 200, 224}};
constexpr std::int32_t kFieldMenuCommandButtonY = 184;

std::optional<std::uint16_t> source_heading_for_input(
    std::int32_t delta_x, std::int32_t delta_y) {
    // ROM 0x195C indexes the 16-entry table at 0x19D2 with the raw
    // active-low D-pad nibble. InputState has already normalized the
    // controller state, so rebuild that nibble from the held directions.
    std::uint8_t source_mask = 0;
    if (delta_y < 0) {
        source_mask |= 0x01U; // Up
    } else if (delta_y > 0) {
        source_mask |= 0x02U; // Down
    }
    if (delta_x < 0) {
        source_mask |= 0x04U; // Left
    } else if (delta_x > 0) {
        source_mask |= 0x08U; // Right
    }

    switch (source_mask) {
    case 0x01U: return 0x0000U;
    case 0x02U: return 0x8000U;
    case 0x04U: return 0xC000U;
    case 0x08U: return 0x4000U;
    case 0x05U: return 0xE000U;
    case 0x06U: return 0xA000U;
    case 0x09U: return 0x2000U;
    case 0x0AU: return 0x6000U;
    default: return std::nullopt;
    }
}

std::size_t source_direction_index(std::uint16_t heading) {
    // This is the repeated ROM idiom at 0xBABC, 0xB39A, and 0xB574:
    // ROL.W #3, compensate a negative signed result, then retain 3 bits.
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) |
        static_cast<std::uint16_t>(heading >> 13U));
    if (static_cast<std::int16_t>(rotated) < 0) {
        rotated = static_cast<std::uint16_t>(rotated + 1U);
    }
    return static_cast<std::size_t>(rotated & 0x0007U);
}

std::uint16_t slew_source_heading(std::uint16_t current,
                                  std::uint16_t target) {
    // Direct host form of 0xB530. The unsigned comparisons intentionally
    // preserve the original wraparound behavior at the 0/0xFFFF boundary.
    const auto difference = static_cast<std::uint16_t>(current - target);
    if (current == target) {
        return current;
    }
    if (static_cast<std::int16_t>(difference) < 0) {
        return difference >= 0xF800U
            ? target : static_cast<std::uint16_t>(current + 0x0800U);
    }
    return difference < 0x0800U
        ? target : static_cast<std::uint16_t>(current - 0x0800U);
}

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

// 0x5AF62 script 0x5ADD0: wait 0x3C, 0x5AEC6 line 0; wait 0x78,
// 0x5AEDA line 1; wait 0xB4, 0x5AEEE lines 2-4; wait 0xF0+0x0F.
constexpr std::uint64_t kIntroSeattleHold = 0x3CU;
constexpr std::uint64_t kIntroSeattlePlace = kIntroSeattleHold + 0x78U;
constexpr std::uint64_t kIntroSeattleTeam = kIntroSeattlePlace + 0xB4U;
constexpr std::uint64_t kIntroSeattleEnd =
    kIntroSeattleTeam + 0xF0U + 0x0FU;
// 0x59D7C: 0xA0/DBF pan, then 0x1F0/DBF SAT loop. 0x5AE9E after the
// first pan vblank. 0x5ADFE/0x5AE3A when the second countdown is 0x164.
constexpr std::uint64_t kIntroWildernessPan = kIntroSeattleEnd + 0xA1U;
constexpr std::uint64_t kIntroWildernessMassacre =
    kIntroWildernessPan + (0x1F0U - 0x164U);
constexpr std::uint64_t kIntroWildernessEnd =
    kIntroWildernessPan + 0x1F1U;
// 0x5AA96 0x114/DBF, then 0x5AD1A 0x528/DBF. 0x5A96A at countdown 0x1E0.
constexpr std::uint64_t kIntroTrividEnd = kIntroWildernessEnd + 0x115U;
constexpr std::uint64_t kIntroAirportText =
    kIntroTrividEnd + (0x528U - 0x1E0U);
constexpr std::uint64_t kIntroDuration = kIntroTrividEnd + 0x529U;

// Capture 10 Hz storyboard: green story text sits in the upper black
// band; the skyline / ambush / cockpit occupy the lower half.
constexpr std::array<IntroStoryGroup, 9> kIntroStoryGroups{{
    {0x000U, 0U, 0U, 16},
    {kIntroSeattleHold, 0U, 1U, 16},
    {kIntroSeattlePlace, 0U, 2U, 16},
    {kIntroSeattleTeam, 0U, 5U, 16},
    {kIntroSeattleEnd, 5U, 2U, 16},
    {kIntroWildernessMassacre, 5U, 2U, 16},
    {kIntroWildernessEnd, 10U, 3U, 16},
    {kIntroTrividEnd, 13U, 3U, 16},
    {kIntroAirportText, 16U, 6U, 16},
}};

const IntroStoryGroup& intro_story_group(std::uint64_t frame) {
    const auto group = std::find_if(
        kIntroStoryGroups.rbegin(), kIntroStoryGroups.rend(),
        [frame](const IntroStoryGroup& candidate) {
            return frame >= candidate.start_frame;
        });
    return group == kIntroStoryGroups.rend() ? kIntroStoryGroups.front()
                                             : *group;
}

std::vector<Rgba8> scale_rgba(const std::vector<Rgba8>& source,
                              std::uint16_t remaining, std::uint16_t total) {
    auto faded = source;
    if (total == 0U || remaining >= total) {
        return faded;
    }
    for (auto& pixel : faded) {
        pixel.red = static_cast<std::uint8_t>(
            (static_cast<std::uint16_t>(pixel.red) * remaining) / total);
        pixel.green = static_cast<std::uint8_t>(
            (static_cast<std::uint16_t>(pixel.green) * remaining) / total);
        pixel.blue = static_cast<std::uint8_t>(
            (static_cast<std::uint16_t>(pixel.blue) * remaining) / total);
    }
    return faded;
}

void blit_intro_actor(ScreenWriter& screen, const NativeImage& image,
                      std::int32_t src_x, std::int32_t src_y,
                      std::int32_t width, std::int32_t height,
                      std::int32_t dest_x, std::int32_t dest_y, bool flash) {
    if (image.pixels.empty() || width <= 0 || height <= 0) {
        return;
    }
    const auto image_width = static_cast<std::int32_t>(image.width);
    const auto image_height = static_cast<std::int32_t>(image.height);
    std::vector<Rgba8> tile(static_cast<std::size_t>(width * height),
                            Rgba8{0, 0, 0, 0});
    for (std::int32_t row = 0; row < height; ++row) {
        const auto source_y = src_y + row;
        if (source_y < 0 || source_y >= image_height) {
            continue;
        }
        for (std::int32_t column = 0; column < width; ++column) {
            const auto source_x = src_x + column;
            if (source_x < 0 || source_x >= image_width) {
                continue;
            }
            auto pixel = image.pixels[static_cast<std::size_t>(source_y) *
                                          image.width +
                                      static_cast<std::size_t>(source_x)];
            if (pixel.alpha == 0U) {
                continue;
            }
            const auto grass = pixel.green > pixel.red + 16U &&
                               pixel.green > pixel.blue + 8U;
            // Dirt is near-neutral brown. Shaman skin is red-dominant; the
            // previous dirt test dropped the head.
            const auto dirt = pixel.red > 40U && pixel.green > 28U &&
                              pixel.blue < 50U && pixel.red < 160U &&
                              pixel.green < 130U &&
                              pixel.red < pixel.green + 24U &&
                              pixel.green < pixel.red + 24U;
            if (grass || dirt) {
                continue;
            }
            if (flash) {
                pixel.red = 0xFF;
                pixel.green = 0xFF;
                pixel.blue = 0xFF;
            }
            tile[static_cast<std::size_t>(row * width + column)] = pixel;
        }
    }
    screen.blit_rgba(dest_x, dest_y, static_cast<std::uint16_t>(width),
                     static_cast<std::uint16_t>(height), tile);
}

void draw_intro_bolt(ScreenWriter& screen, std::int32_t x, std::int32_t y) {
    // Capture muzzle ticks are short cyan capsules, not 8x2 white bars.
    const Rgba8 bolt{0x48, 0xD0, 0xE0, 0xFF};
    std::vector<Rgba8> bar(12U * 3U, bolt);
    screen.blit_rgba(x, y, 12, 3, bar);
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

// ROM 0xC2A6 passes this source table to the common menu writer at 0xC0DA.
// Main Menu and the separator at 0xC1418/0xC1423 are panel text, not
// selectable entries; the five rows beginning at 0xC1430 are the cursor's
// actual option range. Restricted source modes omit the final Save/Load row.
constexpr std::array<std::string_view, 5> kPocketSecretaryLabels{{
    "Notebook",
    "Group Items",
    "Cyberdeck",
    "Dismiss Runner",
    "Save/Load Game",
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
    // ROM 0x11B58 writes the D1DDC table here. Increase/Decrease/Trade
    // Clips/Change Recipient are the nested D1E24 trade table instead.
    "     Reload      ",
    "      Trade      ",
    "   Distribute    ",
    "     Cancel      ",
}};
constexpr std::array<std::string_view, 4> kClipsTradeMenuLabels{{
    // ROM 0x11E7E writes the nested D1E24 table after Trade is selected.
    " Increase Clips ",
    " Decrease Clips ",
    "  Trade Clips  ",
    "Change Recipient",
}};

// 0x11740 selects the standard action table at 0xD1D94 unless the selected
// item code is 0x10..0x13, which selects the medical table at 0xD1E6C.
// 0x118C6 keeps the action selector in the inclusive range 2..5.
constexpr std::array<std::string_view, 4> kInventoryActionLabels{{
    "  Equip/Unequip  ",
    "     Discard     ",
    "      Trade      ",
    "     Cancel      ",
}};
constexpr std::array<std::string_view, 4> kMedicalActionLabels{{
    "    First Aid    ",
    "     Discard     ",
    "      Trade      ",
    "     Cancel      ",
}};
constexpr std::array<std::string_view, 2> kDiscardChoiceLabels{{
    "       No        ",
    "       Yes       ",
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
    : title_(std::move(title)), scene_(NativeScene::Field) {}

std::string_view NativePortShell::field_active_item_detail_label() const {
    return source_item_detail_label(field_entity_.active_item_code);
}

std::string_view NativePortShell::field_selected_item_detail_label() const {
    if (field_menu_selection_ >= 2U) {
        return {};
    }
    const auto slot = field_menu_row_ * 2U + field_menu_selection_;
    if (slot >= field_entity_.inventory_item_codes.size()) {
        return {};
    }
    return source_item_detail_label(field_entity_.inventory_item_codes[slot]);
}

std::string_view NativePortShell::field_menu_action_label() const {
    if (!field_menu_action_active_ || field_menu_action_selection_ < 2U ||
        field_menu_action_selection_ > 5U || field_menu_selection_ >= 2U) {
        return {};
    }
    const auto slot = field_menu_row_ * 2U + field_menu_selection_;
    if (slot >= field_entity_.inventory_item_codes.size() ||
        field_entity_.inventory_item_codes[slot] == 0U) {
        return {};
    }
    const auto item_code = field_entity_.inventory_item_codes[slot];
    const auto& labels = item_code >= 0x10U && item_code <= 0x13U
        ? kMedicalActionLabels : kInventoryActionLabels;
    return labels[field_menu_action_selection_ - 2U];
}

std::string_view NativePortShell::field_discard_message() const {
    if (field_discard_cannot_discard_) {
        // ROM 0x12634 routes protected item codes to D1FFA/Cannot discard
        // through the common acknowledgement path rather than mutating the
        // possession cell.
        return "Cannot discard";
    }
    if (field_discard_item_was_equipped_) {
        // ROM 0x126A6 selects D1CBB/D1CC8 while the confirmation loop is
        // active; it does not make the item undeletable.
        return "This item is equipped";
    }
    return {};
}

std::string_view NativePortShell::field_trade_message() const {
    if (field_trade_cannot_trade_) {
        return "Cannot trade magic items!";
    }
    if (field_trade_full_inventory_ &&
        field_trade_recipient_record_ < field_menu_runner_names_.size()) {
        return field_trade_full_inventory_message_;
    }
    return {};
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
    title_circuit_lights_ = std::move(assets.title_circuit_lights);
    title_logo_plane_ = std::move(assets.title_logo_plane);
    title_text_ = std::move(assets.title_text);
    title_sprite_frames_ = std::move(assets.title_sprite_frames);
    title_animation_frames_ = std::move(assets.title_animation_frames);
    boot_background_ = std::move(assets.boot_background);
    boot_static_ = std::move(assets.boot_static);
    boot_guy_ = std::move(assets.boot_guy);
    boot_sega_frames_ = std::move(assets.boot_sega_frames);
    boot_logo_strobe_ = std::move(assets.boot_logo_strobe);
    boot_fasa_ = std::move(assets.boot_fasa);
    startup_background_ = std::move(assets.startup_background);
    intro_heli_frames_ = std::move(assets.intro_heli_frames);
    field_hud_ = std::move(assets.field_hud);
    pocket_secretary_background_ = std::move(assets.pocket_secretary_background);
    intro_stages_ = std::move(assets.intro_stages);
    if (!boot_background_.pixels.empty()) {
        scene_ = NativeScene::Bootstrap;
    }
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
    field_menu_command_focus_ = std::move(assets.field_menu_command_focus);
    field_menu_cursor_ = std::move(assets.field_menu_cursor);
    field_menu_item_art_ = std::move(assets.field_menu_item_art);
    field_menu_inventory_icons_ = std::move(assets.field_menu_inventory_icons);
    field_menu_runner_names_ = std::move(assets.field_menu_runner_names);
    field_menu_status_bars_ = std::move(assets.field_menu_status_bars);
    field_menu_status_names_ = std::move(assets.field_menu_status_names);
    field_menu_damage_bars_ = std::move(assets.field_menu_damage_bars);
    field_menu_drain_bars_ = std::move(assets.field_menu_drain_bars);
    field_menu_detail_values_ = std::move(assets.field_menu_detail_values);
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
    group_item_names_ = std::move(assets.group_item_names);
    cyberdeck_unavailable_lines_ =
        std::move(assets.cyberdeck_unavailable_lines);
    dismiss_runner_labels_ = std::move(assets.dismiss_runner_labels);
    notebook_labels_ = std::move(assets.notebook_labels);
    notebook_no_run_lines_ = std::move(assets.notebook_no_run_lines);
    notebook_event_gates_ = assets.notebook_event_gates;
    notebook_tip_text_ = std::move(assets.notebook_tip_text);
    notebook_tip_flags_ = assets.notebook_tip_flags;
    notebook_mr_johnson_names_ = std::move(assets.notebook_mr_johnson_names);
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
    field_entity_heading_current_ = 0;
    field_entity_heading_target_ = 0;
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
    field_entity_heading_current_ = 0;
    field_entity_heading_target_ = 0;
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
    save_load_from_pocket_secretary_ = false;
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
        // ROM 0x59172 is shared by startup and Pocket Secretary, but each
        // caller resumes at its own menu after the handler returns.
        if (save_load_from_pocket_secretary_) {
            scene_ = NativeScene::PocketSecretaryMenu;
            field_menu_selection_ = 6;
            save_load_from_pocket_secretary_ = false;
        } else {
            scene_ = NativeScene::StartOptions;
        }
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
        // 0x59C5E fades, sets FFFBF8=2, then 0x59D12/0x134D2 — the cab
        // world-map before the field loop.
        cab_frame_ = 0;
        scene_ = NativeScene::CabMap;
    }
}

void NativePortShell::update_cab_map(const InputState& input) {
    if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
        input.was_pressed(Button::Start) || cab_frame_ >= kCabMapFrames) {
        scene_ = NativeScene::Field;
        if (!field_map_pixels_.empty()) {
            reset_field_state();
        }
        return;
    }
    ++cab_frame_;
}

void NativePortShell::follow_field_camera() {
    if (field_map_pixels_.empty() || field_map_width_ == 0 ||
        field_map_height_ == 0) {
        return;
    }
    const auto max_x = std::max<std::int32_t>(
        0, static_cast<std::int32_t>(field_map_width_) - kFieldViewportWidth);
    const auto max_y = std::max<std::int32_t>(
        0, static_cast<std::int32_t>(field_map_height_) - kFieldViewportHeight);
    const auto desired_x = field_world_x_ - kFieldViewportWidth / 2;
    const auto desired_y = field_world_y_ - kFieldViewportHeight / 2;
    const auto step_toward = [](std::int32_t current, std::int32_t desired) {
        const auto delta = desired - current;
        const auto distance = delta < 0 ? -delta : delta;
        // 0x3A1C/0x3BAC: the camera only writes once the entity is far
        // enough from the 0x700 window, then catches up from the movement
        // table. Host form: 24px deadzone, then a 1/8 catch-up.
        if (distance <= 24) {
            return current;
        }
        auto step = delta / 8;
        if (step == 0) {
            step = delta > 0 ? 1 : -1;
        }
        if (step > 6) {
            step = 6;
        } else if (step < -6) {
            step = -6;
        }
        return current + step;
    };
    field_camera_x_ = std::clamp(step_toward(field_camera_x_, desired_x),
                                 0, max_x);
    field_camera_y_ = std::clamp(step_toward(field_camera_y_, desired_y),
                                 0, max_y);
}

void NativePortShell::open_field_dialog(std::vector<std::string> lines) {
    field_dialog_lines_ = std::move(lines);
    field_dialog_line_index_ = 0;
    scene_ = NativeScene::FieldDialog;
}

void NativePortShell::open_stoker_motel_dialog() {
    // The C0 doorway resolves to mode 0x14. The source interaction callback
    // queued at ROM 0x20E48 uses the following text resources at ROM
    // 0x1167EB..0x116A79. Street vendors and other talk events reuse this
    // same C0DA panel (0xC3E6 28x21 at $C186).
    open_field_dialog({
        "<The battered sign reads Stoker's Coffin Motel.",
        "Motels like these are cheap, each room only large enough to fit a small bed.",
        "Dumps like these tend to attract the dregs of humanity.>",
        "<Stoker squints at you with his one good eye.",
        "So whadda' ya' want?",
        "I got a room for 20$, up front.",
        "You interested?",
    });
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

    const auto requested_heading = source_heading_for_input(delta_x, delta_y);
    if (!requested_heading) {
        return;
    }
    field_entity_heading_target_ = *requested_heading;
    field_entity_heading_current_ = slew_source_heading(
        field_entity_heading_current_, field_entity_heading_target_);
    field_entity_direction_ = source_direction_index(
        field_entity_heading_current_) % field_direction_table_.size();
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
    pocket_secretary_selection_ = 0;
    pocket_secretary_save_load_enabled_ = true;
    field_menu_action_active_ = false;
    field_menu_action_selection_ = 2;
    field_discard_confirmation_active_ = false;
    field_discard_confirmation_yes_ = false;
    field_discard_item_was_equipped_ = false;
    field_discard_cannot_discard_ = false;
    field_trade_active_ = false;
    field_trade_confirmation_yes_ = false;
    field_trade_cannot_trade_ = false;
    field_trade_full_inventory_ = false;
    field_trade_full_inventory_message_.clear();
    field_first_aid_active_ = false;
    field_first_aid_message_active_ = false;
    field_first_aid_target_count_ = 0;
    field_first_aid_target_index_ = 0;
    field_first_aid_message_.clear();
    field_item_message_active_ = false;
    field_item_message_.clear();
    field_last_command_selection_ = 0;
    field_last_command_row_ = 0;
    field_command_message_frames_ = 0;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::open_field_inventory_action() {
    if (field_menu_selection_ >= 2U) {
        return;
    }
    const auto slot = field_menu_row_ * 2U + field_menu_selection_;
    if (slot >= field_entity_.inventory_item_codes.size() ||
        field_entity_.inventory_item_codes[slot] == 0U) {
        // ROM 0x11740 returns immediately when 0x12148 finds an empty slot.
        return;
    }
    // ROM 0x11740 saves the parent selector, changes the visible selector to
    // two, and enters 0x117D4. The first action press only opens this loop;
    // 0x12168 is reached by a later A/B confirmation.
    field_menu_action_active_ = true;
    field_menu_action_selection_ = 2;
    field_discard_confirmation_active_ = false;
    field_discard_confirmation_yes_ = false;
    field_discard_item_was_equipped_ = false;
    field_discard_cannot_discard_ = false;
    field_trade_active_ = false;
    field_trade_confirmation_yes_ = false;
    field_trade_cannot_trade_ = false;
    field_trade_full_inventory_ = false;
    field_trade_full_inventory_message_.clear();
    field_first_aid_active_ = false;
    field_first_aid_message_active_ = false;
    field_first_aid_target_count_ = 0;
    field_first_aid_target_index_ = 0;
    field_first_aid_message_.clear();
    field_item_message_active_ = false;
    field_item_message_.clear();
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::open_field_first_aid() {
    const auto source_slot = field_menu_row_ * 2U + field_menu_selection_;
    if (source_slot >= field_entity_.inventory_item_codes.size()) {
        return;
    }
    const auto item_code = field_entity_.inventory_item_codes[source_slot];
    if (item_code < 0x10U || item_code > 0x13U) {
        return;
    }

    // 0x122C8/0x122EC/0x1231C walk the three 0x100-byte party records in
    // order and test only +0x10 bit 0. This is deliberately not
    // runner_switch_candidate(): First Aid can target an active record even
    // when the separate C-button roster gate rejects it.
    field_first_aid_target_count_ = 0;
    for (std::size_t record = 0;
         record < field_records_.size() &&
             field_first_aid_target_count_ <
                 field_first_aid_target_records_.size();
         ++record) {
        if ((field_records_[record].flags_10 & 0x01U) == 0U) {
            continue;
        }
        field_first_aid_target_records_[field_first_aid_target_count_++] =
            record;
    }
    field_first_aid_target_index_ = 0;
    field_first_aid_message_.clear();
    field_first_aid_message_active_ = false;
    field_first_aid_active_ = true;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::apply_field_first_aid() {
    const auto source_slot = field_menu_row_ * 2U + field_menu_selection_;
    if (source_slot >= field_entity_.inventory_item_codes.size() ||
        field_first_aid_target_index_ >= field_first_aid_target_count_) {
        return;
    }
    const auto item_code = field_entity_.inventory_item_codes[source_slot];
    if (item_code < 0x10U || item_code > 0x13U) {
        return;
    }

    const auto target_record =
        field_first_aid_target_records_[field_first_aid_target_index_];
    if (target_record >= field_entity_records_.size() ||
        target_record >= field_records_.size()) {
        return;
    }
    auto& target = target_record == active_field_record_
        ? field_entity_ : field_entity_records_[target_record];
    // `field_records_` is the native projection of the same +0x10 flag byte
    // used by the source target pointer. Keep the entity copy synchronized
    // before the 0x3674/0x37EC active-bit tests.
    target.flags_10 = field_records_[target_record].flags_10;

    constexpr std::string_view kNotInjured = "This person is not injured!";
    constexpr std::string_view kSlapPatchUsed = "Slap-patch used.";
    constexpr std::string_view kMedkitUsed = "Medkit used.";

    // 0x3674 and 0x37EC are the source physical/mental writers. Their
    // positive First Aid inputs clamp at ten and only apply to an active
    // record. The target list already guarantees the active bit, but retain
    // the branch because the original helpers perform it themselves.
    const auto add_physical = [&target](std::uint8_t amount) {
        if ((target.flags_10 & 0x01U) == 0U) {
            return;
        }
        target.physical_value = static_cast<std::uint8_t>(std::min<int>(
            10, static_cast<int>(target.physical_value) + amount));
    };
    const auto add_mental = [&target](std::uint8_t amount) {
        if ((target.flags_10 & 0x01U) == 0U) {
            return;
        }
        target.mental_value = static_cast<std::uint8_t>(std::min<int>(
            10, static_cast<int>(target.mental_value) + amount));
    };

    bool used = false;
    if (item_code == 0x10U) {
        // 0x12548/0x12552 rejects a fully healed target before consuming the
        // medkit.  The base threshold is the literal 0x12572 branch, then
        // Body (+0x77) / 3 is subtracted and the result is clamped to 2.
        if (target.physical_value == 10U && target.mental_value == 10U) {
            field_first_aid_message_ = std::string(kNotInjured);
        } else {
            const auto base = target.physical_value >= 7U
                ? 4U : target.physical_value >= 4U ? 6U : 8U;
            const auto body_penalty =
                static_cast<unsigned>(target.attribute_values[0]) / 3U;
            const auto threshold = static_cast<unsigned>(
                std::max<int>(2, static_cast<int>(base) -
                                     static_cast<int>(body_penalty)));

            // Native form of 0x0DBA/0x0D3A. Each Biotech die rolls 1..6;
            // sixes explode into the shared D2 bonus, and a die succeeds
            // when its total reaches the computed threshold. The source
            // always executes one die when the skill byte is zero.
            const auto biotech = static_cast<unsigned>(
                field_entity_.skill_values[8]);
            const auto rolls = std::max(1U, biotech);
            unsigned exploding_bonus = 0;
            unsigned amount = 0;
            const auto random_six = [this]() {
                first_aid_random_state_ = static_cast<std::uint16_t>(
                    static_cast<std::uint32_t>(first_aid_random_state_) *
                        0x20E5U + 1U);
                return static_cast<unsigned>(
                    (static_cast<std::uint32_t>(first_aid_random_state_) * 6U) >>
                    16U) + 1U;
            };
            for (unsigned roll = 0; roll < rolls; ++roll) {
                unsigned die = 0;
                do {
                    die = random_six();
                    if (die == 6U) {
                        exploding_bonus += die;
                    }
                } while (die == 6U);
                if (die + exploding_bonus >= threshold) {
                    ++amount;
                }
            }
            // 0x124D8 converts a zero result to one before calling the two
            // source stat writers.
            const auto heal_amount = static_cast<std::uint8_t>(
                std::max(1U, amount));
            add_physical(heal_amount);
            add_mental(heal_amount);
            target.flags_10 = static_cast<std::uint8_t>(
                (target.flags_10 & static_cast<std::uint8_t>(~0x50U)) |
                0x01U);
            target.state_pointer = 0xB45CU;
            field_first_aid_message_ = std::string(kMedkitUsed);
            used = true;
        }
    } else if (item_code == 0x11U) {
        // 0x1247E/0x1248C is the mental-health slap-patch branch. D1=2 is
        // the source's fixed twenty-percent step on the 0..10 scale.
        if (target.mental_value == 10U) {
            field_first_aid_message_ = std::string(kNotInjured);
        } else {
            add_mental(2U);
            if (target.physical_value != 0U) {
                target.flags_10 = static_cast<std::uint8_t>(
                    (target.flags_10 & static_cast<std::uint8_t>(~0x50U)) |
                    0x01U);
            }
            field_first_aid_message_ = std::string(kSlapPatchUsed);
            used = true;
        }
    } else {
        // Codes 0x12 and 0x13 share the 0x12466 physical patch path. The
        // source checks physical health only, then adds D1=2.
        if (target.physical_value == 10U) {
            field_first_aid_message_ = std::string(kNotInjured);
        } else {
            add_physical(2U);
            if (target.mental_value != 0U) {
                target.flags_10 = static_cast<std::uint8_t>(
                    (target.flags_10 & static_cast<std::uint8_t>(~0x50U)) |
                    0x01U);
            }
            field_first_aid_message_ = std::string(kSlapPatchUsed);
            used = true;
        }
    }

    if (!used) {
        field_first_aid_message_active_ = true;
        return;
    }

    // 0x124AE/0x124DC decrements the selected possession phase. A phase of
    // one is consumed through the same 0x1275C source removal path; medical
    // items cannot be live weapon/armor equipment.
    auto& source_phase = field_entity_.inventory_phase_bytes[source_slot];
    if (source_phase > 1U) {
        --source_phase;
    } else {
        field_entity_.inventory_item_codes[source_slot] = 0U;
        source_phase = 0U;
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
    field_entity_records_[active_field_record_] = field_entity_;

    if (target_record != active_field_record_) {
        field_entity_records_[target_record] = target;
    }
    field_records_[target_record].flags_10 = target.flags_10;
    if (target_record == active_field_record_) {
        field_entity_.flags_10 = target.flags_10;
        field_entity_records_[active_field_record_] = field_entity_;
    }
    refresh_field_entity_projection();
    field_first_aid_message_active_ = true;
}

void NativePortShell::update_field_first_aid(const InputState& input) {
    if (field_first_aid_message_active_) {
        // 0x124F8 acknowledges the result before 0x12538 returns to the
        // command overview. Keep the message visible until that edge.
        if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
            input.was_pressed(Button::Start)) {
            field_first_aid_active_ = false;
            field_first_aid_message_active_ = false;
            field_first_aid_message_.clear();
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2U;
        }
        return;
    }

    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step && field_first_aid_target_count_ != 0U) {
        const auto row_count = field_first_aid_target_count_ + 1U;
        if (input.is_held(Button::Up) && !input.is_held(Button::Down)) {
            field_first_aid_target_index_ =
                field_first_aid_target_index_ == 0U
                    ? row_count - 1U : field_first_aid_target_index_ - 1U;
        } else if (input.is_held(Button::Down) &&
                   !input.is_held(Button::Up)) {
            field_first_aid_target_index_ =
                (field_first_aid_target_index_ + 1U) % row_count;
        }
    }

    if (input.was_pressed(Button::A)) {
        if (field_first_aid_cancel_selected()) {
            field_first_aid_active_ = false;
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2U;
        } else {
            apply_field_first_aid();
        }
        return;
    }
    if (input.was_pressed(Button::B) || input.was_pressed(Button::Start)) {
        field_first_aid_active_ = false;
        field_menu_action_active_ = false;
        field_menu_action_selection_ = 2U;
    }
}

void NativePortShell::open_field_discard_confirmation() {
    const auto slot = field_menu_row_ * 2U + field_menu_selection_;
    if (slot >= field_entity_.inventory_item_codes.size()) {
        return;
    }
    const auto item_code = field_entity_.inventory_item_codes[slot];
    if (item_code == 0U) {
        return;
    }

    field_discard_item_was_equipped_ =
        field_entity_.display_state >= 0 &&
        static_cast<std::size_t>(field_entity_.display_state) == slot;
    field_discard_cannot_discard_ =
        slot == 7U && item_code >= 0x29U && item_code <= 0x2BU;
    if (field_discard_cannot_discard_) {
        // ROM 0x12634 takes this protected-item branch before entering the
        // Yes/No loop and sends D1FFA through the common acknowledgement path.
        field_discard_confirmation_active_ = false;
        field_discard_confirmation_yes_ = false;
        field_menu_action_active_ = true;
        field_menu_action_selection_ = 2;
        return;
    }

    // ROM 0x12694 clears FFF114 and 0x127B0 toggles it before the first
    // draw, so the initial visible choice is No. Left/right toggles the
    // source choice; it is not a hidden immediate deletion.
    field_discard_confirmation_active_ = true;
    field_discard_confirmation_yes_ = false;
}

void NativePortShell::discard_selected_inventory_item() {
    const auto slot = field_menu_row_ * 2U + field_menu_selection_;
    if (slot >= field_entity_.inventory_item_codes.size()) {
        return;
    }
    const auto item_code = field_entity_.inventory_item_codes[slot];
    if (item_code == 0U) {
        return;
    }

    // Native form of 0x1275C. Possession (+0x66/+0x6E) is cleared
    // independently from live equipment. If this cell is the equipped
    // weapon, the source also clears +0x56/+0x57/+0x4E. Armor uses the
    // separate +0x76 ordinal and is cleared when this cell owns it.
    const auto equipped_weapon =
        field_entity_.display_state >= 0 &&
        static_cast<std::size_t>(field_entity_.display_state) == slot;
    const auto equipped_armor =
        item_code >= 0x2CU &&
        field_entity_.equipped_armor_index == item_code - 0x2BU;
    if (equipped_weapon) {
        field_entity_.display_state = 0x7F;
        field_entity_.display_substate = 0;
        field_entity_.active_item_code = 0;
        field_entity_.ammo_rounds = 0;
    }
    if (equipped_armor) {
        field_entity_.equipped_armor_index = 0;
    }
    field_entity_.inventory_item_codes[slot] = 0;
    field_entity_.inventory_phase_bytes[slot] = 0;

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
    field_equipped_armor_by_record_[active_field_record_] =
        field_entity_.equipped_armor_index;
    field_entity_records_[active_field_record_] = field_entity_;
    refresh_field_entity_projection();
}

void NativePortShell::open_field_trade() {
    const auto source_slot = field_menu_row_ * 2U + field_menu_selection_;
    if (source_slot >= field_entity_.inventory_item_codes.size()) {
        return;
    }
    const auto item_code = field_entity_.inventory_item_codes[source_slot];
    if (item_code == 0U) {
        return;
    }

    field_trade_cannot_trade_ = item_code >= 0x17U && item_code < 0x2CU;
    field_trade_full_inventory_ = false;
    field_trade_full_inventory_message_.clear();
    field_trade_active_ = false;
    field_trade_confirmation_yes_ = false;
    field_trade_recipient_record_ = active_field_record_;
    field_trade_recipient_slot_ = 0;

    // ROM 0x12836 rejects the inclusive 0x17..0x2B range before it scans
    // party records. Those are the source magic/special items and must not be
    // silently moved as ordinary inventory.
    if (field_trade_cannot_trade_) {
        field_trade_active_ = true;
        return;
    }

    // 0x1288E advances by 0x100-byte records, wraps at the type boundary,
    // and accepts only records with the source active bit. The native roster
    // already contains those reviewed record flags, so use them directly.
    for (std::size_t step = 1; step <= field_records_.size(); ++step) {
        const auto candidate =
            (active_field_record_ + step) % field_records_.size();
        if (candidate == active_field_record_ ||
            !runner_switch_candidate(field_records_[candidate])) {
            continue;
        }
        field_trade_recipient_record_ = candidate;
        const auto& recipient = field_entity_records_[candidate];
        for (std::size_t slot = 0;
             slot < recipient.inventory_item_codes.size(); ++slot) {
            if (recipient.inventory_item_codes[slot] == 0U) {
                field_trade_recipient_slot_ = slot;
                field_trade_active_ = true;
                return;
            }
        }
        // The source displays the first eligible recipient's name followed by
        // D1CE1/D1CEE rather than pretending that another recipient was
        // selected. Preserve that failure as a visible acknowledgement.
        field_trade_full_inventory_ = true;
        field_trade_full_inventory_message_ =
            field_menu_runner_names_[candidate] + " has a full inventory.";
        field_trade_active_ = true;
        return;
    }

    // No eligible party record is the same sound-only/no-mutation boundary as
    // the source roster gate. Do not invent a new error string: leave the
    // action loop open with the source item untouched so another A edge can
    // retry after the roster changes.
    field_trade_full_inventory_ = false;
    field_trade_active_ = false;
}

void NativePortShell::trade_selected_inventory_item() {
    const auto source_slot = field_menu_row_ * 2U + field_menu_selection_;
    if (source_slot >= field_entity_.inventory_item_codes.size() ||
        field_trade_recipient_record_ >= field_entity_records_.size()) {
        return;
    }
    auto& recipient = field_entity_records_[field_trade_recipient_record_];
    if (field_trade_recipient_slot_ >= recipient.inventory_item_codes.size()) {
        return;
    }
    const auto item_code = field_entity_.inventory_item_codes[source_slot];
    if (item_code == 0U || recipient.inventory_item_codes[
                                field_trade_recipient_slot_] != 0U) {
        return;
    }

    // Native form of 0x129FE. Possession and phase bytes move together for
    // ordinary items. Armor has no transferable phase, and an equipped source
    // weapon/armor is cleared on the source record while the item is moved.
    recipient.inventory_item_codes[field_trade_recipient_slot_] = item_code;
    if (item_code >= 0x2CU) {
        recipient.inventory_phase_bytes[field_trade_recipient_slot_] = 0U;
        const auto armor_index = static_cast<std::uint8_t>(item_code - 0x2BU);
        if (field_entity_.equipped_armor_index == armor_index) {
            field_entity_.equipped_armor_index = 0U;
        }
    } else {
        recipient.inventory_phase_bytes[field_trade_recipient_slot_] =
            field_entity_.inventory_phase_bytes[source_slot];
    }
    if (field_entity_.display_state >= 0 &&
        static_cast<std::size_t>(field_entity_.display_state) == source_slot) {
        field_entity_.display_state = 0x7F;
        field_entity_.display_substate = 0;
        field_entity_.active_item_code = 0;
        field_entity_.ammo_rounds = 0;
    }
    field_entity_.inventory_item_codes[source_slot] = 0U;
    field_entity_.inventory_phase_bytes[source_slot] = 0U;

    const auto source_bars = compute_command_bar_indices(
        field_entity_, inventory_items_, field_entity_.menu_status_index, 6, 0);
    field_entity_.attack_bar_index = source_bars.attack;
    field_entity_.defense_bar_index = source_bars.defense;
    field_entity_.attack_bar_indices_by_posture[
        field_entity_.menu_status_index] = source_bars.attack;
    field_entity_.defense_bar_indices_by_posture[
        field_entity_.menu_status_index] = source_bars.defense;
    field_entity_records_[active_field_record_] = field_entity_;

    const auto recipient_bars = compute_command_bar_indices(
        recipient, inventory_items_, recipient.menu_status_index, 6, 0);
    recipient.attack_bar_index = recipient_bars.attack;
    recipient.defense_bar_index = recipient_bars.defense;
    recipient.attack_bar_indices_by_posture[
        recipient.menu_status_index] = recipient_bars.attack;
    recipient.defense_bar_indices_by_posture[
        recipient.menu_status_index] = recipient_bars.defense;
    field_equipped_armor_by_record_[active_field_record_] =
        field_entity_.equipped_armor_index;
    field_equipped_armor_by_record_[field_trade_recipient_record_] =
        recipient.equipped_armor_index;
    refresh_field_entity_projection();
}

void NativePortShell::update_field_trade(const InputState& input) {
    if (field_trade_cannot_trade_ || field_trade_full_inventory_) {
        if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
            input.was_pressed(Button::Start)) {
            field_trade_active_ = false;
            field_trade_cannot_trade_ = false;
            field_trade_full_inventory_ = false;
            field_trade_full_inventory_message_.clear();
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2U;
        }
        return;
    }

    const auto horizontal_step =
        menu_direction_step(input, Button::Left, Button::Right, true);
    if (horizontal_step &&
        ((input.is_held(Button::Left) && !input.is_held(Button::Right)) ||
         (input.is_held(Button::Right) && !input.is_held(Button::Left)))) {
        // 0x129B2 calls the same 0x127B0 writer used by Discard. The source
        // starts at No, and the horizontal edge toggles the visible choice.
        field_trade_confirmation_yes_ = !field_trade_confirmation_yes_;
    }
    if (input.was_pressed(Button::A)) {
        if (field_trade_confirmation_yes_) {
            trade_selected_inventory_item();
            field_trade_active_ = false;
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2U;
        }
        return;
    }
    if (input.was_pressed(Button::B) || input.was_pressed(Button::Start)) {
        field_trade_active_ = false;
        field_menu_action_active_ = false;
        field_menu_action_selection_ = 2U;
    }
}

void NativePortShell::update_field_inventory_action(const InputState& input) {
    if (field_first_aid_active_) {
        update_field_first_aid(input);
        return;
    }
    if (field_item_message_active_) {
        // 0x121C0 tail-calls 0x0D94 after writing D202F. The source waits for
        // an acknowledgement before the parent command loop is redrawn.
        if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
            input.was_pressed(Button::Start)) {
            field_item_message_active_ = false;
            field_item_message_.clear();
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2;
        }
        return;
    }
    if (field_trade_active_) {
        update_field_trade(input);
        return;
    }
    if (field_discard_cannot_discard_) {
        // The source's common acknowledgement path keeps this result separate
        // from the action selector and never deletes the protected item.
        if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
            input.was_pressed(Button::Start)) {
            field_discard_cannot_discard_ = false;
            field_menu_action_active_ = false;
        }
        return;
    }

    if (field_discard_confirmation_active_) {
        const auto horizontal_step =
            menu_direction_step(input, Button::Left, Button::Right, true);
        if (horizontal_step &&
            ((input.is_held(Button::Left) &&
              !input.is_held(Button::Right)) ||
             (input.is_held(Button::Right) &&
              !input.is_held(Button::Left)))) {
            // ROM 0x12704 calls 0x127B0 for either horizontal direction;
            // that routine toggles FFF114 between Yes and No.
            field_discard_confirmation_yes_ =
                !field_discard_confirmation_yes_;
        }
        if (input.was_pressed(Button::A) || input.was_pressed(Button::B)) {
            if (field_discard_confirmation_yes_) {
                discard_selected_inventory_item();
            }
            field_discard_confirmation_active_ = false;
            field_discard_confirmation_yes_ = false;
            field_discard_item_was_equipped_ = false;
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2;
        }
        return;
    }

    // ROM 0x118C6 changes the action selector only on left/right and wraps
    // 2..5. Up/down do not move the parent inventory cursor in this loop.
    const auto horizontal_step =
        menu_direction_step(input, Button::Left, Button::Right, true);
    if (horizontal_step && input.is_held(Button::Left) &&
        !input.is_held(Button::Right)) {
        field_menu_action_selection_ = field_menu_action_selection_ == 2U
            ? 5U : field_menu_action_selection_ - 1U;
    } else if (horizontal_step && input.is_held(Button::Right) &&
               !input.is_held(Button::Left)) {
        field_menu_action_selection_ = field_menu_action_selection_ == 5U
            ? 2U : field_menu_action_selection_ + 1U;
    }

    // Source A and B both enter the selected action dispatch at 0x1185E.
    if (input.was_pressed(Button::A) || input.was_pressed(Button::B)) {
        if (field_menu_action_selection_ == 2U) {
            const auto slot = field_menu_row_ * 2U + field_menu_selection_;
            const auto item_code = slot < field_entity_.inventory_item_codes.size()
                ? field_entity_.inventory_item_codes[slot] : 0U;
            if (item_code >= 0x10U && item_code <= 0x13U) {
                // ROM 0x11740 selects D1E6C for medical codes and dispatches
                // the target-selection routine at 0x12298 on the second
                // action edge. It must not fall through to weapon equip.
                open_field_first_aid();
            } else {
                apply_field_inventory_action();
                if (!field_item_message_active_) {
                    field_menu_action_active_ = false;
                    field_menu_action_selection_ = 2;
                }
            }
        } else if (field_menu_action_selection_ == 3U) {
            open_field_discard_confirmation();
        } else if (field_menu_action_selection_ == 4U) {
            open_field_trade();
        } else if (field_menu_action_selection_ == 5U) {
            // Cancel is the source loop's return path; the saved inventory
            // column/row remains selected when the parent redraws.
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2;
        }
        // Discard and Trade dispatch through separate source handlers. Keep
        // their visible action focus intact until those handlers are ported;
        // silently mutating or deleting an item here would invent behavior.
    }

    // The source Start path restores the saved parent selector and redraws
    // the command screen; it does not leave the field menu.
    if (input.was_pressed(Button::Start)) {
        field_menu_action_active_ = false;
        field_menu_action_selection_ = 2;
        field_discard_confirmation_active_ = false;
        field_discard_confirmation_yes_ = false;
        field_discard_item_was_equipped_ = false;
        field_discard_cannot_discard_ = false;
    }
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
    screen.clear({0x00, 0x00, 0x00, 0xFF});
    if (!startup_background_.pixels.empty()) {
        screen.blit_rgba(0, 0, startup_background_.width,
                         startup_background_.height,
                         startup_background_.pixels);
    }
}

void NativePortShell::open_pocket_secretary_menu() {
    // ROM 0x10E4E dispatches selector six to 0xC2A6. The source cursor is
    // reset by the panel entry path, while the field command selector remains
    // six so return restores Pocket Secretary rather than an unrelated row.
    scene_ = NativeScene::PocketSecretaryMenu;
    pocket_secretary_selection_ = 0;
    pocket_secretary_message_visible_ = false;
    pocket_secretary_save_load_enabled_ = true;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::update_pocket_secretary_menu(const InputState& input) {
    const auto option_count = pocket_secretary_option_count();
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step && input.is_held(Button::Up) &&
        !input.is_held(Button::Down)) {
        pocket_secretary_message_visible_ = false;
        pocket_secretary_selection_ = pocket_secretary_selection_ == 0U
            ? option_count - 1U : pocket_secretary_selection_ - 1U;
    } else if (vertical_step && input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        pocket_secretary_message_visible_ = false;
        pocket_secretary_selection_ =
            (pocket_secretary_selection_ + 1U) % option_count;
    }

    // C2A6's ordinary panel returns to the command loop on the same B/Start
    // boundary as the other command screens. The source A dispatches the
    // selected row; only Save/Load is currently a reconstructed destination.
    if (input.was_pressed(Button::Start) || input.was_pressed(Button::B)) {
        pocket_secretary_message_visible_ = false;
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 6;
        return;
    }
    if (!input.was_pressed(Button::A)) {
        return;
    }

    if (pocket_secretary_save_load_enabled_ &&
        pocket_secretary_selection_ == 4U) {
        // The source Save/Load row enters the same two-slot 0x59172 screen;
        // it is not a New Game shortcut and does not alter field state here.
        scene_ = NativeScene::SaveLoad;
        save_load_selection_ = 0;
        save_load_message_visible_ = false;
        save_load_message_success_ = false;
        save_load_from_pocket_secretary_ = true;
        pocket_secretary_message_visible_ = false;
        return;
    }
    if (pocket_secretary_selection_ == 1U) {
        // ROM 0xC8C8 is a modal catalog. It reads FFFC22, emits only the
        // entries whose bits are set, or the exact empty-catalog message,
        // then waits for any controller edge before returning to 0x10D30.
        scene_ = NativeScene::GroupItemsMenu;
        pocket_secretary_message_visible_ = false;
        return;
    }
    if (pocket_secretary_selection_ == 2U) {
        // ROM 0x18150 first tests FF018F bit 0. The native new-game state has
        // no deck until the corresponding source acquisition path is ported;
        // preserve the source feedback instead of opening an invented Matrix
        // screen. The handler returns to C2A6, so the Pocket Secretary stays
        // open and the message is an in-panel response.
        pocket_secretary_message_visible_ = true;
        return;
    }
    if (pocket_secretary_selection_ == 3U) {
        open_dismiss_runner_menu();
        return;
    }
    if (pocket_secretary_selection_ == 0U) {
        open_notebook_menu();
    }
}

void NativePortShell::open_notebook_menu() {
    // ROM 0xC2A6 dispatches Notebook to 0xF236. F33E rebuilds the panel,
    // resets FFF0DC, and starts with the two base rows from C567E. New Game
    // has the Contacts gate set by 0x59C48; later event setters can expose
    // the remaining two rows once their source mutations are ported.
    scene_ = NativeScene::NotebookMenu;
    notebook_selection_ = 0;
    notebook_option_count_ = 2U + static_cast<std::size_t>(std::count_if(
        notebook_event_gates_.begin(), notebook_event_gates_.end(),
        [](const auto gate) { return gate != 0U; }));
    notebook_message_visible_ = false;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::open_notebook_tips_menu() {
    // ROM 0xF9A4 builds FFEF56 by walking each pointer table in source order
    // while testing D7 from 31 down to 0. Thus table entry zero is controlled
    // by the most-significant flag bit, not by bit zero. Keep that ordering in
    // the host list so page selection follows the source bytecode exactly.
    notebook_visible_tip_count_ = 0;
    for (std::size_t group = 0;
         group < notebook_tip_flags_.size(); ++group) {
        for (std::size_t entry = 0; entry < 32U; ++entry) {
            const auto source_bit = 31U - entry;
            if ((notebook_tip_flags_[group] &
                 (static_cast<std::uint32_t>(1U) << source_bit)) == 0U) {
                continue;
            }
            const auto index = group * 32U + entry;
            if (notebook_visible_tip_count_ >=
                notebook_visible_tip_indices_.size()) {
                return;
            }
            notebook_visible_tip_indices_[notebook_visible_tip_count_++] =
                index;
        }
    }
    scene_ = NativeScene::NotebookTipsMenu;
    notebook_tip_selection_ = 0;
    notebook_message_visible_ = false;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::update_notebook_menu(const InputState& input) {
    // F236 uses the shared C376 selector helper with D3=FFF0B9, so the
    // source range is inclusive and wraps between the visible rows.
    if (notebook_message_visible_) {
        if (input.pressed != 0U) {
            notebook_message_visible_ = false;
        }
        return;
    }
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step && input.is_held(Button::Up) &&
        !input.is_held(Button::Down)) {
        notebook_selection_ = notebook_selection_ == 0U
            ? notebook_option_count_ - 1U : notebook_selection_ - 1U;
    } else if (vertical_step && input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        notebook_selection_ =
            (notebook_selection_ + 1U) % notebook_option_count_;
    }

    if (input.was_pressed(Button::Start) || input.was_pressed(Button::B)) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 6;
        return;
    }
    if (!input.was_pressed(Button::A)) {
        return;
    }
    if (notebook_selection_ == 0U) {
        // F2E2 checks FFBEC. A fresh native session has no active run, so it
        // writes C56E7 to C30C, waits at F312 for any edge, and rebuilds the
        // Notebook panel. Keep that response modal and source-backed.
        notebook_message_visible_ = true;
        return;
    }
    if (notebook_selection_ == 1U) {
        open_notebook_tips_menu();
        return;
    }
    // The conditional Contacts/Johnson/Shadowrunners handlers are separate
    // source screens and remain an explicit audit boundary. Do not turn a
    // visible label into a generic page until its handler is ported.
}

void NativePortShell::update_notebook_tips_menu(const InputState& input) {
    // F9A4 leaves its selected text page on screen and returns to F236 on A,
    // C, or Start. A zero-page state is valid before the story has set any
    // FFFC30/34/38 bit, so navigation remains a no-op in that state.
    if (notebook_visible_tip_count_ != 0U) {
        const auto vertical_step =
            menu_direction_step(input, Button::Up, Button::Down, false);
        if (vertical_step && input.is_held(Button::Up) &&
            !input.is_held(Button::Down)) {
            notebook_tip_selection_ = notebook_tip_selection_ == 0U
                ? notebook_visible_tip_count_ - 1U
                : notebook_tip_selection_ - 1U;
        } else if (vertical_step && input.is_held(Button::Down) &&
                   !input.is_held(Button::Up)) {
            notebook_tip_selection_ =
                (notebook_tip_selection_ + 1U) % notebook_visible_tip_count_;
        }
    }

    if (input.was_pressed(Button::A) || input.was_pressed(Button::C) ||
        input.was_pressed(Button::Start)) {
        scene_ = NativeScene::NotebookMenu;
        notebook_selection_ = 1U;
        return;
    }
    if (!input.was_pressed(Button::B)) {
        return;
    }
    // F9A4 dispatches B to 0x391A and then redraws the same page. Preserve
    // the source's sound-only/rejection behavior when no other active runner
    // exists, while using the already ported roster switch for real parties.
    if (!field_records_.empty()) {
        ++runner_switch_attempts_;
        const auto previous_record = active_field_record_;
        activate_field_record(next_available_field_record());
        if (active_field_record_ == previous_record) {
            ++runner_switch_rejections_;
        }
        ++action_count_;
    }
}

void NativePortShell::update_group_items_menu(const InputState& input) {
    // ROM F312 waits for any newly asserted controller bit, not only A/B.
    // Returning to FieldMenu mirrors the 0x10D30 command-screen redraw after
    // the C8C8 handler returns through the 0x11740 dispatch.
    if (input.pressed != 0U) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 6;
    }
}

void NativePortShell::open_dismiss_runner_menu() {
    // ROM C448 starts the Dismiss panel with selector zero, then enumerates
    // active runner records around FF0200/FF0300. The native record array is
    // already the extracted host form of those slots; do not turn the three
    // archetype templates into a party list.
    scene_ = NativeScene::DismissRunnerMenu;
    dismiss_runner_selection_ = 0;
    dismiss_runner_candidate_count_ = 0;
    for (std::size_t index = 0; index < field_records_.size(); ++index) {
        if (index == active_field_record_ ||
            (field_records_[index].flags_10 & 0x01U) == 0U ||
            dismiss_runner_candidate_count_ >=
                dismiss_runner_candidate_records_.size()) {
            continue;
        }
        dismiss_runner_candidate_records_[dismiss_runner_candidate_count_++] =
            index;
    }
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::update_dismiss_runner_menu(const InputState& input) {
    const auto option_count = dismiss_runner_option_count();
    const auto vertical_step =
        menu_direction_step(input, Button::Up, Button::Down, false);
    if (vertical_step && input.is_held(Button::Up) &&
        !input.is_held(Button::Down)) {
        dismiss_runner_selection_ = dismiss_runner_selection_ == 0U
            ? option_count - 1U : dismiss_runner_selection_ - 1U;
    } else if (vertical_step && input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        dismiss_runner_selection_ =
            (dismiss_runner_selection_ + 1U) % option_count;
    }

    if (input.was_pressed(Button::Start) || input.was_pressed(Button::B)) {
        scene_ = NativeScene::PocketSecretaryMenu;
        pocket_secretary_selection_ = 3U;
        return;
    }
    if (!input.was_pressed(Button::A)) {
        return;
    }
    // C448 exits immediately when selector zero is chosen. The source label
    // for that row is Abort, so it returns to C2A6 without changing roster
    // state.
    if (dismiss_runner_selection_ == 0U) {
        scene_ = NativeScene::PocketSecretaryMenu;
        pocket_secretary_selection_ = 3U;
        return;
    }

    const auto candidate_index = dismiss_runner_selection_ - 1U;
    if (candidate_index >= dismiss_runner_candidate_count_) {
        return;
    }
    const auto record_index =
        dismiss_runner_candidate_records_[candidate_index];
    // The source dismissal event removes the selected active roster record;
    // the native state keeps the record data but clears its active bit so it
    // can no longer be switched to or shown as a live party member.
    field_records_[record_index].flags_10 &= static_cast<std::uint8_t>(~0x01U);
    field_entity_records_[record_index].flags_10 &=
        static_cast<std::uint8_t>(~0x01U);
    scene_ = NativeScene::PocketSecretaryMenu;
    pocket_secretary_selection_ = 3U;
}

void NativePortShell::update_field_menu(const InputState& input) {
    if (field_menu_action_active_) {
        update_field_inventory_action(input);
        return;
    }

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
            // ROM 0x11740 opens the item-action loop. Its default selection
            // is Equip/Unequip (or First Aid for item codes 0x10..0x13);
            // 0x12168 is intentionally deferred until the next A/B edge.
            open_field_inventory_action();
        } else if (field_menu_selection_ == 2U) {
            open_clips_menu();
        } else if (field_menu_selection_ == 4U) {
            open_cyberware_menu();
        } else if (field_menu_selection_ == 3U) {
            open_attributes_menu();
        } else if (field_menu_selection_ == 5U) {
            open_magic_menu();
        } else if (field_menu_selection_ == 6U) {
            open_pocket_secretary_menu();
        }
        field_command_message_frames_ = 30;
        if (field_menu_selection_ >= 2U) {
            ++field_command_count_;
        }
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
    } else if ((item_code >= 0x10U && item_code <= 0x13U) ||
               item_code == 0x14U || item_code == 0x16U ||
               (item_code >= 0x18U && item_code < 0x2CU)) {
        // ROM 0x11740 routes item codes 0x10..0x13 through the D1E6C
        // First-Aid action table. The 0x121C0 path likewise handles 0x13,
        // 0x14, 0x16, and 0x18..0x2B through the source automatic-use
        // message. 0x121C0 does not consume or equip the item; it clears the
        // panel, writes D202F, and waits for acknowledgement.
        if (item_code == 0x14U || item_code == 0x16U ||
            (item_code >= 0x18U && item_code < 0x2CU)) {
            field_item_message_active_ = true;
            field_item_message_ = "This item is automatically used.";
        }
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
    clips_action_message_.clear();
    clips_out_of_ammo_ = false;
    clips_reload_confirmation_active_ = false;
    clips_reload_confirmation_yes_ = false;
    clips_trade_active_ = false;
    clips_trade_recipient_count_ = 0;
    clips_trade_recipient_index_ = 0;
    clips_trade_recipient_record_ = active_field_record_;
    clips_trade_menu_selection_ = 2;
    clips_trade_quantity_ = 0;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
    menu_horizontal_repeat_mask_ = 0;
    menu_vertical_repeat_mask_ = 0;
}

void NativePortShell::update_clips_menu(const InputState& input) {
    if (clips_trade_active_) {
        update_clips_trade(input);
        return;
    }
    if (clips_reload_confirmation_active_) {
        const auto horizontal_step =
            menu_direction_step(input, Button::Left, Button::Right, true);
        if (horizontal_step &&
            ((input.is_held(Button::Left) && !input.is_held(Button::Right)) ||
             (input.is_held(Button::Right) && !input.is_held(Button::Left)))) {
            // ROM 0x11D48 re-enters 0x127B0 for either horizontal edge.
            clips_reload_confirmation_yes_ =
                !clips_reload_confirmation_yes_;
        }
        if (input.was_pressed(Button::A)) {
            if (clips_reload_confirmation_yes_) {
                reload_one_clip();
            }
            clips_reload_confirmation_active_ = false;
            clips_reload_confirmation_yes_ = false;
        } else if (input.was_pressed(Button::B) ||
                   input.was_pressed(Button::Start)) {
            clips_reload_confirmation_active_ = false;
            clips_reload_confirmation_yes_ = false;
        }
        return;
    }

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
    // count. Keep its exact visible result instead of the old invented
    // "NO CLIPS"/"QUEUED" feedback.
    if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
        input.was_pressed(Button::C)) {
        clips_last_action_selection_ = clips_menu_selection_;
        clips_action_message_.clear();
        clips_out_of_ammo_ = field_entity_.ammo_clips == 0;
        clips_action_message_frames_ = 30;
        if (clips_out_of_ammo_) {
            // ROM 0x12062 writes the two extracted lines at D1AC6/D1AD3.
            clips_action_message_ = "is out of ammo clips!";
        } else if (clips_menu_selection_ == 2U &&
                   (field_entity_.display_state == 0x7F ||
                    field_entity_.active_item_code >= 0x0DU)) {
            // ROM 0x11CCA branches to D1B40 when no weapon is equipped or
            // the equipped item is not a clip-using weapon.
            clips_action_message_ = "This item does not use clips!!";
        } else if (clips_menu_selection_ == 3U) {
            // ROM 0x11DD2 builds the recipient list and enters the nested
            // D1E24 loop. It does not perform a transfer on this first edge.
            open_clips_trade();
        } else if (clips_menu_selection_ == 4U) {
            // ROM 0x11FDE distributes the available clips across the active
            // records; it is distinct from the nested one-recipient Trade.
            distribute_clips();
        } else if (clips_menu_selection_ == 2U) {
            // ROM 0x11CCA accepts only an equipped clip-using weapon here.
            // Its source item range is 1..0x0C; no-equipment and later item
            // codes already received the D1B40 message above.
            if (field_entity_.ammo_rounds != 0U) {
                clips_reload_confirmation_active_ = true;
                clips_reload_confirmation_yes_ = false;
            } else {
                reload_one_clip();
            }
        } else {
            // Selector five is the source Cancel path.
        }
    }

    // The ROM's Start edge restores the saved top-level selector and returns
    // to the command-screen loop. It does not reset the active field record.
    if (input.was_pressed(Button::Start)) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 2;
        field_command_message_frames_ = 0;
        clips_action_message_frames_ = 0;
        clips_action_message_.clear();
        clips_reload_confirmation_active_ = false;
        clips_reload_confirmation_yes_ = false;
    }

    if (clips_action_message_frames_ != 0) {
        --clips_action_message_frames_;
    }
}

void NativePortShell::reload_one_clip() {
    if (field_entity_.ammo_clips == 0U ||
        field_entity_.active_item_code == 0U ||
        field_entity_.active_item_code >= 0x0DU) {
        return;
    }
    // Native form of 0x11D86 -> 0x36FC. The source decrements +0x4F and
    // adds literal 0x26 to +0x4E, clamping the result to item record +0x24.
    --field_entity_.ammo_clips;
    const auto capacity = source_item_factor(
        inventory_items_, field_entity_.active_item_code);
    const auto loaded = static_cast<unsigned>(field_entity_.ammo_rounds) +
                        0x26U;
    field_entity_.ammo_rounds = static_cast<std::uint8_t>(
        std::min<unsigned>(loaded, capacity == 0U ? loaded : capacity));
    field_entity_records_[active_field_record_] = field_entity_;
    clips_action_message_ = "Rock & Roll, chummers!!";
    clips_action_message_frames_ = 30;
    ++clips_action_count_;
}

void NativePortShell::distribute_clips() {
    std::size_t active_count = 0;
    unsigned total_clips = 0;
    for (std::size_t record = 0; record < field_records_.size(); ++record) {
        if ((field_records_[record].flags_10 & 0x01U) == 0U) {
            continue;
        }
        ++active_count;
        total_clips += field_entity_records_[record].ammo_clips;
    }

    if (active_count < 2U) {
        // ROM 0x1200C returns to D1B8A when fewer than two active records
        // participate in the distribution operation.
        clips_action_message_ = "You have no party members to trade with.";
        clips_action_message_frames_ = 30;
        return;
    }

    // 0x1201A performs DIVU by the active count. The quotient is written to
    // every active record and the remainder is added only to records whose
    // +0x10 bit 1 is set. Preserve the source byte arithmetic at the host
    // boundary instead of silently saturating the result.
    const auto quotient = total_clips / active_count;
    const auto remainder = total_clips % active_count;
    for (std::size_t record = 0; record < field_records_.size(); ++record) {
        if ((field_records_[record].flags_10 & 0x01U) == 0U) {
            continue;
        }
        auto distributed = static_cast<unsigned>(
            static_cast<std::uint8_t>(quotient & 0xFFU));
        if ((field_records_[record].flags_10 & 0x02U) != 0U) {
            distributed += remainder;
        }
        field_entity_records_[record].ammo_clips =
            static_cast<std::uint8_t>(distributed & 0xFFU);
    }
    field_entity_ = field_entity_records_[active_field_record_];
    clips_action_message_ = "Exchange complete";
    clips_action_message_frames_ = 30;
    ++clips_action_count_;
}

void NativePortShell::open_clips_trade() {
    clips_trade_recipient_count_ = 0;
    clips_trade_recipient_index_ = 0;
    clips_trade_recipient_record_ = active_field_record_;
    clips_trade_menu_selection_ = 2;
    clips_trade_quantity_ = field_entity_.ammo_clips;

    // ROM 0x11DDE accepts an active record with +0x10 bit 1 clear. The
    // current record is the source of the transfer and is not a recipient.
    for (std::size_t record = 0;
         record < field_records_.size() &&
             clips_trade_recipient_count_ <
                 clips_trade_recipient_records_.size();
         ++record) {
        if (record == active_field_record_ ||
            (field_records_[record].flags_10 & 0x02U) != 0U ||
            (field_records_[record].flags_10 & 0x01U) == 0U) {
            continue;
        }
        clips_trade_recipient_records_[clips_trade_recipient_count_++] =
            record;
    }
    if (clips_trade_recipient_count_ == 0U) {
        // ROM 0x11E14 writes D1B8A and returns to the clips command loop.
        clips_action_message_ = "You have no party members to trade with.";
        clips_action_message_frames_ = 30;
        return;
    }
    clips_trade_recipient_record_ = clips_trade_recipient_records_[0];
    clips_trade_active_ = true;
}

void NativePortShell::update_clips_trade(const InputState& input) {
    const auto horizontal_step =
        menu_direction_step(input, Button::Left, Button::Right, true);
    if (horizontal_step && input.is_held(Button::Left) &&
        !input.is_held(Button::Right)) {
        clips_trade_menu_selection_ = clips_trade_menu_selection_ == 2U
            ? 5U : clips_trade_menu_selection_ - 1U;
    } else if (horizontal_step && input.is_held(Button::Right) &&
               !input.is_held(Button::Left)) {
        clips_trade_menu_selection_ = clips_trade_menu_selection_ == 5U
            ? 2U : clips_trade_menu_selection_ + 1U;
    }

    if (input.was_pressed(Button::A)) {
        switch (clips_trade_menu_selection_) {
        case 2U:
            // Native form of 0x120E4 with D2=1. The source wraps the
            // quantity from the available clip count back to one.
            clips_trade_quantity_ = clips_trade_quantity_ >=
                    field_entity_.ammo_clips
                ? 1U : static_cast<std::uint8_t>(clips_trade_quantity_ + 1U);
            break;
        case 3U:
            // Native form of 0x120E4 with D2=-1. A decrement below one wraps
            // to the source record's complete available clip count.
            clips_trade_quantity_ = clips_trade_quantity_ <= 1U
                ? field_entity_.ammo_clips
                : static_cast<std::uint8_t>(clips_trade_quantity_ - 1U);
            break;
        case 4U: {
            const auto recipient = clips_trade_recipient_record_;
            if (recipient < field_entity_records_.size() &&
                clips_trade_quantity_ != 0U) {
                auto& destination = field_entity_records_[recipient];
                field_entity_.ammo_clips = static_cast<std::uint8_t>(
                    field_entity_.ammo_clips - clips_trade_quantity_);
                destination.ammo_clips = static_cast<std::uint8_t>(
                    destination.ammo_clips + clips_trade_quantity_);
                field_entity_records_[active_field_record_] = field_entity_;
                clips_action_message_ = "Exchange complete";
                clips_action_message_frames_ = 30;
                ++clips_action_count_;
            }
            clips_trade_active_ = false;
            break;
        }
        case 5U:
            // Native form of 0x1209A's next-recipient selection. Keep the
            // quantity unchanged while the recipient pointer changes.
            clips_trade_recipient_index_ =
                (clips_trade_recipient_index_ + 1U) %
                clips_trade_recipient_count_;
            clips_trade_recipient_record_ = clips_trade_recipient_records_[
                clips_trade_recipient_index_];
            break;
        default:
            break;
        }
    }
    if (input.was_pressed(Button::B) || input.was_pressed(Button::Start)) {
        clips_trade_active_ = false;
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

void NativePortShell::open_magic_menu() {
    // ROM 0xFBA6: cmpi.b #2,($9B,A0); bmi skips the panel. +0x9B<2 (Decker)
    // never enters 0x11900/C0DA. Samurai=2 and Gator=3 do.
    scene_ = NativeScene::MagicMenu;
    menu_horizontal_repeat_frames_ = 0;
    menu_vertical_repeat_frames_ = 0;
}

void NativePortShell::update_magic_menu(const InputState& input) {
    if (input.was_pressed(Button::B) || input.was_pressed(Button::Start)) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 5;
        field_command_message_frames_ = 0;
    }
}

void NativePortShell::render_command_base(ScreenWriter& screen) const {
    screen.clear({0x00, 0x00, 0x00, 0xFF});
    if (!field_menu_background_pixels_.empty()) {
        screen.blit_rgba(0, 0, field_menu_background_width_,
                         field_menu_background_height_,
                         field_menu_background_pixels_);
    }
}

void NativePortShell::render_dialog_panel(ScreenWriter& screen) const {
    // 0xC3E6 / 0x12C5A: 28x21 tile fill at $C186. Same panel for motel
    // talk, street vendors, and other C0DA conversations.
    screen.fill_rect(kDialogPanelX, kDialogPanelY, kDialogPanelWidth,
                     kDialogPanelHeight, {0x00, 0x00, 0x00, 0xFF});
}

void NativePortShell::render_field_hud(ScreenWriter& screen) const {
    // Capture t1720: 64px sidebar with headshot, gun/01, empty 00 slots.
    if (!field_hud_.pixels.empty()) {
        screen.blit_rgba(0, 0, field_hud_.width, field_hud_.height,
                         field_hud_.pixels);
    }
    const auto portrait_index =
        selected_archetype_ < field_menu_runner_portraits_.size()
            ? selected_archetype_
            : 0U;
    const auto& portrait = field_menu_runner_portraits_[portrait_index];
    if (!portrait.pixels.empty() && portrait.width >= 24U &&
        portrait.height >= 24U) {
        // HUD is a headshot. The 64x80 command portrait is a full pose; take
        // the top-center of it so the gun/sword stay off this panel.
        const auto src_x = static_cast<std::uint16_t>(
            (portrait.width > 40U) ? (portrait.width - 40U) / 2U : 0U);
        const auto src_w = static_cast<std::uint16_t>(
            std::min<std::uint16_t>(40, portrait.width));
        const auto src_h = static_cast<std::uint16_t>(
            std::min<std::uint16_t>(40, portrait.height));
        std::vector<Rgba8> face(
            static_cast<std::size_t>(src_w) * src_h, Rgba8{0, 0, 0, 0});
        for (std::uint16_t y = 0; y < src_h; ++y) {
            for (std::uint16_t x = 0; x < src_w; ++x) {
                face[static_cast<std::size_t>(y) * src_w + x] =
                    portrait.pixels[static_cast<std::size_t>(y) *
                                        portrait.width +
                                    (src_x + x)];
            }
        }
        const auto dest_x =
            kFieldHudFaceX + (kFieldHudFaceWidth - src_w) / 2;
        const auto dest_y =
            kFieldHudFaceY + (kFieldHudFaceHeight - src_h) / 2;
        screen.blit_rgba(dest_x, dest_y, src_w, src_h, face);
    }
    const auto item_code = field_entity_.active_item_code;
    if (item_code > 0U &&
        item_code <= field_menu_inventory_icons_.size()) {
        const auto& icon = field_menu_inventory_icons_[item_code - 1U][0];
        if (!icon.pixels.empty()) {
            // Capture: ammo on the left, gun icon on the right of the slot.
            const auto icon_x =
                kFieldHudGunX + kFieldHudGunWidth -
                static_cast<std::int32_t>(icon.width) - 4;
            const auto icon_y =
                kFieldHudGunY +
                (kFieldHudGunHeight - static_cast<std::int32_t>(icon.height)) /
                    2;
            screen.blit_rgba(icon_x, icon_y, icon.width, icon.height,
                             icon.pixels);
        }
    }
    write_field_menu_text(screen, kFieldHudGunX + 4, kFieldHudGunY + 12, "01");
    write_field_menu_text(screen, kFieldHudFaceX + 4, 124, "00");
    write_field_menu_text(screen, kFieldHudFaceX + 4, 172, "00");
}

void NativePortShell::finish_boot() {
    for (std::uint32_t step = 0; step < 2048U && scene_ == NativeScene::Bootstrap;
         ++step) {
        update(InputState{});
    }
}

void NativePortShell::update_boot() {
    if (boot_loop_left_ != 0U) {
        // ROM 0x5DAC4: 224 line-scroll words, PRNG 0xD44 mulu #$20E5 addq #1,
        // lsr #5, and #$1F, written to the plane-A half of each HSCROLL pair.
        for (std::size_t line = 0; line < boot_hscroll_.size(); ++line) {
            boot_rng_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(boot_rng_ * kBootRngMul) + 1U);
            boot_hscroll_[line] = static_cast<std::uint16_t>(
                (boot_rng_ >> 5U) & 0x1FU);
        }
        // ROM 0x5D8BA runs only while DD3C is set. DF90 counts 4..-1; on
        // wrap it slides DF92 through 0..7 and copies 0x5DCE6 into CRAM.
        if (boot_dd3c_ != 0 && boot_guy_fade_ != 0U) {
            --boot_df90_;
            if (boot_df90_ < 0) {
                boot_df90_ = 4;
                ++boot_df92_;
                if (boot_df92_ >= 8) {
                    boot_df92_ = 0;
                }
            }
            // 0x5D8BA at DF90==2 calls 0x5B2E2: the chrome figure fades
            // while the SEGA logo shines. Capture then holds static SEGA.
            if (boot_guy_fade_ > 8U) {
                boot_guy_fade_ = static_cast<std::uint16_t>(boot_guy_fade_ - 8U);
            } else {
                boot_guy_fade_ = 0;
            }
        }
        // ROM 0x5D938 draws, then 0x5D7A2 mutates SAT Y / SEGA timers.
        if (boot_guy_y_ != static_cast<std::int16_t>(kBootGuyStopY)) {
            --boot_guy_y_;
            if (boot_guy_y_ == static_cast<std::int16_t>(kBootGuyStopY)) {
                boot_dd30_ = static_cast<std::int16_t>(kBootSegaDelay);
                boot_shock_left_ = kBootShockFrames;
                --boot_dd30_;
            }
        } else {
            --boot_dd30_;
            if (boot_dd30_ < 0) {
                ++boot_dd28_;
                if ((boot_dd28_ & 1) == 0) {
                    --boot_dd2a_;
                    if (boot_dd2a_ == 0) {
                        boot_dd3c_ = 1;
                    }
                }
            }
        }
        if (boot_dd30_ < 0 && boot_dd3c_ == 0) {
            if (boot_shock_left_ != 0U) {
                --boot_shock_left_;
            }
            --boot_sega_cadence_;
            if (boot_sega_cadence_ < 0) {
                boot_sega_cadence_ = 3;
                ++boot_sega_phase_;
                if (boot_sega_phase_ > 5) {
                    boot_sega_phase_ = 0;
                }
            }
        }
        --boot_loop_left_;
        if (boot_loop_left_ == 0U) {
            boot_fade_left_ = kBootFadeFrames;
        }
        return;
    }
    if (boot_fade_left_ != 0U) {
        --boot_fade_left_;
        if (boot_fade_left_ == 0U) {
            // Capture: SEGA fades to black, then FASA fades in.
            boot_fasa_fade_in_left_ = kBootFadeFrames;
        }
        return;
    }
    if (boot_fasa_fade_in_left_ != 0U) {
        --boot_fasa_fade_in_left_;
        if (boot_fasa_fade_in_left_ == 0U) {
            boot_hold_left_ = kBootHoldDbf + 1U;
        }
        return;
    }
    if (boot_hold_left_ != 0U) {
        --boot_hold_left_;
        if (boot_hold_left_ == 0U) {
            // Second 0x131C fades the 0x5D886 FASA plane to black.
            boot_fasa_fade_left_ = kBootFadeFrames;
        }
        return;
    }
    if (boot_fasa_fade_left_ != 0U) {
        --boot_fasa_fade_left_;
        if (boot_fasa_fade_left_ == 0U) {
            scene_ = NativeScene::Title;
            frame_number_ = 0;
        }
    }
}

void NativePortShell::update(const InputState& input) {
    input_ = input;

    if (scene_ == NativeScene::Bootstrap) {
        update_boot();
        if (scene_ == NativeScene::Bootstrap) {
            ++frame_number_;
        }
        return;
    }

    if (scene_ == NativeScene::Title) {
        // 0x1A9E samples Start via 0x0BF2 into FFDDC2, breaks the D7 loop,
        // then 0x131C fades before 0x1BAC returns into 0x59092. One edge.
        if (title_fade_left_ != 0U) {
            --title_fade_left_;
            if (title_fade_left_ == 0U) {
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
            ++frame_number_;
            return;
        }
        if (input.was_pressed(Button::Start)) {
            title_fade_left_ = kBootFadeFrames;
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

    if (scene_ == NativeScene::CabMap) {
        update_cab_map(input);
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

    if (scene_ == NativeScene::PocketSecretaryMenu) {
        update_pocket_secretary_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::NotebookMenu) {
        update_notebook_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::NotebookTipsMenu) {
        update_notebook_tips_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::GroupItemsMenu) {
        update_group_items_menu(input);
        ++frame_number_;
        return;
    }

    if (scene_ == NativeScene::DismissRunnerMenu) {
        update_dismiss_runner_menu(input);
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

    if (scene_ == NativeScene::MagicMenu) {
        update_magic_menu(input);
        ++frame_number_;
        return;
    }

    // ROM 0x195C reduces the D-pad to one of eight valid source nibbles.
    // Opposing directions are not a zero-length movement; they produce the
    // source -1 sentinel and leave the entity in its current state.
    auto input_delta_x = 0;
    auto input_delta_y = 0;
    if (input.is_held(Button::Left) && !input.is_held(Button::Right)) {
        input_delta_x = -1;
    } else if (input.is_held(Button::Right) && !input.is_held(Button::Left)) {
        input_delta_x = 1;
    }
    if (input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        input_delta_y = -1;
    } else if (input.is_held(Button::Down) && !input.is_held(Button::Up)) {
        input_delta_y = 1;
    }
    const auto attempted_movement = source_heading_for_input(
        input_delta_x, input_delta_y).has_value();
    if (attempted_movement && !field_direction_table_.empty()) {
        // This updates +0x34, advances +0x2E through the source 0xB530
        // slew, and selects the same C002 vector used by 0xB77E.
        update_field_entity_direction(input_delta_x, input_delta_y);
    }
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
    auto delta_x = input_delta_x;
    auto delta_y = input_delta_y;
    if (!field_map_pixels_.empty() && attempted_movement) {
        const auto& movement_vector = field_direction_table_[
            field_entity_direction_ % field_direction_table_.size()];
        const auto candidate_x = static_cast<std::uint16_t>(
            static_cast<std::int32_t>(field_entity_.world_x_fixed) +
            movement_vector[0]);
        const auto candidate_y = static_cast<std::uint16_t>(
            static_cast<std::int32_t>(field_entity_.world_y_fixed) +
            movement_vector[1]);
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
    } else if (field_map_pixels_.empty() && attempted_movement) {
        // Bootstrap has no extracted field descriptor. Keep its diagnostic
        // marker moving, but do not let that fallback define the real field
        // movement path above.
        marker_x_ += input_delta_x * 2;
        marker_y_ += input_delta_y * 2;
        marker_x_ = std::clamp(marker_x_, 8,
                               static_cast<std::int32_t>(kScreenWidth) - 16);
        marker_y_ = std::clamp(marker_y_, 48,
                               static_cast<std::int32_t>(kScreenHeight) - 16);
    }
    const auto movement_accepted =
        !field_map_pixels_.empty() && attempted_movement &&
        !move_rejected_ && (delta_x != 0 || delta_y != 0);
    if (attempted_movement) {
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
        // 0x3A1C/0x3BAC rewrite FFDDC6/FFDDC8 so the camera catches up
        // when the runner leaves the 0x700 window. The mode descriptor is
        // the starting camera; catch-up runs on accepted steps.
        if (movement_accepted) {
            follow_field_camera();
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
    if (scene_ == NativeScene::Bootstrap) {
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        const auto showing_fasa =
            boot_loop_left_ == 0U && boot_fade_left_ == 0U &&
            (boot_fasa_fade_in_left_ != 0U || boot_hold_left_ != 0U ||
             boot_fasa_fade_left_ != 0U);
        const auto scale_copy = [](const std::vector<Rgba8>& source,
                                   std::uint16_t remaining,
                                   std::uint16_t total) {
            auto faded = source;
            if (total == 0U || remaining >= total) {
                return faded;
            }
            for (auto& pixel : faded) {
                pixel.red = static_cast<std::uint8_t>(
                    (static_cast<std::uint16_t>(pixel.red) * remaining) /
                    total);
                pixel.green = static_cast<std::uint8_t>(
                    (static_cast<std::uint16_t>(pixel.green) * remaining) /
                    total);
                pixel.blue = static_cast<std::uint8_t>(
                    (static_cast<std::uint16_t>(pixel.blue) * remaining) /
                    total);
            }
            return faded;
        };
        if (showing_fasa && !boot_fasa_.pixels.empty()) {
            std::uint16_t fade_weight = kBootFadeFrames;
            if (boot_fasa_fade_in_left_ != 0U) {
                fade_weight = static_cast<std::uint16_t>(
                    kBootFadeFrames - boot_fasa_fade_in_left_ + 1U);
            } else if (boot_hold_left_ != 0U) {
                fade_weight = kBootFadeFrames;
            } else {
                fade_weight = boot_fasa_fade_left_;
            }
            screen.blit_rgba(0, 0, boot_fasa_.width, boot_fasa_.height,
                             scale_copy(boot_fasa_.pixels, fade_weight,
                                        kBootFadeFrames));
            screen.present();
            return;
        }
        const auto boot_layers_visible =
            boot_loop_left_ != 0U || boot_fade_left_ != 0U;
        if (boot_layers_visible && !boot_background_.pixels.empty()) {
            const auto fade_copy = [&scale_copy, this](
                                       const std::vector<Rgba8>& source) {
                return scale_copy(source,
                                  boot_fade_left_ == 0U ? kBootFadeFrames
                                                        : boot_fade_left_,
                                  kBootFadeFrames);
            };
            if (!boot_static_.pixels.empty() &&
                boot_static_.pixels.size() ==
                    static_cast<std::size_t>(boot_static_.width) *
                        boot_static_.height) {
                std::vector<Rgba8> scrolled(boot_static_.pixels.size(),
                                            Rgba8{0, 0, 0, 0});
                const auto width = static_cast<std::int32_t>(boot_static_.width);
                const auto height =
                    static_cast<std::int32_t>(boot_static_.height);
                for (std::int32_t row = 0; row < height; ++row) {
                    const auto scroll =
                        static_cast<std::int32_t>(boot_hscroll_line(
                            static_cast<std::size_t>(row)));
                    for (std::int32_t column = 0; column < width; ++column) {
                        const auto source_x = column + scroll;
                        if (source_x < 0 || source_x >= width) {
                            continue;
                        }
                        scrolled[static_cast<std::size_t>(row) *
                                     static_cast<std::size_t>(width) +
                                 static_cast<std::size_t>(column)] =
                            boot_static_.pixels
                                [static_cast<std::size_t>(row) *
                                     static_cast<std::size_t>(width) +
                                 static_cast<std::size_t>(source_x)];
                    }
                }
                screen.blit_rgba(0, 0, boot_static_.width, boot_static_.height,
                                 fade_copy(scrolled));
            }
            const NativeImage* logo = &boot_background_;
            if (boot_logo_strobe_active() && !boot_logo_strobe_.empty()) {
                const auto phase =
                    static_cast<std::size_t>(std::max<std::int16_t>(
                        boot_df92_, 0)) %
                    boot_logo_strobe_.size();
                if (!boot_logo_strobe_[phase].pixels.empty()) {
                    logo = &boot_logo_strobe_[phase];
                }
            }
            screen.blit_rgba(0, 0, logo->width, logo->height,
                             fade_copy(logo->pixels));
            if (!boot_guy_.pixels.empty() && boot_guy_fade_ != 0U) {
                const auto guy_delta_y = static_cast<std::int32_t>(
                    boot_guy_y_ - static_cast<std::int16_t>(kBootGuyComposeY));
                auto guy = fade_copy(boot_guy_.pixels);
                if (boot_guy_fade_ < 256U) {
                    guy = scale_copy(guy, boot_guy_fade_, 256U);
                }
                screen.blit_rgba(0, guy_delta_y, boot_guy_.width,
                                 boot_guy_.height, guy);
            }
            if (boot_lightning_visible() && !boot_sega_frames_.empty()) {
                const auto phase = static_cast<std::size_t>(boot_sega_phase_) %
                                   boot_sega_frames_.size();
                const auto& sega = boot_sega_frames_[phase];
                if (!sega.pixels.empty()) {
                    screen.blit_rgba(0, 0, sega.width, sega.height,
                                     fade_copy(sega.pixels));
                }
            }
        }
        screen.present();
        return;
    }
    if (scene_ == NativeScene::Title && !title_pixels_.empty()) {
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        // Host recreation of the 0x1A9E title: start black, explode the
        // extracted logo left-to-right, then fade in the circuit plane.
        // 0x2634 slides one CRAM color through pal line 2 colors 8..15;
        // chips stay put. Do not pulse the traces.
        if (!title_background_.pixels.empty() ||
            !title_logo_plane_.pixels.empty()) {
            constexpr std::uint64_t kTitleRevealStart = 0x5AU;
            constexpr std::size_t kTitleRevealPixelsPerFrame = 4U;
            constexpr std::size_t kCircuitFadeFrames = 16U;
            const auto has_title_loop = frame_number_ != 0U;
            const auto title_loop = has_title_loop ? frame_number_ - 1U : 0U;
            const auto title_fade_weight =
                title_fade_left_ == 0U ? kBootFadeFrames : title_fade_left_;
            auto visible_pixels = std::size_t{0};
            if (has_title_loop && title_loop >= kTitleRevealStart) {
                const auto reveal_frames = static_cast<std::size_t>(
                    title_loop - kTitleRevealStart + 1U);
                visible_pixels = reveal_frames * kTitleRevealPixelsPerFrame;
            }
            const auto logo_width = title_logo_plane_.pixels.empty()
                ? std::size_t{320}
                : title_logo_plane_.width;
            const auto last_visible_x =
                std::min(logo_width, visible_pixels);
            const auto logo_complete = last_visible_x >= logo_width;
            if (logo_complete && !title_background_.pixels.empty()) {
                const auto reveal_end =
                    kTitleRevealStart +
                    (logo_width + kTitleRevealPixelsPerFrame - 1U) /
                        kTitleRevealPixelsPerFrame;
                const auto fade_frames = has_title_loop &&
                        title_loop >= reveal_end
                    ? static_cast<std::size_t>(title_loop - reveal_end)
                    : 0U;
                const auto circuit_weight = std::min(
                    fade_frames + 1U, kCircuitFadeFrames);
                const auto light_phase = static_cast<std::size_t>(
                    (title_loop / kTitleCircuitLightTicks) %
                    kNativeTitleCircuitLightCount);
                auto circuits =
                    !title_circuit_lights_[light_phase].pixels.empty()
                        ? title_circuit_lights_[light_phase].pixels
                        : title_background_.pixels;
                if (circuit_weight < kCircuitFadeFrames) {
                    circuits = scale_rgba(
                        circuits,
                        static_cast<std::uint16_t>(circuit_weight),
                        static_cast<std::uint16_t>(kCircuitFadeFrames));
                }
                screen.blit_rgba(0, 0, title_background_.width,
                                 title_background_.height,
                                 scale_rgba(circuits, title_fade_weight,
                                            kBootFadeFrames));
            }
            if (last_visible_x != 0U && !title_logo_plane_.pixels.empty()) {
                auto logo = title_logo_plane_.pixels;
                // Capture: a hot explode edge leads the L-R reveal and is
                // gone once the ram skull is complete. 4px/frame is the
                // 0x1E4E column-every-two-vblanks average without 8px steps.
                const auto edge_start =
                    logo_complete || last_visible_x < 12U
                        ? last_visible_x
                        : last_visible_x - 12U;
                for (std::size_t y = 0; y < title_logo_plane_.height; ++y) {
                    for (std::size_t x = 0; x < title_logo_plane_.width; ++x) {
                        auto& pixel =
                            logo[y * title_logo_plane_.width + x];
                        if (x >= last_visible_x) {
                            pixel.alpha = 0;
                        } else if (x >= edge_start && pixel.alpha != 0U) {
                            pixel.red = 0xFF;
                            pixel.green = static_cast<std::uint8_t>(
                                std::min<std::uint16_t>(
                                    0xFFU, pixel.green + 0x40U));
                            pixel.blue = static_cast<std::uint8_t>(
                                pixel.blue / 2U);
                        }
                    }
                }
                screen.blit_rgba(0, 0, title_logo_plane_.width,
                                 title_logo_plane_.height,
                                 scale_rgba(logo, title_fade_weight,
                                            kBootFadeFrames));
            }
            if (!title_text_.pixels.empty() && title_fade_left_ == 0U) {
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
                // 0x1E4E sets DD5A=1 when FFF1E6 becomes $25 (column 37).
                // 0x1C1E then counts DD5C from $5A down through -1.
                constexpr auto kTitleCopyrightLoop =
                    kTitleRevealStart + 72U + 0x5BU;
                if (has_title_loop &&
                    title_loop >= kTitleCopyrightLoop) {
                    blit_title_rows(22U, 4U);
                }
                // 0x1A9E writes PRESS START only after D7 drops below $C6C,
                // and only when (D7 & $3F) >= 10. The other 10 of 64 values
                // call 0x1DE0 and leave the prompt blank.
                const auto title_timer = static_cast<std::uint64_t>(
                    0xE10U - std::min<std::uint64_t>(title_loop, 0xE10U));
                if (has_title_loop && title_timer < 0xC6CU &&
                    (title_timer & 0x3FU) >= 10U) {
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
            write_field_menu_text(
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
                write_field_menu_text(
                    screen, 72, 72 + static_cast<std::int32_t>(line.row) * 8,
                    line.text);
            }
        } else {
            for (const auto& line : intro_.save_load_layout) {
                write_field_menu_text(
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
        if (archetype_selection_ < field_menu_runner_portraits_.size()) {
            const auto& portrait =
                field_menu_runner_portraits_[archetype_selection_];
            if (!portrait.pixels.empty()) {
                screen.blit_rgba(80, 56, portrait.width, portrait.height,
                                 portrait.pixels);
            }
        }
        for (const auto& line : intro_.archetype_layout) {
            write_field_menu_text(
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
        // Host recreation of 0x59C5E from extracted stage plates plus the
        // capture's 10 Hz beats: seattle/airport skyline, ambush actors on
        // black (rise, muzzle ticks, white hit flashes, drop), then cockpit.
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        const auto& group = intro_story_group(intro_frame_);
        constexpr std::uint16_t kIntroFade = 16U;
        const auto fade_weight = [this](std::uint64_t appear,
                                        std::uint64_t disappear)
            -> std::uint16_t {
            if (intro_frame_ < appear) {
                return 0;
            }
            if (disappear != 0U && intro_frame_ >= disappear) {
                return 0;
            }
            const auto into = intro_frame_ - appear;
            if (into < kIntroFade) {
                return static_cast<std::uint16_t>(into + 1U);
            }
            if (disappear != 0U && intro_frame_ + kIntroFade >= disappear) {
                return static_cast<std::uint16_t>(disappear - intro_frame_);
            }
            return kIntroFade;
        };
        const auto blit_faded_stage = [&screen](const NativeImage& stage,
                                                std::uint16_t weight) {
            if (stage.pixels.empty() || weight == 0U) {
                return;
            }
            screen.blit_rgba(0, 0, stage.width, stage.height,
                             scale_rgba(stage.pixels, weight, kIntroFade));
        };
        if (intro_frame_ < kIntroSeattleEnd && !intro_stages_[0].pixels.empty()) {
            blit_faded_stage(intro_stages_[0],
                             fade_weight(0, kIntroSeattleEnd));
        } else if (intro_frame_ < kIntroWildernessEnd &&
                   !intro_stages_[1].pixels.empty()) {
            // Capture: three runners on black. Crop the 0x5B4A8 plate and
            // drop grass plus near-neutral dirt, not red-dominant skin.
            const auto local = intro_frame_ - kIntroSeattleEnd;
            const auto pan_frames = kIntroWildernessPan - kIntroSeattleEnd;
            const auto rise = local < pan_frames
                ? static_cast<std::int32_t>(
                      (pan_frames - local) * 80U / std::max<std::uint64_t>(
                          pan_frames, 1U))
                : 0;
            const auto gator_alive =
                intro_frame_ < kIntroWildernessMassacre + 0x50U;
            const auto shaman_alive =
                intro_frame_ < kIntroWildernessMassacre + 0x8CU;
            const auto flash_gator =
                gator_alive && intro_frame_ >= kIntroWildernessMassacre &&
                ((intro_frame_ / 4U) % 2U) == 0U;
            const auto flash_shaman =
                shaman_alive &&
                intro_frame_ >= kIntroWildernessMassacre + 0x50U &&
                ((intro_frame_ / 4U) % 2U) == 0U;
            const auto gator_drop =
                gator_alive ? 0
                            : std::min<std::int32_t>(
                                  80, static_cast<std::int32_t>(
                                          intro_frame_ -
                                          (kIntroWildernessMassacre +
                                           0x50U)) *
                                      4);
            const auto shaman_drop =
                shaman_alive ? 0
                             : std::min<std::int32_t>(
                                   80, static_cast<std::int32_t>(
                                           intro_frame_ -
                                           (kIntroWildernessMassacre +
                                            0x8CU)) *
                                       4);
            const auto& plate = intro_stages_[1];
            blit_intro_actor(screen, plate, 8, 80, 128, 144, 8, 80 + rise,
                             false);
            if (gator_alive || gator_drop < 80) {
                blit_intro_actor(screen, plate, 128, 100, 96, 124, 128,
                                 100 + rise + gator_drop, flash_gator);
            }
            if (shaman_alive || shaman_drop < 80) {
                blit_intro_actor(screen, plate, 200, 8, 112, 136, 200,
                                 8 + rise + shaman_drop, flash_shaman);
            }
            if (local > pan_frames) {
                const auto shot = static_cast<std::int32_t>(
                    (local * 3U) % 220U);
                draw_intro_bolt(screen, 40 + (shot % 180),
                                110 + (shot % 40) / 4);
                draw_intro_bolt(screen, 90 + ((shot * 2) % 160), 150);
            }
        } else if (intro_frame_ < kIntroTrividEnd &&
                   !intro_stages_[2].pixels.empty()) {
            blit_faded_stage(intro_stages_[2],
                             fade_weight(kIntroWildernessEnd,
                                         kIntroTrividEnd));
        } else if (!intro_stages_[0].pixels.empty()) {
            // Capture returns to the seattle skyline for Sea-Tac, not a
            // distinct airport painting. 0x5B08E flies a chopper in and
            // lands it on the left building during the last-million hold.
            blit_faded_stage(intro_stages_[0],
                             fade_weight(kIntroTrividEnd, kIntroDuration));
            // Capture t1525-t1560: small SAT chopper enters from the left
            // and lands on the building cluster left of the Needle.
            // 0x5B180/0x5B08E cycle six SAT lists every 8 vblanks.
            const auto fly = std::min<std::uint64_t>(
                intro_frame_ - kIntroTrividEnd, 180U);
            // Capture t1525 starts left of the skyline; t1545-t1560 lands on
            // the building cluster left of the Needle. 0x5B1B6 adds the
            // 0x5B270 deltas onto the SAT origin after frame $225.
            const auto heli_x = static_cast<std::int32_t>(fly) * 2 / 3;
            const auto heli_y = static_cast<std::int32_t>(fly) / 10;
            if (fade_weight(kIntroTrividEnd, kIntroDuration) != 0U &&
                !intro_heli_frames_.empty()) {
                const auto heli_phase = static_cast<std::size_t>(
                    (intro_frame_ / 8U) % intro_heli_frames_.size());
                const auto& heli = intro_heli_frames_[heli_phase];
                if (!heli.pixels.empty()) {
                    screen.blit_rgba(heli_x, heli_y, heli.width, heli.height,
                                     heli.pixels);
                }
            }
        }
        const auto line_end = std::min(
            group.first_line + group.line_count, intro_.story_lines.size());
        for (auto line = group.first_line; line < line_end; ++line) {
            const auto& text = intro_.story_lines[line];
            const auto text_width = static_cast<std::int32_t>(text.size() * 8U);
            const auto x = std::max<std::int32_t>(
                0, (static_cast<std::int32_t>(kScreenWidth) - text_width) / 2);
            const auto y = group.top_y + static_cast<std::int32_t>(
                line - group.first_line) * 16;
            write_field_menu_text(screen, x, y, text);
        }
        screen.present();
        return;
    } else if (scene_ == NativeScene::CabMap) {
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        constexpr std::int32_t kMapX = 56;
        constexpr std::int32_t kMapY = 32;
        constexpr std::int32_t kMapW = 208;
        constexpr std::int32_t kMapH = 140;
        // Capture t1660/t1675 is the 0x134D2 world map (grass/water with a
        // white district cursor), not a scaled Redmond field plane. Until
        // that 0xD4456 resource is extracted, keep the black cab plate and
        // the source district label instead of the wrong map.
        screen.fill_rect(kMapX, kMapY, kMapW, kMapH, {0x00, 0x00, 0x00, 0xFF});
        screen.fill_rect(kMapX + 70, kMapY + 48, 48, 32,
                         {0xF0, 0xF0, 0xF0, 0xFF});
        screen.fill_rect(kMapX + 72, kMapY + 50, 44, 28,
                         {0x00, 0x00, 0x00, 0xFF});
        const auto cab_label =
            cab_frame_ < 45U ? "Downtown Seattle" : "Redmond Barrens";
        const auto label_x = cab_frame_ < 45U ? 96 : 88;
        write_field_menu_text(screen, label_x, 180, cab_label);
        screen.present();
        return;
    } else if ((scene_ == NativeScene::Field ||
                scene_ == NativeScene::FieldDialog ||
                scene_ == NativeScene::FieldMenu ||
                scene_ == NativeScene::PocketSecretaryMenu ||
                scene_ == NativeScene::NotebookMenu ||
                scene_ == NativeScene::NotebookTipsMenu ||
                scene_ == NativeScene::GroupItemsMenu ||
                scene_ == NativeScene::DismissRunnerMenu ||
                scene_ == NativeScene::ClipsMenu ||
                scene_ == NativeScene::CyberwareMenu ||
                scene_ == NativeScene::AttributesMenu ||
                scene_ == NativeScene::MagicMenu) &&
               !field_view_pixels_.empty()) {
        // The field background is the ROM's mode-2 map translated into a host
        // RGBA viewport. NativeFieldEntity is composited after the terrain,
        // matching the ROM's entity/object ordering at this boundary.
        // 0x3084/0x1406 leave the map plane at the native display origin.
        // Do not paint a host-only title bar over the first map rows.
        screen.clear({0x04, 0x08, 0x14, 0xFF});
        screen.blit_rgba(0, 0, field_view_width_, field_view_height_,
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
        if (scene_ == NativeScene::Field ||
            scene_ == NativeScene::FieldDialog) {
            render_field_hud(screen);
        }
        if (scene_ == NativeScene::FieldDialog) {
            render_dialog_panel(screen);
            const auto& paragraph = field_dialog_lines_[field_dialog_line_index_];
            std::vector<std::string> wrapped;
            std::string current;
            std::string word;
            const auto flush_word = [&]() {
                if (word.empty()) {
                    return;
                }
                if (!current.empty() && current.size() + 1U + word.size() >
                        26U) {
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
            for (std::size_t row = 0; row < wrapped.size() && row < 16U;
                 ++row) {
                write_field_menu_text(
                    screen, kDialogPanelX + 8,
                    kDialogPanelY + 16 + static_cast<std::int32_t>(row) * 8,
                    wrapped[row]);
            }
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
                // 0x12A8C writes ammo/uses/rtng only for the selected
                // inventory cell (include_art). Overview 0x11260 keeps
                // Damage/Power and must not be overwritten.
                const auto detail_label = include_art
                    ? source_item_detail_label(item_code)
                    : std::string_view{};
                if (!detail_label.empty()) {
                    write_field_menu_text(screen, 112, 80, detail_label);
                    // The source value is a two-tile entry selected from
                    // 0x6AC46 by the selected slot's +0x6E byte. It is a
                    // source-rendered value, not a host numeric statistic.
                    const auto phase = source_inventory_phase_for_code(
                        field_entity_, item_code);
                    if (phase < field_menu_detail_values_.size()) {
                        const auto& value = field_menu_detail_values_[phase];
                        if (!value.pixels.empty()) {
                            screen.blit_rgba(168, 80, value.width, value.height,
                                             value.pixels);
                        }
                    }
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
            if (field_menu_posture_active_) {
                // 0x119CE moves focus off the five command tabs and onto
                // the posture strip at CAAC. Do not invent a host bar; the
                // extracted status strip is the control.
            } else if (field_menu_selection_ >= 2U &&
                       field_menu_selection_ <= 6U) {
                // ROM 0x11454 writes a source 2x2 tile block at the selected
                // tab's button coordinate. Its x destinations are 24 pixels
                // apart; the extracted block supplies the real white/blue
                // focus treatment seen on the Genesis screen.
                const auto focus_index = field_menu_selection_ - 2U;
                const auto& focus = field_menu_command_focus_[focus_index];
                if (!focus.pixels.empty()) {
                    screen.blit_rgba(
                        kFieldMenuCommandButtonX[focus_index],
                        kFieldMenuCommandButtonY,
                        focus.width, focus.height, focus.pixels);
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
                // ROM 0x11592 creates this selector as a sprite. Its source
                // coordinates are (0x8C,0xF4), which become (12,116) after
                // the Genesis 0x80 sprite origin is adapted to host pixels;
                // the source strides are 0x28 horizontally and 0x18
                // vertically. Keep it above tile-based item art and
                // selected-equipment overlays, as the VDP sprite is above
                // those name-table layers.
                if (!field_menu_cursor_.pixels.empty()) {
                    screen.blit_rgba(
                        12 + static_cast<std::int32_t>(
                                 field_menu_selection_ * 40U),
                        116 + static_cast<std::int32_t>(field_menu_row_ * 24U),
                        field_menu_cursor_.width, field_menu_cursor_.height,
                        field_menu_cursor_.pixels);
                }
            }
            if (field_menu_selection_ < 2U) {
                draw_item_detail(selected_code, true);
            }
            // ROM 0x1170C/0x11712 emits only the currently selected command
            // or item action at plane-B address EC9C (x=112, y=200). The
            // other choices are not a vertical checklist on this screen.
            if (field_menu_action_active_) {
                if (field_first_aid_active_) {
                    // ROM 0x122B2/0x12344 writes the extracted Heal who?? and
                    // CANCEL strings into the command panel, while the
                    // shared selector helper moves the cursor through the
                    // active-record list plus the count/cancel row.
                    write_field_menu_text(screen, 112, 168, " Heal who??");
                    for (std::size_t index = 0;
                         index < field_first_aid_target_count_; ++index) {
                        const auto y = 176 + static_cast<std::int32_t>(index) * 8;
                        const auto record = field_first_aid_target_records_[index];
                        if (record < field_menu_runner_names_.size()) {
                            write_field_menu_text(
                                screen, 112, y,
                                field_menu_runner_names_[record]);
                        }
                    }
                    const auto cancel_y = 176 + static_cast<std::int32_t>(
                        field_first_aid_target_count_) * 8;
                    write_field_menu_text(screen, 112, cancel_y, "CANCEL");
                    if (!field_first_aid_message_active_ &&
                        !field_menu_cursor_.pixels.empty()) {
                        const auto cursor_y = 174 + static_cast<std::int32_t>(
                            std::min(field_first_aid_target_index_,
                                     field_first_aid_target_count_) * 8U);
                        screen.blit_rgba(96, cursor_y, field_menu_cursor_.width,
                                         field_menu_cursor_.height,
                                         field_menu_cursor_.pixels);
                    }
                    if (field_first_aid_message_active_) {
                        write_field_menu_text(screen, 112, 200,
                                              field_first_aid_message_);
                    }
                } else if (field_item_message_active_) {
                    write_field_menu_text(screen, 112, 200,
                                          field_item_message_);
                } else if (field_trade_active_) {
                    if (field_trade_cannot_trade_ ||
                        field_trade_full_inventory_) {
                        write_field_menu_text(
                            screen, 112, 200, field_trade_message());
                    } else {
                        // 0x1287E/0x12908 write the source "Give to" prompt
                        // and recipient name, then 0x127B0 places the shared
                        // Yes/No choice at EC9C. Keep that composition in the
                        // command screen instead of inventing a party list.
                        write_field_menu_text(screen, 112, 176, "Give to");
                        if (field_trade_recipient_record_ <
                            field_menu_runner_names_.size()) {
                            write_field_menu_text(
                                screen, 112, 184,
                                field_menu_runner_names_[
                                    field_trade_recipient_record_]);
                        }
                        write_field_menu_text(
                            screen, 112, 200,
                            kDiscardChoiceLabels[
                                field_trade_confirmation_yes_ ? 1U : 0U]);
                    }
                } else if (field_discard_confirmation_active_) {
                    write_field_menu_text(screen, 112, 176,
                                          "Discard item?");
                    if (!field_discard_message().empty()) {
                        write_field_menu_text(screen, 112, 184,
                                              field_discard_message());
                    }
                    write_field_menu_text(
                        screen, 112, 200,
                        kDiscardChoiceLabels[field_discard_confirmation_yes_
                                                 ? 1U : 0U]);
                } else if (field_discard_cannot_discard_) {
                    write_field_menu_text(screen, 112, 200,
                                          field_discard_message());
                } else {
                    write_field_menu_text(screen, 112, 200,
                                          field_menu_action_label());
                }
            } else {
                write_field_menu_text(
                    screen, 112, 200,
                    kFieldMenuLabels[field_menu_selection_]);
            }
        } else if (scene_ == NativeScene::PocketSecretaryMenu) {
            // ROM 0xC2A6 -> 0xC3E6 fills $C186 then C0DA writes 0xC1418.
            // 0x18252's 0xD9C96 computer is this screen's bezel. Do not reuse
            // the command-overview portrait plane as Main Menu.
            if (!pocket_secretary_background_.pixels.empty()) {
                screen.clear({0x00, 0x00, 0x00, 0xFF});
                screen.blit_rgba(0, 0, pocket_secretary_background_.width,
                                 pocket_secretary_background_.height,
                                 pocket_secretary_background_.pixels);
            } else {
                render_command_base(screen);
                render_dialog_panel(screen);
            }
            write_field_menu_text(screen, 112, 32, " Main Menu");
            write_field_menu_text(screen, 104, 40, "[\\\\\\\\\\\\\\\\");
            const auto option_count = pocket_secretary_option_count();
            for (std::size_t index = 0; index < option_count; ++index) {
                const auto selected = index == pocket_secretary_selection_;
                const auto y = 72 + static_cast<std::int32_t>(index) * 16;
                if (selected && !startup_cursor_.pixels.empty()) {
                    screen.blit_rgba(88, y, startup_cursor_.width,
                                     startup_cursor_.height,
                                     startup_cursor_.pixels);
                }
                write_field_menu_text(screen, 112, y,
                                      kPocketSecretaryLabels[index]);
            }
            if (pocket_secretary_message_visible_) {
                // C0DA receives A2=0xC40E from 0x18150. Preserve the three
                // extracted source lines as an in-panel response; this is
                // deliberately not a synthetic Matrix/cyberdeck screen.
                for (std::size_t index = 0;
                     index < cyberdeck_unavailable_lines_.size(); ++index) {
                    write_field_menu_text(
                        screen, 88,
                        120 + static_cast<std::int32_t>(index) * 12,
                        cyberdeck_unavailable_lines_[index]);
                }
            }
        } else if (scene_ == NativeScene::NotebookMenu) {
            // ROM 0xF33E writes C567E to the Notebook panel: the title,
            // Current Run, and Tips & Clues are the base rows. Conditional
            // rows use C56A5/C56AE/C56BB at the same two-row stride, so keep
            // their source strings available without displaying them until
            // the corresponding event globals are reconstructed.
            render_command_base(screen);
            render_dialog_panel(screen);
            write_field_menu_text(screen, 112, 32, notebook_label(0));
            write_field_menu_text(screen, 104, 40, "[\\\\\\\\\\\\\\\\");
            for (std::size_t index = 0; index < notebook_option_count_; ++index) {
                const auto selected = index == notebook_selection_;
                const auto y = 72 + static_cast<std::int32_t>(index) * 16;
                if (selected && !notebook_message_visible_ &&
                    !startup_cursor_.pixels.empty()) {
                    screen.blit_rgba(88, y, startup_cursor_.width,
                                     startup_cursor_.height,
                                     startup_cursor_.pixels);
                }
                write_field_menu_text(screen, 112, y,
                                      notebook_label(index + 1U));
            }
            if (notebook_message_visible_) {
                // F2F6 passes C56E7 to C0DA at C30C. These are the exact
                // three source rows, including the intentional leading
                // spaces and the blank separator row.
                for (std::size_t index = 0;
                     index < notebook_no_run_lines_.size(); ++index) {
                    write_field_menu_text(
                        screen, 80,
                        112 + static_cast<std::int32_t>(index) * 12,
                        notebook_no_run_lines_[index]);
                }
                write_field_menu_text(screen, 80, 176, " PRESS ANY BUTTON");
            } else {
                write_field_menu_text(screen, 80, 176,
                                      " A SELECT  B RETURN");
            }
        } else if (scene_ == NativeScene::NotebookTipsMenu) {
            // F9A4 writes the selected pointer target through C0DA after the
            // page counter has been emitted from C663A. The extracted text
            // keeps source 0x80 separators as host newlines, so render each
            // source line at the same fixed tile stride instead of flattening
            // the page into a checklist.
            render_command_base(screen);
            render_dialog_panel(screen);
            write_field_menu_text(screen, 104, 32, notebook_label(2));
            const auto page_number = notebook_visible_tip_count_ == 0U
                ? 0U : notebook_tip_selection_ + 1U;
            write_field_menu_text(
                screen, 80, 48,
                "Page " + std::to_string(page_number) + " of " +
                    std::to_string(notebook_visible_tip_count_));
            if (notebook_visible_tip_count_ != 0U) {
                const auto source_index =
                    notebook_visible_tip_indices_[notebook_tip_selection_];
                const auto& text = notebook_tip_text_[source_index];
                std::size_t line_start = 0;
                std::int32_t y = 72;
                for (std::size_t position = 0;
                     position <= text.size() && y < 168; ++position) {
                    if (position != text.size() && text[position] != '\n') {
                        continue;
                    }
                    write_field_menu_text(
                        screen, 72, y,
                        std::string_view(text).substr(
                            line_start, position - line_start));
                    y += 10;
                    line_start = position + 1U;
                }
            }
            write_field_menu_text(screen, 64, 176,
                                  " A/C/START RETURN B SWITCH");
        } else if (scene_ == NativeScene::GroupItemsMenu) {
            // ROM 0xC8C8 clears the command plane with C3E6, writes the
            // extracted C1471 empty message when FFFC22 is zero, otherwise
            // walks the C25C4 pointer table from bit 14 through bit 0.
            render_command_base(screen);
            render_dialog_panel(screen);
            write_field_menu_text(screen, 112, 32, " Group Items");
            std::int32_t row = 64;
            if (group_item_flags_ == 0U) {
                write_field_menu_text(
                    screen, 72, row,
                    "You are not carrying any group items.");
            } else {
                for (std::size_t index = kNativeGroupItemCount; index-- > 0;) {
                    if ((group_item_flags_ &
                         (static_cast<std::uint32_t>(1U) << index)) == 0U) {
                        continue;
                    }
                    const auto& name = group_item_names_[index];
                    write_field_menu_text(
                        screen, 80, row,
                        name.empty() ? "UNKNOWN" : std::string_view(name));
                    row += 10;
                }
            }
            write_field_menu_text(screen, 80, 176, " PRESS ANY BUTTON");
        } else if (scene_ == NativeScene::DismissRunnerMenu) {
            // ROM C448 writes C1499/C14A4/C14BD around the dynamic names
            // emitted from active runner records. Selector zero is the
            // source Abort row; runner candidates follow it.
            render_command_base(screen);
            render_dialog_panel(screen);
            write_field_menu_text(screen, 112, 32,
                                  dismiss_runner_labels_[0]);
            write_field_menu_text(screen, 112, 52,
                                  dismiss_runner_labels_[1]);
            for (std::size_t index = 0;
                 index < dismiss_runner_candidate_count_; ++index) {
                const auto record_index =
                    dismiss_runner_candidate_records_[index];
                const auto selected = dismiss_runner_selection_ == index + 1U;
                const auto y = 76 + static_cast<std::int32_t>(index) * 16;
                if (selected) {
                    screen.fill_rect(80, y - 2, 176, 13,
                                     {0x00, 0x68, 0x88, 0xFF});
                    if (!field_menu_cursor_.pixels.empty()) {
                        screen.blit_rgba(64, y - 10, field_menu_cursor_.width,
                                         field_menu_cursor_.height,
                                         field_menu_cursor_.pixels);
                    }
                }
                const auto name = record_index < field_menu_runner_names_.size()
                    ? std::string_view(field_menu_runner_names_[record_index])
                    : std::string_view{};
                write_field_menu_text(screen, 96, y,
                                      name.empty() ? "UNKNOWN" : name);
            }
            const auto abort_y = 76 + static_cast<std::int32_t>(
                dismiss_runner_candidate_count_) * 16;
            if (dismiss_runner_selection_ == 0U) {
                screen.fill_rect(80, abort_y - 2, 176, 13,
                                 {0x00, 0x68, 0x88, 0xFF});
                if (!field_menu_cursor_.pixels.empty()) {
                    screen.blit_rgba(64, abort_y - 10,
                                     field_menu_cursor_.width,
                                     field_menu_cursor_.height,
                                     field_menu_cursor_.pixels);
                }
            }
            write_field_menu_text(screen, 112, abort_y,
                                  dismiss_runner_labels_[2]);
            write_field_menu_text(screen, 80, 176, " A SELECT  B RETURN");
        } else if (scene_ == NativeScene::ClipsMenu) {
            render_command_base(screen);
            render_dialog_panel(screen);
            screen.fill_rect(24, 44, 272, 2,
                             {0x00, 0xA8, 0xC8, 0xFF});
            screen.write_text(36, 52, "CLIPS", 2,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            if (clips_trade_active_) {
                // ROM 0x11E7E replaces the top-level D1DDC table with the
                // nested D1E24 trade table. The source also keeps the
                // transfer quantity and recipient visible while this loop is
                // active.
                screen.write_text(
                    166, 54,
                    "TO " +
                        (clips_trade_recipient_record_ <
                                 field_menu_runner_names_.size()
                             ? field_menu_runner_names_[
                                   clips_trade_recipient_record_]
                             : std::string("?")),
                    1, {0xA0, 0xE8, 0xF8, 0xFF});
                screen.write_text(
                    166, 66,
                    "CLIPS " + std::to_string(clips_trade_quantity_), 1,
                    {0xA0, 0xE8, 0xF8, 0xFF});
                for (std::size_t index = 0;
                     index < kClipsTradeMenuLabels.size(); ++index) {
                    const auto selector = index + 2U;
                    const auto y = 82 + static_cast<std::int32_t>(index) * 16;
                    const auto selected = selector == clips_trade_menu_selection_;
                    if (selected) {
                        screen.fill_rect(32, y - 2, 256, 13,
                                         {0x00, 0x68, 0x88, 0xFF});
                    }
                    screen.write_text(
                        40, y, kClipsTradeMenuLabels[index], 1,
                        selected ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                                 : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
                }
            } else {
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
                    screen.write_text(
                        40, y, kClipsMenuLabels[index], 1,
                        selected ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                                 : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
                }
            }
            screen.write_text(36, 152,
                              "LEFT/RIGHT SELECT  A ACTION", 1,
                              {0x90, 0xB8, 0xC8, 0xFF});
            if (clips_reload_confirmation_active_) {
                screen.write_text(40, 152,
                                  "You still have ammo left. Reload?", 1,
                                  {0xF0, 0xC0, 0x50, 0xFF});
                screen.write_text(
                    248, 152,
                    clips_reload_confirmation_yes_ ? "Yes" : "No", 1,
                    {0xF0, 0xC0, 0x50, 0xFF});
            } else if (clips_action_message_frames_ != 0 &&
                       !clips_action_message_.empty()) {
                screen.write_text(120, 152, clips_action_message_,
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
            render_command_base(screen);
            render_dialog_panel(screen);
            write_field_menu_text(screen, 112, 32, "    Cyberware");
            write_field_menu_text(screen, 104, 40, "[\\\\\\\\\\\\\\\\");
            for (std::size_t index = 0; index < cyberware_names_.size();
                 ++index) {
                const auto installed =
                    (field_entity_.cyberware_flags &
                     static_cast<std::uint16_t>(1U << index)) != 0;
                const auto& name = cyberware_names_[index];
                const auto line = std::string(installed ? "[ON] " : "[--] ") +
                                  (name.empty() ? "UNKNOWN" : name);
                write_field_menu_text(
                    screen, 40, 56 + static_cast<std::int32_t>(index) * 8,
                    line);
            }
        } else if (scene_ == NativeScene::AttributesMenu) {
            render_command_base(screen);
            render_dialog_panel(screen);
            write_field_menu_text(screen, 96, 32, "Attributes/Skills");
            write_field_menu_text(screen, 104, 40, "[\\\\\\\\\\\\\\\\");
            for (std::size_t column = 0; column < 2U; ++column) {
                for (std::size_t row = 0; row < 10U; ++row) {
                    const auto index = column * 10U + row;
                    const auto selected = column == attributes_menu_column_ &&
                                          row == attributes_menu_row_;
                    const auto x = column == 0U ? 32 : 164;
                    const auto y = 56 + static_cast<std::int32_t>(row) * 10;
                    if (selected && !startup_cursor_.pixels.empty()) {
                        screen.blit_rgba(x - 16, y, startup_cursor_.width,
                                         startup_cursor_.height,
                                         startup_cursor_.pixels);
                    }
                    const auto value = index < kNativeAttributeCount
                        ? field_entity_.attribute_values[index]
                        : field_entity_.skill_values[index -
                                                      kNativeAttributeCount];
                    const auto& name = attribute_skill_names_[index];
                    const auto line = (name.empty() ? "UNKNOWN" : name) +
                                      " " + std::to_string(value);
                    write_field_menu_text(screen, x, y, line);
                }
            }
        } else if (scene_ == NativeScene::MagicMenu) {
            // ROM 0xFBA6 C0DA at 0xC89C0: Magical Defense panel.
            render_command_base(screen);
            render_dialog_panel(screen);
            write_field_menu_text(screen, 104, 32, " Magical Defense");
            write_field_menu_text(screen, 104, 40, "[\\\\\\\\\\\\\\\\");
            if (field_entity_.display_palette_state < 2U) {
                write_field_menu_text(screen, 112, 72, "None");
            } else {
                write_field_menu_text(screen, 48, 64, "  vs. Phys");
                write_field_menu_text(screen, 48, 80, "  vs. Mana");
                write_field_menu_text(screen, 48, 104, "Success");
                write_field_menu_text(screen, 48, 120, "Drain");
                write_field_menu_text(screen, 48, 136, "Defense");
                write_field_menu_text(screen, 48, 152, "Force");
                write_field_menu_text(screen, 48, 168, "Posture");
                write_field_menu_text(screen, 48, 184, "Set Force & Posture.");
            }
        } else {
            // Field mode is the map/object plane plus its source HUD. The
            // old host-only control legend did not exist in the cartridge.
        }
        if (scene_ != NativeScene::Field &&
            scene_ != NativeScene::FieldDialog) {
            // Command, Pocket Secretary, and the other C0DA panels keep the
            // 64px HUD column. Capture character/PS screens never cover the
            // sidebar with the 64x80 overview portrait.
            render_field_hud(screen);
        }
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
