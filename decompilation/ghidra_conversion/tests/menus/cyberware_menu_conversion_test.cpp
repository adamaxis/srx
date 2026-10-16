#include "shadowrun_ghidra/menus/cyberware_menu_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto empty = build_cyberware_menu(0U);
    if (!expect(empty.shows_no_cyberware_message &&
                empty.installed_row_indices.empty())) {
        return 1;
    }

    const auto installed = build_cyberware_menu(
        static_cast<std::uint16_t>((1U << 0U) | (1U << 5U) |
                                   (1U << 14U) | (1U << 15U)));
    if (!expect(!installed.shows_no_cyberware_message &&
                installed.installed_row_indices ==
                    std::vector<std::uint8_t>{0U, 5U, 14U})) {
        return 2;
    }

    const auto high_only = build_cyberware_menu(1U << 15U);
    if (!expect(!high_only.shows_no_cyberware_message &&
                high_only.installed_row_indices.empty())) {
        return 3;
    }
    const auto empty_text = build_cyberware_text_plans(0U);
    const auto installed_text = build_cyberware_text_plans(
        static_cast<std::uint16_t>((1U << 0U) | (1U << 2U)));
    if (!expect(empty_text.size() == 2U &&
                empty_text[0].source == kCyberwareTitleSource &&
                empty_text[0].destination == kCyberwareTitleDestination &&
                empty_text[1].source == kCyberwareEmptySource &&
                empty_text[1].destination == kCyberwareEmptyDestination &&
                installed_text.size() == 3U &&
                installed_text[1].source == kCyberwareEntrySources[0] &&
                installed_text[1].destination ==
                    kCyberwareEntryDestinationBase &&
                installed_text[2].source == kCyberwareEntrySources[2] &&
                installed_text[2].destination ==
                    kCyberwareEntryDestinationBase +
                        kCyberwareEntryDestinationStride)) {
        return 4;
    }
    return 0;
}
