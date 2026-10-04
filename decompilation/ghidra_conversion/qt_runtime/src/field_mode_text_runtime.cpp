#include "field_mode_text_runtime.hpp"

#include <QPainter>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

bool FieldModeTextRuntime::load_assets(
    const ConversionAssetCatalog& assets) {
    failure_.clear();
    assets_loaded_ = presenter_.load_assets(assets);
    if (!assets_loaded_) {
        failure_ = presenter_.failure();
    }
    return assets_loaded_;
}

bool FieldModeTextRuntime::resolve_panel(const std::uint32_t text_key) {
    const auto text = field_mode_text_prompt_text(text_key);
    if (!text.has_value()) {
        failure_ = "mode-text prompt has no source text for its selected key";
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
            text->substr(cursor, end - cursor);
        if (separator == std::string_view::npos) {
            break;
        }
        cursor = separator + 1U;
    }
    return true;
}

bool FieldModeTextRuntime::open(
    const FieldModeTextPromptContext& context) {
    close();
    failure_.clear();
    if (!assets_loaded_) {
        failure_ = "mode-text prompt presentation assets are not loaded";
        return false;
    }

    const auto selected = select_field_mode_text_prompt(context);
    if (!selected.has_value()) {
        failure_ = "field mode/event has no converted mode-text prompt";
        return false;
    }
    prompt_ = *selected;
    if (!resolve_panel(prompt_.title_text)) {
        return false;
    }

    prompt_state_ = ModeTextPromptState{};
    (void)advance_mode_text_prompt(prompt_state_, prompt_, std::nullopt, 0U,
                                   {});
    if (prompt_state_.phase !=
        ModeTextPromptState::Phase::awaiting_any_event) {
        failure_ = "mode-text prompt source record is not title-only";
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
}

void FieldModeTextRuntime::paint(QPainter& painter) const {
    if (open_) {
        presenter_.paint(painter, panel_);
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
