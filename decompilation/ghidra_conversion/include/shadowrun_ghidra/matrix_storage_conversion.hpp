#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

// The Matrix storage screen reads these host-shaped values from the source
// record block. The original records are still the evidence source; this
// view deliberately contains only the fields used by 0x1952C/0x1954A.
struct MatrixStorageUsage {
    std::uint16_t total_memory = 0;
    // FFFBC0, the separate capacity word used by 0x1957E when checking
    // whether a newly selected program can be installed.
    std::uint16_t loaded_program_memory = 0;
    std::array<std::uint8_t, 5> loaded_slot_usage{};
    std::array<std::uint8_t, 12> availability_flags{};
    std::array<std::uint8_t, 12> program_footprints{};
};

struct MatrixStorageCapacity {
    std::uint16_t total_memory = 0;
    std::uint32_t loaded_memory = 0;
    std::uint32_t installed_program_memory = 0;
    std::int32_t available_memory = 0;
};

struct MatrixLoadedProgramCapacity {
    std::uint16_t total_memory = 0;
    std::uint32_t used_memory = 0;
    std::int32_t available_memory = 0;
};

// The storage screen's five datafile records are addressed through a compact
// source slot table. The signed type is the source +0x1F presence marker and
// usage is the source +0x20 byte consumed by 0x1954A.
struct MatrixDataFileRecord {
    std::int8_t type = -1;
    std::uint8_t usage = 0;
};

struct MatrixDataFileEntry {
    std::uint8_t slot = 0;
    std::int8_t type = -1;
    std::uint8_t usage = 0;
    std::uint8_t display_row = 0;
};

struct MatrixDataFileList {
    std::array<MatrixDataFileEntry, 5> entries{};
    std::size_t count = 0;
    bool has_entries = false;
};

// ROM E2DF2: the twelve Matrix program footprints used by 0x1952C. Keeping
// this table named makes the source-derived record layout visible without
// making the runtime depend on a ROM address.
constexpr std::array<std::uint8_t, 12> kMatrixProgramFootprints{
    2U, 4U, 4U, 3U, 4U, 4U, 2U, 3U, 3U, 2U, 2U, 3U};

// ROM 0x1954A: return one loaded Matrix slot's usage byte.
std::uint8_t matrix_loaded_slot_usage(const MatrixStorageUsage& usage,
                                      std::size_t slot);

// ROM 0x1952C: multiply a program's availability flag by its source footprint.
std::uint32_t matrix_program_storage_usage(const MatrixStorageUsage& usage,
                                           std::size_t program);

// ROM 0x19558, as used by the Matrix storage screen at 0x18D1A: subtract
// loaded slots and installed available programs from total storage.
MatrixStorageCapacity calculate_matrix_storage_capacity(
    const MatrixStorageUsage& usage);

// ROM 0x1957E: subtract only the footprints of programs already present in
// the five FFFBD8 slot records from the FFFBC0 program-memory capacity.
MatrixLoadedProgramCapacity calculate_loaded_program_capacity(
    const MatrixStorageUsage& usage,
    const std::array<std::int8_t, 5>& slot_programs);

// ROM 0x18F5E: walk the five storage records from slot 4 down to slot 0,
// retaining occupied entries and their source display order.
MatrixDataFileList collect_matrix_storage_datafiles(
    const std::array<MatrixDataFileRecord, 5>& records);

} // namespace shadowrun::ghidra_conversion
