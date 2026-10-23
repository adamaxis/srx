#pragma once

#include "dialogue_panel.hpp"
#include "dialogue_panel_presenter.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_arrival_command_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_debt_payment_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_room_payment_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <cstdint>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class FieldDialogueAdvanceResult {
    waiting_for_controller,
    prompt_advanced,
    returned_to_field,
    returned_from_world_activity,
    taxi_selector_requested,
    typed_outcome_requested,
    matrix_entry_requested,
    matrix_search_requested,
    hotel_options_requested,
    hospital_options_requested,
    hospital_medical_attention_requested,
    hospital_cyberware_requested,
    hospital_leave_confirmation_requested,
    hospital_left,
    medicine_lodge_store_requested,
    medicine_lodge_wise_one_followup,
    source_boundary,
};

// The source event identity is FFFBF8/FFFBF9. These values select a prompt
// record; speaker text and portrait remain provider data consumed by the
// generic panel runtime.
struct FieldDialogueEventContext {
    std::uint8_t field_mode = 0; // FFFBF8
    std::uint8_t event_code = 0; // FFFBF9
    std::int8_t alternate_result = 0; // FFF0D5
    std::uint32_t world_flags = 0; // FFFC1E
};

// Source-backed router for event-selected dialogue records. It keeps mode
// and event selection out of the generic text and panel presentation code.
class FieldDialogueEventResolver final {
public:
    FieldExitPromptAction resolve(
        const FieldDialogueEventContext& context,
        FieldExitPromptState& state,
        const FieldExitPromptCallbacks& callbacks) const;
};

// Host-side composition for the source 0x1F246/0x1F3C4 prompt loop. Converted
// prompt state and Lua speaker/content properties feed one non-blocking Qt
// presentation path.
class FieldDialogueRuntime final {
public:
    explicit FieldDialogueRuntime(
        std::function<void(std::string_view)> source_boundary,
        StokerMotelArrivalCommandCallbacks arrival_command_callbacks = {});

    bool load_assets(const ConversionAssetCatalog& assets);

    void set_arrival_command_callbacks(
        StokerMotelArrivalCommandCallbacks callbacks);
    void set_room_payment_callback(
        std::function<std::optional<StokerMotelRoomPaymentOutcome>()>
            callback);
    void set_debt_payment_callback(
        std::function<std::optional<StokerMotelDebtPaymentOutcome>()>
            callback);
    void set_followup_command_callback(
        std::function<std::optional<StokerMotelFollowupOutcome>(
            StokerMotelFollowupCommand)> callback);
    void set_native_script_service_callback(
        std::function<std::optional<std::string>(std::string_view)> callback);
    void set_script_runtime(scripting::GameScriptRuntime* runtime) noexcept;

    // Bind the selection made by converted 0x1F65A to a common NPC panel.
    // Profiles whose portrait/text records have not been extracted remain a
    // visible source boundary; they never borrow Stoker's assets.
    bool open_field_exit_prompt(const FieldExitPromptSelection& selection);

    // Diagnostic entry used by the Qt conversion dump path. It exercises the
    // same typed prompt graph as the field-exit callback without pretending
    // that a missing world interaction trigger has been converted.
    bool open_stoker_prompt_for_dump(StokerMotelPromptId prompt);
    bool open_stoker_arrival_result_for_dump(std::uint8_t random_roll);

    // The production Stoker interaction is the JSON graph. The dump helper
    // supplies a source-default trigger context.
    bool open_stoker_interaction();
    bool open_stoker_interaction_for_dump();
    bool open_gameplay_task(std::string_view module,
                            std::string_view function);
    bool present_source_boundary_for_host(std::string reason);

    FieldDialogueAdvanceResult advance_controller_events(
        std::uint16_t event_word);
    // 0x1F3C4 waits with 0xC24 before it samples the pad again (no 0x195C
    // call, so a button still held afterwards is a new edge). Returns true
    // and counts one frame while that wait lasts.
    bool consume_input_lockout_frame() noexcept;
    bool input_lockout_active() const noexcept {
        return panel_.open && input_lockout_frames_ != 0U;
    }
    std::optional<std::uint8_t> take_matrix_entry_system_id() noexcept;
    std::optional<std::uint8_t> take_matrix_entry_node_id() noexcept;
    std::optional<std::string> take_gameplay_task_result();
    std::optional<scripting::DialogueOutcome>
    take_gameplay_task_outcome() noexcept;
    bool uses_field_console_menu_layout() const noexcept;
    std::string_view field_console_menu_title() const noexcept;
    bool uses_hospital_menu_layout() const noexcept;
    bool uses_hospital_medical_attention_layout() const noexcept;

    void close() noexcept;
    bool retain_closed_credits_page_for_title_return();
    bool has_retained_credits_page_for_title_return() const noexcept {
        return !retained_credits_page_asset_alias_.empty() &&
               !retained_credits_page_id_.empty();
    }
    void clear_retained_credits_page_for_title_return() noexcept;
    bool open() const noexcept {
        return panel_.open || story_page_open_ || credits_page_open_;
    }
    const DialoguePanelView& panel() const noexcept { return panel_; }
    const std::string& failure() const noexcept { return failure_; }
#ifdef SHADOWRUN_QT_QA_DRIVER
    std::string_view credits_page_id_for_dump() const noexcept {
        return credits_page_id_;
    }
#endif

    // This is a host presentation of the source prompt grid. It uses the
    // extracted Genesis font and portrait directly; no full-screen bitmap or
    // runtime flipbook is introduced.
    void paint(QPainter& painter) const;
    void paint_retained_credits_page_for_title_return(
        QPainter& painter) const;

private:
    bool open_stoker_prompt(StokerMotelPromptId prompt);
    bool open_source_boundary(std::string reason);
    bool start_script_task(std::string_view module,
                           std::string_view function,
                           StokerMotelPromptId prompt);
    std::optional<FieldDialogueAdvanceResult> resolve_typed_outcome(
        const scripting::ScriptTaskStep& task);
    bool present_script_operation(
        const scripting::ScriptNativeOperation& operation,
        StokerMotelPromptId prompt, std::size_t service_depth = 0U);
    std::optional<std::string> invoke_script_service(
        std::string_view service_id);
    std::string_view resolve_text(std::string_view alias) const;

    std::function<void(std::string_view)> source_boundary_callback_;
    DialoguePanelPresenter presenter_;
    DialoguePanelView panel_{};
    StokerMotelPromptId active_prompt_ = StokerMotelPromptId::source_boundary;
    scripting::GameScriptRuntime* script_runtime_ = nullptr;
    std::optional<scripting::ScriptSpeakerProfile> stoker_speaker_profile_;
    std::optional<scripting::ScriptSpeakerProfile> active_speaker_profile_;
    std::optional<scripting::ScriptNativeOperation>
        active_script_operation_;
    std::optional<scripting::ScriptMenuDefinition> active_script_menu_;
    // A button-choice menu with more rows than button_choice_limit: the host
    // moves a cursor over the rows and A/B/C confirm the highlighted row.
    bool hosted_choice_list_ = false;
    bool script_task_waiting_ = false;
    bool story_page_open_ = false;
    bool credits_page_open_ = false;
    std::string story_page_asset_alias_;
    std::string story_page_text_;
    std::string credits_page_asset_alias_;
    std::string credits_page_id_;
    std::string closed_credits_page_asset_alias_;
    std::string closed_credits_page_id_;
    std::string retained_credits_page_asset_alias_;
    std::string retained_credits_page_id_;
    std::uint8_t input_lockout_frames_ = 0U;
    std::optional<std::uint8_t> pending_matrix_entry_system_id_;
    std::optional<std::uint8_t> pending_matrix_entry_node_id_;
    std::optional<std::string> last_gameplay_task_result_id_;
    std::optional<scripting::DialogueOutcome> last_gameplay_task_outcome_;
    std::optional<FieldDialogueAdvanceResult> inline_task_advance_result_;
    StokerMotelArrivalCommandCallbacks arrival_command_callbacks_;
    std::function<std::optional<StokerMotelRoomPaymentOutcome>()>
        room_payment_callback_;
    std::function<std::optional<StokerMotelDebtPaymentOutcome>()>
        debt_payment_callback_;
    std::function<std::optional<StokerMotelFollowupOutcome>(
        StokerMotelFollowupCommand)> followup_command_callback_;
    std::function<std::optional<std::string>(std::string_view)>
        native_script_service_callback_;
    std::string failure_;
    std::string boundary_reason_;
    bool assets_loaded_ = false;
    bool hotel_options_menu_follows_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
