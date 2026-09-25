#include "shadowrun_ghidra/mode_entry_resource_conversion.hpp"

namespace shadowrun::ghidra_conversion {

ModeEntryResourceAction select_mode_entry_resource(
    ModeEntryResourceState& state) {
    if (state.scene_id == 0U && state.submode == 4U) {
        state.selected_profile = ModeEntryResourceSelection::profile_two;
        return ModeEntryResourceAction::selected;
    }
    if (state.scene_id == 0U && state.submode == 5U) {
        state.selected_profile = ModeEntryResourceSelection::profile_one;
        return ModeEntryResourceAction::selected;
    }
    if (state.scene_id == 3U && state.submode == 2U) {
        state.selected_profile = ModeEntryResourceSelection::profile_one;
        return ModeEntryResourceAction::selected;
    }
    if (state.scene_id == 3U && state.submode == 3U) {
        state.selected_profile = ModeEntryResourceSelection::profile_two;
        return ModeEntryResourceAction::selected;
    }
    if (state.scene_id == 3U && state.submode == 6U) {
        state.selected_profile = ModeEntryResourceSelection::profile_zero;
        return ModeEntryResourceAction::selected;
    }
    if (state.scene_id == 1U && state.submode == 2U) {
        state.selected_profile = ModeEntryResourceSelection::profile_four;
        return ModeEntryResourceAction::selected;
    }
    if (state.scene_id == 2U && state.submode == 0U) {
        state.selected_profile = ModeEntryResourceSelection::profile_zero;
        return ModeEntryResourceAction::selected;
    }
    if (state.scene_id == 5U) {
        state.selected_profile = ModeEntryResourceSelection::profile_three;
        return ModeEntryResourceAction::selected;
    }
    return ModeEntryResourceAction::unchanged;
}

} // namespace shadowrun::ghidra_conversion
