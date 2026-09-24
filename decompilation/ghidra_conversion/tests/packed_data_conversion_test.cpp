#include "shadowrun_ghidra/packed_data_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    NibbleLookup identity{};
    for (std::uint8_t index = 0; index < identity.size(); ++index) {
        identity[index] = index;
    }
    const auto unchanged = expand_packed_nibbles(
        {0x00U, 0x1AU, 0xF3U}, identity);
    if (!expect(unchanged == std::vector<std::uint8_t>{
                    0x00U, 0x1AU, 0xF3U})) {
        return 1;
    }

    NibbleLookup reversed{};
    for (std::uint8_t index = 0; index < reversed.size(); ++index) {
        reversed[index] = static_cast<std::uint8_t>(0x0FU - index);
    }
    const auto remapped = expand_packed_nibbles({0x12U, 0xA0U}, reversed);
    if (!expect(remapped == std::vector<std::uint8_t>{0xEDU, 0x5FU})) {
        return 1;
    }
    return 0;
}
