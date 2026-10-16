#include "shadowrun_ghidra/menus/pocket_secretary_menu_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void initialize_pocket_secretary_menu(
    PocketSecretaryMenuState& state, const bool extended_menu,
    const PocketSecretaryMenuCallbacks& callbacks) {
    call_once(callbacks.transfer_menu_assets);
    call_once(callbacks.render_menu_grid);
    call_once(callbacks.render_menu_text);
    call_once(callbacks.render_menu_numbers);
    state.selection = 0U;
    state.maximum_selection = extended_menu ? 5U : 4U;
    state.open = true;
    state.cyberdeck_unavailable_wait = false;
    state.group_items_wait = false;
    state.group_item_flags = 0U;
}

void begin_pocket_secretary_cyberdeck_unavailable(
    PocketSecretaryMenuState& state) {
    if (state.open) {
        state.cyberdeck_unavailable_wait = true;
        state.group_items_wait = false;
    }
}

void begin_pocket_secretary_group_items(
    PocketSecretaryMenuState& state, const std::uint32_t group_item_flags) {
    if (state.open) {
        state.cyberdeck_unavailable_wait = false;
        state.group_items_wait = true;
        state.group_item_flags = group_item_flags & 0x7FFFU;
        state.group_item_display_order.fill(0U);
        const auto order = pocket_secretary_group_item_display_order(
            state.group_item_flags);
        state.group_item_display_count = static_cast<std::uint8_t>(
            std::min(order.size(), state.group_item_display_order.size()));
        for (std::size_t index = 0;
             index < state.group_item_display_count; ++index) {
            state.group_item_display_order[index] =
                static_cast<std::uint8_t>(order[index]);
        }
    }
}

void begin_pocket_secretary_notebook(
    PocketSecretaryMenuState& state, const bool current_run_available,
    const std::array<bool, 3>& event_gates) {
    if (!state.open) {
        return;
    }
    auto& notebook = state.notebook;
    notebook = NotebookMenuState{};
    notebook.visible_options[notebook.option_count++] =
        NotebookMenuOption::current_run;
    notebook.visible_options[notebook.option_count++] =
        NotebookMenuOption::tips_and_clues;
    if (event_gates[0]) {
        notebook.visible_options[notebook.option_count++] =
            NotebookMenuOption::contacts;
    }
    if (event_gates[1]) {
        notebook.visible_options[notebook.option_count++] =
            NotebookMenuOption::mr_johnsons;
    }
    if (event_gates[2]) {
        notebook.visible_options[notebook.option_count++] =
            NotebookMenuOption::shadowrunners;
    }
    notebook.current_run_available = current_run_available;
    notebook.open = true;
}

void begin_pocket_secretary_notebook_tips(
    PocketSecretaryMenuState& state,
    const std::array<std::uint32_t, 3>& tip_flags) {
    if (!state.open || !state.notebook.open) {
        return;
    }
    auto& tips = state.notebook.tips;
    tips = NotebookMenuState::TipsState{};
    for (std::size_t group = 0; group < tip_flags.size(); ++group) {
        for (std::size_t bit = 0; bit < 32U; ++bit) {
            if ((tip_flags[group] & (std::uint32_t{1} << bit)) == 0U) {
                continue;
            }
            const auto source_index = group * 32U + bit;
            tips.visible_source_indices[tips.visible_count++] =
                static_cast<std::uint8_t>(source_index);
        }
    }
    tips.open = true;
}

void begin_pocket_secretary_notebook_mr_johnsons(
    PocketSecretaryMenuState& state, const std::uint8_t referral_flags) {
    if (!state.open || !state.notebook.open) {
        return;
    }
    auto& page = state.notebook.mr_johnsons;
    page = NotebookMenuState::MrJohnsonsState{};
    for (std::size_t bit = 0; bit < kNotebookMrJohnsonCount; ++bit) {
        if ((referral_flags & (std::uint8_t{1} << bit)) == 0U) {
            continue;
        }
        page.visible_source_indices[page.visible_count++] =
            static_cast<std::uint8_t>(bit);
    }
    page.open = page.visible_count != 0U;
}

void begin_dismiss_runner_menu(
    DismissRunnerMenuState& state,
    const std::array<std::uint8_t, 2>& candidate_slots,
    const std::size_t candidate_count) {
    state = DismissRunnerMenuState{};
    state.candidate_slots = candidate_slots;
    state.candidate_count = static_cast<std::uint8_t>(
        std::min(candidate_count, state.candidate_slots.size()));
    state.open = true;
}

DismissRunnerMenuEvent apply_dismiss_runner_input(
    DismissRunnerMenuState& state, const std::uint16_t input) {
    if (!state.open) {
        return DismissRunnerMenuEvent::returned;
    }

    constexpr std::uint16_t kMovePrevious = 0x0100U;
    constexpr std::uint16_t kMoveNext = 0x0200U;
    constexpr std::uint16_t kButtonB = 0x1000U;
    constexpr std::uint16_t kButtonC = 0x2000U;
    constexpr std::uint16_t kButtonA = 0x4000U;
    constexpr std::uint16_t kStart = 0x8000U;

    if ((input & kMovePrevious) != 0U || (input & kMoveNext) != 0U) {
        const auto option_count =
            static_cast<std::uint8_t>(state.candidate_count + 1U);
        if ((input & kMovePrevious) != 0U) {
            state.selection = state.selection == 0U
                ? static_cast<std::uint8_t>(option_count - 1U)
                : static_cast<std::uint8_t>(state.selection - 1U);
        } else {
            state.selection = static_cast<std::uint8_t>(
                (state.selection + 1U) % option_count);
        }
        return DismissRunnerMenuEvent::selection_moved;
    }

    if ((input & (kButtonA | kButtonB | kButtonC)) != 0U) {
        if (state.selection == 0U) {
            state.open = false;
            return DismissRunnerMenuEvent::abort_selected;
        }
        if (state.selection <= state.candidate_count) {
            state.open = false;
            return DismissRunnerMenuEvent::runner_selected;
        }
    }

    if ((input & kStart) != 0U) {
        state.open = false;
        return DismissRunnerMenuEvent::returned;
    }
    return DismissRunnerMenuEvent::waiting;
}

std::vector<std::size_t> pocket_secretary_group_item_display_order(
    const std::uint32_t group_item_flags) {
    std::vector<std::size_t> result;
    for (std::size_t index = 15U; index-- > 0U;) {
        if ((group_item_flags & (std::uint32_t{1} << index)) != 0U) {
            result.push_back(index);
        }
    }
    return result;
}

PocketSecretaryMenuEvent apply_pocket_secretary_input(
    PocketSecretaryMenuState& state, const std::uint16_t input,
    const PocketSecretaryMenuCallbacks& callbacks) {
    if (!state.open) {
        return PocketSecretaryMenuEvent::exited;
    }

    if (state.notebook.open) {
        auto& notebook = state.notebook;
        if (notebook.mr_johnsons.open) {
            auto& page = notebook.mr_johnsons;
            constexpr std::uint16_t kMovePrevious = 0x0100U;
            constexpr std::uint16_t kMoveNext = 0x0200U;
            constexpr std::uint16_t kButtonB = 0x1000U;
            constexpr std::uint16_t kButtonC = 0x2000U;
            constexpr std::uint16_t kButtonA = 0x4000U;
            constexpr std::uint16_t kStart = 0x8000U;
            if (page.detail_wait) {
                if ((input & 0xFF00U) == 0U) {
                    return PocketSecretaryMenuEvent::waiting;
                }
                page.detail_wait = false;
                return PocketSecretaryMenuEvent::
                    notebook_mr_johnsons_detail_dismissed;
            }
            if ((input & (kMovePrevious | kMoveNext)) != 0U) {
                if (page.visible_count == 0U) {
                    return PocketSecretaryMenuEvent::waiting;
                }
                const auto final_entry = static_cast<std::uint8_t>(
                    page.visible_count - 1U);
                if ((input & kMovePrevious) != 0U) {
                    page.selection = page.selection == 0U
                        ? final_entry
                        : static_cast<std::uint8_t>(page.selection - 1U);
                } else {
                    page.selection = static_cast<std::uint8_t>(
                        (page.selection + 1U) % page.visible_count);
                }
                return PocketSecretaryMenuEvent::
                    notebook_mr_johnsons_selection_moved;
            }
            if ((input & (kButtonA | kButtonB)) != 0U) {
                if (page.selection < page.visible_count) {
                    page.detail_wait = true;
                    return PocketSecretaryMenuEvent::
                        notebook_mr_johnsons_detail_opened;
                }
            }
            if ((input & kButtonC) != 0U) {
                call_once(callbacks.handle_mode_action_event);
                return PocketSecretaryMenuEvent::
                    notebook_mr_johnsons_mode_action;
            }
            if ((input & kStart) != 0U) {
                page.open = false;
                return PocketSecretaryMenuEvent::
                    notebook_mr_johnsons_returned;
            }
            return PocketSecretaryMenuEvent::waiting;
        }
        if (notebook.tips.open) {
            auto& tips = notebook.tips;
            constexpr std::uint16_t kMovePrevious = 0x0400U;
            constexpr std::uint16_t kMoveNext = 0x0800U;
            constexpr std::uint16_t kButtonB = 0x1000U;
            constexpr std::uint16_t kButtonC = 0x2000U;
            constexpr std::uint16_t kButtonA = 0x4000U;
            constexpr std::uint16_t kStart = 0x8000U;
            if ((input & kMovePrevious) != 0U ||
                (input & kMoveNext) != 0U) {
                if (tips.visible_count == 0U) {
                    return PocketSecretaryMenuEvent::waiting;
                }
                if ((input & kMovePrevious) != 0U) {
                    tips.selection = tips.selection == 0U
                        ? 0U
                        : static_cast<std::uint8_t>(tips.selection - 1U);
                } else {
                    const auto final_page = static_cast<std::uint8_t>(
                        tips.visible_count - 1U);
                    tips.selection = tips.selection < final_page
                        ? static_cast<std::uint8_t>(tips.selection + 1U)
                        : final_page;
                }
                return PocketSecretaryMenuEvent::
                    notebook_tips_selection_moved;
            }
            if ((input & (kButtonA | kButtonB | kStart)) != 0U) {
                tips.open = false;
                return PocketSecretaryMenuEvent::notebook_tips_returned;
            }
            if ((input & kButtonC) != 0U) {
                call_once(callbacks.handle_mode_action_event);
                return PocketSecretaryMenuEvent::notebook_tips_mode_action;
            }
            return PocketSecretaryMenuEvent::waiting;
        }
        if (notebook.no_current_run_wait) {
            if ((input & 0xFF00U) == 0U) {
                return PocketSecretaryMenuEvent::waiting;
            }
            notebook.no_current_run_wait = false;
            return PocketSecretaryMenuEvent::notebook_response_dismissed;
        }

        constexpr std::uint16_t kMovePrevious = 0x0100U;
        constexpr std::uint16_t kMoveNext = 0x0200U;
        constexpr std::uint16_t kButtonB = 0x1000U;
        constexpr std::uint16_t kDispatchAction = 0x4000U;
        constexpr std::uint16_t kExit = 0x8000U;
        if ((input & kButtonB) != 0U || (input & kExit) != 0U) {
            notebook.open = false;
            return PocketSecretaryMenuEvent::notebook_returned;
        }
        if (notebook.option_count == 0U) {
            return PocketSecretaryMenuEvent::waiting;
        }
        if ((input & kMovePrevious) != 0U || (input & kMoveNext) != 0U) {
            if ((input & kMovePrevious) != 0U) {
                notebook.selection = notebook.selection == 0U
                    ? static_cast<std::uint8_t>(notebook.option_count - 1U)
                    : static_cast<std::uint8_t>(notebook.selection - 1U);
            } else {
                notebook.selection = static_cast<std::uint8_t>(
                    (notebook.selection + 1U) % notebook.option_count);
            }
            return PocketSecretaryMenuEvent::notebook_selection_moved;
        }
        if ((input & kDispatchAction) != 0U &&
            notebook.selection < notebook.option_count) {
            switch (notebook.visible_options[notebook.selection]) {
            case NotebookMenuOption::current_run:
                if (!notebook.current_run_available) {
                    notebook.no_current_run_wait = true;
                    return PocketSecretaryMenuEvent::
                        notebook_current_run_no_run;
                }
                return PocketSecretaryMenuEvent::notebook_action_boundary;
            case NotebookMenuOption::tips_and_clues:
            case NotebookMenuOption::contacts:
            case NotebookMenuOption::mr_johnsons:
            case NotebookMenuOption::shadowrunners:
                return PocketSecretaryMenuEvent::notebook_action_boundary;
            }
        }
        if ((input & 0x2000U) != 0U) {
            // ROM F236 routes C through 0x391A and resumes this same menu.
            call_once(callbacks.handle_mode_action_event);
            return PocketSecretaryMenuEvent::notebook_mode_action;
        }
        return PocketSecretaryMenuEvent::waiting;
    }

    if (state.cyberdeck_unavailable_wait || state.group_items_wait) {
        if ((input & 0xFF00U) == 0U) {
            return PocketSecretaryMenuEvent::waiting;
        }
        state.cyberdeck_unavailable_wait = false;
        state.group_items_wait = false;
        return PocketSecretaryMenuEvent::subview_dismissed;
    }

    constexpr std::uint16_t kMovePrevious = 0x0100U;
    constexpr std::uint16_t kMoveNext = 0x0200U;
    constexpr std::uint16_t kDispatchAction = 0x4000U;
    constexpr std::uint16_t kModeAction = 0x2000U;
    constexpr std::uint16_t kExit = 0x8000U;

    if ((input & kMovePrevious) != 0U || (input & kMoveNext) != 0U) {
        const auto delta = (input & kMovePrevious) != 0U ? -1 : 1;
        const auto next = static_cast<int>(state.selection) + delta;
        if (next < 0) {
            state.selection = state.maximum_selection;
        } else if (next > static_cast<int>(state.maximum_selection)) {
            state.selection = 0U;
        } else {
            state.selection = static_cast<std::uint8_t>(next);
        }
        return PocketSecretaryMenuEvent::selection_moved;
    }

    if ((input & kDispatchAction) != 0U) {
        const auto keep_open = callbacks.dispatch_menu_handler
                                   ? callbacks.dispatch_menu_handler(
                                         state.selection)
                                   : true;
        if (!keep_open) {
            state.open = false;
            return PocketSecretaryMenuEvent::exited;
        }
        call_once(callbacks.render_menu_grid);
        call_once(callbacks.render_menu_text);
        call_once(callbacks.render_menu_numbers);
        return PocketSecretaryMenuEvent::action_dispatched;
    }

    if ((input & kModeAction) != 0U) {
        call_once(callbacks.handle_mode_action_event);
        return PocketSecretaryMenuEvent::mode_action;
    }

    if ((input & kExit) != 0U) {
        state.open = false;
        return PocketSecretaryMenuEvent::exited;
    }

    return PocketSecretaryMenuEvent::waiting;
}

} // namespace shadowrun::ghidra_conversion
