#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct BitmaskTextSourceEntry {
    std::string_view alias;
    std::vector<std::uint8_t> encoded_text;
    std::uint32_t layout_token = 0;
};

struct BitmaskTextBatchRecord {
    std::string_view alias;
    std::vector<std::uint8_t> encoded_text;
    std::uint32_t layout_token = 0;
};

struct BitmaskTextBatchInput {
    std::uint32_t selection_mask = 0; // D0
    std::size_t source_entry_count = 0; // DBF-expanded D7 count
    std::vector<BitmaskTextSourceEntry> source_entries; // A1/A2 pairs
    BitmaskTextSourceEntry special_entry{
        "menu.field.action.special_separator", {}, 0}; // 0xC661D
    BitmaskTextSourceEntry overflow_entry{
        "menu.field.action.overflow", {}, 0}; // 0xC662B
    bool continuation_page = false; // FFF112
};

struct BitmaskTextBatchState {
    std::vector<BitmaskTextBatchRecord> records; // FFEF16/FFEF56
    std::uint8_t last_record_index = 0xFFU;     // FFF0B9
    std::uint8_t selected_record_count = 0;     // FFF0CF
};

struct BitmaskTextBatchCallbacks {
    // 0xC0DA consumes the encoded text and the destination tile base selected
    // by the caller's A6 plus one 0x100 step per rendered record.
    std::function<void(const BitmaskTextBatchRecord& record,
                       std::uint32_t destination_offset)>
        render_text;
};

struct BitmaskTextBatchResult {
    std::size_t records_built = 0;
    std::size_t records_rendered = 0;
    bool special_entry_inserted = false;
    bool nine_entry_duplicate_applied = false;
    bool overflow_entry_appended = false;
};

// ROM 0xF5F4: build the selected menu text list from a display bitmask and
// render either its first eight records or the continuation records. Source
// strings/layout values are decoded host assets; only the source list rules
// and render order are retained here.
BitmaskTextBatchResult render_bitmask_text_batch(
    BitmaskTextBatchState& state,
    const BitmaskTextBatchInput& input,
    const BitmaskTextBatchCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
