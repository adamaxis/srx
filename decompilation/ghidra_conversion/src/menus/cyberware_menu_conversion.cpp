#include "shadowrun_ghidra/menus/cyberware_menu_conversion.hpp"

namespace shadowrun::ghidra_conversion {

CyberwareMenuModel build_cyberware_menu(const std::uint16_t cyberware_flags) {
    CyberwareMenuModel result{};
    if (cyberware_flags == 0U) {
        result.shows_no_cyberware_message = true;
        return result;
    }

    for (std::uint8_t bit = 0; bit < 15U; ++bit) {
        if ((cyberware_flags & (std::uint16_t{1U} << bit)) != 0U) {
            result.installed_row_indices.push_back(bit);
        }
    }
    return result;
}

std::vector<CyberwareTextPlan> build_cyberware_text_plans(
    const std::uint16_t cyberware_flags) {
    std::vector<CyberwareTextPlan> plans{
        {kCyberwareTitleSource, kCyberwareTitleDestination}};
    const auto model = build_cyberware_menu(cyberware_flags);
    if (model.shows_no_cyberware_message) {
        plans.push_back({kCyberwareEmptySource, kCyberwareEmptyDestination});
        return plans;
    }
    for (std::size_t index = 0; index < model.installed_row_indices.size();
         ++index) {
        const auto bit = model.installed_row_indices[index];
        plans.push_back(
            {kCyberwareEntrySources[bit],
             kCyberwareEntryDestinationBase +
                 static_cast<std::uint32_t>(index) *
                     kCyberwareEntryDestinationStride});
    }
    return plans;
}

} // namespace shadowrun::ghidra_conversion
