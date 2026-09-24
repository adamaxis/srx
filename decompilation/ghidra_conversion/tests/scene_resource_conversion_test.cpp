#include "shadowrun_ghidra/scene_resource_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    SceneResourceLoadState state{};
    std::vector<std::uint8_t> uploaded;
    const auto result = load_scene_resources(
        state, {0xAAU, 0xBBU}, 2U,
        {{{0x1000U, 0x1001U}, {0x2000U, 0x2001U},
          {0x3000U, 0x3001U}}},
        [](const std::vector<std::uint8_t>& compressed) {
            if (compressed != std::vector<std::uint8_t>{0xAAU, 0xBBU}) {
                return std::optional<std::vector<std::uint8_t>>{};
            }
            return std::optional<std::vector<std::uint8_t>>{
                std::vector<std::uint8_t>{1U, 2U, 3U}};
        },
        [&uploaded](const std::vector<std::uint8_t>& decoded) {
            uploaded = decoded;
        });
    if (!expect(result == SceneResourceLoadResult::loaded &&
                state.resource_ready && state.decoded_resource == uploaded &&
                state.decoded_resource == std::vector<std::uint8_t>{1U, 2U,
                                                                      3U} &&
                state.index_entries.size() == 5U &&
                state.index_entries[0].first == 0x1000U &&
                state.index_entries[2].first == 0x2000U &&
                state.index_entries[4].first == 0x3000U)) {
        return 1;
    }

    const auto failed = load_scene_resources(
        state, {0x10U}, 1U, {},
        [](const std::vector<std::uint8_t>&) {
            return std::optional<std::vector<std::uint8_t>>{};
        },
        {});
    if (!expect(failed == SceneResourceLoadResult::decode_failed &&
                !state.resource_ready && state.decoded_resource.empty())) {
        return 2;
    }

    state.index_entries.clear();
    install_scene_resource_index_entries(
        state, 3U,
        {{{0x4000U, 0x4001U}, {0x5000U, 0x5001U},
          {0x6000U, 0x6001U}}});
    if (!expect(state.resource_ready && state.index_entries.size() == 7U &&
                state.index_entries[0].second == 0x4001U &&
                state.index_entries[3].second == 0x5001U &&
                state.index_entries[6].second == 0x6001U)) {
        return 3;
    }

    return 0;
}
