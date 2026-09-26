#pragma once

#include "shadowrun_ghidra/core/word_transfer_conversion.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class FieldActionGridVariant {
    variant_a,
    variant_b,
    variant_c,
    detail_panel,
};

struct FieldActionGridLayout {
    std::uint16_t destination_address = 0;
    std::size_t rows = 0;
    std::size_t words_per_row = 31U; // ROM 0x57C66: D7 = 0x1E
    std::uint32_t source_table_address = 0x000C1320U;
};

// ROM 0x57C3A/0x57C46/0x57C52/0x57C5E: load the four source-selected
// destination/row presets before entering the shared 0x57C66 writer.
std::optional<FieldActionGridLayout> field_action_grid_layout(
    FieldActionGridVariant variant);

// ROM 0x57C66: repeat the extracted 31-word source row for the selected
// number of DBF-derived rows. The host receives the same ordered transfer
// events through WordTransferCallbacks; no VDP or ROM address is required at
// runtime.
std::optional<std::size_t> write_field_action_detail_grid(
    FieldActionGridVariant variant,
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks);

// Named entry points preserve the four original ROM call sites while sharing
// one modern implementation and one resource contract.
std::optional<std::size_t> write_field_action_grid_variant_a(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks);

std::optional<std::size_t> write_field_action_grid_variant_b(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks);

std::optional<std::size_t> write_field_action_grid_variant_c(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks);

std::optional<std::size_t> refresh_field_action_detail_panel(
    const std::vector<std::uint16_t>& source_row,
    const WordTransferCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
