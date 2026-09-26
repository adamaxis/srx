#include "shadowrun/graphics.hpp"
#include "shadowrun/native_game.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace {

using shadowrun::native::Button;
using shadowrun::native::InputState;
using shadowrun::native::NativeGameController;
using shadowrun::native::NativeScene;
using shadowrun::native::SoftwareScreen;

void press(NativeGameController& game, Button button) {
    const auto mask = shadowrun::native::button_mask(button);
    game.update(InputState{mask, mask});
    game.update(InputState{});
}

std::size_t non_black_pixels(const SoftwareScreen& screen) {
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
    const auto asset_root = std::filesystem::path(SHADOWRUN_ASSET_ROOT);
    const auto assets = shadowrun::graphics::read_native_asset_directory(
        asset_root.string());
    NativeGameController game("SHADOWRUN", assets);

    assert(game.has_assets());
    assert(game.has_boot());
    assert(game.has_field_map());
    assert(game.scene() == NativeScene::Bootstrap);

    game.finish_boot();
    assert(game.scene() == NativeScene::Title);
    press(game, Button::Start);
    for (std::size_t frame = 0;
         frame < 64U && game.scene() == NativeScene::Title; ++frame) {
        game.update(InputState{});
    }
    assert(game.scene() == NativeScene::StartOptions);

    press(game, Button::A);
    assert(game.scene() == NativeScene::ArchetypeMenu);
    press(game, Button::A);
    assert(game.scene() == NativeScene::Intro);
    press(game, Button::Start);
    assert(game.scene() == NativeScene::Field);

    SoftwareScreen screen;
    game.render(screen);
    assert(non_black_pixels(screen) > 100U);
    return 0;
}
