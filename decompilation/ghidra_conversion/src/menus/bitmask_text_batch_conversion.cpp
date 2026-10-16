#include "shadowrun_ghidra/menus/bitmask_text_batch_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void append_record(BitmaskTextBatchState& state,
                   const BitmaskTextSourceEntry& source) {
    state.records.push_back(
        {source.alias, source.encoded_text, source.layout_token});
    state.last_record_index = static_cast<std::uint8_t>(
        state.last_record_index + 1U);
    state.selected_record_count = static_cast<std::uint8_t>(
        state.selected_record_count + 1U);
}

} // namespace

BitmaskTextBatchResult render_bitmask_text_batch(
    BitmaskTextBatchState& state,
    const BitmaskTextBatchInput& input,
    const BitmaskTextBatchCallbacks& callbacks) {
    BitmaskTextBatchResult result;

    if (!input.continuation_page) {
        state.records.clear();
        state.selected_record_count = 0;
        state.last_record_index = 0xFFU;

        for (std::size_t index = 0; index < input.source_entry_count; ++index) {
            if ((input.selection_mask & (std::uint32_t{1} <<
                                         (index & 31U))) == 0U) {
                continue;
            }

            if (state.selected_record_count == 7U) {
                append_record(state, input.special_entry);
                result.special_entry_inserted = true;
            }

            if (index < input.source_entries.size()) {
                append_record(state, input.source_entries[index]);
            } else {
                // A source extraction with fewer entries is malformed. Keep
                // the selected slot count and render order deterministic with
                // an empty host record rather than indexing arbitrary memory.
                append_record(state, {});
            }
        }

        if (state.selected_record_count == 9U) {
            if (state.records.size() >= 9U) {
                // Source F5F4 copies record 8 over record 7 when exactly nine
                // records were selected, preserving the visible first page.
                state.records[7] = state.records[8];
            }
            result.nine_entry_duplicate_applied = true;
        } else if (state.selected_record_count > 9U) {
            state.records.push_back({input.overflow_entry.alias,
                                     input.overflow_entry.encoded_text,
                                     input.overflow_entry.layout_token});
            result.overflow_entry_appended = true;
        }
    }

    result.records_built = state.records.size();
    if (state.records.empty()) {
        return result;
    }

    std::size_t first_record = 0;
    std::size_t render_count =
        static_cast<std::size_t>(state.last_record_index) + 1U;
    if (!input.continuation_page) {
        if (render_count > 8U) {
            render_count = 8U;
        }
    } else {
        if (state.last_record_index < 7U) {
            return result;
        }
        first_record = 8U;
        render_count = static_cast<std::size_t>(state.last_record_index) - 7U +
                       1U;
    }

    for (std::size_t index = 0; index < render_count; ++index) {
        const auto record_index = first_record + index;
        if (record_index >= state.records.size()) {
            break;
        }
        if (callbacks.render_text) {
            callbacks.render_text(state.records[record_index],
                                  static_cast<std::uint32_t>(index * 0x100U));
        }
        ++result.records_rendered;
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
