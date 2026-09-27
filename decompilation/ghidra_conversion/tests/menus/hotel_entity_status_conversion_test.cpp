#include "shadowrun_ghidra/menus/hotel_entity_status_conversion.hpp"

#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<HotelEntityStatusRecord, kHotelEntityStatusRecordCount>
        records{{
            {static_cast<std::uint8_t>(kHotelEntityActiveFlag |
                                       kHotelEntityServiceFlag),
             2U, "Jake"},
            {kHotelEntityServiceFlag, 0U, "Unused"},
            {static_cast<std::uint8_t>(kHotelEntityActiveFlag |
                                       kHotelEntityServiceFlag),
             5U, "T-Bird"},
        }};

    std::vector<std::string> order;
    std::vector<std::tuple<std::size_t, std::uint8_t, std::string>>
        behavior_events;
    std::vector<std::tuple<std::size_t, std::uint8_t, std::string>> delay_events;
    std::vector<std::pair<std::size_t, std::string>> names;
    std::vector<std::size_t> medical_rows;
    HotelEntityStatusCallbacks callbacks{};
    callbacks.write_hotel_entity_status_grid =
        [&] { order.emplace_back("grid"); };
    callbacks.advance_behavior_meter =
        [&](const std::size_t index, HotelEntityStatusRecord& record,
            const std::uint8_t delta) {
            order.emplace_back("behavior");
            behavior_events.emplace_back(index, delta, record.display_name);
        };
    callbacks.advance_delay_meter =
        [&](const std::size_t index, HotelEntityStatusRecord& record,
            const std::uint8_t delta) {
            order.emplace_back("delay");
            delay_events.emplace_back(index, delta, record.display_name);
            record.delay_meter = static_cast<std::uint8_t>(record.delay_meter +
                                                            delta);
        };
    callbacks.render_entity_name =
        [&](const std::size_t index, const std::string& name) {
            order.emplace_back("name");
            names.emplace_back(index, name);
        };
    callbacks.render_medical_attention_suffix =
        [&](const std::size_t index) {
            order.emplace_back("medical");
            medical_rows.push_back(index);
        };
    callbacks.render_hotel_recovery_message =
        [&] { order.emplace_back("recovery"); };
    callbacks.wait_for_hotel_entity_status_acknowledgement =
        [&] { order.emplace_back("wait"); };

    service_hotel_entity_status(records, callbacks);

    if (!expect(order == std::vector<std::string>{
                           "grid", "behavior", "name", "medical", "delay",
                           "behavior", "recovery", "wait"}) ||
        !expect(behavior_events.size() == 2U &&
                std::get<0>(behavior_events[0]) == 0U &&
                std::get<1>(behavior_events[0]) == 10U &&
                std::get<0>(behavior_events[1]) == 2U &&
                std::get<1>(behavior_events[1]) == 10U) ||
        !expect(delay_events.size() == 1U &&
                std::get<0>(delay_events[0]) == 2U &&
                std::get<1>(delay_events[0]) == 2U) ||
        !expect(names == std::vector<std::pair<std::size_t, std::string>>{
                           {0U, "Jake"}}) ||
        !expect(medical_rows == std::vector<std::size_t>{0U}) ||
        !expect(records[0].status_flags ==
                    (kHotelEntityActiveFlag | kHotelEntityServiceFlag)) ||
        !expect(records[2].delay_meter == 7U &&
                records[2].status_flags == kHotelEntityActiveFlag)) {
        return 1;
    }

    records[0].delay_meter = 3U;
    records[0].status_flags =
        static_cast<std::uint8_t>(kHotelEntityActiveFlag |
                                  kHotelEntityServiceFlag);
    records[2].status_flags = 0U;
    HotelEntityStatusCallbacks threshold_callbacks{};
    std::uint8_t delay_delta = 0U;
    threshold_callbacks.advance_delay_meter =
        [&](const std::size_t, HotelEntityStatusRecord&,
            const std::uint8_t delta) { delay_delta = delta; };
    service_hotel_entity_status(records, threshold_callbacks);
    if (!expect(delay_delta == 1U &&
                records[0].status_flags == kHotelEntityActiveFlag)) {
        return 2;
    }

    return 0;
}
