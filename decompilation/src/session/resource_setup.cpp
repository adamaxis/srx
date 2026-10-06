#include "shadowrun/session/resource_setup.hpp"

#include "shadowrun/entities/object_list.hpp"
#include "shadowrun/rendering/palette.hpp"

namespace shadowrun {
namespace {

std::uint8_t set_flag(GenesisBus& bus, std::uint8_t mask) {
    const auto flags = static_cast<std::uint8_t>(
        bus.read8(ram::kVblankFlags) | mask);
    bus.write8(ram::kVblankFlags, flags);
    return flags;
}

void perform_final_frame_wait(GenesisBus& bus,
                              const FrameWaitRoutine& frame_wait) {
    if (frame_wait) {
        frame_wait();
        return;
    }
    ShadowrunRuntime runtime(bus);
    runtime.wait_for_frame();
}

} // namespace

void queue_resource_load_0x1E2E0(GenesisBus& bus) {
    // 0x1E2E0: an already pending background load suppresses this request.
    if (bus.read16(ram::kBackgroundQueueTimer) != 0) {
        return;
    }
    bus.write16(ram::kBackgroundQueueTimer, 0x003CU);
    bus.write32(ram::kBackgroundQueuePointer,
                bus.read32(ram::kResourcePointer));
}

void queue_resource_load_0x1E2EC(GenesisBus& bus) {
    // 0x1E2EC is the unconditional entry: it shares the queue write but does
    // not perform 0x1E2E0's timer gate.
    bus.write16(ram::kBackgroundQueueTimer, 0x003CU);
    bus.write32(ram::kBackgroundQueuePointer,
                bus.read32(ram::kResourcePointer));
}

ResourceSetupResult run_resource_setup(
    GenesisBus& bus,
    GenesisAddress setup_block,
    const ResourceSetupHooks& hooks) {
    ShadowrunRuntime runtime(bus);
    const auto fade_result = runtime.fade_palette_to_black(hooks.frame_wait);

    auto cursor = setup_block;
    const auto configuration_source = bus.read32(cursor);
    cursor += sizeof(std::uint32_t);
    runtime.apply_vdp_configuration_from_bus(configuration_source);
    runtime.clear_vdp_block_a();
    runtime.clear_vdp_block_b();
    const auto palette_source = bus.read32(cursor);
    cursor += sizeof(std::uint32_t);
    initialise_object_output(bus);
    PaletteAnimator palette(bus);
    palette.copy_target_from(palette_source);

    // ROM 0x1406 advances A6 through four setup words in this order.
    bus.write16(ram::kVdpDataWord, bus.read16(cursor));
    cursor += sizeof(std::uint16_t);
    bus.write16(ram::kVdpDataWord2, bus.read16(cursor));
    cursor += sizeof(std::uint16_t);
    bus.write16(ram::kVdpRegisterWord0A, bus.read16(cursor));
    cursor += sizeof(std::uint16_t);
    bus.write16(ram::kVdpRegisterWord0E, bus.read16(cursor));
    cursor += sizeof(std::uint16_t);

    ResourceSetupResult result{};
    while (true) {
        const auto raw_source = bus.read32(cursor);
        cursor += sizeof(std::uint32_t);
        if (raw_source == 0) {
            break;
        }
        const auto d1_long = bus.read32(cursor);
        cursor += sizeof(std::uint32_t);
        const auto d2_long = bus.read32(cursor);
        cursor += sizeof(std::uint32_t);
        const PrimaryResourceStream stream{
            raw_source,
            raw_source & ~GenesisAddress{1},
            d1_long,
            d2_long,
            static_cast<std::uint16_t>(d1_long),
            kResourceStreamStaging
        };
        if ((raw_source & 1U) != 0) {
            if (hooks.routine_0x1530) {
                hooks.routine_0x1530(bus, stream);
            } else {
                runtime.decode_compressed_vdp(stream.source,
                                              stream.staging_address,
                                              stream.d0_word);
            }
        } else if (hooks.routine_0x0F5C) {
            hooks.routine_0x0F5C(bus, stream);
        } else {
            runtime.stream_primary_vdp(stream.source,
                                       stream.d1_long,
                                       stream.d2_long);
        }
        ++result.primary_streams;
    }

    while (true) {
        const auto raw_source = bus.read32(cursor);
        cursor += sizeof(std::uint32_t);
        if (raw_source == 0) {
            break;
        }
        const auto d4_long = bus.read32(cursor);
        cursor += sizeof(std::uint32_t);
        const auto d7_word = bus.read16(cursor);
        cursor += sizeof(std::uint16_t);
        const auto d1_word = bus.read16(cursor);
        cursor += sizeof(std::uint16_t);
        const auto d2_word = bus.read16(cursor);
        cursor += sizeof(std::uint16_t);
        const auto bit16_was_set = (raw_source & 0x00010000U) != 0;
        const auto source_after_bit_clear = raw_source & ~0x00010000U;
        const SecondaryResourceStream stream{
            raw_source,
            raw_source,
            source_after_bit_clear,
            d4_long,
            d7_word,
            d1_word,
            d2_word,
            0,
            static_cast<std::uint16_t>(d7_word + 1U),
            bit16_was_set
        };
        if (bit16_was_set) {
            if (hooks.routine_0x1728) {
                hooks.routine_0x1728(bus, stream);
            } else {
                runtime.stream_delta_vdp(stream.source_after_bit_clear,
                                         stream.d4_long,
                                         stream.d7_word,
                                         stream.d5_word,
                                         stream.d1_word,
                                         stream.d2_word,
                                         stream.d3_word);
            }
        } else if (hooks.routine_0x1232) {
            hooks.routine_0x1232(bus, stream);
        } else {
            runtime.stream_vdp_grid(stream.source_before_bit_clear,
                                    stream.d4_long,
                                    stream.d7_word,
                                    stream.d1_word,
                                    stream.d2_word);
        }
        ++result.secondary_streams;
    }

    // 0x14AA..0x14C0: BSET #0, #3, and #2 at FFEF14.
    auto flags = set_flag(bus, 0x01U);
    flags = set_flag(bus, 0x08U);
    flags = set_flag(bus, 0x04U);
    result.flags_after = flags;
    perform_final_frame_wait(bus, hooks.frame_wait);
    result.secondary_list_end = cursor;
    result.frame_waits = fade_result.frame_waits + 1U;
    return result;
}

} // namespace shadowrun
