#pragma once

#include "shadowrun/core/genesis_bus.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun {

namespace ram {
constexpr GenesisAddress kIndexedObjectByteTable = 0xFF1700;
constexpr GenesisAddress kIndexedObjectClearTable = 0xFFD86C;
}

constexpr std::size_t kIndexedObjectByteCount = 5;
constexpr std::size_t kIndexedObjectStateByteCount = 18;

// Direct translation of ROM 0x1A304: the object path clears one byte at the
// signed word index supplied in D0.W before it builds the indexed byte view.
void clear_indexed_object_byte(GenesisBus& bus, std::uint16_t index_word);

// Direct translation of ROM 0x1A310: clear the complete 18-byte state table
// beginning at the same base used by the indexed entry clear.
void clear_indexed_object_state(GenesisBus& bus);

// Direct translation of the scatter loop at ROM 0x4B9A-0x4BAA. The source
// bytes are the five record bytes read through A1; each destination index is
// a separately word-wrapped base plus one of the selected five offsets.
std::size_t scatter_indexed_object_bytes(
    GenesisBus& bus,
    std::uint16_t base_word,
    const std::array<std::int16_t, kIndexedObjectByteCount>& offsets,
    const std::uint8_t* source,
    std::size_t source_capacity);

} // namespace shadowrun
