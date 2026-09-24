#include "shadowrun_ghidra/text_line_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto mapped = normalize_source_text_line(
        {static_cast<std::uint8_t>('A'), 0x80U, 0xBFU, 0xC0U}, 2);
    if (!expect(mapped.terminated && mapped.source_bytes_consumed == 4U &&
                mapped.bytes == std::vector<std::uint8_t>{'A', 0x80U, 0x80U,
                                                          ' ', ' ', ' '})) {
        return 1;
    }

    const auto no_padding = normalize_source_text_line({0xC1U}, -1);
    if (!expect(no_padding.terminated && no_padding.source_bytes_consumed == 1U &&
                no_padding.bytes.empty())) {
        return 2;
    }

    const auto incomplete = normalize_source_text_line({'O', 'K'}, 0);
    if (!expect(!incomplete.terminated &&
                incomplete.source_bytes_consumed == 2U &&
                incomplete.bytes == std::vector<std::uint8_t>{'O', 'K'})) {
        return 3;
    }

    return 0;
}
