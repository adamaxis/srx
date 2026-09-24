#include "shadowrun_ghidra/save_validation_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

void write_big_endian_longword(std::vector<std::uint8_t>& bytes,
                               const std::size_t offset,
                               const std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value >> 24U);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value >> 16U);
    bytes[offset + 2U] = static_cast<std::uint8_t>(value >> 8U);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value);
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint8_t> save(
        kSaveChecksumOffset + sizeof(std::uint32_t), 0U);
    save[kSaveHeaderOffset] = 'G';
    save[kSaveHeaderOffset + 1U] = 'A';
    save[kSaveHeaderOffset + 2U] = 'M';
    save[kSaveHeaderOffset + 3U] = 'E';
    for (std::size_t index = 0; index < kSavePayloadLength; ++index) {
        save[kSavePayloadOffset + index] =
            static_cast<std::uint8_t>((index * 7U + 3U) & 0xFFU);
    }
    const auto checksum = sum_game_save_payload(save);
    write_big_endian_longword(save, kSaveChecksumOffset, checksum);

    if (!expect(checksum != 0U && validate_game_save_image(save) &&
                validate_backup_game_save(save) &&
                validate_sram_game_save(save))) {
        return 1;
    }

    save[kSavePayloadOffset + 17U] ^= 0x01U;
    if (!expect(!validate_game_save_image(save))) {
        return 2;
    }
    save[kSavePayloadOffset + 17U] ^= 0x01U;
    save[kSaveHeaderOffset + 2U] = 'X';
    if (!expect(!validate_sram_game_save(save))) {
        return 3;
    }
    save[kSaveHeaderOffset + 2U] = 'M';
    save[kSaveChecksumOffset + 3U] ^= 0x01U;
    if (!expect(!validate_backup_game_save(save))) {
        return 4;
    }

    if (!expect(sum_game_save_payload({}) == 0U &&
                !validate_game_save_image(std::vector<std::uint8_t>(8U)))) {
        return 5;
    }

    return 0;
}
