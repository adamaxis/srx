#include "shadowrun_ghidra/cyberspace/matrix_storage_conversion.hpp"

#include <array>
#include <cstdint>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    MatrixStorageUsage usage{};
    usage.total_memory = 80U;
    usage.loaded_slot_usage = {2U, 4U, 0U, 3U, 1U};
    usage.availability_flags = {1U, 0U, 1U, 0U, 1U, 0U,
                                0U, 1U, 0U, 0U, 0U, 1U};

    if (!expect(matrix_loaded_slot_usage(usage, 0U) == 2U &&
                matrix_loaded_slot_usage(usage, 4U) == 1U &&
                matrix_loaded_slot_usage(usage, 5U) == 0U)) {
        return 1;
    }
    if (!expect(matrix_program_storage_usage(usage, 0U) == 2U &&
                matrix_program_storage_usage(usage, 2U) == 4U &&
                matrix_program_storage_usage(usage, 11U) == 3U &&
                matrix_program_storage_usage(usage, 12U) == 0U &&
                matrix_program_storage_weight(2U, 4U) == 16U)) {
        return 2;
    }

    const auto capacity = calculate_matrix_storage_capacity(usage);
    if (!expect(capacity.total_memory == 80U && capacity.loaded_memory == 10U &&
                capacity.installed_program_memory == 16U &&
                capacity.available_memory == 54)) {
        return 3;
    }

    usage.total_memory = 3U;
    usage.loaded_slot_usage = {2U, 2U, 0U, 0U, 0U};
    usage.availability_flags.fill(0U);
    const auto exhausted = calculate_matrix_storage_capacity(usage);
    if (!expect(exhausted.available_memory == 0xFFFFU)) {
        return 4;
    }

    // 0x19558 uses ADD.W/SUB.W.  The source returns a wrapped word when the
    // loaded total exceeds capacity; it does not publish a signed -1.
    usage.total_memory = 0U;
    usage.loaded_slot_usage = {0U, 0U, 0U, 0U, 0U};
    usage.availability_flags.fill(0U);
    usage.availability_flags[0] = 0xFFU;
    const auto wrapped_installed = calculate_matrix_storage_capacity(usage);
    if (!expect(wrapped_installed.installed_program_memory == 0xFC02U &&
                wrapped_installed.available_memory == 0x03FEU)) {
        return 8;
    }

    usage.availability_flags = {1U, 0U, 1U, 0U, 1U, 0U,
                                0U, 1U, 0U, 0U, 0U, 1U};
    usage.loaded_program_memory = 20U;
    const auto loaded_capacity = calculate_loaded_program_capacity(
        usage, std::array<std::int8_t, 5>{0, -1, 2, 11, -1});
    if (!expect(loaded_capacity.total_memory == 20U &&
                loaded_capacity.used_memory == 9U &&
                loaded_capacity.available_memory == 11)) {
        return 5;
    }

    const auto datafiles = collect_matrix_storage_datafiles(
        std::array<MatrixDataFileRecord, 5>{
            MatrixDataFileRecord{3, 7}, MatrixDataFileRecord{-1, 0},
            MatrixDataFileRecord{1, 2}, MatrixDataFileRecord{-1, 0},
            MatrixDataFileRecord{5, 9}});
    if (!expect(datafiles.has_entries && datafiles.count == 3U &&
                datafiles.entries[0].slot == 4U &&
                datafiles.entries[0].type == 5 &&
                datafiles.entries[0].usage == 9U &&
                datafiles.entries[0].display_row == 0U &&
                datafiles.entries[1].slot == 2U &&
                datafiles.entries[2].slot == 0U)) {
        return 6;
    }
    const auto empty = collect_matrix_storage_datafiles(
        std::array<MatrixDataFileRecord, 5>{});
    if (!expect(!empty.has_entries && empty.count == 0U)) {
        return 7;
    }
    return 0;
}
