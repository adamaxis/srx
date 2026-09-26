#include "shadowrun_ghidra/title_intro/title_intro_transfer_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroDisplayClearRequest clear_request{};
    clear_title_intro_display_buffer(
        [&clear_request](const auto& request) { clear_request = request; });
    if (clear_request.transfer_command != 0x3000U ||
        clear_request.packed_fill_word != 0x07FF07FFU ||
        clear_request.long_write_count != 0x0400U) {
        return 1;
    }

    std::vector<TitleIntroFillRowRequest> rows;
    fill_title_intro_word_block(
        {0x0786U, 20U, 13U, 0x0080U, 0x07FFU},
        [&rows](const auto& request) { rows.push_back(request); });
    if (rows.size() != 13U || rows.front().destination_offset != 0x0786U ||
        rows.back().destination_offset != 0x0D86U ||
        rows.front().word_count != 20U || rows.back().row_index != 12U ||
        rows.front().fill_word != 0x07FFU) {
        return 2;
    }
    return 0;
}
