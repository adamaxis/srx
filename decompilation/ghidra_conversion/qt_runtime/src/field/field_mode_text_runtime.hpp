#pragma once

#include "dialogue_panel.hpp"
#include "dialogue_panel_presenter.hpp"
#include "shadowrun_ghidra/field/mode_text_prompt_catalog_conversion.hpp"

#include <cstdint>
#include <string>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class FieldModeTextAdvanceResult : std::uint8_t {
    waiting,
    returned_to_route,
};

// Host presenter for the source's generic mode-text prompt. Event properties
// select the native prompt descriptor; this class owns only its non-blocking
// wait state and the resolved text-only panel view.
class FieldModeTextRuntime final {
public:
    bool load_assets(const ConversionAssetCatalog& assets);

    bool open(const FieldModeTextPromptContext& context);
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
    bool resolve_panel(std::uint32_t text_key);

    DialoguePanelPresenter presenter_;
    ModeTextPromptDescriptor prompt_{};
    ModeTextPromptState prompt_state_{};
    DialoguePanelView panel_{};
    std::string failure_;
    bool assets_loaded_ = false;
    bool open_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
