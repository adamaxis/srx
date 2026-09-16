#include "shadowrun/runtime.hpp"

#include "shadowrun/boot.hpp"
#include "shadowrun/object_list.hpp"
#include "shadowrun/palette.hpp"
#include "shadowrun/rom_layout.hpp"

namespace {

using shadowrun::GenesisAddress;

void controller_delay() {
    for (std::uint16_t delay = 0x000A; delay != 0xFFFFU; --delay) {
    }
}

void write_vdp_command_sequence_direct(shadowrun::GenesisBus& bus,
                                       std::uint16_t value) {
    bus.write16(shadowrun::genesis::kVdpControl,
                static_cast<std::uint16_t>((value & 0x3FFFU) | 0x4000U));
    bus.write16(shadowrun::genesis::kVdpControl,
                static_cast<std::uint16_t>(
                    ((static_cast<std::uint32_t>(value) << 2U) |
                     (static_cast<std::uint32_t>(value) >> 14U)) & 0x0003U));
}

struct TilePlaneOffset {
    std::uint16_t table_offset;
    std::uint16_t source_offset;
};

TilePlaneOffset map_tile_plane_offset(shadowrun::GenesisBus& bus,
                                      std::uint16_t source_x,
                                      std::uint16_t source_y) {
    // ROM 0x3FD8 keeps both offsets in word registers. The multiplication is
    // performed before the low word is used as the indexed table offset.
    const auto table_x = static_cast<std::uint16_t>(
        (source_x >> 3U) & 0x0003U);
    const auto table_y = static_cast<std::uint16_t>(
        (source_y >> 3U) & 0x0003U);
    auto table_offset = static_cast<std::uint32_t>(source_y >> 5U) *
        bus.read16(shadowrun::ram::kTilePlaneWidth);
    table_offset += static_cast<std::uint32_t>(
        static_cast<std::uint16_t>((source_x >> 5U) << 1U));
    auto source_offset = static_cast<std::uint16_t>(
        (((table_y << 2U) + table_x) << 1U));
    return {static_cast<std::uint16_t>(table_offset), source_offset};
}

std::uint16_t tile_plane_value(shadowrun::GenesisBus& bus,
                               GenesisAddress source,
                               std::uint32_t tile_data_offset,
                               std::uint16_t source_offset) {
    auto value = bus.read16(source + tile_data_offset + source_offset);
    const auto mask = bus.read16(shadowrun::ram::kTilePlaneMask);
    if (static_cast<std::int16_t>(mask) < 0) {
        value = static_cast<std::uint16_t>(value ^ 0x8000U);
    } else {
        value = static_cast<std::uint16_t>(value | mask);
    }
    return value;
}

std::size_t stream_tile_plane_span_0x3D4A(
    shadowrun::GenesisBus& bus,
    GenesisAddress source,
    GenesisAddress tile_table,
    std::uint16_t vdp_command,
    std::uint16_t source_x,
    std::uint16_t source_y) {
    const auto mapped = map_tile_plane_offset(bus, source_x, source_y);
    auto dbf_counter = static_cast<std::uint16_t>(vdp_command & 0x007FU);
    dbf_counter = static_cast<std::uint16_t>(dbf_counter - 0x0080U);
    dbf_counter = static_cast<std::uint16_t>(0U - dbf_counter);
    dbf_counter = static_cast<std::uint16_t>(dbf_counter >> 1U);
    dbf_counter = static_cast<std::uint16_t>(dbf_counter - 1U);

    const auto emit = [&](std::uint16_t command,
                          std::uint16_t counter) {
        write_vdp_command_sequence_direct(bus, command);
        auto tile_offset = mapped.table_offset;
        auto source_offset = mapped.source_offset;
        std::size_t words = 0;
        for (std::uint32_t index = 0; index <= counter; ++index) {
            const auto tile_data_offset = static_cast<std::uint32_t>(
                bus.read16(tile_table + tile_offset)) << 5U;
            bus.write16(shadowrun::genesis::kVdpData,
                        tile_plane_value(bus, source, tile_data_offset,
                                         source_offset));
            source_offset = static_cast<std::uint16_t>(source_offset + 2U);
            if ((source_offset & 0x0007U) == 0) {
                source_offset = static_cast<std::uint16_t>(source_offset - 8U);
                tile_offset = static_cast<std::uint16_t>(tile_offset + 2U);
            }
            ++words;
        }
        return words;
    };

    // 0x3D4A splits a row at the 0x2B/0x2A boundary and clears the low
    // seven command bits for the second span. The DBF counters are retained
    // as words, including their original wrap arithmetic.
    if (static_cast<std::int16_t>(dbf_counter) <
        static_cast<std::int16_t>(0x002BU)) {
        const auto first_counter = dbf_counter;
        const auto first_words = emit(vdp_command, first_counter);
        dbf_counter = static_cast<std::uint16_t>(
            0U - static_cast<std::uint16_t>(first_counter - 0x002AU));
        const auto second_words = emit(
            static_cast<std::uint16_t>(vdp_command & 0xFF80U), dbf_counter);
        return first_words + second_words;
    }
    return emit(vdp_command, 0x002BU);
}

std::size_t stream_tile_plane_column_0x3DDE(
    shadowrun::GenesisBus& bus,
    GenesisAddress source,
    GenesisAddress tile_table,
    std::uint16_t vdp_command,
    std::uint16_t source_x,
    std::uint16_t source_y) {
    const auto mapped = map_tile_plane_offset(bus, source_x, source_y);
    auto dbf_counter = static_cast<std::uint16_t>(vdp_command & 0x0F80U);
    dbf_counter = static_cast<std::uint16_t>(dbf_counter - 0x1000U);
    dbf_counter = static_cast<std::uint16_t>(0U - dbf_counter);
    dbf_counter = static_cast<std::uint16_t>(dbf_counter >> 7U);
    dbf_counter = static_cast<std::uint16_t>(dbf_counter - 1U);
    if (dbf_counter >= 0x001FU) {
        dbf_counter = 0x001FU;
    }

    // ROM 0x3DDE selects the VDP's auto-increment mode before 0x0E9C emits
    // the caller-derived address. It restores the normal increment on exit.
    bus.write16(shadowrun::genesis::kVdpControl, 0x8F80U);
    write_vdp_command_sequence_direct(bus, vdp_command);
    auto tile_offset = mapped.table_offset;
    auto source_offset = mapped.source_offset;
    std::size_t words = 0;
    for (std::uint32_t index = 0; index <= dbf_counter; ++index) {
        const auto tile_data_offset = static_cast<std::uint32_t>(
            bus.read16(tile_table + tile_offset)) << 5U;
        bus.write16(shadowrun::genesis::kVdpData,
                    tile_plane_value(bus, source, tile_data_offset,
                                     source_offset));
        source_offset = static_cast<std::uint16_t>(source_offset + 8U);
        if (source_offset > 0x001EU) {
            source_offset = static_cast<std::uint16_t>(source_offset - 0x0018U);
            tile_offset = static_cast<std::uint16_t>(
                tile_offset + bus.read16(shadowrun::ram::kTilePlaneWidth));
        }
        ++words;
    }
    bus.write16(shadowrun::genesis::kVdpControl, 0x8F02U);
    return words;
}

struct HorizontalTileOrigin {
    std::uint16_t source_x;
    std::uint16_t vdp_data;
};

HorizontalTileOrigin adjust_tile_origin_0x4216(shadowrun::GenesisBus& bus) {
    auto d3 = bus.read16(shadowrun::ram::kVdpDataWord);
    auto d0 = static_cast<std::uint16_t>(
        bus.read16(shadowrun::ram::kCameraX) >> 4U);
    d0 = static_cast<std::uint16_t>(d0 - 8U);
    if (static_cast<std::int16_t>(d0) < 0) {
        d0 = static_cast<std::uint16_t>(d0 + 8U);
    } else {
        d3 = static_cast<std::uint16_t>(d3 + 8U);
        d0 = static_cast<std::uint16_t>(d0 - 8U);
        if (static_cast<std::int16_t>(d0) < 0) {
            d0 = static_cast<std::uint16_t>(d0 + 8U);
        } else {
            d3 = static_cast<std::uint16_t>(d3 + 8U);
        }
    }
    return {d0, d3};
}

struct VerticalTileOrigin {
    std::uint16_t source_y;
    std::uint16_t vdp_register;
};

VerticalTileOrigin adjust_tile_origin_0x41F0(shadowrun::GenesisBus& bus) {
    auto d4 = bus.read16(shadowrun::ram::kVdpRegisterWord0A);
    auto d1 = static_cast<std::uint16_t>(
        bus.read16(shadowrun::ram::kCameraY) >> 4U);
    d1 = static_cast<std::uint16_t>(d1 - 8U);
    if (static_cast<std::int16_t>(d1) < 0) {
        d1 = static_cast<std::uint16_t>(d1 + 8U);
    } else {
        d4 = static_cast<std::uint16_t>(d4 - 8U);
        d1 = static_cast<std::uint16_t>(d1 - 8U);
        if (static_cast<std::int16_t>(d1) < 0) {
            d1 = static_cast<std::uint16_t>(d1 + 8U);
        } else {
            d4 = static_cast<std::uint16_t>(d4 - 8U);
        }
    }
    return {d1, d4};
}

std::uint16_t calculate_tile_vdp_offset(std::uint16_t d3,
                                        std::uint16_t d4) {
    d3 = static_cast<std::uint16_t>(0U - d3);
    d3 = static_cast<std::uint16_t>(d3 & 0x01FFU);
    d4 = static_cast<std::uint16_t>(d4 & 0x00F8U);
    d3 = static_cast<std::uint16_t>(d3 >> 2U);
    d4 = static_cast<std::uint16_t>(d4 << 4U);
    d4 = static_cast<std::uint16_t>(d4 + d3);
    return static_cast<std::uint16_t>(d4 & 0xFFFEU);
}

std::uint8_t probe_controller_port(shadowrun::GenesisBus& bus,
                                   shadowrun::GenesisAddress data_address,
                                   shadowrun::GenesisAddress control_address,
                                   shadowrun::GenesisAddress signature_address) {
    bus.write16(shadowrun::genesis::kZ80BusRequest, 0x0100);
    while ((bus.read8(shadowrun::genesis::kZ80BusRequest) & 1U) != 0) {
    }

    bus.write8(control_address, 0x40);
    bus.write8(data_address, 0x00);
    controller_delay();
    auto signature = static_cast<std::uint8_t>(
        bus.read8(data_address) << 2U);
    signature = static_cast<std::uint8_t>(signature & 0xC0U);

    bus.write8(data_address, 0x40);
    controller_delay();
    const auto second_phase = static_cast<std::uint8_t>(
        bus.read8(data_address) & 0x3FU);
    signature = static_cast<std::uint8_t>(~(signature | second_phase));
    bus.write8(signature_address, signature);
    bus.write16(shadowrun::genesis::kZ80BusRequest, 0);
    return signature;
}

} // namespace

namespace shadowrun {

void ShadowrunRuntime::wait_for_frame() const {
    bus_.write16(ram::kFrameCounter,
                 static_cast<std::uint16_t>(bus_.read16(ram::kFrameCounter) + 1U));
    while (bus_.read16(ram::kFrameObserved) == bus_.read16(ram::kFrameTarget)) {
    }
    bus_.write16(ram::kFrameObserved, bus_.read16(ram::kFrameTarget));
}

std::uint16_t ShadowrunRuntime::poll_input_flags() const {
    const auto flags = static_cast<std::uint16_t>(
        (bus_.read16(ram::kInputPort0) | bus_.read16(ram::kInputPort1)) & 0xFF00U);
    if (flags != 0) {
        bus_.write16(ram::kInputStatus, 0xFFFFU);
        if ((bus_.read16(ram::kInputPort0) & 0x8000U) != 0) {
            bus_.write16(ram::kInputStatus2, 0xFFFFU);
        }
    }
    return flags;
}

void ShadowrunRuntime::initialise_controller_ports() const {
    // ROM 0x52DC2 requests the Z80 bus and waits for the request bit to clear.
    // GenesisMemoryBus models an immediately available Z80, while an emulator
    // bus can expose the real handshake through the same addresses.
    probe_controller_port(bus_, genesis::kControllerData,
                          genesis::kControllerControl,
                          ram::kControllerSignature);
}

void ShadowrunRuntime::sample_controller_input() const {
    const auto first = probe_controller_port(
        bus_, genesis::kControllerData, genesis::kControllerControl,
        ram::kControllerSignature);
    const auto old_first = bus_.read16(ram::kControllerState0);
    bus_.write16(ram::kControllerState0, first);
    bus_.write16(ram::kInputPort0, static_cast<std::uint16_t>(
        first | static_cast<std::uint16_t>(
            ((first ^ old_first) & first) << 8U)));
    bus_.write16(ram::kControllerDecode0,
                 rom::kControllerInputDecode[first & 0x0FU]);

    const auto second = probe_controller_port(
        bus_, genesis::kControllerData2, genesis::kControllerControl2,
        ram::kControllerSignature + 1U);
    const auto old_second = bus_.read16(ram::kControllerState1);
    bus_.write16(ram::kControllerState1, second);
    bus_.write16(ram::kInputPort1, static_cast<std::uint16_t>(
        second | static_cast<std::uint16_t>(
            ((second ^ old_second) & second) << 8U)));
    bus_.write16(ram::kControllerDecode1,
                 rom::kControllerInputDecode[second & 0x0FU]);
}

void ShadowrunRuntime::upload_vdp_word_buffer() const {
    GenesisAddress source = ram::kVdpUploadBuffer;
    std::uint8_t loop_byte = 0;
    for (;;) {
        bus_.write16(genesis::kVdpData, 0);
        bus_.write16(genesis::kVdpControl, 0);
        bus_.write16(genesis::kVdpData, bus_.read16(source));
        source += 2U;
        loop_byte = static_cast<std::uint8_t>(loop_byte + 2U);
        if ((loop_byte & 0x80U) != 0) {
            break;
        }
    }
}

void ShadowrunRuntime::service_display_interrupt() const {
    bus_.write32(genesis::kVdpControl, 0xC05E0000U);
    const auto counter = static_cast<std::uint16_t>(
        bus_.read16(ram::kFrameSubcounter) + 1U);
    bus_.write16(ram::kFrameSubcounter, counter);
    auto source = (counter & 1U) == 0
        ? ram::kDisplayBufferEven
        : ram::kDisplayBufferOdd;
    for (unsigned int word = 0; word < 2; ++word) {
        for (;;) {
            const auto status = bus_.read8(genesis::kVdpStatusByte);
            bus_.write8(genesis::kVdpStatusByte,
                        static_cast<std::uint8_t>(status & ~0x04U));
            if ((status & 0x04U) != 0) {
                break;
            }
        }
        bus_.write16(genesis::kVdpData, bus_.read16(source));
        source += 2U;
    }
}

void ShadowrunRuntime::write_vdp_addressed_word() const {
    const auto value = bus_.read16(ram::kVdpAddressWord);
    write_vdp_command_sequence(value);
    bus_.write16(genesis::kVdpData, bus_.read16(ram::kVdpDataWord));
}

void ShadowrunRuntime::write_vdp_register_pair() const {
    bus_.write32(genesis::kVdpControl, 0x40000010U);
    bus_.write16(genesis::kVdpData, bus_.read16(ram::kVdpRegisterWord0A));
    bus_.write16(genesis::kVdpData, bus_.read16(ram::kVdpRegisterWord0E));
}

void ShadowrunRuntime::write_vdp_command_sequence(
    std::uint16_t value) const {
    // ROM 0x0E9C preserves D1, writes (D0 & $3FFF) | $4000, then writes the
    // low two bits of ROL.W #2,D0 to the VDP control port.
    write_vdp_command_sequence_direct(bus_, value);
}

void ShadowrunRuntime::stream_vdp_grid(
    GenesisAddress source,
    std::uint32_t command,
    std::uint16_t inner_dbf_counter,
    std::uint16_t outer_dbf_counter,
    std::uint16_t word_addend) const {
    auto row_command = command;
    auto row_counter = outer_dbf_counter;
    for (;;) {
        write_vdp_command_sequence(static_cast<std::uint16_t>(row_command));
        auto word_counter = inner_dbf_counter;
        for (;;) {
            const auto value = static_cast<std::uint16_t>(
                bus_.read16(source) + word_addend);
            bus_.write16(genesis::kVdpData, value);
            source += sizeof(std::uint16_t);
            --word_counter;
            if (word_counter == 0xFFFFU) {
                break;
            }
        }
        row_command = (row_command & 0xFFFF0000U) |
            static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(row_command) + 0x0080U);
        --row_counter;
        if (row_counter == 0xFFFFU) {
            break;
        }
    }
}

DeltaVdpStreamResult ShadowrunRuntime::stream_delta_vdp(
    GenesisAddress source,
    std::uint32_t command,
    std::uint16_t d7_word_count,
    std::uint16_t d5_word_count,
    std::uint16_t d1_row_count,
    std::uint16_t d2_word_mask,
    std::uint16_t d3_output_skip) const {
    const auto initial_source = source;
    const auto base_word = bus_.read16(source);
    source += sizeof(std::uint16_t);
    write_vdp_command_sequence(static_cast<std::uint16_t>(command));

    const auto original_d7 = d7_word_count;
    const auto d7_compare = static_cast<std::uint16_t>(
        d7_word_count - d5_word_count);
    std::uint16_t output_index = 0;
    auto row_counter = d1_row_count;
    std::uint16_t words_remaining = original_d7;
    std::size_t rows_completed = 0;

    const auto emit_candidate = [&](std::uint16_t value) {
        output_index = static_cast<std::uint16_t>(output_index + 1U);
        const auto skip_difference = static_cast<std::uint16_t>(
            d3_output_skip - output_index);
        if (static_cast<std::int16_t>(skip_difference) >= 0) {
            return false;
        }

        if (static_cast<std::int16_t>(words_remaining) >
            static_cast<std::int16_t>(d7_compare)) {
            value = static_cast<std::uint16_t>(value | d2_word_mask);
        }
        bus_.write16(genesis::kVdpData, value);
        --words_remaining;
        if (words_remaining != 0xFFFFU) {
            return false;
        }

        --row_counter;
        ++rows_completed;
        if (static_cast<std::int16_t>(row_counter) < 0) {
            return true;
        }
        command += 0x80U;
        write_vdp_command_sequence(static_cast<std::uint16_t>(command));
        words_remaining = original_d7;
        return false;
    };

    for (;;) {
        const auto token = bus_.read8(source++);
        if (token == 0) {
            return {rows_completed,
                    static_cast<std::size_t>(source - initial_source), true};
        }

        if (token > 0xC0U) {
            const auto count = static_cast<std::uint8_t>(token & 0x1FU);
            const auto wide = (token & 0x20U) != 0;
            const auto iterations = count == 0 ? 1U : count;
            for (unsigned int iteration = 0; iteration < iterations;
                 ++iteration) {
                std::uint16_t value = 0;
                if (wide) {
                    value = static_cast<std::uint16_t>(
                        static_cast<std::uint16_t>(bus_.read8(source++)) << 8U);
                }
                value = static_cast<std::uint16_t>(
                    value | bus_.read8(source++));
                value = static_cast<std::uint16_t>(value + base_word);
                if (emit_candidate(value)) {
                    return {rows_completed,
                            static_cast<std::size_t>(source - initial_source),
                            false};
                }
            }
            continue;
        }

        std::int16_t delta = 1;
        if (token > 0x80U) {
            delta = 1;
        } else if (token > 0x40U) {
            delta = 0;
        } else {
            delta = -1;
        }
        const auto count = static_cast<std::uint8_t>(token & 0x1FU);
        const auto wide = (token & 0x20U) != 0;
        for (unsigned int iteration = 0; iteration <= count; ++iteration) {
            std::uint16_t value = 0;
            if (wide) {
                value = static_cast<std::uint16_t>(
                    static_cast<std::uint16_t>(bus_.read8(source++)) << 8U);
            }
            value = static_cast<std::uint16_t>(value | bus_.read8(source++));
            value = static_cast<std::uint16_t>(value + base_word);
            if (emit_candidate(value)) {
                return {rows_completed,
                        static_cast<std::size_t>(source - initial_source),
                        false};
            }
            value = static_cast<std::uint16_t>(
                value + static_cast<std::uint16_t>(delta));
        }
    }
}

PrimaryVdpStreamResult ShadowrunRuntime::stream_primary_vdp(
    GenesisAddress source,
    std::uint32_t command,
    std::uint32_t byte_count) const {
    const auto size_word = static_cast<std::uint16_t>(byte_count);
    PrimaryVdpStreamResult result{};

    const auto emit_chunk = [&](GenesisAddress chunk_source,
                                std::uint32_t chunk_command,
                                std::uint16_t chunk_size) {
        write_vdp_command_sequence(
            static_cast<std::uint16_t>(chunk_command));
        ++result.segments;

        const auto longword_count = static_cast<std::uint16_t>(
            chunk_size >> 6U);
        for (std::uint32_t index = 0; index < longword_count; ++index) {
            bus_.write32(genesis::kVdpData, bus_.read32(chunk_source));
            chunk_source += 64U;
            ++result.longwords_emitted;
        }

        const auto remainder_longwords = static_cast<std::uint16_t>(
            (chunk_size >> 2U) & 0x000FU);
        for (std::uint32_t index = 0; index < remainder_longwords; ++index) {
            bus_.write32(genesis::kVdpData, bus_.read32(chunk_source));
            chunk_source += 4U;
            ++result.longwords_emitted;
        }

        if ((chunk_size & 0x0002U) != 0) {
            bus_.write16(genesis::kVdpData, bus_.read16(chunk_source));
            ++result.words_emitted;
        }
    };

    const auto low_source = static_cast<std::uint16_t>(source);
    const auto boundary_delta = static_cast<std::uint16_t>(
        0U - low_source);
    const auto sum = source + static_cast<std::uint32_t>(size_word);
    const auto crossed_boundary =
        ((sum ^ source) & 0x00010000U) != 0;

    if (!crossed_boundary) {
        emit_chunk(source, command, size_word);
    } else {
        result.split_at_64k_boundary = true;
        emit_chunk(source, command, boundary_delta);
        source += boundary_delta;
        command += boundary_delta;
        const auto remaining = static_cast<std::uint16_t>(
            size_word - boundary_delta);
        emit_chunk(source, command, remaining);
    }

    result.source_bytes_consumed = static_cast<std::size_t>(
        size_word & 0xFFFEU);
    return result;
}

CompressedVdpStreamResult ShadowrunRuntime::decode_compressed_vdp(
    GenesisAddress source,
    GenesisAddress staging,
    std::uint16_t command) const {
    const auto initial_source = source;
    const auto initial_counter = bus_.read16(source);
    source += sizeof(std::uint16_t);
    auto d2_counter = initial_counter;

    // 0x1540 writes 0x8F02 before 0x0E9C emits the caller's command.
    bus_.write16(genesis::kVdpControl, 0x8F02U);
    write_vdp_command_sequence(command);

    std::uint16_t ring_index = 0;
    std::uint8_t control = 0;
    std::uint16_t control_bits_remaining = 1;
    std::size_t output_bytes = 0;

    const auto emit_byte = [&](std::uint8_t value) {
        bus_.write8(staging + ring_index, value);
        ring_index = static_cast<std::uint16_t>((ring_index + 1U) & 0x07FFU);
        const auto vdp_value = (initial_counter & 1U) == 0
            ? static_cast<std::uint16_t>(value)
            : static_cast<std::uint16_t>(value) << 8U;
        bus_.write16(genesis::kVdpData, vdp_value);
        --d2_counter;
        ++output_bytes;
    };

    for (;;) {
        --control_bits_remaining;
        if (control_bits_remaining == 0) {
            control = bus_.read8(source++);
            control_bits_remaining = 8;
        }
        const auto compressed = (control & 0x80U) != 0;
        control = static_cast<std::uint8_t>(control << 1U);

        if (!compressed) {
            emit_byte(bus_.read8(source++));
            if (d2_counter == 0xFFFFU) {
                break;
            }
            continue;
        }

        const auto run_descriptor = bus_.read8(source++);
        if ((run_descriptor & 0x80U) == 0) {
            const auto run_counter = static_cast<std::uint8_t>(
                (run_descriptor & 0x7FU) + 2U);
            const auto value = bus_.read8(source++);
            // The ROM loads D3 with N+2 and then uses DBF, yielding N+3
            // output bytes.
            for (unsigned int index = 0; index <= run_counter; ++index) {
                emit_byte(value);
            }
        } else {
            const auto offset_low = bus_.read8(source++);
            const auto offset = static_cast<std::uint16_t>(
                ((static_cast<std::uint16_t>(offset_low) << 3U) |
                 ((run_descriptor >> 4U) & 0x0007U)) & 0x07FFU);
            const auto run_counter = static_cast<std::uint8_t>(
                (run_descriptor & 0x0FU) + 2U);
            auto copy_index = static_cast<std::uint16_t>(
                (ring_index - offset) & 0x07FFU);
            for (unsigned int index = 0; index <= run_counter; ++index) {
                const auto value = bus_.read8(staging + copy_index);
                copy_index = static_cast<std::uint16_t>(
                    (copy_index + 1U) & 0x07FFU);
                emit_byte(value);
            }
        }

        // The compressed paths perform SUBQ once per output and finish with
        // ADDQ followed by DBF; their net counter change is one decrement per
        // emitted byte, with the branch testing the resulting word.
        if (d2_counter == 0xFFFFU) {
            break;
        }
    }

    return {output_bytes,
            static_cast<std::size_t>(source - initial_source),
            initial_counter,
            static_cast<std::uint16_t>(initial_counter + 1U)};
}

namespace {

shadowrun::GenesisAddress subtract_low_byte(
    shadowrun::GenesisAddress address, std::uint8_t amount) {
    return (address & 0x00FFFF00U) |
        static_cast<std::uint8_t>(address - amount);
}

shadowrun::GenesisAddress add_low_byte(
    shadowrun::GenesisAddress address, std::uint8_t amount) {
    return (address & 0x00FFFF00U) |
        static_cast<std::uint8_t>(address + amount);
}

} // namespace

void ShadowrunRuntime::service_vblank_upload_queue() const {
    const auto count = bus_.read16(ram::kVblankWorkA);
    if (count == 0) {
        return;
    }

    auto queue_cursor = subtract_low_byte(
        bus_.read32(ram::kVblankWorkAPointer),
        static_cast<std::uint8_t>(static_cast<std::uint8_t>(count) << 2U));
    for (std::uint16_t index = 0; index < count; ++index) {
        const auto object = bus_.read32(queue_cursor);
        auto source = bus_.read32(object + 0x22U);
        const auto flags = bus_.read8(object + 0x11U);
        if ((flags & 0x80U) != 0) {
            source = bus_.read32(object + 0x44U);
        }
        if ((flags & 0x20U) != 0) {
            source = bus_.read32(object + 0x48U);
        }
        stream_primary_vdp(source,
                           bus_.read16(object + 0x26U),
                           bus_.read16(object + 0x28U));
        queue_cursor = add_low_byte(queue_cursor, 4U);
    }
    bus_.write16(ram::kVblankWorkA, 0);
}

void ShadowrunRuntime::service_vblank_descriptor_queue() const {
    const auto count = bus_.read16(ram::kVblankWorkB);
    if (count == 0) {
        return;
    }

    auto queue_cursor = subtract_low_byte(
        bus_.read32(ram::kVblankWorkBPointer),
        static_cast<std::uint8_t>(static_cast<std::uint8_t>(count) << 3U));
    for (std::uint16_t index = 0; index < count; ++index) {
        const auto source = bus_.read32(queue_cursor);
        const auto byte_count = bus_.read16(queue_cursor + 4U);
        const auto command = bus_.read16(queue_cursor + 6U);
        stream_primary_vdp(source, command, byte_count);
        queue_cursor = add_low_byte(queue_cursor, 8U);
    }
    bus_.write16(ram::kVblankWorkB, 0);
}

void ShadowrunRuntime::service_object_list_upload() const {
    const auto count = bus_.read16(ram::kObjectListCount);
    if (count >= 0x0050U) {
        bus_.write8(ram::kObjectUploadStatus, 0);
    }

    const auto count_plus_one = static_cast<std::uint16_t>(count + 1U);
    if (count_plus_one == 0) {
        initialise_object_output(bus_);
        return;
    }

    stream_primary_vdp(ram::kObjectListScratch,
                       bus_.read16(ram::kObjectListVdpCommand),
                       static_cast<std::uint16_t>(count_plus_one << 3U));
}

void ShadowrunRuntime::stream_tile_plane(
    GenesisAddress source,
    GenesisAddress tile_table,
    std::uint16_t vdp_command) const {
    // ROM 0x41B6: convert the camera words to tile coordinates and compensate
    // the VDP origin. The signed tests are word-sized on the 68000.
    auto d3 = bus_.read16(ram::kVdpDataWord);
    auto d4 = bus_.read16(ram::kVdpRegisterWord0A);
    auto d0 = static_cast<std::uint16_t>(bus_.read16(ram::kCameraX) >> 4U);
    auto d1 = static_cast<std::uint16_t>(bus_.read16(ram::kCameraY) >> 4U);
    if (static_cast<std::int16_t>(d0 - 8U) < 0) {
        d0 = static_cast<std::uint16_t>(d0 + 8U);
    } else {
        d3 = static_cast<std::uint16_t>(d3 + 8U);
    }
    if (static_cast<std::int16_t>(d1 - 8U) < 0) {
        d1 = static_cast<std::uint16_t>(d1 + 8U);
    } else {
        d4 = static_cast<std::uint16_t>(d4 - 8U);
    }

    // ROM 0x4000: derive the tile-plane offset from the signed horizontal
    // word and the VDP register word.
    d3 = static_cast<std::uint16_t>(0U - d3);
    d3 = static_cast<std::uint16_t>(d3 & 0x01FFU);
    d4 = static_cast<std::uint16_t>(d4 & 0x00F8U);
    d3 = static_cast<std::uint16_t>(d3 >> 2U);
    d4 = static_cast<std::uint16_t>(d4 << 4U);
    d4 = static_cast<std::uint16_t>(d4 + d3);
    d4 = static_cast<std::uint16_t>(d4 & 0xFFFEU);
    d4 = static_cast<std::uint16_t>(d4 + vdp_command);

    const auto tile_width = bus_.read16(ram::kTilePlaneWidth);
    const auto draw_span = [&](std::uint16_t span_command,
                               std::uint16_t source_x,
                               std::uint16_t source_y) {
        // ROM 0x3FD8: map the source coordinates into a table byte offset and
        // a source byte offset. D3's high product word is immaterial because
        // the subsequent indexed access uses D3.W.
        const auto original_x = source_x;
        const auto original_y = source_y;
        const auto table_x = static_cast<std::uint16_t>(
            (original_x >> 3U) & 0x0003U);
        const auto table_y = static_cast<std::uint16_t>(
            (original_y >> 3U) & 0x0003U);
        auto table_offset_long = static_cast<std::uint32_t>(
            static_cast<std::uint32_t>(original_y >> 5U) * tile_width);
        table_offset_long += static_cast<std::uint32_t>(
            static_cast<std::uint16_t>((original_x >> 5U) << 1U));
        auto table_offset = static_cast<std::uint16_t>(table_offset_long);
        auto source_offset = static_cast<std::uint16_t>(
            (((table_y << 2U) + table_x) << 1U));

        const auto write_span = [&](std::uint16_t command,
                                    std::uint16_t dbf_counter) {
            write_vdp_command_sequence(command);
            auto tile_data_offset = static_cast<std::uint32_t>(
                bus_.read16(tile_table + table_offset)) << 5U;
            auto row_source_offset = source_offset;
            auto row_table_offset = table_offset;
            for (std::uint32_t index = 0; index <= dbf_counter; ++index) {
                auto value = bus_.read16(source + tile_data_offset +
                                         row_source_offset);
                const auto mask = bus_.read16(ram::kTilePlaneMask);
                if (static_cast<std::int16_t>(mask) < 0) {
                    value = static_cast<std::uint16_t>(value ^ 0x8000U);
                } else {
                    value = static_cast<std::uint16_t>(value | mask);
                }
                bus_.write16(genesis::kVdpData, value);
                row_source_offset = static_cast<std::uint16_t>(
                    row_source_offset + 2U);
                if ((row_source_offset & 0x0007U) == 0) {
                    row_source_offset = static_cast<std::uint16_t>(
                        row_source_offset - 8U);
                    row_table_offset = static_cast<std::uint16_t>(
                        row_table_offset + 2U);
                    tile_data_offset = static_cast<std::uint32_t>(
                        bus_.read16(tile_table + row_table_offset)) << 5U;
                }
            }
        };

        auto dbf_counter = static_cast<std::uint16_t>(
            span_command & 0x007FU);
        dbf_counter = static_cast<std::uint16_t>(dbf_counter - 0x0080U);
        dbf_counter = static_cast<std::uint16_t>(0U - dbf_counter);
        dbf_counter = static_cast<std::uint16_t>(dbf_counter >> 1U);
        dbf_counter = static_cast<std::uint16_t>(dbf_counter - 1U);
        if (static_cast<std::int16_t>(dbf_counter) <
            static_cast<std::int16_t>(0x002BU)) {
            const auto first_counter = dbf_counter;
            write_span(span_command, first_counter);
            dbf_counter = static_cast<std::uint16_t>(
                0U - static_cast<std::uint16_t>(first_counter - 0x002AU));
            write_span(static_cast<std::uint16_t>(span_command & 0xFF80U),
                       dbf_counter);
        } else {
            write_span(span_command, 0x002BU);
        }
    };

    auto row_command = d4;
    auto row_source_y = d1;
    for (std::uint32_t row = 0; row <= 0x001EU; ++row) {
        draw_span(row_command, d0, row_source_y);
        const auto high_nibble = static_cast<std::uint16_t>(
            row_command & 0xF000U);
        row_command = static_cast<std::uint16_t>(
            high_nibble | ((row_command + 0x0080U) & 0x0FFFU));
        row_source_y = static_cast<std::uint16_t>(row_source_y + 8U);
    }
}

FrameTilePlaneUpdateResult ShadowrunRuntime::
    stream_frame_tile_plane_vertical_0x4018(std::uint16_t initial_d0) const {
    FrameTilePlaneUpdateResult result{0, 0, false};

    const auto emit_row = [&](std::uint16_t source_y,
                              std::uint16_t base_vdp_offset,
                              std::uint16_t source_x) {
        const auto emit_plane = [&](GenesisAddress source,
                                    GenesisAddress tile_table,
                                    std::uint16_t command) {
            result.words_emitted += stream_tile_plane_span_0x3D4A(
                bus_, source, tile_table, command, source_x, source_y);
            ++result.draw_calls;
        };
        emit_plane(bus_.read32(ram::kTilePlaneSourceA), 0x00FF2700U,
                   static_cast<std::uint16_t>(
                       base_vdp_offset + bus_.read16(ram::kVdpClearCommandB)));
        bus_.write16(ram::kTilePlaneMask, 0x8000U);
        emit_plane(bus_.read32(ram::kTilePlaneSourceB), 0x00FF4700U,
                   static_cast<std::uint16_t>(
                       base_vdp_offset + bus_.read16(ram::kVdpClearCommandA)));
        bus_.write16(ram::kTilePlaneMask, 0);
    };

    auto d1 = static_cast<std::uint16_t>(
        bus_.read16(ram::kCameraY) >> 4U);
    if (static_cast<std::int16_t>(initial_d0) < 0) {
        const auto before_subtract = d1;
        d1 = static_cast<std::uint16_t>(d1 - 8U);
        if (before_subtract < 8U) {
            result.early_return = true;
            return result;
        }

        const auto origin = adjust_tile_origin_0x4216(bus_);
        d1 = static_cast<std::uint16_t>(d1 + 0x00E0U);
        auto d4 = static_cast<std::uint16_t>(
            bus_.read16(ram::kVdpRegisterWord0A) - 8U);
        d4 = calculate_tile_vdp_offset(origin.vdp_data, d4);
        emit_row(d1, d4, origin.source_x);
    }

    for (;;) {
        d1 = static_cast<std::uint16_t>(d1 + 8U);
        if (static_cast<std::int16_t>(d1) >
            static_cast<std::int16_t>(bus_.read16(ram::kVdpRegisterWord0E))) {
            result.early_return = true;
            break;
        }

        const auto origin = adjust_tile_origin_0x4216(bus_);
        d1 = static_cast<std::uint16_t>(d1 + 0x00E0U);
        auto d4 = static_cast<std::uint16_t>(
            bus_.read16(ram::kVdpRegisterWord0A) + 0x00E8U);
        d4 = calculate_tile_vdp_offset(origin.vdp_data, d4);
        emit_row(d1, d4, origin.source_x);
    }
    return result;
}

FrameTilePlaneUpdateResult ShadowrunRuntime::
    stream_frame_tile_plane_horizontal_0x40E2(std::uint16_t initial_d0) const {
    // 0x40E4 is BCS immediately after TST.W D0. TST clears C on the 68000,
    // so the alternate 0x414A loop is unreachable from this entry; the
    // ordinary 0x40E8 path is the one used by 0x3BAC.
    (void)initial_d0;
    FrameTilePlaneUpdateResult result{0, 0, false};
    auto d0 = static_cast<std::uint16_t>(
        bus_.read16(ram::kCameraX) >> 4U);
    if (d0 < 8U) {
        result.early_return = true;
        return result;
    }
    d0 = static_cast<std::uint16_t>(d0 - 8U);

    const auto origin = adjust_tile_origin_0x41F0(bus_);
    auto d3 = static_cast<std::uint16_t>(
        bus_.read16(ram::kVdpDataWord) + 8U);
    auto d4 = calculate_tile_vdp_offset(d3, origin.vdp_register);

    const auto emit_plane = [&](GenesisAddress source,
                                GenesisAddress tile_table,
                                std::uint16_t command) {
        result.words_emitted += stream_tile_plane_column_0x3DDE(
            bus_, source, tile_table, command, d0, origin.source_y);
        ++result.draw_calls;
    };
    emit_plane(bus_.read32(ram::kTilePlaneSourceA), 0x00FF2700U,
               static_cast<std::uint16_t>(
                   d4 + bus_.read16(ram::kVdpClearCommandB)));
    bus_.write16(ram::kTilePlaneMask, 0x8000U);
    emit_plane(bus_.read32(ram::kTilePlaneSourceB), 0x00FF4700U,
               static_cast<std::uint16_t>(
                   d4 + bus_.read16(ram::kVdpClearCommandA)));
    bus_.write16(ram::kTilePlaneMask, 0);
    return result;
}

void ShadowrunRuntime::service_vblank_tile_plane_a() const {
    bus_.write16(ram::kVblankQueueA, 0);
    stream_tile_plane(bus_.read32(ram::kTilePlaneSourceA),
                      0x00FF2700U,
                      bus_.read16(ram::kVdpClearCommandB));
}

void ShadowrunRuntime::service_vblank_tile_plane_b() const {
    bus_.write16(ram::kVblankQueueB, 0);
    bus_.write16(ram::kTilePlaneMask, 0x8000U);
    stream_tile_plane(bus_.read32(ram::kTilePlaneSourceB),
                      0x00FF4700U,
                      bus_.read16(ram::kVdpClearCommandA));
    bus_.write16(ram::kTilePlaneMask, 0);
}

void ShadowrunRuntime::service_vblank_state_animation() const {
    if (bus_.read8(ram::kVblankAnimationLock) != 0) {
        return;
    }

    const auto state = bus_.read16(ram::kVblankAnimationState);
    if (state == 0x0014U) {
        return;
    }

    const auto write_animation_word = [&](std::uint16_t command,
                                          GenesisAddress table,
                                          std::uint16_t table_offset) {
        bus_.write16(genesis::kVdpControl, command);
        bus_.write16(genesis::kVdpControl, 0);
        bus_.write16(genesis::kVdpData,
                     bus_.read16(table + table_offset));
    };

    const auto advance_animation = [&](std::uint16_t reload,
                                       std::uint16_t index_limit,
                                       GenesisAddress table,
                                       std::uint16_t index_stride,
                                       std::uint16_t command) {
        auto timer = static_cast<std::uint16_t>(
            bus_.read16(ram::kVblankAnimationTimer) - 1U);
        bus_.write16(ram::kVblankAnimationTimer, timer);
        if (static_cast<std::int16_t>(timer) >= 0) {
            return;
        }
        bus_.write16(ram::kVblankAnimationTimer, reload);
        auto index = static_cast<std::uint16_t>(
            bus_.read16(ram::kVblankAnimationIndex) + 1U);
        if (index >= index_limit) {
            index = 0;
        }
        bus_.write16(ram::kVblankAnimationIndex, index);
        write_animation_word(command, table,
                             static_cast<std::uint16_t>(
                                 index * index_stride));
    };

    const auto mode = bus_.read16(ram::kVblankAnimationMode);
    if (mode == 1U) {
        advance_animation(6, 4, rom::kVblankAnimationMode1Table, 2,
                           0xC014U);
        return;
    }

    if (mode == 2U) {
        if ((bus_.read16(ram::kFrameCounter) & 0x000FU) != 0x000FU) {
            return;
        }
        auto timer = bus_.read16(ram::kVblankAnimationTimer);
        auto index = bus_.read16(ram::kVblankAnimationIndex);
        if (static_cast<std::int16_t>(timer) < 0) {
            index = static_cast<std::uint16_t>((index - 1U) & 0x0007U);
            if (index == 7U) {
                timer = 0;
                index = 0;
            }
        } else {
            index = static_cast<std::uint16_t>((index + 1U) & 0x0007U);
            if (index == 0U) {
                timer = 0xFFFFU;
                index = 7;
            }
        }
        bus_.write16(ram::kVblankAnimationTimer, timer);
        bus_.write16(ram::kVblankAnimationIndex, index);
        bus_.write16(genesis::kVdpControl, 0xC01EU);
        bus_.write16(genesis::kVdpControl, 0);
        bus_.write16(genesis::kVdpData,
                     static_cast<std::uint16_t>(index << 9U));
        return;
    }

    if (mode != 0U) {
        return;
    }

    if (state >= 6U && state <= 9U) {
        advance_animation(0x000CU, 3,
                           rom::kVblankAnimationState69Table, 8,
                           0xC01AU);
    } else if (state == 4U) {
        advance_animation(0x000AU, 3,
                           rom::kVblankAnimationState4Table, 8,
                           0xC00CU);
    } else if (state == 5U) {
        advance_animation(0x000AU, 3,
                           rom::kVblankAnimationState5Table, 2,
                           0xC014U);
    } else if (state == 0U) {
        advance_animation(6, 6,
                           rom::kVblankAnimationState0Table, 2,
                           0xC014U);
    } else if (state > 0x002BU && state < 0x0032U) {
        advance_animation(9, 4,
                           rom::kVblankAnimationState2CTable, 2,
                           0xC018U);
    }
}

void ShadowrunRuntime::clear_vdp_word_block() const {
    // ROM 0x1056 loads D0 with FFE0, calls 0x0E9C, then emits sixteen zero
    // words to the VDP data port.
    write_vdp_command_sequence(0xFFE0U);
    for (std::size_t word = 0; word < 16; ++word) {
        bus_.write16(genesis::kVdpData, 0);
    }
}

void ShadowrunRuntime::clear_vdp_block_a() const {
    clear_vdp_word_block();
    write_vdp_command_sequence(bus_.read16(ram::kVdpClearCommandA));
    for (std::size_t word = 0; word <= 0x3FFU; ++word) {
        bus_.write32(genesis::kVdpData, 0x07FF07FFU);
    }
}

void ShadowrunRuntime::clear_vdp_block_b() const {
    clear_vdp_word_block();
    write_vdp_command_sequence(bus_.read16(ram::kVdpClearCommandB));
    for (std::size_t word = 0; word <= 0x3FFU; ++word) {
        bus_.write32(genesis::kVdpData, 0x07FF07FFU);
    }
}

VBlankServiceResult ShadowrunRuntime::service_vblank(
    const VBlankRoutineHooks& hooks) const {
    const auto tick = static_cast<std::uint16_t>(
        bus_.read16(ram::kVblankTick) + 1U);
    bus_.write16(ram::kVblankTick, tick);
    bus_.write16(ram::kFrameSubcounter, 0);
    const auto flags_seen = bus_.read8(ram::kVblankFlags);

    const auto clear_flag = [&](std::uint8_t mask) {
        const auto flags = bus_.read8(ram::kVblankFlags);
        bus_.write8(ram::kVblankFlags,
                    static_cast<std::uint8_t>(flags & ~mask));
        return (flags & mask) != 0;
    };

    if (clear_flag(0x02U)) {
        if (hooks.routine_0x0EE6) {
            hooks.routine_0x0EE6(bus_);
        } else {
            service_object_list_upload();
        }
        bus_.write16(ram::kInputStatus, 1);
    }
    if (clear_flag(0x01U)) {
        upload_vdp_word_buffer();
    }
    if (bus_.read16(ram::kVblankIdleGate) == 0) {
        if (hooks.routine_0x5672C) {
            hooks.routine_0x5672C(bus_);
        } else {
            service_vblank_state_animation();
        }
    }
    if (clear_flag(0x04U)) {
        write_vdp_addressed_word();
    }
    if (clear_flag(0x08U)) {
        write_vdp_register_pair();
    }
    if (bus_.read16(ram::kVblankQueueA) != 0) {
        if (hooks.routine_0x2C92) {
            hooks.routine_0x2C92(bus_);
        } else {
            service_vblank_tile_plane_a();
        }
    }
    if (bus_.read16(ram::kVblankQueueB) != 0) {
        if (hooks.routine_0x2CB2) {
            hooks.routine_0x2CB2(bus_);
        } else {
            service_vblank_tile_plane_b();
        }
    }
    if (bus_.read16(ram::kVblankWorkA) != 0) {
        if (hooks.routine_0x6148) {
            hooks.routine_0x6148(bus_);
        } else {
            service_vblank_upload_queue();
        }
    }
    if (bus_.read16(ram::kVblankWorkB) != 0) {
        if (hooks.routine_0x61F4) {
            hooks.routine_0x61F4(bus_);
        } else {
            service_vblank_descriptor_queue();
        }
    }
    return {tick, flags_seen};
}

std::size_t ShadowrunRuntime::service_frame_input(
    std::uint16_t dbf_counter, const FrameWaitRoutine& frame_wait) const {
    std::size_t passes = 0;
    for (;;) {
        if (frame_wait) {
            frame_wait();
        } else {
            wait_for_frame();
        }
        poll_input_flags();
        ++passes;
        --dbf_counter;
        if (dbf_counter == 0xFFFFU) {
            break;
        }
    }
    return passes;
}

PaletteTransitionResult ShadowrunRuntime::fade_palette_to_target(
    const FrameWaitRoutine& frame_wait) const {
    PaletteAnimator palette(bus_);
    std::size_t frame_waits = 0;
    for (std::size_t step = 0; step <= 0x10U; ++step) {
        // ROM 0x1342 starts D5 at one and uses DBF, so this is two complete
        // 0x0BD2/0x0BF2 passes for each outer palette iteration.
        frame_waits += service_frame_input(1, frame_wait);
        palette.step_toward_target();
        bus_.write8(ram::kVblankFlags, static_cast<std::uint8_t>(
            bus_.read8(ram::kVblankFlags) | 0x01U));
    }
    return {0x11U, frame_waits};
}

PaletteTransitionResult ShadowrunRuntime::fade_palette_to_black(
    const FrameWaitRoutine& frame_wait) const {
    PaletteAnimator palette(bus_);
    std::size_t frame_waits = 0;
    for (std::size_t step = 0; step <= 0x10U; ++step) {
        frame_waits += service_frame_input(1, frame_wait);
        palette.step_toward_black();
        bus_.write8(ram::kVblankFlags, static_cast<std::uint8_t>(
            bus_.read8(ram::kVblankFlags) | 0x01U));
    }
    return {0x11U, frame_waits};
}

std::uint16_t ShadowrunRuntime::advance_random() const {
    const auto current = bus_.read16(ram::kRandomState);
    const auto next = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(current) * 0x20E5U + 1U);
    bus_.write16(ram::kRandomState, next);
    return next;
}

std::uint16_t ShadowrunRuntime::random_below(
    std::uint16_t upper_bound) const {
    // ROM 0x0D3A calls 0x0D44, MULU.Ws the 16-bit state by D1, swaps the
    // product words, and returns the former high word. This is an unsigned
    // multiply with an exclusive upper bound for the useful callers.
    const auto random_value = advance_random();
    const auto product = static_cast<std::uint32_t>(random_value) *
                         upper_bound;
    return static_cast<std::uint16_t>(product >> 16U);
}

std::int16_t ShadowrunRuntime::scaled_random(std::uint16_t multiplier) const {
    return static_cast<std::int16_t>(random_below(multiplier));
}

void ShadowrunRuntime::apply_vdp_configuration(
    const std::array<std::uint16_t, 19>& configuration) const {
    bus_.write16(0xFFDD16, configuration[0]);
    bus_.write16(0xFFDD14, configuration[1]);
    bus_.write16(0xFFDD10, configuration[2]);
    bus_.write16(0xFFDD12, configuration[3]);
    for (std::size_t index = 4; index < configuration.size(); ++index) {
        bus_.write16(genesis::kVdpControl, configuration[index]);
    }
}

void ShadowrunRuntime::apply_vdp_configuration_from_bus(
    GenesisAddress source) const {
    const GenesisAddress setup_words[] = {
        ram::kVdpClearCommandA,
        ram::kVdpClearCommandB,
        0xFFDD10U,
        ram::kVdpAddressWord
    };
    auto cursor = source;
    for (const auto address : setup_words) {
        bus_.write16(address, bus_.read16(cursor));
        cursor += sizeof(std::uint16_t);
    }
    for (std::size_t index = 0; index < 15; ++index) {
        bus_.write16(genesis::kVdpControl, bus_.read16(cursor));
        cursor += sizeof(std::uint16_t);
    }
}

void ShadowrunRuntime::clear_or_checksum_ram(std::uint32_t checksum_seed) const {
    if (bus_.read32(ram::kBootstrapMagic) == 0x12345678U) {
        // The ROM uses MOVE.W #$F1E7,D7 followed by DBF. DBF terminates after
        // the counter reaches $FFFF, so this clears $F1E8 bytes.
        std::uint16_t counter = 0xF1E7U;
        GenesisAddress address = ram::kRamStart;
        for (;;) {
            bus_.write8(address++, 0);
            if (counter == 0) {
                break;
            }
            --counter;
        }
        return;
    }

    // The non-magic path sums and clears $3FE0 longwords, matching MOVE.W
    // #$3FDF,D7 / DBF in the ROM, then stores the low word as the new seed.
    std::uint32_t checksum = checksum_seed;
    std::uint16_t counter = 0x3FDFU;
    GenesisAddress address = ram::kRamStart;
    for (;;) {
        checksum += bus_.read32(address);
        bus_.write32(address, 0);
        address += 4;
        if (counter == 0) {
            break;
        }
        --counter;
    }
    bus_.write16(ram::kRandomState, static_cast<std::uint16_t>(checksum));
}

} // namespace shadowrun
