#include "shadowrun_ghidra/entity_template_conversion.hpp"

namespace shadowrun::ghidra_conversion {

EntityRecord initialize_entity_record_from_template(
    const EntityRecordInitialization& initialization,
    const EntityTemplateData& template_data,
    const EntityRecordDefaultFactory& make_default_record,
    const EntityRecordRandomBoundary& next_random,
    const EntityRecordOverrideBoundary& apply_override) {
    EntityRecord record = make_default_record ? make_default_record()
                                              : EntityRecord{};

    record.entity_slot = initialization.entity_slot;
    record.behavior_flags = 0x05U;
    record.state_word = initialization.state_word;
    record.auxiliary_word = initialization.auxiliary_word;
    record.vertical_origin = initialization.vertical_origin;
    record.source_position = initialization.source_position;
    record.world_position = {
        initialization.source_position.x,
        static_cast<std::uint16_t>(initialization.source_position.y +
                                   initialization.vertical_origin)};
    record.base_position = record.world_position;
    record.template_data = template_data;

    const auto random_value = next_random ? next_random() : 0U;
    record.random_variant = initialization.scene_id == 0x0037U
                                ? 0x0028U
                                : static_cast<std::uint16_t>(random_value &
                                                              0x000FU);
    if (initialization.special_state_active) {
        record.effect_flags = static_cast<std::uint8_t>(
            record.effect_flags | 0x40U);
    }

    if (initialization.override_mode == 2U) {
        for (const auto& override_value : initialization.overrides) {
            if (apply_override) {
                apply_override(record, override_value);
            }
        }
    }
    return record;
}

} // namespace shadowrun::ghidra_conversion
