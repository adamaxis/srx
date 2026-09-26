#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityServiceTargetRecord {
    bool active = false;              // +0x10 bit 0
    bool effect_locked = false;       // +0x10 bit 6
    bool selected = false;            // +0x10 bit 4
    bool high_effect = false;         // +0xB0 bit 7
    bool has_action = false;          // +0x0C != 0
    std::uint8_t descriptor_class = 0; // +0x3E
    bool owner_linked = false;        // +0xAC back-link was published
    std::uint16_t selection_code = 0; // +0xA6
};

struct EntityServiceTargetOwner {
    bool special_marker = false;       // +0x56 sign bit
    std::uint8_t mode = 0;             // +0x57
    std::uint8_t service_marker = 0;   // +0x43
    std::optional<std::size_t> selected_target;
    std::optional<std::size_t> source_target; // +0xAC/+0x56 source link
};

struct EntityServiceTargetCallbacks {
    std::function<void(std::size_t target_index)> target_selected;
};

enum class EntityServiceTargetAction {
    selected,
    no_available_target,
};

// ROM 0x1DC4C: release the owner's previous target, scan at most eight
// source records, and publish a selected target and its 0x8009 action code.
// The caller supplies whether the source's D1 cursor was the special
// 0x678CC sentinel; no source pointer arithmetic crosses this API.
EntityServiceTargetAction select_world_entity_service_target(
    EntityServiceTargetOwner& owner,
    std::vector<EntityServiceTargetRecord>& records,
    bool advance_source_cursor,
    const EntityServiceTargetCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
