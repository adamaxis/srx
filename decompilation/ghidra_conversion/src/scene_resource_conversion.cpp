#include "shadowrun_ghidra/scene_resource_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void install_scene_resource_index_entries(
    SceneResourceLoadState& state,
    const std::size_t index_stride,
    const std::array<SceneResourceIndexEntry, 3>& source_entries) {
    const auto final_index = index_stride * 2U;
    if (state.index_entries.size() <= final_index) {
        state.index_entries.resize(final_index + 1U);
    }
    state.index_entries[0] = source_entries[0];
    state.index_entries[index_stride] = source_entries[1];
    state.index_entries[final_index] = source_entries[2];
    state.resource_ready = true;
}

SceneResourceLoadResult load_scene_resources(
    SceneResourceLoadState& state,
    const std::vector<std::uint8_t>& compressed_payload,
    const std::size_t index_stride,
    const std::array<SceneResourceIndexEntry, 3>& source_entries,
    const SceneResourceDecodeBoundary& decode_resource,
    const SceneResourceUploadBoundary& upload_resource) {
    state.resource_ready = false;
    const auto decoded = decode_resource ? decode_resource(compressed_payload)
                                         : std::nullopt;
    if (!decoded.has_value()) {
        state.decoded_resource.clear();
        return SceneResourceLoadResult::decode_failed;
    }

    state.decoded_resource = *decoded;
    if (upload_resource) {
        upload_resource(state.decoded_resource);
    }

    install_scene_resource_index_entries(state, index_stride, source_entries);
    return SceneResourceLoadResult::loaded;
}

} // namespace shadowrun::ghidra_conversion
