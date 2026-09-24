#include "shadowrun_ghidra/compressed_data_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using shadowrun::ghidra_conversion::decode_source_byte_stream;

    const auto literal = decode_source_byte_stream(
        {0x00U, 0x00U, 0x00U, 0x41U});
    if (!expect(literal.has_value() && *literal == std::vector<std::uint8_t>{0x41U})) {
        return 1;
    }

    const auto repeated = decode_source_byte_stream(
        {0x00U, 0x02U, 0x80U, 0x00U, 0x42U});
    if (!expect(repeated.has_value() &&
                *repeated == std::vector<std::uint8_t>{0x42U, 0x42U, 0x42U})) {
        return 1;
    }

    const auto back_reference = decode_source_byte_stream(
        {0x00U, 0x05U, 0x10U, 0x41U, 0x42U, 0x43U, 0xB0U, 0x00U});
    if (!expect(back_reference.has_value() &&
                *back_reference == std::vector<std::uint8_t>{
                    0x41U, 0x42U, 0x43U, 0x41U, 0x42U, 0x43U})) {
        return 1;
    }

    if (!expect(!decode_source_byte_stream({0x00U}).has_value() &&
                !decode_source_byte_stream({0x00U, 0x02U, 0x00U}).has_value() &&
                !decode_source_byte_stream(
                    {0x00U, 0x02U, 0x80U, 0x80U, 0x00U}).has_value())) {
        return 1;
    }
    return 0;
}
