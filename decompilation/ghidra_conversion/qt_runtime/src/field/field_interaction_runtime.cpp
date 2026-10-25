#include "field_interaction_runtime.hpp"
#include "enums.hpp"

#include "shadowrun_ghidra/world/world_effect_conversion.hpp"

#include <QPainter>

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint32_t kUseElectronicsCommand = 0x11U;
constexpr std::uint32_t kUseFragGrenadeCommand = 0x12U;
constexpr std::uint32_t kUseSecurityTerminalCommand = 0xCAU;
constexpr std::uint32_t kTerminalCommand = 0xFAU;
constexpr std::uint8_t kMaglockPasskeyItem = 0x14U;
constexpr std::uint8_t kElectronicsKitItem = 0x15U;
constexpr std::uint8_t kFragGrenadeItem = 0x0DU;
constexpr std::array<std::string_view, 22U> kFieldInteractionTextKeyIds{{
    "door_title",
    "no_passkey",
    "insufficient_passkey",
    "passkey_result",
    "electronics_no_skill",
    "electronics_success",
    "electronics_failure",
    "electronics_alert",
    "grenade_no_item",
    "grenade_result",
    "unlocked_door",
    "security_title",
    "security_failure",
    "security_alarm_disabled",
    "security_system_disabled",
    "security_files",
    "security_floor_first",
    "security_floor_second",
    "security_floor_third",
    "security_floor_fourth",
    "security_floor_fifth",
    "security_floor_sixth",
}};
} // namespace

enum class FieldInteractionRuntime::TextKey : std::uint32_t {
    door_title = 1U,
    no_passkey = 3U,
    insufficient_passkey = 4U,
    passkey_result = 8U,
    electronics_no_skill = 9U,
    electronics_success = 10U,
    electronics_failure = 11U,
    electronics_alert = 12U,
    grenade_no_item = 13U,
    grenade_result = 14U,
    unlocked_door = 15U,
    security_title = 16U,
    security_failure = 17U,
    security_alarm_disabled = 18U,
    security_system_disabled = 19U,
    security_files = 20U,
    security_floor_first = 21U,
    security_floor_second = 22U,
    security_floor_third = 23U,
    security_floor_fourth = 24U,
    security_floor_fifth = 25U,
    security_floor_sixth = 26U,
};

FieldInteractionRuntime::FieldInteractionRuntime(
    std::function<void(std::string_view)> source_boundary,
    std::function<void(std::uint16_t)> source_audio_event,
    std::function<void(std::uint16_t)> source_audio_stop)
    : source_boundary_callback_(std::move(source_boundary)),
      source_audio_event_callback_(std::move(source_audio_event)),
      source_audio_stop_callback_(std::move(source_audio_stop)),
      alert_transition_state_(
          std::make_unique<WorldEffectTransitionState>()),
      owned_alert_timeout_state_(
          std::make_unique<WorldActivityTimeoutState>()),
      alert_timeout_state_(owned_alert_timeout_state_.get()) {}

FieldInteractionRuntime::~FieldInteractionRuntime() = default;

bool FieldInteractionRuntime::load_assets(
    const ConversionAssetCatalog& assets) {
    failure_.clear();
    if (script_runtime_ == nullptr) {
        failure_ = "Lua field-interaction script runtime is not loaded";
        return false;
    }
    auto text_keys =
        script_runtime_->string_map("field_interactions", "text_keys");
    if (!text_keys.has_value() ||
        text_keys->size() != kFieldInteractionTextKeyIds.size()) {
        failure_ = "Lua field-interaction text-key map is missing or incomplete";
        return false;
    }
    for (const auto id : kFieldInteractionTextKeyIds) {
        const auto key = text_keys->find(std::string(id));
        if (key == text_keys->end() ||
            script_runtime_->dialogue_text(key->second) == nullptr) {
            failure_ = "Lua field-interaction text key has no ROM text";
            return false;
        }
    }
    text_keys_ = std::move(*text_keys);

    if (script_runtime_->dialogue_text(text_keys_.at("door_title")) == nullptr) {
        failure_ = "Lua field-interaction title text is not loaded";
        return false;
    }

    const auto load_menu = [this](
        const std::string_view function, const std::string_view profile,
        const std::string_view menu_id,
        const std::vector<std::string_view>& expected_choices)
        -> std::optional<scripting::ScriptMenuDefinition> {
        auto menu = script_runtime_->menu("field_interactions", function,
                                          profile, menu_id);
        if (!menu.has_value() ||
            menu->choices.size() != expected_choices.size()) {
            failure_ = "Lua field-interaction menu is missing or incomplete";
            return std::nullopt;
        }
        for (std::size_t index = 0; index < expected_choices.size(); ++index) {
            const auto& choice = menu->choices[index];
            if (choice.id != expected_choices[index] ||
                script_runtime_->dialogue_text(choice.label_key) == nullptr) {
                failure_ = "Lua field-interaction menu order or label is invalid";
                return std::nullopt;
            }
        }
        return menu;
    };

    maglock_intro_menu_ = load_menu("maglock_intro", "field.maglock",
                                    "door", {"more"});
    maglock_actions_menu_ = load_menu(
        "maglock_actions", "field.interaction", "maglock_actions",
        {"electronics", "grenade", "walk-away"});
    security_actions_menu_ = load_menu(
        "security_terminal_actions", "field.interaction", "security_actions",
        {"electronics", "walk-away"});
    if (!maglock_intro_menu_.has_value() ||
        !maglock_actions_menu_.has_value() ||
        !security_actions_menu_.has_value()) {
        if (failure_.empty()) {
            failure_ = "Lua field-interaction menus are incomplete";
        }
        return false;
    }

    assets_loaded_ = presenter_.load_assets(assets);
    if (!assets_loaded_) {
        failure_ = presenter_.failure();
    }
    return assets_loaded_;
}

void FieldInteractionRuntime::set_script_runtime(
    scripting::GameScriptRuntime* runtime) noexcept {
    script_runtime_ = runtime;
}

void FieldInteractionRuntime::set_security_alarm_callbacks(
    SecurityAlarmRaiseCallback raise, SecurityAlarmClearCallback clear) {
    security_alarm_raise_callback_ = std::move(raise);
    security_alarm_clear_callback_ = std::move(clear);
}

bool FieldInteractionRuntime::open(
    const MovementModeTransitionContinuation continuation,
    const std::uint16_t scene_id, const std::uint16_t world_mode,
    const bool interaction_suppressed,
    const std::uint8_t interaction_selection,
    const std::optional<std::uint8_t> security_floor_index,
    WorldActivityTimeoutState* const world_activity_timeout) {
    security_floor_index_ = security_floor_index;
    if (world_activity_timeout != nullptr) {
        alert_timeout_state_ = world_activity_timeout;
    } else {
        *owned_alert_timeout_state_ = WorldActivityTimeoutState{};
        alert_timeout_state_ = owned_alert_timeout_state_.get();
    }
    alert_timeout_state_->scene_id = scene_id;
    if (continuation == MovementModeTransitionContinuation::maglock_door_interaction) {
        FieldInteractionContinuationState state{};
        state.interaction_suppressed = interaction_suppressed;
        state.interaction_selection = interaction_selection;
        auto plan = prepare_field_interaction_prompt(state);
        if (plan.preparation ==
            FieldInteractionPreparation::write_electromagnetic_lock_choice_grid) {
            plan = prepare_electromagnetic_lock_choice_grid(
                state.interaction_selection);
        }
        *alert_transition_state_ = WorldEffectTransitionState{};
        alert_transition_state_->scene_id = scene_id;
        alert_transition_state_->world_mode = world_mode;
        choice_grid_index_ = plan.choice_grid_index;
        build_prompt(plan.prompt);
        return open();
    }
    if (continuation ==
        MovementModeTransitionContinuation::building_security_terminal) {
        const auto plan = prepare_building_security_terminal_prompt();
        *alert_transition_state_ = WorldEffectTransitionState{};
        alert_transition_state_->scene_id = scene_id;
        alert_transition_state_->world_mode = world_mode;
        choice_grid_index_ = 0U;
        build_prompt(plan.prompt);
        return open();
    }
    failure_ = "field interaction continuation has no native menu route";
    return false;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldInteractionRuntime::open_prompt_for_dump(
    const FieldInteractionPrompt prompt,
    const std::uint8_t interaction_selection,
    const bool interaction_suppressed,
    const std::optional<std::uint8_t> security_floor_index) {
    security_floor_index_ = security_floor_index;
    *owned_alert_timeout_state_ = WorldActivityTimeoutState{};
    alert_timeout_state_ = owned_alert_timeout_state_.get();
    *alert_transition_state_ = WorldEffectTransitionState{};
    alert_transition_state_->world_mode = 2U;
    FieldInteractionPrompt selected = prompt;
    choice_grid_index_ = 0U;
    if (prompt == FieldInteractionPrompt::electromagnetic_lock_door) {
        if (interaction_suppressed) {
            FieldInteractionContinuationState state{};
            state.interaction_suppressed = true;
            state.interaction_selection = interaction_selection;
            selected = prepare_field_interaction_prompt(state).prompt;
        } else {
            const auto plan = prepare_electromagnetic_lock_choice_grid(
                interaction_selection);
            selected = plan.prompt;
            choice_grid_index_ = plan.choice_grid_index;
        }
    }
    build_prompt(selected);
    return open();
}
#endif // SHADOWRUN_QT_QA_DRIVER


void FieldInteractionRuntime::build_prompt(
    const FieldInteractionPrompt prompt) {
    close();
    failure_.clear();
    if (!assets_loaded_) {
        failure_ = "field interaction assets are not loaded";
        return;
    }

    if (prompt == FieldInteractionPrompt::electromagnetic_lock_resolution) {
        build_result_prompt(TextKey::unlocked_door);
    } else if (prompt == FieldInteractionPrompt::electromagnetic_lock_door) {
        prompt_ = ModeTextPromptDescriptor{};
        prompt_.title_text = static_cast<std::uint32_t>(TextKey::door_title);
        if (!maglock_intro_menu_.has_value() ||
            !configure_menu_choices(*maglock_intro_menu_)) {
            return;
        }
    } else if (prompt == FieldInteractionPrompt::building_security_terminal) {
        build_security_choice_prompt();
    }
    if (!failure_.empty()) {
        return;
    }
    prompt_state_ = ModeTextPromptState{};
    (void)advance_mode_text_prompt(
        prompt_state_, prompt_, std::nullopt, 0U,
        { [this] {
              panel_ = DialoguePanelView{};
              panel_.open = true;
              panel_.actor_kind = DialoguePanelActorKind::none;
              panel_.kind = prompt_.first_option_is_exit_sentinel
                                ? DialoguePanelKind::information_page
                                : DialoguePanelKind::subject_choices;
              panel_.source_complete = true;
          },
          [this](const ModeTextPromptTextRole role,
                 const std::uint32_t key) {
              if (role == ModeTextPromptTextRole::title) {
                  add_text(resolve_text(key));
                  if (trailing_result_text_.has_value()) {
                      add_text(resolve_text(static_cast<std::uint32_t>(
                          *trailing_result_text_)));
                      trailing_result_text_.reset();
                  }
              } else {
                  add_option(role, key);
              }
          },
          [](const ModeTextPromptTextRole) {}, {}, {}, {}, {}});
    if (!panel_.open) {
        failure_ = "field interaction prompt did not resolve a display view";
    }
}

void FieldInteractionRuntime::build_door_choice_prompt(
    const bool has_any_maglock_passkey) {
    prompt_ = ModeTextPromptDescriptor{};
    prompt_.title_text = static_cast<std::uint32_t>(
        has_any_maglock_passkey ? TextKey::insufficient_passkey
                                : TextKey::no_passkey);
    if (!maglock_actions_menu_.has_value()) {
        failure_ = "Lua maglock action choices are not loaded";
        return;
    }
    (void)configure_menu_choices(*maglock_actions_menu_);
}

void FieldInteractionRuntime::build_security_choice_prompt() {
    prompt_ = ModeTextPromptDescriptor{};
    prompt_.title_text = static_cast<std::uint32_t>(TextKey::security_title);
    if (!security_actions_menu_.has_value()) {
        failure_ = "Lua Security Terminal choices are not loaded";
        return;
    }
    (void)configure_menu_choices(*security_actions_menu_);
}

void FieldInteractionRuntime::build_result_prompt(const TextKey text_key) {
    trailing_result_text_.reset();
    active_menu_.reset();
    prompt_ = ModeTextPromptDescriptor{};
    prompt_.title_text = static_cast<std::uint32_t>(text_key);
    prompt_.first_option = ModeTextPromptOption{0U, false};
    prompt_.second_option = ModeTextPromptOption{0U, false};
    prompt_.third_option = ModeTextPromptOption{0U, false};
    prompt_.first_option_is_exit_sentinel = true;
}

bool FieldInteractionRuntime::configure_menu_choices(
    const scripting::ScriptMenuDefinition& menu) {
    if (menu.choices.empty() || menu.choices.size() > 3U) {
        failure_ = "Lua field-interaction menu exceeds the source three-choice limit";
        return false;
    }
    active_menu_ = menu;
    prompt_.commands = {};
    for (std::size_t index = 0; index < menu.choices.size(); ++index) {
        const auto& id = menu.choices[index].id;
        std::uint32_t command = 0U;
        if (id == "more" && menu.id == "door") {
            command = 0x10U; // ROM mode command for 0x21628.
        } else if (id == "electronics" &&
                   menu.id == "security_actions") {
            command = kUseSecurityTerminalCommand;
        } else if (id == "electronics") {
            command = kUseElectronicsCommand;
        } else if (id == "grenade") {
            command = kUseFragGrenadeCommand;
        } else if (id == "walk-away") {
            command = kTerminalCommand;
        } else {
            failure_ = "Lua field-interaction menu contains an unknown action ID";
            active_menu_.reset();
            return false;
        }
        const ModeTextPromptOption option{
            static_cast<std::uint32_t>(index + 1U), true};
        if (index == 0U) {
            prompt_.first_option = option;
        } else if (index == 1U) {
            prompt_.second_option = option;
        } else {
            prompt_.third_option = option;
        }
        prompt_.commands[index] = command;
    }
    return true;
}

void FieldInteractionRuntime::add_text(const std::string_view text) {
    if (text.empty()) {
        panel_.source_complete = false;
        panel_.source_boundary_reason = "field interaction text is missing";
        return;
    }
    std::size_t cursor = 0U;
    while (cursor <= text.size() &&
           panel_.text_segment_count < panel_.text_segments.size()) {
        const auto separator = text.find('|', cursor);
        const auto end = separator == std::string_view::npos
                             ? text.size()
                             : separator;
        panel_.text_segments[panel_.text_segment_count++] =
            text.substr(cursor, end - cursor);
        if (separator == std::string_view::npos) {
            break;
        }
        cursor = separator + 1U;
    }
}

void FieldInteractionRuntime::add_option(
    const ModeTextPromptTextRole role, const std::uint32_t key) {
    if (panel_.option_count >= panel_.options.size()) {
        panel_.source_complete = false;
        panel_.source_boundary_reason =
            "field interaction exceeds the source three-option limit";
        return;
    }
    const auto index = panel_.option_count++;
    if (!active_menu_.has_value() || key == 0U ||
        key > active_menu_->choices.size()) {
        panel_.source_complete = false;
        panel_.source_boundary_reason =
            "field interaction option is missing its Lua choice ID";
        return;
    }
    const auto& choice = active_menu_->choices[key - 1U];
    DialoguePanelOptionRole option_role = DialoguePanelOptionRole::field_continue;
    DialoguePanelCommand command = DialoguePanelCommand::field_continue;
    if (choice.id == "electronics") {
        const bool security_prompt = active_menu_->id == "security_actions";
        option_role = security_prompt
                          ? DialoguePanelOptionRole::field_security_access
                          : DialoguePanelOptionRole::field_use_electronics;
        command = security_prompt
                      ? DialoguePanelCommand::field_security_access
                      : DialoguePanelCommand::field_use_electronics;
    } else if (choice.id == "grenade") {
        option_role = DialoguePanelOptionRole::field_use_grenade;
        command = DialoguePanelCommand::field_use_grenade;
    } else if (choice.id == "walk-away") {
        option_role = DialoguePanelOptionRole::field_walk_away;
        command = DialoguePanelCommand::field_walk_away;
    }
    const auto* label = script_runtime_ != nullptr
                            ? script_runtime_->dialogue_text(choice.label_key)
                            : nullptr;
    if (label == nullptr) {
        panel_.source_complete = false;
        panel_.source_boundary_reason =
            "field interaction Lua choice label is missing";
        return;
    }
    auto source_label = std::string_view(*label);
    if (choice.id == "more") {
        // This source record has an empty first text well and puts < MORE > in
        // the lower well. It is a row label here, so keep only that second
        // segment instead of drawing the separator as a literal character.
        const auto separator = source_label.find('|');
        if (separator != std::string_view::npos) {
            source_label.remove_prefix(separator + 1U);
        }
    }
    (void)role;
    panel_.options[index] = {option_role, source_label, command};
}

std::string_view FieldInteractionRuntime::resolve_text(
    const std::uint32_t key) const {
    std::string_view text_id;
    switch (static_cast<TextKey>(key)) {
    case TextKey::door_title: text_id = "door_title"; break;
    case TextKey::no_passkey: text_id = "no_passkey"; break;
    case TextKey::insufficient_passkey:
        text_id = "insufficient_passkey"; break;
    case TextKey::passkey_result:
        text_id = "passkey_result"; break;
    case TextKey::electronics_no_skill:
        text_id = "electronics_no_skill"; break;
    case TextKey::electronics_success:
        text_id = "electronics_success"; break;
    case TextKey::electronics_failure:
        text_id = "electronics_failure"; break;
    case TextKey::electronics_alert:
        text_id = "electronics_alert"; break;
    case TextKey::grenade_no_item:
        text_id = "grenade_no_item"; break;
    case TextKey::grenade_result:
        text_id = "grenade_result"; break;
    case TextKey::unlocked_door:
        text_id = "unlocked_door"; break;
    case TextKey::security_title: text_id = "security_title"; break;
    case TextKey::security_failure:
        text_id = "security_failure"; break;
    case TextKey::security_alarm_disabled:
        text_id = "security_alarm_disabled"; break;
    case TextKey::security_system_disabled:
        text_id = "security_system_disabled"; break;
    case TextKey::security_files:
        text_id = "security_files"; break;
    case TextKey::security_floor_first:
        text_id = "security_floor_first"; break;
    case TextKey::security_floor_second:
        text_id = "security_floor_second"; break;
    case TextKey::security_floor_third:
        text_id = "security_floor_third"; break;
    case TextKey::security_floor_fourth:
        text_id = "security_floor_fourth"; break;
    case TextKey::security_floor_fifth:
        text_id = "security_floor_fifth"; break;
    case TextKey::security_floor_sixth:
        text_id = "security_floor_sixth"; break;
    }
    const auto mapped_key = text_keys_.find(std::string(text_id));
    const auto* text = script_runtime_ != nullptr &&
                               mapped_key != text_keys_.end()
                           ? script_runtime_->dialogue_text(mapped_key->second)
                           : nullptr;
    return text == nullptr ? std::string_view{} : std::string_view(*text);
}

std::uint8_t FieldInteractionRuntime::best_electronics_skill(
    const std::array<RunnerRecordFields, 3>& party,
    const std::array<bool, 3>& active_slots) const noexcept {
    std::uint8_t best = 0U;
    for (std::size_t slot = 0U; slot < party.size(); ++slot) {
        if (active_slots[slot] && (party[slot].flags & 0x0100U) != 0U) {
            // ROM reads record +0x8A. RunnerRecordFields maps +0x8A to skills[9].
            best = std::max(best, party[slot].skills[9]);
        }
    }
    return best;
}

bool FieldInteractionRuntime::party_has_item(
    const std::array<RunnerRecordFields, 3>& party,
    const std::array<bool, 3>& active_slots,
    const std::uint8_t item_code) const noexcept {
    for (std::size_t slot = 0U; slot < party.size(); ++slot) {
        if (!active_slots[slot] || (party[slot].flags & 0x0100U) == 0U) {
            continue;
        }
        if (std::find(party[slot].inventory_item_codes.begin(),
                      party[slot].inventory_item_codes.end(), item_code) !=
            party[slot].inventory_item_codes.end()) {
            return true;
        }
    }
    return false;
}

void FieldInteractionRuntime::begin_security_alert() {
    if (security_alarm_raise_callback_) {
        security_alarm_raise_callback_();
        alert_latched_ = true;
        return;
    }
    const auto action = begin_world_effect_transition(
        *alert_transition_state_, *alert_timeout_state_,
        [this](const std::uint16_t sound_id) {
            if (source_audio_event_callback_) {
                source_audio_event_callback_(sound_id);
            }
        },
        [this] {
            if (source_boundary_callback_) {
                source_boundary_callback_(
                    "field.interaction.alert.clear_pending_effect_flags");
            }
        },
        [this] {
            if (source_boundary_callback_) {
                source_boundary_callback_(
                    "field.interaction.alert.effect_list_pending");
            }
        });
    alert_latched_ = alert_latched_ ||
                     action == WorldEffectTransitionAction::started;
}

void FieldInteractionRuntime::finish_security_alert(const bool disable_alarm) {
    alert_latched_ = false;
    alert_transition_state_->transition_latched = false;
    if (security_alarm_clear_callback_) {
        security_alarm_clear_callback_(disable_alarm);
        security_alarm_disabled_ = security_alarm_disabled_ || disable_alarm;
        return;
    }
    if (source_audio_stop_callback_) {
        // ROM 0x1E09A sends sound 0x30 through 0x530D2's stop-resource path.
        source_audio_stop_callback_(
            static_cast<std::uint16_t>(srx::SoundId::ALARM));
    } else if (source_boundary_callback_) {
        source_boundary_callback_(
            "field.interaction.alert.stop_sound_unavailable");
    }
}

void FieldInteractionRuntime::dispatch_command(
    const std::uint32_t command,
    std::array<RunnerRecordFields, 3>& party,
    const std::array<bool, 3>& active_slots,
    std::uint8_t& interaction_selection,
    std::uint16_t& random_seed) {
    if (command == 0x10U) {
        bool found_passkey = false;
        for (std::size_t runner = 0U; runner < party.size(); ++runner) {
            if (!active_slots[runner] ||
                (party[runner].flags & 0x0100U) == 0U) {
                continue;
            }
            for (std::size_t slot = 0U;
                 slot < party[runner].inventory_item_codes.size(); ++slot) {
                if (party[runner].inventory_item_codes[slot] !=
                    kMaglockPasskeyItem) {
                    continue;
                }
                found_passkey = true;
                if (interaction_selection <=
                    party[runner].inventory_phase_bytes[slot]) {
                    interaction_selection = 0xFFU;
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(static_cast<std::uint16_t>(srx::SoundId::RELOAD_OR_EMPTY));
                    }
                    build_result_prompt(TextKey::passkey_result);
                    return;
                }
            }
        }
        build_door_choice_prompt(found_passkey);
        return;
    }

    if (command == kUseElectronicsCommand) {
        const auto skill = best_electronics_skill(party, active_slots);
        if (skill == 0U) {
            build_result_prompt(TextKey::electronics_no_skill);
            return;
        }
        const auto has_kit = party_has_item(
            party, active_slots, kElectronicsKitItem);
        const auto trial_count = static_cast<std::uint16_t>(
            static_cast<std::uint8_t>(skill + (has_kit ? 2U : 0U)));
        const auto threshold = static_cast<std::int16_t>(has_kit ? 3 : 5);
        const auto successes = count_thresholded_random_rolls(
            trial_count, threshold,
            [&random_seed](const std::uint16_t bound) {
                return sample_source_random_bound(random_seed, bound);
            });
        if (successes == 0U) {
            begin_security_alert();
            build_result_prompt(TextKey::electronics_alert);
            return;
        }

        const auto remaining = static_cast<std::uint8_t>(
            interaction_selection - successes);
        if (static_cast<std::int8_t>(remaining) <= 0) {
            interaction_selection = 0xFFU;
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::SoundId::RELOAD_OR_EMPTY));
            }
            build_result_prompt(TextKey::electronics_success);
            return;
        }
        interaction_selection = remaining;
        build_result_prompt(TextKey::electronics_failure);
        return;
    }

    if (command == kUseFragGrenadeCommand) {
        for (std::size_t runner = 0U; runner < party.size(); ++runner) {
            if (!active_slots[runner] ||
                (party[runner].flags & 0x0100U) == 0U) {
                continue;
            }
            for (std::size_t slot = party[runner].inventory_item_codes.size();
                 slot != 0U; --slot) {
                const auto inventory_slot = slot - 1U;
                if (party[runner].inventory_item_codes[inventory_slot] ==
                    kFragGrenadeItem) {
                    // ROM decrements the selected item property and removes
                    // the inventory entry when its last grenade is consumed.
                    build_result_prompt(TextKey::grenade_result);
                    auto& quantity =
                        party[runner].inventory_phase_bytes[inventory_slot];
                    if (quantity <= 1U) {
                        party[runner].inventory_item_codes[inventory_slot] = 0U;
                        quantity = 0U;
                        if (party[runner].equipped_inventory_slot ==
                            static_cast<std::int8_t>(inventory_slot)) {
                            party[runner].equipped_inventory_slot = -1;
                            party[runner].equipped_item_code = 0U;
                        }
                    } else {
                        --quantity;
                    }
                    interaction_selection = 0xFFU;
                    begin_security_alert();
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(static_cast<std::uint16_t>(srx::SoundId::DAC_SAMPLE_2C));
                    }
                    return;
                }
            }
        }
        build_result_prompt(TextKey::grenade_no_item);
        return;
    }

    if (command == kUseSecurityTerminalCommand) {
        const auto skill = best_electronics_skill(party, active_slots);
        const auto successes = count_thresholded_random_rolls(
            skill, 5,
            [&random_seed](const std::uint16_t bound) {
                return sample_source_random_bound(random_seed, bound);
            });
        if (successes == 0U) {
            begin_security_alert();
            build_result_prompt(TextKey::security_failure);
        } else if (successes == 1U) {
            finish_security_alert();
            build_result_prompt(TextKey::security_alarm_disabled);
        } else if (successes < 3U) {
            finish_security_alert(true);
            build_result_prompt(TextKey::security_system_disabled);
        } else {
            finish_security_alert(true);
            build_result_prompt(TextKey::security_files);
            if (security_floor_index_.has_value() &&
                *security_floor_index_ < 6U) {
                trailing_result_text_ = static_cast<TextKey>(
                    static_cast<std::uint32_t>(TextKey::security_floor_first) +
                    *security_floor_index_);
            } else if (source_boundary_callback_) {
                source_boundary_callback_(
                    security_floor_index_.has_value()
                        ? "field.security_terminal.floor_index_out_of_range"
                        : "field.security_terminal.floor_index_unavailable");
            }
        }
    }
}

FieldInteractionAdvanceResult FieldInteractionRuntime::advance_controller_events(
    const std::uint16_t event_word,
    std::array<RunnerRecordFields, 3>& party,
    const std::array<bool, 3>& active_slots,
    std::uint8_t& interaction_selection,
    std::uint16_t& random_seed) {
    if (!panel_.open) {
        return FieldInteractionAdvanceResult::waiting;
    }
    const auto callbacks = ModeTextPromptCallbacks{
        [this] {
            panel_ = DialoguePanelView{};
            panel_.open = true;
            panel_.actor_kind = DialoguePanelActorKind::none;
            panel_.kind = prompt_.first_option_is_exit_sentinel
                              ? DialoguePanelKind::information_page
                              : DialoguePanelKind::subject_choices;
            panel_.source_complete = true;
        },
        [this](const ModeTextPromptTextRole role, const std::uint32_t key) {
            if (role == ModeTextPromptTextRole::title) {
                add_text(resolve_text(key));
                if (trailing_result_text_.has_value()) {
                    add_text(resolve_text(static_cast<std::uint32_t>(
                        *trailing_result_text_)));
                    trailing_result_text_.reset();
                }
            } else {
                add_option(role, key);
            }
        },
        [](const ModeTextPromptTextRole) {},
        {}, {}, {},
        [this, &party, &active_slots, &interaction_selection, &random_seed](
            const std::uint32_t command) {
            dispatch_command(command, party, active_slots,
                             interaction_selection, random_seed);
        }};
    const auto step = advance_mode_text_prompt(
        prompt_state_, prompt_,
        static_cast<std::uint8_t>((event_word >> 8U) & 0xFFU), 1U,
        callbacks);
    if (step.kind == ModeTextPromptStepKind::subcommand_dispatched) {
        return FieldInteractionAdvanceResult::prompt_advanced;
    }
    if (step.kind == ModeTextPromptStepKind::negative_first_option ||
        step.kind == ModeTextPromptStepKind::terminal_command) {
        close();
        return FieldInteractionAdvanceResult::returned_to_field;
    }
    return FieldInteractionAdvanceResult::waiting;
}

void FieldInteractionRuntime::close() noexcept {
    panel_ = DialoguePanelView{};
    prompt_ = ModeTextPromptDescriptor{};
    prompt_state_ = ModeTextPromptState{};
    active_menu_.reset();
}

void FieldInteractionRuntime::reset_session() noexcept {
    close();
    *alert_transition_state_ = WorldEffectTransitionState{};
    *owned_alert_timeout_state_ = WorldActivityTimeoutState{};
    alert_timeout_state_ = owned_alert_timeout_state_.get();
    choice_grid_index_ = 0U;
    security_floor_index_.reset();
    trailing_result_text_.reset();
    alert_latched_ = false;
    security_alarm_disabled_ = false;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
const WorldActivityTimeoutState&
FieldInteractionRuntime::world_activity_timeout_for_dump() const noexcept {
    return *alert_timeout_state_;
}
#endif // SHADOWRUN_QT_QA_DRIVER


void FieldInteractionRuntime::paint(QPainter& painter) const {
    presenter_.paint(painter, panel_);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
