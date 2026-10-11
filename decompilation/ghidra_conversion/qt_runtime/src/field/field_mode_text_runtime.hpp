#pragma once

#include "dialogue_panel.hpp"
#include "dialogue_panel_presenter.hpp"
#include "shadowrun_ghidra/field/mode_text_prompt_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <cstdint>
#include <string>
#include <string_view>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class FieldModeTextAdvanceResult : std::uint8_t {
    waiting,
    returned_to_route,
};

enum class FieldModeTextOpenResult : std::uint8_t {
    opened,
    no_content,
    failed,
};

// Host presenter for the source's generic mode-text prompt. Lua selects its
// content from native field context; this class owns only generic prompt
// timing and the resolved panel view.
class FieldModeTextRuntime final {
public:
    bool load_assets(const ConversionAssetCatalog& assets);
    void set_script_runtime(
        scripting::GameScriptRuntime* scripts) noexcept;

    bool open(const scripting::ScriptFieldModeContext& context);
    FieldModeTextOpenResult open_if_matched(
        const scripting::ScriptFieldModeContext& context);
    FieldModeTextAdvanceResult advance_controller_events(
        std::uint16_t event_word, std::uint16_t elapsed_frames = 1U);

    void close() noexcept;
    bool open() const noexcept { return open_; }
    const DialoguePanelView& panel() const noexcept { return panel_; }
    std::uint16_t frames_remaining() const noexcept {
        return prompt_state_.frames_remaining;
    }
    const std::string& failure() const noexcept { return failure_; }
    void paint(QPainter& painter) const;

private:
    bool resolve_panel(std::string_view text_key);

    DialoguePanelPresenter presenter_;
    ModeTextPromptDescriptor prompt_{};
    ModeTextPromptState prompt_state_{};
    DialoguePanelView panel_{};
    std::string portrait_alias_storage_;
    scripting::GameScriptRuntime* scripts_ = nullptr;
    std::string failure_;
    bool assets_loaded_ = false;
    bool open_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
