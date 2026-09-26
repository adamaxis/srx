#include "shadowrun_ghidra/entities/entity_service_target_conversion.hpp"

#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<EntityServiceTargetRecord> records(8U);
    records[2] = {true, false, true, false, true, 1U, false, 0U};
    records[3] = {true, false, false, false, true, 1U, false, 0U};
    records[4] = {true, true, false, false, true, 1U, false, 0U};
    EntityServiceTargetOwner owner{};
    owner.service_marker = 7U;
    owner.selected_target = 2U;
    owner.source_target = 2U;
    std::size_t selected = 99U;
    if (select_world_entity_service_target(
            owner, records, true,
            EntityServiceTargetCallbacks{
                [&](const std::size_t index) { selected = index; }}) !=
            EntityServiceTargetAction::selected ||
        selected != 3U || owner.selected_target != 3U || records[2].selected ||
        !records[3].selected || !records[3].owner_linked ||
        records[3].selection_code != 0x8009U || owner.service_marker != 0U) {
        return 1;
    }

    // Special modes use the active-only path, so an otherwise locked record
    // is still eligible after the source cursor advances.
    records.assign(8U, {});
    records[2].active = true;
    records[2].effect_locked = true;
    owner = {};
    owner.special_marker = true;
    owner.mode = 0x0CU;
    owner.source_target = 1U;
    if (select_world_entity_service_target(owner, records, true) !=
            EntityServiceTargetAction::selected ||
        owner.selected_target != 2U || !records[2].selected ||
        records[2].owner_linked) {
        return 2;
    }

    records.assign(8U, {});
    owner = {};
    if (select_world_entity_service_target(owner, records, false) !=
        EntityServiceTargetAction::no_available_target) {
        return 3;
    }
    return 0;
}
