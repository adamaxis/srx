#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityWorldPosition {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

// The source descriptor is a compact entity-template payload. The names of
// individual template values stay source-oriented until a consuming routine
// gives them a reliable game-specific meaning; the transfer itself is typed
// and does not require a ROM/RAM byte view.
struct EntityTemplateData {
    std::uint16_t word_00 = 0;
    std::uint16_t word_02 = 0;
    std::array<std::uint8_t, 6> bytes_04_to_09{};
    std::array<std::uint32_t, 3> longwords_0a_to_15{};
    std::uint16_t word_16 = 0;
    std::array<std::uint32_t, 4> longwords_18_to_27{};
    std::array<std::uint8_t, 24> bytes_28_to_3f{};
    std::uint8_t byte_41 = 0;
    std::uint8_t byte_42 = 0;
    std::uint8_t byte_43 = 0;
    std::uint16_t word_44 = 0;
    std::uint16_t word_46 = 0;
    std::uint8_t byte_48 = 0;
    std::uint8_t byte_49 = 0;
};

struct EntityRecord {
    std::uint16_t entity_slot = 0;
    std::uint16_t effect_slot = 0;
    std::uint8_t behavior_flags = 0;
    std::uint16_t state_word = 0;
    std::uint16_t auxiliary_word = 0;
    std::uint16_t vertical_origin = 0;
    EntityWorldPosition source_position{};
    EntityWorldPosition world_position{};
    EntityWorldPosition base_position{};
    std::optional<std::uint16_t> linked_root_entity_slot;
    EntityTemplateData template_data{};
    std::uint16_t random_variant = 0;
    std::uint8_t effect_flags = 0;
};

struct EntityRecordOverride {
    std::uint16_t source_offset = 0;
    std::uint8_t value = 0;
};

struct EntityRecordInitialization {
    std::uint16_t entity_slot = 0;
    std::uint16_t state_word = 0;
    std::uint16_t auxiliary_word = 0;
    std::uint16_t vertical_origin = 0;
    EntityWorldPosition source_position{};
    std::uint16_t scene_id = 0;
    bool special_state_active = false;
    std::uint16_t override_mode = 0;
    std::vector<EntityRecordOverride> overrides;
};

using EntityRecordDefaultFactory = std::function<EntityRecord()>;
using EntityRecordSlotAllocator =
    std::function<std::optional<std::uint16_t>()>;
using EntityRecordRandomBoundary = std::function<std::uint16_t()>;
using EntityRecordOverrideBoundary =
    std::function<void(EntityRecord&, const EntityRecordOverride&)>;

// ROM 0x44BC: seed one entity record from the shared template, install its
// source-derived position/state fields, choose the effect variant, and apply
// the optional override list. Default-record construction, PRNG state, and
// unresolved override-field semantics are explicit host boundaries.
EntityRecord initialize_entity_record_from_template(
    const EntityRecordInitialization& initialization,
    const EntityTemplateData& template_data,
    const EntityRecordDefaultFactory& make_default_record,
    const EntityRecordRandomBoundary& next_random,
    const EntityRecordOverrideBoundary& apply_override);

struct EntityTemplateSpawnDescriptor {
    EntityTemplateData template_data{};
    EntityWorldPosition source_position{};
    bool root_record = false;
    bool sentinel = false;
};

struct EntityTemplateSpawnState {
    std::uint16_t state_word = 0;
    std::uint16_t auxiliary_word = 0;
    std::uint16_t scene_id = 0;
    bool special_state_active = false;
    std::uint16_t vertical_origin = 0;
    std::optional<std::uint16_t> root_entity_slot;
};

enum class EntityTemplateSpawnAction {
    completed,
    entity_table_full,
    effect_table_full,
};

struct EntityTemplateSpawnResult {
    EntityTemplateSpawnAction action = EntityTemplateSpawnAction::completed;
    std::uint16_t final_vertical_origin = 0;
    std::vector<EntityRecord> records;
};

using EntityTemplateRecordInitializer = std::function<EntityRecord(
    const EntityRecordInitialization&, const EntityTemplateData&)>;

// ROM 0x4466: consume the terminated template list, allocate the paired
// entity/effect slots, group child records by a 0xC0 vertical step, mark each
// root record, and link children to that root. Allocation and record creation
// remain explicit host boundaries.
EntityTemplateSpawnResult spawn_entity_template_list(
    const std::vector<EntityTemplateSpawnDescriptor>& descriptors,
    EntityTemplateSpawnState& state,
    const EntityRecordSlotAllocator& allocate_entity_slot,
    const EntityRecordSlotAllocator& allocate_effect_slot,
    const EntityRecordDefaultFactory& make_default_record,
    const EntityTemplateRecordInitializer& initialize_record);

} // namespace shadowrun::ghidra_conversion
