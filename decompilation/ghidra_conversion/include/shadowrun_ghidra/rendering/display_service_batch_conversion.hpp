#pragma once

#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/field/terrain_map_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kFieldTerrainLayerWordCount = 0xE10U;

struct DisplayServiceCallbacks {
    std::function<void()> service_pending_upload;
    std::function<void()> upload_startup_palette;
    // ROM 0x5672C: advance the mode-specific field display animation when
    // the display-service word is clear.
    std::function<void()> service_field_mode_display_frame;
    std::function<void()> write_transfer_header;
    std::function<void()> write_fixed_transfer_pair;
    std::function<void()> render_normal_viewport;
    std::function<void()> render_alternate_viewport;
    std::function<void()> flush_primary_transfers;
    std::function<void()> flush_secondary_transfers;
};

struct DisplayServiceState {
    std::uint32_t frame_counter = 0;
    std::uint8_t dirty_flags = 0;
    std::uint8_t service_marker = 0;
    // ROM FFDFE8: zero permits the field display-frame service; a non-zero
    // word temporarily suppresses it during world/display transitions.
    bool field_mode_display_service_enabled = true;
    // ROM FFDD02: set after a pending VDP upload is serviced. This latch is
    // an output of the dirty-bit path, not the field-frame service gate.
    bool pending_vdp_upload_completed = false;
    bool render_normal = false;
    bool render_alternate = false;
    bool flush_primary = false;
    bool flush_secondary = false;
};

// ROM 0x01872: service the display dirty bits and field-view/transfer queues
// in source order. The callbacks are host-owned presentation boundaries.
void service_display_frame(DisplayServiceState& state,
                           const DisplayServiceCallbacks& callbacks);

// ROM 0x01070: display service wrapper whose register return values are not
// part of the native contract.
void service_display_frame_wrapper(DisplayServiceState& state,
                                    const DisplayServiceCallbacks& callbacks);

// ROM 0x01080: submit one alternating word-pair transfer.
void write_alternating_word_pair_wrapper(
    const std::function<void()>& write_pair);

// Host-owned interpretation of the direct records that sit beside the
// selected scene profile. The descriptor/event values and collision rectangles
// are direct source data; the rasterized map is the converted 0xEE10 result.
// Keeping them together makes the field-scene handoff explicit without
// carrying ROM pointers into the runtime.
struct FieldSceneSourceMetadata {
    bool available = false;
    std::uint16_t initial_camera_x_fixed = 0;
    std::uint16_t initial_camera_y_fixed = 0;
    std::array<SceneRunnerPosition, 3> initial_field_records{};
    std::uint16_t terrain_x_exclusive_upper = 0;
    std::uint16_t terrain_y_exclusive_upper = 0;
    std::uint16_t terrain_x_inclusive_lower = 0;
    std::uint16_t terrain_y_inclusive_lower = 0;
    bool event_table_available = false;
    std::array<std::uint8_t, 32> event_mode_table{};
    std::vector<TerrainCollisionRectangle> terrain_collision_rectangles;
    std::array<std::uint8_t, 0x1000> terrain_collision_map{};
};

struct FieldSceneBuffers {
    bool initialized = false;
    std::uint8_t scene_mode = 0; // selected profile: 0 normal, 1 alternate
    std::uint8_t field_resource_mode = 0; // ROM FFFBF8: low byte of scene id
    std::uint16_t scene_reference = 0; // ROM FFDDC4 / 0x03032 input
    std::array<std::uint16_t, 48> profile{};
    std::int32_t camera_x = 0;
    std::int32_t camera_y = 0;
    // Offline-decoded host bytes from the source stream published through
    // ROM FFDE22. The runtime never carries the source pointer itself.
    std::vector<std::uint8_t> terrain_stream;
    std::array<SceneRunnerPosition, 3> pending_field_targets{};
    bool pending_position_consumed = false;
    bool pending_position_cleared = false;
    std::vector<std::uint16_t> layer_a;
    std::vector<std::uint16_t> layer_b;
    FieldSceneSourceMetadata source_metadata{};
};

struct FieldSceneBufferInput {
    // ROM 0x03084 preserves FFDDC4 in D0 after the 0x3032 classifier and
    // indexes the complete pointer table at 0x3232. The classifier only
    // publishes FFFC0E; it does not select between two profiles.
    FieldSceneProfileTable scene_profiles{};
    bool scene_profile_table_available = false;
    std::uint16_t scene_reference = 0;
    std::vector<std::uint8_t> terrain_stream;
    std::vector<std::uint16_t> layer_a;
    std::vector<std::uint16_t> layer_b;
    // ROM FFFC0E: profile-specific pending-position selector.
    bool pending_position_available = false;
    // ROM FFE10E: a nonzero value suppresses the pending-position consume.
    bool pending_position_in_use = false;
    // ROM FFE10A/FFE10C: the source pair copied to all three target pairs.
    SceneRunnerPosition pending_position{};
    FieldSceneSourceMetadata source_metadata{};
};

struct FieldScenePreparationCallbacks {
    // ROM 0x03074 invokes 0xC7F8 before copying the selected scene profile
    // when the scene buffers are not already initialized.
    std::function<void()> prepare_field_scene_runtime;
};

// ROM 0x03074 -> 0x03084 -> 0x0313A: prepare the field-scene runtime when the
// source is entering a new scene, select/copy the active 48-word profile,
// publish the low-byte field resource mode and terrain-stream token, consume
// the pending position into three target pairs with the source's saturated
// camera recenter, and copy both fixed 0xE10-word terrain layers.
bool prepare_field_scene_buffers(
    FieldSceneBuffers& state,
    const FieldSceneBufferInput& input,
    const FieldScenePreparationCallbacks& callbacks);

struct HudPanelTransferCallbacks {
    // Each optional result represents one source transfer block and contains
    // the number of words emitted by that block.
    std::function<std::optional<std::size_t>()> begin_transfer;
    std::function<void(std::uint16_t)> write_word;
};

// ROM 0x03648: emit incrementing HUD panel words for each transfer block.
void write_hud_panel_transfer(std::uint16_t initial_word,
                              const HudPanelTransferCallbacks& callbacks);

struct FieldTileColumnInput {
    std::uint16_t source_coordinate = 0;
    std::uint16_t tile_key = 0;
    std::uint16_t palette_or_flip = 0;
    std::size_t source_word_offset = 0;
};

struct FieldTileColumnCallbacks {
    std::function<void()> begin_transfer;
    std::function<std::uint16_t(std::uint16_t tile_key,
                                std::size_t word_offset)>
        read_word;
    std::function<void(std::uint16_t)> write_word;
};

// ROM 0x03D4A/0x03D80: emit the visible portion of a tile column. The source
// writes a partial pre-alignment segment when needed, then continues through
// the shared 0x03D80 transfer body after aligning the destination; each body
// entry changes tile rows every four words and applies the palette/flip
// transform.
std::size_t render_field_tile_column(
    const FieldTileColumnInput& input,
    const FieldTileColumnCallbacks& callbacks);

struct FieldTileRowCallbacks {
    std::function<void()> fold_coordinate;
    std::function<std::int16_t()> column_start;
    std::function<void(std::int16_t row, std::uint16_t column_key,
                       std::int16_t start)> render_column;
};

// ROM 0x03D18: fold the field coordinate and submit the thirty-one columns
// in the source's descending row order.
void render_field_tile_rows(std::uint16_t initial_column_key,
                            std::int16_t initial_row_sum,
                            const FieldTileRowCallbacks& callbacks);

// ROM 0x03F9E: resolve a map tile through the active row-offset table.
std::optional<std::int32_t> calculate_field_tile_source_offset(
    const std::vector<std::int16_t>& row_offsets,
    std::size_t row_index,
    std::int32_t base_offset,
    const std::vector<std::uint16_t>& map_words);

struct ControllerDelayState {
    std::uint8_t controller_flags = 0;
    std::array<std::uint8_t, 2> entity_flags{};
    std::uint8_t delay = 0;
};

// ROM 0x0ABB6: arm the 0x3C-frame controller delay when all three source
// flag predicates permit activity.
bool arm_controller_activity_delay(ControllerDelayState& state);

struct EntityBehaviorDispatchCallbacks {
    std::function<void()> behavior_without_collision_flag;
    std::function<void()> behavior_with_collision_flag;
    std::function<void()> schedule_collision_recheck;
};

// ROM 0x0AC84: dispatch the entity behavior branch and optionally schedule a
// collision recheck when the global interaction marker is active.
void dispatch_entity_collision_behavior(
    std::uint8_t entity_flags,
    bool interaction_marker,
    const EntityBehaviorDispatchCallbacks& callbacks);

struct TitleSpriteWaveState {
    std::uint16_t timer = 0;
    std::uint16_t base_x = 0;
    std::uint16_t base_y = 0;
    std::uint16_t palette_selector = 0;
    bool active = true;
};

struct TitleSpriteWaveCallbacks {
    std::function<void(std::uint16_t x, std::uint16_t y,
                       std::uint16_t offset, std::uint32_t palette)> build;
};

// ROM 0x0AEBA: advance the title sprite wave by eight ticks and emit its
// thirty-two independently positioned sprite records while it is active.
bool advance_title_sprite_wave(
    TitleSpriteWaveState& state,
    const std::array<std::uint32_t, 4>& palettes,
    const TitleSpriteWaveCallbacks& callbacks);

void load_title_font(const std::function<void()>& decode_and_upload);

constexpr std::uint16_t kTitleFontVramDestination = 0xB560U;
constexpr std::uint32_t kTitleFontSourceStream = 0x000C1064U;

struct TitleFontUploadPlan {
    std::uint16_t vram_destination = kTitleFontVramDestination;
    std::uint32_t source_stream = kTitleFontSourceStream;
};

// ROM 0xC0C2: A1=FFBF40 A3=0xC1064 D0=0xB560, then jmp 0x1530.
TitleFontUploadPlan load_title_font();

struct MenuMetadataGridPlan {
    std::uint32_t destination = 0;
    std::uint16_t width = 0;  // 0x11C0 columns = D7+1
    std::uint16_t height = 0; // 0x11C0 rows = D1+1
    std::uint16_t palette = 0x6000;
    std::uint32_t glyph_source = 0x000C1320U;
};

constexpr std::uint32_t kMenuMetadataGridDestination = 0x0000C186U;
constexpr std::uint16_t kMenuMetadataGridD1 = 0x0014U;
constexpr std::uint16_t kMenuMetadataGridD7 = 0x001BU;
constexpr std::uint16_t kMenuMetadataGridWidth = kMenuMetadataGridD7 + 1U;
constexpr std::uint16_t kMenuMetadataGridHeight = kMenuMetadataGridD1 + 1U;
constexpr std::uint32_t kMenuFooterGridDestination = 0x0000C306U;
constexpr std::uint16_t kMenuFooterGridD1 = 0x0010U;
constexpr std::uint16_t kMenuFooterGridD7 = 0x001BU;
constexpr std::uint16_t kMenuFooterGridWidth = kMenuFooterGridD7 + 1U;
constexpr std::uint16_t kMenuFooterGridHeight = kMenuFooterGridD1 + 1U;

// ROM 0xC3E6: D4=0xC186 D1=0x14 D7=0x1B, then 0x12C5A → 0x11C0 dbf.
MenuMetadataGridPlan write_menu_grid_with_metadata();
void write_menu_grid_with_metadata(const std::function<void()>& write_grid);
void render_menu_grid_number_pair(const std::function<void()>& write_grid,
                                  const std::function<void()>& build_glyphs);
// ROM 0xCFFE: prepare the Hotel Options presentation.  This source helper is
// reached only by the Hotel Options flow at 0xCF38; the native name records
// that context instead of preserving Ghidra's generic title-mode label.
void prepare_hotel_options_display(
    const std::function<void()>& refresh_mode,
    const std::function<void()>& refresh_palette,
    const std::function<void()>& prepare_display,
    const std::function<void()>& load_font);
// ROM 0xDA4A: the Attributes/Skills renderer's fixed heading helper. It
// ignores caller register values, renders the encoded resource at 0xC1B3B,
// and submits it to the host text boundary.
void render_attributes_skills_heading(
    const std::function<void()>& render_text);

// ROM 0x0E660: choose the menu/world effect resource using three bounded
// source-random draws, then submit the selected two-row grid. The native
// conversion keeps the table-selection result as typed provenance; it does
// not reproduce Genesis address arithmetic or the VDP write.
enum class RandomGridPrimarySource {
    branch_zero_table,
    branch_nonzero_table,
};

enum class RandomGridVariantSource {
    fixed_three_variant_resources,
    indexed_four_variant_resources,
};

struct RandomGridEffectPlan {
    bool first_selection_nonzero = false;
    RandomGridPrimarySource primary_source =
        RandomGridPrimarySource::branch_zero_table;
    std::uint8_t primary_entry_index = 0;
    RandomGridVariantSource variant_source =
        RandomGridVariantSource::fixed_three_variant_resources;
    std::uint8_t variant_entry_index = 0;
    std::uint8_t variant_byte_offset = 0;
    std::uint8_t grid_row_count = 2;
};

struct RandomGridEffectState {
    std::uint8_t random_samples = 0;
    bool grid_write_requested = false;
    RandomGridEffectPlan plan{};
};

struct RandomGridEffectCallbacks {
    std::function<std::uint16_t(std::uint16_t bound)> sample_random;
    std::function<void(const RandomGridEffectPlan& plan)> write_grid;
};

void advance_random_grid_effect(
    RandomGridEffectState& state,
    const RandomGridEffectCallbacks& callbacks);
// ROM 0xF404: D4=0xC306 D1=0x10 D7=0x1B, then 0x12C5A.
MenuMetadataGridPlan write_menu_footer_grid();
void write_menu_footer_grid(const std::function<void()>& write_grid);
struct MenuGridQuadrupleWrite {
    std::uint32_t destination = 0;
    std::uint32_t glyph_source = 0;
    std::uint16_t width = 0;  // D1
    std::uint16_t height = 0; // D7
};

using MenuGridQuadruplePlan = std::array<MenuGridQuadrupleWrite, 4>;

constexpr std::array<std::array<std::uint32_t, 4>, 4> kMenuGridQuadrupleSources{{
    {0x000C8650U, 0x000C865AU, 0x000C8664U, 0x000C8668U},
    {0x000C866CU, 0x000C8676U, 0x000C8680U, 0x000C8684U},
    {0x000C8688U, 0x000C8692U, 0x000C869CU, 0x000C86A0U},
    {0x000C86A4U, 0x000C86AEU, 0x000C86B8U, 0x000C86BCU},
}};

// ROM 0xFDAE: FFF0DB*4 indexes 0xC8AEE. Two writes dest 0xC112/+0x180 D7=4
// D1=0, then two writes dest 0xC192/+8 D7=0 D1=1. 0x1232 stays a callback.
MenuGridQuadruplePlan write_menu_grid_quadruple(std::uint8_t frame);
void write_menu_grid_quadruple(const std::function<void()>& write_grid);

} // namespace shadowrun::ghidra_conversion
