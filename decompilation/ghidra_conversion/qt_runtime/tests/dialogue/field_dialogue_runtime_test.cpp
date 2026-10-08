#include "field_dialogue_runtime.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QImage>
#include <QPainter>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
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
    FieldDialogueRuntime runtime(
        [&boundary](const std::string_view reason) { boundary = reason; },
        arrival_callbacks);
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
    if (!expect(runtime.load_assets(assets.catalog))) {
        return 2;
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

    // The B button is the source's second option and leaves on its proven
    // sentinel page. The A payment branch is exercised below.
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
        !expect(dialogue_panel_option_command(runtime.panel(), 0U) ==
                DialoguePanelCommand::stoker_open_room_offer) ||
        !expect(dialogue_panel_option_command(runtime.panel(), 1U) ==
                DialoguePanelCommand::stoker_open_leave_notice) ||
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
        !expect(runtime.panel().returns_to_field)) {
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
    QImage unresolved_speaker_scene(320, 224, QImage::Format_RGB32);
    unresolved_speaker_scene.fill(Qt::black);
    QPainter unresolved_speaker_painter(&unresolved_speaker_scene);
    runtime.paint(unresolved_speaker_painter);
    unresolved_speaker_painter.end();
    if (!expect(unresolved_speaker_scene.pixelColor(40, 60) ==
                QColor(Qt::black))) {
        return 34;
    }

    if (!expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::manager_greeting_with_package)) ||
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

    if (!expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(followup_state.story_flags == 3U) ||
        !expect(followup_state.referral_flags == 0U) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.panel().text_segments[0].find("Michael's brother") !=
                std::string_view::npos)) {
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
        !expect(runtime.panel().returns_to_field)) {
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

    // The source A command on the debt question enters 0x213E2 when the
    // typed wallet is below 250. Verify it reaches the same title-only
    // Jump House page and clears the former A/B rows.
    debt_payment_state.followup_state.referral_flags = 0U;
    if (!expect(runtime.open_stoker_prompt_for_dump(
            StokerMotelPromptId::manager_greeting_with_package)) ||
        !expect(runtime.advance_controller_events(0x2000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(runtime.panel().option_count == 2U) ||
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::prompt_advanced) ||
        !expect(debt_payment_outcome.result ==
                StokerMotelDebtPaymentResult::insufficient_funds) ||
        !expect(debt_payment_outcome.remaining_funds == 249U) ||
        !expect(debt_payment_state.followup_state.referral_flags == 1U) ||
        !expect(runtime.panel().text_segments[0].find("Jump House") !=
                std::string_view::npos) ||
        !expect(runtime.panel().option_count == 0U) ||
        !expect(runtime.panel().returns_to_field)) {
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

    if (!expect(runtime.open_stoker_story_entry(
                    StokerMotelStoryEntryAction::opening_dialogue)) ||
        !expect(runtime.panel().text_segments[0].find("battered sign") !=
                std::string_view::npos) ||
        !expect(runtime.panel().has_more) ||
        !expect(runtime.panel().continuation_label.find("MORE") !=
                std::string_view::npos)) {
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
        !expect(runtime.advance_controller_events(0x4000U) ==
                FieldDialogueAdvanceResult::returned_to_field) ||
        !expect(runtime.open_stoker_story_entry(
                    StokerMotelStoryEntryAction::package_offer)) ||
        !expect(runtime.panel().option_count == 3U)) {
        return 72;
    }
    if (!expect(runtime.open_stoker_story_entry(
                    StokerMotelStoryEntryAction::ordinary_motel)) ||
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
