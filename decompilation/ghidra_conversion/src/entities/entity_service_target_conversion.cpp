#include "shadowrun_ghidra/entities/entity_service_target_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {

namespace {

bool is_special_mode(const std::uint8_t mode) {
    return mode == 0x0CU || mode == 0x04U || mode == 0x0BU;
}

void release_previous_target(EntityServiceTargetOwner& owner,
                             std::vector<EntityServiceTargetRecord>& records) {
    if (!owner.selected_target.has_value() ||
        *owner.selected_target >= records.size()) {
        owner.selected_target.reset();
        return;
    }
    records[*owner.selected_target].selected = false;
    owner.selected_target.reset();
}

} // namespace

EntityServiceTargetAction select_world_entity_service_target(
    EntityServiceTargetOwner& owner,
    std::vector<EntityServiceTargetRecord>& records,
    const bool advance_source_cursor,
    const EntityServiceTargetCallbacks& callbacks) {
    release_previous_target(owner, records);
    if (records.empty()) {
        return EntityServiceTargetAction::no_available_target;
    }

    const bool special_path = owner.special_marker && is_special_mode(owner.mode);
    std::size_t start = owner.source_target.value_or(0U);
    if (advance_source_cursor && owner.source_target.has_value()) {
        ++start;
    }
    if (start >= records.size()) {
        start = 0U;
    }

    constexpr std::size_t max_candidates = 8U;
    for (std::size_t offset = 0; offset < max_candidates; ++offset) {
        const auto index = (start + offset) % records.size();
        auto& record = records[index];
        const bool available = special_path
                                   ? record.active
                                   : record.active && !record.effect_locked &&
                                         !record.selected && !record.high_effect &&
                                         record.has_action;
        if (!available) {
            continue;
        }

        owner.service_marker = 0U;
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
