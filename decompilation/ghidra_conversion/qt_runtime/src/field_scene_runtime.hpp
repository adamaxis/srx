#pragma once

#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_bootstrap_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/world/world_entity_conversion.hpp"

#include <QImage>

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

// Composition-only adapter for the separate Ghidra-conversion Qt executable.
// It owns no ROM view and does not duplicate converted field state. The
// conversion bootstrap remains authoritative; this class supplies host
// presentation callbacks and paints the direct source assets it receives.
class FieldSceneRuntime final {
public:
    explicit FieldSceneRuntime(
        std::function<void(std::string_view)> source_boundary);

    bool bootstrap(
        const integration::StartupIntegrationState& startup_state);
    void advance_frame();
    void paint(QPainter& painter) const;

    bool ready() const noexcept { return ready_; }
    bool field_entity_visible() const noexcept {
        return field_entity_.visible;
    }
    const std::string& failure() const noexcept { return failure_; }
    const integration::FieldSessionBootstrapState& state() const noexcept {
        return state_;
    }

private:
    void configure_callbacks();
    void source_boundary(std::string_view boundary) const;
    bool load_image(const ConversionAssetCatalog& catalog,
                    std::string_view alias,
                    QImage& destination);
    bool load_sprite_image(const ConversionAssetCatalog& catalog,
                           std::string_view alias,
                           QImage& destination,
                           int& origin_x,
                           int& origin_y);
    void paint_field_entity(QPainter& painter) const;

    struct FieldEntityPresentation {
        bool active = false;
        SceneRunnerPosition world_position{};
        EntityProjectionState projection{};
        std::uint8_t direction = 0;
        bool visible = false;
    };

    std::function<void(std::string_view)> source_boundary_callback_;
    integration::FieldSessionBootstrapInput input_;
    integration::FieldSessionBootstrapState state_;
    integration::FieldSessionBootstrapCallbacks callbacks_;
    WorldFrameState world_frame_state_;
    WorldFrameCallbacks world_frame_callbacks_;
    FieldViewportState viewport_state_;
    QImage field_map_;
    QImage field_hud_;
    std::array<QImage, 8> field_entity_idle_images_{};
    std::array<int, 8> field_entity_idle_origin_x_{};
    std::array<int, 8> field_entity_idle_origin_y_{};
    FieldEntityPresentation field_entity_{};
    std::string failure_;
    bool ready_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
