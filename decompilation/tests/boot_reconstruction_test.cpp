#include "shadowrun/boot.hpp"
#include "shadowrun/compression.hpp"
#include "shadowrun/entity_projection.hpp"
#include "shadowrun/entity_state.hpp"
#include "shadowrun/object_list.hpp"
#include "shadowrun/palette.hpp"
#include "shadowrun/memory_bus.hpp"
#include "shadowrun/object_mask.hpp"
#include "shadowrun/object_update.hpp"
#include "shadowrun/rom_layout.hpp"
#include "shadowrun/resource_setup.hpp"
#include "shadowrun/runtime.hpp"
#include "shadowrun/session.hpp"
#include "shadowrun/z80_transport.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

namespace {

struct Write {
    unsigned width;
    shadowrun::GenesisAddress address;
    std::uint32_t value;
};

class TraceBus final : public shadowrun::GenesisBus {
public:
    std::uint8_t region_value = 0xC0;
    std::map<shadowrun::GenesisAddress, std::uint16_t> words;
    std::map<shadowrun::GenesisAddress, std::uint32_t> longs;
    std::vector<Write> writes;

    std::uint8_t read8(shadowrun::GenesisAddress) override { return region_value; }
    std::uint16_t read16(shadowrun::GenesisAddress address) override {
        const auto found = words.find(address);
        return found == words.end() ? 0 : found->second;
    }
    std::uint32_t read32(shadowrun::GenesisAddress address) override {
        const auto found = longs.find(address);
        return found == longs.end() ? 0 : found->second;
    }

    void write8(shadowrun::GenesisAddress address, std::uint8_t value) override {
        writes.push_back({8, address, value});
    }
    void write16(shadowrun::GenesisAddress address, std::uint16_t value) override {
        writes.push_back({16, address, value});
        words[address] = value;
    }
    void write32(shadowrun::GenesisAddress address, std::uint32_t value) override {
        writes.push_back({32, address, value});
        longs[address] = value;
    }
};

} // namespace

int main() {
    TraceBus bus;
    shadowrun::ShadowrunBoot boot(bus);

    const auto region = boot.read_region();
    assert(region.raw_io_value == 0xC0);
    assert(region.region_index == 3);

    boot.initialise_display();
    assert(bus.writes.size() == 8);
    assert(bus.writes[0].address == shadowrun::genesis::kVdpControl);
    assert(bus.writes[0].value == 0x8164);
    assert(bus.writes[1].value == 0x8230);
    assert(bus.writes[2].value == 0x8C81);
    assert(bus.writes[3].value == 0x8F02);
    assert(bus.writes[4].value == 0x9001);
    assert(bus.writes[5].width == 32 && bus.writes[5].value == 0xC0020000U);
    assert(bus.writes[6].address == shadowrun::genesis::kVdpData);
    assert(bus.writes[6].value == 0x0EEE);
    assert(bus.writes[7].value == 0x40000000U);

    bus.writes.clear();
    boot.write_startup_text(2, 1, std::string_view("AB\0ignored"));
    assert(bus.writes.size() == 3);
    assert(bus.writes[0].width == 32);
    assert(bus.writes[0].address == shadowrun::genesis::kVdpControl);
    assert(bus.writes[0].value == 0x40840003U);
    assert(bus.writes[1].width == 16 && bus.writes[1].value == 0x21);
    assert(bus.writes[2].width == 16 && bus.writes[2].value == 0x22);

    shadowrun::ShadowrunRuntime runtime(bus);
    bus.writes.clear();
    bus.words[shadowrun::ram::kRandomState] = 1;
    assert(runtime.advance_random() == 0x20E6);
    assert(bus.words[shadowrun::ram::kRandomState] == 0x20E6);
    assert(runtime.random_below(8) == 1);
    assert(bus.words[shadowrun::ram::kRandomState] == 0x2DBF);

    bus.writes.clear();
    bus.words[shadowrun::ram::kInputPort0] = 0x8100;
    bus.words[shadowrun::ram::kInputPort1] = 0x0000;
    assert(runtime.poll_input_flags() == 0x8100);
    assert(bus.words[shadowrun::ram::kInputStatus] == 0xFFFF);
    assert(bus.words[shadowrun::ram::kInputStatus2] == 0xFFFF);

    bus.writes.clear();
    bus.words[shadowrun::ram::kFrameCounter] = 9;
    bus.words[shadowrun::ram::kFrameObserved] = 1;
    bus.words[shadowrun::ram::kFrameTarget] = 2;
    runtime.wait_for_frame();
    assert(bus.words[shadowrun::ram::kFrameCounter] == 10);
    assert(bus.words[shadowrun::ram::kFrameObserved] == 2);

    bus.writes.clear();
    runtime.apply_vdp_configuration(shadowrun::rom::kVdpConfiguration);
    assert(bus.writes.size() == shadowrun::rom::kVdpConfiguration.size());
    assert(bus.writes[0].address == 0xFFDD16 && bus.writes[0].value == 0xC000);
    assert(bus.writes[1].address == 0xFFDD14 && bus.writes[1].value == 0xE000);
    assert(bus.writes[2].address == 0xFFDD10 && bus.writes[2].value == 0xD000);
    assert(bus.writes[3].address == 0xFFDD12 && bus.writes[3].value == 0x0000);
    assert(bus.writes[4].address == shadowrun::genesis::kVdpControl);
    assert(bus.writes[4].value == 0x8230);

    bus.writes.clear();
    bus.longs[shadowrun::ram::kBootstrapMagic] = 0;
    runtime.clear_or_checksum_ram();
    assert(bus.writes.size() == 0x3FE0U + 1U);
    assert(bus.writes.front().width == 32);
    assert(bus.writes.front().address == shadowrun::ram::kRamStart);
    assert(bus.writes.back().address == shadowrun::ram::kRandomState);

    bus.writes.clear();
    bus.longs[shadowrun::ram::kBootstrapMagic] = 0x12345678U;
    runtime.clear_or_checksum_ram();
    assert(bus.writes.size() == 0xF1E8U);
    assert(bus.writes.front().width == 8);
    assert(bus.writes.front().address == shadowrun::ram::kRamStart);
    assert(bus.writes.back().address == shadowrun::ram::kRamStart + 0xF1E7U);

    shadowrun::ShadowrunMainState main_state(bus);
    bus.writes.clear();
    bus.longs[shadowrun::session_ram::kBootstrapMagic] = 0;
    bus.words[shadowrun::session_ram::kRandomState] = 0x1000;
    bus.words[shadowrun::session_ram::kRandomAddend] = 0x0020;
    main_state.initialise_session();
    assert(bus.writes.size() == 5);
    assert(bus.writes[0].width == 32 &&
           bus.writes[0].address == shadowrun::session_ram::kBootstrapMagic &&
           bus.writes[0].value == 0x12345678U);
    assert(bus.writes[1].width == 16 && bus.writes[1].value == 0x1020);
    assert(bus.writes[2].address == shadowrun::session_ram::kReadyFlag0);
    assert(bus.writes[3].address == shadowrun::session_ram::kReadyFlag1);
    assert(bus.writes[4].address == shadowrun::session_ram::kMainLoopControl);

    bus.writes.clear();
    main_state.reset_session_work_fields();
    assert(bus.writes.size() == 16);
    assert(bus.writes[0].width == 32 && bus.writes[0].value == 0x14);
    assert(bus.writes[5].width == 32 && bus.writes[5].address == 0xFFFC22);
    assert(bus.writes[6].width == 16 && bus.writes[6].address == 0xFFFC26);
    assert(bus.writes[13].width == 32 && bus.writes[13].value == 1);
    assert(bus.writes[14].width == 16 && bus.writes[14].value == 2);
    assert(bus.writes[15].width == 8 && bus.writes[15].value == 2);

    bus.writes.clear();
    main_state.begin_frame_window();
    assert(bus.writes.size() == 6);
    assert(bus.writes[0].width == 16 && bus.writes[0].value == 0);
    assert(bus.writes[1].width == 8 && bus.writes[1].value == 1);
    assert(bus.writes[2].width == 16 && bus.writes[2].address == 0xFFDD22);
    assert(bus.writes[3].width == 16 && bus.writes[3].address == 0xFFDD26);
    assert(bus.writes[4].width == 16 && bus.writes[4].value == 0xFFFF);
    assert(bus.writes[5].width == 16 && bus.writes[5].value == 0xFFFF);

    const std::uint8_t literals[] = {0x00, 0x02, 0x00, 'A', 'B', 'C'};
    const auto literal_result = shadowrun::decode_packed_stream(
        literals, sizeof(literals));
    assert((literal_result.bytes == std::vector<std::uint8_t>{'A', 'B', 'C'}));
    assert(literal_result.consumed == sizeof(literals));

    const std::uint8_t repeat[] = {0x00, 0x02, 0x80, 0x00, 'X'};
    const auto repeat_result = shadowrun::decode_packed_stream(
        repeat, sizeof(repeat));
    assert((repeat_result.bytes == std::vector<std::uint8_t>{'X', 'X', 'X'}));

    // Three literals followed by a distance-three back-reference. The copy
    // loop is intentionally overlapping-capable, as is the ROM routine.
    const std::uint8_t back_reference[] = {
        0x00, 0x05, 0x10, 'A', 'B', 'C', 0xB0, 0x00
    };
    const auto back_reference_result = shadowrun::decode_packed_stream(
        back_reference, sizeof(back_reference));
    assert((back_reference_result.bytes ==
            std::vector<std::uint8_t>{'A', 'B', 'C', 'A', 'B', 'C'}));

    assert(shadowrun::PaletteAnimator::toward_black(0x0EEE) == 0x0CCC);
    assert(shadowrun::PaletteAnimator::toward_black(0x0000) == 0x0000);
    assert(shadowrun::PaletteAnimator::toward_target(0x0000, 0x0EEE) == 0x0222);

    bus.writes.clear();
    for (std::size_t index = 0; index < shadowrun::palette_ram::kEntryCount; ++index) {
        bus.words[shadowrun::palette_ram::kCurrent + index * 2] =
            index == 0 ? 0x0EEE : 0;
        bus.words[shadowrun::palette_ram::kTarget + index * 2] = 0x0EEE;
    }
    shadowrun::PaletteAnimator palette(bus);
    palette.step_toward_black();
    assert(bus.words[shadowrun::palette_ram::kCurrent] == 0x0CCC);
    assert(bus.writes.size() == shadowrun::palette_ram::kEntryCount);

    bus.writes.clear();
    palette.step_toward_target();
    assert(bus.words[shadowrun::palette_ram::kCurrent] == 0x0EEE);
    assert(bus.words[shadowrun::palette_ram::kCurrent + 2] == 0x0222);
    assert(bus.writes.size() == shadowrun::palette_ram::kEntryCount);

    bus.writes.clear();
    for (std::size_t index = 0; index < 32; ++index) {
        bus.longs[0x0200U + index * 4U] =
            static_cast<std::uint32_t>(0xA0000000U + index);
    }
    palette.copy_target_from(0x0200U);
    assert(bus.longs[shadowrun::palette_ram::kTarget] == 0xA0000000U);
    assert(bus.longs[shadowrun::palette_ram::kTarget + 0x7CU] ==
           0xA000001FU);
    assert(bus.writes.size() == 32);

    const shadowrun::RawObjectRecord objects[] = {
        {1, 2, 0x34, 3},
        {static_cast<std::uint16_t>(-2), 4, 0x56, 5},
        {0x8000, 0, 0, 0}
    };
    shadowrun::BuiltObjectRecord built_objects[3]{};
    std::uint16_t object_count = 7;
    const auto built = shadowrun::build_object_list(
        objects, 3, built_objects, 3, 20, 10, 0x0200, 0x0100, 5,
        object_count, shadowrun::ObjectListVariant::WithWord3Offset);
    assert(built == 2);
    assert(object_count == 9);
    assert(built_objects[0].word0 == 22);
    assert(built_objects[0].byte2 == 0x34 && built_objects[0].ordinal == 5);
    assert(built_objects[0].word4 == 0x0303);
    assert(built_objects[0].word6 == 11);
    assert(built_objects[1].word0 == 24);
    assert(built_objects[1].byte2 == 0x56 && built_objects[1].ordinal == 6);
    assert(built_objects[1].word4 == 0x0305);
    assert(built_objects[1].word6 == 8);

    shadowrun::BuiltObjectRecord state_objects[3]{};
    shadowrun::ObjectListBuildState list_state{11, 0};
    assert(shadowrun::build_object_list(
               objects, 3, state_objects, 3, 20, 10, 0x0200, 0x0100, 9,
               list_state, shadowrun::ObjectListVariant::Base) == 2);
    assert(list_state.object_count == 13);
    assert(list_state.next_output_index == 2);

    const auto projected = shadowrun::project_entity_field(
        {0x1800, 0x1F00, 0}, 0x1000, 0x1000);
    assert(projected.screen_x == 0x0100);
    assert(projected.screen_y == 0x0170);
    assert(projected.visible);
    assert(projected.return_value == 1);

    // A -8 word delta arithmetic-shifts to -1, then the ROM's bit-3 test
    // rounds it back toward zero. This also checks that word subtraction wraps.
    const auto rounded = shadowrun::project_entity_field(
        {0x0FF8, 0x0FF8, 0}, 0x1000, 0x1000);
    assert(rounded.screen_x == 0x0080);
    assert(rounded.screen_y == 0x0080);
    assert(rounded.visible);
    assert(rounded.return_value == 1);

    const auto clipped = shadowrun::project_entity_field(
        {0x0000, 0x1000, 0}, 0x1000, 0x1000);
    assert(clipped.screen_x == 0xFF80);
    assert(clipped.screen_y == 0);
    assert(!clipped.visible);
    assert(clipped.return_value == 0xFF80);

    const auto alternate_visible = shadowrun::project_alternate_entity_field(
        {0x1800, 0x1F00, 0}, 0x1000, 0x1000);
    assert(alternate_visible.screen_x == 0x0100);
    assert(alternate_visible.screen_y == 0x0170);
    assert(alternate_visible.visible);
    assert(alternate_visible.return_value == 0x0100);

    const auto alternate_clipped = shadowrun::project_alternate_entity_field(
        {0x0000, 0x1000, 0}, 0x1000, 0x1000);
    assert(alternate_clipped.screen_x == 0xFF80);
    assert(alternate_clipped.screen_y == 0);
    assert(!alternate_clipped.visible);
    assert(alternate_clipped.return_value == 0xFF80);

    shadowrun::RawEntitySlot entity_slots[shadowrun::kEntitySlotCount]{};
    entity_slots[0].word_00 = 0x1800;
    entity_slots[0].word_04 = 0x1F00;
    entity_slots[0].flags_10 = 1;
    entity_slots[1].word_00 = 0x1800;
    entity_slots[1].word_04 = 0x1000;
    entity_slots[1].flags_10 = 0;
    assert(shadowrun::project_active_entity_slots(
               entity_slots, shadowrun::kEntitySlotCount, 0x1000, 0x1000) == 1);
    assert(entity_slots[0].word_0A == 0x0100);
    assert(entity_slots[0].word_0C == 0x0170);
    assert(entity_slots[1].word_0A == 0);
    assert(entity_slots[1].word_0C == 0);

    assert(shadowrun::rom::kIndirectDispatchTargets.front() == 0x021326);
    assert(shadowrun::rom::kIndirectDispatchTargets.back() == 0x021992);
    assert(shadowrun::rom::kRoutineSelectTargets.size() == 22);
    assert(shadowrun::rom::kStateRoutinePairs[1][1] == 0x01EB32);
    assert(shadowrun::rom::kEntityDispatchTargets.size() == 15);
    assert(shadowrun::rom::kEntityDispatchTargets[1] == 0x00521E);
    assert(shadowrun::rom::kIndexedVariantDescriptorPointers.size() == 4);
    assert(shadowrun::rom::kIndexedVariantDescriptorPointers[3] == 0x005838);

    shadowrun::GenesisMemoryBus memory_bus({0x12, 0x34, 0x56, 0x78});
    assert(memory_bus.read32(0) == 0x12345678U);
    memory_bus.write16(0xFF0000, 0xABCD);
    assert(memory_bus.read16(0xFF0000) == 0xABCD);
    memory_bus.set_io8(shadowrun::genesis::kIoVersion, 0x80);
    assert(memory_bus.read8(shadowrun::genesis::kIoVersion) == 0x80);
    memory_bus.write32(shadowrun::genesis::kVdpControl, 0x40000003U);
    assert(memory_bus.port_writes().size() == 1);
    assert(memory_bus.port_writes()[0].width == 32);
    assert(memory_bus.port_writes()[0].address == shadowrun::genesis::kVdpControl);
    assert(memory_bus.port_writes()[0].value == 0x40000003U);

    memory_bus.write8(shadowrun::genesis::kZ80CommandIndex, 0x3EU);
    const auto gated_z80 = shadowrun::dispatch_z80_command_0x530DE(
        memory_bus, 0x35U);
    assert(!gated_z80.command_sent);
    memory_bus.write16(shadowrun::session_ram::kReadyFlag1, 1);
    const auto z80 = shadowrun::dispatch_z80_command_0x530DE(
        memory_bus, 0x35U);
    assert(z80.command_sent);
    assert(z80.index_before == 0x3EU);
    assert(z80.index_after == 1);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 0x3EU) ==
           0xFFU);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 0x3FU) ==
           0x10U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing) == 0x35U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandIndex) == 1);
    memory_bus.write16(shadowrun::session_ram::kReadyFlag1, 0);

    memory_bus.write8(shadowrun::genesis::kZ80CommandIndex, 0x3DU);
    std::size_t z80_sync_waits = 0;
    const auto z80_sync = shadowrun::synchronize_z80_0x53126(
        memory_bus, [&]() { ++z80_sync_waits; });
    assert(z80_sync.ramp_commands == 6U);
    assert(z80_sync.frame_waits == 18U);
    assert(z80_sync_waits == 18U);
    assert(z80_sync.initial_index == 0x3DU);
    assert(z80_sync.final_index == 6U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 0x3DU) ==
           0x14U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 0x3EU) ==
           0x28U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 0x3FU) ==
           0x3CU);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing) == 0x50U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 1U) ==
           0x64U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 2U) ==
           0x78U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 3U) ==
           0xFFU);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 4U) ==
           0x16U);
    assert(memory_bus.read8(shadowrun::genesis::kZ80CommandRing + 5U) == 0);

    for (std::size_t index = 0; index <= 6U; ++index) {
        const auto record = static_cast<shadowrun::GenesisAddress>(
            0xFF0400U + index * 0x100U);
        memory_bus.write8(record + 0x10U, 0);
        memory_bus.write16(record + 0x0CU, 1);
        memory_bus.write8(record + 0xB0U, 0);
        memory_bus.write8(record + 0x9EU, 0);
    }
    for (std::size_t index = 0; index <= 2U; ++index) {
        const auto record = static_cast<shadowrun::GenesisAddress>(
            0xFF0100U + index * 0x100U);
        memory_bus.write8(record + 0x10U, 0);
        memory_bus.write16(record + 0x0CU, 1);
    }
    memory_bus.write8(0xFF0400U + 0x10U, 1);
    memory_bus.write8(0xFF0500U + 0x10U, 0x11U);
    memory_bus.write8(0xFF0600U + 0x10U, 0x41U);
    memory_bus.write8(0xFF0700U + 0xB0U, 0x80U);
    memory_bus.write8(0xFF0800U + 0x9EU, 0x20U);
    memory_bus.write8(0xFF0900U + 0x9EU, 0x08U);
    memory_bus.write8(0xFF0A00U + 0x0CU, 0);
    memory_bus.write8(0xFF0100U + 0x10U, 1);
    memory_bus.write8(0xFF0200U + 0x10U, 1);
    memory_bus.write8(0xFF0300U + 0x0CU, 0);
    memory_bus.write16(shadowrun::ram::kVblankIdleGate, 0);
    memory_bus.write16(shadowrun::ram::kFrameCounter, 0x000B);
    const auto primary_frame_lists =
        shadowrun::build_frame_entity_lists_0x13874(memory_bus);
    assert(primary_frame_lists.primary_records == 1);
    assert(primary_frame_lists.secondary_records == 0);
    assert(memory_bus.read16(shadowrun::session_ram::kFrameListPrimaryCount) == 1);
    assert(memory_bus.read32(shadowrun::session_ram::kFrameListPrimary) ==
           0xFF0400U);
    memory_bus.write16(shadowrun::ram::kFrameCounter, 0x000F);
    const auto secondary_frame_lists =
        shadowrun::build_frame_entity_lists_0x13874(memory_bus);
    assert(secondary_frame_lists.primary_records == 0);
    assert(secondary_frame_lists.secondary_records == 2);
    assert(memory_bus.read16(shadowrun::session_ram::kFrameListSecondaryCount) ==
           2);
    assert(memory_bus.read32(shadowrun::session_ram::kFrameListSecondary) ==
           0xFF0100U);
    memory_bus.write16(shadowrun::ram::kVblankIdleGate, 1);
    const auto both_frame_lists =
        shadowrun::build_frame_entity_lists_0x13874(memory_bus);
    assert(both_frame_lists.primary_records == 1);
    assert(both_frame_lists.secondary_records == 2);
    memory_bus.write8(0xFF0100U + 0x10U, 0);
    memory_bus.write8(0xFF0200U + 0x10U, 0);
    for (std::size_t index = 0; index <= 6U; ++index) {
        memory_bus.write8(static_cast<shadowrun::GenesisAddress>(
                              0xFF0400U + index * 0x100U) + 0x10U, 0);
    }
    memory_bus.write16(shadowrun::ram::kVblankIdleGate, 0);

    shadowrun::GenesisMemoryBus a8ea_bus({0, 0, 0, 0});
    constexpr auto a8ea_record = shadowrun::session_ram::kEntityPhaseBase;
    a8ea_bus.write8(a8ea_record + 0x10U, 0x17U);
    a8ea_bus.write8(a8ea_record + 0x11U, 0x16U);
    a8ea_bus.write16(a8ea_record + 0xDCU, 1);
    a8ea_bus.write16(a8ea_record + 0x16U, 0x20U);
    a8ea_bus.write16(a8ea_record + 0x0EU, 1);
    a8ea_bus.write16(a8ea_record + 0x02U, 0x1200U);
    a8ea_bus.write16(a8ea_record, 0x2600U);
    a8ea_bus.write32(a8ea_record + 0x3AU, 0x12345678U);
    a8ea_bus.write16(shadowrun::session_ram::kSessionMode, 5);
    std::size_t a8ea_ab02_calls = 0;
    std::size_t a8ea_391a_calls = 0;
    std::size_t a8ea_eb92_calls = 0;
    std::size_t a8ea_handler_calls = 0;
    shadowrun::FrameEntityUpdateHooks a8ea_hooks;
    a8ea_hooks.routine_0xAB02 =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address) {
            assert(address == a8ea_record);
            ++a8ea_ab02_calls;
        };
    a8ea_hooks.routine_0x0391A =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address) {
            assert(address == a8ea_record);
            ++a8ea_391a_calls;
        };
    a8ea_hooks.routine_0xEB92 =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address) {
            assert(address == a8ea_record);
            ++a8ea_eb92_calls;
        };
    a8ea_hooks.routine_0xA8EA_handler =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address) {
            assert(address == a8ea_record);
            ++a8ea_handler_calls;
        };
    const auto a8ea_result = shadowrun::update_frame_entity_records_0xA8EA(
        a8ea_bus, a8ea_hooks);
    assert(a8ea_result.records_examined == 16);
    assert(a8ea_result.active_records == 1);
    assert(a8ea_result.timer_decrements == 1);
    assert(a8ea_result.animation_fallbacks == 0);
    assert(a8ea_result.handler_calls == 1);
    assert(a8ea_result.unresolved_calls == 0);
    assert(a8ea_ab02_calls == 1);
    assert(a8ea_391a_calls == 1);
    assert(a8ea_eb92_calls == 1);
    assert(a8ea_handler_calls == 1);
    assert(a8ea_bus.read16(a8ea_record + 0xDCU) == 0);
    assert(a8ea_bus.read8(a8ea_record + 0xB3U) == 0x20U);
    assert(a8ea_bus.read16(a8ea_record + 0x02U) == 0x11F8U);
    assert(a8ea_bus.read32(a8ea_record + 0xD4U) == 0x12345678U);
    assert(a8ea_bus.read32(a8ea_record + 0x3AU) == 0x0000B49CU);
    assert(a8ea_bus.read8(a8ea_record + 0x11U) == 0x10U);

    std::vector<std::uint8_t> collision_rom(0x16000U, 0);
    const auto write_collision_word =
        [&collision_rom](std::size_t address, std::uint16_t value) {
            collision_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            collision_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    // Equal coordinates select the zero/zero direction-table entry. The
    // direction bit keeps the reduced axes in their original order.
    write_collision_word(shadowrun::rom::kFrameDeltaDirectionTable + 0x0AU,
                         0x0040U);
    shadowrun::GenesisMemoryBus collision_bus(std::move(collision_rom));
    constexpr auto collision_record = 0xFF0100U;
    constexpr auto collision_candidate = 0xFF0200U;
    collision_bus.write16(collision_record, 0x1100U);
    collision_bus.write16(collision_record + 4U, 0x1100U);
    collision_bus.write8(collision_record + 0x10U, 0x03U);
    collision_bus.write8(collision_record + 0x9EU, 0x08U);
    collision_bus.write16(collision_record + 0x38U, 0x0020U);
    collision_bus.write16(collision_candidate, 0x1100U);
    collision_bus.write16(collision_candidate + 4U, 0x1100U);
    collision_bus.write8(collision_candidate + 0x10U, 0x01U);
    collision_bus.write16(collision_candidate + 0x38U, 0x0010U);
    collision_bus.write32(shadowrun::session_ram::kFrameEntityResourceControl,
                          0x12345678U);
    std::size_t collision_resource_calls = 0;
    shadowrun::GenesisAddress collision_resource = 0;
    shadowrun::FrameEntityUpdateHooks collision_hooks;
    collision_hooks.routine_0x1E2EC =
        [&](shadowrun::GenesisBus& hooked_bus) {
            ++collision_resource_calls;
            collision_resource =
                hooked_bus.read32(shadowrun::ram::kResourcePointer);
        };
    const auto collision_result = shadowrun::run_entity_collision_pass_0xEB92(
        collision_bus, collision_record, collision_hooks);
    assert(collision_result.candidates_examined == 9);
    assert(collision_result.collisions == 1);
    assert(collision_result.transforms == 1);
    assert(collision_result.unresolved_calls == 0);
    assert((collision_bus.read8(collision_record + 0x11U) & 0x02U) != 0);
    assert((collision_bus.read8(collision_candidate + 0x11U) & 0x02U) != 0);
    assert((collision_bus.read8(collision_record + 0xB0U) & 0x80U) != 0);
    assert(collision_bus.read32(shadowrun::session_ram::kFrameEntityGlobalReset) ==
           0x12345678U);
    assert(collision_bus.read32(shadowrun::ram::kResourcePointer) ==
           0x0001F146U);
    assert(collision_resource_calls == 1);
    assert(collision_resource == 0x0001F146U);
    assert(collision_bus.read16(collision_candidate + 0x92U) == 0xC000U);
    assert(collision_bus.read16(collision_record + 0x92U) == 0x8000U);
    assert(collision_bus.read16(collision_candidate + 0x94U) == 0x0040U);
    assert(collision_bus.read16(collision_record + 0x94U) == 0x0020U);

    auto resource_queue_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::vector<std::uint8_t>(0x16000U, 0));
    resource_queue_bus->write32(shadowrun::ram::kResourcePointer,
                                0x00021174U);
    shadowrun::queue_resource_load_0x1E2E0(*resource_queue_bus);
    assert(resource_queue_bus->read16(shadowrun::ram::kBackgroundQueueTimer) ==
           0x003CU);
    assert(resource_queue_bus->read32(
               shadowrun::ram::kBackgroundQueuePointer) == 0x00021174U);
    resource_queue_bus->write32(shadowrun::ram::kResourcePointer,
                                0x0005BAC4U);
    shadowrun::queue_resource_load_0x1E2E0(*resource_queue_bus);
    assert(resource_queue_bus->read32(
               shadowrun::ram::kBackgroundQueuePointer) == 0x00021174U);
    shadowrun::queue_resource_load_0x1E2EC(*resource_queue_bus);
    assert(resource_queue_bus->read32(
               shadowrun::ram::kBackgroundQueuePointer) == 0x0005BAC4U);

    std::vector<std::uint8_t> spawn_rom(0x200000U, 0);
    const auto write_spawn_word =
        [&spawn_rom](std::size_t address, std::uint16_t value) {
            spawn_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            spawn_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    write_spawn_word(shadowrun::rom::kEntitySpawnAnimationTableDefault,
                     0x1234U);
    write_spawn_word(shadowrun::rom::kEntitySpawnAnimationTableDefault + 2U,
                     100U);
    auto spawn_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(spawn_rom));
    constexpr auto spawn_record = 0xFF0100U;
    spawn_bus->write16(spawn_record, 0x1200U);
    spawn_bus->write16(spawn_record + 4U, 0x2300U);
    spawn_bus->write16(spawn_record + 0x0EU, 7U);
    spawn_bus->write16(spawn_record + 0x2CU, 0);
    spawn_bus->write16(shadowrun::session_ram::kSessionMode, 0);
    spawn_bus->write16(shadowrun::session_ram::kEntitySpawnGate, 0);
    const auto spawn_result = shadowrun::update_entity_spawn_slot_0x16B98(
        *spawn_bus, spawn_record);
    assert(spawn_result.allocated);
    assert(spawn_result.animation_selected);
    assert(spawn_result.slot_address == shadowrun::kEntitySlotBase);
    assert(spawn_result.selected_frame == 0x1234U);
    assert(spawn_bus->read16(shadowrun::kEntitySlotBase) == 0x1280U);
    assert(spawn_bus->read16(shadowrun::kEntitySlotBase + 4U) == 0x2380U);
    assert(spawn_bus->read16(shadowrun::kEntitySlotBase + 2U) == 0x1234U);
    assert((spawn_bus->read8(shadowrun::kEntitySlotBase + 0x10U) & 1U) != 0);

    spawn_bus->write16(shadowrun::session_ram::kFrameEntityGlobalGate, 1);
    for (std::size_t index = 0; index < 10U; ++index) {
        const auto record = static_cast<shadowrun::GenesisAddress>(
            shadowrun::session_ram::kEntityPhaseBase + index * 0x100U);
        spawn_bus->write32(record + 0xA2U, 0xFF0200U);
        spawn_bus->write32(record + 0xACU, 0xFF0300U);
        spawn_bus->write8(record + 0x43U, 1);
        spawn_bus->write8(record + 0xB0U, 0x40U);
        spawn_bus->write8(record + 0x11U, 0x10U);
    }
    shadowrun::clear_entity_relationships_0x1DBF0(*spawn_bus);
    assert(spawn_bus->read16(shadowrun::session_ram::kFrameEntityGlobalGate) ==
           0);
    assert(spawn_bus->read32(spawn_record + 0xA2U) == 0);
    assert(spawn_bus->read32(spawn_record + 0xACU) == 0);
    assert(spawn_bus->read8(spawn_record + 0x43U) == 0);
    assert(spawn_bus->read8(spawn_record + 0xB0U) == 0);
    assert(spawn_bus->read8(spawn_record + 0x11U) == 0);

    std::vector<std::uint8_t> reaction_rom(0x200000U, 0);
    const auto write_reaction_word =
        [&reaction_rom](std::size_t address, std::uint16_t value) {
            reaction_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            reaction_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    write_reaction_word(shadowrun::rom::kEntityReactionOffsetTable + 0U,
                        0x0010U);
    write_reaction_word(shadowrun::rom::kEntityReactionOffsetTable + 2U,
                        0xFFF0U);
    write_reaction_word(shadowrun::rom::kEntityReactionOffsetTable + 4U,
                        0x0020U);
    write_reaction_word(shadowrun::rom::kEntityReactionOffsetTable + 6U,
                        0xFFE0U);
    write_reaction_word(shadowrun::rom::kEntityReactionOffsetTable + 8U,
                        0x0030U);
    write_reaction_word(shadowrun::rom::kEntityReactionOffsetTable + 0xAU,
                        0xFFD0U);
    auto reaction_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(reaction_rom));
    constexpr auto reaction_record = 0xFF0100U;
    reaction_bus->write16(reaction_record, 0x1200U);
    reaction_bus->write16(reaction_record + 4U, 0x2300U);
    reaction_bus->write16(shadowrun::ram::kVblankAnimationMode, 0);
    reaction_bus->write16(shadowrun::ram::kCameraX, 0x1000U);
    reaction_bus->write16(shadowrun::ram::kCameraY, 0x2000U);
    assert(shadowrun::prepare_entity_reaction_offsets_0xBC36(
        *reaction_bus, reaction_record));
    assert(reaction_bus->read16(
               shadowrun::session_ram::kFrameEntityReactionOffsets) ==
           0x1210U);
    assert(reaction_bus->read16(
               shadowrun::session_ram::kFrameEntityReactionOffsets + 2U) ==
           0x22F0U);
    assert(reaction_bus->read16(
               shadowrun::session_ram::kFrameEntityReactionOffsets + 8U) ==
           0x1230U);
    assert(reaction_bus->read16(
               shadowrun::session_ram::kFrameEntityReactionCameraX) ==
           0x1000U);
    assert(shadowrun::calculate_entity_reaction_band_0x423C(
               *reaction_bus, 0x1080U) == 2);
    reaction_bus->write16(shadowrun::session_ram::kFrameEntityGlobalGate, 0);
    reaction_bus->write16(shadowrun::session_ram::kSessionMode, 0);
    reaction_bus->write32(shadowrun::session_ram::kFrameEntityGateSource, 0);
    shadowrun::arm_entity_global_gate_0x1DBAA(*reaction_bus);
    assert(reaction_bus->read16(shadowrun::session_ram::kFrameEntityGlobalGate) ==
           0xFFFFU);
    assert(reaction_bus->read16(shadowrun::session_ram::kFrameEntityGateTimer) ==
           0x0078U);
    reaction_bus->write32(shadowrun::session_ram::kFrameEntityGateSource,
                          1U);
    shadowrun::arm_entity_global_gate_0x1DBAA(*reaction_bus);
    assert(reaction_bus->read16(shadowrun::session_ram::kFrameEntityGateTimer) ==
           0x0258U);
    reaction_bus->write16(shadowrun::ram::kTilePlaneWidth, 0x0040U);
    reaction_bus->write16(reaction_record + 0x2EU, 0);
    assert(shadowrun::calculate_entity_reaction_tile_index_0x1669C(
               *reaction_bus, reaction_record) == 0x0412U);
    assert(shadowrun::calculate_entity_reaction_tile_index_0x166D0(
               *reaction_bus, reaction_record) == 0x0452U);
    reaction_bus->write16(shadowrun::session_ram::kEntityReactionTileCounterIndex,
                          2U);
    assert(shadowrun::mark_entity_reaction_tile_0x16760(
        *reaction_bus, reaction_record));
    const auto reaction_tile = static_cast<shadowrun::GenesisAddress>(
        shadowrun::ram::kTileMapIndexTable + 0x0452U + 0x0040U);
    assert(reaction_bus->read16(reaction_tile) == 0x000BU);
    assert(reaction_bus->read8(
               shadowrun::session_ram::kEntityReactionTileCounterBase + 2U) ==
           1U);
    assert(reaction_bus->read16(shadowrun::ram::kBackgroundQueueBusy) == 1U);
    assert(!shadowrun::mark_entity_reaction_tile_0x16760(
        *reaction_bus, reaction_record));

    std::vector<std::uint8_t> entity_reaction_rom(0x200000U, 0);
    auto entity_reaction_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(entity_reaction_rom));
    constexpr auto entity_reaction_record = 0xFF0100U;
    entity_reaction_bus->write8(entity_reaction_record + 0x10U, 0x02U);
    entity_reaction_bus->write8(entity_reaction_record + 0xB0U, 0);
    entity_reaction_bus->write16(shadowrun::ram::kVblankAnimationMode, 2U);
    entity_reaction_bus->write8(
        shadowrun::session_ram::kEntityReactionInputFlags, 0x80U);
    entity_reaction_bus->write8(
        shadowrun::session_ram::kEntityReactionInputFlags + 1U, 0x80U);
    entity_reaction_bus->write16(
        shadowrun::session_ram::kFrameEntityCollisionState, 0x001FU);
    const shadowrun::EntityReactionRegisterContext reaction_registers{
        0, 0x1234U, 0, 0x0200U, 0xFF4700U};
    const auto direct_reaction = shadowrun::run_entity_reaction_0xB7B8(
        *entity_reaction_bus, entity_reaction_record, reaction_registers);
    assert(direct_reaction.entered);
    assert(!direct_reaction.returned_with_gate);
    assert(direct_reaction.resource_loads == 1U);
    assert(direct_reaction.unresolved_calls == 0U);
    assert(entity_reaction_bus->read32(shadowrun::ram::kResourcePointer) ==
           0x0000DB26U);
    assert(entity_reaction_bus->read32(shadowrun::ram::kBackgroundQueuePointer) ==
           0x0000DB26U);
    assert(entity_reaction_bus->read16(shadowrun::ram::kBackgroundQueueTimer) ==
           0x003CU);
    assert(entity_reaction_bus->read16(
               entity_reaction_record + 0xC6U) == 0x001FU);
    assert(entity_reaction_bus->read16(
               entity_reaction_record + 0xCCU) == 0x0000U);
    assert((entity_reaction_bus->read8(entity_reaction_record + 0xB0U) &
            0x21U) == 0x01U);

    entity_reaction_bus->write8(entity_reaction_record + 0xB0U, 0x20U);
    const auto reentry_reaction = shadowrun::run_entity_reaction_0xB7B8(
        *entity_reaction_bus, entity_reaction_record, reaction_registers);
    assert(!reentry_reaction.entered);
    assert(reentry_reaction.resource_loads == 0U);
    assert(entity_reaction_bus->read8(entity_reaction_record + 0xB0U) ==
           0x20U);

    std::vector<std::uint8_t> transition_reaction_rom(0x200000U, 0);
    auto transition_reaction_bus =
        std::make_unique<shadowrun::GenesisMemoryBus>(
            std::move(transition_reaction_rom));
    constexpr auto transition_reaction_record = 0xFF0200U;
    transition_reaction_bus->write8(
        transition_reaction_record + 0x10U, 0x02U);
    transition_reaction_bus->write16(
        shadowrun::ram::kVblankAnimationMode, 2U);
    transition_reaction_bus->write8(
        shadowrun::session_ram::kEntityReactionInputFlags, 0x80U);
    transition_reaction_bus->write8(
        shadowrun::session_ram::kEntityReactionInputFlags + 1U, 0x80U);
    transition_reaction_bus->write16(
        shadowrun::session_ram::kFrameEntityCollisionState, 0);
    std::size_t map_boundary_calls = 0;
    shadowrun::EntityReactionHooks reaction_hooks;
    reaction_hooks.routine_0x1651E =
        [&](shadowrun::GenesisBus& bus, shadowrun::GenesisAddress address,
            const shadowrun::EntityReactionRegisterContext& nested) {
            assert(address == transition_reaction_record);
            assert(nested.d0 == 0U);
            assert(nested.d4 == 0U);
            assert(nested.auxiliary_base == 0xFF4700U);
            ++map_boundary_calls;
            return true;
        };
    const auto transition_reaction = shadowrun::run_entity_reaction_0xB7B8(
        *transition_reaction_bus, transition_reaction_record,
        reaction_registers, reaction_hooks);
    assert(transition_reaction.entered);
    assert(transition_reaction.returned_with_gate);
    assert(transition_reaction.resource_loads == 0U);
    assert(transition_reaction.unresolved_calls == 0U);
    assert(map_boundary_calls == 1U);
    assert(transition_reaction_bus->read16(
               shadowrun::session_ram::kEntityReactionTransitionState) ==
           0xFFFFU);
    assert(transition_reaction_bus->read16(
               shadowrun::session_ram::kEntityReactionTransitionStep) ==
           0x0010U);
    assert(transition_reaction_bus->read16(
               shadowrun::session_ram::kMainLoopControl) == 0xFFFFU);
    assert((transition_reaction_bus->read8(
                transition_reaction_record + 0xB0U) & 0x20U) != 0);

    transition_reaction_bus->write16(
        shadowrun::session_ram::kMainLoopControl, 0);
    transition_reaction_bus->write8(transition_reaction_record + 0xB0U, 0);
    const auto direct_transition_reaction =
        shadowrun::run_entity_reaction_0xB7B8(
            *transition_reaction_bus, transition_reaction_record,
            reaction_registers);
    assert(direct_transition_reaction.entered);
    assert(direct_transition_reaction.returned_with_gate);
    assert(direct_transition_reaction.unresolved_calls == 0U);

    transition_reaction_bus->write16(shadowrun::session_ram::kMainLoopControl,
                                     0);
    transition_reaction_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x10U, 0x40U);
    transition_reaction_bus->write8(transition_reaction_record + 0xB0U, 0);
    transition_reaction_bus->write8(
        shadowrun::genesis::kZ80CommandIndex, 0x3DU);
    std::size_t reaction_sync_waits = 0;
    shadowrun::EntityReactionHooks direct_sync_hooks;
    direct_sync_hooks.frame_wait = [&]() { ++reaction_sync_waits; };
    const auto direct_sync_reaction = shadowrun::run_entity_reaction_0xB7B8(
        *transition_reaction_bus, transition_reaction_record,
        reaction_registers, direct_sync_hooks);
    assert(direct_sync_reaction.entered);
    assert(!direct_sync_reaction.returned_with_gate);
    assert(direct_sync_reaction.resource_loads == 1U);
    assert(direct_sync_reaction.unresolved_calls == 0U);
    assert(reaction_sync_waits == 18U);
    assert(transition_reaction_bus->read8(
               shadowrun::genesis::kZ80CommandRing + 0x3DU) == 0x14U);
    assert(transition_reaction_bus->read8(
               shadowrun::genesis::kZ80CommandRing + 5U) == 0);
    assert(transition_reaction_bus->read8(
               shadowrun::genesis::kZ80CommandIndex) == 6U);

    std::vector<std::uint8_t> terrain_rom(0x200000U, 0);
    auto terrain_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(terrain_rom));
    constexpr auto terrain_record = 0xFF0500U;
    terrain_bus->write16(shadowrun::session_ram::kEntityTerrainBoundsXLower,
                         0x0010U);
    terrain_bus->write16(shadowrun::session_ram::kEntityTerrainBoundsXUpper,
                         0x0200U);
    terrain_bus->write16(shadowrun::session_ram::kEntityTerrainBoundsYLower,
                         0x0010U);
    terrain_bus->write16(shadowrun::session_ram::kEntityTerrainBoundsYUpper,
                         0x0200U);
    terrain_bus->write8(terrain_record + 0x10U, 0);
    constexpr auto terrain_simple_offset = 0x030CU;
    terrain_bus->write8(shadowrun::ram::kIndexedObjectByteTable +
                            terrain_simple_offset,
                        0x11U);
    const auto terrain_simple = shadowrun::sample_entity_terrain_0xE7C8(
        *terrain_bus, terrain_record, 0x1800U, 0x1800U);
    assert(terrain_simple.status == 0U);
    assert(!terrain_simple.out_of_bounds);
    assert(terrain_bus->read16(
               shadowrun::session_ram::kEntityTerrainSelectedOffset) ==
           terrain_simple_offset);
    assert(terrain_bus->read16(
               shadowrun::session_ram::kFrameEntityCollisionState) ==
           0x0011U);
    assert(terrain_bus->read8(
               shadowrun::session_ram::kEntityReactionInputFlags) == 0x11U);

    terrain_bus->write32(shadowrun::session_ram::kEntityTerrainMapRecords,
                         0xFF3000U);
    terrain_bus->write8(shadowrun::ram::kIndexedObjectByteTable +
                            terrain_simple_offset,
                        1U);
    terrain_bus->write32(0xFF3000U + 10U, 0x01800180U);
    terrain_bus->write32(0xFF3000U + 14U, 0x01800180U);
    const auto terrain_rectangle = shadowrun::sample_entity_terrain_0xE7C8(
        *terrain_bus, terrain_record, 0x1800U, 0x1800U);
    assert(terrain_rectangle.status == 0x007FU);
    assert(!terrain_rectangle.out_of_bounds);
    assert(terrain_bus->read16(
               shadowrun::session_ram::kFrameEntityCollisionState) == 1U);

    terrain_bus->write8(terrain_record + 0x10U, 0x04U);
    terrain_bus->write8(shadowrun::ram::kIndexedObjectByteTable + 0x02CBU,
                        0x60U);
    terrain_bus->write8(shadowrun::ram::kIndexedObjectByteTable + 0x02CCU,
                        0);
    terrain_bus->write8(shadowrun::ram::kIndexedObjectByteTable + 0x030BU,
                        0);
    terrain_bus->write8(shadowrun::ram::kIndexedObjectByteTable + 0x030CU,
                        0);
    const auto terrain_directional = shadowrun::sample_entity_terrain_0xE7C8(
        *terrain_bus, terrain_record, 0x1800U, 0x1800U);
    assert(terrain_directional.status == 0x007FU);
    assert(terrain_bus->read16(
               shadowrun::session_ram::kEntityTerrainCollisionGate) ==
           0xFFFFU);

    const auto terrain_out_of_bounds =
        shadowrun::sample_entity_terrain_0xE7C8(
            *terrain_bus, terrain_record, 0x2000U, 0x1800U);
    assert(terrain_out_of_bounds.status == 0x007FU);
    assert(terrain_out_of_bounds.out_of_bounds);
    assert(terrain_bus->read16(
               shadowrun::session_ram::kEntityTerrainOutOfBounds) ==
           0xFFFFU);

    terrain_bus->write16(terrain_record, 0x1800U);
    terrain_bus->write16(terrain_record + 4U, 0x1800U);
    terrain_bus->write16(terrain_record + 0x2EU, 0);
    terrain_bus->write8(terrain_record + 0x10U, 0);
    terrain_bus->write8(terrain_record + 0xB0U, 1U);
    const auto direct_movement = shadowrun::run_entity_movement_0xBABC(
        *terrain_bus, terrain_record,
        shadowrun::EntityReactionRegisterContext{0, 0, 0, 0, 0});
    assert(direct_movement.completed);
    assert(direct_movement.entity_deactivated);
    assert(!direct_movement.direction_failed);
    assert(direct_movement.unresolved_calls == 0U);

    std::vector<std::uint8_t> movement_rom(0x200000U, 0);
    const auto write_movement_word =
        [&movement_rom](std::size_t address, std::uint16_t value) {
            movement_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            movement_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    write_movement_word(shadowrun::rom::kEntityMovementDirectionTable,
                        0x0010U);
    write_movement_word(shadowrun::rom::kEntityMovementDirectionTable + 2U,
                        0x0020U);
    write_movement_word(shadowrun::rom::kEntityMovementDirectionTable + 4U,
                        0x0030U);
    write_movement_word(shadowrun::rom::kEntityMovementDirectionTable + 6U,
                        0x0040U);
    auto movement_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(movement_rom));
    constexpr auto movement_record = 0xFF0100U;
    movement_bus->write16(movement_record, 0x1000U);
    movement_bus->write16(movement_record + 4U, 0x2000U);
    movement_bus->write16(movement_record + 0x2EU, 0);
    movement_bus->write8(movement_record + 0xB0U, 1U);
    std::size_t movement_probe_calls = 0;
    shadowrun::EntityMovementHooks movement_hooks;
    movement_hooks.routine_0xE7C8 =
        [&](shadowrun::GenesisBus& bus, shadowrun::GenesisAddress address,
            std::uint16_t d5, std::uint16_t d6) {
            assert(address == movement_record);
            assert(d5 == 0x1010U);
            assert(d6 == 0x2020U);
            bus.write32(shadowrun::session_ram::kEntityReactionInputFlags,
                        0x12345678U);
            ++movement_probe_calls;
            return static_cast<std::uint16_t>(0);
        };
    const shadowrun::EntityReactionRegisterContext movement_registers{
        0, 0x9999U, 0, 0, 0};
    const auto stopped_movement = shadowrun::run_entity_movement_0xBABC(
        *movement_bus, movement_record, movement_registers, movement_hooks);
    assert(stopped_movement.entered);
    assert(stopped_movement.completed);
    assert(stopped_movement.entity_deactivated);
    assert(!stopped_movement.direction_failed);
    assert(!stopped_movement.reaction_called);
    assert(stopped_movement.initial_collision_state == 0U);
    assert(stopped_movement.final_collision_state == 0U);
    assert(stopped_movement.unresolved_calls == 0U);
    assert(movement_probe_calls == 1U);
    assert(movement_bus->read16(movement_record + 8U) == 0xFFF4U);
    assert(movement_bus->read16(movement_record + 0xCAU) == 0x0030U);
    assert(movement_bus->read16(movement_record) == 0x1010U);
    assert(movement_bus->read16(movement_record + 4U) == 0x2020U);
    assert((movement_bus->read8(movement_record + 0xB0U) & 1U) == 0);
    assert(movement_bus->read32(movement_record + 0xD0U) == 0x12345678U);

    std::vector<std::uint8_t> alternate_movement_rom(0x200000U, 0);
    const auto write_alternate_word =
        [&alternate_movement_rom](std::size_t address, std::uint16_t value) {
            alternate_movement_rom[address] =
                static_cast<std::uint8_t>(value >> 8U);
            alternate_movement_rom[address + 1U] =
                static_cast<std::uint8_t>(value);
        };
    write_alternate_word(shadowrun::rom::kEntityMovementDirectionTable,
                         0x0010U);
    write_alternate_word(shadowrun::rom::kEntityMovementDirectionTable + 2U,
                         0x0020U);
    write_alternate_word(shadowrun::rom::kEntityMovementDirectionTable + 4U,
                         0x0030U);
    write_alternate_word(shadowrun::rom::kEntityMovementDirectionTable + 6U,
                         0x0040U);
    auto alternate_movement_bus =
        std::make_unique<shadowrun::GenesisMemoryBus>(
            std::move(alternate_movement_rom));
    constexpr auto alternate_movement_record = 0xFF0200U;
    alternate_movement_bus->write16(alternate_movement_record, 0x1000U);
    alternate_movement_bus->write16(alternate_movement_record + 4U,
                                    0x2000U);
    alternate_movement_bus->write16(alternate_movement_record + 0x2EU,
                                    0x2000U);
    alternate_movement_bus->write16(alternate_movement_record + 0x34U,
                                    0x3000U);
    alternate_movement_bus->write8(alternate_movement_record + 0xB0U, 1U);
    std::size_t alternate_probe_calls = 0;
    shadowrun::EntityMovementHooks alternate_movement_hooks;
    alternate_movement_hooks.routine_0xE7C8 =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address,
            std::uint16_t, std::uint16_t) {
            assert(address == alternate_movement_record);
            ++alternate_probe_calls;
            return static_cast<std::uint16_t>(
                alternate_probe_calls <= 2U ? 1U : 0U);
        };
    const auto alternate_movement = shadowrun::run_entity_movement_0xBABC(
        *alternate_movement_bus, alternate_movement_record,
        movement_registers, alternate_movement_hooks);
    assert(alternate_movement.entered);
    assert(alternate_movement.completed);
    assert(alternate_movement.entity_deactivated);
    assert(!alternate_movement.direction_failed);
    assert(alternate_probe_calls == 3U);
    assert(alternate_movement_bus->read16(
               alternate_movement_record + 0xB8U) == 2U);
    assert(alternate_movement_bus->read16(
               alternate_movement_record + 0x36U) == 0x4000U);
    assert((alternate_movement_bus->read8(
                alternate_movement_record + 0x11U) & 0x04U) != 0);
    assert(alternate_movement_bus->read16(alternate_movement_record) ==
           0x1030U);
    assert(alternate_movement_bus->read16(alternate_movement_record + 4U) ==
           0x2040U);

    std::vector<std::uint8_t> pointer_movement_rom(0x200000U, 0);
    const auto write_pointer_word =
        [&pointer_movement_rom](std::size_t address, std::uint16_t value) {
            pointer_movement_rom[address] =
                static_cast<std::uint8_t>(value >> 8U);
            pointer_movement_rom[address + 1U] =
                static_cast<std::uint8_t>(value);
        };
    const auto write_pointer_long =
        [&pointer_movement_rom](std::size_t address, std::uint32_t value) {
            pointer_movement_rom[address] =
                static_cast<std::uint8_t>(value >> 24U);
            pointer_movement_rom[address + 1U] =
                static_cast<std::uint8_t>(value >> 16U);
            pointer_movement_rom[address + 2U] =
                static_cast<std::uint8_t>(value >> 8U);
            pointer_movement_rom[address + 3U] =
                static_cast<std::uint8_t>(value);
        };
    write_pointer_word(shadowrun::rom::kEntityMovementDirectionTable,
                       0x0010U);
    write_pointer_word(shadowrun::rom::kEntityMovementDirectionTable + 2U,
                       0x0020U);
    write_pointer_long(shadowrun::rom::kEntityMovementSelectionPointerTable,
                       0x00001000U);
    write_pointer_long(0x1000U, 0x00001010U);
    write_pointer_long(0x1004U, 0x00001020U);
    write_pointer_word(0x1008U, 0);
    auto pointer_movement_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(pointer_movement_rom));
    constexpr auto pointer_movement_record = 0xFF0300U;
    pointer_movement_bus->write16(pointer_movement_record, 0x1000U);
    pointer_movement_bus->write16(pointer_movement_record + 4U, 0x2000U);
    pointer_movement_bus->write16(pointer_movement_record + 0x2EU, 0);
    pointer_movement_bus->write8(pointer_movement_record + 0xB0U, 1U);
    std::size_t pointer_probe_calls = 0;
    shadowrun::EntityMovementHooks pointer_movement_hooks;
    pointer_movement_hooks.routine_0xE7C8 =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address,
            std::uint16_t, std::uint16_t) {
            assert(address == pointer_movement_record);
            ++pointer_probe_calls;
            return static_cast<std::uint16_t>(
                pointer_probe_calls == 1U ? 1U : 0U);
        };
    const auto pointer_movement = shadowrun::run_entity_movement_0xBABC(
        *pointer_movement_bus, pointer_movement_record, movement_registers,
        pointer_movement_hooks);
    assert(pointer_movement.entered);
    assert(pointer_movement.completed);
    assert(pointer_movement.entity_deactivated);
    assert(!pointer_movement.direction_failed);
    assert(pointer_probe_calls == 2U);
    assert(pointer_movement_bus->read16(pointer_movement_record + 0x36U) ==
           0U);

    std::vector<std::uint8_t> animation_rom(0x16000U, 0);
    auto animation_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(animation_rom));
    constexpr auto animation391a_record = 0xFF0100U;
    constexpr auto animation391a_next = 0xFF0200U;
    animation_bus->write32(shadowrun::ram::kEntityObjectControlPointer,
                           animation391a_record);
    animation_bus->write8(animation391a_record + 0x10U, 0x01U);
    animation_bus->write8(animation391a_next + 0x10U, 0x01U);
    std::size_t animation_relationship_calls = 0;
    shadowrun::FrameEntityUpdateHooks animation_hooks;
    animation_hooks.routine_0x1DC4C =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address) {
            assert(address == animation391a_next);
            ++animation_relationship_calls;
        };
    const auto animation_result = shadowrun::run_entity_animation_pass_0x391A(
        *animation_bus, animation_hooks);
    assert(animation_result.records_examined == 2);
    assert(!animation_result.command_dispatched);
    assert(animation_result.selected_record_changed);
    assert(animation_result.reset_written);
    assert(animation_result.unresolved_calls == 0);
    assert(animation_relationship_calls == 1);
    assert(animation_bus->read32(shadowrun::ram::kEntityObjectControlPointer) ==
           animation391a_next);
    assert(animation_bus->read8(shadowrun::session_ram::kEntityAnimationState) ==
           0);
    assert(animation_bus->read16(
               shadowrun::session_ram::kEntityAnimationResetWord) == 0xFFFFU);
    assert(animation_bus->read16(
               shadowrun::session_ram::kEntityAnimationResetCounter) == 3U);
    assert((animation_bus->read8(animation391a_next + 0x10U) & 0x03U) ==
           0x03U);

    auto relationship_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::vector<std::uint8_t>(0x100U, 0));
    constexpr auto relationship_entity = 0xFF0100U;
    constexpr auto relationship_candidate = 0xFF0400U;
    relationship_bus->write8(relationship_candidate + 0x10U, 1);
    relationship_bus->write16(relationship_candidate + 0x0CU, 1);
    const auto relationship_result =
        shadowrun::update_entity_relationship_0x1DC4C(
            *relationship_bus, relationship_entity, 0);
    assert(relationship_result.candidates_examined == 1);
    assert(!relationship_result.special_record_path);
    assert(!relationship_result.link_cleared);
    assert(relationship_result.relation_established);
    assert(relationship_result.selected_record == relationship_candidate);
    assert(relationship_bus->read32(relationship_entity + 0xA2U) ==
           relationship_candidate);
    assert(relationship_bus->read32(relationship_candidate + 0xACU) ==
           relationship_entity);
    assert(relationship_bus->read16(relationship_candidate + 0xA6U) ==
           0x8009U);
    assert((relationship_bus->read8(relationship_candidate + 0x11U) & 0x10U) !=
           0);

    auto animation_entry_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::vector<std::uint8_t>(0x200000U, 0));
    constexpr auto animation_entry_record = 0xFF0100U;
    std::vector<shadowrun::GenesisAddress> animation_targets;
    const shadowrun::EntityAnimationContinuationRoutine animation_continuation =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address,
            shadowrun::GenesisAddress target) {
            assert(address == animation_entry_record);
            animation_targets.push_back(target);
        };

    // 0x138B8 decrements its own word timer and returns before touching the
    // shared gate when the result is still nonnegative.
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 1U);
    const auto animation_wait = shadowrun::run_entity_animation_entry_0x138B8(
        *animation_entry_bus, animation_entry_record, animation_continuation);
    assert(!animation_wait.timer_expired);
    assert(animation_entry_bus->read16(animation_entry_record + 0xB6U) == 0);
    assert(animation_targets.empty());

    // An expired 0x138B8 timer arms FFE175/B6 and enters 0x14508 when the
    // record's +0x4D byte is clear.
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    const auto animation_14508 =
        shadowrun::run_entity_animation_entry_0x138B8(
            *animation_entry_bus, animation_entry_record,
            animation_continuation);
    assert(animation_14508.timer_expired);
    assert(animation_14508.timer_rearmed);
    assert(animation_targets.size() == 1);
    assert(animation_targets[0] == 0x00014508U);
    assert(animation_entry_bus->read8(shadowrun::session_ram::kFrameListBuildMode) ==
           0xFFU);

    // With the global gate set, the alternate entry calls 0x13A92 only for a
    // negative +0xDC word, then falls through to the shared 0x139D8 body.
    animation_targets.clear();
    animation_entry_bus->write8(
        shadowrun::session_ram::kFrameListBuildMode, 0);
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    animation_entry_bus->write8(animation_entry_record + 0x4DU, 1U);
    animation_entry_bus->write16(
        shadowrun::session_ram::kFrameEntityGlobalGate, 1U);
    animation_entry_bus->write16(animation_entry_record + 0xDCU, 0xFFFFU);
    const auto animation_common =
        shadowrun::run_entity_animation_entry_0x138B8(
            *animation_entry_bus, animation_entry_record,
            animation_continuation);
    assert(animation_common.continuation_calls == 2);
    assert(animation_targets.size() == 2);
    assert(animation_targets[0] == 0x00013A92U);
    assert(animation_targets[1] == 0x000139D8U);

    // 0x13904's type-0x0F branch is direct through its small 0x13A4C timer
    // sequence; the remaining resource calls retain their ROM targets.
    animation_targets.clear();
    animation_entry_bus->write8(
        shadowrun::session_ram::kFrameListBuildMode, 0);
    animation_entry_bus->write16(
        shadowrun::session_ram::kFrameEntityGlobalGate, 0);
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0EU, 0x000FU);
    animation_entry_bus->write16(animation_entry_record + 0x16U, 0x0030U);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityAnimationDispatchTimer, 0);
    const auto animation_type_branch =
        shadowrun::run_entity_animation_entry_0x13904(
            *animation_entry_bus, animation_entry_record,
            animation_continuation);
    assert(animation_type_branch.timer_rearmed);
    assert(animation_entry_bus->read8(
               shadowrun::session_ram::kEntityAnimationDispatchTimer) == 8U);
    assert(animation_targets.size() == 3);
    assert(animation_targets[0] == 0x00014492U);
    assert(animation_targets[1] == 0x0000AE6EU);
    assert(animation_targets[2] == 0x0000AF7CU);

    // The shared 0x13E8C branch now directly performs its one-shot BSET and
    // camera-relative coordinate initialization before continuing at 145A2.
    animation_targets.clear();
    animation_entry_bus->write8(
        shadowrun::session_ram::kFrameListBuildMode, 0);
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0EU, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0CU, 1U);
    animation_entry_bus->write8(animation_entry_record + 0x9EU, 0x28U);
    animation_entry_bus->write8(animation_entry_record + 0xB0U, 0);
    animation_entry_bus->write16(
        shadowrun::session_ram::kFrameEntityGlobalGate, 1U);
    animation_entry_bus->write16(shadowrun::ram::kCameraX, 0x1000U);
    animation_entry_bus->write16(shadowrun::ram::kCameraY, 0x0800U);
    animation_entry_bus->write16(
        shadowrun::ram::kCameraXUpperLimit, 0x4000U);
    animation_entry_bus->write16(
        shadowrun::ram::kCameraYUpperLimit, 0x3000U);
    const auto animation_position_branch =
        shadowrun::run_entity_animation_entry_0x13904(
            *animation_entry_bus, animation_entry_record,
            animation_continuation);
    assert(animation_position_branch.continuation_calls == 1);
    assert(animation_targets.size() == 1);
    assert(animation_targets[0] == 0x000145A2U);
    assert((animation_entry_bus->read8(animation_entry_record + 0xB0U) &
            0x40U) != 0);
    assert(animation_entry_bus->read16(animation_entry_record + 0x96U) ==
           0x3F80U);
    assert(animation_entry_bus->read16(animation_entry_record + 0x98U) ==
           0x3780U);

    // The shared 0x139BE tail uses BEQ after TST FFDF84: a zero global gate
    // enters the direct 0x13EF4 body, which selects 0x142AE for type <= 2
    // and returns through that branch without reaching 0x139D8.
    animation_targets.clear();
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0EU, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0CU, 1U);
    animation_entry_bus->write8(animation_entry_record + 0x9EU, 0);
    animation_entry_bus->write8(animation_entry_record + 0xB0U, 0);
    animation_entry_bus->write16(
        shadowrun::session_ram::kFrameEntityGlobalGate, 0);
    const auto animation_zero_gate =
        shadowrun::run_entity_animation_entry_0x13904(
            *animation_entry_bus, animation_entry_record,
            animation_continuation);
    assert(animation_zero_gate.continuation_calls == 1);
    assert(animation_targets.size() == 1);
    assert(animation_targets[0] == 0x000142AEU);

    // Type four uses the direct 0x15CA4 distance test and then the known
    // 0x1DBAA leaf when the control record is nearby.
    animation_targets.clear();
    constexpr auto animation_reference_record = 0xFF0400U;
    animation_entry_bus->write32(
        shadowrun::ram::kEntityObjectControlPointer,
        animation_reference_record);
    animation_entry_bus->write16(animation_entry_record, 0x0100U);
    animation_entry_bus->write16(animation_entry_record + 4U, 0x0200U);
    animation_entry_bus->write16(animation_reference_record, 0x0100U);
    animation_entry_bus->write16(animation_reference_record + 4U, 0x0200U);
    animation_entry_bus->write8(animation_entry_record + 0x4DU, 4U);
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    animation_entry_bus->write8(animation_entry_record + 0x9EU, 0);
    animation_entry_bus->write8(animation_entry_record + 0xB0U, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0CU, 1U);
    animation_entry_bus->write16(
        shadowrun::session_ram::kFrameEntityGlobalGate, 0);
    const auto animation_type_four =
        shadowrun::run_entity_animation_entry_0x13904(
            *animation_entry_bus, animation_entry_record,
            animation_continuation);
    assert(animation_type_four.continuation_calls == 2);
    assert(animation_targets.size() == 2);
    assert(animation_targets[0] == 0x0001DBAAU);
    assert(animation_targets[1] == 0x000139D8U);

    // Type three uses the direct 0x15D3C path and installs B45C/+0x2E when
    // its reduced distance and frame cutoff both pass; the ROM returns from
    // this tail rather than entering the shared 139D8 body.
    animation_targets.clear();
    animation_entry_bus->write8(animation_entry_record + 0x4DU, 3U);
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    animation_entry_bus->write16(animation_entry_record + 0x16U, 0x0020U);
    animation_entry_bus->write16(animation_entry_record + 0x2EU, 0);
    const auto animation_type_three =
        shadowrun::run_entity_animation_entry_0x13904(
            *animation_entry_bus, animation_entry_record,
            animation_continuation);
    assert(animation_type_three.continuation_calls == 0);
    assert(animation_targets.empty());
    assert(animation_entry_bus->read32(animation_entry_record + 0x3AU) ==
           0x0000B45CU);

    // The same 13E40 leaf executes directly when the animation continuation
    // hook is absent.
    animation_entry_bus->write16(animation_entry_record + 0xB6U, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0EU, 0);
    animation_entry_bus->write16(animation_entry_record + 0x0CU, 0);
    animation_entry_bus->write8(animation_entry_record + 0x9EU, 0x28U);
    animation_entry_bus->write8(animation_entry_record + 0xB0U, 0);
    animation_entry_bus->write16(
        shadowrun::session_ram::kFrameEntityGlobalGate, 1U);
    animation_entry_bus->write16(animation_entry_record + 0x16U, 0x0020U);
    animation_entry_bus->write32(animation_entry_record + 0x3AU, 0);
    const auto animation_direct_terminal =
        shadowrun::run_entity_animation_entry_0x13904(
            *animation_entry_bus, animation_entry_record);
    assert(animation_direct_terminal.continuation_calls == 1);
    assert(animation_direct_terminal.unresolved_calls == 0);
    assert(animation_entry_bus->read32(animation_entry_record + 0x3AU) ==
           0x0000B45CU);

    // The adjacent animation leaves preserve their original byte/count
    // widths: 13A40 copies coordinates, 13AF2 returns the first phase below
    // thirteen, and 13C20 selects a phase only on its carry path.
    constexpr auto animation_source_record = 0xFF0800U;
    animation_entry_bus->write16(animation_source_record, 0x1111U);
    animation_entry_bus->write16(animation_source_record + 4U, 0x2222U);
    shadowrun::copy_entity_animation_position_0x13A40(
        *animation_entry_bus, animation_entry_record, animation_source_record);
    assert(animation_entry_bus->read16(animation_entry_record + 0x96U) ==
           0x1111U);
    assert(animation_entry_bus->read16(animation_entry_record + 0x98U) ==
           0x2222U);
    animation_entry_bus->write8(animation_entry_record + 0x66U, 0x0EU);
    animation_entry_bus->write8(animation_entry_record + 0x67U, 0x0CU);
    assert(shadowrun::scan_entity_animation_phase_0x13AF2(
               *animation_entry_bus, animation_entry_record) == 0x0CU);
    for (std::size_t index = 0; index < 8U; ++index) {
        animation_entry_bus->write8(
            animation_entry_record + 0x66U + index, 0x0DU);
    }
    assert(shadowrun::scan_entity_animation_phase_0x13AF2(
               *animation_entry_bus, animation_entry_record) == 0);

    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x4CU, 3U);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x14CU, 4U);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x24CU, 4U);
    animation_entry_bus->write8(0x00FFFC0CU, 0);
    animation_entry_bus->write8(0x00FFFC0DU, 0);
    auto phase_selection =
        shadowrun::select_entity_animation_phase_0x13C20(*animation_entry_bus);
    assert(phase_selection.selected_phase == 0);
    assert(phase_selection.selected);
    assert(animation_entry_bus->read8(0x00FFE190U) == 0);

    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x4CU, 4U);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x14CU, 3U);
    phase_selection = shadowrun::select_entity_animation_phase_0x13C20(
        *animation_entry_bus);
    assert(phase_selection.selected_phase == 1);
    assert(phase_selection.selected);
    assert(animation_entry_bus->read8(0x00FFE190U) == 1);

    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x14CU, 4U);
    animation_entry_bus->write8(0x00FFFC0DU, 0xFFU);
    phase_selection = shadowrun::select_entity_animation_phase_0x13C20(
        *animation_entry_bus);
    assert(phase_selection.selected_phase == 1);
    assert(!phase_selection.selected);
    assert(animation_entry_bus->read8(0x00FFE190U) == 1);

    animation_entry_bus->write8(0x00FFFC0DU, 0);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x14CU, 4U);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x24CU, 3U);
    phase_selection = shadowrun::select_entity_animation_phase_0x13C20(
        *animation_entry_bus);
    assert(phase_selection.selected_phase == 2);
    assert(phase_selection.selected);
    assert(animation_entry_bus->read8(0x00FFE190U) == 2);

    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x24CU, 4U);
    phase_selection = shadowrun::select_entity_animation_phase_0x13C20(
        *animation_entry_bus);
    assert(phase_selection.selected_phase == 2);
    assert(!phase_selection.selected);
    assert(animation_entry_bus->read8(0x00FFE190U) == 2);

    constexpr auto animation_event_table = 0xFF0900U;
    animation_entry_bus->write16(animation_event_table, 4U);
    const std::array<std::uint8_t, 10> animation_events{
        0, 0x10, 0x0B, 0x20, 2, 0x30, 8, 0x40, 1, 0x50};
    for (std::size_t index = 0; index < animation_events.size(); ++index) {
        animation_entry_bus->write8(
            animation_event_table + 2U + index, animation_events[index]);
    }
    const auto parsed_animation_events =
        shadowrun::parse_entity_animation_events_0x13C74(
            *animation_entry_bus, animation_event_table);
    assert(parsed_animation_events.accepted_events == 3U);
    assert(parsed_animation_events.special_events == 1U);
    assert(animation_entry_bus->read16(
               shadowrun::session_ram::kEntityTerrainCollisionMarker) == 3U);
    assert(animation_entry_bus->read8(
               shadowrun::session_ram::kEntityAnimationEventSpecialArgument) ==
           0x20U);
    assert(animation_entry_bus->read16(
               shadowrun::session_ram::kEntityTerrainCollisionMarker + 2U +
               6U) == 0xFFFFU);

    animation_entry_bus->write8(animation_entry_record + 0x57U, 1U);
    animation_entry_bus->write16(animation_entry_record + 0xA0U, 0);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityAnimationEventTable, 0);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 2U, 0);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 3U, 0x10U);
    assert(shadowrun::check_entity_animation_transition_0x13BBA(
               *animation_entry_bus, animation_entry_record));

    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 2U, 0x0BU);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 3U, 0x20U);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x4CU, 3U);
    assert(!shadowrun::check_entity_animation_transition_0x13BBA(
                *animation_entry_bus, animation_entry_record));
    animation_entry_bus->write8(animation_entry_record + 0x57U, 0x0CU);
    assert(shadowrun::check_entity_animation_transition_0x13BBA(
               *animation_entry_bus, animation_entry_record));

    // The 0x13CDE rate leaf is direct: +0x9C==1 is the unconditional carry
    // path, while the other path uses the 0x55F02/0x55F20 quotient and keeps
    // the residual D1 value observable.
    std::vector<std::uint8_t> animation_transition_rom(0x200000U, 0);
    animation_transition_rom[
        shadowrun::rom::kEntityStateGridAnimationLimitTable + 0x24U] = 50U;
    auto animation_transition_bus =
        std::make_unique<shadowrun::GenesisMemoryBus>(
            std::move(animation_transition_rom));
    constexpr auto animation_transition_record = 0xFF0100U;
    animation_transition_bus->write8(animation_transition_record + 0x80U,
                                      100U);
    animation_transition_bus->write8(animation_transition_record + 0x4DU, 4U);
    animation_transition_bus->write8(animation_transition_record + 0x9CU, 5U);
    const auto animation_rate =
        shadowrun::evaluate_entity_animation_rate_0x13CDE(
            *animation_transition_bus, animation_transition_record, 0);
    assert(animation_rate.carry);
    assert(animation_rate.residual_d1 == 0U);
    animation_transition_bus->write8(animation_transition_record + 0x9CU, 1U);
    assert(shadowrun::evaluate_entity_animation_rate_0x13CDE(
               *animation_transition_bus, animation_transition_record,
               0xFFU).carry);

    // A type-B event that fails its rate test and passes 0x13C20 advances to
    // phase C. The old +0x4E accumulator is converted by 0x125FA using the
    // inherited D5 table index before the 0xABFC/0x03404 tail runs.
    animation_transition_bus->write8(animation_transition_record + 0x9CU, 5U);
    animation_transition_bus->write8(animation_transition_record + 0x80U, 0U);
    animation_transition_bus->write8(animation_transition_record + 0x57U, 1U);
    animation_transition_bus->write8(animation_transition_record + 0x4EU, 130U);
    animation_transition_bus->write16(animation_transition_record + 0xA0U, 0);
    animation_transition_bus->write16(
        shadowrun::session_ram::kEntityAnimationEventTable, 0);
    animation_transition_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 2U, 0x0BU);
    animation_transition_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 3U, 0x33U);
    animation_transition_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x4CU, 3U);
    animation_transition_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x14CU, 4U);
    animation_transition_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x24CU, 4U);
    animation_transition_bus->write8(0x00FFFC0CU, 0);
    animation_transition_bus->write8(0x00FFFC0DU, 0);
    const shadowrun::EntityAnimationRegisterContext animation_registers{
        0, 0, 0};
    const auto animation_phase_transition =
        shadowrun::run_entity_animation_transition_0x13B12(
            *animation_transition_bus, animation_transition_record,
            animation_registers);
    assert(!animation_phase_transition.carry);
    assert(animation_phase_transition.phase_advanced);
    assert(animation_phase_transition.previous_phase == 1U);
    assert(animation_phase_transition.new_phase == 0x0CU);
    assert(animation_transition_bus->read8(animation_transition_record + 0x4EU) ==
           5U);
    assert(animation_transition_bus->read8(animation_transition_record + 0x9DU) ==
           0x33U);

    // With no type-B event, 0x13B12 selects one accepted event through the
    // ROM PRNG. A carry from its rate test returns without changing phase.
    animation_transition_bus->write8(animation_transition_record + 0x57U, 0);
    animation_transition_bus->write8(animation_transition_record + 0x9CU, 1U);
    animation_transition_bus->write16(
        shadowrun::session_ram::kEntityAnimationEventTable, 0);
    animation_transition_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 2U, 0);
    animation_transition_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 3U, 0x44U);
    const auto animation_event_carry =
        shadowrun::run_entity_animation_transition_0x13B12(
            *animation_transition_bus, animation_transition_record,
            animation_registers);
    assert(animation_event_carry.carry);
    assert(!animation_event_carry.phase_advanced);
    assert(animation_event_carry.accepted_events_considered == 1U);
    assert(animation_event_carry.new_phase == 0U);

    // 0x13A92's negative-frame path can now complete through 0x13AE6 and
    // 0x13466 without a continuation callback.
    std::vector<std::uint8_t> animation_branch_rom(0x200000U, 0);
    animation_branch_rom[
        shadowrun::rom::kEntityStateGridAnimationLimitTable + 0x24U] = 100U;
    auto animation_branch_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(animation_branch_rom));
    constexpr auto animation_branch_record = 0xFF0100U;
    animation_branch_bus->write8(animation_branch_record + 0x9BU, 0);
    animation_branch_bus->write8(animation_branch_record + 0x56U, 0xFFU);
    animation_branch_bus->write8(animation_branch_record + 0x57U, 1U);
    animation_branch_bus->write8(animation_branch_record + 0x9CU, 5U);
    animation_branch_bus->write8(animation_branch_record + 0x80U, 100U);
    animation_branch_bus->write8(animation_branch_record + 0x4DU, 4U);
    animation_branch_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 2U, 0x0BU);
    animation_branch_bus->write8(
        shadowrun::session_ram::kEntityAnimationEventTable + 3U, 0x55U);
    animation_branch_bus->write16(
        shadowrun::session_ram::kEntityAnimationEventTable, 0);
    animation_branch_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x4CU, 4U);
    animation_branch_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x14CU, 4U);
    animation_branch_bus->write8(
        shadowrun::session_ram::kEntityPhaseBase + 0x24CU, 4U);
    const auto animation_branch =
        shadowrun::run_entity_animation_branch_0x13A92(
            *animation_branch_bus, animation_branch_record,
            animation_registers);
    assert(animation_branch.entered);
    assert(animation_branch.display_updated);
    assert(animation_branch_bus->read8(animation_branch_record + 0x57U) == 0U);
    assert(animation_branch_bus->read8(animation_branch_record + 0x56U) ==
           0x7FU);

    // 0x144E8/0x1460E is the direct close-distance gate used by 0x142AE.
    // Its post-15CA4 minor-axis word is retained for the indexed probe, and
    // the close path writes the B45C state before 0x142AE copies the table's
    // first coordinate pair into +0x96/+0x98.
    std::vector<std::uint8_t> animation_142ae_rom(0x200000U, 0);
    const auto put_animation_word = [](
        std::vector<std::uint8_t>& bytes,
        std::size_t address,
        std::uint16_t value) {
        bytes[address] = static_cast<std::uint8_t>(value >> 8U);
        bytes[address + 1U] = static_cast<std::uint8_t>(value);
    };
    const std::array<std::uint16_t, 8> animation_rom_offsets{
        0x00C0U, 0x0001U, 0x0040U, 0x00FFU,
        0x00A7U, 0x0000U, 0x003CU, 0x0006U};
    for (std::size_t index = 0; index < animation_rom_offsets.size();
         ++index) {
        put_animation_word(
            animation_142ae_rom,
            shadowrun::rom::kAnimationDirectionOffsetTable + index * 2U,
            animation_rom_offsets[index]);
    }
    auto animation_142ae_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::move(animation_142ae_rom));
    constexpr auto animation_142ae_record = 0xFF1100U;
    constexpr auto animation_142ae_table = 0xFF2200U;
    animation_142ae_bus->write16(animation_142ae_record, 0x1000U);
    animation_142ae_bus->write16(animation_142ae_record + 4U, 0x2000U);
    animation_142ae_bus->write16(animation_142ae_record + 0x96U, 0x1000U);
    animation_142ae_bus->write16(animation_142ae_record + 0x98U, 0x2000U);
    animation_142ae_bus->write8(animation_142ae_record + 0x10U, 0x20U);
    animation_142ae_bus->write32(animation_142ae_record + 0xE0U,
                                  animation_142ae_table);
    animation_142ae_bus->write16(animation_142ae_table, 0x0123U);
    animation_142ae_bus->write16(animation_142ae_table + 2U, 0x0456U);
    animation_142ae_bus->write16(animation_142ae_record + 0xE4U, 0x7777U);
    const auto animation_142ae_direct =
        shadowrun::run_entity_animation_continuation_0x142AE(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_142ae_direct.entered);
    assert(animation_142ae_direct.position_gate_called);
    assert(animation_142ae_direct.position_gate_carry);
    assert(animation_142ae_direct.position_distance == 0U);
    assert(animation_142ae_direct.indexed_minor_axis == 0U);
    assert(animation_142ae_direct.phase_position_updated);
    assert(animation_142ae_direct.continuation_calls == 0U);
    assert(animation_142ae_bus->read8(animation_142ae_record + 0xB0U) ==
           0x04U);
    assert(animation_142ae_bus->read8(animation_142ae_record + 0xB1U) == 0U);
    assert(animation_142ae_bus->read32(animation_142ae_record + 0x3AU) ==
           0x0000B45CU);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xE4U) == 0U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0x96U) ==
           0x0123U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0x98U) ==
           0x0456U);

    // A far position preserves the direct gate's carry clear and exposes
    // the exact 0x145A2 tail target rather than guessing its behavior.
    std::vector<shadowrun::GenesisAddress> animation_142ae_targets;
    animation_142ae_bus->write16(animation_142ae_record, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 4U, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 0x96U, 0x0200U);
    animation_142ae_bus->write16(animation_142ae_record + 0x98U, 0U);
    const auto animation_142ae_far =
        shadowrun::run_entity_animation_continuation_0x142AE(
            *animation_142ae_bus, animation_142ae_record,
            [&animation_142ae_targets](shadowrun::GenesisBus&,
                                        shadowrun::GenesisAddress,
                                        shadowrun::GenesisAddress target) {
                animation_142ae_targets.push_back(target);
            });
    assert(!animation_142ae_far.position_gate_carry);
    assert(animation_142ae_far.position_distance == 0x0200U);
    assert(animation_142ae_far.continuation_target == 0x000145A2U);
    assert(animation_142ae_targets.size() == 1U);

    // 0x145A2 now owns the direct 0x14C2E/0x15D3C terminal path. The
    // direction helper's old +0xB0 bit is consumed by BCLR before the flags
    // are recomputed, matching the ROM's one-shot close-distance state.
    animation_142ae_bus->write16(animation_142ae_record + 0x16U, 0x003CU);
    animation_142ae_bus->write8(animation_142ae_record + 0xB0U, 0x04U);
    animation_142ae_bus->write16(animation_142ae_record + 0xB8U, 0);
    const auto animation_145a2_terminal =
        shadowrun::run_entity_animation_update_0x145A2(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_145a2_terminal.entered);
    assert(!animation_145a2_terminal.stopped_at_frame_cutoff);
    assert(!animation_145a2_terminal.routed_to_0x149C0);
    assert(animation_145a2_terminal.direction_flags_updated);
    assert(animation_145a2_terminal.transformed_position);
    assert(animation_145a2_terminal.continuation_calls == 0U);
    assert(animation_142ae_bus->read8(animation_142ae_record + 0xB1U) ==
           0x03U);
    assert(animation_142ae_bus->read32(animation_142ae_record + 0x3AU) ==
           0x0000B574U);

    // A movement word with bit one set takes 0x14BA6's 0x14682 route and
    // then tail-jumps to 0x149C0; both original targets remain observable.
    animation_142ae_targets.clear();
    animation_142ae_bus->write8(animation_142ae_record + 0xB0U, 0);
    animation_142ae_bus->write8(animation_142ae_record + 0xB1U, 0);
    animation_142ae_bus->write16(animation_142ae_record + 0xB8U, 0x0002U);
    animation_142ae_bus->write16(animation_142ae_record, 0);
    animation_142ae_bus->write16(animation_142ae_record + 4U, 0);
    animation_142ae_bus->write16(animation_142ae_record + 0x96U, 1U);
    animation_142ae_bus->write16(animation_142ae_record + 0x98U, 0);
    const auto animation_145a2_route =
        shadowrun::run_entity_animation_update_0x145A2(
            *animation_142ae_bus, animation_142ae_record,
            [&animation_142ae_targets](shadowrun::GenesisBus&,
                                        shadowrun::GenesisAddress,
                                        shadowrun::GenesisAddress target) {
                animation_142ae_targets.push_back(target);
            });
    assert(animation_145a2_route.routed_to_0x149C0);
    assert(!animation_145a2_route.transformed_position);
    assert(animation_145a2_route.continuation_calls == 2U);
    assert(animation_145a2_route.unresolved_calls == 0U);
    assert(animation_142ae_targets.size() == 2U);
    assert(animation_142ae_targets[0] == 0x00014682U);
    assert(animation_142ae_targets[1] == 0x000149C0U);

    // 0x14682 is a real movement body rather than just the continuation
    // target observed above. Seed its eight-byte direction table with the
    // ROM values, then exercise the 0x148AA single-step path, the 0x14854
    // paired path, and the 0x14AF4/0x149C0 escalation.
    animation_142ae_bus->write16(shadowrun::ram::kTilePlaneWidth, 0x003FU);
    animation_142ae_bus->write16(
        shadowrun::session_ram::kEntityTerrainBoundsYLower, 0x00A0U);
    for (std::uint16_t index = 0; index <= 0x0400U; ++index) {
        animation_142ae_bus->write8(
            shadowrun::ram::kIndexedObjectByteTable + index, 0U);
    }
    animation_142ae_bus->write16(animation_142ae_record, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 4U, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 0x96U, 0x0200U);
    animation_142ae_bus->write16(animation_142ae_record + 0x98U, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 0xBAU, 0x0020U);
    animation_142ae_bus->write8(animation_142ae_record + 0xB0U, 0U);
    // D5=0x200, D7=2, and the first 0x148AA candidate is 0x201. The
    // following obstacle makes 0x148AA return 0x201 as its first path.
    animation_142ae_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable + 0x0202U, 1U);
    const auto animation_14682_single =
        shadowrun::run_entity_animation_route_0x14682(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_14682_single.entered);
    assert(!animation_14682_single.map_blocked);
    assert(animation_14682_single.path_written);
    assert(animation_14682_single.path_value == 0x0201U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBCU) ==
           0x0201U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBEU) ==
           0xFFFFU);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBAU) == 0U);

    for (std::uint16_t index = 0; index <= 0x0400U; ++index) {
        animation_142ae_bus->write8(
            shadowrun::ram::kIndexedObjectByteTable + index, 0U);
    }
    animation_142ae_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable + 0x0201U, 1U);
    const auto animation_14682_paired =
        shadowrun::run_entity_animation_route_0x14682(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_14682_paired.path_written);
    assert(!animation_14682_paired.route_escalated);
    assert(animation_14682_paired.path_value == 0x02C0U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBCU) ==
           0x02C0U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBEU) ==
           0x02C1U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xC0U) ==
           0xFFFFU);

    for (std::uint16_t index = 0; index <= 0x0400U; ++index) {
        animation_142ae_bus->write8(
            shadowrun::ram::kIndexedObjectByteTable + index, 0U);
    }
    animation_142ae_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable + 0x0201U, 1U);
    animation_142ae_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable + 0x02C0U, 1U);
    animation_142ae_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable + 0x0240U, 1U);
    animation_142ae_bus->write16(animation_142ae_record + 0x34U, 0U);
    const auto animation_14682_route =
        shadowrun::run_entity_animation_route_0x14682(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_14682_route.route_escalated);
    assert(animation_14682_route.continuation_target == 0x000149C0U);
    assert(animation_14682_route.continuation_calls == 1U);
    assert(animation_14682_route.unresolved_calls == 0U);
    assert((animation_142ae_bus->read8(animation_142ae_record + 0xB0U) &
            0x0AU) == 0U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBCU) ==
           0x00C0U);

    // The 0x14902/0x14914 route helpers use the live FF1700 map bytes. An
    // unobstructed same-cell path clears the route bits through 0x14756.
    assert(shadowrun::calculate_entity_animation_grid_0x14902(0, 0) == 0U);
    assert(shadowrun::calculate_entity_animation_grid_0x14902(
               0x0400U, 0) == 0x20U);
    animation_142ae_bus->write8(animation_142ae_record + 0xB0U, 0x0AU);
    animation_142ae_bus->write16(animation_142ae_record, 0);
    animation_142ae_bus->write16(animation_142ae_record + 4U, 0);
    animation_142ae_bus->write16(animation_142ae_record + 0x96U, 1U);
    animation_142ae_bus->write16(animation_142ae_record + 0x98U, 0);
    const auto animation_149c0_clear =
        shadowrun::run_entity_animation_route_0x149C0(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_149c0_clear.unobstructed_path_cleared);
    assert(!animation_149c0_clear.transformed_position);
    assert((animation_142ae_bus->read8(animation_142ae_record + 0xB0U) &
            0x0AU) == 0U);

    // With route mode clear, 0x149C0 consumes the inline path grid and uses
    // 0x14B4E to emit the target pair before the common distance transform.
    animation_142ae_bus->write8(animation_142ae_record + 0xB0U, 0);
    animation_142ae_bus->write16(animation_142ae_record + 0xBAU, 0);
    animation_142ae_bus->write16(animation_142ae_record + 0xBCU, 0x0041U);
    const auto animation_149c0_path =
        shadowrun::run_entity_animation_route_0x149C0(
            *animation_142ae_bus, animation_142ae_record);
    assert(!animation_149c0_path.path_entry_advanced);
    assert(animation_149c0_path.selected_grid == 0x0041U);
    assert(animation_149c0_path.transformed_position);
    assert(animation_142ae_bus->read16(
               shadowrun::session_ram::kEntityAnimationTargetX) != 0U);
    assert(animation_142ae_bus->read16(shadowrun::session_ram::kEntityAnimationTargetY) != 0U);
    assert(animation_142ae_bus->read32(animation_142ae_record + 0x3AU) ==
           0x0000B574U);

    // End-of-path with route mode clear takes the direct 0x14C2E and common
    // 0x145E8 transform tail; route mode set exposes 0x14A54 instead.
    animation_142ae_bus->write16(animation_142ae_record + 0xBAU, 0);
    animation_142ae_bus->write16(animation_142ae_record + 0xBCU, 0);
    animation_142ae_bus->write16(animation_142ae_record + 0xBEU, 0xFFFFU);
    animation_142ae_bus->write8(animation_142ae_record + 0xB0U, 0);
    const auto animation_149c0_end =
        shadowrun::run_entity_animation_route_0x149C0(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_149c0_end.reached_end_of_path);
    assert(animation_149c0_end.transformed_position);
    animation_142ae_targets.clear();
    animation_142ae_bus->write16(animation_142ae_record + 0x96U, 0x0400U);
    animation_142ae_bus->write16(animation_142ae_record + 0x98U, 0);
    for (std::uint16_t index = 0; index <= 0x20U; ++index) {
        animation_142ae_bus->write8(
            shadowrun::ram::kIndexedObjectByteTable + index, 1U);
    }
    animation_142ae_bus->write8(animation_142ae_record + 0xB0U, 0x08U);
    const auto animation_149c0_alternate =
        shadowrun::run_entity_animation_route_0x149C0(
            *animation_142ae_bus, animation_142ae_record,
            [&animation_142ae_targets](shadowrun::GenesisBus&,
                                        shadowrun::GenesisAddress,
                                        shadowrun::GenesisAddress target) {
                animation_142ae_targets.push_back(target);
            });
    assert(animation_149c0_alternate.alternate_route);
    assert(animation_149c0_alternate.continuation_target == 0x00014A54U);
    assert(animation_142ae_targets.size() == 1U);

    // The same 0x14A54 tail runs directly when no continuation observer is
    // installed. With the primary C0 candidate blocked, its five-attempt
    // 0x14764 search reaches the direction-6 candidate at grid 0xFF.
    animation_142ae_bus->write16(animation_142ae_record + 0x34U, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 0xBAU, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 0xBCU, 0U);
    animation_142ae_bus->write16(animation_142ae_record + 0xBEU, 0xFFFFU);
    animation_142ae_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable + 0x00C0U, 1U);
    const auto animation_149c0_alternate_direct =
        shadowrun::run_entity_animation_route_0x149C0(
            *animation_142ae_bus, animation_142ae_record);
    assert(animation_149c0_alternate_direct.alternate_route);
    assert(animation_149c0_alternate_direct.continuation_target ==
           0x00014A54U);
    assert(animation_149c0_alternate_direct.continuation_calls == 1U);
    assert(animation_149c0_alternate_direct.unresolved_calls == 0U);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBCU) ==
           0x00FFU);
    assert(animation_142ae_bus->read16(animation_142ae_record + 0xBEU) ==
           0xFFFFU);

    // 0x13F76's resource gate now runs the direct 0x14D8E scratch/terrain
    // decision. Keep the scratch point inside the reviewed terrain bounds so
    // the empty map reaches the later 0x1E2E0 queue leaf.
    constexpr auto resource_gate_record = 0xFF0500U;
    constexpr auto resource_control_record = 0xFF0600U;
    animation_entry_bus->write16(
        shadowrun::session_ram::kFrameEntityGlobalGate, 0);
    animation_entry_bus->write8(
        shadowrun::session_ram::kEntityReactionSpecialGate, 0);
    animation_entry_bus->write8(resource_gate_record + 0x10U, 0x20U);
    animation_entry_bus->write8(resource_gate_record + 0x9EU, 0);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityAnimationResourceReference, 0);
    animation_entry_bus->write16(resource_gate_record + 0x2EU, 0);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityAnimationResourceMode, 2U);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityTerrainBoundsXLower, 0U);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityTerrainBoundsXUpper, 0x0200U);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityTerrainBoundsYLower, 0U);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityTerrainBoundsYUpper, 0x0200U);
    animation_entry_bus->write32(
        shadowrun::ram::kEntityObjectControlPointer,
        resource_control_record);
    animation_entry_bus->write8(resource_control_record + 0x11U, 0);
    animation_entry_bus->write16(shadowrun::ram::kBackgroundQueueTimer, 0);
    const auto resource_direct =
        shadowrun::run_entity_animation_resource_gate_0x13F76(
            *animation_entry_bus, resource_gate_record);
    assert(!resource_direct.blocked_by_global_gate);
    assert(resource_direct.resource_queued);
    assert(resource_direct.continuation_calls == 2);
    assert(resource_direct.unresolved_calls == 0);
    assert(animation_entry_bus->read16(
               shadowrun::session_ram::kEntityAnimationResourceGateState) ==
           0U);
    assert((animation_entry_bus->read8(
                shadowrun::session_ram::kEntityAnimationResourceScratch +
                0x10U) & 1U) != 0);
    assert(animation_entry_bus->read32(
               shadowrun::ram::kBackgroundQueuePointer) == 0x00020FECU);
    assert(animation_entry_bus->read16(shadowrun::ram::kBackgroundQueueTimer) ==
           0x003CU);
    assert((animation_entry_bus->read8(resource_gate_record + 0x9EU) & 1U) !=
           0);

    // A nonzero terrain byte returns through 0x14DFC before the occupancy
    // loop. This checks the direct carry result independently of 0x13F76.
    animation_entry_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable, 1U);
    animation_entry_bus->write16(
        shadowrun::session_ram::kEntityAnimationResourceGateState, 0);
    const auto resource_blocked =
        shadowrun::run_entity_animation_resource_decision_0x14D8E(
            *animation_entry_bus, resource_gate_record,
            resource_control_record);
    assert(resource_blocked.carry);
    assert(resource_blocked.terrain_nonzero);
    assert(!resource_blocked.terrain_collision_gate_active);
    assert(!resource_blocked.occupancy_scan_stopped);
    assert(resource_blocked.occupancy_checks == 0U);
    animation_entry_bus->write8(
        shadowrun::ram::kIndexedObjectByteTable, 0U);

    // With terrain clear, a matching active FF0100 record makes 0x15ABA
    // stop the first occupancy pass. 0x14D8E still clears its own carry at
    // 0x14DEE after that scan result.
    animation_entry_bus->write16(resource_gate_record, 0x1000U);
    animation_entry_bus->write16(resource_gate_record + 0x04U, 0x1000U);
    animation_entry_bus->write8(0xFF0100U + 0x10U, 1U);
    animation_entry_bus->write8(0xFF0100U + 0x4CU, 1U);
    animation_entry_bus->write16(0xFF0100U, 0x1000U);
    animation_entry_bus->write16(0xFF0100U + 0x04U, 0x1000U);
    animation_entry_bus->write32(
        shadowrun::ram::kEntityObjectControlPointer, 0xFF0100U);
    const auto resource_occupancy =
        shadowrun::run_entity_animation_resource_decision_0x14D8E(
            *animation_entry_bus, resource_gate_record, 0xFF0100U);
    assert(animation_entry_bus->read32(
               shadowrun::session_ram::kEntityAnimationResourceSource) ==
           0xFF0100U);
    assert(animation_entry_bus->read16(
               resource_occupancy.scratch_record) == 0x1000U);
    assert(animation_entry_bus->read16(
               resource_occupancy.scratch_record + 0x04U) == 0x1000U);
    assert(!resource_occupancy.carry);
    assert(!resource_occupancy.terrain_nonzero);
    assert(resource_occupancy.occupancy_scan_stopped);
    assert(resource_occupancy.occupancy_checks == 1U);
    animation_entry_bus->write8(0xFF0100U + 0x10U, 0U);
    animation_entry_bus->write8(0xFF0100U + 0x4CU, 0U);
    animation_entry_bus->write32(
        shadowrun::ram::kEntityObjectControlPointer,
        resource_control_record);

    std::vector<shadowrun::GenesisAddress> resource_targets;
    const shadowrun::EntityAnimationResourceContinuationRoutine resource_hook =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address,
            shadowrun::EntityAnimationResourceCallContext& context) {
            assert(address == resource_gate_record);
            resource_targets.push_back(context.target);
            if (context.target == 0x00014D8EU) {
                context.carry = true;
            }
        };
    animation_entry_bus->write8(resource_gate_record + 0x9EU, 0);
    animation_entry_bus->write16(shadowrun::ram::kBackgroundQueueTimer, 0);
    const auto resource_carried =
        shadowrun::run_entity_animation_resource_gate_0x13F76(
            *animation_entry_bus, resource_gate_record, resource_hook);
    assert(resource_carried.continuation_calls == 1);
    assert(resource_carried.unresolved_calls == 0);
    assert(!resource_carried.resource_queued);
    assert(resource_targets.size() == 1);
    assert(resource_targets[0] == 0x00014D8EU);

    auto acae_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::vector<std::uint8_t>(0x200000U, 0));
    constexpr auto acae_record = 0xFF0100U;
    std::vector<shadowrun::GenesisAddress> acae_targets;
    const shadowrun::EntityAnimationAcaeContinuationRoutine acae_continuation =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address,
            shadowrun::EntityAnimationAcaeCallContext& context) {
            assert(address == acae_record);
            acae_targets.push_back(context.target);
        };

    // The first AC AE gate is the VBlank-active wakeup path.
    acae_bus->write8(shadowrun::ram::kVblankAnimationLock, 1U);
    const auto acae_locked = shadowrun::run_entity_animation_update_0xACAE(
        *acae_bus, acae_record, acae_continuation);
    assert(acae_locked.entered);
    assert(acae_locked.continuation_calls == 0);
    assert((acae_bus->read8(acae_record + 0x11U) & 1U) != 0);
    assert(acae_bus->read32(acae_record + 0x3AU) == 0x0000B45CU);

    // A nonzero FFDF3A transition writes the CC-derived word and then
    // deliberately overwrites the handler with B574, matching both ROM stores.
    acae_bus->write8(shadowrun::ram::kVblankAnimationLock, 0);
    acae_bus->write16(shadowrun::session_ram::kEntityReactionTransitionState,
                      1U);
    acae_bus->write16(acae_record + 0xCCU, 0x1234U);
    const auto acae_transition =
        shadowrun::run_entity_animation_update_0xACAE(
            *acae_bus, acae_record, acae_continuation);
    assert(acae_transition.continuation_calls == 0);
    assert(acae_bus->read16(acae_record + 0x2EU) == 0x9234U);
    assert(acae_bus->read16(acae_record + 0x34U) == 0x9234U);
    assert(acae_bus->read32(acae_record + 0x3AU) == 0x0000B574U);

    // The FFE18B/FFE18C gate either counts down and wakes the record or
    // enters the zero-D0 B574 terminal path.
    acae_bus->write16(shadowrun::session_ram::kEntityReactionTransitionState,
                      0);
    acae_bus->write8(shadowrun::session_ram::kEntityAnimationUpdateGate, 1U);
    acae_bus->write8(shadowrun::session_ram::kEntityAnimationUpdateTimer, 2U);
    acae_targets.clear();
    const auto acae_countdown =
        shadowrun::run_entity_animation_update_0xACAE(
            *acae_bus, acae_record, acae_continuation);
    assert(acae_countdown.continuation_calls == 0);
    assert(acae_bus->read8(
               shadowrun::session_ram::kEntityAnimationUpdateTimer) == 1U);
    acae_bus->write8(shadowrun::session_ram::kEntityAnimationUpdateTimer, 0);
    const auto acae_zero = shadowrun::run_entity_animation_update_0xACAE(
        *acae_bus, acae_record, acae_continuation);
    assert(acae_zero.continuation_calls == 0);
    assert(acae_bus->read16(acae_record + 0x34U) == 0);
    assert(acae_bus->read32(acae_record + 0x3AU) == 0x0000B574U);

    // A negative timer with no transition consumes the direct input/status
    // path. The ABFC target remains observable through the callback when one
    // is supplied, while its delay leaf also has a direct default.
    acae_bus->write8(shadowrun::session_ram::kEntityAnimationUpdateGate, 0);
    acae_bus->write16(acae_record + 0xDCU, 0xFFFFU);
    acae_bus->write16(shadowrun::ram::kInputPort0, 0);
    acae_bus->write16(shadowrun::ram::kControllerDecode0, 0xFFFFU);
    acae_bus->write16(shadowrun::session_ram::kFrameTransition, 0);
    acae_bus->write8(acae_record + 0x43U, 0);
    acae_bus->write8(acae_record + 0x56U, 0);
    acae_bus->write8(acae_record + 0x57U, 0);
    acae_bus->write32(acae_record + 0x3AU, 0);
    acae_targets.clear();
    const auto acae_input = shadowrun::run_entity_animation_update_0xACAE(
        *acae_bus, acae_record, acae_continuation);
    assert(acae_input.continuation_calls == 1);
    assert(acae_targets[0] == 0x0000ABFCU);
    assert(acae_bus->read8(acae_record + 0x43U) == 1U);
    assert((acae_bus->read8(acae_record + 0x11U) & 1U) != 0);
    assert(acae_bus->read32(acae_record + 0x3AU) == 0x0000B45CU);

    // The negative-frame branch clears +0x43, then selects AE6E followed by
    // AF7C/AFAE from the direct random-bucket byte.
    acae_bus->write8(acae_record + 0x56U, 0xFFU);
    acae_bus->write8(shadowrun::ram::kEntityStateRandomBucket, 0);
    acae_bus->write16(shadowrun::session_ram::kFrameTransition, 1U);
    acae_targets.clear();
    const auto acae_negative_frame =
        shadowrun::run_entity_animation_update_0xACAE(
            *acae_bus, acae_record, acae_continuation);
    assert(acae_negative_frame.continuation_calls == 2);
    assert(acae_targets[0] == 0x0000AE6EU);
    assert(acae_targets[1] == 0x0000AF7CU);

    // AAD2 changes A0 to FFFC04 in the ROM. The typed call context must carry
    // that register change into the later command/timer calls and AE26 tail.
    constexpr auto acae_rebound_record = 0xFF0200U;
    const shadowrun::EntityAnimationAcaeContinuationRoutine acae_registers =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address,
            shadowrun::EntityAnimationAcaeCallContext& context) {
            acae_targets.push_back(context.target);
            if (context.target == 0x0001514EU ||
                context.target == 0x0000AAD2U) {
                context.carry = true;
            }
            if (context.target == 0x0000AAD2U) {
                assert(address == acae_record);
                context.a0 = acae_rebound_record;
            }
            if (context.target == 0x000530DEU ||
                context.target == 0x0000ABFCU) {
                assert(address == acae_rebound_record);
            }
        };
    acae_targets.clear();
    acae_bus->write8(acae_record + 0x11U, 0);
    acae_bus->write32(acae_record + 0x3AU, 0);
    acae_bus->write8(acae_rebound_record + 0x11U, 0);
    acae_bus->write32(acae_rebound_record + 0x3AU, 0);
    acae_bus->write16(acae_record + 0xB6U, 0);
    acae_bus->write16(acae_record + 0xDCU, 0xFFFFU);
    acae_bus->write16(acae_record + 0x0CU, 1U);
    acae_bus->write8(acae_record + 0x56U, 0);
    acae_bus->write8(acae_record + 0x57U, 1U);
    acae_bus->write16(shadowrun::session_ram::kFrameTransition, 1U);
    acae_bus->write16(shadowrun::ram::kControllerDecode0, 0xFFFFU);
    const auto acae_rebound = shadowrun::run_entity_animation_update_0xACAE(
        *acae_bus, acae_record, acae_registers);
    assert(acae_rebound.continuation_calls == 4);
    assert(acae_rebound.unresolved_calls == 0);
    assert(acae_targets.size() == 4);
    assert(acae_targets[0] == 0x0001514EU);
    assert(acae_targets[1] == 0x0000AAD2U);
    assert(acae_targets[2] == 0x000530DEU);
    assert(acae_targets[3] == 0x0000ABFCU);
    assert((acae_bus->read8(acae_rebound_record + 0x11U) & 1U) != 0);
    assert(acae_bus->read32(acae_rebound_record + 0x3AU) == 0x0000B45CU);
    assert(acae_bus->read32(acae_record + 0x3AU) == 0);

    // ABFC's nested D7EA/D81C calculation samples the two byte fields, uses
    // the first set state bit in 0xC..0xE, applies the mode shift, and clamps
    // the resulting signed delay to fifteen.
    constexpr auto delay_record = 0xFF0300U;
    acae_bus->write8(delay_record + 0x78U, 6U);
    acae_bus->write8(delay_record + 0x7BU, 4U);
    acae_bus->write16(delay_record + 0x90U, 0x5000U);
    const auto delay_normal = shadowrun::update_entity_animation_delay_0xABFC(
        *acae_bus, delay_record, 0);
    assert(delay_normal.sampled_base == 5U);
    assert(delay_normal.state_adjustment == 4U);
    assert(delay_normal.delay_written == 0x004AU);
    assert(acae_bus->read16(delay_record + 0xDCU) == 0x004AU);

    const auto delay_double = shadowrun::update_entity_animation_delay_0xABFC(
        *acae_bus, delay_record, 2U);
    assert(delay_double.delay_written == 0x0094U);
    const auto delay_half = shadowrun::update_entity_animation_delay_0xABFC(
        *acae_bus, delay_record, 1U);
    assert(delay_half.delay_written == 0x0025U);

    acae_bus->write8(delay_record + 0x78U, 0U);
    acae_bus->write8(delay_record + 0x7BU, 0U);
    acae_bus->write16(delay_record + 0x90U, 0U);
    const auto delay_clamped = shadowrun::update_entity_animation_delay_0xABFC(
        *acae_bus, delay_record, 0);
    assert(delay_clamped.delay_written == 0x006AU);

    // With no continuation override, AC AE now resolves its ABFC call
    // directly and leaves the calculated delay on the current record.
    acae_bus->write8(acae_record + 0x43U, 0);
    acae_bus->write16(acae_record + 0xDCU, 0xFFFFU);
    acae_bus->write8(acae_record + 0x78U, 6U);
    acae_bus->write8(acae_record + 0x7BU, 4U);
    acae_bus->write16(acae_record + 0x90U, 0x1000U);
    acae_bus->write8(acae_record + 0x56U, 0);
    acae_bus->write8(acae_record + 0x57U, 0);
    acae_bus->write16(shadowrun::ram::kInputPort0, 0);
    acae_bus->write16(shadowrun::ram::kControllerDecode0, 0xFFFFU);
    acae_bus->write16(shadowrun::session_ram::kFrameTransition, 0);
    acae_bus->write8(shadowrun::session_ram::kEntityAnimationUpdateGate, 0);
    const auto acae_direct_delay =
        shadowrun::run_entity_animation_update_0xACAE(
            *acae_bus, acae_record);
    assert(acae_direct_delay.continuation_calls == 1);
    assert(acae_direct_delay.unresolved_calls == 0);
    assert(acae_bus->read16(acae_record + 0xDCU) == 0x0025U);
    assert(acae_bus->read8(acae_record + 0x43U) == 1U);

    auto expired_animation_bus = std::make_unique<shadowrun::GenesisMemoryBus>(
        std::vector<std::uint8_t>(0x200000U, 0));
    shadowrun::refresh_entity_animation_0x03404(*expired_animation_bus);
    // Three records emit 22 primary/secondary 32-byte streams each. The
    // direct 0x03548 tail emits 3 * (16 + 6) data words and command pairs.
    assert(expired_animation_bus->port_writes().size() == 762U);
    assert(expired_animation_bus->port_writes()[0].value == 0x5280U);
    assert(expired_animation_bus->port_writes()[1].value == 0x0003U);

    memory_bus.write8(shadowrun::session_ram::kFrameClipHorizontalMode, 0xFF);
    assert(shadowrun::calculate_frame_horizontal_status_0x3A90(
               memory_bus, 0, 0x100) == 0xFFFFU);
    memory_bus.write8(shadowrun::session_ram::kFrameClipHorizontalMode, 0xFE);
    assert(shadowrun::calculate_frame_horizontal_status_0x3A90(
               memory_bus, 0, 0x200) == 0xFFFEU);
    memory_bus.write8(shadowrun::session_ram::kFrameClipHorizontalMode, 1);
    assert(shadowrun::calculate_frame_horizontal_status_0x3A90(
               memory_bus, 0x102, 0x100) == 1);
    memory_bus.write8(shadowrun::session_ram::kFrameClipHorizontalMode, 2);
    assert(shadowrun::calculate_frame_horizontal_status_0x3A90(
               memory_bus, 0x102, 0x100) == 0);

    memory_bus.write8(shadowrun::session_ram::kFrameClipVerticalMode, 0xFF);
    assert(shadowrun::calculate_frame_vertical_status_0x3B1E(
               memory_bus, 0, 0x100) == 0xFFFFU);
    memory_bus.write8(shadowrun::session_ram::kFrameClipVerticalMode, 1);
    assert(shadowrun::calculate_frame_vertical_status_0x3B1E(
               memory_bus, 0x102, 0x100) == 1);
    memory_bus.write8(shadowrun::session_ram::kFrameClipVerticalMode, 2);
    assert(shadowrun::calculate_frame_vertical_status_0x3B1E(
               memory_bus, 0x102, 0x100) == 0);

    TraceBus frame_vector_bus;
    frame_vector_bus.words[shadowrun::rom::kObjectGenerationInterpolationTable +
                           2U] = 0x4000U;
    frame_vector_bus.words[shadowrun::rom::kObjectGenerationInterpolationTable +
                           0x42U] = 0xE000U;
    const auto frame_vector = shadowrun::transform_frame_vector_0x15DCC(
        frame_vector_bus, 0x0200U, 0x0400U);
    assert(frame_vector.d2 == 0x0100U);
    assert(frame_vector.d3 == 0xFF80U);
    const auto zero_frame_vector = shadowrun::transform_frame_vector_0x15DCC(
        frame_vector_bus, 0x0200U, 0);
    assert(zero_frame_vector.d2 == 0);
    assert(zero_frame_vector.d3 == 0);

    // 0x15CA4 preserves the high word through its word-only differences,
    // exchanges complete D0/D1 longwords when the second axis is larger, and
    // rounds an odd smaller-axis magnitude before adding it.
    assert(shadowrun::calculate_frame_distance_0x15CA4(
               0x12340009U, 0x5678000AU, 0, 0) == 0x5678000FU);
    assert(shadowrun::calculate_frame_distance_0x15CA4(
               0x12340010U, 0x56780020U, 0, 0) == 0x56780028U);
    assert(shadowrun::calculate_frame_distance_0x15CA4(
               0x00008000U, 0x00000000U, 0, 0) == 0x00008000U);

    std::vector<std::uint8_t> spatial_rom(0x16000U, 0);
    const auto write_spatial_word =
        [&spatial_rom](std::size_t address, std::uint16_t value) {
            spatial_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            spatial_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    write_spatial_word(shadowrun::rom::kObjectGenerationInterpolationTable,
                       0x4000U);
    write_spatial_word(shadowrun::rom::kObjectGenerationInterpolationTable +
                           0x40U,
                       0xE000U);
    write_spatial_word(shadowrun::rom::kFrameDeltaDirectionTable + 2U,
                       0x0040U);
    spatial_rom[shadowrun::rom::kFrameDeltaCorrectionTable + 16U] = 5;
    write_spatial_word(shadowrun::rom::kFrameMovementTable + 8U, 0xFFFFU);
    write_spatial_word(shadowrun::rom::kFrameMovementTable + 10U, 0);
    shadowrun::GenesisMemoryBus spatial_bus(std::move(spatial_rom));
    const auto frame_delta = shadowrun::transform_frame_delta_0x15D3C(
        spatial_bus, 0x0100U, 0, 0, 0);
    assert(frame_delta.d2 == 0x4500U);
    assert(frame_delta.d3 == 0x0100U);
    spatial_bus.write8(shadowrun::ram::kEntityStateGlobalPhase, 2);
    spatial_bus.write32(shadowrun::ram::kEntityObjectControlPointer,
                        0xFF3000U);
    spatial_bus.write16(0xFF302EU, 2);
    spatial_bus.write16(0xFF3000U, 0x2000U);
    spatial_bus.write16(0xFF3004U, 0);
    spatial_bus.write16(shadowrun::ram::kCameraX, 0x1000U);
    spatial_bus.write16(shadowrun::ram::kCameraY, 0x1000U);
    spatial_bus.write8(shadowrun::session_ram::kFrameClipHorizontalMode, 0);
    spatial_bus.write8(shadowrun::session_ram::kFrameClipVerticalMode, 0);
    const auto spatial_prelude =
        shadowrun::prepare_frame_spatial_state_0x3A1C(spatial_bus);
    assert(!spatial_prelude.phase_three_path);
    assert(spatial_prelude.d2_after == 0x00F0U);
    assert(spatial_prelude.d3_after == 0x1F88U);
    assert(spatial_prelude.horizontal_status == 2);
    assert(spatial_prelude.vertical_status == 0xFFU);
    spatial_bus.write8(shadowrun::ram::kEntityStateGlobalPhase, 3);
    spatial_bus.write16(shadowrun::session_ram::kFrameTransformState, 0xFFFFU);
    spatial_bus.write16(shadowrun::session_ram::kFrameTransformProgress, 0);
    spatial_bus.write8(shadowrun::session_ram::kFrameClipHorizontalMode, 7);
    spatial_bus.write8(shadowrun::session_ram::kFrameClipVerticalMode, 8);
    const auto phase_three_prelude =
        shadowrun::prepare_frame_spatial_state_0x3A1C(spatial_bus);
    assert(phase_three_prelude.phase_three_path);
    assert(spatial_bus.read16(shadowrun::session_ram::kFrameTransformState) == 0);
    assert(spatial_bus.read16(shadowrun::session_ram::kFrameTransformProgress) ==
           0x0600U);

    spatial_bus.write16(shadowrun::session_ram::kFrameTransformState, 0);
    spatial_bus.write16(shadowrun::session_ram::kFrameTransformProgress, 0);
    spatial_bus.write16(shadowrun::ram::kCameraX, 0);
    spatial_bus.write16(shadowrun::ram::kCameraY, 0);
    spatial_bus.write16(0xFFDF5A, 0xFFFFU);
    const auto short_frame_delta =
        shadowrun::update_frame_spatial_tail_0x3BAC(spatial_bus);
    assert(short_frame_delta.early_return);
    assert(spatial_bus.read16(0xFFDF5A) == 0);

    spatial_bus.write16(shadowrun::session_ram::kFrameTransformState, 0);
    spatial_bus.write16(shadowrun::session_ram::kFrameTransformProgress, 0);
    spatial_bus.write16(shadowrun::ram::kCameraX, 0);
    spatial_bus.write16(shadowrun::ram::kCameraY, 0x0100U);
    spatial_bus.write16(0xFFDDCE, 0xFF00U);
    assert(spatial_bus.read16(0xFFDDCE) == 0xFF00U);
    spatial_bus.write16(shadowrun::ram::kVdpDataWord, 0);
    spatial_bus.write16(shadowrun::ram::kVdpRegisterWord0A, 0);
    spatial_bus.write16(shadowrun::ram::kTilePlaneWidth, 0x20U);
    spatial_bus.write16(shadowrun::ram::kVdpClearCommandA, 0xE000U);
    spatial_bus.write16(shadowrun::ram::kVdpClearCommandB, 0x4000U);
    spatial_bus.write32(shadowrun::ram::kTilePlaneSourceA, 0xFF8000U);
    spatial_bus.write32(shadowrun::ram::kTilePlaneSourceB, 0xFF9000U);
    assert(spatial_bus.read16(shadowrun::rom::kFrameMovementTable + 8U) ==
           0xFFFFU);
    spatial_bus.clear_port_writes();
    const auto moved_frame =
        shadowrun::update_frame_spatial_tail_0x3BAC(spatial_bus);
    assert(!moved_frame.early_return);
    assert(moved_frame.d2_after == 0x4500U);
    assert(moved_frame.d3_after == 0x0100U);
    assert(moved_frame.horizontal_updated);
    assert(!moved_frame.vertical_updated);
    assert(spatial_bus.read16(shadowrun::ram::kCameraX) == 0xFFF0U);
    assert(spatial_bus.read16(0xFFDF38) == 0xFF80U);
    assert(moved_frame.horizontal_stream.draw_calls == 2);
    assert(moved_frame.horizontal_stream.words_emitted == 4);
    assert(spatial_bus.port_writes().size() == 12);

    shadowrun::ShadowrunRuntime memory_runtime(memory_bus);
    memory_runtime.initialise_controller_ports();
    assert(memory_bus.read8(shadowrun::genesis::kControllerControl) == 0x40);
    assert(memory_bus.read8(shadowrun::genesis::kControllerData) == 0x40);
    assert(memory_bus.read8(shadowrun::ram::kControllerSignature) == 0xFF);

    memory_bus.write16(shadowrun::ram::kControllerState0, 0);
    memory_bus.write16(shadowrun::ram::kControllerState1, 0);
    memory_runtime.sample_controller_input();
    assert(memory_bus.read16(shadowrun::ram::kControllerState0) == 0x00FF);
    assert(memory_bus.read16(shadowrun::ram::kInputPort0) == 0xFFFF);
    assert(memory_bus.read16(shadowrun::ram::kControllerDecode0) == 0xFFFF);
    assert(memory_bus.read16(shadowrun::ram::kControllerState1) == 0x00FF);
    assert(memory_bus.read16(shadowrun::ram::kInputPort1) == 0xFFFF);
    assert(memory_bus.read16(shadowrun::ram::kControllerDecode1) == 0xFFFF);
    assert(memory_bus.read8(shadowrun::genesis::kControllerControl2) == 0x40);

    for (std::size_t index = 0;
         index < shadowrun::palette_ram::kEntryCount; ++index) {
        memory_bus.write16(shadowrun::palette_ram::kCurrent + index * 2U, 0);
        memory_bus.write16(shadowrun::palette_ram::kTarget + index * 2U,
                           0x0EEE);
    }
    memory_bus.write8(shadowrun::ram::kVblankFlags, 0);
    std::size_t palette_waits = 0;
    assert(memory_runtime.service_frame_input(
               1, [&]() { ++palette_waits; }) == 2);
    assert(palette_waits == 2);
    const auto target_transition = memory_runtime.fade_palette_to_target(
        [&]() { ++palette_waits; });
    assert(target_transition.palette_steps == 17);
    assert(target_transition.frame_waits == 34);
    assert(palette_waits == 36);
    assert(memory_bus.read16(shadowrun::palette_ram::kCurrent) == 0x0EEE);
    assert((memory_bus.read8(shadowrun::ram::kVblankFlags) & 0x01U) != 0);

    memory_bus.write8(shadowrun::ram::kVblankFlags, 0);
    const auto black_transition = memory_runtime.fade_palette_to_black(
        [&]() { ++palette_waits; });
    assert(black_transition.palette_steps == 17);
    assert(black_transition.frame_waits == 34);
    assert(palette_waits == 70);
    assert(memory_bus.read16(shadowrun::palette_ram::kCurrent) == 0);
    assert((memory_bus.read8(shadowrun::ram::kVblankFlags) & 0x01U) != 0);

    memory_bus.write16(shadowrun::ram::kVblankTick, 4);
    memory_bus.write16(shadowrun::ram::kFrameSubcounter, 0x1234);
    memory_bus.write8(shadowrun::ram::kVblankFlags, 0x02);
    memory_bus.write16(shadowrun::ram::kInputStatus, 0);
    bool flag_two_called = false;
    shadowrun::VBlankRoutineHooks vblank_hooks;
    vblank_hooks.routine_0x0EE6 = [&](shadowrun::GenesisBus& callback_bus) {
        flag_two_called = true;
        assert((callback_bus.read8(shadowrun::ram::kVblankFlags) & 0x02U) == 0);
    };
    const auto vblank_result = memory_runtime.service_vblank(vblank_hooks);
    assert(vblank_result.tick == 5);
    assert(vblank_result.flags_seen == 0x02);
    assert(flag_two_called);
    assert(memory_bus.read16(shadowrun::ram::kFrameSubcounter) == 0);
    assert(memory_bus.read16(shadowrun::ram::kInputStatus) == 1);
    assert((memory_bus.read8(shadowrun::ram::kVblankFlags) & 0x02U) == 0);

    for (std::size_t index = 0; index < 64; ++index) {
        memory_bus.write16(shadowrun::ram::kVdpUploadBuffer + index * 2U,
                           static_cast<std::uint16_t>(index));
    }
    memory_bus.clear_port_writes();
    memory_runtime.upload_vdp_word_buffer();
    assert(memory_bus.port_writes().size() == 64U * 3U);
    assert(memory_bus.port_writes()[0].address == shadowrun::genesis::kVdpData);
    assert(memory_bus.port_writes()[0].value == 0);
    assert(memory_bus.port_writes()[1].address == shadowrun::genesis::kVdpControl);
    assert(memory_bus.port_writes()[2].value == 0);
    assert(memory_bus.port_writes().back().value == 63);

    memory_bus.write16(shadowrun::ram::kFrameSubcounter, 0);
    memory_bus.write16(shadowrun::ram::kDisplayBufferOdd, 0x1234);
    memory_bus.write16(shadowrun::ram::kDisplayBufferOdd + 2U, 0x5678);
    memory_bus.clear_port_writes();
    memory_runtime.service_display_interrupt();
    assert(memory_bus.port_writes().size() == 5);
    assert(memory_bus.port_writes()[0].width == 32);
    assert(memory_bus.port_writes()[0].value == 0xC05E0000U);
    assert(memory_bus.port_writes()[1].address ==
           shadowrun::genesis::kVdpStatusByte);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0x1234);
    assert(memory_bus.port_writes()[3].address ==
           shadowrun::genesis::kVdpStatusByte);
    assert(memory_bus.port_writes()[4].value == 0x5678);

    memory_bus.write8(shadowrun::ram::kVblankAnimationLock, 0);
    memory_bus.write16(shadowrun::ram::kVblankAnimationState, 0);
    memory_bus.write16(shadowrun::ram::kVblankAnimationMode, 0);
    memory_bus.write16(shadowrun::ram::kVblankAnimationTimer, 0);
    memory_bus.write16(shadowrun::ram::kVblankAnimationIndex, 0);
    memory_bus.clear_port_writes();
    memory_runtime.service_vblank_state_animation();
    assert(memory_bus.read16(shadowrun::ram::kVblankAnimationTimer) == 6);
    assert(memory_bus.read16(shadowrun::ram::kVblankAnimationIndex) == 1);
    assert(memory_bus.port_writes().size() == 3);
    assert(memory_bus.port_writes()[0].value == 0xC014);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0);

    constexpr shadowrun::GenesisAddress upload_object = 0xFF7100U;
    constexpr shadowrun::GenesisAddress upload_source = 0xFF7200U;
    for (std::size_t index = 0; index < 70; ++index) {
        memory_bus.write8(upload_source + index,
                          static_cast<std::uint8_t>(0x80U + index));
    }
    memory_bus.write32(upload_object + 0x22U, upload_source);
    memory_bus.write8(upload_object + 0x11U, 0);
    memory_bus.write16(upload_object + 0x26U, 0x2345);
    memory_bus.write16(upload_object + 0x28U, 0x0046);
    memory_bus.write32(0xFF7000U, upload_object);
    memory_bus.write16(shadowrun::ram::kVblankWorkA, 1);
    memory_bus.write32(shadowrun::ram::kVblankWorkAPointer, 0xFF7004U);
    memory_bus.clear_port_writes();
    memory_runtime.service_vblank_upload_queue();
    assert(memory_bus.read16(shadowrun::ram::kVblankWorkA) == 0);
    assert(memory_bus.port_writes().size() == 5);
    assert(memory_bus.port_writes()[0].value == 0x6345);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0x80818283U);

    constexpr shadowrun::GenesisAddress descriptor_source = 0xFF7400U;
    for (std::size_t index = 0; index < 32; ++index) {
        memory_bus.write8(descriptor_source + index,
                          static_cast<std::uint8_t>(0x20U + index));
    }
    memory_bus.write32(0xFF7300U, descriptor_source);
    memory_bus.write16(0xFF7304U, 0x0020);
    memory_bus.write16(0xFF7306U, 0x3456);
    memory_bus.write16(shadowrun::ram::kVblankWorkB, 1);
    memory_bus.write32(shadowrun::ram::kVblankWorkBPointer, 0xFF7308U);
    memory_bus.clear_port_writes();
    memory_runtime.service_vblank_descriptor_queue();
    assert(memory_bus.read16(shadowrun::ram::kVblankWorkB) == 0);
    assert(memory_bus.port_writes().size() == 10);
    assert(memory_bus.port_writes()[0].value == 0x7456);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0x20212223U);

    for (std::size_t index = 0; index < 8; ++index) {
        memory_bus.write8(shadowrun::ram::kObjectListScratch + index,
                          static_cast<std::uint8_t>(0x60U + index));
    }
    memory_bus.write16(shadowrun::ram::kObjectListCount, 0);
    memory_bus.write16(shadowrun::ram::kObjectListVdpCommand, 0x4567);
    memory_bus.clear_port_writes();
    memory_runtime.service_object_list_upload();
    assert(memory_bus.port_writes().size() == 4);
    assert(memory_bus.port_writes()[0].value == 0x4567);
    assert(memory_bus.port_writes()[1].value == 1);
    assert(memory_bus.port_writes()[2].value == 0x60616263U);
    assert(memory_bus.port_writes()[3].value == 0x64656667U);

    memory_bus.write16(shadowrun::ram::kVdpDataWord, 0);
    memory_bus.write16(shadowrun::ram::kVdpRegisterWord0A, 0);
    memory_bus.write16(shadowrun::ram::kCameraX, 0);
    memory_bus.write16(shadowrun::ram::kCameraY, 0);
    memory_bus.write16(shadowrun::ram::kTilePlaneWidth, 0x0020);
    memory_bus.write16(shadowrun::ram::kTilePlaneMask, 0);
    for (std::size_t index = 0; index < 6; ++index) {
        memory_bus.write16(0xFF270AU + index * 2U, 0);
    }
    for (std::size_t index = 0; index < 40; ++index) {
        memory_bus.write16(0xFF8000U + index * 2U,
                           static_cast<std::uint16_t>(0xB000U + index));
    }
    memory_bus.clear_port_writes();
    memory_runtime.stream_tile_plane(0xFF8000U, 0xFF2700U, 0x4000U);
    assert(memory_bus.port_writes().size() == 31U * 46U);
    assert(memory_bus.port_writes()[0].value == 0x4000);
    assert(memory_bus.port_writes()[1].value == 1);
    assert(memory_bus.port_writes()[2].value == 0xB005);
    assert(memory_bus.port_writes()[9].value == 0xB004);
    assert(memory_bus.port_writes()[10].value == 0xB005);
    assert(memory_bus.port_writes()[46].value == 0x4080);
    assert(memory_bus.port_writes()[47].value == 1);

    memory_bus.write32(shadowrun::ram::kTilePlaneSourceA, 0xFF8000U);
    memory_bus.write32(shadowrun::ram::kTilePlaneSourceB, 0xFF9000U);
    memory_bus.write16(shadowrun::ram::kVdpClearCommandA, 0xE000U);
    memory_bus.write16(shadowrun::ram::kVdpClearCommandB, 0x4000U);
    memory_bus.write16(shadowrun::ram::kVdpRegisterWord0E, 0x0200U);
    memory_bus.write16(shadowrun::ram::kCameraY, 0x0100U);
    memory_bus.clear_port_writes();
    const auto vertical_tile_refresh =
        memory_runtime.stream_frame_tile_plane_vertical_0x4018(0);
    assert(vertical_tile_refresh.early_return);
    assert(vertical_tile_refresh.draw_calls == 6);
    assert(vertical_tile_refresh.words_emitted == 264);
    assert(memory_bus.port_writes().size() == 6U * (2U + 44U));

    memory_bus.write16(shadowrun::ram::kCameraX, 0x0100U);
    memory_bus.write16(shadowrun::ram::kCameraY, 0);
    memory_bus.clear_port_writes();
    const auto horizontal_tile_refresh =
        memory_runtime.stream_frame_tile_plane_horizontal_0x40E2(0);
    assert(!horizontal_tile_refresh.early_return);
    assert(horizontal_tile_refresh.draw_calls == 2);
    assert(horizontal_tile_refresh.words_emitted == 64);
    assert(memory_bus.port_writes().size() == 2U * (1U + 2U + 32U + 1U));

    memory_bus.write16(shadowrun::ram::kVdpAddressWord, 0x1234);
    memory_bus.write16(shadowrun::ram::kVdpDataWord, 0xABCD);
    memory_bus.clear_port_writes();
    memory_runtime.write_vdp_addressed_word();
    assert(memory_bus.port_writes().size() == 3);
    assert(memory_bus.port_writes()[0].value == 0x5234);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0xABCD);

    memory_bus.write16(shadowrun::ram::kVdpRegisterWord0A, 0xAAAA);
    memory_bus.write16(shadowrun::ram::kVdpRegisterWord0E, 0xBBBB);
    memory_bus.clear_port_writes();
    memory_runtime.write_vdp_register_pair();
    assert(memory_bus.port_writes().size() == 3);
    assert(memory_bus.port_writes()[0].value == 0x40000010U);
    assert(memory_bus.port_writes()[1].value == 0xAAAA);
    assert(memory_bus.port_writes()[2].value == 0xBBBB);

    for (std::size_t index = 0; index < 4; ++index) {
        memory_bus.write16(0xFF3000U + index * 2U,
                           static_cast<std::uint16_t>(index + 1U));
    }
    memory_bus.clear_port_writes();
    memory_runtime.stream_vdp_grid(0xFF3000U, 0x1234U, 1, 1, 0x0010);
    assert(memory_bus.port_writes().size() == 8);
    assert(memory_bus.port_writes()[0].value == 0x5234);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0x0011);
    assert(memory_bus.port_writes()[3].value == 0x0012);
    assert(memory_bus.port_writes()[4].value == 0x52B4);
    assert(memory_bus.port_writes()[5].value == 0);
    assert(memory_bus.port_writes()[6].value == 0x0013);
    assert(memory_bus.port_writes()[7].value == 0x0014);

    const std::array<std::uint8_t, 11> delta_source = {
        0x10, 0x00,       // D6 base word
        0x60, 0x00, 0x10, // one wide normal token: 0x0010 + base
        0xE2, 0x00, 0x20, 0x00, 0x30, // two wide special values
        0x00};
    for (std::size_t index = 0; index < delta_source.size(); ++index) {
        memory_bus.write8(0xFF4000U + index, delta_source[index]);
    }
    memory_bus.clear_port_writes();
    const auto delta_result = memory_runtime.stream_delta_vdp(
        0xFF4000U, 0x2000U, 2, 3, 0, 0, 0);
    assert(delta_result.rows_completed == 1);
    assert(delta_result.source_bytes_consumed == 10);
    assert(!delta_result.terminated_by_zero);
    assert(memory_bus.port_writes().size() == 5);
    assert(memory_bus.port_writes()[0].value == 0x6000);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0x1010);
    assert(memory_bus.port_writes()[3].value == 0x1020);
    assert(memory_bus.port_writes()[4].value == 0x1030);

    for (std::size_t index = 0; index < 70; ++index) {
        memory_bus.write8(0xFF5000U + index,
                          static_cast<std::uint8_t>(index));
    }
    memory_bus.clear_port_writes();
    const auto primary_result = memory_runtime.stream_primary_vdp(
        0xFF5000U, 0x1234U, 0x0046U);
    assert(primary_result.segments == 1);
    assert(primary_result.longwords_emitted == 2);
    assert(primary_result.words_emitted == 1);
    assert(primary_result.source_bytes_consumed == 0x0046U);
    assert(!primary_result.split_at_64k_boundary);
    assert(memory_bus.port_writes().size() == 5);
    assert(memory_bus.port_writes()[0].value == 0x5234);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[2].value == 0x00010203U);
    assert(memory_bus.port_writes()[3].value == 0x40414243U);
    assert(memory_bus.port_writes()[4].value == 0x4445U);

    for (std::size_t index = 0; index < 16; ++index) {
        memory_bus.write8(0xFFFFF0U + index,
                          static_cast<std::uint8_t>(0xA0U + index));
    }
    memory_bus.clear_port_writes();
    const auto split_result = memory_runtime.stream_primary_vdp(
        0xFFFFF0U, 0x1234U, 0x0020U);
    assert(split_result.segments == 2);
    assert(split_result.longwords_emitted == 8);
    assert(split_result.words_emitted == 0);
    assert(split_result.source_bytes_consumed == 0x0020U);
    assert(split_result.split_at_64k_boundary);
    assert(memory_bus.port_writes().size() == 12);
    assert(memory_bus.port_writes()[0].value == 0x5234);
    assert(memory_bus.port_writes()[1].value == 0);
    assert(memory_bus.port_writes()[6].value == 0x5244);
    assert(memory_bus.port_writes()[7].value == 0);

    const std::array<std::uint8_t, 5> compressed_run = {
        0x00, 0x02, // initial DBF counter
        0x80, 0x00, 0xAB // compressed run: three copies of 0xAB
    };
    for (std::size_t index = 0; index < compressed_run.size(); ++index) {
        memory_bus.write8(0xFF6000U + index, compressed_run[index]);
    }
    memory_bus.clear_port_writes();
    const auto compressed_result = memory_runtime.decode_compressed_vdp(
        0xFF6000U, 0xFF9700U, 0x1234U);
    assert(compressed_result.output_bytes == 3);
    assert(compressed_result.source_bytes_consumed == 5);
    assert(compressed_result.initial_counter == 2);
    assert(compressed_result.returned_word == 3);
    assert(memory_bus.read8(0xFF9700U) == 0xAB);
    assert(memory_bus.read8(0xFF9702U) == 0xAB);
    assert(memory_bus.port_writes().size() == 6);
    assert(memory_bus.port_writes()[0].value == 0x8F02);
    assert(memory_bus.port_writes()[1].value == 0x5234);
    assert(memory_bus.port_writes()[2].value == 0);
    assert(memory_bus.port_writes()[3].value == 0xAB);
    assert(memory_bus.port_writes()[5].value == 0xAB);

    const std::array<std::uint8_t, 6> compressed_backref = {
        0x00, 0x03, // initial DBF counter
        0x40, 0x41, // literal 0x41, then a compressed token
        0x90, 0x00 // offset one, three overlapping copies
    };
    for (std::size_t index = 0; index < compressed_backref.size(); ++index) {
        memory_bus.write8(0xFF6100U + index, compressed_backref[index]);
    }
    memory_bus.clear_port_writes();
    const auto backref_result = memory_runtime.decode_compressed_vdp(
        0xFF6100U, 0xFF9800U, 0x2234U);
    assert(backref_result.output_bytes == 4);
    assert(backref_result.source_bytes_consumed == 6);
    assert(memory_bus.read8(0xFF9800U) == 0x41);
    assert(memory_bus.read8(0xFF9803U) == 0x41);
    assert(memory_bus.port_writes().size() == 7);
    assert(memory_bus.port_writes()[0].value == 0x8F02);
    assert(memory_bus.port_writes()[1].value == 0x6234);
    assert(memory_bus.port_writes()[2].value == 0);
    assert(memory_bus.port_writes()[3].value == 0x4100);
    assert(memory_bus.port_writes()[6].value == 0x4100);

    memory_bus.write16(shadowrun::ram::kObjectListVdpCommand, 0xC000);
    memory_bus.clear_port_writes();
    shadowrun::initialise_object_output(memory_bus);
    assert(memory_bus.port_writes().size() == 4);
    assert(memory_bus.port_writes()[0].address ==
           shadowrun::genesis::kVdpControl);
    assert(memory_bus.port_writes()[0].value == 0x4000);
    assert(memory_bus.port_writes()[1].address ==
           shadowrun::genesis::kVdpControl);
    assert(memory_bus.port_writes()[1].value == 3);
    assert(memory_bus.port_writes()[2].address ==
           shadowrun::genesis::kVdpData);
    assert(memory_bus.port_writes()[2].width == 32);
    assert(memory_bus.port_writes()[3].value == 0);
    assert(memory_bus.read32(shadowrun::ram::kObjectListOutputPointer) ==
           shadowrun::ram::kObjectListInitialOutput);
    assert(memory_bus.read16(shadowrun::ram::kObjectListCountMirror) == 1);
    assert(memory_bus.read16(shadowrun::ram::kObjectListScratch) == 0);

    memory_bus.write16(shadowrun::ram::kVdpClearCommandA, 0xC000);
    memory_bus.clear_port_writes();
    memory_runtime.clear_vdp_block_a();
    assert(memory_bus.port_writes().size() == 1044);
    assert(memory_bus.port_writes()[0].address ==
           shadowrun::genesis::kVdpControl);
    assert(memory_bus.port_writes()[0].value == 0x7FE0);
    assert(memory_bus.port_writes()[1].address ==
           shadowrun::genesis::kVdpControl);
    assert(memory_bus.port_writes()[1].value == 3);
    assert(memory_bus.port_writes()[2].address ==
           shadowrun::genesis::kVdpData);
    assert(memory_bus.port_writes()[17].address ==
           shadowrun::genesis::kVdpData);
    assert(memory_bus.port_writes()[18].address ==
           shadowrun::genesis::kVdpControl);
    assert(memory_bus.port_writes()[18].value == 0x4000);
    assert(memory_bus.port_writes()[19].value == 3);
    assert(memory_bus.port_writes().back().value == 0x07FF07FFU);

    memory_bus.write16(shadowrun::ram::kVdpClearCommandB, 0xE000);
    memory_bus.clear_port_writes();
    memory_runtime.clear_vdp_block_b();
    assert(memory_bus.port_writes().size() == 1044);
    assert(memory_bus.port_writes()[18].value == 0x6000);
    assert(memory_bus.port_writes()[19].value == 3);

    shadowrun::GenesisMemoryBus resource_bus({0, 0, 0, 0});
    constexpr shadowrun::GenesisAddress setup_block = 0xFF1000;
    constexpr shadowrun::GenesisAddress config_source = 0xFF2000;
    constexpr shadowrun::GenesisAddress palette_source = 0xFF2100;
    resource_bus.write32(setup_block, config_source);
    resource_bus.write32(setup_block + 4U, palette_source);
    resource_bus.write16(setup_block + 8U, 0x0101);
    resource_bus.write16(setup_block + 10U, 0x0202);
    resource_bus.write16(setup_block + 12U, 0x0303);
    resource_bus.write16(setup_block + 14U, 0x0404);
    for (std::size_t index = 0; index < shadowrun::rom::kVdpConfiguration.size();
         ++index) {
        resource_bus.write16(config_source + index * 2U,
                             shadowrun::rom::kVdpConfiguration[index]);
    }
    for (std::size_t index = 0; index < 32; ++index) {
        resource_bus.write32(palette_source + index * 4U,
                             static_cast<std::uint32_t>(0xB0000000U + index));
    }
    const auto primary_list = setup_block + 0x10U;
    resource_bus.write32(primary_list, 0xFF2300U);
    resource_bus.write32(primary_list + 4U, 0x00005678U);
    resource_bus.write32(primary_list + 8U, 0x00000040U);
    resource_bus.write32(primary_list + 12U, 0xFF2301U);
    resource_bus.write32(primary_list + 16U, 0x00001234U);
    resource_bus.write32(primary_list + 20U, 0xCAFEBABEU);
    resource_bus.write32(primary_list + 24U, 0);
    const auto secondary_list = primary_list + 28U;
    resource_bus.write32(secondary_list, 0x00010020U);
    resource_bus.write32(secondary_list + 4U, 0x12345678U);
    resource_bus.write16(secondary_list + 8U, 0x0020);
    resource_bus.write16(secondary_list + 10U, 0x1234);
    resource_bus.write16(secondary_list + 12U, 0x5678);
    resource_bus.write32(secondary_list + 14U, 0);

    std::size_t primary_0f5c_calls = 0;
    std::size_t primary_1530_calls = 0;
    std::size_t secondary_1232_calls = 0;
    std::size_t secondary_1728_calls = 0;
    std::size_t resource_waits = 0;
    shadowrun::ResourceSetupHooks resource_hooks;
    resource_hooks.frame_wait = [&]() { ++resource_waits; };
    resource_hooks.routine_0x0F5C =
        [&](shadowrun::GenesisBus&,
            const shadowrun::PrimaryResourceStream&) {
            ++primary_0f5c_calls;
        };
    resource_hooks.routine_0x1530 =
        [&](shadowrun::GenesisBus&,
            const shadowrun::PrimaryResourceStream& stream) {
            ++primary_1530_calls;
            assert(stream.raw_source == 0xFF2301U);
            assert(stream.source == 0xFF2300U);
            assert(stream.d0_word == 0x1234);
            assert(stream.staging_address == 0xFF9700U);
        };
    resource_hooks.routine_0x1232 =
        [&](shadowrun::GenesisBus&,
            const shadowrun::SecondaryResourceStream&) {
            ++secondary_1232_calls;
        };
    resource_hooks.routine_0x1728 =
        [&](shadowrun::GenesisBus&,
            const shadowrun::SecondaryResourceStream& stream) {
            ++secondary_1728_calls;
            assert(stream.raw_source == 0x00010020U);
            assert(stream.source_before_bit_clear == 0x00010020U);
            assert(stream.source_after_bit_clear == 0x00000020U);
            assert(stream.d3_word == 0);
            assert(stream.d5_word == 0x0021);
            assert(stream.bit16_was_set);
        };
    const auto resource_result = shadowrun::run_resource_setup(
        resource_bus, setup_block, resource_hooks);
    assert(resource_result.primary_streams == 2);
    assert(resource_result.secondary_streams == 1);
    assert(resource_result.secondary_list_end == secondary_list + 18U);
    assert(resource_result.flags_after == 0x0D);
    assert(resource_result.frame_waits == 35);
    assert(resource_waits == 35);
    assert(primary_0f5c_calls == 1);
    assert(primary_1530_calls == 1);
    assert(secondary_1232_calls == 0);
    assert(secondary_1728_calls == 1);
    assert(resource_bus.read16(shadowrun::ram::kVdpDataWord) == 0x0101);
    assert(resource_bus.read16(shadowrun::ram::kVdpDataWord2) == 0x0202);
    assert(resource_bus.read16(shadowrun::ram::kVdpRegisterWord0A) == 0x0303);
    assert(resource_bus.read16(shadowrun::ram::kVdpRegisterWord0E) == 0x0404);
    assert(resource_bus.read32(shadowrun::palette_ram::kTarget) ==
           0xB0000000U);
    assert(resource_bus.read32(shadowrun::palette_ram::kTarget + 0x7CU) ==
           0xB000001FU);
    assert(resource_bus.read8(shadowrun::ram::kVblankFlags) == 0x0D);

    memory_bus.write16(shadowrun::ram::kFrameCounter, 0);
    memory_bus.write16(shadowrun::ram::kFrameObserved, 0);
    memory_bus.write16(shadowrun::ram::kFrameTarget, 1);
    memory_bus.write8(shadowrun::ram::kVblankFlags, 0);
    std::size_t update_calls = 0;
    std::size_t render_calls = 0;
    std::size_t entity_calls = 0;
    bool transition_called = false;
    shadowrun::FrameWindowHooks frame_hooks;
    frame_hooks.routine_0xA8EA = [&](shadowrun::GenesisBus&) {
        ++update_calls;
    };
    frame_hooks.routine_0x3A1C = [&](shadowrun::GenesisBus&) {
        ++render_calls;
    };
    frame_hooks.routine_0x4618 = [&](shadowrun::GenesisBus&) {
        ++entity_calls;
    };
    frame_hooks.routine_0x12F6 = [&](shadowrun::GenesisBus&) {
        transition_called = true;
    };
    shadowrun::ShadowrunMainState memory_main(memory_bus);
    const auto frame_result = memory_main.run_frame_window(frame_hooks);
    assert(frame_result.iterations == 31);
    assert(update_calls == 31);
    assert(render_calls == 31);
    assert(entity_calls == 31);
    assert(transition_called);
    assert(memory_bus.read8(shadowrun::session_ram::kFrameActive) == 0);
    assert(memory_bus.read16(shadowrun::session_ram::kMainLoopControl) == 0);
    assert(memory_bus.read16(shadowrun::session_ram::kFrameTransition) == 0);
    assert(memory_bus.read16(shadowrun::ram::kFrameObserved) == 1);

    const auto entity_record = shadowrun::session_ram::kEntityPhaseBase;
    memory_bus.write16(shadowrun::ram::kCameraX, 0x1000);
    memory_bus.write16(shadowrun::ram::kCameraY, 0x1000);
    memory_bus.write16(entity_record + 0x00U, 0x1800);
    memory_bus.write16(entity_record + 0x04U, 0x1F00);
    memory_bus.write16(entity_record + 0x08U, 0);
    memory_bus.write32(entity_record + 0x3AU, 0);
    memory_bus.write8(entity_record + 0x10U, 1);
    memory_bus.write8(entity_record + 0x11U, 1);
    memory_bus.write16(entity_record + 0x18U, 1);
    memory_bus.write16(entity_record + 0x1AU, 2);
    memory_bus.write16(entity_record + 0x1CU, 5);
    memory_bus.write16(entity_record + 0x1EU, 0);
    std::size_t animation_calls = 0;
    std::size_t object_post_passes = 0;
    shadowrun::FrameWindowHooks entity_hooks;
    entity_hooks.routine_0x600E = [&](shadowrun::GenesisBus&,
                                      shadowrun::GenesisAddress address) {
        assert(address == entity_record);
        ++animation_calls;
    };
    entity_hooks.routine_0x4658 = [&](shadowrun::GenesisBus&) {
        ++object_post_passes;
    };
    const auto entity_result = memory_main.run_entity_phase(entity_hooks);
    assert(entity_result.slots_examined == 16);
    assert(entity_result.active_slots == 1);
    assert(entity_result.projected_slots == 1);
    assert(entity_result.unresolved_0x600E_calls == 0);
    assert(!entity_result.unresolved_0x4658_call);
    assert(animation_calls == 1);
    assert(object_post_passes == 1);
    assert(memory_bus.read16(entity_record + 0x0AU) == 0x0100);
    assert(memory_bus.read16(entity_record + 0x0CU) == 0x0170);
    assert(memory_bus.read16(entity_record + 0x1EU) == 1);
    assert(memory_bus.read16(shadowrun::session_ram::kEntityTimingWord) == 0);
    assert(memory_bus.read16(shadowrun::session_ram::kEntityTimingAccumulator) ==
           0xFFFE);
    assert((memory_bus.read8(entity_record + 0x11U) & 1U) == 0);
    assert(memory_bus.read16(shadowrun::session_ram::kEntityPhaseState) == 0);

    std::vector<std::uint8_t> entity_state_rom(0x62B0, 0);
    entity_state_rom[0] = 0x12;
    entity_state_rom[1] = 0x34;
    entity_state_rom[2] = 0x56;
    entity_state_rom[3] = 0x78;
    entity_state_rom[shadowrun::rom::kEntityStatePointerTable + 0] = 0x00;
    entity_state_rom[shadowrun::rom::kEntityStatePointerTable + 1] = 0xFF;
    entity_state_rom[shadowrun::rom::kEntityStatePointerTable + 2] = 0x80;
    entity_state_rom[shadowrun::rom::kEntityStatePointerTable + 3] = 0x00;
    for (std::uint8_t index = 0; index < 0x10U; ++index) {
        entity_state_rom[shadowrun::rom::kPackedSpriteNibbleTable + index] =
            index;
    }
    shadowrun::GenesisMemoryBus entity_state_bus(std::move(entity_state_rom));
    const auto state_record = shadowrun::session_ram::kEntityPhaseBase;
    entity_state_bus.write16(shadowrun::ram::kCameraX, 0x1000);
    entity_state_bus.write16(shadowrun::ram::kCameraY, 0x1000);
    entity_state_bus.write16(state_record + 0x00U, 0x1800);
    entity_state_bus.write16(state_record + 0x04U, 0x1F00);
    entity_state_bus.write16(state_record + 0x08U, 0);
    entity_state_bus.write8(state_record + 0x10U, 1);
    entity_state_bus.write8(state_record + 0x11U, 1);
    entity_state_bus.write16(state_record + 0x0EU, 0);
    entity_state_bus.write16(state_record + 0x16U, 0);
    entity_state_bus.write16(state_record + 0x1AU, 0);
    entity_state_bus.write16(state_record + 0x1CU, 0);
    entity_state_bus.write16(state_record + 0x1EU, 0);
    entity_state_bus.write32(0xFF8000, 0xFF8100);
    entity_state_bus.write16(0xFF8000 + 0x00CAU, 0x0010);
    entity_state_bus.write16(0xFF8100, 0);
    entity_state_bus.write32(0xFF8104, 0xFF8200);
    entity_state_bus.write16(0xFF8200, 0x1234);
    shadowrun::ShadowrunMainState entity_state_main(entity_state_bus);
    const auto direct_state_result = entity_state_main.run_entity_phase();
    assert(direct_state_result.active_slots == 1);
    assert(direct_state_result.projected_slots == 1);
    assert(direct_state_result.unresolved_0x600E_calls == 0);
    assert(direct_state_result.unresolved_0x60FA_calls == 0);
    assert(!direct_state_result.unresolved_0x4658_call);
    assert(entity_state_bus.read16(state_record + 0x18U) == 0x001C);
    assert(entity_state_bus.read32(state_record + 0x12U) == 0xFF8200);

    entity_state_bus.write16(state_record + 0x16U, 0);
    entity_state_bus.write16(state_record + 0x1AU, 5);
    entity_state_bus.write16(state_record + 0x1CU, 0);
    entity_state_bus.write16(state_record + 0x1EU, 2);
    entity_state_bus.write8(state_record + 0x11U, 1);
    entity_state_bus.write16(0xFF8100, 0x8001);
    entity_state_bus.write16(0xFF8104, 0x0042);
    entity_state_bus.write32(0xFF810A, 0xFF8300);
    entity_state_bus.write16(0xFF8300, 0x8002);
    std::size_t nested_state_calls = 0;
    shadowrun::FrameWindowHooks nested_state_hooks;
    nested_state_hooks.routine_0x60FA =
        [&](shadowrun::GenesisBus&, shadowrun::GenesisAddress address) {
            assert(address == state_record);
            ++nested_state_calls;
        };
    const auto negative_state_result =
        entity_state_main.run_entity_phase(nested_state_hooks);
    assert(negative_state_result.active_slots == 1);
    assert(negative_state_result.unresolved_0x600E_calls == 0);
    assert(negative_state_result.unresolved_0x60FA_calls == 0);
    assert(nested_state_calls == 1);
    assert(entity_state_bus.read16(state_record + 0x16U) == 0x8000);
    assert(entity_state_bus.read16(state_record + 0x1EU) == 2);
    assert(entity_state_bus.read16(state_record + 0x18U) == 0x001C);
    assert(entity_state_bus.read32(state_record + 0x12U) == 0xFF8300);

    entity_state_bus.write8(state_record + 0x11U, 0xA1U);
    entity_state_bus.write32(0xFF8302U, 0xFF8400U);
    entity_state_bus.write16(0xFF8306U, 2);
    entity_state_bus.write8(0xFF8400U, 0x12U);
    entity_state_bus.write8(0xFF8401U, 0xF0U);
    entity_state_bus.write32(state_record + 0x48U, 0xFF8500U);
    entity_state_bus.write32(shadowrun::ram::kVblankWorkAPointer,
                             0xFF8600U);
    entity_state_bus.write16(shadowrun::ram::kVblankWorkA, 0);
    const auto direct_frame_result = entity_state_main.run_entity_phase();
    assert(direct_frame_result.active_slots == 1);
    assert(direct_frame_result.unresolved_0x600E_calls == 0);
    assert(direct_frame_result.unresolved_0x60FA_calls == 0);
    assert(entity_state_bus.read32(state_record + 0x22U) == 0xFF8400U);
    assert(entity_state_bus.read16(state_record + 0x28U) == 2);
    assert(entity_state_bus.read32(0xFF8600U) == state_record);
    assert(entity_state_bus.read16(shadowrun::ram::kVblankWorkA) == 1);
    assert(entity_state_bus.read8(0xFF8500U) == 0x12U);
    assert(entity_state_bus.read8(0xFF8501U) == 0xF0U);

    memory_bus.write16(shadowrun::ram::kFrameCounter, 0);
    memory_bus.write16(shadowrun::ram::kFrameObserved, 0);
    memory_bus.write16(shadowrun::ram::kFrameTarget, 1);
    memory_bus.write8(shadowrun::ram::kVblankFlags, 0);
    std::size_t default_transition_waits = 0;
    shadowrun::FrameWindowHooks default_frame_hooks;
    default_frame_hooks.frame_wait = [&]() {
        ++default_transition_waits;
        memory_bus.write16(shadowrun::ram::kFrameObserved,
                           memory_bus.read16(shadowrun::ram::kFrameTarget));
    };
    const auto default_frame_result =
        memory_main.run_frame_window(default_frame_hooks);
    assert(default_frame_result.iterations == 31);
    assert(default_transition_waits == 34);
    assert((memory_bus.read8(shadowrun::ram::kVblankFlags) & 1U) != 0);

    const auto sort_table = shadowrun::session_ram::kEntityPointerTable;
    const std::array<shadowrun::GenesisAddress, 16> sort_records{
        0xFF2000U, 0xFF2100U, 0xFF2200U, 0xFF2300U,
        0xFF2400U, 0xFF2500U, 0xFF2600U, 0xFF2700U,
        0xFF2800U, 0xFF2900U, 0xFF2A00U, 0xFF2B00U,
        0xFF2C00U, 0xFF2D00U, 0xFF2E00U, 0xFF2F00U};
    const std::array<std::int16_t, 16> sort_values{
        2, 9, -3, 9, 1, 0, 7, -8,
        4, 6, 3, -1, 5, 8, -2, 0};
    for (std::size_t index = 0; index < sort_records.size(); ++index) {
        memory_bus.write32(sort_table + index * 4U, sort_records[index]);
        memory_bus.write16(sort_records[index] + 4U,
                           static_cast<std::uint16_t>(sort_values[index]));
        memory_bus.write8(sort_records[index] + 0x10U, 0);
    }
    memory_bus.write8(sort_records[6] + 0x10U, 0x10);
    const auto sort_result =
        shadowrun::sort_entity_pointer_table(memory_bus);
    assert(sort_result.sorted);
    assert(sort_result.passes > 1);
    assert(sort_result.comparisons == sort_result.passes * 15);
    assert(sort_result.swaps > 0);
    std::int16_t previous_sort_value = 0x7FFF;
    for (std::size_t index = 0; index < sort_records.size(); ++index) {
        const auto record = memory_bus.read32(sort_table + index * 4U);
        const auto value = (memory_bus.read8(record + 0x10U) & 0x50U) != 0
                               ? std::int16_t{0}
                               : static_cast<std::int16_t>(
                                     memory_bus.read16(record + 4U));
        assert(previous_sort_value >= value);
        previous_sort_value = value;
    }

    const auto output_entity = 0xFF3000U;
    const auto output_state_list = 0xFF8800U;
    const auto output_entity_record = 0xFF3100U;
    memory_bus.write32(output_entity_record + 0x12U, output_state_list);
    memory_bus.write16(output_entity_record + 0x0AU, 0x0010);
    memory_bus.write16(output_entity_record + 0x0CU, 0x0020);
    memory_bus.write8(output_entity_record + 0x11U, 0);
    memory_bus.write16(output_entity_record + 0x20U, 0x0020);
    memory_bus.write16(output_entity_record + 0x2CU, 0x0004);
    memory_bus.write16(output_state_list, 0x8002);
    memory_bus.write16(output_state_list + 8U, 1);
    memory_bus.write16(output_state_list + 10U, 2);
    memory_bus.write16(output_state_list + 12U, 0x0034);
    memory_bus.write16(output_state_list + 14U, 0x0100);
    memory_bus.write16(output_state_list + 16U, 0x8000);
    memory_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto entity_output_result = shadowrun::append_entity_state_objects(
        memory_bus, output_entity_record, output_entity, 2, 4);
    assert(entity_output_result.emitted == 1);
    assert(entity_output_result.next_output_address == output_entity + 8U);
    assert(entity_output_result.next_count == 2);
    assert(entity_output_result.next_ordinal == 3);
    assert(memory_bus.read16(output_entity) == 0x0022);
    assert(memory_bus.read8(output_entity + 2U) == 0x34);
    assert(memory_bus.read8(output_entity + 3U) == 2);
    assert(memory_bus.read16(output_entity + 4U) == 0x0124);
    assert(memory_bus.read16(output_entity + 6U) == 0x0011);
    assert(memory_bus.read32(shadowrun::ram::kObjectListOutputPointer) ==
           output_entity + 8U);
    assert(memory_bus.read16(shadowrun::ram::kObjectListCountMirror) == 2);

    std::vector<std::uint8_t> generation_rom(0x16000, 0);
    const auto generation_write16 =
        [&generation_rom](std::size_t address, std::uint16_t value) {
            generation_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            generation_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto generation_write32 =
        [&generation_rom](std::size_t address, std::uint32_t value) {
            generation_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            generation_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            generation_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            generation_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    generation_write32(shadowrun::rom::kObjectGenerationPointerTable, 0x0000AF4EU);
    generation_write16(0xAF4EU, 0xFFFCU);
    generation_write16(0xAF50U, 0xFFFCU);
    generation_write16(0xAF52U, 0);
    generation_write16(0xAF54U, 0x05E0U);
    generation_write16(0xAF56U, 0x8000U);
    shadowrun::GenesisMemoryBus generation_bus(std::move(generation_rom));
    generation_bus.write16(shadowrun::ram::kObjectGenerationProgress, 0x0090);
    generation_bus.write16(shadowrun::ram::kObjectGenerationReset, 1);
    generation_bus.write16(shadowrun::ram::kFrameCounter, 0);
    generation_bus.write16(shadowrun::ram::kObjectGenerationBaseX, 0x1000);
    generation_bus.write16(shadowrun::ram::kObjectGenerationBaseY, 0x2000);
    generation_bus.write16(shadowrun::ram::kObjectListCount, 1);
    generation_bus.write32(shadowrun::ram::kObjectListOutputPointer, 0xFF4000U);
    const auto generated_result = shadowrun::generate_object_list_frame(
        generation_bus, 2, 2);
    assert(generated_result.generated);
    assert(generated_result.lists_processed == 32);
    assert(generated_result.objects_emitted == 32);
    assert(generated_result.progress == 0x0098);
    assert(generated_result.next_count == 33);
    assert(generated_result.next_ordinal == 34);
    assert(generation_bus.read8(0xFF4000U + 3U) == 2);
    assert(generation_bus.read16(0xFF4000U + 4U) == 0x65E0);
    assert(generation_bus.read8(0xFF4000U + 31U * 8U + 3U) == 33);
    generation_bus.write16(shadowrun::ram::kObjectGenerationProgress, 0x0098);
    generation_bus.write16(shadowrun::ram::kObjectGenerationReset, 1);
    const auto completed_generation = shadowrun::generate_object_list_frame(
        generation_bus, 34, 2);
    assert(!completed_generation.generated);
    assert(completed_generation.progress == 0x00A0);
    assert(generation_bus.read16(shadowrun::ram::kObjectGenerationReset) == 0);

    std::vector<std::uint8_t> animated_rom(0x5EF0, 0);
    const auto animated_write16 =
        [&animated_rom](std::size_t address, std::uint16_t value) {
            animated_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            animated_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto animated_write32 =
        [&animated_rom](std::size_t address, std::uint32_t value) {
            animated_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            animated_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            animated_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            animated_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    animated_write32(shadowrun::rom::kAnimatedObjectPointerTable, 0x00005EC8U);
    animated_write16(0x5EC8U, 0);
    animated_write16(0x5ECAU, 0);
    animated_write16(0x5ECCU, 0x0012U);
    animated_write16(0x5ECEU, 0x0034U);
    animated_write16(0x5ED0U, 0x8000U);
    shadowrun::GenesisMemoryBus animated_bus(std::move(animated_rom));
    const auto animated_record = 0xFF3300U;
    animated_bus.write16(animated_record + 0x38U, 0);
    animated_bus.write16(animated_record + 0x44U, 8);
    animated_bus.write16(animated_record + 0x0AU, 0x0100U);
    animated_bus.write16(animated_record + 0x0CU, 0x0200U);
    animated_bus.write16(animated_record + 0x2CU, 0x2000U);
    animated_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto animated_output = 0xFF5000U;
    const auto animated_result = shadowrun::emit_animated_entity_objects(
        animated_bus, animated_record, animated_output, 2, 2);
    assert(animated_result.emitted);
    assert(animated_result.passes == 16);
    assert(animated_result.objects_emitted == 16);
    assert(animated_result.next_count == 17);
    assert(animated_result.next_ordinal == 18);
    assert(animated_bus.read16(animated_output) == 0x0200);
    assert(animated_bus.read8(animated_output + 2U) == 0x12);
    assert(animated_bus.read8(animated_output + 3U) == 2);
    assert(animated_bus.read16(animated_output + 4U) == 0x2034);
    animated_bus.write16(animated_record + 0x38U, 0x002A);
    const auto skipped_animated = shadowrun::emit_animated_entity_objects(
        animated_bus, animated_record, animated_output, 18, 2);
    assert(!skipped_animated.emitted);

    std::vector<std::uint8_t> variant_rom(0x5F00, 0);
    const auto variant_write16 =
        [&variant_rom](std::size_t address, std::uint16_t value) {
            variant_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            variant_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto variant_write32 =
        [&variant_rom](std::size_t address, std::uint32_t value) {
            variant_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            variant_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            variant_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            variant_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    variant_write16(0x5D92U, 0x2000U);
    variant_write32(0x5D94U, 0x00005E30U);
    variant_write32(0x5DA4U, 0x00005E30U);
    variant_write16(0x5DD4U, 0x6000U);
    variant_write32(0x5DE6U, 0x00005E30U);
    variant_write16(0x5E30U, 0);
    variant_write16(0x5E32U, 0);
    variant_write16(0x5E34U, 0x0012U);
    variant_write16(0x5E36U, 0x0034U);
    variant_write16(0x5E38U, 0x8000U);
    shadowrun::GenesisMemoryBus variant_bus(std::move(variant_rom));
    const auto variant_record = 0xFF3400U;
    variant_bus.write32(shadowrun::ram::kEntityObjectControlPointer,
                        0xFF9000U);
    variant_bus.write16(0xFF9000U + 0x56U, 0);
    variant_bus.write16(0xFF9000U + 0x57U, 0);
    variant_bus.write8(0xFF9000U + 0x91U, 0);
    variant_bus.write8(variant_record + 0x4CU, 0);
    variant_bus.write8(variant_record + 0x9CU, 0);
    variant_bus.write16(variant_record + 0x0AU, 0x0100U);
    variant_bus.write16(variant_record + 0x0CU, 0x0200U);
    variant_bus.write16(variant_record + 0x0EU, 0);
    variant_bus.write16(variant_record + 0xA6U, 0);
    variant_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto variant_result = shadowrun::emit_entity_variant_objects(
        variant_bus, variant_record, 0xFF6000U, 2, 2);
    assert(variant_result.emitted);
    assert(variant_result.selector == 0);
    assert(variant_result.table_address == 0x5D92U);
    assert(variant_result.lists_processed == 1);
    assert(variant_result.objects_emitted == 1);
    assert(variant_bus.read8(0xFF9000U + 0x91U) == 0x0C);
    assert(variant_bus.read16(0xFF6000U) == 0x0200);
    assert(variant_bus.read16(0xFF6004U) == 0x2034);

    variant_bus.write16(0xFF9000U + 0x56U, 0xFFFFU);
    variant_bus.write16(0xFF9000U + 0x57U, 5);
    variant_bus.write8(variant_record + 0x9CU, 9);
    variant_bus.write16(variant_record + 0xA6U, 0xFFFFU);
    variant_bus.write16(shadowrun::ram::kObjectListCount, 2);
    const auto variant_negative_result = shadowrun::emit_entity_variant_objects(
        variant_bus, variant_record, 0xFF6008U, 3, 2);
    assert(variant_negative_result.selector == 9);
    assert(variant_negative_result.table_address == 0x5DD4U);
    assert(variant_negative_result.objects_emitted == 1);
    assert(variant_bus.read16(variant_record + 0xA6U) == 0);
    assert(variant_bus.read16(0xFF600CU) == 0x6034);

    std::vector<std::uint8_t> cycle_rom(0x5000, 0);
    const auto cycle_write16 =
        [&cycle_rom](std::size_t address, std::uint16_t value) {
            cycle_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            cycle_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto cycle_write32 =
        [&cycle_rom](std::size_t address, std::uint32_t value) {
            cycle_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            cycle_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            cycle_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            cycle_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    cycle_write32(0x4F42U, 0x00004F62U);
    cycle_write32(0x4F62U, 0x00004F80U);
    cycle_write16(0x4F22U, 0xFFFFU);
    cycle_write16(0x4F24U, 0xFFE4U);
    cycle_write32(0x4F80U + 2U, 0x11223344U);
    cycle_write16(0x4F80U + 6U, 0x0055U);
    cycle_write16(0x4F88U + 0U, 0);
    cycle_write16(0x4F88U + 2U, 0);
    cycle_write16(0x4F88U + 4U, 0x0012U);
    cycle_write16(0x4F88U + 6U, 0x0034U);
    cycle_write16(0x4F88U + 8U, 0x8000U);
    shadowrun::GenesisMemoryBus cycle_bus(std::move(cycle_rom));
    const auto cycle_record = 0xFF3500U;
    cycle_bus.write8(cycle_record + 2U, 1);
    cycle_bus.write8(cycle_record + 3U, 0);
    cycle_bus.write16(cycle_record + 0x0AU, 0x0100U);
    cycle_bus.write16(cycle_record + 0x0CU, 0x0200U);
    cycle_bus.write16(cycle_record + 0x2AU, 3);
    cycle_bus.write16(cycle_record + 0x2EU, 0);
    cycle_bus.write16(cycle_record + 0x32U, 0x0012U);
    cycle_bus.write8(cycle_record + 0x11U, 0);
    cycle_bus.write16(shadowrun::ram::kObjectListCount, 1);
    cycle_bus.write32(shadowrun::ram::kObjectOutputPointer, 0xFF8F00U);
    cycle_bus.write8(shadowrun::ram::kObjectOutputCursorByte, 0);
    const auto cycle_result = shadowrun::emit_entity_cycle_objects(
        cycle_bus, cycle_record, 0xFF7000U, 2, 2);
    assert(cycle_result.emitted);
    assert(cycle_result.descriptor_records_emitted == 1);
    assert(cycle_result.list_records_emitted == 1);
    assert(cycle_result.next_output_address == 0xFF7008U);
    assert(cycle_result.next_count == 2);
    assert(cycle_result.next_ordinal == 3);
    assert(cycle_bus.read32(0xFF8F00U) == 0x11223344U);
    assert(cycle_bus.read16(0xFF8F04U) == 0x0055U);
    assert(cycle_bus.read16(0xFF8F06U) == 0x0060U);
    assert(cycle_bus.read16(0xFF7000U) == 0x01E4U);
    assert(cycle_bus.read8(0xFF7002U) == 0x12U);
    assert(cycle_bus.read8(0xFF7003U) == 2);
    assert(cycle_bus.read16(0xFF7004U) == 0x2037U);
    assert(cycle_bus.read16(0xFF7006U) == 0x00FFU);

    cycle_bus.write8(cycle_record + 2U, 0);
    cycle_bus.write8(cycle_record + 3U, 3);
    cycle_bus.write8(cycle_record + 0x11U, 0);
    cycle_bus.write8(0xFFD87EU, 0xA5U);
    const auto cycle_skipped = shadowrun::emit_entity_cycle_objects(
        cycle_bus, cycle_record, 0xFF7010U, 9, 2);
    assert(!cycle_skipped.emitted);
    assert(cycle_bus.read8(cycle_record + 2U) == 6);
    assert(cycle_bus.read8(cycle_record + 3U) == 4);
    assert((cycle_bus.read8(cycle_record + 0x11U) & 0x40U) != 0);
    assert(cycle_bus.read8(0xFFD87EU) == 0);

    std::vector<std::uint8_t> slot_rom(0x1D600, 0);
    const auto slot_write16 =
        [&slot_rom](std::size_t address, std::uint16_t value) {
            slot_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            slot_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto slot_write32 =
        [&slot_rom](std::size_t address, std::uint32_t value) {
            slot_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            slot_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            slot_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            slot_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    slot_write32(shadowrun::rom::kEntitySlotObjectPointerTable,
                 0x00002000U);
    slot_write16(0x2000U + 0U, 0);
    slot_write16(0x2000U + 2U, 0x0010U);
    slot_write16(0x2000U + 4U, 0x0034U);
    slot_write16(0x2000U + 6U, 0x0056U);
    slot_write16(0x2000U + 8U, 0x8000U);
    shadowrun::GenesisMemoryBus slot_bus(std::move(slot_rom));
    slot_bus.write16(shadowrun::ram::kCameraX, 0x1000U);
    slot_bus.write16(shadowrun::ram::kCameraY, 0x1000U);
    const auto slot_record = shadowrun::kEntitySlotBase;
    slot_bus.write16(slot_record + 0U, 0x1000U);
    slot_bus.write16(slot_record + 2U, 0);
    slot_bus.write16(slot_record + 4U, 0x1000U);
    slot_bus.write16(slot_record + 8U, 0);
    slot_bus.write8(slot_record + 0x10U, 1);
    slot_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto slot_result = shadowrun::emit_entity_slot_objects(
        slot_bus, 0xFF7000U, 2, 2);
    assert(slot_result.slots_examined == 10);
    assert(slot_result.active_slots == 1);
    assert(slot_result.projected_slots == 1);
    assert(slot_result.objects_emitted == 1);
    assert(slot_result.next_output_address == 0xFF7008U);
    assert(slot_result.next_count == 2);
    assert(slot_result.next_ordinal == 3);
    assert(slot_bus.read16(0xFF7000U) == 0x0090U);
    assert(slot_bus.read8(0xFF7002U) == 0x34U);
    assert(slot_bus.read8(0xFF7003U) == 2);
    assert(slot_bus.read16(0xFF7004U) == 0x2056U);
    assert(slot_bus.read16(0xFF7006U) == 0x0080U);

    std::vector<std::uint8_t> post_rom(0x200000, 0);
    const auto post_write16 =
        [&post_rom](std::size_t address, std::uint16_t value) {
            post_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            post_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto post_write32 =
        [&post_rom](std::size_t address, std::uint32_t value) {
            post_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            post_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            post_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            post_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    const auto post_list = 0x1FF000U;
    for (const auto table : {
             shadowrun::rom::kPostObjectPointerTableA,
             shadowrun::rom::kPostObjectPointerTableB,
             shadowrun::rom::kPostObjectPointerTableC,
             shadowrun::rom::kPostObjectPointerTableD}) {
        post_write32(table + 4U, post_list);
    }
    post_write16(post_list + 0U, 0);
    post_write16(post_list + 2U, 0x0010U);
    post_write16(post_list + 4U, 0x0012U);
    post_write16(post_list + 6U, 0x0034U);
    post_write16(post_list + 8U, 0x8000U);
    post_write16(shadowrun::rom::kPostObjectOffsetTable + 4U, 0);
    post_write16(shadowrun::rom::kPostObjectOffsetTable + 6U, 0);
    post_write16(shadowrun::rom::kPostObjectBackgroundTileTable + 0U,
                 0x1111U);
    post_write16(shadowrun::rom::kPostObjectBackgroundTileTable + 2U,
                 0x2222U);
    post_write16(shadowrun::rom::kPostObjectBackgroundTileTable + 4U,
                 0x3333U);
    post_write16(shadowrun::rom::kPostObjectBackgroundTileTable + 6U,
                 0x4444U);
    post_write16(shadowrun::rom::kPostObjectBackgroundTileTable + 8U,
                 0x5555U);
    post_write16(shadowrun::rom::kPostObjectBackgroundTileTable + 0xAU,
                 0x6666U);
    shadowrun::GenesisMemoryBus post_bus(std::move(post_rom));
    post_bus.write8(shadowrun::ram::kEntityStateGlobalPhase, 2);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x22U, 2);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x0AU,
                     0x0100U);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x0CU,
                     0x0200U);
    post_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto post_result = shadowrun::ShadowrunMainState(post_bus)
        .run_object_output_pass({}, 2);
    assert(!post_result.unresolved_0x5D29C_call);
    assert(post_result.objects_emitted == 4);
    assert(post_result.next_ordinal == 6);
    assert(post_result.next_output_address ==
           shadowrun::ram::kObjectListInitialOutput + 0x18U);
    assert(post_bus.read16(shadowrun::ram::kObjectListCount) == 5);
    post_bus.write16(shadowrun::ram::kTilePlaneWidth, 4);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x22U, 0);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x24U, 0);
    const auto post_background = shadowrun::ShadowrunMainState(post_bus)
        .run_object_output_pass({}, 2);
    assert(!post_background.unresolved_0x5D29C_call);
    assert(post_bus.read16(shadowrun::ram::kBackgroundTileTable + 0U) ==
           0x1111U);
    assert(post_bus.read16(shadowrun::ram::kBackgroundTileTable + 2U) ==
           0x2222U);
    assert(post_bus.read16(shadowrun::ram::kBackgroundTileTable + 4U) ==
           0x3333U);
    assert(post_bus.read16(shadowrun::ram::kBackgroundTileTable + 6U) ==
           0x4444U);
    assert(post_bus.read16(shadowrun::ram::kBackgroundTileTable + 8U) ==
           0x5555U);
    assert(post_bus.read16(shadowrun::ram::kBackgroundTileTable + 0xAU) ==
           0x6666U);
    assert(post_bus.read16(shadowrun::ram::kBackgroundQueueBusy) == 1);
    assert(post_bus.read16(shadowrun::session_ram::kPostObjectState + 0x22U) ==
           1);
    assert(post_bus.read16(shadowrun::session_ram::kPostObjectState + 0x24U) ==
           0x100U);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x22U, 1);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 4U, 0xFFF0U);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x24U, 1);
    shadowrun::ShadowrunMainState(post_bus).run_object_output_pass({}, 2);
    assert(post_bus.read16(shadowrun::session_ram::kPostObjectState + 4U) ==
           0xFFF0U);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 4U, 0x248U);
    post_bus.write16(shadowrun::session_ram::kPostObjectState + 0x24U, 1);
    shadowrun::ShadowrunMainState(post_bus).run_object_output_pass({}, 2);
    assert(post_bus.read16(shadowrun::session_ram::kPostObjectState + 4U) ==
           0x248U);

    std::vector<std::uint8_t> allocator_rom(
        shadowrun::rom::kPostObjectEntityTemplate + 0x100U, 0);
    const auto allocator_write16 =
        [&allocator_rom](std::size_t address, std::uint16_t value) {
            allocator_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            allocator_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto allocator_write32 =
        [&allocator_rom](std::size_t address, std::uint32_t value) {
            allocator_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            allocator_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            allocator_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            allocator_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    for (std::size_t index = 0; index < 0x100U; ++index) {
        allocator_rom[shadowrun::rom::kPostObjectEntityTemplate + index] =
            static_cast<std::uint8_t>(0x80U + index);
    }
    allocator_write16(shadowrun::rom::kEntitySlotCommandTable + 4U, 0x0034U);
    allocator_write32(shadowrun::rom::kEntitySlotHandlerTable + 8U,
                      shadowrun::rom::kEntitySlotNoOpHandler);
    shadowrun::GenesisMemoryBus allocator_bus(std::move(allocator_rom));

    const auto first_sprite = shadowrun::allocate_sprite_slot_0x1A2CC(
        allocator_bus);
    assert(first_sprite.allocated);
    assert(first_sprite.slot_index == 0);
    assert(first_sprite.d1_value == 0x0049U);
    assert(first_sprite.a2_value == 0x0920U);
    assert(allocator_bus.read8(shadowrun::session_ram::kSpriteSlotFlags) == 1);
    const auto second_sprite = shadowrun::allocate_sprite_slot_0x1A2CC(
        allocator_bus);
    assert(second_sprite.allocated);
    assert(second_sprite.slot_index == 1);
    assert(second_sprite.d1_value == 0x0061U);
    assert(second_sprite.a2_value == 0x0C20U);
    for (std::size_t index = 2; index < shadowrun::session_ram::kSpriteSlotCount;
         ++index) {
        allocator_bus.write8(shadowrun::session_ram::kSpriteSlotFlags + index,
                             1);
    }
    const auto saturated_sprite = shadowrun::allocate_sprite_slot_0x1A2CC(
        allocator_bus);
    assert(!saturated_sprite.allocated);
    assert(saturated_sprite.slot_index == shadowrun::session_ram::kSpriteSlotCount);

    for (std::size_t index = 0; index < shadowrun::session_ram::kSpriteSlotCount;
         ++index) {
        allocator_bus.write8(shadowrun::session_ram::kSpriteSlotFlags + index,
                             0);
    }
    const auto free_record = shadowrun::allocate_post_object_record_0x15A8C(
        allocator_bus);
    assert(free_record.allocated);
    assert(free_record.record_address == shadowrun::session_ram::kPostObjectRecordBase);
    assert(free_record.record_index == 9);
    allocator_bus.write8(free_record.record_address + 0x10U, 1);
    const auto next_record = shadowrun::allocate_post_object_record_0x15A8C(
        allocator_bus);
    assert(next_record.record_address ==
           shadowrun::session_ram::kPostObjectRecordBase + 0x100U);
    assert(next_record.record_index == 10);
    allocator_bus.write8(free_record.record_address + 0x10U, 0);

    allocator_bus.write16(shadowrun::session_ram::kReadyFlag1, 1);
    allocator_bus.write8(shadowrun::genesis::kZ80CommandIndex, 0x3EU);
    allocator_bus.write8(0xFF2002U, 0xAAU);
    allocator_bus.write8(0xFF2003U, 0xBBU);
    allocator_bus.write8(0xFF2004U, 0xCCU);
    allocator_bus.write8(shadowrun::session_ram::kSpriteSlotFlags, 0);
    const auto initialized_slot = shadowrun::initialize_entity_slot_0x15930(
        allocator_bus, 0xFF2000U);
    assert(initialized_slot.sprite_allocated);
    assert(!initialized_slot.unresolved_handler);
    assert(allocator_bus.read8(0xFF2000U + 0xB2U) == 2);
    assert(allocator_bus.read16(0xFF2000U + 0x32U) == 0);
    assert(allocator_bus.read16(0xFF2000U + 0x2AU) == 0x0049U);
    assert(allocator_bus.read8(0xFF2000U + 2U) == 0);
    assert(allocator_bus.read8(0xFF2000U + 3U) == 0);
    assert(allocator_bus.read8(0xFF2000U + 4U) == 0xCCU);
    assert(allocator_bus.z80_ram()[0x1B7EU] == 0xFFU);
    assert(allocator_bus.z80_ram()[0x1B7FU] == 0x10U);
    assert(allocator_bus.z80_ram()[0x1B40U] == 0x34U);
    assert(allocator_bus.read8(shadowrun::genesis::kZ80CommandIndex) == 1);

    const auto spawned_post_entity =
        shadowrun::spawn_post_object_entity_0x5D6BC(allocator_bus);
    assert(spawned_post_entity.record_allocated);
    assert(spawned_post_entity.sprite_allocated);
    assert(spawned_post_entity.record_address ==
           shadowrun::session_ram::kPostObjectRecordBase);
    assert(spawned_post_entity.record_index == 9);
    assert(spawned_post_entity.sprite_slot_index == 1);
    assert(spawned_post_entity.sprite_d1_value == 0x0061U);
    assert(spawned_post_entity.sprite_a2_value == 0x0C20U);
    assert(allocator_bus.read8(
               spawned_post_entity.record_address + 0x50U) == 0xD0U);
    assert(allocator_bus.read32(
               spawned_post_entity.record_address + 0x3AU) == 0x00015390U);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 0x2CU) == 0x2000U);
    assert(allocator_bus.read16(spawned_post_entity.record_address) == 0x1200U);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 4U) == 0x02C0U);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 0x3EU) == 9);
    assert(allocator_bus.read8(
               spawned_post_entity.record_address + 0x10U) == 0x91U);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 0x16U) == 0x000CU);
    assert(allocator_bus.read8(
               spawned_post_entity.record_address + 0x41U) == 3);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 0x30U) == 1);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 0x20U) == 0x0061U);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 0x26U) == 0x0C20U);
    assert(allocator_bus.read16(
               spawned_post_entity.record_address + 0x38U) == 0x002AU);

    allocator_bus.write8(shadowrun::ram::kEntityStateGlobalPhase, 2);
    allocator_bus.write16(shadowrun::session_ram::kPostObjectState + 0x22U, 0);
    allocator_bus.write16(shadowrun::session_ram::kPostObjectState + 0x24U,
                          0x15U);
    const auto state_spawn = shadowrun::ShadowrunMainState(allocator_bus)
        .run_object_output_pass({}, 2);
    assert(!state_spawn.unresolved_0x5D29C_call);
    assert(allocator_bus.read16(0xFF0C00U + 0x3EU) == 10);
    assert(allocator_bus.read16(0xFF0C00U + 0x30U) == 2);
    assert(allocator_bus.read16(0xFF0C00U + 0x20U) == 0x0079U);
    assert(allocator_bus.read16(0xFF0C00U + 0x26U) == 0x0F20U);

    std::vector<std::uint8_t> phase_rom(0xD4F00, 0);
    const auto phase_write16 =
        [&phase_rom](std::size_t address, std::uint16_t value) {
            phase_rom[address] = static_cast<std::uint8_t>(value >> 8U);
            phase_rom[address + 1U] = static_cast<std::uint8_t>(value);
        };
    const auto phase_write32 =
        [&phase_rom](std::size_t address, std::uint32_t value) {
            phase_rom[address] = static_cast<std::uint8_t>(value >> 24U);
            phase_rom[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
            phase_rom[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
            phase_rom[address + 3U] = static_cast<std::uint8_t>(value);
        };
    const auto phase_write8 =
        [&phase_rom](std::size_t address, std::uint8_t value) {
            phase_rom[address] = value;
        };
    phase_write32(shadowrun::rom::kEntityPhaseObjectPointerTable,
                  0x00002000U);
    phase_write16(0x2000U + 0U, 0);
    phase_write16(0x2000U + 2U, 0);
    phase_write16(0x2000U + 4U, 0x0012U);
    phase_write16(0x2000U + 6U, 0x0034U);
    phase_write16(0x2000U + 8U, 0x8000U);
    phase_write32(0x5756U, 0x00005796U);
    phase_write32(0x575AU, 0x00005796U);
    phase_write32(shadowrun::rom::kEntityDispatchTable + 12U * 4U,
                  0x000056DEU);
    phase_write32(shadowrun::rom::kEntityDispatchTable + 5U * 4U,
                  0x000052BEU);
    phase_write32(shadowrun::rom::kEntityDispatchTable + 1U * 4U,
                  0x0000521EU);
    phase_write32(shadowrun::rom::kEntityDispatchTable + 3U * 4U,
                  0x00004CDEU);
    phase_write32(shadowrun::rom::kEntityDispatchTable + 8U * 4U,
                  0x0000508AU);
    phase_write32(shadowrun::rom::kEntityDispatchTable + 10U * 4U,
                  0x00004D54U);
    phase_write32(shadowrun::rom::kEntityStateObjectPointerTable,
                  0x00005796U);
    phase_write32(shadowrun::rom::kEntityStateObjectPointerTable0x521E,
                  0x00005796U);
    phase_write32(shadowrun::rom::kEntityStateObjectPointerTable0x508A,
                  0x00005796U);
    phase_write32(shadowrun::rom::kEntityStateObjectPointerTable0x5152,
                  0x00005796U);
    phase_write32(shadowrun::rom::kEntityStateObjectPointerTable0x4FE2,
                  0x00005796U);
    phase_write32(0x585EU, 0x00005796U);
    phase_write32(0x586AU, 0x00005796U);
    phase_write16(shadowrun::rom::kEntityStateGridPositionTable + 0x20U,
                  0x1111U);
    phase_write16(shadowrun::rom::kEntityStateGridPositionTable + 0x22U,
                  0x2222U);
    phase_write16(shadowrun::rom::kEntityStateGridPositionTable + 0x24U,
                  0x3333U);
    phase_write16(shadowrun::rom::kEntityStateGridPositionTable + 0x26U,
                  0x4444U);
    phase_write16(shadowrun::rom::kEntityStateGridAnimationTableDefault + 8U,
                  0x5555U);
    phase_write16(shadowrun::rom::kEntityStateGridAnimationTableDefault + 10U,
                  0x6666U);
    phase_write16(shadowrun::rom::kEntityStateGridAnimationTableDefault + 12U,
                  0x7777U);
    phase_write16(shadowrun::rom::kEntityStateGridAnimationTableDefault + 14U,
                  0x8888U);
    phase_write8(shadowrun::rom::kEntityStateModifierTable + 0x25U, 7);
    phase_write8(shadowrun::rom::kEntityStateModifierTable + 0x26U, 9);
    phase_write16(shadowrun::rom::kEntityAnimationFrameTable, 0x0060U);
    phase_write16(shadowrun::rom::kEntityAnimationFallbackCommandTable,
                  0x0037U);
    phase_write16(shadowrun::rom::kEntityAnimationFallbackFrameTable,
                  0x00E8U);
    phase_write32(0x5798U, 0xAABBCCDDU);
    phase_write16(0x579CU, 0x0066U);
    phase_write16(0x579EU + 0U, 0);
    phase_write16(0x579EU + 2U, 0);
    phase_write16(0x579EU + 4U, 0x0012U);
    phase_write16(0x579EU + 6U, 0x0034U);
    phase_write16(0x579EU + 8U, 0x8000U);
    shadowrun::GenesisMemoryBus phase_bus(std::move(phase_rom));
    const auto phase_record = 0xFF3600U;
    phase_bus.write16(phase_record + 0x0AU, 0x0100U);
    phase_bus.write16(phase_record + 0x0CU, 0x0200U);
    phase_bus.write16(phase_record + 0x0EU, 0);
    phase_bus.write8(phase_record + 6U, 1);
    phase_bus.write8(phase_record + 7U, 0);
    const auto auxiliary_record = 0xFF8000U;
    phase_bus.write8(auxiliary_record + 0x9FU, 2);
    phase_bus.write16(phase_record + 0x3EU, 1);
    phase_bus.write8(phase_record + 0x4CU, 2);
    phase_bus.write8(phase_record + 0x9CU, 0);
    phase_bus.write8(phase_record + 0x10U, 1);
    phase_bus.write32(shadowrun::ram::kEntityObjectControlPointer,
                      0xFF9000U);
    const auto auxiliary_state_result =
        shadowrun::update_entity_auxiliary_state_0x55C2C(
            phase_bus, phase_record, auxiliary_record);
    assert(auxiliary_state_result.source_delta_was_nonzero);
    assert(auxiliary_state_result.position_stream.streamed);
    assert(auxiliary_state_result.position_stream.phase == 4);
    assert(auxiliary_state_result.position_stream.source ==
           shadowrun::rom::kEntityStateGridPositionTable + 0x20U);
    assert(auxiliary_state_result.position_stream.command == 0xF5C4U);
    assert(auxiliary_state_result.animation_stream.streamed);
    assert(auxiliary_state_result.animation_stream.phase == 1);
    assert(auxiliary_state_result.animation_stream.source ==
           shadowrun::rom::kEntityStateGridAnimationTableDefault + 8U);
    assert(auxiliary_state_result.animation_stream.command == 0xF7C6U);
    assert(auxiliary_state_result.flags_after == 0x51U);
    assert(auxiliary_state_result.field_0x3A_after == 0x0000B45CU);
    phase_bus.write8(phase_record + 0x78U, 10);
    phase_bus.write8(phase_record + 0x7BU, 6);
    phase_bus.write8(phase_record + 0x7CU, 4);
    phase_bus.write8(phase_record + 0x68U, 0x27U);
    phase_bus.write8(phase_record + 0x70U, 6);
    phase_bus.write8(phase_record + 0x69U, 0x28U);
    phase_bus.write8(phase_record + 0x71U, 5);
    phase_bus.write8(phase_record + 0x11U, 0);
    phase_bus.write8(phase_record + 0x76U, 1);
    phase_bus.write8(phase_record + 0x4DU, 3);
    const auto base_state = shadowrun::calculate_entity_base_0x55ED6(
        phase_bus, phase_record);
    assert(base_state == 15);
    assert(shadowrun::adjust_entity_cost_0x55E62(
               phase_bus, phase_record, 100) == 97);
    assert(shadowrun::subtract_entity_modifier_0x55E92(
               phase_bus, phase_record, 0x1234U) == 0x122DU);
    assert(shadowrun::subtract_entity_modifier_0x55EB4(
               phase_bus, phase_record, 0x1234U) == 0x122BU);
    const auto positive_rate = shadowrun::calculate_entity_rate_0x55F20(
        phase_bus, phase_record, 100, false);
    assert(positive_rate.quotient == 75);
    assert(positive_rate.residual == 25);
    assert(positive_rate.selected_result == 75);
    const auto residual_rate = shadowrun::calculate_entity_rate_0x55F20(
        phase_bus, phase_record, 100, true);
    assert(residual_rate.selected_result == 25);
    const auto scaling_entity = 0xFF9000U;
    phase_bus.write8(scaling_entity + 0x78U, 10);
    phase_bus.write8(scaling_entity + 0x7BU, 6);
    phase_bus.write8(scaling_entity + 0x88U, 3);
    phase_bus.write8(shadowrun::ram::kEntityStateScalingBase, 20);
    phase_bus.write8(shadowrun::ram::kEntityStateScalingBase + 7U, 4);
    phase_bus.write8(shadowrun::ram::kEntityStateScalingMode, 0);
    const auto half_scaled = shadowrun::scale_entity_value_0x55F44(
        phase_bus, 1);
    assert(half_scaled.half_base == 8);
    assert(half_scaled.adjusted_base == 19);
    assert(half_scaled.selected_result == 10);
    phase_bus.write8(shadowrun::ram::kEntityStateScalingMode, 1);
    assert(shadowrun::scale_entity_value_0x55F44(phase_bus, 1)
               .selected_result == 13);
    assert(shadowrun::scale_entity_value_0x55F44(phase_bus, 0)
               .selected_result == 6);
    phase_bus.write8(shadowrun::ram::kEntityStateScalingMode, 0xFFU);
    assert(shadowrun::scale_entity_value_0x55F44(phase_bus, 0xFFFFU)
               .selected_result == 6);
    assert(shadowrun::scale_entity_value_0x55F44(phase_bus, 1)
               .selected_result == 13);
    phase_bus.write8(shadowrun::ram::kEntityStateScalingMode, 0);
    const auto reaction_entity = 0xFF3200U;
    phase_bus.write16(reaction_entity + 0x90U, 2);
    phase_bus.write8(reaction_entity + 0x85U, 2);
    phase_bus.write8(reaction_entity + 0x9AU, 0);
    phase_bus.write8(reaction_entity + 0xDFU, 0);
    phase_bus.write8(phase_record + 0x10U, 0);
    phase_bus.write16(phase_record + 0x90U, 2);
    phase_bus.write16(phase_record + 0x3EU, 0);
    phase_bus.write8(phase_record + 0x4CU, 2);
    phase_bus.write8(phase_record + 0x9CU, 3);
    phase_bus.write8(phase_record + 0x9BU, 0);
    phase_bus.write8(phase_record + 0x11U, 0);
    phase_bus.write8(phase_record + 0x76U, 0);
    phase_bus.write8(phase_record + 0xDFU, 0);
    phase_bus.write16(shadowrun::ram::kRandomState, 7);
    phase_bus.write8(shadowrun::ram::kEntityStateGlobalPhase, 0);
    const auto reaction_result = shadowrun::run_entity_reaction_0x55C76(
        phase_bus, reaction_entity, phase_record);
    assert(reaction_result.first_threshold_passed);
    assert(reaction_result.second_threshold_passed);
    assert(reaction_result.third_threshold_passed);
    assert(!reaction_result.position_update_called);
    assert(reaction_result.animation_update_called);
    assert(reaction_result.random_bucket == 1);
    assert(phase_bus.read8(phase_record + 0x9CU) == 0);
    const auto animation_record = 0xFF3A00U;
    phase_bus.write8(animation_record + 0x10U, 0);
    phase_bus.write8(animation_record + 0x4CU, 1);
    phase_bus.write16(animation_record + 0x2EU, 0x0000U);
    phase_bus.write16(animation_record + 0x1AU, 0x1234U);
    phase_bus.write16(animation_record + 0x1EU, 0x5678U);
    phase_bus.write8(animation_record + 0x11U, 0);
    const auto animation_start =
        shadowrun::initialize_entity_animation_0xAFF2(
            phase_bus, animation_record);
    assert(animation_start.started);
    assert(!animation_start.skipped_by_flag);
    assert(!animation_start.unresolved_0xB330_path);
    assert(animation_start.selected_frame == 0x0060U);
    assert(animation_start.timer_after == 8);
    assert(animation_start.flags_after == 0);
    assert(animation_start.routine_pointer_after == 0x0000B07EU);
    assert(phase_bus.read16(animation_record + 0x1AU) == 0);
    assert((phase_bus.read8(animation_record + 0x11U) & 1U) != 0);
    phase_bus.write8(animation_record + 0x10U, 0x10U);
    const auto skipped_animation =
        shadowrun::initialize_entity_animation_0xAFF2(
            phase_bus, animation_record);
    assert(skipped_animation.skipped_by_flag);
    assert(!skipped_animation.started);
    phase_bus.write8(animation_record + 0x10U, 0);
    phase_bus.write8(animation_record + 0x4CU, 0);
    phase_bus.write16(shadowrun::session_ram::kReadyFlag1, 1);
    phase_bus.write8(shadowrun::genesis::kZ80CommandIndex, 0);
    phase_bus.write8(shadowrun::session_ram::kEntityPhaseBase + 0x10U,
                     0x40U);
    phase_bus.write8(shadowrun::session_ram::kEntityPhaseBase + 0x110U, 0);
    phase_bus.write8(shadowrun::session_ram::kEntityPhaseBase + 0x210U,
                     0x41U);
    phase_bus.write16(shadowrun::ram::kEntityAnimationWakeTimer, 0);
    const auto fallback_animation =
        shadowrun::initialize_entity_animation_0xAFF2(
            phase_bus, animation_record);
    assert(!fallback_animation.unresolved_0xB330_path);
    assert(fallback_animation.started);
    assert(fallback_animation.selected_frame == 0x00E8U);
    assert(fallback_animation.flags_after == 0x10U);
    assert(fallback_animation.routine_pointer_after == 0x0000B38AU);
    assert(phase_bus.read16(shadowrun::ram::kEntityAnimationWakeTimer) ==
           0x003CU);
    assert(phase_bus.z80_ram()[0x1B40U] == 0xFFU);
    assert(phase_bus.z80_ram()[0x1B41U] == 0x10U);
    assert(phase_bus.z80_ram()[0x1B42U] == 0x37U);
    assert(phase_bus.read8(shadowrun::genesis::kZ80CommandIndex) == 3);
    phase_bus.write8(shadowrun::session_ram::kEntityPhaseBase + 0x10U, 0);
    phase_bus.write8(shadowrun::session_ram::kEntityPhaseBase + 0x210U, 0);
    phase_bus.write16(shadowrun::session_ram::kReadyFlag1, 0);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto phase_result = shadowrun::emit_entity_phase_objects_0x5936(
        phase_bus, phase_record, 0xFF7200U, 2, 2);
    assert(phase_result.lists_processed == 1);
    assert(phase_result.objects_emitted == 1);
    assert(phase_result.next_output_address == 0xFF7208U);
    assert(phase_result.next_count == 2);
    assert(phase_result.next_ordinal == 3);
    assert(phase_bus.read16(0xFF7200U) == 0x0200U);
    assert(phase_bus.read16(0xFF7204U) == 0x4034U);
    assert(phase_bus.read16(0xFF7206U) == 0x0100U);

    phase_bus.write16(phase_record + 0x0EU, 0x000FU);
    phase_bus.write8(phase_record + 6U, 1);
    phase_bus.write8(phase_record + 7U, 0);
    const auto paired_phase_result =
        shadowrun::emit_entity_phase_objects_0x5936(
            phase_bus, phase_record, 0xFF7208U, 3, 2);
    assert(paired_phase_result.lists_processed == 2);
    assert(paired_phase_result.objects_emitted == 2);
    assert(paired_phase_result.next_output_address == 0xFF7218U);
    assert(phase_bus.read16(0xFF7208U) == 0x01E0U);
    assert(phase_bus.read16(0xFF720EU) == 0x00D2U);
    assert(phase_bus.read16(0xFF7210U) == 0x01E0U);
    assert(phase_bus.read16(0xFF7216U) == 0x0129U);

    phase_bus.write16(phase_record + 0x0EU, 0);
    phase_bus.write8(phase_record + 6U, 1);
    phase_bus.write8(phase_record + 7U, 0);
    const auto paired_59ea_result =
        shadowrun::emit_entity_phase_objects_0x59EA(
            phase_bus, phase_record, 0xFF7218U, 5, 2);
    assert(paired_59ea_result.lists_processed == 2);
    assert(paired_59ea_result.objects_emitted == 2);
    assert(paired_59ea_result.next_output_address == 0xFF7228U);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write16(shadowrun::ram::kEntityRandomXOffset, 0);
    phase_bus.write16(shadowrun::ram::kEntityRandomYOffset, 0);
    phase_bus.write16(shadowrun::ram::kEntityRandomValue, 0);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto random_object_result = shadowrun::emit_entity_random_objects_0x5494(
        phase_bus, phase_record, 0xFF7400U, 2, 2);
    assert(random_object_result.lists_processed == 1);
    assert(random_object_result.objects_emitted == 1);
    assert(random_object_result.next_output_address == 0xFF7408U);
    assert(phase_bus.read16(0xFF7400U) == 0x0200U);
    assert(phase_bus.read16(0xFF7404U) == 0x6034U);
    assert(phase_bus.read16(0xFF7406U) == 0x0100U);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 3);
    phase_bus.write16(shadowrun::ram::kEntityRandomPhaseGate, 1);
    phase_bus.write16(shadowrun::ram::kRandomState, 0);
    phase_bus.write32(shadowrun::ram::kObjectOutputPointer, 0xFF8F00U);
    phase_bus.write8(shadowrun::ram::kObjectOutputCursorByte, 0);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 2);
    const auto randomized_object_result =
        shadowrun::emit_entity_random_objects_0x5494(
            phase_bus, phase_record, 0xFF7408U, 3, 2);
    assert(randomized_object_result.lists_processed == 1);
    assert(randomized_object_result.objects_emitted == 1);
    assert(randomized_object_result.next_output_address == 0xFF7410U);
    assert(phase_bus.read16(shadowrun::ram::kEntityRandomXOffset) == 0xFFFFU);
    assert(phase_bus.read16(shadowrun::ram::kEntityRandomYOffset) == 0xFFFFU);
    assert(phase_bus.read8(phase_record + 2U) == 5);
    assert(phase_bus.read8(phase_record + 3U) == 0);
    assert(phase_bus.read32(0xFF8F00U) == 0xAABBCCDDU);
    assert(phase_bus.read16(0xFF7408U) == 0x01FFU);
    assert(phase_bus.read16(0xFF740EU) == 0x00FFU);

    phase_bus.write16(shadowrun::ram::kEntityRandomGate, 0xFFFFU);
    phase_bus.write16(shadowrun::ram::kEntityRandomGateDelay, 0);
    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 0x0DU);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto random_scan_result =
        shadowrun::run_entity_random_scan_0x53C4(
            phase_bus, phase_record, 0xFF7370U, 4, 2);
    assert(random_scan_result.triggered);
    assert(!random_scan_result.scan_entered);
    assert(!random_scan_result.unresolved_0xAFF2_call);
    assert(random_scan_result.objects_emitted == 1);
    assert(random_scan_result.next_output_address == 0xFF7378U);

    const auto child_record = phase_record + 0x100U;
    phase_bus.write16(child_record + 0x3EU, 0);
    phase_bus.write8(child_record + 0xDEU, 0x0DU);
    phase_bus.write8(child_record + 0x10U, 1);
    phase_bus.write8(child_record + 0x4CU, 1);
    phase_bus.write16(child_record + 0x2EU, 0x0000U);
    phase_bus.write16(phase_record + 0x32U, 0x3456U);
    phase_bus.write16(phase_record + 0x2AU, 0x0123U);
    phase_bus.write32(phase_record + 0xD8U, 0xABCDEF01U);
    phase_bus.write16(shadowrun::ram::kEntityRandomPhaseGate, 0);
    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 3);
    phase_bus.write8(phase_record + 0xB2U, 0x0DU);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto child_scan_result =
        shadowrun::run_entity_random_scan_0x53C4(
            phase_bus, phase_record, 0xFF7380U, 5, 2);
    assert(child_scan_result.triggered);
    assert(child_scan_result.scan_entered);
    assert(!child_scan_result.unresolved_0xAFF2_call);
    assert(child_scan_result.candidate_records_examined == 1);
    assert(child_scan_result.child_records_initialised == 1);
    assert(phase_bus.read8(child_record + 0xB2U) == 0x0DU);
    assert(phase_bus.read8(child_record + 2U) == 5);
    assert(phase_bus.read16(child_record + 0x16U) == 0x0060U);
    assert(phase_bus.read8(child_record + 0xCEU) == 8);
    assert(phase_bus.read16(child_record + 0x32U) == 0x3456U);
    assert(phase_bus.read32(child_record + 0xD8U) == 0xABCDEF01U);
    assert(phase_bus.read16(shadowrun::ram::kEntityRandomPhaseGate) == 4);

    phase_bus.write8(child_record + 0xDEU, 0);
    phase_bus.write16(shadowrun::ram::kEntityRandomPhaseGate, 0);
    phase_bus.write16(shadowrun::ram::kEntityRandomGateDelay, 0xFFFFU);
    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 3);
    phase_bus.write8(phase_record + 0xB2U, 0x0DU);
    phase_bus.write16(phase_record + 0x32U, 0x0012U);
    phase_bus.write8(0xFFD87EU, 0xA5U);
    phase_bus.write8(0xFF01DEU, 0xA5U);
    phase_bus.write8(0xFF01DFU, 0xFFU);
    const auto cleanup_scan_result =
        shadowrun::run_entity_random_scan_0x53C4(
            phase_bus, phase_record, 0xFF7390U, 6, 2);
    assert(cleanup_scan_result.triggered);
    assert(cleanup_scan_result.scan_entered);
    assert(!cleanup_scan_result.unresolved_0xAFF2_call);
    assert(cleanup_scan_result.candidate_records_examined == 9);
    assert(phase_bus.read8(0xFFD87EU) == 0);
    assert(phase_bus.read8(0xFF01DEU) == 0);
    assert(phase_bus.read8(0xFF01DFU) == 0xFDU);
    phase_bus.write16(phase_record + 0x2AU, 0);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 7);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto timed_result = shadowrun::emit_entity_timed_objects_0x5662(
        phase_bus, phase_record, 0xFF7300U, 2, 2);
    assert(timed_result.lists_processed == 1);
    assert(timed_result.objects_emitted == 1);
    assert(timed_result.next_output_address == 0xFF7308U);
    assert(phase_bus.read16(0xFF7300U) == 0x0200U);
    assert(phase_bus.read16(0xFF7304U) == 0x6034U);
    assert(phase_bus.read16(0xFF7306U) == 0x0100U);

    phase_bus.write8(phase_record + 2U, 0);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write32(shadowrun::ram::kObjectOutputPointer, 0xFF8F00U);
    phase_bus.write8(shadowrun::ram::kObjectOutputCursorByte, 0);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 2);
    const auto timed_descriptor_result =
        shadowrun::emit_entity_timed_objects_0x5662(
            phase_bus, phase_record, 0xFF7308U, 3, 2);
    assert(timed_descriptor_result.lists_processed == 1);
    assert(timed_descriptor_result.objects_emitted == 1);
    assert(phase_bus.read32(0xFF8F00U) == 0xAABBCCDDU);
    assert(phase_bus.read16(0xFF8F04U) == 0x0066U);
    assert(phase_bus.read16(0xFF7308U) == 0x0200U);
    assert(phase_bus.read8(phase_record + 2U) == 4);
    assert(phase_bus.read8(phase_record + 3U) == 1);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 12);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto dispatched_state_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7318U, 4, 2);
    assert(dispatched_state_result.handled);
    assert(dispatched_state_result.target_address == 0x56DEU);
    assert(dispatched_state_result.objects_emitted == 1);
    assert(dispatched_state_result.next_output_address == 0xFF7320U);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 5);
    phase_bus.write16(shadowrun::ram::kRandomState, 1);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto dispatched_5322_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7338U, 5, 2);
    assert(dispatched_5322_result.handled);
    assert(dispatched_5322_result.target_address == 0x52BEU);
    assert(dispatched_5322_result.objects_emitted == 1);
    assert(dispatched_5322_result.next_output_address == 0xFF7340U);
    assert(phase_bus.read16(0xFF7338U) == 0x0211U);

    phase_bus.write16(phase_record + 0x32U, 0x3456U);
    phase_bus.write16(phase_record + 0x2AU, 0x0123U);
    phase_bus.write32(phase_record + 0xD8U, 0xABCDEF01U);
    const auto scan_target = 0xFF0300U;
    phase_bus.write8(scan_target + 0xDEU, 5);
    phase_bus.write8(scan_target + 0x10U, 1);
    const auto state_scan = shadowrun::scan_entity_state_records_0x52CC(
        phase_bus, phase_record);
    assert(state_scan.records_examined == 10);
    assert(state_scan.records_initialised == 1);
    assert(phase_bus.read8(scan_target + 0xB2U) == 5);
    assert(phase_bus.read8(scan_target + 2U) == 5);
    assert(phase_bus.read8(scan_target + 3U) == 0);
    assert(phase_bus.read16(scan_target + 0x32U) == 0x3456U);
    assert(phase_bus.read16(scan_target + 0x2AU) == 0x0123U);
    assert(phase_bus.read32(scan_target + 0xD8U) == 0xABCDEF01U);

    const auto dispatch_scan_target = 0xFF0200U;
    phase_bus.write8(dispatch_scan_target + 0xDEU, 5);
    phase_bus.write8(dispatch_scan_target + 0x10U, 1);
    phase_bus.write8(phase_record + 2U, 0);
    phase_bus.write8(phase_record + 3U, 8);
    phase_bus.write8(phase_record + 0xB2U, 5);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto dispatched_5322_cutoff_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7348U, 6, 2);
    assert(dispatched_5322_cutoff_result.handled);
    assert(dispatched_5322_cutoff_result.target_address == 0x52BEU);
    assert(dispatched_5322_cutoff_result.objects_emitted == 0);
    assert(phase_bus.read8(dispatch_scan_target + 0xB2U) == 5);
    assert(phase_bus.read16(dispatch_scan_target + 0x32U) == 0x3456U);
    phase_bus.write16(phase_record + 0x2AU, 0);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 1);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto dispatched_521e_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7348U, 6, 2);
    assert(dispatched_521e_result.handled);
    assert(dispatched_521e_result.target_address == 0x521EU);
    assert(dispatched_521e_result.objects_emitted == 1);
    assert(phase_bus.read16(0xFF7348U) == 0x0200U);
    assert(phase_bus.read16(0xFF734CU) == 0x2034U);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 8);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto dispatched_508a_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7350U, 7, 2);
    assert(dispatched_508a_result.handled);
    assert(dispatched_508a_result.target_address == 0x508AU);
    assert(dispatched_508a_result.objects_emitted == 1);
    assert(phase_bus.read16(0xFF7354U) == 0x4034U);

    phase_bus.write16(phase_record + 0x32U, 0x3456U);
    phase_bus.write16(phase_record + 0x2AU, 0x0123U);
    phase_bus.write32(phase_record + 0xD8U, 0xABCDEF01U);
    phase_bus.write16(shadowrun::session_ram::kReadyFlag1, 0);
    const auto variant_target_3 = 0xFF0400U;
    phase_bus.write8(variant_target_3 + 0xDEU, 3);
    phase_bus.write8(variant_target_3 + 0x10U, 1);
    phase_bus.write8(variant_target_3 + 0x4CU, 1);
    phase_bus.write16(variant_target_3 + 0x2EU, 0);
    phase_bus.write8(phase_record + 2U, 0);
    phase_bus.write8(phase_record + 3U, 5);
    phase_bus.write8(phase_record + 0xB2U, 3);
    const auto dispatched_4cde_cutoff_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7368U, 10, 2);
    assert(dispatched_4cde_cutoff_result.handled);
    assert(dispatched_4cde_cutoff_result.target_address == 0x4CDEU);
    assert(dispatched_4cde_cutoff_result.objects_emitted == 0);
    assert(phase_bus.read8(variant_target_3 + 0xB2U) == 3);
    assert(phase_bus.read8(variant_target_3 + 2U) == 5);
    assert(phase_bus.read16(variant_target_3 + 0x32U) == 0x3456U);
    assert(phase_bus.read16(variant_target_3 + 0x16U) == 0x0060U);
    assert(phase_bus.read8(variant_target_3 + 0xCEU) == 8);
    phase_bus.write16(shadowrun::session_ram::kReadyFlag1, 1);
    const auto unresolved_4cde_tail =
        shadowrun::scan_entity_state_records_0x4CEC(
            phase_bus, phase_record);
    assert(unresolved_4cde_tail.records_initialised == 1);
    assert(!unresolved_4cde_tail.unresolved_0xB330_path);
    assert(unresolved_4cde_tail.z80_command_dispatched);
    phase_bus.write16(shadowrun::session_ram::kReadyFlag1, 0);

    const auto variant_target_10 = 0xFF0500U;
    phase_bus.write8(variant_target_10 + 0xDEU, 0x0AU);
    phase_bus.write8(variant_target_10 + 0x10U, 1);
    phase_bus.write8(variant_target_10 + 0x4CU, 1);
    phase_bus.write16(variant_target_10 + 0x2EU, 0);
    phase_bus.write8(phase_record + 2U, 0);
    phase_bus.write8(phase_record + 3U, 5);
    phase_bus.write8(phase_record + 0xB2U, 0x0AU);
    const auto dispatched_4d54_cutoff_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7370U, 11, 2);
    assert(dispatched_4d54_cutoff_result.handled);
    assert(dispatched_4d54_cutoff_result.target_address == 0x4D54U);
    assert(dispatched_4d54_cutoff_result.objects_emitted == 0);
    assert(phase_bus.read8(variant_target_10 + 0xB2U) == 0x0AU);
    assert(phase_bus.read8(variant_target_10 + 2U) == 5);
    assert(phase_bus.read16(variant_target_10 + 0x16U) == 0x0060U);
    assert(phase_bus.read8(variant_target_10 + 0xCEU) == 8);
    phase_bus.write8(variant_target_3 + 0xDEU, 0);
    phase_bus.write8(variant_target_10 + 0xDEU, 0);
    phase_bus.write16(phase_record + 0x2AU, 0);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 3);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto dispatched_4cde_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7358U, 8, 2);
    assert(dispatched_4cde_result.handled);
    assert(dispatched_4cde_result.target_address == 0x4CDEU);
    assert(dispatched_4cde_result.objects_emitted == 1);
    assert(phase_bus.read16(0xFF735CU) == 0x2034U);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 10);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto dispatched_4d54_result =
        shadowrun::dispatch_entity_state_objects_0x490A(
            phase_bus, phase_record, 0xFF7360U, 9, 2);
    assert(dispatched_4d54_result.handled);
    assert(dispatched_4d54_result.target_address == 0x4D54U);
    assert(dispatched_4d54_result.objects_emitted == 1);
    assert(phase_bus.read16(0xFF7364U) == 0x4034U);

    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    phase_bus.write8(phase_record + 0xB2U, 12);
    phase_bus.write16(shadowrun::ram::kObjectListCount, 1);
    const auto state_object_result =
        shadowrun::emit_entity_state_objects_0x56DE(
            phase_bus, phase_record, 0xFF7320U, 4, 2);
    assert(state_object_result.lists_processed == 1);
    assert(state_object_result.objects_emitted == 1);
    assert(state_object_result.next_output_address == 0xFF7328U);
    assert(phase_bus.read16(0xFF7320U) == 0x0200U);
    assert(phase_bus.read16(0xFF7324U) == 0x6034U);
    assert(phase_bus.read16(0xFF7326U) == 0x0100U);

    phase_bus.write8(phase_record + 2U, 0);
    phase_bus.write8(phase_record + 3U, 15);
    phase_bus.write8(phase_record + 0xB2U, 12);
    phase_bus.write16(phase_record + 0x32U, 0x0012U);
    phase_bus.write8(0xFFD87EU, 0xA5U);
    const auto state_cutoff_result =
        shadowrun::emit_entity_state_objects_0x56DE(
            phase_bus, phase_record, 0xFF7330U, 5, 2);
    assert(state_cutoff_result.lists_processed == 0);
    assert(phase_bus.read8(phase_record + 0xB2U) == 0);
    assert(phase_bus.read8(0xFFD87EU) == 0);
    assert(phase_bus.read8(phase_record + 0x10U) == 0x51U);
    assert(phase_bus.read32(phase_record + 0x3AU) == 0x0000B45CU);

    phase_bus.write16(shadowrun::ram::kEntityTimedGate, 0);
    phase_bus.write16(shadowrun::ram::kEntityTimedGateDelay, 1);
    phase_bus.write8(phase_record + 2U, 1);
    phase_bus.write8(phase_record + 3U, 0);
    const auto gated_result = shadowrun::emit_entity_timed_objects_0x557E(
        phase_bus, phase_record, 0xFF7310U, 4, 2);
    assert(gated_result.lists_processed == 1);
    assert(phase_bus.read16(shadowrun::ram::kEntityTimedGate) == 0xFFFFU);
    assert(phase_bus.read16(shadowrun::ram::kEntityTimedGateDelay) == 0);

    const auto timer_record = 0xFF3200U;
    memory_bus.write16(shadowrun::ram::kFrameCounter, 0);
    memory_bus.write8(timer_record + 0x9EU, 0);
    memory_bus.write8(timer_record + 0x40U, 0);
    memory_bus.write8(timer_record + 0x11U, 0);
    assert(shadowrun::update_entity_timer_flag_0x4E38(
        memory_bus, timer_record));
    assert(memory_bus.read8(timer_record + 0x40U) == 0xFF);
    assert((memory_bus.read8(timer_record + 0x11U) & 0x80U) != 0);
    memory_bus.write8(timer_record + 0x9EU, 0x40);
    memory_bus.write8(timer_record + 0x40U, 3);
    assert(!shadowrun::update_entity_timer_flag_0x4E38(
        memory_bus, timer_record));
    assert(memory_bus.read8(timer_record + 0x40U) == 3);

    memory_bus.write16(shadowrun::ram::kFrameCounter, 1);
    memory_bus.write8(timer_record + 0x41U, 0);
    memory_bus.write8(timer_record + 0x11U, 0);
    assert(shadowrun::update_entity_timer_flag_0x4E64(
        memory_bus, timer_record));
    assert(memory_bus.read8(timer_record + 0x41U) == 0xFF);
    assert((memory_bus.read8(timer_record + 0x11U) & 0x20U) != 0);
    memory_bus.write8(timer_record + 0x41U, 3);
    assert(!shadowrun::update_entity_timer_flag_0x4E64(
        memory_bus, timer_record));
    assert(memory_bus.read8(timer_record + 0x41U) == 2);

    const auto projection_record = 0xFF1B00U;
    memory_bus.write16(projection_record + 0x00U, 0x1800);
    memory_bus.write16(projection_record + 0x04U, 0x1F00);
    memory_bus.write16(projection_record + 0x08U, 0);
    memory_bus.write32(projection_record + 0x3AU,
                       shadowrun::kAlternateProjectionRoutine);
    assert(shadowrun::project_entity_record(
               memory_bus, projection_record, 0x1000, 0x1000) == 0x0100);
    assert(memory_bus.read16(projection_record + 0x0AU) == 0x0100);
    assert(memory_bus.read16(projection_record + 0x0CU) == 0x0170);

    const auto clipped_record = 0xFF1C00U;
    memory_bus.write16(clipped_record + 0x00U, 0x0000);
    memory_bus.write16(clipped_record + 0x04U, 0x1000);
    memory_bus.write16(clipped_record + 0x08U, 0);
    memory_bus.write16(clipped_record + 0x0AU, 0xBEEF);
    memory_bus.write32(clipped_record + 0x3AU,
                       shadowrun::kAlternateProjectionRoutine);
    assert(shadowrun::project_entity_record(
               memory_bus, clipped_record, 0x1000, 0x1000) == 0xFF80);
    assert(memory_bus.read16(clipped_record + 0x0AU) == 0xBEEF);
    assert(memory_bus.read16(clipped_record + 0x0CU) == 0);

    const std::uint8_t object_bytes[] = {'A', 'B', 'C', 'D', 'E'};
    memory_bus.clear_port_writes();
    const auto scattered = shadowrun::scatter_indexed_object_bytes(
        memory_bus, 0x0010,
        {{0, 1, -1, 2, -2}}, object_bytes, sizeof(object_bytes));
    assert(scattered == shadowrun::kIndexedObjectByteCount);
    assert(memory_bus.read8(0xFF1710) == 'A');
    assert(memory_bus.read8(0xFF1711) == 'B');
    assert(memory_bus.read8(0xFF170F) == 'C');
    assert(memory_bus.read8(0xFF1712) == 'D');
    assert(memory_bus.read8(0xFF170E) == 'E');

    memory_bus.write8(0xFFD87E, 0xA5);
    shadowrun::clear_indexed_object_byte(memory_bus, 0x0012);
    assert(memory_bus.read8(0xFFD87E) == 0);
    for (std::size_t index = 0;
         index < shadowrun::kIndexedObjectStateByteCount; ++index) {
        memory_bus.write8(shadowrun::ram::kIndexedObjectClearTable + index,
                          0xA5);
    }
    shadowrun::clear_indexed_object_state(memory_bus);
    for (std::size_t index = 0;
         index < shadowrun::kIndexedObjectStateByteCount; ++index) {
        assert(memory_bus.read8(
                   shadowrun::ram::kIndexedObjectClearTable + index) == 0);
    }

    memory_bus.write32(shadowrun::ram::kObjectOutputPointer, 0xFF1800);
    memory_bus.write16(shadowrun::ram::kObjectOutputCount, 0x0010);
    memory_bus.write32(0xFF1902, 0x11223344U);
    memory_bus.write16(0xFF1906, 0x5566);
    memory_bus.write16(0xFF1A2A, 3);
    shadowrun::append_descriptor_object(memory_bus, 0xFF1900, 0xFF1A00);
    assert(memory_bus.read32(0xFF1800) == 0x11223344U);
    assert(memory_bus.read16(0xFF1804) == 0x5566);
    assert(memory_bus.read16(0xFF1806) == 0x0060);
    assert(memory_bus.read32(shadowrun::ram::kObjectOutputPointer) == 0xFF1808U);
    assert(memory_bus.read16(shadowrun::ram::kObjectOutputCount) == 0x0011);

    const auto input_address = 0xFF1D00U;
    const auto output_address = 0xFF1E00U;
    memory_bus.write16(shadowrun::ram::kObjectListCount, 7);
    memory_bus.write16(input_address + 0, 1);
    memory_bus.write16(input_address + 2, 2);
    memory_bus.write16(input_address + 4, 0x34);
    memory_bus.write16(input_address + 6, 3);
    memory_bus.write16(input_address + 8, 0xFFFE);
    memory_bus.write16(input_address + 10, 4);
    memory_bus.write16(input_address + 12, 0x56);
    memory_bus.write16(input_address + 14, 5);
    memory_bus.write16(input_address + 16, 0x8000);
    const auto bus_list = shadowrun::build_object_list_from_bus(
        memory_bus, input_address, output_address, 3, 20, 10, 0x0200,
        0x0100, 5, shadowrun::ObjectListVariant::WithWord3Offset);
    assert(bus_list.built == 2);
    assert(bus_list.next_output_address == output_address + 16U);
    assert(bus_list.object_count == 9);
    assert(bus_list.next_ordinal == 7);
    assert(memory_bus.read16(output_address + 0) == 22);
    assert(memory_bus.read8(output_address + 2) == 0x34);
    assert(memory_bus.read8(output_address + 3) == 5);
    assert(memory_bus.read16(output_address + 4) == 0x0303);
    assert(memory_bus.read16(output_address + 6) == 11);
    assert(memory_bus.read32(shadowrun::ram::kObjectListOutputPointer) ==
           output_address + 16U);
    assert(memory_bus.read16(shadowrun::ram::kObjectListCountMirror) == 9);

    std::vector<std::uint8_t> object_rom(0x6000, 0);
    const auto put16 = [](std::vector<std::uint8_t>& bytes,
                          std::size_t address, std::uint16_t value) {
        bytes[address] = static_cast<std::uint8_t>(value >> 8U);
        bytes[address + 1U] = static_cast<std::uint8_t>(value);
    };
    put16(object_rom, 0x4C2C, 1);
    put16(object_rom, 0x4C2E, 2);
    put16(object_rom, 0x580A, 1);
    put16(object_rom, 0x580C, 2);
    put16(object_rom, 0x580E, 0x34);
    put16(object_rom, 0x5810, 3);
    put16(object_rom, 0x5812, 0x8000);
    shadowrun::GenesisMemoryBus object_bus(std::move(object_rom));
    const auto update_record = 0xFF1F00U;
    object_bus.write16(update_record + 0x00U, 0x1800);
    object_bus.write16(update_record + 0x04U, 0x1F00);
    object_bus.write16(update_record + 0x08U, 0);
    object_bus.write16(update_record + 0x06U, 0);
    object_bus.write16(update_record + 0x0EU, 1);
    object_bus.write16(update_record + 0x2AU, 0x0100);
    object_bus.write32(update_record + 0x3AU, 0);
    object_bus.write32(shadowrun::ram::kObjectListOutputPointer, 0xFF2000);
    object_bus.write16(shadowrun::ram::kObjectListCount, 0);
    const auto update = shadowrun::update_indexed_object_record(
        object_bus, update_record, 0x1000, 0x1000, 7, 4);
    assert(update.path == shadowrun::IndexedObjectUpdatePath::ObjectList);
    assert(update.timer_after == 0);
    assert(update.projection_return == 1);
    assert(update.emitted_records == 1);
    assert(update.next_ordinal == 8);
    assert(object_bus.read16(0xFF2000) == 0x0174);
    assert(object_bus.read16(0xFF2006) == 0x0102);
    assert(object_bus.read16(0xFF2004) == 0x2103);

    const auto expired_record = 0xFF2100U;
    object_bus.write16(expired_record + 0x02U, 0x1234);
    object_bus.write16(expired_record + 0x06U, 0);
    object_bus.write16(expired_record + 0x0EU, 0);
    object_bus.write16(expired_record + 0x32U, 0x0012);
    object_bus.write16(expired_record + 0x34U, 0x0010);
    object_bus.write8(expired_record + 0x36U, 'A');
    object_bus.write8(expired_record + 0x37U, 'B');
    object_bus.write8(expired_record + 0x38U, 'C');
    object_bus.write8(expired_record + 0x39U, 'D');
    object_bus.write8(expired_record + 0x3AU, 'E');
    object_bus.write8(0xFFD87E, 0xA5);
    const auto expired_update = shadowrun::update_indexed_object_record(
        object_bus, expired_record, 0x1000, 0x1000, 3, 4);
    assert(expired_update.path ==
           shadowrun::IndexedObjectUpdatePath::ExpiredByteScatter);
    assert(expired_update.timer_after == -1);
    assert(object_bus.read16(expired_record + 0x02U) == 0);
    assert(object_bus.read8(0xFFD87E) == 0);
    assert(object_bus.read8(0xFF1710) == 'A');
    assert(object_bus.read8(0xFF1711) == 'B');
    assert(object_bus.read8(0xFF170F) == 'C');
    assert(object_bus.read8(0xFF1712) == 'D');
    assert(object_bus.read8(0xFF170E) == 'E');

    return 0;
}
