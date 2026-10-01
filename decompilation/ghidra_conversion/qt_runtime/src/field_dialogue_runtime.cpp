#include "field_dialogue_runtime.hpp"

#include "genesis_image_compositor.hpp"

#include <QColor>
#include <QPainter>
#include <QRect>

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint16_t kControllerAEvent = 0x4000U;
constexpr std::uint16_t kControllerBEvent = 0x1000U;
constexpr std::uint16_t kControllerCEvent = 0x2000U;
constexpr std::uint16_t kControllerStartEvent = 0x8000U;
constexpr std::uint16_t kAnyDialogueEvent = kControllerAEvent |
                                             kControllerBEvent |
                                             kControllerCEvent |
                                             kControllerStartEvent;

std::optional<StokerMotelPromptId> stoker_prompt_for_asset(
    const FieldExitPromptAsset asset) {
    switch (asset) {
    case FieldExitPromptAsset::stoker_motel_arrival:
        return StokerMotelPromptId::arrival;
    case FieldExitPromptAsset::stoker_motel_package_question:
        return StokerMotelPromptId::package_question;
    case FieldExitPromptAsset::stoker_motel_package_handoff:
        return StokerMotelPromptId::package_handoff;
    default:
        return std::nullopt;
    }
}

QColor source_panel_dark() { return QColor(48, 48, 48); }
QColor source_panel_mid() { return QColor(112, 112, 112); }
QColor source_panel_light() { return QColor(184, 184, 184); }

} // namespace

FieldDialogueRuntime::FieldDialogueRuntime(
    std::function<void(std::string_view)> source_boundary)
    : source_boundary_callback_(std::move(source_boundary)) {}

bool FieldDialogueRuntime::load_assets(const ConversionAssetCatalog& assets) {
    failure_.clear();
    const auto text = load_dialogue_text_catalog(assets, "dialogue.stoker.motel");
    text_catalog_ = text.catalog;
    if (!text.complete()) {
        failure_ = text.errors.empty() ? "Stoker dialogue text is unavailable"
                                       : text.errors.front();
        assets_loaded_ = false;
        if (source_boundary_callback_) {
            source_boundary_callback_("field.dialogue.text_assets_missing");
        }
        return false;
    }

    const auto* portrait_record =
        assets.find("dialogue.portraits.stoker.motel");
    const auto* font_record = assets.find("menu.field.font");
    if (portrait_record == nullptr || font_record == nullptr) {
        failure_ = "Stoker dialogue portrait or font asset is unavailable";
        assets_loaded_ = false;
        if (source_boundary_callback_) {
            source_boundary_callback_("field.dialogue.image_assets_missing");
        }
        return false;
    }

    portrait_ = QImage(QString::fromStdWString(
                           portrait_record->absolute_path.wstring()))
                     .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    font_ = QImage(QString::fromStdWString(font_record->absolute_path.wstring()))
                .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (portrait_.isNull() || font_.isNull()) {
        failure_ = "Stoker dialogue image assets could not be loaded";
        assets_loaded_ = false;
        if (source_boundary_callback_) {
            source_boundary_callback_("field.dialogue.image_assets_unreadable");
        }
        return false;
    }

    make_genesis_sprite_color_zero_transparent(portrait_);
    assets_loaded_ = true;
    return true;
}

bool FieldDialogueRuntime::open_field_exit_prompt(
    const FieldExitPromptSelection& selection) {
    if (selection.resource_profile != FieldExitResourceProfile::stoker_motel) {
        return open_source_boundary(
            "field exit selected a prompt profile without a converted dialogue asset");
    }
    const auto prompt = stoker_prompt_for_asset(selection.prompt_asset);
    if (!prompt.has_value()) {
        return open_source_boundary(
            "Stoker field-exit selection has no mapped prompt record");
    }
    return open_stoker_prompt(*prompt);
}

bool FieldDialogueRuntime::open_stoker_prompt_for_dump(
    const StokerMotelPromptId prompt) {
    return open_stoker_prompt(prompt);
}

bool FieldDialogueRuntime::open_stoker_prompt(
    const StokerMotelPromptId prompt) {
    if (!assets_loaded_) {
        return open_source_boundary(
            failure_.empty() ? "Stoker dialogue assets are not loaded"
                             : failure_);
    }
    boundary_reason_.clear();
    active_prompt_ = prompt;
    const auto source_view = make_stoker_motel_prompt(prompt);
    const DialogueTextResolver resolver = [this](const std::string_view alias) {
        return resolve_text(alias);
    };
    panel_ = make_dialogue_panel_view(source_view, resolver);
    if (!panel_.source_complete && panel_.source_boundary_reason.empty()) {
        panel_.source_boundary_reason =
            "Stoker dialogue source projection is incomplete";
    }
    return true;
}

bool FieldDialogueRuntime::open_source_boundary(std::string reason) {
    boundary_reason_ = std::move(reason);
    panel_ = DialoguePanelView{};
    panel_.open = true;
    panel_.actor_kind = DialoguePanelActorKind::field_npc;
    panel_.actor_name = "";
    panel_.kind = DialoguePanelKind::source_boundary;
    panel_.source_complete = false;
    panel_.source_boundary_reason = boundary_reason_;
    active_prompt_ = StokerMotelPromptId::source_boundary;
    if (source_boundary_callback_) {
        source_boundary_callback_(panel_.source_boundary_reason);
    }
    return false;
}

std::string_view FieldDialogueRuntime::resolve_text(
    const std::string_view alias) const {
    const auto* value = text_catalog_.find(alias);
    return value == nullptr ? std::string_view{} : std::string_view(*value);
}

FieldDialogueAdvanceResult FieldDialogueRuntime::advance_controller_events(
    const std::uint16_t event_word) {
    if (!panel_.open || (event_word & kAnyDialogueEvent) == 0U) {
        return FieldDialogueAdvanceResult::waiting_for_controller;
    }
    if (!panel_.source_complete) {
        return FieldDialogueAdvanceResult::source_boundary;
    }

    // 0x1F3C4 tests A first, then B and C. A title-only source record uses
    // the shared wait-for-controller path and returns to field after any
    // button edge; it is not a hidden fourth option.
    if (panel_.option_count == 0U) {
        close();
        return FieldDialogueAdvanceResult::returned_to_field;
    }

    std::size_t option_index = 0U;
    if ((event_word & kControllerAEvent) != 0U) {
        option_index = 0U;
    } else if ((event_word & kControllerBEvent) != 0U) {
        option_index = 1U;
    } else if ((event_word & kControllerCEvent) != 0U) {
        option_index = 2U;
    }
    if (option_index >= panel_.option_count) {
        return FieldDialogueAdvanceResult::source_boundary;
    }

    panel_.selected_option = option_index;
    const auto target = stoker_motel_prompt_target(active_prompt_, option_index);
    if (!target.has_value()) {
        open_source_boundary(
            "selected Stoker command has no fully converted continuation");
        return FieldDialogueAdvanceResult::source_boundary;
    }
    if (*target == StokerMotelPromptId::source_boundary) {
        open_source_boundary(
            "selected Stoker command returns to an unconverted source record");
        return FieldDialogueAdvanceResult::source_boundary;
    }
    if (!open_stoker_prompt(*target)) {
        return FieldDialogueAdvanceResult::source_boundary;
    }
    return FieldDialogueAdvanceResult::prompt_advanced;
}

void FieldDialogueRuntime::close() noexcept {
    panel_ = DialoguePanelView{};
    active_prompt_ = StokerMotelPromptId::source_boundary;
    boundary_reason_.clear();
}

void FieldDialogueRuntime::draw_beveled_panel(QPainter& painter, const int x,
                                              const int y, const int width,
                                              const int height) const {
    painter.fillRect(QRect(x, y, width, height), source_panel_dark());
    painter.setPen(source_panel_light());
    painter.drawLine(x, y, x + width - 4, y);
    painter.drawLine(x, y, x, y + height - 4);
    painter.setPen(source_panel_mid());
    painter.drawLine(x + 2, y + 2, x + width - 6, y + 2);
    painter.drawLine(x + 2, y + 2, x + 2, y + height - 6);
    painter.setPen(QColor(16, 16, 16));
    painter.drawLine(x + 3, y + height - 1, x + width - 1, y + height - 1);
    painter.drawLine(x + width - 1, y + 3, x + width - 1, y + height - 1);
    painter.fillRect(QRect(x + 5, y + 5, width - 10, height - 10),
                     Qt::black);
}

void FieldDialogueRuntime::draw_pixel_text(QPainter& painter, const int x,
                                            const int y,
                                            const std::string_view text,
                                            const int max_columns,
                                            const int max_lines) const {
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
                    glyph_x, glyph_y, QColor(0, 255, 0, alpha));
            }
        }
        painter.drawImage(cursor, y, tinted_glyph);
        cursor += 8;
    }
    (void)max_lines;
}

int FieldDialogueRuntime::draw_wrapped_segment(
    QPainter& painter, const int x, const int y, const std::string_view text,
    const int max_columns, const int max_lines) const {
    std::size_t cursor = 0U;
    int line = 0;
    while (cursor < text.size() && line < max_lines) {
        while (cursor < text.size() && text[cursor] == ' ') {
            ++cursor;
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
                        remaining.substr(0U, length), max_columns, 1);
        cursor += length;
        ++line;
        while (cursor < text.size() && text[cursor] == ' ') {
            ++cursor;
        }
    }
    return line;
}

void FieldDialogueRuntime::paint(QPainter& painter) const {
    if (!panel_.open) {
        return;
    }

    painter.fillRect(0, 0, 320, 224, Qt::black);
    // The source performs two repeated menu-grid writes before the text loop.
    // These host panels retain the same three-region geometry observed in the
    // independent Genesis capture while keeping the shell as ordinary Qt
    // presentation, not as a bitmap flipbook.
    draw_beveled_panel(painter, 8, 8, 68, 136);
    draw_beveled_panel(painter, 76, 8, 236, 136);
    draw_beveled_panel(painter, 8, 145, 304, 74);

    if (panel_.actor_kind == DialoguePanelActorKind::field_npc &&
        !portrait_.isNull()) {
        painter.drawImage(16, 20, portrait_);
    }

    int text_line = 0;
    for (std::size_t index = 0U; index < panel_.text_segment_count &&
                                  text_line < 15;
         ++index) {
        text_line += draw_wrapped_segment(
            painter, 82, 16 + text_line * 8, panel_.text_segments[index],
            28, 15 - text_line);
    }

    if (panel_.kind == DialoguePanelKind::source_boundary) {
        draw_wrapped_segment(painter, 82, 16, panel_.source_boundary_reason,
                             28, 15);
        return;
    }

    constexpr std::array<char, 3> kButtonLabels{'A', 'B', 'C'};
    for (std::size_t index = 0U; index < panel_.option_count && index < 3U;
         ++index) {
        const auto y = 151 + static_cast<int>(index * 23U);
        painter.setPen(source_panel_light());
        painter.drawRect(QRect(17, y, 11, 11));
        draw_pixel_text(painter, 19, y + 2,
                        std::string_view(&kButtonLabels[index], 1), 1, 1);
        draw_wrapped_segment(painter, 48, y,
                             panel_.options[index].source_label, 32, 2);
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
