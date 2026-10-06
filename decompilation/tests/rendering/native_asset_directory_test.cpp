#include "shadowrun/rendering/graphics.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::vector<std::string> split_tsv(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (start <= line.size()) {
        const auto end = line.find('\t', start);
        if (end == std::string::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, end - start));
        start = end + 1;
    }
    return fields;
}

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void require_image(const shadowrun::native::NativeImage& image,
                   const std::string& alias) {
    require(image.width != 0U && image.height != 0U &&
                image.pixels.size() ==
                    static_cast<std::size_t>(image.width) * image.height,
            "invalid image resource: " + alias);
}

} // namespace

int main() {
    try {
        const std::filesystem::path root(SHADOWRUN_ASSET_ROOT);
        const auto assets = shadowrun::graphics::read_native_asset_directory(
            root.string());
        require(assets.recruited_runner_templates.size() == 10U,
                "direct state lost recruited roster templates");
        require(assets.recruited_runner_templates[0].display_name == "Ricky" &&
                    assets.recruited_runner_templates[9].display_name == "Stark",
                "direct state lost recruited roster names");
        require(assets.recruited_runner_templates[0]
                        .record.inventory_item_codes[1] == 0x2DU,
                "direct state lost recruited runner inventory");

        std::ifstream manifest(root / "resources.tsv", std::ios::binary);
        require(static_cast<bool>(manifest), "missing resources.tsv");

        std::string line;
        require(static_cast<bool>(std::getline(manifest, line)) &&
                    line == "alias\tpath\tkind\torigin_x\torigin_y",
                "invalid resources.tsv header");

        std::size_t image_count = 0;
        std::size_t audio_count = 0;
        std::size_t state_count = 0;
        std::size_t text_count = 0;
        bool native_state_seen = false;
        while (std::getline(manifest, line)) {
            if (line.empty() || line.front() == '#') {
                continue;
            }

            const auto fields = split_tsv(line);
            require(fields.size() == 5U, "invalid resources.tsv row");

            const auto& alias = fields[0];
            const auto& relative_path = fields[1];
            const auto& kind = fields[2];
            require(!alias.empty() && !relative_path.empty(),
                    "manifest row has an empty alias or path");
            require(std::filesystem::exists(root / relative_path),
                    "manifest path does not exist: " + relative_path);

            if (kind == "image") {
                require_image(assets[alias], alias);
                ++image_count;
            } else if (kind == "audio") {
                require(std::filesystem::path(relative_path).extension() ==
                            ".flac",
                        "audio resource is not a FLAC: " + relative_path);
                require(assets.audio_resources.find(alias) !=
                            assets.audio_resources.end(),
                        "audio manifest alias was not loaded: " + alias);
                require(assets.audio_path(alias) == root / relative_path,
                        "audio alias resolved to the wrong path: " + alias);
                ++audio_count;
            } else if (kind == "state") {
                if (alias == "state") {
                    native_state_seen = true;
                }
                ++state_count;
            } else if (kind == "text") {
                require(alias == "menu.field.text" ||
                            alias == "dialogue.stoker.motel" ||
                            alias == "menu.pocket_secretary.notebook.tips" ||
                            alias == "dialogue.field.interactions",
                        "unexpected text resource alias: " + alias);
                ++text_count;
            } else {
                throw std::runtime_error("unknown manifest resource kind: " +
                                         kind);
            }
        }

        require(native_state_seen && state_count != 0U,
                "resources.tsv must contain a native state row");
        require(text_count == 4U,
                "resources.tsv must contain field, dialogue, interaction, and Notebook Tips text rows");
        require(image_count == assets.resources.size(),
                "manifest image rows and loaded resources differ");
        require(audio_count == 65U,
                "unexpected number of extracted audio resources");
        require(audio_count == assets.audio_resources.size(),
                "manifest audio rows and loaded resources differ");
        require(assets.audio_loop_points.size() == 11U,
                "unexpected number of source-verified audio loops");
        const auto title_loop =
            assets.audio_loop("audio.music.resource.000");
        require(title_loop.has_value() && title_loop->start_frame == 1785366U &&
                    title_loop->end_frame == 3562596U &&
                    title_loop->sample_rate == 44100U,
                "audio loop points were not loaded exactly");
        for (const auto resource : {"005", "014", "015", "040"}) {
            require(!assets.audio_loop("audio.music.resource." +
                                       std::string(resource)).has_value(),
                    "unverified audio restart was incorrectly marked loopable");
        }
        require(!std::filesystem::exists(
                    root / "field" / "entities" / "idle-gray-0.bmp"),
                "unused grayscale resource still present");
        require(!std::filesystem::exists(
                    root / "intro" / "wilderness-pan-0.bmp"),
                "old intro flipbook resource still present");
        require(!std::filesystem::exists(root / "intro" / "ambush-0.bmp"),
                "old intro flipbook resource still present");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
