#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class ModeEntryResourceSelection {
    profile_zero,
    profile_one,
    profile_two,
    profile_three,
    profile_four,
};

struct ModeEntryResourceState {
    std::uint8_t scene_id = 0;
    std::uint8_t submode = 0;
    ModeEntryResourceSelection selected_profile =
        ModeEntryResourceSelection::profile_zero; // FFF0B4
};

enum class ModeEntryResourceAction {
    unchanged,
    selected,
};

// ROM 0x20634: select the mode-resource profile for the current scene and
// submode. The later resource loader consumes this typed profile instead of
// reading the source's indexed pointer table at runtime.
ModeEntryResourceAction select_mode_entry_resource(
    ModeEntryResourceState& state);

} // namespace shadowrun::ghidra_conversion
