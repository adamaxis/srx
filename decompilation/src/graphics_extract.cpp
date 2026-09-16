#include "shadowrun/graphics.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::uint64_t parse_number(const std::string& value) {
    std::size_t consumed = 0;
    const auto number = std::stoull(value, &consumed, 0);
    if (consumed != value.size()) {
        throw std::invalid_argument("invalid numeric argument: " + value);
    }
    return number;
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open ROM: " + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

shadowrun::graphics::RgbaImage make_field_entity_state_sheet(
    const std::vector<shadowrun::graphics::RgbaImage>& frames) {
    constexpr std::uint16_t kCellWidth = 40;
    constexpr std::uint16_t kCellHeight = 40;
    constexpr std::size_t kColumns = 4;
    constexpr std::size_t kRows = 2;
    shadowrun::graphics::RgbaImage sheet{
        static_cast<std::uint16_t>(kCellWidth * kColumns),
        static_cast<std::uint16_t>(kCellHeight * kRows),
        std::vector<shadowrun::native::Rgba8>(
            static_cast<std::size_t>(kCellWidth * kColumns) *
                (kCellHeight * kRows),
            {0x04, 0x08, 0x14, 0xFF}),
    };
    for (std::size_t index = 0; index < frames.size(); ++index) {
        const auto& frame = frames[index];
        const auto cell_x = static_cast<std::int32_t>(
            (index % kColumns) * kCellWidth);
        const auto cell_y = static_cast<std::int32_t>(
            (index / kColumns) * kCellHeight);
        const auto origin_x = cell_x +
            (static_cast<std::int32_t>(kCellWidth) - frame.width) / 2;
        const auto origin_y = cell_y +
            (static_cast<std::int32_t>(kCellHeight) - frame.height) / 2;
        for (std::int32_t y = 0; y < frame.height; ++y) {
            for (std::int32_t x = 0; x < frame.width; ++x) {
                const auto& pixel = frame.pixels[
                    static_cast<std::size_t>(y) * frame.width + x];
                if (pixel.alpha == 0) {
                    continue;
                }
                const auto target_x = origin_x + x;
                const auto target_y = origin_y + y;
                sheet.pixels[static_cast<std::size_t>(target_y) * sheet.width +
                             target_x] = pixel;
            }
        }
    }
    return sheet;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 4 && std::string(argv[3]) == "--title") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_startup_screen(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "startup scene: " << image.width << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field") {
        try {
            const auto rom = read_file(argv[1]);
            shadowrun::native::SoftwareScreen screen;
            shadowrun::native::NativePortShell shell("SHADOWRUN", rom);
            shell.update(shadowrun::native::InputState{
                shadowrun::native::button_mask(
                    shadowrun::native::Button::Start),
                shadowrun::native::button_mask(
                    shadowrun::native::Button::Start)});
            shell.render(screen);

            shadowrun::graphics::RgbaImage image{
                screen.width(), screen.height(),
                std::vector<shadowrun::native::Rgba8>(screen.pixels().size()),
            };
            for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
                const auto packed = screen.pixels()[index];
                image.pixels[index] = {
                    static_cast<std::uint8_t>(packed >> 16U),
                    static_cast<std::uint8_t>(packed >> 8U),
                    static_cast<std::uint8_t>(packed), 255,
                };
            }
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--mode2-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_mode2_field_map(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "mode-2 field map: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--entity-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_entity_frame(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "entity frame: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-states") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity state sheet: " << sheet.width << "x"
                      << sheet.height << " (" << frames.size() << " states)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-entity-state") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            if (index >= frames.size()) {
                throw std::out_of_range("field entity state index is out of range");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "field entity state " << index << ": "
                      << frames[index].width << "x" << frames[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc != 6 && argc != 7) {
        std::cerr << "usage: shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "<tile-offset> <tile-count> <palette-offset> [--resource]\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> --title\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--native-field\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--mode2-map\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--entity-frame\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-states\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-state <index>\n";
        std::cerr << "numeric offsets accept decimal or 0x-prefixed hex\n";
        return 2;
    }
    try {
        const auto rom = read_file(argv[1]);
        const auto tile_offset = static_cast<std::size_t>(parse_number(argv[3]));
        const auto tile_count = static_cast<std::size_t>(parse_number(argv[4]));
        const auto palette_offset = static_cast<std::size_t>(parse_number(argv[5]));
        const auto resource_stream = argc == 7 && std::string(argv[6]) == "--resource";
        if (argc == 7 && !resource_stream) {
            throw std::invalid_argument("unknown extractor mode");
        }
        const auto palette = shadowrun::graphics::read_genesis_palette(
            rom, palette_offset);
        const auto resource = resource_stream
            ? shadowrun::graphics::decode_genesis_resource_stream(rom, tile_offset)
            : std::vector<std::uint8_t>{};
        const auto& tile_source = resource_stream ? resource : rom;
        const auto image = shadowrun::graphics::decode_tile_atlas(
            tile_source, resource_stream ? 0U : tile_offset, tile_count, palette);
        shadowrun::graphics::write_bmp32(argv[2], image);
        std::cout << "tiles: " << tile_count << "\n"
                  << "image: " << image.width << "x" << image.height << "\n"
                  << (resource_stream ? "decoded bytes: " +
                                             std::to_string(resource.size()) +
                                             "\n" : "")
                  << "output: " << argv[2] << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
