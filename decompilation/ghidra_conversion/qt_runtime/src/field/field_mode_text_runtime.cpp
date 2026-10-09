#include "field_mode_text_runtime.hpp"

#include <QPainter>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

ModeTextPromptDescriptor narrative_prompt() noexcept {
    ModeTextPromptDescriptor prompt{};
    // ROM source records use a negative first-option sentinel. The shared
    // prompt waits 30 frames and then returns 0xFB on any controller event.
    prompt.first_option_is_exit_sentinel = true;
    return prompt;
}

} // namespace

bool FieldModeTextRuntime::load_assets(
    const ConversionAssetCatalog& assets) {
    failure_.clear();
    assets_loaded_ = presenter_.load_assets(assets);
    if (!assets_loaded_) {
        failure_ = presenter_.failure();
    }
    return assets_loaded_;
}

void FieldModeTextRuntime::set_script_runtime(
    scripting::GameScriptRuntime* scripts) noexcept {
    scripts_ = scripts;
}

bool FieldModeTextRuntime::resolve_panel(const std::string_view text_key) {
    if (scripts_ == nullptr) {
        failure_ = "mode-text prompt has no Lua script runtime";
        return false;
    }
    const auto* text = scripts_->dialogue_text(text_key);
    if (text == nullptr) {
        failure_ = "mode-text prompt Lua text is missing: " +
                   std::string(text_key);
        return false;
    }

    panel_ = DialoguePanelView{};
    panel_.open = true;
    panel_.actor_kind = DialoguePanelActorKind::none;
    panel_.actor_name = {};
    panel_.portrait_alias = {};
    panel_.kind = DialoguePanelKind::information_page;
    panel_.source_complete = true;

    std::size_t cursor = 0U;
    while (cursor <= text->size()) {
        if (panel_.text_segment_count >= panel_.text_segments.size()) {
            failure_ = "mode-text prompt exceeds the shared panel line limit";
            panel_ = DialoguePanelView{};
            return false;
        }
        const auto separator = text->find('\n', cursor);
        const auto end = separator == std::string_view::npos
                             ? text->size()
                             : separator;
        panel_.text_segments[panel_.text_segment_count++] =
            std::string_view(*text).substr(cursor, end - cursor);
        if (separator == std::string_view::npos) {
            break;
        }
        cursor = separator + 1U;
    }
    return true;
}

bool FieldModeTextRuntime::open(
    const scripting::ScriptFieldModeContext& context) {
    close();
    failure_.clear();
    if (!assets_loaded_) {
        failure_ = "mode-text prompt presentation assets are not loaded";
        return false;
    }

    if (scripts_ == nullptr) {
        failure_ = "mode-text prompt has no Lua script runtime";
        return false;
    }
    scripts_->set_field_mode_context(context);
    const auto content =
        scripts_->string_map("field_mode", "mode_text_content");
    if (!content.has_value()) {
        failure_ = scripts_->failure().empty()
                       ? "Lua field-mode prompt content is unavailable"
                       : scripts_->failure();
        return false;
    }
    const auto status = content->find("status");
    const auto text_key = content->find("text_key");
    const auto portrait_alias = content->find("portrait_alias");
    if (status == content->end() || text_key == content->end() ||
        portrait_alias == content->end()) {
        failure_ = "Lua field-mode prompt content is incomplete";
        return false;
    }
    if (status->second == "unmatched") {
        failure_ = "field mode/event has no Lua mode-text prompt";
        return false;
    }
    if (status->second != "matched") {
        failure_ = "Lua field-mode prompt has an unknown status";
        return false;
    }
    if (!resolve_panel(text_key->second)) {
        return false;
    }
    if (portrait_alias->second != "none") {
        portrait_alias_storage_ = portrait_alias->second;
        panel_.portrait_alias = portrait_alias_storage_;
    }

    prompt_ = narrative_prompt();
    prompt_state_ = ModeTextPromptState{};
    (void)advance_mode_text_prompt(prompt_state_, prompt_, std::nullopt, 0U,
                                   {});
    if (prompt_state_.phase !=
        ModeTextPromptState::Phase::awaiting_any_event) {
        failure_ =
            "generic mode-text prompt did not enter its timed exit phase";
        panel_ = DialoguePanelView{};
        return false;
    }
    open_ = true;
    return true;
}

FieldModeTextAdvanceResult FieldModeTextRuntime::advance_controller_events(
    const std::uint16_t event_word, const std::uint16_t elapsed_frames) {
    if (!open_) {
        return FieldModeTextAdvanceResult::waiting;
    }

    // The generic prompt tests the high byte of FFDD22. Convert the focused
    // Qt controller's newly pressed edge word once, then let the shared typed
    // prompt state enforce its 30-frame sentinel delay and any-button exit.
    const auto controller_mask = static_cast<std::uint8_t>(event_word >> 8U);
    const auto step = advance_mode_text_prompt(
        prompt_state_, prompt_, controller_mask, elapsed_frames, {});
    if (step.kind == ModeTextPromptStepKind::negative_first_option ||
        step.kind == ModeTextPromptStepKind::terminal_command) {
        close();
        return FieldModeTextAdvanceResult::returned_to_route;
    }
    return FieldModeTextAdvanceResult::waiting;
}

void FieldModeTextRuntime::close() noexcept {
    open_ = false;
    prompt_ = ModeTextPromptDescriptor{};
    prompt_state_ = ModeTextPromptState{};
    panel_ = DialoguePanelView{};
    portrait_alias_storage_.clear();
}

void FieldModeTextRuntime::paint(QPainter& painter) const {
    if (open_) {
        presenter_.paint(painter, panel_);
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
