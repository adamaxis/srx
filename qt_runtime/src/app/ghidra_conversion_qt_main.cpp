#include "ghidra_conversion_qt_window.hpp"

int main(int argc, char** argv) {
    try {
        std::optional<std::filesystem::path> explicit_assets;
        for (int index = 1; index < argc; ++index) {
            const std::string argument(argv[index]);
            if (argument == "--help") {
                std::cout << "usage: shadowrun_ghidra_conversion_qt [--assets <asset-directory>]\n";
                return 0;
            }
            if (argument == "--assets" && index + 1 < argc) {
                explicit_assets = std::filesystem::path(argv[++index]);
                continue;
            }
            throw std::invalid_argument(
                "unsupported game argument: " + argument);
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
            if (std::string_view(argv[qt_argument_index]) == "--assets") {
                qt_argument_index += 2;
                continue;
            }
            qt_arguments.push_back(argv[qt_argument_index++]);
        }
        int qt_argument_count = static_cast<int>(qt_arguments.size());
        qt_arguments.push_back(nullptr);
        QApplication application(qt_argument_count, qt_arguments.data());
        GhidraConversionWindow window(asset_root);
        window.show();
        return application.exec();
    } catch (const std::exception& error) {
        std::cerr << "shadowrun_ghidra_conversion_qt: " << error.what() << '\n';
        return 1;
    }
}
