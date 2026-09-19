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
    press(shell, Button::Start);
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
           "[--frames 0,90,128,500]\n"
        << "scenes: boot, title, start-options, save-load-empty, archetype, "
           "intro, field\n";
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
        std::vector<std::uint64_t> frames{0, 90, 128, 500};
        for (int index = 3; index < argc; ++index) {
            const std::string arg(argv[index]);
            if (arg == "--scene" && index + 1 < argc) {
                scene = argv[++index];
            } else if (arg == "--frames" && index + 1 < argc) {
                frames = parse_frames(argv[++index]);
            } else {
                throw std::invalid_argument("unknown argument: " + arg);
            }
        }
        if (scene.empty()) {
            usage();
            return 1;
        }
        std::filesystem::create_directories(out);
        NativePortShell shell(
            "SHADOWRUN",
            shadowrun::graphics::read_native_asset_pack(pack.string()));

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
            return 0;
        }
        if (scene == "intro") {
            enter_start_options(shell);
            press(shell, Button::A);
            press(shell, Button::A);
            if (shell.scene() != NativeScene::Intro) {
                throw std::runtime_error("failed to enter Intro");
            }
            std::uint64_t current = 0;
            const auto intro_frames =
                frames.size() == 4 && frames[0] == 0 && frames[1] == 90
                    ? std::vector<std::uint64_t>{0, 0xA0, 0x290, 0x3A4, 0x6EC}
                    : frames;
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
            press(shell, Button::A);
            press(shell, Button::Start);
            if (shell.scene() != NativeScene::Field) {
                throw std::runtime_error("failed to enter Field");
            }
            write_frame(shell, out / "field.bmp");
            return 0;
        }
        throw std::invalid_argument("unknown scene: " + scene);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
