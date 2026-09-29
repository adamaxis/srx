#include "shadowrun_ghidra/core/packed_data_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(kPackedNibbleTable[0x06] == 0x04 &&
                kPackedNibbleTable[0x07] == 0x03 &&
                kPackedNibbleTable[0x0D] == 0x04 &&
                kPackedNibbleTable[0x0F] == 0x02)) {
        return 1;
    }

    const auto remapped = expand_packed_nibbles(
        {0x00U, 0x1AU, 0xF3U, 0x65U, 0x7DU, 0xFFU});
    if (!expect(remapped == std::vector<std::uint8_t>{
                    0x00U, 0x13U, 0x23U, 0x45U, 0x34U, 0x22U})) {
        return 2;
    }
    if (!expect(expand_packed_nibbles({}).empty())) {
        return 3;
    }
    return 0;
}
