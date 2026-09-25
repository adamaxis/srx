#include "shadowrun_ghidra/matrix_storage_conversion.hpp"

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
    usage.program_footprints = kMatrixProgramFootprints;

    if (!expect(matrix_loaded_slot_usage(usage, 0U) == 2U &&
                matrix_loaded_slot_usage(usage, 4U) == 1U &&
                matrix_loaded_slot_usage(usage, 5U) == 0U)) {
        return 1;
    }
    if (!expect(matrix_program_storage_usage(usage, 0U) == 2U &&
                matrix_program_storage_usage(usage, 2U) == 4U &&
                matrix_program_storage_usage(usage, 11U) == 3U &&
                matrix_program_storage_usage(usage, 12U) == 0U)) {
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
    if (!expect(exhausted.available_memory == -1)) {
        return 4;
    }
    return 0;
}
