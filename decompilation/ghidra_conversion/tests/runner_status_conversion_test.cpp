#include "shadowrun_ghidra/runner_status_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<RunnerStatusRecord, 3> records{{
        {0xFFU, 0x71U, 0x72U},
        {0x50U, 0x73U, 0x74U},
        {0x0FU, 0x75U, 0x76U},
    }};
    std::vector<std::size_t> preparation_order;
    std::vector<std::size_t> finalization_order;
    std::vector<std::uint8_t> preparation_arguments;

    reset_runner_status_triplet(
        records,
        [&preparation_order, &preparation_arguments](
            const std::size_t index, RunnerStatusRecord& record,
            const std::uint8_t argument) {
            preparation_order.push_back(index);
            preparation_arguments.push_back(argument);
            record.flags_at_0x10 |= 0x01U;
        },
        [&finalization_order](const std::size_t index,
                              RunnerStatusRecord&) {
            finalization_order.push_back(index);
        });

    if (preparation_order != std::vector<std::size_t>{0U, 1U, 2U} ||
        finalization_order != std::vector<std::size_t>{0U, 1U, 2U} ||
        preparation_arguments !=
            std::vector<std::uint8_t>{0x0AU, 0x0AU, 0x0AU}) {
        return 1;
    }
    if (records[0].flags_at_0x10 != 0xAFU ||
        records[1].flags_at_0x10 != 0x01U ||
        records[2].flags_at_0x10 != 0x0FU) {
        return 2;
    }
    for (const auto& record : records) {
        if (record.timer_at_0x4c != 0x0AU ||
            record.timer_at_0x9c != 0x0AU) {
            return 3;
        }
    }

    std::array<std::uint8_t, 3> source_bytes{{0x00U, 0x7FU, 0xFFU}};
    adjust_runner_source_byte_triplet(source_bytes, 0x82U);
    if (source_bytes != std::array<std::uint8_t, 3>{{0x82U, 0x01U, 0x81U}}) {
        return 4;
    }
    return 0;
}
