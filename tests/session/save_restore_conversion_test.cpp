#include "shadowrun_ghidra/session/save_restore_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const SaveSlotValidator validate =
        [](const std::vector<std::uint8_t>& bytes) {
            return bytes.size() == 3U && bytes[0] == 0x47U;
        };
    std::vector<std::uint8_t> work;

    SaveSlotPair backup{{0x47U, 1U, 2U}, {0U, 3U, 4U}};
    const auto primary = load_save_work_buffer(backup, work, validate);
    if (!expect(primary.action ==
                    SaveWorkLoadAction::
                        loaded_from_primary_backup_repaired_secondary &&
                work == std::vector<std::uint8_t>{0x47U, 1U, 2U} &&
                backup.second == backup.first)) {
        return 1;
    }

    backup = {{0U, 1U, 2U}, {0x47U, 9U, 8U}};
    const auto secondary = load_save_work_buffer(backup, work, validate);
    if (!expect(secondary.action ==
                    SaveWorkLoadAction::
                        loaded_from_secondary_backup_repaired_primary &&
                work == backup.first && work[1] == 9U)) {
        return 2;
    }

    backup = {{0x47U, 1U, 2U}, {0x47U, 9U, 8U}};
    const auto both_valid = load_save_work_buffer(backup, work, validate);
    if (!expect(both_valid.action ==
                    SaveWorkLoadAction::loaded_from_primary_backup &&
                backup.first == std::vector<std::uint8_t>{0x47U, 1U, 2U} &&
                backup.second == std::vector<std::uint8_t>{0x47U, 9U, 8U})) {
        return 3;
    }

    backup = {{0U, 1U, 2U}, {0U, 9U, 8U}};
    const auto failed = load_save_work_buffer(backup, work, validate);
    if (!expect(failed.action == SaveWorkLoadAction::no_valid_save)) {
        return 4;
    }

    bool prepared = false;
    bool loaded = false;
    const auto coordinated = coordinate_save_load(
        [&prepared]() {
            prepared = true;
            return SaveSlotNormalizationResult{
                SaveSlotNormalizationAction::both_valid};
        },
        [&loaded]() {
            loaded = true;
            return SaveWorkLoadResult{
                SaveWorkLoadAction::loaded_from_primary_backup};
        });
    if (!expect(prepared && loaded &&
                coordinated.action == SaveLoadCoordinatorAction::loaded &&
                coordinated.load.action ==
                    SaveWorkLoadAction::loaded_from_primary_backup)) {
        return 5;
    }

    loaded = false;
    const auto blocked = coordinate_save_load(
        []() {
            return SaveSlotNormalizationResult{
                SaveSlotNormalizationAction::no_valid_slot};
        },
        [&loaded]() {
            loaded = true;
            return SaveWorkLoadResult{};
        });
    if (!expect(!loaded &&
                blocked.action == SaveLoadCoordinatorAction::sram_slots_unavailable)) {
        return 6;
    }

    return 0;
}
