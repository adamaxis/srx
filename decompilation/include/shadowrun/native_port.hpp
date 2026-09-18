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
constexpr std::size_t kNativeInventorySlotCount = 8;
constexpr std::size_t kNativeInventoryItemCount = 50;
constexpr std::size_t kNativeTitleAnimationFrameCount = 6;
constexpr std::size_t kNativeMenuStatusCount = 5;
// ROM 0xC376 clamps the startup archetype selector to 0..2. The fourth
// string in NativeIntroData is the continuation line of "Gator Shaman", not
// another selectable archetype.
constexpr std::size_t kNativeArchetypeChoiceCount = 3;
constexpr std::size_t kNativeCyberwareCount = 15;
// ROM 0xC8C8 walks bits 0..14 of FFFC22 and indexes the fifteen longwords
// at 0xC25C4 when drawing Group Items.
constexpr std::size_t kNativeGroupItemCount = 15;
// ROM 0x18150 writes three fixed strings at 0xE43D1 when FF018F bit 0 is
// clear. Keep the lines separate because C0DA receives one source pointer.
constexpr std::size_t kNativeCyberdeckUnavailableLineCount = 3;
// ROM 0xC448 emits these fixed Dismiss Runner panel lines around its dynamic
// runner-name list. The placeholder line at 0xC14AE is VDP layout padding and
// is intentionally not carried as user-facing text.
constexpr std::size_t kNativeDismissRunnerLabelCount = 3;
// ROM 0xF33E writes two base Notebook entries from 0xC567E, then appends
// Contacts/Mr. Johnsons/Shadowrunners when their persistent event gates are
// set. Keep the complete extracted label catalog even though the native
// runtime currently starts with only the two ungated entries.
constexpr std::size_t kNativeNotebookLabelCount = 6;
// ROM 0xF2F6 writes the three 0x80-delimited lines at 0xC56E7 when there is
// no active shadowrun. Leading spaces are source layout and are preserved.
constexpr std::size_t kNativeNotebookNoRunLineCount = 3;
// F9A4 selects one of three 32-entry pointer tables for Tips & Clues.
constexpr std::size_t kNativeNotebookTipCount = 96;
constexpr std::size_t kNativeNotebookTipFlagGroupCount = 3;
// F416 exposes five source Mr. Johnson records behind FFFC27 bits 0..4.
constexpr std::size_t kNativeNotebookMrJohnsonCount = 5;
constexpr std::size_t kNativeAttributeCount = 8;
constexpr std::size_t kNativeSkillCount = 12;
constexpr std::size_t kNativeAttributeSkillAuxiliaryCount = 4;

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
    StartOptions,
    SaveLoad,
    ArchetypeMenu,
    Intro,
    Field,
    FieldDialog,
    FieldMenu,
    PocketSecretaryMenu,
    NotebookMenu,
    NotebookTipsMenu,
    GroupItemsMenu,
    DismissRunnerMenu,
    ClipsMenu,
    CyberwareMenu,
    AttributesMenu,
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

struct NativeFieldAnimation {
    std::uint16_t state_selector = 0;
    std::uint16_t state_marker = 0;
    std::uint16_t state_auxiliary_word = 0;
    std::uint16_t source_timing_value = 0;
    std::uint16_t timing_multiplier = 0;
    std::vector<NativeEntityFrame> frames;
};

// Host text extracted from one 0x30-byte item record in ROM 0xD4540. The
// numeric code remains the value stored in a field record's +0x66 inventory
// byte; the two text lines are expanded into ordinary strings for the native
// menu instead of retaining Genesis text placement/pointers.
struct NativeInventoryItem {
    std::uint8_t code = 0;
    // ROM item record +0x24. The weapon/equipment handlers use this source
    // factor when transferring ammunition between equipped weapons.
    std::uint8_t source_combat_factor = 0;
    // ROM 0x12F60/0x13084 use these two record bytes as indexes into the
    // source detail tables. They are not derived statistics.
    std::uint8_t damage_table_index = 0; // item record +0x25
    std::uint8_t power_table_index = 0;  // item record +0x26 (one-based)
    std::string name;
    std::string category;
};

struct NativeImage {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::vector<Rgba8> pixels;
};

// One line emitted by the ROM's C0DA text writer. The row is relative to the
// destination name-table address passed by the caller; preserving it keeps
// the native startup screens in the source's tile geometry instead of
// reflowing the strings as a host UI.
struct NativeStartupTextLine {
    std::uint8_t row = 0;
    std::string text;
};

// Host form of the title/startup text resources. The labels and story lines
// are extracted from the ROM. The opening story is emitted by the source
// 0x59C5E sequence, so the native frontend keeps the text as source data and
// drives its timed write groups from the reviewed call path.
struct NativeIntroData {
    std::string start_options_title;
    std::array<std::string, 2> start_options{};
    std::string archetype_title;
    // The fourth entry is retained because it is a separately written source
    // line in the extracted layout; only entries 0..2 are choices.
    std::array<std::string, 4> archetypes{};
    std::vector<std::string> story_lines;
    std::vector<NativeStartupTextLine> start_options_layout;
    std::vector<NativeStartupTextLine> save_load_layout;
    std::vector<NativeStartupTextLine> archetype_layout;
    std::vector<NativeStartupTextLine> no_save_layout;
    std::vector<NativeStartupTextLine> save_success_layout;
};

struct NativeFieldRecordPosition {
    std::uint16_t world_x_fixed = 0;
    std::uint16_t world_y_fixed = 0;
};

// The first three field records receive their positions from the selected
// mode record at ROM 0x2B34. Their other selection fields already exist in
// the normal 0x100-byte entity records; keep those fields at the native asset
// boundary as well so 0x391A can select records by state instead of cycling
// through coordinates blindly.
struct NativeFieldRecord {
    NativeFieldRecordPosition position{};
    // ROM 0x391A uses +0x3E to detect the wrap boundary. Type 2 is the
    // third selectable startup record, not an ineligible/sentinel record.
    std::uint16_t record_type = 0;
    std::uint8_t flags_10 = 0;
    std::uint8_t flags_11 = 0;
};

// Host-owned field area data. The mode id is the value written to the ROM's
// FFDDC4 mode word; everything needed to render and walk the area is already
// converted to ordinary host data. The optional reaction-mode table is the
// host form of the descriptor pointer consumed by ROM 0x167AE. Its first 32
// bytes are sufficient for the reaction substates reachable from terrain.
struct NativeFieldArea {
    std::uint16_t mode_id = 0;
    NativeImage map;
    terrain::Map terrain;
    std::uint16_t initial_camera_x_fixed = 0;
    std::uint16_t initial_camera_y_fixed = 0;
    std::array<NativeFieldRecord, 3> initial_field_records{};
    std::array<std::uint8_t, 32> event_mode_table{};
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
    std::uint16_t record_type = 0;
    // The command overview reads these directly from each 0x100-byte runner
    // record. They are source indexes/values, not host-generated labels.
    std::uint8_t race_index = 0;
    std::uint8_t archetype_index = 0;
    std::uint8_t physical_value = 0;
    std::uint8_t mental_value = 0;
    // ROM +0x57: the currently equipped weapon/gear item. Inventory slot
    // contents at +0x66 are only possession; they do not imply this value.
    std::uint8_t active_item_code = 0;
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
    // The active image within the selected native walking sequence. The raw
    // ROM selector and its current sequence index remain separate below.
    std::size_t frame_index = 0;
    std::uint16_t state_selector = 0;
    std::uint16_t state_index = 0;
    // Direct host forms of the signed bytes at template offsets +0x56/+0x57.
    // +0x56 is the equipped inventory slot (0x7F means no weapon); ROM
    // 0x36FC suppresses display when it is negative or 0x7F. +0x57 mirrors
    // active_item_code for the original record layout.
    std::int8_t display_state = 0;
    std::int8_t display_substate = 0;
    // ROM 0x3602 reads +0x9B to select the item-icon palette line.
    std::uint8_t display_palette_state = 0;
    // ROM 0x11928 indexes the five command-screen status strips with +0x4D.
    std::uint8_t menu_status_index = 0;
    // Source output of 0x12CEE for this runner's command overview. The
    // indexes select the 13-tile strips extracted from CF3D4/CF610.
    std::uint8_t attack_bar_index = 0;
    std::uint8_t defense_bar_index = 0;
    // 0x11ABE calls 0x12CEE again after changing +0x4D. These are cached
    // source results for the extracted initial record; runtime equipment
    // changes recompute them through compute_command_bar_indices().
    std::array<std::uint8_t, kNativeMenuStatusCount>
        attack_bar_indices_by_posture{};
    std::array<std::uint8_t, kNativeMenuStatusCount>
        defense_bar_indices_by_posture{};
    // 0x11352 receives the raw armor +0x26/+0x25 values from 0x55EB4/
    // 0x55E92 and uses them as indexes into the seven-tile CF884 table.
    std::uint8_t weapons_bar_index = 0;
    std::uint8_t guns_bar_index = 0;
    std::uint16_t animation_accumulator = 0;
    std::uint16_t animation_multiplier = 0;
    std::uint16_t animation_threshold = 0;
    std::uint32_t animation_descriptor = 0;
    // ROM 0x12148 indexes these eight item codes as 2*FFF0D1+FFF0D0.
    std::array<std::uint8_t, kNativeInventorySlotCount>
        inventory_item_codes{};
    // The adjacent +0x6E bytes are phase/status bytes used by the ROM's
    // animation and item handlers. Preserve them at extraction even though
    // the native inventory view does not reinterpret them as quantities.
    std::array<std::uint8_t, kNativeInventorySlotCount>
        inventory_phase_bytes{};
    // ROM 0x12168 uses record +0x76 as the one-based equipped armor index
    // when an item code is at or above 0x2C.
    std::uint8_t equipped_armor_index = 0;
    // ROM 0x11B76 displays +0x4F as the available clip count. The adjacent
    // +0x4E byte is retained as the current weapon ammunition value.
    std::uint8_t ammo_rounds = 0;
    std::uint8_t ammo_clips = 0;
    // ROM 0x11950 reads the active record's +0x90 word as fifteen installed
    // cyberware flags. The native port preserves that bitfield directly.
    std::uint16_t cyberware_flags = 0;
    // ROM 0xD01C displays these named fields directly. The skipped bytes are
    // kept separately because their upgrade/display roles are not resolved.
    std::array<std::uint8_t, kNativeAttributeCount> attribute_values{};
    std::array<std::uint8_t, kNativeSkillCount> skill_values{};
    // Exact record offsets: +0x7F, +0x86, +0x8D, +0x8E.
    std::array<std::uint8_t, kNativeAttributeSkillAuxiliaryCount>
        attribute_skill_auxiliary{};
};

struct NativeCommandBarIndices {
    std::uint8_t attack = 0;
    std::uint8_t defense = 0;
};

// Native C++ transcription of the command overview's 0x12CEE branch. It
// preserves the source helper boundaries used by 0x55ED6/0x55F20/0x12C74
// and consumes host-extracted item records rather than Genesis addresses.
NativeCommandBarIndices compute_command_bar_indices(
    const NativeFieldEntity& entity,
    const std::array<NativeInventoryItem, kNativeInventoryItemCount>& items,
    std::uint8_t posture, std::uint8_t command_selector = 6,
    std::uint8_t command_row = 0);

// Host-owned data required by the current native title-to-field slice. This
// contains expanded RGBA pixels and typed collision/state data, not Genesis
// addresses or palette-indexed runtime resources. It can be serialized into
// a native asset pack and loaded without the ROM present.
struct NativePortAssets {
    NativeImage title_scene;
    // Separate host layers preserve the ROM's title timing: the circuit
    // plane is present first, the logo columns arrive one per title-loop
    // pass after the initial delay, the sprite overlay advances every four
    // calls, and text is a fixed plane.
    NativeImage title_background;
    NativeImage title_logo_plane;
    NativeImage title_text;
    std::array<NativeImage, kNativeTitleAnimationFrameCount>
        title_sprite_frames{};
    // ROM 0x1C6A selects one of six title sprite lists. These are complete
    // host frames with the source logo plane and title text already composed;
    // the native shell advances them on the same four-vblank cadence.
    std::array<NativeImage, kNativeTitleAnimationFrameCount>
        title_animation_frames{};
    // Plane-B base written by the ROM command-screen initializer at 0x10F50.
    // It contains the real menu panel and inventory-box geometry; dynamic
    // labels and cursor state are rendered by the native command loop.
    NativeImage field_menu_background;
    NativeImage field_menu_inventory_boxes;
    NativeImage field_menu_font;
    NativeImage startup_font;
    NativeImage startup_cursor;
    std::array<NativeImage, 3> field_menu_runner_portraits{};
    // ROM 0x11678 has two 5x4 name-table overlays for the selected inventory
    // cell. They remain separate because the source chooses them by the
    // selected display-state pair, not by a host highlight color.
    std::array<NativeImage, 2> field_menu_selected_cells{};
    // ROM 0x11592 emits the 3x4-tile selector sprite from D29F6. It is kept
    // as a transparent host image and positioned from the same 0x28/0x18
    // selector strides as the source routine.
    NativeImage field_menu_cursor;
    // ROM 0x12A8C writes the selected item's 9x5 artwork from the item
    // record's +0x1E table to Plane A at C1A4.
    std::array<NativeImage, kNativeInventoryItemCount>
        field_menu_item_art{};
    std::array<std::array<NativeImage, 2>, kNativeInventoryItemCount>
        field_menu_inventory_icons{};
    std::array<std::string, 3> field_menu_runner_names{};
    std::array<NativeImage, kNativeMenuStatusCount>
        field_menu_status_bars{};
    std::array<std::string, kNativeMenuStatusCount>
        field_menu_status_names{};
    // ROM 0x12F70 and 0x12FA4 write seven-tile detail bars from these
    // tables. The arrays preserve the source table index; the runtime does
    // not synthesize a percentage bar from a guessed item statistic.
    std::array<NativeImage, 14> field_menu_damage_bars{};
    std::array<NativeImage, 14> field_menu_drain_bars{};
    std::array<NativeImage, 256> field_menu_detail_values{};
    // ROM 0x12CEE selects the 13-tile command-screen attack/defense strips
    // from CF3D4 and CF610. These are separate from the seven-tile weapon
    // damage/drain bars above.
    std::array<NativeImage, 23> field_menu_attack_bars{};
    std::array<NativeImage, 23> field_menu_defense_bars{};
    std::array<std::string, 13> field_menu_power_labels{};
    // The seven armor records at ROM D4D50 are the exact two-row labels
    // written by the command overview's 0x11352 path.
    std::array<std::array<std::string, 2>, 7> field_menu_armor_labels{};
    NativeIntroData intro;
    NativeImage field_map;
    std::vector<NativeEntityFrame> entity_frames;
    std::array<NativeFieldAnimation, 8> field_entity_animations{};
    terrain::Map field_terrain;
    std::array<NativeFieldRecord, 3> initial_field_records{};
    std::uint16_t initial_camera_x_fixed = 0;
    std::uint16_t initial_camera_y_fixed = 0;
    std::array<std::array<std::int16_t, 2>, 8> field_direction_table{};
    // Mode 2 remains available in the legacy fields above. The area catalog
    // adds the extracted destination maps used by native field transitions.
    std::vector<NativeFieldArea> field_areas;
    std::array<std::uint8_t, 32> field_event_mode_table{};
    NativeFieldEntity entity_template;
    // These are the three source records copied by 0x42B0 when Joshua's
    // startup archetype is selected. They are templates, not switchable
    // party members.
    std::array<NativeFieldEntity, kNativeArchetypeChoiceCount>
        protagonist_archetype_records{};
    // Runtime party slots. New Game populates slot zero with Joshua and
    // leaves the other slots inactive until recruited records are recovered.
    std::array<NativeFieldEntity, 3> field_entity_records{};
    std::array<NativeInventoryItem, kNativeInventoryItemCount>
        inventory_items{};
    std::array<std::string, kNativeCyberwareCount> cyberware_names{};
    std::array<std::string, kNativeAttributeCount + kNativeSkillCount>
        attribute_skill_names{};
    std::array<std::string, kNativeGroupItemCount> group_item_names{};
    std::array<std::string, kNativeCyberdeckUnavailableLineCount>
        cyberdeck_unavailable_lines{};
    std::array<std::string, kNativeDismissRunnerLabelCount>
        dismiss_runner_labels{};
    std::array<std::string, kNativeNotebookLabelCount> notebook_labels{};
    std::array<std::string, kNativeNotebookNoRunLineCount>
        notebook_no_run_lines{};
    // New Game's 0x59C48 initializer sets FFFC2C to 0x1C7F, which makes
    // Contacts visible immediately after the opening. The other two rows
    // remain gated until their source event setters are ported.
    std::array<std::uint8_t, 3> notebook_event_gates{{1, 0, 0}};
    std::array<std::string, kNativeNotebookTipCount> notebook_tip_text{};
    // F9A4 reads these three source longwords at FFFC30/34/38 and tests
    // their bits before appending a pointer to the visible page list. The
    // new-game initializer leaves these event-owned fields clear; setters
    // will be ported as their story/event writers are audited.
    std::array<std::uint32_t, kNativeNotebookTipFlagGroupCount>
        notebook_tip_flags{};
    std::array<std::string, kNativeNotebookMrJohnsonCount>
        notebook_mr_johnson_names{};
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
    bool has_intro() const { return !intro_.story_lines.empty(); }
    bool has_startup_menus() const {
        return !intro_.start_options_layout.empty() &&
               !intro_.save_load_layout.empty() &&
               !intro_.archetype_layout.empty();
    }
    bool has_field_map() const { return !field_map_pixels_.empty(); }
    bool has_terrain() const { return !field_terrain_.statuses.empty(); }
    bool has_entity_frame() const { return !entity_frames_.empty(); }
    bool field_menu_open() const { return scene_ == NativeScene::FieldMenu; }
    bool pocket_secretary_menu_open() const {
        return scene_ == NativeScene::PocketSecretaryMenu;
    }
    bool notebook_menu_open() const {
        return scene_ == NativeScene::NotebookMenu;
    }
    bool notebook_tips_menu_open() const {
        return scene_ == NativeScene::NotebookTipsMenu;
    }
    std::size_t notebook_selection() const { return notebook_selection_; }
    std::size_t notebook_option_count() const {
        return notebook_option_count_;
    }
    bool notebook_message_visible() const {
        return notebook_message_visible_;
    }
    std::string_view notebook_label(std::size_t index) const {
        return index < notebook_labels_.size()
            ? std::string_view(notebook_labels_[index])
            : std::string_view{};
    }
    std::string_view notebook_no_run_line(std::size_t index) const {
        return index < notebook_no_run_lines_.size()
            ? std::string_view(notebook_no_run_lines_[index])
            : std::string_view{};
    }
    std::string_view notebook_tip_text(std::size_t index) const {
        return index < notebook_tip_text_.size()
            ? std::string_view(notebook_tip_text_[index])
            : std::string_view{};
    }
    std::size_t notebook_tip_selection() const {
        return notebook_tip_selection_;
    }
    std::size_t notebook_visible_tip_count() const {
        return notebook_visible_tip_count_;
    }
    std::size_t notebook_visible_tip_index(std::size_t index) const {
        return index < notebook_visible_tip_count_
            ? notebook_visible_tip_indices_[index]
            : kNativeNotebookTipCount;
    }
    std::size_t pocket_secretary_selection() const {
        return pocket_secretary_selection_;
    }
    std::size_t pocket_secretary_option_count() const {
        return pocket_secretary_save_load_enabled_ ? 5U : 4U;
    }
    bool pocket_secretary_save_load_enabled() const {
        return pocket_secretary_save_load_enabled_;
    }
    bool group_items_menu_open() const {
        return scene_ == NativeScene::GroupItemsMenu;
    }
    bool dismiss_runner_menu_open() const {
        return scene_ == NativeScene::DismissRunnerMenu;
    }
    std::size_t dismiss_runner_selection() const {
        return dismiss_runner_selection_;
    }
    std::size_t dismiss_runner_option_count() const {
        return dismiss_runner_candidate_count_ + 1U;
    }
    std::uint32_t group_item_flags() const { return group_item_flags_; }
    std::size_t group_item_count() const { return kNativeGroupItemCount; }
    std::string_view group_item_name(std::size_t index) const {
        return index < group_item_names_.size()
            ? std::string_view(group_item_names_[index])
            : std::string_view{};
    }
    bool pocket_secretary_message_visible() const {
        return pocket_secretary_message_visible_;
    }
    std::string_view pocket_secretary_message_line(std::size_t index) const {
        return index < cyberdeck_unavailable_lines_.size()
            ? std::string_view(cyberdeck_unavailable_lines_[index])
            : std::string_view{};
    }
    bool field_dialog_open() const {
        return scene_ == NativeScene::FieldDialog;
    }
    std::size_t field_dialog_line_index() const {
        return field_dialog_line_index_;
    }
    std::string_view field_dialog_line() const {
        return field_dialog_line_index_ < field_dialog_lines_.size()
            ? std::string_view(field_dialog_lines_[field_dialog_line_index_])
            : std::string_view{};
    }
    bool clips_menu_open() const { return scene_ == NativeScene::ClipsMenu; }
    bool cyberware_menu_open() const {
        return scene_ == NativeScene::CyberwareMenu;
    }
    bool attributes_menu_open() const {
        return scene_ == NativeScene::AttributesMenu;
    }
    std::size_t field_menu_selection() const { return field_menu_selection_; }
    std::size_t field_menu_row() const { return field_menu_row_; }
    bool field_menu_posture_active() const { return field_menu_posture_active_; }
    std::uint8_t field_posture_index() const {
        return field_entity_.menu_status_index;
    }
    std::uint8_t field_attack_bar_index() const {
        return field_entity_.attack_bar_index;
    }
    std::uint8_t field_defense_bar_index() const {
        return field_entity_.defense_bar_index;
    }
    std::uint8_t field_active_item_code() const {
        return field_entity_.active_item_code;
    }
    // ROM 0x12A8C selects this source label independently of the weapon
    // Damage/Power writer. Medical and utility items therefore expose Uses or
    // Rtng instead of being presented as weapons.
    std::string_view field_active_item_detail_label() const;
    std::string_view field_selected_item_detail_label() const;
    bool field_menu_action_active() const { return field_menu_action_active_; }
    std::size_t field_menu_action_selection() const {
        return field_menu_action_selection_;
    }
    std::string_view field_menu_action_label() const;
    // ROM 0x121C0 displays D202F for the item classes that are consumed by
    // field events rather than equipped from the inventory action loop.
    bool field_item_message_active() const {
        return field_item_message_active_;
    }
    std::string_view field_item_message() const {
        return field_item_message_;
    }
    bool field_discard_confirmation_active() const {
        return field_discard_confirmation_active_;
    }
    bool field_discard_confirmation_yes() const {
        return field_discard_confirmation_yes_;
    }
    std::string_view field_discard_message() const;
    bool field_trade_active() const { return field_trade_active_; }
    bool field_trade_confirmation_yes() const {
        return field_trade_confirmation_yes_;
    }
    bool field_trade_cannot_trade() const {
        return field_trade_cannot_trade_;
    }
    std::size_t field_trade_recipient_record() const {
        return field_trade_recipient_record_;
    }
    std::size_t field_trade_recipient_slot() const {
        return field_trade_recipient_slot_;
    }
    std::string_view field_trade_message() const;
    bool field_first_aid_active() const { return field_first_aid_active_; }
    bool field_first_aid_message_active() const {
        return field_first_aid_message_active_;
    }
    std::size_t field_first_aid_target_count() const {
        return field_first_aid_target_count_;
    }
    std::size_t field_first_aid_target_index() const {
        return field_first_aid_target_index_;
    }
    std::size_t field_first_aid_target_record() const {
        return field_first_aid_target_index_ < field_first_aid_target_count_
            ? field_first_aid_target_records_[field_first_aid_target_index_]
            : active_field_record_;
    }
    bool field_first_aid_cancel_selected() const {
        return field_first_aid_target_index_ >= field_first_aid_target_count_;
    }
    std::string_view field_first_aid_message() const {
        return field_first_aid_message_;
    }
    std::int8_t field_equipped_weapon_slot() const {
        return field_entity_.display_state;
    }
    std::size_t clips_menu_selection() const { return clips_menu_selection_; }
    std::uint32_t clips_action_count() const { return clips_action_count_; }
    std::string_view clips_action_message() const {
        return clips_action_message_;
    }
    bool clips_reload_confirmation_active() const {
        return clips_reload_confirmation_active_;
    }
    bool clips_reload_confirmation_yes() const {
        return clips_reload_confirmation_yes_;
    }
    bool clips_trade_menu_open() const { return clips_trade_active_; }
    std::size_t clips_trade_menu_selection() const {
        return clips_trade_menu_selection_;
    }
    std::size_t clips_trade_recipient_record() const {
        return clips_trade_recipient_record_;
    }
    std::uint8_t clips_trade_quantity() const {
        return clips_trade_quantity_;
    }
    std::uint8_t field_ammo_rounds() const {
        return field_entity_.ammo_rounds;
    }
    std::uint8_t field_ammo_clips() const { return field_entity_.ammo_clips; }
    std::uint16_t field_cyberware_flags() const {
        return field_entity_.cyberware_flags;
    }
    bool field_cyberware_installed(std::size_t index) const {
        return index < kNativeCyberwareCount &&
               (field_entity_.cyberware_flags &
                static_cast<std::uint16_t>(1U << index)) != 0;
    }
    std::string_view field_cyberware_name(std::size_t index) const {
        return index < cyberware_names_.size()
            ? std::string_view(cyberware_names_[index])
            : std::string_view{};
    }
    std::size_t attributes_menu_column() const {
        return attributes_menu_column_;
    }
    std::size_t attributes_menu_row() const { return attributes_menu_row_; }
    std::uint8_t field_attribute_value(std::size_t index) const {
        return index < field_entity_.attribute_values.size()
            ? field_entity_.attribute_values[index]
            : 0;
    }
    std::uint8_t field_skill_value(std::size_t index) const {
        return index < field_entity_.skill_values.size()
            ? field_entity_.skill_values[index]
            : 0;
    }
    std::uint8_t field_attribute_skill_auxiliary(std::size_t index) const {
        return index < field_entity_.attribute_skill_auxiliary.size()
            ? field_entity_.attribute_skill_auxiliary[index]
            : 0;
    }
    std::string_view field_attribute_skill_name(std::size_t index) const {
        return index < attribute_skill_names_.size()
            ? std::string_view(attribute_skill_names_[index])
            : std::string_view{};
    }
    std::size_t field_last_command_selection() const {
        return field_last_command_selection_;
    }
    std::size_t field_last_command_row() const {
        return field_last_command_row_;
    }
    std::uint32_t field_command_count() const { return field_command_count_; }
    std::uint8_t field_inventory_item_code(std::size_t slot) const {
        return slot < field_entity_.inventory_item_codes.size()
            ? field_entity_.inventory_item_codes[slot]
            : 0;
    }
    std::uint8_t field_inventory_phase(std::size_t slot) const {
        return slot < field_entity_.inventory_phase_bytes.size()
            ? field_entity_.inventory_phase_bytes[slot]
            : 0;
    }
    std::uint8_t field_record_inventory_item_code(std::size_t record,
                                                  std::size_t slot) const {
        return record < field_entity_records_.size() &&
                       slot < field_entity_records_[record]
                                  .inventory_item_codes.size()
            ? field_entity_records_[record].inventory_item_codes[slot]
            : 0;
    }
    std::uint8_t field_record_physical_value(std::size_t record) const {
        return record < field_entity_records_.size()
            ? field_entity_records_[record].physical_value : 0;
    }
    std::uint8_t field_record_mental_value(std::size_t record) const {
        return record < field_entity_records_.size()
            ? field_entity_records_[record].mental_value : 0;
    }
    std::uint8_t field_record_ammo_clips(std::size_t record) const {
        return record < field_entity_records_.size()
            ? field_entity_records_[record].ammo_clips : 0;
    }
    std::uint8_t field_equipped_armor_index() const {
        return field_entity_.equipped_armor_index;
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
    std::uint8_t field_collision_status() const {
        return field_collision_.status;
    }
    // The movement result above intentionally collapses traversable positive
    // statuses to zero, matching ROM 0xE7C8. Keep the selected raw grid byte
    // available for the later field interaction/event dispatcher.
    std::uint8_t field_raw_status() const {
        return field_collision_.raw_status;
    }
    bool field_collision_blocked() const { return field_collision_.blocked; }
    bool field_move_rejected() const { return move_rejected_; }
    std::size_t field_entity_frame_index() const {
        return field_entity_.frame_index;
    }
    std::uint16_t field_entity_state_selector() const {
        return field_entity_.state_selector;
    }
    std::uint16_t field_entity_state_index() const {
        return field_entity_.state_index;
    }
    std::size_t field_active_record_index() const {
        return active_field_record_;
    }
    std::size_t start_options_selection() const {
        return start_options_selection_;
    }
    std::size_t save_load_selection() const { return save_load_selection_; }
    bool save_load_message_visible() const {
        return save_load_message_visible_;
    }
    std::size_t archetype_selection() const { return archetype_selection_; }
    std::size_t selected_archetype() const { return selected_archetype_; }
    std::size_t intro_line_start() const { return intro_line_start_; }
    std::uint64_t intro_frame() const { return intro_frame_; }
    std::size_t intro_story_line_count() const {
        return intro_.story_lines.size();
    }
    std::uint32_t field_runner_switch_attempts() const {
        return runner_switch_attempts_;
    }
    std::uint32_t field_runner_switch_rejections() const {
        return runner_switch_rejections_;
    }
    std::int32_t field_world_x() const { return field_world_x_; }
    std::int32_t field_world_y() const { return field_world_y_; }
    std::int32_t field_camera_x() const { return field_camera_x_; }
    std::int32_t field_camera_y() const { return field_camera_y_; }
    std::uint16_t field_mode() const { return field_mode_; }
    std::uint32_t field_area_transition_count() const {
        return field_area_transition_count_;
    }
    std::uint64_t frame_number() const { return frame_number_; }

private:
    void refresh_field_view();
    void refresh_field_entity_projection();
    void refresh_field_collision();
    void reset_field_state();
    bool load_field_area(std::uint16_t mode_id);
    void update_start_options(const InputState& input);
    void update_save_load(const InputState& input);
    void update_archetype_menu(const InputState& input);
    void update_intro(const InputState& input);
    void update_field_dialog(const InputState& input);
    void open_stoker_motel_dialog();
    void apply_protagonist_archetype();
    void activate_field_record(std::size_t record_index);
    std::size_t next_available_field_record() const;
    void select_field_entity_idle(std::size_t direction);
    void select_field_entity_animation(std::size_t direction);
    void update_field_entity_direction(std::int32_t delta_x,
                                       std::int32_t delta_y);
    void update_field_entity_animation(bool movement_attempted);
    void open_field_menu();
    void update_field_menu(const InputState& input);
    void open_pocket_secretary_menu();
    void update_pocket_secretary_menu(const InputState& input);
    void open_notebook_menu();
    void update_notebook_menu(const InputState& input);
    void open_notebook_tips_menu();
    void update_notebook_tips_menu(const InputState& input);
    void update_group_items_menu(const InputState& input);
    void open_dismiss_runner_menu();
    void update_dismiss_runner_menu(const InputState& input);
    void write_field_menu_text(ScreenWriter& screen, std::int32_t x,
                               std::int32_t y, std::string_view text) const;
    void write_startup_text(ScreenWriter& screen, std::int32_t x,
                            std::int32_t y, std::string_view text) const;
    void render_startup_background(ScreenWriter& screen) const;
    bool menu_direction_step(const InputState& input, Button negative,
                             Button positive, bool horizontal);
    void open_field_inventory_action();
    void open_field_first_aid();
    void update_field_first_aid(const InputState& input);
    void apply_field_first_aid();
    void open_field_discard_confirmation();
    void open_field_trade();
    void update_field_trade(const InputState& input);
    void trade_selected_inventory_item();
    void update_field_inventory_action(const InputState& input);
    void apply_field_inventory_action();
    void discard_selected_inventory_item();
    void open_clips_menu();
    void update_clips_menu(const InputState& input);
    void open_clips_trade();
    void update_clips_trade(const InputState& input);
    void distribute_clips();
    void reload_one_clip();
    void open_cyberware_menu();
    void update_cyberware_menu(const InputState& input);
    void open_attributes_menu();
    void update_attributes_menu(const InputState& input);

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
    NativeImage title_background_{};
    NativeImage title_logo_plane_{};
    NativeImage title_text_{};
    std::array<NativeImage, kNativeTitleAnimationFrameCount>
        title_sprite_frames_{};
    std::array<NativeImage, kNativeTitleAnimationFrameCount>
        title_animation_frames_{};
    std::uint16_t field_menu_background_width_ = 0;
    std::uint16_t field_menu_background_height_ = 0;
    std::vector<Rgba8> field_menu_background_pixels_;
    std::uint16_t field_menu_inventory_boxes_width_ = 0;
    std::uint16_t field_menu_inventory_boxes_height_ = 0;
    std::vector<Rgba8> field_menu_inventory_boxes_pixels_;
    std::uint16_t field_menu_font_width_ = 0;
    std::uint16_t field_menu_font_height_ = 0;
    std::vector<Rgba8> field_menu_font_pixels_;
    NativeImage startup_font_{};
    NativeImage startup_cursor_{};
    std::array<NativeImage, 3> field_menu_runner_portraits_{};
    std::array<NativeImage, 2> field_menu_selected_cells_{};
    NativeImage field_menu_cursor_{};
    std::array<NativeImage, kNativeInventoryItemCount> field_menu_item_art_{};
    std::array<std::array<NativeImage, 2>, kNativeInventoryItemCount>
        field_menu_inventory_icons_{};
    std::array<std::string, 3> field_menu_runner_names_{};
    std::array<NativeImage, kNativeMenuStatusCount>
        field_menu_status_bars_{};
    std::array<std::string, kNativeMenuStatusCount>
        field_menu_status_names_{};
    std::array<NativeImage, 14> field_menu_damage_bars_{};
    std::array<NativeImage, 14> field_menu_drain_bars_{};
    std::array<NativeImage, 256> field_menu_detail_values_{};
    std::array<NativeImage, 23> field_menu_attack_bars_{};
    std::array<NativeImage, 23> field_menu_defense_bars_{};
    std::array<std::string, 13> field_menu_power_labels_{};
    // ROM 0x11352 writes the selected armor record from D4D50 at C820;
    // preserve its two source text rows instead of substituting generic
    // Armor/Clothing labels.
    std::array<std::array<std::string, 2>, 7> field_menu_armor_labels_{};
    NativeIntroData intro_{};
    std::size_t start_options_selection_ = 0;
    std::size_t save_load_selection_ = 0;
    std::size_t archetype_selection_ = 0;
    std::size_t selected_archetype_ = 0;
    std::size_t intro_line_start_ = 0;
    std::uint64_t intro_frame_ = 0;
    bool start_options_load_rejected_ = false;
    bool save_load_message_visible_ = false;
    bool save_load_message_success_ = false;
    // The same 0x59172 save/load handler is reachable from startup and from
    // Pocket Secretary; its return target is different in those two callers.
    bool save_load_from_pocket_secretary_ = false;
    // An early Start finishes the source title presentation; the following
    // edge enters the startup-options routine.
    bool title_skip_requested_ = false;
    std::vector<NativeEntityFrame> entity_frames_;
    std::array<NativeFieldAnimation, 8> field_entity_animations_{};
    // Source text routed by the motel interaction callback at ROM 0x20E48.
    // These are paragraphs rather than invented UI labels; the renderer wraps
    // the original strings into the native dialogue window.
    std::vector<std::string> field_dialog_lines_;
    std::size_t field_dialog_line_index_ = 0;
    std::array<std::array<std::int16_t, 2>, 8> field_direction_table_{};
    NativeFieldEntity field_entity_{};
    std::array<NativeFieldEntity, kNativeArchetypeChoiceCount>
        protagonist_archetype_records_{};
    std::array<NativeFieldEntity, 3> field_entity_records_{};
    std::array<NativeFieldEntity, 3> field_initial_entity_records_{};
    // ROM starts a controlled record on its one-frame directional state. The
    // multi-frame state list is selected only while the movement state is
    // active; keeping that distinction prevents an idle runner from showing
    // a walking pose before the first accepted step.
    std::size_t field_entity_direction_ = 0;
    bool field_entity_walking_ = false;
    std::array<NativeInventoryItem, kNativeInventoryItemCount>
        inventory_items_{};
    std::array<std::string, kNativeCyberwareCount> cyberware_names_{};
    std::array<std::string, kNativeAttributeCount + kNativeSkillCount>
        attribute_skill_names_{};
    std::array<std::string, kNativeGroupItemCount> group_item_names_{};
    std::array<std::string, kNativeCyberdeckUnavailableLineCount>
        cyberdeck_unavailable_lines_{};
    std::array<std::string, kNativeDismissRunnerLabelCount>
        dismiss_runner_labels_{};
    std::array<std::string, kNativeNotebookLabelCount> notebook_labels_{};
    std::array<std::string, kNativeNotebookNoRunLineCount>
        notebook_no_run_lines_{};
    std::array<std::uint8_t, 3> notebook_event_gates_{{1, 0, 0}};
    std::array<std::string, kNativeNotebookTipCount> notebook_tip_text_{};
    std::array<std::uint32_t, kNativeNotebookTipFlagGroupCount>
        notebook_tip_flags_{};
    std::array<std::string, kNativeNotebookMrJohnsonCount>
        notebook_mr_johnson_names_{};
    std::array<std::size_t, kNativeNotebookTipCount>
        notebook_visible_tip_indices_{};
    std::size_t notebook_visible_tip_count_ = 0;
    std::size_t notebook_tip_selection_ = 0;
    bool pocket_secretary_message_visible_ = false;
    std::array<std::size_t, 2> dismiss_runner_candidate_records_{};
    std::size_t dismiss_runner_candidate_count_ = 0;
    std::size_t dismiss_runner_selection_ = 0;
    std::array<std::uint8_t, 3> field_equipped_armor_by_record_{};
    std::uint8_t field_initial_equipped_armor_ = 0;
    std::array<NativeFieldRecord, 3> field_initial_records_{};
    std::array<NativeFieldRecord, 3> field_records_{};
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
    std::uint16_t field_mode_ = 2;
    std::vector<NativeFieldArea> field_areas_;
    std::array<std::uint8_t, 32> field_event_mode_table_{};
    std::uint32_t field_area_transition_count_ = 0;
    // ROM 0x10D30 stores the command-menu selector in FFF0D0 and the
    // secondary row/slot in FFF0D1. The host keeps those as typed indices;
    // the renderer adapts the reviewed ROM labels to ordinary text.
    std::size_t field_menu_selection_ = 6;
    std::size_t field_menu_row_ = 0;
    bool field_menu_posture_active_ = false;
    // ROM 0xC2A6 draws five Pocket Secretary entries in ordinary field mode;
    // the source removes Save/Load in its restricted mode branches.
    std::size_t pocket_secretary_selection_ = 0;
    bool pocket_secretary_save_load_enabled_ = true;
    // ROM 0xF33E starts this at one (two selectable rows, 0..1) and
    // increments it for each nonzero event gate. New Game's 0x59C48 setup
    // enables Contacts, so the default fresh-session bound is three rows.
    std::size_t notebook_selection_ = 0;
    std::size_t notebook_option_count_ = 3;
    bool notebook_message_visible_ = false;
    // ROM 0xC8C8 reads the persistent global FFFC22 bitfield. The current
    // native field slice has not yet ported the event setters, so a new game
    // starts at the source's empty catalog state until those setters land.
    std::uint32_t group_item_flags_ = 0;
    // ROM 0x11740 saves the inventory column/row, sets the action selector to
    // 2, and enters the four-choice loop at 0x117D4. Keep that focus separate
    // from the parent inventory cursor so the first button press cannot equip.
    bool field_menu_action_active_ = false;
    std::size_t field_menu_action_selection_ = 2;
    // ROM 0x12634 uses a separate Yes/No loop before 0x1275C removes the
    // selected possession cell. The default source choice is No.
    bool field_discard_confirmation_active_ = false;
    bool field_discard_confirmation_yes_ = false;
    bool field_discard_item_was_equipped_ = false;
    bool field_discard_cannot_discard_ = false;
    // ROM 0x12836 reuses the source Yes/No writer at 0x127B0 after finding
    // the first eligible recipient and an empty destination slot. Keep the
    // recipient and destination slot explicit so Trade cannot be confused
    // with mere inventory possession.
    bool field_trade_active_ = false;
    bool field_trade_confirmation_yes_ = false;
    bool field_trade_cannot_trade_ = false;
    bool field_trade_full_inventory_ = false;
    std::size_t field_trade_recipient_record_ = 0;
    std::size_t field_trade_recipient_slot_ = 0;
    std::string field_trade_full_inventory_message_;
    // ROM 0x12298 builds a pointer list of every record with +0x10 bit 0,
    // then uses the count entry as the visible CANCEL row. Keep that target
    // list separate from the runner-switch predicate: First Aid only tests
    // the source active bit and does not apply the switcher's bit 4/6 gates.
    bool field_first_aid_active_ = false;
    bool field_first_aid_message_active_ = false;
    std::array<std::size_t, 3> field_first_aid_target_records_{};
    std::size_t field_first_aid_target_count_ = 0;
    std::size_t field_first_aid_target_index_ = 0;
    std::string field_first_aid_message_;
    // ROM 0x121C0 clears the command panel, writes D202F, then tail-calls the
    // common button wait at 0x0D94. Keep that message modal until an A/B/
    // Start edge acknowledges it; it is not an equipment mutation.
    bool field_item_message_active_ = false;
    std::string field_item_message_;
    // Native form of the Genesis 0xD44 LCG state used by 0x0DBA. The
    // original seed comes from boot-time RAM entropy; a fixed host seed keeps
    // this extracted item path deterministic until that unrelated boot seed
    // is ported.
    std::uint16_t first_aid_random_state_ = 1;
    std::size_t clips_menu_selection_ = 2;
    std::size_t clips_last_action_selection_ = 2;
    std::uint32_t clips_action_count_ = 0;
    std::uint8_t clips_action_message_frames_ = 0;
    std::string clips_action_message_;
    bool clips_out_of_ammo_ = false;
    bool clips_reload_confirmation_active_ = false;
    bool clips_reload_confirmation_yes_ = false;
    bool clips_trade_active_ = false;
    std::array<std::size_t, 3> clips_trade_recipient_records_{};
    std::size_t clips_trade_recipient_count_ = 0;
    std::size_t clips_trade_recipient_index_ = 0;
    std::size_t clips_trade_recipient_record_ = 0;
    std::size_t clips_trade_menu_selection_ = 2;
    std::uint8_t clips_trade_quantity_ = 0;
    std::size_t attributes_menu_column_ = 0;
    std::size_t attributes_menu_row_ = 0;
    std::size_t field_last_command_selection_ = 0;
    std::size_t field_last_command_row_ = 0;
    std::uint32_t field_command_count_ = 0;
    std::uint8_t field_command_message_frames_ = 0;
    std::uint16_t menu_horizontal_repeat_frames_ = 0;
    std::uint16_t menu_vertical_repeat_frames_ = 0;
    std::uint16_t menu_horizontal_repeat_mask_ = 0;
    std::uint16_t menu_vertical_repeat_mask_ = 0;
    std::uint8_t action_flash_frames_ = 0;
    std::uint32_t action_count_ = 0;
    std::uint32_t runner_switch_attempts_ = 0;
    std::uint32_t runner_switch_rejections_ = 0;
};

} // namespace shadowrun::native
