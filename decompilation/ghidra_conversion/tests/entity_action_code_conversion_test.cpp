#include "shadowrun_ghidra/entity_action_code_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint8_t> record(0x166U, 0U);
    record[0x66U + 7U] = 0x31U;
    record[0x66U + 8U] = 0x32U;
    record[0x66U + 0xFFU] = 0xFEU;
    if (read_entity_action_code(record, 3U, 2U) != 0x31U ||
        read_entity_action_code(record, 4U, 2U) != 0x32U ||
        read_entity_action_code(record, 1U, 0x7FU) != 0xFEU) {
        return 1;
    }

    if (read_entity_action_code(std::vector<std::uint8_t>(0x66U), 0U, 0U)
            .has_value()) {
        return 2;
    }

    return 0;
}
