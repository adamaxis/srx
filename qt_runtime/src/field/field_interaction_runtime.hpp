#pragma once

#include "dialogue_panel.hpp"
#include "dialogue_panel_presenter.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_mode_transition_conversion.hpp"
#include "shadowrun_ghidra/field/field_interaction_continuation_conversion.hpp"
#include "shadowrun_ghidra/field/mode_text_prompt_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

class QPainter;

namespace shadowrun::ghidra_conversion {
struct WorldActivityTimeoutState;
struct WorldEffectTransitionState;
} // namespace shadowrun::ghidra_conversion

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class FieldInteractionAdvanceResult : std::uint8_t {
    waiting,
    prompt_advanced,
    returned_to_field,
};

// Source-selected field object prompts. The controller owns menu actions and
// typed interaction state; the shared presenter only paints its resolved view.
class FieldInteractionRuntime final {
public:
    explicit FieldInteractionRuntime(
        std::function<void(std::string_view)> source_boundary,
        std::function<void(std::uint16_t)> source_audio_event);
    ~FieldInteractionRuntime();

    bool load_assets(const ConversionAssetCatalog& assets);
    void set_script_runtime(scripting::GameScriptRuntime* runtime) noexcept;
    bool open(MovementModeTransitionContinuation continuation,
              std::uint16_t scene_id, std::uint16_t world_mode,
              bool interaction_suppressed,
              std::uint8_t interaction_selection,
              std::optional<std::uint8_t> security_floor_index = std::nullopt,
              WorldActivityTimeoutState* world_activity_timeout = nullptr);
    bool open_prompt_for_dump(FieldInteractionPrompt prompt,
                              std::uint8_t interaction_selection = 2U,
                              bool interaction_suppressed = false,
                              std::optional<std::uint8_t> security_floor_index =
                                  std::nullopt);

    FieldInteractionAdvanceResult advance_controller_events(
        std::uint16_t event_word,
        std::array<RunnerRecordFields, 3>& party,
        const std::array<bool, 3>& active_slots,
        std::uint8_t& interaction_selection,
        std::uint16_t& random_seed);

    void close() noexcept;
    void reset_session() noexcept;
    bool open() const noexcept { return panel_.open; }
    const DialoguePanelView& panel() const noexcept { return panel_; }
    const std::string& failure() const noexcept { return failure_; }
    std::uint8_t choice_grid_index() const noexcept {
        return choice_grid_index_;
    }
    bool security_alarm_disabled() const noexcept {
        return security_alarm_disabled_;
    }
    const WorldActivityTimeoutState& world_activity_timeout_for_dump()
        const noexcept;

    void paint(QPainter& painter) const;

private:
    enum class TextKey : std::uint32_t;

    void build_prompt(FieldInteractionPrompt prompt);
    void build_door_choice_prompt(bool has_any_maglock_passkey);
    void build_security_choice_prompt();
    void build_result_prompt(TextKey text_key);
    bool configure_menu_choices(
        const scripting::ScriptMenuDefinition& menu);
    void dispatch_command(std::uint32_t command,
                         std::array<RunnerRecordFields, 3>& party,
                         const std::array<bool, 3>& active_slots,
                         std::uint8_t& interaction_selection,
                         std::uint16_t& random_seed);
    std::uint8_t best_electronics_skill(
        const std::array<RunnerRecordFields, 3>& party,
        const std::array<bool, 3>& active_slots) const noexcept;
    bool party_has_item(const std::array<RunnerRecordFields, 3>& party,
                        const std::array<bool, 3>& active_slots,
                        std::uint8_t item_code) const noexcept;
    void begin_security_alert();
    void finish_security_alert();
    std::string_view resolve_text(std::uint32_t key) const;
    void add_text(std::string_view text);
    void add_option(ModeTextPromptTextRole role, std::uint32_t key);

    std::function<void(std::string_view)> source_boundary_callback_;
    std::function<void(std::uint16_t)> source_audio_event_callback_;
    scripting::GameScriptRuntime* script_runtime_ = nullptr;
    scripting::ScriptStringMap text_keys_;
    std::optional<scripting::ScriptMenuDefinition> maglock_intro_menu_;
    std::optional<scripting::ScriptMenuDefinition> maglock_actions_menu_;
    std::optional<scripting::ScriptMenuDefinition> security_actions_menu_;
    std::optional<scripting::ScriptMenuDefinition> active_menu_;
    DialoguePanelPresenter presenter_;
    ModeTextPromptDescriptor prompt_{};
    ModeTextPromptState prompt_state_{};
    DialoguePanelView panel_{};
    std::unique_ptr<WorldEffectTransitionState> alert_transition_state_;
    std::unique_ptr<WorldActivityTimeoutState> owned_alert_timeout_state_;
    WorldActivityTimeoutState* alert_timeout_state_ = nullptr;
    std::string failure_;
    std::optional<TextKey> trailing_result_text_;
    std::uint8_t choice_grid_index_ = 0U;
    std::optional<std::uint8_t> security_floor_index_;
    bool assets_loaded_ = false;
    bool alert_latched_ = false;
    bool security_alarm_disabled_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
