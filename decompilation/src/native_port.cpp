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

NativePortShell::NativePortShell(std::string title)
    : title_(std::move(title)) {}

NativePortShell::NativePortShell(
    std::string title, const std::vector<std::uint8_t>& rom)
    : title_(std::move(title)), scene_(NativeScene::Title) {
    const auto scene = graphics::decode_genesis_startup_screen(rom);
    rom_scene_width_ = scene.width;
    rom_scene_height_ = scene.height;
    rom_scene_pixels_ = scene.pixels;

    // ROM 0x600E reaches the 0x8002 frame descriptor at 0x9540 for the first
    // reviewed entity-state branch. 0x3E86 expands its packed 0x86846 source
    // before the VBlank queue consumes it; extraction converts that result to
    // transparent host RGBA so the native scene can draw it directly.
    const auto entity_frame = graphics::decode_genesis_entity_frame(rom);
    auto field_map = graphics::decode_genesis_mode2_field_map(rom);
    rom_entity_frame_width_ = entity_frame.width;
    rom_entity_frame_height_ = entity_frame.height;
    rom_entity_frame_pixels_ = entity_frame.pixels;

    // Mode 2 starts with camera coordinates 0x3400/0x2280 in the selected
    // ROM record. They are 1/16-pixel values, so the native state keeps their
    // pixel conversion and follows the same world position as the marker.
    rom_field_map_width_ = field_map.width;
    rom_field_map_height_ = field_map.height;
    rom_field_map_pixels_ = std::move(field_map.pixels);
    field_world_x_ = static_cast<std::int32_t>(0x3400U >> 4U);
    field_world_y_ = static_cast<std::int32_t>(0x2280U >> 4U);
    field_camera_x_ = field_world_x_;
    field_camera_y_ = field_world_y_;
    refresh_field_view();
    refresh_field_entity_projection();
}

namespace {

std::uint16_t fixed_world_coordinate(std::int32_t pixels) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(pixels) << 4U);
}

constexpr std::int32_t kEntityWorldOffsetX = 32;
constexpr std::int32_t kEntityWorldOffsetY = -16;

} // namespace

void NativePortShell::refresh_field_view() {
    if (rom_field_map_pixels_.empty() || rom_field_map_width_ == 0 ||
        rom_field_map_height_ == 0) {
        return;
    }
    constexpr std::int32_t kFieldViewportWidth = kScreenWidth;
    constexpr std::int32_t kFieldViewportHeight = 168;
    const auto max_viewport_x = static_cast<std::int32_t>(
        rom_field_map_width_) - kFieldViewportWidth;
    const auto max_viewport_y = static_cast<std::int32_t>(
        rom_field_map_height_) - kFieldViewportHeight;
    const auto viewport_x = std::clamp(
        field_camera_x_ - kFieldViewportWidth / 2, 0, max_viewport_x);
    const auto viewport_y = std::clamp(
        field_camera_y_ - kFieldViewportHeight / 2, 0, max_viewport_y);
    rom_field_width_ = static_cast<std::uint16_t>(kFieldViewportWidth);
    rom_field_height_ = static_cast<std::uint16_t>(kFieldViewportHeight);
    rom_field_pixels_.resize(static_cast<std::size_t>(
        kFieldViewportWidth * kFieldViewportHeight));
    for (std::int32_t y = 0; y < kFieldViewportHeight; ++y) {
        for (std::int32_t x = 0; x < kFieldViewportWidth; ++x) {
            rom_field_pixels_[static_cast<std::size_t>(y) * kFieldViewportWidth +
                              x] = rom_field_map_pixels_[
                static_cast<std::size_t>(viewport_y + y) * rom_field_map_width_ +
                viewport_x + x];
        }
    }
}

void NativePortShell::refresh_field_entity_projection() {
    if (rom_entity_frame_pixels_.empty() || rom_field_map_pixels_.empty()) {
        field_entity_visible_ = false;
        field_entity_screen_x_ = 0;
        field_entity_screen_y_ = 0;
        return;
    }

    // The field cursor is the native control anchor. The small world offsets
    // place an entity's feet at that anchor; projection itself is the
    // reviewed ROM 0x5A62 fixed-point transform.
    const shadowrun::EntityProjectionInput entity{
        fixed_world_coordinate(field_world_x_ + kEntityWorldOffsetX),
        fixed_world_coordinate(field_world_y_ + kEntityWorldOffsetY),
        0,
    };
    const auto result = shadowrun::project_entity_field(
        entity, fixed_world_coordinate(field_camera_x_),
        fixed_world_coordinate(field_camera_y_));
    field_entity_screen_x_ = static_cast<std::int32_t>(
        static_cast<std::int16_t>(result.screen_x));
    field_entity_screen_y_ = static_cast<std::int32_t>(
        static_cast<std::int16_t>(result.screen_y));
    field_entity_visible_ = result.visible;
}

void NativePortShell::update(const InputState& input) {
    input_ = input;

    if (scene_ == NativeScene::Title) {
        // ROM 0x0BF2 records Start as a pressed high-byte input and the
        // 0x1A9E loop exits when FFDDC2 is set. The native equivalent is the
        // supplied pressed edge, not a held-button poll.
        if (input.was_pressed(Button::Start)) {
            scene_ = NativeScene::Field;
            marker_x_ = kScreenWidth / 2;
            marker_y_ = kScreenHeight / 2;
            if (!rom_field_map_pixels_.empty()) {
                field_world_x_ = static_cast<std::int32_t>(0x3400U >> 4U);
                field_world_y_ = static_cast<std::int32_t>(0x2280U >> 4U);
                field_camera_x_ = field_world_x_;
                field_camera_y_ = field_world_y_;
                refresh_field_view();
                refresh_field_entity_projection();
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
    marker_x_ += delta_x;
    marker_y_ += delta_y;
    if (input.was_pressed(Button::Start)) {
        marker_x_ = kScreenWidth / 2;
        marker_y_ = kScreenHeight / 2;
        action_flash_frames_ = 0;
    }
    if (input.was_pressed(Button::A)) {
        marker_color_ = {0x00, 0xD8, 0xFF, 0xFF};
        action_flash_frames_ = 8;
        ++action_count_;
    } else if (input.was_pressed(Button::B)) {
        marker_color_ = {0xFF, 0x40, 0x70, 0xFF};
        action_flash_frames_ = 8;
        ++action_count_;
    } else if (input.was_pressed(Button::C)) {
        marker_color_ = {0xFF, 0xD0, 0x40, 0xFF};
        action_flash_frames_ = 8;
        ++action_count_;
    }
    marker_x_ = std::clamp(marker_x_, 8, static_cast<std::int32_t>(kScreenWidth) - 16);
    marker_y_ = std::clamp(marker_y_, 48, static_cast<std::int32_t>(kScreenHeight) - 16);
    if (!rom_field_map_pixels_.empty()) {
        field_world_x_ += delta_x;
        field_world_y_ += delta_y;
        field_camera_x_ = field_world_x_ -
                          (marker_x_ - static_cast<std::int32_t>(kScreenWidth / 2));
        field_camera_y_ = field_world_y_ -
                          (marker_y_ - static_cast<std::int32_t>(kScreenHeight / 2));
        refresh_field_view();
        refresh_field_entity_projection();
    }
    if (action_flash_frames_ != 0) {
        --action_flash_frames_;
    }
    ++frame_number_;
}

void NativePortShell::render(ScreenWriter& screen) const {
    screen.begin_frame();
    if (scene_ == NativeScene::Title && !rom_scene_pixels_.empty()) {
        screen.clear({0x00, 0x00, 0x00, 0xFF});
        screen.blit_rgba(0, 0, rom_scene_width_, rom_scene_height_,
                         rom_scene_pixels_);
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
        screen.write_text(8, 211, "ARROWS MOVE  Z X C  ENTER RESET",
                          1, {0xE8, 0xF4, 0xFF, 0xFF});
    } else if (scene_ == NativeScene::Field && !rom_field_pixels_.empty()) {
        // The field background is now the ROM's mode-2 map translated into a
        // host RGBA viewport. The old strip assets remain available for later
        // entity/UI reconstruction, but are no longer the scene background.
        screen.clear({0x04, 0x08, 0x14, 0xFF});
        screen.fill_rect(0, 0, kScreenWidth, 34,
                         {0x12, 0x20, 0x38, 0xFF});
        screen.fill_rect(0, 34, kScreenWidth, 2,
                         {0x00, 0xA8, 0xC8, 0xFF});
        screen.write_text(12, 8, "SHADOWRUN FIELD", 2,
                          {0xE8, 0xF4, 0xFF, 0xFF});
        screen.write_text(204, 10, "MODE 2", 1,
                          {0xA0, 0xE8, 0xF8, 0xFF});

        screen.blit_rgba(0, 36, rom_field_width_, rom_field_height_,
                         rom_field_pixels_);

        if (field_entity_visible_ && !rom_entity_frame_pixels_.empty()) {
            screen.fill_rect(field_entity_screen_x_ - 7,
                             field_entity_screen_y_ - 2, 14, 4,
                             {0x00, 0x00, 0x00, 0xB0});
            screen.blit_rgba(
                field_entity_screen_x_ - rom_entity_frame_width_ / 2,
                field_entity_screen_y_ - rom_entity_frame_height_,
                rom_entity_frame_width_, rom_entity_frame_height_,
                rom_entity_frame_pixels_);
        } else {
            // Keep the input seam visible if a ROM is supplied without the
            // reviewed entity descriptor.
            screen.fill_rect(marker_x_, marker_y_, 8, 12, marker_color_);
        }
        if (action_flash_frames_ != 0) {
            const auto action_x = field_entity_visible_
                ? field_entity_screen_x_ : marker_x_;
            const auto action_y = field_entity_visible_
                ? field_entity_screen_y_ : marker_y_;
            screen.fill_rect(action_x - 4, action_y + 4, 16, 2,
                             {0xFF, 0xF0, 0x90, 0xFF});
            screen.fill_rect(action_x + 3, action_y - 8, 2, 24,
                             {0xFF, 0xF0, 0x90, 0xFF});
        }
        screen.fill_rect(0, 204, kScreenWidth, 20,
                         {0x02, 0x05, 0x0C, 0xFF});
        screen.write_text(8, 208, "ARROWS MOVE ENTITY  Z X C ACTION", 1,
                          {0xE8, 0xF4, 0xFF, 0xFF});
        screen.write_text(8, 216, "ENTER RESET  ACTIONS", 1,
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
