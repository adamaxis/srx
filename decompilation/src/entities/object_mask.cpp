#include "shadowrun/entities/object_mask.hpp"

namespace shadowrun {
namespace {

std::uint16_t add_word(std::uint16_t left, std::int16_t right) {
    return static_cast<std::uint16_t>(left +
                                      static_cast<std::uint16_t>(right));
}

GenesisAddress indexed_address(std::uint16_t word_index) {
    const auto signed_index = static_cast<std::int16_t>(word_index);
    return static_cast<GenesisAddress>(
               static_cast<std::int32_t>(ram::kIndexedObjectByteTable) +
               signed_index) & 0x00FFFFFFU;
}

} // namespace

void clear_indexed_object_byte(GenesisBus& bus, std::uint16_t index_word) {
    const auto signed_index = static_cast<std::int16_t>(index_word);
    bus.write8(static_cast<GenesisAddress>(
                   static_cast<std::int32_t>(ram::kIndexedObjectClearTable) +
                   signed_index) & 0x00FFFFFFU,
               0);
}

void clear_indexed_object_state(GenesisBus& bus) {
    for (std::size_t index = 0; index < kIndexedObjectStateByteCount; ++index) {
        bus.write8(ram::kIndexedObjectClearTable + index, 0);
    }
}

std::size_t scatter_indexed_object_bytes(
    GenesisBus& bus,
    std::uint16_t base_word,
    const std::array<std::int16_t, kIndexedObjectByteCount>& offsets,
    const std::uint8_t* source,
    std::size_t source_capacity) {
    if (source == nullptr) {
        return 0;
    }

    const auto count = source_capacity < kIndexedObjectByteCount
        ? source_capacity
        : kIndexedObjectByteCount;
    for (std::size_t index = 0; index < count; ++index) {
        bus.write8(indexed_address(add_word(base_word, offsets[index])),
                   source[index]);
    }
    return count;
}

} // namespace shadowrun
