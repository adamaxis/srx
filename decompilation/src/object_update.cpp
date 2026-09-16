#include "shadowrun/object_update.hpp"

#include "shadowrun/entity_projection.hpp"
#include "shadowrun/object_list.hpp"
#include "shadowrun/object_mask.hpp"
#include "shadowrun/rom_layout.hpp"

#include <array>

namespace shadowrun {

IndexedObjectUpdateResult update_indexed_object_record(
    GenesisBus& bus,
    GenesisAddress record_address,
    std::uint16_t camera_x,
    std::uint16_t camera_y,
    std::uint8_t first_ordinal,
    std::size_t max_records) {
    const auto timer_after_word = static_cast<std::uint16_t>(
        bus.read16(record_address + 0x0EU) - 1U);
    bus.write16(record_address + 0x0EU, timer_after_word);
    const auto timer_after = static_cast<std::int16_t>(timer_after_word);
    if (timer_after < 0) {
        bus.write16(record_address + 2U, 0);
        clear_indexed_object_byte(bus, bus.read16(record_address + 0x32U));

        const auto vector_index = bus.read16(record_address + 6U);
        if (vector_index >= rom::kIndexedOffsetVectors.size()) {
            return {IndexedObjectUpdatePath::InvalidTableIndex, timer_after,
                    0, 0, first_ordinal};
        }
        std::array<std::uint8_t, kIndexedObjectByteCount> source{};
        for (std::size_t index = 0; index < source.size(); ++index) {
            source[index] = bus.read8(record_address + 0x36U + index);
        }
        const auto copied = scatter_indexed_object_bytes(
            bus, bus.read16(record_address + 0x34U),
            rom::kIndexedOffsetVectors[vector_index], source.data(),
            source.size());
        return {IndexedObjectUpdatePath::ExpiredByteScatter, timer_after,
                0, copied, first_ordinal};
    }

    const auto projection_return = project_entity_record(
        bus, record_address, camera_x, camera_y);
    const auto descriptor_index = static_cast<std::size_t>(
        timer_after_word & 3U);
    const auto vector_index = static_cast<std::size_t>(
        bus.read16(record_address + 6U));
    if (vector_index >= rom::kIndexedDescriptorPointers.size()) {
        return {IndexedObjectUpdatePath::InvalidTableIndex, timer_after,
                projection_return, 0, first_ordinal};
    }

    // A2 points eight bytes into the selected 0x4ACC descriptor, after the
    // two-word header consumed into the object-list bases below.
    const auto descriptor_base =
        rom::kIndexedVariantDescriptorPointers[descriptor_index];
    const auto input_address = descriptor_base + 8U;
    const auto offset_descriptor = rom::kIndexedDescriptorPointers[vector_index];
    const auto word6_base = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(bus.read16(record_address + 0x0AU) +
                                   bus.read16(offset_descriptor)));
    const auto word0_base = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(bus.read16(record_address + 0x0CU) +
                                   bus.read16(offset_descriptor + 2U)));
    const auto list = build_object_list_from_bus(
        bus, input_address, bus.read32(ram::kObjectListOutputPointer),
        max_records, word0_base, word6_base, 0x2000,
        static_cast<std::int16_t>(bus.read16(record_address + 0x2AU)),
        first_ordinal, ObjectListVariant::WithWord3Offset);
    return {IndexedObjectUpdatePath::ObjectList, timer_after,
            projection_return, list.built, list.next_ordinal};
}

} // namespace shadowrun
