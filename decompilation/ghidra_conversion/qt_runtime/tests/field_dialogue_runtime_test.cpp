#include "field_dialogue_runtime.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QImage>
#include <QPainter>

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::qt_runtime;

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    const auto assets = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT));
    if (!expect(assets.complete())) {
        return 1;
    }

    std::string_view boundary;
    FieldDialogueRuntime runtime(
        [&boundary](const std::string_view reason) { boundary = reason; });
    if (!expect(runtime.load_assets(assets.catalog))) {
        return 2;
    }

    if (!expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::manager_greeting)) ||
        !expect(runtime.panel().actor_name == "Stoker") ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.panel().text_segment_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("manager yawns") !=
                std::string_view::npos)) {
        return 3;
    }

    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find("room") !=
                std::string_view::npos)) {
        return 4;
    }

    // The B button is the source's second option. The room-offer payment
    // branch is not yet fully connected, while leaving is a proven sentinel
    // page and therefore remains executable.
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(runtime.panel().returns_to_field)) {
        return 5;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 6;
    }

    const FieldExitPromptSelection arrival{
        FieldExitResourceProfile::stoker_motel,
        FieldExitPromptAsset::stoker_motel_arrival,
        0x14U};
    if (!expect(runtime.open_field_exit_prompt(arrival)) ||
        !expect(runtime.panel().has_more) ||
        !expect(runtime.panel().options[0].source_label.find("MORE") !=
                std::string_view::npos)) {
        return 7;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::source_boundary) ||
        !expect(runtime.panel().kind == DialoguePanelKind::source_boundary) ||
        !expect(boundary.find("no fully converted continuation") !=
                std::string_view::npos)) {
        return 8;
    }

    const FieldExitPromptSelection package_question{
        FieldExitResourceProfile::stoker_motel,
        FieldExitPromptAsset::stoker_motel_package_question,
        0x14U};
    if (!expect(runtime.open_field_exit_prompt(package_question)) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field)) {
        return 9;
    }

    const FieldExitPromptSelection unconverted_profile{
        FieldExitResourceProfile::scene_zero_submode_three,
        FieldExitPromptAsset::scene_zero_submode_three_default,
        0x4BU};
    if (!expect(!runtime.open_field_exit_prompt(unconverted_profile)) ||
        !expect(runtime.panel().kind == DialoguePanelKind::source_boundary)) {
        return 10;
    }

    if (!expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::manager_greeting_with_package)) ||
        !expect(runtime.panel().text_segment_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("manager yawns") !=
                std::string_view::npos)) {
        return 11;
    }
    QImage scene(320, 224, QImage::Format_RGB32);
    scene.fill(Qt::black);
    QPainter painter(&scene);
    runtime.paint(painter);
    painter.end();
    int lit_pixels = 0;
    for (int y = 0; y < scene.height(); ++y) {
        for (int x = 0; x < scene.width(); ++x) {
            const auto pixel = scene.pixelColor(x, y);
            if (pixel.red() != 0 || pixel.green() != 0 || pixel.blue() != 0) {
                ++lit_pixels;
            }
        }
    }
    if (!expect(lit_pixels > 1000)) {
        return 12;
    }
    return 0;
}
