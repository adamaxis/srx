#include "field_dialogue_runtime.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QImage>
#include <QPainter>

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::qt_runtime;

constexpr std::array<std::string_view, 13> kContactIds{{
    "capt_quinton_jaymes", "alesandro_hobbs", "sharkey", "kipp_david",
    "wilma_temmenhoff", "macklemann", "gregory_wilns",
    "mr_faradouchi", "agira_tetsumi", "henry_j_culver", "max",
    "chillicut", "professor_jefferson",
}};

bool expect(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

bool save_panel(FieldDialogueRuntime& runtime,
                const std::filesystem::path& root,
                const std::string_view name) {
    const auto directory = root / name;
    std::filesystem::create_directories(directory);
    QImage image(320, 224, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    QPainter painter(&image);
    runtime.paint(painter);
    painter.end();
    return image.save(QString::fromStdWString(
                          (directory / "actual.bmp").wstring()),
                      "BMP");
}

bool contact_cursor_is_visible(FieldDialogueRuntime& runtime) {
    QImage image(320, 224, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    QPainter painter(&image);
    runtime.paint(painter);
    painter.end();

    for (int y = 16; y < 24; ++y) {
        for (int x = 80; x < 88; ++x) {
            const auto pixel = image.pixelColor(x, y);
            if (pixel.green() > 180 && pixel.red() < 80 &&
                pixel.blue() < 100) {
                return true;
            }
        }
    }
    return false;
}

bool advance(FieldDialogueRuntime& runtime, const std::uint16_t input,
             const std::string_view expected_message) {
    const auto result = runtime.advance_controller_events(input);
    return expect(result == FieldDialogueAdvanceResult::prompt_advanced,
                  expected_message);
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: shadowrun_vid_phone_visual_test output-directory\n";
        return 64;
    }
    const std::filesystem::path output_root(argv[1]);

    const auto assets = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT));
    if (!expect(assets.complete(), "Vid-Phone asset catalog is incomplete")) {
        for (const auto& error : assets.errors) {
            std::cerr << error << '\n';
        }
        return 1;
    }

    std::string source_boundary;
    FieldDialogueRuntime runtime(
        [&source_boundary](const std::string_view reason) {
            source_boundary.assign(reason);
        });
    scripting::GameScriptRuntime scripts;
    if (!expect(scripts.load_package(
                    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
                    std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                        "dialogue" / "script-text.lua"),
                "Vid-Phone Lua package did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 2;
    }
    for (const auto contact_id : kContactIds) {
        const auto profile = scripts.speaker_profile(
            "vid_phone_" + std::string(contact_id));
        if (!expect(profile.has_value(),
                    "Vid-Phone contact has no speaker profile") ||
            !expect(assets.catalog.find(profile->portrait_alias) != nullptr,
                    "Vid-Phone contact portrait is missing from the asset catalog")) {
            return 11;
        }
    }
    runtime.set_script_runtime(&scripts);
    if (!expect(runtime.load_assets(assets.catalog),
                "Vid-Phone panel assets did not load")) {
        std::cerr << runtime.failure() << '\n';
        return 3;
    }

    scripts.set_location_context(
        "field_map", "scene_2", "field.main_menu", "");
    scripts.set_story_state({{"vid_phone_contacts_resolved", true}});
    if (!expect(runtime.open_gameplay_task(
                    "field_activities", "ordinary_field_terminal"),
                "ordinary field terminal did not open") ||
        !expect(runtime.panel().option_count == 4U,
                "ordinary field terminal does not show its four choices") ||
        !expect(save_panel(runtime, output_root, "01-field-menu"),
                "could not save ordinary terminal screen")) {
        std::cerr << runtime.failure() << '\n';
        return 4;
    }

    if (!advance(runtime, 0x0200U,
                 "Down did not select the Vid-Phone row") ||
        !expect(runtime.panel().selected_option == 1U,
                "Vid-Phone row selection is not visible") ||
        !expect(save_panel(runtime, output_root,
                           "02-field-menu-vid-phone-selected"),
                "could not save selected terminal row") ||
        !advance(runtime, 0x4000U,
                 "A did not open the Vid-Phone choice") ||
        !expect(runtime.panel().text_segment_count != 0U,
                "empty phonebook did not display its source text") ||
        !expect(save_panel(runtime, output_root, "03-no-contacts"),
                "could not save empty contact list screen") ||
        !advance(runtime, 0x4000U,
                 "A did not return from the no-contact message")) {
        std::cerr << runtime.failure() << '\n';
        return 5;
    }

    scripting::StoryFacts contact_state{
        {"vid_phone_contacts_resolved", true},
    };
    for (unsigned index = 0U; index < 13U; ++index) {
        contact_state.emplace("vid_phone_contact_" + std::to_string(index),
                              true);
    }
    scripts.set_story_state(std::move(contact_state));
    if (!advance(runtime, 0x0200U,
                 "Down did not return selection to Vid-Phone") ||
        !advance(runtime, 0x4000U,
                 "A did not open contact list page one") ||
        !expect(runtime.panel().compact_cursor_list,
                "contact list did not use its compact cursor layout") ||
        !expect(contact_cursor_is_visible(runtime),
                "selected contact cursor is hidden behind the panel border") ||
        !expect(runtime.panel().option_count == 8U,
                "contact page one does not show seven contacts and Next") ||
        !expect(save_panel(runtime, output_root, "04-contact-page-1"),
                "could not save contact page one")) {
        std::cerr << runtime.failure() << '\n';
        return 6;
    }
    if (!expect(runtime.advance_controller_events(0x2000U) ==
                    FieldDialogueAdvanceResult::waiting_for_controller,
                "C incorrectly changed the contact list page") ||
        !expect(runtime.panel().option_count == 8U,
                "ignored C input changed the contact list choices")) {
        std::cerr << runtime.failure() << '\n';
        return 6;
    }
    for (unsigned row = 0U; row < 7U; ++row) {
        if (!advance(runtime, 0x0200U,
                     "Down did not select the Next Screen row")) {
            std::cerr << runtime.failure() << '\n';
            return 6;
        }
    }
    if (!advance(runtime, 0x1000U,
                 "B did not activate the Next Screen row") ||
        !expect(runtime.panel().option_count == 7U,
                "contact page two does not show six contacts and Prior") ||
        !expect(runtime.panel().selected_option == 0U,
                "contact page two cursor did not reset to its first row") ||
        !expect(contact_cursor_is_visible(runtime),
                "page two selected contact cursor is not visible") ||
        !expect(save_panel(runtime, output_root, "05-contact-page-2"),
                "could not save contact page two")) {
        std::cerr << runtime.failure() << '\n';
        return 6;
    }

    if (!advance(runtime, 0x0200U, "Down did not select Agira") ||
        !advance(runtime, 0x0200U, "Down did not select Henry") ||
        !advance(runtime, 0x4000U, "A did not open Henry's first call") ||
        !expect(runtime.panel().portrait_alias ==
                    "dialogue.portraits.vid.phone.henry.j.culver",
                "Henry call did not select his source portrait") ||
        !expect(runtime.panel().option_count == 2U,
                "Henry call did not show Services and Goodbye") ||
        !expect(save_panel(runtime, output_root, "06-henry-first-call"),
                "could not save Henry call screen")) {
        std::cerr << runtime.failure() << '\n';
        return 7;
    }

    if (!advance(runtime, 0x1000U,
                 "B did not exit Henry's call to the contact list") ||
        !advance(runtime, 0x0200U,
                 "Down did not select the third contact row") ||
        !advance(runtime, 0x0200U,
                 "Down did not select the fourth contact row") ||
        !advance(runtime, 0x0200U,
                 "Down did not select the fifth contact row") ||
        !advance(runtime, 0x0200U,
                 "Down did not select the sixth contact row") ||
        !advance(runtime, 0x1000U,
                 "B did not activate Prior Screen") ||
        !advance(runtime, 0x4000U,
                 "A did not call Captain Quinton") ||
        !expect(runtime.panel().portrait_alias ==
                    "dialogue.portraits.vid.phone.capt.quinton.jaymes",
                "Captain Quinton call did not draw his portrait") ||
        !expect(save_panel(runtime, output_root, "07-captain-first-call"),
                "could not save first Captain Quinton call")) {
        std::cerr << runtime.failure() << '\n';
        return 8;
    }
    if (!advance(runtime, 0x1000U,
                 "B did not return from the first Captain Quinton call") ||
        !advance(runtime, 0x4000U,
                 "A did not repeat the Captain Quinton call") ||
        !expect(runtime.panel().portrait_alias ==
                    "dialogue.portraits.vid.phone.capt.quinton.jaymes",
                "repeat call lost Captain Quinton's portrait") ||
        !expect(save_panel(runtime, output_root, "08-captain-repeat-call"),
                "could not save repeat Captain Quinton call")) {
        std::cerr << runtime.failure() << '\n';
        return 9;
    }
    if (!advance(runtime, 0x1000U,
                 "B did not return from the repeat call") ||
        !advance(runtime, 0x8000U,
                 "Start did not return from contacts to the field menu") ||
        !expect(runtime.advance_controller_events(0x8000U) ==
                    FieldDialogueAdvanceResult::returned_to_field,
                "Start did not leave the ordinary field terminal") ||
        !expect(!runtime.open(), "Vid-Phone panel did not clear on exit")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 10;
    }
    return 0;
}
