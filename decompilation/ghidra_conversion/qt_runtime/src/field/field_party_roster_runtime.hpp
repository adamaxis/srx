#pragma once

#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

// The source field menu always scans three live-record slots.  This is the
// Qt composition-side projection of that roster, not a second game rules
// implementation: record contents still come from the converted startup or
// save-record routines, and the converted menu routines consume this array.
class FieldPartyRosterRuntime final {
public:
    static constexpr std::size_t kSlotCount = 3U;

    enum class Source {
        unavailable,
        new_game_joshua,
        restored_save,
    };

    // Builds the three typed field records from the already-converted startup
    // state.  Save templates are not treated as active party members: only
    // the signed primary/secondary save markers can activate those slots.
    Source initialize(
        const integration::StartupIntegrationState& startup_state) noexcept;

    void clear() noexcept;

    Source source() const noexcept { return source_; }
    bool available() const noexcept { return source_ != Source::unavailable; }
    std::size_t active_slot() const noexcept { return active_slot_; }
    void set_active_slot(std::size_t slot) noexcept;
    std::uint8_t primary_compact_marker() const noexcept {
        return primary_compact_marker_;
    }
    std::uint8_t secondary_compact_marker() const noexcept {
        return secondary_compact_marker_;
    }
    bool restore_saved_party(
        const std::array<RunnerRecordFields, kSlotCount>& records,
        const std::array<std::string, kSlotCount>& names,
        const std::array<bool, kSlotCount>& active_slots,
        const std::array<std::uint8_t, kSlotCount>& portrait_families,
        std::uint8_t primary_compact_marker,
        std::uint8_t secondary_compact_marker,
        std::size_t active_slot) noexcept;
    void install_party_member_for_dump(std::size_t slot,
                                       RunnerRecordFields record,
                                       std::string name,
                                       std::uint8_t portrait_asset_family = 0U);
    void set_compact_markers_for_dump(std::uint8_t primary_marker,
                                      std::uint8_t secondary_marker) noexcept {
        primary_compact_marker_ = primary_marker;
        secondary_compact_marker_ = secondary_marker;
    }
    // Clear the compact roster membership marker represented by this host
    // slot. The live record remains available for field/world state.
    bool deactivate_party_member(std::size_t slot) noexcept;

    bool slot_active(std::size_t slot) const noexcept {
        return slot < active_slots_.size() && active_slots_[slot];
    }

    std::uint8_t portrait_asset_family(std::size_t slot) const noexcept {
        return slot < portrait_asset_families_.size()
                   ? portrait_asset_families_[slot]
                   : 0U;
    }

    const std::string& name(std::size_t slot) const noexcept {
        static const std::string kEmptyName;
        return slot < names_.size() ? names_[slot] : kEmptyName;
    }

    std::array<RunnerRecordFields, kSlotCount>& records() noexcept {
        return records_;
    }
    const std::array<RunnerRecordFields, kSlotCount>& records() const noexcept {
        return records_;
    }

    RunnerRecordFields& active_record() noexcept {
        return records_[active_slot_];
    }
    const RunnerRecordFields& active_record() const noexcept {
        return records_[active_slot_];
    }

private:
    static std::string read_runner_name(
        const LiveRunnerRecord& record);
    static void mark_slot_active(RunnerRecordFields& record) noexcept;

    std::array<RunnerRecordFields, kSlotCount> records_{};
    std::array<std::string, kSlotCount> names_{};
    std::array<bool, kSlotCount> active_slots_{};
    std::array<std::uint8_t, kSlotCount> portrait_asset_families_{};
    std::size_t active_slot_ = 0U;
    std::uint8_t primary_compact_marker_ = 0xFFU;
    std::uint8_t secondary_compact_marker_ = 0xFFU;
    Source source_ = Source::unavailable;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
