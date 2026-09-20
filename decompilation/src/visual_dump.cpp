#include "shadowrun/graphics.hpp"
#include "shadowrun/native_port.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
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

void press(NativePortShell& shell, Button button) {
    const auto mask = button_mask(button);
    shell.update(InputState{mask, mask});
    shell.update(InputState{});
}

void write_frame(const NativePortShell& shell, const std::filesystem::path& path) {
    SoftwareScreen screen;
    shell.render(screen);
    shadowrun::graphics::write_bmp32(
        path.string(), shadowrun::graphics::software_screen_image(screen));
}

void enter_start_options(NativePortShell& shell) {
    shell.finish_boot();
    press(shell, Button::Start);
    for (int frame = 0; frame < 48 &&
         shell.scene() == NativeScene::Title; ++frame) {
        shell.update(InputState{});
    }
    if (shell.scene() != NativeScene::StartOptions) {
        throw std::runtime_error("failed to enter Start Options");
    }
}

std::vector<std::uint64_t> parse_frames(const std::string& value) {
    std::vector<std::uint64_t> frames;
    std::stringstream stream(value);
    std::string token;
    while (std::getline(stream, token, ',')) {
        if (token.empty()) {
            continue;
        }
        frames.push_back(std::stoull(token, nullptr, 0));
    }
    if (frames.empty()) {
        throw std::invalid_argument("empty --frames list");
    }
    return frames;
}

void dump_title(NativePortShell& shell, const std::filesystem::path& out,
                const std::vector<std::uint64_t>& frames) {
    shell.finish_boot();
    std::uint64_t current = 0;
    for (const auto target : frames) {
        while (current < target) {
            shell.update(InputState{});
            ++current;
        }
        write_frame(shell, out / ("title-" + std::to_string(target) + ".bmp"));
    }
}

void usage() {
    std::cerr
        << "shadowrun_visual_dump <pack.srna> <out-dir> --scene <name> "
           "[--frames 0,90,128,500] [--archetype 0|1|2]\n"
        << "scenes: boot, title, start-options, save-load-empty, archetype, "
           "intro, intro-ambush-asset, cab-map, field, field-menu, "
           "pocket-secretary, clips, cyberware, attributes, magic, posture, "
           "inventory\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 5) {
            usage();
            return 1;
        }
        const std::filesystem::path pack(argv[1]);
        const std::filesystem::path out(argv[2]);
        std::string scene;
        std::size_t requested_archetype = 0;
        std::vector<std::uint64_t> frames{0, 90, 110, 160, 200, 500};
        auto frames_specified = false;
        for (int index = 3; index < argc; ++index) {
            const std::string arg(argv[index]);
            if (arg == "--scene" && index + 1 < argc) {
                scene = argv[++index];
            } else if (arg == "--frames" && index + 1 < argc) {
                frames = parse_frames(argv[++index]);
                frames_specified = true;
            } else if (arg == "--archetype" && index + 1 < argc) {
                requested_archetype =
                    std::stoull(argv[++index], nullptr, 0);
                if (requested_archetype >=
                    shadowrun::native::kNativeArchetypeChoiceCount) {
                    throw std::invalid_argument(
                        "--archetype must be 0, 1, or 2");
                }
            } else {
                throw std::invalid_argument("unknown argument: " + arg);
            }
        }
        if (scene.empty()) {
            usage();
            return 1;
        }
        std::filesystem::create_directories(out);
        auto assets = shadowrun::graphics::read_native_asset_pack(pack.string());
        NativePortShell shell("SHADOWRUN", assets);

        if (scene == "intro-ambush-asset") {
            for (const auto target : frames) {
                if (target >= assets.intro_ambush_frames.size()) {
                    throw std::out_of_range(
                        "intro ambush asset frame is outside the pack");
                }
                const auto& image = assets.intro_ambush_frames[target];
                shadowrun::graphics::write_bmp32(
                    (out / ("intro-ambush-asset-" +
                            std::to_string(target) + ".bmp")).string(),
                    {image.width, image.height, image.pixels});
            }
            return 0;
        }

        if (scene == "boot") {
            std::uint64_t current = 0;
            for (const auto target : frames) {
                while (current < target) {
                    shell.update(InputState{});
                    ++current;
                }
                write_frame(shell, out / ("boot-" + std::to_string(target) + ".bmp"));
            }
            return 0;
        }
        if (scene == "title") {
            dump_title(shell, out, frames);
            return 0;
        }
        if (scene == "start-options") {
            enter_start_options(shell);
            write_frame(shell, out / "start-options.bmp");
            return 0;
        }
        if (scene == "save-load-empty") {
            enter_start_options(shell);
            press(shell, Button::Down);
            press(shell, Button::A);
            press(shell, Button::A);
            if (shell.scene() != NativeScene::SaveLoad) {
                throw std::runtime_error("failed to enter Save/Load");
            }
            write_frame(shell, out / "save-load-empty.bmp");
            return 0;
        }
        if (scene == "archetype") {
            enter_start_options(shell);
            press(shell, Button::A);
            if (shell.scene() != NativeScene::ArchetypeMenu) {
                throw std::runtime_error("failed to enter Archetype");
            }
            write_frame(shell, out / "archetype.bmp");
            press(shell, Button::Down);
            write_frame(shell, out / "archetype-decker.bmp");
            press(shell, Button::Down);
            write_frame(shell, out / "archetype-gator.bmp");
            return 0;
        }
        if (scene == "intro") {
            enter_start_options(shell);
            press(shell, Button::A);
            for (std::size_t index = 0; index < requested_archetype; ++index) {
                press(shell, Button::Down);
            }
            press(shell, Button::A);
            if (shell.scene() != NativeScene::Intro) {
                throw std::runtime_error("failed to enter Intro");
            }
            std::uint64_t current = 0;
            const auto intro_frames =
                frames_specified
                    ? frames
                    : std::vector<std::uint64_t>{0, 0x3C, 0x168, 0x267, 0x394,
                                                 0x4F9, 0x60E, 0x956};
            for (const auto target : intro_frames) {
                while (current < target) {
                    shell.update(InputState{});
                    ++current;
                    if (shell.scene() != NativeScene::Intro) {
                        throw std::runtime_error(
                            "intro ended before dump frame " +
                            std::to_string(target));
                    }
                }
                write_frame(shell, out / ("intro-" + std::to_string(target) +
                                          ".bmp"));
            }
            return 0;
        }
        if (scene == "field") {
            enter_start_options(shell);
            press(shell, Button::A);
            for (std::size_t index = 0; index < requested_archetype; ++index) {
                press(shell, Button::Down);
            }
            press(shell, Button::A);
            press(shell, Button::Start);
            if (shell.scene() != NativeScene::Field) {
                throw std::runtime_error("failed to enter Field");
            }
            write_frame(shell, out / "field.bmp");
            return 0;
        }
        if (scene == "cab-map") {
            enter_start_options(shell);
            press(shell, Button::A);
            for (std::size_t index = 0; index < requested_archetype; ++index) {
                press(shell, Button::Down);
            }
            press(shell, Button::A);
            if (shell.scene() != NativeScene::Intro) {
                throw std::runtime_error("failed to enter Intro for cab map");
            }
            std::uint64_t current = 0;
            while (shell.scene() == NativeScene::Intro && current < 0x2000U) {
                shell.update(InputState{});
                ++current;
            }
            if (shell.scene() != NativeScene::CabMap) {
                throw std::runtime_error("intro did not reach cab map");
            }
            for (const auto target : frames) {
                while (current < target) {
                    shell.update(InputState{});
                    ++current;
                    if (shell.scene() != NativeScene::CabMap) {
                        break;
                    }
                }
                write_frame(shell, out / ("cab-map-" +
                                           std::to_string(target) + ".bmp"));
            }
            return 0;
        }
        if (scene == "field-menu" || scene == "pocket-secretary") {
            enter_start_options(shell);
            press(shell, Button::A);
            for (std::size_t index = 0; index < requested_archetype; ++index) {
                press(shell, Button::Down);
            }
            press(shell, Button::A);
            press(shell, Button::Start);
            press(shell, Button::Start);
            if (shell.scene() != NativeScene::FieldMenu) {
                throw std::runtime_error("failed to enter command menu");
            }
            write_frame(shell, out / "field-menu.bmp");
            if (scene == "pocket-secretary") {
                press(shell, Button::A);
                write_frame(shell, out / "pocket-secretary.bmp");
            }
            return 0;
        }
        if (scene == "clips" || scene == "cyberware" ||
            scene == "attributes" || scene == "magic" ||
            scene == "posture") {
            enter_start_options(shell);
            press(shell, Button::A);
            for (std::size_t index = 0; index < requested_archetype; ++index) {
                press(shell, Button::Down);
            }
            press(shell, Button::A);
            press(shell, Button::Start);
            press(shell, Button::Start);
            if (shell.scene() != NativeScene::FieldMenu) {
                throw std::runtime_error("failed to enter command menu");
            }
            const auto rights = scene == "clips"
                ? 3U : scene == "cyberware" ? 5U
                : scene == "attributes" ? 4U : 0U;
            for (std::size_t index = 0; index < rights; ++index) {
                press(shell, Button::Right);
            }
            if (scene == "magic") {
                press(shell, Button::Left);
            }
            if (scene == "posture") {
                press(shell, Button::Up);
            } else {
                press(shell, Button::A);
            }
            const auto expected_scene = scene == "clips"
                ? NativeScene::ClipsMenu
                : scene == "cyberware" ? NativeScene::CyberwareMenu
                : scene == "attributes" ? NativeScene::AttributesMenu
                : scene == "magic" ? NativeScene::MagicMenu
                                     : NativeScene::FieldMenu;
            if (shell.scene() != expected_scene) {
                throw std::runtime_error("failed to enter submenu");
            }
            if (scene == "posture" && !shell.field_menu_posture_active()) {
                throw std::runtime_error("failed to enter posture focus");
            }
            write_frame(shell, out / (scene + ".bmp"));
            return 0;
        }
        if (scene == "inventory") {
            enter_start_options(shell);
            press(shell, Button::A);
            for (std::size_t index = 0; index < requested_archetype; ++index) {
                press(shell, Button::Down);
            }
            press(shell, Button::A);
            press(shell, Button::Start);
            press(shell, Button::Start);
            if (shell.scene() != NativeScene::FieldMenu) {
                throw std::runtime_error("failed to enter command menu");
            }
            for (std::size_t index = 0; index < 6U; ++index) {
                press(shell, Button::Left);
            }
            if (shell.scene() != NativeScene::FieldMenu) {
                throw std::runtime_error("failed to select inventory");
            }
            write_frame(shell, out / "inventory-row0-left.bmp");
            press(shell, Button::Right);
            write_frame(shell, out / "inventory-row0-right.bmp");
            press(shell, Button::Down);
            write_frame(shell, out / "inventory-row1-right.bmp");
            press(shell, Button::Left);
            write_frame(shell, out / "inventory-row1-left.bmp");
            return 0;
        }
        throw std::invalid_argument("unknown scene: " + scene);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
