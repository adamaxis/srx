#include "shadowrun/graphics.hpp"

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

        std::ifstream manifest(root / "resources.tsv", std::ios::binary);
        require(static_cast<bool>(manifest), "missing resources.tsv");

        std::string line;
        require(static_cast<bool>(std::getline(manifest, line)) &&
                    line == "alias\tpath\tkind\torigin_x\torigin_y",
                "invalid resources.tsv header");

        std::size_t image_count = 0;
        std::size_t state_count = 0;
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
            } else if (kind == "state") {
                require(alias == "state", "unexpected state alias");
                ++state_count;
            } else {
                throw std::runtime_error("unknown manifest resource kind: " +
                                         kind);
            }
        }

        require(state_count == 1U, "resources.tsv must contain one state row");
        require(image_count == assets.resources.size(),
                "manifest image rows and loaded resources differ");
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
