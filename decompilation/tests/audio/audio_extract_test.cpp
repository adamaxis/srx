#include "shadowrun/audio/audio_extract.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace {

std::vector<std::uint8_t> read_file(const char* path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

bool fail(const std::string& message) {
    std::cerr << "audio extraction error: " << message << "\n";
    return false;
}

} // namespace

int main() {
    const auto rom = read_file(SHADOWRUN_TEST_ROM);
    if (rom.empty()) {
        return fail("test ROM is unavailable") ? 0 : 1;
    }
    const auto resources = shadowrun::audio::extract_genesis_audio_resources(rom);
    if (resources.size() != 5U) {
        return fail("expected five source-backed sound resources") ? 0 : 1;
    }
    const std::vector<std::pair<std::size_t, std::size_t>> expected = {
        {0x26C5E, 0x1886},
        {0x284E4, 0x10B1},
        {0x29595, 0x74},
        {0x29609, 0x7BB6},
        {0x311BF, 0x108},
    };
    for (std::size_t index = 0; index < resources.size(); ++index) {
        if (resources[index].source_offset != expected[index].first ||
            resources[index].byte_count != expected[index].second ||
            resources[index].bytes.size() != expected[index].second) {
            return fail("resource metadata does not match the ROM bootstrap")
                ? 0
                : 1;
        }
        for (std::size_t byte = 0; byte < expected[index].second; ++byte) {
            if (resources[index].bytes[byte] !=
                rom[expected[index].first + byte]) {
                return fail("resource bytes are not an exact ROM slice") ? 0 : 1;
            }
        }
    }
    const auto analysis_root = std::filesystem::temp_directory_path() /
                               "shadowrun_audio_source_analysis_test";
    std::filesystem::remove_all(analysis_root);
    shadowrun::audio::write_genesis_audio_resources(rom, analysis_root);
    if (!std::filesystem::is_regular_file(
            analysis_root / "audio_raw" / "catalog.tsv") ||
        std::filesystem::exists(analysis_root / "audio") ||
        std::filesystem::exists(analysis_root / "assets")) {
        std::filesystem::remove_all(analysis_root);
        return fail("audio source slices escaped the analysis-only directory")
            ? 0
            : 1;
    }
    std::filesystem::remove_all(analysis_root);
    const auto forbidden_root = std::filesystem::temp_directory_path() /
                                "shadowrun_audio_assets" / "assets";
    std::filesystem::remove_all(forbidden_root.parent_path());
    bool rejected_assets_root = false;
    try {
        shadowrun::audio::write_genesis_audio_resources(rom, forbidden_root);
    } catch (const std::invalid_argument&) {
        rejected_assets_root = true;
    }
    std::filesystem::remove_all(forbidden_root.parent_path());
    if (!rejected_assets_root) {
        return fail("audio extractor accepted the runtime assets directory")
            ? 0
            : 1;
    }
    std::cout << "audio resource slices verified: " << resources.size()
              << "\n";
    return 0;
}
