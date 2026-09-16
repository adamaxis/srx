#include "shadowrun/entity_state.hpp"

#include "shadowrun/object_list.hpp"
#include "shadowrun/rom_layout.hpp"
#include "shadowrun/runtime.hpp"
#include "shadowrun/session.hpp"
#include "shadowrun/z80_transport.hpp"

#include <cstddef>

namespace shadowrun {
namespace {

EntityStateStreamResult make_empty_stream_result() {
    return {false, false, 0, 0, 0, 0, 0};
}

// ADD.B followed by BGT uses the 68000's signed overflow flags. For two
// signed bytes, the resulting BGT condition is equivalent to testing the
// mathematical signed sum, while the stored byte remains the low eight bits.
bool add_byte_is_positive(std::uint8_t left, std::uint8_t right) {
    return static_cast<std::int16_t>(static_cast<std::int8_t>(left)) +
               static_cast<std::int16_t>(static_cast<std::int8_t>(right)) >
        0;
}

std::uint32_t state_grid_command(GenesisBus& bus,
                                 GenesisAddress entity_record_address,
                                 std::uint32_t zero_state,
                                 std::uint32_t one_state,
                                 std::uint32_t other_state) {
    const auto selector = bus.read16(entity_record_address + 0x3EU);
    if (selector == 0) {
        return zero_state;
    }
    if (selector == 1) {
        return one_state;
    }
    return other_state;
}

void stream_state_grid(GenesisBus& bus,
                       EntityStateStreamResult& result) {
    ShadowrunRuntime(bus).stream_vdp_grid(
        result.source, result.command, result.inner_dbf_counter,
        result.outer_dbf_counter, 0x6000U);
    result.streamed = true;
}

void update_animation_wake_timer_0xABB6(GenesisBus& bus) {
    // ROM 0xABB6 tests the three fixed entity records without changing their
    // flags. The bit-test branches are kept in their original order because a
    // set bit 0 skips the corresponding bit-6 test.
    const auto first_flags = bus.read8(session_ram::kEntityPhaseBase + 0x10U);
    if ((first_flags & 0x40U) == 0) {
        return;
    }
    const auto second_flags = bus.read8(
        session_ram::kEntityPhaseBase + 0x110U);
    if ((second_flags & 1U) != 0 && (second_flags & 0x40U) == 0) {
        return;
    }
    const auto third_flags = bus.read8(
        session_ram::kEntityPhaseBase + 0x210U);
    if ((third_flags & 1U) != 0 && (third_flags & 0x40U) == 0) {
        return;
    }
    bus.write16(ram::kEntityAnimationWakeTimer, 0x3CU);
}

} // namespace

EntityStateStreamResult update_entity_state_0x3674(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t delta) {
    auto result = make_empty_stream_result();
    auto phase = bus.read8(entity_record_address + 0x4CU);
    if ((bus.read8(entity_record_address + 0x10U) & 1U) != 0) {
        const auto next = static_cast<std::uint8_t>(phase + delta);
        if (add_byte_is_positive(phase, delta)) {
            phase = next;
            // CMP.B #10 uses the signed byte result. The normal game data
            // stays in 0..10, but retain the instruction's signed compare.
            if (static_cast<std::int8_t>(phase) > 10) {
                phase = 10;
            }
        } else {
            phase = 0;
        }
    } else {
        phase = 0;
    }
    bus.write8(entity_record_address + 0x4CU, phase);

    result.phase = phase;
    result.source = static_cast<GenesisAddress>(
        rom::kEntityStateGridPositionTable +
        static_cast<GenesisAddress>(phase) * 8U);
    result.command = state_grid_command(bus, entity_record_address,
                                        0xF1C4U, 0xF5C4U, 0xF9C4U);
    result.inner_dbf_counter = 0;
    result.outer_dbf_counter = 3;
    stream_state_grid(bus, result);
    return result;
}

EntityStateStreamResult update_entity_state_0x37EC(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t delta) {
    auto result = make_empty_stream_result();
    if ((bus.read8(entity_record_address + 0x10U) & 1U) != 0) {
        auto phase = bus.read8(entity_record_address + 0x9CU);
        const auto next = static_cast<std::uint8_t>(phase + delta);
        if (add_byte_is_positive(phase, delta)) {
            phase = next;
            if (static_cast<std::int8_t>(phase) > 10) {
                phase = 10;
            }
        } else {
            // 0x382E copies the wrapped byte to D1 before calling 0x3674.
            phase = next;
            const auto nested = update_entity_state_0x3674(
                bus, entity_record_address, phase);
            result.nested_0x3674_call = nested.streamed;
            phase = 0;
        }
        bus.write8(entity_record_address + 0x9CU, phase);
    } else {
        bus.write8(entity_record_address + 0x9CU, 0);
    }

    const auto table = bus.read32(ram::kEntityObjectControlPointer) ==
            entity_record_address
        ? rom::kEntityStateGridAnimationTableControl
        : rom::kEntityStateGridAnimationTableDefault;
    const auto phase = bus.read8(entity_record_address + 0x9CU);
    result.phase = phase;
    result.source = static_cast<GenesisAddress>(
        table + static_cast<GenesisAddress>(phase) * 8U);
    result.command = state_grid_command(bus, entity_record_address,
                                        0xF3C6U, 0xF7C6U, 0xFBC6U);
    result.inner_dbf_counter = 3;
    result.outer_dbf_counter = 0;
    stream_state_grid(bus, result);
    return result;
}

EntityAuxiliaryStateResult update_entity_auxiliary_state_0x55C2C(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress source_record_address) {
    const auto source_delta = bus.read8(source_record_address + 0x9FU);
    const auto delta = source_delta != 0 ? source_delta : 1U;

    EntityAuxiliaryStateResult result{
        source_delta != 0,
        make_empty_stream_result(),
        make_empty_stream_result(),
        0,
        0
    };
    result.position_stream = update_entity_state_0x3674(
        bus, entity_record_address, delta);
    if (bus.read8(entity_record_address + 0x9CU) == 0) {
        result.animation_stream = update_entity_state_0x37EC(
            bus, entity_record_address, 1);
    }

    // 0x55C56, 0x55C5C, and 0x55C62 are BSET #4/#6/#0,(0x10,A0).
    auto flags = bus.read8(entity_record_address + 0x10U);
    flags = static_cast<std::uint8_t>(flags | 0x51U);
    bus.write8(entity_record_address + 0x10U, flags);
    bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);

    result.flags_after = flags;
    result.field_0x3A_after = bus.read32(entity_record_address + 0x3AU);
    return result;
}

std::uint16_t adjust_entity_cost_0x55E62(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5) {
    if ((bus.read8(entity_record_address + 0x11U) & 0x20U) != 0) {
        return static_cast<std::uint16_t>(d5 - 2U);
    }
    for (std::size_t index = 0; index < 8; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            entity_record_address + index);
        if (bus.read8(record_address + 0x66U) == 0x27U) {
            const auto reduction = static_cast<std::uint16_t>(
                bus.read8(record_address + 0x6EU) >> 1U);
            return static_cast<std::uint16_t>(d5 - reduction);
        }
    }
    return d5;
}

std::uint16_t subtract_entity_modifier(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5,
    GenesisAddress modifier_offset) {
    const auto kind = bus.read8(entity_record_address + 0x76U);
    if (kind == 0) {
        return d5;
    }
    const auto table_entry = static_cast<GenesisAddress>(
        rom::kEntityStateModifierTable +
        static_cast<GenesisAddress>(kind - 1U) * 0x30U + modifier_offset);
    const auto modifier = bus.read8(table_entry);
    const auto low = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(d5) - modifier);
    return static_cast<std::uint16_t>((d5 & 0xFF00U) | low);
}

std::uint16_t subtract_entity_modifier_0x55E92(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5) {
    return subtract_entity_modifier(bus, entity_record_address, d5, 0x25U);
}

std::uint16_t subtract_entity_modifier_0x55EB4(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5) {
    return subtract_entity_modifier(bus, entity_record_address, d5, 0x26U);
}

bool modifier_subtraction_is_positive(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5,
    GenesisAddress modifier_offset) {
    const auto kind = bus.read8(entity_record_address + 0x76U);
    if (kind == 0) {
        return static_cast<std::int8_t>(static_cast<std::uint8_t>(d5)) > 0;
    }
    const auto table_entry = static_cast<GenesisAddress>(
        rom::kEntityStateModifierTable +
        static_cast<GenesisAddress>(kind - 1U) * 0x30U + modifier_offset);
    return static_cast<std::int16_t>(
        static_cast<std::int8_t>(static_cast<std::uint8_t>(d5))) -
        static_cast<std::int16_t>(static_cast<std::int8_t>(
            bus.read8(table_entry))) > 0;
}

std::uint16_t calculate_entity_base_0x55ED6(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    auto value = static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 0x78U) +
        bus.read8(entity_record_address + 0x7BU) +
        bus.read8(entity_record_address + 0x7CU));
    value = static_cast<std::uint8_t>(value >> 1U);
    for (std::size_t index = 0; index < 8; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            entity_record_address + index);
        if (bus.read8(record_address + 0x66U) == 0x28U) {
            value = static_cast<std::uint8_t>(
                value + bus.read8(record_address + 0x6EU));
            break;
        }
    }
    return value;
}

EntityRateResult calculate_entity_rate_0x55F20(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d1,
    bool d7_negative) {
    const auto factor = static_cast<std::uint32_t>(
        bus.read8(entity_record_address + 0x4DU)) * 0x19U;
    const auto quotient = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(d1) * factor) / 0x64U);
    const auto residual = static_cast<std::uint16_t>(d1 - quotient);
    return {quotient, residual, d7_negative ? residual : quotient};
}

std::uint16_t calculate_entity_half_base_0xD81C(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    const auto sum = static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 0x78U) +
        bus.read8(entity_record_address + 0x7BU));
    return static_cast<std::uint16_t>(sum >> 1U);
}

EntityScalingResult scale_entity_value_0x55F44(
    GenesisBus& bus,
    std::uint16_t d1) {
    const auto control_entity = bus.read32(ram::kEntityObjectControlPointer);
    const auto half_base = calculate_entity_half_base_0xD81C(
        bus, control_entity);

    // 0x55F44 performs ADD.B operations on D6.  Keep the upper word bits
    // untouched if an input ever makes the low byte wrap.
    const auto add_low_byte = [](std::uint16_t value,
                                 std::uint8_t addend) {
        const auto low = static_cast<std::uint8_t>(
            static_cast<std::uint8_t>(value) + addend);
        return static_cast<std::uint16_t>((value & 0xFF00U) | low);
    };

    auto adjusted = add_low_byte(
        half_base, bus.read8(control_entity + 0x88U));
    const auto global_base = bus.read8(ram::kEntityStateScalingBase + 7U);
    adjusted = add_low_byte(adjusted, global_base);
    adjusted = add_low_byte(adjusted, global_base);

    const auto mode = static_cast<std::int8_t>(
        bus.read8(ram::kEntityStateScalingMode));
    std::uint16_t selected = adjusted;
    if (mode > 0) {
        const auto third = static_cast<std::uint16_t>(adjusted / 3U);
        const auto two_thirds = static_cast<std::uint16_t>(adjusted - third);
        selected = static_cast<std::int16_t>(d1) == 0
            ? third
            : two_thirds;
    } else if (mode < 0) {
        const auto third = static_cast<std::uint16_t>(adjusted / 3U);
        const auto two_thirds = static_cast<std::uint16_t>(adjusted - third);
        selected = static_cast<std::int16_t>(d1) < 0
            ? third
            : two_thirds;
    } else {
        const auto odd = (adjusted & 1U) != 0;
        selected = static_cast<std::uint16_t>(adjusted >> 1U);
        if (odd && d1 != 0) {
            selected = static_cast<std::uint16_t>(selected + 1U);
        }
    }

    if (static_cast<std::int16_t>(d1) < 0) {
        const auto cap = static_cast<std::uint16_t>(
            bus.read8(ram::kEntityStateScalingBase));
        if (static_cast<std::int16_t>(selected - cap) > 0) {
            selected = cap;
        }
    }
    return {half_base, adjusted, selected};
}

EntityAnimationStartResult initialize_entity_animation_0xB330(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    // 0xB330 temporarily marks bit 6 while 0xABB6 decides whether the fixed
    // three-record state has reached its wake point. It then clears that bit,
    // sends the record-type command, and installs the alternate animation.
    bus.write8(entity_record_address + 0x10U, static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 0x10U) | 0x40U));
    update_animation_wake_timer_0xABB6(bus);
    bus.write8(entity_record_address + 0x10U, static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 0x10U) & ~0x40U));

    const auto command_offset = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x0EU) * 2U);
    const auto command = bus.read16(
        rom::kEntityAnimationFallbackCommandTable + command_offset);
    if (command != 0) {
        dispatch_z80_command_0x530DE(bus, static_cast<std::uint8_t>(command));
    }

    bus.write8(entity_record_address + 0x10U, static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 0x10U) | 0x10U));
    bus.write32(entity_record_address + 0x3AU,
                rom::kEntityAnimationFallbackRoutine);
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bus.read16(entity_record_address + 0x2EU)
                                   << 3U) |
        static_cast<std::uint16_t>(bus.read16(entity_record_address + 0x2EU)
                                   >> 13U));
    if (static_cast<std::int16_t>(rotated) < 0) {
        rotated = static_cast<std::uint16_t>(rotated + 1U);
    }
    const auto table_index = static_cast<GenesisAddress>(
        static_cast<std::uint16_t>(rotated & 7U) * 2U);
    const auto selected_frame = bus.read16(
        rom::kEntityAnimationFallbackFrameTable + table_index);
    bus.write16(entity_record_address + 0x16U, selected_frame);
    bus.write16(entity_record_address + 0x1AU, 0);
    bus.write8(entity_record_address + 0x11U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0x11U) | 1U));
    bus.write16(entity_record_address + 0x1EU, 0);
    return {true, false, false, selected_frame,
            bus.read8(entity_record_address + 0xCEU),
            bus.read8(entity_record_address + 0x10U),
            bus.read32(entity_record_address + 0x3AU)};
}

EntityAnimationStartResult initialize_entity_animation_0xAFF2(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    const auto flags = bus.read8(entity_record_address + 0x10U);
    if ((flags & 0x10U) != 0) {
        return {false, true, false, bus.read16(entity_record_address + 0x16U),
                bus.read8(entity_record_address + 0xCEU), flags,
                bus.read32(entity_record_address + 0x3AU)};
    }
    if (bus.read8(entity_record_address + 0x4CU) == 0) {
        return initialize_entity_animation_0xB330(
            bus, entity_record_address);
    }

    // 0xB006 rounds the top three bits of the rotated phase word before
    // reducing it to one of the eight word entries at 0xC072.
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bus.read16(entity_record_address + 0x2EU)
                                   << 3U) |
        static_cast<std::uint16_t>(bus.read16(entity_record_address + 0x2EU)
                                   >> 13U));
    if (static_cast<std::int16_t>(rotated) < 0) {
        rotated = static_cast<std::uint16_t>(rotated + 1U);
    }
    const auto table_index = static_cast<GenesisAddress>(
        static_cast<std::uint16_t>(rotated & 7U) * 2U);
    const auto selected_frame = bus.read16(
        rom::kEntityAnimationFrameTable + table_index);

    bus.write32(entity_record_address + 0x3AU, 0x0000B07EU);
    bus.write16(entity_record_address + 0x16U, selected_frame);
    bus.write16(entity_record_address + 0x1AU, 0);
    bus.write8(entity_record_address + 0x11U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0x11U) | 1U));
    bus.write16(entity_record_address + 0x1EU, 0);
    bus.write8(entity_record_address + 0xCEU, 8);
    return {true, false, false, selected_frame,
            bus.read8(entity_record_address + 0xCEU),
            bus.read8(entity_record_address + 0x10U),
            bus.read32(entity_record_address + 0x3AU)};
}

std::uint8_t roll_entity_threshold_0x0DBA(
    GenesisBus& bus,
    std::uint16_t d4,
    std::uint16_t d6) {
    // 0x0DBA changes a zero or negative post-SUBQ D6 into one DBF pass.
    const auto signed_count = static_cast<std::int16_t>(d6);
    const auto iterations = signed_count > 0
        ? static_cast<std::size_t>(signed_count)
        : static_cast<std::size_t>(1);
    std::uint8_t bucket = 0;
    for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
        std::uint16_t skipped = 0;
        std::uint16_t value = 0;
        do {
            value = static_cast<std::uint16_t>(
                ShadowrunRuntime(bus).random_below(6) + 1U);
            if (value == 6) {
                skipped = static_cast<std::uint16_t>(skipped + value);
            }
        } while (value == 6);
        value = static_cast<std::uint16_t>(value + skipped);
        if (static_cast<std::int16_t>(value - d4) >= 0) {
            bucket = static_cast<std::uint8_t>(bucket + 1U);
        }
    }
    bus.write8(ram::kEntityStateRandomBucket, bucket);
    return bucket;
}

std::uint16_t adjust_entity_timing_0x55740(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress target_record_address,
    std::uint16_t d4) {
    const auto flags = bus.read16(entity_record_address + 0x90U);
    const auto bit_one = (flags & 2U) != 0;
    if (bit_one) {
        d4 = static_cast<std::uint16_t>(d4 - 1U);
    }
    if (bus.read8(target_record_address + 0x9BU) == 0x16U) {
        return static_cast<std::uint16_t>(d4 + 2U);
    }
    if (bit_one || bus.read8(entity_record_address + 0x9AU) == 4U ||
        bus.read8(entity_record_address + 0x9AU) == 2U ||
        (bus.read8(target_record_address + 0x11U) & 0x80U) != 0) {
        return d4;
    }
    return static_cast<std::uint16_t>(d4 + 4U);
}

GenesisAddress select_entity_parameter_block_0x55828(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    const auto auxiliary = bus.read32(entity_record_address + 0xD8U);
    const auto index = bus.read16(auxiliary + 0xA0U);
    return static_cast<GenesisAddress>(
        0x00FFF9FAU + static_cast<GenesisAddress>(index) * 0x22U);
}

std::uint8_t calculate_entity_phase_value_0x55F02(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    auto value = bus.read8(entity_record_address + 0x80U);
    for (int index = 6; index >= 0; --index) {
        const auto lane = static_cast<GenesisAddress>(
            entity_record_address + 0x66U + index);
        if (bus.read8(lane) == 0x18U) {
            value = static_cast<std::uint8_t>(
                value + bus.read8(lane + 8U));
            break;
        }
    }
    return value;
}

std::uint16_t calculate_global_phase_bonus_0x1047E(GenesisBus& bus) {
    std::uint16_t result = 0;
    for (std::size_t index = 0; index < 3; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase + index * 0x100U);
        if ((bus.read8(record_address + 0x10U) & 1U) == 0 ||
            static_cast<std::int8_t>(bus.read8(record_address + 0x9BU)) > 1) {
            continue;
        }
        const auto value = calculate_entity_phase_value_0x55F02(
            bus, record_address);
        result = static_cast<std::uint16_t>(result + value / 4U +
            ((value % 4U) >= 3U ? 1U : 0U));
    }
    return result;
}

std::uint16_t adjust_entity_timing_0x55786(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d4,
    std::uint16_t d6) {
    const auto flags = bus.read8(entity_record_address + 0xDFU);
    const bool first_path = (flags & 1U) != 0;
    const bool second_path = (flags & 2U) != 0;
    if (!first_path && !second_path) {
        return d4;
    }

    const auto parameter_block = select_entity_parameter_block_0x55828(
        bus, entity_record_address);
    const auto field_d0 = bus.read8(parameter_block +
        (first_path ? 0x1EU : 0x20U));
    const auto field_d4 = bus.read8(parameter_block +
        (first_path ? 0x1FU : 0x21U));
    const auto table_row = first_path ? 5U : 12U;
    if (static_cast<std::int16_t>(
            bus.read16(entity_record_address + 0x3EU)) < 3) {
        d6 = calculate_global_phase_bonus_0x1047E(bus);
    }

    const auto table_flag = bus.read8(static_cast<GenesisAddress>(
        rom::kEntityTimingAdjustmentTable + table_row * 0x30U + 0x27U));
    const auto addition = bus.read8(entity_record_address +
        (table_flag & 0x80U ? 0x7CU : 0x77U));
    d6 = static_cast<std::uint16_t>((d6 & 0xFF00U) |
        static_cast<std::uint8_t>(static_cast<std::uint8_t>(d6) + addition));
    const auto bucket = roll_entity_threshold_0x0DBA(bus, field_d4, d6);

    auto bonus = static_cast<std::uint8_t>(field_d0 - bucket);
    if (static_cast<std::int8_t>(bonus) <= 0) {
        bonus = 0;
    } else if (second_path) {
        bonus = static_cast<std::uint8_t>(bonus >> 1U);
    }
    return static_cast<std::uint16_t>(d4 + bonus);
}

namespace {

std::uint8_t add_byte(std::uint8_t left, std::uint8_t right) {
    return static_cast<std::uint8_t>(left + right);
}

bool sub_byte_is_positive_result(std::uint8_t left, std::uint8_t right) {
    return static_cast<std::int16_t>(static_cast<std::int8_t>(left)) -
               static_cast<std::int16_t>(static_cast<std::int8_t>(right)) >
        0;
}

void advance_entity_frame_tick(GenesisBus& bus,
                                bool& advanced) {
    auto timer = bus.read8(ram::kEntityStateFrameTimer);
    timer = static_cast<std::uint8_t>(timer - 1U);
    bus.write8(ram::kEntityStateFrameTimer, timer);
    if (static_cast<std::int8_t>(timer) >= 0) {
        return;
    }
    bus.write8(ram::kEntityStateFrameTimer, 0x0FU);
    bus.write8(ram::kEntityStateFrameCounter0,
               static_cast<std::uint8_t>(
                   bus.read8(ram::kEntityStateFrameCounter0) + 1U));
    bus.write8(ram::kEntityStateFrameCounter1,
               static_cast<std::uint8_t>(
                   bus.read8(ram::kEntityStateFrameCounter1) + 1U));
    bus.write8(ram::kEntityStateFrameCounter2,
               static_cast<std::uint8_t>(
                   bus.read8(ram::kEntityStateFrameCounter2) + 1U));
    advanced = true;
}

} // namespace

EntityReactionResult run_entity_reaction_0x55C76(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress target_record_address) {
    EntityReactionResult result{
        false, false, false, false, false, false, 0,
        bus.read8(target_record_address + 0x4CU),
        bus.read8(target_record_address + 0x9CU)
    };

    auto d1 = calculate_entity_base_0x55ED6(bus, entity_record_address);
    auto d6 = calculate_entity_rate_0x55F20(
        bus, entity_record_address, d1, false).selected_result;
    auto d7 = static_cast<std::uint16_t>(
        bus.read8(entity_record_address + 0x85U));
    if (d7 < d6) {
        d6 = d7;
    }
    d6 = static_cast<std::uint16_t>(d6 + d7);
    if (d6 == 0) {
        d6 = 1;
    }
    auto d4 = adjust_entity_timing_0x55740(
        bus, entity_record_address, target_record_address, 4);
    d4 = adjust_entity_timing_0x55786(
        bus, entity_record_address, d4, d6);
    if (static_cast<std::int16_t>(d4) < 2) {
        d4 = 2;
    }
    auto bucket = roll_entity_threshold_0x0DBA(bus, d4, d6);
    result.random_bucket = bucket;
    auto d5 = static_cast<std::uint16_t>(bucket);
    if (d5 == 0) {
        return result;
    }
    result.first_threshold_passed = true;
    d5 = static_cast<std::uint16_t>(d5 + 3U);
    const auto modified_d5 = subtract_entity_modifier_0x55EB4(
        bus, target_record_address, d5);
    if (!modifier_subtraction_is_positive(
            bus, target_record_address, d5, 0x26U)) {
        return result;
    }
    d5 = modified_d5;
    d5 = adjust_entity_cost_0x55E62(bus, entity_record_address, d5);
    if (static_cast<std::int16_t>(d5) <= 0) {
        return result;
    }

    d1 = calculate_entity_base_0x55ED6(bus, target_record_address);
    d6 = calculate_entity_rate_0x55F20(
        bus, target_record_address, d1, false).selected_result;
    d7 = static_cast<std::uint16_t>(
        bus.read8(target_record_address + 0x85U));
    if (d7 < d6) {
        d6 = d7;
    }
    if (bus.read8(target_record_address + 0x57U) == 0) {
        d6 = static_cast<std::uint16_t>(d6 + d7);
    }
    if (d6 == 0) {
        d6 = 1;
    }
    d4 = adjust_entity_timing_0x55740(
        bus, target_record_address, entity_record_address, 4);
    d4 = adjust_entity_timing_0x55786(
        bus, target_record_address, d4, d6);
    if (static_cast<std::int16_t>(d4) < 2) {
        d4 = 2;
    }
    bucket = roll_entity_threshold_0x0DBA(bus, d4, d6);
    result.random_bucket = bucket;
    const auto old_d5_byte = static_cast<std::uint8_t>(d5);
    d5 = static_cast<std::uint16_t>(d5 - bucket);
    if (!sub_byte_is_positive_result(old_d5_byte, bucket)) {
        return result;
    }
    result.second_threshold_passed = true;

    d4 = bus.read8(entity_record_address + 0x79U);
    auto control = bus.read16(entity_record_address + 0x90U);
    d6 = 5;
    d7 = 3;
    for (;;) {
        if ((control & static_cast<std::uint16_t>(1U << d6)) != 0) {
            d6 = static_cast<std::uint16_t>(d6 - 4U);
            d4 = static_cast<std::uint16_t>(d4 + d6);
            break;
        }
        d6 = static_cast<std::uint16_t>(d6 + 1U);
        if (d7-- == 0) {
            break;
        }
    }

    d6 = calculate_entity_rate_0x55F20(
        bus, target_record_address,
        calculate_entity_base_0x55ED6(bus, target_record_address), true)
        .selected_result;
    d6 = adjust_entity_timing_0x55786(
        bus, target_record_address, d4, d6);
    d6 = static_cast<std::uint16_t>(
        d6 + bus.read8(target_record_address + 0x77U));

    control = bus.read16(target_record_address + 0x90U);
    for (std::uint16_t bit = 9, passes = 3; passes != 0; ++bit, --passes) {
        if ((control & static_cast<std::uint16_t>(1U << bit)) != 0) {
            d6 = static_cast<std::uint16_t>(
                d6 + static_cast<std::uint16_t>(bit - 8U));
            break;
        }
    }
    if (static_cast<std::int16_t>(d4) < 2) {
        d4 = 2;
    }
    bucket = roll_entity_threshold_0x0DBA(bus, d4, d6);
    result.random_bucket = bucket;
    const auto final_d5_byte = static_cast<std::uint8_t>(d5);
    d5 = static_cast<std::uint16_t>(d5 - bucket);
    if (!sub_byte_is_positive_result(final_d5_byte, bucket)) {
        return result;
    }
    result.third_threshold_passed = true;

    const auto target_control = bus.read16(target_record_address + 0x90U);
    if ((target_control & 8U) == 0 && (target_control & 4U) == 0) {
        d5 = static_cast<std::uint16_t>(d5 - 2U);
        if (static_cast<std::int8_t>(static_cast<std::uint8_t>(d5)) <= 0) {
            return result;
        }
    }
    const auto delta = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(0U - d5));
    if ((target_control & 8U) != 0 || (target_control & 4U) != 0) {
        if (static_cast<std::int16_t>(
                bus.read16(target_record_address + 0x3EU)) < 3) {
            if (bus.read8(ram::kEntityStateGlobalPhase) == 3U) {
                return result;
            }
            result.position_update_called = true;
            update_entity_state_0x3674(
                bus, target_record_address, delta);
            result.final_position_phase = bus.read8(
                target_record_address + 0x4CU);
            result.final_animation_phase = bus.read8(
                target_record_address + 0x9CU);
            return result;
        }
        auto update_delta = delta;
        if (static_cast<std::int8_t>(
                bus.read8(target_record_address + 0x9AU)) < 0) {
            update_delta = 0xFFU;
        }
        const auto updated = add_byte(
            bus.read8(target_record_address + 0x4CU), update_delta);
        bus.write8(target_record_address + 0x4CU, updated);
        if (static_cast<std::int8_t>(updated) < 0) {
            bus.write8(target_record_address + 0x4CU, 0);
            advance_entity_frame_tick(bus, result.frame_tick_advanced);
        }
        result.final_position_phase = bus.read8(
            target_record_address + 0x4CU);
    } else {
        if (static_cast<std::int16_t>(
                bus.read16(target_record_address + 0x3EU)) < 3) {
            if (bus.read8(ram::kEntityStateGlobalPhase) == 3U) {
                return result;
            }
            result.animation_update_called = true;
            update_entity_state_0x37EC(
                bus, target_record_address, delta);
            result.final_animation_phase = bus.read8(
                target_record_address + 0x9CU);
            result.final_position_phase = bus.read8(
                target_record_address + 0x4CU);
            return result;
        }
        auto update_delta = delta;
        if (static_cast<std::int8_t>(
                bus.read8(target_record_address + 0x9AU)) < 0) {
            update_delta = 0xFFU;
        }
        const auto updated = add_byte(
            bus.read8(target_record_address + 0x9CU), update_delta);
        bus.write8(target_record_address + 0x9CU, updated);
        if (static_cast<std::int8_t>(updated) < 0) {
            bus.write8(target_record_address + 0x9CU, 0);
        }
        advance_entity_frame_tick(bus, result.frame_tick_advanced);
        result.final_position_phase = bus.read8(
            target_record_address + 0x4CU);
        result.final_animation_phase = bus.read8(
            target_record_address + 0x9CU);
    }
    return result;
}

} // namespace shadowrun
