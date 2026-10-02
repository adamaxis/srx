#include "shadowrun/native_port.hpp"

#include "shadowrun/entity_projection.hpp"
#include "shadowrun/intro_scene.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <stdexcept>
#include <utility>

namespace shadowrun::native {

const NativeImage& NativePortAssets::operator[](std::string_view alias) const {
    const auto found = resources.find(std::string(alias));
    if (found == resources.end()) {
        throw std::out_of_range("unknown native resource alias: " +
                                std::string(alias));
    }
    return found->second;
}

const std::filesystem::path& NativePortAssets::audio_path(
    std::string_view alias) const {
    const auto found = audio_resources.find(std::string(alias));
    if (found == audio_resources.end()) {
        throw std::out_of_range("unknown native audio resource alias: " +
                                std::string(alias));
    }
    return found->second;
}

std::optional<NativeAudioLoop> NativePortAssets::audio_loop(
    std::string_view alias) const {
    const auto found = audio_loop_points.find(std::string(alias));
    if (found == audio_loop_points.end()) {
        return std::nullopt;
    }
    return found->second;
}

namespace {

using Glyph = std::array<std::uint8_t, 7>;

std::string center_source_fixed_field(std::string_view value,
                                      std::size_t width = 12U) {
    if (value.size() >= width) {
        return std::string(value);
    }
    const auto left_padding = (width - value.size()) / 2U;
    return std::string(left_padding, ' ') + std::string(value);
}

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
// ROM 0x13726 walks the five D4536 cursor sizes while consuming the five
// position words at D4464.  The source redraws the map between the two
// destination records used by the opening cab ride: Downtown Seattle first,
// then Redmond Barrens.  These are presentation states, not a ROM/VDP
// emulator; the extracted cab plate remains the map texture underneath.
struct CabMapCursorState {
    std::uint16_t position;
    std::uint8_t width_tiles;
    std::uint8_t height_tiles;
};
constexpr std::array<CabMapCursorState, 5> kCabDowntownCursor{{
    {0x0000U, 0x17U, 0x0EU}, {0x0184U, 0x11U, 0x0BU},
    {0x030AU, 0x0CU, 0x08U}, {0x0410U, 0x06U, 0x05U},
    {0x0514U, 0x00U, 0x00U}}};
constexpr std::array<CabMapCursorState, 5> kCabRedmondCursor{{
    {0x0000U, 0x17U, 0x0EU}, {0x010CU, 0x11U, 0x0BU},
    {0x0216U, 0x0CU, 0x08U}, {0x0322U, 0x06U, 0x05U},
    {0x03AEU, 0x00U, 0x00U}}};
constexpr std::uint64_t kCabCursorPhaseFrames = 6;
// The cab plate's plane window (32,32) lands at the capture's (62,30), so
// plane coordinates shift by (+30,-2) at the host's one-to-one tile scale.
constexpr std::int32_t kCabPlaneToScreenX = 30;
constexpr std::int32_t kCabPlaneToScreenY = -2;

void draw_cab_cursor(ScreenWriter& screen, const CabMapCursorState& state) {
    // D4464 positions are VDP name-table addresses relative to $C000.  The
    // decoder uses the same $C208 base as 0x13726's D4 register.
    const auto relative = static_cast<std::uint32_t>(0x0208U) +
        static_cast<std::uint32_t>(state.position);
    const auto source_x = static_cast<std::int32_t>(
        ((relative % 0x80U) / 2U) * 8U);
    const auto source_y = static_cast<std::int32_t>((relative / 0x80U) * 8U);
    // 0x13726's border spans inner + three tiles (both edges plus the
    // corner columns). The plane-to-screen offset for the cab plate is
    // (+30,-2) at one-to-one tile scale.
    const auto source_width =
        static_cast<std::int32_t>(state.width_tiles + 3U) * 8;
    const auto source_height =
        static_cast<std::int32_t>(state.height_tiles + 3U) * 8;
    const auto left = source_x + kCabPlaneToScreenX;
    const auto top = source_y + kCabPlaneToScreenY;
    const auto width = source_width;
    const auto height = source_height;
    // The cursor tiles in D4446 are a pale, two-tone outline. Preserve that
    // visual hierarchy at the host presentation scale while the map itself
    // remains the extracted ROM art.
    screen.fill_rect(left, top, width, 2, {0xF0, 0xF0, 0xF0, 0xFF});
    screen.fill_rect(left, top + height - 2, width, 2,
                     {0xA8, 0xB8, 0xC0, 0xFF});
    screen.fill_rect(left, top, 2, height, {0xF0, 0xF0, 0xF0, 0xFF});
    screen.fill_rect(left + width - 2, top, 2, height,
                     {0xA8, 0xB8, 0xC0, 0xFF});
}
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

// ROM strings at C1BC0..C2119 are the descriptions emitted by the
// Attributes/Skills cursor. Keep the three source rows per entry together so
// moving the selector cannot leave a previous entry's lines on the panel.
constexpr std::array<std::array<std::string_view, 3>, 20>
    kAttributeSkillDescriptions{{
        {{"Body resists damage from weapons,",
          "firearms, physical spells, and", "Black Ice."}},
        {{"Quickness determines movement and", "attack speed, and increases combat",
          "and cybercombat success."}},
        {{"Strength increases melee damage", "and grenade accuracy.", ""}},
        {{"Charisma reduces the hiring price", "of Shadowrunners and increases",
          "fast-talk success."}},
        {{"Intelligence determines attack", "speed, plus increases combat and",
          "cybercombat success."}},
        {{"Willpower resists damage from mana", "spells and magical drain. It also",
          "increases combat success."}},
        {{"Essence allows fast magical", "healing.  It decreases when",
          "cyberware is installed."}},
        {{"Magic determines the maximum Force", "of spells cast.  It decreases when",
          "cyberware is installed."}},
        {{"Sorcery determines success with", "spell casting.", ""}},
        {{"Firearms determines success", "with ALL types of guns.", ""}},
        {{"Pistols determines success with", "both light and heavy pistols.", ""}},
        {{"SMGs determines success with", "sub-machine guns.", ""}},
        {{"Shotgun determines success with", "various forms of shotguns.", ""}},
        {{"Melee Combat determines success", "while fighting hand-to-hand.", ""}},
        {{"Throwing increases accuracy and", "damage when tossing a grenade.", ""}},
        {{"Computer increases success in", "cybercombat and the operation of",
          "Nodes."}},
        {{"Biotech determines success while", "using a Medkit.", ""}},
        {{"Electronics determines success", "when attempting to force open",
          "a Maglock."}},
        {{"Reputation determines overall", "notoriety of the character.", ""}},
        {{"Negotiation increases Shadowrun", "payments and influences buying and",
          "selling."}},
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

NativePortShell::~NativePortShell() = default;
NativePortShell::NativePortShell(NativePortShell&&) noexcept = default;
NativePortShell& NativePortShell::operator=(NativePortShell&&) noexcept =
    default;

bool NativePortShell::has_intro() const {
    return intro_scene_ != nullptr && intro_scene_->has_story();
}

std::size_t NativePortShell::intro_line_start() const {
    return intro_scene_ == nullptr ? 0U : intro_scene_->line_start();
}

std::uint64_t NativePortShell::intro_frame() const {
    return intro_scene_ == nullptr ? 0U : intro_scene_->frame();
}

std::size_t NativePortShell::intro_story_line_count() const {
    return intro_scene_ == nullptr ? 0U : intro_scene_->story_line_count();
}

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
    field_hud_ = std::move(assets.field_hud);
    field_hud_faces_ = std::move(assets.field_hud_faces);
    field_hud_item_blocks_ = std::move(assets.field_hud_item_blocks);
    field_hud_unarmed_item_blocks_ =
        std::move(assets.field_hud_unarmed_item_blocks);
    field_hud_vertical_counters_ =
        std::move(assets.field_hud_vertical_counters);
    field_hud_horizontal_counters_ =
        std::move(assets.field_hud_horizontal_counters);
    field_hud_ammo_counters_ = std::move(assets.field_hud_ammo_counters);
    pocket_secretary_background_ = std::move(assets.pocket_secretary_background);
    cab_map_ = std::move(assets.cab_map);
    if (!boot_background_.pixels.empty()) {
        scene_ = NativeScene::Bootstrap;
    }
      field_menu_background_ = std::move(assets.field_menu_background);
    field_menu_action_background_ =
        std::move(assets.field_menu_action_background);
    field_menu_attributes_background_ =
        std::move(assets.field_menu_attributes_background);
      field_menu_inventory_boxes_ =
          std::move(assets.field_menu_inventory_boxes);
      field_menu_font_ = std::move(assets.field_menu_font);
    startup_font_ = std::move(assets.startup_font);
    startup_cursor_ = std::move(assets.startup_cursor);
    field_menu_runner_portraits_ = std::move(assets.field_menu_runner_portraits);
    field_menu_selected_cells_ = std::move(assets.field_menu_selected_cells);
    field_menu_command_focus_ = std::move(assets.field_menu_command_focus);
    field_menu_action_focus_ = std::move(assets.field_menu_action_focus);
    field_menu_clips_focus_ = std::move(assets.field_menu_clips_focus);
    field_menu_clips_trade_focus_ =
        std::move(assets.field_menu_clips_trade_focus);
    field_menu_posture_focus_ = std::move(assets.field_menu_posture_focus);
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
    IntroSceneAssets intro_assets;
    intro_assets.story_lines = std::move(intro_.story_lines);
    intro_assets.stages = std::move(assets.intro_stages);
    intro_assets.aircraft_plane_a = std::move(assets.intro_aircraft_plane_a);
    intro_assets.trivid_background = std::move(assets.intro_trivid_background);
    intro_assets.sprites = std::move(assets.intro_sprites);
    intro_assets.background_patches = std::move(assets.intro_background_patches);
    intro_assets.font = field_menu_font_;
    intro_scene_ = std::make_unique<IntroScene>(std::move(intro_assets));
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
    recruited_runner_templates_ =
        std::move(assets.recruited_runner_templates);
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

namespace {

const NativeImage& empty_native_image() {
    static const NativeImage empty{};
    return empty;
}

} // namespace

std::string NativePortShell::field_menu_class_label() const {
    switch (field_entity_.archetype_index) {
    case 1U:
        return "Human " + intro_.archetypes[2] + " " + intro_.archetypes[3];
    case 2U:
        return intro_.archetypes[0];
    case 3U:
        return "Human " + intro_.archetypes[1];
    default:
        return "Unknown";
    }
}

const NativeImage& NativePortShell::field_hud_face_asset(
    std::size_t index) const {
    return index < field_hud_faces_.size() ? field_hud_faces_[index]
                                           : empty_native_image();
}

const NativeImage& NativePortShell::field_hud_vertical_counter_asset(
    std::size_t index) const {
    return index < field_hud_vertical_counters_.size()
        ? field_hud_vertical_counters_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_hud_horizontal_counter_asset(
    std::size_t index) const {
    return index < field_hud_horizontal_counters_.size()
        ? field_hud_horizontal_counters_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_hud_ammo_counter_asset(
    std::size_t index) const {
    return index < field_hud_ammo_counters_.size()
        ? field_hud_ammo_counters_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_runner_portrait_asset(
    std::size_t index) const {
    return index < field_menu_runner_portraits_.size()
        ? field_menu_runner_portraits_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_selected_cell_asset(
    std::size_t index) const {
    return index < field_menu_selected_cells_.size()
        ? field_menu_selected_cells_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_item_art_asset(
    std::uint8_t code) const {
    return code != 0U && code <= field_menu_item_art_.size()
        ? field_menu_item_art_[code - 1U] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_inventory_icon_asset(
    std::uint8_t code, std::size_t palette) const {
    return code != 0U && code <= field_menu_inventory_icons_.size() &&
            palette < field_menu_inventory_icons_[code - 1U].size()
        ? field_menu_inventory_icons_[code - 1U][palette]
        : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_status_bar_asset(
    std::size_t index) const {
    return index < field_menu_status_bars_.size()
        ? field_menu_status_bars_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_command_focus_asset(
    std::size_t index) const {
    return index < field_menu_command_focus_.size()
        ? field_menu_command_focus_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_damage_bar_asset(
    std::size_t index) const {
    return index < field_menu_damage_bars_.size()
        ? field_menu_damage_bars_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_drain_bar_asset(
    std::size_t index) const {
    return index < field_menu_drain_bars_.size()
        ? field_menu_drain_bars_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_attack_bar_asset(
    std::size_t index) const {
    return index < field_menu_attack_bars_.size()
        ? field_menu_attack_bars_[index] : empty_native_image();
}

const NativeImage& NativePortShell::field_menu_defense_bar_asset(
    std::size_t index) const {
    return index < field_menu_defense_bars_.size()
        ? field_menu_defense_bars_[index] : empty_native_image();
}

std::string_view NativePortShell::field_menu_runner_name(
    std::size_t index) const {
    return index < field_menu_runner_names_.size()
        ? std::string_view(field_menu_runner_names_[index])
        : std::string_view{};
}

std::string_view NativePortShell::field_menu_status_name(
    std::size_t index) const {
    return index < field_menu_status_names_.size()
        ? std::string_view(field_menu_status_names_[index])
        : std::string_view{};
}

const NativeInventoryItem* NativePortShell::field_menu_item(
    std::uint8_t code) const {
    return code != 0U && code <= inventory_items_.size()
        ? &inventory_items_[code - 1U] : nullptr;
}

std::string_view NativePortShell::field_menu_power_label(
    std::size_t index) const {
    return index < field_menu_power_labels_.size()
        ? std::string_view(field_menu_power_labels_[index])
        : std::string_view{};
}

const std::array<std::string, 2>* NativePortShell::field_menu_armor_label(
    std::size_t index) const {
    return index < field_menu_armor_labels_.size()
        ? &field_menu_armor_labels_[index] : nullptr;
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
    if (intro_scene_ != nullptr) {
        intro_scene_->reset();
    }
    scene_ = has_intro() ? NativeScene::Intro : NativeScene::Field;
    if (scene_ == NativeScene::Field && !field_map_pixels_.empty()) {
        reset_field_state();
    }
}

void NativePortShell::update_intro(const InputState& input) {
    if (intro_scene_ == nullptr) {
        scene_ = NativeScene::Field;
        if (!field_map_pixels_.empty()) {
            reset_field_state();
        }
        return;
    }

    const auto result = intro_scene_->update(input);
    if (result == IntroSceneResult::Skipped) {
        scene_ = NativeScene::Field;
        if (!field_map_pixels_.empty()) {
            reset_field_state();
        }
    } else if (result == IntroSceneResult::Finished) {
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
        // ROM 0x600E resolves the active negative-marker walking list and
        // copies its auxiliary word into record +0x18. 0x5FDC consumes that
        // record field as the animation clock increment.
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

    if (input.was_pressed(Button::B) || input.was_pressed(Button::Start)) {
        field_first_aid_active_ = false;
        field_menu_action_active_ = false;
        field_menu_action_selection_ = 2U;
        return;
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
    if (input.was_pressed(Button::B)) {
        field_trade_active_ = false;
        field_menu_action_active_ = false;
        field_menu_action_selection_ = 2U;
        return;
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
    if (input.was_pressed(Button::Start)) {
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
        if (input.was_pressed(Button::B)) {
            field_discard_confirmation_active_ = false;
            field_discard_confirmation_yes_ = false;
            field_discard_item_was_equipped_ = false;
            field_menu_action_active_ = false;
            field_menu_action_selection_ = 2;
            return;
        }
        if (input.was_pressed(Button::A)) {
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

    // B is the universal cancel edge for the native menu. It returns from the
    // action loop without dispatching the currently focused command.
    if (input.was_pressed(Button::B)) {
        field_menu_action_active_ = false;
        field_menu_action_selection_ = 2;
        field_discard_confirmation_active_ = false;
        field_discard_confirmation_yes_ = false;
        field_discard_item_was_equipped_ = false;
        field_discard_cannot_discard_ = false;
        return;
    }
    if (input.was_pressed(Button::A)) {
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
                                            std::string_view text,
                                            std::optional<Rgba8> tint) const {
      if (field_menu_font_.pixels.size() !=
          static_cast<std::size_t>(field_menu_font_.width) *
              field_menu_font_.height ||
          field_menu_font_.width < 8U || field_menu_font_.height < 8U) {
        screen.write_text(x, y, text, 1,
                          tint.value_or(Rgba8{0xE8, 0xF4, 0xFF, 0xFF}));
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
          if ((glyph_row + 1U) * 8U > field_menu_font_.height) {
            cursor += 8;
            continue;
        }
        std::vector<Rgba8> glyph(64U);
        auto glyph_has_ink = false;
        for (std::size_t row = 0; row < 8U; ++row) {
            for (std::size_t column = 0; column < 8U; ++column) {
                  glyph[row * 8U + column] = field_menu_font_.pixels[
                      (glyph_row * 8U + row) * field_menu_font_.width +
                    glyph_column * 8U + column];
                if (tint && glyph[row * 8U + column].alpha != 0U) {
                    glyph[row * 8U + column].red = tint->red;
                    glyph[row * 8U + column].green = tint->green;
                    glyph[row * 8U + column].blue = tint->blue;
                }
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
                              tint.value_or(Rgba8{0xA0, 0xE8, 0xF8, 0xFF}));
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
                         startup_background_.height, startup_background_.pixels);
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

    // Cyberdeck's unavailable response is a child window of C2A6. Its B edge
    // dismisses that response and redraws Pocket Secretary; it must not fall
    // through to the field command loop.
    if (pocket_secretary_message_visible_) {
        if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
            input.was_pressed(Button::Start)) {
            pocket_secretary_message_visible_ = false;
        }
        return;
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
        scene_ = NativeScene::PocketSecretaryMenu;
        pocket_secretary_selection_ = 0;
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
    if (input.was_pressed(Button::B)) {
        // B is the parent-navigation key throughout the Pocket Secretary
        // tree. The old path incorrectly treated it as the runner-switch
        // shortcut and left the user on (or altered) the field menu.
        scene_ = NativeScene::NotebookMenu;
        notebook_selection_ = 1U;
    }
}

void NativePortShell::update_group_items_menu(const InputState& input) {
    // ROM F312 waits for any newly asserted controller bit.  B is the parent
    // navigation edge from this sub-menu; other edges retain the reviewed
    // command-loop return path.
    if (input.was_pressed(Button::B)) {
        scene_ = NativeScene::PocketSecretaryMenu;
        pocket_secretary_selection_ = 1;
        return;
    }
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

    // B is the universal cancel edge. Test it before A so a simultaneous
    // controller sample can never confirm or open a command while cancelling.
    if (input.was_pressed(Button::B)) {
        scene_ = NativeScene::Field;
        field_menu_posture_active_ = false;
        field_command_message_frames_ = 0;
        return;
    }

    // A enters the selected command.
    if (input.was_pressed(Button::A)) {
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
        if (input.was_pressed(Button::B) || input.was_pressed(Button::Start)) {
            clips_reload_confirmation_active_ = false;
            clips_reload_confirmation_yes_ = false;
            return;
        }
        if (input.was_pressed(Button::A)) {
            if (clips_reload_confirmation_yes_) {
                reload_one_clip();
            }
            clips_reload_confirmation_active_ = false;
            clips_reload_confirmation_yes_ = false;
        }
        return;
    }

    if (input.was_pressed(Button::B) || input.was_pressed(Button::Start)) {
        scene_ = NativeScene::FieldMenu;
        field_menu_selection_ = 2;
        field_command_message_frames_ = 0;
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
    if (input.was_pressed(Button::A) || input.was_pressed(Button::C)) {
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

    if (input.was_pressed(Button::B)) {
        clips_trade_active_ = false;
        return;
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
    if (input.was_pressed(Button::Start)) {
        // B/Start leave the nested clip-trade loop but remain in Clips until
        // the parent command-screen edge is pressed again.
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
    // The original handler has no selection or mutation loop. B/Start return
    // to the command selector; arrows and action buttons are ignored.
    if (input.was_pressed(Button::Start) || input.was_pressed(Button::B)) {
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
    // and are deliberately not guessed here. B/Start return to the same
    // top-level command selector.
    if (input.was_pressed(Button::Start) || input.was_pressed(Button::B)) {
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
    if (!field_menu_background_.pixels.empty()) {
        screen.blit_rgba(0, 0, field_menu_background_.width,
                         field_menu_background_.height,
                         field_menu_background_.pixels);
    }
}

void NativePortShell::render_command_static_layers(ScreenWriter& screen) const {
    // The ROM enters Clips/Cyberware/Magic from the already initialized
    // command screen. Those routines clear and rewrite only the center plane;
    // the portrait, runner name, inventory cells/icons, equipment borders,
    // and lower status strip remain visible around the submenu.
    render_command_base(screen);
    if (!field_menu_inventory_boxes_.pixels.empty()) {
        screen.blit_rgba(0, 0, field_menu_inventory_boxes_.width,
                         field_menu_inventory_boxes_.height,
                         field_menu_inventory_boxes_.pixels);
    }
    render_command_dynamic_layers(screen);
}

void NativePortShell::render_command_dynamic_layers(
    ScreenWriter& screen, bool attributes_frame) const {
    // These writes are the live record-dependent portion of the command
    // screen. Attributes/Skills uses a different source background, but the
    // same runner portrait, status, equipment, and item-icon writers remain
    // active around that plane.
    if (attributes_frame) {
        // 0xD01C uses the dedicated full-screen attributes plane. It does
        // not redraw the ordinary command inventory/portrait column; the
        // only runner identity sprite left on this plane is the small face
        // in the upper title box.
        const auto data_index = active_field_record_ == 0U
            ? (selected_archetype_ < field_hud_faces_.size()
                   ? selected_archetype_ : 0U)
            : std::min<std::size_t>(active_field_record_,
                                    field_hud_faces_.size() - 1U);
        const auto& face = field_hud_faces_[data_index];
        if (!face.pixels.empty()) {
            screen.blit_rgba(192, 8, face.width, face.height, face.pixels);
        }
        if (active_field_record_ < field_menu_runner_names_.size()) {
            const auto& name = field_menu_runner_names_[active_field_record_];
            if (!name.empty()) {
                write_field_menu_text(screen, 120, 24, name);
                const auto class_name = field_entity_.archetype_index == 3U
                    ? std::string("Human ") + intro_.archetypes[1]
                    : field_entity_.archetype_index == 2U
                        ? intro_.archetypes[0]
                        : field_entity_.archetype_index == 1U
                            ? intro_.archetypes[2] + " " +
                                  intro_.archetypes[3]
                            : std::string{};
                if (!class_name.empty()) {
                    write_field_menu_text(screen, 104, 32, class_name);
                }
            }
        }
        return;
    }
    const auto side_offset = 0;
    if (active_field_record_ < field_menu_runner_portraits_.size()) {
        const auto portrait_index = active_field_record_ == 0U
            ? selected_archetype_ : active_field_record_;
        if (portrait_index < field_menu_runner_portraits_.size()) {
            const auto& portrait = field_menu_runner_portraits_[portrait_index];
            if (!portrait.pixels.empty()) {
                screen.blit_rgba(16 + side_offset, 16, portrait.width,
                                 portrait.height, portrait.pixels);
            }
        }
        if (active_field_record_ < field_menu_runner_names_.size()) {
            const auto& name = field_menu_runner_names_[active_field_record_];
            if (!name.empty()) {
                write_field_menu_text(screen, attributes_frame ? 120 : 128, 8,
                                      name);
                if (attributes_frame) {
                    const auto class_name = field_entity_.archetype_index == 3U
                        ? std::string("Human ") + intro_.archetypes[1]
                        : field_entity_.archetype_index == 2U
                            ? intro_.archetypes[0]
                            : field_entity_.archetype_index == 1U
                                ? intro_.archetypes[2] + " " +
                                      intro_.archetypes[3]
                                : std::string{};
                    if (!class_name.empty()) {
                        write_field_menu_text(screen, 104, 16, class_name);
                    }
                }
            }
        }
    }
    if (!attributes_frame) {
        const auto status_index = std::min<std::size_t>(
            field_entity_.menu_status_index, field_menu_status_bars_.size() - 1U);
        const auto& status_bar = field_menu_status_bars_[status_index];
        if (!status_bar.pixels.empty()) {
            screen.blit_rgba(176, 168, status_bar.width, status_bar.height,
                             status_bar.pixels);
        }
        const auto& status_name = field_menu_status_names_[status_index];
        if (!status_name.empty()) {
            write_field_menu_text(screen, 136, 176, status_name);
        }
    }

    constexpr std::array<std::array<std::int32_t, 2>,
                         kNativeInventorySlotCount>
        kInventoryIconPositions{{
            {{16, 120}}, {{56, 120}},
            {{16, 144}}, {{56, 144}},
            {{16, 168}}, {{56, 168}},
            {{16, 192}}, {{56, 192}},
        }};
    const auto draw_equipment_border = [this, &screen, side_offset](
                                           std::size_t slot) {
        if (slot >= kNativeInventorySlotCount) {
            return;
        }
        const auto overlay_index = ((slot / 2U) & 1U) == 0U ? 0U : 1U;
        const auto& overlay = field_menu_selected_cells_[overlay_index];
        if (!overlay.pixels.empty()) {
            screen.blit_rgba(
                8 + side_offset + static_cast<std::int32_t>((slot % 2U) * 40U),
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
    const auto icon_palette_variant =
        field_menu_uses_alt_item_palette(field_entity_) ? 1U : 0U;
    for (std::size_t slot = 0;
         slot < field_entity_.inventory_item_codes.size(); ++slot) {
        const auto item_code = field_entity_.inventory_item_codes[slot];
        if (item_code == 0U || item_code > kNativeInventoryItemCount) {
            continue;
        }
        const auto& icon = field_menu_inventory_icons_[item_code - 1U]
            [icon_palette_variant];
        if (!icon.pixels.empty()) {
            screen.blit_rgba(kInventoryIconPositions[slot][0] + side_offset,
                             kInventoryIconPositions[slot][1], icon.width,
                             icon.height, icon.pixels);
        }
    }
}

void NativePortShell::render_dialog_panel(ScreenWriter& screen) const {
    // 0xC3E6 / 0x12C5A: 28x21 tile fill at $C186. Same panel for motel
    // talk, street vendors, and other C0DA conversations.
    screen.fill_rect(kDialogPanelX, kDialogPanelY, kDialogPanelWidth,
                     kDialogPanelHeight, {0x00, 0x00, 0x00, 0xFF});
}

void NativePortShell::render_field_hud(ScreenWriter& screen) const {
    // 0x334C supplies the static chrome. 0x39F2 then calls 0x36FC, 0x37EC,
    // and 0x35D4/0x35F0 for each runner. The face is the source 4x4 block at
    // F1C6; the item is the following 3x2 block at F448, with the counters at
    // F4C4 and F3C6.
    if (!field_hud_.pixels.empty()) {
        screen.blit_rgba(0, 0, field_hud_.width, field_hud_.height,
                         field_hud_.pixels);
    }
    const auto slot_index = active_field_record_ < field_hud_faces_.size()
        ? active_field_record_
        : 0U;
    const auto data_index = active_field_record_ == 0U
        ? (selected_archetype_ < field_hud_faces_.size() ? selected_archetype_
                                                           : 0U)
        : slot_index;
    const auto slot_y = static_cast<std::int32_t>(slot_index * 64U);
    if (!field_hud_faces_[data_index].pixels.empty()) {
        const auto& face = field_hud_faces_[data_index];
        screen.blit_rgba(kFieldHudFaceX, kFieldHudFaceY + slot_y, face.width,
                         face.height, face.pixels);
    }
    if (!field_hud_vertical_counters_[data_index].pixels.empty()) {
        const auto& counter = field_hud_vertical_counters_[data_index];
        screen.blit_rgba(kFieldHudVerticalCounterX,
                         kFieldHudVerticalCounterY + slot_y,
                         counter.width, counter.height, counter.pixels);
    }
    if (!field_hud_horizontal_counters_[data_index].pixels.empty()) {
        const auto& counter = field_hud_horizontal_counters_[data_index];
        screen.blit_rgba(kFieldHudHorizontalCounterX,
                         kFieldHudHorizontalCounterY + slot_y,
                         counter.width, counter.height, counter.pixels);
    }
    // The extracted HUD plate contains zero-filled ammo digits for all three
    // roster slots.  0x36FC rewrites only the active record's slot; clear the
    // other baked digits first so inactive records do not look armed.
    for (std::size_t slot = 0; slot < field_hud_faces_.size(); ++slot) {
        screen.fill_rect(
            kFieldHudAmmoCounterX,
            kFieldHudAmmoCounterY + static_cast<std::int32_t>(slot * 64U),
            16, 8, {0x00, 0x00, 0x00, 0xFF});
    }
    if (field_entity_.ammo_rounds < field_hud_ammo_counters_.size() &&
        !field_hud_ammo_counters_[field_entity_.ammo_rounds].pixels.empty()) {
        const auto& counter = field_hud_ammo_counters_[field_entity_.ammo_rounds];
        screen.blit_rgba(kFieldHudAmmoCounterX,
                         kFieldHudAmmoCounterY + slot_y,
                         counter.width, counter.height, counter.pixels);
    }
    // The extracted HUD item block is the startup record's upload.  Once the
    // native record changes equipment it is stale, so redraw this six-tile
    // area from the live equipped item exactly as the ROM's 0x3404/0x35D4
    // path does.  A 0x7F display slot is the source's unarmed state and must
    // not leave the startup pistol behind.
    screen.fill_rect(kFieldHudGunX, kFieldHudGunY + slot_y, 24, 16,
                     {0x00, 0x00, 0x00, 0xFF});
    if (field_entity_.display_state >= 0 &&
        field_entity_.display_state != 0x7F &&
        field_entity_.active_item_code > 0U &&
        field_entity_.active_item_code <= kNativeInventoryItemCount) {
        const auto icon_index = static_cast<std::size_t>(
            field_entity_.active_item_code - 1U);
        const auto palette = field_menu_uses_alt_item_palette(field_entity_)
            ? 1U : 0U;
        const auto& item = field_entity_.active_item_code ==
                                   field_entity_records_[0].active_item_code &&
                               data_index < field_hud_item_blocks_.size()
            ? field_hud_item_blocks_[data_index]
            : field_menu_inventory_icons_[icon_index][palette];
        if (!item.pixels.empty()) {
            screen.blit_rgba(kFieldHudGunX, kFieldHudGunY + slot_y,
                             item.width, item.height, item.pixels);
        }
    } else if (field_entity_.display_state == 0x7F &&
               field_entity_.active_item_code == 0U &&
               data_index < field_hud_unarmed_item_blocks_.size()) {
        const auto& item = field_hud_unarmed_item_blocks_[data_index];
        if (!item.pixels.empty()) {
            screen.blit_rgba(kFieldHudGunX, kFieldHudGunY + slot_y,
                             item.width, item.height, item.pixels);
        }
    }
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

void NativePortShell::update_title(const InputState& input) {
    // 0x1C1E runs before the reveal dispatch. Once 0x1E4E sets DD5A to one,
    // its $5A countdown reaches state two and the source writes the completed
    // copyright block. Keep this state explicit even though the renderer only
    // needs the two visible thresholds.
    if (title_completion_state_ == 1U) {
        --title_text_timer_;
        if (title_text_timer_ < 0) {
            title_completion_state_ = 2U;
            title_copyright_frame_ = frame_number_;
        }
    }

    // 0x1B24/0x1B62: DD2C is a signed countdown. The reveal branch starts on
    // the pass after it reaches -1, not when it reaches zero.
    --title_reveal_delay_;
    if (title_reveal_delay_ < 0 &&
        title_revealed_columns_ <= 0x25U) {
        // 0x1E4E: DD40 starts at zero, then gates successive logo columns to
        // every other title-loop pass. The source writes one complete 8px
        // column per call and stops after the 38th column.
        --title_column_cadence_;
        if (title_column_cadence_ < 0) {
            title_column_cadence_ = 1;
            if (title_revealed_columns_ < kTitleRevealColumnCount) {
                ++title_revealed_columns_;
                // 0x1EAA sets DD5A when FFF1E6 becomes $25, before the final
                // column pass at $25. That is the source completion boundary.
                if (title_revealed_columns_ == 0x25U) {
                    title_completion_state_ = 1U;
                    title_text_timer_ = kTitleTextDelay;
                }
            }
        }

        // 0x1C6A is called on every pass in the reveal branch, independently
        // of DD40. Its first call advances phase 0 -> 1 and origin $98 ->
        // $9C; each later phase lasts four calls.
        --title_sprite_cadence_;
        if (title_sprite_cadence_ < 0) {
            title_sprite_cadence_ = 3;
            ++title_sprite_phase_;
            if (title_sprite_phase_ >= kNativeTitleAnimationFrameCount) {
                title_sprite_phase_ = 0;
            }
        }
        title_sprite_x_ = static_cast<std::int16_t>(title_sprite_x_ + 4);
    }

    // 0x0BF2 samples an edge during the title loop. The source leaves the
    // reveal state intact, then enters the 0x131C palette fade.
    if (input.was_pressed(Button::Start)) {
        title_fade_left_ = kBootFadeFrames;
    }
    ++frame_number_;
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
        update_title(input);
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
            // 0x600E selects the direction-specific walking record from the
            // source selector table at 0xC072.  The values are ROM data, not
            // an arithmetic 0x60 + direction*4 convention; that convention
            // selected an unrelated effect state for several directions.
            const auto desired_selector =
                field_entity_direction_ < field_entity_animations_.size()
                    ? field_entity_animations_[field_entity_direction_]
                          .state_selector
                    : static_cast<std::uint16_t>(0);
            const auto animation = std::find_if(
                field_entity_animations_.begin(),
                field_entity_animations_.end(),
                [desired_selector](const NativeFieldAnimation& candidate) {
                    return candidate.state_selector == desired_selector;
                });
            const auto direction_changed =
                field_entity_.state_selector != desired_selector;
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
    // ROM 0x4618 runs 0x5FDC in the entity phase every frame while the
    // walking state remains active; the clock is not advanced only when a
    // movement step happens. Keep the source cadence visible on a held
    // direction without falling back to the idle/effect table.
    update_field_entity_animation(field_entity_walking_);
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
            screen.blit_rgba(boot_fasa_.origin_x, boot_fasa_.origin_y,
                             boot_fasa_.width, boot_fasa_.height,
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
            const auto logo_x = logo == &boot_background_ ? 0 : logo->origin_x;
            const auto logo_y = logo == &boot_background_ ? 0 : logo->origin_y;
            screen.blit_rgba(logo_x, logo_y, logo->width, logo->height,
                             fade_copy(logo->pixels));
            if (!boot_guy_.pixels.empty() && boot_guy_fade_ != 0U) {
                const auto guy_delta_y = static_cast<std::int32_t>(
                    boot_guy_y_ - static_cast<std::int16_t>(kBootGuyComposeY));
                auto guy = fade_copy(boot_guy_.pixels);
                if (boot_guy_fade_ < 256U) {
                    guy = scale_copy(guy, boot_guy_fade_, 256U);
                }
                screen.blit_rgba(boot_guy_.origin_x,
                                 guy_delta_y + boot_guy_.origin_y,
                                 boot_guy_.width,
                                 boot_guy_.height, guy);
            }
            if (boot_lightning_visible() && !boot_sega_frames_.empty()) {
                const auto phase = static_cast<std::size_t>(boot_sega_phase_) %
                                   boot_sega_frames_.size();
                const auto& sega = boot_sega_frames_[phase];
                if (!sega.pixels.empty()) {
                    screen.blit_rgba(sega.origin_x, sega.origin_y,
                                     sega.width, sega.height,
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
            constexpr std::size_t kCircuitFadeFrames = 16U;
            const auto has_title_loop = frame_number_ != 0U;
            const auto title_loop = has_title_loop ? frame_number_ - 1U : 0U;
            const auto title_fade_weight =
                title_fade_left_ == 0U ? kBootFadeFrames : title_fade_left_;
            // FFF1E6 is the source column index. It is deliberately kept as
            // typed state by update_title; deriving this from host frame time
            // loses the ROM's every-other-pass DD40 gate.
            // Direct assets are cropped to their opaque bounds.  The reveal
            // counter is still expressed in the original 320px source
            // coordinate space, so include the crop origin when determining
            // the drawable extent.
            const auto logo_origin_x = static_cast<std::size_t>(
                std::max<std::int16_t>(title_logo_plane_.origin_x, 0));
            const auto logo_source_width = logo_origin_x +
                                           title_logo_plane_.width;
            const auto visible_pixels = std::min<std::size_t>(
                logo_source_width,
                kTitleLogoDestinationX +
                    static_cast<std::size_t>(title_revealed_columns_) * 8U);
            const auto logo_complete =
                title_revealed_columns_ >= kTitleRevealColumnCount;
            if (logo_complete && !title_background_.pixels.empty()) {
                const auto reveal_end =
                    static_cast<std::uint64_t>(kTitleRevealDelay) +
                    static_cast<std::uint64_t>(
                        (kTitleRevealColumnCount - 1U) * 2U + 1U);
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
            if (visible_pixels != 0U && !title_logo_plane_.pixels.empty()) {
                auto logo = title_logo_plane_.pixels;
                for (std::size_t y = 0; y < title_logo_plane_.height; ++y) {
                    for (std::size_t x = 0; x < title_logo_plane_.width; ++x) {
                        auto& pixel =
                            logo[y * title_logo_plane_.width + x];
                        if (logo_origin_x + x >= visible_pixels) {
                            pixel.alpha = 0;
                        }
                    }
                }
                screen.blit_rgba(title_logo_plane_.origin_x,
                                 title_logo_plane_.origin_y,
                                 title_logo_plane_.width,
                                 title_logo_plane_.height,
                                 scale_rgba(logo, title_fade_weight,
                                            kBootFadeFrames));
            }
            // 0x1EAA marks the logo-complete state as soon as the visible
            // explosion has settled. The source still performs the final
            // column/SAT pass on that boundary, but the transient SAT flame
            // must not remain over the settled logo.
            if (title_completion_state_ == 0U &&
                title_sprite_phase_ < title_sprite_frames_.size() &&
                title_sprite_x_ != kTitleSpriteInitialX) {
                const auto& sprite = title_sprite_frames_[title_sprite_phase_];
                if (!sprite.pixels.empty()) {
                    const auto sprite_delta = static_cast<std::int32_t>(
                        title_sprite_x_ -
                        static_cast<std::int16_t>(kTitleSpriteInitialX + 4));
                    screen.blit_rgba(
                        sprite_delta + sprite.origin_x, sprite.origin_y,
                        sprite.width, sprite.height,
                        scale_rgba(sprite.pixels, title_fade_weight,
                                   kBootFadeFrames));
                }
            }
            if (!title_text_.pixels.empty()) {
                auto blit_title_rows = [&screen, this](std::size_t first_row,
                                                        std::size_t row_count) {
                    auto text = title_text_.pixels;
                    const auto first_y = static_cast<std::int32_t>(first_row * 8U) -
                                         title_text_.origin_y;
                    const auto last_y = static_cast<std::int32_t>(
                        (first_row + row_count) * 8U) - title_text_.origin_y;
                    for (std::size_t y = 0; y < title_text_.height; ++y) {
                        if (static_cast<std::int32_t>(y) < first_y ||
                            static_cast<std::int32_t>(y) >= last_y) {
                            for (std::size_t x = 0; x < title_text_.width; ++x) {
                                text[y * title_text_.width + x].alpha = 0;
                            }
                        }
                    }
                    screen.blit_rgba(title_text_.origin_x,
                                     title_text_.origin_y,
                                     title_text_.width,
                                     title_text_.height, text);
                };
                // 0x1E4E sets DD5A=1 when FFF1E6 becomes $25 (column 37),
                // then 0x1C1E counts DD5C from $5A through -1 before the
                // copyright writer runs. The state is updated in source order
                // by update_title, so this does not guess from frame numbers.
                if (title_completion_state_ >= 2U) {
                    blit_title_rows(22U, 4U);
                }
                // The ROM alternates the prompt and its blank string through
                // 0x1DCA/0x1DE0. The visual capture shows the prompt arriving
                // shortly after the copyright block, so anchor that cadence
                // to the actual copyright-write state instead of the old
                // unrelated 0xE10 countdown (which delayed it several
                // seconds too long).
                constexpr std::uint64_t kPromptDelay = 45U;
                const auto after_copyright =
                    has_title_loop && title_completion_state_ >= 2U &&
                    title_loop >= title_copyright_frame_;
                const auto copyright_elapsed =
                    after_copyright
                        ? title_loop - title_copyright_frame_
                        : 0U;
                if (after_copyright && title_fade_left_ == 0U &&
                    title_completion_state_ >= 2U &&
                    copyright_elapsed >= kPromptDelay &&
                    (copyright_elapsed & 0x3FU) >= 10U) {
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
                104, 80 + static_cast<std::int32_t>(start_options_selection_) * 16,
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
                    136, 112 + static_cast<std::int32_t>(save_load_selection_) * 16,
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
        if (intro_scene_ != nullptr) {
            intro_scene_->render(screen);
        } else {
            screen.clear({0x00, 0x00, 0x00, 0xFF});
        }
        screen.present();
        return;
    } else if (scene_ == NativeScene::CabMap) {
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        // ROM 0x134D2's 26x17 map plate is extracted one-to-one with its
        // plane origin (16,16); that lands at screen (46,14). The selection
        // outline is a separate 0xD4446 border composition, never baked
        // into the map art.
        if (!cab_map_.pixels.empty()) {
            screen.blit_rgba(46, 14, cab_map_.width, cab_map_.height,
                             cab_map_.pixels);
        }
        // 0x13726 does not jump directly to the terminal cursor. It walks
        // D4536's shrinking outline over the D4464 position words, then
        // writes the caption. Reconstruct the two source-backed destination
        // zooms in native presentation state so the cab ride is an animation
        // rather than a static map plate.
        const auto destination_frame = cab_frame_ <
                5U * kCabCursorPhaseFrames
            ? cab_frame_
            : cab_frame_ - 5U * kCabCursorPhaseFrames;
        const auto cursor_phase = static_cast<std::size_t>(
            std::min<std::uint64_t>(
                destination_frame / kCabCursorPhaseFrames, 4U));
        const auto& cursor = cab_frame_ < 5U * kCabCursorPhaseFrames
            ? kCabDowntownCursor[cursor_phase]
            : kCabRedmondCursor[cursor_phase];
        draw_cab_cursor(screen, cursor);
        // 0x13726 ultimately calls C0DA with the startup character table
        // (C1320) and its source text is rendered as the pale map caption,
        // not with the green command-menu palette.  The two native atlases
        // share the same glyph table; use the startup palette here so the
        // cab caption keeps the source's white-on-black appearance.
        write_startup_text(
            screen, 88, 152,
            cab_frame_ < 5U * kCabCursorPhaseFrames
                ? " Downtown Seattle" : "  Redmond Barrens");
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
        if (field_entity_walking_) {
            // A walking state is a distinct negative-marker list in the ROM.
            // Never fall back to the directional idle/effect table here: that
            // fallback was the visible "blocking/fire-hit" pose defect when
            // a walking selector was unresolved.
            if (animation != field_entity_animations_.end() &&
                field_entity_.frame_index < animation->frames.size()) {
                frame = &animation->frames[field_entity_.frame_index];
            }
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
            if (!field_menu_background_.pixels.empty()) {
                screen.blit_rgba(0, 0, field_menu_background_.width,
                                 field_menu_background_.height,
                                 field_menu_background_.pixels);
            }
            if (!field_menu_inventory_boxes_.pixels.empty()) {
                screen.blit_rgba(0, 0, field_menu_inventory_boxes_.width,
                                 field_menu_inventory_boxes_.height,
                                 field_menu_inventory_boxes_.pixels);
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
                    if (include_art) {
                        write_field_menu_text(
                            screen, 128, 64,
                            center_source_fixed_field("This slot is"));
                        write_field_menu_text(
                            screen, 128, 72,
                            center_source_fixed_field("empty"));
                    } else if (field_entity_.display_state == 0x7F &&
                               field_entity_.active_item_code == 0U) {
                        // ROM 0x12D74 takes the no-equipped-item branch of
                        // 0x12CEE.  Its +0x85 source values are rendered by
                        // the same detail writer as a weapon, but the label
                        // is the literal Empty Hand pair and its fixed
                        // unarmed Damage/Power values.  The previous native
                        // path returned here and left the overview blank.
                        write_field_menu_text(
                            screen, 128, 64,
                            center_source_fixed_field("Empty"));
                        write_field_menu_text(
                            screen, 128, 72,
                            center_source_fixed_field("Hand"));
                        write_field_menu_text(screen, 112, 80, "Damage");
                        write_field_menu_text(screen, 208, 80, "Stun");
                        write_field_menu_text(screen, 112, 88, "Power");
                        write_field_menu_text(screen, 168, 88, "Minor");
                    }
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
                    write_field_menu_text(
                        screen, 128, 64,
                        center_source_fixed_field(item.name));
                }
                if (!item.category.empty()) {
                    write_field_menu_text(
                        screen, 128, 72,
                        center_source_fixed_field(item.category));
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
                // 0x115F2 skips 0x12E9E for armor. The selected armor view
                // keeps the source armor-specific bars: the extractor maps
                // D4D50+$26 to Wpns and D4D50+$25 to Guns, just as the main
                // overview's 0x11352 path does. A generic red Armor bar is
                // not part of this selected-item screen.
                const auto is_armor_detail = item_code >= 0x2CU &&
                                              item_code <= 0x32U;
                if (is_armor_detail) {
                    write_field_menu_text(screen, 112, 80, "Wpns");
                    if (field_entity_.weapons_bar_index <
                        field_menu_damage_bars_.size()) {
                        const auto& weapons_bar = field_menu_damage_bars_[
                            field_entity_.weapons_bar_index];
                        if (!weapons_bar.pixels.empty()) {
                            screen.blit_rgba(168, 80, weapons_bar.width,
                                             weapons_bar.height,
                                             weapons_bar.pixels);
                        }
                    }
                    write_field_menu_text(screen, 112, 88, "Guns");
                    if (field_entity_.guns_bar_index <
                        field_menu_drain_bars_.size()) {
                        const auto& guns_bar = field_menu_drain_bars_[
                            field_entity_.guns_bar_index];
                        if (!guns_bar.pixels.empty()) {
                            screen.blit_rgba(168, 88, guns_bar.width,
                                             guns_bar.height, guns_bar.pixels);
                        }
                    }
                }
                // 0x12A8C writes ammo/uses/rtng only for the selected
                // inventory cell (include_art). Overview 0x11260 keeps
                // Damage/Power and must not be overwritten.
                const auto detail_label = include_art
                    ? source_item_detail_label(item_code)
                    : std::string_view{};
                if (!detail_label.empty()) {
                    // 0x12B8E emits ammo/uses/rtng at C31C (x=112,y=48),
                    // while its two-tile phase value goes to C39E (x=120,
                    // y=56). Keeping these above the C420 item name avoids
                    // the source label/value being painted over Damage.
                    write_field_menu_text(screen, 112, 48, detail_label);
                    // The source value is a two-tile entry selected from
                    // 0x6AC46 by the selected slot's +0x6E byte. It is a
                    // source-rendered value, not a host numeric statistic.
                    const auto phase = source_inventory_phase_for_code(
                        field_entity_, item_code);
                    if (phase < field_menu_detail_values_.size()) {
                        const auto& value = field_menu_detail_values_[phase];
                        if (!value.pixels.empty()) {
                            screen.blit_rgba(120, 56, value.width, value.height,
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
                // the posture strip at CAAC. The six-tile block at CD514 is
                // the source's light focus outline, written at CA9E.
                if (!field_menu_posture_focus_.pixels.empty()) {
                    screen.blit_rgba(
                        120, 168, field_menu_posture_focus_.width,
                        field_menu_posture_focus_.height,
                        field_menu_posture_focus_.pixels);
                }
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
            // 0x11592 emits the source 3x4-tile selector at the same 0x28/0x18
            // strides used by the cursor bytes. Do not redraw it as a host
            // rectangle: its gaps, palette, and corner treatment are visible
            // in the original menu.
            if (field_menu_selection_ < 2U) {
                const auto x = 8 + static_cast<std::int32_t>(
                    field_menu_selection_ * 40U);
                const auto y = 112 + static_cast<std::int32_t>(
                    field_menu_row_ * 24U);
                if (!field_menu_cursor_.pixels.empty()) {
                    screen.blit_rgba(x, y, field_menu_cursor_.width,
                                     field_menu_cursor_.height,
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
                // 0x117A0 first writes the complete CD302 action row at
                // $EB9E, then 0x117B6 overlays the selected 2x2 block from
                // CD41A. Keep both operations source-derived so the old
                // CP/AS/CY/HG/PS row cannot leak through and no host button
                // geometry is substituted for the cartridge's bevels.
                if (!field_menu_action_background_.pixels.empty()) {
                    screen.blit_rgba(
                        120, 184, field_menu_action_background_.width,
                        field_menu_action_background_.height,
                        field_menu_action_background_.pixels);
                }
                const auto action_focus_index =
                    field_menu_action_selection_ >= 2U &&
                            field_menu_action_selection_ <= 5U
                        ? field_menu_action_selection_ - 2U : 0U;
                const auto& action_focus =
                    field_menu_action_focus_[action_focus_index];
                if (!action_focus.pixels.empty()) {
                    screen.blit_rgba(
                        kFieldMenuCommandButtonX[action_focus_index],
                        kFieldMenuCommandButtonY, action_focus.width,
                        action_focus.height, action_focus.pixels);
                }
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
            // 0xC2A6 passes C1418 to C0DA with A2=C216, which maps to
            // x=88 and puts the first option on the row at y=56.
            write_field_menu_text(screen, 88, 32, " Main Menu");
            write_field_menu_text(screen, 88, 40, "[\\\\\\\\\\\\\\\\");
            const auto option_count = pocket_secretary_option_count();
            for (std::size_t index = 0; index < option_count; ++index) {
                const auto selected = index == pocket_secretary_selection_;
                const auto y = 56 + static_cast<std::int32_t>(index) * 16;
                if (selected && !startup_cursor_.pixels.empty()) {
                    screen.blit_rgba(64, y, startup_cursor_.width,
                                     startup_cursor_.height,
                                     startup_cursor_.pixels);
                }
                write_field_menu_text(screen, 88, y,
                                      kPocketSecretaryLabels[index]);
            }
            if (pocket_secretary_message_visible_) {
                // C0DA receives A2=0xC40E from 0x18150. Preserve the three
                // extracted source lines as an in-panel response; this is
                // deliberately not a synthetic Matrix/cyberdeck screen.
                for (std::size_t index = 0;
                     index < cyberdeck_unavailable_lines_.size(); ++index) {
                    write_field_menu_text(
                        screen, 56,
                        64 + static_cast<std::int32_t>(index) * 8,
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
            // C448 writes C1499 at C216: title, prompt, separator, blank row,
            // then Abort at x=88. Dynamic names begin at C416 (y=64) and
            // advance one 16-pixel source row per active runner.
            write_field_menu_text(screen, 88, 32,
                                  dismiss_runner_labels_[0]);
            write_field_menu_text(screen, 88, 40,
                                  dismiss_runner_labels_[1]);
            for (std::size_t index = 0;
                 index < dismiss_runner_candidate_count_; ++index) {
                const auto record_index =
                    dismiss_runner_candidate_records_[index];
                const auto selected = dismiss_runner_selection_ == index + 1U;
                const auto y = 64 + static_cast<std::int32_t>(index) * 16;
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
                write_field_menu_text(screen, 88, y,
                                      name.empty() ? "UNKNOWN" : name);
            }
            const auto abort_y = 64 + static_cast<std::int32_t>(
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
            write_field_menu_text(screen, 88, abort_y,
                                  dismiss_runner_labels_[2]);
            write_field_menu_text(screen, 80, 176, " A SELECT  B RETURN");
        } else if (scene_ == NativeScene::ClipsMenu) {
            // 0x11B58 keeps the command-screen plane from 0x11900 and adds
            // only the source C2A2/C4B4 clip summary. The old host panel,
            // legend, and full-width selection bars covered that plane.
            render_command_static_layers(screen);
            screen.fill_rect(96, 16, 168, 144,
                             {0x00, 0x00, 0x00, 0xFF});
            const auto draw_clip_focus = [this, &screen](std::size_t selection) {
                if (selection < 2U || selection > 5U) {
                    return;
                }
                const auto& focus = clips_trade_active_
                    ? field_menu_clips_trade_focus_[selection - 2U]
                    : field_menu_clips_focus_[selection - 2U];
                if (!focus.pixels.empty()) {
                    screen.blit_rgba(
                        128 + static_cast<std::int32_t>((selection - 2U) * 24U),
                        184, focus.width, focus.height, focus.pixels);
                }
            };
            if (clips_trade_active_) {
                // 0x11E7E writes D1B61 at C4A4, the quantity at C4B2,
                // and 0x1209A writes the selected runner at C59E.
                write_field_menu_text(screen, 144, 72, "Trade");
                write_field_menu_text(screen, 144, 80, "clips to");
                const auto recipient = clips_trade_recipient_record_ <
                        field_menu_runner_names_.size()
                    ? std::string_view(field_menu_runner_names_[
                          clips_trade_recipient_record_])
                    : std::string_view("????");
                write_field_menu_text(screen, 120, 88, recipient);
                write_field_menu_text(screen, 144, 96, "  ????");
                write_field_menu_text(screen, 200, 72,
                                      std::to_string(clips_trade_quantity_));
                draw_clip_focus(clips_trade_menu_selection_);
                write_field_menu_text(
                    screen, 112, 200,
                    kClipsTradeMenuLabels[clips_trade_menu_selection_ - 2U]);
            } else if (clips_reload_confirmation_active_) {
                // 0x11D14 writes D1AFF at C49E and 0x127B0 places the
                // selected Yes/No row at EC9C.
                write_field_menu_text(screen, 120, 72, "You still have");
                write_field_menu_text(screen, 120, 80, "  ammo left.");
                write_field_menu_text(screen, 120, 96, "   Reload?");
                write_field_menu_text(
                    screen, 112, 200,
                    clips_reload_confirmation_yes_
                        ? "       Yes       " : "       No        ");
            } else if (clips_action_message_frames_ != 0 &&
                       !clips_action_message_.empty()) {
                if (clips_out_of_ammo_) {
                    const auto& name = active_field_record_ <
                            field_menu_runner_names_.size()
                        ? field_menu_runner_names_[active_field_record_]
                        : std::string{};
                    write_field_menu_text(screen, 112, 80, name);
                    write_field_menu_text(screen, 112, 88, "  is out of");
                    write_field_menu_text(screen, 112, 96, " ammo clips!");
                } else if (clips_action_message_ ==
                           "This item does not use clips!!") {
                    write_field_menu_text(screen, 136, 80, " This item");
                    write_field_menu_text(screen, 136, 88, " does not");
                    write_field_menu_text(screen, 136, 96, "use clips!!");
                } else if (clips_action_message_ ==
                           "You have no party members to trade with.") {
                    write_field_menu_text(screen, 120, 72, " You have no");
                    write_field_menu_text(screen, 120, 80, "party members");
                    write_field_menu_text(screen, 120, 88, "to trade with.");
                } else if (clips_action_message_ ==
                           "Rock & Roll, chummers!!") {
                    write_field_menu_text(screen, 112, 80, "Rock & Roll,");
                    write_field_menu_text(screen, 112, 88, " chummers!!");
                } else if (clips_action_message_ ==
                           "Exchange complete") {
                    write_field_menu_text(screen, 112, 80, "Exchange");
                    write_field_menu_text(screen, 112, 88, "complete");
                }
            } else {
                // D1AE0 through its first FF: C2A2 title/separator and the
                // C4B4 clip total. The action label is the single selected
                // row at EC9C, not a vertical checklist.
                write_field_menu_text(screen, 136, 40, "    CLIPS");
                const auto separator = std::string("   [") +
                    std::string(9U, '\\');
                write_field_menu_text(screen, 136, 48, separator);
                write_field_menu_text(screen, 136, 72, "  Clips");
                write_field_menu_text(screen, 208, 72,
                                      std::to_string(field_entity_.ammo_clips));
                draw_clip_focus(clips_menu_selection_);
                write_field_menu_text(
                    screen, 112, 200,
                    kClipsMenuLabels[clips_menu_selection_ - 2U]);
            }
        } else if (scene_ == NativeScene::ClipsMenu && false) {
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
            // ROM 0x11950 calls the same command-plane initializer, then
            // writes D2054 at C1A2 and only the installed bit names at C31C,
            // advancing each emitted name by 0x100. It does not show an ON/
            // OFF checklist.
            render_command_static_layers(screen);
            screen.fill_rect(96, 16, 168, 144,
                             {0x00, 0x00, 0x00, 0xFF});
            write_field_menu_text(screen, 136, 24, " CYBERWARE");
            const auto separator = std::string("[") +
                std::string(10U, '\\');
            write_field_menu_text(screen, 136, 32, separator);
            if (field_entity_.cyberware_flags == 0U) {
                write_field_menu_text(screen, 160, 48, "NONE");
            } else {
                std::size_t row = 0;
                for (std::size_t index = 0; index < cyberware_names_.size();
                     ++index) {
                    if ((field_entity_.cyberware_flags &
                         static_cast<std::uint16_t>(1U << index)) == 0U) {
                        continue;
                    }
                    const auto& name = cyberware_names_[index];
                    write_field_menu_text(
                        screen, 112, 48 + static_cast<std::int32_t>(row) * 16,
                        name.empty() ? "UNKNOWN" : std::string_view(name));
                    ++row;
                }
            }
        } else if (scene_ == NativeScene::CyberwareMenu && false) {
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
            // 0xD01C writes four attributes in each upper column and six
            // skills in each lower column. The name/value destinations are
            // C386/C3A2, C3AC/C3C6, C686/C6A2, and C6AC/C6C6.
            screen.clear({0x00, 0x00, 0x00, 0xFF});
            if (!field_menu_attributes_background_.pixels.empty()) {
                screen.blit_rgba(
                    0, 0, field_menu_attributes_background_.width,
                    field_menu_attributes_background_.height,
                    field_menu_attributes_background_.pixels);
            } else {
                // Old packs can still render the typed values while the
                // source-specific background is regenerated.
                render_command_base(screen);
            }
            render_command_dynamic_layers(screen, true);
            constexpr std::array<std::array<std::size_t, 10>, 2>
                kAttributeSkillColumns{{
                    {{0U, 1U, 2U, 3U, 8U, 9U, 10U, 11U, 12U, 13U}},
                    {{4U, 5U, 6U, 7U, 14U, 15U, 16U, 17U, 18U, 19U}},
                }};
            for (std::size_t column = 0; column < 2U; ++column) {
                const auto name_x = column == 0U ? 56 : 176;
                const auto value_x = column == 0U ? 144 : 280;
                for (std::size_t row = 0; row < 10U; ++row) {
                    const auto index = kAttributeSkillColumns[column][row];
                    const auto y = row < 4U
                        ? 56 + static_cast<std::int32_t>(row) * 8
                        : 104 + static_cast<std::int32_t>(row - 4U) * 8;
                    const auto value = index < kNativeAttributeCount
                        ? field_entity_.attribute_values[index]
                        : field_entity_.skill_values[index -
                                                      kNativeAttributeCount];
                    const auto& name = attribute_skill_names_[index];
                    // ROM 0x101FE/0xFEBE/0xFF06 keeps the selected attribute
                    // cursor in the source row/column state. The selected
                    // row is emitted through the light command palette line
                    // (the capture shows Quickness as pale text), not a host
                    // rectangle behind the row.
                    const auto selected = index ==
                        kAttributeSkillColumns[attributes_menu_column_]
                            [attributes_menu_row_];
                    const std::optional<Rgba8> selected_tint = selected
                        ? std::optional<Rgba8>{Rgba8{0xDB, 0xDB, 0xDB, 0xFF}}
                        : std::nullopt;
                    write_field_menu_text(
                        screen, name_x, y,
                        name.empty() ? "UNKNOWN" : std::string_view(name),
                        selected_tint);
                    write_field_menu_text(screen, value_x, y,
                                          std::to_string(value), selected_tint);
                }
            }
            // 0xD01C writes the current karma total beside the lower panel.
            // New-game records start at zero; keep the description tied to
            // the same cursor rather than leaving Quickness text painted
            // after navigating to another row.
            // KARMA is already part of the source panel tab.  0xD01C writes
            // only the current total beside that tab; painting the word again
            // at y=152 was the overlap seen between SKILLS and KARMA.
            write_field_menu_text(screen, 120, 160, "0");
            const auto selected_index =
                kAttributeSkillColumns[attributes_menu_column_]
                    [attributes_menu_row_];
            if (selected_index < kAttributeSkillDescriptions.size()) {
                const auto& description =
                    kAttributeSkillDescriptions[selected_index];
                for (std::size_t line = 0; line < description.size(); ++line) {
                    if (!description[line].empty()) {
                        write_field_menu_text(
                            screen, 56,
                            176 + static_cast<std::int32_t>(line * 8U),
                            description[line]);
                    }
                }
            }
        } else if (scene_ == NativeScene::AttributesMenu && false) {
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
            // ROM 0xFBA6 clears the center command plane, writes C89C0 at
            // C31C, and calls 0x106DA twice for the two defense strips at
            // C51E/C69E. It does not open the generic C3E6 dialog panel and
            // does not draw a host checklist of invented fields.
            render_command_static_layers(screen);
            screen.fill_rect(96, 16, 168, 144,
                             {0x00, 0x00, 0x00, 0xFF});
            write_field_menu_text(screen, 112, 48, " Magical Defense");
            write_field_menu_text(
                screen, 112, 56,
                std::string("[") + std::string(16U, '\\'));
            write_field_menu_text(screen, 112, 72, "  vs. Phys");
            write_field_menu_text(screen, 112, 96, "  vs. Mana");
            const auto defense_bar_index = field_entity_.defense_bar_index <
                    field_menu_defense_bars_.size()
                ? field_entity_.defense_bar_index : 0U;
            const auto& physical_bar =
                field_menu_defense_bars_[defense_bar_index];
            if (!physical_bar.pixels.empty()) {
                screen.blit_rgba(120, 80, physical_bar.width,
                                 physical_bar.height, physical_bar.pixels);
            }
            const auto& mana_bar = field_menu_defense_bars_[defense_bar_index];
            if (!mana_bar.pixels.empty()) {
                screen.blit_rgba(120, 104, mana_bar.width, mana_bar.height,
                                 mana_bar.pixels);
            }
        } else {
            // Field mode is the map/object plane plus its source HUD. The
            // old host-only control legend did not exist in the cartridge.
        }
        if (scene_ != NativeScene::Field &&
            scene_ != NativeScene::FieldDialog &&
            scene_ != NativeScene::AttributesMenu) {
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
