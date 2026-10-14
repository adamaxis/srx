#include "shadowrun/entities/object_list.hpp"

#include "shadowrun/title_intro/boot.hpp"
#include "shadowrun/entities/entity_projection.hpp"
#include "shadowrun/entities/entity_state.hpp"
#include "shadowrun/entities/object_mask.hpp"
#include "shadowrun/core/rom_layout.hpp"
#include "shadowrun/core/runtime.hpp"
#include "shadowrun/session/session.hpp"
#include "shadowrun/audio/z80_transport.hpp"

namespace shadowrun {
namespace {

std::int16_t add_word(std::int16_t left, std::int16_t right) {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(left) +
                                     static_cast<std::uint16_t>(right));
}

std::uint16_t add_word_unsigned(std::uint16_t left, std::uint16_t right) {
    return static_cast<std::uint16_t>(left + right);
}

} // namespace

void initialise_object_output(GenesisBus& bus) {
    ShadowrunRuntime runtime(bus);
    runtime.write_vdp_command_sequence(
        bus.read16(ram::kObjectListVdpCommand));
    bus.write32(genesis::kVdpData, 0);
    bus.write32(genesis::kVdpData, 0);
    bus.write32(ram::kObjectListOutputPointer,
                ram::kObjectListInitialOutput);
    bus.write16(ram::kObjectListCountMirror, 1);
    bus.write16(ram::kObjectListScratch, 0);
}

void append_descriptor_object(GenesisBus& bus,
                              GenesisAddress descriptor_address,
                              GenesisAddress source_record_address) {
    const auto output_address = bus.read32(ram::kObjectOutputPointer);
    bus.write32(output_address, bus.read32(descriptor_address + 2U));
    bus.write16(output_address + 4U, bus.read16(descriptor_address + 6U));
    bus.write16(output_address + 6U, static_cast<std::uint16_t>(
        bus.read16(source_record_address + 0x2AU) << 5U));

    // ADDQ.B #8,(0xFFDF35).L modifies the low byte of the longword pointer;
    // the byte write deliberately preserves the ROM's wraparound behavior.
    bus.write8(ram::kObjectOutputCursorByte,
               static_cast<std::uint8_t>(
                   bus.read8(ram::kObjectOutputCursorByte) + 8U));
    bus.write16(ram::kObjectOutputCount,
                static_cast<std::uint16_t>(
                    bus.read16(ram::kObjectOutputCount) + 1U));
}

EntityObjectOutputResult append_entity_state_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint16_t ordinal,
    std::size_t max_records) {
    // ROM 0x622C peeks at the first state word. The 0x8002 marker selects the
    // alternate list eight bytes farther on and adds entity +0x20 to each
    // emitted word at output +4.
    auto state_address = bus.read32(entity_record_address + 0x12U);
    bool alternate_list = false;
    if (bus.read16(state_address) == 0x8002U) {
        alternate_list = true;
        state_address += 8U;
    }

    auto next_output = output_address;
    auto count = bus.read16(ram::kObjectListCount);
    std::size_t emitted = 0;
    while (emitted < max_records && bus.read16(state_address) != 0x8000U) {
        const auto x = static_cast<std::uint16_t>(
            bus.read16(state_address) + bus.read16(entity_record_address + 0x0AU));
        const auto y = static_cast<std::uint16_t>(
            bus.read16(state_address + 2U) +
            bus.read16(entity_record_address + 0x0CU));
        bus.write16(next_output + 6U, x);
        bus.write16(next_output, y);
        bus.write8(next_output + 2U,
                   static_cast<std::uint8_t>(bus.read16(state_address + 4U)));
        bus.write8(next_output + 3U, static_cast<std::uint8_t>(ordinal));

        auto flags = bus.read16(state_address + 6U);
        if ((bus.read8(entity_record_address + 0x11U) & 0x80U) == 0) {
            flags = static_cast<std::uint16_t>(
                flags | bus.read16(entity_record_address + 0x2CU));
        }
        if (alternate_list) {
            flags = static_cast<std::uint16_t>(
                flags + bus.read16(entity_record_address + 0x20U));
        }
        bus.write16(next_output + 4U, flags);

        ++ordinal;
        ++count;
        ++emitted;
        state_address += 8U;
        next_output += 8U;
        bus.write16(ram::kObjectListCount, count);
        bus.write32(ram::kObjectListOutputPointer, next_output);
        bus.write16(ram::kObjectListCountMirror, count);
    }

    return {emitted, next_output, count, ordinal};
}

GeneratedObjectListResult generate_object_list_frame(
    GenesisBus& bus,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    auto progress = static_cast<std::uint16_t>(
        bus.read16(ram::kObjectGenerationProgress) + 8U);
    bus.write16(ram::kObjectGenerationProgress, progress);
    if (progress >= 0x00A0U) {
        bus.write16(ram::kObjectGenerationReset, 0);
        return {false, 0, 0, progress,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    const auto selector = static_cast<std::uint16_t>(
        (bus.read16(ram::kFrameCounter) >> 2U) & 3U);
    const auto source_list = bus.read32(
        rom::kObjectGenerationPointerTable + selector * 4U);
    auto output_address = bus.read32(ram::kObjectListOutputPointer);
    auto ordinal = first_ordinal;
    std::size_t objects_emitted = 0;

    const auto high_product_word = [](std::uint16_t sample,
                                      std::uint16_t multiplier) {
        const auto product = static_cast<std::int32_t>(
            static_cast<std::int16_t>(sample)) *
            static_cast<std::int32_t>(static_cast<std::int16_t>(multiplier));
        return static_cast<std::int16_t>(
            static_cast<std::uint32_t>(product) >> 16U);
    };

    for (std::size_t row = 0; row < 32; ++row) {
        const auto table_offset = static_cast<GenesisAddress>(row * 8U);
        const auto x_delta = high_product_word(
            bus.read16(rom::kObjectGenerationInterpolationTable + table_offset),
            progress);
        const auto y_delta = high_product_word(
            bus.read16(rom::kObjectGenerationInterpolationTable +
                       table_offset + 0x40U), progress);
        const auto x_base = static_cast<std::int16_t>(static_cast<std::uint16_t>(
            bus.read16(ram::kObjectGenerationBaseX) +
            static_cast<std::uint16_t>(x_delta)));
        const auto y_base = static_cast<std::int16_t>(static_cast<std::uint16_t>(
            bus.read16(ram::kObjectGenerationBaseY) +
            static_cast<std::uint16_t>(y_delta)));
        const auto built = build_object_list_from_bus(
            bus, source_list, output_address, max_records_per_list,
            y_base, x_base, 0x6000U, 0, ordinal,
            ObjectListVariant::Base);
        output_address = built.next_output_address;
        ordinal = built.next_ordinal;
        objects_emitted += built.built;
    }

    return {true, 32, objects_emitted, progress,
            bus.read16(ram::kObjectListCount), ordinal};
}

AnimatedEntityOutputResult emit_animated_entity_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    // The first compare/branch pair is ROM 0x5B98's early-out gate.
    if (bus.read16(entity_record_address + 0x38U) == 0x002AU) {
        return {false, 0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    const auto scale = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x44U) - 8U);
    if (static_cast<std::int16_t>(scale) < 0) {
        return {false, 0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    const auto pointer_offset = static_cast<GenesisAddress>(
        static_cast<std::uint16_t>(scale & 3U) << 2U);
    const auto source_list = bus.read32(
        rom::kAnimatedObjectPointerTable + pointer_offset);
    auto next_output = output_address;
    auto ordinal = first_ordinal;
    std::size_t objects_emitted = 0;

    for (std::size_t row = 0; row < 16; ++row) {
        // ROM uses MULU.W followed by ADD.W, so only the low product word is
        // observable in each word-sized base calculation.
        const auto x_factor = bus.read16(
            rom::kAnimatedObjectFactorTable + row * 4U);
        const auto y_factor = bus.read16(
            rom::kAnimatedObjectFactorTable + row * 4U + 2U);
        const auto x_delta = static_cast<std::uint16_t>(x_factor * scale);
        const auto y_delta = static_cast<std::uint16_t>(y_factor * scale);
        const auto x_base = static_cast<std::int16_t>(static_cast<std::uint16_t>(
            bus.read16(entity_record_address + 0x0AU) + x_delta));
        const auto y_base = static_cast<std::int16_t>(static_cast<std::uint16_t>(
            bus.read16(entity_record_address + 0x0CU) + y_delta));
        const auto list = build_object_list_from_bus(
            bus, source_list, next_output, max_records_per_list,
            y_base, x_base, bus.read16(entity_record_address + 0x2CU), 0,
            ordinal, ObjectListVariant::Base);
        next_output = list.next_output_address;
        ordinal = list.next_ordinal;
        objects_emitted += list.built;
    }

    return {true, 16, objects_emitted, next_output,
            bus.read16(ram::kObjectListCount), ordinal};
}

EntityVariantOutputResult emit_entity_variant_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    auto selector = static_cast<std::uint16_t>(
        bus.read8(entity_record_address + 0x4CU));
    const auto control = bus.read32(ram::kEntityObjectControlPointer);
    const auto control_state = bus.read16(control + 0x56U);
    const auto control_mode = bus.read16(control + 0x57U);

    if (static_cast<std::int16_t>(control_state) < 0) {
        if (control_mode == 5U) {
            selector = bus.read8(entity_record_address + 0x9CU);
        }
    } else if (control_mode == 0U) {
        auto control_flags = bus.read8(control + 0x91U);
        const bool bit3_was_set = (control_flags & 0x08U) != 0;
        control_flags = static_cast<std::uint8_t>(control_flags | 0x08U);
        bus.write8(control + 0x91U, control_flags);
        if (!bit3_was_set) {
            const bool bit2_was_set = (control_flags & 0x04U) != 0;
            control_flags = static_cast<std::uint8_t>(control_flags | 0x04U);
            bus.write8(control + 0x91U, control_flags);
            if (!bit2_was_set) {
                selector = bus.read8(entity_record_address + 0x9CU);
            }
        }
    }

    const auto selector_word = selector;
    std::size_t table_index = 0;
    if (static_cast<std::int16_t>(selector_word) < 3) {
        table_index = 0;
    } else if (static_cast<std::int16_t>(selector_word) < 6) {
        table_index = 1;
    } else if (static_cast<std::int16_t>(selector_word) < 9) {
        table_index = 2;
    } else {
        table_index = 3;
    }
    const auto table_address = rom::kEntityObjectVariantTables[table_index];
    const auto object_flags = bus.read16(table_address);
    const auto pointer_block = table_address + 2U;

    auto x_base = bus.read16(entity_record_address + 0x0AU);
    auto y_base = bus.read16(entity_record_address + 0x0CU);
    if (bus.read16(entity_record_address + 0x0EU) == 0x000FU) {
        y_base = static_cast<std::uint16_t>(y_base - 0x20U);
    }
    auto state = bus.read16(entity_record_address + 0xA6U);
    auto next_output = output_address;
    auto ordinal = first_ordinal;
    std::size_t lists_processed = 0;
    std::size_t objects_emitted = 0;

    const auto emit_one = [&](GenesisAddress source_list,
                              std::uint16_t x,
                              std::uint16_t y,
                              GenesisAddress& output,
                              std::uint8_t& next_ordinal) {
        const auto list = build_object_list_from_bus(
            bus, source_list, output,
            max_records_per_list,
            static_cast<std::int16_t>(y),
            static_cast<std::int16_t>(x),
            object_flags, 0, next_ordinal, ObjectListVariant::Base);
        output = list.next_output_address;
        next_ordinal = list.next_ordinal;
        objects_emitted += list.built;
        ++lists_processed;
    };

    // ROM tests the sign of +0xA6, subtracts one, and branches on the
    // subtraction carry before entering its four-list offset case. A word
    // smaller than one is the only 68000 SUBQ.W #1 borrow case; because the
    // preceding sign test excludes zero, the four-list path is normally
    // unreachable but is retained literally for malformed/alternate state.
    const bool state_negative = static_cast<std::int16_t>(state) < 0;
    bool four_list_case = false;
    if (state_negative) {
        const auto decremented = static_cast<std::uint16_t>(state - 1U);
        const bool subtraction_borrow = state < 1U;
        if (!subtraction_borrow) {
            bus.write16(entity_record_address + 0xA6U, 0);
        } else {
            state = decremented;
            bus.write16(entity_record_address + 0xA6U, state);
            four_list_case = true;
        }
    }

    if (four_list_case) {
        const auto offset = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(state & 0x7FU) << 3U);
        emit_one(bus.read32(pointer_block),
                 static_cast<std::uint16_t>(x_base - offset),
                 static_cast<std::uint16_t>(y_base - offset),
                 next_output, ordinal);
        emit_one(bus.read32(pointer_block + 4U),
                 static_cast<std::uint16_t>(x_base + offset),
                 static_cast<std::uint16_t>(y_base - offset),
                 next_output, ordinal);
        emit_one(bus.read32(pointer_block + 8U),
                 static_cast<std::uint16_t>(x_base - offset),
                 static_cast<std::uint16_t>(y_base + offset),
                 next_output, ordinal);
        emit_one(bus.read32(pointer_block + 12U),
                 static_cast<std::uint16_t>(x_base + offset),
                 static_cast<std::uint16_t>(y_base + offset),
                 next_output, ordinal);
    } else {
        // 0x5D76 adds 0x10 to A5 after its post-incremented D6 word, so the
        // normal path reads the fourth pointer at table+0x12.
        emit_one(bus.read32(pointer_block + 0x10U),
                 x_base, y_base, next_output, ordinal);
    }

    return {true, static_cast<std::uint8_t>(selector_word), table_address,
            lists_processed, objects_emitted, next_output,
            bus.read16(ram::kObjectListCount), ordinal};
}

EntityCycleOutputResult emit_entity_cycle_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    // ROM 0x4E8E decrements the cycle byte. A nonnegative result enters the
    // descriptor/list path; an underflow resets the byte and advances the
    // phase byte before deciding whether to clear indexed state and set bit 6.
    const auto timer_address = entity_record_address + 2U;
    const auto timer = bus.read8(timer_address);
    const auto timer_after = static_cast<std::uint8_t>(timer - 1U);
    bus.write8(timer_address, timer_after);
    if (static_cast<std::int8_t>(timer_after) < 0) {
        bus.write8(timer_address, 6);
        const auto phase_address = entity_record_address + 3U;
        const auto phase = static_cast<std::uint8_t>(
            bus.read8(phase_address) + 1U);
        bus.write8(phase_address, phase);
        if (static_cast<std::int8_t>(phase) >= 4) {
            clear_indexed_object_byte(
                bus, bus.read16(entity_record_address + 0x32U));
            bus.write8(entity_record_address + 0x11U,
                       static_cast<std::uint8_t>(
                           bus.read8(entity_record_address + 0x11U) | 0x40U));
        }
        return {false, 0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    // D0 is shifted three places, conditionally incremented on its sign, then
    // reduced to a three-bit pointer-table index. All operations are words.
    auto table_index_word = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x2EU) << 3U);
    if (static_cast<std::int16_t>(table_index_word) < 0) {
        table_index_word = static_cast<std::uint16_t>(table_index_word + 1U);
    }
    const auto table_offset = static_cast<GenesisAddress>(
        static_cast<std::uint16_t>(table_index_word & 7U) << 2U);
    const auto descriptor_table = bus.read32(
        rom::kEntityCyclePointerTable + table_offset);

    const auto signed_phase = static_cast<std::int16_t>(
        static_cast<std::int8_t>(bus.read8(entity_record_address + 3U)));
    const auto descriptor = bus.read32(
        static_cast<GenesisAddress>(
            static_cast<std::int32_t>(descriptor_table) + signed_phase * 4));
    append_descriptor_object(bus, descriptor, entity_record_address);

    // A2 now holds the selected descriptor itself. The ROM advances that
    // register after 0x61A0, so the list begins eight bytes into the selected
    // descriptor, not eight bytes into the outer table.
    const auto list_address = static_cast<GenesisAddress>(descriptor + 8U);
    auto word6_base = bus.read16(entity_record_address + 0x0AU);
    auto word0_base = bus.read16(entity_record_address + 0x0CU);
    const auto inherited_d4 = static_cast<std::uint16_t>(0);
    word6_base = static_cast<std::uint16_t>(word6_base + inherited_d4);
    word6_base = static_cast<std::uint16_t>(
        word6_base + bus.read16(rom::kEntityCycleOffsetTable));
    word0_base = static_cast<std::uint16_t>(
        word0_base + bus.read16(rom::kEntityCycleOffsetTable + 2U));
    const auto list = build_object_list_from_bus(
        bus, list_address, output_address, max_records_per_list,
        static_cast<std::int16_t>(word0_base),
        static_cast<std::int16_t>(word6_base), 0x2000U,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x2AU)),
        first_ordinal, ObjectListVariant::WithWord3Offset);
    return {true, 1, list.built, list.next_output_address,
            list.object_count, list.next_ordinal};
}

EntitySlotObjectOutputResult emit_entity_slot_objects(
    GenesisBus& bus,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    auto next_output = output_address;
    auto ordinal = first_ordinal;
    std::size_t active_slots = 0;
    std::size_t projected_slots = 0;
    std::size_t objects_emitted = 0;

    for (std::size_t index = 0; index < kEntitySlotCount; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            kEntitySlotBase + index * kEntitySlotStride);
        if ((bus.read8(record_address + 0x10U) & 1U) == 0) {
            continue;
        }
        ++active_slots;

        const EntityProjectionInput input{
            bus.read16(record_address),
            bus.read16(record_address + 4U),
            bus.read16(record_address + 8U)};
        const auto projected = project_entity_field(
            input, bus.read16(ram::kCameraX), bus.read16(ram::kCameraY));
        bus.write16(record_address + 0x0AU, projected.screen_x);
        bus.write16(record_address + 0x0CU, projected.screen_y);
        if (bus.read16(record_address + 0x0CU) == 0) {
            continue;
        }
        ++projected_slots;

        const auto object_type = bus.read16(record_address + 2U);
        GenesisAddress source_list = 0;
        if (static_cast<std::int16_t>(object_type) < 0) {
            const auto high_byte = static_cast<std::uint16_t>(
                object_type & 0xFF00U);
            source_list = high_byte == 0xC000U
                ? rom::kEntitySlotObjectListNegativeA
                : rom::kEntitySlotObjectListNegativeB;
        } else if (object_type < 0x0063U) {
            const auto table_offset = static_cast<GenesisAddress>(
                static_cast<std::uint16_t>(object_type << 2U));
            source_list = bus.read32(
                rom::kEntitySlotObjectPointerTable + table_offset);
        } else if (object_type == 0x0064U) {
            source_list = rom::kEntitySlotObjectListType64;
        } else {
            source_list = rom::kEntitySlotObjectListOther;
        }

        const auto list = build_object_list_from_bus(
            bus, source_list, next_output, max_records_per_list,
            static_cast<std::int16_t>(bus.read16(record_address + 0x0CU)),
            static_cast<std::int16_t>(bus.read16(record_address + 0x0AU)),
            0x2000U, 0, ordinal, ObjectListVariant::Base);
        next_output = list.next_output_address;
        ordinal = list.next_ordinal;
        objects_emitted += list.built;
    }

    return {kEntitySlotCount, active_slots, projected_slots, objects_emitted,
            next_output, bus.read16(ram::kObjectListCount), ordinal};
}

namespace {

void advance_entity_phase_timer(GenesisBus& bus,
                                GenesisAddress entity_record_address) {
    const auto timer_address = entity_record_address + 6U;
    const auto timer_after = static_cast<std::uint8_t>(
        bus.read8(timer_address) - 1U);
    bus.write8(timer_address, timer_after);
    if (static_cast<std::int8_t>(timer_after) < 0) {
        bus.write8(timer_address, 3);
        bus.write8(entity_record_address + 7U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 7U) + 1U));
        if (static_cast<std::int8_t>(
                bus.read8(entity_record_address + 7U)) >= 4) {
            bus.write8(entity_record_address + 7U, 0);
        }
    }
}

EntityPhaseObjectOutputResult emit_entity_phase_lists(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list,
    bool paired_lists) {
    const auto phase = static_cast<std::uint16_t>(
        bus.read8(entity_record_address + 7U) & 3U);
    const auto source_list = bus.read32(
        rom::kEntityPhaseObjectPointerTable + phase * 4U);
    auto next_output = output_address;
    auto ordinal = first_ordinal;
    std::size_t objects_emitted = 0;
    std::size_t lists_processed = 0;

    const auto emit_one = [&](std::uint16_t x_base,
                              std::uint16_t y_base) {
        const auto list = build_object_list_from_bus(
            bus, source_list, next_output, max_records_per_list,
            static_cast<std::int16_t>(y_base),
            static_cast<std::int16_t>(x_base), 0x4000U, 0, ordinal,
            ObjectListVariant::Base);
        next_output = list.next_output_address;
        ordinal = list.next_ordinal;
        objects_emitted += list.built;
        ++lists_processed;
    };

    const auto x_base = bus.read16(entity_record_address + 0x0AU);
    const auto y_base = bus.read16(entity_record_address + 0x0CU);
    if (paired_lists) {
        emit_one(static_cast<std::uint16_t>(x_base - 0x2EU),
                 static_cast<std::uint16_t>(y_base - 0x20U));
        emit_one(static_cast<std::uint16_t>(x_base + 0x29U),
                 static_cast<std::uint16_t>(y_base - 0x20U));
    } else {
        emit_one(x_base, y_base);
    }

    return {lists_processed, objects_emitted, next_output,
            bus.read16(ram::kObjectListCount), ordinal};
}

} // namespace

EntityPhaseObjectOutputResult emit_entity_phase_objects_0x5936(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    // ROM selects the paired form for +0x0E == 0x000F. Otherwise the byte
    // timer/phase update at 0x5952-0x596E selects the ordinary form until
    // the phase rolls past three.
    if (bus.read16(entity_record_address + 0x0EU) != 0x000FU) {
        advance_entity_phase_timer(bus, entity_record_address);
        return emit_entity_phase_lists(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list, false);
    } else {
        advance_entity_phase_timer(bus, entity_record_address);
    }

    return emit_entity_phase_lists(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list, true);
}

EntityPhaseObjectOutputResult emit_entity_phase_objects_0x59EA(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    advance_entity_phase_timer(bus, entity_record_address);
    return emit_entity_phase_lists(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list, true);
}

EntityPhaseObjectOutputResult emit_entity_timed_objects_0x5662(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    auto timer = bus.read8(entity_record_address + 2U);
    timer = static_cast<std::uint8_t>(timer - 1U);
    bus.write8(entity_record_address + 2U, timer);
    bool descriptor_needed = false;
    if (static_cast<std::int8_t>(timer) < 0) {
        bus.write8(entity_record_address + 2U, 4);
        bus.write8(entity_record_address + 3U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 3U) + 1U));
        descriptor_needed = true;
    }

    const auto phase_byte = bus.read8(entity_record_address + 3U);
    const auto signed_phase = static_cast<std::int16_t>(
        static_cast<std::int8_t>(phase_byte));
    if (signed_phase >= 5) {
        bus.write8(entity_record_address + 0xB2U, 0);
        return {0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    const auto pointer_offset = static_cast<GenesisAddress>(
        static_cast<std::int32_t>(signed_phase) * 4);
    const auto descriptor = bus.read32(static_cast<GenesisAddress>(
        static_cast<std::int32_t>(rom::kEntityTimedObjectPointerTable) +
        static_cast<std::int32_t>(pointer_offset)));
    if (descriptor_needed) {
        append_descriptor_object(bus, descriptor, entity_record_address);
    }

    auto x_base = bus.read16(entity_record_address + 0x0AU);
    auto y_base = bus.read16(entity_record_address + 0x0CU);
    const auto phase_delta = static_cast<std::int16_t>(signed_phase - 4);
    if (phase_delta >= 0) {
        y_base = static_cast<std::uint16_t>(
            y_base + static_cast<std::uint16_t>(phase_delta << 2U));
        x_base = static_cast<std::uint16_t>(
            x_base + static_cast<std::uint16_t>(
                bus.read16(ram::kRandomState) & 1U));
    }

    const auto list = build_object_list_from_bus(
        bus, static_cast<GenesisAddress>(descriptor + 8U), output_address,
        max_records_per_list, static_cast<std::int16_t>(y_base),
        static_cast<std::int16_t>(x_base), 0x6000U,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x2AU)),
        first_ordinal, ObjectListVariant::WithWord3Offset);
    return {1, list.built, list.next_output_address,
            list.object_count, list.next_ordinal};
}

EntityPhaseObjectOutputResult emit_entity_state_objects_0x56DE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    // ROM 0x56DE starts with CLR.W D1, decrements the byte timer at +2, and
    // marks a timer wrap so 0x61A0 appends the newly selected descriptor.
    auto timer = static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 2U) - 1U);
    bus.write8(entity_record_address + 2U, timer);
    bool descriptor_needed = false;
    if (static_cast<std::int8_t>(timer) < 0) {
        bus.write8(entity_record_address + 2U, 4);
        bus.write8(entity_record_address + 3U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 3U) + 1U));
        descriptor_needed = true;
    }

    const auto signed_phase = static_cast<std::int16_t>(
        static_cast<std::int8_t>(bus.read8(entity_record_address + 3U)));
    if (signed_phase >= 16) {
        // ROM 0x5704 clears the dispatch state and 0x5708 calls 0x1A304
        // with the indexed-object selector at +0x32. A1 then receives the
        // auxiliary pointer at +0xD8 before entering 0x55C2C.
        bus.write8(entity_record_address + 0xB2U, 0);
        clear_indexed_object_byte(
            bus, bus.read16(entity_record_address + 0x32U));
        update_entity_auxiliary_state_0x55C2C(
            bus, entity_record_address,
            bus.read32(entity_record_address + 0xD8U));
        return {0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    const auto descriptor = bus.read32(static_cast<GenesisAddress>(
        static_cast<std::int32_t>(rom::kEntityTimedObjectPointerTable) +
        static_cast<std::int32_t>(signed_phase) * 4));
    if (descriptor_needed) {
        append_descriptor_object(bus, descriptor, entity_record_address);
    }

    const auto list = build_object_list_from_bus(
        bus, static_cast<GenesisAddress>(descriptor + 8U), output_address,
        max_records_per_list,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x0CU)),
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x0AU)),
        0x6000U,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x2AU)),
        first_ordinal, ObjectListVariant::WithWord3Offset);
    return {1, list.built, list.next_output_address,
            list.object_count, list.next_ordinal};
}

namespace {

EntityPhaseObjectOutputResult emit_simple_state_object_path(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list,
    GenesisAddress pointer_table,
    std::uint8_t timer_reset,
    std::int16_t phase_limit,
    std::uint16_t object_flags,
    bool clear_cleanup_byte) {
    auto timer = static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 2U) - 1U);
    bus.write8(entity_record_address + 2U, timer);
    bool descriptor_needed = false;
    if (static_cast<std::int8_t>(timer) < 0) {
        bus.write8(entity_record_address + 2U, timer_reset);
        bus.write8(entity_record_address + 3U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 3U) + 1U));
        descriptor_needed = true;
    }

    const auto signed_phase = static_cast<std::int16_t>(
        static_cast<std::int8_t>(bus.read8(entity_record_address + 3U)));
    if (signed_phase >= phase_limit) {
        bus.write8(entity_record_address + 0xB2U, 0);
        if (clear_cleanup_byte) {
            bus.write8(entity_record_address + 0xDEU, 0);
        }
        clear_indexed_object_byte(
            bus, bus.read16(entity_record_address + 0x32U));
        return {0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    const auto descriptor = bus.read32(static_cast<GenesisAddress>(
        static_cast<std::int32_t>(pointer_table) +
        static_cast<std::int32_t>(signed_phase) * 4));
    if (descriptor_needed) {
        append_descriptor_object(bus, descriptor, entity_record_address);
    }

    const auto list = build_object_list_from_bus(
        bus, static_cast<GenesisAddress>(descriptor + 8U), output_address,
        max_records_per_list,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x0CU)),
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x0AU)),
        object_flags,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x2AU)),
        first_ordinal, ObjectListVariant::WithWord3Offset);
    return {1, list.built, list.next_output_address,
            list.object_count, list.next_ordinal};
}

} // namespace

EntityPhaseObjectOutputResult emit_entity_state_objects_0x5322(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    // ROM 0x5322 uses D1 as a local "descriptor changed" flag. The entity
    // timer is byte-sized; a wrap selects the next phase and resets +2 to 6.
    auto timer = static_cast<std::uint8_t>(
        bus.read8(entity_record_address + 2U) - 1U);
    bus.write8(entity_record_address + 2U, timer);
    bool descriptor_needed = false;
    if (static_cast<std::int8_t>(timer) < 0) {
        bus.write8(entity_record_address + 2U, 6);
        bus.write8(entity_record_address + 3U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 3U) + 1U));
        descriptor_needed = true;
    }

    const auto signed_phase = static_cast<std::int16_t>(
        static_cast<std::int8_t>(bus.read8(entity_record_address + 3U)));
    if (signed_phase >= 9) {
        // 0x5348/0x534C clear the dispatch and per-record cleanup bytes,
        // then call the already reviewed indexed-byte reset. The following
        // 0x55B78 subsystem is deliberately left outside this leaf.
        bus.write8(entity_record_address + 0xB2U, 0);
        bus.write8(entity_record_address + 0xDEU, 0);
        clear_indexed_object_byte(
            bus, bus.read16(entity_record_address + 0x32U));
        return {0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    const auto descriptor = bus.read32(static_cast<GenesisAddress>(
        static_cast<std::int32_t>(rom::kEntityStateObjectPointerTable) +
        static_cast<std::int32_t>(signed_phase) * 4));
    if (descriptor_needed) {
        append_descriptor_object(bus, descriptor, entity_record_address);
    }

    const auto phase_adjustment = static_cast<std::int16_t>(signed_phase + 4);
    auto x_base = bus.read16(entity_record_address + 0x0AU);
    auto y_base = bus.read16(entity_record_address + 0x0CU);
    if (phase_adjustment >= 0) {
        y_base = static_cast<std::uint16_t>(
            y_base + static_cast<std::uint16_t>(phase_adjustment << 2U));
        y_base = static_cast<std::uint16_t>(
            y_base + static_cast<std::uint16_t>(
                bus.read16(ram::kRandomState) & 1U));
    }

    const auto list = build_object_list_from_bus(
        bus, static_cast<GenesisAddress>(descriptor + 8U), output_address,
        max_records_per_list, static_cast<std::int16_t>(y_base),
        static_cast<std::int16_t>(x_base), 0x6000U,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x2AU)),
        first_ordinal, ObjectListVariant::WithWord3Offset);
    return {1, list.built, list.next_output_address,
            list.object_count, list.next_ordinal};
}

EntityPhaseObjectOutputResult emit_entity_state_objects_0x521E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    return emit_simple_state_object_path(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list, rom::kEntityStateObjectPointerTable0x521E,
        1, 4, 0x2000U, false);
}

EntityPhaseObjectOutputResult emit_entity_state_objects_0x508A(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    return emit_simple_state_object_path(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list, rom::kEntityStateObjectPointerTable0x508A,
        1, 3, 0x4000U, false);
}

EntityPhaseObjectOutputResult emit_entity_state_objects_0x4FE2(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    return emit_simple_state_object_path(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list, rom::kEntityStateObjectPointerTable0x4FE2,
        4, 6, 0x4000U, true);
}

EntityPhaseObjectOutputResult emit_entity_state_objects_0x5152(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    return emit_simple_state_object_path(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list, rom::kEntityStateObjectPointerTable0x5152,
        4, 6, 0x2000U, true);
}

EntityDispatchOutputResult dispatch_entity_state_objects_0x490A(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    const auto dispatch_index = bus.read8(entity_record_address + 0xB2U);
    if (dispatch_index >= rom::kEntityDispatchTargets.size()) {
        return {false, 0, 0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    // The table is ROM data, but reading it through the bus keeps this
    // routine useful with both the original image and small table fixtures.
    const auto target = bus.read32(static_cast<GenesisAddress>(
        rom::kEntityDispatchTable +
        static_cast<GenesisAddress>(dispatch_index) * 4U));
    if (target == 0) {
        return {false, target, 0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    if (target == 0x000049A0U) {
        // ROM 0x49A0 is an RTS-only handler.
        return {true, target, 0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    EntityPhaseObjectOutputResult emitted{};
    if (target == 0x0000557EU) {
        emitted = emit_entity_timed_objects_0x557E(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
    } else if (target == 0x000056DEU) {
        emitted = emit_entity_state_objects_0x56DE(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
    } else if (target == 0x000052BEU) {
        emitted = emit_entity_state_objects_0x5322(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
        // ROM 0x52BE returns after 0x5322 when +0xB2 remains nonzero. A
        // cutoff enters the direct ten-record 0x52CC scan.
        if (bus.read8(entity_record_address + 0xB2U) == 0) {
            scan_entity_state_records_0x52CC(bus, entity_record_address);
            return {true, target, emitted.lists_processed,
                    emitted.objects_emitted, emitted.next_output_address,
                    emitted.next_count, emitted.next_ordinal};
        }
    } else if (target == 0x0000521EU) {
        emitted = emit_entity_state_objects_0x521E(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
    } else if (target == 0x0000508AU) {
        emitted = emit_entity_state_objects_0x508A(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
    } else if (target == 0x00004FE2U) {
        emitted = emit_entity_state_objects_0x4FE2(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
    } else if (target == 0x00005152U) {
        emitted = emit_entity_state_objects_0x5152(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
    } else if (target == 0x00004CDEU) {
        emitted = emit_entity_state_objects_0x5152(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
        // 0x4CDE returns after 0x5152 when +0xB2 remains nonzero. A cutoff
        // falls through to the larger 0x4CEC scan.
        if (bus.read8(entity_record_address + 0xB2U) == 0) {
            const auto continuation = scan_entity_state_records_0x4CEC(
                bus, entity_record_address);
            if (continuation.unresolved_0xB330_path) {
                return {false, target, emitted.lists_processed,
                        emitted.objects_emitted, emitted.next_output_address,
                        emitted.next_count, emitted.next_ordinal};
            }
            return {true, target, emitted.lists_processed,
                    emitted.objects_emitted, emitted.next_output_address,
                    emitted.next_count, emitted.next_ordinal};
        }
    } else if (target == 0x00004D54U) {
        emitted = emit_entity_state_objects_0x4FE2(
            bus, entity_record_address, output_address, first_ordinal,
            max_records_per_list);
        // 0x4D54 has the same wrapper shape around 0x4FE2; its zero-state
        // continuation begins at 0x4D62.
        if (bus.read8(entity_record_address + 0xB2U) == 0) {
            const auto continuation = scan_entity_state_records_0x4D62(
                bus, entity_record_address);
            if (continuation.unresolved_0xB330_path) {
                return {false, target, emitted.lists_processed,
                        emitted.objects_emitted, emitted.next_output_address,
                        emitted.next_count, emitted.next_ordinal};
            }
            return {true, target, emitted.lists_processed,
                    emitted.objects_emitted, emitted.next_output_address,
                    emitted.next_count, emitted.next_ordinal};
        }
    } else {
        return {false, target, 0, 0, output_address,
                bus.read16(ram::kObjectListCount), first_ordinal};
    }

    return {true, target, emitted.lists_processed, emitted.objects_emitted,
            emitted.next_output_address, emitted.next_count,
            emitted.next_ordinal};
}

void initialise_entity_random_child_0x5468(
    GenesisBus& bus,
    GenesisAddress source_record_address,
    GenesisAddress target_record_address) {
    // ROM 0x5468 initializes the shared phase gate before copying the child
    // record fields. All copies retain the original word/longword widths.
    bus.write16(ram::kEntityRandomPhaseGate, 4);
    bus.write8(target_record_address + 0xB2U, 0x0DU);
    bus.write16(target_record_address + 0x32U,
                bus.read16(source_record_address + 0x32U));
    bus.write16(target_record_address + 0x2AU,
                bus.read16(source_record_address + 0x2AU));
    bus.write8(target_record_address + 2U, 5);
    bus.write8(target_record_address + 3U, 0);
    bus.write32(target_record_address + 0xD8U,
                bus.read32(source_record_address + 0xD8U));
}

std::size_t clear_entity_generation_records_0x5622(
    GenesisBus& bus,
    std::uint8_t mask) {
    for (std::size_t index = 0; index < 10; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase + index * 0x100U);
        bus.write8(record_address + 0xDEU, 0);
        bus.write8(record_address + 0xDFU,
                   static_cast<std::uint8_t>(
                       bus.read8(record_address + 0xDFU) & mask));
    }
    return 10;
}

EntityRandomScanResult run_entity_random_scan_0x53C4(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    EntityRandomScanResult result{
        false, false, false, 0, 0, 0, output_address,
        bus.read16(ram::kObjectListCount), first_ordinal};

    // 0x53C4 tests the longword at FFDFF6 but decrements only its low word.
    // The sign test is therefore equivalent to the signed gate word for this
    // adjacent gate/delay layout.
    auto gate = bus.read16(ram::kEntityRandomGate);
    if (static_cast<std::int16_t>(gate) >= 0) {
        gate = static_cast<std::uint16_t>(gate - 1U);
        bus.write16(ram::kEntityRandomGate, gate);
        if (static_cast<std::int16_t>(gate) >= 0) {
            return result;
        }
        bus.write16(ram::kEntityRandomGateDelay,
                    static_cast<std::uint16_t>(
                        bus.read16(ram::kEntityRandomGateDelay) - 1U));
    }

    result.triggered = true;
    const auto random_output = emit_entity_random_objects_0x5494(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list);
    result.objects_emitted = random_output.objects_emitted;
    result.next_output_address = random_output.next_output_address;
    result.next_count = random_output.next_count;
    result.next_ordinal = random_output.next_ordinal;

    // 0x53E4 returns immediately when 0x5494 left the entity dispatch byte
    // set. Only its cleared-state path enters the nine-record scan.
    if (bus.read8(entity_record_address + 0xB2U) != 0) {
        return result;
    }
    result.scan_entered = true;

    auto candidate_address = static_cast<GenesisAddress>(
        entity_record_address + 0x100U);
    for (std::size_t index = 0; index < 9; ++index,
                              candidate_address += 0x100U) {
        ++result.candidate_records_examined;
        if (static_cast<std::int16_t>(
                bus.read16(candidate_address + 0x3EU)) >= 9) {
            break;
        }
        if (bus.read8(candidate_address + 0xDEU) != 0x0DU) {
            continue;
        }
        const auto flags = bus.read8(candidate_address + 0x10U);
        if ((flags & 1U) == 0 || (flags & 0x40U) != 0) {
            continue;
        }

        initialise_entity_random_child_0x5468(
            bus, entity_record_address, candidate_address);
        ++result.child_records_initialised;
        const auto animation_start = initialize_entity_animation_0xAFF2(
            bus, candidate_address);
        result.unresolved_0xAFF2_call =
            animation_start.unresolved_0xB330_path;
        return result;
    }

    if (static_cast<std::int16_t>(
            bus.read16(ram::kEntityRandomGateDelay)) < 0) {
        clear_indexed_object_byte(
            bus, bus.read16(entity_record_address + 0x32U));
        clear_entity_generation_records_0x5622(bus, 0xFDU);
    } else {
        bus.write16(ram::kEntityRandomGate, 0x001EU);
        const auto auxiliary_record = bus.read32(ram::kEntityRandomAux);
        initialise_entity_random_child_0x5468(
            bus, entity_record_address, auxiliary_record);
    }
    return result;
}

EntityStateScanResult scan_entity_state_records_0x52CC(
    GenesisBus& bus,
    GenesisAddress source_record_address) {
    EntityStateScanResult result{0, 0};
    for (std::size_t index = 0; index < 10; ++index) {
        const auto target_record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase + index * 0x100U);
        ++result.records_examined;
        const auto flags = bus.read8(target_record_address + 0x10U);
        if (bus.read8(target_record_address + 0xDEU) != 5U ||
            (flags & 1U) == 0 || (flags & 0x40U) != 0) {
            continue;
        }
        bus.write8(target_record_address + 0xB2U, 5);
        bus.write16(target_record_address + 0x32U,
                   bus.read16(source_record_address + 0x32U));
        bus.write16(target_record_address + 0x2AU,
                   bus.read16(source_record_address + 0x2AU));
        bus.write8(target_record_address + 2U, 5);
        bus.write8(target_record_address + 3U, 0);
        bus.write32(target_record_address + 0xD8U,
                   bus.read32(source_record_address + 0xD8U));
        ++result.records_initialised;
    }
    return result;
}

namespace {

EntityStateContinuationResult scan_entity_state_records_after_cutoff(
    GenesisBus& bus,
    GenesisAddress source_record_address,
    std::uint8_t discriminator,
    std::uint8_t state_value) {
    EntityStateContinuationResult result{0, 0, false, false};
    for (std::size_t index = 0; index < 10; ++index) {
        const auto target_record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase + index * 0x100U);
        ++result.records_examined;
        const auto flags = bus.read8(target_record_address + 0x10U);
        if (bus.read8(target_record_address + 0xDEU) != discriminator ||
            (flags & 1U) == 0 || (flags & 0x40U) != 0) {
            continue;
        }
        bus.write8(target_record_address + 0xB2U, state_value);
        bus.write16(target_record_address + 0x32U,
                   bus.read16(source_record_address + 0x32U));
        bus.write16(target_record_address + 0x2AU,
                   bus.read16(source_record_address + 0x2AU));
        bus.write8(target_record_address + 2U, 5);
        bus.write8(target_record_address + 3U, 0);
        bus.write32(target_record_address + 0xD8U,
                   bus.read32(source_record_address + 0xD8U));
        ++result.records_initialised;

        const auto animation_start = initialize_entity_animation_0xAFF2(
            bus, target_record_address);
        result.unresolved_0xB330_path =
            animation_start.unresolved_0xB330_path;
        // 0x4CEC/0x4D62 load D0 with 0x35 and tail-call 0x530DE. The
        // 0x530DE/0x52F3C CPU-side transport is now direct; the bus owns the
        // eventual Z80 execution and hardware timing.
        if (!result.unresolved_0xB330_path) {
            result.z80_command_dispatched =
                dispatch_z80_command_0x530DE(bus, 0x35U).command_sent;
        }
        return result;
    }
    return result;
}

} // namespace

EntityStateContinuationResult scan_entity_state_records_0x4CEC(
    GenesisBus& bus,
    GenesisAddress source_record_address) {
    return scan_entity_state_records_after_cutoff(
        bus, source_record_address, 3, 3);
}

EntityStateContinuationResult scan_entity_state_records_0x4D62(
    GenesisBus& bus,
    GenesisAddress source_record_address) {
    return scan_entity_state_records_after_cutoff(
        bus, source_record_address, 0x0AU, 0x0AU);
}

EntityPhaseObjectOutputResult emit_entity_timed_objects_0x557E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    auto timer = bus.read16(ram::kEntityTimedGate);
    if (static_cast<std::int16_t>(timer) >= 0) {
        timer = static_cast<std::uint16_t>(timer - 1U);
        bus.write16(ram::kEntityTimedGate, timer);
        if (static_cast<std::int16_t>(timer) >= 0) {
            return {0, 0, output_address,
                    bus.read16(ram::kObjectListCount), first_ordinal};
        }
    }

    bus.write16(ram::kEntityTimedGateDelay,
                static_cast<std::uint16_t>(
                    bus.read16(ram::kEntityTimedGateDelay) - 1U));
    return emit_entity_timed_objects_0x5662(
        bus, entity_record_address, output_address, first_ordinal,
        max_records_per_list);
}

EntityPhaseObjectOutputResult emit_entity_random_objects_0x5494(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list) {
    auto timer = bus.read8(entity_record_address + 2U);
    timer = static_cast<std::uint8_t>(timer - 1U);
    bus.write8(entity_record_address + 2U, timer);
    bool descriptor_needed = false;
    if (static_cast<std::int8_t>(timer) < 0) {
        bus.write8(entity_record_address + 2U, 6);
        bus.write8(entity_record_address + 3U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 3U) + 1U));
        descriptor_needed = true;
    }

    auto phase = static_cast<std::int16_t>(static_cast<std::int8_t>(
        bus.read8(entity_record_address + 3U)));
    if (phase >= 3) {
        auto gate = bus.read16(ram::kEntityRandomPhaseGate);
        gate = static_cast<std::uint16_t>(gate - 1U);
        bus.write16(ram::kEntityRandomPhaseGate, gate);
        if (static_cast<std::int16_t>(gate) < 0) {
            bus.write8(entity_record_address + 0xB2U, 0);
            return {0, 0, output_address,
                    bus.read16(ram::kObjectListCount), first_ordinal};
        }

        const auto random = ShadowrunRuntime(bus).advance_random();
        bus.write16(ram::kEntityRandomValue, random);
        auto x_offset = static_cast<std::uint16_t>(random & 7U);
        if ((x_offset & 2U) == 0) {
            x_offset = static_cast<std::uint16_t>(0U - x_offset);
        }
        bus.write16(ram::kEntityRandomXOffset, x_offset);
        auto y_offset = static_cast<std::uint16_t>(random & 7U);
        if ((y_offset & 4U) == 0) {
            y_offset = static_cast<std::uint16_t>(0U - y_offset);
        }
        bus.write16(ram::kEntityRandomYOffset, y_offset);
        bus.write8(entity_record_address + 3U, 0);
        bus.write8(entity_record_address + 2U, 5);
        phase = 0;
        descriptor_needed = y_offset != 0;
    }

    const auto table_address = (bus.read16(ram::kEntityRandomValue) & 0x10U) != 0
        ? rom::kEntityRandomObjectPointerTable
        : rom::kEntityRandomObjectPointerTableAlternate;
    const auto phase_offset = static_cast<GenesisAddress>(
        static_cast<std::int32_t>(phase) * 4);
    const auto descriptor = bus.read32(static_cast<GenesisAddress>(
        static_cast<std::int32_t>(table_address) +
        static_cast<std::int32_t>(phase_offset)));
    if (descriptor_needed) {
        append_descriptor_object(bus, descriptor, entity_record_address);
    }

    auto x_base = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x0AU) +
        bus.read16(ram::kEntityRandomXOffset));
    auto y_base = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x0CU) +
        bus.read16(ram::kEntityRandomYOffset));
    const auto phase_delta = static_cast<std::int16_t>(phase - 4);
    if (phase_delta >= 0) {
        y_base = static_cast<std::uint16_t>(
            y_base + static_cast<std::uint16_t>(phase_delta << 2U));
        x_base = static_cast<std::uint16_t>(
            x_base + static_cast<std::uint16_t>(
                bus.read16(ram::kRandomState) & 1U));
    }

    const auto list = build_object_list_from_bus(
        bus, static_cast<GenesisAddress>(descriptor + 8U), output_address,
        max_records_per_list, static_cast<std::int16_t>(y_base),
        static_cast<std::int16_t>(x_base), 0x6000U,
        static_cast<std::int16_t>(bus.read16(entity_record_address + 0x2AU)),
        first_ordinal, ObjectListVariant::WithWord3Offset);
    return {1, list.built, list.next_output_address,
            list.object_count, list.next_ordinal};
}

std::size_t build_object_list(const RawObjectRecord* input,
                              std::size_t input_capacity,
                              BuiltObjectRecord* output,
                              std::size_t output_capacity,
                              std::int16_t word0_base,
                              std::int16_t word6_base,
                              std::uint16_t word4_flags,
                              std::int16_t word3_offset,
                              std::uint8_t first_ordinal,
                              ObjectListBuildState& state,
                              ObjectListVariant variant) {
    if (input == nullptr || output == nullptr) {
        return 0;
    }

    std::size_t built = 0;
    auto ordinal = first_ordinal;
    while (built < input_capacity && built < output_capacity) {
        const auto& source = input[built];
        if (source.word0 == 0x8000U) {
            break;
        }

        auto& destination = output[built];
        destination.word0 = add_word(
            word0_base, static_cast<std::int16_t>(source.word1));
        destination.byte2 = static_cast<std::uint8_t>(source.word2);
        destination.ordinal = ordinal++;
        const auto adjusted_word3 = variant == ObjectListVariant::WithWord3Offset
            ? add_word_unsigned(static_cast<std::uint16_t>(word3_offset), source.word3)
            : source.word3;
        destination.word4 = static_cast<std::uint16_t>(
            word4_flags | adjusted_word3);
        destination.word6 = add_word(
            word6_base, static_cast<std::int16_t>(source.word0));
        ++built;
        state.next_output_index = built;
        state.object_count = static_cast<std::uint16_t>(state.object_count + 1U);
    }

    return built;
}

ObjectListBusResult build_object_list_from_bus(
    GenesisBus& bus,
    GenesisAddress input_address,
    GenesisAddress output_address,
    std::size_t max_records,
    std::int16_t word0_base,
    std::int16_t word6_base,
    std::uint16_t word4_flags,
    std::int16_t word3_offset,
    std::uint8_t first_ordinal,
    ObjectListVariant variant) {
    auto next_input = input_address;
    auto next_output = output_address;
    auto ordinal = first_ordinal;
    std::size_t built = 0;

    while (built < max_records) {
        const auto source_word0 = bus.read16(next_input);
        if (source_word0 == 0x8000U) {
            break;
        }

        const auto source_word1 = bus.read16(next_input + 2U);
        const auto source_word2 = bus.read16(next_input + 4U);
        const auto source_word3 = bus.read16(next_input + 6U);
        bus.write16(next_output,
                    static_cast<std::uint16_t>(
                        static_cast<std::uint16_t>(word0_base) +
                        source_word1));
        bus.write8(next_output + 2U,
                   static_cast<std::uint8_t>(source_word2));
        bus.write8(next_output + 3U, ordinal++);
        const auto adjusted_word3 = variant == ObjectListVariant::WithWord3Offset
            ? static_cast<std::uint16_t>(
                  static_cast<std::uint16_t>(word3_offset) + source_word3)
            : source_word3;
        bus.write16(next_output + 4U,
                    static_cast<std::uint16_t>(word4_flags | adjusted_word3));
        bus.write16(next_output + 6U,
                    static_cast<std::uint16_t>(
                        static_cast<std::uint16_t>(word6_base) +
                        source_word0));

        next_input += 8U;
        next_output += 8U;
        ++built;
        const auto object_count = static_cast<std::uint16_t>(
            bus.read16(ram::kObjectListCount) + 1U);
        bus.write16(ram::kObjectListCount, object_count);
        bus.write32(ram::kObjectListOutputPointer, next_output);
        bus.write16(ram::kObjectListCountMirror, object_count);
    }

    return {built, next_output, bus.read16(ram::kObjectListCount), ordinal};
}

std::size_t build_object_list(const RawObjectRecord* input,
                              std::size_t input_capacity,
                              BuiltObjectRecord* output,
                              std::size_t output_capacity,
                              std::int16_t word0_base,
                              std::int16_t word6_base,
                              std::uint16_t word4_flags,
                              std::int16_t word3_offset,
                              std::uint8_t first_ordinal,
                              std::uint16_t& object_count,
                              ObjectListVariant variant) {
    ObjectListBuildState state{object_count, 0};
    const auto built = build_object_list(
        input, input_capacity, output, output_capacity, word0_base, word6_base,
        word4_flags, word3_offset, first_ordinal, state, variant);
    object_count = state.object_count;
    return built;
}

} // namespace shadowrun
