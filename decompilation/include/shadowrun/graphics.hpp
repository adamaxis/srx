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

// Reviewed fields from the 0x100-byte entity template copied by ROM 0x42B0.
// These are kept as field roles only where the update/projection paths already
// establish them; the rest of the template remains untyped until recovered.
struct GenesisFieldEntityTemplate {
    std::size_t record_offset = 0;
    std::uint16_t projection_word_08 = 0;
    std::uint16_t kind = 0;
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
};

using GenesisTerrainMap = terrain::Map;

// Read the reviewed entity-template fields at ROM 0x7291A. The returned
// record is a host value object and has no dependency on Genesis RAM.
GenesisFieldEntityTemplate read_genesis_field_entity_template(
    const std::vector<std::uint8_t>& rom,
    std::size_t record_offset = 0x7291A);

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

// Decode one Genesis 4bpp tile. Each row contains four interleaved bitplane
// bytes: one byte for each color bit, with bit 7 representing the leftmost
// pixel.
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

// Expand the sentinel-terminated 10-byte terrain rectangles selected by the
// mode-2 record into the host grid used by ROM 0xE8A0/0xEE10.
GenesisTerrainMap decode_genesis_mode2_terrain_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources);

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

RgbaImage decode_genesis_mode2_field_map(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_data_offset = 0x166B82,
    std::size_t pattern_byte_count = 0x6E20,
    std::size_t metatile_stream_offset = 0x170102,
    std::size_t primary_tilemap_offset = 0x16D9A2,
    std::size_t secondary_tilemap_offset = 0x16ED52,
    std::size_t tilemap_width = 60,
    std::size_t tilemap_height = 42,
    std::size_t palette_offset = 0xD29D6);

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

// Resolve the field-record directional states selected by ROM 0x600E. The
// record template's +0x0E kind indexes the pointer table at 0x62AC; its +0x16
// state is a byte offset into that kind's state-pointer table. Each selected
// state list supplies an 0x8002 frame descriptor. The kind-3 field table has
// eight entries, which are returned in the table's original order as ordinary
// host RGBA images.
std::vector<RgbaImage> decode_genesis_field_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset = 0x7291A,
    std::size_t entity_state_pointer_table_offset = 0x62AC,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A,
    std::size_t state_count = 8);

// Reconstruct the startup scene assembled by ROM 0x1D00/0x1D58. The source
// offsets and dimensions are the operands of that path, not guessed graphic
// locations: 0x1F310E is the compressed pattern stream, 0x67DF6 is its
// delta-coded 40-column tilemap, and 0x1FF0 is the four-line palette image
// selected by the 0x1FD8 resource setup block.
RgbaImage decode_genesis_startup_screen(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_stream_offset = 0x1F310E,
    std::size_t tilemap_stream_offset = 0x67DF6,
    std::size_t palette_offset = 0x1FF0);

// Write a 32-bit uncompressed BMP. This is intentionally dependency-free and
// can be opened directly by Windows or used as an intermediate asset format.
void write_bmp32(const std::string& path, const RgbaImage& image);

} // namespace shadowrun::graphics
