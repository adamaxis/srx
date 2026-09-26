#include "shadowrun_ghidra/field/field_action_grid_conversion.hpp"

#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

namespace {

bool expect_grid(const FieldActionGridVariant variant,
                 const std::uint16_t destination,
                 const std::size_t rows) {
    const auto layout = field_action_grid_layout(variant);
    return layout.has_value() && layout->destination_address == destination &&
           layout->rows == rows && layout->words_per_row == 31U &&
           layout->source_table_address == 0xC1320U;
}

} // namespace

int main() {
    if (!expect_grid(FieldActionGridVariant::variant_a, 0xC604U, 3U) ||
        !expect_grid(FieldActionGridVariant::variant_b, 0xC184U, 12U) ||
        !expect_grid(FieldActionGridVariant::variant_c, 0xC904U, 2U) ||
        !expect_grid(FieldActionGridVariant::detail_panel, 0xC484U, 6U)) {
        return 1;
    }

    std::vector<std::uint16_t> source_row;
    for (std::uint16_t word = 0; word < 31U; ++word) {
        source_row.push_back(static_cast<std::uint16_t>(0x100U + word));
    }

    std::vector<VdpTransferCommand> commands;
    std::vector<std::uint16_t> words;
    const WordTransferCallbacks callbacks{
        [&](const VdpTransferCommand& command) { commands.push_back(command); },
        [&](const std::uint16_t word) { words.push_back(word); },
        {},
    };

    const auto written = refresh_field_action_detail_panel(source_row, callbacks);
    if (!written.has_value() || *written != 186U || commands.size() != 6U ||
        words.size() != 186U ||
        commands.front().low_word != make_vdp_transfer_command(0xC484U).low_word ||
        commands.back().low_word != make_vdp_transfer_command(0xC704U).low_word ||
        words[0] != 0x100U || words[31] != 0x100U || words.back() != 0x11EU) {
        return 2;
    }

    commands.clear();
    words.clear();
    const auto variant_b = write_field_action_grid_variant_b(source_row, callbacks);
    if (!variant_b.has_value() || *variant_b != 372U || commands.size() != 12U ||
        words.size() != 372U ||
        commands.back().low_word != make_vdp_transfer_command(0xC704U).low_word) {
        return 3;
    }

    commands.clear();
    words.clear();
    if (write_field_action_grid_variant_a({1U, 2U}, callbacks).has_value() ||
        !commands.empty() || !words.empty()) {
        return 4;
    }
    return 0;
}
