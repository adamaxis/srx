#include "field_command_overview_bridge.hpp"

#include <cstdint>
#include <string_view>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    using namespace shadowrun::ghidra_conversion::qt_runtime;

    FieldCommandOverviewBridge bridge;
    int boundaries = 0;
    bool prepared = false;
    bool panel_prepared = false;
    bool details_refreshed = false;
    const auto asset_available = [](const std::string_view alias) {
        return alias == "menu.field.command_overview.background" ||
               alias == "menu.field.cursor";
    };
    bridge.begin(
        CommandOverviewInput{0U, 0}, CommandOverviewPreparationInput{},
        CommandOverviewHudPanelInput{2U, 0x1234U},
        {[&](const std::string_view) { ++boundaries; }, asset_available,
         [] { return static_cast<std::uint8_t>(0U); },
         [&](const CommandOverviewPreparationResult& result) {
             prepared = result.primary_page == 6U && result.secondary_page == 0U;
         },
         [&](const CommandOverviewHudPanelPlan& plan) {
             panel_prepared = plan.destination_tile_base == 0xE104U &&
                              plan.field_parameter == 0x1234U &&
                              plan.rows == 10U && plan.words_per_row == 8U;
         },
         [&](const CommandOverviewActionRefreshResult& result) {
             details_refreshed = result.action_code == 0U &&
                                 result.sprite_plan.source_alias ==
                                     "menu.field.cursor";
         }});

    if (!expect(bridge.open()) || !expect(prepared) ||
        !expect(panel_prepared) || !expect(details_refreshed) ||
        !expect(bridge.view().source_tile_data_pending) ||
        !expect(bridge.view().source_state.primary_page == 6U) ||
        !expect(bridge.view().selected_title_text.has_value()) ||
        !expect(!bridge.view().selected_title_text_available) ||
        !expect(boundaries > 0)) {
        return 1;
    }

    // ROM 0x10DD8 gives Left priority and wraps the source selector through
    // the converted primary-page helper.
    bridge.advance(0x0400U);
    if (!expect(bridge.view().source_state.primary_page == 5U)) {
        return 2;
    }
    bridge.advance(0x0800U);
    if (!expect(bridge.view().source_state.primary_page == 6U)) {
        return 3;
    }

    // Start is the field command loop's exit edge, not the startup menu's A
    // action. It must close the bridge without running a field frame.
    bridge.advance(0x8000U);
    if (!expect(!bridge.open()) ||
        !expect(bridge.view().source_state.action_marker == 0) ||
        !expect(bridge.view().source_state.exit_marker == 0)) {
        return 4;
    }

    return 0;
}
