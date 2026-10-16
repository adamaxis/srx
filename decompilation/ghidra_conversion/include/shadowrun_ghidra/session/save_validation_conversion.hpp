#pragma once

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kSaveHeaderOffset = 0x04U;
constexpr std::size_t kSavePayloadOffset = 0x14U;
constexpr std::size_t kSavePayloadLength = 0x61CU;
constexpr std::size_t kSaveChecksumOffset = 0x630U;

// ROM 0x59786: sum the extracted save payload.  The source walks 0x61C bytes
// and compares the resulting 32-bit sum with the word following that payload.
std::uint32_t sum_game_save_payload(
    const std::vector<std::uint8_t>& extracted_save);

// Shared source behavior used by both save-buffer validators.  The input is
// already in the host-side result of the source even-byte extraction step.
bool validate_game_save_image(
    const std::vector<std::uint8_t>& extracted_save);

// ROM 0x593FC: validate the extracted backup/save work buffer.
bool validate_backup_game_save(
    const std::vector<std::uint8_t>& extracted_save);

// ROM 0x5961C: validate the extracted SRAM save buffer.
bool validate_sram_game_save(
    const std::vector<std::uint8_t>& extracted_save);

} // namespace shadowrun::ghidra_conversion
