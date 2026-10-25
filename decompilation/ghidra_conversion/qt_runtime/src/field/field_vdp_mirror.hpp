#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "vdp_model.hpp"

#include <QImage>
#include <QRect>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

// Optional field-scene mirror for R2/R9 QA. The normal field renderer keeps
// using the flattened images; this class preserves the extracted plane
// images as indexed patterns and streams the camera window through
// PlaneStreamer so runtime cell writes can be checked against those images.
class FieldVdpMirror final {
public:
    bool rebuild(const ConversionAssetCatalog& catalog,
                 std::uint16_t scene_reference, const QImage& field_map,
                 const QImage& priority_overlay,
                 std::uint16_t camera_x_fixed,
                 std::uint16_t camera_y_fixed,
                 std::int16_t plane_b_vertical_offset);

    bool update_camera(std::uint16_t camera_x_fixed,
                       std::uint16_t camera_y_fixed,
                       std::int16_t plane_b_vertical_offset);
    bool write_metatile_cell(std::size_t target_cell,
                             std::size_t donor_cell,
                             std::size_t cell_columns);

    const VdpModel& model() const noexcept { return model_; }
    QRect viewport_rect() const noexcept {
        return QRect(viewport_x_, viewport_y_, VdpModel::kDisplayWidth,
                     VdpModel::kDisplayHeight);
    }

private:
    VdpModel model_;
    std::vector<std::uint16_t> plane_a_words_;
    // Plane B's extracted words carry the pre-inverted FFDF58 priority bit.
    std::vector<std::uint16_t> plane_b_words_;
    std::size_t width_tiles_ = 0U;
    std::size_t height_tiles_ = 0U;
    std::size_t cell_columns_ = 0U;
    int viewport_x_ = 0;
    int viewport_y_ = 0;
    std::int16_t plane_b_vertical_offset_ = 0;

    bool stream_viewport();
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
