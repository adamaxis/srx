#include "shadowrun/native_port.hpp"

#include <cassert>

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
    return 0;
}
