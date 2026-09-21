#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

std::vector<std::string> split_tabs(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t begin = 0;
    for (;;) {
        const auto end = line.find('\t', begin);
        fields.push_back(line.substr(begin, end - begin));
        if (end == std::string::npos) {
            return fields;
        }
        begin = end + 1U;
    }
}

std::uint32_t read_u32(const std::vector<std::uint8_t>& bytes,
                       std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

bool fail(const std::string& message) {
    std::cerr << "graphics catalog error: " << message << "\n";
    return false;
}

} // namespace

int main() {
    const std::filesystem::path root = SHADOWRUN_ASSET_ROOT;
    const auto catalog_path = root / "catalog.tsv";
    std::ifstream catalog(catalog_path, std::ios::binary);
    if (!catalog) {
        return fail("cannot open " + catalog_path.string()) ? 0 : 1;
    }

    std::string line;
    if (!std::getline(catalog, line) ||
        line != "asset\tkind\tx\ty\twidth\theight\tsource\tstatus") {
        return fail("catalog header is invalid") ? 0 : 1;
    }

    std::set<std::string> listed;
    std::size_t row_count = 0;
    while (std::getline(catalog, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = split_tabs(line);
        if (fields.size() != 8U) {
            return fail("catalog row does not have eight fields") ? 0 : 1;
        }
        const auto& relative = fields[0];
        const auto has_bmp_suffix =
            relative.size() >= 4U && relative.compare(relative.size() - 4U, 4U,
                                                       ".bmp") == 0;
        if (relative.empty() || relative.find("..") != std::string::npos ||
            !has_bmp_suffix) {
            return fail("catalog path is not a safe BMP path: " + relative)
                ? 0
                : 1;
        }
        if (fields[1] != "background" && fields[1] != "sprite") {
            return fail("unknown asset kind for " + relative) ? 0 : 1;
        }
        if (fields[7] != "extracted" && fields[7] != "extracted-empty") {
            return fail("asset is not extracted: " + relative) ? 0 : 1;
        }
        if (!listed.insert(relative).second) {
            return fail("duplicate catalog path: " + relative) ? 0 : 1;
        }
        const auto path = root / relative;
        std::ifstream image(path, std::ios::binary);
        if (!image) {
            return fail("catalog file is missing: " + path.string()) ? 0 : 1;
        }
        std::vector<std::uint8_t> header(26U, 0U);
        image.read(reinterpret_cast<char*>(header.data()),
                   static_cast<std::streamsize>(header.size()));
        if (image.gcount() != static_cast<std::streamsize>(header.size()) ||
            header[0] != 'B' || header[1] != 'M') {
            return fail("asset is not a readable BMP: " + path.string()) ? 0
                                                                            : 1;
        }
        const auto width = std::stoul(fields[4]);
        const auto height = std::stoul(fields[5]);
        if (read_u32(header, 18U) != width || read_u32(header, 22U) != height ||
            width == 0U || height == 0U) {
            return fail("catalog dimensions do not match " + path.string())
                ? 0
                : 1;
        }
        ++row_count;
    }

    std::set<std::string> files;
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".bmp") {
            continue;
        }
        auto relative = std::filesystem::relative(entry.path(), root).string();
        for (auto& character : relative) {
            if (character == '\\') {
                character = '/';
            }
        }
        files.insert(relative);
    }
    if (files != listed) {
        return fail("catalog and BMP file set differ") ? 0 : 1;
    }
    if (row_count == 0U ||
        listed.find("intro/background-0.bmp") == listed.end() ||
        listed.find("intro/sprites/ambush-michael-0-44.bmp") == listed.end()) {
        return fail("required opening background or sprite resource is absent")
            ? 0
            : 1;
    }

    std::cout << "graphics catalog verified: " << row_count << " resources\n";
    return 0;
}
