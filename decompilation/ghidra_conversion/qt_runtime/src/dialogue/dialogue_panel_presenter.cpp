#include "dialogue_panel_presenter.hpp"

#include <QPainter>
#include <QRect>
#include <QSize>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

bool DialoguePanelPresenter::load_assets(
    const ConversionAssetCatalog& assets) {
    failure_.clear();
    portraits_.clear();
    story_page_assets_.clear();

    constexpr std::string_view kPortraitPrefix = "dialogue.portraits.";
    constexpr std::string_view kFieldModePortraitPrefix =
        "field.mode.entry.";
    constexpr std::array<std::string_view, 2> kFrameAliases{{
        "dialogue.panel.frame",
        "dialogue.panel.frame.alternate.left.panel",
    }};
    constexpr std::array<std::string_view, 3> kOptionAliases{{
        "dialogue.panel.button.a",
        "dialogue.panel.button.b",
        "dialogue.panel.button.c",
    }};
    for (std::size_t index = 0; index < kFrameAliases.size(); ++index) {
        const auto* record = assets.find(kFrameAliases[index]);
        if (record == nullptr || record->kind != "image") {
            failure_ = "shared dialogue frame asset is unavailable";
            return false;
        }
        panel_frames_[index] = QImage(QString::fromStdWString(
            record->absolute_path.wstring()))
                                   .convertToFormat(
                                       QImage::Format_ARGB32_Premultiplied);
        if (panel_frames_[index].isNull() ||
            panel_frames_[index].size() != QSize(320, 224)) {
            failure_ = "shared dialogue frame asset is unreadable";
            return false;
        }
    }

    for (std::size_t index = 0; index < kOptionAliases.size(); ++index) {
        const auto* record = assets.find(kOptionAliases[index]);
        if (record == nullptr || record->kind != "image") {
            failure_ = "dialogue option marker asset is unavailable";
            return false;
        }
        option_markers_[index] = QImage(QString::fromStdWString(
            record->absolute_path.wstring()))
                                     .convertToFormat(
                                         QImage::Format_ARGB32_Premultiplied);
        if (option_markers_[index].isNull() ||
            option_markers_[index].size() != QSize(16, 16)) {
            failure_ = "dialogue option marker asset is unreadable";
            return false;
        }
    }

    const auto* cursor_record = assets.find("menu.field.hotel.cursor");
    if (cursor_record == nullptr || cursor_record->kind != "image") {
        failure_ = "source menu cursor asset is unavailable";
        return false;
    }
    cursor_marker_ = QImage(QString::fromStdWString(
        cursor_record->absolute_path.wstring()))
                            .convertToFormat(
                                QImage::Format_ARGB32_Premultiplied);
    if (cursor_marker_.isNull() || cursor_marker_.size() != QSize(7, 8)) {
        failure_ = "source menu cursor asset is unreadable";
        return false;
    }

    const auto* font_record = assets.find("menu.field.font");
    if (font_record == nullptr || font_record->kind != "image") {
        failure_ = "shared dialogue font asset is unavailable";
        return false;
    }
    font_ = QImage(QString::fromStdWString(font_record->absolute_path.wstring()))
                .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (font_.isNull() || font_.width() < 8 || font_.height() < 8) {
        failure_ = "shared dialogue font asset is unreadable";
        return false;
    }

    for (const auto& record : assets.records) {
        const auto alias = std::string_view(record.alias);
        const bool is_dialogue_portrait =
            alias.size() >= kPortraitPrefix.size() &&
            alias.substr(0U, kPortraitPrefix.size()) == kPortraitPrefix;
        const bool is_mode_profile =
            alias.size() >= kFieldModePortraitPrefix.size() &&
            alias.substr(0U, kFieldModePortraitPrefix.size()) ==
                kFieldModePortraitPrefix;
        if (record.kind != "image" ||
            (!is_dialogue_portrait && !is_mode_profile)) {
            continue;
        }
        QImage portrait(QString::fromStdWString(
            record.absolute_path.wstring()));
        if (!portrait.isNull()) {
            portraits_.emplace(
                record.alias,
                portrait.convertToFormat(QImage::Format_ARGB32_Premultiplied));
        }
    }
    constexpr std::string_view kStoryPagePrefix = "cinematic.";
    for (const auto& record : assets.records) {
        const auto alias = std::string_view(record.alias);
        if (record.kind != "image" ||
            alias.size() < kStoryPagePrefix.size() ||
            alias.substr(0U, kStoryPagePrefix.size()) != kStoryPagePrefix) {
            continue;
        }
        QImage image(QString::fromStdWString(
            record.absolute_path.wstring()));
        if (image.isNull() || image.size() != QSize(320, 224)) {
            continue;
        }
        story_page_assets_.emplace(
            record.alias,
            image.convertToFormat(QImage::Format_ARGB32_Premultiplied));
    }
    return true;
}

bool DialoguePanelPresenter::has_story_page_asset(
    const std::string_view asset_alias) const {
    return story_page_assets_.find(std::string(asset_alias)) !=
           story_page_assets_.end();
}

void DialoguePanelPresenter::draw_pixel_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text, const int max_columns,
    const int max_lines, const QColor ink) const {
    if (font_.isNull() || font_.width() < 8 || font_.height() < 8) {
        return;
    }
    const auto bounded = std::min<std::size_t>(
        text.size(), static_cast<std::size_t>(std::max(0, max_columns)));
    auto cursor = x;
    for (std::size_t index = 0U; index < bounded; ++index) {
        const auto code = static_cast<unsigned char>(text[index]);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 > font_.width() || source_y + 8 > font_.height()) {
            cursor += 8;
            continue;
        }
        QImage tinted_glyph(8, 8, QImage::Format_ARGB32_Premultiplied);
        tinted_glyph.fill(Qt::transparent);
        for (int glyph_y = 0; glyph_y < 8; ++glyph_y) {
            for (int glyph_x = 0; glyph_x < 8; ++glyph_x) {
                const auto source_pixel =
                    font_.pixelColor(source_x + glyph_x, source_y + glyph_y);
                const auto coverage = std::max(
                    {source_pixel.red(), source_pixel.green(),
                     source_pixel.blue()});
                if (source_pixel.alpha() == 0 || coverage == 0) {
                    continue;
                }
                const auto alpha = static_cast<int>(
                    (static_cast<std::uint32_t>(source_pixel.alpha()) *
                     static_cast<std::uint32_t>(coverage)) /
                    255U);
                tinted_glyph.setPixelColor(
                    glyph_x, glyph_y,
                    QColor(ink.red(), ink.green(), ink.blue(), alpha));
            }
        }
        painter.drawImage(cursor, y, tinted_glyph);
        cursor += 8;
    }
    (void)max_lines;
}

int DialoguePanelPresenter::draw_wrapped_segment(
    QPainter& painter, const int x, const int y, const std::string_view text,
    const int max_columns, const int max_lines, const QColor ink) const {
    if (text.empty()) {
        return 1;
    }

    std::size_t cursor = 0U;
    int line = 0;
    while (cursor < text.size() && line < max_lines) {
        if (line != 0) {
            while (cursor < text.size() && text[cursor] == ' ') {
                ++cursor;
            }
        }
        if (cursor >= text.size()) {
            break;
        }
        const auto remaining = text.substr(cursor);
        auto length = std::min<std::size_t>(
            remaining.size(), static_cast<std::size_t>(max_columns));
        if (length < remaining.size()) {
            const auto break_at = remaining.rfind(' ', length);
            if (break_at != std::string_view::npos && break_at != 0U) {
                length = break_at;
            }
        }
        draw_pixel_text(painter, x, y + line * 8,
                        remaining.substr(0U, length), max_columns, 1, ink);
        cursor += length;
        ++line;
        while (cursor < text.size() && text[cursor] == ' ') {
            ++cursor;
        }
    }
    return line;
}

void DialoguePanelPresenter::paint(QPainter& painter,
                                   const DialoguePanelView& panel) const {
    if (!panel.open) {
        return;
    }

    const QColor dialogue_green(0, 255, 0);
    const auto frame_index = panel.frame_variant ==
                                     DialoguePanelFrameVariant::alternate_left_panel
                                 ? 1U
                                 : 0U;
    painter.fillRect(0, 0, 320, 224, Qt::black);
    painter.drawImage(0, 0, panel_frames_[frame_index]);

    // 0x1F5A8 clears both Plane-A content wells after the shared Plane-B
    // frame is installed. Repeating those black fills also guarantees each
    // changed page removes the previous text and choice rows before drawing.
    painter.fillRect(QRect(88, 16, 216, 120), Qt::black);
    painter.fillRect(QRect(16, 152, 296, 72), Qt::black);

    if (!panel.portrait_alias.empty()) {
        const auto portrait = portraits_.find(
            std::string(panel.portrait_alias));
        if (portrait != portraits_.end()) {
            painter.drawImage(16, 32, portrait->second);
        }
    }

    int text_line = 0;
    for (std::size_t index = 0U; index < panel.text_segment_count &&
                                  text_line < 15;
         ++index) {
        text_line += draw_wrapped_segment(
            painter, 88, 16 + text_line * 8, panel.text_segments[index],
            26, 15 - text_line, dialogue_green);
    }

    if (panel.kind == DialoguePanelKind::source_boundary) {
        draw_wrapped_segment(painter, 88, 16, panel.source_boundary_reason,
                             26, 15, dialogue_green);
        return;
    }

    if (panel.has_more && !panel.continuation_label.empty()) {
        painter.drawImage(16, 152, option_markers_[0]);
        draw_wrapped_segment(painter, 48, 159, panel.continuation_label, 32,
                             2, dialogue_green);
        return;
    }

    // Button rows (A/B/C markers, 24px pitch from y=152) only exist for the
    // three ROM buttons. Anything longer is drawn as a cursor list so a
    // data-driven choice count can never index past option_markers_.
    const bool cursor_rows = panel.cursor_selection ||
                             panel.option_count > option_markers_.size();
    const auto row_count = std::min(panel.option_count, panel.options.size());
    for (std::size_t index = 0U; index < row_count; ++index) {
        const auto row_y = panel.compact_cursor_list
            ? 16 + static_cast<int>(index * 16U)
            : 152 + static_cast<int>(index *
                (cursor_rows && row_count > option_markers_.size() ? 16U
                                                                   : 24U));
        const auto cursor_x = panel.compact_cursor_list ? 80 : 24;
        const auto label_x = panel.compact_cursor_list ? 96 : 48;
        if (cursor_rows) {
            if (index == panel.selected_option) {
                // ROM 0xC376 writes the selected-row cursor tile. Use its
                // extracted 0x5FF sprite instead of drawing a text glyph.
                painter.drawImage(cursor_x, row_y, cursor_marker_);
            }
        } else {
            painter.drawImage(16, row_y, option_markers_[index]);
        }
        draw_wrapped_segment(painter, label_x, row_y,
                             panel.options[index].source_label,
                             panel.compact_cursor_list ? 26U : 32U, 2,
                             dialogue_green);
    }
}

void DialoguePanelPresenter::paint_story_page(
    QPainter& painter, const std::string_view asset_alias,
    const std::string_view text, const int text_x, const int text_y,
    const int text_columns, const int text_rows, const QColor text_ink) const {
    const auto asset = story_page_assets_.find(std::string(asset_alias));
    if (asset == story_page_assets_.end()) {
        return;
    }
    painter.fillRect(QRect(0, 0, 320, 224), Qt::black);
    painter.drawImage(0, 0, asset->second);

    int line = 0;
    std::size_t start = 0U;
    while (start <= text.size() && line < text_rows) {
        const auto separator = text.find('|', start);
        const auto end = separator == std::string_view::npos
                             ? text.size()
                             : separator;
        const auto used_lines = draw_wrapped_segment(
            painter, text_x, text_y + line * 8,
            text.substr(start, end - start), text_columns,
            text_rows - line, text_ink);
        line += std::max(used_lines, 1);
        if (separator == std::string_view::npos) {
            break;
        }
        start = separator + 1U;
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
