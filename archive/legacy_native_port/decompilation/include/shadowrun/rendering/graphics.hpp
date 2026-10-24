#pragma once

#include "shadowrun/field/native_port.hpp"
#include "shadowrun/field/terrain.hpp"

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

// The flattened image and priority pass remain the renderer-facing map art.
// The two authored tilemap planes are retained for offline map inspection and
// object extraction; the runtime does not replay the ROM tilemap operations.
struct GenesisFieldMapLayers {
    RgbaImage map;
    RgbaImage priority_overlay;
    RgbaImage source_primary_plane;
    RgbaImage source_secondary_plane;
};

// A source sprite/map resource extracted from a ROM pointer table. The
// renderer-facing image is precolored RGBA; the table and pointer metadata
// remain in the catalog so a duplicate-looking resource is still auditable
// against the ROM source that selected it.
struct GenesisGraphicsResource {
    std::string role;
    std::size_t table_offset = 0;
    std::size_t table_index = 0;
    // Index of the individual SAT entry within the selected list. A resource
    // is one hardware sprite entry, never a composed SAT-list bitmap.
    std::size_t sprite_index = 0;
    std::size_t source_offset = 0;
    std::uint16_t attr_or = 0;
    // Optional source-region bounds for map-backed resources. These preserve
    // transparent cells owned by the ROM write while still allowing the
    // offline extractor to emit a compact host image instead of a 320x224
    // plate. Zero width/height means ordinary visible-bounds cropping.
    std::int32_t source_origin_x = 0;
    std::int32_t source_origin_y = 0;
    std::uint16_t source_width = 0;
    std::uint16_t source_height = 0;
    RgbaImage image;
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
    // The 16-byte source name at +0x58 is part of the roster/template
    // record. It is extracted here so recruited templates do not disappear
    // behind an anonymous numeric index.
    std::string display_name;
    std::uint16_t projection_word_08 = 0;
    std::uint16_t kind = 0;
    // ROM 0x391A reads the entity-record type at +0x3E. This is distinct
    // from the graphics/state kind at +0x0E.
    std::uint16_t record_type = 0;
    // ROM 0x110BC/0x110E8 reads the HUD panel attributes at +0x2C. The
    // 0x3404 field refresh reads the current 8-column portrait table at
    // +0x50 and its byte offset at +0x54 before uploading the 4x4 face.
    std::uint16_t command_overview_panel_parameter = 0;
    std::uint32_t hud_portrait_table_pointer = 0;
    std::uint16_t hud_portrait_table_offset = 0;
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
    // The field setup block also installs the shared entity/projectile atlas
    // at VRAM $B000. This remains an offline extraction input; game runtime
    // consumes the individually named frames produced from it.
    std::size_t object_pattern_data_offset = 0;
    std::size_t object_pattern_vram_offset = 0;
    std::size_t object_pattern_byte_count = 0;
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

// ROM 0x72C1A contains the ten named recruited-runner templates. The
// zero-kind record immediately following Stark is the source terminator; it
// is not a party member and must not be exposed as one.
std::vector<GenesisFieldEntityTemplate> read_genesis_recruited_roster(
    const std::vector<std::uint8_t>& rom);

// Read the 50 fixed-size item records at ROM 0xD4540. The native runtime uses
// these expanded names to present the item codes stored in +0x66 without
// retaining ROM pointers or Genesis text encoding.
std::array<native::NativeInventoryItem, native::kNativeInventoryItemCount>
read_genesis_inventory_catalog(const std::vector<std::uint8_t>& rom);

// ROM 0x1170C supplies the command-overview table at 0xD1D3A while selectors
// zero and one fall back to the Inventory record at 0xD1D28. Extract the
// seven selector-visible labels as ordinary host text for the direct runtime
// resource catalog; the conversion runtime never reads these ROM offsets.
std::array<std::string, 7> read_genesis_command_overview_labels(
    const std::vector<std::uint8_t>& rom);

struct GenesisCommandOverviewNames {
    std::array<std::string, 5> races{};
    std::array<std::string, 4> classes{};
};

// ROM 0x111E8 indexes the live record's +0x9A race and +0x9B class bytes
// into the fixed-width text tables at D19C7 (6-byte rows) and D19E5
// (8-byte rows), then writes them separately at C1A0 and C1AC.
GenesisCommandOverviewNames read_genesis_command_overview_names(
    const std::vector<std::uint8_t>& rom);

// ROM 0x12B8E selects the three item-detail label records at 0xD29C4,
// 0xD29CA, and 0xD29D0. Extract them once for the host item-detail catalog;
// the runtime does not carry the source text-table offsets.
std::array<std::string, 3> read_genesis_field_menu_item_detail_labels(
    const std::vector<std::uint8_t>& rom);

// ROM 0x111E8/0x12CEE and the field armor/detail branches use these fixed
// presentation records. Extract them once so the native menu consumes the
// exact source labels without retaining Genesis text-table addresses.
struct GenesisFieldMenuLabels {
    std::string physical;
    std::string mental;
    std::string attack;
    std::string defense;
    std::string damage;
    std::string drain;
    std::string none;
    std::string no_armor;
    std::string weapons;
    std::string guns;
    std::string power;
};

GenesisFieldMenuLabels read_genesis_field_menu_labels(
    const std::vector<std::uint8_t>& rom);

struct GenesisFieldServiceTitle {
    // The first word of each 0x1AAB50 title record is the source text's
    // destination cell in the field menu plane.
    std::uint16_t destination_word = 0;
    std::string text;
};

// ROM 0x56AC0 indexes the 12-entry title pointer table at 0x1AAB50 with
// FFF0B6, then renders the record's destination and encoded title. Export
// those values once so the native menu can bind a title to its source layout.
std::array<GenesisFieldServiceTitle, 12>
read_genesis_field_service_titles(const std::vector<std::uint8_t>& rom);

// ROM 0x1AA8AF contains the 32 fixed-width field-service option headings
// used above the source item row. Each record occupies 0x15 bytes; the
// visible 20-character label is followed by one separator byte.
std::array<std::string, 32>
read_genesis_field_service_action_titles(
    const std::vector<std::uint8_t>& rom);

struct GenesisFieldServiceUniversalClipsText {
    std::string title;
    std::string description;
};

// The source E offer at 0x1AADD7 has its own two-line title and description
// streams at 0x1AACCE/0x1AACE2, including explicit 0x80 line breaks.
GenesisFieldServiceUniversalClipsText
read_genesis_field_service_universal_clips_text(
    const std::vector<std::uint8_t>& rom);

// ROM 0x12634/0x127B0 supply the inventory action feedback strings.  Extract
// the individual source lines once so the host command screen can preserve
// the discard confirmation without carrying a Genesis text-table address.
struct GenesisFieldMenuActionText {
    std::string item_equipped_prefix;
    std::string item_equipped_suffix;
    std::string discard_prompt;
    std::string cannot_discard;
    std::string cannot_trade;
    std::string choice_no;
    std::string choice_yes;
    std::string automatic_use_message;
    std::string first_aid_prompt;
    std::string first_aid_label;
    std::string first_aid_cancel;
    std::string first_aid_not_injured;
    std::string first_aid_slap_patch_used;
    std::string first_aid_medkit_used;
    std::string trade_recipient_label;
    std::string trade_full_inventory_message;
    // ROM 0x57B8C and 0x5794E transaction prompt/failure text, plus the
    // 0x56F4A sale quote heading. These are host text data for the Qt shop.
    std::string transaction_prompt;
    std::string no_inventory_slot_message;
    std::string insufficient_funds_message;
    std::string special_action_limit_message;
    std::string protected_sale_message;
    std::string sale_quote_heading;
    std::string purchase_price_heading;
};

GenesisFieldMenuActionText read_genesis_field_menu_action_text(
    const std::vector<std::uint8_t>& rom);

inline constexpr std::size_t kGenesisFieldActionDetailTextCount = 32U;
using GenesisFieldActionDetailTextTable =
    std::array<std::string, kGenesisFieldActionDetailTextCount>;

// ROM 0x5878E indexes the pointer table at 0x1AC7FC. Resolve the strings at
// extraction time so the host runtime receives only named text records.
GenesisFieldActionDetailTextTable read_genesis_field_action_detail_texts(
    const std::vector<std::uint8_t>& rom);

// ROM 0x11B58/0x11E7E select the four-entry Clips rows at 0xD1DDC and
// 0xD1E24.  The Clips screen also uses these source text records for its
// title, counter label, reload prompt, and feedback lines.  Extract them once
// for the host catalog; the Qt runtime never retains these ROM offsets.
struct GenesisFieldMenuClipsText {
    std::array<std::string, 4> commands;
    std::array<std::string, 4> trade_commands;
    // 0x11E34 renders the three source lines used while a recipient trade is
    // active.  Keep the placeholder question marks as source text; the
    // recipient-name writer is a separate runtime record consumer.
    std::array<std::string, 3> trade_prompt;
    // 0x11FD4 renders these lines after a successful trade/distribution.
    std::array<std::string, 2> exchange_complete;
    std::string title;
    std::string separator;
    std::string counter_label;
    std::array<std::string, 3> reload_prompt;
    std::array<std::string, 2> reload_success;
    std::array<std::string, 2> out_of_ammo;
    std::array<std::string, 3> item_does_not_use_clips;
    std::array<std::string, 3> no_party_members;
};

GenesisFieldMenuClipsText read_genesis_field_menu_clips_text(
    const std::vector<std::uint8_t>& rom);

// Field/dialogue text records use printable bytes, 0x80-0xFE segment
// separators, and 0xFF record termination. Extract the complete segments so
// the host catalog can preserve source page boundaries instead of flattening
// every NPC line into one guessed string.
std::vector<std::string> read_genesis_dialogue_segments(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset);

// The ordinary event-31 field menu and its Cyberspace submenu use separate
// ROM text rows. Preserve the pages and the conditional empty-phone message
// for host-side inspection instead of embedding the copy in a runtime script.
struct GenesisFieldMainMenuText {
    std::string ordinary_title;
    std::array<std::string, 4> ordinary_choices;
    std::string activity_title;
    std::array<std::string, 2> activity_choices;
    std::string cyberspace_title;
    std::array<std::string, 3> cyberspace_choices;
    std::array<std::string, 2> empty_phone_message;
};

GenesisFieldMainMenuText read_genesis_field_main_menu_text(
    const std::vector<std::uint8_t>& rom);

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
// ROM 0xC8C8 writes these two lines at C50E when FFFC22 is zero, then waits
// for any controller event. Preserve the source's separate text rows.
std::array<std::string, native::kNativeGroupItemEmptyLineCount>
read_genesis_group_items_empty_text(const std::vector<std::uint8_t>& rom);

// ROM 0x18150 writes these three source strings when FF018F bit 0 says the
// player has no cyberdeck. They are extracted before the native runtime starts
// so the menu does not read the ROM at runtime.
std::array<std::string, native::kNativeCyberdeckUnavailableLineCount>
read_genesis_cyberdeck_unavailable_text(
    const std::vector<std::uint8_t>& rom);

// ROM 0xC448 emits the fixed Dismiss/Who/Abort labels and visible separator
// around the active runner names.
std::array<std::string, native::kNativeDismissRunnerLabelCount>
read_genesis_dismiss_runner_labels(const std::vector<std::uint8_t>& rom);
// ROM 0xC448 separator written between Who?? and Abort.
std::string read_genesis_dismiss_runner_separator(
    const std::vector<std::uint8_t>& rom);

// ROM 0xF33E/0xF2F6 supplies the Notebook menu labels and the exact
// no-current-run response. Preserve source leading spaces and separators in
// host-owned strings before the runtime starts.
std::array<std::string, native::kNativeNotebookLabelCount>
read_genesis_notebook_labels(const std::vector<std::uint8_t>& rom);
std::array<std::string, native::kNativeNotebookNoRunLineCount>
read_genesis_notebook_no_run_text(const std::vector<std::uint8_t>& rom);
// ROM 0xC2A6 writes the Pocket Secretary heading and five field-mode rows
// from 0xC1418. Preserve source spacing and the separator as extracted host
// text; the Qt menu draws these strings from its text catalog.
struct GenesisPocketSecretaryText {
    std::string title;
    std::string separator;
    std::array<std::string, 5> options{};
};
GenesisPocketSecretaryText read_genesis_pocket_secretary_text(
    const std::vector<std::uint8_t>& rom);

// ROM 0xCF38 renders Hotel Options from C14D0; checkout prompt 0xDA6A uses
// C164C and E387E. The Hotel status pass at 0xEF08 uses C2643/C265E. Keep the
// exact source rows as extracted host strings so the Qt screen can use the
// shared text renderer without reading ROM data at runtime.
struct GenesisHotelOptionsText {
    std::string title;
    std::string separator;
    std::array<std::string, 2> options{};
    std::array<std::string, 2> checkout_question{};
    std::array<std::string, 3> checkout_options{};
    std::string medical_attention_suffix;
    std::array<std::string, 5> recovery_message{};
};
GenesisHotelOptionsText read_genesis_hotel_options_text(
    const std::vector<std::uint8_t>& rom);

// ROM 0xD82A/0xD53C messages for the Attributes/Skills upgrade panel. The
// offline importer emits these source strings into script-text.lua.
struct GenesisAttributesSkillsText {
    std::string upgrade_header;
    std::string maximum_level_message;
    std::string mage_only_message;
    std::string insufficient_karma_message;
    std::string upgrade_prompt;
};
GenesisAttributesSkillsText read_genesis_attributes_skills_text(
    const std::vector<std::uint8_t>& rom);

// ROM 0xC0C2 uploads the menu font at VRAM 0xB560. ROM 0xC376 clears the old
// row with tile 0x5AB and selects the new row with tile 0x5FF. Options use
// palette line three; the nested checkout prompt uses palette line two. This
// is the source's 8x8 Hotel cursor, not the larger 32x24 command-screen
// inventory selector.
RgbaImage decode_genesis_hotel_options_cursor(
    const std::vector<std::uint8_t>& rom,
    std::size_t palette_line = 3U);
struct GenesisNotebookTipsText {
    std::string title;
    std::string separator;
    std::string page_count_label;
    std::string page_number_clear;
};
GenesisNotebookTipsText read_genesis_notebook_tips_text(
    const std::vector<std::uint8_t>& rom);
// ROM 0xF9A4 reads three 32-entry pointer tables selected by FFFC30/34/38.
// 0x80 is converted to a host newline and 0xFF terminates each source item.
std::array<std::string, native::kNativeNotebookTipCount>
read_genesis_notebook_tip_catalog(const std::vector<std::uint8_t>& rom);
// ROM 0xF416 emits these fixed names into the dynamic Mr. Johnson list;
// FFFC27 bits 0..4 gate the entries at runtime.
std::array<std::string, native::kNativeNotebookMrJohnsonCount>
read_genesis_notebook_mr_johnson_names(const std::vector<std::uint8_t>& rom);
// ROM 0xF416's A/B detail view clears the child panel and writes the selected
// source entry at 0xC38C; 0x80 separators become host newlines.
std::array<std::string, native::kNativeNotebookMrJohnsonCount>
read_genesis_notebook_mr_johnson_descriptions(
    const std::vector<std::uint8_t>& rom);

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
// to build the direct runtime asset directory; it does not execute ROM code.
native::NativePortAssets decode_genesis_native_assets(
    const std::vector<std::uint8_t>& rom);

// Persist/load the non-graphics native state beside the individually
// extracted BMP resources. This is deliberately not an image pack: the Qt
// runtime resolves every image through the assets/resources.tsv alias table.
void write_native_asset_state(const std::string& path,
                              const native::NativePortAssets& assets);
native::NativePortAssets read_native_asset_directory(
    const std::string& root);

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

// Apply one source 0x1530 graphics upload into a 64 KiB host buffer during
// offline extraction. The game runtime consumes the named images produced
// from these patterns, not this VRAM-shaped buffer.
std::vector<std::uint8_t> decode_genesis_1530_vram(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset,
    std::size_t target_offset);

// ROM 0x1F65A / 0x1F292: decode one field-dialogue resource profile's
// portrait stream into a host image. The resource stream and the 7x9
// portrait writer are offline extraction inputs; no VDP/name-table state is
// retained by the native or Ghidra-conversion runtimes.
RgbaImage decode_genesis_field_dialogue_portrait(
    const std::vector<std::uint8_t>& rom, std::size_t profile_index);

// ROM 0x1F292 consumes the same palette/pattern/7x9 sprite-profile shape as
// 0x1F65A. Decode a directly selected profile into a named host image for
// offline extraction; the runtime does not retain its ROM pointer or VDP map.
RgbaImage decode_genesis_field_mode_profile_sprite(
    const std::vector<std::uint8_t>& rom, std::size_t profile_offset);

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

GenesisFieldMapLayers decode_genesis_mode2_field_map_layers(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources);

RgbaImage decode_genesis_field_mode_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisFieldModeResources& resources);

GenesisFieldMapLayers decode_genesis_field_mode_map_layers(
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

GenesisFieldMapLayers decode_genesis_mode2_field_map_layers(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_data_offset,
    std::size_t pattern_byte_count,
    std::size_t metatile_stream_offset,
    std::size_t primary_tilemap_offset,
    std::size_t secondary_tilemap_offset,
    std::size_t tilemap_width,
    std::size_t tilemap_height,
    std::size_t palette_offset,
    GenesisMetatileEncoding metatile_encoding =
        GenesisMetatileEncoding::CompressedResource);

// Decode the reviewed 0x8002 entity frame descriptor at ROM 0x9540. ROM
// 0x3E86 is conditional: only records with +0x11 bit 5 set expand the source
// through the nibble table at 0x3EDC. Ordinary field records upload their raw
// packed tile bytes. Extraction bakes the selected color line into RGBA; a
// grayscale twin is stored beside it. The native renderer never palette-swaps
// at runtime.
RgbaImage decode_genesis_entity_frame(
    const std::vector<std::uint8_t>& rom,
    std::size_t descriptor_offset = 0x9540,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A,
    bool expand_packed_source = true,
    int palette_line = -1,
    bool compose_field_palette = false);

// Host luminance copy of a baked RGBA sprite. Used so a grey runner is a
// second image, not a live CRAM line.
RgbaImage make_grayscale_image(const RgbaImage& color);

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
// older source-analysis data before the full walking lists were extracted.
std::vector<RgbaImage> decode_genesis_field_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset = 0x7291A,
    std::size_t entity_state_pointer_table_offset = 0x62AC,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A,
    std::size_t state_count = 8);

struct GenesisFieldEntityAnimation {
    // The source selector table writes this byte offset into the kind-3 table.
    std::uint16_t state_selector = 0;
    // A positive marker loops after this many descriptors; a high-bit marker
    // is a finite transition list that holds its last descriptor.
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

struct GenesisFieldEntityStateFrame {
    RgbaImage image;
    std::size_t descriptor_offset = 0;
    std::int16_t origin_x = 0;
    std::int16_t origin_y = 0;
};

// Resolve one live field entity's kind/state/frame selection through the
// source tables and return its ordinary host image plus the signed sprite
// origin from the selected 0x8002 descriptor. This is an offline import
// helper; runtime rendering consumes the resulting named image and typed
// origin, never the ROM tables.
GenesisFieldEntityStateFrame decode_genesis_field_entity_state_frame(
    const std::vector<std::uint8_t>& rom,
    std::uint16_t entity_kind,
    int palette_line,
    std::uint16_t state_selector = 0,
    std::uint16_t state_index = 0,
    std::size_t entity_state_pointer_table_offset = 0x62AC,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A);

struct GenesisFieldSpawnSpriteFrame {
    RgbaImage image;
    // Cropped image's top-left relative to the source entity's +0x0A/+0x0C
    // sprite origin, preserving the list placement when drawn by the host.
    std::int16_t origin_x = 0;
    std::int16_t origin_y = 0;
};

struct GenesisFieldSpawnAnimation {
    std::uint16_t selector = 0;
    std::uint16_t state_marker = 0;
    std::uint16_t frame_increment = 0;
    std::vector<GenesisFieldSpawnSpriteFrame> frames;
};

// Decode one kind-3 selector family into transparent host frames. The default
// 0xC0B2 family is used by behavior transitions; normal accepted field
// movement selects the positive-marker 0xC062 family. Neighboring tables
// remain available through the explicit offset parameter for source audits.
// Runtime clients consume named RGBA files and typed timing/state rather than
// retaining ROM pointers or reproducing VDP state.
std::vector<GenesisFieldEntityAnimation>
decode_genesis_field_entity_animation_sequences(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset = 0x7291A,
    std::size_t entity_state_pointer_table_offset = 0x62AC,
    std::size_t animation_frame_table_offset = 0xC0B2,
    std::size_t nibble_table_offset = 0x3EDC,
    std::size_t palette_offset = 0x172D8A,
    std::size_t state_count = 8);

// Decode the 16 ordinary/special direction lists selected by ROM 0x62EC and
// 0x62FC, using the shared object atlas referenced by the field setup block.
// This is offline import only. The runtime loads the named frames and uses
// the selector and source animation cadence as typed state.
std::array<GenesisFieldSpawnAnimation, 16>
decode_genesis_field_spawn_animations(const std::vector<std::uint8_t>& rom);

// Offline extraction for the four source action-code variants selected by
// field entity targeting at 0x5C7E. The images are the individual 0x5E5,
// 0x5E6, and 0x5E7 object-pattern tiles with the source field palette lines;
// descriptor flips and placement remain separate runtime presentation facts.
std::array<RgbaImage, 4> decode_genesis_field_target_marker_images(
    const std::vector<std::uint8_t>& rom, std::uint16_t scene_id = 0U);

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
    std::uint16_t name_word_mask = 0x4000,
    int circuit_light_phase = -1);
std::array<RgbaImage, native::kNativeTitleCircuitLightCount>
decode_genesis_title_circuit_lights(const std::vector<std::uint8_t>& rom);
// The returned frames are the six source $5B180 SAT overlay phases used by
// 0x5B100. They are transparent 512px canvases so the source rotor/aircraft
// strip that begins off the right edge is preserved before DD08 scrolls it
// through the 320px viewport. The separate Plane-A map is exposed below.
std::array<RgbaImage, native::kNativeIntroHeliFrameCount>
decode_genesis_intro_heli_frames(const std::vector<std::uint8_t>& rom);
// ROM 0x5B1B6's late movement path alternates the two source SAT lists at
// 0x1DF3F4/0x1DF3FE. These are exposed separately from the earlier 0x5B100
// overlay phases so the landing sprite, including its rotor tile, can be
// checked against the actual source lists.
std::array<RgbaImage, native::kNativeIntroAircraftMovementFrameCount>
decode_genesis_intro_aircraft_movement_frames(
    const std::vector<std::uint8_t>& rom);
// ROM 0x5B528's first 0x1406 map is the transient Plane-A vehicle layer.
// This diagnostic keeps it separate from the settled Plane-B airport plate
// so its source HSCROLL path can be inspected independently.
RgbaImage decode_genesis_intro_aircraft_plane_a(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, native::kNativeIntroWildernessPanFrameCount>
decode_genesis_intro_wilderness_pan_frames(
    const std::vector<std::uint8_t>& rom);
// ROM 0x334C field HUD: 0x6A5BE -> $DAC0, 6x2 caps 0x6ACE2, and 6x8
// slots 0x6AA7E at the $F000 plane alias. The native image retains the
// visible 48px right column.
RgbaImage decode_genesis_field_hud(const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_field_hud_active_frame(
    const std::vector<std::uint8_t>& rom);
struct GenesisFieldHudLayers {
    std::array<RgbaImage, 3> faces{};
    std::array<RgbaImage, 3> item_blocks{};
    std::array<RgbaImage, 3> unarmed_item_blocks{};
    // 0x3404 selects each spell row from D4EA0 when +0x56 is negative. The
    // source 0x3602 palette selector yields attribute words 0x2000/0x6000,
    // which are palette lines 1/3 in the extracted host images.
    std::array<std::array<RgbaImage, 2>, native::kNativeMagicSpellCount>
        magic_item_blocks{};
    // Direct value rows for the live +0x4C and +0x9C HUD meters. 0x3674
    // selects one of eleven vertical rows; 0x37EC selects one of eleven
    // horizontal rows from either the primary-runner or companion table.
    std::array<RgbaImage, 11> delay_meter_counters{};
    std::array<std::array<RgbaImage, 11>, 2> behavior_meter_counters{};
    // Starter-record snapshots retained for the NativePort reference path.
    std::array<RgbaImage, 3> vertical_counters{};
    std::array<RgbaImage, 3> horizontal_counters{};
    std::array<RgbaImage, 64> ammo_counters{};
};
// Direct field-HUD assets for every selectable runner portrait and ordinary
// inventory item. Runtime code indexes these named files from typed roster
// state; it does not retain ROM pointers or emulate the 0x3404 upload buffer.
// Face variants correspond to runner-record +0x2C palette lines 1, 2, and 3.
// Ordinary and unarmed item variants preserve the three extracted palette
// lines; the live 0x3602 rule selects line 1 or line 3 for field presentation.
inline constexpr std::size_t kGenesisFieldHudJoshuaPortraitCount = 3U;
inline constexpr std::size_t kGenesisFieldHudRecruitedPortraitCount = 10U;
inline constexpr std::size_t kGenesisFieldHudPortraitFamilyCount =
    kGenesisFieldHudJoshuaPortraitCount +
    kGenesisFieldHudRecruitedPortraitCount;
inline constexpr std::size_t kGenesisFieldHudPortraitPaletteVariantCount = 3U;
inline constexpr std::size_t kGenesisFieldHudItemPaletteVariantCount = 3U;
inline constexpr std::size_t kGenesisFieldHudUnarmedVariantCount = 3U;
struct GenesisFieldHudPartyAssets {
    std::array<std::array<RgbaImage,
                          kGenesisFieldHudPortraitPaletteVariantCount>,
               kGenesisFieldHudPortraitFamilyCount>
        portraits{};
    std::array<std::array<RgbaImage,
                          kGenesisFieldHudItemPaletteVariantCount>,
               native::kNativeInventoryItemCount>
        items{};
    std::array<std::array<RgbaImage,
                          kGenesisFieldHudItemPaletteVariantCount>,
               kGenesisFieldHudUnarmedVariantCount>
        unarmed_items{};
};
// ROM 0x3404 uploads each runner's raw 4x4 portrait and six-tile item block;
// 0x35D4/0x35F0/0x36FC/0x37EC then stamp those resources at the four fixed
// field-HUD destinations. Extraction keeps the layers separate so the native
// shell can select the typed runner record without retaining VRAM state.
GenesisFieldHudLayers decode_genesis_field_hud_layers(
    const std::vector<std::uint8_t>& rom,
    const std::array<GenesisFieldEntityTemplate, 3>& records);
// ROM 0x3404/0x35D4/0x35F0: extract all named portrait, equipped-item, and
// unarmed-item variants as standalone RGBA host assets.
GenesisFieldHudPartyAssets decode_genesis_field_hud_party_assets(
    const std::vector<std::uint8_t>& rom);
// Pocket Secretary bezel from ROM 0x10A7E's four raw 0x1232 name-table
// writes, independent of the field command-screen base.
RgbaImage decode_genesis_pocket_secretary_background(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_pocket_secretary_nuyen_bar(
    const std::vector<std::uint8_t>& rom);
// ROM 0xC3E6 repeats the C1320 space-glyph tile with palette line 3 across
// the 28x21 Pocket Secretary content area. Extract that layer separately.
RgbaImage decode_genesis_pocket_secretary_grid(
    const std::vector<std::uint8_t>& rom);

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

// ROM 0x5DA50 / 0x5D746 pre-title layers. Background is plane B at $E51A
// (SEGA letterforms). Static is plane A at $C400 (1-bit snow). Guy and
// lightning frames are 0x1378 SAT compositions. Logo strobe frames apply
// the 0x5DCE6 CRAM slide used by 0x5D8BA. HSCROLL 0x5DAC4 is applied at
// native render time, not baked into these images.
RgbaImage decode_genesis_boot_background(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_boot_static(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_boot_guy(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, native::kNativeBootSegaPhaseCount>
decode_genesis_boot_sega_frames(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, native::kNativeBootLogoStrobeCount>
decode_genesis_boot_logo_strobe(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_boot_fasa(const std::vector<std::uint8_t>& rom);

// ROM 0x59C5E opening stages. Each image is the settled 0x1406 (or 0x5AA96)
// plane composition; transient vehicle/actor layers are returned separately
// where their source timing requires them.
std::array<RgbaImage, native::kNativeIntroStageCount>
decode_genesis_intro_stages(const std::vector<std::uint8_t>& rom);
// Static plane backgrounds for the four 0x59C5E opening stages. Dynamic actor,
// projectile, and map-hit layers are intentionally excluded from these
// plates and are exposed as individual resources below.
std::array<RgbaImage, native::kNativeIntroStageCount>
decode_genesis_intro_backgrounds(const std::vector<std::uint8_t>& rom);

// Every SAT list selected by the 0x5Axxx ambush update tables, rendered as an
// independent transparent sprite resource. The source table metadata is
// retained so extraction does not turn animation into sampled screenshots.
std::vector<GenesisGraphicsResource>
decode_genesis_intro_sprite_resources(
    const std::vector<std::uint8_t>& rom);

// The four 0x1728 map patches used by the ambush hit/death sequence, kept as
// transparent background overlays rather than baked into full-screen frames.
std::vector<GenesisGraphicsResource>
decode_genesis_intro_ambush_background_resources(
    const std::vector<std::uint8_t>& rom);

// ROM 0x5A1EA's wilderness animation calls the eight 0x1378 sprite update
// routines and 0x5A0B2's four 0x1728 map patches. The returned samples are
// full 320x224 compositions at source frames 0,16,...,496 of that loop.
std::array<RgbaImage, native::kNativeIntroAmbushFrameCount>
decode_genesis_intro_ambush_frames(const std::vector<std::uint8_t>& rom);

// ROM 0x134D2's cab transition uploads the shared map patterns from 0xD2A2E
// to VRAM $6420 and expands the 0xD4298 0x1728 stream into a 26x17 name
// table. The extraction returns that plate cropped to its 208x136 tile
// geometry with no selection outline baked in; the outline is the separate
// 0xD4446 border below and the host owns destination text/cadence.
RgbaImage decode_genesis_cab_map(const std::vector<std::uint8_t>& rom);

struct GenesisCabCursorTiles {
    // 0xD4446 order: top-left, top-middle, top-right, left, right,
    // bottom-left, bottom-middle, bottom-right.
    std::array<RgbaImage, 8> tiles{};
};

// ROM 0x13726 stamps the cab selection outline from the eight-word tile
// list at 0xD4446. Each tile is extracted as its own 8x8 host sprite so the
// border can be composed at any destination's width and height.
GenesisCabCursorTiles decode_genesis_cab_cursor_tiles(
    const std::vector<std::uint8_t>& rom);

// ROM 0xC1A2A: the 20 pointer table entries naming the eight attributes
// followed by the twelve skills used by the Attributes/Skills command page
// (0xD01C). The source renders each through 0xC0DA's encoded text writer.
std::array<std::string, 20> read_genesis_attribute_skill_labels(
    const std::vector<std::uint8_t>& rom);

struct GenesisCyberwareText {
    std::string title;
    std::string empty_message;
    std::array<std::string, 15> entries{};
};

struct GenesisMagicMenuText {
    std::string set_force_and_posture;
    std::string success_label;
    std::string force_label;
    std::string availability_label;
    // ROM 0x102EC passes the complete C89FC block to the text writer. Its
    // 0x80 separators encode Success, Drain, Defense, vs. Phys, and vs. Mana.
    std::string panel_metric_labels;
    // ROM 0x102EC passes C8A3A as one two-column label block: Force, a blank
    // row, then Posture.
    std::string force_posture_labels;
    std::array<std::string, 14> spell_names{};
    std::array<std::string, 14> spell_descriptions{};
};

// ROM 0x11950: the Cyberware page title at 0xD2054 (including its 0x80
// second-line control), the NONE message at 0xD206B, and the fifteen padded
// entry strings at 0xD20AE..0xD2194 indexed by record +0x90 bits 0..14.
GenesisCyberwareText read_genesis_cyberware_text(
    const std::vector<std::uint8_t>& rom);
GenesisMagicMenuText read_genesis_magic_menu_text(
    const std::vector<std::uint8_t>& rom);

// Reconstruct the field command screen's plane-B base from the exact 0x1232
// writers called by ROM 0x10F50. This is host RGBA output, not a Genesis
// name-table or palette dependency retained by the runtime.
RgbaImage decode_genesis_field_menu_background(
    const std::vector<std::uint8_t>& rom);
// ROM 0x58DE0 builds the full-screen elevator selector. The fixed backdrop
// and animated elevator column are separate from the two independently
// updated floor labels so the native runtime can reflect both indices.
RgbaImage decode_genesis_field_elevator_selector_background(
    const std::vector<std::uint8_t>& rom,
    std::size_t elevator_animation_frame);
RgbaImage decode_genesis_field_elevator_selector_floor_layer(
    const std::vector<std::uint8_t>& rom, std::size_t floor_index,
    bool selected_floor_label);
RgbaImage decode_genesis_field_elevator_selector_feedback_layer(
    const std::vector<std::uint8_t>& rom, bool previous_floor_input,
    bool active_phase);
// ROM 0x10BC4 installs the shared field-service screen: the service frame on
// Plane A and its right-side field HUD backing on Plane B. This is an offline
// extraction result; runtime consumers receive RGBA pixels, not VDP state.
RgbaImage decode_genesis_field_service_background(
    const std::vector<std::uint8_t>& rom);

// ROM 0x58B5E/0x58BB6 paint the six-tab Ares selector row. Each host image
// contains the source tab row with one selected category overlay.
std::array<RgbaImage, 6> decode_genesis_field_service_selection_tabs(
    const std::vector<std::uint8_t>& rom);

// ROM 0x586F6 draws the special E offer's 3x2 item art from 0xCE28E using
// field palette line 1 from 0xD29D6/0x18AC56; keep it as a named 24x16 image.
RgbaImage decode_genesis_field_service_universal_clips_icon(
    const std::vector<std::uint8_t>& rom);
// ROM 0x10984 submits the shared dialogue frame as nine named tile-map
// regions over the patterns loaded by 0x10918. The FFF0AF mode flag optionally
// replaces the portrait-side panel region.
RgbaImage decode_genesis_field_dialogue_panel_background(
    const std::vector<std::uint8_t>& rom, bool alternate_portrait_panel);
std::array<RgbaImage, 3> decode_genesis_field_dialogue_option_markers(
    const std::vector<std::uint8_t>& rom);
// ROM 0xFBA6 replaces Plane A with a 34x28 spell-capable panel. The compressed
// pattern upload and 0x1728 tile stream are decoded here during offline
// extraction; the Qt runtime consumes this transparent RGBA layer directly.
RgbaImage decode_genesis_field_magic_panel_background(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 14> decode_genesis_field_magic_spell_icons(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 4> decode_genesis_field_magic_cursor_frames(
    const std::vector<std::uint8_t>& rom);
// ROM 0x107CE writes the selected eight-tile posture row from C8600 at C8A8.
// ROM 0x10172 composes the live current/maximum force meter from these five
// one-tile source rows at C85A2/C85A4/C85A6/C85A8/C85AC.
std::array<RgbaImage, 5> decode_genesis_field_magic_posture_bars(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 5> decode_genesis_field_magic_force_meter_tiles(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 4> decode_genesis_field_magic_availability_bars(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 10> decode_genesis_field_magic_damage_bars(
    const std::vector<std::uint8_t>& rom);
// ROM 0x117A0 redraws the lower action row from the source CD302 name table
// before applying the selected action focus block.
RgbaImage decode_genesis_field_menu_action_background(
    const std::vector<std::uint8_t>& rom);
// ROM 0x11B58 installs CD33E for the Clips selector row.
RgbaImage decode_genesis_field_menu_clips_background(
    const std::vector<std::uint8_t>& rom);
// ROM 0x11E7E installs CD37A for the nested Clips trade row.
RgbaImage decode_genesis_field_menu_clips_trade_background(
    const std::vector<std::uint8_t>& rom);
// ROM 0x127B0 replaces the action row with the source Yes/No bar at CD3B6.
RgbaImage decode_genesis_field_menu_action_confirmation_background(
    const std::vector<std::uint8_t>& rom);
// ROM 0xD01C uses the separate 0x10C96 frame: two upper panels, two side
// rails, and three lower separator rows. It is not the 0x10F50 command base.
RgbaImage decode_genesis_field_menu_attributes_background(
    const std::vector<std::uint8_t>& rom);
RgbaImage decode_genesis_startup_background(
    const std::vector<std::uint8_t>& rom);

RgbaImage decode_genesis_field_menu_inventory_boxes(
    const std::vector<std::uint8_t>& rom);

RgbaImage decode_genesis_field_menu_font(
    const std::vector<std::uint8_t>& rom,
    std::size_t palette_line = 3U);
// ROM 0xFBA6 selects the Magic palette set at C8A60 while 0xC0DA writes
// spell-page text with palette flags zero. Keep that font color separate from
// the command-menu font palette decoded from D29D6/18AC56.
RgbaImage decode_genesis_field_magic_menu_font(
    const std::vector<std::uint8_t>& rom);
constexpr std::size_t kGenesisFieldMagicMetricBarCount = 22U;
// ROM 0x1055E selects thirteen-cell rows from C8784. Extract the rows as
// independent RGBA assets using the Magic palette at C8A60.
std::array<RgbaImage, kGenesisFieldMagicMetricBarCount>
decode_genesis_field_magic_metric_bars(
    const std::vector<std::uint8_t>& rom);

// The clip counter at C4B4 is emitted with C0DA's palette-line zero rather
// than the command text's palette-line three. Keep that source palette
// variant separate so the native renderer does not recolor it heuristically.
// Extract the dynamic command-screen artwork emitted after the static
// 0x10F50 plane setup. These are host RGBA equivalents of the ROM's
// 0x110BC runner portrait and 0x11136/0x11190 item-icon writers.
std::array<RgbaImage, 3> decode_genesis_field_menu_runner_portraits(
    const std::vector<std::uint8_t>& rom);
// ROM 0xC4C4 calls 0x110E8/0x110EC to draw the selected runner's 4x4 face at
// C91E. 0x3404 supplies its current tile words from the live record's +0x50
// table and +0x54 offset, then uploads the referenced tiles from 0x6ACFA.
// Materialize all four +0x2C palette-line variants as direct host images.
std::array<RgbaImage, 4>
decode_genesis_field_menu_dismiss_runner_preview_panels(
    const std::vector<std::uint8_t>& rom,
    const GenesisFieldEntityTemplate& record);
// ROM 0xE660 writes one random two-cell decoration each VBlank. These seven
// named overlays are the three 0xCE126/0xCE12E/0xCE136 pairs and four
// 0xCE13E table pairs after the command-screen pattern upload is resolved.
std::array<RgbaImage, 7>
decode_genesis_field_menu_random_grid_effects(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 2> decode_genesis_field_menu_selected_cells(
    const std::vector<std::uint8_t>& rom);
// Extract the five source focus blocks written by ROM 0x11454. Each block is
// a 2x2 tile update from 0xCD3F2, one block per command tab (Clips through
// Pocket Secretary); it is not a host-generated rectangle.
std::array<RgbaImage, 5> decode_genesis_field_menu_command_focus(
    const std::vector<std::uint8_t>& rom);
// ROM 0x117A4 passes CD41A to 0x11454 for the four inventory-action
// buttons. These source blocks replace the five top-level command tabs while
// Equip/Discard/Trade/Cancel (or First Aid/Discard/Trade/Cancel) is active.
std::array<RgbaImage, 4> decode_genesis_field_menu_action_focus(
    const std::vector<std::uint8_t>& rom);
// ROM 0x127FC writes Yes/No focus blocks from CD47A; Yes is the first block.
std::array<RgbaImage, 2>
decode_genesis_field_menu_action_confirmation_focus(
    const std::vector<std::uint8_t>& rom);
// ROM 0x11B58/0x11E7E pass CD43A/CD45A to the same writer for the Clips
// action rows. These four 2x2 blocks are separate from the command tabs.
std::array<RgbaImage, 4> decode_genesis_field_menu_clips_focus(
    const std::vector<std::uint8_t>& rom);
std::array<RgbaImage, 4> decode_genesis_field_menu_clips_trade_focus(
    const std::vector<std::uint8_t>& rom);
// ROM 0x119CE writes the six-tile posture focus strip from CD514 at CA9E.
RgbaImage decode_genesis_field_menu_posture_focus(
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
// ROM 0x11352 indexes six-tile summary strips through CF884 using the
// selected D4D50 armor record's +0x26 (Wpns) and +0x25 (Guns) bytes.
// Preserve source indexes 0..8 so those bytes select the corresponding row.
std::array<RgbaImage, 9> decode_genesis_field_menu_armor_summary_bars(
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
RgbaImage read_bmp32(const std::string& path);

// Convert the native software framebuffer (0x00RRGGBB) to RGBA8 for dumps.
RgbaImage software_screen_image(const native::SoftwareScreen& screen);

} // namespace shadowrun::graphics
