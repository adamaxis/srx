// Native runtime asset loader and presentation helpers. This file does not
// decode ROM resources or emit extracted assets.

#include "shadowrun/rendering/graphics.hpp"
#include "shadowrun/core/rom_layout.hpp"
#include "shadowrun/rendering/asset_taxonomy.hpp"
#include "shadowrun/rendering/resource_manifest.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <zlib.h>

namespace shadowrun::graphics {

namespace {

void write_u16(std::ostream& output, std::uint16_t value) {
    output.put(static_cast<char>(value & 0xFFU));
    output.put(static_cast<char>((value >> 8U) & 0xFFU));
}

void write_u32(std::ostream& output, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        output.put(static_cast<char>((value >> shift) & 0xFFU));
    }
}


std::uint8_t read_pack_u8(std::istream& input, const char* what) {
    const auto value = input.get();
    if (value == std::char_traits<char>::eof()) {
        throw std::out_of_range(what);
    }
    return static_cast<std::uint8_t>(value);
}

std::uint16_t read_pack_u16(std::istream& input, const char* what) {
    const auto low = read_pack_u8(input, what);
    const auto high = read_pack_u8(input, what);
    return static_cast<std::uint16_t>(low | (high << 8U));
}

std::uint32_t read_pack_u32(std::istream& input, const char* what) {
    const auto b0 = read_pack_u8(input, what);
    const auto b1 = read_pack_u8(input, what);
    const auto b2 = read_pack_u8(input, what);
    const auto b3 = read_pack_u8(input, what);
    return static_cast<std::uint32_t>(b0) |
           (static_cast<std::uint32_t>(b1) << 8U) |
           (static_cast<std::uint32_t>(b2) << 16U) |
           (static_cast<std::uint32_t>(b3) << 24U);
}

void read_pack_bytes(std::istream& input, std::uint8_t* destination,
                     std::size_t size, const char* what) {
    if (size > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
        throw std::out_of_range(what);
    }
    input.read(reinterpret_cast<char*>(destination),
               static_cast<std::streamsize>(size));
    if (!input) {
        throw std::out_of_range(what);
    }
}




std::string read_pack_string(std::istream& input, const char* what) {
    const auto size = static_cast<std::size_t>(read_pack_u16(input, what));
    if (size > 1024U) {
        throw std::runtime_error("native asset pack string is too long");
    }
    std::string value(size, '\0');
    if (size != 0U) {
        input.read(value.data(), static_cast<std::streamsize>(size));
        if (!input) {
            throw std::out_of_range(what);
        }
    }
    return value;
}

struct ResourceManifestEntry {
    std::string alias;
    std::string relative_path;
    std::string kind;
    std::int16_t origin_x = 0;
    std::int16_t origin_y = 0;
};

std::vector<std::string> split_manifest_line(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    for (;;) {
        const auto end = line.find('\t', start);
        fields.push_back(line.substr(
            start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            return fields;
        }
        start = end + 1U;
    }
}

std::vector<ResourceManifestEntry> read_resource_manifest(
    const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("missing direct resource manifest: " +
                                 path.string());
    }
    std::string line;
    if (!std::getline(input, line) ||
        line != "alias\tpath\tkind\torigin_x\torigin_y") {
        throw std::runtime_error("invalid direct resource manifest header: " +
                                 path.string());
    }

    std::vector<ResourceManifestEntry> entries;
    std::unordered_map<std::string, bool> aliases;
    while (std::getline(input, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }
        const auto fields = split_manifest_line(line);
        if (fields.size() != 5U || fields[0].empty() || fields[1].empty() ||
            (fields[2] != "image" && fields[2] != "audio" &&
             fields[2] != "state" && fields[2] != "text")) {
            throw std::runtime_error("invalid direct resource manifest row: " +
                                     line);
        }
        if (!aliases.emplace(fields[0], true).second) {
            throw std::runtime_error("duplicate direct resource alias: " +
                                     fields[0]);
        }
        const std::filesystem::path relative(fields[1]);
        if (relative.is_absolute() || relative.has_root_name() ||
            relative.has_root_directory()) {
            throw std::runtime_error("direct resource path is not relative: " +
                                     fields[1]);
        }
        for (const auto& component : relative) {
            if (component == "..") {
                throw std::runtime_error(
                    "direct resource path escapes the asset directory: " +
                    fields[1]);
            }
        }
        ResourceManifestEntry entry;
        entry.alias = fields[0];
        entry.relative_path = relative.generic_string();
        entry.kind = fields[2];
        try {
            entry.origin_x = static_cast<std::int16_t>(std::stoi(fields[3]));
            entry.origin_y = static_cast<std::int16_t>(std::stoi(fields[4]));
        } catch (const std::exception&) {
            throw std::runtime_error("invalid direct resource origin: " + line);
        }
        entries.push_back(std::move(entry));
    }
    return entries;
}

const ResourceManifestEntry& find_resource_manifest_entry(
    const std::vector<ResourceManifestEntry>& entries, std::string_view alias,
    const char* kind) {
    const auto found = std::find_if(
        entries.begin(), entries.end(),
        [alias, kind](const auto& entry) {
            return entry.alias == alias && entry.kind == kind;
        });
    if (found == entries.end()) {
        throw std::runtime_error("resource manifest is missing " +
                                 std::string(kind) + " alias: " +
                                 std::string(alias));
    }
    return *found;
}


} // namespace

static RgbaImage read_png32(const std::string& path);

static void fill_grayscale_entity_assets(native::NativePortAssets& assets);



// Direct-directory state helpers are defined below the legacy pack reader so
// the old serializer remains easy to compare with its historical format.
static native::NativeFieldRecord read_native_field_record_state(
    std::istream& input);
static native::NativeFieldEntity read_native_field_entity_state(
    std::istream& input, bool has_hud_portrait_fields);
static terrain::Map read_native_terrain_state(std::istream& input);
static native::NativeIntroData read_native_intro_state(std::istream& input);


native::NativePortAssets read_native_asset_directory(const std::string& root) {
    const std::filesystem::path asset_root(root);
    const auto manifest_entries = read_resource_manifest(
        asset_root / std::string(resources::kManifestFile));
    const auto& state_entry = find_resource_manifest_entry(
        manifest_entries, "state", "state");
    const auto state_path = asset_root / state_entry.relative_path;
    std::ifstream input(state_path, std::ios::binary);
    if (!input) {
        throw std::runtime_error(
            "missing direct asset state: " + state_path.string() +
            "; run shadowrun_graphics_extract --all-assets");
    }
    char magic[4]{};
    input.read(magic, sizeof(magic));
    const auto version = read_pack_u16(
        input, "native asset state version is truncated");
    if (!input || std::string(magic, sizeof(magic)) != "SRNS" ||
        (version != 2U && version != 3U && version != 4U && version != 5U)) {
        throw std::runtime_error("invalid direct native asset state header");
    }
    (void)read_pack_u16(input, "native asset state header is truncated");

    native::NativePortAssets assets;
    assets.field_terrain = read_native_terrain_state(input);
    assets.initial_camera_x_fixed = read_pack_u16(
        input, "native asset state camera is truncated");
    assets.initial_camera_y_fixed = read_pack_u16(
        input, "native asset state camera is truncated");
    for (auto& record : assets.initial_field_records) {
        record = read_native_field_record_state(input);
    }
    const auto has_hud_portrait_fields = version >= 4U;
    assets.entity_template = read_native_field_entity_state(
        input, has_hud_portrait_fields);
    for (auto& entity : assets.protagonist_archetype_records) {
        entity = read_native_field_entity_state(input,
                                               has_hud_portrait_fields);
    }
    for (auto& entity : assets.field_entity_records) {
        entity = read_native_field_entity_state(input,
                                               has_hud_portrait_fields);
    }
    if (version >= 3U) {
        const auto roster_count = static_cast<std::size_t>(read_pack_u32(
            input, "native asset state recruited roster count is truncated"));
        if (roster_count > 32U) {
            throw std::runtime_error(
                "native asset state has too many recruited templates");
        }
        assets.recruited_runner_templates.reserve(roster_count);
        for (std::size_t index = 0; index < roster_count; ++index) {
            native::NativeRunnerTemplate candidate;
            candidate.source_index = index;
            candidate.display_name = read_pack_string(
                input, "native asset state recruited runner name is truncated");
            candidate.record = read_native_field_entity_state(
                input, has_hud_portrait_fields);
            assets.recruited_runner_templates.push_back(std::move(candidate));
        }
    }
    for (auto& item : assets.inventory_items) {
        item.code = read_pack_u8(input, "native asset state item is truncated");
        item.source_combat_factor = read_pack_u8(
            input, "native asset state item is truncated");
        item.damage_table_index = read_pack_u8(
            input, "native asset state item is truncated");
        item.power_table_index = read_pack_u8(
            input, "native asset state item is truncated");
        item.name = read_pack_string(input,
                                     "native asset state item is truncated");
        item.category = read_pack_string(input,
                                         "native asset state item is truncated");
    }
    for (auto& value : assets.field_menu_runner_names) {
        value = read_pack_string(input,
                                 "native asset state runner name is truncated");
    }
    for (auto& value : assets.field_menu_status_names) {
        value = read_pack_string(
            input, "native asset state status name is truncated");
    }
    for (auto& value : assets.field_menu_power_labels) {
        value = read_pack_string(input,
                                 "native asset state power label is truncated");
    }
    for (auto& armor : assets.field_menu_armor_labels) {
        for (auto& value : armor) {
            value = read_pack_string(
                input, "native asset state armor label is truncated");
        }
    }
    if (version >= 5U) {
        for (auto& modifiers : assets.field_menu_armor_summary_modifiers) {
            modifiers.weapons = read_pack_u8(
                input, "native asset state armor modifiers are truncated");
            modifiers.guns = read_pack_u8(
                input, "native asset state armor modifiers are truncated");
        }
    }
    for (auto& value : assets.cyberware_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.attribute_skill_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.group_item_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.cyberdeck_unavailable_lines) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.dismiss_runner_labels) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_labels) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_no_run_lines) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_event_gates) {
        value = read_pack_u8(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_tip_text) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_tip_flags) {
        value = read_pack_u32(input, "native asset state is truncated");
    }
    for (auto& value : assets.notebook_mr_johnson_names) {
        value = read_pack_string(input, "native asset state is truncated");
    }
    for (auto& direction : assets.field_direction_table) {
        direction[0] = static_cast<std::int16_t>(read_pack_u16(
            input, "native asset state direction table is truncated"));
        direction[1] = static_cast<std::int16_t>(read_pack_u16(
            input, "native asset state direction table is truncated"));
    }
    assets.intro = read_native_intro_state(input);
    for (auto& mode : assets.field_event_mode_table) {
        mode = read_pack_u8(input, "native asset state event table is truncated");
    }
    const auto area_count = static_cast<std::size_t>(read_pack_u16(
        input, "native asset state area count is truncated"));
    if (area_count > 56U) {
        throw std::runtime_error("native asset state has too many field areas");
    }
    assets.field_areas.reserve(area_count);
    for (std::size_t index = 0; index < area_count; ++index) {
        native::NativeFieldArea area;
        area.mode_id = read_pack_u16(
            input, "native asset state area mode is truncated");
        area.terrain = read_native_terrain_state(input);
        area.initial_camera_x_fixed = read_pack_u16(
            input, "native asset state area camera is truncated");
        area.initial_camera_y_fixed = read_pack_u16(
            input, "native asset state area camera is truncated");
        for (auto& record : area.initial_field_records) {
            record = read_native_field_record_state(input);
        }
        for (auto& mode : area.event_mode_table) {
            mode = read_pack_u8(input, "native asset state area event table is truncated");
        }
        assets.field_areas.push_back(std::move(area));
    }
    for (auto& animation : assets.field_entity_animations) {
        animation.state_selector = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.state_marker = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.state_auxiliary_word = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.source_timing_value = read_pack_u16(
            input, "native asset state animation is truncated");
        animation.timing_multiplier = read_pack_u16(
            input, "native asset state animation is truncated");
        const auto frame_count = static_cast<std::size_t>(read_pack_u32(
            input, "native asset state animation frame count is truncated"));
        if (frame_count > 256U) {
            throw std::runtime_error(
                "native asset state has too many animation frames");
        }
        animation.frames.resize(frame_count);
    }
    const auto entity_count = static_cast<std::size_t>(read_pack_u32(
        input, "native asset state entity frame count is truncated"));
    if (entity_count > 256U) {
        throw std::runtime_error("native asset state has too many entity frames");
    }
    assets.entity_frames.resize(entity_count);

    // The manifest is the complete direct asset list. Images are decoded into
    // native pixels; audio remains a Windows-playable file path so playback
    // can use a host audio API without carrying the Genesis driver into the
    // runtime.
    for (const auto& entry : manifest_entries) {
        if (entry.kind == "state") {
            continue;
        }
        if (entry.kind == "text") {
            // Text/data records are direct resources for the conversion-side
            // catalog. The legacy NativePortAssets structure has no text
            // store, so validate their manifest path here and leave loading
            // to the typed consumer that owns the record.
            if (!std::filesystem::is_regular_file(
                    asset_root / entry.relative_path)) {
                throw std::runtime_error("cannot load direct text resource " +
                                         (asset_root / entry.relative_path).string());
            }
            continue;
        }
        const auto path = asset_root / entry.relative_path;
        if (entry.kind == "audio") {
            const auto extension = path.extension();
            if (!std::filesystem::is_regular_file(path) ||
                (extension != ".flac" && extension != ".wav")) {
                throw std::runtime_error(
                    "cannot load direct audio resource " + path.string() +
                    ": expected a regular .flac or .wav file");
            }
            if (!assets.audio_resources.emplace(entry.alias, path).second) {
                throw std::runtime_error("duplicate loaded audio alias: " +
                                         entry.alias);
            }
            continue;
        }
        if (entry.kind != "image") {
            throw std::runtime_error("unsupported direct resource kind: " +
                                     entry.kind);
        }
        try {
            const auto decoded = path.extension() == ".png"
                ? read_png32(path.string())
                : read_bmp32(path.string());
            native::NativeImage image{
                decoded.width, decoded.height, decoded.pixels,
                entry.origin_x, entry.origin_y};
            if (!assets.resources.emplace(entry.alias, std::move(image)).second) {
                throw std::runtime_error("duplicate loaded resource alias: " +
                                         entry.alias);
            }
        } catch (const std::exception& error) {
            throw std::runtime_error("cannot load direct resource " +
                                     path.string() + ": " + error.what());
        }
    }
    const auto loop_points_path = asset_root / "audio" / "loop-points.tsv";
    if (std::filesystem::is_regular_file(loop_points_path)) {
        std::ifstream loop_input(loop_points_path, std::ios::binary);
        std::string loop_line;
        if (!std::getline(loop_input, loop_line) ||
            loop_line !=
                "alias\tloop_start_frame\tloop_end_frame\tsample_rate\tsource_start_ticks\tsource_end_ticks") {
            throw std::runtime_error("invalid direct audio loop-point header: " +
                                     loop_points_path.string());
        }
        while (std::getline(loop_input, loop_line)) {
            if (loop_line.empty() || loop_line.front() == '#') {
                continue;
            }
            const auto fields = split_manifest_line(loop_line);
            if (fields.size() != 6U || fields[0].empty()) {
                throw std::runtime_error(
                    "invalid direct audio loop-point row: " + loop_line);
            }
            if (assets.audio_resources.find(fields[0]) ==
                assets.audio_resources.end()) {
                throw std::runtime_error(
                    "audio loop-point alias is not an audio resource: " +
                    fields[0]);
            }
            try {
                const native::NativeAudioLoop loop{
                    std::stoull(fields[1]), std::stoull(fields[2]),
                    static_cast<std::uint32_t>(std::stoul(fields[3]))};
                (void)std::stoull(fields[4]);
                (void)std::stoull(fields[5]);
                if (loop.end_frame <= loop.start_frame ||
                    loop.sample_rate == 0U ||
                    !assets.audio_loop_points.emplace(fields[0], loop).second) {
                    throw std::runtime_error(
                        "audio loop-point range is invalid or duplicated");
                }
            } catch (const std::exception& error) {
                throw std::runtime_error(
                    "invalid direct audio loop-point row: " + loop_line +
                    ": " + error.what());
            }
        }
    }
    const auto load = [&assets](const std::string& relative) {
        try {
            return assets[resources::alias_for_path(relative)];
        } catch (const std::exception& error) {
            throw std::runtime_error("cannot resolve direct resource " +
                                     relative + ": " + error.what());
        }
    };
    assets.title_scene = load("title/background.bmp");
    assets.title_background = assets.title_scene;
    for (std::size_t index = 0; index < assets.title_circuit_lights.size(); ++index) {
        assets.title_circuit_lights[index] =
            load("title/circuit-background-" + std::to_string(index) + ".bmp");
    }
    assets.title_logo_plane = load("title/logo.bmp");
    assets.title_text = load("title/text.bmp");
    for (std::size_t index = 0; index < assets.title_sprite_frames.size(); ++index) {
        assets.title_sprite_frames[index] =
            load("title/sprite-" + std::to_string(index) + ".bmp");
        assets.title_animation_frames[index] =
            load("title/animation-" + std::to_string(index) + ".bmp");
    }
    assets.boot_static = load("boot/snow.bmp");
    assets.boot_background = load("boot/sega-background.bmp");
    assets.boot_guy = load("boot/guy.bmp");
    for (std::size_t index = 0; index < assets.boot_sega_frames.size(); ++index) {
        assets.boot_sega_frames[index] =
            load("boot/sega-lightning-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.boot_logo_strobe.size(); ++index) {
        assets.boot_logo_strobe[index] =
            load("boot/sega-strobe-" + std::to_string(index) + ".bmp");
    }
    assets.boot_fasa = load("boot/fasa.bmp");
    assets.startup_background = load("menu/startup/background.bmp");
    assets.startup_font = load("menu/startup/font.bmp");
    assets.startup_cursor = load("menu/startup/cursor.bmp");
    for (std::size_t index = 0; index < assets.intro_stages.size(); ++index) {
        assets.intro_stages[index] =
            load("intro/background-" + std::to_string(index) + ".bmp");
    }
    for (const auto& entry : manifest_entries) {
        if (entry.kind != "image" ||
            entry.relative_path.rfind("intro/sprites/", 0) != 0) {
            continue;
        }
        const auto stem = std::filesystem::path(entry.relative_path)
                              .stem().string();
        const auto last_dash = stem.rfind('-');
        const auto previous_dash = last_dash == std::string::npos
            ? std::string::npos : stem.rfind('-', last_dash - 1U);
        if (last_dash == std::string::npos ||
            previous_dash == std::string::npos) {
            throw std::runtime_error("invalid intro sprite manifest path: " +
                                     entry.relative_path);
        }
        native::NativeIntroSprite sprite;
        sprite.role = stem.substr(0, previous_dash);
        try {
            sprite.table_index = std::stoull(
                stem.substr(previous_dash + 1U,
                            last_dash - previous_dash - 1U));
            sprite.sprite_index = std::stoull(stem.substr(last_dash + 1U));
        } catch (const std::exception&) {
            throw std::runtime_error("invalid intro sprite manifest path: " +
                                     entry.relative_path);
        }
        auto image = assets[entry.alias];
        sprite.draw_offset_x = static_cast<std::int32_t>(image.origin_x) -
                               0x100;
        sprite.draw_offset_y = static_cast<std::int32_t>(image.origin_y) -
                               0x100;
        sprite.image = std::move(image);
        assets.intro_sprites.push_back(std::move(sprite));
    }
    for (std::size_t index = 0; index < assets.intro_background_patches.size();
         ++index) {
        assets.intro_background_patches[index] = load(
            "intro/background-patch-" + std::to_string(index) + ".bmp");
    }
    assets.intro_aircraft_plane_a = load("intro/aircraft-plane-a.bmp");
    assets.intro_trivid_background = load("intro/trivid-background.bmp");
    assets.cab_map = load("intro/cab-map.bmp");
    constexpr std::array<std::string_view, 8> kCabCursorTilePaths{{
        "intro/cab/cursor-top-left.bmp",
        "intro/cab/cursor-top-middle.bmp",
        "intro/cab/cursor-top-right.bmp",
        "intro/cab/cursor-left.bmp",
        "intro/cab/cursor-right.bmp",
        "intro/cab/cursor-bottom-left.bmp",
        "intro/cab/cursor-bottom-middle.bmp",
        "intro/cab/cursor-bottom-right.bmp",
    }};
    for (std::size_t index = 0; index < kCabCursorTilePaths.size(); ++index) {
        assets.cab_cursor_tiles[index] =
            load(std::string(kCabCursorTilePaths[index]));
    }

    assets.field_hud = load("field/hud/background.bmp");
    for (std::size_t index = 0; index < assets.field_hud_faces.size(); ++index) {
        assets.field_hud_faces[index] = load(
            "field/hud/face-" + std::to_string(index) + ".bmp");
        assets.field_hud_item_blocks[index] = load(
            "field/hud/item-" + std::to_string(index) + ".bmp");
        assets.field_hud_unarmed_item_blocks[index] = load(
            "field/hud/unarmed-item-" + std::to_string(index) + ".bmp");
        assets.field_hud_vertical_counters[index] = load(
            "field/hud/vertical-counter-" + std::to_string(index) + ".bmp");
        assets.field_hud_horizontal_counters[index] = load(
            "field/hud/horizontal-counter-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t spell = 0;
         spell < assets.field_hud_magic_item_blocks.size(); ++spell) {
        for (std::size_t palette = 0;
             palette < assets.field_hud_magic_item_blocks[spell].size();
             ++palette) {
            assets.field_hud_magic_item_blocks[spell][palette] = load(
                "field/hud/magic-item-" + std::to_string(spell) +
                "-palette-" + std::to_string(palette) + ".bmp");
        }
    }
    for (std::size_t index = 0; index < assets.field_hud_ammo_counters.size(); ++index) {
        assets.field_hud_ammo_counters[index] = load(
            "field/hud/ammo-counter-" + std::to_string(index) + ".bmp");
    }
    assets.pocket_secretary_background =
        load("menu/pocket-secretary/background.bmp");
    assets.field_menu_background = load("menu/field/background.bmp");
    assets.field_menu_action_background =
        load("menu/field/action-background.bmp");
    assets.field_menu_attributes_background =
        load("menu/field/attributes-background.bmp");
    assets.field_menu_inventory_boxes = load("menu/field/inventory-boxes.bmp");
    assets.field_menu_font = load("menu/field/font.bmp");
    for (std::size_t index = 0; index < assets.field_menu_runner_portraits.size(); ++index) {
        assets.field_menu_runner_portraits[index] = load(
            "menu/field/runner-portrait-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_selected_cells.size(); ++index) {
        assets.field_menu_selected_cells[index] = load(
            "menu/field/selected-cell-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_command_focus.size(); ++index) {
        assets.field_menu_command_focus[index] = load(
            "menu/field/command-focus-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_action_focus.size(); ++index) {
        assets.field_menu_action_focus[index] = load(
            "menu/field/action-focus-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_clips_focus.size(); ++index) {
        assets.field_menu_clips_focus[index] = load(
            "menu/field/clips-focus-" + std::to_string(index) + ".bmp");
        assets.field_menu_clips_trade_focus[index] = load(
            "menu/field/clips-trade-focus-" + std::to_string(index) + ".bmp");
    }
    assets.field_menu_posture_focus = load("menu/field/posture-focus.bmp");
    assets.field_menu_cursor = load("menu/field/cursor.bmp");
    for (std::size_t index = 0; index < assets.field_menu_item_art.size(); ++index) {
        assets.field_menu_item_art[index] = load(
            resources::item_detail_asset_path(index + 1U));
        for (std::size_t variant = 0;
             variant < assets.field_menu_inventory_icons[index].size(); ++variant) {
            assets.field_menu_inventory_icons[index][variant] = load(
                resources::item_icon_asset_path(index + 1U, variant));
        }
    }
    for (std::size_t index = 0; index < assets.field_menu_status_bars.size(); ++index) {
        assets.field_menu_status_bars[index] = load(
            "menu/field/status-bar-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_damage_bars.size(); ++index) {
        assets.field_menu_damage_bars[index] = load(
            "menu/field/damage-bar-" + std::to_string(index) + ".bmp");
        assets.field_menu_drain_bars[index] = load(
            "menu/field/drain-bar-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0;
         index < assets.field_menu_armor_summary_bars.size(); ++index) {
        assets.field_menu_armor_summary_bars[index] = load(
            "menu/field/armor-summary-bar-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_detail_values.size(); ++index) {
        assets.field_menu_detail_values[index] = load(
            "menu/field/detail-value-" + std::to_string(index) + ".bmp");
    }
    for (std::size_t index = 0; index < assets.field_menu_attack_bars.size(); ++index) {
        assets.field_menu_attack_bars[index] = load(
            "menu/field/attack-bar-" + std::to_string(index) + ".bmp");
        assets.field_menu_defense_bars[index] = load(
            "menu/field/defense-bar-" + std::to_string(index) + ".bmp");
    }

    assets.field_map = load("field/maps/mode-2.bmp");
    assets.field_map_priority_overlay =
        load("field/maps/mode-2-priority-overlay.png");
    for (auto& area : assets.field_areas) {
        area.map = load("field/maps/mode-" + std::to_string(area.mode_id) + ".bmp");
        area.priority_overlay = load(
            "field/maps/mode-" + std::to_string(area.mode_id) +
            "-priority-overlay.png");
    }
    for (std::size_t index = 0; index < assets.entity_frames.size(); ++index) {
        const auto image = load("field/entities/idle-" + std::to_string(index) + ".bmp");
        assets.entity_frames[index] = {image.width, image.height, image.pixels};
    }
    for (std::size_t direction = 0;
         direction < assets.field_entity_animations.size(); ++direction) {
        auto& animation = assets.field_entity_animations[direction];
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            const auto image = load("field/entities/walk-" +
                                    std::to_string(direction) + "-" +
                                    std::to_string(frame) + ".bmp");
            animation.frames[frame] = {image.width, image.height, image.pixels};
        }
    }
    fill_grayscale_entity_assets(assets);
    return assets;
}


static RgbaImage make_grayscale_image_impl(const RgbaImage& color) {
    RgbaImage gray = color;
    for (auto& pixel : gray.pixels) {
        const auto luminance = static_cast<std::uint8_t>(
            (static_cast<unsigned>(pixel.red) * 77U +
             static_cast<unsigned>(pixel.green) * 150U +
             static_cast<unsigned>(pixel.blue) * 29U) /
            256U);
        pixel.red = luminance;
        pixel.green = luminance;
        pixel.blue = luminance;
    }
    return gray;
}

RgbaImage make_grayscale_image(const RgbaImage& color) {
    return make_grayscale_image_impl(color);
}

static native::NativeEntityFrame grayscale_entity_frame(
    const native::NativeEntityFrame& color) {
    const RgbaImage gray = make_grayscale_image_impl(
        {color.width, color.height, color.pixels});
    return {gray.width, gray.height, gray.pixels};
}

static void fill_grayscale_entity_assets(native::NativePortAssets& assets) {
    assets.entity_frames_gray.clear();
    assets.entity_frames_gray.reserve(assets.entity_frames.size());
    for (const auto& frame : assets.entity_frames) {
        assets.entity_frames_gray.push_back(grayscale_entity_frame(frame));
    }
    for (std::size_t index = 0; index < assets.field_entity_animations.size();
         ++index) {
        const auto& color = assets.field_entity_animations[index];
        auto& gray = assets.field_entity_animations_gray[index];
        gray = color;
        gray.frames.clear();
        gray.frames.reserve(color.frames.size());
        for (const auto& frame : color.frames) {
            gray.frames.push_back(grayscale_entity_frame(frame));
        }
    }
}



static native::NativeFieldRecord read_native_field_record_state(
    std::istream& input) {
    native::NativeFieldRecord record;
    record.position.world_x_fixed = read_pack_u16(
        input, "native field record state is truncated");
    record.position.world_y_fixed = read_pack_u16(
        input, "native field record state is truncated");
    record.record_type = read_pack_u16(
        input, "native field record state is truncated");
    record.flags_10 = read_pack_u8(
        input, "native field record state is truncated");
    record.flags_11 = read_pack_u8(
        input, "native field record state is truncated");
    return record;
}


static native::NativeFieldEntity read_native_field_entity_state(
    std::istream& input, const bool has_hud_portrait_fields) {
    native::NativeFieldEntity entity;
    entity.record_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.world_x_fixed = read_pack_u16(
        input, "native field entity state is truncated");
    entity.world_y_fixed = read_pack_u16(
        input, "native field entity state is truncated");
    entity.projection_word_08 = read_pack_u16(
        input, "native field entity state is truncated");
    entity.kind = read_pack_u16(input, "native field entity state is truncated");
    entity.record_type = read_pack_u16(
        input, "native field entity state is truncated");
    entity.race_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.archetype_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.physical_value = read_pack_u8(
        input, "native field entity state is truncated");
    entity.mental_value = read_pack_u8(
        input, "native field entity state is truncated");
    entity.active_item_code = read_pack_u8(
        input, "native field entity state is truncated");
    entity.flags_10 = read_pack_u8(
        input, "native field entity state is truncated");
    entity.flags_11 = read_pack_u8(
        input, "native field entity state is truncated");
    entity.state_pointer = read_pack_u32(
        input, "native field entity state is truncated");
    entity.vdp_screen_x = read_pack_u16(
        input, "native field entity state is truncated");
    entity.vdp_screen_y = read_pack_u16(
        input, "native field entity state is truncated");
    entity.screen_x = read_pack_u16(
        input, "native field entity state is truncated");
    entity.screen_y = read_pack_u16(
        input, "native field entity state is truncated");
    entity.visible = read_pack_u8(
        input, "native field entity state is truncated") != 0U;
    entity.frame_index = read_pack_u32(
        input, "native field entity state is truncated");
    entity.state_selector = read_pack_u16(
        input, "native field entity state is truncated");
    entity.state_index = read_pack_u16(
        input, "native field entity state is truncated");
    entity.display_state = static_cast<std::int8_t>(read_pack_u8(
        input, "native field entity state is truncated"));
    entity.display_substate = static_cast<std::int8_t>(read_pack_u8(
        input, "native field entity state is truncated"));
    entity.display_palette_state = read_pack_u8(
        input, "native field entity state is truncated");
    entity.menu_status_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.attack_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.defense_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    for (auto& index : entity.attack_bar_indices_by_posture) {
        index = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& index : entity.defense_bar_indices_by_posture) {
        index = read_pack_u8(input, "native field entity state is truncated");
    }
    entity.weapons_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.guns_bar_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.animation_accumulator = read_pack_u16(
        input, "native field entity state is truncated");
    entity.animation_multiplier = read_pack_u16(
        input, "native field entity state is truncated");
    entity.animation_threshold = read_pack_u16(
        input, "native field entity state is truncated");
    entity.animation_descriptor = read_pack_u32(
        input, "native field entity state is truncated");
    for (auto& code : entity.inventory_item_codes) {
        code = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& phase : entity.inventory_phase_bytes) {
        phase = read_pack_u8(input, "native field entity state is truncated");
    }
    entity.equipped_armor_index = read_pack_u8(
        input, "native field entity state is truncated");
    entity.ammo_rounds = read_pack_u8(
        input, "native field entity state is truncated");
    entity.ammo_clips = read_pack_u8(
        input, "native field entity state is truncated");
    entity.cyberware_flags = read_pack_u16(
        input, "native field entity state is truncated");
    for (auto& value : entity.attribute_values) {
        value = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& value : entity.skill_values) {
        value = read_pack_u8(input, "native field entity state is truncated");
    }
    for (auto& value : entity.attribute_skill_auxiliary) {
        value = read_pack_u8(input, "native field entity state is truncated");
    }
    if (has_hud_portrait_fields) {
        entity.command_overview_panel_parameter = read_pack_u16(
            input, "native field entity HUD panel state is truncated");
        entity.hud_portrait_table_pointer = read_pack_u32(
            input, "native field entity portrait table state is truncated");
        entity.hud_portrait_table_offset = read_pack_u16(
            input, "native field entity portrait offset state is truncated");
    }
    return entity;
}


static terrain::Map read_native_terrain_state(std::istream& input) {
    terrain::Map map;
    map.x_exclusive_upper = read_pack_u16(
        input, "native terrain state is truncated");
    map.y_exclusive_upper = read_pack_u16(
        input, "native terrain state is truncated");
    map.x_inclusive_lower = read_pack_u16(
        input, "native terrain state is truncated");
    map.y_inclusive_lower = read_pack_u16(
        input, "native terrain state is truncated");
    map.region_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain state is truncated"));
    if (map.region_count > 65536U) {
        throw std::runtime_error("native terrain state has too many regions");
    }
    map.regions.resize(map.region_count);
    for (auto& region : map.regions) {
        region.x_start = read_pack_u16(input, "native terrain state is truncated");
        region.x_end = read_pack_u16(input, "native terrain state is truncated");
        region.y_start = read_pack_u16(input, "native terrain state is truncated");
        region.y_end = read_pack_u16(input, "native terrain state is truncated");
        region.status = read_pack_u8(input, "native terrain state is truncated");
    }
    const auto status_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain state is truncated"));
    if (status_count != terrain::Map::kWidth * terrain::Map::kHeight) {
        throw std::runtime_error("native terrain state has an invalid grid");
    }
    map.statuses.resize(status_count);
    read_pack_bytes(input, map.statuses.data(), status_count,
                    "native terrain state is truncated");
    return map;
}


static native::NativeIntroData read_native_intro_state(std::istream& input) {
    native::NativeIntroData intro;
    intro.start_options_title = read_pack_string(
        input, "native state startup title is truncated");
    for (auto& value : intro.start_options) {
        value = read_pack_string(input, "native state startup option is truncated");
    }
    intro.archetype_title = read_pack_string(
        input, "native state archetype title is truncated");
    for (auto& value : intro.archetypes) {
        value = read_pack_string(input, "native state archetype is truncated");
    }
    const auto story_count = static_cast<std::size_t>(read_pack_u32(
        input, "native state story count is truncated"));
    if (story_count > 4096U) {
        throw std::runtime_error("native state has too many story lines");
    }
    intro.story_lines.reserve(story_count);
    for (std::size_t index = 0; index < story_count; ++index) {
        intro.story_lines.push_back(read_pack_string(
            input, "native state story line is truncated"));
    }
    const auto read_layout = [&input]() {
        const auto count = static_cast<std::size_t>(read_pack_u32(
            input, "native state layout count is truncated"));
        if (count > 1024U) {
            throw std::runtime_error("native state has too many layout lines");
        }
        std::vector<native::NativeStartupTextLine> layout;
        layout.reserve(count);
        for (std::size_t index = 0; index < count; ++index) {
            native::NativeStartupTextLine line;
            line.row = read_pack_u8(input, "native state layout is truncated");
            line.text = read_pack_string(input,
                                         "native state layout is truncated");
            layout.push_back(std::move(line));
        }
        return layout;
    };
    intro.start_options_layout = read_layout();
    intro.save_load_layout = read_layout();
    intro.archetype_layout = read_layout();
    intro.no_save_layout = read_layout();
    intro.save_success_layout = read_layout();
    return intro;
}


RgbaImage software_screen_image(const native::SoftwareScreen& screen) {
    RgbaImage image{
        screen.width(), screen.height(),
        std::vector<native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

void write_bmp32(const std::string& path, const RgbaImage& image) {
    if (image.width == 0 || image.height == 0 ||
        image.pixels.size() != static_cast<std::size_t>(image.width) *
                                    image.height) {
        throw std::invalid_argument("invalid RGBA image dimensions");
    }
    const auto row_bytes = static_cast<std::uint32_t>(image.width) * 4U;
    const auto pixel_bytes = row_bytes * image.height;
    // BITMAPV4HEADER (108 bytes). Qt's BMP loader ignores the fourth pixel
    // byte of a plain 40-byte 32bpp BMP and rejects the 56-byte V3 form; it
    // honors the alpha mask carried by the V4 extension. The native reader
    // reads the fourth byte directly and skips the extension fields.
    const auto file_offset = 14U + 108U;
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot open BMP output: " + path);
    }
    output.put('B');
    output.put('M');
    write_u32(output, file_offset + pixel_bytes);
    write_u16(output, 0);
    write_u16(output, 0);
    write_u32(output, file_offset);
    write_u32(output, 108);
    write_u32(output, image.width);
    write_u32(output, image.height);
    write_u16(output, 1);
    write_u16(output, 32);
    write_u32(output, 0);
    write_u32(output, pixel_bytes);
    write_u32(output, 2835);
    write_u32(output, 2835);
    write_u32(output, 0);
    write_u32(output, 0);
    write_u32(output, 0x00FF0000U); // red mask
    write_u32(output, 0x0000FF00U); // green mask
    write_u32(output, 0x000000FFU); // blue mask
    write_u32(output, 0xFF000000U); // alpha mask
    output.write("RGB ", 4);         // CSType
    for (unsigned extra = 0; extra < 12U; ++extra) {
        write_u32(output, 0);        // endpoints, gamma, and profile fields
    }

    for (std::size_t row = image.height; row-- > 0;) {
        for (std::size_t column = 0; column < image.width; ++column) {
            const auto pixel = image.pixels[row * image.width + column];
            output.put(static_cast<char>(pixel.blue));
            output.put(static_cast<char>(pixel.green));
            output.put(static_cast<char>(pixel.red));
            output.put(static_cast<char>(pixel.alpha));
        }
    }
    if (!output) {
        throw std::runtime_error("cannot write BMP output");
    }
}

RgbaImage read_bmp32(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open BMP input");
    }
    const auto read_u16 = [&input](const char* what) {
        const auto b0 = input.get();
        const auto b1 = input.get();
        if (b0 == std::char_traits<char>::eof() ||
            b1 == std::char_traits<char>::eof()) {
            throw std::out_of_range(what);
        }
        return static_cast<std::uint16_t>(
            static_cast<std::uint8_t>(b0) |
            (static_cast<std::uint16_t>(static_cast<std::uint8_t>(b1)) << 8U));
    };
    const auto read_u32 = [&input](const char* what) {
        std::uint32_t value = 0;
        for (unsigned shift = 0; shift < 32; shift += 8) {
            const auto byte = input.get();
            if (byte == std::char_traits<char>::eof()) {
                throw std::out_of_range(what);
            }
            value |= static_cast<std::uint32_t>(
                         static_cast<std::uint8_t>(byte))
                     << shift;
        }
        return value;
    };
    if (input.get() != 'B' || input.get() != 'M') {
        throw std::runtime_error("invalid BMP signature");
    }
    (void)read_u32("BMP header is truncated");
    (void)read_u16("BMP header is truncated");
    (void)read_u16("BMP header is truncated");
    const auto pixel_offset = read_u32("BMP header is truncated");
    const auto header_size = read_u32("BMP info header is truncated");
    if (header_size < 40U) {
        throw std::runtime_error("unsupported BMP info header");
    }
    const auto width = read_u32("BMP dimensions are truncated");
    const auto raw_height = read_u32("BMP dimensions are truncated");
    const auto planes = read_u16("BMP format is truncated");
    const auto bits = read_u16("BMP format is truncated");
    const auto compression = read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    (void)read_u32("BMP format is truncated");
    if (width == 0U || raw_height == 0U || width > 8192U ||
        raw_height > 8192U || planes != 1U || bits != 32U ||
        (compression != 0U && compression != 3U)) {
        throw std::runtime_error("unsupported BMP format");
    }
    if (header_size > 40U) {
        input.seekg(static_cast<std::streamoff>(header_size - 40U),
                    std::ios::cur);
    }
    if (!input) {
        throw std::out_of_range("BMP info header is truncated");
    }
    const auto height = static_cast<std::size_t>(raw_height);
    const auto row_bytes = static_cast<std::size_t>(width) * 4U;
    if (height > std::numeric_limits<std::size_t>::max() / row_bytes) {
        throw std::overflow_error("BMP dimensions overflow host size");
    }
    input.seekg(static_cast<std::streamoff>(pixel_offset), std::ios::beg);
    if (!input) {
        throw std::out_of_range("BMP pixel offset is outside the file");
    }
    RgbaImage image{static_cast<std::uint16_t>(width),
                    static_cast<std::uint16_t>(height),
                    std::vector<native::Rgba8>(
                        static_cast<std::size_t>(width) * height)};
    for (std::size_t source_row = height; source_row-- > 0;) {
        for (std::size_t column = 0; column < width; ++column) {
            const auto blue = input.get();
            const auto green = input.get();
            const auto red = input.get();
            const auto alpha = input.get();
            if (blue == std::char_traits<char>::eof() ||
                green == std::char_traits<char>::eof() ||
                red == std::char_traits<char>::eof() ||
                alpha == std::char_traits<char>::eof()) {
                throw std::out_of_range("BMP pixel data is truncated");
            }
            image.pixels[source_row * width + column] = {
                static_cast<std::uint8_t>(red),
                static_cast<std::uint8_t>(green),
                static_cast<std::uint8_t>(blue),
                static_cast<std::uint8_t>(alpha),
            };
        }
    }
    return image;
}

static RgbaImage read_png32(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open PNG input");
    }
    std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    constexpr std::array<std::uint8_t, 8> signature{
        0x89U, 'P', 'N', 'G', 0x0DU, 0x0AU, 0x1AU, 0x0AU};
    if (bytes.size() < signature.size() ||
        !std::equal(signature.begin(), signature.end(), bytes.begin())) {
        throw std::runtime_error("invalid PNG signature");
    }

    const auto read_big_endian = [&bytes](const std::size_t offset) {
        if (offset > bytes.size() || bytes.size() - offset < 4U) {
            throw std::out_of_range("PNG chunk header is truncated");
        }
        return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
               (static_cast<std::uint32_t>(bytes[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(bytes[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(bytes[offset + 3U]);
    };
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    std::vector<std::uint8_t> compressed;
    bool saw_header = false;
    bool saw_end = false;
    std::size_t cursor = signature.size();
    while (cursor < bytes.size()) {
        const auto length = static_cast<std::size_t>(read_big_endian(cursor));
        cursor += 4U;
        if (cursor > bytes.size() || bytes.size() - cursor < 4U ||
            length > bytes.size() - cursor - 4U ||
            bytes.size() - cursor - 4U - length < 4U) {
            throw std::out_of_range("PNG chunk is truncated");
        }
        const auto type_offset = cursor;
        const auto data_offset = cursor + 4U;
        const auto crc_offset = data_offset + length;
        const auto expected_crc = read_big_endian(crc_offset);
        const auto actual_crc = crc32(
            crc32(0L, Z_NULL, 0), bytes.data() + type_offset,
            static_cast<uInt>(4U + length));
        if (static_cast<std::uint32_t>(actual_crc) != expected_crc) {
            throw std::runtime_error("PNG chunk checksum mismatch");
        }

        const auto is_type = [&bytes, type_offset](const char* type) {
            return bytes[type_offset] == static_cast<std::uint8_t>(type[0]) &&
                   bytes[type_offset + 1U] == static_cast<std::uint8_t>(type[1]) &&
                   bytes[type_offset + 2U] == static_cast<std::uint8_t>(type[2]) &&
                   bytes[type_offset + 3U] == static_cast<std::uint8_t>(type[3]);
        };
        if (is_type("IHDR")) {
            if (saw_header || length != 13U) {
                throw std::runtime_error("invalid PNG header chunk");
            }
            width = read_big_endian(data_offset);
            height = read_big_endian(data_offset + 4U);
            if (width == 0U || height == 0U || width > 8192U ||
                height > 8192U || bytes[data_offset + 8U] != 8U ||
                bytes[data_offset + 9U] != 6U ||
                bytes[data_offset + 10U] != 0U ||
                bytes[data_offset + 11U] != 0U ||
                bytes[data_offset + 12U] != 0U) {
                throw std::runtime_error(
                    "unsupported PNG (expected non-interlaced RGBA8)");
            }
            saw_header = true;
        } else if (is_type("IDAT")) {
            if (!saw_header || saw_end ||
                length > std::numeric_limits<std::size_t>::max() -
                             compressed.size()) {
                throw std::runtime_error("invalid PNG image-data chunk");
            }
            compressed.insert(compressed.end(), bytes.begin() + data_offset,
                              bytes.begin() + data_offset + length);
        } else if (is_type("IEND")) {
            if (length != 0U) {
                throw std::runtime_error("invalid PNG end chunk");
            }
            saw_end = true;
            cursor = crc_offset + 4U;
            break;
        }
        cursor = crc_offset + 4U;
    }
    if (!saw_header || !saw_end || compressed.empty()) {
        throw std::runtime_error("PNG is missing required chunks");
    }

    const auto row_bytes = static_cast<std::size_t>(width) * 4U;
    if (height > std::numeric_limits<std::size_t>::max() /
                     (row_bytes + 1U)) {
        throw std::overflow_error("PNG dimensions overflow host size");
    }
    const auto decoded_size =
        (row_bytes + 1U) * static_cast<std::size_t>(height);
    if (decoded_size > std::numeric_limits<uLongf>::max() ||
        compressed.size() > std::numeric_limits<uLong>::max()) {
        throw std::overflow_error("PNG image exceeds zlib limits");
    }
    std::vector<std::uint8_t> filtered(decoded_size);
    auto filtered_size = static_cast<uLongf>(filtered.size());
    const auto inflate_result = uncompress(
        filtered.data(), &filtered_size, compressed.data(),
        static_cast<uLong>(compressed.size()));
    if (inflate_result != Z_OK || filtered_size != filtered.size()) {
        throw std::runtime_error("cannot decompress PNG image rows");
    }

    const auto paeth = [](const std::uint8_t left, const std::uint8_t above,
                          const std::uint8_t upper_left) {
        const auto estimate = static_cast<int>(left) + static_cast<int>(above) -
                              static_cast<int>(upper_left);
        const auto left_distance = std::abs(estimate - static_cast<int>(left));
        const auto above_distance =
            std::abs(estimate - static_cast<int>(above));
        const auto upper_left_distance =
            std::abs(estimate - static_cast<int>(upper_left));
        if (left_distance <= above_distance &&
            left_distance <= upper_left_distance) {
            return left;
        }
        return above_distance <= upper_left_distance ? above : upper_left;
    };
    std::vector<std::uint8_t> pixels(
        row_bytes * static_cast<std::size_t>(height));
    for (std::size_t y = 0U; y < height; ++y) {
        const auto source_offset = y * (row_bytes + 1U);
        const auto filter = filtered[source_offset];
        if (filter > 4U) {
            throw std::runtime_error("unsupported PNG row filter");
        }
        auto* destination = pixels.data() + y * row_bytes;
        const auto* source = filtered.data() + source_offset + 1U;
        const auto* previous = y == 0U ? nullptr : destination - row_bytes;
        for (std::size_t x = 0U; x < row_bytes; ++x) {
            const auto left = x >= 4U ? destination[x - 4U] : 0U;
            const auto above = previous == nullptr ? 0U : previous[x];
            const auto upper_left =
                previous == nullptr || x < 4U ? 0U : previous[x - 4U];
            std::uint8_t predictor = 0U;
            switch (filter) {
            case 0U: predictor = 0U; break;
            case 1U: predictor = left; break;
            case 2U: predictor = above; break;
            case 3U:
                predictor = static_cast<std::uint8_t>(
                    (static_cast<unsigned>(left) + above) / 2U);
                break;
            case 4U: predictor = paeth(left, above, upper_left); break;
            }
            destination[x] = static_cast<std::uint8_t>(source[x] + predictor);
        }
    }

    RgbaImage image{static_cast<std::uint16_t>(width),
                    static_cast<std::uint16_t>(height),
                    std::vector<native::Rgba8>(
                        static_cast<std::size_t>(width) * height)};
    for (std::size_t index = 0U; index < image.pixels.size(); ++index) {
        image.pixels[index] = {pixels[index * 4U], pixels[index * 4U + 1U],
                               pixels[index * 4U + 2U],
                               pixels[index * 4U + 3U]};
    }
    return image;
}


} // namespace shadowrun::graphics
