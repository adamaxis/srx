#pragma once

#include "shadowrun/genesis_bus.hpp"

#include <cstddef>
#include <cstdint>

namespace shadowrun {

enum class IndexedObjectUpdatePath {
    ObjectList,
    ExpiredByteScatter,
    InvalidTableIndex
};

struct IndexedObjectUpdateResult {
    IndexedObjectUpdatePath path;
    std::int16_t timer_after;
    std::uint16_t projection_return;
    std::size_t emitted_records;
    std::uint8_t next_ordinal;
};

// Directly composed translation of the reviewed object update at ROM 0x4ADC.
// record_address is the A0 entity/object record. The output ordinal and
// max_records are supplied because they are caller-held state and the host
// needs a finite sentinel-list guard.
IndexedObjectUpdateResult update_indexed_object_record(
    GenesisBus& bus,
    GenesisAddress record_address,
    std::uint16_t camera_x,
    std::uint16_t camera_y,
    std::uint8_t first_ordinal,
    std::size_t max_records);

} // namespace shadowrun
