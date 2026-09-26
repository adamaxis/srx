#include "shadowrun_ghidra/field/field_action_grid_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<FieldActionGridLayout> field_action_grid_layout(
    const FieldActionGridVariant variant) {
    switch (variant) {
    case FieldActionGridVariant::variant_a:
        // ROM 0x57C3A: D4 = 0xC604, D1 = 2, so three rows.
        return FieldActionGridLayout{0xC604U, 3U};
    case FieldActionGridVariant::variant_b:
        // ROM 0x57C46: D4 = 0xC184, D1 = 0xB, so twelve rows.
        return FieldActionGridLayout{0xC184U, 12U};
    case FieldActionGridVariant::variant_c:
        // ROM 0x57C52: D4 = 0xC904, D1 = 1, so two rows.
        return FieldActionGridLayout{0xC904U, 2U};
    case FieldActionGridVariant::detail_panel:
        // ROM 0x57C5E: D4 = 0xC484, D1 = 5, so six rows.
        return FieldActionGridLayout{0xC484U, 6U};
    }
    return std::nullopt;
}

std::optional<std::size_t> write_field_action_detail_grid(
    const FieldActionGridVariant variant,
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks) {
    const auto layout = field_action_grid_layout(variant);
    if (!layout.has_value() || source_row.size() < layout->words_per_row) {
        return std::nullopt;
    }

    // ROM 0x57C66 loads A1 with 0xC1320, D2 with zero, and enters 0x11C0.
    // write_repeated_vdp_grid uses host row/height counts rather than the
    // source DBF register values, while preserving the 0x80-byte row stride.
    write_repeated_vdp_grid(layout->destination_address, layout->words_per_row,
                            layout->rows, 0U, source_row, callbacks);
    return layout->words_per_row * layout->rows;
}

std::optional<std::size_t> write_field_action_grid_variant_a(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks) {
    return write_field_action_detail_grid(FieldActionGridVariant::variant_a,
                                           source_row, callbacks);
}

std::optional<std::size_t> write_field_action_grid_variant_b(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks) {
    return write_field_action_detail_grid(FieldActionGridVariant::variant_b,
                                           source_row, callbacks);
}

std::optional<std::size_t> write_field_action_grid_variant_c(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks) {
    return write_field_action_detail_grid(FieldActionGridVariant::variant_c,
                                           source_row, callbacks);
}

std::optional<std::size_t> refresh_field_action_detail_panel(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks) {
    return write_field_action_detail_grid(FieldActionGridVariant::detail_panel,
                                           source_row, callbacks);
}

} // namespace shadowrun::ghidra_conversion
