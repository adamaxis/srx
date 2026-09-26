#include "shadowrun_ghidra/field/mode_entry_resource_conversion.hpp"

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeEntryResourceState state{0U, 4U,
                                 ModeEntryResourceSelection::profile_four};
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::selected &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_two) != 0) {
        return 1;
    }

    state.scene_id = 3U;
    state.submode = 2U;
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::selected &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_one) != 0) {
        return 2;
    }
    state.submode = 3U;
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::selected &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_two) != 0) {
        return 3;
    }
    state.submode = 6U;
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::selected &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_zero) != 0) {
        return 4;
    }

    state.scene_id = 1U;
    state.submode = 2U;
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::selected &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_four) != 0) {
        return 5;
    }
    state.scene_id = 2U;
    state.submode = 0U;
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::selected &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_zero) != 0) {
        return 6;
    }
    state.scene_id = 5U;
    state.submode = 9U;
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::selected &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_three) != 0) {
        return 7;
    }

    state.scene_id = 9U;
    state.submode = 0U;
    state.selected_profile = ModeEntryResourceSelection::profile_four;
    if (expect(select_mode_entry_resource(state) ==
                   ModeEntryResourceAction::unchanged &&
               state.selected_profile ==
                   ModeEntryResourceSelection::profile_four) != 0) {
        return 8;
    }
    return 0;
}
