#include "shadowrun_ghidra/audio_service_batch_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<std::uint16_t> written;
    std::vector<std::uint16_t> controls;
    int begins = 0;
    assert(write_masked_transfer(
               0x8000, {1, 2, 3}, 2,
               {[&] { ++begins; }, [&](std::uint16_t word) { written.push_back(word); },
                [&](std::uint16_t value) { controls.push_back(value); }},
               true) == 2);
    assert(begins == 1 && written == std::vector<std::uint16_t>({0x8001, 0x8002}) &&
           controls == std::vector<std::uint16_t>({0x8F80, 0x8F02}));

    std::vector<std::uint32_t> three;
    int ones = 0;
    initialize_audio_sequence({1, 2, 3, 4},
                               {[&] { ++begins; }, [&] { ++begins; },
                                [&](std::int32_t value) {
                                    if (value == -1 || value == 0xB) ++ones;
                                },
                                [&](std::uint32_t value) { three.push_back(value); }});
    assert(ones == 2 && three.size() == 4);

    AudioBusCallbacks bus{[&] { written.push_back(1); }, [&] { written.push_back(2); },
                          [&] { written.push_back(3); }, [&] { written.push_back(4); }};
    run_audio_bus_transaction(bus);
    run_audio_transaction_wrapper(bus);
    run_fixed_audio_pair([&](std::uint16_t a, std::uint16_t b) {
        assert(a == 1 && b == 0);
    });
    run_controller_access([&](std::uint16_t value) { assert(value == 1); });

    int flushed = 0;
    initialize_title_audio_assets({1, 2, 3, 4}, {}, [&] { ++flushed; });
    assert(flushed == 1);
    std::vector<std::uint32_t> snapshot;
    copy_audio_state({1, 2}, snapshot, 4);
    assert(snapshot == std::vector<std::uint32_t>({1, 2, 0, 0}));
    const std::vector<AudioStreamTableEntry> audio_table{
        {"music/title.ogg", "music/title-secondary.ogg"},
        {"music/field.ogg", ""},
    };
    const auto primary = select_audio_primary_stream(audio_table, 1);
    const auto secondary = select_audio_secondary_stream(audio_table, 0);
    if (!primary || primary->alias != "music/field.ogg" ||
        primary->bank != AudioStreamBank::primary || !secondary ||
        secondary->alias != "music/title-secondary.ogg" ||
        select_audio_secondary_stream(audio_table, 1)) {
        return 10;
    }

    AudioTitlePreparationState prep{};
    prepare_audio_title(prep, [&] { ++begins; });
    build_title_sprite_once([&] { ++begins; });
    assert(prep.scratch == 0 && (prep.dirty_flags & 2U) != 0U);
    std::vector<std::int32_t> first{1}, second{2};
    reset_audio_menu_arrays(first, second);
    assert(first.size() == 35 && first[0] == 0);
    fill_audio_menu_sentinel(first);
    assert(first.size() == 35 && first[0] == -1);
    begin_title_audio_sequence(prep, {}, {});

    std::vector<std::size_t> indexes;
    std::vector<std::function<bool(std::size_t)>> handlers(25);
    handlers[0] = [](std::size_t) { return true; };
    handlers[22] = [](std::size_t) { return true; };
    assert(run_indexed_title_services(handlers, [&](std::size_t index) { indexes.push_back(index); }) ==
           2);
    assert(indexes.size() == 23 && indexes.back() == 22);
    int grids = 0;
    write_menu_repeated_grid_service([&] { ++grids; });
    assert(grids == 1);
    assert(select_display_status(0) == 0xCC && select_display_status(-1) == 0xC0 &&
           select_display_status(1) == 0x0C);
    return 0;
}
