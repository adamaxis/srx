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
    active_slot_ = 0U;
    source_ = Source::unavailable;
}

void FieldPartyRosterRuntime::mark_slot_active(
    RunnerRecordFields& record) noexcept {
    // The converted Clips/field roster scans +0x10 bit 0.  The startup
    // archetype record is a template result, so the field handoff publishes
    // the source's active-slot bit here instead of fabricating a second record.
    record.flags = static_cast<std::uint16_t>(record.flags | 0x0001U);
}

void FieldPartyRosterRuntime::set_active_slot(const std::size_t slot) noexcept {
    if (slot < active_slots_.size() && active_slots_[slot]) {
        active_slot_ = slot;
        return;
    }
    active_slot_ = 0U;
}

FieldPartyRosterRuntime::Source FieldPartyRosterRuntime::initialize(
    const integration::StartupIntegrationState& startup_state) noexcept {
    clear();

    if (startup_state.restored_field_state.has_value()) {
        const auto& restored = *startup_state.restored_field_state;
        records_[0] = read_runner_record_fields(restored.roster.default_record);
        names_[0] = read_runner_name(restored.roster.default_record);
        mark_slot_active(records_[0]);
        active_slots_[0] = true;

        if (save_marker_active(restored.roster.primary_marker)) {
            records_[1] =
                read_runner_record_fields(restored.roster.primary_record);
            names_[1] = read_runner_name(restored.roster.primary_record);
            mark_slot_active(records_[1]);
            active_slots_[1] = true;
        }
        if (save_marker_active(restored.roster.secondary_marker)) {
            records_[2] =
                read_runner_record_fields(restored.roster.secondary_record);
            names_[2] = read_runner_name(restored.roster.secondary_record);
            mark_slot_active(records_[2]);
            active_slots_[2] = true;
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
        source_ = Source::new_game_joshua;
        active_slot_ = 0U;
        return source_;
    }

    return source_;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
