#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::uint16_t kWorldEffectInitializerFirstRollBound = 8U;

struct WorldEffectInitializationRecord {
    std::uint8_t handler_index = 0;
    std::uint8_t auxiliary_value = 0;
    std::uint16_t base_value = 0;
    std::uint16_t random_weight = 0;
};

struct WorldEffectInitializationTable {
    // The source table at F3EBC is eight six-byte records per context group.
    // Records remain host data; the native port does not index a ROM image.
    std::vector<WorldEffectInitializationRecord> records;
};

struct WorldEffectInitializationState {
    std::uint8_t context = 0;             // FFF0AE
    std::uint8_t setup_marker = 0;        // FFF0B6, cleared on entry
    std::uint8_t selected_context = 0;    // FFFBEC
    std::uint8_t handler_index = 0xFFU;   // FFFBED
    std::uint8_t auxiliary_value = 0;     // FFFBF3
    std::uint16_t randomized_value = 0;   // FFFBF4
    std::uint16_t base_value = 0;          // FFF0C0
    std::uint8_t random_scale = 0;        // FF018C
};

struct WorldEffectInitializationCallbacks {
    // The source's 0x0D3A bounded PRNG is kept as a typed host boundary.
    std::function<std::optional<std::uint16_t>(std::uint16_t bound)>
        next_random;
    // ROM 0x1E68C: clear the shared effect-selection/collision globals.
    std::function<void()> reset_effect_globals;
    // F3DC6[handler_index]: invoke the selected effect initializer.
    std::function<bool(std::uint8_t handler_index)> dispatch_initializer;
};

enum class WorldEffectInitializationAction {
    initialized,
    random_unresolved,
    table_unresolved,
    initializer_unresolved,
};

// ROM 0x1E69C: select one of eight context records, derive its randomized
// value, publish its handler fields, and invoke the selected initializer.
WorldEffectInitializationAction initialize_world_effect_selection(
    WorldEffectInitializationState& state,
    const WorldEffectInitializationTable& table,
    const WorldEffectInitializationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
