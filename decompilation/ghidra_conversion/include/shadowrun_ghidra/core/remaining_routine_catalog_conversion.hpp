#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class RoutineFamily {
    interrupt,
    field,
    menu,
    entity,
    world,
    audio,
    save,
    intro,
    graphics,
};

struct RemainingRoutine {
    std::uint32_t rom_entry = 0;
    std::string_view alias;
    RoutineFamily family = RoutineFamily::graphics;
    std::size_t source_bytes = 0;
};

const std::vector<RemainingRoutine>& remaining_routine_catalog();
const RemainingRoutine* find_remaining_routine(std::uint32_t rom_entry);

struct RemainingRoutineCounters {
    std::size_t calls = 0;
    std::uint32_t last_entry = 0;
    std::size_t interrupt_calls = 0;
    std::size_t field_calls = 0;
    std::size_t menu_calls = 0;
    std::size_t entity_calls = 0;
    std::size_t world_calls = 0;
    std::size_t audio_calls = 0;
    std::size_t save_calls = 0;
    std::size_t intro_calls = 0;
    std::size_t graphics_calls = 0;
};

struct RemainingRoutineCallbacks {
    std::function<void(const RemainingRoutine&)> on_interrupt;
    std::function<void(const RemainingRoutine&)> on_field;
    std::function<void(const RemainingRoutine&)> on_menu;
    std::function<void(const RemainingRoutine&)> on_entity;
    std::function<void(const RemainingRoutine&)> on_world;
    std::function<void(const RemainingRoutine&)> on_audio;
    std::function<void(const RemainingRoutine&)> on_save;
    std::function<void(const RemainingRoutine&)> on_intro;
    std::function<void(const RemainingRoutine&)> on_graphics;
};

bool record_remaining_routine(
    RemainingRoutineCounters& counters,
    std::uint32_t rom_entry,
    const RemainingRoutineCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
