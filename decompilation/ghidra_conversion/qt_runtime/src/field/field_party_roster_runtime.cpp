#include "field_party_roster_runtime.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

bool save_marker_active(const std::uint8_t marker) noexcept {
    // The source tests the marker as a signed byte; 0xFF means no recruited
    // runner, while any non-negative value is an index-plus-one.
    return static_cast<std::int8_t>(marker) >= 0;
}

} // namespace

std::string FieldPartyRosterRuntime::read_runner_name(
    const LiveRunnerRecord& record) {
    // ROM 0x1111A passes active record +0x58 to the common text writer.
    // The save projection copies the same 0x0E-byte span from compact +0x0A,
    // so restored recruited records already carry the selected runner's
    // source name. Decode the source's printable menu bytes and trim only
    // fixed-field padding; this is not a fixed three-name UI table.
    std::string value;
    value.reserve(0x0EU);
    for (std::size_t index = 0; index < 0x0EU; ++index) {
        const auto byte = record.bytes[0x58U + index];
        if (byte == 0xFFU) {
            break;
        }
        if (byte == 0x80U) {
            continue;
        }
        if (byte >= 0x20U && byte <= 0x7EU) {
            value.push_back(static_cast<char>(byte));
        }
    }
    const auto first = value.find_first_not_of(' ');
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(' ');
    return value.substr(first, last - first + 1U);
}

void FieldPartyRosterRuntime::clear() noexcept {
    records_ = {};
    names_ = {};
    active_slots_ = {};
    portrait_asset_families_ = {};
    active_slot_ = 0U;
    primary_compact_marker_ = 0xFFU;
    secondary_compact_marker_ = 0xFFU;
    source_ = Source::unavailable;
}

void FieldPartyRosterRuntime::mark_slot_active(
    RunnerRecordFields& record) noexcept {
    // First Aid and the field HUD test byte +0x10, which is the high byte of
    // this big-endian word. Keep the pre-existing +0x11 active marker too:
    // command-overview routing consumes that adjacent byte independently.
    record.flags = static_cast<std::uint16_t>(record.flags | 0x0101U);
}

void FieldPartyRosterRuntime::set_active_slot(const std::size_t slot) noexcept {
    if (slot < active_slots_.size() && active_slots_[slot]) {
        active_slot_ = slot;
        return;
    }
    active_slot_ = 0U;
}

bool FieldPartyRosterRuntime::restore_saved_party(
    const std::array<RunnerRecordFields, kSlotCount>& records,
    const std::array<std::string, kSlotCount>& names,
    const std::array<bool, kSlotCount>& active_slots,
    const std::array<std::uint8_t, kSlotCount>& portrait_families,
    const std::uint8_t primary_compact_marker,
    const std::uint8_t secondary_compact_marker,
    const std::size_t active_slot) noexcept {
    if (active_slot >= kSlotCount || !active_slots[active_slot] ||
        !active_slots[0]) {
        return false;
    }
    records_ = records;
    names_ = names;
    active_slots_ = active_slots;
    portrait_asset_families_ = portrait_families;
    primary_compact_marker_ = primary_compact_marker;
    secondary_compact_marker_ = secondary_compact_marker;
    active_slot_ = active_slot;
    source_ = Source::restored_save;
    return true;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
void FieldPartyRosterRuntime::install_party_member_for_dump(
    const std::size_t slot, RunnerRecordFields record, std::string name,
    const std::uint8_t portrait_asset_family) {
    if (slot >= kSlotCount) {
        return;
    }
    mark_slot_active(record);
    records_[slot] = std::move(record);
    names_[slot] = std::move(name);
    active_slots_[slot] = true;
    portrait_asset_families_[slot] = portrait_asset_family;
    if (source_ == Source::unavailable) {
        source_ = Source::restored_save;
    }
}
#endif // SHADOWRUN_QT_QA_DRIVER


bool FieldPartyRosterRuntime::deactivate_party_member(
    const std::size_t slot) noexcept {
    if (slot == 0U || slot >= active_slots_.size() || !active_slots_[slot]) {
        return false;
    }
    // Dismiss Runner removes the FFFC0C/FFFC0D compact party marker. It does
    // not erase the live record or its field-presence byte at +0x10.
    active_slots_[slot] = false;
    if (slot == 1U) {
        primary_compact_marker_ = 0xFFU;
    } else {
        secondary_compact_marker_ = 0xFFU;
    }
    return true;
}

FieldPartyRosterRuntime::Source FieldPartyRosterRuntime::initialize(
    const integration::StartupIntegrationState& startup_state) noexcept {
    clear();

    if (startup_state.restored_field_state.has_value()) {
        const auto& restored = *startup_state.restored_field_state;
        primary_compact_marker_ = restored.roster.primary_marker;
        secondary_compact_marker_ = restored.roster.secondary_marker;
        records_[0] = read_runner_record_fields(restored.roster.default_record);
        names_[0] = read_runner_name(restored.roster.default_record);
        mark_slot_active(records_[0]);
        active_slots_[0] = true;
        if (startup_state.startup_records.has_value()) {
            for (std::size_t family = 0;
                 family < startup_state.startup_records->joshua_archetypes.size();
                 ++family) {
                if (startup_state.startup_records
                            ->joshua_archetypes[family]
                            .record[0x9BU] == records_[0].archetype_index) {
                    portrait_asset_families_[0] =
                        static_cast<std::uint8_t>(family);
                    break;
                }
            }
        }

        if (save_marker_active(restored.roster.primary_marker)) {
            records_[1] =
                read_runner_record_fields(restored.roster.primary_record);
            names_[1] = read_runner_name(restored.roster.primary_record);
            mark_slot_active(records_[1]);
            active_slots_[1] = true;
            if (restored.roster.primary_marker > 0U &&
                restored.roster.primary_marker <= 10U) {
                portrait_asset_families_[1] = static_cast<std::uint8_t>(
                    3U + restored.roster.primary_marker - 1U);
            }
        }
        if (save_marker_active(restored.roster.secondary_marker)) {
            records_[2] =
                read_runner_record_fields(restored.roster.secondary_record);
            names_[2] = read_runner_name(restored.roster.secondary_record);
            mark_slot_active(records_[2]);
            active_slots_[2] = true;
            if (restored.roster.secondary_marker > 0U &&
                restored.roster.secondary_marker <= 10U) {
                portrait_asset_families_[2] = static_cast<std::uint8_t>(
                    3U + restored.roster.secondary_marker - 1U);
            }
        }
        source_ = Source::restored_save;
        set_active_slot(std::min(restored.active_runner_slot, kSlotCount - 1U));
        return source_;
    }

    if (startup_state.joshua_selection.has_value()) {
        records_[0] = startup_state.joshua_selection->typed_live_joshua_record;
        LiveRunnerRecord raw_joshua{};
        raw_joshua.bytes = startup_state.joshua_selection->live_joshua_record;
        names_[0] = read_runner_name(raw_joshua);
        mark_slot_active(records_[0]);
        active_slots_[0] = true;
        const auto selected = static_cast<std::uint8_t>(
            startup_state.joshua_archetype.selected);
        portrait_asset_families_[0] = selected < 3U ? selected : 0U;
        source_ = Source::new_game_joshua;
        active_slot_ = 0U;
        return source_;
    }

    return source_;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
