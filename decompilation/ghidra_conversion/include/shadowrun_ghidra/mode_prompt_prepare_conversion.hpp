#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ModePromptPrepareState {
    std::uint8_t preparation_passes = 0;
};

// ROM 0x1F5A8: invoke the shared prompt preparation leaf twice.
void prepare_mode_text_prompt(
    ModePromptPrepareState& state,
    const std::function<void()>& prepare_text_assets);

} // namespace shadowrun::ghidra_conversion
