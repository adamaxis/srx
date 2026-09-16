#include "shadowrun/session.hpp"

#include "shadowrun/boot.hpp"
#include "shadowrun/entity_projection.hpp"
#include "shadowrun/entity_state.hpp"
#include "shadowrun/object_list.hpp"
#include "shadowrun/object_mask.hpp"
#include "shadowrun/object_update.hpp"
#include "shadowrun/rom_layout.hpp"
#include "shadowrun/resource_setup.hpp"
#include "shadowrun/runtime.hpp"
#include "shadowrun/z80_transport.hpp"

#include <array>
#include <utility>

namespace shadowrun {

namespace {

void update_entity_timing(GenesisBus& bus, GenesisAddress record_address) {
    // ROM 0x5FDC. The comparison is signed because the source uses BLT after
    // a word-sized CMP; every intermediate remains a 68000 word.
    bus.write16(session_ram::kEntityTimingWord,
                bus.read16(record_address + 0x1EU));
    auto accumulated = bus.read16(record_address + 0x1AU);
    bus.write16(session_ram::kEntityTimingAccumulator, accumulated);
    accumulated = static_cast<std::uint16_t>(
        accumulated + bus.read16(record_address + 0x18U));
    if (static_cast<std::int16_t>(accumulated) <
        static_cast<std::int16_t>(bus.read16(record_address + 0x1CU))) {
        bus.write16(record_address + 0x1EU,
                    static_cast<std::uint16_t>(
                        bus.read16(record_address + 0x1EU) + 1U));
        bus.write8(record_address + 0x11U,
                   static_cast<std::uint8_t>(bus.read8(record_address + 0x11U) & ~1U));
        accumulated = static_cast<std::uint16_t>(
            accumulated - bus.read16(record_address + 0x1CU));
        bus.write16(session_ram::kEntityTimingAccumulator, accumulated);
    }
}

std::uint8_t decode_packed_sprite_nibble(GenesisBus& bus,
                                         std::uint8_t value) {
    const auto high = bus.read8(rom::kPackedSpriteNibbleTable +
                                static_cast<std::uint8_t>(value >> 4U));
    const auto low = bus.read8(rom::kPackedSpriteNibbleTable +
                               static_cast<std::uint8_t>(value & 0x0FU));
    return static_cast<std::uint8_t>((high << 4U) | low);
}

GenesisAddress entity_tile_source_0x3F9E(
    GenesisBus& bus,
    std::uint16_t x,
    std::uint16_t y) {
    // ROM 0x3FD8 computes a byte offset in the tile map using the four-bit
    // tile coordinates and the current plane width. Every intermediate here
    // is a word, as in the 68000 implementation.
    auto d0 = static_cast<std::uint16_t>((x >> 3U) & 0x0003U);
    auto d1 = static_cast<std::uint16_t>((y >> 3U) & 0x0003U);
    auto d2 = x;
    auto d3 = y;
    d3 = static_cast<std::uint16_t>(d3 >> 5U);
    d3 = static_cast<std::uint16_t>(
        d3 * bus.read16(ram::kTilePlaneWidth));
    d2 = static_cast<std::uint16_t>(d2 >> 5U);
    d2 = static_cast<std::uint16_t>(d2 << 1U);
    d3 = static_cast<std::uint16_t>(d3 + d2);
    d3 = static_cast<std::uint16_t>(d3 << 2U);
    d1 = static_cast<std::uint16_t>(d1 << 2U);
    d1 = static_cast<std::uint16_t>(d1 + d0);
    d1 = static_cast<std::uint16_t>(d1 << 1U);

    const auto tile_row = static_cast<std::int16_t>(
        bus.read16(ram::kTileMapIndexTable + d3));
    auto tile_byte_offset = static_cast<std::int32_t>(tile_row) << 5;
    tile_byte_offset += static_cast<std::int16_t>(d1);
    const auto tile_map = bus.read32(ram::kTilePlaneSourceA);
    auto tile_number = bus.read16(static_cast<GenesisAddress>(
        tile_map + tile_byte_offset));
    tile_number = static_cast<std::uint16_t>(tile_number & 0x07FFU);
    tile_number = static_cast<std::uint16_t>(tile_number - 0x0201U);
    const auto pixel_offset = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(tile_number << 5U));
    return static_cast<GenesisAddress>(
        bus.read32(ram::kTilePixelBufferPointer) + pixel_offset);
}

void render_entity_mask_0x3EFE(GenesisBus& bus,
                               GenesisAddress record_address,
                               GenesisAddress state_pointer) {
    auto x = static_cast<std::uint16_t>(
        bus.read16(state_pointer + 8U) +
        bus.read16(record_address + 0x0AU) - 0x0080U +
        (bus.read16(ram::kCameraX) >> 4U));
    auto y = static_cast<std::uint16_t>(
        bus.read16(state_pointer + 0x0AU) +
        bus.read16(record_address + 0x0CU) - 0x0080U +
        (bus.read16(ram::kCameraY) >> 4U));
    const auto dimensions = bus.read16(state_pointer + 0x0CU);
    const auto columns = static_cast<std::size_t>((dimensions >> 2U) + 1U);
    const auto rows = static_cast<std::size_t>((dimensions & 3U) + 1U);
    const auto output = bus.read32(record_address + 0x44U);
    auto destination = output;
    auto mask_source = bus.read32(state_pointer + 2U);

    // 0x3EFE reads the packed mask stream from state+2, while 0x3F9E returns
    // the corresponding tile-pixel block that is masked into entity+0x44.
    for (std::size_t column = 0; column < columns; ++column) {
        for (std::size_t row = 0; row < rows; ++row) {
            const auto tile_source = entity_tile_source_0x3F9E(bus, x, y);
            for (std::size_t byte_index = 0; byte_index < 32; ++byte_index) {
                const auto value = bus.read8(mask_source++);
                std::uint8_t mask = value;
                if (value != 0) {
                    if ((value & 0xF0U) == 0) {
                        mask = 0x0FU;
                    } else {
                        mask = (value & 0x0FU) == 0 ? 0xF0U : 0xFFU;
                    }
                }
                bus.write8(destination, static_cast<std::uint8_t>(
                    bus.read8(tile_source + byte_index) & mask));
                ++destination;
            }
            y = static_cast<std::uint16_t>(y + 8U);
        }
        x = static_cast<std::uint16_t>(x + 8U);
        y = static_cast<std::uint16_t>(
            y - static_cast<std::uint16_t>(rows * 8U));
    }
}

void decode_entity_sprite_0x3E86(GenesisBus& bus,
                                 GenesisAddress record_address,
                                 GenesisAddress state_pointer) {
    const auto source = bus.read32(state_pointer + 2U);
    auto destination = bus.read32(record_address + 0x48U);
    const auto length = bus.read16(state_pointer + 6U);
    for (std::size_t index = 0; index < length; ++index) {
        bus.write8(destination++, decode_packed_sprite_nibble(
            bus, bus.read8(source + index)));
    }
}

void run_entity_frame_0x60FA(GenesisBus& bus,
                             GenesisAddress record_address,
                             GenesisAddress state_pointer) {
    // ROM 0x60FA conditionally regenerates the mask and packed sprite buffers,
    // then queues the entity for the VBlank-side 0x6148 consumer.
    const auto flags = bus.read8(record_address + 0x11U);
    if ((flags & 0x80U) != 0) {
        if (bus.read16(record_address + 0x0CU) == 0) {
            return;
        }
        render_entity_mask_0x3EFE(bus, record_address, state_pointer);
    }
    if ((flags & 0x20U) != 0) {
        decode_entity_sprite_0x3E86(bus, record_address, state_pointer);
    }

    const auto next_state = bus.read32(state_pointer + 2U);
    bus.write32(record_address + 0x22U, next_state);
    bus.write16(record_address + 0x28U, bus.read16(state_pointer + 6U));
    const auto queue = bus.read32(ram::kVblankWorkAPointer);
    bus.write32(queue, record_address);
    bus.write8(ram::kVblankWorkAPointer + 3U,
               static_cast<std::uint8_t>(
                   bus.read8(ram::kVblankWorkAPointer + 3U) + 4U));
    bus.write16(ram::kVblankWorkA,
                static_cast<std::uint16_t>(bus.read16(ram::kVblankWorkA) +
                                           1U));
}

struct PostObjectUpdateResult {
    std::size_t objects_emitted;
    std::size_t unresolved_calls;
    GenesisAddress next_output_address;
    std::uint8_t next_ordinal;
};

std::int16_t add_post_object_offset(std::uint16_t base,
                                    std::int16_t offset) {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(
        base + static_cast<std::uint16_t>(offset)));
}

void append_post_object_list(GenesisBus& bus,
                             GenesisAddress pointer_table,
                             std::uint16_t phase,
                             std::int16_t word0_base,
                             std::int16_t word6_base,
                             GenesisAddress& output_address,
                             std::uint8_t& ordinal,
                             std::size_t max_records_per_list,
                             PostObjectUpdateResult& result) {
    const auto list_address = bus.read32(
        pointer_table + static_cast<GenesisAddress>(phase * 4U));
    const auto list = build_object_list_from_bus(
        bus, list_address, output_address, max_records_per_list,
        word0_base, word6_base, 0x4000U, 0, ordinal,
        ObjectListVariant::Base);
    output_address = list.next_output_address;
    ordinal = list.next_ordinal;
    result.objects_emitted += list.built;
}

void update_post_object_timer_0x5D55C(GenesisBus& bus,
                                      GenesisAddress state_address) {
    auto timer = static_cast<std::uint16_t>(
        bus.read16(state_address + 2U) - 1U);
    bus.write16(state_address + 2U, timer);
    if (static_cast<std::int16_t>(timer) < 0) {
        bus.write16(state_address + 2U, 10);
        auto phase = static_cast<std::uint16_t>(
            bus.read16(state_address + 0x18U) + 1U);
        if (phase > 7U) {
            phase = 0;
        }
        bus.write16(state_address + 0x18U, phase);
    }
}

void update_post_object_timer_0x5D5A2(
    GenesisBus& bus,
    GenesisAddress state_address,
    GenesisAddress& output_address,
    std::uint8_t& ordinal,
    std::size_t max_records_per_list,
    PostObjectUpdateResult& result) {
    auto timer = static_cast<std::uint16_t>(
        bus.read16(state_address + 6U) - 1U);
    bus.write16(state_address + 6U, timer);
    if (static_cast<std::int16_t>(timer) < 0) {
        bus.write16(state_address + 6U, 0x14U);
        auto phase = static_cast<std::uint16_t>(
            bus.read16(state_address + 0x16U) + 1U);
        if (phase > 8U) {
            phase = 0;
        }
        bus.write16(state_address + 0x16U, phase);
    }

    const auto offset_index = static_cast<std::uint16_t>(
        bus.read16(state_address + 0x18U) * 4U);
    const auto x_offset = static_cast<std::int16_t>(bus.read16(
        rom::kPostObjectOffsetTable + offset_index));
    const auto y_offset = static_cast<std::int16_t>(bus.read16(
        rom::kPostObjectOffsetTable + offset_index + 2U));
    append_post_object_list(
        bus, rom::kPostObjectPointerTableB,
        bus.read16(state_address + 0x16U),
        add_post_object_offset(
            static_cast<std::uint16_t>(bus.read16(state_address + 0x0CU)),
            y_offset),
        add_post_object_offset(
            static_cast<std::uint16_t>(bus.read16(state_address + 0x0AU)),
            x_offset),
        output_address, ordinal, max_records_per_list, result);
}

void update_post_object_timer_0x5D5FC(
    GenesisBus& bus,
    GenesisAddress state_address,
    GenesisAddress& output_address,
    std::uint8_t& ordinal,
    std::size_t max_records_per_list,
    PostObjectUpdateResult& result) {
    auto timer = static_cast<std::uint16_t>(
        bus.read16(state_address + 0x1AU) - 1U);
    bus.write16(state_address + 0x1AU, timer);
    if (static_cast<std::int16_t>(timer) < 0) {
        bus.write16(state_address + 0x1AU, 10);
        auto phase = static_cast<std::uint16_t>(
            bus.read16(state_address + 0x1CU) + 1U);
        if (phase > 9U) {
            phase = 0;
        }
        bus.write16(state_address + 0x1CU, phase);
    }

    const auto offset_index = static_cast<std::uint16_t>(
        bus.read16(state_address + 0x18U) * 4U);
    const auto x_offset = static_cast<std::int16_t>(bus.read16(
        rom::kPostObjectOffsetTable + offset_index));
    const auto y_offset = static_cast<std::int16_t>(bus.read16(
        rom::kPostObjectOffsetTable + offset_index + 2U));
    append_post_object_list(
        bus, rom::kPostObjectPointerTableC,
        bus.read16(state_address + 0x1CU),
        static_cast<std::int16_t>(
            add_post_object_offset(
                static_cast<std::uint16_t>(bus.read16(
                    state_address + 0x0CU)), y_offset) - 0x18U),
        add_post_object_offset(
            static_cast<std::uint16_t>(bus.read16(state_address + 0x0AU)),
            x_offset),
        output_address, ordinal, max_records_per_list, result);
}

void update_post_object_timer_0x5D65A(
    GenesisBus& bus,
    GenesisAddress state_address,
    GenesisAddress& output_address,
    std::uint8_t& ordinal,
    std::size_t max_records_per_list,
    PostObjectUpdateResult& result) {
    auto timer = static_cast<std::uint16_t>(
        bus.read16(state_address + 0x20U) - 1U);
    bus.write16(state_address + 0x20U, timer);
    if (static_cast<std::int16_t>(timer) < 0) {
        bus.write16(state_address + 0x20U, 4);
        auto phase = static_cast<std::uint16_t>(
            bus.read16(state_address + 0x1EU) + 1U);
        if (phase > 0x27U) {
            phase = 0;
        }
        bus.write16(state_address + 0x1EU, phase);
    }

    const auto offset_index = static_cast<std::uint16_t>(
        bus.read16(state_address + 0x18U) * 4U);
    const auto x_offset = static_cast<std::int16_t>(bus.read16(
        rom::kPostObjectOffsetTable + offset_index));
    const auto y_offset = static_cast<std::int16_t>(bus.read16(
        rom::kPostObjectOffsetTable + offset_index + 2U));
    append_post_object_list(
        bus, rom::kPostObjectPointerTableD,
        bus.read16(state_address + 0x1EU),
        add_post_object_offset(
            static_cast<std::uint16_t>(bus.read16(state_address + 0x0CU)),
            static_cast<std::int16_t>(y_offset + 0x30)),
        add_post_object_offset(
            static_cast<std::uint16_t>(bus.read16(state_address + 0x0AU)),
            static_cast<std::int16_t>(x_offset + 0x12)),
        output_address, ordinal, max_records_per_list, result);
}

void emit_post_object_timer_0x5D57E(
    GenesisBus& bus,
    GenesisAddress state_address,
    GenesisAddress& output_address,
    std::uint8_t& ordinal,
    std::size_t max_records_per_list,
    PostObjectUpdateResult& result) {
    append_post_object_list(
        bus, rom::kPostObjectPointerTableA,
        bus.read16(state_address + 0x18U),
        static_cast<std::int16_t>(bus.read16(state_address + 0x0CU)),
        static_cast<std::int16_t>(bus.read16(state_address + 0x0AU)),
        output_address, ordinal, max_records_per_list, result);
}

void copy_post_object_upload(GenesisBus& bus,
                             GenesisAddress source_address) {
    for (std::size_t index = 0; index < 0x20U; ++index) {
        bus.write8(ram::kVdpUploadBuffer + index,
                   bus.read8(source_address + index));
    }
    bus.write8(ram::kVblankFlags, static_cast<std::uint8_t>(
        bus.read8(ram::kVblankFlags) | 1U));
}

void refresh_post_object_background(GenesisBus& bus) {
    const auto stride = bus.read16(ram::kTilePlaneWidth);
    for (std::size_t pair = 0; pair < 3; ++pair) {
        const auto source = rom::kPostObjectBackgroundTileTable + pair * 4U;
        const auto destination = ram::kBackgroundTileTable +
            static_cast<GenesisAddress>(pair * stride);
        bus.write16(destination, bus.read16(source));
        bus.write16(destination + 2U, bus.read16(source + 2U));
    }
    bus.write16(ram::kBackgroundQueueBusy, 1);
}

void complete_post_object_background(GenesisBus& bus) {
    if (bus.read16(ram::kBackgroundQueueTimer) != 0) {
        return;
    }
    bus.write16(ram::kBackgroundQueueTimer, 0x3CU);
    bus.write32(ram::kBackgroundQueuePointer,
                bus.read32(ram::kResourcePointer));
}

std::size_t run_post_object_entity_initializer(
    GenesisBus& bus,
    GenesisAddress target_record_address) {
    // 0x5D3E6 sets the target flag, runs 0x15930 on A1, then invokes the
    // reviewed 0xAFF2 initializer on FF0400. The indirect 0x15930 handler is
    // the verified 0x49A0 RTS entry for this fixed table index.
    bus.write8(target_record_address + 0x11U, static_cast<std::uint8_t>(
        bus.read8(target_record_address + 0x11U) | 1U));
    const auto slot = initialize_entity_slot_0x15930(
        bus, target_record_address);
    const auto animation = initialize_entity_animation_0xAFF2(
        bus, 0xFF0400U);
    return (slot.unresolved_handler ? 1U : 0U) +
           (animation.unresolved_0xB330_path ? 1U : 0U);
}

std::size_t step_post_object_state_0x5D416(
    GenesisBus& bus,
    GenesisAddress state_address,
    GenesisAddress& output_address,
    std::uint8_t& ordinal) {
    auto result = PostObjectUpdateResult{0, 0, output_address, ordinal};
    const auto counter = bus.read16(state_address + 0x24U);
    if (counter > 0x32U) {
        return 0;
    }

    if (counter == 0x14U) {
        dispatch_z80_command_0x530DE(bus, 0x2CU);
        // 0x5D6BC is a distinct path from 0x5D3E6: it allocates one of the
        // six FF0B00 records, copies its 0x100-byte ROM template, and then
        // allocates the sprite slot used by that new record.
        spawn_post_object_entity_0x5D6BC(bus);
    } else if (counter == 0x19U || counter == 0x1BU ||
               counter == 0x25U || counter == 0x26U ||
               counter == 0x28U || counter == 0x29U) {
        dispatch_z80_command_0x530DE(bus, 0x39U);
        bus.write16(ram::kVdpDataWord, static_cast<std::uint16_t>(
            bus.read16(ram::kVdpDataWord) + 8U));
        bus.write16(ram::kVdpDataWord2, static_cast<std::uint16_t>(
            bus.read16(ram::kVdpDataWord2) + 8U));
    } else if (counter == 0x18U || counter == 0x1AU ||
               counter == 0x24U || counter == 0x27U) {
        bus.write16(ram::kVdpDataWord, static_cast<std::uint16_t>(
            bus.read16(ram::kVdpDataWord) - 8U));
        bus.write16(ram::kVdpDataWord2, static_cast<std::uint16_t>(
            bus.read16(ram::kVdpDataWord2) - 8U));
    } else if (counter == 0x16U || counter == 0x20U ||
               counter == 0x21U || counter == 0x23U) {
        copy_post_object_upload(bus, rom::kPostObjectUploadTableB);
    } else {
        copy_post_object_upload(bus, rom::kPostObjectUploadTable);
    }

    bus.write8(ram::kVblankFlags, static_cast<std::uint8_t>(
        bus.read8(ram::kVblankFlags) | 0x0DU));
    output_address = result.next_output_address;
    ordinal = result.next_ordinal;
    return result.unresolved_calls;
}

PostObjectUpdateResult run_post_object_update_0x5D29C(
    GenesisBus& bus,
    GenesisAddress output_address,
    std::uint8_t ordinal,
    std::size_t max_records_per_list) {
    PostObjectUpdateResult result{0, 0, output_address, ordinal};
    const auto global_phase = bus.read8(ram::kEntityStateGlobalPhase);
    if (global_phase != 2U && global_phase != 3U) {
        return result;
    }

    const auto state_address = session_ram::kPostObjectState;
    const auto state = bus.read16(state_address + 0x22U);
    if (static_cast<std::int16_t>(state) < 0) {
        return result;
    }

    bus.write32(ram::kObjectListOutputPointer, output_address);
    auto next_output = output_address;
    auto next_ordinal = ordinal;
    const auto emit_common = [&]() {
        project_entity_record(bus, state_address,
                              bus.read16(ram::kCameraX),
                              bus.read16(ram::kCameraY));
        update_post_object_timer_0x5D55C(bus, state_address);
    };

    if (state == 2U) {
        emit_common();
        update_post_object_timer_0x5D65A(
            bus, state_address, next_output, next_ordinal,
            max_records_per_list, result);
        update_post_object_timer_0x5D5FC(
            bus, state_address, next_output, next_ordinal,
            max_records_per_list, result);
        update_post_object_timer_0x5D5A2(
            bus, state_address, next_output, next_ordinal,
            max_records_per_list, result);
        emit_post_object_timer_0x5D57E(
            bus, state_address, next_output, next_ordinal,
            max_records_per_list, result);

        if (bus.read16(state_address + 0x18U) == 3U) {
            auto candidate = 0xFF0500U;
            for (std::size_t index = 0; index < 6; ++index,
                                      candidate += 0x100U) {
                const auto flags = bus.read8(candidate + 0x10U);
                if ((flags & 1U) != 0 && (flags & 0x10U) == 0 &&
                    (flags & 0x40U) == 0 &&
                    bus.read8(candidate + 0x4CU) != 0) {
                    result.unresolved_calls +=
                        run_post_object_entity_initializer(bus, candidate);
                    result.next_output_address = next_output;
                    result.next_ordinal = next_ordinal;
                    return result;
                }
            }

            auto timer = static_cast<std::uint16_t>(
                bus.read16(state_address + 0x24U) - 1U);
            bus.write16(state_address + 0x24U, timer);
            if (static_cast<std::int16_t>(timer) < 0) {
                bus.write32(ram::kResourcePointer, 0x0005BAC4U);
                complete_post_object_background(bus);
                result.next_output_address = next_output;
                result.next_ordinal = next_ordinal;
                return result;
            }
            bus.write8(state_address + 0x9DU, 1);
            bus.write8(0xFF0477U, 0x20U);
            bus.write8(0xFF047CU, 0x20U);
            bus.write8(0xFF0480U, 0x20U);
            result.unresolved_calls +=
                run_post_object_entity_initializer(bus, 0xFF0400U);
            bus.write8(0xFF0400U + 0x4CU, 4);
            bus.write8(0xFF0400U + 0x9CU, 4);
            bus.write8(0xFF0400U + 0x10U, 0x21U);
        }
    } else if (state == 0U) {
        const auto timer = static_cast<std::uint16_t>(
            bus.read16(state_address + 0x24U) - 1U);
        bus.write16(state_address + 0x24U, timer);
        if (static_cast<std::int16_t>(timer) >= 0) {
            result.unresolved_calls += step_post_object_state_0x5D416(
                bus, state_address, next_output, next_ordinal);
        } else {
            refresh_post_object_background(bus);
            bus.write16(state_address + 0x22U, 1);
            bus.write16(state_address + 0x24U, 0x100U);
        }
    } else {
        const auto scroll = bus.read16(state_address + 4U);
        if (static_cast<std::int16_t>(scroll) >= 0 && scroll < 0x248U) {
            bus.write16(state_address + 4U, static_cast<std::uint16_t>(
                scroll + 8U));
        }
        emit_common();
        update_post_object_timer_0x5D5FC(
            bus, state_address, next_output, next_ordinal,
            max_records_per_list, result);
        update_post_object_timer_0x5D5A2(
            bus, state_address, next_output, next_ordinal,
            max_records_per_list, result);
        emit_post_object_timer_0x5D57E(
            bus, state_address, next_output, next_ordinal,
            max_records_per_list, result);
        const auto timer = static_cast<std::uint16_t>(
            bus.read16(state_address + 0x24U) - 1U);
        bus.write16(state_address + 0x24U, timer);
        if (static_cast<std::int16_t>(timer) < 0) {
            bus.write16(state_address + 0x22U, 2);
            bus.write16(state_address + 0x24U, 0x28U);
        }
    }

    result.next_output_address = next_output;
    result.next_ordinal = next_ordinal;
    return result;
}

bool update_entity_state(GenesisBus& bus, GenesisAddress record_address,
                         const FrameEntityRoutine& routine_0x60FA) {
    // ROM 0x600E. The first table lookup uses the record's word at +0x0E as
    // a four-byte index; the second uses the masked word at +0x16 as a byte
    // offset into the selected pointer table.
    auto index = bus.read16(record_address + 0x0EU);
    const auto first_table_entry = static_cast<GenesisAddress>(
        rom::kEntityStatePointerTable +
        static_cast<std::uint16_t>(index << 2U));
    const auto first_pointer = bus.read32(first_table_entry);

    index = static_cast<std::uint16_t>(
        bus.read16(record_address + 0x16U) & 0x7FFFU);
    auto state_pointer = bus.read32(first_pointer + index);
    auto state_index = bus.read16(state_pointer);
    state_pointer += 2U;

    if (static_cast<std::int16_t>(state_index) < 0) {
        state_index = static_cast<std::uint16_t>(state_index & 0x7FFFU);
        if (static_cast<std::int16_t>(state_index) <=
            static_cast<std::int16_t>(bus.read16(record_address + 0x1EU))) {
            bus.write16(record_address + 0x16U,
                        static_cast<std::uint16_t>(
                            bus.read16(record_address + 0x16U) | 0x8000U));
            state_index = static_cast<std::uint16_t>(state_index + 1U);
            bus.write16(record_address + 0x1EU, state_index);
            bus.write16(record_address + 0x18U,
                        bus.read16(state_pointer + state_index));
        } else {
            state_index = bus.read16(record_address + 0x1EU);
            bus.write16(record_address + 0x18U,
                        bus.read16(state_pointer + state_index));
        }
    } else {
        if (static_cast<std::int16_t>(state_index) <=
            static_cast<std::int16_t>(bus.read16(record_address + 0x1EU))) {
            bus.write16(record_address + 0x1EU, 0);
        }
        state_pointer += 2U;
    }

    state_index = bus.read16(record_address + 0x1EU);
    std::uint16_t multiplier = 0;
    if (bus.read16(record_address + 0x0EU) == 0x000DU) {
        multiplier = state_index == 2U ? 0x0090U : 0x0180U;
    } else {
        auto timing = bus.read16(first_pointer + 0x00CAU);
        multiplier = timing >= 0x0016U ? 0x0010U : 0x001CU;
    }
    bus.write16(record_address + 0x18U, multiplier);

    state_index = static_cast<std::uint16_t>(state_index << 2U);
    state_pointer = bus.read32(state_pointer + state_index);
    if (bus.read16(state_pointer) == 0x8002U &&
        (bus.read8(record_address + 0x11U) & 1U) != 0) {
        // ROM 0x60F4 is ORI.B #1,CCR; its carry is therefore always set and
        // the following BCC is never taken. BSET then calls 0x60FA.
        bus.write8(record_address + 0x11U,
                   static_cast<std::uint8_t>(bus.read8(record_address + 0x11U) | 1U));
        if (routine_0x60FA) {
            routine_0x60FA(bus, record_address);
        } else {
            run_entity_frame_0x60FA(bus, record_address, state_pointer);
        }
        bus.write32(record_address + 0x12U, state_pointer);
        return false;
    }

    bus.write32(record_address + 0x12U, state_pointer);
    return false;
}

bool set_entity_flag(GenesisBus& bus, GenesisAddress record_address,
                     GenesisAddress field_offset, std::uint8_t mask) {
    const auto field_address = record_address + field_offset;
    const auto value = bus.read8(field_address);
    const bool was_set = (value & mask) != 0;
    bus.write8(field_address, static_cast<std::uint8_t>(value | mask));
    return was_set;
}

} // namespace

SpriteSlotAllocationResult allocate_sprite_slot_0x1A2CC(GenesisBus& bus) {
    // The 68000 helper keeps D1 and A2 as byte/word addressing accumulators
    // while D0 is the returned byte-slot index. All three are word-sized at
    // the call boundary.
    auto d1 = static_cast<std::uint16_t>(0x49U);
    auto a2 = static_cast<std::uint16_t>(0x0920U);
    for (std::size_t index = 0; index < session_ram::kSpriteSlotCount;
         ++index) {
        if (bus.read8(session_ram::kSpriteSlotFlags + index) == 0) {
            bus.write8(session_ram::kSpriteSlotFlags + index, 1);
            return {true, static_cast<std::uint16_t>(index), d1, a2};
        }
        d1 = static_cast<std::uint16_t>(d1 + 0x18U);
        a2 = static_cast<std::uint16_t>(a2 + 0x0300U);
    }
    return {false, static_cast<std::uint16_t>(session_ram::kSpriteSlotCount),
            d1, a2};
}

PostObjectRecordAllocationResult allocate_post_object_record_0x15A8C(
    GenesisBus& bus) {
    // 0x15A8C starts D0 at nine and advances A1/D0 together through six
    // records. A free record is selected by a clear bit 0 at +0x10.
    for (std::size_t index = 0; index < session_ram::kPostObjectRecordCount;
         ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kPostObjectRecordBase + index * 0x100U);
        if ((bus.read8(record_address + 0x10U) & 1U) == 0) {
            return {true, record_address,
                    static_cast<std::uint16_t>(9U + index)};
        }
    }
    return {false, 0, 15};
}

EntitySlotInitialisationResult initialize_entity_slot_0x15930(
    GenesisBus& bus, GenesisAddress record_address) {
    // 0x15930 begins with the 0x1582A jump into 0x1A2CC. The latter's D1/A2
    // values are part of the record state written by 0x15930, so preserve them
    // in the typed allocation result rather than reducing the call to a bool.
    const auto sprite = allocate_sprite_slot_0x1A2CC(bus);
    if (!sprite.allocated) {
        return {false, false};
    }

    bus.write8(record_address + 0xB2U, 2);
    bus.write16(record_address + 0x32U, sprite.slot_index);
    bus.write16(record_address + 0x2AU, sprite.d1_value);
    bus.write8(record_address + 2U, 0);
    bus.write8(record_address + 3U, 0);

    // D0 is replaced with the fixed command-table index 2 before the command
    // and handler lookups. The table values are read from the ROM image so the
    // call remains data-driven even though this exact entry is known to be the
    // 0x49A0 RTS handler.
    const auto command = bus.read16(rom::kEntitySlotCommandTable + 4U);
    if (command != 0) {
        dispatch_z80_command_0x530DE(bus, static_cast<std::uint8_t>(command));
    }
    const auto handler = bus.read32(rom::kEntitySlotHandlerTable + 8U);
    return {true, handler != rom::kEntitySlotNoOpHandler};
}

EntitySpawnSlotResult update_entity_spawn_slot_0x16B98(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    // 0x16E08: the ten 18-byte slots use bit 0 at +0x10 as their allocation
    // flag. The allocator itself only returns A1; each caller sets the flag
    // after initializing the selected slot.
    GenesisAddress slot_address = 0;
    for (std::size_t index = 0; index < kEntitySlotCount; ++index) {
        const auto candidate = static_cast<GenesisAddress>(
            kEntitySlotBase + index * kEntitySlotStride);
        if ((bus.read8(candidate + 0x10U) & 1U) == 0) {
            slot_address = candidate;
            break;
        }
    }
    if (slot_address == 0) {
        return {false, false, 0, 0};
    }

    const auto entity_type = bus.read16(entity_record_address + 0x0EU);
    const auto special_flags = bus.read8(session_ram::kEntitySpawnFlags);
    const auto special_type_d = entity_type == 0x000DU;
    const auto special_type_c = entity_type == 0x000CU;

    // The first branch accepts type D only when spawn flag bit 0 is set and
    // type C only when spawn flag bit 1 is set. Both use their fixed frame
    // values and bypass the weighted table.
    if (special_type_d && (special_flags & 0x01U) == 0) {
        return {false, false, 0, 0};
    }
    if (special_type_c && (special_flags & 0x02U) == 0) {
        return {false, false, 0, 0};
    }

    const auto record_counter = bus.read16(entity_record_address + 0x2CU);
    const auto session_mode = bus.read16(session_ram::kSessionMode);
    if (!special_type_d && !special_type_c) {
        // 0x16B98's ordinary path is available only in session modes 0 and 2,
        // excludes type E, and is gated by FFFC0E.
        if (session_mode != 0 && session_mode != 2) {
            return {false, false, 0, 0};
        }
        if (entity_type == 0x000EU ||
            bus.read16(session_ram::kEntitySpawnGate) != 0) {
            return {false, false, 0, 0};
        }
    }

    const auto write_position = [&]() {
        bus.write16(slot_address, static_cast<std::uint16_t>(
            bus.read16(entity_record_address) + 0x0080U));
        bus.write16(slot_address + 4U, static_cast<std::uint16_t>(
            bus.read16(entity_record_address + 4U) + 0x0080U));
    };

    if (special_type_c || special_type_d) {
        write_position();
        bus.write8(slot_address + 0x10U, static_cast<std::uint8_t>(
            bus.read8(slot_address + 0x10U) | 1U));
        const auto fixed_frame = static_cast<std::uint16_t>(
            special_type_c ? 0x0065U : 0x0064U);
        bus.write16(slot_address + 2U, fixed_frame);
        if (special_type_c) {
            // 0x16D60 additionally stores the low word of A0, arms the
            // source entity, and installs the fixed no-op handler.
            bus.write16(slot_address + 6U,
                        static_cast<std::uint16_t>(entity_record_address));
            bus.write16(entity_record_address + 0x16U, 0x0080U);
            bus.write8(entity_record_address + 0x11U,
                       static_cast<std::uint8_t>(
                           bus.read8(entity_record_address + 0x11U) | 1U));
            bus.write32(entity_record_address + 0x3AU,
                        rom::kEntitySlotNoOpHandler);
        }
        return {true, true, slot_address, fixed_frame};
    }

    write_position();
    bus.write8(slot_address + 0x10U, static_cast<std::uint8_t>(
        bus.read8(slot_address + 0x10U) | 1U));

    GenesisAddress animation_table = 0;
    if (session_mode == 2) {
        animation_table = rom::kEntitySpawnAnimationTableMode2;
    } else {
        animation_table = rom::kEntitySpawnAnimationTableDefault;
        if (record_counter != 0 || entity_type != 7U) {
            const auto table_offset = static_cast<GenesisAddress>(
                static_cast<std::uint32_t>(session_mode) * 4U);
            animation_table = bus.read32(
                rom::kEntitySpawnAnimationPointerTable + table_offset);
        }
    }

    // 0x16C70: D0 is a bounded random value in [0, 100), then the table is
    // consumed as {frame, weight} pairs until the unsigned subtraction
    // borrows. Valid ROM tables always terminate within this defensive bound.
    auto remaining = ShadowrunRuntime(bus).random_below(100U);
    std::uint16_t selected_frame = 0;
    bool selected = false;
    for (std::size_t pair = 0; pair < 0x100U; ++pair) {
        const auto frame = bus.read16(animation_table);
        const auto weight = bus.read16(animation_table + 2U);
        animation_table += 4U;
        if (remaining < weight) {
            selected_frame = frame;
            selected = true;
            break;
        }
        remaining = static_cast<std::uint16_t>(remaining - weight);
    }
    if (!selected) {
        // This cannot occur with the shipped tables, but avoids inventing a
        // frame if a caller supplies a truncated synthetic ROM image.
        bus.write8(slot_address + 0x10U, static_cast<std::uint8_t>(
            bus.read8(slot_address + 0x10U) & ~1U));
        return {true, false, slot_address, 0};
    }

    if (selected_frame == 0x005AU) {
        const auto subtype = static_cast<std::int8_t>(
            bus.read8(entity_record_address + 0x57U));
        if (static_cast<std::int8_t>(bus.read8(entity_record_address + 0x56U)) <
                0 || subtype <= 0 || subtype > 0x0E) {
            bus.write8(slot_address + 0x10U, static_cast<std::uint8_t>(
                bus.read8(slot_address + 0x10U) & ~1U));
            return {true, false, slot_address, 0};
        }
        selected_frame = static_cast<std::uint16_t>(subtype);
    } else if (selected_frame == 0x005BU) {
        bus.write8(slot_address + 0x10U, static_cast<std::uint8_t>(
            bus.read8(slot_address + 0x10U) & ~1U));
        return {true, false, slot_address, 0};
    }

    bus.write16(slot_address + 2U, selected_frame);
    return {true, true, slot_address, selected_frame};
}

void clear_entity_relationships_0x1DBF0(GenesisBus& bus) {
    bus.write16(session_ram::kFrameEntityGlobalGate, 0);
    for (std::size_t index = 0; index < 10U; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase + index * 0x100U);
        bus.write32(record_address + 0xA2U, 0);
        bus.write32(record_address + 0xACU, 0);
        bus.write8(record_address + 0x43U, 0);
        bus.write8(record_address + 0xB0U, static_cast<std::uint8_t>(
            bus.read8(record_address + 0xB0U) & ~0x40U));
        bus.write8(record_address + 0x11U, static_cast<std::uint8_t>(
            bus.read8(record_address + 0x11U) & ~0x10U));
    }
}

void arm_entity_global_gate_0x1DBAA(GenesisBus& bus) {
    if (bus.read16(session_ram::kSessionMode) == 0x0036U &&
        bus.read8(session_ram::kFrameEntityTransitionCounter) == 0) {
        return;
    }
    if (bus.read16(session_ram::kFrameEntityGlobalGate) == 0) {
        bus.write16(session_ram::kFrameEntityGlobalGate, 0xFFFFU);
    }
    bus.write16(session_ram::kFrameEntityGateTimer, 0x0078U);
    if (bus.read32(session_ram::kFrameEntityGateSource) != 0) {
        bus.write16(session_ram::kFrameEntityGateTimer, 0x0258U);
    }
}

namespace {

std::uint16_t select_entity_reaction_offset_row(GenesisBus& bus) {
    std::uint16_t selector = 0;
    for (std::size_t index = 0; index < 3U; ++index) {
        const auto value = bus.read8(
            session_ram::kEntityReactionInputFlags + index);
        selector = value;
        if (static_cast<std::int8_t>(value) < 0) {
            break;
        }
    }
    return static_cast<std::uint16_t>(
        ((selector << 3U) | (selector >> 5U)) & 0x03U);
}

} // namespace

bool prepare_entity_reaction_offsets_0xBC36(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    // 0xBC36 first selects a twelve-byte row from the four-row table. The
    // byte used is the first negative input flag among FF EF10..12, or the
    // third byte when none of those three is negative; ROL/AND then reduce
    // it to a row in [0, 3].
    const auto selector = select_entity_reaction_offset_row(bus);
    if (bus.read16(ram::kVblankAnimationMode) != 0) {
        return false;
    }
    const auto entity_x = bus.read16(entity_record_address);
    const auto entity_y = bus.read16(entity_record_address + 4U);
    for (std::size_t index = 0; index < 3U; ++index) {
        const auto table_offset = static_cast<GenesisAddress>(
            selector * 0x0CU + index * 4U);
        bus.write16(session_ram::kFrameEntityReactionOffsets + index * 4U,
                    static_cast<std::uint16_t>(
                        entity_x + bus.read16(
                            rom::kEntityReactionOffsetTable + table_offset)));
        bus.write16(session_ram::kFrameEntityReactionOffsets + index * 4U + 2U,
                    static_cast<std::uint16_t>(
                        entity_y + bus.read16(
                            rom::kEntityReactionOffsetTable + table_offset + 2U)));
    }
    bus.write16(session_ram::kFrameEntityReactionCameraX,
                bus.read16(ram::kCameraX));
    bus.write16(session_ram::kFrameEntityReactionCameraY,
                bus.read16(ram::kCameraY));
    return true;
}

std::int16_t calculate_entity_reaction_band_0x423C(
    GenesisBus& bus, std::uint16_t d0, std::uint16_t d1) {
    // Keep every intermediate at 68000 word width. ASR is signed while the
    // logical shifts and arithmetic below wrap as word operations.
    const auto asr_word = [](std::uint16_t value, unsigned shift) {
        return static_cast<std::uint16_t>(
            static_cast<std::int16_t>(value) >> shift);
    };
    d0 = static_cast<std::uint16_t>(d0 - bus.read16(ram::kCameraX));
    d1 = static_cast<std::uint16_t>(d1 - bus.read16(ram::kCameraY));
    auto d3 = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(0U - bus.read16(ram::kVdpDataWord)) &
        0x01FFU);
    auto d4 = static_cast<std::uint16_t>(
        bus.read16(ram::kVdpRegisterWord0A) & 0x00F8U);
    d0 = asr_word(d0, 4U);
    d1 = asr_word(d1, 4U);
    d3 = static_cast<std::uint16_t>(d3 >> 2U);
    d4 = static_cast<std::uint16_t>(d4 << 4U);
    d4 = static_cast<std::uint16_t>(d4 + d3);
    d4 = static_cast<std::uint16_t>(d4 & 0xFFFEU);
    d0 = static_cast<std::uint16_t>(asr_word(d0, 3U) << 1U);
    d1 = static_cast<std::uint16_t>(asr_word(d1, 3U) << 7U);
    d3 = static_cast<std::uint16_t>(d4 & 0xFF80U);
    d4 = static_cast<std::uint16_t>(d4 + d0);
    d4 = static_cast<std::uint16_t>(d4 & 0x007EU);
    d4 = static_cast<std::uint16_t>(d4 | d3);
    d4 = static_cast<std::uint16_t>(d4 + d1);
    return static_cast<std::int16_t>(d4 & 0x0FFFU);
}

std::uint16_t calculate_entity_reaction_tile_index_0x1669C(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    auto x = static_cast<std::uint16_t>(
        bus.read16(entity_record_address) >> 4U);
    auto y = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 4U) >> 4U);
    const auto direction = bus.read16(entity_record_address + 0x2EU);
    if (direction < 0x8000U) {
        x = static_cast<std::uint16_t>(x + 0x10U);
    } else {
        x = static_cast<std::uint16_t>(x - 0x10U);
    }
    x = static_cast<std::uint16_t>(x >> 5U);
    y = static_cast<std::uint16_t>((y >> 5U) - 1U);
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(x << 1U) +
        static_cast<std::uint32_t>(y) * bus.read16(ram::kTilePlaneWidth));
}

std::uint16_t calculate_entity_reaction_tile_index_0x166D0(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    auto x = static_cast<std::uint16_t>(
        bus.read16(entity_record_address) >> 4U);
    auto y = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 4U) >> 4U);
    const auto direction = bus.read16(entity_record_address + 0x2EU);
    if (direction < 0x4000U || direction >= 0xC000U) {
        y = static_cast<std::uint16_t>(y + 0x10U);
    } else {
        y = static_cast<std::uint16_t>(y - 0x10U);
    }
    x = static_cast<std::uint16_t>(x >> 5U);
    y = static_cast<std::uint16_t>((y >> 5U) - 1U);
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(x << 1U) +
        static_cast<std::uint32_t>(y) * bus.read16(ram::kTilePlaneWidth));
}

bool mark_entity_reaction_tile_0x16760(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    const auto tile_index = calculate_entity_reaction_tile_index_0x166D0(
        bus, entity_record_address);
    const auto tile_address = static_cast<GenesisAddress>(
        ram::kTileMapIndexTable + tile_index +
        bus.read16(ram::kTilePlaneWidth));
    if (bus.read16(tile_address) == 0x000BU) {
        return false;
    }
    const auto counter_address = static_cast<GenesisAddress>(
        session_ram::kEntityReactionTileCounterBase +
        bus.read16(session_ram::kEntityReactionTileCounterIndex));
    bus.write8(counter_address, static_cast<std::uint8_t>(
        bus.read8(counter_address) + 1U));
    bus.write16(tile_address, 0x000BU);
    bus.write16(ram::kBackgroundQueueBusy, 1);
    return true;
}

bool run_entity_reaction_map_0x1651E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityReactionRegisterContext& registers) {
    const auto mode = bus.read16(ram::kVblankAnimationMode);
    if (mode == 0) {
        if (bus.read16(session_ram::kSessionMode) == 4U &&
            (bus.read16(entity_record_address + 0xC6U) & 0x001FU) == 6U) {
            bus.write32(0x00FFE114U, 0x0001667AU);
        }
        if ((bus.read16(entity_record_address + 0xC6U) & 0x00E0U) !=
            0x00C0U) {
            return true;
        }

        // ROM 0x164F2: return the first empty 0x1E-byte slot in the ten-entry
        // table. The source also increments a transient D1 index, which no
        // caller consumes after this allocation.
        auto output = session_ram::kEntityReactionOutputTable;
        for (std::size_t index = 0; index < 10U; ++index) {
            if (bus.read16(output) == 0) {
                break;
            }
            output = static_cast<GenesisAddress>(output + 0x1EU);
        }
        bus.write16(output, 0xFFFFU);
        bus.write16(output + 6U, static_cast<std::uint16_t>(
            registers.d4 + bus.read16(0x00FFDD16U)));
        bus.write16(output + 0x0CU, 3U);
        bus.write16(output + 0x0EU, 3U);
        bus.write32(output + 8U, 0x0001669AU);
        return true;
    }

    if (registers.d0 == 0) {
        return true;
    }

    const auto auxiliary = static_cast<GenesisAddress>(
        registers.auxiliary_base +
        bus.read16(session_ram::kEntityReactionAuxiliaryOffset));
    if (bus.read16(session_ram::kSessionMode) == 0x0014U) {
        bus.write8(auxiliary, 0);
        const auto tile_index = calculate_entity_reaction_tile_index_0x166D0(
            bus, entity_record_address);
        const auto tile_address = static_cast<GenesisAddress>(
            ram::kTileMapIndexTable + tile_index);
        bus.write16(tile_address, bus.read16(0x001CD296U));
        bus.write16(tile_address + bus.read16(ram::kTilePlaneWidth),
                    bus.read16(0x001CD298U));
        bus.write16(ram::kVblankQueueA, 1);
        return false;
    }

    if (mode != 2U) {
        return false;
    }

    bus.write8(auxiliary, 0);
    const auto collision_kind = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0xC6U) & 0x00E0U);
    if (collision_kind == 0x00A0U) {
        const auto tile_index = calculate_entity_reaction_tile_index_0x1669C(
            bus, entity_record_address);
        auto tile_address = static_cast<GenesisAddress>(
            session_ram::kEntityReactionAlternateTileMap + tile_index -
            bus.read16(ram::kTilePlaneWidth));
        bus.write16(tile_address, 3U);
        tile_address = static_cast<GenesisAddress>(
            tile_address + bus.read16(ram::kTilePlaneWidth));
        bus.write16(tile_address, 4U);
        tile_address = static_cast<GenesisAddress>(
            tile_address + bus.read16(ram::kTilePlaneWidth));
        bus.write16(tile_address, 5U);
        bus.write16(ram::kVblankQueueB, 1);
    } else {
        const auto tile_index = calculate_entity_reaction_tile_index_0x166D0(
            bus, entity_record_address);
        const auto tile_address = static_cast<GenesisAddress>(
            ram::kTileMapIndexTable + tile_index);
        bus.write16(tile_address, 6U);
        bus.write16(tile_address + bus.read16(ram::kTilePlaneWidth), 7U);
        bus.write16(ram::kVblankQueueA, 1);
    }
    return false;
}

namespace {

GenesisAddress sign_extend_terrain_index_address(
    GenesisAddress base, std::uint16_t index) {
    return static_cast<GenesisAddress>(
        static_cast<std::int32_t>(base) +
        static_cast<std::int16_t>(index));
}

std::uint16_t terrain_map_offset_0xE8A0(
    std::uint16_t d5, std::uint16_t d6,
    std::int16_t d5_delta, std::int16_t d6_delta) {
    auto d1 = static_cast<std::uint16_t>(
        d5 + static_cast<std::uint16_t>(d5_delta));
    auto d2 = static_cast<std::uint16_t>(
        d6 + static_cast<std::uint16_t>(d6_delta));
    d2 = static_cast<std::uint16_t>(d2 << 1U);
    d2 = static_cast<std::uint16_t>(d2 & 0xFFC0U);
    d1 = static_cast<std::uint16_t>(d1 >> 5U);
    return static_cast<std::uint16_t>(d2 + d1);
}

std::uint8_t read_terrain_byte(GenesisBus& bus, std::uint16_t offset) {
    const auto address = sign_extend_terrain_index_address(
        ram::kIndexedObjectByteTable, offset);
    return bus.read8(address);
}

struct TerrainGeometry {
    std::uint16_t d1;
    std::uint16_t d2;
    std::uint16_t d3;
};

TerrainGeometry terrain_geometry_0xEDFA(std::uint16_t offset) {
    auto d1 = offset;
    auto d2 = static_cast<std::uint16_t>(offset & 0xFFC0U);
    d2 = static_cast<std::uint16_t>(d2 >> 1U);
    d1 = static_cast<std::uint16_t>(d1 & 0x003FU);
    d1 = static_cast<std::uint16_t>(d1 << 5U);
    const auto d3 = static_cast<std::uint16_t>(d2 + 0x001FU);
    return {d1, d2, d3};
}

bool terrain_collision_test_0xEAD2(
    GenesisBus& bus,
    std::uint8_t status,
    std::uint16_t d5,
    std::uint16_t d6) {
    const auto geometry = terrain_geometry_0xEDFA(
        bus.read16(session_ram::kEntityTerrainSelectedOffset));
    const auto kind = static_cast<std::uint16_t>(status & 0x0038U);
    const auto negate_difference = [](std::uint16_t left,
                                      std::uint16_t right) {
        return static_cast<std::uint16_t>(
            0U - static_cast<std::uint16_t>(left - right));
    };

    if (kind == 0x0020U) {
        const auto distance_x = negate_difference(geometry.d1, d5);
        const auto distance_y = static_cast<std::uint16_t>(
            geometry.d3 - d6);
        // CMP.W D3,D1 followed by BCS: carry clear is the collision case.
        return distance_x >= distance_y;
    }

    if (kind == 0x0028U) {
        auto left = geometry.d1;
        auto right = geometry.d2;
        std::swap(left, right); // EXG.L D1,D2; only the low words matter.
        left = negate_difference(left, d6);
        right = negate_difference(right, d5);
        // CMP.W D1,D2 followed by BCC: carry clear is the collision case's
        // complement, so equality is also a non-collision result here.
        return right < left;
    }

    if (kind == 0x0038U) {
        const auto distance_x = negate_difference(geometry.d1, d5);
        const auto distance_y = static_cast<std::uint16_t>(
            geometry.d3 - d6);
        // This branch keeps D1/D3 unswapped and uses BCC instead of BCS.
        return distance_x < distance_y;
    }

    auto distance_x = negate_difference(geometry.d1, d5);
    const auto distance_y = negate_difference(geometry.d2, d6);
    // CMP.W D1,D2 followed by BCS: carry clear is the collision case.
    return distance_x >= distance_y;
}

bool terrain_collision_test_0xEA36(
    GenesisBus& bus,
    std::uint16_t d5,
    std::uint16_t d6) {
    struct Corner {
        GenesisAddress flag;
        std::int16_t d5_delta;
        std::int16_t d6_delta;
    };
    constexpr std::array<Corner, 4> corners{{
        {session_ram::kEntityReactionInputFlags, static_cast<std::int16_t>(-0x000C), static_cast<std::int16_t>(-0x0007)},
        {session_ram::kEntityReactionInputFlags + 1U,  0x000CU, -0x0007},
        {session_ram::kEntityReactionInputFlags + 2U, static_cast<std::int16_t>(-0x000C), static_cast<std::int16_t>(0x0007)},
        {session_ram::kEntityReactionInputFlags + 3U,  0x000CU,  0x0007},
    }};

    for (const auto& corner : corners) {
        const auto status = bus.read8(corner.flag);
        if ((status & 0x20U) == 0) {
            continue;
        }
        const auto adjusted_d5 = static_cast<std::uint16_t>(
            d5 + static_cast<std::uint16_t>(corner.d5_delta));
        const auto adjusted_d6 = static_cast<std::uint16_t>(
            d6 + static_cast<std::uint16_t>(corner.d6_delta));
        if (terrain_collision_test_0xEAD2(
                bus, status, adjusted_d5, adjusted_d6)) {
            return true;
        }
    }
    return false;
}

std::uint8_t sample_terrain_map_0xE8A0(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5,
    std::uint16_t d6) {
    bus.write16(session_ram::kEntityTerrainCollisionMarker, 0);

    if ((bus.read8(entity_record_address + 0x10U) & 0x04U) == 0) {
        const auto offset = terrain_map_offset_0xE8A0(d5, d6, 0, 0);
        const auto status = read_terrain_byte(bus, offset);
        bus.write16(session_ram::kEntityTerrainSelectedOffset, offset);
        bus.write8(session_ram::kEntityReactionInputFlags, status);
        return status;
    }

    const auto first_offset = terrain_map_offset_0xE8A0(d5, d6, -0x000C, -0x0007);
    auto best = read_terrain_byte(bus, first_offset);
    bus.write16(session_ram::kEntityTerrainSelectedOffset, first_offset);
    bus.write8(session_ram::kEntityReactionInputFlags, best);
    if (best == 0x7FU) {
        bus.write8(session_ram::kEntityTerrainCollisionMarker, best);
    }

    const auto second_offset = terrain_map_offset_0xE8A0(d5, d6, 0x000C, -0x0007);
    auto candidate = read_terrain_byte(bus, second_offset);
    bus.write8(session_ram::kEntityReactionInputFlags + 1U, candidate);
    if (candidate == 0x7FU) {
        bus.write8(session_ram::kEntityTerrainCollisionMarker, candidate);
    }
    if (candidate >= best) {
        best = candidate;
        bus.write16(session_ram::kEntityTerrainSelectedOffset, second_offset);
    }

    const auto third_offset = terrain_map_offset_0xE8A0(d5, d6, -0x000C, 0x0007);
    candidate = read_terrain_byte(bus, third_offset);
    bus.write8(session_ram::kEntityReactionInputFlags + 2U, candidate);
    if (candidate == 0x7FU) {
        bus.write8(session_ram::kEntityTerrainCollisionMarker, candidate);
    }
    if (candidate >= best) {
        best = candidate;
        bus.write16(session_ram::kEntityTerrainSelectedOffset, third_offset);
    }

    const auto fourth_offset = terrain_map_offset_0xE8A0(d5, d6, 0x000C, 0x0007);
    candidate = read_terrain_byte(bus, fourth_offset);
    bus.write8(session_ram::kEntityReactionInputFlags + 3U, candidate);
    if (candidate == 0x7FU) {
        bus.write8(session_ram::kEntityTerrainCollisionMarker, candidate);
    }
    // The ROM updates the selected offset here but does not copy D0 into D7;
    // preserve that asymmetry instead of normalizing it into a max operation.
    if (candidate >= best) {
        bus.write16(session_ram::kEntityTerrainSelectedOffset, fourth_offset);
    }

    bus.write8(session_ram::kEntityTerrainCollisionMarker + 1U, 0);
    if (bus.read8(session_ram::kEntityTerrainCollisionMarker) != 0) {
        return bus.read8(session_ram::kEntityTerrainCollisionMarker);
    }
    return best;
}

} // namespace

EntityTerrainSampleResult sample_entity_terrain_0xE7C8(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5,
    std::uint16_t d6,
    const EntityTerrainHooks& hooks) {
    bus.write16(session_ram::kEntityTerrainOutOfBounds, 0);
    bus.write16(session_ram::kEntityTerrainCollisionGate, 0);

    d5 = static_cast<std::uint16_t>(d5 >> 4U);
    d6 = static_cast<std::uint16_t>(d6 >> 4U);
    const auto x_lower = bus.read16(session_ram::kEntityTerrainBoundsXLower);
    const auto x_upper = bus.read16(session_ram::kEntityTerrainBoundsXUpper);
    const auto y_lower = bus.read16(session_ram::kEntityTerrainBoundsYLower);
    const auto y_upper = bus.read16(session_ram::kEntityTerrainBoundsYUpper);
    if (d5 < x_lower || d5 >= x_upper || d6 < y_lower || d6 >= y_upper) {
        bus.write16(session_ram::kEntityTerrainOutOfBounds, 0xFFFFU);
        bus.write16(session_ram::kEntityReactionInputFlags, 0x007FU);
        bus.write16(session_ram::kEntityReactionInputFlags + 2U, 0x007FU);
        return {0x007FU, true};
    }

    const auto raw_status = sample_terrain_map_0xE8A0(
        bus, entity_record_address, d5, d6);
    const auto status = static_cast<std::uint16_t>(
        static_cast<std::int16_t>(static_cast<std::int8_t>(raw_status)));
    bus.write16(session_ram::kFrameEntityCollisionState, status);
    if (status == 0 || status == 0x007FU ||
        static_cast<std::int16_t>(status) < 0) {
        return {status, false};
    }

    bool blocked = false;
    if ((status & 0x0040U) != 0) {
        bus.write16(session_ram::kEntityTerrainCollisionGate, 0xFFFFU);
        if (status == 0x007EU || (status & 0x0020U) == 0) {
            return {0x007FU, false};
        }
        blocked = hooks.routine_0xEA36
            ? hooks.routine_0xEA36(bus, entity_record_address, d5, d6)
            : terrain_collision_test_0xEA36(bus, d5, d6);
    } else if ((status & 0x0020U) != 0) {
        blocked = hooks.routine_0xEA36
            ? hooks.routine_0xEA36(bus, entity_record_address, d5, d6)
            : terrain_collision_test_0xEA36(bus, d5, d6);
    } else {
        const auto record_base = bus.read32(session_ram::kEntityTerrainMapRecords);
        const auto table_address = static_cast<GenesisAddress>(
            static_cast<std::int32_t>(record_base) +
            static_cast<std::int16_t>(
                static_cast<std::uint16_t>(status * 10U)));
        const auto x_record = bus.read32(table_address);
        const auto y_record = bus.read32(table_address + 4U);
        const auto x_center = static_cast<std::uint16_t>(x_record >> 16U);
        const auto x_extent = static_cast<std::uint16_t>(x_record);
        const auto y_center = static_cast<std::uint16_t>(y_record >> 16U);
        const auto y_extent = static_cast<std::uint16_t>(y_record);
        const auto x_min = static_cast<std::uint16_t>(x_center - 0x000CU);
        const auto x_max = static_cast<std::uint16_t>(x_extent + 0x000CU);
        const auto y_min = static_cast<std::uint16_t>(y_center - 0x0007U);
        const auto y_max = static_cast<std::uint16_t>(y_extent + 0x0007U);
        blocked = static_cast<std::int16_t>(d5) >=
                      static_cast<std::int16_t>(x_min) &&
                  static_cast<std::int16_t>(d5) <=
                      static_cast<std::int16_t>(x_max) &&
                  static_cast<std::int16_t>(d6) >=
                      static_cast<std::int16_t>(y_min) &&
                  static_cast<std::int16_t>(d6) <=
                      static_cast<std::int16_t>(y_max);
    }

    return {static_cast<std::uint16_t>(blocked ? 0x007FU : 0), false};
}

EntityReactionPassResult run_entity_reaction_0xB7B8(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityReactionRegisterContext& registers,
    const EntityReactionHooks& hooks) {
    EntityReactionPassResult result{false, false, 0, 0};

    if (bus.read16(session_ram::kMainLoopControl) != 0) {
        return result;
    }

    const auto old_b0 = bus.read8(entity_record_address + 0xB0U);
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(old_b0 | 0x20U));
    // BSET reports the previous bit in Z. A second in-flight reaction leaves
    // the bit armed and returns through 0xB7CC without touching bit 0.
    if ((old_b0 & 0x20U) != 0) {
        return result;
    }
    result.entered = true;

    const auto finish = [&]() {
        const auto flags = bus.read8(entity_record_address + 0xB0U);
        bus.write8(entity_record_address + 0xB0U,
                   static_cast<std::uint8_t>((flags & ~0x20U) | 1U));
    };

    if ((bus.read8(entity_record_address + 0x10U) & 0x02U) == 0) {
        finish();
        return result;
    }

    const auto mode = bus.read16(ram::kVblankAnimationMode);
    const auto session = bus.read16(session_ram::kSessionMode);
    if (mode != 2U && session != 0x0014U &&
        bus.read16(session_ram::kFrameEntityGlobalGate) != 0) {
        finish();
        return result;
    }

    std::size_t negative_inputs = 0;
    for (std::size_t index = 0; index < 4U; ++index) {
        if (static_cast<std::int8_t>(bus.read8(
                session_ram::kEntityReactionInputFlags + index)) < 0) {
            ++negative_inputs;
        }
    }
    if (negative_inputs < 2U) {
        finish();
        return result;
    }

    // 0xBC36 and 0x423C are state-only helpers. Their writes and word-sized
    // arithmetic are retained even though the 0xB7B8 body only consumes the
    // resulting 0x423C D4 value through the later 0x1651E boundary.
    prepare_entity_reaction_offsets_0xBC36(bus, entity_record_address);
    // BC36 leaves the selected row in D4 before B7B8 rotates it.
    const auto reaction_selector = select_entity_reaction_offset_row(bus);
    const auto rotated_d4 = static_cast<std::uint16_t>(
        (reaction_selector >> 2U) | (reaction_selector << 14U));
    bus.write16(entity_record_address + 0xCCU, rotated_d4);
    const auto d0_for_423c = static_cast<std::uint16_t>(
        (registers.d5 & 0xFE00U) + 0x0070U);
    const auto d1_for_423c = static_cast<std::uint16_t>(
        (registers.d6 & 0xFE00U) - 0x0190U);
    const auto reaction_position = static_cast<std::uint16_t>(
        calculate_entity_reaction_band_0x423C(
            bus, d0_for_423c, d1_for_423c));

    const auto collision_state = bus.read16(
        session_ram::kFrameEntityCollisionState);
    bus.write16(entity_record_address + 0xC6U, collision_state);
    auto state = static_cast<std::uint16_t>(collision_state & 0x001FU);

    const auto request_resource = [&](GenesisAddress resource,
                                      bool unconditional) {
        bus.write32(ram::kResourcePointer, resource);
        if (unconditional) {
            queue_resource_load_0x1E2EC(bus);
        } else {
            queue_resource_load_0x1E2E0(bus);
        }
        ++result.resource_loads;
    };

    if (state == 0x001FU) {
        request_resource(0x0000DB26U, false);
        finish();
        return result;
    }
    if (state == 0x001EU) {
        request_resource(0x00020990U, false);
        finish();
        return result;
    }
    if (state == 0x001DU) {
        if (bus.read8(session_ram::kEntityReactionSpecialGate) == 0) {
            request_resource(0x00058D94U, false);
        }
        finish();
        return result;
    }
    if (state == 0x001CU) {
        if (bus.read8(session_ram::kEntityReactionSpecialGate) == 0) {
            request_resource(0x00058DB2U, false);
        }
        finish();
        return result;
    }
    if (state == 0x001BU) {
        request_resource(0x00053D5AU, false);
        finish();
        return result;
    }
    if (state == 0x001AU) {
        bus.write8(session_ram::kEntityReactionStateByte, 0);
        const auto c6_low = static_cast<std::uint8_t>(
            bus.read16(entity_record_address + 0xC6U));
        if (c6_low != 0xDAU &&
            (bus.read8(session_ram::kEntityReactionResetInput) & 0x80U) == 0) {
            bus.write8(session_ram::kEntityReactionStateByte, 1);
        }
        if (mark_entity_reaction_tile_0x16760(
                bus, entity_record_address)) {
            request_resource(0x00020C90U, true);
        }
        finish();
        return result;
    }

    if ((session == 0x0014U || mode == 2U) && state >= 2U && state <= 5U &&
        bus.read8(session_ram::kEntityReactionSubmode) == 0) {
        request_resource(0x00020E48U, false);
        bus.write8(session_ram::kEntityReactionSelection,
                   static_cast<std::uint8_t>(state));
        bus.write16(session_ram::kEntityReactionTimer,
                    bus.read16(session_ram::kEntityReactionAuxiliaryOffset));
        finish();
        return result;
    }
    if (state == 0x0018U) {
        request_resource(0x00020EC2U, false);
        finish();
        return result;
    }
    if (state == 0x0019U) {
        if (bus.read16(session_ram::kEntityReactionModeGate) == 0x0013U) {
            request_resource(0x00053EFAU, false);
            finish();
            return result;
        }
        bus.write8(session_ram::kEntityReactionInputFlags, 0xA8U);
        bus.write16(session_ram::kFrameEntityCollisionState, 0x00A8U);
        bus.write16(entity_record_address + 0xC6U, 0x00A8U);
        state = 8U;
    }

    if ((bus.read8(session_ram::kEntityPhaseBase + 0x10U) & 0x50U) != 0) {
        if (hooks.routine_0x53126) {
            hooks.routine_0x53126(bus);
        } else {
            synchronize_z80_0x53126(bus, hooks.frame_wait);
        }
        request_resource(0x0000C64CU, true);
        finish();
        return result;
    }

    bus.write8(session_ram::kFrameEntityResourceSubmode,
               static_cast<std::uint8_t>(state));
    const auto call_1651e = mode == 0U || session == 0x0014U ||
                            mode != 2U || state != 0U ||
                            (bus.read16(session_ram::kFrameEntityGlobalGate) == 0 &&
                             bus.read8(session_ram::kEntityReactionSpecialGate) == 0);
    if (!call_1651e) {
        finish();
        return result;
    }

    auto nested_registers = registers;
    nested_registers.d0 = state;
    nested_registers.d4 = reaction_position;
    bool carried = false;
    if (hooks.routine_0x1651E) {
        carried = hooks.routine_0x1651E(
            bus, entity_record_address, nested_registers);
    } else {
        carried = run_entity_reaction_map_0x1651E(
            bus, entity_record_address, nested_registers);
    }
    if (carried) {
        bus.write16(session_ram::kEntityReactionTransitionState, 0xFFFFU);
        bus.write16(session_ram::kEntityReactionTransitionStep, 0x0010U);
        bus.write16(session_ram::kMainLoopControl, 0xFFFFU);
        result.returned_with_gate = true;
        return result;
    }
    bus.write16(entity_record_address + 0xC6U, collision_state);
    return result;
}

EntityMovementPassResult run_entity_movement_0xBABC(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityReactionRegisterContext& registers,
    const EntityMovementHooks& hooks) {
    EntityMovementPassResult result{
        false, false, false, false, false, 0, 0, 0};

    struct ProbeResult {
        bool known;
        std::uint16_t status;
        std::uint16_t d5;
        std::uint16_t d6;
    };

    const auto rotate_direction_3 = [](std::uint16_t value) {
        return static_cast<std::uint16_t>(
            (value << 3U) | (value >> 13U));
    };
    const auto b77e = [&](std::uint16_t table_offset) {
        const auto d5 = static_cast<std::uint16_t>(
            bus.read16(rom::kEntityMovementDirectionTable + table_offset) +
            bus.read16(entity_record_address));
        const auto d6 = static_cast<std::uint16_t>(
            bus.read16(rom::kEntityMovementDirectionTable + table_offset + 2U) +
            bus.read16(entity_record_address + 4U));
        bus.write16(0x00FFDF66U, table_offset);
        auto status = hooks.routine_0xE7C8
            ? hooks.routine_0xE7C8(bus, entity_record_address, d5, d6)
            : sample_entity_terrain_0xE7C8(
                bus, entity_record_address, d5, d6).status;
        // B48C leaves E7C8's FFDF64 side effect intact, then copies the input
        // flags longword into the entity's +D0 field. A replacement hook is
        // responsible for preserving that same side effect if its caller
        // consumes FFDF64 later in the reaction path.
        bus.write32(entity_record_address + 0xD0U,
                    bus.read32(session_ram::kEntityReactionInputFlags));
        if (static_cast<std::int16_t>(status) < 0 &&
            (bus.read8(entity_record_address + 0x10U) & 0x02U) == 0) {
            status = 0x007FU;
        }
        return ProbeResult{true, status, d5, d6};
    };

    const auto b732 = [&](std::uint16_t table_offset) {
        auto probe = b77e(table_offset);
        if (!probe.known || static_cast<std::int16_t>(probe.status) >= 0) {
            return probe;
        }

        const auto state = static_cast<std::uint16_t>(probe.status & 0x001FU);
        bool blocked_by_reaction =
            bus.read16(session_ram::kFrameEntityGlobalGate) != 0 ||
            state >= 0x0018U;
        if (!blocked_by_reaction &&
            (bus.read16(session_ram::kSessionMode) == 0x0014U ||
             bus.read16(ram::kVblankAnimationMode) == 2U)) {
            blocked_by_reaction = state >= 2U && state <= 5U;
        }
        if (blocked_by_reaction) {
            probe.status = 0x007FU;
        }
        return probe;
    };

    const auto deactivate = [&](const ProbeResult& probe) {
        bus.write8(entity_record_address + 0xB0U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 0xB0U) & ~1U));
        bus.write16(entity_record_address, probe.d5);
        bus.write16(entity_record_address + 4U, probe.d6);
        result.completed = true;
        result.entity_deactivated = true;
        result.final_collision_state = bus.read16(
            session_ram::kFrameEntityCollisionState);
    };

    result.entered = true;
    bus.write16(entity_record_address + 8U, 0xFFF4U);
    const auto rotated = rotate_direction_3(
        bus.read16(entity_record_address + 0x2EU));
    const auto direction_index = static_cast<std::uint16_t>(rotated & 7U);
    bus.write16(entity_record_address + 0xCAU,
                bus.read16(rom::kEntityMovementDirectionTable + 4U));

    auto probe = b77e(static_cast<std::uint16_t>(direction_index << 1U));
    if (!probe.known) {
        result.final_collision_state = bus.read16(
            session_ram::kFrameEntityCollisionState);
        return result;
    }
    result.initial_collision_state = probe.status;
    if (probe.status == 0) {
        deactivate(probe);
        return result;
    }

    if (static_cast<std::int16_t>(probe.status) < 0) {
        const EntityReactionRegisterContext reaction_registers{
            probe.status, registers.d4, probe.d5, probe.d6,
            registers.auxiliary_base};
        EntityReactionPassResult reaction{};
        if (hooks.routine_0xB7B8) {
            reaction = hooks.routine_0xB7B8(
                bus, entity_record_address, reaction_registers);
        } else {
            reaction = run_entity_reaction_0xB7B8(
                bus, entity_record_address, reaction_registers);
        }
        result.reaction_called = true;
        result.unresolved_calls += reaction.unresolved_calls;
        if ((bus.read8(entity_record_address + 0xB0U) & 1U) == 0) {
            bus.write16(entity_record_address + 0xB8U, 0x1000U);
            deactivate(probe);
            return result;
        }
    }

    const auto orientation = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x2EU) & 0x3FFFU);
    if (orientation == 0) {
        // BB86 selects a pair of optional state pointers from the four-row
        // pointer table. Its carry means that no movement candidate passed.
        auto pointer_row = static_cast<std::uint16_t>(
            rotate_direction_3(bus.read16(entity_record_address + 0x2EU)) &
            7U);
        pointer_row = static_cast<std::uint16_t>(pointer_row & ~1U);
        pointer_row = static_cast<std::uint16_t>(pointer_row << 1U);
        auto state_row = bus.read32(
            rom::kEntityMovementSelectionPointerTable + pointer_row);
        auto first_state = bus.read32(state_row);
        if (static_cast<std::int8_t>(bus.read8(first_state)) <= 0) {
            const auto fallback = bus.read16(state_row + 8U);
            auto candidate = b732(static_cast<std::uint16_t>(fallback << 1U));
            if (!candidate.known) {
                result.final_collision_state = bus.read16(
                    session_ram::kFrameEntityCollisionState);
                return result;
            }
            probe = candidate;
            if (static_cast<std::int16_t>(candidate.status) > 0) {
                result.direction_failed = true;
                bus.write16(entity_record_address + 0xB8U, 0xFFFFU);
                bus.write8(entity_record_address + 0xB0U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0xB0U) &
                               ~0x02U));
                bus.write16(entity_record_address + 0x2EU,
                            bus.read16(entity_record_address + 0x34U));
                bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);
                bus.write8(entity_record_address + 0x11U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0x11U) | 1U));
                result.completed = true;
                result.final_collision_state = bus.read16(
                    session_ram::kFrameEntityCollisionState);
                return result;
            }
        } else {
            const auto second_state = bus.read32(state_row + 4U);
            if (static_cast<std::int8_t>(bus.read8(second_state)) > 0) {
                result.direction_failed = true;
                bus.write16(entity_record_address + 0xB8U, 0xFFFFU);
                bus.write8(entity_record_address + 0xB0U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0xB0U) &
                               ~0x02U));
                bus.write16(entity_record_address + 0x2EU,
                            bus.read16(entity_record_address + 0x34U));
                bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);
                bus.write8(entity_record_address + 0x11U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0x11U) | 1U));
                result.completed = true;
                result.final_collision_state = bus.read16(
                    session_ram::kFrameEntityCollisionState);
                return result;
            }
            const auto fallback = bus.read16(state_row + 0x0AU);
            auto candidate = b732(static_cast<std::uint16_t>(fallback << 1U));
            if (!candidate.known) {
                result.final_collision_state = bus.read16(
                    session_ram::kFrameEntityCollisionState);
                return result;
            }
            probe = candidate;
            if (static_cast<std::int16_t>(candidate.status) > 0) {
                result.direction_failed = true;
                bus.write16(entity_record_address + 0xB8U, 0xFFFFU);
                bus.write8(entity_record_address + 0xB0U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0xB0U) &
                               ~0x02U));
                bus.write16(entity_record_address + 0x2EU,
                            bus.read16(entity_record_address + 0x34U));
                bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);
                bus.write8(entity_record_address + 0x11U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0x11U) | 1U));
                result.completed = true;
                result.final_collision_state = bus.read16(
                    session_ram::kFrameEntityCollisionState);
                return result;
            }
        }
    } else {
        const auto first_direction = static_cast<std::uint16_t>(
            rotate_direction_3(orientation) & 6U);
        bus.write16(0x00FFDD3CU, first_direction);
        bus.write16(entity_record_address + 0xB8U, first_direction);
        auto first = b732(static_cast<std::uint16_t>(first_direction << 1U));
        if (!first.known) {
            result.final_collision_state = bus.read16(
                session_ram::kFrameEntityCollisionState);
            return result;
        }
        probe = first;
        if (first.status != 0 &&
            static_cast<std::int16_t>(first.status) >= 0) {
            const auto second_direction = static_cast<std::uint16_t>(
                (bus.read16(0x00FFDD3CU) + 2U) & 7U);
            bus.write16(entity_record_address + 0xB8U, second_direction);
            auto second = b732(static_cast<std::uint16_t>(
                second_direction << 1U));
            if (!second.known) {
                result.final_collision_state = bus.read16(
                    session_ram::kFrameEntityCollisionState);
                return result;
            }
            probe = second;
            if (second.status != 0 &&
                static_cast<std::int16_t>(second.status) >= 0) {
                result.direction_failed = true;
                bus.write16(entity_record_address + 0xB8U, 0xFFFFU);
                bus.write8(entity_record_address + 0xB0U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0xB0U) &
                               ~0x02U));
                bus.write16(entity_record_address + 0x2EU,
                            bus.read16(entity_record_address + 0x34U));
                bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);
                bus.write8(entity_record_address + 0x11U,
                           static_cast<std::uint8_t>(
                               bus.read8(entity_record_address + 0x11U) | 1U));
                result.completed = true;
                result.final_collision_state = bus.read16(
                    session_ram::kFrameEntityCollisionState);
                return result;
            }
        }
    }

    bus.write16(entity_record_address + 0x36U,
                static_cast<std::uint16_t>(
                    (bus.read16(0x00FFDF66U) >> 4U) |
                    (bus.read16(0x00FFDF66U) << 12U)));
    bus.write8(entity_record_address + 0x11U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0x11U) | 0x04U));
    deactivate(probe);
    return result;
}

namespace {

bool is_primary_frame_list_record(GenesisBus& bus,
                                  GenesisAddress record_address) {
    const auto flags_10 = bus.read8(record_address + 0x10U);
    if ((flags_10 & 1U) == 0 || (flags_10 & 0x40U) != 0 ||
        (flags_10 & 0x10U) != 0 ||
        (bus.read8(record_address + 0xB0U) & 0x80U) != 0 ||
        (bus.read8(record_address + 0x9EU) & 0x20U) != 0 ||
        (bus.read8(record_address + 0x9EU) & 0x08U) != 0) {
        return false;
    }
    return bus.read16(record_address + 0x0CU) != 0;
}

bool is_secondary_frame_list_record(GenesisBus& bus,
                                    GenesisAddress record_address) {
    const auto flags_10 = bus.read8(record_address + 0x10U);
    if ((flags_10 & 1U) == 0 || (flags_10 & 0x40U) != 0 ||
        (flags_10 & 0x10U) != 0) {
        return false;
    }
    return bus.read16(record_address + 0x0CU) != 0;
}

std::size_t build_primary_frame_list(GenesisBus& bus) {
    auto output = session_ram::kFrameListPrimary;
    std::size_t count = 0;
    // MOVE.W #6,D7 followed by DBF visits seven records: FF0400 through
    // FF0A00 inclusive.
    for (std::size_t index = 0; index <= 6U; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            0xFF0400U + index * 0x100U);
        if (!is_primary_frame_list_record(bus, record_address)) {
            continue;
        }
        bus.write32(output, record_address);
        output += 4U;
        ++count;
    }
    bus.write16(session_ram::kFrameListPrimaryCount,
                static_cast<std::uint16_t>(count));
    return count;
}

std::size_t build_secondary_frame_list(GenesisBus& bus) {
    auto output = session_ram::kFrameListSecondary;
    std::size_t count = 0;
    // MOVE.W #2,D7 followed by DBF visits three records: FF0100 through
    // FF0300 inclusive.
    for (std::size_t index = 0; index <= 2U; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            0xFF0100U + index * 0x100U);
        if (!is_secondary_frame_list_record(bus, record_address)) {
            continue;
        }
        bus.write32(output, record_address);
        output += 4U;
        ++count;
    }
    bus.write16(session_ram::kFrameListSecondaryCount,
                static_cast<std::uint16_t>(count));
    return count;
}

} // namespace

FrameEntityListResult build_frame_entity_lists_0x13874(GenesisBus& bus) {
    // 0x13874 begins with CLR.B (A absolute), even though the neighboring
    // count fields are word stores.
    bus.write8(session_ram::kFrameListBuildMode, 0);

    std::size_t primary = 0;
    std::size_t secondary = 0;
    if (bus.read16(ram::kVblankIdleGate) == 0) {
        const auto frame_phase = static_cast<std::uint16_t>(
            bus.read16(ram::kFrameCounter) & 0x003FU);
        if (frame_phase == 0x000BU) {
            primary = build_primary_frame_list(bus);
        } else if (frame_phase == 0x000FU) {
            secondary = build_secondary_frame_list(bus);
        }
    } else {
        primary = build_primary_frame_list(bus);
        secondary = build_secondary_frame_list(bus);
    }
    return {primary, secondary};
}

namespace {

struct EntityDisplayStateResult {
    bool expired;
    std::size_t unresolved_calls;
};

bool signed_byte_sum_is_positive(std::uint8_t left, std::uint8_t right) {
    return static_cast<std::int16_t>(static_cast<std::int8_t>(left)) +
               static_cast<std::int16_t>(static_cast<std::int8_t>(right)) >
           0;
}

void emit_entity_display_block(GenesisBus& bus,
                               std::uint16_t command,
                               std::uint16_t base,
                               std::uint16_t mask,
                               std::uint16_t rows,
                               std::uint16_t columns) {
    ShadowrunRuntime runtime(bus);
    for (std::uint16_t row = 0; row <= rows; ++row) {
        runtime.write_vdp_command_sequence(static_cast<std::uint16_t>(
            command + static_cast<std::uint16_t>(row * 0x80U)));
        auto value = static_cast<std::uint16_t>(base | mask);
        for (std::uint16_t column = 0; column <= columns; ++column) {
            bus.write16(genesis::kVdpData, value);
            value = static_cast<std::uint16_t>(value + 1U);
        }
    }
}

std::uint16_t display_mask_0x3602(GenesisBus& bus,
                                  GenesisAddress record_address) {
    const auto phase = bus.read8(record_address + 0x57U);
    if (phase == 0) {
        return 0x2000U;
    }
    const auto phase_limit_is_low = static_cast<std::int8_t>(
        static_cast<std::uint8_t>(phase - 2U)) < 0;
    const auto frame_is_negative = static_cast<std::int8_t>(
        bus.read8(record_address + 0x56U)) < 0;
    if (phase_limit_is_low && frame_is_negative &&
        static_cast<std::int8_t>(static_cast<std::uint8_t>(phase - 6U)) < 0) {
        return 0x2000U;
    }
    if (static_cast<std::int8_t>(static_cast<std::uint8_t>(phase - 0x0FU)) <
        0) {
        return 0x6000U;
    }
    return 0x2000U;
}

void run_entity_display_pass_0x03548(GenesisBus& bus) {
    struct DisplayPair {
        std::uint16_t first_base;
        std::uint16_t first_command;
        std::uint16_t second_base;
        std::uint16_t second_command;
    };
    const std::array<DisplayPair, 3> pairs{{
        {static_cast<std::uint16_t>(0U - 0x795CU), 0xF448U,
         static_cast<std::uint16_t>(0U - 0x796CU), 0xF1C6U},
        {static_cast<std::uint16_t>(0U - 0x7946U), 0xF848U,
         static_cast<std::uint16_t>(0U - 0x7956U), 0xF5C6U},
        {static_cast<std::uint16_t>(0U - 0x7930U), 0xFC48U,
         static_cast<std::uint16_t>(0U - 0x7940U), 0xF9C6U}
    }};

    for (std::size_t selector = 0; selector < pairs.size(); ++selector) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase +
            (pairs.size() - 1U - selector) * 0x100U);
        const auto& pair = pairs[pairs.size() - 1U - selector];
        auto first_mask = bus.read16(record_address + 0x2CU);
        if (first_mask == 0) {
            first_mask = 0x2000U;
        }
        emit_entity_display_block(bus, pair.first_command,
                                  pair.first_base, first_mask, 3U, 3U);
        const auto second_mask = display_mask_0x3602(bus, record_address);
        emit_entity_display_block(bus, pair.second_command,
                                  pair.second_base, second_mask, 1U, 2U);
    }
}

std::uint32_t add_word_to_long(std::uint32_t value, std::uint16_t addend) {
    return (value & 0xFFFF0000U) |
           static_cast<std::uint32_t>(
               static_cast<std::uint16_t>(value) + addend);
}

void advance_display_command(std::uint32_t& command) {
    command = (command & 0xFFFF0000U) |
              static_cast<std::uint32_t>(
                  static_cast<std::uint16_t>(command) + 0x20U);
}

} // namespace

void refresh_entity_animation_0x03404(GenesisBus& bus) {
    GenesisAddress record_address = session_ram::kEntityPhaseBase;
    std::uint32_t command = 0x0000D280U;
    for (std::size_t record_index = 0; record_index < 3U; ++record_index) {
        GenesisAddress primary_table = 0x000034F8U;
        if ((bus.read8(record_address + 0x10U) & 1U) != 0) {
            primary_table = add_word_to_long(
                bus.read32(record_address + 0x50U),
                bus.read16(record_address + 0x54U));
        }

        for (std::size_t group = 0; group < 4U; ++group) {
            for (std::size_t entry = 0; entry < 4U; ++entry) {
                const auto tile = bus.read16(primary_table);
                primary_table += 2U;
                const auto source = static_cast<GenesisAddress>(
                    0x0006ACFAU + (static_cast<std::uint32_t>(tile) << 5U));
                ShadowrunRuntime(bus).stream_primary_vdp(
                    source, command, 0x20U);
                advance_display_command(command);
            }
            primary_table += 8U;
        }

        GenesisAddress secondary_table = 0x000034F8U;
        if ((bus.read8(record_address + 0x10U) & 1U) != 0) {
            if (bus.read8(record_address + 0x57U) != 0) {
                const auto frame = static_cast<std::uint16_t>(
                    bus.read8(record_address + 0x57U) - 1U);
                const auto frame_table = (static_cast<GenesisAddress>(
                    bus.read8(record_address + 0x56U) < 0x80U
                        ? 0x000D4540U
                        : 0x000D4EA0U));
                secondary_table = bus.read32(frame_table +
                                             static_cast<GenesisAddress>(
                                                 frame) * 0x30U + 0x1AU);
            } else {
                const auto state = bus.read16(record_address + 0x90U);
                if ((state & 0x0008U) != 0) {
                    secondary_table = 0x000D1878U;
                } else if ((state & 0x0004U) != 0) {
                    secondary_table = 0x000D186CU;
                } else {
                    secondary_table = 0x000D1860U;
                }
            }
        }

        for (std::size_t entry = 0; entry < 6U; ++entry) {
            const auto tile = bus.read16(secondary_table);
            secondary_table += 2U;
            const auto source = static_cast<GenesisAddress>(
                0x000CF97CU + (static_cast<std::uint32_t>(tile) << 5U));
            ShadowrunRuntime(bus).stream_primary_vdp(
                source, command, 0x20U);
            advance_display_command(command);
        }
        record_address += session_ram::kEntityPhaseStride;
    }

    run_entity_display_pass_0x03548(bus);
}

namespace {

EntityDisplayStateResult update_entity_display_state_0x36FC(
    GenesisBus& bus,
    GenesisAddress record_address,
    std::uint8_t delta,
    const FrameEntityUpdateHooks& hooks) {
    auto result = EntityDisplayStateResult{false, 0};
    const auto type = bus.read16(record_address + 0x3EU);
    const auto command = type == 0
        ? 0xF4C4U
        : (type == 1 ? 0xF8C4U : 0xFCC4U);
    std::uint16_t source_index = 0;
    const auto active = (bus.read8(record_address + 0x10U) & 1U) != 0;
    const auto frame_signed = static_cast<std::int8_t>(
        bus.read8(record_address + 0x56U));
    const auto frame_is_usable = frame_signed >= 0 &&
                                 frame_signed != 0x7F;
    if (active && frame_is_usable) {
        const auto phase = bus.read8(record_address + 0x57U);
        const auto phase_below_13 = static_cast<std::int8_t>(
            static_cast<std::uint8_t>(phase - 0x0DU)) < 0;
        if (phase_below_13) {
            const auto old_value = bus.read8(record_address + 0x4EU);
            const auto next_value = static_cast<std::uint8_t>(
                old_value + delta);
            if (!signed_byte_sum_is_positive(old_value, delta)) {
                bus.write8(record_address + 0x4EU, 0);
            } else {
                auto limited = next_value;
                const auto limit = bus.read8(
                    rom::kEntityStateGridAnimationLimitTable +
                    static_cast<GenesisAddress>(
                        static_cast<std::uint8_t>(phase - 1U)) * 0x30U +
                    0x24U);
                if (static_cast<std::int8_t>(
                        static_cast<std::uint8_t>(limit - next_value)) < 0) {
                    limited = limit;
                }
                bus.write8(record_address + 0x4EU, limited);
            }
        } else if (static_cast<std::int8_t>(
                       static_cast<std::uint8_t>(phase - 0x10U)) < 0) {
            const auto frame_index = bus.read8(record_address + 0x56U);
            const auto value_address = static_cast<GenesisAddress>(
                record_address + 0x6EU + frame_index);
            const auto old_value = bus.read8(value_address);
            const auto next_value = static_cast<std::uint8_t>(
                old_value + delta);
            if (!signed_byte_sum_is_positive(old_value, delta)) {
                bus.write8(value_address, 0);
                bus.write8(record_address + 0x66U + frame_index, 0);
                bus.write8(record_address + 0x56U, 0x7FU);
                bus.write8(record_address + 0x57U, 0);
                result.expired = true;
                if (hooks.routine_0x03404) {
                    hooks.routine_0x03404(bus, record_address);
                } else {
                    refresh_entity_animation_0x03404(bus);
                }
            } else {
                bus.write8(value_address, next_value);
                source_index = next_value;
            }
        }
    }

    ShadowrunRuntime(bus).stream_vdp_grid(
        static_cast<GenesisAddress>(rom::kEntityStateGridAnimationTable +
                                    source_index * 4U),
        command, 1U, 0U, 0x6000U);
    return result;
}

std::size_t run_entity_display_pass_0x038FA(
    GenesisBus& bus,
    const FrameEntityUpdateHooks& hooks) {
    std::size_t unresolved_calls = 0;
    for (std::size_t index = 0; index <= 2U; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase + index * 0x100U);
        update_entity_state_0x3674(bus, record_address, 0);
        update_entity_state_0x37EC(bus, record_address, 0);
        const auto display = update_entity_display_state_0x36FC(
            bus, record_address, 0, hooks);
        unresolved_calls += display.unresolved_calls;
    }
    return unresolved_calls;
}

} // namespace

EntityCollisionPassResult run_entity_collision_pass_0xEB92(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const FrameEntityUpdateHooks& hooks) {
    EntityCollisionPassResult result{0, 0, 0, 0};
    if ((bus.read8(entity_record_address + 0x10U) & 0x40U) != 0 ||
        entity_record_address == 0xFF0A00U) {
        return result;
    }

    const auto current_x = bus.read16(entity_record_address);
    const auto current_y = bus.read16(entity_record_address + 4U);
    for (auto candidate = static_cast<GenesisAddress>(
             entity_record_address + 0x100U);
         candidate < 0xFF0B00U;
         candidate = static_cast<GenesisAddress>(candidate + 0x100U)) {
        ++result.candidates_examined;
        const auto candidate_flags = bus.read8(candidate + 0x10U);
        if ((candidate_flags & 1U) == 0 ||
            (candidate_flags & 0x40U) != 0) {
            continue;
        }

        const auto linked =
            bus.read32(candidate + 0xA2U) == entity_record_address ||
            bus.read32(entity_record_address + 0xA2U) == candidate ||
            bus.read32(candidate + 0xACU) == entity_record_address ||
            bus.read32(entity_record_address + 0xACU) == candidate;
        const auto radius = static_cast<std::uint16_t>(
            linked ? 0x0100U : 0x0040U);
        const auto candidate_x = bus.read16(candidate);
        const auto candidate_y = bus.read16(candidate + 4U);
        const auto x_lower = static_cast<std::uint16_t>(candidate_x - radius);
        const auto x_upper = static_cast<std::uint16_t>(candidate_x + radius);
        const auto y_lower = static_cast<std::uint16_t>(candidate_y - radius);
        const auto y_upper = static_cast<std::uint16_t>(candidate_y + radius);
        if (current_x < x_lower || current_x >= x_upper ||
            current_y < y_lower || current_y >= y_upper) {
            continue;
        }

        ++result.collisions;
        bus.write8(entity_record_address + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 0x11U) | 0x02U));
        bus.write8(candidate + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(candidate + 0x11U) | 0x02U));

        const auto transform = transform_frame_delta_0x15D3C(
            bus, candidate_y, candidate_x, current_y, current_x);
        ++result.transforms;
        bus.write16(candidate + 0x92U,
                    static_cast<std::uint16_t>(transform.d2 + 0x4000U));
        bus.write16(entity_record_address + 0x92U,
                    static_cast<std::uint16_t>(transform.d2 - 0x8000U));
        bus.write16(candidate + 0x94U,
                    bus.read16(entity_record_address + 0x38U));
        bus.write16(entity_record_address + 0x94U,
                    bus.read16(candidate + 0x38U));

        // Directly reproduce the flag/resource half of 0xECCC. The resource
        // leaves are bus-level queue writes; hooks can still observe or
        // replace them while the transfer consumer remains separate.
        const auto call_resource_1e2e0 = [&](GenesisAddress resource) {
            bus.write32(ram::kResourcePointer, resource);
            if (hooks.routine_0x1E2E0) {
                hooks.routine_0x1E2E0(bus);
            } else {
                queue_resource_load_0x1E2E0(bus);
            }
        };
        const auto call_resource_1e2ec = [&](GenesisAddress resource) {
            bus.write32(ram::kResourcePointer, resource);
            if (hooks.routine_0x1E2EC) {
                hooks.routine_0x1E2EC(bus);
            } else {
                queue_resource_load_0x1E2EC(bus);
            }
        };
        if ((bus.read8(candidate + 0x9EU) & 0x10U) != 0) {
            if ((bus.read8(entity_record_address + 0x10U) & 0x02U) != 0) {
                bus.write32(session_ram::kFrameEntityGlobalPointer, candidate);
                if (bus.read8(session_ram::kFrameEntityResourceGate) == 0) {
                    call_resource_1e2e0(0x00021174U);
                    const auto mode = bus.read16(session_ram::kSessionMode);
                    if (mode == 10U) {
                        auto resource_mode = static_cast<std::uint8_t>(4);
                        if (bus.read8(session_ram::kFrameEntityResourceSubmode) ==
                            6U) {
                            resource_mode = 2;
                        }
                        bus.write8(session_ram::kFrameEntityResourceMode,
                                   resource_mode);
                    } else {
                        const auto mode_offset = static_cast<std::int16_t>(
                            static_cast<std::uint16_t>(mode - 10U));
                        bus.write8(
                            session_ram::kFrameEntityResourceMode,
                            bus.read8(static_cast<GenesisAddress>(
                                static_cast<std::int32_t>(
                                    rom::kCollisionResourceModeTable) +
                                mode_offset) & 0x00FFFFFFU));
                    }
                }
            }
        }

        const auto candidate_hit =
            (bus.read8(candidate + 0x9EU) & 0x08U) != 0;
        if (candidate_hit) {
            if ((bus.read8(candidate + 0xB0U) & 0x80U) != 0 ||
                (bus.read8(entity_record_address + 0x10U) & 0x02U) == 0) {
                continue;
            }
            bus.write8(candidate + 0xB0U,
                       static_cast<std::uint8_t>(
                           bus.read8(candidate + 0xB0U) | 0x80U));
        } else {
            if ((bus.read8(entity_record_address + 0x9EU) & 0x08U) == 0 ||
                (bus.read8(entity_record_address + 0xB0U) & 0x80U) != 0 ||
                (bus.read8(entity_record_address + 0x10U) & 0x02U) == 0) {
                continue;
            }
            bus.write8(entity_record_address + 0xB0U,
                       static_cast<std::uint8_t>(
                           bus.read8(entity_record_address + 0xB0U) | 0x80U));
        }
        bus.write32(session_ram::kFrameEntityGlobalReset,
                    bus.read32(session_ram::kFrameEntityResourceControl));
        auto resource = static_cast<GenesisAddress>(0x0001F146U);
        if (bus.read8(session_ram::kFrameEntityResourceStateA) != 0) {
            bus.write8(session_ram::kFrameEntityResourceStateA, 0);
            resource = 0x00053C90U;
        } else if (bus.read8(session_ram::kFrameEntityResourceStateB) != 0) {
            bus.write8(session_ram::kFrameEntityResourceStateB, 0);
            resource = 0x00054402U;
        }
        call_resource_1e2ec(resource);

        if ((ShadowrunRuntime(bus).advance_random() & 0x7000U) == 0) {
            bus.write16(candidate + 0x92U,
                        static_cast<std::uint16_t>(
                            bus.read16(candidate + 0x92U) + 0x4000U));
            bus.write16(entity_record_address + 0x92U,
                        static_cast<std::uint16_t>(
                            bus.read16(entity_record_address + 0x92U) -
                            0x4000U));
            bus.write16(candidate + 0x94U,
                        static_cast<std::uint16_t>(
                            bus.read16(candidate + 0x94U) * 2U));
            bus.write16(entity_record_address + 0x94U,
                        static_cast<std::uint16_t>(
                            bus.read16(entity_record_address + 0x94U) * 2U));
        }
    }
    return result;
}

EntityRelationshipPassResult update_entity_relationship_0x1DC4C(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress selector,
    std::uint16_t inherited_scan_counter) {
    EntityRelationshipPassResult result{0, false, false, false, 0};
    const auto establish_relation = [&](GenesisAddress candidate) {
        bus.write8(entity_record_address + 0x43U, 0);
        bus.write8(candidate + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(candidate + 0x11U) | 0x10U));
        bus.write32(entity_record_address + 0xA2U, candidate);
        bus.write32(candidate + 0xACU, entity_record_address);
        bus.write16(candidate + 0xA6U, 0x8009U);
        result.relation_established = true;
        result.selected_record = candidate;
    };
    const auto candidate_is_normal_match = [&](GenesisAddress candidate) {
        const auto flags = bus.read8(candidate + 0x10U);
        return (flags & 1U) != 0 && (flags & 0x40U) == 0 &&
               (flags & 0x10U) == 0 &&
               (bus.read8(candidate + 0xB0U) & 0x80U) == 0 &&
               bus.read16(candidate + 0x0CU) != 0;
    };

    const auto special = static_cast<std::int8_t>(
                             bus.read8(entity_record_address + 0x56U)) < 0 &&
                         (bus.read8(entity_record_address + 0x57U) == 0x0CU ||
                          bus.read8(entity_record_address + 0x57U) == 0x04U ||
                          bus.read8(entity_record_address + 0x57U) == 0x0BU);
    if (special) {
        result.special_record_path = true;
        auto candidate = bus.read32(
            entity_record_address + 0xA2U);
        const auto linked = candidate != 0;
        if (linked) {
            bus.write8(candidate + 0x11U,
                       static_cast<std::uint8_t>(
                           bus.read8(candidate + 0x11U) & ~0x10U));
            bus.write32(entity_record_address + 0xA2U, 0);
            result.link_cleared = true;
        } else {
            candidate = bus.read32(ram::kEntityObjectControlPointer);
        }

        auto remaining = inherited_scan_counter;
        for (;;) {
            if (selector == 0x000678CCU) {
                selector = 0x00060000U;
            } else {
                candidate = static_cast<GenesisAddress>(candidate + 0x100U);
            }
            if (bus.read16(candidate + 0x3EU) > 2U) {
                candidate = session_ram::kEntityPhaseBase;
            }
            ++result.candidates_examined;
            if ((bus.read8(candidate + 0x10U) & 1U) != 0) {
                establish_relation(candidate);
                return result;
            }
            if (remaining == 0) {
                return result;
            }
            --remaining;
        }
    }

    auto candidate = bus.read32(entity_record_address + 0xA2U);
    if (candidate != 0) {
        bus.write8(candidate + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(candidate + 0x11U) & ~0x10U));
        bus.write32(entity_record_address + 0xA2U, 0);
        result.link_cleared = true;
        if (candidate >= 0xFF0A00U) {
            candidate = 0xFF0400U;
        } else if (selector != 0x000678CCU) {
            candidate = static_cast<GenesisAddress>(candidate + 0x100U);
        }
    } else {
        candidate = bus.read32(entity_record_address + 0xACU);
        if (candidate == 0) {
            candidate = 0xFF0400U;
        }
    }

    auto remaining = static_cast<std::uint16_t>(7U);
    for (;;) {
        auto normalized = candidate;
        if (bus.read16(normalized + 0x3EU) < 3U) {
            normalized = 0xFF0400U;
        }
        if (normalized > 0xFF0AFFU) {
            normalized = 0xFF0400U;
        }
        ++result.candidates_examined;
        if (candidate_is_normal_match(normalized)) {
            establish_relation(normalized);
            return result;
        }
        if (remaining == 0) {
            return result;
        }
        --remaining;
        candidate = static_cast<GenesisAddress>(normalized + 0x100U);
    }
}

namespace {

constexpr GenesisAddress kAnimationContinuation_0x13A92 = 0x00013A92U;
constexpr GenesisAddress kAnimationContinuation_0x139D8 = 0x000139D8U;
constexpr GenesisAddress kAnimationContinuation_0x14508 = 0x00014508U;
constexpr GenesisAddress kAnimationContinuation_0x1450E = 0x0001450EU;
constexpr GenesisAddress kAnimationContinuation_0x13E8C = 0x00013E8CU;
constexpr GenesisAddress kAnimationContinuation_0x1E514 = 0x0001E514U;
constexpr GenesisAddress kAnimationContinuation_0x142AE = 0x000142AEU;
constexpr GenesisAddress kAnimationContinuation_0x13E40 = 0x00013E40U;
constexpr GenesisAddress kAnimationContinuation_0x1DBAA = 0x0001DBAAU;
constexpr GenesisAddress kAnimationContinuation_0x14682 = 0x00014682U;
constexpr GenesisAddress kAnimationContinuation_0x149C0 = 0x000149C0U;
constexpr GenesisAddress kAnimationContinuation_0x14A54 = 0x00014A54U;
constexpr GenesisAddress kAnimationContinuation_0x13EF4 = 0x00013EF4U;
constexpr GenesisAddress kAnimationContinuation_0x14492 = 0x00014492U;
constexpr GenesisAddress kAnimationContinuation_0x145A2 = 0x000145A2U;
constexpr GenesisAddress kAnimationContinuation_0xAE6E = 0x0000AE6EU;
constexpr GenesisAddress kAnimationContinuation_0xAF7C = 0x0000AF7CU;
constexpr GenesisAddress kAnimationContinuation_0xAFAE = 0x0000AFAEU;

void dispatch_animation_continuation(
    GenesisAddress target,
    const EntityAnimationContinuationRoutine& continuation,
    EntityAnimationEntryResult& result,
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    ++result.continuation_calls;
    if (continuation) {
        continuation(bus, entity_record_address, target);
    } else if (target == kAnimationContinuation_0x13A92) {
        // 0x13A92 has now been recovered as a direct branch. Its inherited
        // D4/D5/D7 values are an explicit context at the public entry point;
        // this legacy entry wrapper starts with the zeroed context used by
        // the surrounding C++ call boundary.
        run_entity_animation_branch_0x13A92(bus, entity_record_address);
    } else if (target == kAnimationContinuation_0x142AE) {
        const auto direct = run_entity_animation_continuation_0x142AE(
            bus, entity_record_address);
        result.continuation_calls += direct.continuation_calls;
        result.unresolved_calls += direct.unresolved_calls;
    } else if (target == kAnimationContinuation_0x14682) {
        const auto direct = run_entity_animation_route_0x14682(
            bus, entity_record_address);
        result.continuation_calls += direct.continuation_calls;
        result.unresolved_calls += direct.unresolved_calls;
    } else if (target == kAnimationContinuation_0x145A2) {
        const auto direct = run_entity_animation_update_0x145A2(
            bus, entity_record_address);
        result.continuation_calls += direct.continuation_calls;
        result.unresolved_calls += direct.unresolved_calls;
    } else if (target == kAnimationContinuation_0x149C0) {
        const auto direct = run_entity_animation_route_0x149C0(
            bus, entity_record_address);
        result.continuation_calls += direct.continuation_calls;
        result.unresolved_calls += direct.unresolved_calls;
    } else if (target == kAnimationContinuation_0x13E40) {
        finish_entity_animation_0x13E40(bus, entity_record_address);
    } else {
        ++result.unresolved_calls;
    }
}

struct FrameDistanceRegisters {
    std::uint32_t d0;
    std::uint32_t d1;
};

std::uint16_t arithmetic_shift_right_word(std::uint16_t value) {
    const auto shifted = static_cast<std::uint16_t>(value >> 1U);
    return (value & 0x8000U) != 0
        ? static_cast<std::uint16_t>(shifted | 0x8000U)
        : shifted;
}

FrameDistanceRegisters reduce_frame_distance_registers_0x15CA4(
    std::uint32_t d0,
    std::uint32_t d1,
    std::uint16_t d2,
    std::uint16_t d3) {
    auto d1_difference = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(d1) - d3);
    if (static_cast<std::int16_t>(d1_difference) < 0) {
        d1_difference = static_cast<std::uint16_t>(0U - d1_difference);
    }
    d1 = (d1 & 0xFFFF0000U) | d1_difference;

    auto d0_difference = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(d0) - d2);
    if (static_cast<std::int16_t>(d0_difference) < 0) {
        d0_difference = static_cast<std::uint16_t>(0U - d0_difference);
    }
    d0 = (d0 & 0xFFFF0000U) | d0_difference;

    if (static_cast<std::uint16_t>(d0) <
        static_cast<std::uint16_t>(d1)) {
        std::swap(d0, d1);
    }

    const auto carry = (static_cast<std::uint16_t>(d1) & 1U) != 0;
    auto half = arithmetic_shift_right_word(static_cast<std::uint16_t>(d1));
    if (carry) {
        half = static_cast<std::uint16_t>(half + 1U);
    }
    d0 = (d0 & 0xFFFF0000U) |
         static_cast<std::uint16_t>(static_cast<std::uint16_t>(d0) + half);
    d1 = (d1 & 0xFFFF0000U) | half;
    return {d0, d1};
}

void run_animation_branch_0x13A4C(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation,
    EntityAnimationEntryResult& result) {
    // 0x13A4C is an internal branch target of 0x13904. Its timer and
    // resource-dispatch sequence is small enough to retain directly even
    // though the three callees still belong to the larger animation system.
    const auto frame = bus.read16(entity_record_address + 0x16U);
    if (static_cast<std::int16_t>(frame) > 0x003C) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x13E40, continuation, result, bus,
            entity_record_address);
        return;
    }

    const auto timer = static_cast<std::uint8_t>(
        bus.read8(session_ram::kEntityAnimationDispatchTimer) - 1U);
    bus.write8(session_ram::kEntityAnimationDispatchTimer, timer);
    if (static_cast<std::int8_t>(timer) >= 0) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x13E40, continuation, result, bus,
            entity_record_address);
        return;
    }

    bus.write8(session_ram::kEntityAnimationDispatchTimer, 0x08U);
    dispatch_animation_continuation(
        kAnimationContinuation_0x14492, continuation, result, bus,
        entity_record_address);
    dispatch_animation_continuation(
        kAnimationContinuation_0xAE6E, continuation, result, bus,
        entity_record_address);
    dispatch_animation_continuation(
        bus.read8(ram::kEntityStateRandomBucket) != 0
            ? kAnimationContinuation_0xAFAE
            : kAnimationContinuation_0xAF7C,
        continuation, result, bus, entity_record_address);
}

void run_animation_branch_0x13E8C(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation,
    EntityAnimationEntryResult& result) {
    // 0x13E8C is another shared state-machine branch. A zero state word
    // bypasses the coordinate initializer and continues at 0x13E40.
    if (bus.read16(entity_record_address + 0x0CU) == 0) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x13E40, continuation, result, bus,
            entity_record_address);
        return;
    }

    // 0x13E9E uses BSET's old-bit result as a one-shot guard. The camera
    // values are all word operations; the two half-screen offsets are
    // deliberately kept as unsigned 16-bit arithmetic.
    const auto flags = bus.read8(entity_record_address + 0xB0U);
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(flags | 0x40U));
    if ((flags & 0x40U) == 0) {
        auto horizontal = static_cast<std::uint16_t>(0x2580U);
        const auto camera_x = static_cast<std::uint16_t>(
            bus.read16(ram::kCameraX) + 0x0A00U);
        const auto upper_x = static_cast<std::uint16_t>(
            bus.read16(ram::kCameraXUpperLimit) >> 1U);
        if (upper_x < camera_x) {
            horizontal = static_cast<std::uint16_t>(0U - horizontal);
        }
        horizontal = static_cast<std::uint16_t>(horizontal + camera_x);

        auto vertical = static_cast<std::uint16_t>(0x2580U);
        const auto camera_y = static_cast<std::uint16_t>(
            bus.read16(ram::kCameraY) + 0x0A00U);
        const auto upper_y = static_cast<std::uint16_t>(
            bus.read16(ram::kCameraYUpperLimit) >> 1U);
        if (upper_y < camera_y) {
            vertical = static_cast<std::uint16_t>(0U - vertical);
        }
        vertical = static_cast<std::uint16_t>(vertical + camera_y);

        bus.write16(entity_record_address + 0x96U, horizontal);
        bus.write16(entity_record_address + 0x98U, vertical);
    }

    dispatch_animation_continuation(
        kAnimationContinuation_0x145A2, continuation, result, bus,
        entity_record_address);
}

std::uint16_t update_animation_progress_0x125FA(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5) {
    // 0x125FA converts the old phase accumulator in +0x4E into the new
    // phase's scale. The table access is a word-indexed MULU.W exactly as in
    // the ROM; the routine also clears +0x4E before returning D1.
    const auto table_entry = static_cast<GenesisAddress>(
        rom::kEntityStateGridAnimationLimitTable +
        static_cast<GenesisAddress>(d5) * 0x30U + 0x24U);
    const auto divisor = bus.read8(table_entry);
    const auto old_accumulator = bus.read8(entity_record_address + 0x4EU);
    std::uint32_t value = static_cast<std::uint32_t>(old_accumulator) * 0x64U;
    bus.write8(entity_record_address + 0x4EU, 0);
    if (divisor != 0) {
        // DIVU.W leaves the word quotient in D1's low word and the word
        // remainder in its high word. The following MOVE.L/LSR.L sequence
        // is intentionally unusual: it increments whenever that shifted
        // packed value is nonzero, not merely when the remainder is >= 50.
        const auto quotient = value / divisor;
        const auto remainder = value % divisor;
        value = (static_cast<std::uint32_t>(remainder) << 16U) |
                static_cast<std::uint16_t>(quotient);
    }
    if ((value >> 8U) != 0) {
        value = (value & 0xFFFF0000U) |
                static_cast<std::uint16_t>(
                    static_cast<std::uint16_t>(value) + 1U);
    }
    return static_cast<std::uint16_t>(value);
}

void apply_entity_animation_phase_0x13466(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t phase,
    std::uint8_t frame_index) {
    // D0 is the phase returned by 0x13AF2 and D7 is the number of entries
    // skipped by that scan.  0x13466 retains both as byte values in the
    // entity record before running the display/update leaves.
    bus.write8(entity_record_address + 0x57U, phase);
    std::uint16_t delta = 0;
    if (phase == 0) {
        bus.write8(entity_record_address + 0x57U, 0);
        bus.write8(entity_record_address + 0x9DU, 0);
        bus.write8(entity_record_address + 0x56U, 0x7FU);
    } else {
        bus.write8(entity_record_address + 0x56U, frame_index);
        delta = bus.read8(entity_record_address + 0x4EU);
        if (delta != 0) {
            const auto old_delta = delta;
            bus.write8(entity_record_address + 0x4EU, 0);
            bus.write8(entity_record_address + 0x9DU, 0);
            const auto table_entry = static_cast<GenesisAddress>(
                rom::kEntityStateGridAnimationLimitTable +
                static_cast<GenesisAddress>(phase - 1U) * 0x30U + 0x24U);
            const auto scale = bus.read8(table_entry);
            auto quotient = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(old_delta) * scale);
            const auto remainder = static_cast<std::uint16_t>(quotient % 0x64U);
            quotient = static_cast<std::uint16_t>(quotient / 0x64U);
            if (remainder >= 0x32U) {
                ++quotient;
            }
            delta = quotient;
        }
    }

    // 0x13466 calls 0x036FC with the low byte of D1 and then unconditionally
    // calls 0x03404. The display streamer already has a direct bus-backed
    // implementation, so no renderer callback is needed here.
    update_entity_display_state_0x36FC(
        bus, entity_record_address, static_cast<std::uint8_t>(delta), {});
    refresh_entity_animation_0x03404(bus);
}

void update_entity_animation_from_phase_scan_0x13AE6(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    std::uint8_t phase = 0;
    std::uint8_t skipped = 0;
    for (std::size_t index = 0; index < 8U; ++index) {
        const auto value = bus.read8(entity_record_address + 0x66U + index);
        if (value < 0x0DU) {
            phase = value;
            break;
        }
        ++skipped;
    }
    apply_entity_animation_phase_0x13466(
        bus, entity_record_address, phase, skipped);
}

void dispatch_animation_known_0x1DBAA(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation,
    EntityAnimationEntryResult& result) {
    ++result.continuation_calls;
    if (continuation) {
        continuation(bus, entity_record_address, kAnimationContinuation_0x1DBAA);
    } else {
        arm_entity_global_gate_0x1DBAA(bus);
    }
}

bool run_animation_branch_0x13EF4(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation,
    const EntityAnimationResourceContinuationRoutine& resource_continuation,
    EntityAnimationEntryResult& result) {
    const auto resource = run_entity_animation_resource_gate_0x13F76(
        bus, entity_record_address, resource_continuation);
    result.continuation_calls += resource.continuation_calls;
    result.unresolved_calls += resource.unresolved_calls;

    const auto animation_type = static_cast<std::int8_t>(
        bus.read8(entity_record_address + 0x4DU));
    if (animation_type <= 2) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x142AE, continuation, result, bus,
            entity_record_address);
        return false;
    }

    if (animation_type == 4) {
        const auto reference_record = bus.read32(
            ram::kEntityObjectControlPointer);
        const auto distance = calculate_frame_distance_0x15CA4(
            bus.read16(entity_record_address),
            bus.read16(entity_record_address + 4U),
            bus.read16(reference_record),
            bus.read16(reference_record + 4U));
        if (static_cast<std::uint16_t>(distance) >= 0x0500U) {
            dispatch_animation_continuation(
                kAnimationContinuation_0x142AE, continuation, result, bus,
                entity_record_address);
            return false;
        }
        dispatch_animation_known_0x1DBAA(
            bus, entity_record_address, continuation, result);
        return true;
    }

    // The preceding byte comparison leaves only type three in this path;
    // larger signed byte values branch to the common 139D8 tail.
    if (animation_type > 4) {
        // The caller emits the common 139D8 continuation after this helper
        // reports the ROM's explicit branch target.
        return true;
    }

    const auto transform = transform_frame_delta_0x15D3C(
        bus,
        bus.read16(bus.read32(ram::kEntityObjectControlPointer) + 4U),
        bus.read16(bus.read32(ram::kEntityObjectControlPointer)),
        bus.read16(entity_record_address + 4U),
        bus.read16(entity_record_address));
    if (transform.d3 >= 0x0640U) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x142AE, continuation, result, bus,
            entity_record_address);
        return false;
    }

    if (static_cast<std::int16_t>(
            bus.read16(entity_record_address + 0x16U)) <= 0x003C) {
        bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);
        bus.write16(entity_record_address + 0x2EU, transform.d2);
    }
    return false;
}

} // namespace

bool finish_entity_animation_0x13E40(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    if (static_cast<std::int16_t>(
            bus.read16(entity_record_address + 0x16U)) > 0x003C) {
        return false;
    }
    bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);
    return true;
}

void copy_entity_animation_position_0x13A40(
    GenesisBus& bus,
    GenesisAddress target_record_address,
    GenesisAddress source_record_address) {
    bus.write16(target_record_address + 0x96U,
                bus.read16(source_record_address));
    bus.write16(target_record_address + 0x98U,
                bus.read16(source_record_address + 4U));
}

std::uint16_t scan_entity_animation_phase_0x13AF2(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    auto cursor = static_cast<GenesisAddress>(entity_record_address + 0x66U);
    for (std::size_t index = 0; index < 8U; ++index) {
        const auto value = bus.read8(cursor++);
        if (value < 0x0DU) {
            return value;
        }
    }
    return 0;
}

EntityAnimationPhaseSelectionResult select_entity_animation_phase_0x13C20(
    GenesisBus& bus) {
    const auto write_selection = [&](std::uint8_t selected) {
        bus.write8(0x00FFE190U, selected);
        return EntityAnimationPhaseSelectionResult{selected, true};
    };
    std::uint8_t selected = 0;
    if (bus.read8(session_ram::kEntityPhaseBase + 0x4CU) < 4U) {
        return write_selection(selected);
    }

    selected = 1;
    if (static_cast<std::int8_t>(bus.read8(0x00FFFC0CU)) >= 0 &&
        bus.read8(session_ram::kEntityPhaseBase + 0x14CU) < 4U) {
        return write_selection(selected);
    }
    if (static_cast<std::int8_t>(bus.read8(0x00FFFC0DU)) < 0) {
        return {selected, false};
    }

    selected = 2;
    if (bus.read8(session_ram::kEntityPhaseBase + 0x24CU) < 4U) {
        return write_selection(selected);
    }
    return {selected, false};
}

EntityAnimationEventParseResult parse_entity_animation_events_0x13C74(
    GenesisBus& bus,
    GenesisAddress event_table) {
    const auto count = bus.read16(event_table);
    auto cursor = static_cast<GenesisAddress>(event_table + 2U);
    auto output = static_cast<GenesisAddress>(
        session_ram::kEntityTerrainCollisionMarker + 2U);
    std::uint16_t accepted = 0;
    std::uint16_t special = 0;

    // DBF executes count+1 iterations, including the count value zero.
    for (std::uint32_t index = 0;
         index <= static_cast<std::uint32_t>(count); ++index) {
        const auto type = bus.read8(cursor);
        const auto argument = bus.read8(cursor + 1U);
        if (type == 0 || type == 1 || type == 7 || type == 8) {
            bus.write8(output++, type);
            bus.write8(output++, argument);
            ++accepted;
        } else if (type == 0x0BU) {
            bus.write8(session_ram::kEntityAnimationEventSpecialArgument,
                       argument);
            ++special;
        }
        cursor = static_cast<GenesisAddress>(cursor + 2U);
    }
    bus.write16(session_ram::kEntityTerrainCollisionMarker, accepted);
    bus.write16(output, 0xFFFFU);
    return {accepted, special, output};
}

bool check_entity_animation_transition_0x13BBA(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    const auto phase = bus.read8(entity_record_address + 0x57U);
    const auto uses_event_table = phase == 1U || phase == 2U ||
                                  phase == 8U || phase == 9U;
    if (uses_event_table) {
        const auto table_offset = static_cast<std::uint16_t>(
            bus.read16(entity_record_address + 0xA0U) * 0x0022U);
        const auto parsed = parse_entity_animation_events_0x13C74(
            bus, static_cast<GenesisAddress>(
                     session_ram::kEntityAnimationEventTable + table_offset));
        if (parsed.special_events == 0) {
            return true;
        }
        return !select_entity_animation_phase_0x13C20(bus).selected;
    }
    if (phase == 0x0CU) {
        return select_entity_animation_phase_0x13C20(bus).selected;
    }
    return false;
}

EntityAnimationRateDecisionResult evaluate_entity_animation_rate_0x13CDE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t event_type) {
    // 0x13CD4 supplies D7 as (phase - 1); 0x13B12 supplies the event type.
    // Both enter the same 0x13CDE body.  A +0x9C value of one is the ROM's
    // unconditional carry path and does not call 0x10660.
    if (bus.read8(entity_record_address + 0x9CU) == 1U) {
        return {true, 0};
    }

    const auto base = calculate_entity_phase_value_0x55F02(
        bus, entity_record_address);
    const auto rate = calculate_entity_rate_0x55F20(
        bus, entity_record_address, base,
        static_cast<std::int8_t>(event_type) < 0);
    const auto carry = static_cast<std::int16_t>(rate.quotient) >= 0 &&
                       static_cast<std::uint8_t>(rate.quotient) >=
                           bus.read8(entity_record_address + 0x9CU);
    return {carry, rate.residual};
}

EntityAnimationPositionGateResult evaluate_entity_animation_position_gate_0x144E8(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    // 0x144E8 loads D0..D3 and immediately enters 0x1460E. Keep the complete
    // 0x15CA4 register result because 0x142AE consumes the post-call D1 word
    // as an indexed address component on its close-distance path.
    const auto registers = reduce_frame_distance_registers_0x15CA4(
        bus.read16(entity_record_address),
        bus.read16(entity_record_address + 4U),
        bus.read16(entity_record_address + 0x96U),
        bus.read16(entity_record_address + 0x98U));
    const auto distance = static_cast<std::uint16_t>(registers.d0);
    const auto minor_axis = static_cast<std::uint16_t>(registers.d1);
    if (distance >= 0x01E0U) {
        return {false, distance, minor_axis};
    }

    // 0x1460E retains only the old sign bit, clears +0xB1, sets bit two,
    // installs B45C, and returns with carry set.
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0xB0U) & 0x80U));
    bus.write8(entity_record_address + 0xB1U, 0);
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0xB0U) | 0x04U));
    bus.write32(entity_record_address + 0x3AU, 0x0000B45CU);
    return {true, distance, minor_axis};
}

EntityAnimation142AeResult run_entity_animation_continuation_0x142AE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation) {
    EntityAnimation142AeResult result{
        true, false, false, false, false, 0, 0, 0, 0, 0};

    const auto call_tail = [&](GenesisAddress target) {
        result.continuation_target = target;
        ++result.continuation_calls;
        if (continuation) {
            continuation(bus, entity_record_address, target);
        } else if (target == kAnimationContinuation_0x149C0) {
            const auto direct = run_entity_animation_route_0x149C0(
                bus, entity_record_address);
            result.continuation_calls += direct.continuation_calls;
            result.unresolved_calls += direct.unresolved_calls;
        } else {
            ++result.unresolved_calls;
        }
    };

    // 0x142AE first bypasses 0x144E8 when the record's bit +0x9E.6 is set.
    if ((bus.read8(entity_record_address + 0x9EU) & 0x40U) != 0) {
        call_tail(kAnimationContinuation_0x1450E);
        return result;
    }

    result.linked_record_path = bus.read32(entity_record_address + 0xA8U) != 0;
    const auto position = evaluate_entity_animation_position_gate_0x144E8(
        bus, entity_record_address);
    result.position_gate_called = true;
    result.position_gate_carry = position.carry;
    result.position_distance = position.distance;
    result.indexed_minor_axis = position.rounded_minor_axis;
    if (!position.carry) {
        call_tail(kAnimationContinuation_0x145A2);
        return result;
    }

    if ((bus.read8(entity_record_address + 0x10U) & 0x20U) == 0) {
        call_tail(kAnimationContinuation_0x1450E);
        return result;
    }

    const auto table = bus.read32(entity_record_address + 0xE0U);
    if (static_cast<std::int16_t>(bus.read16(table)) < 0) {
        call_tail(kAnimationContinuation_0x1450E);
        return result;
    }

    // The probe at 0x142F6 is a word-indexed read using the D1 left by
    // 0x15CA4. Both its zero and nonzero branches converge at 0x14300, so
    // the observable state is the same: clear +0xE4 and copy table words
    // zero and two into the animation-position pair.
    const auto indexed_address = static_cast<GenesisAddress>(
        static_cast<std::int64_t>(table) +
        static_cast<std::int16_t>(position.rounded_minor_axis));
    static_cast<void>(bus.read16(indexed_address));
    bus.write16(entity_record_address + 0xE4U, 0);
    bus.write16(entity_record_address + 0x96U, bus.read16(table));
    bus.write16(entity_record_address + 0x98U, bus.read16(table + 2U));
    result.phase_position_updated = true;
    return result;
}

namespace {

void update_entity_animation_direction_flags_0x14C2E(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    // CMP.W destination,source followed by BCS sets a bit when the target
    // coordinate is at or beyond the current coordinate. The two tests are
    // unsigned word comparisons, exactly as in the ROM.
    std::uint8_t flags = 0;
    if (bus.read16(entity_record_address + 0x96U) >=
        bus.read16(entity_record_address)) {
        flags = static_cast<std::uint8_t>(flags | 0x01U);
    }
    if (bus.read16(entity_record_address + 0x98U) >=
        bus.read16(entity_record_address + 4U)) {
        flags = static_cast<std::uint8_t>(flags | 0x02U);
    }
    bus.write8(entity_record_address + 0xB1U, flags);
}

} // namespace

EntityAnimationDirectionUpdateResult update_entity_animation_direction_0x14BA6(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation) {
    // 0x14BA6 compares the current coordinate delta against the two
    // direction bits already held in +0xB1, then decides whether the entity
    // must enter the larger 0x14682 collision/route body.
    const auto delta_x = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x96U) -
        bus.read16(entity_record_address));
    const auto delta_y = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x98U) -
        bus.read16(entity_record_address + 4U));
    auto direction_flags = bus.read8(entity_record_address + 0xB1U);
    const auto x_sign_bit = (direction_flags & 0x01U) != 0;
    const auto y_sign_bit = (direction_flags & 0x02U) != 0;
    const auto x_negative = static_cast<std::int16_t>(delta_x) < 0;
    const auto y_negative = static_cast<std::int16_t>(delta_y) < 0;

    if ((x_sign_bit && x_negative) || (!x_sign_bit && !x_negative)) {
        direction_flags = static_cast<std::uint8_t>(direction_flags | 0x04U);
    }
    if ((y_sign_bit && y_negative) || (!y_sign_bit && !y_negative)) {
        direction_flags = static_cast<std::uint8_t>(direction_flags | 0x08U);
    }
    bus.write8(entity_record_address + 0xB1U, direction_flags);

    const auto movement_word = bus.read16(entity_record_address + 0xB8U);
    auto requests_route = static_cast<std::int16_t>(movement_word) < 0;
    if (!requests_route) {
        const auto x_match = (direction_flags & 0x04U) != 0;
        const auto y_match = (direction_flags & 0x08U) != 0;
        const auto movement_bit = (movement_word & 0x02U) != 0;
        if (x_match) {
            requests_route = movement_bit;
        } else if (y_match) {
            requests_route = !movement_bit;
        }
    }

    EntityAnimationDirectionUpdateResult result{
        requests_route, direction_flags, 0, 0};
    if (requests_route) {
        bus.write8(entity_record_address + 0xB0U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 0xB0U) | 0x02U));
        ++result.continuation_calls;
        if (continuation) {
            continuation(bus, entity_record_address,
                         kAnimationContinuation_0x14682);
        } else {
            const auto direct = run_entity_animation_route_0x14682(
                bus, entity_record_address);
            result.continuation_calls += direct.continuation_calls;
            result.unresolved_calls += direct.unresolved_calls;
        }
    }
    result.direction_flags = bus.read8(entity_record_address + 0xB1U);
    return result;
}

EntityAnimation145A2Result run_entity_animation_update_0x145A2(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation) {
    EntityAnimation145A2Result result{
        true, false, false, false, false, 0, 0, 0, 0, 0};
    const auto call_tail = [&](GenesisAddress target) {
        result.continuation_target = target;
        ++result.continuation_calls;
        if (continuation) {
            continuation(bus, entity_record_address, target);
        } else if (target == kAnimationContinuation_0x149C0) {
            const auto direct = run_entity_animation_route_0x149C0(
                bus, entity_record_address);
            result.continuation_calls += direct.continuation_calls;
            result.unresolved_calls += direct.unresolved_calls;
        } else {
            ++result.unresolved_calls;
        }
    };

    if (static_cast<std::int16_t>(
            bus.read16(entity_record_address + 0x16U)) > 0x003C) {
        result.stopped_at_frame_cutoff = true;
        return result;
    }

    if ((bus.read8(entity_record_address + 0xB0U) & 0x02U) != 0) {
        result.routed_to_0x149C0 = true;
        call_tail(kAnimationContinuation_0x149C0);
        return result;
    }

    const auto flags_before = bus.read8(entity_record_address + 0xB0U);
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(flags_before & ~0x04U));
    if ((flags_before & 0x04U) != 0) {
        update_entity_animation_direction_flags_0x14C2E(
            bus, entity_record_address);
        result.direction_flags_updated = true;
    } else if (bus.read16(entity_record_address + 0xB8U) != 0x1000U) {
        const auto direction = update_entity_animation_direction_0x14BA6(
            bus, entity_record_address, continuation);
        result.direction_flags_updated = true;
        result.continuation_calls += direction.continuation_calls;
        result.unresolved_calls += direction.unresolved_calls;
        if ((bus.read8(entity_record_address + 0xB0U) & 0x02U) != 0) {
            result.routed_to_0x149C0 = true;
            call_tail(kAnimationContinuation_0x149C0);
            return result;
        }
        update_entity_animation_direction_flags_0x14C2E(
            bus, entity_record_address);
        result.direction_flags_updated = true;
    } else {
        // The 0x1000 movement word bypasses both direction helpers and goes
        // directly to the common transform tail.
        result.direction_flags_updated = false;
    }

    const auto transform = transform_frame_delta_0x15D3C(
        bus,
        bus.read16(entity_record_address + 0x98U),
        bus.read16(entity_record_address + 0x96U),
        bus.read16(entity_record_address + 4U),
        bus.read16(entity_record_address));
    bus.write16(entity_record_address + 0x34U, transform.d2);
    bus.write16(entity_record_address + 0xC8U, transform.d3);
    bus.write32(entity_record_address + 0x3AU, 0x0000B574U);
    result.transformed_position = true;
    result.transformed_direction = transform.d2;
    result.transformed_distance = transform.d3;
    return result;
}

namespace {

std::uint16_t rotate_right_word(std::uint16_t value, unsigned count) {
    count %= 16U;
    if (count == 0U) {
        return value;
    }
    return static_cast<std::uint16_t>(
        (value >> count) | (value << (16U - count)));
}

GenesisAddress add_signed_word_to_address(
    GenesisAddress base,
    std::uint16_t offset) {
    return static_cast<GenesisAddress>(
        static_cast<std::int64_t>(base) +
        static_cast<std::int16_t>(offset));
}

bool animation_map_cell_is_blocked(GenesisBus& bus, std::uint16_t index) {
    const auto address = static_cast<GenesisAddress>(
        static_cast<std::int64_t>(ram::kIndexedObjectByteTable) +
        static_cast<std::int16_t>(index)) & 0x00FFFFFFU;
    return bus.read8(address) != 0;
}

std::uint16_t rotate_left_word(std::uint16_t value, unsigned count) {
    count %= 16U;
    if (count == 0U) {
        return value;
    }
    return static_cast<std::uint16_t>(
        (value << count) | (value >> (16U - count)));
}

std::uint16_t animation_direction_offset(
    GenesisBus& bus,
    std::uint16_t direction) {
    return bus.read16(static_cast<GenesisAddress>(
        rom::kAnimationDirectionOffsetTable +
        static_cast<GenesisAddress>(direction & 0x0006U)));
}

bool animation_grid_index_in_bounds(
    GenesisBus& bus,
    std::uint16_t index) {
    if (static_cast<std::int16_t>(index) < 0) {
        return false;
    }

    const auto width = bus.read16(ram::kTilePlaneWidth);
    const auto low = static_cast<std::uint16_t>(index & 0x003FU);
    if (width < low) {
        return false;
    }

    const auto lower_y = static_cast<std::uint16_t>(
        bus.read16(session_ram::kEntityTerrainBoundsYLower) >> 5U);
    const auto row = static_cast<std::uint16_t>(index >> 6U);
    return lower_y >= row;
}

struct Animation148AaResult {
    std::uint16_t step;
    std::uint16_t last_free;
};

Animation148AaResult run_animation_probe_0x148AA(
    GenesisBus& bus,
    std::uint16_t base,
    std::uint16_t direction) {
    // 0x148AA has no terminating condition other than a nonzero map byte.
    // The ROM's index is a word, so a full 16-bit cycle is the finite safety
    // boundary for malformed or synthetic maps whose selected step is zero.
    const auto step = animation_direction_offset(bus, direction);
    auto cursor = base;
    for (std::size_t iteration = 0; iteration < 0x10000U; ++iteration) {
        const auto candidate = static_cast<std::uint16_t>(cursor + step);
        if (animation_map_cell_is_blocked(bus, candidate)) {
            return {step, cursor};
        }
        cursor = candidate;
    }
    return {step, cursor};
}

struct Animation14854Result {
    bool carry;
    std::uint16_t last_free;
    std::uint16_t paired_probe;
};

Animation14854Result run_animation_probe_0x14854(
    GenesisBus& bus,
    std::uint16_t base,
    std::uint16_t primary_direction,
    std::uint16_t secondary_direction) {
    auto primary_step = animation_direction_offset(bus, primary_direction);
    const auto secondary_step = animation_direction_offset(
        bus, secondary_direction);
    auto cursor = base;
    auto paired_probe = base;

    for (std::size_t iteration = 0; iteration < 0x10000U; ++iteration) {
        cursor = static_cast<std::uint16_t>(cursor + primary_step);
        if (!animation_map_cell_is_blocked(bus, cursor)) {
            paired_probe = static_cast<std::uint16_t>(
                cursor + secondary_step);
            if (!animation_map_cell_is_blocked(bus, paired_probe)) {
                return {false, cursor, paired_probe};
            }
            continue;
        }

        primary_step = animation_direction_offset(
            bus, static_cast<std::uint16_t>(primary_direction ^ 0x0004U));
        cursor = static_cast<std::uint16_t>(base + primary_step);
        for (std::size_t alternate_iteration = 0;
             alternate_iteration < 0x10000U; ++alternate_iteration) {
            if (animation_map_cell_is_blocked(bus, cursor)) {
                return {true, cursor, cursor};
            }
            paired_probe = static_cast<std::uint16_t>(
                cursor + secondary_step);
            if (!animation_map_cell_is_blocked(bus, paired_probe)) {
                return {false, cursor, paired_probe};
            }
            cursor = static_cast<std::uint16_t>(cursor + primary_step);
        }
        return {false, cursor, paired_probe};
    }
    return {false, cursor, paired_probe};
}

struct Animation14800Result {
    std::uint16_t returned_grid;
};

Animation14800Result run_animation_probe_0x14800(
    GenesisBus& bus,
    std::uint16_t base,
    std::uint16_t primary_direction,
    std::uint16_t secondary_direction) {
    const auto primary_step = animation_direction_offset(
        bus, primary_direction);
    const auto secondary_step = animation_direction_offset(
        bus, secondary_direction);
    auto cursor = base;

    for (std::size_t iteration = 0; iteration < 0x10000U; ++iteration) {
        const auto candidate = static_cast<std::uint16_t>(
            cursor + primary_step);
        if (!animation_grid_index_in_bounds(bus, candidate) ||
            animation_map_cell_is_blocked(bus, candidate)) {
            return {cursor};
        }

        const auto paired_probe = static_cast<std::uint16_t>(
            candidate + secondary_step);
        if (!animation_map_cell_is_blocked(bus, paired_probe)) {
            return {candidate};
        }
        cursor = candidate;
    }
    return {cursor};
}

struct Animation14764Result {
    std::uint16_t returned_grid;
    std::uint16_t paired_probe;
};

Animation14764Result run_animation_probe_0x14764(
    GenesisBus& bus,
    std::uint16_t base,
    std::uint16_t primary_direction,
    std::uint16_t secondary_direction) {
    auto primary_step = animation_direction_offset(bus, primary_direction);
    auto secondary_step = animation_direction_offset(bus, secondary_direction);

    auto diagonal = static_cast<std::uint16_t>(base + secondary_step);
    if (animation_grid_index_in_bounds(bus, diagonal) &&
        !animation_map_cell_is_blocked(bus, diagonal)) {
        primary_direction = static_cast<std::uint16_t>(
            (primary_direction + 2U) & 0x0006U);
        secondary_direction = static_cast<std::uint16_t>(
            (secondary_direction + 2U) & 0x0006U);
        primary_step = animation_direction_offset(bus, primary_direction);
        secondary_step = animation_direction_offset(bus, secondary_direction);
    }

    for (std::size_t iteration = 0; iteration < 0x10000U; ++iteration) {
        const auto candidate = static_cast<std::uint16_t>(
            base + primary_step);
        if (!animation_grid_index_in_bounds(bus, candidate) ||
            animation_map_cell_is_blocked(bus, candidate)) {
            return {base, base};
        }

        const auto paired_probe = static_cast<std::uint16_t>(
            candidate + secondary_step);
        if (!animation_map_cell_is_blocked(bus, paired_probe)) {
            return {candidate, paired_probe};
        }
    }
    return {base, base};
}

void clear_animation_route_bits_0x14756(
    GenesisBus& bus,
    GenesisAddress entity_record_address);

bool run_animation_route_search_0x14AF4(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t current_grid) {
    auto direction = static_cast<std::uint16_t>(
        rotate_left_word(bus.read16(entity_record_address + 0x34U), 3U) &
        0x0006U);
    auto secondary_direction = static_cast<std::uint16_t>(
        (direction + 2U) & 0x0006U);

    for (std::size_t attempt = 0; attempt < 5U; ++attempt) {
        const auto probe = run_animation_probe_0x14800(
            bus, current_grid, direction, secondary_direction);
        if (probe.returned_grid != current_grid) {
            const auto path_address = static_cast<GenesisAddress>(
                entity_record_address + 0xBCU);
            bus.write16(entity_record_address + 0xBAU, 0);
            bus.write16(path_address, probe.returned_grid);
            bus.write16(path_address + 2U, 0xFFFFU);
            return true;
        }
        direction = static_cast<std::uint16_t>(
            (direction + 6U) & 0x0006U);
        secondary_direction = static_cast<std::uint16_t>(
            (secondary_direction + 6U) & 0x0006U);
    }

    clear_animation_route_bits_0x14756(
        bus, entity_record_address);
    return false;
}

bool check_animation_path_line_0x1494C(
    GenesisBus& bus,
    std::uint16_t target_grid,
    std::uint16_t current_grid) {
    auto target_low = static_cast<std::uint16_t>(target_grid & 0x003FU);
    auto current_low = static_cast<std::uint16_t>(current_grid & 0x003FU);
    auto probe = current_grid;
    while (target_low != current_low) {
        if (current_low < target_low) {
            current_low = static_cast<std::uint16_t>(current_low + 1U);
            probe = static_cast<std::uint16_t>(probe + 1U);
        } else {
            current_low = static_cast<std::uint16_t>(current_low - 1U);
            probe = static_cast<std::uint16_t>(probe - 1U);
        }
        if (animation_map_cell_is_blocked(bus, probe)) {
            return false;
        }
    }

    auto target_coarse = static_cast<std::uint16_t>(target_grid & 0xFFC0U);
    auto current_coarse = static_cast<std::uint16_t>(current_grid & 0xFFC0U);
    probe = current_grid;
    while (target_coarse != current_coarse) {
        if (current_coarse < target_coarse) {
            current_coarse = static_cast<std::uint16_t>(
                current_coarse + 0x0040U);
            probe = static_cast<std::uint16_t>(probe + 0x0040U);
        } else {
            current_coarse = static_cast<std::uint16_t>(
                current_coarse - 0x0040U);
            probe = static_cast<std::uint16_t>(probe - 0x0040U);
        }
        if (animation_map_cell_is_blocked(bus, probe)) {
            return false;
        }
    }
    return true;
}

void initialize_animation_resource_scratch_0x15118(
    GenesisBus& bus,
    GenesisAddress scratch_record_address) {
    // 0x15118 clears exactly 128 words, not an abstract C++ record size.
    for (std::size_t index = 0; index < 0x80U; ++index) {
        bus.write16(scratch_record_address + index * 2U, 0);
    }
    bus.write16(scratch_record_address + 0x08U, 0xFFF4U);
    bus.write16(scratch_record_address + 0x0EU, 1U);
    bus.write16(scratch_record_address + 0x1CU, 0x1000U);
    bus.write16(scratch_record_address + 0x2CU, 0x4000U);
    bus.write8(scratch_record_address + 0x10U, 1U);
    bus.write32(scratch_record_address + 0x3AU, 0x00014E0AU);
}

std::uint16_t swapped_low_word(std::uint32_t value) {
    return static_cast<std::uint16_t>(value >> 16U);
}

std::uint16_t scaled_animation_resource_offset(
    GenesisBus& bus,
    GenesisAddress table_address,
    std::uint16_t index) {
    const auto source = static_cast<std::int32_t>(
        static_cast<std::int16_t>(bus.read16(table_address + index)));
    const auto product = source * 0x000CU;
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(product) >> 4U);
}

GenesisAddress prepare_animation_resource_scratch_0x14FF8(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress source_record_address) {
    constexpr auto scratch = session_ram::kEntityAnimationResourceScratch;

    initialize_animation_resource_scratch_0x15118(bus, scratch);
    bus.write16(scratch, bus.read16(entity_record_address));
    bus.write16(scratch + 0x04U,
                bus.read16(entity_record_address + 0x04U));
    bus.write16(scratch + 0x2EU,
                bus.read16(entity_record_address + 0x2EU));
    bus.write8(scratch + 0x9DU,
               bus.read8(entity_record_address + 0x9DU));
    bus.write8(scratch + 0x57U,
               bus.read8(entity_record_address + 0x57U));
    bus.write8(scratch + 0x56U,
               bus.read8(entity_record_address + 0x56U));
    bus.write16(scratch + 0x3EU, 0xFFFFU);
    bus.write16(scratch + 0x16U, 0x0014U);
    bus.write32(scratch + 0x60U, entity_record_address);

    if ((bus.read8(entity_record_address + 0x10U) & 0x08U) != 0) {
        bus.write8(scratch + 0x10U,
                   static_cast<std::uint8_t>(
                       bus.read8(scratch + 0x10U) | 0x08U));
    }

    // 0x14D8E sets FFDFD8 before entering 0x14FF8, so the 0x55640 side
    // effect is deliberately not entered on this path. When the source A1
    // record is active, 0x14FF8 derives the scratch direction through the
    // direct 0x15D3C transform.
    if (source_record_address != 0 &&
        (bus.read8(source_record_address + 0x10U) & 0x01U) != 0) {
        const auto transform = transform_frame_delta_0x15D3C(
            bus,
            bus.read16(source_record_address + 0x04U),
            bus.read16(source_record_address),
            bus.read16(scratch + 0x04U),
            bus.read16(scratch));
        bus.write16(scratch + 0x2EU, transform.d2);
    }

    // 0x1509A..0x150E8 first records the direction-adjusted probe position,
    // then applies the smaller coordinate offsets and the fixed-point motion
    // tables at 0x15E16/0x15E56.
    auto direction_index = rotate_left_word(
        bus.read16(scratch + 0x2EU), 3U);
    if ((direction_index & 0x8000U) != 0) {
        direction_index = static_cast<std::uint16_t>(direction_index + 1U);
    }
    direction_index = static_cast<std::uint16_t>(direction_index & 0x0007U);
    auto table_offset = static_cast<std::uint16_t>(direction_index << 2U);

    bus.write16(
        session_ram::kEntityAnimationResourceProbeX,
        static_cast<std::uint16_t>(
            bus.read16(scratch) +
            bus.read16(rom::kAnimationResourceProbeOffsetTable + table_offset)));
    table_offset = static_cast<std::uint16_t>(table_offset + 2U);
    bus.write16(
        session_ram::kEntityAnimationResourceProbeY,
        static_cast<std::uint16_t>(
            bus.read16(scratch + 0x04U) +
            bus.read16(rom::kAnimationResourceProbeOffsetTable + table_offset)));

    table_offset = static_cast<std::uint16_t>(table_offset - 2U);
    bus.write16(
        scratch,
        static_cast<std::uint16_t>(
            bus.read16(scratch) +
            bus.read16(rom::kAnimationResourceCoordinateOffsetTable +
                       table_offset)));
    table_offset = static_cast<std::uint16_t>(table_offset + 2U);
    bus.write16(
        scratch + 0x04U,
        static_cast<std::uint16_t>(
            bus.read16(scratch + 0x04U) +
            bus.read16(rom::kAnimationResourceCoordinateOffsetTable +
                       table_offset)));

    const auto fixed_point_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bus.read16(scratch + 0x2EU) >> 8U) &
        0x00FEU);
    bus.write16(
        scratch + 0x4CU,
        scaled_animation_resource_offset(
            bus, rom::kObjectGenerationInterpolationTable + 0x40U,
            fixed_point_index));
    bus.write16(
        scratch + 0x4EU,
        scaled_animation_resource_offset(
            bus, rom::kObjectGenerationInterpolationTable,
            fixed_point_index));
    return scratch;
}

struct AnimationResourceTerrainProbeResult {
    EntityTerrainSampleResult terrain;
    std::uint32_t fixed_d5;
    std::uint32_t fixed_d6;
    std::uint16_t d5;
    std::uint16_t d6;
};

AnimationResourceTerrainProbeResult run_animation_resource_terrain_probe_0x14F26(
    GenesisBus& bus,
    GenesisAddress scratch_record_address) {
    const auto x_offset = static_cast<std::int32_t>(
        static_cast<std::int16_t>(bus.read16(scratch_record_address + 0x4CU)));
    const auto y_offset = static_cast<std::int32_t>(
        static_cast<std::int16_t>(bus.read16(scratch_record_address + 0x4EU)));
    const auto d5_long = static_cast<std::uint32_t>(
        (static_cast<std::uint32_t>(x_offset) << 8U) +
        bus.read32(scratch_record_address));
    const auto d6_long = static_cast<std::uint32_t>(
        (static_cast<std::uint32_t>(y_offset) << 8U) +
        bus.read32(scratch_record_address + 0x04U));

    // 0x14F26 swaps each longword only while calling 0xE7C8. The terrain
    // routine consumes the low word after that swap, while 0x14D8E later
    // stores the unswapped longwords back into FF1380.
    const auto d5 = swapped_low_word(d5_long);
    const auto d6 = swapped_low_word(d6_long);
    const auto terrain = sample_entity_terrain_0xE7C8(
        bus, scratch_record_address, d5, d6);
    return {terrain, d5_long, d6_long, d5, d6};
}

bool run_animation_occupancy_scan_0x15ABA(
    GenesisBus& bus,
    GenesisAddress scratch_record_address,
    std::uint16_t d5,
    std::uint16_t d6) {
    for (std::size_t index = 0;
         index < session_ram::kEntityAnimationResourceEntityCount; ++index) {
        const auto candidate = static_cast<GenesisAddress>(
            session_ram::kEntityAnimationResourceEntityBase + index * 0x100U);
        const auto flags = bus.read8(candidate + 0x10U);
        if ((flags & 0x01U) == 0 || (flags & 0x40U) != 0 ||
            (flags & 0x10U) != 0 || bus.read8(candidate + 0x4CU) == 0) {
            continue;
        }

        const auto x = bus.read16(candidate);
        const auto x_lower = static_cast<std::uint16_t>(x - 0x0090U);
        // The ROM adds 0x120 to the already-subtracted lower bound, so the
        // effective half-open upper edge is candidate X + 0x90.
        const auto x_upper = static_cast<std::uint16_t>(x + 0x0090U);
        if (d5 < x_lower || d5 >= x_upper) {
            continue;
        }
        const auto y = bus.read16(candidate + 0x04U);
        const auto y_lower = static_cast<std::uint16_t>(y - 0x0090U);
        const auto y_upper = static_cast<std::uint16_t>(y + 0x0090U);
        if (d6 < y_lower || d6 >= y_upper) {
            continue;
        }

        // 0x14D8E keeps FFDFD8 nonzero for the entire scan. In this mode
        // 0x15ABA only accepts the source record as the terminating match.
        if (bus.read16(session_ram::kEntityAnimationResourceGateState) != 0) {
            if (candidate == bus.read32(session_ram::kEntityAnimationResourceSource)) {
                return true;
            }
            continue;
        }

        if (bus.read8(scratch_record_address + 0x9FU) == 0) {
            const auto scratch_flags = bus.read8(scratch_record_address + 0x10U);
            const auto candidate_flags_b0 = bus.read8(candidate + 0xB0U);
            const auto candidate_flags_10 = bus.read8(candidate + 0x10U);
            const auto candidate_flags_9e = bus.read8(candidate + 0x9EU);
            if ((scratch_flags & 0x08U) == 0 ||
                ((candidate_flags_b0 & 0x80U) == 0 &&
                 (candidate_flags_10 & 0x08U) == 0 &&
                 (candidate_flags_9e & 0x10U) == 0)) {
                bus.write16(scratch_record_address,
                            bus.read16(candidate));
                bus.write16(scratch_record_address + 0x04U,
                            static_cast<std::uint16_t>(
                                bus.read16(candidate + 0x04U) + 0x0010U));
                return true;
            }
        } else {
            const auto scratch_flags = bus.read8(scratch_record_address + 0x10U);
            if ((scratch_flags & 0x08U) == 0) {
                return true;
            }
            if ((bus.read8(candidate + 0xB0U) & 0x80U) == 0 &&
                (bus.read8(candidate + 0x10U) & 0x08U) == 0 &&
                (bus.read8(candidate + 0x9EU) & 0x10U) == 0) {
                return true;
            }
        }
    }
    return false;
}

void clear_animation_route_bits_0x14756(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0xB0U) & ~0x02U));
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0xB0U) & ~0x08U));
}

} // namespace

EntityAnimation14682Result run_entity_animation_route_0x14682(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation) {
    EntityAnimation14682Result result{
        true, false, false, false, false, 0, 0, 0, 0, 0, 0, 0, 0};

    // 0x14682 begins with BSET and immediately returns if the route-search
    // bit is already set. This is a genuine one-byte state-machine guard,
    // not a host-side recursion check.
    const auto flags_before = bus.read8(entity_record_address + 0xB0U);
    bus.write8(entity_record_address + 0xB0U,
               static_cast<std::uint8_t>(flags_before | 0x02U));
    if ((flags_before & 0x08U) != 0) {
        result.route_already_active = true;
        return result;
    }

    const auto current_x = bus.read16(entity_record_address);
    const auto current_y = bus.read16(entity_record_address + 4U);
    const auto target_x = bus.read16(entity_record_address + 0x96U);
    const auto target_y = bus.read16(entity_record_address + 0x98U);

    // 0x148D2 returns D5=abs(current X-target X), while D6/D7 select the
    // two perpendicular table entries. Its final CMP is unsigned, but the
    // magnitudes themselves come from signed word subtraction.
    const auto x_delta = static_cast<std::uint16_t>(current_x - target_x);
    const auto y_delta = static_cast<std::uint16_t>(current_y - target_y);
    auto x_magnitude = x_delta;
    auto y_magnitude = y_delta;
    auto d7 = static_cast<std::uint16_t>(6U);
    auto d6 = static_cast<std::uint16_t>(0U);
    if (static_cast<std::int16_t>(x_delta) < 0) {
        x_magnitude = static_cast<std::uint16_t>(0U - x_delta);
        d7 = 2U;
    }
    if (static_cast<std::int16_t>(y_delta) < 0) {
        y_magnitude = static_cast<std::uint16_t>(0U - y_delta);
        d6 = 4U;
    }
    if (x_magnitude < y_magnitude) {
        std::swap(d6, d7);
        std::swap(x_magnitude, y_magnitude);
    }
    const auto probe_index = x_magnitude;
    result.probe_index = probe_index;
    result.primary_direction = d6;
    result.secondary_direction = d7;
    result.target_grid = calculate_entity_animation_grid_0x14902(
        target_x, target_y);

    // The first map test uses D5 directly, before the target coordinate is
    // converted to a grid index. This signed-index detail is easy to lose if
    // the routine is rewritten as a conventional tile-coordinate helper.
    if (animation_map_cell_is_blocked(bus, probe_index)) {
        clear_animation_route_bits_0x14756(
            bus, entity_record_address);
        result.map_blocked = true;
        return result;
    }

    bus.write16(entity_record_address + 0xBAU, 0);
    const auto line = run_animation_probe_0x148AA(bus, probe_index, d7);
    if (line.last_free != probe_index) {
        const auto path_value = static_cast<std::uint16_t>(
            probe_index + line.step);
        const auto path_address = static_cast<GenesisAddress>(
            entity_record_address + 0xBCU);
        bus.write16(path_address, path_value);
        bus.write16(path_address + 2U, 0xFFFFU);
        bus.write16(entity_record_address + 0xBAU, 0);
        result.path_written = true;
        result.path_value = path_value;
        return result;
    }

    // 0x14854 either finds a paired free probe or returns carry after its
    // alternate primary direction is blocked. D4 is part of that helper's
    // live register result and is the second word stored in the path entry.
    const auto paired = run_animation_probe_0x14854(
        bus, line.last_free, d6, d7);
    if (paired.carry) {
        bus.write8(entity_record_address + 0xB0U,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 0xB0U) | 0x08U));
        static_cast<void>(run_animation_route_search_0x14AF4(
            bus, entity_record_address,
            calculate_entity_animation_grid_0x14902(
                current_x, current_y)));
        result.route_escalated = true;
        result.continuation_target = kAnimationContinuation_0x149C0;
        ++result.continuation_calls;
        if (continuation) {
            continuation(bus, entity_record_address,
                         kAnimationContinuation_0x149C0);
        } else {
            const auto route = run_entity_animation_route_0x149C0(
                bus, entity_record_address);
            result.continuation_calls += route.continuation_calls;
            result.unresolved_calls += route.unresolved_calls;
        }
        return result;
    }

    const auto path_address = static_cast<GenesisAddress>(
        entity_record_address + 0xBCU +
        bus.read16(entity_record_address + 0xBAU));
    bus.write16(path_address, paired.last_free);
    bus.write16(path_address + 2U, paired.paired_probe);
    bus.write16(path_address + 4U, 0xFFFFU);
    bus.write16(entity_record_address + 0xBAU, 0);
    result.path_written = true;
    result.path_value = paired.last_free;
    return result;
}

EntityAnimationAlternateRouteResult run_entity_animation_alternate_route_0x14A54(
    GenesisBus& bus,
    GenesisAddress entity_record_address) {
    EntityAnimationAlternateRouteResult result{
        true, false, false, false, false, 0, 0};
    const auto current_grid = calculate_entity_animation_grid_0x14902(
        bus.read16(entity_record_address),
        bus.read16(entity_record_address + 4U));
    result.current_grid = current_grid;

    auto direction = static_cast<std::uint16_t>(
        rotate_left_word(bus.read16(entity_record_address + 0x34U), 3U) &
        0x0006U);
    const auto first_candidate = static_cast<std::uint16_t>(
        current_grid + animation_direction_offset(bus, direction));
    result.primary_candidate_clear =
        !animation_map_cell_is_blocked(bus, first_candidate);

    if (result.primary_candidate_clear) {
        const auto diagonal_direction = static_cast<std::uint16_t>(
            (direction + 4U) & 0x0006U);
        const auto orthogonal_direction = static_cast<std::uint16_t>(
            (direction + 2U) & 0x0006U);
        const auto compound_probe = static_cast<std::uint16_t>(
            current_grid + animation_direction_offset(
                bus, diagonal_direction) +
            animation_direction_offset(bus, orthogonal_direction));
        if (!animation_map_cell_is_blocked(bus, compound_probe)) {
            const auto line = run_animation_probe_0x148AA(
                bus, current_grid, direction);
            const auto path_address = static_cast<GenesisAddress>(
                entity_record_address + 0xBCU);
            bus.write16(entity_record_address + 0xBAU, 0);
            bus.write16(path_address, line.last_free);
            bus.write16(path_address + 2U, 0xFFFFU);
            result.path_written = true;
            result.path_value = line.last_free;
            return result;
        }
    }

    // 0x14A54 tries five DBF iterations of 0x14764, rotating both direction
    // selectors backward by one table pair after each no-progress result.
    result.fallback_used = true;
    auto secondary_direction = static_cast<std::uint16_t>(
        (direction + 2U) & 0x0006U);
    for (std::size_t attempt = 0; attempt < 5U; ++attempt) {
        const auto probe = run_animation_probe_0x14764(
            bus, current_grid, direction, secondary_direction);
        if (probe.returned_grid != current_grid) {
            const auto path_address = static_cast<GenesisAddress>(
                entity_record_address + 0xBCU);
            bus.write16(entity_record_address + 0xBAU, 0);
            bus.write16(path_address, probe.returned_grid);
            bus.write16(path_address + 2U, 0xFFFFU);
            result.path_written = true;
            result.path_value = probe.returned_grid;
            return result;
        }
        direction = static_cast<std::uint16_t>((direction + 6U) & 0x0006U);
        secondary_direction = static_cast<std::uint16_t>(
            (secondary_direction + 6U) & 0x0006U);
    }

    clear_animation_route_bits_0x14756(
        bus, entity_record_address);
    result.route_cleared = true;
    return result;
}

std::uint16_t calculate_entity_animation_grid_0x14902(
    std::uint16_t x,
    std::uint16_t y) {
    x = static_cast<std::uint16_t>(x >> 4U);
    y = static_cast<std::uint16_t>(y >> 4U);
    y = static_cast<std::uint16_t>(y << 1U);
    y = static_cast<std::uint16_t>(y & 0xFFC0U);
    x = static_cast<std::uint16_t>(x >> 1U);
    return static_cast<std::uint16_t>(x + y);
}

EntityAnimationGridTargetResult select_entity_animation_grid_target_0x14B4E(
    GenesisBus& bus,
    std::uint16_t selected_grid,
    std::uint16_t current_grid) {
    const auto target_y = static_cast<std::uint16_t>(
        rotate_right_word(static_cast<std::uint16_t>(selected_grid >> 6U), 7U) +
        0x0100U);
    const auto target_x = static_cast<std::uint16_t>(
        rotate_right_word(static_cast<std::uint16_t>(selected_grid & 0x003FU),
                          7U) + 0x0100U);
    bus.write16(session_ram::kEntityAnimationTargetX, target_x);
    bus.write16(session_ram::kEntityAnimationTargetY, target_y);

    std::uint16_t direction = 0x0004U;
    if ((selected_grid & 0xFFC0U) == (current_grid & 0xFFC0U)) {
        direction = 0x0002U;
        if ((selected_grid & 0x003FU) >= (current_grid & 0x003FU)) {
            direction = 0x0006U;
        }
    } else if ((current_grid & 0xFFC0U) >=
               (selected_grid & 0xFFC0U)) {
        direction = 0;
    }
    return {target_x, target_y, direction};
}

EntityAnimationPathCheckResult check_entity_animation_path_0x14914(
    GenesisBus& bus,
    std::uint16_t target_grid,
    std::uint16_t current_grid) {
    // 0x14914 tests the low-coordinate path and then repeats the same
    // 0x1494C walk with D4/D5 exchanged. Either unobstructed walk sets carry.
    if (check_animation_path_line_0x1494C(
            bus, target_grid, current_grid) ||
        check_animation_path_line_0x1494C(
            bus, current_grid, target_grid)) {
        return {true, target_grid, current_grid};
    }
    return {false, target_grid, current_grid};
}

EntityAnimation149C0Result run_entity_animation_route_0x149C0(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation) {
    EntityAnimation149C0Result result{
        true, false, false, false, false, false, 0, 0, 0, 0, 0, 0, 0};
    const auto call_tail = [&](GenesisAddress target) {
        result.continuation_target = target;
        ++result.continuation_calls;
        if (continuation) {
            continuation(bus, entity_record_address, target);
        } else if (target == kAnimationContinuation_0x14A54) {
            static_cast<void>(run_entity_animation_alternate_route_0x14A54(
                bus, entity_record_address));
        } else {
            ++result.unresolved_calls;
        }
    };

    const auto current_grid = calculate_entity_animation_grid_0x14902(
        bus.read16(entity_record_address),
        bus.read16(entity_record_address + 4U));
    const auto target_grid = calculate_entity_animation_grid_0x14902(
        bus.read16(entity_record_address + 0x96U),
        bus.read16(entity_record_address + 0x98U));
    result.current_grid = current_grid;
    result.selected_grid = target_grid;

    // 0x149C0 first gives 0x14914 a chance to clear the route bits when an
    // unobstructed line exists.
    if ((bus.read8(entity_record_address + 0xB0U) & 0x08U) != 0) {
        const auto path = check_entity_animation_path_0x14914(
            bus, target_grid, current_grid);
        if (path.unobstructed) {
            clear_animation_route_bits_0x14756(
                bus, entity_record_address);
            result.unobstructed_path_cleared = true;
            return result;
        }
    }

    const auto path_cursor = add_signed_word_to_address(
        entity_record_address + 0xBCU,
        bus.read16(entity_record_address + 0xBAU));
    auto selected_grid = bus.read16(path_cursor);
    if (selected_grid == current_grid) {
        const auto next_grid = bus.read16(path_cursor + 2U);
        if (next_grid == 0xFFFFU) {
            result.reached_end_of_path = true;
            if ((bus.read8(entity_record_address + 0xB0U) & 0x08U) != 0) {
                result.alternate_route = true;
                call_tail(kAnimationContinuation_0x14A54);
                return result;
            }

            clear_animation_route_bits_0x14756(
                bus, entity_record_address);
            update_entity_animation_direction_flags_0x14C2E(
                bus, entity_record_address);
            const auto transform = transform_frame_delta_0x15D3C(
                bus,
                bus.read16(entity_record_address + 0x98U),
                bus.read16(entity_record_address + 0x96U),
                bus.read16(entity_record_address + 4U),
                bus.read16(entity_record_address));
            bus.write16(entity_record_address + 0x34U, transform.d2);
            bus.write16(entity_record_address + 0xC8U, transform.d3);
            bus.write32(entity_record_address + 0x3AU, 0x0000B574U);
            result.transformed_position = true;
            result.transformed_direction = transform.d2;
            result.transformed_distance = transform.d3;
            return result;
        }
        selected_grid = next_grid;
        bus.write16(entity_record_address + 0xBAU,
                    static_cast<std::uint16_t>(
                        bus.read16(entity_record_address + 0xBAU) + 2U));
        result.path_entry_advanced = true;
    }

    const auto target = select_entity_animation_grid_target_0x14B4E(
        bus, selected_grid, current_grid);
    const auto transformed_direction = rotate_right_word(target.direction, 3U);
    const auto distance = static_cast<std::uint16_t>(
        calculate_frame_distance_0x15CA4(
            bus.read16(entity_record_address),
            bus.read16(entity_record_address + 4U),
            target.target_x,
            target.target_y));
    bus.write16(entity_record_address + 0x34U, transformed_direction);
    bus.write16(entity_record_address + 0xC8U, distance);
    bus.write32(entity_record_address + 0x3AU, 0x0000B574U);
    result.transformed_position = true;
    result.transformed_direction = transformed_direction;
    result.transformed_distance = distance;
    result.selected_grid = selected_grid;
    return result;
}

namespace {

EntityAnimationPhaseTransitionResult advance_entity_animation_phase(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t event_type,
    std::uint8_t event_argument,
    const EntityAnimationRegisterContext& registers,
    std::uint16_t accepted_events_considered) {
    const auto previous_phase = bus.read8(entity_record_address + 0x57U);
    const auto signed_phase = static_cast<std::int8_t>(previous_phase);
    if (signed_phase != 0 && signed_phase < 0x0D) {
        const auto converted = update_animation_progress_0x125FA(
            bus, entity_record_address, registers.d5);
        bus.write8(entity_record_address + 0x4EU,
                   static_cast<std::uint8_t>(converted));
    }

    bus.write8(entity_record_address + 0x56U, 0xFFU);
    bus.write8(entity_record_address + 0x57U,
               static_cast<std::uint8_t>(event_type + 1U));
    bus.write8(entity_record_address + 0x9DU, event_argument);
    update_entity_animation_delay_0xABFC(bus, entity_record_address, 0);
    refresh_entity_animation_0x03404(bus);

    return {false, true, previous_phase,
            bus.read8(entity_record_address + 0x57U),
            accepted_events_considered};
}

} // namespace

EntityAnimationPhaseTransitionResult run_entity_animation_transition_0x13B12(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationRegisterContext& registers) {
    const auto table_offset = static_cast<GenesisAddress>(
        bus.read16(entity_record_address + 0xA0U) * 0x22U);
    const auto parsed = parse_entity_animation_events_0x13C74(
        bus, static_cast<GenesisAddress>(
                 session_ram::kEntityAnimationEventTable + table_offset));

    auto candidate_type = static_cast<std::uint8_t>(registers.d7);
    auto candidate_argument = static_cast<std::uint8_t>(registers.d4);
    auto should_process_accepted = false;
    if (parsed.special_events != 0) {
        candidate_type = 0x0BU;
        candidate_argument = bus.read8(
            session_ram::kEntityAnimationEventSpecialArgument);
        const auto special = evaluate_entity_animation_rate_0x13CDE(
            bus, entity_record_address, candidate_type);
        if (!special.carry &&
            !select_entity_animation_phase_0x13C20(bus).selected) {
            should_process_accepted = true;
        } else if (special.carry) {
            should_process_accepted = true;
        }
    } else {
        // 0x13B12 branches directly to 0x13B4E when 0x13C74 found no type-B
        // event; 0x13C20 is not consulted on this path.
        should_process_accepted = true;
    }

    if (should_process_accepted) {
        if (parsed.accepted_events == 0) {
            return {false, false,
                    bus.read8(entity_record_address + 0x57U),
                    bus.read8(entity_record_address + 0x57U), 0};
        }
        const auto selected = static_cast<std::uint16_t>(
            ShadowrunRuntime(bus).random_below(parsed.accepted_events));
        const auto selected_address = static_cast<GenesisAddress>(
            session_ram::kEntityTerrainCollisionMarker + 2U +
            selected * 2U);
        candidate_type = bus.read8(selected_address);
        candidate_argument = bus.read8(selected_address + 1U);
        const auto event = evaluate_entity_animation_rate_0x13CDE(
            bus, entity_record_address, candidate_type);
        if (event.carry) {
            return {true, false,
                    bus.read8(entity_record_address + 0x57U),
                    bus.read8(entity_record_address + 0x57U),
                    parsed.accepted_events};
        }
    }

    // The ROM reaches this tail either from a successful phase-selection
    // check or after an event rate test fails. In the no-event case D4/D7
    // are inherited registers, which is why they remain explicit above.
    return advance_entity_animation_phase(
        bus, entity_record_address, candidate_type, candidate_argument,
        registers, parsed.accepted_events);
}

EntityAnimationBranchResult run_entity_animation_branch_0x13A92(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationRegisterContext& registers) {
    const auto previous_phase = bus.read8(entity_record_address + 0x57U);
    EntityAnimationBranchResult result{
        false, false, false, false, previous_phase, previous_phase};
    if (static_cast<std::int8_t>(
            bus.read8(entity_record_address + 0x9BU)) >= 2) {
        return result;
    }
    result.entered = true;

    const auto apply_scan = [&]() {
        update_entity_animation_from_phase_scan_0x13AE6(
            bus, entity_record_address);
        result.display_updated = true;
        result.new_phase = bus.read8(entity_record_address + 0x57U);
    };

    if (static_cast<std::int8_t>(
            bus.read8(entity_record_address + 0x56U)) < 0) {
        if (check_entity_animation_transition_0x13BBA(
                bus, entity_record_address)) {
            const auto phase = bus.read8(entity_record_address + 0x57U);
            if (evaluate_entity_animation_rate_0x13CDE(
                    bus, entity_record_address,
                    static_cast<std::uint8_t>(phase - 1U)).carry) {
                const auto current_frame = static_cast<std::int8_t>(
                    bus.read8(entity_record_address + 0x56U));
                const auto current_phase = static_cast<std::int8_t>(
                    bus.read8(entity_record_address + 0x57U));
                if (current_frame < 0 || current_phase == 0 ||
                    current_phase >= 0x0D) {
                    apply_scan();
                }
                result.phase_transition_called = true;
                return result;
            }
            return result;
        }
    }

    if (static_cast<std::int8_t>(
            bus.read8(entity_record_address + 0x9CU)) < 5) {
        return result;
    }

    result.phase_transition_called = true;
    const auto transition = run_entity_animation_transition_0x13B12(
        bus, entity_record_address, registers);
    result.phase_advanced = transition.phase_advanced;
    result.new_phase = transition.new_phase;
    if (transition.carry) {
        apply_scan();
    }
    return result;
}

EntityAnimationEntryResult run_entity_animation_entry_0x138B8(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation,
    const EntityAnimationResourceContinuationRoutine&) {
    EntityAnimationEntryResult result{false, false, false, 0, 0};

    const auto timer = bus.read16(entity_record_address + 0xB6U);
    const auto next_timer = static_cast<std::uint16_t>(timer - 1U);
    bus.write16(entity_record_address + 0xB6U, next_timer);
    if (static_cast<std::int16_t>(next_timer) >= 0) {
        return result;
    }
    result.timer_expired = true;

    if (bus.read8(session_ram::kFrameListBuildMode) != 0) {
        result.suppressed_by_global_gate = true;
        return result;
    }

    bus.write8(session_ram::kFrameListBuildMode, 0xFFU);
    bus.write16(entity_record_address + 0xB6U, 0x0014U);
    result.timer_rearmed = true;

    if (bus.read8(entity_record_address + 0x4DU) == 0 ||
        bus.read16(session_ram::kFrameEntityGlobalGate) == 0) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x14508, continuation, result, bus,
            entity_record_address);
        return result;
    }

    if (static_cast<std::int16_t>(
            bus.read16(entity_record_address + 0xDCU)) < 0) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x13A92, continuation, result, bus,
            entity_record_address);
    }
    dispatch_animation_continuation(
        kAnimationContinuation_0x139D8, continuation, result, bus,
        entity_record_address);
    return result;
}

EntityAnimationEntryResult run_entity_animation_entry_0x13904(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation,
    const EntityAnimationResourceContinuationRoutine& resource_continuation) {
    EntityAnimationEntryResult result{false, false, false, 0, 0};

    const auto timer = bus.read16(entity_record_address + 0xB6U);
    const auto next_timer = static_cast<std::uint16_t>(timer - 1U);
    bus.write16(entity_record_address + 0xB6U, next_timer);
    if (static_cast<std::int16_t>(next_timer) >= 0) {
        return result;
    }
    result.timer_expired = true;

    if (bus.read8(session_ram::kFrameListBuildMode) != 0) {
        result.suppressed_by_global_gate = true;
        return result;
    }

    bus.write16(entity_record_address + 0xB6U, 0x0014U);
    result.timer_rearmed = true;

    if (bus.read16(entity_record_address + 0x0EU) == 0x000FU) {
        run_animation_branch_0x13A4C(
            bus, entity_record_address, continuation, result);
        return result;
    }
    if ((bus.read8(entity_record_address + 0xB0U) & 0x80U) != 0) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x14508, continuation, result, bus,
            entity_record_address);
        return result;
    }

    if ((bus.read8(entity_record_address + 0x9EU) & 0x08U) != 0 &&
        (bus.read8(entity_record_address + 0x9EU) & 0x20U) != 0 &&
        bus.read16(session_ram::kFrameEntityGlobalGate) != 0) {
        run_animation_branch_0x13E8C(
            bus, entity_record_address, continuation, result);
        return result;
    }

    const auto state_word = bus.read16(entity_record_address + 0x0CU);
    if (state_word == 0 &&
        (bus.read8(entity_record_address + 0x9EU) & 0x04U) != 0) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x1E514, continuation, result, bus,
            entity_record_address);
        return result;
    }
    if ((bus.read8(entity_record_address + 0xB0U) & 0x40U) != 0) {
        run_animation_branch_0x13E8C(
            bus, entity_record_address, continuation, result);
        return result;
    }

    if (state_word == 0) {
        if (bus.read16(session_ram::kFrameEntityGlobalGate) == 0) {
            dispatch_animation_continuation(
                kAnimationContinuation_0x142AE, continuation, result, bus,
                entity_record_address);
            return result;
        }
        if (bus.read8(ram::kEntityStateGlobalPhase) == 0 ||
            (bus.read8(entity_record_address + 0x10U) & 0x08U) != 0) {
            dispatch_animation_continuation(
                kAnimationContinuation_0x1450E, continuation, result, bus,
                entity_record_address);
            return result;
        }
        if (bus.read8(entity_record_address + 0x4EU) == 0x28U) {
            dispatch_animation_continuation(
                kAnimationContinuation_0x13E40, continuation, result, bus,
                entity_record_address);
            return result;
        }
        dispatch_animation_continuation(
            kAnimationContinuation_0x1450E, continuation, result, bus,
            entity_record_address);
        return result;
    }

    if ((bus.read8(entity_record_address + 0x9EU) & 0x40U) != 0) {
        dispatch_animation_continuation(
            kAnimationContinuation_0x1DBAA, continuation, result, bus,
            entity_record_address);
    }
    if (bus.read16(session_ram::kFrameEntityGlobalGate) == 0) {
        if (!run_animation_branch_0x13EF4(
            bus, entity_record_address, continuation, resource_continuation,
            result)) {
            return result;
        }
    }
    dispatch_animation_continuation(
        kAnimationContinuation_0x139D8, continuation, result, bus,
        entity_record_address);
    return result;
}

EntityAnimationEntryResult run_entity_animation_dispatch_0xAC84(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation,
    const EntityAnimationResourceContinuationRoutine& resource_continuation) {
    if ((bus.read8(entity_record_address + 0x10U) & 0x08U) != 0) {
        return run_entity_animation_entry_0x138B8(
            bus, entity_record_address, continuation, resource_continuation);
    }
    return run_entity_animation_entry_0x13904(
        bus, entity_record_address, continuation, resource_continuation);
}

EntityAnimationDelayResult update_entity_animation_delay_0xABFC(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d0) {
    // 0xD7EA scans state bits 12..14. On the first set bit it subtracts 11
    // from the bit number and multiplies the resulting word by four; with no
    // set bit the adjustment is zero.
    const auto state = bus.read16(entity_record_address + 0x90U);
    std::uint16_t adjustment = 0;
    for (std::uint16_t bit = 0x0CU; bit <= 0x0EU; ++bit) {
        if ((state & static_cast<std::uint16_t>(1U << bit)) != 0) {
            adjustment = static_cast<std::uint16_t>((bit - 0x0BU) * 4U);
            break;
        }
    }

    // 0xD81C computes (byte + byte) >> 1 and 0xD7EA then applies the state
    // adjustment and subtracts one, clamping a negative signed word to zero.
    auto sampled_base = calculate_entity_half_base_0xD81C(
        bus, entity_record_address);
    auto working = static_cast<std::uint16_t>(sampled_base + adjustment - 1U);
    if (static_cast<std::int16_t>(working) < 0) {
        working = 0;
    }

    auto delay = static_cast<std::uint16_t>(0x006AU -
                                             static_cast<std::uint16_t>(
                                                 static_cast<std::uint32_t>(
                                                     working) * 4U));
    if (d0 == 2U) {
        delay = static_cast<std::uint16_t>(delay << 1U);
    } else if (d0 != 0) {
        // 68000 ASR.W rounds negative values toward negative infinity; spell
        // that out instead of depending on the host compiler's signed shift.
        const auto signed_delay = static_cast<std::int16_t>(delay);
        const auto shifted = signed_delay < 0
            ? -static_cast<std::int16_t>(
                  (-(static_cast<std::int32_t>(signed_delay)) + 1) / 2)
            : static_cast<std::int16_t>(signed_delay / 2);
        delay = static_cast<std::uint16_t>(shifted);
    }
    if (static_cast<std::int16_t>(delay) < static_cast<std::int16_t>(0x000F)) {
        delay = 0x000FU;
    }
    bus.write16(entity_record_address + 0xDCU, delay);
    return {sampled_base, adjustment, delay};
}

EntityAnimationResourceDecisionResult
run_entity_animation_resource_decision_0x14D8E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress source_record_address) {
    bus.write16(session_ram::kEntityAnimationResourceGateState, 0xFFFFU);
    bus.write32(session_ram::kEntityAnimationResourceSource,
                source_record_address);

    const auto scratch_record = prepare_animation_resource_scratch_0x14FF8(
        bus, entity_record_address, source_record_address);
    const auto probe = run_animation_resource_terrain_probe_0x14F26(
        bus, scratch_record);
    bus.write32(scratch_record, probe.fixed_d5);
    bus.write32(scratch_record + 0x04U, probe.fixed_d6);

    const auto terrain_nonzero = probe.terrain.status != 0;
    const auto terrain_collision_gate_active =
        bus.read16(session_ram::kEntityTerrainCollisionGate) != 0;
    if (terrain_nonzero && !terrain_collision_gate_active) {
        bus.write16(session_ram::kEntityAnimationResourceGateState, 0);
        return {true, true, false, false, 0, scratch_record};
    }

    std::uint16_t occupancy_checks = 0;
    bool occupancy_scan_stopped = false;
    for (std::size_t iteration = 0; iteration <= 0x1AU; ++iteration) {
        ++occupancy_checks;
        if (run_animation_occupancy_scan_0x15ABA(
                bus, scratch_record, probe.d5, probe.d6)) {
            occupancy_scan_stopped = true;
            break;
        }
    }

    bus.write16(session_ram::kEntityAnimationResourceGateState, 0);
    return {false, terrain_nonzero, terrain_collision_gate_active,
            occupancy_scan_stopped, occupancy_checks, scratch_record};
}

EntityAnimationResourceGateResult run_entity_animation_resource_gate_0x13F76(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationResourceContinuationRoutine& continuation) {
    EntityAnimationResourceGateResult result{false, false, 0, 0};
    GenesisAddress a1 = 0;

    const auto call = [&](GenesisAddress target) {
        ++result.continuation_calls;
        EntityAnimationResourceCallContext context{target, a1, false};
        if (continuation) {
            continuation(bus, entity_record_address, context);
        } else if (target == 0x0001DBAAU) {
            arm_entity_global_gate_0x1DBAA(bus);
        } else if (target == 0x0001E2E0U) {
            queue_resource_load_0x1E2E0(bus);
        } else if (target == 0x00014D8EU) {
            const auto decision =
                run_entity_animation_resource_decision_0x14D8E(
                    bus, entity_record_address, a1);
            context.a1 = decision.scratch_record;
            context.carry = decision.carry;
        } else {
            ++result.unresolved_calls;
        }
        a1 = context.a1;
        return context.carry;
    };

    // 0x13F76 exits immediately while the shared entity gate is active.
    if (bus.read16(session_ram::kFrameEntityGlobalGate) != 0) {
        result.blocked_by_global_gate = true;
        return result;
    }

    // A nonzero FFE16F tail-jumps directly to 0x1DBAA.
    if (bus.read8(session_ram::kEntityReactionSpecialGate) != 0) {
        call(0x0001DBAAU);
        return result;
    }
    if ((bus.read8(entity_record_address + 0x9EU) & 0x01U) != 0 ||
        (bus.read8(entity_record_address + 0x10U) & 0x20U) == 0) {
        return result;
    }

    a1 = bus.read32(ram::kEntityObjectControlPointer);
    if (call(0x00014D8EU)) {
        return result;
    }

    auto difference = static_cast<std::uint16_t>(
        bus.read16(entity_record_address + 0x2EU) -
        bus.read16(session_ram::kEntityAnimationResourceReference));
    if (static_cast<std::int16_t>(difference) < 0) {
        difference = static_cast<std::uint16_t>(0U - difference);
    }
    if (difference >= 0x4000U) {
        return result;
    }

    if (bus.read16(session_ram::kEntityAnimationResourceMode) != 2U) {
        bus.write8(entity_record_address + 0x9EU,
                   static_cast<std::uint8_t>(
                       bus.read8(entity_record_address + 0x9EU) | 0x01U));
        return result;
    }
    if ((bus.read8(a1 + 0x11U) & 0x80U) != 0) {
        return result;
    }

    bus.write32(ram::kResourcePointer, 0x00020FECU);
    call(0x0001E2E0U);
    bus.write8(entity_record_address + 0x9EU,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0x9EU) | 0x01U));
    result.resource_queued = true;
    return result;
}

EntityAnimationAcaeResult run_entity_animation_update_0xACAE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationAcaeContinuationRoutine& continuation) {
    EntityAnimationAcaeResult result{true, 0, 0};
    std::uint16_t d0 = 0;
    GenesisAddress a1 = 0;
    GenesisAddress a0 = entity_record_address;

    const auto call = [&](GenesisAddress target) {
        ++result.continuation_calls;
        EntityAnimationAcaeCallContext context{target, d0, a1, a0, false};
        if (continuation) {
            continuation(bus, a0, context);
        } else if (target == 0x0001DBAAU) {
            // This leaf is already typed and has no return-register contract
            // used by 0xACAE.
            arm_entity_global_gate_0x1DBAA(bus);
        } else if (target == 0x000530DEU) {
            // 0xACAE supplies D0=0x4B at this one call site. The transport
            // gate is direct; its command result does not affect the branch.
            dispatch_z80_command_0x530DE(bus, context.d0);
        } else if (target == 0x0000ABFCU) {
            // ABFC discards the inherited D1, derives its delay from the
            // record state, and writes the result through the current A0.
            update_entity_animation_delay_0xABFC(
                bus, context.a0, context.d0);
        } else {
            ++result.unresolved_calls;
        }
        d0 = context.d0;
        a1 = context.a1;
        a0 = context.a0;
        return context.carry;
    };

    const auto finish_ready_or_mark = [&]() {
        // 0xAE26 selects between the ordinary B574 continuation and the
        // B45C wakeup marker using the controller/status word.
        d0 = bus.read16(ram::kControllerDecode0);
        if (d0 != 0xFFFFU) {
            bus.write16(a0 + 0x34U, d0);
            bus.write32(a0 + 0x3AU, 0x0000B574U);
            return;
        }
        if (bus.read32(a0 + 0x3AU) == 0x0000B45CU) {
            return;
        }
        bus.write8(a0 + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(a0 + 0x11U) | 1U));
        bus.write32(a0 + 0x3AU, 0x0000B45CU);
    };

    const auto mark_wakeup = [&]() {
        bus.write8(a0 + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(a0 + 0x11U) | 1U));
        bus.write32(a0 + 0x3AU, 0x0000B45CU);
    };

    const auto finish_with_d0 = [&](std::uint16_t value) {
        bus.write16(a0 + 0x34U, value);
        bus.write32(a0 + 0x3AU, 0x0000B574U);
    };

    if (bus.read8(ram::kVblankAnimationLock) != 0) {
        mark_wakeup();
        return result;
    }

    if (bus.read16(session_ram::kEntityReactionTransitionState) != 0) {
        d0 = static_cast<std::uint16_t>(
            bus.read16(a0 + 0xCCU) + 0x8000U);
        bus.write16(a0 + 0x2EU, d0);
        bus.write16(a0 + 0x34U, d0);
        // The ROM writes B45C immediately before overwriting the handler with
        // B574; both stores are retained in the direct bus trace.
        bus.write32(a0 + 0x3AU, 0x0000B45CU);
        bus.write32(a0 + 0x3AU, 0x0000B574U);
        return result;
    }

    if (bus.read8(session_ram::kEntityAnimationUpdateGate) != 0) {
        if (bus.read8(session_ram::kEntityAnimationUpdateTimer) != 0) {
            const auto next = static_cast<std::uint8_t>(
                bus.read8(session_ram::kEntityAnimationUpdateTimer) - 1U);
            bus.write8(session_ram::kEntityAnimationUpdateTimer, next);
            mark_wakeup();
            return result;
        }
        finish_with_d0(0);
        return result;
    }

    // 0xAD08 first handles the A-button relationship call. Its inherited
    // selector/register state is intentionally a typed continuation until
    // that second 0x1DC4C caller is recovered.
    if ((bus.read16(ram::kInputPort0) & 0x1000U) != 0) {
        call(0x0001DC4CU);
    }

    const auto timer = bus.read16(a0 + 0xDCU);
    if (static_cast<std::int16_t>(timer) >= 0) {
        if ((bus.read16(ram::kInputPort0) & 0x4000U) != 0) {
            bus.write16(session_ram::kFrameTransition, 0xFFFFU);
        }
        finish_ready_or_mark();
        return result;
    }

    if (bus.read16(session_ram::kFrameTransition) != 0) {
        bus.write16(session_ram::kFrameTransition, 0);
        bus.write8(a0 + 0x43U, 0);
    } else if ((bus.read16(ram::kInputPort0) & 0x0040U) != 0) {
        bus.write8(a0 + 0x43U, 0);
    } else if (bus.read16(ram::kControllerDecode0) != 0xFFFFU) {
        bus.write8(a0 + 0x43U, 0);
        finish_ready_or_mark();
        return result;
    } else {
        d0 = 1;
        call(0x0000ABFCU);
        bus.write8(a0 + 0x43U,
                   static_cast<std::uint8_t>(
                       bus.read8(a0 + 0x43U) + 1U));
        finish_ready_or_mark();
        return result;
    }

    bus.write8(a0 + 0x43U, 0);
    if (static_cast<std::int8_t>(bus.read8(a0 + 0x56U)) <
        0) {
        call(0x0000AE6EU);
        if (bus.read8(ram::kEntityStateRandomBucket) != 0) {
            call(0x0000AFAEU);
        } else {
            call(0x0000AF7CU);
        }
        return result;
    }

    const auto phase = static_cast<std::int8_t>(
        bus.read8(a0 + 0x57U));
    if (phase >= static_cast<std::int8_t>(0x0D) &&
        phase < static_cast<std::int8_t>(0x10)) {
        const auto has_carry = call(0x00015646U);
        if (has_carry) {
            return result;
        }
        d0 = bus.read16(a1 + 0x2EU);
        call(0x0000B3FEU);
        return result;
    }

    if (phase == 0) {
        call(0x0000B1DCU);
        return result;
    }
    if (phase > 0x0E) {
        return result;
    }

    const auto has_carry = call(0x0001514EU);
    if (!has_carry) {
        d0 = bus.read16(a1 + 0x2EU);
        call(0x0000B39AU);
        return result;
    }

    if (!call(0x0000AAD2U)) {
        mark_wakeup();
        return result;
    }
    d0 = 0x004BU;
    call(0x000530DEU);
    d0 = 0;
    call(0x0000ABFCU);
    finish_ready_or_mark();
    return result;
}

EntityAnimationPassResult run_entity_animation_pass_0x391A(
    GenesisBus& bus,
    const FrameEntityUpdateHooks& hooks) {
    EntityAnimationPassResult result{1, false, false, false, 0};
    bus.write8(session_ram::kEntityAnimationState, 1);

    const auto command_for_record = [&](GenesisAddress record_address) {
        // 0x391A forms D4 from the entity type before each 0x033AA strip.
        // The addends are word operations after the unsigned multiply.
        const auto type = bus.read16(record_address + 0x3EU);
        return static_cast<std::uint32_t>(type) * 0x0400U +
               0x0100U - 0x0FBCU;
    };
    const auto stream_strip = [&](GenesisAddress source,
                                  GenesisAddress record_address) {
        ShadowrunRuntime(bus).stream_vdp_grid(
            source, command_for_record(record_address), 5U, 7U, 0x6000U);
    };

    const auto command = dispatch_z80_command_0x530DE(bus, 0x25U);
    result.command_dispatched = command.command_sent;

    const auto initial_record = bus.read32(ram::kEntityObjectControlPointer);
    stream_strip(0x0006AA7EU, initial_record);
    const auto linked_record = bus.read32(initial_record + 0xA2U);
    if (linked_record != 0) {
        bus.write8(linked_record + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(linked_record + 0x11U) & ~0x10U));
    }

    GenesisAddress terminal_record = initial_record;
    for (;;) {
        const auto current_record = bus.read32(ram::kEntityObjectControlPointer);
        const auto type = bus.read16(current_record + 0x3EU);
        if (type == 2U) {
            const auto state = static_cast<std::uint8_t>(
                bus.read8(session_ram::kEntityAnimationState) - 1U);
            bus.write8(session_ram::kEntityAnimationState, state);
            bus.write8(current_record + 0x10U,
                       static_cast<std::uint8_t>(
                           bus.read8(current_record + 0x10U) & ~0x02U));
            bus.write32(ram::kEntityObjectControlPointer,
                        session_ram::kEntityPhaseBase);
            terminal_record = current_record;
            if (static_cast<std::int8_t>(state) < 0) {
                break;
            }
        } else {
            bus.write8(current_record + 0x10U,
                       static_cast<std::uint8_t>(
                           bus.read8(current_record + 0x10U) & ~0x02U));
            bus.write32(ram::kEntityObjectControlPointer,
                        current_record + session_ram::kEntityPhaseStride);
        }

        const auto next_record = bus.read32(ram::kEntityObjectControlPointer);
        bus.write8(next_record + 0x10U,
                   static_cast<std::uint8_t>(
                       bus.read8(next_record + 0x10U) | 0x02U));
        terminal_record = next_record;
        ++result.records_examined;
        if ((bus.read8(next_record + 0x10U) & 1U) == 0) {
            continue;
        }
        if (bus.read8(session_ram::kEntityAnimationGate) != 0) {
            break;
        }
        if ((bus.read8(next_record + 0x10U) & 0x40U) != 0) {
            continue;
        }
        if ((bus.read8(next_record + 0x10U) & 0x10U) == 0) {
            break;
        }
    }

    bus.write8(session_ram::kEntityAnimationState, 0);
    result.selected_record_changed = terminal_record != initial_record;
    if (result.selected_record_changed) {
        bus.write16(session_ram::kEntityAnimationResetWord, 0xFFFFU);
        bus.write16(session_ram::kEntityAnimationResetCounter, 3U);
        result.reset_written = true;
    }

    const auto final_record = bus.read32(ram::kEntityObjectControlPointer);
    if (hooks.routine_0x1DC4C) {
        hooks.routine_0x1DC4C(bus, final_record);
    } else {
        // 0x033AA establishes D1 as seven before its 0x01232 stream. That
        // leaf preserves D1, so the 0x391A caller reaches 0x1DC4C with this
        // exact selector even though 0x1DC4C itself has no argument setup.
        update_entity_relationship_0x1DC4C(bus, final_record, 7U);
    }
    stream_strip(0x0006AADEU, final_record);

    if (hooks.routine_0x03548) {
        hooks.routine_0x03548(bus);
    } else {
        run_entity_display_pass_0x03548(bus);
    }
    if (hooks.routine_0x038FA) {
        hooks.routine_0x038FA(bus);
    } else {
        result.unresolved_calls += run_entity_display_pass_0x038FA(
            bus, hooks);
    }
    return result;
}

FrameEntityUpdateResult update_frame_entity_records_0xA8EA(
    GenesisBus& bus, const FrameEntityUpdateHooks& hooks) {
    build_frame_entity_lists_0x13874(bus);

    FrameEntityUpdateResult result{
        session_ram::kEntityPhaseCount, 0, 0, 0, 0, 0
    };

    const auto call_unresolved = [&](const FrameEntityRoutine& routine,
                                     GenesisAddress record_address) {
        if (routine) {
            routine(bus, record_address);
        } else {
            ++result.unresolved_calls;
        }
    };

    const auto call_ab02 = [&](GenesisAddress record_address) {
        if (hooks.routine_0xAB02) {
            hooks.routine_0xAB02(bus, record_address);
            return;
        }

        // The direct flag/list work at 0xAB02 retains its typed resource and
        // 0x16B98 entity-slot leaves; the PRNG/table choice remains
        // data-driven from the ROM image.
        if ((bus.read8(record_address + 0xB0U) & 0x80U) != 0) {
            bus.write32(ram::kResourcePointer, 0x0001F210U);
            queue_resource_load_0x1E2EC(bus);
        }
        if ((bus.read8(record_address + 0x10U) & 0x08U) == 0) {
            update_entity_spawn_slot_0x16B98(bus, record_address);
            if ((bus.read8(record_address + 0x9EU) & 0x40U) != 0) {
                const auto counter = static_cast<std::uint8_t>(
                    bus.read8(session_ram::kFrameEntityTransitionCounter) - 1U);
                bus.write8(session_ram::kFrameEntityTransitionCounter, counter);
                if (counter == 0) {
                    clear_entity_relationships_0x1DBF0(bus);
                    const auto queued = bus.read32(
                        session_ram::kFrameEntityGlobalPointer);
                    if (queued != 0) {
                        bus.write32(ram::kResourcePointer, queued);
                        queue_resource_load_0x1E2EC(bus);
                        bus.write32(session_ram::kFrameEntityGlobalPointer, 0);
                    }
                }
            }
        }

        bus.write8(record_address + 0x10U, static_cast<std::uint8_t>(
            bus.read8(record_address + 0x10U) | 0x40U));
        if ((bus.read8(record_address + 0x10U) & 0x20U) == 0) {
            return;
        }
        for (std::size_t index = 0; index <= 6U; ++index) {
            const auto primary_record = static_cast<GenesisAddress>(
                0xFF0400U + index * 0x100U);
            if ((bus.read8(primary_record + 0x10U) & 1U) != 0 &&
                bus.read32(primary_record + 0xA8U) == record_address) {
                bus.write8(primary_record + 0x10U,
                           static_cast<std::uint8_t>(
                               bus.read8(primary_record + 0x10U) | 0x20U));
            }
        }
    };

    const auto call_1e514 = [&](GenesisAddress record_address) {
        const auto old_b0 = bus.read8(record_address + 0xB0U);
        bus.write8(record_address + 0xB0U,
                   static_cast<std::uint8_t>(old_b0 & ~0x80U));
        if ((old_b0 & 0x80U) != 0) {
            bus.write16(session_ram::kFrameEntityGlobalReset, 0);
        }
        bus.write8(record_address + 0x10U,
                   static_cast<std::uint8_t>(
                       bus.read8(record_address + 0x10U) & ~1U));
        if ((bus.read8(record_address + 0x10U) & 0x08U) == 0) {
            if (hooks.routine_0x1A304) {
                hooks.routine_0x1A304(bus, record_address);
            } else {
                clear_indexed_object_byte(
                    bus, bus.read16(record_address + 0x30U));
            }
        }
    };

    const auto call_ac5e = [&](GenesisAddress record_address) {
        if ((bus.read8(record_address + 0x10U) & 0x08U) == 0) {
            return;
        }
        const auto collision_state = bus.read16(
            session_ram::kFrameEntityCollisionState);
        if (collision_state == 0) {
            bus.write8(record_address + 0xB0U,
                       static_cast<std::uint8_t>(
                           bus.read8(record_address + 0xB0U) & ~0x20U));
        }
        if (static_cast<std::int16_t>(collision_state) < 0) {
            if (hooks.routine_0xB7B8) {
                hooks.routine_0xB7B8(bus, record_address);
            } else if (hooks.entity_reaction_registers) {
                const auto reaction = run_entity_reaction_0xB7B8(
                    bus, record_address,
                    hooks.entity_reaction_registers(bus, record_address));
                result.unresolved_calls += reaction.unresolved_calls;
            } else {
                ++result.unresolved_calls;
            }
        }
    };

    const auto call_ac84 = [&](GenesisAddress record_address) {
        if (hooks.routine_0xAC84) {
            hooks.routine_0xAC84(bus, record_address);
        } else {
            const auto animation = run_entity_animation_dispatch_0xAC84(
                bus, record_address, hooks.entity_animation_continuation);
            result.unresolved_calls += animation.unresolved_calls;
        }
        // 0xAC84's post-call 0xAE5E leaf is fully known: it arms the record
        // and installs the ordinary B45C continuation while the frame is live.
        if (bus.read8(ram::kVblankAnimationLock) != 0) {
            bus.write8(record_address + 0x11U,
                       static_cast<std::uint8_t>(
                           bus.read8(record_address + 0x11U) | 1U));
            bus.write32(record_address + 0x3AU, 0x0000B45CU);
        }
    };

    const auto call_bcae = [&](GenesisAddress record_address) {
        if (bus.read16(session_ram::kSessionMode) != 5U ||
            bus.read16(record_address + 0x02U) <= 0x11FFU ||
            bus.read16(record_address + 0x02U) >= 0x1F80U ||
            bus.read16(record_address) <= 0x25FFU) {
            return;
        }
        const auto x = bus.read16(record_address);
        if (x < 0x2800U) {
            bus.write16(record_address + 0x02U, static_cast<std::uint16_t>(
                bus.read16(record_address + 0x02U) - 8U));
        } else if (x > 0x3BFFU && x < 0x3E00U) {
            bus.write16(record_address + 0x02U, static_cast<std::uint16_t>(
                bus.read16(record_address + 0x02U) + 8U));
        }
    };

    const auto call_handler = [&](GenesisAddress record_address) {
        const auto handler = bus.read32(record_address + 0x3AU);
        if (handler == rom::kEntitySlotNoOpHandler) {
            return;
        }
        ++result.handler_calls;
        call_unresolved(hooks.routine_0xA8EA_handler, record_address);
    };

    const auto finish_record = [&](GenesisAddress record_address) {
        if ((bus.read8(record_address + 0x10U) & 0x04U) != 0) {
            call_bcae(record_address);
            if (hooks.routine_0xEB92) {
                hooks.routine_0xEB92(bus, record_address);
            } else {
                const auto collision = run_entity_collision_pass_0xEB92(
                    bus, record_address, hooks);
                result.unresolved_calls += collision.unresolved_calls;
            }

            const auto old_flags = bus.read8(record_address + 0x11U);
            bus.write8(record_address + 0x11U,
                       static_cast<std::uint8_t>(old_flags & ~0x02U));
            if ((old_flags & 0x02U) != 0) {
                bus.write32(record_address + 0xD4U,
                            bus.read32(record_address + 0x3AU));
                bus.write32(record_address + 0x3AU, 0x0000B49CU);
            }
        }

        bus.write8(record_address + 0x11U,
                   static_cast<std::uint8_t>(
                       bus.read8(record_address + 0x11U) & ~0x04U));
        const auto old_frame = bus.read16(record_address + 0x16U);
        call_handler(record_address);
        call_ac5e(record_address);
        if (old_frame != bus.read16(record_address + 0x16U)) {
            bus.write8(record_address + 0x11U,
                       static_cast<std::uint8_t>(
                           bus.read8(record_address + 0x11U) | 1U));
        }
    };

    const auto call_aa54 = [&](GenesisAddress record_address) {
        if ((bus.read8(record_address + 0x10U) & 0x02U) != 0) {
            if (hooks.routine_0xACAE) {
                hooks.routine_0xACAE(bus, record_address);
            } else {
                const auto animation = run_entity_animation_update_0xACAE(
                    bus, record_address,
                    hooks.entity_animation_acae_continuation);
                result.unresolved_calls += animation.unresolved_calls;
            }
        } else if ((bus.read8(record_address + 0x10U) & 0x04U) != 0) {
            call_ac84(record_address);
        }
    };

    for (std::size_t index = 0; index < session_ram::kEntityPhaseCount;
         ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase +
            index * session_ram::kEntityPhaseStride);
        if ((bus.read8(record_address + 0x10U) & 1U) == 0) {
            continue;
        }
        ++result.active_records;

        const auto timer = bus.read16(record_address + 0xDCU);
        if (static_cast<std::int16_t>(timer) >= 0) {
            bus.write16(record_address + 0xDCU,
                        static_cast<std::uint16_t>(timer - 1U));
            ++result.timer_decrements;
        }

        if ((bus.read8(record_address + 0x10U) & 0x04U) == 0) {
            call_aa54(record_address);
            finish_record(record_address);
            continue;
        }
        if ((bus.read8(record_address + 0x10U) & 0x40U) != 0) {
            finish_record(record_address);
            continue;
        }

        if ((bus.read8(record_address + 0x11U) & 0x10U) != 0) {
            // 0x15BB4 is an RTS leaf, so D0's low byte is the current +0x16
            // value when the ROM stores it into +0xB3.
            bus.write8(record_address + 0xB3U,
                       static_cast<std::uint8_t>(
                           bus.read16(record_address + 0x16U)));
        }

        const auto frame = bus.read16(record_address + 0x16U);
        const auto frame_signed = static_cast<std::int16_t>(frame);
        const auto old_transition = bus.read32(record_address + 0x3AU);
        const auto run_transition = [&]() {
            const auto old_flags = bus.read8(record_address + 0x10U);
            bus.write8(record_address + 0x10U,
                       static_cast<std::uint8_t>(old_flags | 0x10U));
            return (old_flags & 0x10U) != 0;
        };
        const auto maybe_391a = [&]() {
            if ((bus.read8(record_address + 0x10U) & 0x02U) != 0) {
                if (hooks.routine_0x0391A) {
                    hooks.routine_0x0391A(bus, record_address);
                } else {
                    const auto animation = run_entity_animation_pass_0x391A(
                        bus, hooks);
                    result.unresolved_calls += animation.unresolved_calls;
                }
            }
        };

        if (frame_signed < 0) {
            if (old_transition == 0x0000B0D4U ||
                old_transition == 0x0000B10CU) {
                call_ab02(record_address);
                call_1e514(record_address);
            } else if (bus.read8(record_address + 0x9CU) == 0) {
                if (run_transition()) {
                    if (bus.read16(record_address + 0x0EU) != 0x000EU) {
                        call_ab02(record_address);
                        maybe_391a();
                    } else {
                        call_ab02(record_address);
                        call_1e514(record_address);
                    }
                } else {
                    ++result.animation_fallbacks;
                    initialize_entity_animation_0xB330(
                        bus, record_address);
                }
            } else if (bus.read8(record_address + 0x4CU) == 0) {
                if (run_transition()) {
                    if (bus.read16(record_address + 0x0EU) != 0x000EU) {
                        call_ab02(record_address);
                        maybe_391a();
                    } else {
                        call_ab02(record_address);
                        call_1e514(record_address);
                    }
                } else {
                    ++result.animation_fallbacks;
                    initialize_entity_animation_0xB330(
                        bus, record_address);
                }
            } else {
                call_aa54(record_address);
                finish_record(record_address);
                continue;
            }
        } else if (frame_signed <= 0x003C) {
            if (bus.read8(record_address + 0x9CU) == 0) {
                if (run_transition()) {
                    call_ab02(record_address);
                    maybe_391a();
                } else {
                    ++result.animation_fallbacks;
                    initialize_entity_animation_0xB330(
                        bus, record_address);
                }
            } else if (bus.read8(record_address + 0x4CU) != 0) {
                call_aa54(record_address);
                finish_record(record_address);
                continue;
            } else if (run_transition()) {
                call_ab02(record_address);
                maybe_391a();
            } else {
                ++result.animation_fallbacks;
                initialize_entity_animation_0xB330(
                    bus, record_address);
            }
        }

        finish_record(record_address);
    }
    return result;
}

std::uint16_t calculate_frame_horizontal_status_0x3A90(
    GenesisBus& bus, std::uint16_t d3, std::uint16_t d6) {
    auto d1 = static_cast<std::uint16_t>(0);
    const auto difference = static_cast<std::uint16_t>(d6 - d3);
    if (d6 >= d3) {
        // The ROM takes the carry-set branches for the two lower-bound
        // comparisons before selecting the same marker-dependent result.
        if (difference < 3U) {
            if (difference < 0x180U) {
                return bus.read8(session_ram::kFrameClipHorizontalMode) ==
                               0xFFU
                           ? 0xFFFFU
                           : d1;
            }
            return bus.read8(session_ram::kFrameClipHorizontalMode) == 0xFEU
                       ? 0xFFFEU
                       : 0xFFFFU;
        }
        if (difference < 0x180U) {
            return bus.read8(session_ram::kFrameClipHorizontalMode) == 0xFFU
                       ? 0xFFFFU
                       : d1;
        }
        return bus.read8(session_ram::kFrameClipHorizontalMode) == 0xFEU
                   ? 0xFFFEU
                   : 0xFFFFU;
    }

    const auto reflected = static_cast<std::uint16_t>(0U - difference);
    if (reflected >= 3U) {
        return 2U;
    }
    d1 = 2U;
    if (reflected >= 0x180U) {
        return bus.read8(session_ram::kFrameClipHorizontalMode) == 2U
                   ? d1
                   : 1U;
    }
    return bus.read8(session_ram::kFrameClipHorizontalMode) == 2U ? 0U : 1U;
}

std::uint16_t calculate_frame_vertical_status_0x3B1E(
    GenesisBus& bus, std::uint16_t d4, std::uint16_t d7) {
    auto d2 = static_cast<std::uint16_t>(0);
    const auto difference = static_cast<std::uint16_t>(d7 - d4);
    if (d7 >= d4) {
        if (difference < 3U) {
            if (difference < 0x180U) {
                return bus.read8(session_ram::kFrameClipVerticalMode) ==
                               0xFFU
                           ? 0xFFFFU
                           : d2;
            }
            return bus.read8(session_ram::kFrameClipVerticalMode) == 0xFEU
                       ? 0xFFFEU
                       : 0xFFFFU;
        }
        if (difference < 0x180U) {
            return bus.read8(session_ram::kFrameClipVerticalMode) == 0xFFU
                       ? 0xFFFFU
                       : d2;
        }
        return bus.read8(session_ram::kFrameClipVerticalMode) == 0xFEU
                   ? 0xFFFEU
                   : 0xFFFFU;
    }

    const auto reflected = static_cast<std::uint16_t>(0U - difference);
    if (reflected >= 3U) {
        return 2U;
    }
    d2 = 2U;
    if (reflected >= 0x180U) {
        return bus.read8(session_ram::kFrameClipVerticalMode) == 2U
                   ? d2
                   : 1U;
    }
    return bus.read8(session_ram::kFrameClipVerticalMode) == 2U ? 0U : 1U;
}

FrameVectorTransformResult transform_frame_vector_0x15DCC(
    GenesisBus& bus, std::uint16_t d2, std::uint16_t d3) {
    // MOVE.W D2,D4 followed by MOVE.W D3,D2 leaves D3 as the signed
    // magnitude for both MULS operations. The zero test is on the copied D3
    // word and the two table lanes are forty bytes apart.
    if (d3 == 0) {
        return {0, 0};
    }

    // 0x15DCC copies D2 to D4, shifts that copy right by eight bits, and
    // clears bit 0 before the indexed word reads. The low byte of D2 is not
    // part of the interpolation-table index.
    const auto table_offset = static_cast<GenesisAddress>(
        static_cast<std::uint16_t>((d2 >> 8U) & 0x00FEU));
    const auto high_product = [&](GenesisAddress address) {
        const auto factor = static_cast<std::int32_t>(
            static_cast<std::int16_t>(bus.read16(address)));
        const auto magnitude = static_cast<std::int32_t>(
            static_cast<std::int16_t>(d3));
        const auto product = static_cast<std::int32_t>(factor * magnitude);
        return static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(product) >> 16U);
    };

    const auto table = rom::kObjectGenerationInterpolationTable + table_offset;
    return {high_product(table), high_product(table + 0x40U)};
}

std::uint32_t calculate_frame_distance_0x15CA4(
    std::uint32_t d0,
    std::uint32_t d1,
    std::uint16_t d2,
    std::uint16_t d3) {
    return reduce_frame_distance_registers_0x15CA4(d0, d1, d2, d3).d0;
}

FrameDeltaTransformResult transform_frame_delta_0x15D3C(
    GenesisBus& bus,
    std::uint16_t d2,
    std::uint16_t d3,
    std::uint16_t d4,
    std::uint16_t d5) {
    // 0x15D3C performs the two reference subtractions before entering
    // 0x15D4E. Every value below is a word, matching the source's MOVE.W,
    // NEG.W, LSR.W, and ADD.W operations.
    d2 = static_cast<std::uint16_t>(d2 - d4);
    d3 = static_cast<std::uint16_t>(d3 - d5);
    const auto signed_d2 = static_cast<std::int16_t>(d2);
    const auto signed_d3 = static_cast<std::int16_t>(d3);
    auto major_d2 = static_cast<std::uint16_t>(
        signed_d2 < 0 ? 0U - d2 : d2);
    auto major_d3 = static_cast<std::uint16_t>(
        signed_d3 < 0 ? 0U - d3 : d3);

    const auto d3_is_smaller = major_d3 < major_d2;
    auto major = d3_is_smaller ? major_d2 : major_d3;
    auto minor = d3_is_smaller ? major_d3 : major_d2;
    const auto average = static_cast<std::uint16_t>(
        major + static_cast<std::uint16_t>(minor >> 1U));

    // 0x15D74..0x15D7C reduce both axes until the major axis is below 0x20.
    while (major >= 0x0020U) {
        major = static_cast<std::uint16_t>(major >> 1U);
        minor = static_cast<std::uint16_t>(minor >> 1U);
    }

    // The first swap is selected by bit 0 of the pre-reduction average.
    if ((average & 1U) == 0) {
        std::swap(major, minor);
    }

    // TST.W D2/D3 contributes N/Z bits to the sixteen-entry direction table.
    // The table index is a word offset, hence the final shift by one.
    std::uint16_t direction_index = 0;
    if (signed_d2 < 0) {
        direction_index = static_cast<std::uint16_t>(direction_index | 0x0008U);
    } else if (d2 == 0) {
        direction_index = static_cast<std::uint16_t>(direction_index | 0x0004U);
    }
    if (signed_d3 < 0) {
        direction_index = static_cast<std::uint16_t>(direction_index | 0x0002U);
    } else if (d3 == 0) {
        direction_index = static_cast<std::uint16_t>(direction_index | 0x0001U);
    }
    direction_index = static_cast<std::uint16_t>(direction_index << 1U);
    auto direction = bus.read16(rom::kFrameDeltaDirectionTable +
                                direction_index);

    // 0x15DAC exchanges the reduced axes unless the selected direction word
    // has bit 6 set, then 0x15DB4 forms the packed correction-table index.
    if ((direction & 0x0040U) == 0) {
        std::swap(major, minor);
    }
    const auto correction_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(major << 5U) + minor);
    const auto correction = bus.read8(rom::kFrameDeltaCorrectionTable +
                                      correction_index);
    const auto low_direction = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(direction) + correction);
    direction = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(low_direction) << 8U);
    return {direction, average};
}

EntityAnimationLinkCheckResult check_entity_animation_link_0x15C84(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    // 0x15C84 takes the linked record from +0xA2 and leaves carry clear when
    // that pointer is null. The two flag tests belong to 0x15C58 and happen
    // before 0x14D58/0x15D3C is entered.
    const auto linked_record = bus.read32(entity_record_address + 0xA2U);
    if (linked_record == 0) {
        return {false, false, false, 0, 0, 0};
    }

    const auto flags = bus.read8(linked_record + 0x10U);
    if ((flags & 0x01U) == 0 || (flags & 0x40U) != 0) {
        return {false, true, false, linked_record, 0, 0};
    }

    // 0x14D58 loads D5/D4 from A0 and D3/D2 from A1 before calling 0x15D3C.
    // The public transform follows the callee's D2,D3,D4,D5 ordering.
    const auto transform = transform_frame_delta_0x15D3C(
        bus,
        bus.read16(linked_record + 0x04U),
        bus.read16(linked_record),
        bus.read16(entity_record_address + 0x04U),
        bus.read16(entity_record_address));
    const auto accepted = transform.d3 < 0x0200U;
    if (accepted) {
        bus.write16(entity_record_address + 0x34U, transform.d2);
    }
    return {accepted, true, true, linked_record, transform.d2,
            transform.d3};
}

FrameSpatialTailResult update_frame_spatial_tail_0x3BAC(GenesisBus& bus) {
    const auto transform = transform_frame_delta_0x15D3C(
        bus,
        bus.read16(ram::kCameraY),
        bus.read16(ram::kCameraX),
        bus.read16(session_ram::kFrameTransformState),
        bus.read16(session_ram::kFrameTransformProgress));
    FrameSpatialTailResult result{
        false, false, false, transform.d2, transform.d3, {0, 0, false},
        {0, 0, false}};

    // 0x3BCA copies D3 to D4, and the carry branch returns when the reduced
    // length is below the 0x20 threshold. The ROM clears FFDF5A on that path.
    if (transform.d3 < 0x0020U) {
        bus.write16(session_ram::kFrameSpatialStatus, 0);
        result.early_return = true;
        return result;
    }

    // E75A is ROL.W #3,D2, not a plain left shift; the three high bits wrap
    // into the low direction index before the ANDI.W #7.
    auto d2 = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(transform.d2 << 3U) |
        static_cast<std::uint16_t>(transform.d2 >> 13U));
    d2 = static_cast<std::uint16_t>(d2 & 0x0007U);
    auto d0 = static_cast<std::uint16_t>(d2 << 2U);
    auto d1 = bus.read16(rom::kFrameMovementTable + d0);
    d0 = static_cast<std::uint16_t>(d0 + 2U);
    d2 = bus.read16(rom::kFrameMovementTable + d0);

    // The original four equality checks bypass the doubled movement when a
    // camera edge is already at one of the two horizontal or vertical limits.
    if (transform.d3 >= 0x0028U &&
        bus.read16(ram::kCameraX) != bus.read16(ram::kCameraXUpperLimit) &&
        bus.read16(ram::kCameraX) != bus.read16(ram::kCameraXLowerLimit) &&
        bus.read16(ram::kCameraY) != bus.read16(ram::kCameraYUpperLimit) &&
        bus.read16(ram::kCameraY) != bus.read16(ram::kCameraYLowerLimit)) {
        d1 = static_cast<std::uint16_t>(d1 << 1U);
        d2 = static_cast<std::uint16_t>(d2 << 1U);
    }

    // 0x3C36 gates camera writes while either tile queue or the frame gate is
    // active. The transform result is still observable even when output is
    // suppressed.
    if (bus.read16(ram::kVblankQueueA) != 0 ||
        bus.read16(ram::kVblankQueueB) != 0 ||
        bus.read16(ram::kVblankIdleGate) != 0) {
        return result;
    }

    const auto old_horizontal = d1;
    auto horizontal_delta = static_cast<std::uint16_t>(d1 << 4U);
    if (horizontal_delta != 0) {
        const auto horizontal_negative =
            static_cast<std::int16_t>(horizontal_delta) < 0;
        const auto candidate = static_cast<std::uint16_t>(
            bus.read16(ram::kCameraX) + horizontal_delta);
        const auto lower_or_upper = bus.read16(ram::kCameraXUpperLimit);
        const auto outside = horizontal_negative
            ? static_cast<std::int16_t>(candidate) <
                  static_cast<std::int16_t>(lower_or_upper)
            : static_cast<std::int16_t>(candidate) >
                  static_cast<std::int16_t>(lower_or_upper);
        if (!outside) {
            bus.write16(ram::kCameraX, candidate);
            result.horizontal_updated = true;
            const auto stream_word = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(0U - old_horizontal) +
                bus.read16(ram::kVdpDataWord) +
                bus.read16(ram::kVdpDataWord2));
            const auto cache = static_cast<std::uint16_t>(
                horizontal_delta & 0xFF80U);
            if (cache != bus.read16(session_ram::kFrameMovementCacheHorizontal)) {
                bus.write16(session_ram::kFrameMovementCacheHorizontal, cache);
                result.horizontal_stream =
                    ShadowrunRuntime(bus).stream_frame_tile_plane_horizontal_0x40E2(
                        stream_word);
            }
        }
    }

    const auto old_vertical = d2;
    auto vertical_delta = static_cast<std::uint16_t>(d2 << 4U);
    if (vertical_delta != 0) {
        const auto vertical_negative =
            static_cast<std::int16_t>(vertical_delta) < 0;
        const auto candidate = static_cast<std::uint16_t>(
            bus.read16(ram::kCameraY) + vertical_delta);
        const auto limit = vertical_negative
            ? bus.read16(ram::kCameraYLowerLimit)
            : bus.read16(ram::kCameraYUpperLimit);
        const auto outside = vertical_negative
            ? static_cast<std::int16_t>(candidate) <
                  static_cast<std::int16_t>(limit)
            : static_cast<std::int16_t>(candidate) >
                  static_cast<std::int16_t>(limit);
        if (!outside) {
            bus.write16(ram::kCameraY, candidate);
            result.vertical_updated = true;
            const auto cache = static_cast<std::uint16_t>(
                vertical_delta & 0xFF80U);
            if (cache != bus.read16(session_ram::kFrameMovementCacheVertical)) {
                bus.write16(session_ram::kFrameMovementCacheVertical, cache);
                result.vertical_stream =
                    ShadowrunRuntime(bus).stream_frame_tile_plane_vertical_0x4018(
                        old_vertical);
            }
        }
    }
    return result;
}

FrameSpatialPreludeResult prepare_frame_spatial_state_0x3A1C(
    GenesisBus& bus) {
    if (bus.read8(ram::kEntityStateGlobalPhase) == 3U) {
        bus.write16(session_ram::kFrameTransformState, 0);
        bus.write16(session_ram::kFrameTransformProgress, 0x0600U);
        return {true, 0, 0,
                bus.read8(session_ram::kFrameClipHorizontalMode),
                bus.read8(session_ram::kFrameClipVerticalMode)};
    }

    const auto control = bus.read32(ram::kEntityObjectControlPointer);
    auto vector = transform_frame_vector_0x15DCC(
        bus, bus.read16(control + 0x2EU), 0x03C0U);
    vector.d3 = static_cast<std::uint16_t>(
        vector.d3 + bus.read16(control));
    if (static_cast<std::int16_t>(vector.d3) < 0) {
        vector.d3 = 0;
    }
    vector.d2 = static_cast<std::uint16_t>(
        vector.d2 + bus.read16(control + 4U));
    if (static_cast<std::int16_t>(vector.d2) < 0) {
        vector.d2 = 0;
    }

    const auto camera_x = static_cast<std::uint16_t>(
        bus.read16(ram::kCameraX) + 0x0700U);
    const auto camera_y = static_cast<std::uint16_t>(
        bus.read16(ram::kCameraY) + 0x0700U);
    const auto horizontal = calculate_frame_horizontal_status_0x3A90(
        bus, vector.d3, camera_x);
    bus.write8(session_ram::kFrameClipHorizontalMode,
               static_cast<std::uint8_t>(horizontal));
    const auto vertical = calculate_frame_vertical_status_0x3B1E(
        bus, vector.d2, camera_y);
    bus.write8(session_ram::kFrameClipVerticalMode,
               static_cast<std::uint8_t>(vertical));
    return {false, vector.d2, vector.d3,
            static_cast<std::uint8_t>(horizontal),
            static_cast<std::uint8_t>(vertical)};
}

PostObjectSpawnResult spawn_post_object_entity_0x5D6BC(
    GenesisBus& bus) {
    const auto record = allocate_post_object_record_0x15A8C(bus);
    if (!record.allocated) {
        return {false, false, 0, 0, 0, 0, 0};
    }

    // 0x5D6CC..0x5D6D4 copies exactly 128 words and advances both pointers;
    // the saved A1 restores the original record base for the fixed fields.
    auto source = static_cast<GenesisAddress>(rom::kPostObjectEntityTemplate);
    auto destination = record.record_address;
    for (std::size_t index = 0; index < 0x80U; ++index) {
        bus.write16(destination, bus.read16(source));
        source += 2U;
        destination += 2U;
    }

    bus.write32(record.record_address + 0x3AU, 0x00015390U);
    bus.write16(record.record_address + 0x2CU, 0x2000U);
    bus.write16(record.record_address, 0x1200U);
    bus.write16(record.record_address + 4U, 0x02C0U);
    bus.write16(record.record_address + 0x3EU, record.record_index);
    bus.write8(record.record_address + 0x10U, static_cast<std::uint8_t>(
        bus.read8(record.record_address + 0x10U) | 0x81U));
    bus.write16(record.record_address + 0x16U, 0x000CU);
    bus.write8(record.record_address + 0x41U, 3);
    bus.write16(record.record_address + 0x44U, 0);
    bus.write16(record.record_address + 0x46U, 0xFFFFU);

    const auto sprite = allocate_sprite_slot_0x1A2CC(bus);
    if (!sprite.allocated) {
        bus.write8(record.record_address + 0x10U, static_cast<std::uint8_t>(
            bus.read8(record.record_address + 0x10U) & ~1U));
        return {true, false, record.record_address, record.record_index,
                sprite.slot_index, sprite.d1_value, sprite.a2_value};
    }

    bus.write16(record.record_address + 0x30U, sprite.slot_index);
    // 0x1A2CC initializes D1/A2 as it scans the sprite-slot flags; those
    // returned words are copied into the new entity record here.
    bus.write16(record.record_address + 0x20U, sprite.d1_value);
    bus.write16(record.record_address + 0x26U, sprite.a2_value);
    bus.write16(record.record_address + 0x38U, 0x002AU);
    return {true, true, record.record_address, record.record_index,
            sprite.slot_index, sprite.d1_value, sprite.a2_value};
}

EntityPointerSortResult sort_entity_pointer_table(
    GenesisBus& bus, GenesisAddress pointer_table) {
    // ROM 0x5B1E is a bubble-sort-like pass. D3 starts at 0xE and DBF makes
    // fifteen adjacent comparisons per pass. The address-register post-
    // increments are represented by the explicit table index here; writes
    // remain 68000 longword writes to the original Genesis address.
    EntityPointerSortResult result{0, 0, 0, false};
    do {
        ++result.passes;
        bool swapped = false;
        for (std::size_t index = 0; index < 15; ++index) {
            ++result.comparisons;
            const auto left_pointer = bus.read32(pointer_table + index * 4U);
            const auto right_pointer =
                bus.read32(pointer_table + (index + 1U) * 4U);

            const auto effective_value = [&bus](GenesisAddress record) {
                if ((bus.read8(record + 0x10U) & 0x50U) != 0) {
                    return std::int16_t{0};
                }
                return static_cast<std::int16_t>(bus.read16(record + 4U));
            };

            // CMP.W D1,D0 followed by BGE leaves the pair untouched when
            // left >= right using signed 68000 condition-code semantics.
            if (effective_value(left_pointer) >= effective_value(right_pointer)) {
                continue;
            }
            bus.write32(pointer_table + index * 4U, right_pointer);
            bus.write32(pointer_table + (index + 1U) * 4U, left_pointer);
            ++result.swaps;
            swapped = true;
        }
        if (!swapped) {
            result.sorted = true;
            break;
        }
    } while (true);
    return result;
}

bool update_entity_timer_flag_0x4E38(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    // BTST #6,(0x9E,A0); RTS when the entity is in the guarded state.
    if ((bus.read8(entity_record_address + 0x9EU) & 0x40U) != 0) {
        return false;
    }
    if ((bus.read16(ram::kFrameCounter) & 0x003FU) != 0) {
        return false;
    }

    const auto timer_address = entity_record_address + 0x40U;
    const auto timer = bus.read8(timer_address);
    bus.write8(timer_address, static_cast<std::uint8_t>(timer - 1U));
    if (timer != 0) {
        return false;
    }
    bus.write8(entity_record_address + 0x11U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0x11U) | 0x80U));
    return true;
}

bool update_entity_timer_flag_0x4E64(
    GenesisBus& bus, GenesisAddress entity_record_address) {
    // The second helper runs only for nonzero six-bit frame phases.
    if ((bus.read16(ram::kFrameCounter) & 0x003FU) == 0) {
        return false;
    }

    const auto timer_address = entity_record_address + 0x41U;
    const auto timer = bus.read8(timer_address);
    bus.write8(timer_address, static_cast<std::uint8_t>(timer - 1U));
    if (timer != 0) {
        return false;
    }
    bus.write8(entity_record_address + 0x11U,
               static_cast<std::uint8_t>(
                   bus.read8(entity_record_address + 0x11U) | 0x20U));
    return true;
}

ObjectOutputPassResult ShadowrunMainState::run_object_output_pass(
    const ObjectOutputPassHooks& hooks,
    std::size_t max_records_per_list) const {
    // ROM 0x4658 begins by sorting the pointer table and then uses A3/D5 as
    // caller-held output state. The bus fields are updated only by the
    // original emitters, while this local pair preserves the register state
    // between nested calls.
    sort_entity_pointer_table(bus_);
    bus_.write16(ram::kObjectListCount, 1);
    auto output_address = ram::kObjectListInitialOutput;
    auto ordinal = static_cast<std::uint8_t>(2);
    ObjectOutputPassResult result{
        16, 0, 0, 0, false, false, output_address, ordinal
    };

    const auto emit_state_objects =
        [&](GenesisAddress entity_record) {
            const auto emitted = append_entity_state_objects(
                bus_, entity_record, output_address, ordinal,
                max_records_per_list);
            output_address = emitted.next_output_address;
            ordinal = static_cast<std::uint8_t>(emitted.next_ordinal);
            result.objects_emitted += emitted.emitted;
        };

    for (std::size_t index = 0; index < session_ram::kEntityPhaseCount; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPointerTable + index * 4U);
        const auto entity_record = bus_.read32(record_address);
        if ((bus_.read8(entity_record + 0x10U) & 1U) == 0) {
            continue;
        }

        ++result.active_records;
        if (bus_.read16(entity_record + 0x0CU) == 0) {
            const auto state = bus_.read16(entity_record + 0xB2U);
            if (state == 6U) {
                if (hooks.routine_0x557E) {
                    hooks.routine_0x557E(bus_, entity_record);
                } else {
                    const auto emitted = emit_entity_timed_objects_0x557E(
                        bus_, entity_record, output_address, ordinal,
                        max_records_per_list);
                    output_address = emitted.next_output_address;
                    ordinal = emitted.next_ordinal;
                    result.objects_emitted += emitted.objects_emitted;
                }
                continue;
            }
            if (state == 0x000DU) {
                if (hooks.routine_0x53C4) {
                    hooks.routine_0x53C4(bus_, entity_record);
                } else {
                    const auto random_scan = run_entity_random_scan_0x53C4(
                        bus_, entity_record, output_address, ordinal,
                        max_records_per_list);
                    output_address = random_scan.next_output_address;
                    ordinal = random_scan.next_ordinal;
                    result.objects_emitted += random_scan.objects_emitted;
                    if (random_scan.unresolved_0xAFF2_call) {
                        ++result.unresolved_calls;
                    }
                }
                continue;
            }
        } else {
            const auto routine = bus_.read32(entity_record + 0x3AU);
            if (routine == 0x0000AFE2U) {
                if (hooks.routine_0x5936) {
                    hooks.routine_0x5936(bus_, entity_record);
                } else {
                    const auto emitted = emit_entity_phase_objects_0x5936(
                        bus_, entity_record, output_address, ordinal,
                        max_records_per_list);
                    output_address = emitted.next_output_address;
                    ordinal = emitted.next_ordinal;
                    result.objects_emitted += emitted.objects_emitted;
                }
            } else if (bus_.read16(entity_record + 0x0EU) == 0x000DU &&
                       routine != 0x0000AFE0U) {
                if (hooks.routine_0x59EA) {
                    hooks.routine_0x59EA(bus_, entity_record);
                } else {
                    const auto emitted = emit_entity_phase_objects_0x59EA(
                        bus_, entity_record, output_address, ordinal,
                        max_records_per_list);
                    output_address = emitted.next_output_address;
                    ordinal = emitted.next_ordinal;
                    result.objects_emitted += emitted.objects_emitted;
                }
            }
        }

        const auto dispatch_index = bus_.read8(entity_record + 0xB2U);
        if (dispatch_index != 0) {
            if (hooks.routine_0x490A) {
                hooks.routine_0x490A(bus_, entity_record);
            } else {
                const auto dispatched = dispatch_entity_state_objects_0x490A(
                    bus_, entity_record, output_address, ordinal,
                    max_records_per_list);
                if (dispatched.handled) {
                    output_address = dispatched.next_output_address;
                    ordinal = dispatched.next_ordinal;
                    result.objects_emitted += dispatched.objects_emitted;
                } else {
                    ++result.unresolved_calls;
                }
            }
        }

        // ROM 0x46F8: BSET #5,(0x11,A0); only an already-set bit enters
        // 0x4E64. The helper itself is fully translated above.
        if (set_entity_flag(bus_, entity_record, 0x11U, 0x20U)) {
            update_entity_timer_flag_0x4E64(bus_, entity_record);
        }

        // ROM 0x4706: BSET #7,(0x11,A0); only an already-set bit enters
        // 0x4E38.
        if (set_entity_flag(bus_, entity_record, 0x11U, 0x80U)) {
            update_entity_timer_flag_0x4E38(bus_, entity_record);
        }

        // ROM 0x4714 invokes 0x5C7E only when bit 4 was already set.
        if (set_entity_flag(bus_, entity_record, 0x11U, 0x10U)) {
            const auto emitted = emit_entity_variant_objects(
                bus_, entity_record, output_address, ordinal,
                max_records_per_list);
            output_address = emitted.next_output_address;
            ordinal = emitted.next_ordinal;
            result.objects_emitted += emitted.objects_emitted;
        }

        // ROM 0x472E: BSET #6,(0x11,A0); the existing-bit path calls the
        // cycle/output helper.
        if (set_entity_flag(bus_, entity_record, 0x11U, 0x40U)) {
            if (hooks.routine_0x4E86) {
                hooks.routine_0x4E86(bus_, entity_record);
            } else {
                const auto emitted = emit_entity_cycle_objects(
                    bus_, entity_record, output_address, ordinal,
                    max_records_per_list);
                output_address = emitted.next_output_address;
                ordinal = emitted.next_ordinal;
                result.objects_emitted += emitted.descriptor_records_emitted;
                result.objects_emitted += emitted.list_records_emitted;
            }
        }

        // ROM 0x473C invokes 0x5B98 only when bit 7 of the entity flags is
        // already set.
        if (set_entity_flag(bus_, entity_record, 0x10U, 0x80U)) {
            const auto emitted = emit_animated_entity_objects(
                bus_, entity_record, output_address, ordinal,
                max_records_per_list);
            output_address = emitted.next_output_address;
            ordinal = emitted.next_ordinal;
            result.objects_emitted += emitted.objects_emitted;
        }

        const auto special_routine = bus_.read32(entity_record + 0x3AU);
        if (special_routine == 0x00015390U &&
            static_cast<std::int16_t>(bus_.read16(entity_record + 0x46U)) < 0 &&
            set_entity_flag(bus_, entity_record, 0x10U, 0x02U)) {
            if (bus_.read16(ram::kObjectGenerationReset) != 0) {
                const auto phase = static_cast<std::uint16_t>(
                    bus_.read16(ram::kFrameCounter) & 0x000FU);
                bool call_state_list = phase < 7U;
                if (phase == 7U) {
                    const auto timer = bus_.read16(0xFFDFDC);
                    const auto decremented = static_cast<std::uint16_t>(timer - 1U);
                    bus_.write16(0xFFDFDC, decremented);
                    if (static_cast<std::int16_t>(decremented) < 0) {
                        bus_.write16(ram::kObjectGenerationReset, 0);
                        call_state_list = true;
                    } else {
                        call_state_list = false;
                    }
                } else if (phase > 7U) {
                    call_state_list = false;
                }
                if (call_state_list) {
                    emit_state_objects(entity_record);
                }
            }
        }

        if (special_routine == 0x00015390U &&
            bus_.read16(entity_record + 0x34U) != 0) {
            const auto saved_c = bus_.read16(entity_record + 0x0CU);
            const auto saved_pointer = bus_.read32(entity_record + 0x12U);
            bus_.write16(entity_record + 0x0CU,
                         static_cast<std::uint16_t>(
                             saved_c - bus_.read16(entity_record + 8U)));
            bus_.write32(entity_record + 0x12U, 0x0000959AU);
            emit_state_objects(entity_record);
            bus_.write32(entity_record + 0x12U, saved_pointer);
            bus_.write16(entity_record + 0x0CU, saved_c);
        }

        bus_.write16(ram::kObjectListScratch, 1);
    }

    if (bus_.read16(0xFFDC9EU + 2U) != 0) {
        bus_.write32(ram::kObjectListOutputPointer, output_address);
        const auto update = update_indexed_object_record(
            bus_, 0xFFDC9EU, bus_.read16(ram::kCameraX),
            bus_.read16(ram::kCameraY), ordinal, max_records_per_list);
        output_address = bus_.read32(ram::kObjectListOutputPointer);
        ordinal = update.next_ordinal;
        result.objects_emitted += update.emitted_records;
    }
    if (bus_.read16(ram::kObjectGenerationReset) != 0) {
        bus_.write32(ram::kObjectListOutputPointer, output_address);
        const auto generated = generate_object_list_frame(
            bus_, ordinal, max_records_per_list);
        output_address = bus_.read32(ram::kObjectListOutputPointer);
        ordinal = generated.next_ordinal;
        result.objects_emitted += generated.objects_emitted;
    }

    if (hooks.routine_0x481E) {
        hooks.routine_0x481E(bus_);
    } else {
        const auto emitted = emit_entity_slot_objects(
            bus_, output_address, ordinal, max_records_per_list);
        output_address = emitted.next_output_address;
        ordinal = emitted.next_ordinal;
        result.objects_emitted += emitted.objects_emitted;
    }
    if (hooks.routine_0x5D29C) {
        hooks.routine_0x5D29C(bus_);
    } else {
        const auto post = run_post_object_update_0x5D29C(
            bus_, output_address, ordinal, max_records_per_list);
        output_address = post.next_output_address;
        ordinal = post.next_ordinal;
        result.objects_emitted += post.objects_emitted;
        result.unresolved_0x5D29C_call = post.unresolved_calls != 0;
        result.unresolved_calls += post.unresolved_calls;
    }

    output_address -= 8U;
    bus_.write8(output_address + 3U, 0);
    result.next_output_address = output_address;
    result.next_ordinal = ordinal;
    return result;
}

void ShadowrunMainState::initialise_session() const {
    if (bus_.read32(session_ram::kBootstrapMagic) != 0x12345678U) {
        bus_.write32(session_ram::kBootstrapMagic, 0x12345678U);
    }
    const auto random = static_cast<std::uint16_t>(
        bus_.read16(session_ram::kRandomState) +
        bus_.read16(session_ram::kRandomAddend));
    bus_.write16(session_ram::kRandomState, random);
    bus_.write16(session_ram::kReadyFlag0, 1);
    bus_.write16(session_ram::kReadyFlag1, 1);
    bus_.write16(session_ram::kMainLoopControl, 0xFFFFU);
}

void ShadowrunMainState::reset_session_work_fields() const {
    bus_.write32(session_ram::kModeBlock, 0x00000014U);
    bus_.write32(session_ram::kWorkLong12, 0);
    bus_.write32(session_ram::kWorkLong16, 0);
    bus_.write32(session_ram::kWorkLong1A, 0);
    bus_.write32(session_ram::kWorkLong1E, 0);
    bus_.write32(session_ram::kWorkLong22, 0);
    bus_.write16(session_ram::kWorkWord26, 0);
    bus_.write16(session_ram::kWorkWord27, 0);
    bus_.write32(session_ram::kWorkLong28, 0);
    bus_.write32(session_ram::kWorkLong2A, 0);
    bus_.write32(session_ram::kWorkLong2C, 0);
    bus_.write32(session_ram::kWorkLong34, 0);
    bus_.write32(session_ram::kWorkLong38, 0);
    bus_.write32(session_ram::kWorkLong30, 1);
    bus_.write16(session_ram::kSessionMode, 2);
    bus_.write8(session_ram::kModeByte, 2);
}

void ShadowrunMainState::begin_frame_window() const {
    bus_.write16(session_ram::kMainLoopControl, 0);
    bus_.write8(session_ram::kFrameActive, 1);
    bus_.write16(session_ram::kFrameInput0, 0);
    bus_.write16(session_ram::kFrameInput1, 0);
    bus_.write16(session_ram::kFrameInputLatch0, 0xFFFFU);
    bus_.write16(session_ram::kFrameInputLatch1, 0xFFFFU);
}

EntityPhaseResult ShadowrunMainState::run_entity_phase(
    const FrameWindowHooks& hooks) const {
    // ROM 0x4618 writes this marker before entering the fixed sixteen-record
    // loop and clears it after the 0x4658 post-pass returns.
    bus_.write16(session_ram::kEntityPhaseState, 1);

    EntityPhaseResult result{
        session_ram::kEntityPhaseCount, 0, 0, 0, 0, false
    };
    for (std::size_t index = 0; index < session_ram::kEntityPhaseCount; ++index) {
        const auto record_address = static_cast<GenesisAddress>(
            session_ram::kEntityPhaseBase +
            index * session_ram::kEntityPhaseStride);
        if ((bus_.read8(record_address + 0x10U) & 1U) == 0) {
            continue;
        }

        ++result.active_slots;
        project_entity_record(bus_, record_address,
                              bus_.read16(ram::kCameraX),
                              bus_.read16(ram::kCameraY));
        ++result.projected_slots;
        update_entity_timing(bus_, record_address);

        if (hooks.routine_0x600E) {
            hooks.routine_0x600E(bus_, record_address);
        } else {
            if (update_entity_state(bus_, record_address,
                                    hooks.routine_0x60FA)) {
                ++result.unresolved_0x60FA_calls;
            }
        }
    }

    if (hooks.routine_0x4658) {
        hooks.routine_0x4658(bus_);
    } else {
        const auto post_pass = run_object_output_pass(
            hooks.object_output);
        result.unresolved_0x4658_call = post_pass.unresolved_calls != 0;
    }
    bus_.write16(session_ram::kEntityPhaseState, 0);
    return result;
}

FrameWindowResult ShadowrunMainState::run_frame_window(
    const FrameWindowHooks& hooks) const {
    begin_frame_window();
    for (std::size_t iteration = 0; iteration <= 0x1EU; ++iteration) {
        if (hooks.routine_0xA8EA) {
            hooks.routine_0xA8EA(bus_);
        } else {
            update_frame_entity_records_0xA8EA(bus_, hooks.entity_update);
        }
        if (hooks.routine_0x3A1C) {
            hooks.routine_0x3A1C(bus_);
        } else {
            // The 0x3A1C prelude and its reviewed 0x3BAC camera/output tail
            // are directly typed. The two tile refresh callees retain their
            // Genesis VDP boundary inside ShadowrunRuntime.
            prepare_frame_spatial_state_0x3A1C(bus_);
            update_frame_spatial_tail_0x3BAC(bus_);
        }
        if (hooks.routine_0x4618) {
            hooks.routine_0x4618(bus_);
        } else {
            run_entity_phase(hooks);
        }
        for (const auto mask : {std::uint8_t{0x02},
                                std::uint8_t{0x04},
                                std::uint8_t{0x08}}) {
            bus_.write8(ram::kVblankFlags,
                        static_cast<std::uint8_t>(
                            bus_.read8(ram::kVblankFlags) | mask));
        }
    }

    ShadowrunRuntime runtime(bus_);
    runtime.wait_for_frame();
    bus_.write16(session_ram::kMainLoopControl, 0xFFFFU);
    if (hooks.routine_0x12F6) {
        hooks.routine_0x12F6(bus_);
    } else {
        runtime.fade_palette_to_target(hooks.frame_wait);
    }
    bus_.write8(session_ram::kFrameActive, 0);
    runtime.sample_controller_input();
    runtime.sample_controller_input();
    bus_.write16(session_ram::kFrameTransition, 0);
    bus_.write16(session_ram::kMainLoopControl, 0);
    return {0x1FU};
}

} // namespace shadowrun
