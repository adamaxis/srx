#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct SceneResourceIndexEntry {
    std::uint16_t first = 0;
    std::uint16_t second = 0;
};

struct SceneResourceLoadState {
    std::vector<std::uint8_t> decoded_resource;
    std::vector<SceneResourceIndexEntry> index_entries;
    bool resource_ready = false;
};

using SceneResourceDecodeBoundary = std::function<
    std::optional<std::vector<std::uint8_t>>(
        const std::vector<std::uint8_t>& compressed_payload)>;
using SceneResourceUploadBoundary =
    std::function<void(const std::vector<std::uint8_t>& decoded_resource)>;

enum class SceneResourceLoadResult {
    loaded,
    decode_failed,
};

// ROM 0x5BA66: install the three source index pairs at the runtime stride and
// publish the source-ready flag.
void install_scene_resource_index_entries(
    SceneResourceLoadState& state,
    std::size_t index_stride,
    const std::array<SceneResourceIndexEntry, 3>& source_entries);

// ROM 0x5BA50: decode the scene resource, hand the host bytes to the
// presentation/import boundary, install three source index entries at the
// supplied typed stride, and publish the ready flag. The source's 0x1530 VDP
// transfer is deliberately represented only by the upload boundary.
SceneResourceLoadResult load_scene_resources(
    SceneResourceLoadState& state,
    const std::vector<std::uint8_t>& compressed_payload,
    std::size_t index_stride,
    const std::array<SceneResourceIndexEntry, 3>& source_entries,
    const SceneResourceDecodeBoundary& decode_resource,
    const SceneResourceUploadBoundary& upload_resource);

} // namespace shadowrun::ghidra_conversion
