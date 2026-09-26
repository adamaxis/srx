#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"

#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<int> bus_events;
    const AudioBusBoundaryCallbacks bus{
        [&] { bus_events.push_back(1); }, [&] { bus_events.push_back(2); }};
    begin_controller_bus_access(bus);
    end_controller_bus_access(bus);
    begin_audio_bus_access(bus);
    end_audio_bus_access(bus);
    if (bus_events != std::vector<int>{1, 2, 1, 2}) {
        return 1;
    }

    AudioRamState ram{};
    initialize_audio_ram(ram, {1, 2, 3}, bus);
    if (!ram.reset_asserted || ram.ram.size() != 0x2000U || ram.ram[2] != 3) {
        return 2;
    }
    int reset_asserts = 0;
    int delays = 0;
    int bus_release = 0;
    int reset_release = 0;
    reset_audio_processor(
        ram,
        {[&] { ++reset_asserts; }, [&] { ++delays; }, [&] { ++bus_release; },
         [&] { ++reset_release; }});
    if (ram.reset_asserted || reset_asserts != 1 || delays != 16 ||
        bus_release != 1 || reset_release != 1) {
        return 3;
    }

    AudioCommandQueueState queue{};
    append_audio_command(queue, 0x11);
    flush_audio_command(queue, 0x22);
    if (queue.bytes[0] != 0x11 || queue.bytes[1] != 0xFF ||
        queue.bytes[2] != 0x22 || queue.write_index != 3) {
        return 4;
    }

    std::vector<int> events;
    const AudioCommandSequenceCallbacks sequence{
        [&] { events.push_back(1); }, [&] { events.push_back(2); },
        [&] { events.push_back(3); }, [&] { events.push_back(4); }};
    if (!append_one_with_audio_bus(sequence) ||
        !append_three_with_audio_bus(sequence) ||
        !service_audio_command_queue_batch(sequence) ||
        !flush_with_audio_bus(sequence) || !flush_append_pair(sequence) ||
        !begin_bus_then_flush_append_pair(sequence) ||
        !controller_access_pair(bus)) {
        return 5;
    }
    if (events != std::vector<int>{1, 4, 2, 1, 4, 4, 4, 2, 1, 3, 4, 2,
                                   1, 3, 2, 3, 4, 4, 2, 1, 3, 4, 4, 2}) {
        return 6;
    }

    if (service_audio_if_enabled_variant(false, sequence) ||
        !service_audio_if_enabled_variant(true, sequence)) {
        return 7;
    }
    int prepared = 0;
    if (!service_audio_command_frame_variant(
            true, {[&] { ++prepared; }, sequence}) || prepared != 1 ||
        service_audio_command_frame_variant(false, {[&] { ++prepared; }, sequence})) {
        return 8;
    }
    if (!service_audio_command_wrapper(sequence) ||
        !service_audio_tick_wrapper(sequence)) {
        return 9;
    }

    int ticks = 0;
    int waits = 0;
    int flushes = 0;
    int finalizes = 0;
    prepare_audio_mode_entry(
        {[&] { ++ticks; }, [&] { ++waits; }, [&] { ++flushes; },
         [&] { ++finalizes; }});
    if (ticks != 6 || waits != 6 || flushes != 1 || finalizes != 1) {
        return 10;
    }
    return 0;
}
