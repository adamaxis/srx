#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {

// ROM 0x5BAC4 spends six 0x53126 service iterations after the final credits
// input. Each iteration uses D5=2 with the DBRA wait at 0x0C24, for three
// VBlanks. The callback then fades to black with 0x131C (17 passes, two
// VBlanks per pass); the outer field loop repeats that fade once at 0x078A.
constexpr std::uint8_t kTarislarCreditsPostPanelServiceVblanks = 18U;
constexpr std::uint8_t kTarislarCreditsPalettePassCount = 17U;
constexpr std::uint8_t kTarislarCreditsPalettePassVblanks = 2U;

enum class TarislarCreditsTitleReturnPhase : std::uint8_t {
    idle,
    post_panel_service,
    fade_to_black,
    outer_loop_black_wait,
    complete,
};

struct TarislarCreditsTitleReturnUpdate {
    bool palette_pass_changed = false;
    bool completed = false;
};

// Typed host presentation state for the final Scene-55 credits-to-title tail.
// The black image is presented directly; no palette RAM or source loop is
// reproduced at runtime.
class TarislarCreditsTitleReturnRuntime final {
public:
    bool begin() noexcept;
    void reset() noexcept;
    TarislarCreditsTitleReturnUpdate advance() noexcept;

    TarislarCreditsTitleReturnPhase phase() const noexcept { return phase_; }
    bool is_presenting() const noexcept {
        return phase_ != TarislarCreditsTitleReturnPhase::idle;
    }
    bool complete() const noexcept {
        return phase_ == TarislarCreditsTitleReturnPhase::complete;
    }
    std::uint8_t palette_passes_to_black() const noexcept;

private:
    TarislarCreditsTitleReturnPhase phase_ =
        TarislarCreditsTitleReturnPhase::idle;
    std::uint8_t remaining_vblanks_ = 0U;
    std::uint8_t vblanks_in_palette_pass_ = 0U;
    std::uint8_t palette_passes_ = 0U;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
