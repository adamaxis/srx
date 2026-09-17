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
constexpr std::size_t kIntroLinesPerPage = 5;

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
    "Inventory",
    "Inventory",
    "Clips",
    "Attributes/Skills",
    "Cyberware",
    "Magic",
    "Pocket Secretary",
}};

// ROM 0x11B58 enters this four-option clip submenu with selector two. The
// 0x118C6 helper keeps the selector in the inclusive range 2..5.
constexpr std::array<std::string_view, 4> kClipsMenuLabels{{
    "Increase Clips",
    "Decrease Clips",
    "Trade Clips",
    "Change Recipient",
}};

} // namespace

NativePortShell::NativePortShell(std::string title)
    : title_(std::move(title)) {}

NativePortShell::NativePortShell(
    std::string title, const std::vector<std::uint8_t>& rom)
    : NativePortShell(
          std::move(title), graphics::decode_genesis_native_assets(rom)) {}

NativePortShell::NativePortShell(std::string title, NativePortAssets assets)
    : title_(std::move(title)), scene_(NativeScene::Title) {
    title_width_ = assets.title_scene.width;
    title_height_ = assets.title_scene.height;
    title_pixels_ = std::move(assets.title_scene.pixels);
    intro_ = std::move(assets.intro);
    entity_frames_ = std::move(assets.entity_frames);
    field_direction_table_ = assets.field_direction_table;
    inventory_items_ = std::move(assets.inventory_items);
    cyberware_names_ = std::move(assets.cyberware_names);
    attribute_skill_names_ = std::move(assets.attribute_skill_names);
    field_initial_records_ = assets.initial_field_records;
    field_records_ = field_initial_records_;
    field_terrain_ = std::move(assets.field_terrain);
    field_map_width_ = assets.field_map.width;
    field_map_height_ = assets.field_map.height;
    field_map_pixels_ = std::move(assets.field_map.pixels);

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
    field_entity_ = std::move(assets.entity_template);
    field_entity_.record_index = 0;
    field_entity_.record_type = field_records_[0].record_type;
    field_entity_.flags_10 = field_records_[0].flags_10;
    field_entity_.flags_11 = field_records_[0].flags_11;
    field_initial_equipped_armor_ = field_entity_.equipped_armor_index;
    field_equipped_armor_by_record_.fill(field_initial_equipped_armor_);
    field_entity_.world_x_fixed =
        assets.initial_field_records[0].position.world_x_fixed;
    field_entity_.world_y_fixed =
        assets.initial_field_records[0].position.world_y_fixed;
    refresh_field_view();
    refresh_field_entity_projection();
    marker_x_ = field_entity_screen_x();
    marker_y_ = field_entity_screen_y();
    refresh_field_collision();
}

void NativePortShell::reset_field_state() {
    field_records_ = field_initial_records_;
    active_field_record_ = 0;
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
    field_entity_.record_index = 0;
    field_entity_.record_type = field_records_[0].record_type;
    field_entity_.flags_10 = field_records_[0].flags_10;
    field_entity_.flags_11 = field_records_[0].flags_11;
    field_entity_.world_x_fixed = field_initial_world_x_fixed_;
    field_entity_.world_y_fixed = field_initial_world_y_fixed_;
    field_equipped_armor_by_record_.fill(field_initial_equipped_armor_);
    field_entity_.equipped_armor_index = field_initial_equipped_armor_;
    field_entity_.frame_index = 0;
    field_entity_.state_selector = 0;
    field_entity_.state_index = 0;
    field_entity_.animation_accumulator = 0;
    refresh_field_view();
    refresh_field_entity_projection();
    marker_x_ = field_entity_screen_x();
    marker_y_ = field_entity_screen_y();
    move_rejected_ = false;
    refresh_field_collision();
}

void NativePortShell::update_start_options(const InputState& input) {
    if (input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        start_options_selection_ = start_options_selection_ == 0
            ? intro_.start_options.size() - 1U
            : start_options_selection_ - 1U;
    } else if (input.is_held(Button::Down) &&
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

    // The reviewed title data contains a Load Game branch, but no native save
    // format has been reconstructed yet. Preserve its actual no-save result
    // rather than silently starting a new game from the wrong menu entry.
    start_options_load_rejected_ = true;
}

void NativePortShell::update_archetype_menu(const InputState& input) {
    if (input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        archetype_selection_ = archetype_selection_ == 0
            ? intro_.archetypes.size() - 1U
            : archetype_selection_ - 1U;
    } else if (input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        archetype_selection_ =
            (archetype_selection_ + 1U) % intro_.archetypes.size();
    }

    if (input.was_pressed(Button::Start)) {
        scene_ = NativeScene::StartOptions;
        return;
    }
    if (!input.was_pressed(Button::A) && !input.was_pressed(Button::B)) {
        return;
    }
    intro_line_start_ = 0;
    scene_ = intro_.story_lines.empty() ? NativeScene::Field
                                        : NativeScene::Intro;
    if (scene_ == NativeScene::Field && !field_map_pixels_.empty()) {
        reset_field_state();
    }
}

void NativePortShell::update_intro(const InputState& input) {
    if (!input.was_pressed(Button::A) && !input.was_pressed(Button::B) &&
        !input.was_pressed(Button::Start)) {
        return;
    }
    if (intro_line_start_ + kIntroLinesPerPage < intro_.story_lines.size()) {
        intro_line_start_ += kIntroLinesPerPage;
        return;
    }
    scene_ = NativeScene::Field;
    if (!field_map_pixels_.empty()) {
        reset_field_state();
    }
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
    active_field_record_ = record_index;
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
    // ROM 0x36FC checks the signed template byte at +0x56 after projection.
    // Keep that display-state gate in native state even though the current
    // extracted startup template is displayable (0x00).
    const auto displayable = field_entity_.display_state >= 0 &&
                             field_entity_.display_state != 0x7F;
    field_entity_.visible = result.visible && displayable;
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
    if ((delta_x == 0 && delta_y == 0) || entity_frames_.empty()) {
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
    field_entity_.frame_index = direction % entity_frames_.size();
    field_entity_.state_selector = static_cast<std::uint16_t>(direction * 4U);
    field_entity_.state_index = 0;
    field_entity_.animation_accumulator = 0;
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
    field_last_command_selection_ = 0;
    field_last_command_row_ = 0;
    field_command_message_frames_ = 0;
}

void NativePortShell::update_field_menu(const InputState& input) {
    // 0x114A0 changes FFF0D0 for left/right. It wraps the seven native
    // selector values with the same 6 -> 0 and 0 -> 6 behavior.
    const auto cycle_selection = [this](std::int32_t delta) {
        constexpr auto count = kFieldMenuLabels.size();
        const auto current = static_cast<std::int32_t>(field_menu_selection_);
        const auto next = (current + delta + static_cast<std::int32_t>(count)) %
                          static_cast<std::int32_t>(count);
        field_menu_selection_ = static_cast<std::size_t>(next);
    };
    if (input.is_held(Button::Left) && !input.is_held(Button::Right)) {
        cycle_selection(-1);
    } else if (input.is_held(Button::Right) &&
               !input.is_held(Button::Left)) {
        cycle_selection(1);
    }

    // 0x11564 changes FFF0D1 between zero and three. This secondary row is
    // retained even where the corresponding submenu has not been ported.
    if (input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        field_menu_row_ = field_menu_row_ == 0 ? 3 : field_menu_row_ - 1;
    } else if (input.is_held(Button::Down) &&
               !input.is_held(Button::Up)) {
        field_menu_row_ = (field_menu_row_ + 1U) % 4U;
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
    // ROM 0x1219C enters this branch for armor item codes 0x2C and above;
    // 0x121B4 stores code-0x2B in +0x76 and the selected +0x6E byte. A second
    // activation clears both, matching the ROM toggle without pretending the
    // unrelated weapon/medical handlers are already ported.
    if (item_code < 0x2CU) {
        return;
    }
    if (field_entity_.equipped_armor_index != 0U) {
        field_entity_.equipped_armor_index = 0;
        field_entity_.inventory_phase_bytes[slot] = 0;
    } else {
        field_entity_.equipped_armor_index =
            static_cast<std::uint8_t>(item_code - 0x2BU);
        field_entity_.inventory_phase_bytes[slot] =
            field_entity_.equipped_armor_index;
    }
    field_equipped_armor_by_record_[active_field_record_] =
        field_entity_.equipped_armor_index;
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
}

void NativePortShell::update_clips_menu(const InputState& input) {
    // ROM 0x118C6 applies the left/right delta and wraps 2 -> 5 and 5 -> 2.
    if (input.is_held(Button::Left) && !input.is_held(Button::Right)) {
        clips_menu_selection_ = clips_menu_selection_ == 2U
            ? 5U : clips_menu_selection_ - 1U;
    } else if (input.is_held(Button::Right) &&
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
}

void NativePortShell::update_attributes_menu(const InputState& input) {
    // ROM 0xD27C keeps the first selector in the two-column range 0..1.
    if (input.is_held(Button::Left) && !input.is_held(Button::Right)) {
        attributes_menu_column_ = 0;
    } else if (input.is_held(Button::Right) &&
               !input.is_held(Button::Left)) {
        attributes_menu_column_ = 1;
    }
    // ROM 0xD2DA/0xD302 clamps the secondary selector to rows 0..9. The
    // native view uses the same two-column, ten-row presentation boundary.
    if (input.is_held(Button::Up) && !input.is_held(Button::Down)) {
        if (attributes_menu_row_ != 0) {
            --attributes_menu_row_;
        }
    } else if (input.is_held(Button::Down) &&
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
        // 0x1A9E loop exits when FFDDC2 is set. The native equivalent is the
        // supplied pressed edge, not a held-button poll.
        if (input.was_pressed(Button::Start)) {
            scene_ = has_intro() ? NativeScene::StartOptions
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

    if (scene_ == NativeScene::StartOptions) {
        update_start_options(input);
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
            delta_x = 0;
            delta_y = 0;
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
    update_field_entity_direction(delta_x, delta_y);
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
        screen.blit_rgba(0, 0, title_width_, title_height_, title_pixels_);
        // These are the actual strings selected by the ROM's 0x1DCA and
        // 0x1DF6 startup paths. Their original tile codes are converted by
        // the host text primitive instead of requiring a Genesis font table.
        screen.write_text(112, 168, "PRESS START", 1,
                          {0xF0, 0xC0, 0x50, 0xFF});
        screen.write_text(16, 176,
                          "SHADOWRUN (c) 1989, 1993 FASA CORP.", 1,
                          {0xB0, 0xB8, 0xC8, 0xFF});
        screen.fill_rect(0, 208, kScreenWidth, 16,
                         {0x00, 0x00, 0x00, 0xB0});
        screen.write_text(8, 211, "ARROWS MOVE  Z X C  ENTER START",
                          1, {0xE8, 0xF4, 0xFF, 0xFF});
    } else if (scene_ == NativeScene::StartOptions) {
        screen.clear({0x04, 0x08, 0x18, 0xFF});
        if (!title_pixels_.empty()) {
            screen.blit_rgba(0, 0, title_width_, title_height_, title_pixels_);
        }
        screen.fill_rect(34, 48, 252, 128, {0x02, 0x08, 0x18, 0xF4});
        screen.fill_rect(34, 48, 252, 2, {0x00, 0xA8, 0xC8, 0xFF});
        screen.write_text(48, 58, intro_.start_options_title, 2,
                          {0xE8, 0xF4, 0xFF, 0xFF});
        for (std::size_t index = 0; index < intro_.start_options.size();
             ++index) {
            const auto y = 92 + static_cast<std::int32_t>(index) * 20;
            const auto selected = index == start_options_selection_;
            if (selected) {
                screen.fill_rect(48, y - 3, 220, 15,
                                 {0x00, 0x68, 0x88, 0xFF});
            }
            screen.write_text(60, y, intro_.start_options[index], 1,
                              selected
                                  ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                                  : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
        }
        if (start_options_load_rejected_) {
            screen.write_text(50, 136, "THERE'S NO GAME SAVED HERE!", 1,
                              {0xF0, 0xC0, 0x50, 0xFF});
        }
        screen.write_text(48, 160, "ARROWS SELECT  Z/X CHOOSE", 1,
                          {0x90, 0xB8, 0xC8, 0xFF});
        screen.present();
        return;
    } else if (scene_ == NativeScene::ArchetypeMenu) {
        screen.clear({0x04, 0x08, 0x18, 0xFF});
        if (!title_pixels_.empty()) {
            screen.blit_rgba(0, 0, title_width_, title_height_, title_pixels_);
        }
        screen.fill_rect(34, 40, 252, 148, {0x02, 0x08, 0x18, 0xF4});
        screen.fill_rect(34, 40, 252, 2, {0x00, 0xA8, 0xC8, 0xFF});
        screen.write_text(48, 50, intro_.archetype_title, 2,
                          {0xE8, 0xF4, 0xFF, 0xFF});
        for (std::size_t index = 0; index < intro_.archetypes.size();
             ++index) {
            const auto y = 84 + static_cast<std::int32_t>(index) * 18;
            const auto selected = index == archetype_selection_;
            if (selected) {
                screen.fill_rect(48, y - 3, 220, 14,
                                 {0x00, 0x68, 0x88, 0xFF});
            }
            screen.write_text(60, y, intro_.archetypes[index], 1,
                              selected
                                  ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                                  : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
        }
        screen.write_text(48, 166, "ARROWS SELECT  Z/X CHOOSE", 1,
                          {0x90, 0xB8, 0xC8, 0xFF});
        screen.write_text(48, 178, "ENTER BACK", 1,
                          {0x90, 0xB8, 0xC8, 0xFF});
        screen.present();
        return;
    } else if (scene_ == NativeScene::Intro) {
        screen.clear({0x02, 0x06, 0x12, 0xFF});
        screen.fill_rect(0, 0, kScreenWidth, 3, {0x00, 0xA8, 0xC8, 0xFF});
        screen.write_text(20, 16, "SHADOWRUN", 2,
                          {0xE8, 0xF4, 0xFF, 0xFF});
        screen.write_text(202, 20, intro_.archetypes[archetype_selection_], 1,
                          {0xA0, 0xE8, 0xF8, 0xFF});
        screen.fill_rect(14, 48, 292, 122, {0x08, 0x12, 0x28, 0xFF});
        screen.fill_rect(14, 48, 292, 2, {0x00, 0x68, 0x88, 0xFF});
        const auto line_end = std::min(
            intro_line_start_ + kIntroLinesPerPage, intro_.story_lines.size());
        for (auto line = intro_line_start_; line < line_end; ++line) {
            screen.write_text(
                22, 62 + static_cast<std::int32_t>(line - intro_line_start_) * 18,
                intro_.story_lines[line], 1, {0xD0, 0xE0, 0xF0, 0xFF});
        }
        screen.write_text(20, 184, "Z/X/ENTER NEXT", 1,
                          {0x90, 0xB8, 0xC8, 0xFF});
        screen.present();
        return;
    } else if ((scene_ == NativeScene::Field ||
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
        screen.write_text(204, 10, "MODE 2", 1,
                          {0xA0, 0xE8, 0xF8, 0xFF});

        screen.blit_rgba(0, 36, field_view_width_, field_view_height_,
                         field_view_pixels_);

        if (field_entity_.visible && !entity_frames_.empty()) {
            const auto& frame = entity_frames_[field_entity_.frame_index];
            const auto entity_x = static_cast<std::int32_t>(
                static_cast<std::int16_t>(field_entity_.screen_x));
            const auto entity_y = static_cast<std::int32_t>(
                static_cast<std::int16_t>(field_entity_.screen_y)) +
                kFieldScreenOriginY;
            screen.fill_rect(entity_x - 7, entity_y - 2, 14, 4,
                             {0x00, 0x00, 0x00, 0xB0});
            screen.blit_rgba(
                entity_x - frame.width / 2, entity_y - frame.height,
                frame.width, frame.height, frame.pixels);
        } else {
            // Keep the input seam visible if a ROM is supplied without the
            // reviewed entity descriptor.
            screen.fill_rect(marker_x_, marker_y_, 8, 12, marker_color_);
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
            // This is a renderer-neutral host presentation of the ROM
            // command screen. The field remains underneath because the
            // original routine changes the VDP/menu state without changing
            // the active field record.
            screen.fill_rect(24, 44, 272, 144,
                             {0x02, 0x08, 0x18, 0xF0});
            screen.fill_rect(24, 44, 272, 2,
                             {0x00, 0xA8, 0xC8, 0xFF});
            screen.write_text(36, 52, "FIELD COMMANDS", 2,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            for (std::size_t index = 0; index < kFieldMenuLabels.size();
                 ++index) {
                const auto y = 72 + static_cast<std::int32_t>(index) * 14;
                const auto selected = index == field_menu_selection_;
                if (selected) {
                    screen.fill_rect(32, y - 2, 256, 11,
                                     {0x00, 0x68, 0x88, 0xFF});
                }
                screen.write_text(40, y, kFieldMenuLabels[index], 1,
                                  selected
                                      ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                                      : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
            }
            if (field_menu_selection_ < 2U) {
                // ROM 0x12148 addresses one of eight item bytes using the
                // selected inventory column and row. Show that same slot
                // selection with the extracted host item catalog.
                screen.fill_rect(154, 60, 132, 100,
                                 {0x04, 0x12, 0x24, 0xFF});
                screen.write_text(162, 64,
                                  "INVENTORY " +
                                      std::to_string(field_menu_selection_),
                                  1, {0xA0, 0xE8, 0xF8, 0xFF});
                for (std::size_t row = 0; row < 4U; ++row) {
                    const auto slot = row * 2U + field_menu_selection_;
                    const auto selected = row == field_menu_row_;
                    const auto y = 80 + static_cast<std::int32_t>(row) * 18;
                    if (selected) {
                        screen.fill_rect(158, y - 2, 124, 14,
                                         {0x00, 0x68, 0x88, 0xFF});
                    }
                    const auto code = field_entity_.inventory_item_codes[slot];
                    std::string item_name = "EMPTY";
                    if (code != 0U &&
                        code <= inventory_items_.size() &&
                        !inventory_items_[code - 1U].name.empty()) {
                        item_name = inventory_items_[code - 1U].name;
                    }
                    if (code >= 0x2CU &&
                        field_entity_.equipped_armor_index ==
                            static_cast<std::uint8_t>(code - 0x2BU)) {
                        item_name += " [ON]";
                    }
                    screen.write_text(
                        162, y,
                        "R" + std::to_string(row) + " " + item_name,
                        1,
                        selected
                            ? Rgba8{0xFF, 0xF0, 0x90, 0xFF}
                            : Rgba8{0xD0, 0xE0, 0xF0, 0xFF});
                }
            }
            screen.write_text(36, 174,
                              "ARROWS SELECT  Z/X USE  C RUNNER  ENTER EXIT",
                              1, {0x90, 0xB8, 0xC8, 0xFF});
            screen.fill_rect(0, 204, kScreenWidth, 20,
                             {0x02, 0x05, 0x0C, 0xFF});
            screen.write_text(8, 208, "FIELD COMMAND SCREEN", 1,
                              {0xE8, 0xF4, 0xFF, 0xFF});
            screen.write_text(8, 216,
                              "ROW " + std::to_string(field_menu_row_) +
                                  "  COMMAND " +
                                  std::to_string(field_command_count_),
                              1, {0x90, 0xB8, 0xC8, 0xFF});
            if (field_command_message_frames_ != 0) {
                screen.write_text(174, 216, "USED", 1,
                                  {0xF0, 0xC0, 0x50, 0xFF});
            }
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
