#include "shadowrun_ghidra/field/field_action_detail_purchase_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionDetailPurchaseState state{};
    state.runner_slot = 2U;
    state.category = 3U;
    state.runner.action_codes[2U] = 4U;
    state.runner.companion_values[2U] = 0x01U;
    state.cost = 120U;
    state.funds = 500U;
    int grids = 0;
    int owned = 0;
    int incompatible = 0;
    int poor = 0;
    int confirmations = 0;
    const FieldActionDetailPurchaseCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++owned; },
        [&] { ++incompatible; },
        [&] { ++poor; },
        [&] {
            ++confirmations;
            return true;
        },
    };

    if (!expect(apply_field_action_detail_purchase(state, callbacks) ==
                    FieldActionDetailPurchaseResult::applied &&
                grids == 1 && state.funds == 380U &&
                state.runner.companion_values[2U] == 0x09U &&
                confirmations == 1)) {
        return 1;
    }

    if (!expect(apply_field_action_detail_purchase(state, callbacks) ==
                    FieldActionDetailPurchaseResult::already_owned &&
                owned == 1 && state.funds == 380U)) {
        return 2;
    }

    state.runner.companion_values[2U] = 0U;
    state.category = 2U;
    if (!expect(apply_field_action_detail_purchase(state, callbacks) ==
                    FieldActionDetailPurchaseResult::incompatible_action &&
                incompatible == 1 && state.funds == 380U)) {
        return 3;
    }

    state.runner.action_codes[2U] = 8U;
    if (!expect(apply_field_action_detail_purchase(state, callbacks) ==
                    FieldActionDetailPurchaseResult::applied &&
                state.runner.companion_values[2U] == 0x04U &&
                state.funds == 260U)) {
        return 4;
    }

    state.category = 0U;
    state.runner.companion_values[2U] = 0x08U;
    if (!expect(apply_field_action_detail_purchase(state, callbacks) ==
                    FieldActionDetailPurchaseResult::applied &&
                state.runner.companion_values[2U] == 0x01U &&
                state.funds == 140U)) {
        return 5;
    }

    state.category = 4U;
    state.runner.companion_values[2U] = 0U;
    state.funds = 10U;
    if (!expect(apply_field_action_detail_purchase(state, callbacks) ==
                    FieldActionDetailPurchaseResult::insufficient_funds &&
                poor == 1 && state.funds == 10U)) {
        return 6;
    }

    state.funds = 500U;
    const FieldActionDetailPurchaseCallbacks cancel_callbacks{
        {}, {}, {}, {}, [] { return false; }};
    if (!expect(apply_field_action_detail_purchase(state, cancel_callbacks) ==
                    FieldActionDetailPurchaseResult::cancelled &&
                state.runner.companion_values[2U] == 0U &&
                state.funds == 500U)) {
        return 7;
    }

    state.category = 6U;
    if (!expect(apply_field_action_detail_purchase(state, callbacks) ==
                    FieldActionDetailPurchaseResult::invalid_selection)) {
        return 8;
    }

    return 0;
}
