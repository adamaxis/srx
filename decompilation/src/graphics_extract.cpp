#include "shadowrun/graphics.hpp"
#include "shadowrun/rom_layout.hpp"
#include "shadowrun/asset_taxonomy.hpp"
#include "shadowrun/resource_manifest.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

std::uint64_t parse_number(const std::string& value) {
    std::size_t consumed = 0;
    const auto number = std::stoull(value, &consumed, 0);
    if (consumed != value.size()) {
        throw std::invalid_argument("invalid numeric argument: " + value);
    }
    return number;
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open ROM: " + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

struct CroppedAsset {
    shadowrun::graphics::RgbaImage image;
    std::int32_t origin_x = 0;
    std::int32_t origin_y = 0;
};

CroppedAsset crop_transparent_bounds(
    const shadowrun::graphics::RgbaImage& source) {
    if (source.width == 0 || source.height == 0 || source.pixels.empty()) {
        throw std::invalid_argument("cannot crop an empty asset");
    }
    std::int32_t left = source.width;
    std::int32_t top = source.height;
    std::int32_t right = -1;
    std::int32_t bottom = -1;
    for (std::int32_t y = 0; y < source.height; ++y) {
        for (std::int32_t x = 0; x < source.width; ++x) {
            const auto& pixel = source.pixels[
                static_cast<std::size_t>(y) * source.width + x];
            if (pixel.alpha == 0U) {
                continue;
            }
            left = std::min(left, x);
            top = std::min(top, y);
            right = std::max(right, x);
            bottom = std::max(bottom, y);
        }
    }
    if (right < left || bottom < top) {
        throw std::runtime_error("asset contains no opaque pixels");
    }
    CroppedAsset result;
    result.origin_x = left;
    result.origin_y = top;
    result.image.width = static_cast<std::uint16_t>(right - left + 1);
    result.image.height = static_cast<std::uint16_t>(bottom - top + 1);
    result.image.pixels.resize(static_cast<std::size_t>(result.image.width) *
                               result.image.height);
    for (std::int32_t y = top; y <= bottom; ++y) {
        for (std::int32_t x = left; x <= right; ++x) {
            result.image.pixels[
                static_cast<std::size_t>(y - top) * result.image.width +
                (x - left)] = source.pixels[
                    static_cast<std::size_t>(y) * source.width + x];
        }
    }
    return result;
}

void write_scene_asset(const std::filesystem::path& root,
                       const std::string& relative_name,
                       const shadowrun::graphics::RgbaImage& source,
                       std::ofstream& manifest, bool crop) {
    const auto asset = crop ? crop_transparent_bounds(source)
                            : CroppedAsset{source, 0, 0};
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    shadowrun::graphics::write_bmp32(path.string(), asset.image);
    manifest << relative_name << '\t' << asset.origin_x << '\t'
             << asset.origin_y << '\t' << asset.image.width << '\t'
             << asset.image.height << '\n';
}

void write_circuit_backgrounds(
    const std::filesystem::path& root,
    const std::array<shadowrun::graphics::RgbaImage, 7>& phases,
    std::ofstream& manifest) {
    // Each decoder result is already the complete 320x224 background with
    // one ROM-selected circuit-light phase applied. Store those seven frames
    // directly so runtime animation is a small frame selection, not a
    // reconstruction from hundreds of cropped fragments.
    for (std::size_t phase_index = 0; phase_index < phases.size();
         ++phase_index) {
        write_scene_asset(root,
                          "title/circuit-background-" +
                              std::to_string(phase_index) + ".bmp",
                          phases[phase_index], manifest, false);
    }
}

void remove_legacy_circuit_fragments(const std::filesystem::path& root) {
    const auto title_root = root / "title";
    if (!std::filesystem::exists(title_root)) {
        return;
    }
    for (const auto& entry :
         std::filesystem::directory_iterator(title_root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto name = entry.path().filename().string();
        if (name.rfind("circuit-light-", 0) != 0 ||
            entry.path().extension() != ".bmp") {
            continue;
        }
        std::error_code error;
        std::filesystem::remove(entry.path(), error);
        if (error) {
            throw std::runtime_error(
                "cannot remove legacy circuit light fragment: " +
                entry.path().string());
        }
    }
}

void extract_title_boot_assets(const std::vector<std::uint8_t>& rom,
                               const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    std::ofstream manifest(root / "manifest.tsv", std::ios::binary);
    if (!manifest) {
        throw std::runtime_error("cannot create title/boot asset manifest");
    }
    manifest << "asset\tx\ty\twidth\theight\n";

    const auto title_background =
        shadowrun::graphics::decode_genesis_title_background(rom);
    write_scene_asset(root, "title/background.bmp", title_background,
                      manifest, false);
    const auto title_lights =
        shadowrun::graphics::decode_genesis_title_circuit_lights(rom);
    remove_legacy_circuit_fragments(root);
    write_circuit_backgrounds(root, title_lights, manifest);
    write_scene_asset(root, "title/logo.bmp",
                      shadowrun::graphics::decode_genesis_title_logo_plane(rom),
                      manifest, true);
    write_scene_asset(root, "title/text.bmp",
                      shadowrun::graphics::decode_genesis_title_text(rom),
                      manifest, true);
    const auto title_sprites =
        shadowrun::graphics::decode_genesis_title_sprite_frames(rom);
    for (std::size_t index = 0; index < title_sprites.size(); ++index) {
        write_scene_asset(root,
                          "title/sprite-" + std::to_string(index) + ".bmp",
                          title_sprites[index], manifest, true);
    }

    write_scene_asset(root, "boot/snow.bmp",
                      shadowrun::graphics::decode_genesis_boot_static(rom),
                      manifest, false);
    write_scene_asset(root, "boot/sega-background.bmp",
                      shadowrun::graphics::decode_genesis_boot_background(rom),
                      manifest, true);
    write_scene_asset(root, "boot/guy.bmp",
                      shadowrun::graphics::decode_genesis_boot_guy(rom),
                      manifest, true);
    const auto sega = shadowrun::graphics::decode_genesis_boot_sega_frames(rom);
    for (std::size_t index = 0; index < sega.size(); ++index) {
        write_scene_asset(root,
                          "boot/sega-lightning-" + std::to_string(index) +
                              ".bmp",
                          sega[index], manifest, true);
    }
    const auto strobe =
        shadowrun::graphics::decode_genesis_boot_logo_strobe(rom);
    for (std::size_t index = 0; index < strobe.size(); ++index) {
        write_scene_asset(root,
                          "boot/sega-strobe-" + std::to_string(index) +
                              ".bmp",
                          strobe[index], manifest, true);
    }
    write_scene_asset(root, "boot/fasa.bmp",
                      shadowrun::graphics::decode_genesis_boot_fasa(rom),
                      manifest, true);
}

shadowrun::graphics::RgbaImage to_rgba_image(
    const shadowrun::native::NativeImage& source) {
    return {source.width, source.height, source.pixels};
}

std::string format_resource_source(
    const shadowrun::graphics::GenesisGraphicsResource& resource) {
    std::ostringstream output;
    output << "table=0x" << std::hex << resource.table_offset << " index="
           << std::dec << resource.table_index << " sprite="
           << resource.sprite_index << " source=0x" << std::hex
           << resource.source_offset << " attr=0x" << resource.attr_or;
    return output.str();
}

void write_catalog_asset(const std::filesystem::path& root,
                         const std::string& relative_name,
                         const shadowrun::graphics::RgbaImage& image,
                         const std::string& kind,
                         const std::string& provenance, bool crop,
                         std::ofstream& manifest, std::size_t& written,
                         std::size_t& missing, std::size_t& blank) {
    if (image.width == 0U || image.height == 0U || image.pixels.empty()) {
        ++missing;
        manifest << relative_name << '\t' << kind
                 << "\t0\t0\t0\t0\t" << provenance << "\tmissing\n";
        return;
    }
    auto status = "extracted";
    CroppedAsset asset;
    if (crop) {
        try {
            asset = crop_transparent_bounds(image);
        } catch (const std::runtime_error& error) {
            if (std::string(error.what()) != "asset contains no opaque pixels") {
                throw;
            }
            // A state-specific source can legitimately be fully transparent
            // (for example, an empty initial HUD counter). Preserve it as an
            // individual transparent sprite instead of silently dropping the
            // resource or treating it as a decoder failure.
            asset = CroppedAsset{image, 0, 0};
            status = "extracted-empty";
            ++blank;
        }
    } else {
        asset = CroppedAsset{image, 0, 0};
    }
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    shadowrun::graphics::write_bmp32(path.string(), asset.image);
    manifest << relative_name << '\t' << kind << '\t' << asset.origin_x
             << '\t' << asset.origin_y << '\t' << asset.image.width << '\t'
             << asset.image.height << '\t' << provenance << '\t' << status
             << '\n';
    ++written;
}

void append_title_boot_catalog(const std::filesystem::path& root,
                               std::ofstream& manifest) {
    std::ifstream source(root / "manifest.tsv", std::ios::binary);
    if (!source) {
        throw std::runtime_error("title/boot manifest was not generated");
    }
    std::string line;
    std::getline(source, line);
    while (std::getline(source, line)) {
        if (line.empty()) {
            continue;
        }
        const auto kind = line.find("background.bmp") != std::string::npos ||
                line.find("circuit-background-") != std::string::npos ||
                line.find("snow.bmp") != std::string::npos
            ? "background"
            : "sprite";
        manifest << line.substr(0, line.find('\t')) << '\t' << kind
                 << '\t' << line.substr(line.find('\t') + 1)
                 << '\t' << "ROM title/boot resource\textracted\n";
    }
}

void write_asset_readme(const std::filesystem::path& root) {
    std::ofstream readme(root / "README.md", std::ios::binary);
    if (!readme) {
        throw std::runtime_error("cannot create direct asset README");
    }
    readme << "# Direct Shadowrun assets\n\n"
           << "Generated from the US Genesis ROM by "
              "shadowrun_graphics_extract --all-assets.\n\n"
           << "The native runtime consumes these individually named BMPs and "
              "native/state.bin and resources.tsv directly. This tree deliberately contains "
              "semantic folders: menu, intro, field, and item categories. "
              "Static intro plates and independently extracted intro sprites "
              "remain separate; "
              "it does not contain full-screen flipbook samples or an aggregate "
              "serializer. Item art is grouped under items/weapons, "
              "items/first-aid, items/miscellaneous, items/magic, and items/armor. "
              "Separately traced cinematic resources belong under movies/<scene>; "
              "they must not be mixed into intro or field assets.\n\n"
           << "Audio is emitted here only as complete Windows-playable lossless "
              "FLAC files under\n"
              "`audio/`. The offline Z80/VGM extraction sources and intermediate "
              "event data\n"
              "remain outside this runtime asset tree. Verified repeating audio "
              "uses\n"
              "`audio/loop-points.tsv` for exact PCM frame boundaries. Use "
              "`resources.tsv` for stable aliases,\n"
              "such as `assets.audio_path(\"audio.intro.title.stage-1\")`; do "
              "not add raw ROM\n"
              "slices or split event fragments here. A missing loop-point entry "
              "is\n"
              "intentional: the native player must not guess a repeat boundary "
              "from an\n"
              "internal channel restart.\n";
}

void write_runtime_resource_manifest(const std::filesystem::path& root) {
    std::ifstream catalog(root / "catalog.tsv", std::ios::binary);
    if (!catalog) {
        throw std::runtime_error("cannot read generated asset catalog");
    }
    // Graphics extraction is an explicit import step, but it must not erase
    // independently imported runtime resources. Preserve existing state and
    // audio rows while regenerating the image aliases from catalog.tsv.
    std::vector<std::string> preserved_state_rows;
    std::vector<std::string> preserved_runtime_rows;
    std::ifstream prior_manifest(
        root / std::string(shadowrun::resources::kManifestFile),
        std::ios::binary);
    if (prior_manifest) {
        std::string prior_line;
        std::getline(prior_manifest, prior_line);
        while (std::getline(prior_manifest, prior_line)) {
            if (prior_line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::size_t start = 0;
            for (;;) {
                const auto end = prior_line.find('\t', start);
                fields.push_back(prior_line.substr(
                    start, end == std::string::npos ? std::string::npos
                                                     : end - start));
                if (end == std::string::npos) {
                    break;
                }
                start = end + 1U;
            }
            if (fields.size() == 5U && fields[0] != "state" &&
                fields[2] != "image" &&
                std::filesystem::is_regular_file(root / fields[1])) {
                if (fields[2] == "state") {
                    preserved_state_rows.push_back(prior_line);
                } else {
                    preserved_runtime_rows.push_back(prior_line);
                }
            }
        }
    }
    std::ofstream output(root / std::string(
                              shadowrun::resources::kManifestFile),
                         std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot create runtime resource manifest");
    }
    output << "alias\tpath\tkind\torigin_x\torigin_y\n"
           << "state\tnative/state.bin\tstate\t0\t0\n";
    std::unordered_set<std::string> aliases{"state"};
    const auto emit_preserved_rows = [&output, &aliases](
                                         const std::vector<std::string>& rows) {
        for (const auto& preserved : rows) {
            const auto separator = preserved.find('\t');
            if (separator == std::string::npos) {
                continue;
            }
            const auto alias = preserved.substr(0, separator);
            if (aliases.insert(alias).second) {
                output << preserved << '\n';
            }
        }
    };
    emit_preserved_rows(preserved_state_rows);
    std::string line;
    std::getline(catalog, line);
    while (std::getline(catalog, line)) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> fields;
        std::size_t start = 0;
        for (;;) {
            const auto end = line.find('\t', start);
            fields.push_back(line.substr(
                start, end == std::string::npos ? std::string::npos
                                                 : end - start));
            if (end == std::string::npos) {
                break;
            }
            start = end + 1U;
        }
        if (fields.size() < 8U) {
            throw std::runtime_error("invalid generated asset catalog row: " +
                                     line);
        }
        const auto& relative = fields[0];
        if (relative.find("-gray-") != std::string::npos) {
            continue;
        }
        const auto alias = shadowrun::resources::alias_for_path(relative);
        if (!aliases.insert(alias).second) {
            throw std::runtime_error("duplicate generated resource alias: " +
                                     alias);
        }
        output << alias << '\t' << relative << "\timage\t" << fields[2]
               << '\t' << fields[3] << '\n';
    }
    emit_preserved_rows(preserved_runtime_rows);
}

void extract_all_assets(const std::vector<std::uint8_t>& rom,
                        const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    extract_title_boot_assets(rom, root);

    std::ofstream manifest(root / "catalog.tsv", std::ios::binary);
    if (!manifest) {
        throw std::runtime_error("cannot create all-resource catalog");
    }
    manifest << "asset\tkind\tx\ty\twidth\theight\tsource\tstatus\n";
    write_asset_readme(root);
    append_title_boot_catalog(root, manifest);
    std::size_t written = 0;
    std::size_t missing = 0;
    std::size_t blank = 0;
    const auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    const auto write = [&](const std::string& name,
                           const shadowrun::native::NativeImage& image,
                           const std::string& kind,
                           const std::string& source, bool crop) {
        write_catalog_asset(root, name, to_rgba_image(image), kind, source,
                            crop, manifest, written, missing, blank);
    };
    const auto write_frame = [&](const std::string& name,
                                 const shadowrun::native::NativeEntityFrame& frame,
                                 const std::string& kind,
                                 const std::string& source, bool crop) {
        write(name,
              shadowrun::native::NativeImage{frame.width, frame.height,
                                             frame.pixels},
              kind, source, crop);
    };
    const auto write_array = [&](const std::string& prefix,
                                 const auto& images, const std::string& kind,
                                 const std::string& source, bool crop) {
        for (std::size_t index = 0; index < images.size(); ++index) {
            write(prefix + std::to_string(index) + ".bmp", images[index], kind,
                  source + "[" + std::to_string(index) + "]", crop);
        }
    };

    // Startup and opening source layers.
    write_array("title/animation-", assets.title_animation_frames, "background",
                "native title source composition", false);
    write("menu/startup/background.bmp", assets.startup_background, "background",
          "0x10B5A/0xCDCE6", false);
    write("menu/startup/font.bmp", assets.startup_font, "sprite", "0xC1320", true);
    write("menu/startup/cursor.bmp", assets.startup_cursor, "sprite", "0xD29F6", true);
    write("intro/aircraft-plane-a.bmp", assets.intro_aircraft_plane_a, "sprite",
          "0x5B528/0x5B7D8", true);
    write("intro/trivid-background.bmp", assets.intro_trivid_background,
          "background", "0x5AA96", false);
    const auto intro_backgrounds =
        shadowrun::graphics::decode_genesis_intro_backgrounds(rom);
    for (std::size_t index = 0; index < intro_backgrounds.size(); ++index) {
        write_catalog_asset(root, "intro/background-" + std::to_string(index) +
                                  ".bmp",
                            intro_backgrounds[index], "background", "0x59C5E",
                            false, manifest, written, missing, blank);
    }
    const auto intro_sprites =
        shadowrun::graphics::decode_genesis_intro_sprite_resources(rom);
    for (std::size_t index = 0; index < intro_sprites.size(); ++index) {
        const auto& resource = intro_sprites[index];
        write_catalog_asset(
            root, "intro/sprites/" + resource.role + "-" +
                      std::to_string(resource.table_index) + "-" +
                      std::to_string(resource.sprite_index) + ".bmp",
            resource.image, "sprite",
            format_resource_source(resource),
            true, manifest, written, missing, blank);
    }
    const auto intro_patches =
        shadowrun::graphics::decode_genesis_intro_ambush_background_resources(
            rom);
    for (std::size_t index = 0; index < intro_patches.size(); ++index) {
        const auto& resource = intro_patches[index];
        write_catalog_asset(
            root, "intro/background-patch-" + std::to_string(index) + ".bmp",
            resource.image, "background",
            format_resource_source(resource),
            true, manifest, written, missing, blank);
    }
    write("intro/cab-map.bmp", assets.cab_map, "background", "0x134D2/0xD2A2E",
          false);

    // Field HUD and menu source layers.
    write("field/hud/background.bmp", assets.field_hud, "background",
          "0x334C/0x39F2", false);
    write_array("field/hud/face-", assets.field_hud_faces, "sprite", "0x3404",
                true);
    write_array("field/hud/item-", assets.field_hud_item_blocks, "sprite",
                "0x3404", true);
    write_array("field/hud/unarmed-item-",
                assets.field_hud_unarmed_item_blocks, "sprite",
                "0x3404/0xD1860-0xD1878", true);
    write_array("field/hud/vertical-counter-", assets.field_hud_vertical_counters,
                "sprite", "0x35D4", true);
    write_array("field/hud/horizontal-counter-",
                assets.field_hud_horizontal_counters, "sprite", "0x35D4", true);
    write_array("field/hud/ammo-counter-", assets.field_hud_ammo_counters,
                "sprite", "0x36FC", true);
    write("menu/pocket-secretary/background.bmp",
          assets.pocket_secretary_background, "background", "0x10A7E", false);
    write("menu/field/background.bmp", assets.field_menu_background, "background",
          "0x10F50", false);
    write("menu/field/action-background.bmp",
          assets.field_menu_action_background, "background", "0x117A0", false);
    write("menu/field/attributes-background.bmp",
          assets.field_menu_attributes_background, "background", "0x10C96", false);
    write("menu/field/inventory-boxes.bmp", assets.field_menu_inventory_boxes,
          "background", "0x10F50/0x1232", false);
    write("menu/field/font.bmp", assets.field_menu_font, "sprite", "0xC1064",
          true);
    write_array("menu/field/runner-portrait-", assets.field_menu_runner_portraits,
                "sprite", "0x7291A", true);
    write_array("menu/field/selected-cell-", assets.field_menu_selected_cells,
                "sprite", "0x11678", true);
    write_array("menu/field/command-focus-", assets.field_menu_command_focus,
                "sprite", "0x11454", true);
    write_array("menu/field/action-focus-", assets.field_menu_action_focus,
                "sprite", "0x117A4", true);
    write_array("menu/field/clips-focus-", assets.field_menu_clips_focus, "sprite",
                "0x11B58", true);
    write_array("menu/field/clips-trade-focus-", assets.field_menu_clips_trade_focus,
                "sprite", "0x11E7E", true);
    write("menu/field/posture-focus.bmp", assets.field_menu_posture_focus, "sprite",
          "0x119CE", true);
    write("menu/field/cursor.bmp", assets.field_menu_cursor, "sprite", "0x11592",
          true);
    for (std::size_t item = 0; item < assets.field_menu_item_art.size(); ++item) {
        const auto item_code = item + 1U;
        write(shadowrun::resources::item_detail_asset_path(item_code),
              assets.field_menu_item_art[item], "sprite",
              "0x12A8C/0xD4540[item=" + std::to_string(item_code) + "]", true);
    }
    for (std::size_t item = 0; item < assets.field_menu_inventory_icons.size();
         ++item) {
        for (std::size_t variant = 0;
             variant < assets.field_menu_inventory_icons[item].size(); ++variant) {
            write(shadowrun::resources::item_icon_asset_path(item + 1U, variant),
                  assets.field_menu_inventory_icons[item][variant], "sprite",
                  "0x35D4/0xD4540[item=" + std::to_string(item + 1U) +
                      "][variant=" + std::to_string(variant) + "]", true);
        }
    }
    write_array("menu/field/status-bar-", assets.field_menu_status_bars, "sprite",
                "0x11928", true);
    write_array("menu/field/damage-bar-", assets.field_menu_damage_bars, "sprite",
                "0x12F70", true);
    write_array("menu/field/drain-bar-", assets.field_menu_drain_bars, "sprite",
                "0x12FA4", true);
    write_array("menu/field/detail-value-", assets.field_menu_detail_values, "sprite",
                "0x13000", true);
    write_array("menu/field/attack-bar-", assets.field_menu_attack_bars, "sprite",
                "0x12CEE/0xCF3D4", true);
    write_array("menu/field/defense-bar-", assets.field_menu_defense_bars, "sprite",
                "0x12CEE/0xCF610", true);

    // Field maps are backgrounds; entity and walking frames are sprites.
    // The mode-2 map is already the first entry in field_areas; emitting the
    // legacy field_map field as a second row would overwrite the same asset
    // path and make the catalog appear to contain a duplicate resource.
    for (const auto& area : assets.field_areas) {
        write("field/maps/mode-" + std::to_string(area.mode_id) + ".bmp",
              area.map, "background", "mode resource catalog", false);
    }

    // Keep each rendered map tied to the descriptor that selected it. This is
    // separate from catalog.tsv: that file describes host images, while this
    // index audits the complete ROM table used by 0x3074.
    std::ofstream map_index(root / "field/maps/index.tsv", std::ios::binary);
    if (!map_index) {
        throw std::runtime_error("cannot create field map resource index");
    }
    map_index
        << "mode_id\ttable_entry\trecord_offset\tcamera_x_fixed"
           "\tcamera_y_fixed\tbound_x\tbound_y\tmin_x\tmin_y"
           "\tterrain_record\tsetup_block\tpalette\tpattern\tpattern_bytes"
           "\tmetatile_stream\tprimary_tilemap\tsecondary_tilemap"
           "\ttilemap_width\ttilemap_height\tmetatile_encoding"
           "\tevent_table\tasset\timage_width\timage_height\n";
    const auto write_hex = [&map_index](std::size_t value) {
        map_index << "0x" << std::uppercase << std::hex << value << std::dec;
    };
    for (std::uint16_t mode_id = 0;
         mode_id < shadowrun::rom::kFieldModeCount; ++mode_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(rom, mode_id);
        const auto area = std::find_if(
            assets.field_areas.begin(), assets.field_areas.end(),
            [mode_id](const auto& candidate) { return candidate.mode_id == mode_id; });
        if (area == assets.field_areas.end()) {
            throw std::runtime_error("field map index is missing mode " +
                                     std::to_string(mode_id));
        }
        map_index << mode_id << '\t';
        write_hex(shadowrun::rom::kFieldModePointerTable +
                  static_cast<std::size_t>(mode_id) * 4U);
        map_index << '\t';
        write_hex(resources.record_offset);
        map_index << '\t' << resources.initial_camera_x_fixed << '\t'
                  << resources.initial_camera_y_fixed << '\t'
                  << resources.terrain_x_exclusive_upper << '\t'
                  << resources.terrain_y_exclusive_upper << '\t'
                  << resources.terrain_x_inclusive_lower << '\t'
                  << resources.terrain_y_inclusive_lower << '\t';
        write_hex(resources.terrain_record_offset);
        map_index << '\t';
        write_hex(resources.setup_block_offset);
        map_index << '\t';
        write_hex(resources.palette_offset);
        map_index << '\t';
        write_hex(resources.pattern_data_offset);
        map_index << '\t' << resources.pattern_byte_count << '\t';
        write_hex(resources.metatile_stream_offset);
        map_index << '\t';
        write_hex(resources.primary_tilemap_offset);
        map_index << '\t';
        write_hex(resources.secondary_tilemap_offset);
        map_index << '\t' << resources.tilemap_width << '\t'
                  << resources.tilemap_height << '\t'
                  << (resources.metatile_encoding ==
                              shadowrun::graphics::GenesisMetatileEncoding::RawNameWords
                          ? "raw-name-words"
                          : "compressed-resource")
                  << '\t';
        write_hex(resources.event_table_offset);
        map_index << "\tfield/maps/mode-" << mode_id << ".bmp\t"
                  << area->map.width << '\t' << area->map.height << '\n';
    }
    map_index.flush();

    for (std::size_t index = 0; index < assets.entity_frames.size(); ++index) {
        write_frame("field/entities/idle-" + std::to_string(index) + ".bmp",
                    assets.entity_frames[index], "sprite", "0x5A62/0x5FDC",
                    true);
    }
    for (std::size_t direction = 0;
         direction < assets.field_entity_animations.size(); ++direction) {
        const auto& animation = assets.field_entity_animations[direction];
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            write_frame("field/entities/walk-" + std::to_string(direction) + "-" +
                            std::to_string(frame) + ".bmp",
                        animation.frames[frame], "sprite", "0x5A62/0x5FDC", true);
        }
    }

    manifest.flush();
    std::filesystem::create_directories(root / "native");
    shadowrun::graphics::write_native_asset_state(
        (root / "native" / "state.bin").string(), assets);
    write_runtime_resource_manifest(root);
    std::cout << "all graphics assets written to " << root << "\n"
              << "extracted=" << written << " blank=" << blank
              << " missing=" << missing << "\n";
    if (missing != 0U) {
        throw std::runtime_error(
            "asset catalog contains missing image resources; extraction coverage is incomplete");
    }
}

shadowrun::graphics::RgbaImage make_field_entity_state_sheet(
    const std::vector<shadowrun::graphics::RgbaImage>& frames) {
    constexpr std::uint16_t kCellWidth = 40;
    constexpr std::uint16_t kCellHeight = 40;
    constexpr std::size_t kColumns = 4;
    const auto rows = std::max<std::size_t>(
        1U, (frames.size() + kColumns - 1U) / kColumns);
    shadowrun::graphics::RgbaImage sheet{
        static_cast<std::uint16_t>(kCellWidth * kColumns),
        static_cast<std::uint16_t>(kCellHeight * rows),
        std::vector<shadowrun::native::Rgba8>(
            static_cast<std::size_t>(kCellWidth * kColumns) *
                (kCellHeight * rows),
            {0x04, 0x08, 0x14, 0xFF}),
    };
    for (std::size_t index = 0; index < frames.size(); ++index) {
        const auto& frame = frames[index];
        const auto cell_x = static_cast<std::int32_t>(
            (index % kColumns) * kCellWidth);
        const auto cell_y = static_cast<std::int32_t>(
            (index / kColumns) * kCellHeight);
        const auto origin_x = cell_x +
            (static_cast<std::int32_t>(kCellWidth) - frame.width) / 2;
        const auto origin_y = cell_y +
            (static_cast<std::int32_t>(kCellHeight) - frame.height) / 2;
        for (std::int32_t y = 0; y < frame.height; ++y) {
            for (std::int32_t x = 0; x < frame.width; ++x) {
                const auto& pixel = frame.pixels[
                    static_cast<std::size_t>(y) * frame.width + x];
                if (pixel.alpha == 0) {
                    continue;
                }
                const auto target_x = origin_x + x;
                const auto target_y = origin_y + y;
                sheet.pixels[static_cast<std::size_t>(target_y) * sheet.width +
                             target_x] = pixel;
            }
        }
    }
    return sheet;
}

std::vector<shadowrun::graphics::RgbaImage> flatten_field_entity_animations(
    const std::vector<shadowrun::graphics::GenesisFieldEntityAnimation>&
        animations) {
    std::vector<shadowrun::graphics::RgbaImage> frames;
    for (const auto& animation : animations) {
        for (const auto& frame : animation.frames) {
            frames.push_back(frame);
        }
    }
    return frames;
}

void print_field_event_cells(
    const shadowrun::terrain::Map& terrain,
    const std::array<std::uint8_t, 32>& event_mode_table) {
    std::size_t count = 0;
    for (std::size_t row = 0; row < shadowrun::terrain::Map::kHeight; ++row) {
        for (std::size_t column = 0;
             column < shadowrun::terrain::Map::kWidth; ++column) {
            const auto status = terrain.statuses[
                row * shadowrun::terrain::Map::kWidth + column];
            if ((status & 0xE0U) != 0xC0U) {
                continue;
            }
            const auto event_index = static_cast<std::size_t>(status & 0x1FU);
            const auto destination = event_index < event_mode_table.size()
                ? event_mode_table[event_index]
                : 0U;
            std::cout << "cell (" << column * 32U << "," << row * 32U
                      << ") raw=0x" << std::hex << static_cast<unsigned>(status)
                      << " event=" << std::dec << event_index
                      << " destination=" << destination << "\n";
            ++count;
        }
    }
    std::cout << "C0 event cells: " << count << "\n";
}

shadowrun::graphics::RgbaImage render_native_field(
    shadowrun::native::NativePortAssets assets) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    // Follow the reviewed title -> start options -> new game -> archetype
    // path. New Game enters 0x59C5E; skip that source-timed opening for this
    // field-only diagnostic, just as a user can with an action edge.
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::A);
    press(shadowrun::native::Button::A);
    if (shell.scene() == shadowrun::native::NativeScene::Intro) {
        press(shadowrun::native::Button::Start);
    }
    if (shell.scene() != shadowrun::native::NativeScene::Field) {
        throw std::runtime_error(
            "native field diagnostic did not reach the field scene");
    }
    shell.render(screen);

    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size()),
    };
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255,
        };
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_field_menu(
    shadowrun::native::NativePortAssets assets, bool inventory) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::A);
    press(shadowrun::native::Button::A);
    if (shell.scene() == shadowrun::native::NativeScene::Intro) {
        press(shadowrun::native::Button::Start);
    }
    if (shell.scene() != shadowrun::native::NativeScene::Field) {
        throw std::runtime_error(
            "native field-menu diagnostic did not reach the field scene");
    }
    press(shadowrun::native::Button::Start);
    if (shell.scene() != shadowrun::native::NativeScene::FieldMenu) {
        throw std::runtime_error(
            "native field-menu diagnostic did not open the command screen");
    }
    if (inventory) {
        const auto press_menu_edge = [&shell](shadowrun::native::Button button) {
            const auto mask = shadowrun::native::button_mask(button);
            shell.update(shadowrun::native::InputState{mask, mask});
            shell.update(shadowrun::native::InputState{});
        };
        // The ROM starts at Pocket Secretary (selector 6). Six left edges
        // select the first Inventory column, which exercises the real
        // selected-cell, item-art, and item-stat destinations.
        for (int step = 0; step < 6; ++step) {
            press_menu_edge(shadowrun::native::Button::Left);
        }
    }
    shell.render(screen);

    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size()),
    };
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255,
        };
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_startup_scene(
    shadowrun::native::NativePortAssets assets, const std::string& scene_name) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    if (scene_name == "start-options") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
    } else if (scene_name == "save-load" || scene_name == "no-save") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Down);
        press(shadowrun::native::Button::A);
        if (scene_name == "no-save") {
            press(shadowrun::native::Button::A);
        }
    } else if (scene_name == "archetype") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::A);
    } else if (scene_name == "intro") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::A);
        press(shadowrun::native::Button::A);
    } else {
        throw std::invalid_argument("unknown native startup scene");
    }
    shell.render(screen);
    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_title_frame(
    shadowrun::native::NativePortAssets assets, std::size_t frame) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    for (std::size_t index = 0; index < frame; ++index) {
        shell.update(shadowrun::native::InputState{});
    }
    shell.render(screen);
    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 4 && std::string(argv[3]) == "--field-event-cells") {
        try {
            const auto rom = read_file(argv[1]);
            const auto assets =
                shadowrun::graphics::decode_genesis_native_assets(rom);
            print_field_event_cells(assets.field_terrain,
                                    assets.field_event_mode_table);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-event-cells") {
        try {
            const auto rom = read_file(argv[1]);
            const auto assets =
                shadowrun::graphics::decode_genesis_native_assets(rom);
            const auto mode_id = static_cast<std::uint16_t>(
                parse_number(argv[4]));
            const auto area = std::find_if(
                assets.field_areas.begin(), assets.field_areas.end(),
                [mode_id](const shadowrun::native::NativeFieldArea& candidate) {
                    return candidate.mode_id == mode_id;
                });
            if (area == assets.field_areas.end()) {
                throw std::out_of_range("field mode is not in the native catalog");
            }
            std::cout << "field mode " << mode_id << " event cells:\n";
            print_field_event_cells(area->terrain, area->event_mode_table);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--title") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_title_screen(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "startup scene: " << image.width << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-title-text") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_title_text(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native title text layer: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        (std::string(argv[3]) == "--native-start-options" ||
         std::string(argv[3]) == "--native-save-load" ||
         std::string(argv[3]) == "--native-no-save" ||
         std::string(argv[3]) == "--native-archetype" ||
         std::string(argv[3]) == "--native-intro")) {
        try {
            const auto rom = read_file(argv[1]);
            const auto name = std::string(argv[3]).substr(9);
            const auto image = render_native_startup_scene(
                shadowrun::graphics::decode_genesis_native_assets(rom), name);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native startup scene: " << name << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--native-title-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frame = static_cast<std::size_t>(
                parse_number(argv[4]));
            const auto image = render_native_title_frame(
                shadowrun::graphics::decode_genesis_native_assets(rom), frame);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native title frame: " << frame << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-stage") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto stages = shadowrun::graphics::decode_genesis_intro_stages(rom);
            if (index >= stages.size()) {
                throw std::out_of_range("intro stage is outside the ROM stage set");
            }
            shadowrun::graphics::write_bmp32(argv[2], stages[index]);
            std::cout << "intro stage " << index << ": " << stages[index].width
                      << "x" << stages[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-heli") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames = shadowrun::graphics::decode_genesis_intro_heli_frames(rom);
            if (index >= frames.size()) {
                throw std::out_of_range("intro helicopter frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro helicopter frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-aircraft-movement") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_aircraft_movement_frames(
                    rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro aircraft movement frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro aircraft movement frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--intro-aircraft-plane-a") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_intro_aircraft_plane_a(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "intro aircraft Plane-A: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-wilderness-pan") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_wilderness_pan_frames(
                    rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro wilderness pan frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro wilderness pan frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-ambush") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_ambush_frames(rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro ambush frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro ambush frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--cab-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_cab_map(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "cab map: " << image.width << "x" << image.height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--startup-base") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_startup_screen(
                rom, 0x1F310EU, 0x67DF6U, 0x1FF0U);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "startup base scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--title-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_title_animation_frames(rom);
            const auto frame_index = static_cast<std::size_t>(
                parse_number(argv[4]));
            if (frame_index >= frames.size()) {
                throw std::out_of_range("title frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[frame_index]);
            std::cout << "title animation frame " << frame_index << ": "
                      << frames[frame_index].width << "x"
                      << frames[frame_index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field(
                shadowrun::graphics::decode_genesis_native_assets(rom));
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field-menu") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field_menu(
                shadowrun::graphics::decode_genesis_native_assets(rom), false);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field menu scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field-menu-inventory") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field_menu(
                shadowrun::graphics::decode_genesis_native_assets(rom), true);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field inventory menu scene: " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-menu") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_menu_background(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field menu base: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-menu-inventory") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_field_menu_inventory_boxes(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field inventory boxes: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--mode2-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_mode2_field_map(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "mode-2 field map: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--mode-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto mode_id = static_cast<std::uint16_t>(
                parse_number(argv[4]));
            const auto resources =
                shadowrun::graphics::read_genesis_field_mode_resources(
                    rom, mode_id);
            const auto image = shadowrun::graphics::decode_genesis_field_mode_map(
                rom, resources);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field mode " << mode_id << ": " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--entity-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_entity_frame(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "entity frame: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-states") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity state sheet: " << sheet.width << "x"
                      << sheet.height << " (" << frames.size() << " states)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-walking") {
        try {
            const auto rom = read_file(argv[1]);
            const auto animations =
                shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
                    rom);
            const auto frames = flatten_field_entity_animations(animations);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity walking sheet: " << sheet.width << "x"
                      << sheet.height << " (" << frames.size() << " frames)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-entity-state") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            if (index >= frames.size()) {
                throw std::out_of_range("field entity state index is out of range");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "field entity state " << index << ": "
                      << frames[index].width << "x" << frames[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--title-boot-assets") {
        try {
            const auto rom = read_file(argv[1]);
            extract_title_boot_assets(rom, argv[2]);
            std::cout << "individual title/boot assets written to "
                      << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--all-assets") {
        try {
            const auto rom = read_file(argv[1]);
            extract_all_assets(rom, argv[2]);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc != 6 && argc != 7) {
        std::cerr << "usage: shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "<tile-offset> <tile-count> <palette-offset> [--resource]\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> --title\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-field\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--intro-stage <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--intro-heli <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-aircraft-plane-a\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-wilderness-pan <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-ambush <index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--cab-map\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-menu\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-menu-inventory\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-field-menu-inventory\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <ignored> "
                     "--field-event-cells\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <ignored> "
                     "--field-event-cells <mode-id>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--mode2-map\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--mode-map <mode-id>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--entity-frame\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-states\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-walking\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-state <index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <asset-dir> "
                     "--title-boot-assets\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <asset-dir> "
                     "--all-assets\n";
        std::cerr << "numeric offsets accept decimal or 0x-prefixed hex\n";
        return 2;
    }
    try {
        const auto rom = read_file(argv[1]);
        const auto tile_offset = static_cast<std::size_t>(parse_number(argv[3]));
        const auto tile_count = static_cast<std::size_t>(parse_number(argv[4]));
        const auto palette_offset = static_cast<std::size_t>(parse_number(argv[5]));
        const auto resource_stream = argc == 7 && std::string(argv[6]) == "--resource";
        if (argc == 7 && !resource_stream) {
            throw std::invalid_argument("unknown extractor mode");
        }
        const auto palette = shadowrun::graphics::read_genesis_palette(
            rom, palette_offset);
        const auto resource = resource_stream
            ? shadowrun::graphics::decode_genesis_resource_stream(rom, tile_offset)
            : std::vector<std::uint8_t>{};
        const auto& tile_source = resource_stream ? resource : rom;
        const auto image = shadowrun::graphics::decode_tile_atlas(
            tile_source, resource_stream ? 0U : tile_offset, tile_count, palette);
        shadowrun::graphics::write_bmp32(argv[2], image);
        std::cout << "tiles: " << tile_count << "\n"
                  << "image: " << image.width << "x" << image.height << "\n"
                  << (resource_stream ? "decoded bytes: " +
                                             std::to_string(resource.size()) +
                                             "\n" : "")
                  << "output: " << argv[2] << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
