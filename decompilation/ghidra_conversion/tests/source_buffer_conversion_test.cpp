#include "shadowrun_ghidra/source_buffer_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    SourceWordBlock source{};
    source[3] = 0x12345678U;
    SourceWordBlock destination{};
    copy_source_word_block(destination, source);
    if (!expect(destination == source)) {
        return 1;
    }

    const std::vector<std::uint8_t> table{
        0x00U, 0xFFU, 0x11U, 0xEEU, 0x22U, 0xDDU, 0x33U, 0xCCU};
    const auto extracted = extract_even_bytes(table, 1U, 3U);
    if (!expect(extracted.has_value() &&
                *extracted == std::vector<std::uint8_t>{0x11U, 0x22U, 0x33U})) {
        return 1;
    }
    if (!expect(!extract_even_bytes(table, 3U, 3U).has_value())) {
        return 1;
    }

    std::vector<std::uint8_t> copied = table;
    if (!expect(copy_even_bytes(copied, 0U, 1U, 3U) &&
                copied == std::vector<std::uint8_t>{
                    0x11U, 0xFFU, 0x22U, 0xEEU, 0x33U, 0xDDU,
                    0x33U, 0xCCU})) {
        return 1;
    }
    if (!expect(!copy_even_bytes(copied, 3U, 0U, 2U))) {
        return 1;
    }
    return 0;
}
