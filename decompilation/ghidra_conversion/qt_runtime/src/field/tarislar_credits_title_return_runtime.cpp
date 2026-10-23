#include "tarislar_credits_title_return_runtime.hpp"

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint8_t kVblanksPerWaitAtC024 = 3U;
constexpr std::uint8_t kCreditsServiceIterations = 6U;
constexpr std::uint8_t kOuterFadeWaitVblanks =
    kTarislarCreditsPalettePassCount *
    kTarislarCreditsPalettePassVblanks;

static_assert(kCreditsServiceIterations * kVblanksPerWaitAtC024 ==
              kTarislarCreditsPostPanelServiceVblanks);

} // namespace

bool TarislarCreditsTitleReturnRuntime::begin() noexcept {
    if (phase_ != TarislarCreditsTitleReturnPhase::idle) {
        return false;
    }
    phase_ = TarislarCreditsTitleReturnPhase::post_panel_service;
    remaining_vblanks_ = kTarislarCreditsPostPanelServiceVblanks;
    vblanks_in_palette_pass_ = 0U;
    palette_passes_ = 0U;
    return true;
}

void TarislarCreditsTitleReturnRuntime::reset() noexcept {
    phase_ = TarislarCreditsTitleReturnPhase::idle;
    remaining_vblanks_ = 0U;
    vblanks_in_palette_pass_ = 0U;
    palette_passes_ = 0U;
}

TarislarCreditsTitleReturnUpdate
TarislarCreditsTitleReturnRuntime::advance() noexcept {
    TarislarCreditsTitleReturnUpdate update{};
    switch (phase_) {
    case TarislarCreditsTitleReturnPhase::idle:
    case TarislarCreditsTitleReturnPhase::complete:
        return update;
    case TarislarCreditsTitleReturnPhase::post_panel_service:
        if (remaining_vblanks_ != 0U) {
            --remaining_vblanks_;
        }
        if (remaining_vblanks_ == 0U) {
            phase_ = TarislarCreditsTitleReturnPhase::fade_to_black;
        }
        return update;
    case TarislarCreditsTitleReturnPhase::fade_to_black:
        ++vblanks_in_palette_pass_;
        if (vblanks_in_palette_pass_ <
            kTarislarCreditsPalettePassVblanks) {
            return update;
        }
        vblanks_in_palette_pass_ = 0U;
        ++palette_passes_;
        update.palette_pass_changed = true;
        if (palette_passes_ == kTarislarCreditsPalettePassCount) {
            phase_ = TarislarCreditsTitleReturnPhase::outer_loop_black_wait;
            remaining_vblanks_ = kOuterFadeWaitVblanks;
        }
        return update;
    case TarislarCreditsTitleReturnPhase::outer_loop_black_wait:
        if (remaining_vblanks_ != 0U) {
            --remaining_vblanks_;
        }
        if (remaining_vblanks_ == 0U) {
            phase_ = TarislarCreditsTitleReturnPhase::complete;
            update.completed = true;
        }
        return update;
    }
    return update;
}

std::uint8_t
TarislarCreditsTitleReturnRuntime::palette_passes_to_black() const noexcept {
    switch (phase_) {
    case TarislarCreditsTitleReturnPhase::fade_to_black:
    case TarislarCreditsTitleReturnPhase::outer_loop_black_wait:
    case TarislarCreditsTitleReturnPhase::complete:
        return palette_passes_;
    case TarislarCreditsTitleReturnPhase::idle:
    case TarislarCreditsTitleReturnPhase::post_panel_service:
        return 0U;
    }
    return 0U;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
