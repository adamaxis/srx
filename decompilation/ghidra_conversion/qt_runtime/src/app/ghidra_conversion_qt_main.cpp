#include "ghidra_conversion_qt_window.hpp"

#include <QCoreApplication>

int main(int argc, char** argv) {
    try {
        std::optional<std::filesystem::path> explicit_assets;
        std::optional<std::filesystem::path> frame_dump;
        std::optional<std::uint16_t> debug_port;
        debug::InputLogConfig input_log;
        bool frame_debug_controls = false;
        bool headless_replay = false;
        for (int index = 1; index < argc; ++index) {
            const std::string argument(argv[index]);
            if (argument == "--help") {
                std::cout << "usage: shadowrun_ghidra_conversion_qt [--assets <asset-directory>] "
                             "[--debug-frames] [--record-input <file> | --replay-input <file>]\n"
                             "                             [--headless-replay] [--dump-frame <file>] [--debug-port <port>]\n"
                             "  --debug-frames  Enable Pause, . (step), and Shift+. (step 60) controls.\n"
                             "  --record-input  Record accepted keyboard input for replay from a fresh launch.\n"
                             "  --replay-input  Replay a recorded keyboard session from a fresh launch.\n"
                             "  --headless-replay Run the replay to completion without a window or audio output.\n"
                             "  --dump-frame    Save the final replay frame as an image.\n"
                             "  --debug-port    Start the paused, headless local debugger on 127.0.0.1.\n";
                return 0;
            }
            if (argument == "--assets" && index + 1 < argc) {
                explicit_assets = std::filesystem::path(argv[++index]);
                continue;
            }
            if (argument == "--debug-frames") {
                frame_debug_controls = true;
                continue;
            }
            if (argument == "--headless-replay") {
                headless_replay = true;
                continue;
            }
            if (argument == "--dump-frame") {
                if (index + 1 >= argc) {
                    throw std::invalid_argument("--dump-frame requires a file path");
                }
                frame_dump = std::filesystem::path(argv[++index]);
                continue;
            }
            if (argument == "--debug-port") {
                if (index + 1 >= argc) {
                    throw std::invalid_argument("--debug-port requires a port number");
                }
                const std::string value(argv[++index]);
                std::size_t consumed = 0U;
                const auto port = std::stoul(value, &consumed, 10);
                if (consumed != value.size() || port == 0U || port > 65535U) {
                    throw std::invalid_argument(
                        "--debug-port must be an integer from 1 to 65535");
                }
                debug_port = static_cast<std::uint16_t>(port);
                continue;
            }
            if (argument == "--record-input" || argument == "--replay-input") {
                if (index + 1 >= argc) {
                    throw std::invalid_argument(argument + " requires a file path");
                }
                if (input_log.mode != debug::InputLogConfig::Mode::disabled) {
                    throw std::invalid_argument(
                        "--record-input and --replay-input are mutually exclusive");
                }
                input_log.mode = argument == "--record-input"
                    ? debug::InputLogConfig::Mode::record
                    : debug::InputLogConfig::Mode::replay;
                input_log.path = argv[++index];
                continue;
            }
            throw std::invalid_argument(
                "unsupported game argument: " + argument);
        }
        if (headless_replay &&
            input_log.mode != debug::InputLogConfig::Mode::replay) {
            throw std::invalid_argument(
                "--headless-replay requires --replay-input <file>");
        }
        if (frame_dump.has_value() && !headless_replay) {
            throw std::invalid_argument(
                "--dump-frame is only available with --headless-replay");
        }
        if (debug_port.has_value() &&
            input_log.mode == debug::InputLogConfig::Mode::replay) {
            throw std::invalid_argument(
                "--debug-port cannot be combined with --replay-input");
        }
        if (debug_port.has_value() && headless_replay) {
            throw std::invalid_argument(
                "--debug-port cannot be combined with --headless-replay");
        }

        const auto executable = std::filesystem::absolute(argv[0]);
        const auto executable_directory = executable.parent_path();
        std::vector<std::filesystem::path> candidates;
        if (explicit_assets.has_value()) {
            candidates.push_back(*explicit_assets);
        }
        candidates.insert(candidates.end(),
                          {"assets", "../assets", "../../assets"});
        for (auto directory = executable_directory; !directory.empty();) {
            candidates.push_back(directory / "assets");
            const auto parent = directory.parent_path();
            if (parent == directory) {
                break;
            }
            directory = parent;
        }

        std::filesystem::path asset_root;
        for (const auto& candidate : candidates) {
            if (std::filesystem::is_regular_file(candidate / "resources.tsv")) {
                asset_root = candidate;
                break;
            }
        }
        if (asset_root.empty()) {
            throw std::runtime_error(
                "could not locate assets/resources.tsv; pass --assets <asset-directory>");
        }

        std::vector<char*> qt_arguments{argv[0]};
        int qt_argument_index = 1;
        while (qt_argument_index < argc) {
            const std::string_view argument(argv[qt_argument_index]);
            if (argument == "--assets" || argument == "--record-input" ||
                argument == "--replay-input") {
                qt_argument_index += 2;
                continue;
            }
            if (argument == "--debug-frames" ||
                argument == "--headless-replay") {
                ++qt_argument_index;
                continue;
            }
            if (argument == "--dump-frame") {
                qt_argument_index += 2;
                continue;
            }
            if (argument == "--debug-port") {
                qt_argument_index += 2;
                continue;
            }
            qt_arguments.push_back(argv[qt_argument_index++]);
        }
        int qt_argument_count = static_cast<int>(qt_arguments.size());
        qt_arguments.push_back(nullptr);
        QCoreApplication::setOrganizationName(QStringLiteral("Shadowrun"));
        QCoreApplication::setApplicationName(
            QStringLiteral("Shadowrun Ghidra Conversion"));
        const bool headless_mode = headless_replay || debug_port.has_value();
        if (headless_mode) {
            const auto offscreen_plugin =
                executable_directory / "platforms" / "qoffscreen.dll";
            if (!std::filesystem::is_regular_file(offscreen_plugin)) {
                throw std::runtime_error(
                    "headless mode requires the deployed Qt platforms/qoffscreen.dll plugin");
            }
            // Qt reads its platform plugin setting during application
            // construction. No widget is shown in this path.
            qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
        }
        QApplication application(qt_argument_count, qt_arguments.data());
        GhidraConversionWindow window(asset_root, std::move(input_log),
                                      frame_debug_controls ||
                                          debug_port.has_value(),
                                      headless_mode, debug_port);
        if (headless_replay) {
            std::string error;
            if (!window.run_headless_replay(
                    frame_dump.value_or(std::filesystem::path{}), &error)) {
                std::cerr << "headless replay: " << error << '\n';
                return 2;
            }
            std::cout << "headless replay completed "
                      << window.source_frame_count_for_debug() << " source frames";
            if (frame_dump.has_value()) {
                std::cout << "; final frame saved to "
                          << frame_dump->string();
            }
            std::cout << '\n';
            return 0;
        }
        if (debug_port.has_value()) {
            std::cout << "local debugger listening on 127.0.0.1:"
                      << *debug_port << " (paused)\n";
            return application.exec();
        }
        window.show();
        return application.exec();
    } catch (const std::exception& error) {
        std::cerr << "shadowrun_ghidra_conversion_qt: " << error.what() << '\n';
        return 1;
    }
}
