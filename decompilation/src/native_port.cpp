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
    entity_frames_ = std::move(assets.entity_frames);
    field_direction_table_ = assets.field_direction_table;
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
        assets.initial_field_records[0].world_x_fixed >> 4U);
    field_initial_world_y_ = static_cast<std::int32_t>(
        assets.initial_field_records[0].world_y_fixed >> 4U);
    field_initial_world_x_fixed_ =
        assets.initial_field_records[0].world_x_fixed;
    field_initial_world_y_fixed_ =
        assets.initial_field_records[0].world_y_fixed;
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
    field_entity_.world_x_fixed =
        assets.initial_field_records[0].world_x_fixed;
    field_entity_.world_y_fixed =
        assets.initial_field_records[0].world_y_fixed;
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
        field_initial_records_[0].world_x_fixed >> 4U);
    field_initial_world_y_ = static_cast<std::int32_t>(
        field_initial_records_[0].world_y_fixed >> 4U);
    field_initial_world_x_fixed_ = field_initial_records_[0].world_x_fixed;
    field_initial_world_y_fixed_ = field_initial_records_[0].world_y_fixed;
    field_world_x_ = field_initial_world_x_;
    field_world_y_ = field_initial_world_y_;
    field_camera_x_ = field_initial_camera_x_;
    field_camera_y_ = field_initial_camera_y_;
    field_entity_.record_index = 0;
    field_entity_.world_x_fixed = field_initial_world_x_fixed_;
    field_entity_.world_y_fixed = field_initial_world_y_fixed_;
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

void NativePortShell::activate_field_record(std::size_t record_index) {
    if (record_index >= field_records_.size() ||
        record_index == active_field_record_) {
        return;
    }

    // ROM 0x391A changes the controlled record, but does not discard the
    // record that was just left. Retain its current fixed-point position in
    // the native party state before selecting the next proven slot.
    field_records_[active_field_record_] = {
        field_entity_.world_x_fixed, field_entity_.world_y_fixed};
    active_field_record_ = record_index;
    field_entity_.record_index = static_cast<std::uint8_t>(record_index);
    field_entity_.world_x_fixed = field_records_[record_index].world_x_fixed;
    field_entity_.world_y_fixed = field_records_[record_index].world_y_fixed;
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

void NativePortShell::update(const InputState& input) {
    input_ = input;

    if (scene_ == NativeScene::Title) {
        // ROM 0x0BF2 records Start as a pressed high-byte input and the
        // 0x1A9E loop exits when FFDDC2 is set. The native equivalent is the
        // supplied pressed edge, not a held-button poll.
        if (input.was_pressed(Button::Start)) {
            scene_ = NativeScene::Field;
            if (!field_map_pixels_.empty()) {
                reset_field_state();
            } else {
                marker_x_ = kScreenWidth / 2;
                marker_y_ = kScreenHeight / 2;
            }
        }
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
            reset_field_state();
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
        field_records_[active_field_record_] = {
            field_entity_.world_x_fixed, field_entity_.world_y_fixed};
    }
    if (input.was_pressed(Button::C) && !field_map_pixels_.empty()) {
        // The main field loop dispatches raw 0x2000 to 0x391A after its
        // ordinary entity pass. Raw 0x2000 is the Genesis C edge; on a
        // three-record startup party this selects the next controlled record
        // in the same order as the ROM loop.
        activate_field_record(
            (active_field_record_ + 1U) % field_records_.size());
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
    } else if (scene_ == NativeScene::Field && !field_view_pixels_.empty()) {
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
        screen.fill_rect(0, 204, kScreenWidth, 20,
                         {0x02, 0x05, 0x0C, 0xFF});
        screen.write_text(8, 208, "ARROWS MOVE ENTITY  C SWITCH RUNNER", 1,
                          {0xE8, 0xF4, 0xFF, 0xFF});
        screen.write_text(8, 216, "ENTER RESET  Z/X PENDING", 1,
                          {0x90, 0xB8, 0xC8, 0xFF});
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
