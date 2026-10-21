#include "field_mode_text_runtime.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <QColor>
#include <QImage>
#include <QPainter>

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::qt_runtime;

bool expect(const bool condition) { return condition; }

bool has_green_ink(const QImage& image, const int x, const int y,
                   const int width, const int height) {
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            const auto pixel = image.pixelColor(column, row);
            if (pixel.green() > 150 && pixel.red() < 100 &&
                pixel.blue() < 140) {
                return true;
            }
        }
    }
    return false;
}

bool region_is_color(const QImage& image, const int x, const int y,
                     const int width, const int height,
                     const QColor expected) {
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            if (image.pixelColor(column, row) != expected) {
                return false;
            }
        }
    }
    return true;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion::scripting;

    const auto assets = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT));
    if (!expect(assets.complete())) {
        return 1;
    }

    GameScriptRuntime scripts;
    if (!expect(scripts.load_package(
                    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
                    std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                        "dialogue" / "script-text.json"))) {
        return 2;
    }

    FieldModeTextRuntime runtime;
    runtime.set_script_runtime(&scripts);
    if (!expect(runtime.load_assets(assets.catalog))) {
        return 3;
    }
    if (!expect(runtime.open(ScriptFieldModeContext{2U, 11U, 0})) ||
        !expect(runtime.open()) ||
        !expect(runtime.frames_remaining() == 30U)) {
        return 4;
    }

    const auto& intro = runtime.panel();
    if (!expect(intro.actor_kind == DialoguePanelActorKind::none) ||
        !expect(intro.actor_name.empty()) ||
        !expect(intro.portrait_alias ==
                "field.mode.entry.ares.sprite") ||
        !expect(intro.option_count == 0U) ||
        !expect(intro.text_segment_count >= 5U) ||
        !expect(intro.text_segments[1].empty()) ||
        !expect(intro.text_segments[0].find("Ares logo") !=
                std::string_view::npos)) {
        return 5;
    }

    QImage image(320, 224, QImage::Format_RGB32);
    image.fill(Qt::black);
    QPainter painter(&image);
    runtime.paint(painter);
    painter.end();
    if (!expect(has_green_ink(image, 88, 16, 216, 120)) ||
        !expect(!region_is_color(image, 16, 32, 56, 72,
                                 QColor(0, 0, 0))) ||
        !expect(region_is_color(image, 16, 152, 296, 72,
                                QColor(0, 0, 0)))) {
        return 6;
    }

    // The source sentinel waits a full 30 frames. Button edges during the
    // delay do not dismiss it; any fresh high-byte edge after the delay does.
    if (!expect(runtime.advance_controller_events(0x4000U, 29U) ==
                FieldModeTextAdvanceResult::waiting) ||
        !expect(runtime.open()) || !expect(runtime.frames_remaining() == 1U) ||
        !expect(runtime.advance_controller_events(0U, 1U) ==
                FieldModeTextAdvanceResult::waiting) ||
        !expect(runtime.open()) || !expect(runtime.frames_remaining() == 0U) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldModeTextAdvanceResult::returned_to_route) ||
        !expect(!runtime.open()) || !expect(!runtime.panel().open)) {
        return 7;
    }

    if (!expect(runtime.open(ScriptFieldModeContext{2U, 11U, -1})) ||
        !expect(runtime.panel().text_segments[4].find("fake name") !=
                std::string_view::npos)) {
        return 8;
    }
    runtime.close();
    if (!expect(runtime.open(ScriptFieldModeContext{2U, 11U, 1})) ||
        !expect(runtime.panel().text_segments[0].find("receptionist") !=
                std::string_view::npos)) {
        return 9;
    }
    runtime.close();
    if (!expect(runtime.open_if_matched(
                    ScriptFieldModeContext{2U, 1U, 0}) ==
                FieldModeTextOpenResult::opened) ||
        !expect(runtime.open()) ||
        !expect(runtime.frames_remaining() == 30U) ||
        !expect(runtime.panel().portrait_alias ==
                "field.mode.entry.little.chiba.sprite") ||
        !expect(runtime.panel().text_segment_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("Little Chiba's") !=
                std::string_view::npos) ||
        !expect(runtime.panel().text_segments[2].find("cyberware") !=
                std::string_view::npos)) {
        return 10;
    }
    image.fill(Qt::black);
    QPainter little_chiba_painter(&image);
    runtime.paint(little_chiba_painter);
    little_chiba_painter.end();
    if (!expect(has_green_ink(image, 88, 16, 216, 120)) ||
        !expect(!region_is_color(image, 16, 32, 56, 72, QColor(0, 0, 0)))) {
        return 11;
    }
    runtime.close();
    if (!expect(runtime.open(ScriptFieldModeContext{2U, 1U, -1})) ||
        !expect(runtime.panel().text_segments[0].find("bulky package") !=
                std::string_view::npos)) {
        return 12;
    }
    runtime.close();
    if (!expect(runtime.open(ScriptFieldModeContext{2U, 1U, 1})) ||
        !expect(runtime.panel().text_segments[0].find("eyes light up") !=
                std::string_view::npos)) {
        return 13;
    }
    runtime.close();
    if (!expect(!runtime.open(ScriptFieldModeContext{2U, 2U, 0})) ||
        !expect(!runtime.open()) || !expect(runtime.failure().empty())) {
        return 14;
    }
    return 0;
}
