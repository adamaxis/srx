#include "shadowrun/native_port.hpp"

#include <cassert>
#include <fstream>
#include <iterator>
#include <vector>

namespace {

std::vector<std::uint8_t> read_rom() {
    std::ifstream input(SHADOWRUN_TEST_ROM, std::ios::binary);
    assert(input);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

} // namespace

int main() {
    using namespace shadowrun::native;

    SoftwareScreen screen;
    NativePortShell shell("SHADOWRUN");
    shell.render(screen);
    assert(screen.presented_frames() == 1);
    assert(screen.pixel(319, 223) == 0x00060818U);
    assert(screen.pixel(20, 20) != 0x00060818U);

    const auto start_x = shell.marker_x();
    const auto start_y = shell.marker_y();
    shell.update(InputState{
        button_mask(Button::Right) | button_mask(Button::Down), 0});
    assert(shell.marker_x() == start_x + 2);
    assert(shell.marker_y() == start_y + 2);

    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    shell.render(screen);
    assert(shell.frame_number() == 2);
    assert(screen.presented_frames() == 2);

    shell.update(InputState{
        button_mask(Button::Start), button_mask(Button::Start)});
    assert(shell.marker_x() == kScreenWidth / 2);
    assert(shell.marker_y() == kScreenHeight / 2);

    const auto rom = read_rom();
    NativePortShell rom_shell("SHADOWRUN", rom);
    assert(rom_shell.has_rom_assets());
    assert(rom_shell.has_rom_field_map());
    assert(rom_shell.scene() == NativeScene::Title);
    rom_shell.render(screen);
    assert(screen.presented_frames() == 3);

    // Start is an edge-triggered title transition. Holding it without a
    // pressed edge must not repeatedly re-enter or reset the field scene.
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::Field);
    const auto field_start_x = rom_shell.marker_x();
    rom_shell.update(InputState{button_mask(Button::Start), 0});
    assert(rom_shell.scene() == NativeScene::Field);
    assert(rom_shell.marker_x() == field_start_x);

    rom_shell.update(InputState{
        button_mask(Button::Right) | button_mask(Button::Down), 0});
    assert(rom_shell.marker_x() == field_start_x + 2);
    assert(rom_shell.marker_y() == kScreenHeight / 2 + 2);
    rom_shell.render(screen);
    const auto field_before_scroll = screen.pixels();
    const auto initial_camera_x = rom_shell.field_camera_x();
    for (int frame = 0; frame < 100; ++frame) {
        rom_shell.update(InputState{button_mask(Button::Right), 0});
    }
    assert(rom_shell.marker_x() == kScreenWidth - 16);
    assert(rom_shell.field_world_x() > initial_camera_x);
    assert(rom_shell.field_camera_x() > initial_camera_x);
    rom_shell.render(screen);
    bool background_scrolled = false;
    for (std::int32_t y = 36; y < 204 && !background_scrolled; ++y) {
        for (std::int32_t x = 0; x < 140; ++x) {
            if (screen.pixel(x, y) !=
                field_before_scroll[static_cast<std::size_t>(y) *
                                     screen.width() + x]) {
                background_scrolled = true;
                break;
            }
        }
    }
    assert(background_scrolled);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    rom_shell.render(screen);
    assert(rom_shell.frame_number() == 104);
    assert(screen.presented_frames() == 6);
    return 0;
}
