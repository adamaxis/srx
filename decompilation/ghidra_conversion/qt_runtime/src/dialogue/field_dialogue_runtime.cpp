#include "field_dialogue_runtime.hpp"

#include <QPainter>

#include <array>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint16_t kControllerAEvent = 0x4000U;
constexpr std::uint16_t kControllerBEvent = 0x1000U;
constexpr std::uint16_t kControllerCEvent = 0x2000U;
constexpr std::uint16_t kControllerStartEvent = 0x8000U;
constexpr std::uint16_t kControllerUpEvent = 0x0100U;
constexpr std::uint16_t kControllerDownEvent = 0x0200U;
// 0x1F3C4 page waits: the pad is not sampled for this many frames after the
// press that drew the page. A choice page runs 0xC24 with d5=0x0A (0x1F502)
// and then the 0x1F50C loop's 0xBD2 before 0x195C: 12 frames [C: ROM;
// emulator taps at +12 ignored, +13 accepted]. A record without options
// takes 0x1F592 (0xC24 with d5=0x1E, then 0xD94 waits for any FFDD22
// high-byte edge) [C: emulator taps at +31 ignored, +32 accepted].
enum class DialogueInputLockout : std::uint8_t {
    none = 0U,
    choice_page = 12U,
    closing_page = 31U,
};

constexpr std::uint16_t kAnyDialogueEvent = kControllerAEvent |
                                             kControllerBEvent |
                                             kControllerCEvent |
                                             kControllerStartEvent;

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

void FieldDialogueRuntime::set_native_script_service_callback(
    std::function<std::optional<std::string>(std::string_view)> callback) {
    native_script_service_callback_ = std::move(callback);
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
    if (script_runtime_ == nullptr) {
        return open_source_boundary(
            "Stoker field-exit graph has no gameplay script runtime");
    }
    script_runtime_->set_location_context(
        "motel", "stoker_motel", "stoker.motel", "stoker");
    // 0x1F65A's typed selection is carried as field mode/event/result facts;
    // the JSON graph resolves the signed branch and the prompt commands.
    return open_gameplay_task("michael_package",
                              "present_field_exit_prompt");
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldDialogueRuntime::open_stoker_prompt_for_dump(
    const StokerMotelPromptId prompt) {
    if (prompt == StokerMotelPromptId::manager_greeting_with_package) {
        // This prompt ID is a diagnostic alias for indexed trigger 0. Start
        // its JSON graph with the source-default New Game gate and location.
        return open_stoker_interaction_for_dump();
    }
    if (prompt == StokerMotelPromptId::manager_greeting) {
        if (script_runtime_ != nullptr) {
            script_runtime_->set_location_context(
                "motel", "stoker_motel", "stoker.motel", "stoker");
        }
    }
    return open_stoker_prompt(prompt);
}
#endif // SHADOWRUN_QT_QA_DRIVER


bool FieldDialogueRuntime::open_stoker_interaction() {
    if (script_runtime_ != nullptr) {
        script_runtime_->set_location_context(
            "motel", "stoker_motel", "stoker.motel", "stoker");
    }
    return start_script_task("michael_package", "stoker_interaction",
                             StokerMotelPromptId::manager_greeting);
}

bool FieldDialogueRuntime::open_gameplay_task(
    const std::string_view module, const std::string_view function) {
    if (panel_.open) {
        return false;
    }
    return start_script_task(module, function,
                             StokerMotelPromptId::source_boundary);
}

bool FieldDialogueRuntime::present_source_boundary_for_host(
    std::string reason) {
    return open_source_boundary(std::move(reason));
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldDialogueRuntime::open_stoker_interaction_for_dump() {
    if (script_runtime_ != nullptr) {
        script_runtime_->set_story_state({
            {"story_flag_bit_0", false}});
        script_runtime_->set_location_context(
            "motel", "stoker_motel", "stoker.motel", "stoker");
        script_runtime_->set_field_mode_context(
            scripting::ScriptFieldModeContext{2U, 2U, 0});
    }
    return open_stoker_interaction();
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
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
#endif // SHADOWRUN_QT_QA_DRIVER


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
    hosted_choice_list_ = false;
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
    if (script_runtime_ == nullptr && source_view.source_complete) {
        return open_source_boundary(
            "Stoker prompt has no gameplay script runtime");
    }
    active_speaker_profile_ = stoker_speaker_profile_;
    std::string title_key;
    std::array<std::string, 3> option_label_key_storage{};
    if (source_view.source_complete && !source_view.event_id.empty()) {
        const auto text_keys = script_runtime_->dialogue_event_text_keys(
            "stoker", "stoker.motel", source_view.event_id);
        if (!text_keys.has_value()) {
            return open_source_boundary(
                script_runtime_->failure().empty()
                    ? "Stoker prompt event has no Lua text mapping"
                    : script_runtime_->failure());
        }
        title_key = text_keys->title_key;
        if (source_view.has_more && source_view.option_count != 0U) {
            option_label_key_storage[0] = text_keys->continuation_key;
        }
    } else if (source_view.source_complete && !source_view.menu_id.empty()) {
        scripting::ScriptMenuDefinition menu;
        menu.id = std::string(source_view.menu_id);
        menu.text_profile = "stoker.motel";
        menu.speaker_id = "stoker";
        for (std::size_t index = 0U; index < source_view.option_count;
             ++index) {
            menu.choices.push_back({
                std::string(source_view.options[index].choice_id), {}});
        }
        const auto resolved_menu =
            script_runtime_->resolve_dialogue_menu(std::move(menu));
        if (!resolved_menu.has_value()) {
            return open_source_boundary(
                script_runtime_->failure().empty()
                    ? "Stoker prompt menu has no Lua text mapping"
                    : script_runtime_->failure());
        }
        title_key = resolved_menu->title_key;
        for (std::size_t index = 0U;
             index < resolved_menu->choices.size(); ++index) {
            option_label_key_storage[index] =
                resolved_menu->choices[index].label_key;
        }
    }
    std::array<std::string_view, 3> option_label_keys{};
    for (std::size_t index = 0U; index < option_label_keys.size(); ++index) {
        option_label_keys[index] = option_label_key_storage[index];
    }
    const DialogueTextResolver resolver = [this](const std::string_view alias) {
        return resolve_text(alias);
    };
    panel_ = make_dialogue_panel_view(
        source_view, present_speaker(*active_speaker_profile_), title_key,
        option_label_keys, resolver);
    if (!panel_.source_complete && panel_.source_boundary_reason.empty()) {
        panel_.source_boundary_reason =
            "Stoker dialogue source projection is incomplete";
    }
    input_lockout_frames_ = static_cast<std::uint8_t>(
        panel_.option_count == 0U ? DialogueInputLockout::closing_page
                                  : DialogueInputLockout::choice_page);
    return true;
}

bool FieldDialogueRuntime::start_script_task(
    const std::string_view module, const std::string_view function,
    const StokerMotelPromptId prompt) {
    last_gameplay_task_result_id_.reset();
    last_gameplay_task_outcome_.reset();
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

std::optional<FieldDialogueAdvanceResult>
FieldDialogueRuntime::resolve_typed_outcome(
    const scripting::ScriptTaskStep& task) {
    if (!task.dialogue_outcome.has_value()) {
        return std::nullopt;
    }
    last_gameplay_task_outcome_ = task.dialogue_outcome;
    close();
    if (const auto* boundary = std::get_if<
            scripting::SourceBoundaryOutcome>(&*task.dialogue_outcome)) {
        open_source_boundary(boundary->reason);
        return FieldDialogueAdvanceResult::source_boundary;
    }
    return FieldDialogueAdvanceResult::typed_outcome_requested;
}

bool FieldDialogueRuntime::present_script_operation(
    const scripting::ScriptNativeOperation& operation,
    const StokerMotelPromptId prompt, const std::size_t service_depth) {
    active_prompt_ = prompt;
    active_script_operation_ = operation;
    active_script_menu_.reset();
    hosted_choice_list_ = false;
    script_task_waiting_ = true;
    if (operation.type == scripting::ScriptNativeOperation::Type::ui_say) {
        if (!operation.dialogue.has_value()) {
            return open_source_boundary(
                "dialogue script emitted an empty speaker/event request");
        }
        const auto& dialogue = *active_script_operation_->dialogue;
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
        std::optional<DialoguePanelView> view;
        if (dialogue.presentation ==
            scripting::ScriptDialoguePresentation::field_mode_text) {
            if (!dialogue.inline_text.empty() ||
                !dialogue.continuation_key.empty()) {
                return open_source_boundary(
                    "field mode-text page cannot contain inline text or a continuation label");
            }
            const auto text_keys =
                script_runtime_->dialogue_event_text_keys(
                    dialogue.speaker_id, dialogue.text_profile,
                    dialogue.event_id);
            if (!text_keys.has_value()) {
                return open_source_boundary(
                    script_runtime_->failure().empty()
                        ? "field mode-text page selected an unbound Lua event"
                        : script_runtime_->failure());
            }
            view = make_script_mode_text_panel_view(
                present_speaker(*active_speaker_profile_),
                text_keys->title_key, resolver);
        } else if (!dialogue.inline_text.empty()) {
            if (!dialogue.event_id.empty()) {
                return open_source_boundary(
                    "inline dialogue also supplied a catalog event id");
            }
            view = make_script_dialogue_panel_view_text(
                present_speaker(*active_speaker_profile_),
                dialogue.inline_text, dialogue.continuation_key, resolver);
        } else {
            if (!dialogue.continuation_key.empty()) {
                return open_source_boundary(
                    "catalog dialogue supplied an inline continuation key");
            }
            const auto text_keys =
                script_runtime_->dialogue_event_text_keys(
                    dialogue.speaker_id, dialogue.text_profile,
                    dialogue.event_id);
            if (!text_keys.has_value()) {
                return open_source_boundary(
                    script_runtime_->failure().empty()
                        ? "dialogue script selected an unbound Lua event"
                        : script_runtime_->failure());
            }
            view = make_script_dialogue_panel_view(
                present_speaker(*active_speaker_profile_),
                text_keys->title_key, text_keys->continuation_key, resolver);
        }
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
        if (dialogue.presentation ==
                scripting::ScriptDialoguePresentation::field_mode_text ||
            dialogue.input_delay_frames != 0U) {
            input_lockout_frames_ = dialogue.input_delay_frames;
        } else {
            // A regular closing page uses the 0x1F592 30-frame wait.
            input_lockout_frames_ = static_cast<std::uint8_t>(
                panel_.has_more ? DialogueInputLockout::none
                                : DialogueInputLockout::closing_page);
        }
        return true;
    }
    if (operation.type == scripting::ScriptNativeOperation::Type::ui_menu &&
        operation.menu.has_value()) {
        if (script_runtime_ == nullptr) {
            return open_source_boundary(
                "dialogue menu has no gameplay script runtime");
        }
        active_script_menu_ =
            script_runtime_->resolve_dialogue_menu(*operation.menu);
        if (!active_script_menu_.has_value()) {
            return open_source_boundary(
                script_runtime_->failure().empty()
                    ? "dialogue script menu has no Lua content mapping"
                    : script_runtime_->failure());
        }
        for (auto& choice : active_script_menu_->choices) {
            if (choice.inline_label.empty()) {
                continue;
            }
            const auto* source_label =
                script_runtime_->dialogue_text(choice.label_key);
            if (source_label == nullptr || source_label->empty()) {
                return open_source_boundary(
                    "Lua menu inline label has no ROM label text");
            }
            choice.inline_label = *source_label + choice.inline_label;
        }
        const auto& menu = *active_script_menu_;
        active_speaker_profile_.reset();
        std::optional<DialogueSpeakerPresentation> speaker;
        if (!menu.speaker_id.empty()) {
            active_speaker_profile_ =
                script_runtime_ != nullptr
                    ? script_runtime_->speaker_profile(menu.speaker_id)
                    : std::nullopt;
            if (!active_speaker_profile_.has_value() ||
                active_speaker_profile_->text_profile !=
                    menu.text_profile) {
                return open_source_boundary(
                    "dialogue script menu selected an unbound Lua speaker profile");
            }
            speaker = present_speaker(*active_speaker_profile_);
        }
        constexpr std::string_view kInlineChoicePrefix = "\x1Finline:";
        const DialogueTextResolver resolver =
            [this, kInlineChoicePrefix](
                const std::string_view alias) -> std::string_view {
                if (alias.size() >= kInlineChoicePrefix.size() &&
                    alias.compare(0U, kInlineChoicePrefix.size(),
                                  kInlineChoicePrefix) == 0 &&
                    active_script_menu_.has_value()) {
                    const auto choice_id =
                        alias.substr(kInlineChoicePrefix.size());
                    const auto choice = std::find_if(
                        active_script_menu_->choices.begin(),
                        active_script_menu_->choices.end(),
                        [choice_id](const auto& candidate) {
                            return candidate.id == choice_id;
                        });
                    if (choice != active_script_menu_->choices.end()) {
                        return choice->inline_label;
                    }
                }
                return resolve_text(alias);
            };
        std::vector<std::pair<std::string, std::string>> choices;
        choices.reserve(menu.choices.size());
        for (const auto& choice : menu.choices) {
            if (choice.inline_label.empty()) {
                choices.emplace_back(choice.id, choice.label_key);
            } else {
                choices.emplace_back(
                    choice.id,
                    std::string(kInlineChoicePrefix) + choice.id);
            }
        }
        if (menu.input_mode == scripting::ScriptMenuInputMode::cursor &&
            !menu.choices.empty() &&
            menu.selected_index >= menu.choices.size()) {
            return open_source_boundary(
                "Lua cursor menu selected an index outside its choices");
        }
        hosted_choice_list_ =
            menu.input_mode == scripting::ScriptMenuInputMode::button_choice &&
            menu.choices.size() > menu.button_choice_limit;
        const bool cursor_rows =
            menu.input_mode == scripting::ScriptMenuInputMode::cursor ||
            hosted_choice_list_;
        const auto selected = hosted_choice_list_ ? 0U : menu.selected_index;
        if (!menu.inline_prompt) {
            panel_ = make_script_menu_panel_view(
                speaker, menu.title_key, choices, resolver, cursor_rows,
                selected,
                menu.layout == scripting::ScriptMenuLayout::compact_list);
        } else {
            panel_ = make_script_menu_prompt_panel_view(
                speaker, menu.inline_text, choices, resolver, cursor_rows,
                selected);
        }
        if (!panel_.source_complete) {
            return open_source_boundary(
                panel_.source_boundary_reason.empty()
                    ? "dialogue script menu has missing content"
                    : std::string(panel_.source_boundary_reason));
        }
        input_lockout_frames_ = static_cast<std::uint8_t>(
            cursor_rows ? DialogueInputLockout::none
                        : DialogueInputLockout::choice_page);
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
    hosted_choice_list_ = false;
            last_gameplay_task_result_id_ = task.result_id;
            if (const auto outcome = resolve_typed_outcome(task);
                outcome.has_value()) {
                inline_task_advance_result_ = *outcome;
                return true;
            }
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
    if (service_id == "stoker_motel.resolve_arrival") {
        const auto result = resolve_stoker_motel_arrival_command(
            arrival_command_callbacks_);
        if (result == StokerMotelArrivalCommandResult::manager_greeting) {
            return "manager";
        }
        if (result == StokerMotelArrivalCommandResult::street_ambush) {
            return "street_ambush";
        }
        // Preserve the graph's explicit service-unavailable branch. Returning
        // no host value would abort the script before JSON can select its
        // source-boundary node.
        return "service_unavailable";
    }
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
    return native_script_service_callback_
               ? native_script_service_callback_(service_id)
               : std::nullopt;
}

bool FieldDialogueRuntime::open_source_boundary(std::string reason) {
    if (script_task_waiting_ && script_runtime_ != nullptr) {
        script_runtime_->cancel_task();
    }
    boundary_reason_ = std::move(reason);
    input_lockout_frames_ = 0U;
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
    hosted_choice_list_ = false;
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

bool FieldDialogueRuntime::consume_input_lockout_frame() noexcept {
    if (!panel_.open || input_lockout_frames_ == 0U) {
        return false;
    }
    --input_lockout_frames_;
    return true;
}

FieldDialogueAdvanceResult FieldDialogueRuntime::advance_controller_events(
    const std::uint16_t event_word) {
    if (!panel_.open || event_word == 0U) {
        return FieldDialogueAdvanceResult::waiting_for_controller;
    }
    const bool cursor_menu = active_script_menu_.has_value() &&
        active_script_menu_->input_mode ==
            scripting::ScriptMenuInputMode::cursor;
    const bool hosted_list = hosted_choice_list_ &&
                             active_script_menu_.has_value();
    // A closing speaker page (no MORE) is dismissed by any new press,
    // D-pad included [C: ROM 0x1F592 -> 0xD94 tests FFDD22 & 0xFF00 for
    // every record without options; emulator motel 'towels' page].
    constexpr std::uint16_t kControllerDirectionEvents = 0x0F00U;
    const bool closing_say_page =
        (active_script_operation_.has_value() &&
         active_script_operation_->type ==
             scripting::ScriptNativeOperation::Type::ui_say &&
         !panel_.has_more) ||
        (!active_script_operation_.has_value() &&
         !active_script_menu_.has_value() &&
         panel_.kind != DialoguePanelKind::source_boundary &&
         panel_.option_count == 0U);
    const std::uint16_t accepted_events =
        closing_say_page
            ? static_cast<std::uint16_t>(kAnyDialogueEvent |
                                         kControllerDirectionEvents)
            : kAnyDialogueEvent;
    if (!cursor_menu && !hosted_list &&
        (event_word & accepted_events) == 0U) {
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
            if (inline_task_advance_result_.has_value()) {
                const auto result = *inline_task_advance_result_;
                inline_task_advance_result_.reset();
                return result;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        script_task_waiting_ = false;
        active_script_operation_.reset();
        if (task.status == scripting::ScriptTaskStep::Status::completed) {
            last_gameplay_task_result_id_ = task.result_id;
            if (const auto outcome = resolve_typed_outcome(task);
                outcome.has_value()) {
                return *outcome;
            }
            const bool hotel_options_requested =
                task.result_id == std::optional<std::string>{"hotel-options"};
            const bool hospital_options_requested = task.result_id ==
                std::optional<std::string>{"hospital-options"};
            const bool medicine_lodge_wise_one_followup = task.result_id ==
                std::optional<std::string>{
                    "medicine_lodge_wise_one_followup"};
            close();
            if (task.result_id == std::optional<std::string>{
                                    "tabatha_state_unavailable"}) {
                open_source_boundary(
                    "Tarislar City Inn could not apply Tabatha's ROM state changes");
                return FieldDialogueAdvanceResult::source_boundary;
            }
            if (task.result_id == std::optional<std::string>{
                                    "boris_strike_team_unconverted"}) {
                open_source_boundary(
                    "Boris' Brother? branch reaches command 0x2C at 0x21BE4, which starts a Strike Team world effect that is not converted yet");
                return FieldDialogueAdvanceResult::source_boundary;
            }
            if (task.result_id == std::optional<std::string>{
                                    "boris_dialogue_state_unavailable"}) {
                open_source_boundary(
                    "Boris' Greenhouse dialogue needs the saved FFFC26 byte, which this session does not provide");
                return FieldDialogueAdvanceResult::source_boundary;
            }
            if (task.result_id == std::optional<std::string>{
                                    "boris_payment_unavailable"}) {
                open_source_boundary(
                    "Boris' Greenhouse payment needs a resolved credstick balance");
                return FieldDialogueAdvanceResult::source_boundary;
            }
            if (task.result_id == std::optional<std::string>{
                                    "wylie_feature_state_unavailable"}) {
                open_source_boundary(
                    "Wylie's Gala Inn free-stay branch requires runner status byte FF018F, which this restored save does not provide");
                return FieldDialogueAdvanceResult::source_boundary;
            }
            if (task.result_id == std::optional<std::string>{
                                    "wylie_payment_unavailable"}) {
                open_source_boundary(
                    "Wylie's Gala Inn room payment requires a resolved credstick balance");
                return FieldDialogueAdvanceResult::source_boundary;
            }
            if (medicine_lodge_wise_one_followup) {
                return FieldDialogueAdvanceResult::
                    medicine_lodge_wise_one_followup;
            }
            return hotel_options_requested
                       ? FieldDialogueAdvanceResult::hotel_options_requested
                   : hospital_options_requested
                       ? FieldDialogueAdvanceResult::hospital_options_requested
                       : FieldDialogueAdvanceResult::returned_to_field;
        }
        open_source_boundary(
            script_runtime_->failure().empty()
                ? "dialogue script failed after the speaker page"
                : script_runtime_->failure());
        return FieldDialogueAdvanceResult::source_boundary;
    }

    if (cursor_menu) {
        std::string_view input_id;
        const bool vid_phone_contact_menu =
            active_script_menu_->id == "contacts" &&
            active_script_menu_->text_profile == "field.vid_phone";
        if (active_script_menu_->choices.empty()) {
            if ((event_word & kControllerBEvent) != 0U) {
                input_id = "back";
            } else {
                // The source medical screen may have no treatment rows. In
                // that state only its B-button return is actionable.
                return FieldDialogueAdvanceResult::waiting_for_controller;
            }
        } else if ((event_word & kControllerUpEvent) != 0U) {
            input_id = "up";
        } else if ((event_word & kControllerDownEvent) != 0U) {
            input_id = "down";
        } else if (vid_phone_contact_menu &&
                   (event_word & (kControllerAEvent | kControllerBEvent)) !=
                       0U) {
            // ROM 0xF90C/0xF972: A or B activates the selected contact row,
            // including the explicit Next/Prior Screen rows.
            input_id = "confirm";
        } else if (vid_phone_contact_menu &&
                   (event_word & kControllerStartEvent) != 0U) {
            input_id = "cancel";
        } else if (vid_phone_contact_menu &&
                   (event_word & kControllerCEvent) != 0U) {
            // ROM 0xF980 routes C through the field action service; it does
            // not change pages. Leave the phone list open here rather than
            // treating C as an undocumented page shortcut.
            return FieldDialogueAdvanceResult::waiting_for_controller;
        } else if ((event_word & kControllerCEvent) != 0U) {
            input_id = "page";
        } else if ((event_word & kControllerBEvent) != 0U) {
            input_id = "back";
        } else if ((event_word & kControllerAEvent) != 0U) {
            input_id = "confirm";
        } else if ((event_word & kControllerStartEvent) != 0U) {
            input_id = "cancel";
        } else {
            return FieldDialogueAdvanceResult::waiting_for_controller;
        }
        if (!script_task_waiting_ || script_runtime_ == nullptr) {
            open_source_boundary(
                "Lua cursor menu is not waiting for controller input");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        const auto prompt = active_prompt_;
        const auto task =
            script_runtime_->complete_native_menu_input(input_id);
        if (task.status ==
            scripting::ScriptTaskStep::Status::awaiting_native_operation) {
            if (!task.operation.has_value() ||
                !present_script_operation(*task.operation, prompt)) {
                return FieldDialogueAdvanceResult::source_boundary;
            }
            if (inline_task_advance_result_.has_value()) {
                const auto result = *inline_task_advance_result_;
                inline_task_advance_result_.reset();
                return result;
            }
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        if (task.status != scripting::ScriptTaskStep::Status::completed) {
            open_source_boundary(script_runtime_->failure().empty()
                                     ? "Lua cursor menu failed to process input"
                                     : script_runtime_->failure());
            return FieldDialogueAdvanceResult::source_boundary;
        }
        script_task_waiting_ = false;
        active_script_operation_.reset();
        active_script_menu_.reset();
        hosted_choice_list_ = false;
        last_gameplay_task_result_id_ = task.result_id;
        if (const auto outcome = resolve_typed_outcome(task);
            outcome.has_value()) {
            return *outcome;
        }
        const bool returned_from_activity =
            task.result_id ==
            std::optional<std::string>{"return_from_world_activity"};
        const bool unsupported_cyberspace =
            task.result_id ==
            std::optional<std::string>{"cyberspace_not_converted"};
        const bool taxi_selector_requested = task.result_id ==
            std::optional<std::string>{"taxi_selector_requested"};
        const bool hospital_options_requested = task.result_id ==
            std::optional<std::string>{"hospital-options"};
        const bool hospital_medical_attention_requested = task.result_id ==
            std::optional<std::string>{
                "hospital_medical_attention_requested"};
        const bool hospital_cyberware_requested = task.result_id ==
            std::optional<std::string>{"hospital_cyberware_requested"};
        const bool hospital_leave_confirmation_requested = task.result_id ==
            std::optional<std::string>{
                "hospital_leave_confirmation_requested"};
        const bool hospital_left = task.result_id ==
            std::optional<std::string>{"hospital_left"};
        const bool medicine_lodge_store_requested = task.result_id ==
            std::optional<std::string>{"medicine_lodge_store_requested"};
        const bool medicine_lodge_wise_one_followup = task.result_id ==
            std::optional<std::string>{
                "medicine_lodge_wise_one_followup"};
        const bool unresolved_phone_flags = task.result_id ==
            std::optional<std::string>{"phone_contacts_flags_unresolved"};
        const bool unresolved_cyberspace_facts = task.result_id ==
            std::optional<std::string>{"cyberspace_access_facts_unresolved"};
        const bool unresolved_passcode_flags = task.result_id ==
            std::optional<std::string>{"matrix_passcode_flags_unresolved"};
        const bool matrix_search_requested = task.result_id ==
            std::optional<std::string>{"matrix_system_search_requested"};
        std::optional<std::uint8_t> matrix_entry_system_id;
        std::optional<std::uint8_t> matrix_entry_node_id;
        bool invalid_resource_matrix_target = false;
        if (task.result_id.has_value()) {
            constexpr std::string_view kMatrixEntryPrefix = "matrix_entry_";
            constexpr std::string_view kMatrixTargetPrefix =
                "matrix_entry_target_";
            const auto& result_id = *task.result_id;
            if (result_id == "matrix_entry_current") {
                // 0xFFFBBC is the current passcode target in the ROM.
                matrix_entry_system_id = 0xFFU;
            } else if (result_id.compare(0U, kMatrixTargetPrefix.size(),
                                         kMatrixTargetPrefix) == 0) {
                bool target_parsed = false;
                if (result_id.size() > kMatrixTargetPrefix.size()) {
                    const auto suffix = std::string_view(result_id).substr(
                        kMatrixTargetPrefix.size());
                    const auto separator = suffix.find('_');
                    if (separator != std::string_view::npos) {
                        unsigned system_id = 0U;
                        unsigned node_id = 0U;
                        const auto system_text =
                            suffix.substr(0U, separator);
                        const auto node_text = suffix.substr(separator + 1U);
                        const auto parsed_system = std::from_chars(
                            system_text.data(),
                            system_text.data() + system_text.size(), system_id);
                        const auto parsed_node = std::from_chars(
                            node_text.data(),
                            node_text.data() + node_text.size(), node_id);
                        if (parsed_system.ec == std::errc{} &&
                            parsed_system.ptr ==
                                system_text.data() + system_text.size() &&
                            parsed_node.ec == std::errc{} &&
                            parsed_node.ptr ==
                                node_text.data() + node_text.size() &&
                            system_id < 25U && node_id < 27U) {
                            matrix_entry_system_id =
                                static_cast<std::uint8_t>(system_id);
                            matrix_entry_node_id =
                                static_cast<std::uint8_t>(node_id);
                            target_parsed = true;
                        }
                    }
                }
                invalid_resource_matrix_target = !target_parsed;
            } else if (result_id.size() > kMatrixEntryPrefix.size() &&
                       result_id.compare(0U, kMatrixEntryPrefix.size(),
                                         kMatrixEntryPrefix) == 0) {
                unsigned system_id = 0U;
                const auto suffix = std::string_view(result_id).substr(
                    kMatrixEntryPrefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), system_id);
                if (parsed.ec == std::errc{} &&
                    parsed.ptr == suffix.data() + suffix.size() &&
                    system_id < 25U) {
                    matrix_entry_system_id =
                        static_cast<std::uint8_t>(system_id);
                }
            }
        }
        close();
        pending_matrix_entry_system_id_ = matrix_entry_system_id;
        pending_matrix_entry_node_id_ = matrix_entry_node_id;
        if (matrix_search_requested) {
            return FieldDialogueAdvanceResult::matrix_search_requested;
        }
        if (taxi_selector_requested) {
            return FieldDialogueAdvanceResult::taxi_selector_requested;
        }
        if (hospital_options_requested) {
            return FieldDialogueAdvanceResult::hospital_options_requested;
        }
        if (hospital_medical_attention_requested) {
            return FieldDialogueAdvanceResult::
                hospital_medical_attention_requested;
        }
        if (hospital_cyberware_requested) {
            return FieldDialogueAdvanceResult::hospital_cyberware_requested;
        }
        if (hospital_leave_confirmation_requested) {
            return FieldDialogueAdvanceResult::
                hospital_leave_confirmation_requested;
        }
        if (hospital_left) {
            return FieldDialogueAdvanceResult::hospital_left;
        }
        if (medicine_lodge_store_requested) {
            return FieldDialogueAdvanceResult::
                medicine_lodge_store_requested;
        }
        if (medicine_lodge_wise_one_followup) {
            return FieldDialogueAdvanceResult::
                medicine_lodge_wise_one_followup;
        }
        if (matrix_entry_system_id.has_value()) {
            return FieldDialogueAdvanceResult::matrix_entry_requested;
        }
        if (invalid_resource_matrix_target) {
            open_source_boundary(
                "Lua returned an invalid resource-scene Matrix target");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (returned_from_activity) {
            return FieldDialogueAdvanceResult::returned_from_world_activity;
        }
        if (unsupported_cyberspace) {
            open_source_boundary(
                "Cyberspace entry from this field menu is not converted yet");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (unresolved_phone_flags) {
            open_source_boundary(
                "Vid-Phone contact flags are unresolved for this save");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (unresolved_cyberspace_facts) {
            open_source_boundary(
                "Cyberspace prerequisite facts are unresolved for this save");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (task.result_id == std::optional<std::string>{
                "resource_matrix_target_unresolved"}) {
            open_source_boundary(
                "The ROM Matrix target for this resource scene is not mapped");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (unresolved_passcode_flags) {
            open_source_boundary(
                "Matrix passcode flags are unresolved for this save");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        return FieldDialogueAdvanceResult::returned_to_field;
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
    if (hosted_list && panel_.option_count != 0U) {
        const auto count = panel_.option_count;
        if ((event_word & kControllerUpEvent) != 0U) {
            panel_.selected_option = (panel_.selected_option + count - 1U) % count;
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        if ((event_word & kControllerDownEvent) != 0U) {
            panel_.selected_option = (panel_.selected_option + 1U) % count;
            return FieldDialogueAdvanceResult::prompt_advanced;
        }
        if ((event_word & (kControllerAEvent | kControllerBEvent |
                           kControllerCEvent)) == 0U) {
            return FieldDialogueAdvanceResult::waiting_for_controller;
        }
        option_index = panel_.selected_option;
    } else if ((event_word & kControllerAEvent) != 0U) {
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
            if (inline_task_advance_result_.has_value()) {
                const auto result = *inline_task_advance_result_;
                inline_task_advance_result_.reset();
                return result;
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
        hosted_choice_list_ = false;
        last_gameplay_task_result_id_ = task.result_id;
        if (const auto outcome = resolve_typed_outcome(task);
            outcome.has_value()) {
            return *outcome;
        }
        const bool hotel_options_requested =
            task.result_id == std::optional<std::string>{"hotel-options"};
        const bool hospital_options_requested =
            task.result_id == std::optional<std::string>{"hospital-options"};
        const bool medicine_lodge_store_requested = task.result_id ==
            std::optional<std::string>{"medicine_lodge_store_requested"};
        const bool phone_services_unconverted = task.result_id ==
            std::optional<std::string>{"phone_services_not_converted"};
        const bool tabatha_state_unavailable = task.result_id ==
            std::optional<std::string>{"tabatha_state_unavailable"};
        close();
        if (phone_services_unconverted) {
            open_source_boundary(
                "This contact's service transactions are not converted yet");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (tabatha_state_unavailable) {
            open_source_boundary(
                "Tarislar City Inn could not apply Tabatha's ROM state changes");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (task.result_id == std::optional<std::string>{
                                "wylie_feature_state_unavailable"}) {
            open_source_boundary(
                "Wylie's Gala Inn free-stay branch requires runner status byte FF018F, which this restored save does not provide");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (task.result_id == std::optional<std::string>{
                                "wylie_payment_unavailable"}) {
            open_source_boundary(
                "Wylie's Gala Inn room payment requires a resolved credstick balance");
            return FieldDialogueAdvanceResult::source_boundary;
        }
        if (hotel_options_requested) {
            return FieldDialogueAdvanceResult::hotel_options_requested;
        }
        if (hospital_options_requested) {
            return FieldDialogueAdvanceResult::hospital_options_requested;
        }
        if (medicine_lodge_store_requested) {
            return FieldDialogueAdvanceResult::
                medicine_lodge_store_requested;
        }
        return FieldDialogueAdvanceResult::returned_to_field;
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
    input_lockout_frames_ = 0U;
    active_prompt_ = StokerMotelPromptId::source_boundary;
    active_speaker_profile_.reset();
    active_script_operation_.reset();
    active_script_menu_.reset();
    hosted_choice_list_ = false;
    script_task_waiting_ = false;
    pending_matrix_entry_system_id_.reset();
    pending_matrix_entry_node_id_.reset();
    inline_task_advance_result_.reset();
    boundary_reason_.clear();
    hotel_options_menu_follows_ = false;
}

std::optional<std::uint8_t>
FieldDialogueRuntime::take_matrix_entry_system_id() noexcept {
    auto requested = pending_matrix_entry_system_id_;
    pending_matrix_entry_system_id_.reset();
    return requested;
}

std::optional<std::uint8_t>
FieldDialogueRuntime::take_matrix_entry_node_id() noexcept {
    auto requested = pending_matrix_entry_node_id_;
    pending_matrix_entry_node_id_.reset();
    return requested;
}

std::optional<std::string>
FieldDialogueRuntime::take_gameplay_task_result() {
    auto result = std::move(last_gameplay_task_result_id_);
    last_gameplay_task_result_id_.reset();
    return result;
}

std::optional<scripting::DialogueOutcome>
FieldDialogueRuntime::take_gameplay_task_outcome() noexcept {
    auto outcome = std::move(last_gameplay_task_outcome_);
    last_gameplay_task_outcome_.reset();
    return outcome;
}

bool FieldDialogueRuntime::uses_field_console_menu_layout() const noexcept {
    if (!panel_.open || !active_script_menu_.has_value() ||
        !active_script_menu_->speaker_id.empty()) {
        return false;
    }
    const auto& id = active_script_menu_->id;
    return id == "ordinary" || id == "cyberspace" || id == "passcodes";
}

std::string_view FieldDialogueRuntime::field_console_menu_title() const noexcept {
    return uses_field_console_menu_layout() && panel_.text_segment_count != 0U
               ? panel_.text_segments[0]
               : std::string_view{};
}

bool FieldDialogueRuntime::uses_hospital_menu_layout() const noexcept {
    if (!panel_.open || !active_script_menu_.has_value() ||
        active_script_menu_->text_profile != "council_island_hospital") {
        return false;
    }
    const auto& id = active_script_menu_->id;
    return id == "hospital_options" ||
           id == "hospital_medical_attention" ||
           id == "hospital_leave_confirmation";
}

bool FieldDialogueRuntime::uses_hospital_medical_attention_layout()
    const noexcept {
    return panel_.open && active_script_menu_.has_value() &&
           active_script_menu_->text_profile == "council_island_hospital" &&
           active_script_menu_->id == "hospital_medical_attention";
}

void FieldDialogueRuntime::paint(QPainter& painter) const {
    presenter_.paint(painter, panel_);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
