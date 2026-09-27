#include "shadowrun_ghidra/field/field_action_detail_purchase_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldActionDetailPurchaseResult apply_field_action_detail_purchase(
    FieldActionDetailPurchaseState& state,
    const FieldActionDetailPurchaseCallbacks& callbacks) {
    if (callbacks.render_detail_grid) {
        callbacks.render_detail_grid();
    }

    if (state.category >= 6U ||
        state.runner_slot >= state.runner.action_codes.size()) {
        return FieldActionDetailPurchaseResult::invalid_selection;
    }

    auto& companion = state.runner.companion_values[state.runner_slot];
    const auto category_bit = static_cast<std::uint8_t>(1U << state.category);
    if ((companion & category_bit) != 0U) {
        if (callbacks.show_already_owned) {
            callbacks.show_already_owned();
        }
        return FieldActionDetailPurchaseResult::already_owned;
    }

    // ROM 0x570EC has an additional category-3 gate: when the companion
    // action already owns bit 0, the source shows the same blocked/owned
    // message even though category 3's own bit is clear.
    if (state.category == 3U && (companion & 0x01U) != 0U) {
        if (callbacks.show_already_owned) {
            callbacks.show_already_owned();
        }
        return FieldActionDetailPurchaseResult::already_owned;
    }

    const auto action_code = state.runner.action_codes[state.runner_slot];
    const bool action_code_is_low = action_code < 8U;
    const bool compatible = action_code_is_low
                                ? (state.category == 1U || state.category == 3U)
                                : state.category != 1U;
    if (!compatible) {
        if (callbacks.show_incompatible_action) {
            callbacks.show_incompatible_action();
        }
        return FieldActionDetailPurchaseResult::incompatible_action;
    }

    if (state.funds < state.cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        return FieldActionDetailPurchaseResult::insufficient_funds;
    }

    if (callbacks.confirm_purchase && !callbacks.confirm_purchase()) {
        return FieldActionDetailPurchaseResult::cancelled;
    }

    companion = static_cast<std::uint8_t>(companion | category_bit);
    if (state.category == 0U) {
        companion = static_cast<std::uint8_t>(companion & ~0x08U);
    }
    state.funds -= state.cost;
    return FieldActionDetailPurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion
