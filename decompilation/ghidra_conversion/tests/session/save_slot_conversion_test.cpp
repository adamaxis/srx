#include "shadowrun_ghidra/session/save_slot_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const SaveSlotValidator validate =
        [](const std::vector<std::uint8_t>& bytes) {
            return bytes.size() == 4U && bytes[0] == 0x47U;
        };

    SaveSlotPair slots{{0x47U, 1U, 2U, 3U}, {0x47U, 4U, 5U, 6U}};
    if (!expect(normalize_sram_save_slots(slots, validate).action ==
                    SaveSlotNormalizationAction::both_valid &&
                slots.first[1] == 1U && slots.second[1] == 4U)) {
        return 1;
    }

    slots.second = {0U, 0U, 0U, 0U};
    if (!expect(normalize_sram_save_slots(slots, validate).action ==
                    SaveSlotNormalizationAction::repaired_second_from_first &&
                slots.second == slots.first)) {
        return 2;
    }

    slots.first = {0U, 0U, 0U, 0U};
    slots.second = {0x47U, 9U, 8U, 7U};
    if (!expect(normalize_sram_save_slots(slots, validate).action ==
                    SaveSlotNormalizationAction::repaired_first_from_second &&
                slots.first == slots.second)) {
        return 3;
    }

    slots.first = {0U};
    slots.second = {0U};
    if (!expect(normalize_sram_save_slots(slots, validate).action ==
                SaveSlotNormalizationAction::no_valid_slot)) {
        return 4;
    }

    return 0;
}
