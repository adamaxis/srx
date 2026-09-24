#include "shadowrun_ghidra/mode_prompt_prepare_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;
    ModePromptPrepareState state{};
    int calls = 0;
    prepare_mode_text_prompt(state, [&] { ++calls; });
    if (state.preparation_passes != 2U || calls != 2) {
        return 1;
    }
    return 0;
}
