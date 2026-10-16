#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <filesystem>
#include <fstream>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto root = std::filesystem::temp_directory_path() /
                      "shadowrun_conversion_asset_catalog_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    std::filesystem::create_directories(root / "boot", error);
    if (error) {
        return 1;
    }
    {
        std::ofstream asset(root / "boot" / "snow.bmp",
                            std::ios::binary | std::ios::trunc);
        asset << "asset";
    }
    {
        std::ofstream manifest(root / "resources.tsv",
                               std::ios::binary | std::ios::trunc);
        manifest << "alias\tpath\tkind\torigin_x\torigin_y\n"
                 << "boot.snow\tboot/snow.bmp\timage\t0\t0\n";
    }

    const auto loaded = load_conversion_asset_catalog(root);
    const auto verified = verify_conversion_asset_set(
        loaded.catalog, {"boot.snow", "missing.alias"});
    const auto* snow = loaded.catalog.find("boot.snow");
    const bool correct = loaded.complete() && snow != nullptr &&
                         snow->byte_size == 5U &&
                         verified.missing_aliases.size() == 1U &&
                         verified.missing_aliases.front() == "missing.alias" &&
                         verified.total_bytes == 5U;
    std::filesystem::remove_all(root, error);
    return correct ? 0 : 2;
}
