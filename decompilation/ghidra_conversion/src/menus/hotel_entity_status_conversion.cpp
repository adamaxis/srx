#include "shadowrun_ghidra/menus/hotel_entity_status_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void service_hotel_entity_status(
    std::array<HotelEntityStatusRecord, kHotelEntityStatusRecordCount>&
        records,
    const HotelEntityStatusCallbacks& callbacks) {
    if (callbacks.write_hotel_entity_status_grid) {
        callbacks.write_hotel_entity_status_grid();
    }

    for (std::size_t index = 0; index < records.size(); ++index) {
        auto& record = records[index];
        if ((record.status_flags & kHotelEntityActiveFlag) == 0U) {
            continue;
        }

        // The source zero-extends the byte loaded from +0x4C before both
        // word comparisons. Therefore this is an unsigned 0..255 split,
        // despite the signed-looking Ghidra casts.
        if (record.delay_meter < 3U) {
            if (callbacks.advance_behavior_meter) {
                callbacks.advance_behavior_meter(index, record, 10U);
            }
            if (callbacks.render_entity_name) {
                callbacks.render_entity_name(index, record.display_name);
            }
            if (callbacks.render_medical_attention_suffix) {
                callbacks.render_medical_attention_suffix(index);
            }
            continue;
        }

        if (callbacks.advance_delay_meter) {
            callbacks.advance_delay_meter(
                index, record, record.delay_meter < 5U ? 1U : 2U);
        }
        if (callbacks.advance_behavior_meter) {
            callbacks.advance_behavior_meter(index, record, 10U);
        }
        record.status_flags = static_cast<std::uint8_t>(
            record.status_flags & ~kHotelEntityServiceFlag);
    }

    if (callbacks.render_hotel_recovery_message) {
        callbacks.render_hotel_recovery_message();
    }
    if (callbacks.wait_for_hotel_entity_status_acknowledgement) {
        callbacks.wait_for_hotel_entity_status_acknowledgement();
    }
}

} // namespace shadowrun::ghidra_conversion
