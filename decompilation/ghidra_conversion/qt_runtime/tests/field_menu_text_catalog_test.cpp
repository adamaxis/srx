#include "field_menu_text_catalog.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <iostream>

int main() {
    const auto loaded = shadowrun::ghidra_conversion::load_conversion_asset_catalog(
        SHADOWRUN_GHIDRA_ASSET_ROOT);
    if (!loaded.complete()) {
        std::cerr << "direct asset catalog is incomplete\n";
        return 1;
    }
    const auto text =
        shadowrun::ghidra_conversion::qt_runtime::load_field_menu_text_catalog(
            loaded.catalog);
    if (!text.complete()) {
        std::cerr << "field menu text catalog is incomplete\n";
        return 2;
    }
    using namespace shadowrun::ghidra_conversion::qt_runtime;
    if (text.catalog.command_labels[0] != "Inventory" ||
        text.catalog.command_labels[6] != "Pocket Secretary" ||
        text.catalog.archetype_names[2] != "Gator" ||
        text.catalog.archetype_names[3] != "Shaman" ||
        text.catalog.runner_names[0] != "Joshua" ||
        !text.catalog.runner_names[1].empty() ||
        !text.catalog.item_categories[15].empty()) {
        std::cerr << "field menu text catalog does not match source labels\n";
        return 3;
    }
    return 0;
}
