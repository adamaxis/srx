#include "shadowrun_ghidra/menus/bitmask_text_batch_conversion.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<BitmaskTextSourceEntry> entries;
    std::vector<std::string> aliases;
    aliases.reserve(13U);
    entries.reserve(13U);
    for (std::size_t index = 0; index < 13U; ++index) {
        aliases.push_back("menu.field.action." + std::to_string(index));
        entries.push_back({aliases.back(),
                           {static_cast<std::uint8_t>(index)},
                           static_cast<std::uint32_t>(0x100U + index)});
    }

    BitmaskTextBatchInput input;
    input.selection_mask = 0x03FFU; // ten selected entries
    input.source_entry_count = 13U; // source D7=0xC, DBF => thirteen scans
    input.source_entries = entries;
    input.special_entry = {"menu.field.action.special_separator", {0xEEU}, 0};
    input.overflow_entry = {"menu.field.action.overflow", {0xEFU}, 0};

    BitmaskTextBatchState state;
    std::vector<std::pair<std::string, std::uint32_t>> rendered;
    const auto first_page = render_bitmask_text_batch(
        state, input,
        {[&](const BitmaskTextBatchRecord& record,
             const std::uint32_t destination) {
             rendered.emplace_back(std::string(record.alias), destination);
         }});
    if (!expect(first_page.records_built == 12U) ||
        !expect(first_page.records_rendered == 8U) ||
        !expect(first_page.special_entry_inserted) ||
        !expect(first_page.overflow_entry_appended) ||
        !expect(!first_page.nine_entry_duplicate_applied) ||
        !expect(state.selected_record_count == 11U) ||
        !expect(state.last_record_index == 10U) ||
        !expect(rendered.size() == 8U) ||
        !expect(rendered[0].first == "menu.field.action.0") ||
        !expect(rendered[7].first == "menu.field.action.special_separator") ||
        !expect(rendered[7].second == 0x700U)) {
        return 1;
    }

    rendered.clear();
    input.continuation_page = true;
    const auto continuation = render_bitmask_text_batch(
        state, input,
        {[&](const BitmaskTextBatchRecord& record,
             const std::uint32_t destination) {
             rendered.emplace_back(std::string(record.alias), destination);
         }});
    if (!expect(continuation.records_built == 12U) ||
        !expect(continuation.records_rendered == 4U) ||
        !expect(rendered.size() == 4U) ||
        !expect(rendered[0].first == "menu.field.action.7") ||
        !expect(rendered[0].second == 0U) ||
        !expect(rendered[3].first == "menu.field.action.overflow") ||
        !expect(rendered[3].second == 0x300U)) {
        return 2;
    }

    BitmaskTextBatchState nine_state;
    BitmaskTextBatchInput nine_input;
    nine_input.selection_mask = 0x00FFU; // eight selected + inserted special
    nine_input.source_entry_count = 8U;
    nine_input.source_entries = entries;
    const auto nine = render_bitmask_text_batch(nine_state, nine_input, {});
    if (!expect(nine.nine_entry_duplicate_applied) ||
        !expect(nine.records_built == 9U) ||
        !expect(nine.records_rendered == 8U) ||
        !expect(nine_state.records[7].alias == nine_state.records[8].alias)) {
        return 3;
    }

    return 0;
}
