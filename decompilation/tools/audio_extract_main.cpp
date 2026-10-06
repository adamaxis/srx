#include "shadowrun/audio/audio_extract.hpp"

#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open ROM: " + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: shadowrun_audio_extract <rom.bin> <analysis-dir>\n";
        return 2;
    }
    try {
        const auto rom = read_file(argv[1]);
        shadowrun::audio::write_genesis_audio_resources(rom, argv[2]);
        std::cout << "Genesis sound source-analysis slices written to " << argv[2]
                  << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
