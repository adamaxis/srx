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
    const auto& normal_profile = source.assets->normal_scene_profile_source;
    if (normal_profile[0] != 0x0A00U || normal_profile[1] != 0x0280U ||
        normal_profile[4] != 0x6700U || normal_profile[15] != 0x7CECU ||
        normal_profile.back() != 0x1215U) {
        return 5;
    }
    const auto& alternate_profile =
        source.assets->alternate_scene_profile_source;
    if (alternate_profile[0] != 0x4080U || alternate_profile[1] != 0x4C80U ||
        alternate_profile[4] != 0x5300U || alternate_profile[15] != 0xAD3AU ||
        alternate_profile.back() != 0x1A0DU) {
        return 6;
    }
    const auto& terrain_a = source.assets->mode2_terrain_layer_a_source;
    const auto& terrain_b = source.assets->mode2_terrain_layer_b_source;
    if (terrain_a.size() != kFieldSessionTerrainLayerWordCount ||
        terrain_b.size() != kFieldSessionTerrainLayerWordCount ||
        terrain_a.front() != 0x0013U || terrain_a[0x708U] != 0x013BU ||
        terrain_a.back() != 0x013BU || terrain_b.front() != 0x013BU ||
        terrain_b[0x708U] != 0x34FFU || terrain_b.back() != 0x8664U) {
        return 7;
    }
    const auto& terrain_stream = source.assets->mode2_terrain_stream_source;
    if (terrain_stream.size() != kFieldSessionTerrainStreamByteCount ||
        terrain_stream[0] != 0x02U || terrain_stream[1] != 0x4CU ||
        terrain_stream[2] != 0x02U || terrain_stream[3] != 0x4DU ||
        terrain_stream[terrain_stream.size() - 2U] != 0x04U ||
        terrain_stream.back() != 0x65U) {
        return 8;
    }
    const auto scene_input =
        make_initial_mode2_field_scene_input(*source.assets);
    if (scene_input.scene_reference != 2U ||
        scene_input.normal_profile != source.assets->normal_scene_profile_source ||
        scene_input.alternate_profile !=
            source.assets->alternate_scene_profile_source ||
        scene_input.layer_a != source.assets->mode2_terrain_layer_a_source ||
        scene_input.layer_b != source.assets->mode2_terrain_layer_b_source ||
        scene_input.terrain_stream != source.assets->mode2_terrain_stream_source) {
        return 9;
    }
    return 0;
}
