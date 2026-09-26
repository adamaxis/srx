#include "shadowrun_ghidra/core/controller_port_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    std::vector<std::uint8_t> selects;
    const auto result = read_controller_port({
        [&order] { order.push_back(1); },
        [&order, &selects](const std::uint8_t select) {
            order.push_back(2);
            selects.push_back(select);
            return select == 0U ? 0x30U : 0x15U;
        },
        [&order] { order.push_back(3); },
    });
    // ~(0x15 | ((0x30 & 0x30) << 2)) = ~0xD5 = 0x2A.
    if (!expect(result == 0x2AU && order == std::vector<int>{1, 2, 2, 3} &&
                selects == std::vector<std::uint8_t>{0x00U, 0x40U})) {
        return 1;
    }

    if (!expect(read_controller_port({}) == 0U)) {
        return 2;
    }

    return 0;
}
