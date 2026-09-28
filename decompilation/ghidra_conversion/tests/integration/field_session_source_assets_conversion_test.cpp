#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"

#include <cstdint>
#include <filesystem>

#ifndef SHADOWRUN_CONVERSION_ASSET_ROOT
#error "The field-session source asset test needs the direct asset root"
#endif

int main() {
    using namespace shadowrun::ghidra_conversion;
    using namespace shadowrun::ghidra_conversion::integration;

    const auto loaded = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_CONVERSION_ASSET_ROOT));
    if (!loaded.complete()) {
        return 1;
    }
    const auto source = load_field_session_source_assets(loaded.catalog);
    if (!source.complete() ||
        source.assets->action_selection_profile_source.size() !=
            kFieldSessionActionProfileByteCount ||
        source.assets->field_mode_selection_state_source.size() !=
            kFieldSessionModeSelectionWordCount) {
        return 2;
    }

    const auto& profiles = source.assets->action_selection_profile_source;
    if (profiles[0] != 0x00U || profiles[1] != 0x03U || profiles[2] != 0x05U ||
        profiles[0x22U] != 0x00U || profiles[0x24U] != 0x00U ||
        profiles[0x25U] != 0x02U) {
        return 3;
    }
    const auto& mode_state = source.assets->field_mode_selection_state_source;
    if (mode_state.front() != 0x0300U || mode_state[1] != 0x001EU ||
        mode_state[2] != 0x0064U || mode_state.back() != 0x0000U) {
        return 4;
    }
    return 0;
}
