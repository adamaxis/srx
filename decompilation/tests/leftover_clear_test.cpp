#include "shadowrun/graphics.hpp"
#include "shadowrun/native_port.hpp"

#include <cassert>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using shadowrun::native::Button;
using shadowrun::native::InputState;
using shadowrun::native::NativePortShell;
using shadowrun::native::NativeScene;
using shadowrun::native::SoftwareScreen;
using shadowrun::native::button_mask;

constexpr const char* kAssetRoot = SHADOWRUN_ASSET_ROOT;

void press(NativePortShell& shell, Button button) {
    const auto mask = button_mask(button);
    shell.update(InputState{mask, mask});
    shell.update(InputState{});
}

void enter_start_options(NativePortShell& shell) {
    shell.finish_boot();
    press(shell, Button::Start);
    for (int frame = 0; frame < 48 && shell.scene() == NativeScene::Title;
         ++frame) {
        shell.update(InputState{});
    }
    assert(shell.scene() == NativeScene::StartOptions);
}

SoftwareScreen render(NativePortShell& shell) {
    SoftwareScreen screen;
    shell.render(screen);
    return screen;
}

void require_same_pixels(const SoftwareScreen& left, const SoftwareScreen& right,
                         const char* label) {
    assert(left.pixels().size() == right.pixels().size());
    std::size_t mismatches = 0;
    for (std::size_t index = 0; index < left.pixels().size(); ++index) {
        if (left.pixels()[index] != right.pixels()[index]) {
            ++mismatches;
        }
    }
    if (mismatches != 0U) {
        throw std::runtime_error(std::string(label) + " leftover pixels: " +
                                 std::to_string(mismatches));
    }
}

void test_save_load_does_not_stain_start_options(NativePortShell& shell,
                                                 const std::filesystem::path&
                                                     artifact_dir) {
    enter_start_options(shell);
    const auto fresh = render(shell);
    shadowrun::graphics::write_bmp32(
        (artifact_dir / "start-options-fresh.bmp").string(),
        shadowrun::graphics::software_screen_image(fresh));

    press(shell, Button::Down);
    press(shell, Button::A);
    assert(shell.scene() == NativeScene::SaveLoad);
    press(shell, Button::A);
    assert(shell.save_load_message_visible());
    const auto dirty = render(shell);
    shadowrun::graphics::write_bmp32(
        (artifact_dir / "save-load-empty.bmp").string(),
        shadowrun::graphics::software_screen_image(dirty));

    press(shell, Button::B);
    assert(shell.scene() == NativeScene::StartOptions);
    press(shell, Button::Up);
    assert(shell.start_options_selection() == 0U);
    const auto returned = render(shell);
    shadowrun::graphics::write_bmp32(
        (artifact_dir / "start-options-after-saveload.bmp").string(),
        shadowrun::graphics::software_screen_image(returned));
    require_same_pixels(fresh, returned,
                        "Start Options after Save/Load empty-slot message");
}

void test_field_menu_does_not_stain_field(NativePortShell& shell,
                                          const std::filesystem::path&
                                              artifact_dir) {
    press(shell, Button::A);
    assert(shell.scene() == NativeScene::ArchetypeMenu);
    press(shell, Button::A);
    assert(shell.scene() == NativeScene::Intro);
    press(shell, Button::Start);
    assert(shell.scene() == NativeScene::Field);
    const auto field = render(shell);
    shadowrun::graphics::write_bmp32(
        (artifact_dir / "field.bmp").string(),
        shadowrun::graphics::software_screen_image(field));

    press(shell, Button::Start);
    assert(shell.scene() == NativeScene::FieldMenu);
    const auto menu = render(shell);
    shadowrun::graphics::write_bmp32(
        (artifact_dir / "field-menu.bmp").string(),
        shadowrun::graphics::software_screen_image(menu));

    press(shell, Button::Start);
    assert(shell.scene() == NativeScene::Field);
    const auto returned = render(shell);
    shadowrun::graphics::write_bmp32(
        (artifact_dir / "field-after-menu.bmp").string(),
        shadowrun::graphics::software_screen_image(returned));
    require_same_pixels(field, returned, "Field after command menu");
}

} // namespace

int main() {
    const std::filesystem::path asset_root(kAssetRoot);
    if (!std::filesystem::exists(asset_root / "native" / "state.bin")) {
        throw std::runtime_error("missing direct asset state: " +
                                 (asset_root / "native" / "state.bin").string());
    }
    NativePortShell shell(
        "SHADOWRUN",
        shadowrun::graphics::read_native_asset_directory(asset_root.string()));
    const auto artifact_dir =
        std::filesystem::path(SHADOWRUN_VISUAL_ARTIFACT_DIR) / "leftover-clear";
    std::filesystem::create_directories(artifact_dir);
    test_save_load_does_not_stain_start_options(shell, artifact_dir);
    test_field_menu_does_not_stain_field(shell, artifact_dir);
    return 0;
}
