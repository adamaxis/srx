#include "shadowrun_ghidra/field/field_action_quantity_offer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldActionQuantityOfferResult render_field_action_quantity_offer(
    FieldActionQuantityOfferState& state,
    const FieldActionQuantityOfferCallbacks& callbacks) {
    FieldActionQuantityOfferResult result{};
    result.category = state.category;

    // The parent dispatcher returns before 0x58394 for categories five and
    // above. Keep this leaf bounded rather than turning an invalid byte into a
    // host array access.
    if (state.category >= state.attribute_ratings.size() + 1U) {
        result.kind = FieldActionQuantityOfferKind::invalid_category;
        return result;
    }

    state.availability_marker = 0;
    bool limited = false;
    if (state.category == 0U) {
        // The source performs ADDQ.B before indexing 0x1AB190. Preserve the
        // byte wrap, then report a missing table row instead of reading an
        // unrelated host object when malformed source state exceeds the
        // extracted table.
        const auto lookup_index = static_cast<std::uint8_t>(
            state.master_persona_level + static_cast<std::uint8_t>(1U));
        if (lookup_index >= kMasterPersonaNextRatings.size()) {
            result.kind = FieldActionQuantityOfferKind::missing_source_data;
            return result;
        }
        state.quantity = kMasterPersonaNextRatings[lookup_index];
        if (state.quantity == state.matrix_capacity) {
            state.availability_marker = -1;
            limited = true;
            result.kind = FieldActionQuantityOfferKind::master_persona_limit;
            if (callbacks.render_master_persona_limit_notice) {
                callbacks.render_master_persona_limit_notice();
            }
        }
    } else {
        const auto attribute_index = static_cast<std::size_t>(state.category - 1U);
        const auto current_rating = state.attribute_ratings[attribute_index];
        // 0x583C8 clears D6 from FFFBBE and compares it to the selected
        // rating with BGT. The signed-byte comparison is retained for source
        // malformed/high-bit values.
        if (static_cast<std::int8_t>(state.matrix_capacity) <=
            static_cast<std::int8_t>(current_rating)) {
            state.availability_marker = -1;
            limited = true;
            result.kind = FieldActionQuantityOfferKind::attribute_limit;
            if (callbacks.render_attribute_limit_notice) {
                callbacks.render_attribute_limit_notice();
            }
        } else {
            state.quantity = static_cast<std::uint8_t>(current_rating + 1U);
        }
    }

    state.cost = calculate_field_action_quantity_cost(
        static_cast<std::int8_t>(state.category), state.quantity);

    if (state.category == 0U) {
        if (callbacks.render_master_persona_label) {
            callbacks.render_master_persona_label();
        }
        if (callbacks.render_master_persona_name) {
            callbacks.render_master_persona_name();
        }
    } else {
        if (callbacks.render_attribute_label) {
            callbacks.render_attribute_label();
        }
        if (callbacks.render_attribute_name) {
            callbacks.render_attribute_name(
                static_cast<std::uint8_t>(state.category - 1U));
        }
    }
    if (callbacks.render_category_heading) {
        callbacks.render_category_heading(state.category);
    }

    if (!limited) {
        result.kind = FieldActionQuantityOfferKind::offered;
        if (callbacks.render_upgrade_label) {
            callbacks.render_upgrade_label();
        }
        if (callbacks.render_quantity) {
            callbacks.render_quantity(state.quantity);
        }
        if (callbacks.render_cost) {
            callbacks.render_cost(state.cost);
        }
    }

    if (callbacks.write_quantity_grid) {
        callbacks.write_quantity_grid(state.category);
    }
    if (state.category != 0U && callbacks.render_tail_text) {
        callbacks.render_tail_text();
    }

    result.quantity = state.quantity;
    result.cost = state.cost;
    result.availability_marker = state.availability_marker;
    return result;
}

} // namespace shadowrun::ghidra_conversion
