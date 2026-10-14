#pragma once

#include "dialogue_panel.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QColor>
#include <QImage>

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

// Stateless shared renderer for any already-resolved dialogue view. Speaker,
// event, text, portrait, choices, and frame mode come from the game controller.
class DialoguePanelPresenter final {
public:
    bool load_assets(const ConversionAssetCatalog& assets);
    const std::string& failure() const noexcept { return failure_; }
    void paint(QPainter& painter, const DialoguePanelView& panel) const;

private:
    void draw_pixel_text(QPainter& painter, int x, int y,
                         std::string_view text, int max_columns,
                         int max_lines, QColor ink) const;
    int draw_wrapped_segment(QPainter& painter, int x, int y,
                             std::string_view text, int max_columns,
                             int max_lines, QColor ink) const;

    std::array<QImage, 2> panel_frames_{};
    std::array<QImage, 3> option_markers_{};
    std::unordered_map<std::string, QImage> portraits_;
    QImage font_;
    std::string failure_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
