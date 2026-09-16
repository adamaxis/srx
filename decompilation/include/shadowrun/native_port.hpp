#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace shadowrun::native {

constexpr std::uint16_t kScreenWidth = 320;
constexpr std::uint16_t kScreenHeight = 224;

enum class Button : std::uint16_t {
    Up = 1U << 0U,
    Down = 1U << 1U,
    Left = 1U << 2U,
    Right = 1U << 3U,
    A = 1U << 4U,
    B = 1U << 5U,
    C = 1U << 6U,
    Start = 1U << 7U,
};

constexpr std::uint16_t button_mask(Button button) {
    return static_cast<std::uint16_t>(button);
}

struct InputState {
    std::uint16_t held = 0;
    std::uint16_t pressed = 0;

    bool is_held(Button button) const {
        return (held & button_mask(button)) != 0;
    }

    bool was_pressed(Button button) const {
        return (pressed & button_mask(button)) != 0;
    }
};

enum class NativeScene {
    Bootstrap,
    Title,
    Field,
};

struct Rgba8 {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t alpha = 255;
};

struct NativeEntityFrame {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::vector<Rgba8> pixels;
};

// Native representation of the field record fields needed by the current
// presentation slice. World coordinates retain the ROM's 1/16-pixel unit so
// the recovered projection can consume them without a second rounding rule.
// The remaining 0x100-byte record fields stay in the transitional bus-backed
// session code until their meanings are recovered.
struct NativeFieldEntity {
    std::uint16_t world_x_fixed = 0;
    std::uint16_t world_y_fixed = 0;
    std::uint16_t projection_word_08 = 0;
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    bool visible = false;
    std::size_t frame_index = 0;
    std::uint8_t animation_timer = 0;
};

// Native game code writes to this interface instead of issuing Genesis VDP
// commands. The first implementation is a software surface; SDL/DirectX or
// another Windows backend can implement the same operations later.
class ScreenWriter {
public:
    virtual ~ScreenWriter() = default;

    virtual void begin_frame() = 0;
    virtual void clear(Rgba8 color) = 0;
    virtual void fill_rect(std::int32_t x, std::int32_t y,
                           std::int32_t width, std::int32_t height,
                           Rgba8 color) = 0;
    virtual void blit_rgba(std::int32_t x, std::int32_t y,
                           std::uint16_t width, std::uint16_t height,
                           const std::vector<Rgba8>& pixels) = 0;
    virtual void write_text(std::int32_t x, std::int32_t y,
                            std::string_view text, std::uint8_t scale,
                            Rgba8 color) = 0;
    virtual void present() = 0;
};

// Renderer-neutral RGBA8 framebuffer used by headless tests and the initial
// Windows software window. Pixels are stored as 0x00RRGGBB so that the Windows
// GDI presenter can consume the same buffer as a 32-bit BGRX DIB.
class SoftwareScreen final : public ScreenWriter {
public:
    SoftwareScreen(std::uint16_t width = kScreenWidth,
                   std::uint16_t height = kScreenHeight);

    void begin_frame() override;
    void clear(Rgba8 color) override;
    void fill_rect(std::int32_t x, std::int32_t y,
                   std::int32_t width, std::int32_t height,
                   Rgba8 color) override;
    void blit_rgba(std::int32_t x, std::int32_t y,
                   std::uint16_t width, std::uint16_t height,
                   const std::vector<Rgba8>& pixels) override;
    void write_text(std::int32_t x, std::int32_t y,
                    std::string_view text, std::uint8_t scale,
                    Rgba8 color) override;
    void present() override;

    std::uint16_t width() const { return width_; }
    std::uint16_t height() const { return height_; }
    std::uint64_t presented_frames() const { return presented_frames_; }
    const std::vector<std::uint32_t>& pixels() const { return pixels_; }
    std::uint32_t pixel(std::int32_t x, std::int32_t y) const;

private:
    static std::uint32_t pack(Rgba8 color);
    void write_glyph(std::int32_t x, std::int32_t y, char character,
                     std::uint8_t scale, std::uint32_t packed_color);

    std::uint16_t width_;
    std::uint16_t height_;
    std::vector<std::uint32_t> pixels_;
    std::uint64_t presented_frames_ = 0;
};

// Small native update/render shell. It exists to make the controls and screen
// boundary executable now; reconstructed gameplay systems will replace its
// bootstrap scene as their typed state becomes available.
class NativePortShell {
public:
    explicit NativePortShell(std::string title = "SHADOWRUN");
    NativePortShell(std::string title, const std::vector<std::uint8_t>& rom);

    void update(const InputState& input);
    void render(ScreenWriter& screen) const;

    NativeScene scene() const { return scene_; }
    bool has_rom_assets() const { return !rom_scene_pixels_.empty(); }
    bool has_rom_field_map() const { return !rom_field_map_pixels_.empty(); }
    bool has_rom_entity_frame() const {
        return !rom_entity_frames_.empty();
    }
    std::int32_t marker_x() const { return marker_x_; }
    std::int32_t marker_y() const { return marker_y_; }
    std::int32_t field_entity_screen_x() const {
        return static_cast<std::int32_t>(
            static_cast<std::int16_t>(field_entity_.screen_x));
    }
    std::int32_t field_entity_screen_y() const {
        return static_cast<std::int32_t>(
            static_cast<std::int16_t>(field_entity_.screen_y));
    }
    bool field_entity_visible() const { return field_entity_.visible; }
    std::size_t field_entity_frame_index() const {
        return field_entity_.frame_index;
    }
    std::int32_t field_world_x() const { return field_world_x_; }
    std::int32_t field_world_y() const { return field_world_y_; }
    std::int32_t field_camera_x() const { return field_camera_x_; }
    std::int32_t field_camera_y() const { return field_camera_y_; }
    std::uint64_t frame_number() const { return frame_number_; }

private:
    void refresh_field_view();
    void refresh_field_entity_projection();

    std::string title_;
    InputState input_{};
    NativeScene scene_ = NativeScene::Bootstrap;
    std::int32_t marker_x_ = kScreenWidth / 2;
    std::int32_t marker_y_ = kScreenHeight / 2;
    std::uint64_t frame_number_ = 0;
    Rgba8 marker_color_{0x00, 0xD8, 0xFF, 0xFF};
    std::uint16_t rom_scene_width_ = 0;
    std::uint16_t rom_scene_height_ = 0;
    std::vector<Rgba8> rom_scene_pixels_;
    std::vector<NativeEntityFrame> rom_entity_frames_;
    NativeFieldEntity field_entity_{};
    std::uint16_t rom_field_map_width_ = 0;
    std::uint16_t rom_field_map_height_ = 0;
    std::vector<Rgba8> rom_field_map_pixels_;
    std::uint16_t rom_field_width_ = 0;
    std::uint16_t rom_field_height_ = 0;
    std::vector<Rgba8> rom_field_pixels_;
    std::int32_t field_world_x_ = 0;
    std::int32_t field_world_y_ = 0;
    std::int32_t field_camera_x_ = 0;
    std::int32_t field_camera_y_ = 0;
    std::uint8_t action_flash_frames_ = 0;
    std::uint32_t action_count_ = 0;
};

} // namespace shadowrun::native
