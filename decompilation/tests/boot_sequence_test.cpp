#include "shadowrun/graphics.hpp"
#include "shadowrun/native_port.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using shadowrun::native::InputState;
using shadowrun::native::NativePortShell;
using shadowrun::native::NativeScene;
using shadowrun::native::SoftwareScreen;
using shadowrun::native::kBootGuyStartY;
using shadowrun::native::kBootGuyStopY;

std::vector<std::uint8_t> read_rom() {
    std::ifstream input(SHADOWRUN_TEST_ROM, std::ios::binary);
    assert(input);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void write_dump(const NativePortShell& shell, const std::filesystem::path& path) {
    SoftwareScreen screen;
    shell.render(screen);
    shadowrun::graphics::write_bmp32(
        path.string(), shadowrun::graphics::software_screen_image(screen));
}

std::size_t opaque_count(const SoftwareScreen& screen) {
    std::size_t count = 0;
    for (const auto pixel : screen.pixels()) {
        if (pixel != 0U) {
            ++count;
        }
    }
    return count;
}

} // namespace

int main() {
    const auto rom = read_rom();
    const auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    assert(assets.boot_background.width == 320);
    assert(assets.boot_guy.width == 320);
    assert(!assets.boot_sega_frames[0].pixels.empty());

    const auto artifact_dir =
        std::filesystem::path(SHADOWRUN_VISUAL_ARTIFACT_DIR) / "boot";
    std::filesystem::create_directories(artifact_dir);
    auto write_layer = [&artifact_dir](const shadowrun::native::NativeImage& image,
                                       const char* name) {
        shadowrun::graphics::RgbaImage layer{
            image.width, image.height, image.pixels};
        shadowrun::graphics::write_bmp32(
            (artifact_dir / name).string(), layer);
    };
    write_layer(assets.boot_background, "layer-background.bmp");
    write_layer(assets.boot_guy, "layer-guy.bmp");
    write_layer(assets.boot_sega_frames[0], "layer-sega-0.bmp");
    write_layer(assets.boot_sega_frames[3], "layer-sega-3.bmp");

    NativePortShell shell("SHADOWRUN", assets);
    assert(shell.scene() == NativeScene::Bootstrap);
    assert(shell.boot_guy_sat_y() == static_cast<std::int16_t>(kBootGuyStartY));
    assert(!shell.boot_sega_visible());

    write_dump(shell, artifact_dir / "boot-0.bmp");

    SoftwareScreen before;
    shell.render(before);
    const auto start_pixels = opaque_count(before);

    const auto rise = static_cast<int>(kBootGuyStartY - kBootGuyStopY);
    for (int step = 0; step < rise; ++step) {
        shell.update(InputState{});
    }
    assert(shell.scene() == NativeScene::Bootstrap);
    assert(shell.boot_guy_sat_y() == static_cast<std::int16_t>(kBootGuyStopY));
    write_dump(shell, artifact_dir / "boot-arrived.bmp");

    // 0x5D996 hides SEGA until DD30 is negative. The $14 delay starts the
    // frame the guy lands, then decrements once per pass.
    while (shell.scene() == NativeScene::Bootstrap &&
           !shell.boot_sega_visible()) {
        shell.update(InputState{});
    }
    assert(shell.boot_sega_visible());
    write_dump(shell, artifact_dir / "boot-sega.bmp");
    SoftwareScreen with_sega;
    shell.render(with_sega);
    if (opaque_count(with_sega) <= start_pixels) {
        throw std::runtime_error(
            "SEGA SAT phase did not add opaque pixels over the boot planes");
    }

    shell.finish_boot();
    assert(shell.scene() == NativeScene::Title);
    write_dump(shell, artifact_dir / "boot-to-title.bmp");
    return 0;
}
