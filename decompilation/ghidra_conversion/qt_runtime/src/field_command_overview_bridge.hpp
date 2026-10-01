#pragma once

#include "shadowrun_ghidra/menus/menu_service_sequence_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

// The source command loop at 0x10D30 blocks until an edge arrives. A Qt
// event loop cannot call that routine directly, so this adapter retains its
// converted leaf calls and exposes one source frame at a time. It is a host
// presentation/controller seam, not another menu implementation.
struct FieldCommandOverviewCallbacks {
    std::function<void(std::string_view)> source_boundary;
    // The Qt host may map a source tile-table alias to an already extracted
    // direct image (for example the source background becomes the named
    // menu.field.background asset). Missing tables remain observable.
    std::function<bool(std::string_view)> asset_available;
    std::function<std::uint8_t()> read_active_action_code;
    std::function<void(const CommandOverviewPreparationResult&)>
        preparation_complete;
    std::function<void(const CommandOverviewHudPanelPlan&)> hud_panel_prepared;
    std::function<void(const CommandOverviewActionRefreshResult&)>
        action_refresh_complete;
};

struct FieldCommandOverviewView {
    bool open = false;
    CommandOverviewState source_state{};
    CommandOverviewPreparationResult preparation{};
    CommandOverviewHudPanelPlan hud_panel{};
    bool hud_panel_ready = false;
    std::optional<MenuTitleTextPlan> selected_title_text;
    bool selected_title_text_available = false;
    std::optional<CommandOverviewActionRefreshResult> action_refresh;
    bool source_tile_data_pending = false;
    bool action_dispatch_pending = false;
    bool action_dialog_pending = false;
};

class FieldCommandOverviewBridge final {
public:
    void begin(
        const CommandOverviewInput& input,
        const CommandOverviewPreparationInput& preparation_input,
        const CommandOverviewHudPanelInput& hud_panel_input,
        FieldCommandOverviewCallbacks callbacks);

    // The argument is the same expanded controller word consumed by
    // run_command_overview(): d-pad edges occupy 0x0100..0x0800, A/B occupy
    // 0x1000/0x4000, mode action is 0x2000, and Genesis Start is 0x8000.
    void advance(std::uint16_t controller_events);
    void close() noexcept;

    bool open() const noexcept { return view_.open; }
    const FieldCommandOverviewView& view() const noexcept { return view_; }

    // The converted overview loop reports the A/B action edge through a
    // typed view flag.  Detail refreshes are presentation work and do not set
    // this flag.  The host consumes the edge once before entering the
    // corresponding converted sub-loop.
    bool take_action_dispatch_pending() noexcept {
        const bool pending = view_.action_dispatch_pending;
        view_.action_dispatch_pending = false;
        return pending;
    }

private:
    bool asset_available(std::string_view alias) const;
    void source_boundary(std::string_view boundary) const;
    void mark_missing_source_asset(std::string_view alias);
    void refresh_view();
    void advance_primary(std::int8_t delta);
    void advance_secondary(std::int8_t delta,
                           std::uint16_t controller_events);

    CommandOverviewInput input_{};
    CommandOverviewState state_{};
    FieldCommandOverviewCallbacks callbacks_{};
    FieldCommandOverviewView view_{};
    bool missing_asset_reported_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
