#include "shadowrun_ghidra/cyberspace/matrix_storage_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint8_t matrix_loaded_slot_usage(const MatrixStorageUsage& usage,
                                      const std::size_t slot) {
    if (slot >= usage.loaded_slot_usage.size()) {
        return 0;
    }
    return usage.loaded_slot_usage[slot];
}

std::uint32_t matrix_program_storage_weight(
    const std::uint8_t program, const std::uint8_t amount) {
    if (program >= kMatrixProgramFootprints.size()) {
        return 0U;
    }
    const auto value = static_cast<std::uint32_t>(amount);
    return value * value *
           static_cast<std::uint32_t>(kMatrixProgramFootprints[program]);
}

std::uint32_t matrix_program_storage_usage(const MatrixStorageUsage& usage,
                                           const std::size_t program) {
    if (program >= usage.availability_flags.size() ||
        program >= kMatrixProgramFootprints.size()) {
        return 0;
    }
    return matrix_program_storage_weight(
        static_cast<std::uint8_t>(program), usage.availability_flags[program]);
}

MatrixStorageCapacity calculate_matrix_storage_capacity(
    const MatrixStorageUsage& usage) {
    MatrixStorageCapacity result{};
    result.total_memory = usage.total_memory;

    for (std::size_t slot = 0; slot < usage.loaded_slot_usage.size(); ++slot) {
        result.loaded_memory = static_cast<std::uint16_t>(
            result.loaded_memory + matrix_loaded_slot_usage(usage, slot));
    }
    for (std::size_t program = 0;
         program < usage.availability_flags.size(); ++program) {
        result.installed_program_memory = static_cast<std::uint16_t>(
            result.installed_program_memory +
            static_cast<std::uint16_t>(matrix_program_storage_usage(
                usage, program)));
    }

    result.available_memory = static_cast<std::uint16_t>(
        result.total_memory - result.loaded_memory -
        result.installed_program_memory);
    return result;
}

MatrixLoadedProgramCapacity calculate_loaded_program_capacity(
    const MatrixStorageUsage& usage,
    const std::array<std::int8_t, 5>& slot_programs) {
    MatrixLoadedProgramCapacity result{};
    result.total_memory = usage.loaded_program_memory;
    for (const auto slot_program : slot_programs) {
        if (slot_program < 0) {
            continue;
        }
        result.used_memory = static_cast<std::uint16_t>(
            result.used_memory +
            static_cast<std::uint16_t>(matrix_program_storage_usage(
                usage, static_cast<std::size_t>(slot_program))));
    }
    result.available_memory = static_cast<std::uint16_t>(
        result.total_memory - result.used_memory);
    return result;
}

MatrixDataFileList collect_matrix_storage_datafiles(
    const std::array<MatrixDataFileRecord, 5>& records) {
    MatrixDataFileList result{};
    for (std::size_t slot = records.size(); slot-- > 0;) {
        const auto& record = records[slot];
        if (record.type < 0) {
            continue;
        }
        result.entries[result.count] = MatrixDataFileEntry{
            static_cast<std::uint8_t>(slot), record.type, record.usage,
            static_cast<std::uint8_t>(result.count)};
        ++result.count;
    }
    result.has_entries = result.count != 0U;
    return result;
}

} // namespace shadowrun::ghidra_conversion
