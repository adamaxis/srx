#include "shadowrun_ghidra/entities/entity_service_target_conversion.hpp"

#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<EntityServiceTargetRecord> records(8U);
    records[2] = {true, false, false, true, false, true, 1U, false, 0U};
    records[3] = {true, false, false, false, false, true, 1U, false, 0U};
    records[4] = {true, true, false, false, false, true, 1U, false, 0U};
    EntityServiceTargetOwner owner{};
    owner.service_marker = 7U;
    owner.selected_target = 2U;
    owner.linked_target = 2U;
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
    owner.owner_record = 1U;
    if (select_world_entity_service_target(owner, records, true) !=
            EntityServiceTargetAction::selected ||
        owner.selected_target != 2U || !records[2].selected ||
        records[2].owner_linked) {
        return 2;
    }

    // The special branch does not clear +0x43, and its active-only scan does
    // not reject a locked/high-effect candidate.
    records.assign(8U, {});
    records[1].active = true;
    records[1].descriptor_class = 1U;
    owner = {};
    owner.special_marker = true;
    owner.mode = 0x0BU;
    owner.service_marker = 9U;
    owner.owner_record = 1U;
    if (select_world_entity_service_target(owner, records, false) !=
            EntityServiceTargetAction::selected ||
        owner.service_marker != 9U || owner.selected_target != 1U ||
        records[1].owner_linked) {
        return 4;
    }

    records.assign(8U, {});
    owner = {};
    if (select_world_entity_service_target(owner, records, false) !=
        EntityServiceTargetAction::no_available_target) {
        return 3;
    }

    // A source target at the end of the table advances only forward; the ROM
    // does not wrap an eight-record scan back to FF0100.
    records.assign(10U, {});
    records[0].active = true;
    records[0].has_action = true;
    records[9].descriptor_class = 3U;
    owner = {};
    owner.selected_target = 9U;
    records[9].selected = true;
    if (select_world_entity_service_target(owner, records, false) !=
            EntityServiceTargetAction::no_available_target ||
        records[0].selected) {
        return 5;
    }
    return 0;
}
