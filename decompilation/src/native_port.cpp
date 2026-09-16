#include "shadowrun/native_port.hpp"

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

graphics::RgbaImage scale_image(const graphics::RgbaImage& source,
                                std::uint8_t scale) {
    if (scale == 0 || source.width == 0 || source.height == 0) {
        return {};
    }
    graphics::RgbaImage scaled{
        static_cast<std::uint16_t>(source.width * scale),
        static_cast<std::uint16_t>(source.height * scale),
        std::vector<Rgba8>(static_cast<std::size_t>(source.width) * scale *
                           source.height * scale),
    };
    for (std::size_t y = 0; y < source.height; ++y) {
        for (std::size_t x = 0; x < source.width; ++x) {
            const auto& pixel = source.pixels[y * source.width + x];
            for (std::size_t dy = 0; dy < scale; ++dy) {
                for (std::size_t dx = 0; dx < scale; ++dx) {
                    scaled.pixels[(y * scale + dy) * scaled.width +
                                  x * scale + dx] = pixel;
                }
            }
        }
    }
    return scaled;
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

    // 0x59C5E/0x5AA96 uses these two reviewed compressed resources after the
    // title path. They are kept as host RGBA strips here so the native field
    // scene has ROM-derived visual material without retaining a Genesis VDP
    // or palette-indexed runtime.
    const auto palette = graphics::read_genesis_palette(rom, 0xD29D6);
    const auto art = graphics::decode_tile_atlas(
        graphics::decode_genesis_resource_stream(rom, 0x1E3F38),
        0, 4, palette, 4, 0);
    const auto actor = graphics::decode_tile_atlas(
        graphics::decode_genesis_resource_stream(rom, 0x1E62BE),
        0, 2, palette, 2, 0);
    const auto field_map = graphics::decode_genesis_mode2_field_map(rom);
    const auto scaled_art = scale_image(art, 4);
    const auto scaled_actor = scale_image(actor, 4);
    rom_art_width_ = scaled_art.width;
    rom_art_height_ = scaled_art.height;
    rom_art_pixels_ = scaled_art.pixels;
    rom_actor_width_ = scaled_actor.width;
    rom_actor_height_ = scaled_actor.height;
    rom_actor_pixels_ = scaled_actor.pixels;

    // Mode 2 starts with camera coordinates 0x3400/0x2280 in the selected
    // ROM record. They are 1/16-pixel values; the native viewport is centered
    // at that same world location and then rendered as ordinary RGBA pixels.
    constexpr std::size_t kFieldViewportWidth = kScreenWidth;
    constexpr std::size_t kFieldViewportHeight = 168;
    const auto camera_x = static_cast<std::size_t>(0x3400U >> 4U);
    const auto camera_y = static_cast<std::size_t>(0x2280U >> 4U);
    const auto viewport_x = camera_x - kFieldViewportWidth / 2U;
    const auto viewport_y = camera_y - kFieldViewportHeight / 2U;
    rom_field_width_ = static_cast<std::uint16_t>(kFieldViewportWidth);
    rom_field_height_ = static_cast<std::uint16_t>(kFieldViewportHeight);
    rom_field_pixels_.resize(kFieldViewportWidth * kFieldViewportHeight);
    for (std::size_t y = 0; y < kFieldViewportHeight; ++y) {
        for (std::size_t x = 0; x < kFieldViewportWidth; ++x) {
            rom_field_pixels_[y * kFieldViewportWidth + x] =
                field_map.pixels[(viewport_y + y) * field_map.width +
                                 viewport_x + x];
        }
    }
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
        }
        ++frame_number_;
        return;
    }

    constexpr std::int32_t kSpeed = 2;
    if (input.is_held(Button::Up)) {
        marker_y_ -= kSpeed;
    }
    if (input.is_held(Button::Down)) {
        marker_y_ += kSpeed;
    }
    if (input.is_held(Button::Left)) {
        marker_x_ -= kSpeed;
    }
    if (input.is_held(Button::Right)) {
        marker_x_ += kSpeed;
    }
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

        screen.fill_rect(marker_x_ - 6, marker_y_ + 10, 20, 4,
                         {0x00, 0x00, 0x00, 0xA0});
        screen.fill_rect(marker_x_, marker_y_, 8, 12, marker_color_);
        screen.fill_rect(marker_x_ + 2, marker_y_ - 4, 4, 4,
                         {0xFF, 0xFF, 0xFF, 0xFF});
        if (action_flash_frames_ != 0) {
            screen.fill_rect(marker_x_ - 4, marker_y_ + 4, 16, 2,
                             {0xFF, 0xF0, 0x90, 0xFF});
            screen.fill_rect(marker_x_ + 3, marker_y_ - 8, 2, 24,
                             {0xFF, 0xF0, 0x90, 0xFF});
        }
        screen.fill_rect(0, 204, kScreenWidth, 20,
                         {0x02, 0x05, 0x0C, 0xFF});
        screen.write_text(8, 208, "ARROWS MOVE  Z X C ACTION", 1,
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
