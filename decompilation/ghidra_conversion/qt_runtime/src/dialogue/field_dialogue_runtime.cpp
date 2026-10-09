#include "field_dialogue_runtime.hpp"

#include <QPainter>

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

DialogueSpeakerPresentation present_speaker(
    const scripting::ScriptSpeakerProfile& profile) {
    return {profile.id, profile.name, profile.text_profile,
            profile.portrait_alias};
}

} // namespace

FieldExitPromptAction FieldDialogueEventResolver::resolve(
    const FieldDialogueEventContext& context,
    FieldExitPromptState& state,
    const FieldExitPromptCallbacks& callbacks) const {
    state.world_flags = context.world_flags;
    return select_field_exit_prompt(context.field_mode, context.event_code,
                                    context.alternate_result, state, callbacks);
}

FieldDialogueRuntime::FieldDialogueRuntime(
    std::function<void(std::string_view)> source_boundary,
    StokerMotelArrivalCommandCallbacks arrival_command_callbacks)
    : source_boundary_callback_(std::move(source_boundary)),
      arrival_command_callbacks_(std::move(arrival_command_callbacks)) {}

void FieldDialogueRuntime::set_arrival_command_callbacks(
    StokerMotelArrivalCommandCallbacks callbacks) {
    arrival_command_callbacks_ = std::move(callbacks);
}

void FieldDialogueRuntime::set_room_payment_callback(
    std::function<std::optional<StokerMotelRoomPaymentOutcome>()> callback) {
    room_payment_callback_ = std::move(callback);
}

void FieldDialogueRuntime::set_debt_payment_callback(
    std::function<std::optional<StokerMotelDebtPaymentOutcome>()> callback) {
    debt_payment_callback_ = std::move(callback);
}

void FieldDialogueRuntime::set_followup_command_callback(
    std::function<std::optional<StokerMotelFollowupOutcome>(
        StokerMotelFollowupCommand)> callback) {
    followup_command_callback_ = std::move(callback);
}

void FieldDialogueRuntime::set_script_runtime(
    scripting::GameScriptRuntime* runtime) noexcept {
    script_runtime_ = runtime;
}

bool FieldDialogueRuntime::load_assets(const ConversionAssetCatalog& assets) {
    failure_.clear();
    if (script_runtime_ == nullptr ||
        script_runtime_->dialogue_text("stoker.motel.arrival.title") ==
            nullptr) {
        failure_ = "Lua dialogue content is not loaded";
        assets_loaded_ = false;
        if (source_boundary_callback_) {
            source_boundary_callback_("field.dialogue.script_text_unavailable");
        }
        return false;
    }

    auto stoker_profile = script_runtime_->speaker_profile("stoker");
    const auto* stoker_portrait =
        stoker_profile.has_value()
            ? assets.find(stoker_profile->portrait_alias)
            : nullptr;
    if (!stoker_profile.has_value() ||
        stoker_profile->text_profile != "stoker.motel" ||
        stoker_portrait == nullptr || stoker_portrait->kind != "image") {
        failure_ = "Lua dialogue speaker profile or portrait is unavailable";
        assets_loaded_ = false;
        if (source_boundary_callback_) {
            source_boundary_callback_(
                "field.dialogue.script_speaker_unavailable");
        }
        return false;
    }

    if (!presenter_.load_assets(assets)) {
        failure_ = presenter_.failure();
        assets_loaded_ = false;
        if (source_boundary_callback_) {
            source_boundary_callback_("field.dialogue.shared_assets_unavailable");
        }
        return false;
    }

    stoker_speaker_profile_ = std::move(stoker_profile);
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
    if (prompt == StokerMotelPromptId::manager_greeting_with_package) {
        // The package option belongs to the quest-wrapped shared motel
        // interaction. Start at a repeat visit so this diagnostic exercises
        // the real Lua menu and its debt/referral service hooks.
        return open_stoker_interaction_for_dump(
            StokerMotelStoryState{true, false, false});
    }
    if (prompt == StokerMotelPromptId::manager_greeting) {
        if (script_runtime_ != nullptr) {
            script_runtime_->set_location_context(
                "motel", "stoker_motel", "stoker.motel", "stoker");
        }
        if (script_runtime_ != nullptr) {
            script_runtime_->set_story_state({
                {"michael_debt_paid", true},
                {"michael_package_collected", true}});
        }
    }
    return open_stoker_prompt(prompt);
}

bool FieldDialogueRuntime::open_stoker_interaction() {
    if (script_runtime_ != nullptr) {
        script_runtime_->set_location_context(
            "motel", "stoker_motel", "stoker.motel", "stoker");
    }
    return start_script_task("michael_package", "stoker_interaction",
                             StokerMotelPromptId::manager_greeting);
}

bool FieldDialogueRuntime::open_stoker_interaction_for_dump(
    const StokerMotelStoryState& story_state) {
    if (script_runtime_ != nullptr) {
        script_runtime_->set_story_state({
            {"opening_dialogue_seen", story_state.opening_dialogue_seen},
            {"michael_debt_paid", story_state.michael_debt_paid},
            {"michael_package_collected",
             story_state.michael_package_collected}});
    }
    return open_stoker_interaction();
}

bool FieldDialogueRuntime::open_stoker_arrival_result_for_dump(
    const std::uint8_t random_roll) {
    const auto saved_callbacks = arrival_command_callbacks_;
    arrival_command_callbacks_.sample_random_bound =
        [random_roll](const std::uint8_t bound)
        -> std::optional<std::uint8_t> {
        if (bound != 100U || random_roll >= bound) {
            return std::nullopt;
        }
        return random_roll;
    };
    arrival_command_callbacks_.forward_ambush_effect_sequence = {};

    const bool arrival_opened = open_stoker_prompt(StokerMotelPromptId::arrival);
    const bool result_opened =
        arrival_opened &&
        advance_controller_events(kControllerAEvent) ==
            FieldDialogueAdvanceResult::prompt_advanced;
    arrival_command_callbacks_ = saved_callbacks;
    return result_opened;
}

bool FieldDialogueRuntime::open_stoker_prompt(
    const StokerMotelPromptId prompt) {
    if (script_task_waiting_ && script_runtime_ != nullptr) {
        script_runtime_->cancel_task();
    }
    if (!assets_loaded_) {
        return open_source_boundary(
            failure_.empty() ? "Stoker dialogue assets are not loaded"
                             : failure_);
    }
    boundary_reason_.clear();
    active_prompt_ = prompt;
    hotel_options_menu_follows_ = false;
    active_script_operation_.reset();
    active_script_menu_.reset();
    StokerMotelPromptView source_view;
    if (prompt == StokerMotelPromptId::manager_greeting ||
        prompt == StokerMotelPromptId::manager_greeting_with_package) {
        if (script_runtime_ == nullptr) {
            return open_source_boundary(
                "Stoker manager menu has no gameplay script runtime");
        }
        script_runtime_->set_location_context(
            "motel", "stoker_motel", "stoker.motel", "stoker");
        return start_script_task("motel", "manager_interaction", prompt);
    } else {
        source_view = make_stoker_motel_prompt(prompt);
    }
    if (!stoker_speaker_profile_.has_value()) {
        return open_source_boundary(
            "Stoker speaker profile was not loaded from Lua");
    }
    active_speaker_profile_ = stoker_speaker_profile_;
    const DialogueTextResolver resolver = [this](const std::string_view alias) {
        return resolve_text(alias);
    };
    panel_ = make_dialogue_panel_view(
        source_view, present_speaker(*active_speaker_profile_), resolver);
    if (!panel_.source_complete && panel_.source_boundary_reason.empty()) {
        panel_.source_boundary_reason =
            "Stoker dialogue source projection is incomplete";
    }
    return true;
}

bool FieldDialogueRuntime::start_script_task(
    const std::string_view module, const std::string_view function,
    const StokerMotelPromptId prompt) {
    if (script_runtime_ == nullptr) {
        return open_source_boundary(
            "dialogue script has no gameplay script runtime");
    }
    const auto task = script_runtime_->start_task(module, function);
    if (task.status !=
            scripting::ScriptTaskStep::Status::awaiting_native_operation ||
        !task.operation.has_value()) {
        return open_source_boundary(
            script_runtime_->failure().empty()
                ? "dialogue script did not yield a presentation operation"
                : script_runtime_->failure());
    }
    return present_script_operation(*task.operation, prompt);
}

bool FieldDialogueRuntime::present_script_operation(
    const scripting::ScriptNativeOperation& operation,
    const StokerMotelPromptId prompt, const std::size_t service_depth) {
    active_prompt_ = prompt;
    active_script_operation_ = operation;
    active_script_menu_.reset();
    script_task_waiting_ = true;
    if (operation.type == scripting::ScriptNativeOperation::Type::ui_say) {
        if (!operation.dialogue.has_value()) {
            return open_source_boundary(
                "dialogue script emitted an empty speaker/event request");
        }
        const auto& dialogue = *operation.dialogue;
        active_speaker_profile_ =
            script_runtime_ != nullptr
                ? script_runtime_->speaker_profile(dialogue.speaker_id)
                : std::nullopt;
        if (!active_speaker_profile_.has_value() ||
            active_speaker_profile_->text_profile != dialogue.text_profile) {
            return open_source_boundary(
                "dialogue script selected an unbound Lua speaker profile");
        }
        const DialogueTextResolver resolver =
            [this](const std::string_view alias) {
                return resolve_text(alias);
            };
        const auto view = make_script_dialogue_panel_view(
            present_speaker(*active_speaker_profile_), dialogue.event_id,
            resolver);
        if (!view.has_value()) {
            return open_source_boundary(
                "dialogue script selected an unbound speaker profile");
        }
        panel_ = *view;
        if (!panel_.source_complete) {
            return open_source_boundary(
                panel_.source_boundary_reason.empty()
                    ? "dialogue script selected an unbound text event"
                    : std::string(panel_.source_boundary_reason));
        }
        return true;
    }
    if (operation.type == scripting::ScriptNativeOperation::Type::ui_menu &&
        operation.menu.has_value()) {
        active_script_menu_ = operation.menu;
        active_speaker_profile_.reset();
        std::optional<DialogueSpeakerPresentation> speaker;
        if (!operation.menu->speaker_id.empty()) {
            active_speaker_profile_ =
                script_runtime_ != nullptr
                    ? script_runtime_->speaker_profile(
                          operation.menu->speaker_id)
                    : std::nullopt;
            if (!active_speaker_profile_.has_value() ||
                active_speaker_profile_->text_profile !=
                    operation.menu->text_profile) {
                return open_source_boundary(
                    "dialogue script menu selected an unbound Lua speaker profile");
            }
            speaker = present_speaker(*active_speaker_profile_);
        }
        const DialogueTextResolver resolver =
            [this](const std::string_view alias) {
                return resolve_text(alias);
            };
        std::vector<std::pair<std::string, std::string>> choices;
        choices.reserve(operation.menu->choices.size());
        for (const auto& choice : operation.menu->choices) {
            choices.emplace_back(choice.id, choice.label_key);
        }
        panel_ = make_script_menu_panel_view(
            speaker, operation.menu->title_key, choices, resolver);
        if (!panel_.source_complete) {
            return open_source_boundary(
                panel_.source_boundary_reason.empty()
                    ? "dialogue script menu has missing content"
                    : std::string(panel_.source_boundary_reason));
        }
        return true;
    }
    if (operation.type ==
        scripting::ScriptNativeOperation::Type::service_call) {
        if (service_depth >= 8U || script_runtime_ == nullptr ||
            operation.service_id.empty()) {
            return open_source_boundary(
                "dialogue script service call exceeded its limit");
        }
        const auto result = invoke_script_service(operation.service_id);
        if (!result.has_value()) {
            return open_source_boundary(
                "dialogue script requested an unavailable native service");
        }
        const auto task = script_runtime_->complete_native_service_result(
            *result);
        if (task.status ==
            scripting::ScriptTaskStep::Status::awaiting_native_operation) {
            if (!task.operation.has_value()) {
                return open_source_boundary(
                    "native service resumed without a script operation");
            }
            return present_script_operation(*task.operation, prompt,
                                            service_depth + 1U);
        }
        if (task.status == scripting::ScriptTaskStep::Status::completed) {
            script_task_waiting_ = false;
            active_script_operation_.reset();
            active_script_menu_.reset();
            close();
            return true;
        }
        return open_source_boundary(
            script_runtime_->failure().empty()
                ? "dialogue script failed after a native service"
                : script_runtime_->failure());
    }
    return open_source_boundary(
        "dialogue script requested an unsupported native operation");
}

std::optional<std::string> FieldDialogueRuntime::invoke_script_service(
    const std::string_view service_id) {
    if (service_id == "motel.pay_room") {
        if (!room_payment_callback_) {
            return std::nullopt;
        }
        const auto outcome = room_payment_callback_();
        if (!outcome.has_value()) {
            return std::nullopt;
        }
        if (outcome->result ==
                StokerMotelRoomPaymentResult::insufficient_funds &&
            outcome->prompt == StokerMotelRoomPaymentPrompt::get_patched &&
            !outcome->hotel_options_menu_follows) {
            hotel_options_menu_follows_ = false;
            return "insufficient";
        }
        if (outcome->result == StokerMotelRoomPaymentResult::room_paid &&
            outcome->prompt ==
                StokerMotelRoomPaymentPrompt::bring_your_own_towels &&
            outcome->hotel_options_menu_follows) {
            hotel_options_menu_follows_ = true;
            return "paid";
        }
        return std::nullopt;
    }
    if (service_id == "michael_package.prepare_debt_offer" ||
        service_id == "michael_package.jump_house_referral") {
        if (!followup_command_callback_) {
            return std::nullopt;
        }
        const bool debt_offer =
            service_id == "michael_package.prepare_debt_offer";
        const auto command = debt_offer
            ? StokerMotelFollowupCommand::debt_collection
            : StokerMotelFollowupCommand::jump_house_referral;
        const auto expected = debt_offer
            ? StokerMotelFollowupPrompt::michael_debt_question
            : StokerMotelFollowupPrompt::jump_house_nuyen_referral;
        const auto outcome = followup_command_callback_(command);
        if (!outcome.has_value() || outcome->prompt != expected) {
            return std::nullopt;
        }
        return "ready";
    }
    if (service_id == "michael_package.pay_debt") {
        if (!debt_payment_callback_) {
            return std::nullopt;
        }
        const auto outcome = debt_payment_callback_();
        if (!outcome.has_value()) {
            return std::nullopt;
        }
        if (outcome->result ==
                StokerMotelDebtPaymentResult::insufficient_funds &&
            outcome->prompt ==
                StokerMotelDebtPaymentPrompt::jump_house_nuyen_referral &&
            !outcome->michael_package_sequence_follows) {
            return "insufficient";
        }
        if (outcome->result == StokerMotelDebtPaymentResult::debt_paid &&
            outcome->prompt ==
                StokerMotelDebtPaymentPrompt::michael_package_contents &&
            outcome->michael_package_sequence_follows) {
            return "paid";
        }
        return std::nullopt;
    }
    return std::nullopt;
}

bool FieldDialogueRuntime::open_source_boundary(std::string reason) {
    if (script_task_waiting_ && script_runtime_ != nullptr) {
        script_runtime_->cancel_task();
    }
    boundary_reason_ = std::move(reason);
    panel_ = DialoguePanelView{};
    panel_.open = true;
    panel_.actor_kind = DialoguePanelActorKind::field_npc;
    panel_.actor_name = "";
    panel_.kind = DialoguePanelKind::source_boundary;
    panel_.source_complete = false;
    panel_.source_boundary_reason = boundary_reason_;
    active_prompt_ = StokerMotelPromptId::source_boundary;
    active_speaker_profile_.reset();
    active_script_operation_.reset();
    active_script_menu_.reset();
    script_task_waiting_ = false;
    hotel_options_menu_follows_ = false;
    if (source_boundary_callback_) {
        source_boundary_callback_(panel_.source_boundary_reason);
    }
    return false;
}

std::string_view FieldDialogueRuntime::resolve_text(
    const std::string_view alias) const {
    const auto* value = script_runtime_ != nullptr
                            ? script_runtime_->dialogue_text(alias)
                            : nullptr;
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

    if (active_script_operation_.has_value() &&
        active_script_operation_->type ==
            scripting::ScriptNativeOperation::Type::ui_say) {
        if (panel_.has_more && (event_word & kControllerAEvent) == 0U) {
            return FieldDialogueAdvanceResult::waiting_for_controller;
        }
        if (!script_task_waiting_ || script_runtime_ == nullptr) {
            open_source_boundary(
                "dialogue script is not waiting for the displayed page");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        const auto prompt = active_prompt_;
        const auto task = script_runtime_->complete_native_operation(true);
        if (task.status ==
            scripting::ScriptTaskStep::Status::awaiting_native_operation) {
            if (!task.operation.has_value() ||
                !present_script_operation(*task.operation, prompt)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        script_task_waiting_ = false;
        active_script_operation_.reset();
        if (task.status == scripting::ScriptTaskStep::Status::completed) {
            const bool hotel_options_requested =
                task.result_id == std::optional<std::string>{"hotel-options"};
            close();
            return hotel_options_requested
                       ? FieldDialogueAdvanceResult::hotel_options_requested
                       : FieldDialogueAdvanceResult::returned_to_field;
        }
        open_source_boundary(
            script_runtime_->failure().empty()
                ? "dialogue script failed after the speaker page"
                : script_runtime_->failure());
        return FieldDialogueAdvanceResult::source_boundary;
    }

    // 0x1F3C4 tests A first, then B and C. A title-only source record uses
    // the shared wait-for-controller path; it is not a hidden fourth option.
    // The successful room-payment record carries a caller follow-up marker,
    // so its wait ends at the unresolved Hotel Options boundary.
    if (panel_.option_count == 0U) {
        const bool hotel_options_menu_follows =
            active_prompt_ == StokerMotelPromptId::room_paid &&
            hotel_options_menu_follows_;
        if (hotel_options_menu_follows) {
            close();
            return FieldDialogueAdvanceResult::hotel_options_requested;
        }
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
    std::optional<StokerMotelPromptCommand> command;
    if (active_script_menu_.has_value()) {
        if (!script_task_waiting_ || script_runtime_ == nullptr ||
            option_index >= active_script_menu_->choices.size()) {
            open_source_boundary(
                "Stoker script menu is not waiting for a valid selection");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        const auto& selected_id =
            active_script_menu_->choices[option_index].id;
        const auto prompt = active_prompt_;
        const auto task =
            script_runtime_->complete_native_menu_choice(selected_id);
        if (task.status ==
            scripting::ScriptTaskStep::Status::awaiting_native_operation) {
            if (!task.operation.has_value() ||
                !present_script_operation(*task.operation, prompt)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        if (task.status != scripting::ScriptTaskStep::Status::completed) {
            open_source_boundary(script_runtime_->failure().empty()
                                     ? "dialogue script failed after a menu choice"
                                     : script_runtime_->failure());
            return FieldDialogueAdvanceResult::source_boundary;
        }
        script_task_waiting_ = false;
        active_script_operation_.reset();
        active_script_menu_.reset();
        const bool hotel_options_requested =
            task.result_id == std::optional<std::string>{"hotel-options"};
        close();
        return hotel_options_requested
                   ? FieldDialogueAdvanceResult::hotel_options_requested
                   : FieldDialogueAdvanceResult::returned_to_field;
    } else {
        command = stoker_motel_prompt_command(active_prompt_, option_index);
    }
    if (!command.has_value()) {
        open_source_boundary("selected Stoker option has no command record");
        return FieldDialogueAdvanceResult::source_boundary;
    }
    if (*command == StokerMotelPromptCommand::resolve_arrival) {
        const auto result = resolve_stoker_motel_arrival_command(
            arrival_command_callbacks_);
        if (result == StokerMotelArrivalCommandResult::manager_greeting) {
            if (!open_stoker_prompt(StokerMotelPromptId::manager_greeting)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        if (result == StokerMotelArrivalCommandResult::street_ambush) {
            if (!open_stoker_prompt(StokerMotelPromptId::street_ambush)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        open_source_boundary(
            "Stoker arrival random or ambush-effect callback is unresolved");
        return FieldDialogueAdvanceResult::source_boundary;
    }
    if (*command == StokerMotelPromptCommand::pay_room) {
        if (!room_payment_callback_) {
            open_source_boundary("Stoker room-payment state is unresolved");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        const auto outcome = room_payment_callback_();
        if (!outcome.has_value()) {
            open_source_boundary("Stoker room-payment state is unresolved");
            return FieldDialogueAdvanceResult::source_boundary;
        }

        StokerMotelPromptId result_prompt{};
        if (outcome->result ==
                StokerMotelRoomPaymentResult::insufficient_funds &&
            outcome->prompt == StokerMotelRoomPaymentPrompt::get_patched &&
            !outcome->hotel_options_menu_follows) {
            result_prompt = StokerMotelPromptId::room_insufficient;
        } else if (outcome->result ==
                       StokerMotelRoomPaymentResult::room_paid &&
                   outcome->prompt ==
                       StokerMotelRoomPaymentPrompt::bring_your_own_towels &&
                   outcome->hotel_options_menu_follows) {
            result_prompt = StokerMotelPromptId::room_paid;
        } else {
            open_source_boundary(
                "Stoker room-payment result is inconsistent");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (!open_stoker_prompt(result_prompt)) {
            return FieldDialogueAdvanceResult::source_boundary;
        }
        hotel_options_menu_follows_ = outcome->hotel_options_menu_follows;
        return FieldDialogueAdvanceResult::prompt_advanced;
    }

    if (*command == StokerMotelPromptCommand::pay_michael_debt) {
        if (!debt_payment_callback_) {
            open_source_boundary("Stoker debt-payment state is unresolved");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        const auto outcome = debt_payment_callback_();
        if (!outcome.has_value()) {
            open_source_boundary(
                "Stoker paid-debt effect and package state are unresolved");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (outcome->result ==
                StokerMotelDebtPaymentResult::insufficient_funds &&
            outcome->prompt ==
                StokerMotelDebtPaymentPrompt::jump_house_nuyen_referral &&
            !outcome->michael_package_sequence_follows) {
            if (!open_stoker_prompt(StokerMotelPromptId::jump_house_referral)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        if (outcome->result == StokerMotelDebtPaymentResult::debt_paid &&
            outcome->prompt ==
                StokerMotelDebtPaymentPrompt::michael_package_contents &&
            outcome->michael_package_sequence_follows) {
            if (!open_stoker_prompt(StokerMotelPromptId::package_handoff)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        open_source_boundary(
            "Stoker debt-payment result is inconsistent");
        return FieldDialogueAdvanceResult::source_boundary;
    }

    if (*command == StokerMotelPromptCommand::open_debt_question ||
        *command == StokerMotelPromptCommand::open_jump_house_referral) {
        if (!followup_command_callback_) {
            open_source_boundary("Stoker follow-up flag state is unresolved");
            return FieldDialogueAdvanceResult::source_boundary;
        }

        const bool opens_debt_question =
            *command == StokerMotelPromptCommand::open_debt_question;
        const auto followup_command = opens_debt_question
            ? StokerMotelFollowupCommand::debt_collection
            : StokerMotelFollowupCommand::jump_house_referral;
        const auto expected_prompt = opens_debt_question
            ? StokerMotelFollowupPrompt::michael_debt_question
            : StokerMotelFollowupPrompt::jump_house_nuyen_referral;
        const auto outcome = followup_command_callback_(followup_command);
        if (!outcome.has_value()) {
            open_source_boundary("Stoker follow-up flag state is unresolved");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (outcome->prompt != expected_prompt) {
            open_source_boundary("Stoker follow-up command returned an inconsistent prompt");
            return FieldDialogueAdvanceResult::source_boundary;
        }

        const auto followup_prompt = opens_debt_question
            ? StokerMotelPromptId::debt_question
            : StokerMotelPromptId::jump_house_referral;
        if (!open_stoker_prompt(followup_prompt)) {
            return FieldDialogueAdvanceResult::source_boundary;
        }
        return FieldDialogueAdvanceResult::prompt_advanced;
    }

    if (*command == StokerMotelPromptCommand::open_room_offer) {
        return open_stoker_prompt(StokerMotelPromptId::room_offer)
                   ? FieldDialogueAdvanceResult::prompt_advanced
                   : FieldDialogueAdvanceResult::source_boundary;
    }
    if (*command == StokerMotelPromptCommand::open_leave_notice) {
        return open_stoker_prompt(StokerMotelPromptId::leave_notice)
                   ? FieldDialogueAdvanceResult::prompt_advanced
                   : FieldDialogueAdvanceResult::source_boundary;
    }

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
    if (script_task_waiting_ && script_runtime_ != nullptr) {
        script_runtime_->cancel_task();
    }
    panel_ = DialoguePanelView{};
    active_prompt_ = StokerMotelPromptId::source_boundary;
    active_speaker_profile_.reset();
    active_script_operation_.reset();
    active_script_menu_.reset();
    script_task_waiting_ = false;
    boundary_reason_.clear();
    hotel_options_menu_follows_ = false;
}

void FieldDialogueRuntime::paint(QPainter& painter) const {
    presenter_.paint(painter, panel_);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
