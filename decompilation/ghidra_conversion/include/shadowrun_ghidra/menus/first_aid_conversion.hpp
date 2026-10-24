#pragma once

#include <cstdint>
#include <functional>
#include <array>

namespace shadowrun::ghidra_conversion {

// The First Aid menu passes the active runner's +0x89 value to the source
// roll routine. This is the proven skill input; the host model does not
// expose the original live-record pointer.
struct FirstAidUserState {
    std::uint8_t biotech_roll_count = 0;
};

// ROM 0x1253E consumes these target fields. The names describe their source
// meter roles, not a guessed complete runner schema.
struct FirstAidTargetState {
    std::uint8_t physical_meter = 0;  // source +0x4C
    std::uint8_t mental_meter = 0;    // source +0x9C
    std::uint8_t body_attribute = 0;  // source +0x77
    std::uint8_t flags = 0;           // low byte of source +0x10
    std::uint32_t continuation = 0;   // source +0x3A
};

enum class FirstAidResult {
    treated,
    target_already_full,
    roll_service_unavailable,
};

struct FirstAidRollRequest {
    // This is the source D6 input to 0x0DBA. The source roll helper still
    // performs its minimum one iteration when this value is zero.
    std::uint8_t roll_count = 0;
    std::uint8_t difficulty = 0;
};

struct FirstAidResolution {
    FirstAidResult result = FirstAidResult::roll_service_unavailable;
    FirstAidRollRequest roll{};
    std::uint8_t successes = 0;
    std::uint8_t applied_delta = 0;
};

// 0x0DBA is an RNG-backed source service. The callback receives the exact
// source roll count and threshold and returns the source FFF0DD success byte.
using FirstAidRollService =
    std::function<std::uint8_t(std::uint8_t roll_count,
                               std::uint8_t difficulty)>;

// ROM 0x1253E: reject a target whose two meters are both ten, derive the
// target difficulty from +0x4C and +0x77, use the source roll result (or one
// when it is zero), advance the two target meters, clear status bits 6 and 4,
// set bit 0, and install continuation 0xB45C.
FirstAidResolution apply_first_aid_menu_effect(
    const FirstAidUserState& user,
    FirstAidTargetState& target,
    const FirstAidRollService& roll_service);

// ROM 0x12298/0x1231C: the target panel enumerates the three live party
// records in source order and appends one CANCEL row. Only +0x10 bit 0 is
// consumed by this list builder.
struct FirstAidTargetList {
    std::array<std::size_t, 3> active_record_indices{};
    std::size_t active_count = 0;

    std::size_t row_count() const { return active_count + 1U; }
    std::size_t cancel_row() const { return active_count; }
};

FirstAidTargetList build_first_aid_target_list(
    const std::array<std::uint8_t, 3>& record_flags);

// The source C376 cursor wraps over active targets plus the appended CANCEL
// row. The input delta is the signed Up/Down step supplied by the controller.
std::size_t advance_first_aid_target_cursor(
    std::size_t current_row,
    std::size_t active_count,
    std::int8_t delta);

enum class FirstAidItemResult {
    used,
    target_not_injured,
    invalid_item,
    empty_item,
    roll_service_unavailable,
};

enum class FirstAidMessage {
    none,
    medkit_used,
    slap_patch_used,
    target_not_injured,
};

struct FirstAidItemResolution {
    FirstAidItemResult result = FirstAidItemResult::invalid_item;
    FirstAidMessage message = FirstAidMessage::none;
    std::uint8_t applied_delta = 0;
};

// ROM 0x12466/0x1247E/0x124AE plus the 0x1253E medkit leaf. Item codes
// 0x10..0x12 are the source First Aid entries. A phase byte is consumed
// only after a target effect succeeds; a final phase clears the item code as
// 0x1275C does for a medical cell.
FirstAidItemResolution use_first_aid_item(
    std::uint8_t& item_code,
    std::uint8_t& phase,
    const FirstAidUserState& user,
    FirstAidTargetState& target,
    const FirstAidRollService& roll_service);

} // namespace shadowrun::ghidra_conversion
