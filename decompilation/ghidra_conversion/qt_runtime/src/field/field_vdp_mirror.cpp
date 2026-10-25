#include "field_vdp_mirror.hpp"

#include "genesis_image_compositor.hpp"
#include "plane_streamer.hpp"

#include <QColor>
#include <QString>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

using ColorSet = std::vector<QRgb>;
using PaletteColorSlots = std::array<std::map<QRgb, std::uint8_t>, 4U>;
using PaletteColorSets = std::array<std::set<QRgb>, 4U>;

constexpr std::uint16_t kNameWordPriorityMask = 0x8000U;
constexpr std::size_t kCellTileWidth = 4U;
constexpr std::size_t kCellTileHeight = 4U;
constexpr std::size_t kCellTileCount = kCellTileWidth * kCellTileHeight;

QRgb opaque_rgb(const QRgb pixel) noexcept {
    return qRgb(qRed(pixel), qGreen(pixel), qBlue(pixel));
}

ColorSet tile_color_set(const QImage& source, const int origin_x,
                        const int origin_y) {
    std::set<QRgb> colors;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const auto pixel = source.pixel(origin_x + x, origin_y + y);
            if (qAlpha(pixel) != 0) {
                colors.insert(opaque_rgb(pixel));
            }
        }
    }
    return ColorSet(colors.begin(), colors.end());
}

bool collect_tile_color_sets(const QImage& source,
                             std::set<ColorSet>& unique_sets) {
    if (source.isNull() || source.width() % 8 != 0 ||
        source.height() % 8 != 0) {
        return false;
    }
    for (int y = 0; y < source.height(); y += 8) {
        for (int x = 0; x < source.width(); x += 8) {
            auto colors = tile_color_set(source, x, y);
            if (colors.size() > 15U) {
                return false;
            }
            if (!colors.empty()) {
                unique_sets.emplace(std::move(colors));
            }
        }
    }
    return true;
}

bool make_palette_assignment(const std::set<ColorSet>& unique_sets,
                             PaletteColorSets& palette_colors,
                             PaletteColorSlots& palette_slots,
                             std::map<ColorSet, std::uint8_t>& tile_palettes) {
    std::vector<ColorSet> ordered_sets;
    ordered_sets.reserve(unique_sets.size());
    for (const auto& colors : unique_sets) {
        ordered_sets.push_back(colors);
    }
    std::sort(ordered_sets.begin(), ordered_sets.end(),
              [](const ColorSet& lhs, const ColorSet& rhs) {
                  if (lhs.size() != rhs.size()) {
                      return lhs.size() > rhs.size();
                  }
                  return lhs < rhs;
              });

    for (const auto& colors : ordered_sets) {
        std::optional<std::size_t> selected_palette;
        std::size_t selected_overlap = 0U;
        std::size_t selected_size = std::numeric_limits<std::size_t>::max();
        for (std::size_t palette = 0U; palette < palette_colors.size();
             ++palette) {
            std::size_t overlap = 0U;
            for (const auto color : colors) {
                overlap += palette_colors[palette].count(color);
            }
            const auto combined_size = palette_colors[palette].size() +
                                      colors.size() - overlap;
            if (combined_size > 15U) {
                continue;
            }
            if (!selected_palette.has_value() ||
                overlap > selected_overlap ||
                (overlap == selected_overlap &&
                 combined_size < selected_size)) {
                selected_palette = palette;
                selected_overlap = overlap;
                selected_size = combined_size;
            }
        }
        if (!selected_palette.has_value()) {
            return false;
        }
        auto& group = palette_colors[*selected_palette];
        group.insert(colors.begin(), colors.end());
        tile_palettes.emplace(colors,
                              static_cast<std::uint8_t>(*selected_palette));
    }

    for (std::size_t palette = 0U; palette < palette_colors.size(); ++palette) {
        std::uint8_t color_index = 1U;
        for (const auto color : palette_colors[palette]) {
            const auto red = static_cast<std::uint16_t>(
                (static_cast<unsigned>(qRed(color)) * 7U + 127U) / 255U);
            const auto green = static_cast<std::uint16_t>(
                (static_cast<unsigned>(qGreen(color)) * 7U + 127U) / 255U);
            const auto blue = static_cast<std::uint16_t>(
                (static_cast<unsigned>(qBlue(color)) * 7U + 127U) / 255U);
            const auto cram_word = static_cast<std::uint16_t>(
                (red << 1U) | (green << 5U) | (blue << 9U));
            if (genesis_cram_word_to_color(cram_word).rgb() != color) {
                return false;
            }
            palette_slots[palette].emplace(color, color_index);
            ++color_index;
        }
    }
    return true;
}

bool tile_is_high_priority(const QImage& plane, const QImage& map,
                           const QImage& other_plane,
                           const QImage& overlay, const int origin_x,
                           const int origin_y) {
    // The offline export has the composed image and priority overlay, not the
    // original name words. Infer priority only from pixels that distinguish
    // this extracted plane from the other plane [H], matching the parity
    // harness.
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const auto plane_pixel = plane.pixel(origin_x + x, origin_y + y);
            const auto other_pixel =
                other_plane.pixel(origin_x + x, origin_y + y);
            const auto overlay_pixel = overlay.pixel(origin_x + x,
                                                     origin_y + y);
            if (qAlpha(plane_pixel) == 0 || qAlpha(overlay_pixel) == 0) {
                continue;
            }
            const auto selected_pixel = opaque_rgb(map.pixel(origin_x + x,
                                                             origin_y + y));
            const auto source_pixel = opaque_rgb(plane_pixel);
            const auto other_is_transparent = qAlpha(other_pixel) == 0;
            if (selected_pixel == source_pixel &&
                (other_is_transparent ||
                 selected_pixel != opaque_rgb(other_pixel))) {
                return true;
            }
        }
    }
    return false;
}

std::uint16_t pattern_word(
    const QImage& source, const int origin_x, const int origin_y,
    const std::map<ColorSet, std::uint8_t>& tile_palettes,
    const PaletteColorSlots& palette_slots,
    std::map<VdpModel::Pattern, std::uint16_t>& pattern_ids,
    VdpModel& model) {
    const auto colors = tile_color_set(source, origin_x, origin_y);
    const auto palette_found = tile_palettes.find(colors);
    const auto palette = palette_found == tile_palettes.end()
        ? std::uint8_t{0U}
        : palette_found->second;
    VdpModel::Pattern pattern{};
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const auto pixel = source.pixel(origin_x + x, origin_y + y);
            if (qAlpha(pixel) == 0) {
                continue;
            }
            const auto found = palette_slots[palette].find(opaque_rgb(pixel));
            if (found == palette_slots[palette].end()) {
                return std::numeric_limits<std::uint16_t>::max();
            }
            pattern[static_cast<std::size_t>(y * 8 + x)] = found->second;
        }
    }

    const auto known = pattern_ids.find(pattern);
    std::uint16_t pattern_id = 0U;
    if (known != pattern_ids.end()) {
        pattern_id = known->second;
    } else {
        if (pattern_ids.size() >= VdpModel::kPatternCount) {
            return std::numeric_limits<std::uint16_t>::max();
        }
        pattern_id = static_cast<std::uint16_t>(pattern_ids.size());
        pattern_ids.emplace(pattern, pattern_id);
        if (!model.set_pattern(pattern_id, pattern)) {
            return std::numeric_limits<std::uint16_t>::max();
        }
    }
    return VdpNameTableEntry{false, palette, false, false,
                             pattern_id}.encode();
}

bool build_plane_words(
    const QImage& source, const QImage& other_plane, const QImage& map,
    const QImage& overlay,
    const std::map<ColorSet, std::uint8_t>& tile_palettes,
    const PaletteColorSlots& palette_slots,
    std::map<VdpModel::Pattern, std::uint16_t>& pattern_ids,
    VdpModel& model, const bool plane_b,
    std::vector<std::uint16_t>& words) {
    words.resize(static_cast<std::size_t>(source.width() / 8) *
                 static_cast<std::size_t>(source.height() / 8));
    const auto width_tiles = static_cast<std::size_t>(source.width() / 8);
    for (int y = 0; y < source.height(); y += 8) {
        for (int x = 0; x < source.width(); x += 8) {
            auto word = pattern_word(source, x, y, tile_palettes,
                                     palette_slots, pattern_ids, model);
            if (word == std::numeric_limits<std::uint16_t>::max()) {
                return false;
            }
            const bool high = tile_is_high_priority(
                source, map, other_plane, overlay, x, y);
            // FFDF58 toggles bit 15 at 0x3DAE/0x3E4A [C]. Store Plane B's
            // pre-inverted source word so PlaneStreamer produces the inferred
            // final priority bit on both planes.
            const bool source_priority = plane_b ? !high : high;
            auto entry = VdpNameTableEntry::decode(word);
            entry.priority = source_priority;
            const auto index = static_cast<std::size_t>(y / 8) * width_tiles +
                               static_cast<std::size_t>(x / 8);
            words[index] = entry.encode();
        }
    }
    return true;
}

} // namespace

bool FieldVdpMirror::rebuild(const ConversionAssetCatalog& catalog,
                              const std::uint16_t scene_reference,
                              const QImage& field_map,
                              const QImage& priority_overlay,
                              const std::uint16_t camera_x_fixed,
                              const std::uint16_t camera_y_fixed,
                              const std::int16_t plane_b_vertical_offset) {
    const auto stem = std::string("mode-") +
                      std::to_string(scene_reference);
    const auto map_root = catalog.asset_root / "field" / "maps";
    const QImage primary_raw(QString::fromStdString(
        (map_root / (stem + "-primary-plane.png")).string()));
    const QImage secondary_raw(QString::fromStdString(
        (map_root / (stem + "-secondary-plane.png")).string()));
    if (field_map.isNull() || priority_overlay.isNull() ||
        primary_raw.isNull() || secondary_raw.isNull() ||
        field_map.size() != priority_overlay.size() ||
        field_map.size() != primary_raw.size() ||
        field_map.size() != secondary_raw.size() ||
        field_map.width() % 32 != 0 || field_map.height() % 32 != 0) {
        return false;
    }

    const auto map = field_map.convertToFormat(QImage::Format_ARGB32);
    const auto overlay =
        priority_overlay.convertToFormat(QImage::Format_ARGB32);
    const auto primary = primary_raw.convertToFormat(QImage::Format_ARGB32);
    const auto secondary = secondary_raw.convertToFormat(QImage::Format_ARGB32);

    std::set<ColorSet> unique_sets;
    if (!collect_tile_color_sets(primary, unique_sets) ||
        !collect_tile_color_sets(secondary, unique_sets)) {
        return false;
    }
    PaletteColorSets palette_colors{};
    PaletteColorSlots palette_slots{};
    std::map<ColorSet, std::uint8_t> tile_palettes;
    if (!make_palette_assignment(unique_sets, palette_colors,
                                 palette_slots, tile_palettes)) {
        return false;
    }

    model_.clear();
    for (std::size_t palette = 0U; palette < palette_colors.size(); ++palette) {
        for (const auto color : palette_colors[palette]) {
            const auto color_index = palette_slots[palette].at(color);
            const auto red = static_cast<std::uint16_t>(
                (static_cast<unsigned>(qRed(color)) * 7U + 127U) / 255U);
            const auto green = static_cast<std::uint16_t>(
                (static_cast<unsigned>(qGreen(color)) * 7U + 127U) / 255U);
            const auto blue = static_cast<std::uint16_t>(
                (static_cast<unsigned>(qBlue(color)) * 7U + 127U) / 255U);
            const auto cram_word = static_cast<std::uint16_t>(
                (red << 1U) | (green << 5U) | (blue << 9U));
            if (!model_.set_cram_word(static_cast<std::uint8_t>(palette),
                                      color_index, cram_word)) {
                return false;
            }
        }
    }

    std::map<VdpModel::Pattern, std::uint16_t> pattern_ids;
    VdpModel::Pattern blank{};
    pattern_ids.emplace(blank, 0U);
    if (!model_.set_pattern(0U, blank) ||
        !build_plane_words(primary, secondary, map, overlay, tile_palettes,
                           palette_slots, pattern_ids, model_, false,
                           plane_a_words_) ||
        !build_plane_words(secondary, primary, map, overlay, tile_palettes,
                           palette_slots, pattern_ids, model_, true,
                           plane_b_words_)) {
        return false;
    }

    width_tiles_ = static_cast<std::size_t>(primary.width() / 8);
    height_tiles_ = static_cast<std::size_t>(primary.height() / 8);
    cell_columns_ = static_cast<std::size_t>(field_map.width() / 32);
    return update_camera(camera_x_fixed, camera_y_fixed,
                         plane_b_vertical_offset);
}

bool FieldVdpMirror::update_camera(
    const std::uint16_t camera_x_fixed,
    const std::uint16_t camera_y_fixed,
    const std::int16_t plane_b_vertical_offset) {
    if (width_tiles_ == 0U || height_tiles_ == 0U) {
        return false;
    }
    const auto camera_x = std::max(0, static_cast<int>(camera_x_fixed >> 4U));
    const auto camera_y = std::max(0, static_cast<int>(camera_y_fixed >> 4U));
    const auto map_width = static_cast<int>(width_tiles_ * 8U);
    const auto map_height = static_cast<int>(height_tiles_ * 8U);
    viewport_x_ = std::min(camera_x,
                           std::max(0, map_width - VdpModel::kDisplayWidth));
    viewport_y_ = std::min(camera_y,
                           std::max(0, map_height - VdpModel::kDisplayHeight));
    plane_b_vertical_offset_ = plane_b_vertical_offset;
    return stream_viewport();
}

bool FieldVdpMirror::stream_viewport() {
    if (plane_a_words_.empty() || plane_b_words_.empty() ||
        width_tiles_ == 0U || height_tiles_ == 0U) {
        return false;
    }
    const PlaneTilemapSource plane_a{
        &plane_a_words_, width_tiles_, height_tiles_,
        static_cast<std::uint16_t>(viewport_x_),
        static_cast<std::uint16_t>(viewport_y_)};
    const PlaneTilemapSource plane_b{
        &plane_b_words_, width_tiles_, height_tiles_,
        static_cast<std::uint16_t>(viewport_x_),
        static_cast<std::uint16_t>(viewport_y_)};
    FieldViewportState viewport{};
    viewport.normal_render_pending = 1U;
    viewport.alternate_render_pending = 1U;
    PlaneStreamer streamer(model_);
    constexpr auto kVisibleNameWordCount = 44U * 31U;
    if (streamer.stream_normal_viewport(plane_a, viewport) !=
            kVisibleNameWordCount ||
        streamer.stream_alternate_viewport(plane_b, viewport) !=
            kVisibleNameWordCount) {
        return false;
    }

    const auto horizontal = static_cast<std::int16_t>(viewport_x_ & 0x01FF);
    const auto vertical = static_cast<std::int16_t>(viewport_y_ & 0x00FF);
    const auto plane_b_vertical = static_cast<std::int16_t>(
        (viewport_y_ + plane_b_vertical_offset_) & 0x00FF);
    // ROM 0x1AB0 copies FFE00A to FFDD0E (0x1AA8 zeroes FFE00A first), and
    // 0x3CF2..0x3CFE add the same camera step to FFDD0A and FFDD0E [C]. The
    // port tracks those steps in vertical_bias_a/b; their difference is the
    // Plane-B offset. In the field it is 0: 23 emulator field states show
    // FFDD0E - FFDD0A == 0 [C, emulator]. The 0x26D0/0x26E2 increment belongs
    // to the 0x1A9E title loop, outside the field path [C].
    model_.set_scroll({horizontal, vertical, horizontal, plane_b_vertical});
    return true;
}

bool FieldVdpMirror::write_metatile_cell(
    const std::size_t target_cell, const std::size_t donor_cell,
    const std::size_t cell_columns) {
    const auto cell_count = cell_columns_ *
        (height_tiles_ / kCellTileHeight);
    if (cell_columns == 0U || cell_columns != cell_columns_ ||
        target_cell >= cell_count || donor_cell >= cell_count ||
        target_cell == donor_cell ||
        plane_a_words_.size() != width_tiles_ * height_tiles_ ||
        plane_b_words_.size() != width_tiles_ * height_tiles_) {
        return false;
    }

    const auto target_x = (target_cell % cell_columns) * kCellTileWidth;
    const auto target_y = (target_cell / cell_columns) * kCellTileHeight;
    const auto donor_x = (donor_cell % cell_columns) * kCellTileWidth;
    const auto donor_y = (donor_cell / cell_columns) * kCellTileHeight;
    std::array<std::uint16_t, kCellTileCount> plane_a_metatile{};
    std::array<std::uint16_t, kCellTileCount> plane_b_metatile{};
    for (std::size_t y = 0U; y < kCellTileHeight; ++y) {
        for (std::size_t x = 0U; x < kCellTileWidth; ++x) {
            const auto target_index = (target_y + y) * width_tiles_ +
                                      target_x + x;
            const auto donor_index = (donor_y + y) * width_tiles_ + donor_x + x;
            const auto plane_a_word = plane_a_words_[donor_index];
            const auto plane_b_word = plane_b_words_[donor_index];
            // Runtime writers wrap FFDF58 around each source stamp [C].
            plane_a_metatile[y * kCellTileWidth + x] =
                static_cast<std::uint16_t>(plane_a_word ^
                                           kNameWordPriorityMask);
            plane_b_metatile[y * kCellTileWidth + x] = plane_b_word;
            plane_a_words_[target_index] = plane_a_word;
            plane_b_words_[target_index] = plane_b_word;
        }
    }

    PlaneStreamer streamer(model_);
    const auto plane_a_written = streamer.write_metatile(
        VdpPlane::plane_a, target_x, target_y, plane_a_metatile);
    const auto plane_b_written = streamer.write_metatile(
        VdpPlane::plane_b, target_x, target_y, plane_b_metatile);
    if (plane_a_written != kCellTileCount ||
        plane_b_written != kCellTileCount) {
        return false;
    }
    // Re-stream the active camera window after the direct stamp so off-screen
    // cells cannot alias a different visible ring slot in the 64x32 table.
    return stream_viewport();
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
