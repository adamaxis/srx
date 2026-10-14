#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

inline constexpr std::size_t kNotebookTipCount = 96U;
inline constexpr std::size_t kNotebookMrJohnsonCount = 5U;
// Field-mode caller at ROM 0xC2A6 selects this five-row variant.
inline constexpr std::size_t kPocketSecretaryFieldMenuRowCount = 5U;

enum class NotebookMenuOption : std::uint8_t {
    current_run,
    tips_and_clues,
    contacts,
    mr_johnsons,
    shadowrunners,
};

struct NotebookMenuState {
    std::array<NotebookMenuOption, 5> visible_options{};
    std::uint8_t option_count = 0;
    std::uint8_t selection = 0;
    bool open = false;
    bool no_current_run_wait = false;
    bool current_run_available = false;
    struct TipsState {
        std::array<std::uint8_t, kNotebookTipCount> visible_source_indices{};
        std::uint8_t visible_count = 0U;
        std::uint8_t selection = 0U;
        bool open = false;
    } tips{};
    struct MrJohnsonsState {
        std::array<std::uint8_t, kNotebookMrJohnsonCount>
            visible_source_indices{};
        std::uint8_t visible_count = 0U;
        std::uint8_t selection = 0U;
        bool open = false;
        bool detail_wait = false;
    } mr_johnsons{};
};

struct PocketSecretaryMenuState {
    std::uint8_t selection = 0;
    std::uint8_t maximum_selection = 4;
    bool open = false;
    bool cyberdeck_unavailable_wait = false;
    bool group_items_wait = false;
    std::uint32_t group_item_flags = 0;
    NotebookMenuState notebook{};
};

// ROM 0xC448 scans the two recruited live records in source order. Selection
// zero is the fixed Abort row; nonzero selections address these compact host
// slot indices. The list never moves the fixed Abort row below the names.
struct DismissRunnerMenuState {
    std::array<std::uint8_t, 2> candidate_slots{};
    std::uint8_t candidate_count = 0;
    std::uint8_t selection = 0;
    bool open = false;
};

struct PocketSecretaryMenuCallbacks {
    // ROM 0xC2A6's initial resource/grid/text/number presentation order.
    std::function<void()> transfer_menu_assets;
    std::function<void()> render_menu_grid;
    std::function<void()> render_menu_text;
    std::function<void()> render_menu_numbers;

    // The handler table at ROM 0xC1400. Return false when the handler has
    // set the source cursor to its negative exit sentinel.
    std::function<bool(std::uint8_t)> dispatch_menu_handler;
    std::function<void()> handle_mode_action_event;
};

enum class PocketSecretaryMenuEvent {
    waiting,
    selection_moved,
    action_dispatched,
    mode_action,
    subview_dismissed,
    notebook_selection_moved,
    notebook_mode_action,
    notebook_current_run_no_run,
    notebook_response_dismissed,
    notebook_returned,
    notebook_action_boundary,
    notebook_tips_selection_moved,
    notebook_tips_returned,
    notebook_tips_mode_action,
    notebook_mr_johnsons_selection_moved,
    notebook_mr_johnsons_detail_opened,
    notebook_mr_johnsons_detail_dismissed,
    notebook_mr_johnsons_returned,
    notebook_mr_johnsons_mode_action,
    exited,
};

enum class DismissRunnerMenuEvent {
    waiting,
    selection_moved,
    abort_selected,
    runner_selected,
    returned,
};

// ROM 0xC2A6: FFF101 selects the five-entry or six-entry Pocket Secretary
// menu. The source always resets FFF0DC to the first selection on entry.
void initialize_pocket_secretary_menu(
    PocketSecretaryMenuState& state, bool extended_menu,
    const PocketSecretaryMenuCallbacks& callbacks);

// ROM 0x18150's no-deck branch displays its source feedback and waits for a
// fresh controller event before returning to the C2A6 parent loop.
void begin_pocket_secretary_cyberdeck_unavailable(
    PocketSecretaryMenuState& state);

// ROM 0xC8C8 reads FFFC22. Its empty branch writes the two C1471/C1486 lines;
// the populated branch visits bits 14 down to 0 and then waits at 0xF312.
void begin_pocket_secretary_group_items(PocketSecretaryMenuState& state,
                                        std::uint32_t group_item_flags);
// ROM 0xF33E always lists Current Run and Tips & Clues, then appends Contacts,
// Mr. Johnsons, and Shadowrunners when FFFC2C, FFFC27, and FFFC2A are nonzero.
void begin_pocket_secretary_notebook(
    PocketSecretaryMenuState& state, bool current_run_available,
    const std::array<bool, 3>& event_gates);
// ROM 0xF9A4 scans each FFFC30/34/38 longword from bit 0 through bit 31 and
// appends unlocked source indices to its compact page list.
void begin_pocket_secretary_notebook_tips(
    PocketSecretaryMenuState& state,
    const std::array<std::uint32_t, 3>& tip_flags);
// ROM 0xF416 emits Johnson rows selected by FFFC27 bits 0..4.
void begin_pocket_secretary_notebook_mr_johnsons(
    PocketSecretaryMenuState& state, std::uint8_t referral_flags);
// ROM 0xC448 always places Abort at row zero and lists active recruited
// records from FF0200 then FF0300. The original loop refreshes E660 once per
// VBlank; the caller owns that separate display-service boundary.
void begin_dismiss_runner_menu(
    DismissRunnerMenuState& state,
    const std::array<std::uint8_t, 2>& candidate_slots,
    std::size_t candidate_count);
// C448 moves the selector with Up/Down, treats A/B/C as selecting the current
// row, and returns to its parent on Start.
DismissRunnerMenuEvent apply_dismiss_runner_input(
    DismissRunnerMenuState& state, std::uint16_t input);
std::vector<std::size_t> pocket_secretary_group_item_display_order(
    std::uint32_t group_item_flags);

// ROM 0xC2D4..0xC374: apply one sampled controller event. Source bit 8 has
// priority over bit 9, then action (bit 14), mode action (bit 13), and exit
// (bit 15). Cursor movement wraps between zero and the source maximum index.
PocketSecretaryMenuEvent apply_pocket_secretary_input(
    PocketSecretaryMenuState& state, std::uint16_t input,
    const PocketSecretaryMenuCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
