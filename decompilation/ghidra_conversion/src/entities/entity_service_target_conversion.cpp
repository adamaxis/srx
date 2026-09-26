#include "shadowrun_ghidra/entities/entity_service_target_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::size_t kNormalCandidateStart = 3U; // FF0400 relative to FF0100
constexpr std::size_t kCandidateCount = 8U;       // D7 = 7

bool is_special_mode(const std::uint8_t mode) {
    return mode == 0x0CU || mode == 0x04U || mode == 0x0BU;
}

std::optional<std::size_t> release_previous_target(
    EntityServiceTargetOwner& owner,
    std::vector<EntityServiceTargetRecord>& records) {
    const auto previous = owner.selected_target;
    if (previous.has_value() && *previous < records.size()) {
        records[*previous].selected = false;
    }
    owner.selected_target.reset();
    return previous;
}

std::optional<std::size_t> normal_candidate_start(
    const std::optional<std::size_t> previous_target,
    const EntityServiceTargetOwner& owner,
    const std::vector<EntityServiceTargetRecord>& records,
    const bool advance_source_cursor) {
    std::size_t start = kNormalCandidateStart;
    if (previous_target.has_value()) {
        // The ROM rejects an old target at FF0A00 or above before applying the
        // descriptor/cursor rules, then restarts at FF0400.
        if (*previous_target < 9U) {
            start = *previous_target;
            if (advance_source_cursor) {
                ++start;
            }
        }
    } else if (owner.linked_target.has_value()) {
        // +0xAC is already the candidate pointer; unlike +0xA2 it is not
        // advanced by the D1 cursor test.
        start = *owner.linked_target;
    }

    if (start >= records.size()) {
        return records.size() > kNormalCandidateStart
                   ? std::optional<std::size_t>(kNormalCandidateStart)
                   : std::nullopt;
    }
    if (records[start].descriptor_class < 3U) {
        start = kNormalCandidateStart;
    }
    if (start >= records.size()) {
        return std::nullopt;
    }
    return start;
}

std::optional<std::size_t> special_candidate_start(
    const std::optional<std::size_t> previous_target,
    const EntityServiceTargetOwner& owner,
    const std::vector<EntityServiceTargetRecord>& records,
    const bool advance_source_cursor) {
    std::optional<std::size_t> start = previous_target;
    if (!start.has_value()) {
        start = owner.owner_record;
    }
    if (!start.has_value() || *start >= records.size()) {
        return std::nullopt;
    }
    if (advance_source_cursor) {
        ++*start;
    }
    // The special path resets to FF0100 when the descriptor class is 3+.
    if (*start >= records.size() || records[*start].descriptor_class >= 3U) {
        *start = 0U;
    }
    return start;
}

} // namespace

EntityServiceTargetAction select_world_entity_service_target(
    EntityServiceTargetOwner& owner,
    std::vector<EntityServiceTargetRecord>& records,
    const bool advance_source_cursor,
    const EntityServiceTargetCallbacks& callbacks) {
    // ROM 0x1DC4C releases +0xA2 before selecting either branch. It leaves
    // the old record's +0xAC link untouched.
    const auto previous_target = release_previous_target(owner, records);
    const bool special_path = owner.special_marker && is_special_mode(owner.mode);
    const auto start = special_path
                           ? special_candidate_start(previous_target, owner, records,
                                                     advance_source_cursor)
                           : normal_candidate_start(previous_target, owner, records,
                                                    advance_source_cursor);
    if (!start.has_value()) {
        return EntityServiceTargetAction::no_available_target;
    }

    for (std::size_t offset = 0; offset < kCandidateCount; ++offset) {
        const auto index = *start + offset;
        if (index >= records.size()) {
            // The ROM continues to advance its source address, but the typed
            // table intentionally represents absent records as unavailable;
            // it never wraps around to FF0100.
            continue;
        }
        auto& record = records[index];
        const bool available = special_path
                                   ? record.active
                                   : record.active && !record.effect_locked &&
                                         !record.selected && !record.high_effect &&
                                         record.has_action;
        if (!available) {
            continue;
        }

        if (!special_path) {
            owner.service_marker = 0U;
        }
        record.selected = true;
        record.selection_code = 0x8009U;
        owner.selected_target = index;
        record.owner_linked = !special_path;
        if (callbacks.target_selected) {
            callbacks.target_selected(index);
        }
        return EntityServiceTargetAction::selected;
    }
    return EntityServiceTargetAction::no_available_target;
}

} // namespace shadowrun::ghidra_conversion
