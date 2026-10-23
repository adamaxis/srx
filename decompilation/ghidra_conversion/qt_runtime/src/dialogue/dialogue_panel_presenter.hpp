#pragma once

#include "dialogue_panel.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QColor>
#include <QImage>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

// Stateless shared renderer for any already-resolved dialogue view. Speaker,
// event, text, portrait, choices, and frame mode come from the game controller.
class DialoguePanelPresenter final {
public:
    bool load_assets(const ConversionAssetCatalog& assets);
    const std::string& failure() const noexcept { return failure_; }
    void paint(QPainter& painter, const DialoguePanelView& panel) const;
    bool has_story_page_asset(std::string_view asset_alias) const;
    void paint_story_page(QPainter& painter, std::string_view asset_alias,
                          std::string_view text, int text_x, int text_y,
                          int text_columns, int text_rows,
                          QColor text_ink) const;
    bool has_credits_page(std::string_view asset_alias,
                          std::string_view page_id) const;
    void paint_credits_page(QPainter& painter, std::string_view asset_alias,
                            std::string_view page_id) const;

private:
    void draw_pixel_text(QPainter& painter, int x, int y,
                         std::string_view text, int max_columns,
                         int max_lines, QColor ink) const;
    int draw_wrapped_segment(QPainter& painter, int x, int y,
                             std::string_view text, int max_columns,
                             int max_lines, QColor ink) const;

    struct CreditsPage {
        std::string id;
        std::uint8_t top_row_offset = 0U;
        std::vector<std::string> lines;
    };
    struct CreditsAsset {
        int origin_x = 0;
        int origin_y = 0;
        int columns = 0;
        QColor ink;
        std::vector<CreditsPage> pages;
    };

    std::array<QImage, 2> panel_frames_{};
    std::array<QImage, 3> option_markers_{};
    QImage cursor_marker_;
    std::unordered_map<std::string, QImage> portraits_;
    std::unordered_map<std::string, QImage> story_page_assets_;
    std::unordered_map<std::string, CreditsAsset> credits_assets_;
    QImage font_;
    std::string failure_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
