#include "shadowrun_ghidra/save_validation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool has_save_layout(const std::vector<std::uint8_t>& extracted_save) {
    return extracted_save.size() >=
           kSaveChecksumOffset + sizeof(std::uint32_t);
}

std::uint32_t read_big_endian_longword(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3U]);
}

} // namespace

std::uint32_t sum_game_save_payload(
    const std::vector<std::uint8_t>& extracted_save) {
    if (extracted_save.size() < kSavePayloadOffset + kSavePayloadLength) {
        return 0U;
    }

    std::uint32_t sum = 0U;
    for (std::size_t index = 0; index < kSavePayloadLength; ++index) {
        sum += extracted_save[kSavePayloadOffset + index];
    }
    return sum;
}

bool validate_game_save_image(
    const std::vector<std::uint8_t>& extracted_save) {
    if (!has_save_layout(extracted_save) ||
        extracted_save[kSaveHeaderOffset] != static_cast<std::uint8_t>('G') ||
        extracted_save[kSaveHeaderOffset + 1U] !=
            static_cast<std::uint8_t>('A') ||
        extracted_save[kSaveHeaderOffset + 2U] !=
            static_cast<std::uint8_t>('M') ||
        extracted_save[kSaveHeaderOffset + 3U] !=
            static_cast<std::uint8_t>('E')) {
        return false;
    }
    return sum_game_save_payload(extracted_save) ==
           read_big_endian_longword(extracted_save, kSaveChecksumOffset);
}

bool validate_backup_game_save(
    const std::vector<std::uint8_t>& extracted_save) {
    return validate_game_save_image(extracted_save);
}

bool validate_sram_game_save(
    const std::vector<std::uint8_t>& extracted_save) {
    return validate_game_save_image(extracted_save);
}

} // namespace shadowrun::ghidra_conversion
