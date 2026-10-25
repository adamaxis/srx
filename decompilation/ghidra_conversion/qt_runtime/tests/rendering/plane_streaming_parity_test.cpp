#include "field_map_composition.hpp"
#include "genesis_image_compositor.hpp"
#include "plane_streamer.hpp"

#include <QColor>
#include <QImage>
#include <QPainter>

#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion::qt_runtime;
using shadowrun::ghidra_conversion::FieldViewportState;
using shadowrun::ghidra_conversion::kFieldAlternateTileTable;
using shadowrun::ghidra_conversion::kFieldNormalTileTable;

constexpr int kSceneWidth = VdpModel::kDisplayWidth;
constexpr int kSceneHeight = VdpModel::kDisplayHeight;
constexpr std::size_t kScenePixelCount =
    static_cast<std::size_t>(kSceneWidth * kSceneHeight);
constexpr std::size_t kMaximumMismatches = 0U;

struct PreparedPlane {
    std::vector<std::uint16_t> words;
    std::size_t width_tiles = 0U;
    std::size_t height_tiles = 0U;
};

bool fail(const std::string& message) {
    std::cerr << "plane streaming parity: " << message << '\n';
    return false;
}

std::vector<std::string> split_tabs(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0U;
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

std::optional<std::size_t> palette_offset_for_scene(
    const std::filesystem::path& root, const int wanted_scene) {
    std::ifstream index(root / "field/maps/index.tsv", std::ios::binary);
    std::string line;
    if (!std::getline(index, line)) {
        return std::nullopt;
    }
    while (std::getline(index, line)) {
        const auto fields = split_tabs(line);
        if (fields.size() < 12U || std::stoi(fields[0]) != wanted_scene) {
            continue;
        }
        return static_cast<std::size_t>(std::stoul(fields[11], nullptr, 0));
    }
    return std::nullopt;
}

std::optional<std::vector<std::uint8_t>> read_file(
    const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> bytes;
    for (char byte = 0; input.get(byte);) {
        bytes.push_back(static_cast<std::uint8_t>(byte));
    }
    return bytes;
}

std::array<std::array<QRgb, 16>, 4> make_rgb_palettes(
    const std::vector<std::uint8_t>& rom, const std::size_t palette_offset,
    VdpModel& vdp) {
    std::array<std::array<QRgb, 16>, 4> colors{};
    for (std::size_t palette = 0U; palette < 4U; ++palette) {
        for (std::size_t index = 0U; index < 16U; ++index) {
            const auto cram_index = palette * 16U + index;
            // 0x1406 loads the scene palette's first line; 0x566C2 replaces
            // CRAM lines 1..3 from the shared palette tail at 0x18AC56 [C].
            const auto offset = cram_index < 16U
                ? palette_offset + cram_index * 2U
                : 0x18AC56U + (cram_index - 16U) * 2U;
            const auto word = static_cast<std::uint16_t>(
                (static_cast<std::uint16_t>(rom[offset]) << 8U) |
                rom[offset + 1U]);
            vdp.set_cram_word(static_cast<std::uint8_t>(palette),
                              static_cast<std::uint8_t>(index), word);
            colors[palette][index] =
                genesis_cram_word_to_color(word).rgb();
        }
    }
    return colors;
}

std::optional<std::uint8_t> palette_index_for_rgb(
    const std::array<QRgb, 16>& palette, const QRgb rgb) {
    // Color zero is transparent on a Genesis plane. Prefer any nonzero CRAM
    // duplicate so an opaque extracted color can remain an opaque VDP pixel.
    for (std::uint8_t index = 1U; index < 16U; ++index) {
        if (palette[index] == rgb) {
            return index;
        }
    }
    if (palette[0] == rgb) {
        return 0U;
    }
    return std::nullopt;
}

struct IndexedTile {
    VdpModel::Pattern pixels{};
    std::uint8_t palette = 0U;
};

std::optional<IndexedTile> index_source_tile(
    const QImage& source, const int origin_x, const int origin_y,
    const std::array<std::array<QRgb, 16>, 4>& palettes) {
    for (std::uint8_t palette = 0U; palette < 4U; ++palette) {
        IndexedTile result{};
        result.palette = palette;
        bool matches = true;
        for (int y = 0; y < 8 && matches; ++y) {
            for (int x = 0; x < 8; ++x) {
                const auto pixel = source.pixelColor(origin_x + x,
                                                     origin_y + y);
                auto color_index = std::uint8_t{0U};
                if (pixel.alpha() != 0) {
                    const auto selected = palette_index_for_rgb(
                        palettes[palette], pixel.rgb());
                    if (!selected.has_value()) {
                        matches = false;
                        break;
                    }
                    color_index = *selected;
                }
                result.pixels[static_cast<std::size_t>(y * 8 + x)] =
                    color_index;
            }
        }
        if (matches) {
            return result;
        }
    }
    return std::nullopt;
}

bool tile_is_high_priority(const QImage& plane, const QImage& map,
                           const QImage& other_plane, const QImage& overlay,
                           const int origin_x, const int origin_y) {
    // [H] The export has the priority overlay, not source name words, so this
    // harness infers a tile's priority from pixels that distinguish the two
    // extracted planes. This is suitable for parity, not independent ROM proof.
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const auto plane_pixel = plane.pixelColor(origin_x + x,
                                                      origin_y + y);
            const auto other_pixel = other_plane.pixelColor(origin_x + x,
                                                            origin_y + y);
            if (plane_pixel.alpha() == 0 ||
                overlay.pixelColor(origin_x + x, origin_y + y).alpha() == 0) {
                continue;
            }
            const auto selected_pixel =
                map.pixelColor(origin_x + x, origin_y + y).rgb();
            // RGB equality alone cannot identify the selected source tile:
            // equal CRAM colors can make an occluded plane look selected too.
            // Require a pixel that distinguishes this plane from the other.
            if (selected_pixel == plane_pixel.rgb() &&
                (other_pixel.alpha() == 0 ||
                 selected_pixel != other_pixel.rgb())) {
                return true;
            }
        }
    }
    return false;
}

std::optional<PreparedPlane> prepare_plane(
    const QImage& source, const QImage& other_plane, const QImage& map,
    const QImage& overlay,
    const std::array<std::array<QRgb, 16>, 4>& palettes, VdpModel& vdp,
    std::map<VdpModel::Pattern, std::uint16_t>& pattern_ids,
    const bool source_priority_inverted) {
    if (source.isNull() || source.width() % 8 != 0 ||
        source.height() % 8 != 0 || source.size() != map.size() ||
        source.size() != overlay.size()) {
        return std::nullopt;
    }
    PreparedPlane result;
    result.width_tiles = static_cast<std::size_t>(source.width() / 8);
    result.height_tiles = static_cast<std::size_t>(source.height() / 8);
    result.words.resize(result.width_tiles * result.height_tiles);

    for (int tile_y = 0; tile_y < source.height() / 8; ++tile_y) {
        for (int tile_x = 0; tile_x < source.width() / 8; ++tile_x) {
            const auto origin_x = tile_x * 8;
            const auto origin_y = tile_y * 8;
            auto indexed = index_source_tile(source, origin_x, origin_y,
                                             palettes);
            if (!indexed.has_value()) {
                return std::nullopt;
            }
            auto pattern_id = pattern_ids.find(indexed->pixels);
            std::uint16_t tile_index = 0U;
            if (pattern_id == pattern_ids.end()) {
                if (pattern_ids.size() >= VdpModel::kPatternCount) {
                    return std::nullopt;
                }
                tile_index = static_cast<std::uint16_t>(pattern_ids.size());
                pattern_ids.emplace(indexed->pixels, tile_index);
                if (!vdp.set_pattern(tile_index, indexed->pixels)) {
                    return std::nullopt;
                }
            } else {
                tile_index = pattern_id->second;
            }
            const auto high = tile_is_high_priority(
                source, map, other_plane, overlay, origin_x, origin_y);
            const auto source_priority =
                source_priority_inverted ? !high : high;
            result.words[static_cast<std::size_t>(tile_y) *
                             result.width_tiles +
                         static_cast<std::size_t>(tile_x)] =
                VdpNameTableEntry{source_priority, indexed->palette, false,
                                  false, tile_index}.encode();
        }
    }
    return result;
}

bool check_streamer_contracts() {
    VdpModel vdp;
    std::vector<std::uint16_t> words(64U * 32U, 0x8001U);
    PlaneTilemapSource source{&words, 64U, 32U, 0U, 0U};
    FieldViewportState state{};
    state.normal_render_pending = 1U;
    state.alternate_render_pending = 1U;
    PlaneStreamer streamer(vdp);
    const auto normal_words = streamer.stream_normal_viewport(source, state);
    if (normal_words != 44U * 31U || state.normal_render_pending != 0U ||
        state.selected_tile_table != kFieldNormalTileTable ||
        state.selected_destination != state.plane_a_destination ||
        vdp.name_word(VdpPlane::plane_a, 0U, 0U) != 0x8001U) {
        return fail("0x2C92 normal stream did not fill the Plane-A window");
    }
    const auto alternate_words =
        streamer.stream_alternate_viewport(source, state);
    if (alternate_words != 44U * 31U ||
        state.alternate_render_pending != 0U || state.render_mask != 0U ||
        state.selected_tile_table != kFieldAlternateTileTable ||
        state.selected_destination != state.plane_b_destination ||
        vdp.name_word(VdpPlane::plane_b, 0U, 0U) != 0x0001U) {
        return fail("0x2CB2 must clear its flag and invert bit 15");
    }
    std::array<std::uint16_t, 16> metatile{};
    for (std::size_t index = 0U; index < metatile.size(); ++index) {
        metatile[index] = static_cast<std::uint16_t>(0x8000U | (index + 1U));
    }
    if (streamer.write_metatile(VdpPlane::plane_a, 12U, 4U, metatile) != 16U ||
        vdp.name_word(VdpPlane::plane_a, 12U, 4U) != 0x0001U ||
        vdp.name_word(VdpPlane::plane_a, 15U, 7U) != 0x0010U) {
        return fail("runtime metatile writes must invert priority across 4x4 words");
    }
    return true;
}

bool compare_scene(const int scene_id,
                   const std::filesystem::path& asset_root,
                   const std::vector<std::uint8_t>& rom) {
    const auto stem = "mode-" + std::to_string(scene_id);
    const auto map = QImage(QString::fromStdString(
        (asset_root / "field/maps" / (stem + ".bmp")).string()));
    const auto overlay = QImage(QString::fromStdString(
        (asset_root / "field/maps" /
         (stem + "-priority-overlay.png")).string()));
    const auto primary = QImage(QString::fromStdString(
        (asset_root / "field/maps" /
         (stem + "-primary-plane.png")).string()));
    const auto secondary = QImage(QString::fromStdString(
        (asset_root / "field/maps" /
         (stem + "-secondary-plane.png")).string()));
    const auto palette_offset = palette_offset_for_scene(asset_root, scene_id);
    if (map.isNull() || overlay.isNull() || primary.isNull() ||
        secondary.isNull() || !palette_offset.has_value() ||
        map.size() != overlay.size() || map.size() != primary.size() ||
        map.size() != secondary.size()) {
        return fail("missing or misaligned scene assets for mode " +
                    std::to_string(scene_id));
    }
    if (*palette_offset > rom.size() ||
        rom.size() - *palette_offset < VdpModel::kCramWordCount * 2U) {
        return fail("scene CRAM source is outside the US ROM");
    }

    VdpModel vdp;
    const auto palettes = make_rgb_palettes(rom, *palette_offset, vdp);
    std::map<VdpModel::Pattern, std::uint16_t> pattern_ids;
    VdpModel::Pattern blank{};
    pattern_ids.emplace(blank, 0U);
    vdp.set_pattern(0U, blank);
    auto plane_a = prepare_plane(primary, secondary, map, overlay, palettes,
                                 vdp, pattern_ids, false);
    // The alternate FF4700 source is written with FFDF58=$8000, so the
    // streamer toggles each pre-inverted priority bit before storing it.
    auto plane_b = prepare_plane(secondary, primary, map, overlay, palettes,
                                 vdp, pattern_ids, true);
    if (!plane_a.has_value() || !plane_b.has_value()) {
        return fail("scene " + std::to_string(scene_id) +
                    " could not be represented by the source CRAM and 0x800 patterns");
    }

    const auto scroll_x = static_cast<std::uint16_t>(
        ((map.width() - kSceneWidth) * 2) / 3);
    const auto scroll_y = static_cast<std::uint16_t>(
        ((map.height() - kSceneHeight) / 3));
    FieldViewportState viewport{};
    viewport.normal_render_pending = 1U;
    viewport.alternate_render_pending = 1U;
    PlaneStreamer streamer(vdp);
    const auto a_words = streamer.stream_normal_viewport(
        PlaneTilemapSource{&plane_a->words, plane_a->width_tiles,
                           plane_a->height_tiles, scroll_x, scroll_y},
        viewport);
    const auto b_words = streamer.stream_alternate_viewport(
        PlaneTilemapSource{&plane_b->words, plane_b->width_tiles,
                           plane_b->height_tiles, scroll_x, scroll_y},
        viewport);
    if (a_words != 44U * 31U || b_words != 44U * 31U) {
        return fail("scene " + std::to_string(scene_id) +
                    " did not stream the full 0x3D18 viewport window");
    }
    vdp.set_scroll({static_cast<std::int16_t>(scroll_x),
                    static_cast<std::int16_t>(scroll_y),
                    static_cast<std::int16_t>(scroll_x),
                    static_cast<std::int16_t>(scroll_y)});

    QImage actual;
    vdp.compose(actual);
    QImage expected(kSceneWidth, kSceneHeight,
                    QImage::Format_ARGB32_Premultiplied);
    expected.fill(Qt::black);
    QPainter painter(&expected);
    paint_field_map_composition(
        painter, map, overlay,
        QRect(scroll_x, scroll_y, kSceneWidth, kSceneHeight), {});
    painter.end();

    std::size_t mismatches = 0U;
    for (int y = 0; y < kSceneHeight; ++y) {
        for (int x = 0; x < kSceneWidth; ++x) {
            if (actual.pixel(x, y) != expected.pixel(x, y)) {
                ++mismatches;
            }
        }
    }
    const auto percent = static_cast<double>(mismatches) * 100.0 /
                         static_cast<double>(kScenePixelCount);
    std::cout << "scene=" << scene_id << " mismatches=" << mismatches << '/'
              << kScenePixelCount << " percent=" << percent << "%"
              << " patterns=" << pattern_ids.size() << '\n';
    if (mismatches > kMaximumMismatches) {
        return fail("scene " + std::to_string(scene_id) +
                    " exceeds the exact-parity bound; mismatch count=" +
                    std::to_string(mismatches));
    }
    return true;
}

} // namespace

int main() {
    const std::filesystem::path asset_root(SHADOWRUN_FIELD_MAP_ASSET_ROOT);
    const auto rom = read_file(SHADOWRUN_US_ROM_PATH);
    if (!rom.has_value()) {
        return fail("cannot read the US ROM") ? 0 : 1;
    }
    if (!check_streamer_contracts()) {
        return 1;
    }
    for (const auto scene_id : {0, 2, 26}) {
        if (!compare_scene(scene_id, asset_root, *rom)) {
            return 1;
        }
    }
    return 0;
}
