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
    std::function<void(std::string_view)> source_boundary)
    : source_boundary_callback_(std::move(source_boundary)) {}

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
    return open_stoker_prompt(prompt);
}

bool FieldDialogueRuntime::open_stoker_prompt(
    const StokerMotelPromptId prompt) {
    if (!assets_loaded_) {
        return open_source_boundary(
            failure_.empty() ? "Stoker dialogue assets are not loaded"
                             : failure_);
    }
    boundary_reason_.clear();
    active_prompt_ = prompt;
    const auto source_view = make_stoker_motel_prompt(prompt);
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
    boundary_reason_ = std::move(reason);
    panel_ = DialoguePanelView{};
    panel_.open = true;
    panel_.actor_kind = DialoguePanelActorKind::field_npc;
    panel_.actor_name = "";
    panel_.kind = DialoguePanelKind::source_boundary;
    panel_.source_complete = false;
    panel_.source_boundary_reason = boundary_reason_;
    active_prompt_ = StokerMotelPromptId::source_boundary;
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
    // the shared wait-for-controller path and returns to field after any
    // button edge; it is not a hidden fourth option.
    if (panel_.option_count == 0U) {
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
    panel_ = DialoguePanelView{};
    active_prompt_ = StokerMotelPromptId::source_boundary;
    boundary_reason_.clear();
}

void FieldDialogueRuntime::paint(QPainter& painter) const {
    presenter_.paint(painter, panel_);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
