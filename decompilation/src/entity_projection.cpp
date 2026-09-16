#include "shadowrun/entity_projection.hpp"

namespace shadowrun {
namespace {

std::int16_t arithmetic_shift_right_word(std::int16_t value,
                                          unsigned int amount) {
    const auto raw = static_cast<std::uint16_t>(value);
    const auto shifted = static_cast<std::uint16_t>(raw >> amount);
    if (value >= 0) {
        return static_cast<std::int16_t>(shifted);
    }

    const auto sign_fill = static_cast<std::uint16_t>(0xFFFFU <<
                                                       (16U - amount));
    return static_cast<std::int16_t>(shifted | sign_fill);
}

std::uint16_t subtract_word(std::uint16_t left, std::uint16_t right) {
    return static_cast<std::uint16_t>(left - right);
}

std::uint16_t add_word(std::uint16_t left, std::int16_t right) {
    return static_cast<std::uint16_t>(left +
                                      static_cast<std::uint16_t>(right));
}

} // namespace

EntityProjectionResult project_entity_field(const EntityProjectionInput& input,
                                             std::uint16_t camera_x,
                                             std::uint16_t camera_y) {
    const auto x_difference = subtract_word(input.word_00, camera_x);
    auto x_delta = arithmetic_shift_right_word(
        static_cast<std::int16_t>(x_difference), 4);
    if (((x_difference >> 3U) & 1U) != 0) {
        ++x_delta;
    }

    const auto screen_x = add_word(0x0080U, x_delta);

    const auto y_difference = subtract_word(input.word_04, camera_y);
    auto y_delta = arithmetic_shift_right_word(
        static_cast<std::int16_t>(y_difference), 4);
    if (((y_difference >> 3U) & 1U) != 0) {
        ++y_delta;
    }

    const auto screen_y = add_word(
        add_word(input.word_08, y_delta), static_cast<std::int16_t>(0x0080));
    const auto signed_x = static_cast<std::int16_t>(screen_x);
    const auto signed_y = static_cast<std::int16_t>(screen_y);
    const bool visible = signed_x > 0x006F && signed_x < 0x01AE &&
                         signed_y > 0x0062 && signed_y < 0x017A;

    return {
        screen_x,
        visible ? screen_y : static_cast<std::uint16_t>(0),
        visible,
        visible ? static_cast<std::uint16_t>(1) : screen_x,
    };
}

AlternateEntityProjectionResult project_alternate_entity_field(
    const EntityProjectionInput& input,
    std::uint16_t camera_x,
    std::uint16_t camera_y) {
    const auto x_difference = subtract_word(input.word_00, camera_x);
    auto x_delta = arithmetic_shift_right_word(
        static_cast<std::int16_t>(x_difference), 4);
    if (((x_difference >> 3U) & 1U) != 0) {
        ++x_delta;
    }

    const auto screen_x = add_word(0x0080U, x_delta);
    const auto signed_x = static_cast<std::int16_t>(screen_x);
    if (signed_x < 0 || screen_x >= 0x0200U) {
        return {screen_x, 0, false, screen_x};
    }

    const auto y_difference = subtract_word(input.word_04, camera_y);
    auto y_delta = arithmetic_shift_right_word(
        static_cast<std::int16_t>(y_difference), 4);
    if (((y_difference >> 3U) & 1U) != 0) {
        ++y_delta;
    }

    const auto screen_y = add_word(
        add_word(input.word_08, y_delta), static_cast<std::int16_t>(0x0080));
    const auto signed_y = static_cast<std::int16_t>(screen_y);
    if (signed_y < 0 || screen_y >= 0x01A0U) {
        return {screen_x, 0, false, screen_x};
    }

    return {screen_x, screen_y, true, screen_x};
}

std::uint16_t project_entity_record(GenesisBus& bus,
                                    GenesisAddress record_address,
                                    std::uint16_t camera_x,
                                    std::uint16_t camera_y) {
    const EntityProjectionInput input{
        bus.read16(record_address),
        bus.read16(record_address + 4U),
        bus.read16(record_address + 8U)
    };
    if (bus.read32(record_address + 0x3AU) ==
        kAlternateProjectionRoutine) {
        const auto result = project_alternate_entity_field(
            input, camera_x, camera_y);
        const auto signed_x = static_cast<std::int16_t>(result.screen_x);
        if (signed_x >= 0 && result.screen_x < 0x0200U) {
            bus.write16(record_address + 0x0AU, result.screen_x);
        }
        bus.write16(record_address + 0x0CU, result.screen_y);
        return result.return_value;
    }

    const auto result = project_entity_field(input, camera_x, camera_y);
    bus.write16(record_address + 0x0AU, result.screen_x);
    bus.write16(record_address + 0x0CU, result.screen_y);
    return result.return_value;
}

EntityProjectionResult project_entity_slot(RawEntitySlot& slot,
                                            std::uint16_t camera_x,
                                            std::uint16_t camera_y) {
    const auto result = project_entity_field(
        {slot.word_00, slot.word_04, slot.word_08}, camera_x, camera_y);
    slot.word_0A = result.screen_x;
    slot.word_0C = result.screen_y;
    return result;
}

std::size_t project_active_entity_slots(RawEntitySlot* slots,
                                        std::size_t slot_capacity,
                                        std::uint16_t camera_x,
                                        std::uint16_t camera_y) {
    if (slots == nullptr) {
        return 0;
    }

    std::size_t visible_count = 0;
    const auto count = slot_capacity < kEntitySlotCount
        ? slot_capacity
        : kEntitySlotCount;
    for (std::size_t index = 0; index < count; ++index) {
        if ((slots[index].flags_10 & 1U) == 0) {
            continue;
        }
        if (project_entity_slot(slots[index], camera_x, camera_y).visible) {
            ++visible_count;
        }
    }
    return visible_count;
}

} // namespace shadowrun
