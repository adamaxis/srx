#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kHotelEntityStatusRecordCount = 3U;
constexpr std::uint8_t kHotelEntityActiveFlag = 0x01U;
constexpr std::uint8_t kHotelEntityServiceFlag = 0x10U;

// The Hotel service walks the three live party records in source order. The
// native conversion keeps the fields it actually consumes typed: flags, the
// entity delay meter, and the already-extracted display name. It does not
// expose the source FF0100/FF0200/FF0300 addresses.
struct HotelEntityStatusRecord {
    std::uint8_t status_flags = 0;
    std::uint8_t delay_meter = 0;
    std::string display_name;
};

using HotelEntityMeterBoundary =
    std::function<void(std::size_t, HotelEntityStatusRecord&, std::uint8_t)>;

struct HotelEntityStatusCallbacks {
    // ROM 0xEF08 begins with the Hotel party-status grid submission.
    std::function<void()> write_hotel_entity_status_grid;

    // ROM 0x37EC and 0x3674 remain explicit typed service boundaries. The
    // callbacks receive the record so the central layer can bind them to the
    // shared meter implementation without reproducing that callee here.
    HotelEntityMeterBoundary advance_behavior_meter;
    HotelEntityMeterBoundary advance_delay_meter;

    // ROM 0xEF5E renders the record name at the row's name column, then
    // renders the fixed " needs medical attention!" suffix at the row start.
    std::function<void(std::size_t, const std::string&)> render_entity_name;
    std::function<void(std::size_t)> render_medical_attention_suffix;

    // ROM 0xEFA8 renders the fixed Hotel recovery/soykaf message after all
    // three records have been serviced.
    std::function<void()> render_hotel_recovery_message;

    // ROM 0xEFAE tails directly into 0xF312, which waits for a controller
    // event and performs the source's final sample.
    std::function<void()> wait_for_hotel_entity_status_acknowledgement;
};

// ROM 0xEF08: update the three party records shown by the Hotel status pass.
// Delay-meter values 0..2 receive only behavior progress and the medical
// attention row. Values 3..4 receive delay +1, values 5..255 receive delay
// +2; both recovery branches also receive behavior +10 and clear source flag
// bit 4. The fixed recovery message and controller wait follow the loop.
void service_hotel_entity_status(
    std::array<HotelEntityStatusRecord, kHotelEntityStatusRecordCount>&
        records,
    const HotelEntityStatusCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
