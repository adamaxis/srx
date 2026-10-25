#include "plane_streamer.hpp"

#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint16_t kNameWordPriorityMask = 0x8000U;
constexpr std::uint16_t kNameTableRowBytes =
    static_cast<std::uint16_t>(VdpModel::kNameTableWidth * 2U);

std::uint16_t name_word_at(const PlaneTilemapSource& source,
                           const std::size_t x, const std::size_t y) {
    if (source.name_words == nullptr || x >= source.width_tiles ||
        y >= source.height_tiles ||
        y > (source.name_words->size() / source.width_tiles)) {
        return 0U;
    }
    const auto index = y * source.width_tiles + x;
    return index < source.name_words->size() ? (*source.name_words)[index]
                                             : 0U;
}

std::uint16_t wrapped_destination(const PlaneTilemapSource& source) {
    const auto source_tile_x = static_cast<std::size_t>(
        source.source_x_pixels / 8U);
    const auto source_tile_y = static_cast<std::size_t>(
        source.source_y_pixels / 8U);
    return static_cast<std::uint16_t>(
        ((source_tile_y % VdpModel::kNameTableHeight) * kNameTableRowBytes) +
        ((source_tile_x % VdpModel::kNameTableWidth) * 2U));
}

} // namespace

std::size_t PlaneStreamer::stream_normal_viewport(
    const PlaneTilemapSource& source, FieldViewportState& state) {
    std::size_t words_written = 0U;
    render_normal_field_viewport(state, [&] {
        words_written = stream_selected_plane(
            source, state, VdpPlane::plane_a, state.render_mask);
    });
    return words_written;
}

std::size_t PlaneStreamer::stream_alternate_viewport(
    const PlaneTilemapSource& source, FieldViewportState& state) {
    std::size_t words_written = 0U;
    render_alternate_field_viewport(state, [&] {
        words_written = stream_selected_plane(
            source, state, VdpPlane::plane_b, state.render_mask);
    });
    return words_written;
}

std::size_t PlaneStreamer::stream_selected_plane(
    const PlaneTilemapSource& source, const FieldViewportState& state,
    const VdpPlane plane, const std::uint16_t priority_mask) {
    if (source.name_words == nullptr || source.width_tiles == 0U ||
        source.height_tiles == 0U ||
        source.width_tiles > source.name_words->size() / source.height_tiles) {
        return 0U;
    }

    const auto source_origin_x = static_cast<std::size_t>(
        source.source_x_pixels / 8U);
    const auto source_origin_y = static_cast<std::size_t>(
        source.source_y_pixels / 8U);
    FieldTilePlaneFillState fill{};
    fill.destination = wrapped_destination(source);
    fill.plane_offset = state.selected_plane_offset;
    std::size_t words_written = 0U;

    const auto copy_segment = [&](const std::size_t count,
                                  std::size_t source_x,
                                  std::size_t destination_x,
                                  const std::size_t source_y,
                                  const std::size_t destination_y) {
        if (count == 0U) {
            return std::size_t{0U};
        }
        const auto initial_tile_key = static_cast<std::uint16_t>(
            ((source_x / 4U) * 2U) & 0xFFFFU);
        const auto initial_word_offset = (source_x % 4U) * 2U;
        std::size_t written = 0U;
        TileWordStreamCallbacks callbacks;
        callbacks.read_word = [&](const std::uint16_t tile_key,
                                  const std::size_t word_offset) {
            const auto group_x = static_cast<std::size_t>(tile_key / 2U) * 4U;
            const auto within_group = word_offset / 2U;
            return name_word_at(source, group_x + within_group, source_y);
        };
        callbacks.write_word = [&](const std::uint16_t word) {
            vdp_.set_name_word(plane,
                               (destination_x + written) %
                                   VdpModel::kNameTableWidth,
                               destination_y % VdpModel::kNameTableHeight,
                               word);
            ++written;
        };
        return emit_field_tile_column_words(
            count, initial_tile_key, initial_word_offset, priority_mask,
            callbacks);
    };

    render_field_tile_plane_rows(
        fill,
        {{},
         [&] { return wrapped_destination(source); },
         [&](const std::uint16_t destination, const std::uint16_t y) {
             const auto plan = plan_field_tile_column(destination);
             const auto destination_x =
                 static_cast<std::size_t>((destination & 0x007FU) / 2U);
             const auto destination_y =
                 static_cast<std::size_t>((destination >> 7U) & 0x001FU);
             const auto source_y = source_origin_y +
                                   static_cast<std::size_t>(y / 8U);
             const auto first_count = copy_segment(
                 plan.first_word_count, source_origin_x, destination_x,
                 source_y, destination_y);
             const auto second_count = copy_segment(
                 plan.second_word_count, source_origin_x + first_count,
                 static_cast<std::size_t>(
                     (plan.wrapped_destination & 0x007FU) / 2U),
                 source_y, destination_y);
             words_written += first_count + second_count;
             // The source helper renders 31 successive 8px rows in one
             // 0x3D18 pass; its Y parameter is the tile-row displacement.
         }});
    return words_written;
}

std::size_t PlaneStreamer::write_metatile(
    const VdpPlane plane, const std::size_t destination_x,
    const std::size_t destination_y,
    const std::array<std::uint16_t, 16>& words) {
    std::size_t written_total = 0U;
    for (std::size_t row = 0U; row < 4U; ++row) {
        std::size_t write_x = 0U;
        const TileWordStreamCallbacks callbacks{
            {},
            [&, row](const std::uint16_t tile_key,
                     const std::size_t word_offset) {
                const auto source_x =
                    static_cast<std::size_t>(tile_key / 2U) * 4U +
                    word_offset / 2U;
                return words[row * 4U + source_x];
            },
            [&](const std::uint16_t word) {
                vdp_.set_name_word(
                    plane, (destination_x + write_x) %
                               VdpModel::kNameTableWidth,
                    (destination_y + row) % VdpModel::kNameTableHeight,
                    word);
                ++write_x;
            }};
        written_total += emit_field_tile_row_words(
            4U, 0U, 0U, kNameWordPriorityMask, callbacks);
    }
    return written_total;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
