#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct CommandOverviewHudPanelInput {
    // Source active record +0x3E. The ROM multiplies this word by 0x50
    // before adding the signed base tile word.
    std::uint16_t panel_variant = 0;
    // Source active record +0x2C, passed as the panel draw's D2 parameter.
    std::uint16_t field_parameter = 0;
};

struct CommandOverviewHudPanelPlan {
    std::int16_t tile_origin = 0;
    std::uint32_t destination_tile_base = 0;
    std::uint16_t field_parameter = 0;
    std::uint8_t rows = 10;
    std::uint8_t words_per_row = 8;
};

// ROM 0x110BC: the command-overview entry reads +0x3E/+0x2C, selects the
// panel tile word, and submits ten rows of eight words at D4=0xE104.
void draw_command_overview_hud_panel(
    const CommandOverviewHudPanelInput& input,
    const std::function<void(const CommandOverviewHudPanelPlan&)>& draw_panel);

struct ActiveRunnerHudPanelInput {
    // Source active record +0x3E. The three source ranges select the panel's
    // tile origin; this is a panel-layout discriminator, not a menu ID.
    std::uint16_t panel_variant = 0;
    // Caller-provided D2. The ROM 0x110E8 entry loads active record +0x2C;
    // direct 0x110EC callers prepare this parameter themselves.
    std::uint16_t field_parameter = 0;
    // Caller-provided D4. The shared entry does not initialize the panel
    // destination; its callers select the destination for their menu.
    std::uint32_t destination_tile_base = 0;
};

struct ActiveRunnerHudPanelPlan {
    std::int16_t tile_origin = 0;
    std::uint16_t field_parameter = 0;
    std::uint32_t destination_tile_base = 0;
    std::uint8_t rows = 4;
    std::uint8_t words_per_row = 4;
};

// ROM 0x110E8 -> 0x110EC: 0x110E8 loads active record +0x2C into D2 and
// enters the shared 0x110EC panel builder. 0x110EC selects the tile origin
// from +0x3E, preserves the caller's D4 destination, and submits a 4x4
// panel. The attributes/skills, field-service, and inventory/action paths
// own the surrounding menu state and share this panel layout.
void draw_active_runner_hud_panel(
    const ActiveRunnerHudPanelInput& input,
    const std::function<void(const ActiveRunnerHudPanelPlan&)>& draw_panel);

struct ActiveRunnerNameTextPlan {
    // ROM 0x1111A passes the active record's encoded name at +0x58 to the
    // common text writer.
    std::uint16_t source_record_offset = 0x58;
    std::uint16_t destination_tile_base = 0xC0A0;
    std::uint16_t palette_flags = 0x6000;
};

// ROM 0x1111A: render the selected runner's name source into the command
// overview title area. The text decoder remains the separate converted
// 0xC0DA boundary; this function only owns the source/layout selection.
void render_active_runner_name_text(
    const std::function<void(const ActiveRunnerNameTextPlan&)>& render_text);

struct CommandOverviewSpriteBuildPlan {
    // ROM 0x11592 uses the extracted 0xD29F6 cursor records. The table is a
    // source-data boundary here; the callback owns the already-decoded host
    // sprite records and does not receive a Genesis address.
    std::string_view source_alias = "menu.field.cursor";
    // The source multiplies the two command-overview selectors before
    // entering 0x1378: primary rows are 0x28 pixels apart and secondary
    // columns are 0x18 pixels apart.
    std::int16_t y_offset = 0;
    std::int16_t x_offset = 0;
    std::uint8_t first_sequence = 2;
    std::uint16_t attribute_flags = 0x2000;
};

struct CommandOverviewActionTitleTextPlan {
    // The action-zero branch supplies the source table at 0xD1CA5 to the
    // common text writer. Its host extraction alias is still an open data
    // table boundary, so this symbolic name is not a runtime ROM lookup.
    std::string_view source_table =
        "menu.field.command_overview_action_title_table";
    std::uint16_t destination_tile_base = 0xC520;
    std::uint16_t palette_flags = 0x6000;
};

struct CommandOverviewActionRefreshState {
    // Source FFF0D0/FFF0D1 are the primary action page and secondary item
    // page, not generic menu counters.
    std::uint8_t primary_page = 0;
    std::uint8_t secondary_page = 0;
    // FFDD02 starts at one before 0x1378 appends cursor records.
    std::uint16_t sprite_record_count = 1;
    // FFD980 is the sprite-list-ready marker published after the build.
    bool sprite_list_ready = false;
    // FFEF14 bit 1 requests the sprite-record presentation upload.
    std::uint8_t display_dirty_flags = 0;
};

struct CommandOverviewActionRefreshCallbacks {
    // Return the number of typed host records built from the cursor table.
    std::function<std::size_t(const CommandOverviewSpriteBuildPlan&)>
        build_title_sprite_records;
    // The source backs A3 up one record and clears that record's sequence
    // byte after 0x1378 returns.
    std::function<void()> clear_last_sprite_sequence;
    std::function<void()> prepare_menu_grid_variant_b;
    // ROM 0x12148 is represented as a typed record read, not a raw address.
    std::function<std::uint8_t()> read_entity_action_code;
    // 0x12A8C restores its incoming D0 before returning, so the caller's
    // action code remains the value used by the 0x10 threshold below.
    std::function<void()> render_item_action_details;
    std::function<void()> service_entity_action_menu;
    std::function<void()> prepare_menu_dialog;
    std::function<void(const CommandOverviewActionTitleTextPlan&)>
        render_action_title_text;
};

struct CommandOverviewActionRefreshResult {
    std::uint8_t action_code = 0;
    std::uint8_t item_details_result = 0x10;
    bool item_details_rendered = false;
    bool action_menu_serviced = false;
    bool dialog_prepared = false;
    bool action_title_rendered = false;
    CommandOverviewSpriteBuildPlan sprite_plan{};
};

// ROM 0x11592: rebuild the command-overview cursor at the selected page and
// column, publish its sprite-upload flags, then choose either the selected
// entity action-detail/dialog path or the action-zero title-text path. The
// Ghidra C export loses this read-result branch; the disassembly at
// 0x115E6/0x1160C is the source of truth for the typed contract below.
CommandOverviewActionRefreshResult refresh_command_overview_action_details(
    CommandOverviewActionRefreshState& state,
    const CommandOverviewActionRefreshCallbacks& callbacks);

struct MenuTextTableRef {
    // Host asset alias supplied by the caller; no ROM address is required by
    // the conversion boundary.
    std::string_view alias;
};

struct MenuTitleTextInput {
    // Source FFF0D0 selector. Values below two use the fixed Inventory
    // fallback record; values two and above index the caller's table.
    std::uint8_t selector = 0;
    MenuTextTableRef caller_table{};
};

struct MenuTitleTextPlan {
    std::string_view source_alias;
    std::size_t record_index = 0;
    std::size_t record_offset_bytes = 0;
    std::uint16_t destination_tile_base = 0xEC9C;
    std::uint16_t palette_flags = 0x6000;
    bool used_inventory_fallback = false;
};

// ROM 0x11712: select the caller-supplied 0x12-byte text record for selectors
// two and above, or the first Inventory record from 0xD1D28 for selectors
// zero and one. This models the source-table choice before 0xC0DA.
std::optional<MenuTitleTextPlan> select_menu_title_text(
    const MenuTitleTextInput& input);

// ROM 0x1170C -> 0x11712: supply the command-overview label table at
// 0xD1D3A, then apply the shared selector above.
std::optional<MenuTitleTextPlan> select_command_overview_title_text(
    std::uint8_t selector);

struct HudGlyphAttributeInput {
    // 0x03602 compares raw bytes and branches on the sign bit of the wrapped
    // CMP.B result. Keep the original byte patterns instead of converting
    // sentinel values into ordinary host ordering.
    std::int8_t action_code = 0; // source record +0x57
    std::int8_t class_code = 0;  // source record +0x9B
    std::int8_t equipped_slot = -1; // signed source record +0x56
};

// ROM 0x03602: select the palette/attribute word ORed into each menu glyph.
// The source uses the action code, class code, and signed equipped-slot state;
// it is not an empty post-panel callback.
std::uint16_t select_hud_glyph_attribute_flags(
    const HudGlyphAttributeInput& input);

struct InventoryGlyphSlot {
    // Byte at *FFFC04 + 0x66 + i. Zero selects the empty glyph at 0x34F8.
    std::uint8_t item_code = 0;
    // 0x3602 reads +0x57/+0x9B/+0x56 from A0. ROM ADDQ.L #1,A0 each pass, so
    // slot i uses record+i for those offsets.
    HudGlyphAttributeInput attributes{};
};

struct InventoryGlyphWrite {
    std::uint16_t destination = 0; // D4, starts 0xC784
    std::uint32_t glyph_source = 0; // A1
    std::uint16_t attribute_flags = 0; // D2 from 0x3602
};

using InventoryGlyphRowSlots = std::array<InventoryGlyphSlot, 8>;
using InventoryGlyphRowPlan = std::array<InventoryGlyphWrite, 8>;

constexpr std::uint16_t kInventoryGlyphRowOrigin = 0xC784U;
constexpr std::uint32_t kEmptyInventoryGlyphSource = 0x000034F8U;

// ROM 0x11136: eight inventory glyphs from record +0x66, dest 0xC784 with
// column +0xA and even-D3 wrap +0x180-0x14. 0x11190 stays a host write.
InventoryGlyphRowPlan render_inventory_glyph_row(
    const InventoryGlyphRowSlots& slots);
void render_inventory_glyph_row(
    const InventoryGlyphRowSlots& slots,
    const std::function<void(const InventoryGlyphWrite&)>& write_glyph);

// ROM 0x11442: reset the upload cursor before submitting one menu tile block.
// D0 is preserved by the upload leaf and is returned to the caller; callers
// use it as the signed page-navigation delta.
std::int8_t reset_and_upload_menu_tile_block(
    const std::function<void()>& reset_upload_state,
    const std::function<void()>& upload_tile_block,
    std::int8_t preserved_d0 = 0);

struct CommandOverviewPrimaryPageState {
    // FFF0D0: command action-page selector, normally 0..6.
    std::uint8_t page = 0;
    // FFF112: source marker used when entering the command-overview panel.
    std::int8_t auxiliary_marker = 0;
};

struct CommandOverviewPrimaryPageCallbacks {
    // ROM 0x11442 returns the caller's signed D0 delta after uploading.
    std::function<std::int8_t()> reset_and_upload_menu_tile_block;
    std::function<void()> reset_vdp_upload_state;
    std::function<void()> reset_menu_upload_cursor_and_upload;
    std::function<void()> refresh_action_details;
};

// ROM 0x114A0: advance the command-overview action page, wrap the seven-page
// selector, publish the panel marker at the page-2 boundary, and either
// refresh the action details or reset the upload path for the larger panels.
void advance_command_overview_primary_page(
    CommandOverviewPrimaryPageState& state,
    const CommandOverviewPrimaryPageCallbacks& callbacks);

struct CommandOverviewSecondaryPageState {
    // Shared FFF0D0 action-page selector and FFF0D1 item/detail page.
    std::uint8_t primary_page = 0;
    std::uint8_t secondary_page = 0;
    // FFF115: one-shot action-dialog gate; negative means armed/consumed.
    std::int8_t action_dialog_gate = 0;
    // FFDD22: source controller edge mask used only by the dialog gate.
    std::uint16_t controller_events = 0;
};

struct CommandOverviewSecondaryPageCallbacks {
    std::function<std::int8_t()> reset_and_upload_menu_tile_block;
    std::function<void()> run_action_dialog;
    std::function<void()> reset_menu_upload_cursor_and_upload;
    std::function<void()> refresh_action_details;
};

// ROM 0x11506: on action pages two and above, service the one-shot action
// dialog when no navigation edge is present; on pages zero and one, advance
// the secondary item page, clamp four back to three, and refresh details at
// the source underflow/wrap boundary.
void advance_command_overview_secondary_page(
    CommandOverviewSecondaryPageState& state,
    const CommandOverviewSecondaryPageCallbacks& callbacks);

enum class CommandOverviewExitReason {
    controller,
    action_dialog,
    inventory,
    controller_input_missing,
};

struct CommandOverviewState {
    // FFF0D0/FFF0D1: command-overview primary and secondary selectors.
    std::uint8_t primary_page = 6;
    std::uint8_t secondary_page = 0;
    // FFF09D, FFF0B1, FFF0CD, FFF0E0, FFF0E7, FFF100, FFF0F2/FFF112,
    // and FFF115 are named host state rather than byte-address lookups.
    std::int8_t action_marker = 0;
    std::int8_t exit_marker = 0;
    std::int8_t inventory_exit_marker = 0;
    std::int8_t panel_marker = 0;
    std::int8_t graphics_marker = 0;
    std::int8_t resource_variant = 0;
    std::int8_t refresh_marker = 0;
    std::int8_t action_dialog_gate = 0;
};

struct CommandOverviewInput {
    // Active record +0x10. Bits 6 and 4 request the source mode-action hook
    // on both entry and exit.
    std::uint8_t active_mode_flags = 0;
    // Active record +0x9B. The signed compare controls the inventory return
    // path for the class-one equipment panel.
    std::int8_t active_class_code = 0;
};

struct CommandOverviewTileUploadPlan {
    std::string_view asset_alias;
};

struct CommandOverviewGridPlan {
    std::string_view asset_alias;
    std::uint16_t destination_tile_base = 0;
    // 0x1232 writes D1+1 rows of D7+1 words and ORs D2 into every word.
    std::uint8_t rows = 0;
    std::uint8_t words_per_row = 0;
    std::uint16_t word_mask = 0;
};

struct CommandOverviewRepeatedAssetGroup {
    // Source FF0100 + 0x10 bit 0 selects the entity-provided table at +0x50;
    // otherwise the fixed 0x720FA table is used. Both become typed host data.
    bool uses_entity_asset_table = false;
    std::array<std::uint16_t, 80> asset_indices{};
};

struct CommandOverviewRepeatedAssetInput {
    std::array<CommandOverviewRepeatedAssetGroup, 3> groups{};
};

struct CommandOverviewRepeatedAssetPlan {
    std::size_t group_index = 0;
    std::size_t entry_index = 0;
    std::uint16_t asset_index = 0;
    std::uint16_t destination_offset = 0x20;
    std::uint16_t byte_count = 0x20;
    bool uses_entity_asset_table = false;
};

enum class CommandOverviewResourceVariant {
    primary,
    alternate,
};

struct CommandOverviewResourceUploadPlan {
    CommandOverviewResourceVariant variant =
        CommandOverviewResourceVariant::primary;
    std::string_view asset_alias;
    std::uint32_t byte_count = 0x5C80;
};

struct CommandOverviewBlockTransferPlan {
    std::string_view asset_alias;
    std::uint32_t destination = 0;
    std::uint32_t byte_count = 0;
};

struct CommandOverviewPreparationInput {
    CommandOverviewRepeatedAssetInput repeated_assets{};
    bool alternate_resource_variant = false;
};

struct CommandOverviewPreparationCallbacks {
    std::function<void()> prepare_menu_graphics;
    std::function<void(const CommandOverviewGridPlan&)> write_grid;
    std::function<void(const CommandOverviewRepeatedAssetPlan&)>
        write_repeated_asset;
    std::function<void(const CommandOverviewResourceUploadPlan&)>
        upload_resource_stream;
    std::function<void(const CommandOverviewBlockTransferPlan&)> write_block;
};

struct CommandOverviewPreparationResult {
    std::uint8_t primary_page = 6;
    std::uint8_t secondary_page = 0;
    CommandOverviewResourceVariant resource_variant =
        CommandOverviewResourceVariant::primary;
    std::size_t grid_writes = 0;
    std::size_t repeated_asset_writes = 0;
    std::size_t resource_uploads = 0;
    std::size_t block_writes = 0;
    bool refresh_marker_cleared = false;
};

// ROM 0x10FF4: translate the three 0x100-byte entity groups into 240 typed
// 0x20-byte asset transfers. No fixed-table or runtime pointer survives.
std::size_t write_command_overview_repeated_asset_batch(
    const CommandOverviewRepeatedAssetInput& input,
    const std::function<void(const CommandOverviewRepeatedAssetPlan&)>&
        write_asset);

// ROM 0x1107E: select the decoded command-overview resource variant from the
// source resource marker and submit it to the host asset boundary.
void upload_command_overview_resource(
    bool alternate_resource_variant,
    const std::function<void(const CommandOverviewResourceUploadPlan&)>&
        upload_resource);

// ROM 0x11048: upload the selected resource, then write the two fixed source
// blocks used by the command-overview presentation.
std::size_t upload_command_overview_resource_and_blocks(
    bool alternate_resource_variant,
    const CommandOverviewPreparationCallbacks& callbacks);

// ROM 0x10F50: prepare command-overview state and source-derived layout data.
// The output is an ordered list of typed host presentation requests.
CommandOverviewPreparationResult prepare_command_overview(
    const CommandOverviewPreparationInput& input,
    const CommandOverviewPreparationCallbacks& callbacks);

struct CommandOverviewCallbacks {
    std::function<void()> handle_mode_action_event;
    std::function<void(std::uint16_t display_control)> set_display_control;
    std::function<void()> prepare_command_overview;
    std::function<void()> refresh_mode_palette_data;
    std::function<void()> load_title_font;
    std::function<void()> draw_hud_panel;
    std::function<void()> render_active_runner_name;
    std::function<void()> render_menu_row;
    std::function<void()> render_equipment_highlights;
    std::function<void(const CommandOverviewTileUploadPlan&)>
        upload_menu_tile_block;
    std::function<void(const CommandOverviewTileUploadPlan&)>
        reset_upload_cursor_and_upload;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_events;
    std::function<void(std::uint16_t audio_event)> service_audio;
    std::function<void(std::uint8_t selector)> select_title_text;
    std::function<void()> render_equipment_details;
    std::function<void(CommandOverviewState&, std::int8_t delta)>
        advance_primary_page;
    std::function<void(CommandOverviewState&, std::int8_t delta)>
        advance_secondary_page;
    std::function<void(CommandOverviewState&)> run_inventory_actions;
    std::function<void()> refresh_action_details;
    std::function<void()> reset_vdp_upload_state;
};

struct CommandOverviewRunResult {
    CommandOverviewExitReason exit_reason =
        CommandOverviewExitReason::controller;
    std::uint64_t controller_iterations = 0;
    std::uint64_t detail_refreshes = 0;
    std::uint64_t full_entry_initializations = 0;
    std::uint64_t partial_refreshes = 0;
    std::uint64_t inventory_actions = 0;
    bool mode_action_handled_on_entry = false;
    bool mode_action_handled_on_exit = false;
};

// ROM 0x10D30: own the command-overview lifecycle. Entry and partial refresh
// are expressed as native presentation callbacks; controller priority,
// inventory return branches, action-dialog exits, and marker cleanup retain
// the source control flow without importing Genesis memory or VDP state.
CommandOverviewRunResult run_command_overview(
    CommandOverviewState& state,
    const CommandOverviewInput& input,
    const CommandOverviewCallbacks& callbacks);

struct InventoryHighlightGridPlan {
    std::uint32_t glyph_source = 0;
    std::uint16_t destination = 0;
    std::uint16_t height = 4; // D7
    std::uint16_t width = 3;  // D1
};

constexpr std::uint32_t kInventoryHighlightGlyphEven = 0x000CD548U;
constexpr std::uint32_t kInventoryHighlightGlyphOdd = 0x000CD520U;
constexpr std::uint16_t kInventoryHighlightDestEven = 0xE702U;
constexpr std::uint16_t kInventoryHighlightDestOdd = 0xC702U;

// ROM 0x11678: DIVU.W #2 of the inventory slot. Quotient 0 or 2 uses
// 0xCD548/0xE702; else 0xCD520/0xC702. Dest += rem*0xA + quot*0x180, then
// 0x1232 with D7=4 D1=3.
InventoryHighlightGridPlan write_inventory_highlight_grid(std::uint8_t slot);
void write_menu_grid(const std::function<void()>& write_grid);
void write_menu_grid(
    std::uint8_t slot,
    const std::function<void(const InventoryHighlightGridPlan&)>& write_grid);

struct MenuGridQuadWrite {
    std::uint32_t glyph_source = 0x000CD570U;
    std::uint16_t destination = 0;
    std::uint16_t height = 4; // D7
    std::uint16_t width = 3;  // D1
};

struct MenuGridQuadMetadata {
    std::uint16_t destination = 0xC882U;
    std::uint16_t height = 9; // D7
    std::uint16_t width = 9;  // D1
    std::uint16_t palette = 0x6000;
    std::uint32_t glyph_source = 0x000C1320U;
};

struct MenuGridQuadPlan {
    std::array<MenuGridQuadWrite, 4> grids{};
    MenuGridQuadMetadata metadata{};
};

// ROM 0x116C4: four 0x1232 writes at E702, E70C, EA0C, EA02 from 0xCD570,
// then 0x12C5A at C882 D7=9 D1=9.
MenuGridQuadPlan write_menu_grid_quad();
void write_menu_grid_quad(const std::function<void()>& write_grid,
                          const std::function<void()>& refresh_metadata);

// ROM 0x11900: prepare the menu row, then submit its repeated grid.
void prepare_menu_row_grid(const std::function<void()>& prepare_row,
                           const std::function<void()>& write_repeated_grid);

// ROM 0x11928: submit the grid before rendering the corresponding menu row.
void render_menu_row(const std::function<void()>& write_grid,
                     const std::function<void()>& render_row);

// ROM 0x11ABE: preserve the preparation result while applying the shared
// dialog service twice.
std::uint64_t prepare_menu_dialog(
    const std::function<std::uint64_t()>& prepare_dialog,
    const std::function<void()>& service_dialog);

struct MenuTextRefreshState {
    std::int8_t pending_text_count = 0;
};

// ROM 0x12062: when no text is pending, prepare and render the text twice,
// then clear the pending count.
bool refresh_menu_text(
    MenuTextRefreshState& state,
    const std::function<void()>& prepare_text,
    const std::function<void()>& render_text);

struct MenuTextCursorState {
    std::int8_t selector = 0;
    std::int16_t primary_marker = 0;
    std::int16_t secondary_marker = 0;
};

// ROM 0x1209A: alternate the two text slots until one is available, then
// render through the text boundary and run the cursor-service leaf.
void advance_menu_text_cursor(
    MenuTextCursorState& state,
    const std::function<void()>& render_text,
    const std::function<void()>& service_cursor);

struct MenuNumberRenderState {
    std::int16_t cursor = 0;
    std::uint8_t available_count = 0;
};

struct MenuNumberRenderCallbacks {
    std::function<void(std::uint16_t word)> write_number_word;
    std::function<void(std::int16_t value)> build_decimal_glyphs;
};

// ROM 0x120E4: write the number marker, advance the signed cursor with the
// source zero/overflow wrap rules, clamp past-count values, and build digits.
void render_menu_number(MenuNumberRenderState& state,
                        std::int16_t delta,
                        const MenuNumberRenderCallbacks& callbacks);

struct MenuSlotCleanupState {
    std::uint8_t slot_type = 0;
    std::uint8_t slot_subtype = 0;
    std::int8_t action_marker = 0;
    std::array<std::uint8_t, 8> pending_actions{};
    std::array<std::uint8_t, 8> active_slots{};
};

// ROM 0x1275C: clear one menu-slot pair, taking the source low-action asset
// setup branch when the shared loader sentinel is active.
void clear_menu_slot(MenuSlotCleanupState& state,
                     std::int8_t action,
                     std::size_t slot_index,
                     bool asset_load_pending,
                     const std::function<void()>& prepare_assets);

struct MenuToggleGridPlan {
    std::uint32_t glyph_source = 0x000CD47AU;
    std::uint32_t destination = 0x0000EBA8U;
    std::uint16_t column_max = 1;
    std::uint16_t row_max = 1;
    std::uint16_t palette = 0;
};

struct MenuPanelRefreshState {
    std::uint8_t toggle = 0; // FFF114
    MenuToggleGridPlan grid{};
    std::uint32_t text_source = 0x000D1EB4U;
    std::uint32_t text_destination = 0x0000EC9CU;
    std::uint16_t text_palette = 0x6000;
};

constexpr std::uint32_t kMenuToggleGridGlyphBase = 0x000CD47AU;
constexpr std::uint32_t kMenuToggleGridDestinationBase = 0x0000EBA2U;
constexpr std::uint32_t kMenuPanelTextBase = 0x000D1EB4U;

// ROM 0x127FC: FFF114 BMI selects D3=1 D4=2, else D3=0 D4=1. A1 += D3*8,
// dest = D4*6 + 0xEBA2. D7=1 D1=1 D2=0, then 0x1232.
MenuToggleGridPlan plan_menu_toggle_grid(std::uint8_t toggle);

// ROM 0x127B0: upload 0xCD3B6, XOR FFF114 with 0xFF, 0x127FC at 0xCD47A,
// then 0xC0DA at 0xD1EB4 + (negative ? 0x12 : 0) dest 0xEC9C palette 0x6000.
void refresh_menu_panel(MenuPanelRefreshState& state,
                        const std::function<void()>& upload_panel,
                        const std::function<void()>& update_layout,
                        const std::function<void()>& render_text);

struct MenuGridVariantPlan {
    std::uint16_t destination = 0;
    std::uint16_t count = 0;
    std::uint16_t palette = 0x6000;
    std::uint32_t glyph_source = 0x000C1320U;
};

constexpr std::uint16_t kMenuGridVariantADestination = 0xC518U;
constexpr std::uint16_t kMenuGridVariantACount = 0x0007U;
constexpr std::uint16_t kMenuGridVariantBDestination = 0xC118U;
constexpr std::uint16_t kMenuGridVariantBCount = 0x0013U;
constexpr std::uint16_t kMenuGridVariantCDestination = 0xC518U;
constexpr std::uint16_t kMenuGridVariantCCount = 0x000BU;
constexpr std::uint16_t kMenuGridVariantDDestination = 0xC818U;

// ROM 0x12C1E: D4=0xC518 D1=7, then 0x12C5A (A1=0xC1320 D2=0x6000).
MenuGridVariantPlan prepare_menu_grid_variant_a();
void prepare_menu_grid_variant_a(const std::function<void()>& prepare_grid);

// ROM 0x12C36: D4=0xC118 D1=0x13, then 0x12C5A.
MenuGridVariantPlan prepare_menu_grid_variant_b();
void prepare_menu_grid_variant_b(const std::function<void()>& prepare_grid);

// ROM 0x12C42: D4=0xC518 D1=0xB, then 0x12C5A.
MenuGridVariantPlan prepare_menu_grid_variant_c();
void prepare_menu_grid_variant_c(const std::function<void()>& prepare_grid);

// ROM 0x12C56: submit one repeated menu-grid write.
void write_menu_repeated_grid(const std::function<void()>& write_grid);

// ROM 0x12C5A: submit the repeated-grid write used by the neighboring menu
// service path.
void write_menu_repeated_grid_with_metadata(
    const std::function<void()>& write_grid);

// ROM 0x12CD6: submit one menu text asset.
void render_menu_text_asset(const std::function<void()>& render_text);

enum class CommandOverviewActionDialogGridKind {
    initial,
    final,
};

struct CommandOverviewActionDialogGridPlan {
    CommandOverviewActionDialogGridKind kind =
        CommandOverviewActionDialogGridKind::initial;
    std::uint32_t destination_tile_base = 0xCA9EU;
    std::uint8_t outer_count = 0;
    std::uint8_t inner_count = 5;
    std::uint16_t palette_flags = 0;
};

struct CommandOverviewActionDialogPreparationPlan {
    std::int8_t posture_delta = 0;
};

struct CommandOverviewActionDialogBarPlan {
    std::int8_t mode = 0;
    std::uint32_t destination_tile_base = 0;
};

struct CommandOverviewActionDialogInput {
    // Source FFF115. A negative value enables the guarded Down/close path.
    std::int8_t dialog_gate = 0;
};

struct CommandOverviewActionDialogCallbacks {
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_events;
    std::function<void(std::uint16_t audio_event)> service_audio;
    std::function<void(const CommandOverviewActionDialogPreparationPlan&)>
        prepare_dialog;
    std::function<void(const CommandOverviewActionDialogGridPlan&)> write_grid;
    std::function<void()> adjust_dialog_posture;
    std::function<void(const CommandOverviewActionDialogBarPlan&)>
        refresh_combat_bars;
};

struct CommandOverviewActionDialogResult {
    bool initial_grid_written = false;
    bool final_grid_written = false;
    bool closed_by_armed_gate = false;
    bool closed_by_controller = false;
    bool exit_marker_set = false;
    bool controller_input_missing = false;
    bool dialog_posture_adjusted = false;
    bool combat_bars_refreshed = false;
    std::uint64_t iterations = 0;
    std::vector<std::int8_t> posture_deltas;
};

// ROM 0x119CE: draw the command-overview action dialog, service left/right
// posture changes with audio, and close through the source's armed Down or
// controller-exit path before restoring the shared equipment bars.
CommandOverviewActionDialogResult run_command_overview_action_dialog(
    const CommandOverviewActionDialogInput& input,
    const CommandOverviewActionDialogCallbacks& callbacks);

struct CyberwareMenuState {
    // Source 0x11950 consumes bits 14..0; bit 15 belongs to a separate
    // record concern and is intentionally not treated as an installed entry.
    std::uint16_t installed_flags = 0;
    std::uint16_t rendered_entries = 0;
};

struct CyberwareMenuCallbacks {
    std::function<void()> prepare_grid_variant_b;
    std::function<void()> prepare_row_grid;
    std::function<void()> render_title_text;
    std::function<void()> wait_for_controller_event;
    std::function<void()> render_menu_row;
};

// ROM 0x11950: prepare the cyberware panel, render its installed fifteen-bit
// entries in ascending bit order, then wait for input and render the row.
std::uint16_t run_cyberware_menu(CyberwareMenuState& state,
                                 const CyberwareMenuCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
