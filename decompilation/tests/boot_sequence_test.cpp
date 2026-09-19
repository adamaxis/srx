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
    assert(assets.boot_static.width == 320);
    assert(assets.boot_guy.width == 320);
    assert(!assets.boot_sega_frames[0].pixels.empty());
    assert(!assets.boot_logo_strobe[0].pixels.empty());
    assert(!assets.boot_fasa.pixels.empty());
    assert(!assets.startup_background.pixels.empty());

    std::size_t logo_pixels = 0;
    for (const auto& pixel : assets.boot_background.pixels) {
        if (pixel.red != 0 || pixel.green != 0 || pixel.blue != 0) {
            ++logo_pixels;
        }
    }
    if (logo_pixels == 0) {
        throw std::runtime_error(
            "plane B SEGA letterforms produced no opaque pixels");
    }

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
    write_layer(assets.boot_static, "layer-static.bmp");
    write_layer(assets.boot_guy, "layer-guy.bmp");
    write_layer(assets.boot_sega_frames[0], "layer-sega-0.bmp");
    write_layer(assets.boot_sega_frames[3], "layer-sega-3.bmp");
    write_layer(assets.boot_logo_strobe[0], "layer-strobe-0.bmp");
    write_layer(assets.boot_logo_strobe[3], "layer-strobe-3.bmp");
    write_layer(assets.boot_fasa, "layer-fasa.bmp");
    write_layer(assets.startup_background, "layer-startup.bmp");

    NativePortShell shell("SHADOWRUN", assets);
    assert(shell.scene() == NativeScene::Bootstrap);
    assert(shell.boot_guy_sat_y() == static_cast<std::int16_t>(kBootGuyStartY));
    assert(!shell.boot_sega_visible());
    assert(!shell.boot_logo_strobe_active());

    write_dump(shell, artifact_dir / "boot-0.bmp");
    SoftwareScreen boot_zero;
    shell.render(boot_zero);
    if (opaque_count(boot_zero) == 0) {
        throw std::runtime_error("boot-0 framebuffer has no SEGA letter pixels");
    }

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
    if (shell.boot_hscroll_line(80) > 0x1FU) {
        throw std::runtime_error("boot HSCROLL left the 0x5DAC4 0..31 range");
    }

    while (shell.scene() == NativeScene::Bootstrap &&
           !shell.boot_logo_strobe_active()) {
        shell.update(InputState{});
    }
    if (shell.scene() == NativeScene::Bootstrap) {
        write_dump(shell, artifact_dir / "boot-strobe.bmp");
        assert(shell.boot_logo_strobe_active());
        assert(!shell.boot_lightning_visible());
        const auto strobe_phase = shell.boot_logo_strobe_phase();
        for (int step = 0; step < 16 &&
                           shell.scene() == NativeScene::Bootstrap; ++step) {
            shell.update(InputState{});
        }
        if (shell.scene() == NativeScene::Bootstrap &&
            shell.boot_logo_strobe_phase() == strobe_phase &&
            shell.boot_logo_strobe_active()) {
            throw std::runtime_error(
                "0x5D8BA logo strobe phase did not advance");
        }
    }

    while (shell.scene() == NativeScene::Bootstrap && !shell.boot_fading()) {
        shell.update(InputState{});
    }
    assert(shell.boot_fading());
    write_dump(shell, artifact_dir / "boot-fade-start.bmp");
    SoftwareScreen fade_start;
    shell.render(fade_start);
    const auto fade_start_ink = opaque_count(fade_start);
    assert(fade_start_ink > 0);
    for (int step = 0; step < 16 && shell.boot_fading(); ++step) {
        shell.update(InputState{});
    }
    write_dump(shell, artifact_dir / "boot-fade-mid.bmp");
    SoftwareScreen fade_mid;
    shell.render(fade_mid);
    auto brightness = [](const SoftwareScreen& screen) {
        std::uint64_t sum = 0;
        for (const auto pixel : screen.pixels()) {
            sum += (pixel >> 16U) & 0xFFU;
            sum += (pixel >> 8U) & 0xFFU;
            sum += pixel & 0xFFU;
        }
        return sum;
    };
    assert(brightness(fade_mid) < brightness(fade_start));
    assert(opaque_count(fade_mid) > 0);

    while (shell.scene() == NativeScene::Bootstrap &&
           !shell.boot_fasa_visible()) {
        shell.update(InputState{});
    }
    assert(shell.boot_fasa_visible());
    SoftwareScreen fasa_in;
    shell.render(fasa_in);
    for (int step = 0; step < 12 && shell.boot_fasa_visible(); ++step) {
        shell.update(InputState{});
    }
    write_dump(shell, artifact_dir / "boot-fasa.bmp");
    SoftwareScreen fasa_screen;
    shell.render(fasa_screen);
    if (opaque_count(fasa_screen) == 0) {
        throw std::runtime_error("FASA splash produced no opaque pixels");
    }
    if (brightness(fasa_screen) <= brightness(fasa_in)) {
        throw std::runtime_error("FASA splash did not fade in from black");
    }

    shell.finish_boot();
    assert(shell.scene() == NativeScene::Title);
    write_dump(shell, artifact_dir / "boot-to-title.bmp");
    return 0;
}
