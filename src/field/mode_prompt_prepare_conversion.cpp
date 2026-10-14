#include "shadowrun_ghidra/field/mode_prompt_prepare_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void prepare_mode_text_prompt(
    ModePromptPrepareState& state,
    const std::function<void()>& prepare_text_assets) {
    for (std::uint8_t pass = 0; pass < 2U; ++pass) {
        if (prepare_text_assets) {
            prepare_text_assets();
        }
        ++state.preparation_passes;
    }
}

} // namespace shadowrun::ghidra_conversion
