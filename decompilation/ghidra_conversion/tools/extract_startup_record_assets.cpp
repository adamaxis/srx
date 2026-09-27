#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kRecordSize = 0x100U;
constexpr std::size_t kJoshuaOffset = 0x7291AU;
constexpr std::size_t kJoshuaCount = 3U;
constexpr std::size_t kRecruitedOffset = 0x72C1AU;
constexpr std::size_t kRecruitedCount = 12U;
constexpr std::size_t kSaveRosterHeaderOffset = 0x59972U;
constexpr std::size_t kSaveRosterHeaderSize = 0x14U;

std::vector<std::uint8_t> read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        throw std::runtime_error("cannot open ROM: " + path.string());
    }
    const auto size = input.tellg();
    if (size < 0) {
        throw std::runtime_error("cannot determine ROM size");
    }
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    if (!input) {
        throw std::runtime_error("cannot read ROM: " + path.string());
    }
    return bytes;
}

void write_slice(const std::filesystem::path& path,
                 const std::vector<std::uint8_t>& rom,
                 const std::size_t offset,
                 const std::size_t size) {
    if (offset > rom.size() || rom.size() - offset < size) {
        throw std::runtime_error("ROM is truncated at source offset 0x" +
                                 std::to_string(offset));
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create extracted asset: " +
                                 path.string());
    }
    output.write(reinterpret_cast<const char*>(rom.data() + offset),
                 static_cast<std::streamsize>(size));
    if (!output) {
        throw std::runtime_error("cannot write extracted asset: " +
                                 path.string());
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string(argv[1]) == "--help") {
        std::cout << "usage: shadowrun_ghidra_startup_record_extract"
                     " <shadowrun_genesis_us.bin> <asset-root>\n";
        return argc == 2 && std::string(argv[1]) == "--help" ? 0 : 2;
    }

    try {
        const auto rom = read_file(argv[1]);
        const std::filesystem::path asset_root(argv[2]);
        const auto output_root = asset_root / "records" / "startup";
        std::filesystem::create_directories(output_root);

        write_slice(output_root / "joshua-archetypes.bin", rom,
                    kJoshuaOffset, kJoshuaCount * kRecordSize);
        write_slice(output_root / "recruited-runner-records.bin", rom,
                    kRecruitedOffset, kRecruitedCount * kRecordSize);
        write_slice(output_root / "save-roster-header.bin", rom,
                    kSaveRosterHeaderOffset, kSaveRosterHeaderSize);
        std::cout << "extracted startup records: " << output_root.string()
                  << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "shadowrun_ghidra_startup_record_extract: "
                  << error.what() << '\n';
        return 1;
    }
}
