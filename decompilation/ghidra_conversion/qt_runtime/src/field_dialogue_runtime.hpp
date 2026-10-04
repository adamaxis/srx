#pragma once

#include "dialogue_panel.hpp"
#include "dialogue_panel_presenter.hpp"
#include "dialogue_text_catalog.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_arrival_command_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_room_payment_conversion.hpp"

#include <cstdint>
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

// Host-side composition for the source 0x1F246/0x1F3C4 prompt loop.  The
// converted prompt graph owns the records and transitions; this class owns
// only the non-blocking Qt presentation state and direct assets needed to
// display one prompt at a time.
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

    // Bind the selection made by converted 0x1F65A to a common NPC panel.
    // Profiles whose portrait/text records have not been extracted remain a
    // visible source boundary; they never borrow Stoker's assets.
    bool open_field_exit_prompt(const FieldExitPromptSelection& selection);

    // Diagnostic entry used by the Qt conversion dump path. It exercises the
    // same typed prompt graph as the field-exit callback without pretending
    // that a missing world interaction trigger has been converted.
    bool open_stoker_prompt_for_dump(StokerMotelPromptId prompt);
    bool open_stoker_arrival_result_for_dump(std::uint8_t random_roll);

    FieldDialogueAdvanceResult advance_controller_events(
        std::uint16_t event_word);

    void close() noexcept;
    bool open() const noexcept { return panel_.open; }
    const DialoguePanelView& panel() const noexcept { return panel_; }
    const std::string& failure() const noexcept { return failure_; }

    // This is a host presentation of the source prompt grid. It uses the
    // extracted Genesis font and portrait directly; no full-screen bitmap or
    // runtime flipbook is introduced.
    void paint(QPainter& painter) const;

private:
    bool open_stoker_prompt(StokerMotelPromptId prompt);
    bool open_source_boundary(std::string reason);
    std::string_view resolve_text(std::string_view alias) const;

    std::function<void(std::string_view)> source_boundary_callback_;
    DialogueTextCatalog text_catalog_;
    DialoguePanelPresenter presenter_;
    DialoguePanelView panel_{};
    StokerMotelPromptId active_prompt_ = StokerMotelPromptId::source_boundary;
    StokerMotelArrivalCommandCallbacks arrival_command_callbacks_;
    std::function<std::optional<StokerMotelRoomPaymentOutcome>()>
        room_payment_callback_;
    std::string failure_;
    std::string boundary_reason_;
    bool assets_loaded_ = false;
    bool hotel_options_menu_follows_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
