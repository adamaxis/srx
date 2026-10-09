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

std::optional<StokerMotelPromptOptionRole> script_option_role(
    const std::string_view id) {
    if (id == "crash") {
        return StokerMotelPromptOptionRole::rent_room;
    }
    if (id == "leave") {
        return StokerMotelPromptOptionRole::leave_motel;
    }
    if (id == "package") {
        return StokerMotelPromptOptionRole::collect_package;
    }
    return std::nullopt;
}

std::optional<StokerMotelPromptCommand> script_option_command(
    const std::string_view id) {
    if (id == "crash") {
        return StokerMotelPromptCommand::open_room_offer;
    }
    if (id == "leave") {
        return StokerMotelPromptCommand::open_leave_notice;
    }
    if (id == "package") {
        return StokerMotelPromptCommand::open_debt_question;
    }
    return std::nullopt;
}

bool script_menu_choice_supported(
    const StokerMotelPromptOptionRole role,
    const StokerMotelPromptCommand command) noexcept {
    return (role == StokerMotelPromptOptionRole::rent_room &&
            command == StokerMotelPromptCommand::open_room_offer) ||
           (role == StokerMotelPromptOptionRole::leave_motel &&
            command == StokerMotelPromptCommand::open_leave_notice) ||
           (role == StokerMotelPromptOptionRole::collect_package &&
            command == StokerMotelPromptCommand::open_debt_question);
}

std::optional<StokerMotelPromptView> script_prompt_view(
    const StokerMotelPromptId prompt,
    const scripting::ScriptMenuDefinition& menu) {
    StokerMotelPromptView view;
    view.id = prompt;
    view.title_alias = menu.title_key;
    for (const auto& script_choice : menu.choices) {
        const auto role = script_option_role(script_choice.id);
        const auto command = script_option_command(script_choice.id);
        if (!role.has_value() || !command.has_value() ||
            !script_menu_choice_supported(*role, *command) ||
            view.option_count >= view.options.size()) {
            return std::nullopt;
        }
        view.options[view.option_count++] =
            {*role, script_choice.label_key, *command};
    }
    return view;
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
    const auto text = load_dialogue_text_catalog(assets, "dialogue.stoker.motel");
    text_catalog_ = text.catalog;

    if (!presenter_.load_assets(assets)) {
        failure_ = presenter_.failure();
        assets_loaded_ = false;
        if (source_boundary_callback_) {
            source_boundary_callback_("field.dialogue.shared_assets_unavailable");
        }
        return false;
    }

    // Speaker-specific tables remain controller-owned, while the presenter
    // loads all available named portraits without choosing one itself.
    assets_loaded_ = true;
    return true;
}

bool FieldDialogueRuntime::open_field_exit_prompt(
    const FieldExitPromptSelection& selection) {
    opening_intro_package_offer_follows_ = false;
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
    opening_intro_package_offer_follows_ = false;
    if (prompt == StokerMotelPromptId::manager_greeting ||
        prompt == StokerMotelPromptId::manager_greeting_with_package) {
        if (script_runtime_ != nullptr) {
            script_runtime_->set_location_context(
                "motel", "stoker_motel", "stoker.motel");
        }
        const bool completed =
            prompt == StokerMotelPromptId::manager_greeting;
        story_facts_["michael_debt_paid"] = completed;
        story_facts_["michael_package_collected"] = completed;
        if (script_runtime_ != nullptr) {
            script_runtime_->set_story_state(story_facts_);
        }
    }
    return open_stoker_prompt(prompt);
}

bool FieldDialogueRuntime::open_stoker_story_entry(
    const StokerMotelStoryEntryAction action,
    const StokerMotelStoryState& story_state) {
    story_facts_ = {
        {"opening_dialogue_seen", story_state.opening_dialogue_seen},
        {"michael_debt_paid", story_state.michael_debt_paid},
        {"michael_package_collected", story_state.michael_package_collected}};
    if (script_runtime_ != nullptr) {
        script_runtime_->set_location_context("motel", "stoker_motel");
        script_runtime_->set_story_state(story_facts_);
    }
    opening_intro_package_offer_follows_ = false;
    switch (action) {
    case StokerMotelStoryEntryAction::opening_dialogue:
        opening_intro_package_offer_follows_ = true;
        if (open_stoker_prompt(StokerMotelPromptId::arrival)) {
            return true;
        }
        opening_intro_package_offer_follows_ = false;
        return false;
    case StokerMotelStoryEntryAction::package_offer:
        return open_stoker_prompt(
            StokerMotelPromptId::manager_greeting_with_package);
    case StokerMotelStoryEntryAction::ordinary_motel:
        return open_stoker_prompt(StokerMotelPromptId::manager_greeting);
    }
    return open_source_boundary("Stoker story entry action is not represented");
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
    if (script_menu_task_waiting_ && script_runtime_ != nullptr) {
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
    active_script_menu_.reset();
    script_menu_task_waiting_ = false;
    StokerMotelPromptView source_view;
    if (prompt == StokerMotelPromptId::manager_greeting ||
        prompt == StokerMotelPromptId::manager_greeting_with_package) {
        if (script_runtime_ == nullptr) {
            return open_source_boundary(
                "Stoker manager menu has no gameplay script runtime");
        }
        script_runtime_->set_location_context(
            "motel", "stoker_motel", "stoker.motel");
        script_runtime_->set_story_state(story_facts_);
        const auto task = script_runtime_->start_task(
            "motel", "manager_interaction");
        if (task.status !=
                scripting::ScriptTaskStep::Status::awaiting_native_operation ||
            !task.operation.has_value() ||
            task.operation->type !=
                scripting::ScriptNativeOperation::Type::ui_menu ||
            !task.operation->menu.has_value()) {
            return open_source_boundary(script_runtime_->failure());
        }
        active_script_menu_ = task.operation->menu;
        script_menu_task_waiting_ = true;
        const auto script_view = script_prompt_view(
            prompt, *active_script_menu_);
        if (!script_view.has_value()) {
            return open_source_boundary(
                "Stoker manager script contains an unsupported role or action");
        }
        source_view = *script_view;
    } else {
        active_script_menu_.reset();
        source_view = make_stoker_motel_prompt(prompt);
    }
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
    if (script_menu_task_waiting_ && script_runtime_ != nullptr) {
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
    active_script_menu_.reset();
    script_menu_task_waiting_ = false;
    hotel_options_menu_follows_ = false;
    opening_intro_package_offer_follows_ = false;
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
        if (!script_menu_task_waiting_ || script_runtime_ == nullptr ||
            option_index >= active_script_menu_->choices.size()) {
            open_source_boundary(
                "Stoker script menu is not waiting for a valid selection");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        const auto& selected_id =
            active_script_menu_->choices[option_index].id;
        const auto task =
            script_runtime_->complete_native_menu_choice(selected_id);
        if (task.status != scripting::ScriptTaskStep::Status::completed ||
            task.result_id != selected_id) {
            open_source_boundary(script_runtime_->failure().empty()
                                     ? "Stoker script did not return its selected choice"
                                     : script_runtime_->failure());
            return FieldDialogueAdvanceResult::source_boundary;
        }
        script_menu_task_waiting_ = false;
        command = script_option_command(*task.result_id);
    } else {
        command = stoker_motel_prompt_command(active_prompt_, option_index);
    }
    if (!command.has_value()) {
        open_source_boundary("selected Stoker option has no command record");
        return FieldDialogueAdvanceResult::source_boundary;
    }
    if (*command == StokerMotelPromptCommand::resolve_arrival) {
        if (opening_intro_package_offer_follows_) {
            opening_intro_package_offer_follows_ = false;
            if (!open_stoker_prompt(
                    StokerMotelPromptId::manager_greeting_with_package)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
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
    if (script_menu_task_waiting_ && script_runtime_ != nullptr) {
        script_runtime_->cancel_task();
    }
    panel_ = DialoguePanelView{};
    active_prompt_ = StokerMotelPromptId::source_boundary;
    active_script_menu_.reset();
    script_menu_task_waiting_ = false;
    boundary_reason_.clear();
    hotel_options_menu_follows_ = false;
    opening_intro_package_offer_follows_ = false;
}

void FieldDialogueRuntime::paint(QPainter& painter) const {
    presenter_.paint(painter, panel_);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
