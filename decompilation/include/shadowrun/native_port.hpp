#pragma once

#include "shadowrun/terrain.hpp"

#include <array>
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

// ROM 0x195C stores the sampled controller state in the low byte of FFDD22
// and the newly asserted (edge) bits in the high byte. The Genesis hardware
// order is Up, Down, Left, Right, B, C, A, Start from bit 0 through bit 7.
// Keeping this word format at the host boundary lets Windows, SDL, or another
// frontend feed the same native input decoder.
constexpr std::uint16_t kGenesisInputUp = 0x0001;
constexpr std::uint16_t kGenesisInputDown = 0x0002;
constexpr std::uint16_t kGenesisInputLeft = 0x0004;
constexpr std::uint16_t kGenesisInputRight = 0x0008;
constexpr std::uint16_t kGenesisInputB = 0x0010;
constexpr std::uint16_t kGenesisInputC = 0x0020;
constexpr std::uint16_t kGenesisInputA = 0x0040;
constexpr std::uint16_t kGenesisInputStart = 0x0080;

InputState decode_genesis_controller_word(std::uint16_t sampled_word);

enum class NativeScene {
    Bootstrap,
    Title,
    Field,
    FieldMenu,
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

struct NativeImage {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::vector<Rgba8> pixels;
};

struct NativeFieldRecordPosition {
    std::uint16_t world_x_fixed = 0;
    std::uint16_t world_y_fixed = 0;
};

// Native representation of the field record fields needed by the current
// presentation slice. World coordinates retain the ROM's 1/16-pixel unit so
// the recovered projection can consume them without a second rounding rule.
// The remaining 0x100-byte record fields stay in the transitional bus-backed
// session code until their meanings are recovered.
struct NativeFieldEntity {
    // ROM 0x2B34 initializes the first three records from the selected field
    // descriptor. The native shell currently drives record zero; the index is
    // explicit so later party/entity population can replace that choice.
    std::uint8_t record_index = 0;
    std::uint16_t world_x_fixed = 0;
    std::uint16_t world_y_fixed = 0;
    std::uint16_t projection_word_08 = 0;
    std::uint16_t kind = 0;
    std::uint8_t flags_10 = 0;
    std::uint8_t flags_11 = 0;
    std::uint32_t state_pointer = 0;
    // These retain the exact words written by ROM 0x5A62 before the VDP
    // origin is removed at the native screen boundary.
    std::uint16_t vdp_screen_x = 0;
    std::uint16_t vdp_screen_y = 0;
    // Host pixels after adapting the Genesis sprite origin (0x80, 0x80).
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    bool visible = false;
    // The field kind-3 table is a directional state table. The native
    // renderer keeps the selected table ordinal as the extracted-frame index
    // and the ROM byte offset used by record+0x16 separately.
    std::size_t frame_index = 0;
    std::uint16_t state_selector = 0;
    std::uint16_t state_index = 0;
    // Direct host forms of the signed bytes at template offsets +0x56/+0x57.
    // ROM 0x36FC suppresses display when +0x56 is negative or 0x7F.
    std::int8_t display_state = 0;
    std::int8_t display_substate = 0;
    std::uint16_t animation_accumulator = 0;
    std::uint16_t animation_multiplier = 0;
    std::uint16_t animation_threshold = 0;
    std::uint32_t animation_descriptor = 0;
};

// Host-owned data required by the current native title-to-field slice. This
// contains expanded RGBA pixels and typed collision/state data, not Genesis
// addresses or palette-indexed runtime resources. It can be serialized into
// a native asset pack and loaded without the ROM present.
struct NativePortAssets {
    NativeImage title_scene;
    NativeImage field_map;
    std::vector<NativeEntityFrame> entity_frames;
    terrain::Map field_terrain;
    std::array<NativeFieldRecordPosition, 3> initial_field_records{};
    std::uint16_t initial_camera_x_fixed = 0;
    std::uint16_t initial_camera_y_fixed = 0;
    std::array<std::array<std::int16_t, 2>, 8> field_direction_table{};
    NativeFieldEntity entity_template;
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
    NativePortShell(std::string title, NativePortAssets assets);

    void update(const InputState& input);
    void render(ScreenWriter& screen) const;

    NativeScene scene() const { return scene_; }
    bool has_assets() const { return !title_pixels_.empty(); }
    bool has_field_map() const { return !field_map_pixels_.empty(); }
    bool has_terrain() const { return !field_terrain_.statuses.empty(); }
    bool has_entity_frame() const { return !entity_frames_.empty(); }
    bool field_menu_open() const { return scene_ == NativeScene::FieldMenu; }
    std::size_t field_menu_selection() const { return field_menu_selection_; }
    std::size_t field_menu_row() const { return field_menu_row_; }
    std::size_t field_last_command_selection() const {
        return field_last_command_selection_;
    }
    std::size_t field_last_command_row() const {
        return field_last_command_row_;
    }
    std::uint32_t field_command_count() const { return field_command_count_; }
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
    std::uint8_t field_collision_status() const {
        return field_collision_.status;
    }
    bool field_collision_blocked() const { return field_collision_.blocked; }
    bool field_move_rejected() const { return move_rejected_; }
    std::size_t field_entity_frame_index() const {
        return field_entity_.frame_index;
    }
    std::size_t field_active_record_index() const {
        return active_field_record_;
    }
    std::int32_t field_world_x() const { return field_world_x_; }
    std::int32_t field_world_y() const { return field_world_y_; }
    std::int32_t field_camera_x() const { return field_camera_x_; }
    std::int32_t field_camera_y() const { return field_camera_y_; }
    std::uint64_t frame_number() const { return frame_number_; }

private:
    void refresh_field_view();
    void refresh_field_entity_projection();
    void refresh_field_collision();
    void reset_field_state();
    void activate_field_record(std::size_t record_index);
    void update_field_entity_direction(std::int32_t delta_x,
                                       std::int32_t delta_y);
    void open_field_menu();
    void update_field_menu(const InputState& input);

    std::string title_;
    InputState input_{};
    NativeScene scene_ = NativeScene::Bootstrap;
    std::int32_t marker_x_ = kScreenWidth / 2;
    std::int32_t marker_y_ = kScreenHeight / 2;
    std::uint64_t frame_number_ = 0;
    Rgba8 marker_color_{0x00, 0xD8, 0xFF, 0xFF};
    std::uint16_t title_width_ = 0;
    std::uint16_t title_height_ = 0;
    std::vector<Rgba8> title_pixels_;
    std::vector<NativeEntityFrame> entity_frames_;
    std::array<std::array<std::int16_t, 2>, 8> field_direction_table_{};
    NativeFieldEntity field_entity_{};
    std::array<NativeFieldRecordPosition, 3> field_initial_records_{};
    std::array<NativeFieldRecordPosition, 3> field_records_{};
    std::size_t active_field_record_ = 0;
    terrain::Map field_terrain_{};
    terrain::ProbeResult field_collision_{};
    bool move_rejected_ = false;
    std::uint16_t field_map_width_ = 0;
    std::uint16_t field_map_height_ = 0;
    std::vector<Rgba8> field_map_pixels_;
    std::uint16_t field_view_width_ = 0;
    std::uint16_t field_view_height_ = 0;
    std::vector<Rgba8> field_view_pixels_;
    std::int32_t field_initial_world_x_ = 0;
    std::int32_t field_initial_world_y_ = 0;
    std::uint16_t field_initial_world_x_fixed_ = 0;
    std::uint16_t field_initial_world_y_fixed_ = 0;
    std::int32_t field_initial_camera_x_ = 0;
    std::int32_t field_initial_camera_y_ = 0;
    std::int32_t field_world_x_ = 0;
    std::int32_t field_world_y_ = 0;
    std::int32_t field_camera_x_ = 0;
    std::int32_t field_camera_y_ = 0;
    // ROM 0x10D30 stores the command-menu selector in FFF0D0 and the
    // secondary row/slot in FFF0D1. The host keeps those as typed indices;
    // the renderer adapts the reviewed ROM labels to ordinary text.
    std::size_t field_menu_selection_ = 6;
    std::size_t field_menu_row_ = 0;
    std::size_t field_last_command_selection_ = 0;
    std::size_t field_last_command_row_ = 0;
    std::uint32_t field_command_count_ = 0;
    std::uint8_t field_command_message_frames_ = 0;
    std::uint8_t action_flash_frames_ = 0;
    std::uint32_t action_count_ = 0;
};

} // namespace shadowrun::native
