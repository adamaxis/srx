#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ArmorSelectionState {
    // Source +0x76 on the active runner.
    std::uint8_t equipped_armor_index = 0;
    // Source +0x6E + selected inventory slot.
    std::uint8_t selected_inventory_phase = 0;
};

struct ArmorSelectionCallbacks {
    // 0xC0DA is called twice before the two source effect services.
    std::function<void()> render_text;
    std::function<void()> refresh_first_effect;
    std::function<void()> write_grid;
    std::function<void()> refresh_second_effect;
    std::function<void()> show_no_equipped_armor;
};

enum class ArmorSelectionResult {
    refreshed_current,
    previewed_and_restored,
    rejected_item_code,
    rejected_no_equipped_armor,
};

// ROM 0x11352. The caller supplies the selected armor item code (D0) and the
// source FFF0B1 preview gate. A preview writes the selected armor index and
// phase for the duration of the source refresh sequence, then restores both
// values; the ordinary path refuses to refresh with no equipped armor.
ArmorSelectionResult apply_armor_selection(
    ArmorSelectionState& state,
    std::uint8_t selected_item_code,
    bool preview_new_selection,
    const ArmorSelectionCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
