#include "shadowrun_ghidra/core/coordinate_data_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using shadowrun::ghidra_conversion::decode_source_coordinate_stream;

    const auto descending = decode_source_coordinate_stream(
        {0x00U, 0x64U, 0x01U, 0x02U, 0x00U});
    if (!expect(descending.has_value() &&
                *descending == std::vector<std::int16_t>{102, 101})) {
        return 1;
    }

    const auto signed_word = decode_source_coordinate_stream(
        {0x01U, 0x00U, 0xA1U, 0xFFU, 0xFEU, 0x00U});
    if (!expect(signed_word.has_value() &&
                *signed_word == std::vector<std::int16_t>{254, 255})) {
        return 1;
    }

    const auto repeated = decode_source_coordinate_stream(
        {0x00U, 0x64U, 0xE1U, 0x00U, 0x03U, 0x00U});
    if (!expect(repeated.has_value() &&
                *repeated == std::vector<std::int16_t>{103, 103})) {
        return 1;
    }

    if (!expect(!decode_source_coordinate_stream({0x00U}).has_value() &&
                !decode_source_coordinate_stream(
                    {0x00U, 0x64U, 0x01U}).has_value() &&
                !decode_source_coordinate_stream(
                    {0x00U, 0x64U, 0x21U, 0x00U}).has_value())) {
        return 1;
    }
    return 0;
}
