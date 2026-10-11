#include "field_dialogue_runtime.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QImage>
#include <QPainter>

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

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

bool has_nonblack_pixel(const QImage& image, const int x, const int y,
                        const int width, const int height) {
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            if (image.pixelColor(column, row) != QColor(0, 0, 0)) {
                return true;
            }
        }
    }
    return false;
}

bool has_exact_color(const QImage& image, const int x, const int y,
                     const int width, const int height,
                     const QColor expected) {
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            if (image.pixelColor(column, row) == expected) {
                return true;
            }
        }
    }
    return false;
}

bool matches_region(const QImage& image, const QImage& reference,
                   const int x, const int y, const int width,
                   const int height) {
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            if (image.pixelColor(column, row) !=
                reference.pixelColor(column, row)) {
                return false;
            }
        }
    }
    return true;
}

bool matches_overlay(const QImage& image, const QImage& overlay,
                     const int x, const int y) {
    for (int row = 0; row < overlay.height(); ++row) {
        for (int column = 0; column < overlay.width(); ++column) {
            const auto actual = image.pixelColor(x + column, y + row);
            const auto source = overlay.pixelColor(column, row);
            const auto alpha = source.alpha();
            const auto composite_on_black = [alpha](const int component) {
                return (component * alpha + 127) / 255;
            };
            if (actual.red() != composite_on_black(source.red()) ||
                actual.green() != composite_on_black(source.green()) ||
                actual.blue() != composite_on_black(source.blue())) {
                return false;
            }
        }
    }
    return true;
}

} // namespace

int main() {
    const auto assets = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT));
    if (!expect(assets.complete())) {
        return 1;
    }

    std::string_view boundary;
    std::uint8_t arrival_roll = 79U;
    std::uint8_t observed_arrival_bound = 0U;
    std::size_t arrival_roll_count = 0U;
    StokerMotelArrivalCommandCallbacks arrival_callbacks{};
    arrival_callbacks.sample_random_bound =
        [&arrival_roll, &observed_arrival_bound,
         &arrival_roll_count](const std::uint8_t bound)
        -> std::optional<std::uint8_t> {
        observed_arrival_bound = bound;
        ++arrival_roll_count;
        return bound == 100U ? std::optional<std::uint8_t>(arrival_roll)
                             : std::nullopt;
    };
    std::uint32_t room_payment_funds = 20U;
    StokerMotelRoomPaymentOutcome room_payment_outcome{};
    std::uint32_t decker_contact_funds = 1000U;
    std::size_t decker_contact_purchase_calls = 0U;
    FieldDialogueRuntime runtime(
        [&boundary](const std::string_view reason) { boundary = reason; },
        arrival_callbacks);
    scripting::GameScriptRuntime gameplay_scripts;
    if (!gameplay_scripts.load_package(
            SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
            std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                "dialogue" / "script-text.lua")) {
        std::cerr << gameplay_scripts.failure() << '\n';
        return 70;
    }
    gameplay_scripts.set_story_state({
        {"vigore_lead_available", false},
        {"vigore_search_pending", false},
        {"errascoe_question_available", false},
    });
    runtime.set_script_runtime(&gameplay_scripts);
    runtime.set_room_payment_callback([&room_payment_funds,
                                       &room_payment_outcome]() {
        StokerMotelRoomPaymentState state{};
        state.funds = room_payment_funds;
        room_payment_outcome = resolve_stoker_motel_room_payment(state);
        room_payment_funds = state.funds;
        return std::optional<StokerMotelRoomPaymentOutcome>(
            room_payment_outcome);
    });
    StokerMotelFollowupState followup_state{1U, 0U};
    runtime.set_followup_command_callback(
        [&followup_state](const StokerMotelFollowupCommand command) {
            return std::optional<StokerMotelFollowupOutcome>{
                resolve_stoker_motel_followup_command(command,
                                                       followup_state)};
        });
    StokerMotelDebtPaymentState debt_payment_state{};
    debt_payment_state.funds = 249U;
    debt_payment_state.followup_state.story_flags = 3U;
    StokerMotelDebtPaymentOutcome debt_payment_outcome{};
    runtime.set_debt_payment_callback([&debt_payment_state,
                                      &debt_payment_outcome]() {
        debt_payment_outcome =
            resolve_stoker_motel_debt_payment(debt_payment_state);
        return std::optional<StokerMotelDebtPaymentOutcome>(
            debt_payment_outcome);
    });
    runtime.set_native_script_service_callback(
        [&decker_contact_funds, &decker_contact_purchase_calls](
            const std::string_view service_id) -> std::optional<std::string> {
            if (service_id != "johnson.purchase_decker_contact") {
                return std::nullopt;
            }
            ++decker_contact_purchase_calls;
            constexpr std::uint32_t kFinderFee = 1000U;
            if (decker_contact_funds < kFinderFee) {
                return "insufficient";
            }
            decker_contact_funds -= kFinderFee;
            return "purchased";
        });
    if (!expect(runtime.load_assets(assets.catalog))) {
        return 2;
    }

    // The entry service and the booth conversation are separate source
    // states with separate ROM portraits. Exercise the script package through
    // the shared presenter, and save the full panels for whole-screen review.
    const auto capture_jump_house = [&runtime](const std::string_view scene) {
        const auto output_dir =
            std::filesystem::current_path() / "jump-house-visual-audit" /
            std::filesystem::path(scene);
        std::filesystem::create_directories(output_dir);
        QImage image(320, 224, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::black);
        QPainter painter(&image);
        runtime.paint(painter);
        painter.end();
        return image.save(QString::fromStdWString(
                              (output_dir / "actual.bmp").wstring()),
                          "BMP");
    };
    gameplay_scripts.set_location_context(
        "building", "jump_house", "jump_house.entry", "jump_house_entry");
    if (!expect(runtime.open_gameplay_task(
            "building", "present_entry_description")) ||
        !expect(runtime.panel().source_complete) ||
        !expect(runtime.panel().portrait_alias ==
                "dialogue.portraits.jump.house.entry") ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(runtime.panel().text_segment_count == 5U) ||
        !expect(runtime.panel().text_segments[4].find("fat man") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("entry"))) {
        std::cerr << "Jump House entry page did not use its ROM profile and copy\n";
        return 90;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 91;
    }

    gameplay_scripts.set_location_context(
        "building", "jump_house", "gunderson.jump_house", "gunderson");
    if (!expect(runtime.open_gameplay_task("johnson", "interaction")) ||
        !expect(runtime.panel().source_complete) ||
        !expect(runtime.panel().portrait_alias ==
                "dialogue.portraits.gunderson.jump.house") ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("Hoi, friend") !=
                std::string_view::npos) ||
        !expect(runtime.panel().options[0].source_label ==
                "I'm looking for some information.") ||
        !expect(runtime.panel().options[1].source_label ==
                "I'm lookin' for some work.") ||
        !expect(runtime.panel().options[2].source_label ==
                "I'll talk to you later, chum.") ||
        !expect(capture_jump_house("gunderson-root"))) {
        std::cerr << "Gunderson root page did not use the ROM menu profile\n";
        return 92;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().has_more) ||
        !expect(runtime.panel().text_segments[0].find("Info, eh?") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("information-opening"))) {
        return 93;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("special contacts") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("information-topics"))) {
        return 94;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 1U) ||
        !expect(runtime.panel().text_segments[0].find("Hurry up and ask") !=
                std::string_view::npos) ||
        !expect(runtime.panel().options[0].source_label ==
                "What exactly is a Mr. Johnson?") ||
        !expect(capture_jump_house("question-selection"))) {
        std::cerr << "Gunderson question selector did not use ROM builder "
                     "0x22F90's base question\n";
        return 94;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().has_more) ||
        !expect(runtime.panel().text_segments[0].find("Geeze, you are green") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("question-johnson-answer"))) {
        return 95;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.panel().text_segments[0].find("I receive jobs") !=
                std::string_view::npos) ||
        !expect(runtime.panel().options[0].source_label ==
                "I've got another question for you.") ||
        !expect(capture_jump_house("question-followup"))) {
        return 96;
    }
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("You just tell me") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("gunderson-return"))) {
        std::cerr << "Gunderson different-subject choice did not use ROM "
                     "descriptor 0x10514C\n";
        return 97;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("Natch, chum") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("information-after-return"))) {
        std::cerr << "Gunderson alternate root did not use ROM descriptor "
                     "0x105198's information menu\n";
        return 98;
    }
    if (!expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("You just tell me") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("gunderson-return-after-information"))) {
        return 99;
    }
    if (!expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 100;
    }

    // 0x22F90 adds story-dependent questions to the same Lua menu. Selecting
    // Vigore's question runs action 0xE7 and returns the 0x105270 follow-up.
    gameplay_scripts.set_story_state({
        {"vigore_lead_available", true},
        {"vigore_search_pending", false},
        {"errascoe_question_available", false},
    });
    if (!expect(runtime.open_gameplay_task("johnson", "interaction")) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.panel().options[1].source_label ==
                "What do you know about a Johnson named Vigore?") ||
        !expect(capture_jump_house("question-vigore-option"))) {
        return 101;
    }
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(gameplay_scripts.story_state().at(
            "vigore_search_pending")) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.panel().text_segments[0].find(
                    "Word on the Johnson grapevine") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("question-vigore-response"))) {
        return 102;
    }
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find("You just tell me") !=
                std::string_view::npos) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::returned_to_field)) {
        return 103;
    }

    gameplay_scripts.set_story_state({
        {"vigore_lead_available", false},
        {"vigore_search_pending", false},
        {"errascoe_question_available", true},
    });
    if (!expect(runtime.open_gameplay_task("johnson", "interaction")) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.panel().options[1].source_label ==
                "Who's this guy Errascoe?") ||
        !expect(capture_jump_house("question-errascoe-option"))) {
        return 104;
    }
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find("Strictly small time") !=
                std::string_view::npos) ||
        !expect(runtime.panel().options[0].source_label ==
                "I've got another question for you.") ||
        !expect(capture_jump_house("question-errascoe-response")) ||
        !expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::returned_to_field)) {
        return 105;
    }

    // The site walkthrough names Gunderson's work, while the native purchase
    // flow below follows ROM descriptor 0x105200/action 0x4D. The successful
    // paid descriptor and its low-funds branch have no matching longplay
    // capture, so these are functional/full-screen regression checks only.
    const auto reach_decker_offer = [&runtime, &capture_jump_house]() {
        if (!runtime.open_gameplay_task("johnson", "interaction") ||
            runtime.advance_controller_events(0x4000U) !=
                FieldDialogueAdvanceResult::prompt_advanced ||
            runtime.advance_controller_events(0x4000U) !=
                FieldDialogueAdvanceResult::prompt_advanced ||
            runtime.advance_controller_events(0x1000U) !=
                FieldDialogueAdvanceResult::prompt_advanced ||
            runtime.panel().text_segments[0].find("Contacts!") ==
                std::string_view::npos ||
            !capture_jump_house("contacts-introduction") ||
            runtime.advance_controller_events(0x4000U) !=
                FieldDialogueAdvanceResult::prompt_advanced) {
            std::cerr << "Failed while moving from Gunderson topics to the "
                         "Decker offer\n";
            return false;
        }
        if (runtime.panel().option_count != 2U ||
            runtime.panel().text_segments[0].find("Decker") ==
                std::string_view::npos ||
            runtime.panel().text_segments[4].find("1000$") ==
                std::string_view::npos ||
            runtime.panel().options[0].source_label !=
                "Easy money, chummer.  I'll pay." ||
            runtime.panel().options[1].source_label !=
                "Let's talk about a different subject.") {
            std::cerr << "Decker offer projection: options="
                      << runtime.panel().option_count << " segments="
                      << runtime.panel().text_segment_count << " text='"
                      << runtime.panel().text_segments[0] << "' | '"
                      << runtime.panel().text_segments[1] << "' | '"
                      << runtime.panel().text_segments[2] << "' | '"
                      << runtime.panel().text_segments[3] << "' | '"
                      << runtime.panel().text_segments[4] << "'"
                      << " labels='" << runtime.panel().options[0].source_label
                      << "' / '" << runtime.panel().options[1].source_label
                      << "'\n";
            capture_jump_house("decker-offer-debug");
            return false;
        }
        return capture_jump_house("decker-offer");
    };

    if (!expect(reach_decker_offer()) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(decker_contact_purchase_calls == 1U) ||
        !expect(decker_contact_funds == 0U) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("Credstick") !=
                std::string_view::npos) ||
        !expect(runtime.panel().text_segments[2].find("finders fee") !=
                std::string_view::npos) ||
        !expect(runtime.panel().text_segments[4].find("Wilma Temmenhoff") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("contact-purchased")) ||
        !expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find("Contacts!") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("contacts-after-purchase")) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(decker_contact_purchase_calls == 2U) ||
        !expect(decker_contact_funds == 0U) ||
        !expect(runtime.panel().text_segments[2].find("short on nuyen") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("contact-insufficient-after-purchase")) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        std::cerr << "Gunderson's paid Decker-contact route did not debit, "
                     "return to the ROM contacts path, and reject a repeat "
                     "purchase without sufficient funds\n";
        return 106;
    }

    decker_contact_funds = 999U;
    if (!expect(reach_decker_offer()) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(decker_contact_purchase_calls == 3U) ||
        !expect(decker_contact_funds == 999U) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("Credstick") !=
                std::string_view::npos) ||
        !expect(runtime.panel().text_segments[2].find("short on nuyen") !=
                std::string_view::npos) ||
        !expect(capture_jump_house("contact-insufficient-999")) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        std::cerr << "Gunderson's low-funds contact route changed the wallet "
                     "or missed the ROM fallback descriptor\n";
        return 107;
    }

    FieldDialogueEventResolver event_resolver;
    FieldExitPromptState resolver_state{};
    FieldExitPromptSelection resolved_selection{};
    bool selection_published = false;
    FieldExitPromptCallbacks resolver_callbacks{};
    resolver_callbacks.run_mode_exit_services =
        [&resolved_selection, &selection_published](
            const FieldExitPromptSelection& selection,
            FieldExitPromptState&) {
            resolved_selection = selection;
            selection_published = true;
        };
    const auto stoker_route = event_resolver.resolve(
        FieldDialogueEventContext{2U, 2U, 1, 0U}, resolver_state,
        resolver_callbacks);
    if (!expect(stoker_route == FieldExitPromptAction::prompt_presented) ||
        !expect(selection_published) ||
        !expect(resolved_selection.resource_profile ==
                FieldExitResourceProfile::stoker_motel) ||
        !expect(resolved_selection.prompt_asset ==
                FieldExitPromptAsset::stoker_motel_package_question) ||
        !expect(resolver_state.exit_marker == 0x14U)) {
        return 47;
    }

    resolver_state = FieldExitPromptState{};
    selection_published = false;
    const auto ares_route = event_resolver.resolve(
        FieldDialogueEventContext{2U, 11U, 0, 0U}, resolver_state,
        resolver_callbacks);
    if (!expect(ares_route == FieldExitPromptAction::no_matching_scene) ||
        !expect(!selection_published) ||
        !expect(resolver_state.caller_exit_status == 0)) {
        return 48;
    }

    const auto* standard_frame_record =
        assets.catalog.find("dialogue.panel.frame");
    const auto* alternate_frame_record =
        assets.catalog.find("dialogue.panel.frame.alternate.left.panel");
    const auto* button_a_record = assets.catalog.find(
        "dialogue.panel.button.a");
    if (!expect(standard_frame_record != nullptr) ||
        !expect(alternate_frame_record != nullptr) ||
        !expect(button_a_record != nullptr)) {
        return 35;
    }
    const QImage standard_frame(QString::fromStdWString(
        standard_frame_record->absolute_path.wstring()));
    const QImage button_a(QString::fromStdWString(
        button_a_record->absolute_path.wstring()));

    if (!expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::manager_greeting))) {
        std::cerr << "manager prompt boundary: " << boundary << '\n';
        return 3;
    }
    if (!expect(runtime.panel().actor_name == "Stoker")) return 39;
    if (!expect(runtime.panel().option_count == 2U)) return 40;
    if (!expect(runtime.panel().text_segment_count == 5U)) return 41;
    if (!expect(runtime.panel().text_segments[1].empty()) ||
        !expect(runtime.panel().text_segments[3].empty())) return 42;
    if (!expect(runtime.panel().text_segments[0].find("manager yawns") !=
                std::string_view::npos)) return 43;

    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find("room") !=
                std::string_view::npos)) {
        return 4;
    }

    // The B button is the room menu's Leave option; Lua requests its shared
    // farewell page, which returns to the field after the next A press.
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(!runtime.panel().returns_to_field)) {
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
    if (!expect(runtime.open_field_exit_prompt(arrival))) {
        return 7;
    }
    if (!expect(runtime.panel().has_more)) {
        return 14;
    }
    if (!expect(runtime.panel().option_count == 1U)) {
        return 15;
    }
    if (!expect(runtime.panel().continuation_label == "          < MORE >")) {
        return 16;
    }
    if (!expect(runtime.panel().text_segment_count == 5U)) {
        return 17;
    }
    if (!expect(runtime.panel().text_segments[1].empty()) ||
        !expect(runtime.panel().text_segments[3].empty())) {
        return 18;
    }
    QImage continuation_scene(320, 224, QImage::Format_RGB32);
    continuation_scene.fill(Qt::magenta);
    QPainter continuation_painter(&continuation_scene);
    runtime.paint(continuation_painter);
    continuation_painter.end();
    if (!expect(!has_exact_color(continuation_scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(has_green_ink(continuation_scene, 88, 16, 216, 8)) ||
        !expect(!has_green_ink(continuation_scene, 16, 151, 16, 16)) ||
        !expect(!has_green_ink(continuation_scene, 48, 159, 80, 8)) ||
        !expect(has_green_ink(continuation_scene, 128, 159, 64, 8))) {
        return 14;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(arrival_roll_count == 1U) ||
        !expect(observed_arrival_bound == 100U) ||
        !expect(runtime.panel().actor_name == "Stoker") ||
        !expect(runtime.panel().actor_kind ==
                DialoguePanelActorKind::field_npc) ||
        !expect(runtime.panel().portrait_alias ==
                "dialogue.portraits.stoker.motel") ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(!runtime.panel().has_more) ||
        !expect(runtime.panel().options[0].role ==
                DialoguePanelOptionRole::script_menu_choice) ||
        !expect(runtime.panel().options[1].role ==
                DialoguePanelOptionRole::script_menu_choice) ||
        !expect(dialogue_panel_option_command(runtime.panel(), 0U) ==
                DialoguePanelCommand::script_menu_choice) ||
        !expect(dialogue_panel_option_command(runtime.panel(), 1U) ==
                DialoguePanelCommand::script_menu_choice) ||
        !expect(runtime.panel().text_segments[0].find("manager yawns") !=
                std::string_view::npos) ||
        !expect(runtime.panel().options[2].source_label.empty())) {
        return 8;
    }
    QImage manager_after_arrival(320, 224, QImage::Format_RGB32);
    manager_after_arrival.fill(Qt::magenta);
    QPainter manager_after_arrival_painter(&manager_after_arrival);
    runtime.paint(manager_after_arrival_painter);
    manager_after_arrival_painter.end();
    if (!expect(!has_exact_color(manager_after_arrival, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(has_green_ink(manager_after_arrival, 88, 16, 216, 8)) ||
        !expect(has_green_ink(manager_after_arrival, 48, 152, 216, 8)) ||
        !expect(!has_green_ink(manager_after_arrival, 48, 200, 256, 16)) ||
        !expect(!has_nonblack_pixel(manager_after_arrival, 17, 203, 11, 11))) {
        return 53;
    }
    for (int frame = 0; frame < 30; ++frame) {
        if (!expect(runtime.advance_controller_events(0U) ==
                    FieldDialogueAdvanceResult::waiting_for_controller)) {
            return 60;
        }
    }
    QImage manager_idle(320, 224, QImage::Format_RGB32);
    manager_idle.fill(Qt::magenta);
    QPainter manager_idle_painter(&manager_idle);
    runtime.paint(manager_idle_painter);
    manager_idle_painter.end();
    if (!expect(matches_region(manager_after_arrival, manager_idle, 0, 0,
                               320, 224))) {
        return 61;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find("20$") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 2U)) {
        return 56;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(room_payment_funds == 0U) ||
        !expect(room_payment_outcome.result ==
                StokerMotelRoomPaymentResult::room_paid) ||
        !expect(room_payment_outcome.remaining_funds == 0U) ||
        !expect(room_payment_outcome.hotel_options_menu_follows) ||
        !expect(runtime.panel().text_segments[0].find("own towels") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(!runtime.panel().returns_to_field)) {
        return 62;
    }
    QImage room_paid_scene(320, 224, QImage::Format_RGB32);
    room_paid_scene.fill(Qt::magenta);
    QPainter room_paid_painter(&room_paid_scene);
    runtime.paint(room_paid_painter);
    room_paid_painter.end();
    if (!expect(!has_exact_color(room_paid_scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(has_green_ink(room_paid_scene, 88, 16, 216, 40)) ||
        !expect(matches_region(room_paid_scene, standard_frame, 16, 152, 16, 16)) ||
        !expect(matches_region(room_paid_scene, standard_frame, 16, 176, 16, 16)) ||
        !expect(matches_region(room_paid_scene, standard_frame, 16, 200, 16, 16))) {
        return 63;
    }
    for (int frame = 0; frame < 30; ++frame) {
        if (!expect(runtime.advance_controller_events(0U) ==
                    FieldDialogueAdvanceResult::waiting_for_controller)) {
            return 64;
        }
    }
    QImage room_paid_idle(320, 224, QImage::Format_RGB32);
    room_paid_idle.fill(Qt::magenta);
    QPainter room_paid_idle_painter(&room_paid_idle);
    runtime.paint(room_paid_idle_painter);
    room_paid_idle_painter.end();
    if (!expect(matches_region(room_paid_scene, room_paid_idle, 0, 0, 320,
                               224)) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::hotel_options_requested) ||
        !expect(!runtime.open()) ||
        !expect(boundary.empty())) {
        return 65;
    }

    room_payment_funds = 19U;
    if (!expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::room_offer)) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(room_payment_funds == 19U) ||
        !expect(room_payment_outcome.result ==
                StokerMotelRoomPaymentResult::insufficient_funds) ||
        !expect(!room_payment_outcome.hotel_options_menu_follows) ||
        !expect(runtime.panel().text_segments[0].find("low on creds") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(runtime.panel().returns_to_field)) {
        return 66;
    }
    QImage room_insufficient_scene(320, 224, QImage::Format_RGB32);
    room_insufficient_scene.fill(Qt::magenta);
    QPainter room_insufficient_painter(&room_insufficient_scene);
    runtime.paint(room_insufficient_painter);
    room_insufficient_painter.end();
    if (!expect(!has_exact_color(room_insufficient_scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(has_green_ink(room_insufficient_scene, 88, 16, 216, 56)) ||
        !expect(matches_region(room_insufficient_scene, standard_frame, 16, 152,
                               16, 16)) ||
        !expect(matches_region(room_insufficient_scene, standard_frame, 16, 176,
                               16, 16)) ||
        !expect(matches_region(room_insufficient_scene, standard_frame, 16, 200,
                               16, 16)) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 67;
    }

    runtime.set_room_payment_callback({});
    if (!expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::room_offer)) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::source_boundary) ||
        !expect(boundary.find("room-payment state is unresolved") !=
                std::string_view::npos)) {
        return 68;
    }

    // The 80..99 path must not be misreported as the manager greeting while
    // the source 0x1E1FC effect sequence is still unbound.
    if (!expect(runtime.open_field_exit_prompt(arrival))) {
        return 54;
    }
    arrival_roll = 80U;
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::source_boundary) ||
        !expect(runtime.panel().kind == DialoguePanelKind::source_boundary) ||
        !expect(boundary.find("ambush-effect callback is unresolved") !=
                std::string_view::npos)) {
        return 55;
    }

    // If the source effect handler reports success, command 0xE9 selects its
    // own text record and clears the previous manager buttons from the panel.
    arrival_callbacks.forward_ambush_effect_sequence = [] { return true; };
    runtime.set_arrival_command_callbacks(arrival_callbacks);
    if (!expect(runtime.open_field_exit_prompt(arrival)) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find(
                    "leap from the shadows") != std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(runtime.panel().returns_to_field)) {
        return 57;
    }
    QImage ambush_scene(320, 224, QImage::Format_RGB32);
    ambush_scene.fill(Qt::magenta);
    QPainter ambush_painter(&ambush_scene);
    runtime.paint(ambush_painter);
    ambush_painter.end();
    if (!expect(!has_exact_color(ambush_scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(matches_region(ambush_scene, standard_frame, 16, 152, 16, 16)) ||
        !expect(matches_region(ambush_scene, standard_frame, 16, 176, 16, 16)) ||
        !expect(matches_region(ambush_scene, standard_frame, 16, 200, 16, 16))) {
        return 58;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 59;
    }

    const FieldExitPromptSelection package_question{
        FieldExitResourceProfile::stoker_motel,
        FieldExitPromptAsset::stoker_motel_package_question,
        0x14U};
    if (!expect(runtime.open_field_exit_prompt(package_question)) ||
        !expect(runtime.panel().text_segment_count != 0U) ||
        !expect(runtime.panel().text_segments[0].find("Do you have the package") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field)) {
        return 9;
    }

    constexpr std::array<std::pair<StokerMotelPromptId, std::string_view>, 3>
        kLuaResolvedTitleOnlyPrompts{{
            {StokerMotelPromptId::package_handoff, "Here's the package"},
            {StokerMotelPromptId::leave_notice, "seeing you, chum"},
            {StokerMotelPromptId::jump_house_referral, "Jump House"},
        }};
    for (std::size_t index = 0U;
         index < kLuaResolvedTitleOnlyPrompts.size(); ++index) {
        const auto& [prompt, expected_text] =
            kLuaResolvedTitleOnlyPrompts[index];
        bool expected_text_found = false;
        if (runtime.open_stoker_prompt_for_dump(prompt)) {
            for (std::size_t segment = 0U;
                 segment < runtime.panel().text_segment_count; ++segment) {
                expected_text_found = expected_text_found ||
                    runtime.panel().text_segments[segment].find(expected_text) !=
                        std::string_view::npos;
            }
        }
        if (!expect(expected_text_found) ||
            !expect(runtime.panel().option_count == 0U) ||
            !expect(runtime.panel().text_segment_count != 0U) ||
            !expect(runtime.advance_controller_events(0x4000U) ==
                    FieldDialogueAdvanceResult::returned_to_field) ||
            !expect(!runtime.open())) {
            std::cerr << "Lua prompt event resolution failed for event index "
                      << index << '\n';
            return static_cast<int>(60U + index);
        }
    }

    const FieldExitPromptSelection unconverted_profile{
        FieldExitResourceProfile::scene_zero_submode_three,
        FieldExitPromptAsset::scene_zero_submode_three_default,
        0x4BU};
    if (!expect(!runtime.open_field_exit_prompt(unconverted_profile)) ||
        !expect(runtime.panel().kind == DialoguePanelKind::source_boundary)) {
        return 10;
    }
    QImage unresolved_speaker_scene(320, 224, QImage::Format_RGB32);
    unresolved_speaker_scene.fill(Qt::black);
    QPainter unresolved_speaker_painter(&unresolved_speaker_scene);
    runtime.paint(unresolved_speaker_painter);
    unresolved_speaker_painter.end();
    if (!expect(unresolved_speaker_scene.pixelColor(40, 60) ==
                QColor(Qt::black))) {
        return 34;
    }

    if (!expect(runtime.open_stoker_interaction_for_dump(
            StokerMotelStoryState{true, false, false})) ||
        !expect(runtime.panel().text_segment_count == 5U) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().text_segments[0].find("manager yawns") !=
                std::string_view::npos)) {
        return 11;
    }
    QImage scene(320, 224, QImage::Format_RGB32);
    scene.fill(Qt::magenta);
    QPainter painter(&scene);
    runtime.paint(painter);
    painter.end();
    if (!expect(!has_exact_color(scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta)))) {
        return 13;
    }
    if (!expect(has_green_ink(scene, 88, 16, 216, 8))) {
        return 19;
    }
    if (!expect(!has_green_ink(scene, 82, 16, 6, 8))) {
        return 20;
    }
    if (!expect(has_green_ink(scene, 88, 32, 216, 8))) {
        return 21;
    }
    if (!expect(!has_green_ink(scene, 88, 24, 216, 8))) {
        return 22;
    }
    if (!expect(has_nonblack_pixel(scene, 16, 32, 56, 72))) {
        return 23;
    }
    if (!expect(matches_region(scene, standard_frame, 16, 20, 54, 8))) {
        return 24;
    }
    if (!expect(has_green_ink(scene, 88, 56, 216, 8))) {
        return 25;
    }
    if (!expect(!has_green_ink(scene, 88, 48, 216, 8))) {
        return 26;
    }
    if (!expect(matches_region(scene, standard_frame, 0, 146, 320, 6))) {
        return 27;
    }
    if (!expect(matches_region(scene, standard_frame, 312, 152, 8, 72))) {
        return 29;
    }
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

    const auto debt_choice_result = runtime.advance_controller_events(0x2000U);
    if (debt_choice_result != FieldDialogueAdvanceResult::prompt_advanced ||
        !expect(followup_state.story_flags == 3U) ||
        !expect(followup_state.referral_flags == 0U) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.panel().text_segments[0].find("Michael's brother") !=
                std::string_view::npos)) {
        std::cerr << "debt menu state: result="
                  << static_cast<int>(debt_choice_result)
                  << " followup_flags=" << followup_state.story_flags
                  << " referral_flags="
                  << static_cast<unsigned>(followup_state.referral_flags)
                  << " options=" << runtime.panel().option_count
                  << " text=" << runtime.panel().text_segments[0]
                  << " panel_reason="
                  << runtime.panel().source_boundary_reason
                  << " boundary=" << boundary << '\n';
        return 30;
    }
    scene.fill(Qt::magenta);
    QPainter debt_painter(&scene);
    runtime.paint(debt_painter);
    debt_painter.end();
    if (!expect(!has_exact_color(scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(has_green_ink(scene, 88, 16, 216, 8)) ||
        !expect(!has_green_ink(scene, 88, 32, 216, 8)) ||
        !expect(has_green_ink(scene, 88, 40, 216, 8)) ||
        !expect(has_green_ink(scene, 88, 80, 216, 8)) ||
        !expect(!has_green_ink(scene, 88, 88, 216, 8)) ||
        !expect(has_green_ink(scene, 88, 96, 216, 8)) ||
        !expect(!has_green_ink(scene, 48, 197, 256, 16))) {
        return 31;
    }
    const auto held_debt_scene = scene;
    for (int frame = 0; frame < 30; ++frame) {
        if (!expect(runtime.advance_controller_events(0U) ==
                    FieldDialogueAdvanceResult::waiting_for_controller)) {
            return 60;
        }
    }
    scene.fill(Qt::magenta);
    QPainter held_debt_painter(&scene);
    runtime.paint(held_debt_painter);
    held_debt_painter.end();
    if (!expect(matches_region(scene, held_debt_scene, 0, 0, 320, 224))) {
        return 61;
    }
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(followup_state.story_flags == 3U) ||
        !expect(followup_state.referral_flags == 1U) ||
        !expect(runtime.panel().text_segments[0].find("Jump House") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(!runtime.panel().returns_to_field)) {
        return 32;
    }
    scene.fill(Qt::magenta);
    QPainter referral_painter(&scene);
    runtime.paint(referral_painter);
    referral_painter.end();
    if (!expect(!has_exact_color(scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(!has_green_ink(scene, 48, 152, 256, 64)) ||
        !expect(!has_green_ink(scene, 296, 16, 8, 120)) ||
        !expect(!has_nonblack_pixel(scene, 17, 203, 11, 11))) {
        return 62;
    }
    const auto held_referral_scene = scene;
    for (int frame = 0; frame < 30; ++frame) {
        if (!expect(runtime.advance_controller_events(0U) ==
                    FieldDialogueAdvanceResult::waiting_for_controller)) {
            return 63;
        }
    }
    scene.fill(Qt::magenta);
    QPainter held_referral_painter(&scene);
    runtime.paint(held_referral_painter);
    held_referral_painter.end();
    if (!expect(matches_region(scene, held_referral_scene, 0, 0, 320, 224))) {
        return 64;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 33;
    }

    // Paying the debt with a typed wallet below 250 returns the script to the
    // source Jump House page and clears the former debt choices.
    debt_payment_state.followup_state.referral_flags = 0U;
    if (!expect(runtime.open_stoker_interaction_for_dump(
            StokerMotelStoryState{true, false, false})) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(debt_payment_outcome.result ==
                StokerMotelDebtPaymentResult::insufficient_funds) ||
        !expect(debt_payment_outcome.remaining_funds == 249U) ||
        !expect(debt_payment_state.followup_state.referral_flags == 1U) ||
        !expect(!gameplay_scripts.story_state().at("michael_debt_paid")) ||
        !expect(!gameplay_scripts.story_state().at(
                    "michael_package_collected")) ||
        !expect(runtime.panel().text_segments[0].find("Jump House") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(!runtime.panel().returns_to_field)) {
        return 65;
    }
    scene.fill(Qt::magenta);
    QPainter debt_payment_painter(&scene);
    runtime.paint(debt_payment_painter);
    debt_payment_painter.end();
    if (!expect(!has_exact_color(scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(!has_green_ink(scene, 48, 152, 256, 64)) ||
        !expect(!has_green_ink(scene, 296, 16, 8, 120)) ||
        !expect(!has_nonblack_pixel(scene, 17, 203, 11, 11))) {
        return 66;
    }
    const auto debt_payment_scene = scene;
    for (int frame = 0; frame < 30; ++frame) {
        if (!expect(runtime.advance_controller_events(0U) ==
                    FieldDialogueAdvanceResult::waiting_for_controller)) {
            return 67;
        }
    }
    scene.fill(Qt::magenta);
    QPainter held_debt_payment_painter(&scene);
    runtime.paint(held_debt_payment_painter);
    held_debt_payment_painter.end();
    if (!expect(matches_region(scene, debt_payment_scene, 0, 0, 320, 224)) ||
        !expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::jump_house_referral))) {
        return 68;
    }
    scene.fill(Qt::magenta);
    QPainter direct_referral_painter(&scene);
    runtime.paint(direct_referral_painter);
    direct_referral_painter.end();
    if (!expect(matches_region(scene, debt_payment_scene, 0, 0, 320, 224))) {
        return 69;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 70;
    }

    // At exactly 250 nuyen the source pays the debt, applies its world and
    // runner-state mutations, then presents the package handoff record.
    debt_payment_state.funds = 250U;
    debt_payment_state.world_flags = 0U;
    debt_payment_state.effect_marker = 0U;
    debt_payment_state.runner_feature_flags = 0x40U;
    debt_payment_state.runner_source_bytes = {0xFFU, 0x00U, 0x7FU};
    debt_payment_state.followup_state.story_flags = 3U;
    if (!expect(runtime.open_stoker_interaction_for_dump(
            StokerMotelStoryState{true, false, false})) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::prompt_advanced)) {
        return 78;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced)) {
        return 79;
    }
    if (!expect(debt_payment_outcome.result ==
                    StokerMotelDebtPaymentResult::debt_paid) ||
        !expect(debt_payment_outcome.remaining_funds == 500U) ||
        !expect(debt_payment_outcome.michael_package_sequence_follows) ||
        !expect(gameplay_scripts.story_state().at("michael_debt_paid")) ||
        !expect(gameplay_scripts.story_state().at(
                    "michael_package_collected"))) {
        return 81;
    }
    if (!expect(debt_payment_state.world_flags == 0x01U) ||
        !expect(debt_payment_state.runner_feature_flags == 0x41U) ||
        !expect(debt_payment_state.runner_source_bytes ==
                std::array<std::uint8_t, 3>{0x00U, 0x01U, 0x80U}) ||
        !expect(debt_payment_state.followup_state.story_flags == 0x1CU)) {
        return 82;
    }
    if (!expect(runtime.panel().text_segment_count == 3U) ||
        !expect(runtime.panel().text_segments[2].find("Here's the package") !=
                std::string_view::npos) ||
        !expect(!runtime.panel().returns_to_field)) {
        return 83;
    }
    scene.fill(Qt::magenta);
    QPainter package_handoff_painter(&scene);
    runtime.paint(package_handoff_painter);
    package_handoff_painter.end();
    if (!expect(!has_exact_color(scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta))) ||
        !expect(!has_green_ink(scene, 48, 152, 256, 64)) ||
        !expect(!has_nonblack_pixel(scene, 17, 203, 11, 11))) {
        return 76;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(!runtime.open())) {
        return 77;
    }

    StokerMotelStoryState first_visit_state{};
    if (!expect(runtime.open_stoker_interaction_for_dump(first_visit_state)) ||
        !expect(gameplay_scripts.story_state().at("opening_dialogue_seen")) ||
        !expect(runtime.panel().text_segments[0].find("battered sign") !=
                std::string_view::npos) ||
        !expect(runtime.panel().actor_name == "Stoker") ||
        !expect(runtime.panel().portrait_alias ==
                "dialogue.portraits.stoker.motel") ||
        !expect(runtime.panel().has_more) ||
        !expect(runtime.panel().continuation_label.find("MORE") !=
                std::string_view::npos)) {
        return 71;
    }
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                    FieldDialogueAdvanceResult::waiting_for_controller) ||
        !expect(runtime.open())) {
        return 71;
    }
    QImage arrival_scene(320, 224, QImage::Format_RGB32);
    arrival_scene.fill(Qt::black);
    QPainter arrival_painter(&arrival_scene);
    runtime.paint(arrival_painter);
    arrival_painter.end();
    if (!expect(matches_overlay(arrival_scene, button_a, 16, 152))) {
        return 74;
    }
    if (!expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 3U) ||
        !expect(runtime.panel().options[2].source_label.find(
                    "Michael's stuff") != std::string_view::npos)) {
        return 71;
    }
    if (!expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().text_segments[0].find("seeing you") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(!runtime.panel().has_more) ||
        !expect(runtime.advance_controller_events(0x1000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(runtime.open_stoker_interaction_for_dump(
                    StokerMotelStoryState{true, false, false})) ||
        !expect(runtime.panel().option_count == 3U)) {
        return 72;
    }
    runtime.close();
    if (!expect(runtime.open_stoker_interaction_for_dump(
                    StokerMotelStoryState{true, true, true})) ||
        !expect(runtime.panel().option_count == 2U)) {
        return 73;
    }
    runtime.close();

    // A different source can supply its resolved speaker/event view without
    // inheriting Stoker content. Repainting fewer options removes the old
    // bottom row and selector before applying the source-selected frame mode.
    DialoguePanelPresenter presenter;
    if (!expect(presenter.load_assets(assets.catalog))) {
        return 36;
    }
    DialoguePanelView other_speaker;
    other_speaker.open = true;
    other_speaker.actor_kind = DialoguePanelActorKind::field_npc;
    other_speaker.actor_name = "Mara";
    other_speaker.kind = DialoguePanelKind::field_npc_prompt;
    other_speaker.text_segments[0] =
        "Mara asks what brings you to the building.";
    other_speaker.text_segment_count = 1U;
    other_speaker.options[0] = {
        DialoguePanelOptionRole::field_continue,
        "Ask about the building.",
        DialoguePanelCommand::stoker_resolve_arrival};
    other_speaker.options[1] = {
        DialoguePanelOptionRole::field_leave_motel,
        "Leave for now.",
        DialoguePanelCommand::stoker_open_leave_notice};
    other_speaker.options[2] = {
        DialoguePanelOptionRole::field_pay_debt,
        "Ask who sent you.",
        DialoguePanelCommand::stoker_open_debt_question};
    other_speaker.option_count = 3U;
    QImage generic_scene(320, 224, QImage::Format_RGB32);
    generic_scene.fill(Qt::magenta);
    QPainter generic_painter(&generic_scene);
    presenter.paint(generic_painter, other_speaker);
    other_speaker.option_count = 1U;
    presenter.paint(generic_painter, other_speaker);
    generic_painter.end();
    if (!expect(!has_exact_color(generic_scene, 0, 0, 320, 224,
                                 QColor(Qt::magenta)))) return 47;
    if (!expect(has_green_ink(generic_scene, 88, 16, 216, 8))) return 48;
    if (!expect(has_green_ink(generic_scene, 48, 152, 216, 8))) return 49;
    if (!expect(!has_green_ink(generic_scene, 48, 200, 256, 16))) return 50;
    if (!expect(!has_nonblack_pixel(generic_scene, 17, 203, 11, 11))) return 51;
    if (!expect(generic_scene.pixelColor(40, 60) == QColor(Qt::black))) return 52;
    if (!matches_overlay(generic_scene, button_a, 16, 152)) {
        for (int row = 0; row < button_a.height(); ++row) {
            for (int column = 0; column < button_a.width(); ++column) {
                const auto actual =
                    generic_scene.pixelColor(16 + column, 152 + row);
                const auto expected = button_a.pixelColor(column, row);
                if (actual != expected) {
                    std::cerr << "dialogue A marker differs at " << column
                              << ',' << row << ": actual="
                              << actual.name(QColor::HexArgb).toStdString()
                              << " expected="
                              << expected.name(QColor::HexArgb).toStdString()
                              << '\n';
                    row = button_a.height();
                    break;
                }
            }
        }
        return 53;
    }

    QImage alternate_scene(320, 224, QImage::Format_RGB32);
    alternate_scene.fill(Qt::black);
    other_speaker.frame_variant =
        DialoguePanelFrameVariant::alternate_left_panel;
    QPainter alternate_painter(&alternate_scene);
    presenter.paint(alternate_painter, other_speaker);
    alternate_painter.end();
    bool selected_frame_differs = false;
    for (int y = 32; y < 152 && !selected_frame_differs; ++y) {
        for (int x = 0; x < 80; ++x) {
            if (generic_scene.pixelColor(x, y) !=
                alternate_scene.pixelColor(x, y)) {
                selected_frame_differs = true;
                break;
            }
        }
    }
    if (!expect(selected_frame_differs)) {
        return 38;
    }
    return 0;
}
