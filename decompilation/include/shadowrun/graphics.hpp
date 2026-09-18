#pragma once

#include "shadowrun/native_port.hpp"
#include "shadowrun/terrain.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace shadowrun::graphics {

struct IndexedTile {
    static constexpr std::size_t kWidth = 8;
    static constexpr std::size_t kHeight = 8;
    std::array<std::uint8_t, kWidth * kHeight> pixels{};
};

struct RgbaImage {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::vector<native::Rgba8> pixels;
};

// The three coordinate pairs at mode-record offsets +0x42..+0x4C are copied
// by ROM 0x2B34 into the first three 0x100-byte field records. Their final
// high-level identities are not assumed here; they are retained as the
// direct field-record initialization data they demonstrably are.
struct GenesisFieldRecordPosition {
    std::uint16_t world_x_fixed = 0;
    std::uint16_t world_y_fixed = 0;
};

// The ROM has both compressed 0x1530-style metatile streams and raw 16-bit
// name-word stamp tables. Keep the distinction in the extracted descriptor
// instead of making the renderer guess from bytes.
enum class GenesisMetatileEncoding {
    CompressedResource,
    RawNameWords,
};

// Reviewed fields from the 0x100-byte entity template copied by ROM 0x42B0.
// These are kept as field roles only where the update/projection paths already
// establish them; the rest of the template remains untyped until recovered.
struct GenesisFieldEntityTemplate {
    std::size_t record_offset = 0;
    std::uint16_t projection_word_08 = 0;
    std::uint16_t kind = 0;
    // ROM 0x391A reads the entity-record type at +0x3E. This is distinct
    // from the graphics/state kind at +0x0E.
    std::uint16_t record_type = 0;
    // Command overview source fields: race/archetype indexes and the two
    // current condition percentages read from each runner record.
    std::uint8_t race_index = 0;
    std::uint8_t archetype_index = 0;
    std::uint8_t physical_value = 0;
    std::uint8_t mental_value = 0;
    // ROM command-screen routines 0x11260/0x12A8C use +0x57 as the
    // one-based equipped-item record shown in the center panel.
    std::uint8_t active_item_code = 0;
    std::uint8_t flags_10 = 0;
    std::uint8_t flags_11 = 0;
    std::uint32_t state_pointer = 0;
    std::uint16_t state_selector = 0;
    std::uint16_t timing_multiplier = 0;
    std::uint16_t timing_accumulator = 0;
    std::uint16_t timing_threshold = 0;
    std::uint16_t state_index = 0;
    // ROM 0x36FC tests +0x56 before emitting the sprite and uses +0x57 in
    // the following state branch. Preserve both signed bytes at extraction.
    std::int8_t display_state = 0;
    std::int8_t display_substate = 0;
    // ROM 0x3602 reads +0x9B to select the item-icon palette line.
    std::uint8_t display_palette_state = 0;
    // ROM 0x11928 indexes the five command-screen status strips with +0x4D.
    std::uint8_t menu_status_index = 0;
    std::array<std::uint8_t, native::kNativeInventorySlotCount>
        inventory_item_codes{};
    std::array<std::uint8_t, native::kNativeInventorySlotCount>
        inventory_phase_bytes{};
    std::uint8_t equipped_armor_index = 0;
    std::uint8_t ammo_rounds = 0;
    std::uint8_t ammo_clips = 0;
    std::uint16_t cyberware_flags = 0;
    std::array<std::uint8_t, native::kNativeAttributeCount>
        attribute_values{};
    std::array<std::uint8_t, native::kNativeSkillCount> skill_values{};
    std::array<std::uint8_t, native::kNativeAttributeSkillAuxiliaryCount>
        attribute_skill_auxiliary{};
};

// Typed view of the selected mode-2 record at ROM 0x172C80. The names here
// describe the values' role at the resource boundary; they intentionally do
// not invent meanings for the record's remaining words.
struct GenesisMode2FieldResources {
    std::size_t record_offset = 0;
    std::uint16_t initial_camera_x_fixed = 0;
    std::uint16_t initial_camera_y_fixed = 0;
    std::array<GenesisFieldRecordPosition, 3> initial_field_records{};
    std::uint16_t terrain_x_exclusive_upper = 0;
    std::uint16_t terrain_y_exclusive_upper = 0;
    std::uint16_t terrain_x_inclusive_lower = 0;
    std::uint16_t terrain_y_inclusive_lower = 0;
    std::size_t terrain_record_offset = 0;
    std::size_t setup_block_offset = 0;
    std::size_t palette_offset = 0;
    std::size_t pattern_data_offset = 0;
    std::size_t pattern_byte_count = 0;
    std::size_t metatile_stream_offset = 0;
    std::size_t primary_tilemap_offset = 0;
    std::size_t secondary_tilemap_offset = 0;
    std::size_t tilemap_width = 0;
    std::size_t tilemap_height = 0;
    GenesisMetatileEncoding metatile_encoding =
        GenesisMetatileEncoding::CompressedResource;
    // The descriptor is copied to FFDDC6 by ROM 0x3074, making this pointer
    // available at FFDE14. ROM 0x167AE indexes the pointed-to bytes with the
    // reaction substate in FFFBF9; for mode 2, the first entries select the
    // next field/session mode after a terrain reaction. This is not a direct
    // index by the raw terrain byte: 0xB7B8 first copies that byte's low five
    // bits into the reaction substate.
    std::size_t event_table_offset = 0;
    std::array<std::uint8_t, 32> event_mode_table{};
};

// The mode table at ROM 0x3232 uses the same descriptor layout for every
// field/map mode. The mode-2 name is retained above for source compatibility;
// this alias makes the shared layout explicit for other reconstructed areas.
using GenesisFieldModeResources = GenesisMode2FieldResources;

using GenesisTerrainMap = terrain::Map;

// Read the reviewed entity-template fields at ROM 0x7291A. The returned
// record is a host value object and has no dependency on Genesis RAM.
GenesisFieldEntityTemplate read_genesis_field_entity_template(
    const std::vector<std::uint8_t>& rom,
    std::size_t record_offset = 0x7291A);

// Read the 50 fixed-size item records at ROM 0xD4540. The native runtime uses
// these expanded names to present the item codes stored in +0x66 without
// retaining ROM pointers or Genesis text encoding.
std::array<native::NativeInventoryItem, native::kNativeInventoryItemCount>
read_genesis_inventory_catalog(const std::vector<std::uint8_t>& rom);

// ROM 0x11950 indexes this pointer table with the active record's +0x90
// bitfield. Convert the FF-terminated Genesis strings into host text so the
// native cyberware view has no ROM dependency at runtime.
std::array<std::string, native::kNativeCyberwareCount>
read_genesis_cyberware_catalog(const std::vector<std::uint8_t>& rom);

// ROM 0xC8C8 indexes the fifteen Group Items names through the longword
// table at 0xC25C4. These strings are FF-terminated and are extracted into
// host storage before the native runtime starts.
std::array<std::string, native::kNativeGroupItemCount>
read_genesis_group_item_catalog(const std::vector<std::uint8_t>& rom);

// ROM 0x18150 writes these three source strings when FF018F bit 0 says the
// player has no cyberdeck. They are extracted before the native runtime starts
// so the menu does not read the ROM at runtime.
std::array<std::string, native::kNativeCyberdeckUnavailableLineCount>
read_genesis_cyberdeck_unavailable_text(
    const std::vector<std::uint8_t>& rom);

// ROM 0xC448 emits the fixed Dismiss/Who/Abort labels around the active
// runner names. The VDP placeholder line at 0xC14AE is not user-facing.
std::array<std::string, native::kNativeDismissRunnerLabelCount>
read_genesis_dismiss_runner_labels(const std::vector<std::uint8_t>& rom);

// ROM 0xF33E/0xF2F6 supplies the Notebook menu labels and the exact
// no-current-run response. Preserve source leading spaces and separators in
// host-owned strings before the runtime starts.
std::array<std::string, native::kNativeNotebookLabelCount>
read_genesis_notebook_labels(const std::vector<std::uint8_t>& rom);
std::array<std::string, native::kNativeNotebookNoRunLineCount>
read_genesis_notebook_no_run_text(const std::vector<std::uint8_t>& rom);
// ROM 0xF9A4 reads three 32-entry pointer tables selected by FFFC30/34/38.
// 0x80 is converted to a host newline and 0xFF terminates each source item.
std::array<std::string, native::kNativeNotebookTipCount>
read_genesis_notebook_tip_catalog(const std::vector<std::uint8_t>& rom);

// ROM 0xD01C loads twenty labels through the pointer table at 0xC1A2A.
std::array<std::string,
          native::kNativeAttributeCount + native::kNativeSkillCount>
read_genesis_attribute_skill_catalog(const std::vector<std::uint8_t>& rom);

// Extract the startup menu labels at ROM 0x1B6499 and the 0xFF-delimited
// opening story block at ROM 0x5B7D8. The native boundary retains ordinary
// host text; the opening renderer follows the source routine's timed C0DA
// write groups instead of carrying VDP state into the runtime.
native::NativeIntroData read_genesis_intro_data(
    const std::vector<std::uint8_t>& rom);

// Assemble the currently reconstructed native title-to-field data from the
// ROM into host-owned RGBA/state values. This is the extraction boundary used
// to build a runtime asset pack; it does not execute ROM code.
native::NativePortAssets decode_genesis_native_assets(
    const std::vector<std::uint8_t>& rom);

// Persist/load the host-only asset pack consumed by NativePortShell. The pack
// contains RGBA images, terrain/status data, initial field positions, and the
// reviewed entity/direction state; no 68000 interpreter state is serialized.
void write_native_asset_pack(const std::string& path,
                             const native::NativePortAssets& assets);
native::NativePortAssets read_native_asset_pack(const std::string& path);

// Genesis CRAM stores three 3-bit channels in the spaced bit positions of a
// 16-bit word. This expands each channel to the normal 0..255 host range.
native::Rgba8 decode_genesis_cram_word(std::uint16_t cram_word,
                                       bool transparent = false);

// Decode one Genesis 4bpp tile. Each row contains four packed bytes, with the
// high nibble holding the left pixel and the low nibble holding the right
// pixel. The result is host-native color-index pixels.
IndexedTile decode_genesis_tile(const std::uint8_t* source,
                                std::size_t source_size);

// Decode the ROM 0x1530 resource stream without emitting to a VDP. The first
// word is the DBF counter and the remaining stream uses the ROM's literal,
// repeated-byte, and 11-bit overlapping back-reference tokens.
std::vector<std::uint8_t> decode_genesis_resource_stream(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset);

std::array<native::Rgba8, 16> read_genesis_palette(
    const std::vector<std::uint8_t>& rom, std::size_t byte_offset);

// Read all four 16-color Genesis palette lines from a contiguous CRAM image.
// Palette expansion is a load-time conversion; native rendering consumes the
// resulting RGBA8 colors rather than palette indices.
std::array<native::Rgba8, 64> read_genesis_palette_set(
    const std::vector<std::uint8_t>& rom, std::size_t byte_offset);

// Read the resource pointers and dimensions used by the first mode-2 field.
// The setup block is followed through the record's setup pointer, so callers
// do not need to duplicate the ROM layout or rely on fixed camera literals.
GenesisMode2FieldResources read_genesis_mode2_field_resources(
    const std::vector<std::uint8_t>& rom,
    std::size_t record_offset = 0x172C80);

// Resolve one of the 56 field descriptors selected by ROM 0x3074. Map
// pointers are not required to be ordered: the small mode-10..19 maps store
// the two layers in either order, but their absolute separation still gives
// the exact map entry count.
GenesisFieldModeResources read_genesis_field_mode_resources(
    const std::vector<std::uint8_t>& rom, std::uint16_t mode_id);

// Expand the sentinel-terminated 10-byte terrain rectangles selected by the
// mode-2 record into the host grid used by ROM 0xE8A0/0xEE10.
GenesisTerrainMap decode_genesis_mode2_terrain_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources);

GenesisTerrainMap decode_genesis_field_mode_terrain_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisFieldModeResources& resources);

RgbaImage decode_tile_atlas(
    const std::vector<std::uint8_t>& rom,
    std::size_t tile_byte_offset,
    std::size_t tile_count,
    const std::array<native::Rgba8, 16>& palette,
    std::size_t columns = 16,
    int transparent_index = -1);

// Decode the first native field presentation selected by the mode-2 record at
// ROM 0x172C80. The ROM expands 0x166B82 into pattern graphics, expands the
// tagged 0x170102 source into 4x4 VDP name-word stamps, and indexes those
// stamps through the two 60x42 tables at 0x16D9A2 and 0x16ED52. This produces
// a host RGBA map (1920x1344) rather than a VDP/VRAM representation.
RgbaImage decode_genesis_mode2_field_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources);

RgbaImage decode_genesis_field_mode_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisFieldModeResources& resources);

RgbaImage decode_genesis_mode2_field_map(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_data_offset = 0x166B82,
    std::size_t pattern_byte_count = 0x6E20,
    std::size_t metatile_stream_offset = 0x170102,
    std::size_t primary_tilemap_offset = 0x16D9A2,
    std::size_t secondary_tilemap_offset = 0x16ED52,
    std::size_t tilemap_width = 60,
    std::size_t tilemap_height = 42,
    std::size_t palette_offset = 0xD29D6,
    GenesisMetatileEncoding metatile_encoding =
        GenesisMetatileEncoding::CompressedResource);

// Decode the reviewed 0x8002 entity frame descriptor at ROM 0x9540. ROM
// 0x3E86 expands the packed source through the nibble table at 0x3EDC and
// writes the result as column-major Genesis tile data. This helper performs
// those extraction steps once and returns an ordinary transparent RGBA image;
// the native renderer never needs the packed source or a palette index.
RgbaImage decode_genesis_entity_frame(
    const std::vector<std::uint8_t>& rom,
    std::size_t descriptor_offset = 0x9540,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A);

// Extract the seven frame descriptors referenced by the reviewed state block
// at ROM 0x72B0. The block's 0x8007 marker supplies the pointer count; each
// pointer resolves through decode_genesis_entity_frame above. The returned
// images are independent host assets and retain no ROM/VDP state.
std::vector<RgbaImage> decode_genesis_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t state_block_offset = 0x72B0,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A);

// Resolve the initial field-record directional states selected by ROM 0x600E.
// The record template's +0x0E kind indexes the pointer table at 0x62AC; its
// +0x16 state is a byte offset into that kind's state-pointer table. Each
// selected state list supplies one 0x8002 frame descriptor. The kind-3 field
// table has eight entries, which are returned in the table's original order
// as ordinary host RGBA images. This is retained as the fallback path for
// asset packs written before the full walking lists were extracted.
std::vector<RgbaImage> decode_genesis_field_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset = 0x7291A,
    std::size_t entity_state_pointer_table_offset = 0x62AC,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A,
    std::size_t state_count = 8);

struct GenesisFieldEntityAnimation {
    // ROM 0xAFF2 writes this byte offset from 0xC072 into the kind-3 table.
    std::uint16_t state_selector = 0;
    // The signed 0x8002 list marker gives the number of frame descriptors.
    std::uint16_t state_marker = 0;
    // The word between the marker and descriptor table is preserved without
    // assigning it a host meaning; it is part of the reviewed list layout.
    std::uint16_t state_auxiliary_word = 0;
    // ROM 0x600E reads this word at first_pointer + 0xCA, then selects the
    // 0x10/0x1C record multiplier. Both are retained at extraction.
    std::uint16_t source_timing_value = 0;
    std::uint16_t timing_multiplier = 0;
    std::vector<RgbaImage> frames;
};

// Extract the complete kind-3 field walking table selected by ROM 0xAFF2's
// 0xC072 entries. The native runtime consumes the resulting transparent RGBA
// frame sequences and does not retain ROM pointers, VDP state, or palettes.
std::vector<GenesisFieldEntityAnimation>
decode_genesis_field_entity_animation_sequences(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset = 0x7291A,
    std::size_t entity_state_pointer_table_offset = 0x62AC,
    std::size_t animation_frame_table_offset = 0xC072,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A,
    std::size_t state_count = 8);

// Reconstruct the startup scene assembled by ROM 0x1D00/0x1D58. The source
// offsets and dimensions are the operands of that path, not guessed graphic
// locations: 0x1F310E is the compressed pattern stream, 0x67DF6 is its
// delta-coded 40-column tilemap. The circuit field and logo/text layers are
// kept as separate host layers because the ROM reveals and updates them at
// different times during startup.
RgbaImage decode_genesis_startup_screen(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_stream_offset = 0x1F310E,
    std::size_t tilemap_stream_offset = 0x67DF6,
    std::size_t palette_offset = 0x1FF0,
    std::uint16_t name_word_mask = 0x4000);

// Title layers are extracted separately because the ROM does not present a
// completed bitmap in one operation. These functions return host RGBA
// surfaces, with transparent pixels where the corresponding Genesis plane
// leaves the underlying layer visible.
RgbaImage decode_genesis_title_background(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_title_logo_plane(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_title_text(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, native::kNativeTitleAnimationFrameCount>
decode_genesis_title_sprite_frames(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_startup_font(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_startup_cursor(
    const std::vector<std::uint8_t>& rom);

// Compose the title frame after the ROM 0x1DCA/0x1DF6 VRAM text writes. The
// returned image contains the extracted logo plus the ROM's own glyph tiles
// for PRESS START and the copyright line; the native runtime supplies the
// original 10/64-frame prompt blanking cadence.
RgbaImage decode_genesis_title_screen(
    const std::vector<std::uint8_t>& rom);

// Extract the six source title sprite phases used by ROM 0x1C6A. Each image
// is fully composed for the host surface; no VDP/SAT state is required by
// the native runtime.
std::array<RgbaImage, native::kNativeTitleAnimationFrameCount>
decode_genesis_title_animation_frames(
    const std::vector<std::uint8_t>& rom);

// Reconstruct the field command screen's plane-B base from the exact 0x1232
// writers called by ROM 0x10F50. This is host RGBA output, not a Genesis
// name-table or palette dependency retained by the runtime.
RgbaImage decode_genesis_field_menu_background(
    const std::vector<std::uint8_t>& rom);

RgbaImage decode_genesis_field_menu_inventory_boxes(
    const std::vector<std::uint8_t>& rom);

RgbaImage decode_genesis_field_menu_font(
    const std::vector<std::uint8_t>& rom);

// Extract the dynamic command-screen artwork emitted after the static
// 0x10F50 plane setup. These are host RGBA equivalents of the ROM's
// 0x110BC runner portrait and 0x11136/0x11190 item-icon writers.
std::array<RgbaImage, 3> decode_genesis_field_menu_runner_portraits(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 2> decode_genesis_field_menu_selected_cells(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_field_menu_cursor(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, native::kNativeInventoryItemCount>
decode_genesis_field_menu_item_art(const std::vector<std::uint8_t>& rom);
std::array<std::array<RgbaImage, 2>, native::kNativeInventoryItemCount>
decode_genesis_field_menu_inventory_icons(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, native::kNativeMenuStatusCount>
decode_genesis_field_menu_status_bars(const std::vector<std::uint8_t>& rom);

// Extract the source item-detail tables used by 0x12F70, 0x12FA4, and
// 0x13000. Each bar is a seven-tile name-table row; power labels are the
// twelve-byte FF-terminated strings selected by the item record's +0x26.
std::array<RgbaImage, 14> decode_genesis_field_menu_damage_bars(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 14> decode_genesis_field_menu_drain_bars(
    const std::vector<std::uint8_t>& rom);
// ROM 0x12A8C selects a two-tile value from 0x6AC46 using the selected
// inventory record's +0x6E byte. Entries are kept by source index so the
// native command screen can render Uses/Rtng values without inventing a
// numeric conversion.
std::array<RgbaImage, 256> decode_genesis_field_menu_detail_values(
    const std::vector<std::uint8_t>& rom);
std::array<std::string, 13> decode_genesis_field_menu_power_labels(
    const std::vector<std::uint8_t>& rom);

// Write a 32-bit uncompressed BMP. This is intentionally dependency-free and
// can be opened directly by Windows or used as an intermediate asset format.
void write_bmp32(const std::string& path, const RgbaImage& image);

} // namespace shadowrun::graphics
