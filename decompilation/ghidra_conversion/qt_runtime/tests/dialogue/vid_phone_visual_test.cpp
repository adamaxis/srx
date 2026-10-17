#include "field_dialogue_runtime.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/vid_phone_service_conversion.hpp"

#include <QImage>
#include <QPainter>

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
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

QImage render_panel(FieldDialogueRuntime& runtime) {
    QImage image(320, 224, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    QPainter painter(&image);
    runtime.paint(painter);
    painter.end();
    return image;
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

bool select_phone_option(FieldDialogueRuntime& runtime) {
    for (std::size_t attempt = 0U; attempt < 4U; ++attempt) {
        if (runtime.panel().selected_option == 1U) {
            return true;
        }
        if (runtime.advance_controller_events(0x0200U) !=
            FieldDialogueAdvanceResult::prompt_advanced) {
            return false;
        }
    }
    return runtime.panel().selected_option == 1U;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: shadowrun_vid_phone_visual_test output-directory\n";
        return 64;
    }
    const std::filesystem::path output_root(argv[1]);
    std::filesystem::remove_all(output_root);
    std::filesystem::create_directories(output_root);

    const auto assets = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT));
    if (!expect(assets.complete(), "Vid-Phone asset catalog is incomplete")) {
        for (const auto& error : assets.errors) {
            std::cerr << error << '\n';
        }
        return 1;
    }
    const auto* frag_item =
        find_us_item_definition(UsItemCode::frag_grenade);
    const auto* hk227_item =
        find_us_item_definition(UsItemCode::hk227_s_smg);
    const auto hk227_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_hk227_s);
    if (!expect(frag_item != nullptr && hk227_item != nullptr &&
                    hk227_offer.has_value(),
                "Quinton service items did not resolve from the US catalog")) {
        return 1;
    }

    std::string source_boundary;
    FieldDialogueRuntime runtime(
        [&source_boundary](const std::string_view reason) {
            source_boundary.assign(reason);
        });
    std::optional<std::uint32_t> phone_funds = 3200U;
    std::optional<std::uint32_t> phone_contacts_flags = 0U;
    std::array<std::uint8_t, 8> phone_item_codes{};
    std::array<std::uint8_t, 8> phone_item_phases{};
    runtime.set_native_script_service_callback(
        [&phone_funds, &phone_contacts_flags, &phone_item_codes,
         &phone_item_phases](const std::string_view service_id)
            -> std::optional<std::string> {
            const auto action = vid_phone_service_action_for_id(service_id);
            if (!action.has_value()) {
                return std::nullopt;
            }
            const auto result = apply_vid_phone_service(
                *action, phone_funds, phone_item_codes, phone_item_phases,
                phone_contacts_flags);
            const auto script_result = vid_phone_service_script_result(result);
            return script_result.has_value()
                       ? std::optional<std::string>{
                             std::string(*script_result)}
                       : std::nullopt;
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
    const auto henry_call_screen = render_panel(runtime);
    const auto henry_portrait_sample = henry_call_screen.pixelColor(40, 56);
    if (!expect(henry_portrait_sample.red() > 80 &&
                    henry_portrait_sample.green() < 60 &&
                    henry_portrait_sample.blue() < 40,
                "Henry's recovered portrait pixels are absent from the call screen")) {
        return 7;
    }

    if (!advance(runtime, 0x1000U,
                 "B did not return from Henry's call to the field terminal") ||
        !expect(runtime.panel().option_count == 4U,
                "ending the call did not restore the four field choices") ||
        !expect(save_panel(runtime, output_root, "07-return-to-field-menu"),
                "could not save the field terminal after the call")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 8;
    }

    if (!select_phone_option(runtime)) {
        std::cerr << "could not select Vid-Phone from the field terminal\n";
        return 9;
    }
    if (!advance(runtime, 0x4000U,
                 "A did not reopen the phone contact list") ||
        !advance(runtime, 0x4000U,
                 "A did not open Captain Quinton's call") ||
        !expect(runtime.panel().actor_name == "Capt Quinton Jaymes" &&
                    runtime.panel().option_count == 2U,
                "Captain Quinton call screen has the wrong speaker or choices") ||
        !save_panel(runtime, output_root, "08-quinton-first-call")) {
        std::cerr << runtime.failure() << '\n';
        return 9;
    }
    if (!advance(runtime, 0x4000U,
                 "A did not open Captain Quinton's Services menu") ||
        !expect(runtime.panel().option_count == 3U &&
                    runtime.panel().actor_name == "Capt Quinton Jaymes" &&
                    runtime.panel().portrait_alias ==
                        "dialogue.portraits.vid.phone.capt.quinton.jaymes",
                "Quinton Services menu did not redraw with his profile") ||
        !save_panel(runtime, output_root, "09-quinton-services")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 10;
    }

    if (!advance(runtime, 0x4000U,
                 "A did not open Quinton's Hollywood information") ||
        !expect(runtime.panel().text_segment_count > 0U &&
                    runtime.panel().option_count == 1U,
                "Hollywood introduction did not replace the Services panel") ||
        !save_panel(runtime, output_root, "10-quinton-hollywood-intro") ||
        !advance(runtime, 0x4000U,
                 "A did not open Quinton's Hollywood topics") ||
        !expect(runtime.panel().option_count == 3U,
                "Hollywood topic menu did not show its three source choices") ||
        !save_panel(runtime, output_root, "11-quinton-hollywood-topics") ||
        !advance(runtime, 0x4000U,
                 "A did not open Quinton's Weller information") ||
        !expect(runtime.panel().text_segment_count > 0U &&
                    runtime.panel().option_count == 1U,
                "Weller introduction did not replace the topic menu") ||
        !save_panel(runtime, output_root, "12-quinton-weller-intro") ||
        !advance(runtime, 0x4000U,
                 "A did not open Quinton's Weller advice") ||
        !expect(runtime.panel().text_segment_count > 0U &&
                    runtime.panel().option_count == 2U,
                "Weller advice did not show Services and Goodbye") ||
        !save_panel(runtime, output_root, "13-quinton-weller-advice") ||
        !advance(runtime, 0x4000U,
                 "A did not return to Quinton's Services menu")) {
        std::cerr << runtime.failure() << '\n';
        return 11;
    }

    if (!advance(runtime, 0x1000U,
                 "B did not open Quinton's firepower information") ||
        !expect(runtime.panel().text_segment_count > 0U &&
                    runtime.panel().option_count == 1U,
                "firepower introduction did not replace the Services panel") ||
        !save_panel(runtime, output_root, "14-quinton-firepower-intro") ||
        !advance(runtime, 0x4000U,
                 "A did not advance to the firepower choices") ||
        !expect(runtime.panel().text_segment_count > 0U &&
                    runtime.panel().option_count == 3U,
                "firepower choices did not replace the introduction panel") ||
        !save_panel(runtime, output_root, "15-quinton-firepower-choices") ||
        !advance(runtime, 0x4000U,
                 "A did not purchase the Frag Grenades") ||
        !expect(runtime.panel().option_count == 2U &&
                    runtime.panel().text_segments[0].find("pick a target") !=
                        std::string_view::npos &&
                    phone_funds == 2800U && phone_item_codes[0] ==
                        frag_item->item_code &&
                    phone_item_phases[0] == 6U,
                "grenade purchase did not update state and show its success panel") ||
        !save_panel(runtime, output_root, "16-quinton-purchase-result")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 11;
    }

    if (!advance(runtime, 0x4000U,
                 "A did not return to Quinton's Services after purchase") ||
        !advance(runtime, 0x2000U,
                 "C did not open Quinton's contacts offer") ||
        !expect(runtime.panel().option_count == 2U &&
                    runtime.panel().text_segments[0].find("Rigger") !=
                        std::string_view::npos,
                "rigger offer did not replace the Services menu") ||
        !save_panel(runtime, output_root, "17-quinton-rigger-offer") ||
        !advance(runtime, 0x4000U,
                 "A did not buy Quinton's rigger contact") ||
        !expect(runtime.panel().option_count == 2U &&
                    runtime.panel().text_segments[0].find("Sharkey") !=
                        std::string_view::npos &&
                    phone_funds == 0U && phone_contacts_flags == 4U,
                "rigger purchase did not add Sharkey or show the result panel") ||
        !save_panel(runtime, output_root, "18-quinton-sharkey-result") ||
        !advance(runtime, 0x1000U,
                 "B did not leave the Quinton call") ||
        !expect(runtime.panel().option_count == 4U &&
                    runtime.panel().portrait_alias.empty() &&
                    runtime.panel().actor_name.empty(),
                "the field menu retained Quinton's call portrait or buttons") ||
        !save_panel(runtime, output_root, "19-return-to-field-menu")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 12;
    }

    if (!select_phone_option(runtime) ||
        !advance(runtime, 0x4000U,
                 "A did not reopen the phone for low-funds QA") ||
        !advance(runtime, 0x4000U,
                 "A did not select Quinton for low-funds QA") ||
        !advance(runtime, 0x4000U,
                 "A did not reopen Quinton's services") ||
        !advance(runtime, 0x1000U,
                 "B did not open firepower for low-funds QA") ||
        !advance(runtime, 0x4000U,
                 "A did not open firepower choices for low-funds QA") ||
        !advance(runtime, 0x1000U,
                 "B did not request the HK227-S") ||
        !expect(runtime.panel().option_count == 0U &&
                    runtime.panel().text_segments[0].find("can't pay") !=
                        std::string_view::npos &&
                    phone_funds == 0U,
                "low funds did not show the source denial without a charge") ||
        !save_panel(runtime, output_root, "20-quinton-insufficient-funds") ||
        !advance(runtime, 0x4000U,
                 "A did not dismiss the low-funds response")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 14;
    }

    phone_item_codes.fill(1U);
    phone_item_phases.fill(1U);
    if (!select_phone_option(runtime) ||
        !advance(runtime, 0x4000U,
                 "A did not reopen the phone for full-inventory QA") ||
        !advance(runtime, 0x4000U,
                 "A did not select Quinton for full-inventory QA") ||
        !advance(runtime, 0x4000U,
                 "A did not reopen Quinton's services for full-inventory QA") ||
        !advance(runtime, 0x1000U,
                 "B did not open firepower for full-inventory QA") ||
        !advance(runtime, 0x4000U,
                 "A did not open firepower choices for full-inventory QA") ||
        !advance(runtime, 0x4000U,
                 "A did not request Frag Grenades with a full inventory") ||
        !expect(runtime.panel().option_count == 0U &&
                    runtime.panel().text_segments[0].find("outta' stock") !=
                        std::string_view::npos &&
                    phone_funds == 0U,
                "full inventory did not show the source stock denial cleanly") ||
        !save_panel(runtime, output_root, "21-quinton-out-of-stock") ||
        !advance(runtime, 0x4000U,
                 "A did not dismiss the out-of-stock response") ||
        !expect(runtime.panel().option_count == 4U &&
                    runtime.panel().portrait_alias.empty() &&
                    runtime.panel().actor_name.empty(),
                "return after failed purchase retained dialogue graphics") ||
        !save_panel(runtime, output_root, "22-return-after-failed-purchase")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 15;
    }

    phone_funds = 1000U;
    phone_item_codes = {};
    phone_item_phases = {};
    if (!select_phone_option(runtime) ||
        !advance(runtime, 0x4000U,
                 "A did not reopen the phone for the HK227-S purchase") ||
        !advance(runtime, 0x4000U,
                 "A did not select Quinton for the HK227-S purchase") ||
        !advance(runtime, 0x4000U,
                 "A did not reopen Quinton's services for the HK227-S purchase") ||
        !advance(runtime, 0x1000U,
                 "B did not select Quinton's firepower service") ||
        !advance(runtime, 0x4000U,
                 "A did not open Quinton's firepower offers") ||
        !advance(runtime, 0x1000U,
                 "B did not select the HK227-S offer") ||
        !expect(runtime.panel().option_count == 2U &&
                    runtime.panel().text_segments[0].find("pick a target") !=
                        std::string_view::npos &&
                    phone_funds == 0U &&
                    phone_item_codes[0] == hk227_item->item_code &&
                    phone_item_phases[0] == hk227_offer->item_phase,
                "HK227-S transaction did not write the catalog item and "
                "source accessory phase" ) ||
        !save_panel(runtime, output_root, "23-quinton-hk227-purchase-result") ||
        !advance(runtime, 0x1000U,
                 "B did not select Goodbye after the HK227-S purchase") ||
        !expect(runtime.panel().option_count == 4U &&
                    runtime.panel().portrait_alias.empty() &&
                    runtime.panel().actor_name.empty(),
                "the field menu retained Quinton UI after the HK227-S purchase") ||
        !save_panel(runtime, output_root, "24-hk227-return-to-field-menu")) {
        std::cerr << runtime.failure() << '\n';
        if (!source_boundary.empty()) {
            std::cerr << source_boundary << '\n';
        }
        return 16;
    }

    if (!expect(runtime.advance_controller_events(0x8000U) ==
                    FieldDialogueAdvanceResult::returned_to_field,
                "Start did not leave the field terminal after the phone call") ||
        !expect(!runtime.open(), "Vid-Phone panel did not clear on exit")) {
        std::cerr << runtime.failure() << '\n';
        return 13;
    }

    const auto returned_screen = render_panel(runtime);
    const auto stale_portrait_sample = returned_screen.pixelColor(40, 56);
    if (!expect(stale_portrait_sample.red() == 0 &&
                    stale_portrait_sample.green() == 0 &&
                    stale_portrait_sample.blue() == 0,
                "the old phone portrait remained over the field menu")) {
        return 8;
    }
    return 0;
}
