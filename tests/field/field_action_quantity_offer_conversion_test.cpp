#include "shadowrun_ghidra/field/field_action_quantity_offer_conversion.hpp"

#include <string>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<std::string> events;
    const FieldActionQuantityOfferCallbacks callbacks{
        [&] { events.emplace_back("mpcp-limit"); },
        [&] { events.emplace_back("attribute-limit"); },
        [&] { events.emplace_back("mpcp-label"); },
        [&] { events.emplace_back("master-persona"); },
        [&] { events.emplace_back("attribute-label"); },
        [&](const std::uint8_t index) {
            events.emplace_back("attribute-name:" + std::to_string(index));
        },
        [&](const std::uint8_t category) {
            events.emplace_back("heading:" + std::to_string(category));
        },
        [&] { events.emplace_back("upgrade"); },
        [&](const std::uint8_t quantity) {
            events.emplace_back("quantity:" + std::to_string(quantity));
        },
        [&](const std::uint32_t cost) {
            events.emplace_back("cost:" + std::to_string(cost));
        },
        [&](const std::uint8_t category) {
            events.emplace_back("grid:" + std::to_string(category));
        },
        [&] { events.emplace_back("tail"); },
    };

    FieldActionQuantityOfferState mpcp{};
    mpcp.matrix_capacity = 6U;
    mpcp.master_persona_level = 0U;
    const auto mpcp_offer = render_field_action_quantity_offer(mpcp, callbacks);
    if (mpcp_offer.kind != FieldActionQuantityOfferKind::offered ||
        mpcp_offer.quantity != 4U || mpcp_offer.cost != 1600U ||
        mpcp_offer.availability_marker != 0 ||
        events != std::vector<std::string>{"mpcp-label", "master-persona",
                                           "heading:0", "upgrade", "quantity:4",
                                           "cost:1600", "grid:0"}) {
        return 1;
    }

    events.clear();
    FieldActionQuantityOfferState mpcp_limit{};
    mpcp_limit.matrix_capacity = 6U;
    mpcp_limit.master_persona_level = 1U;
    mpcp_limit.quantity = 3U;
    const auto mpcp_limit_result =
        render_field_action_quantity_offer(mpcp_limit, callbacks);
    if (mpcp_limit_result.kind !=
            FieldActionQuantityOfferKind::master_persona_limit ||
        mpcp_limit_result.quantity != 6U || mpcp_limit_result.cost != 5400U ||
        mpcp_limit_result.availability_marker != -1 ||
        events != std::vector<std::string>{"mpcp-limit", "mpcp-label",
                                           "master-persona", "heading:0",
                                           "grid:0"}) {
        return 2;
    }

    events.clear();
    FieldActionQuantityOfferState attribute{};
    attribute.category = 2U;
    attribute.matrix_capacity = 7U;
    attribute.attribute_ratings = {3U, 4U, 0U, 0U};
    const auto attribute_offer =
        render_field_action_quantity_offer(attribute, callbacks);
    if (attribute_offer.kind != FieldActionQuantityOfferKind::offered ||
        attribute_offer.quantity != 5U || attribute_offer.cost != 7500U ||
        events != std::vector<std::string>{"attribute-label", "attribute-name:1",
                                           "heading:2", "upgrade", "quantity:5",
                                           "cost:7500", "grid:2", "tail"}) {
        return 3;
    }

    events.clear();
    FieldActionQuantityOfferState attribute_limit{};
    attribute_limit.category = 4U;
    attribute_limit.matrix_capacity = 5U;
    attribute_limit.attribute_ratings = {0U, 0U, 0U, 5U};
    attribute_limit.quantity = 9U;
    const auto attribute_limit_result =
        render_field_action_quantity_offer(attribute_limit, callbacks);
    if (attribute_limit_result.kind !=
            FieldActionQuantityOfferKind::attribute_limit ||
        attribute_limit_result.quantity != 9U ||
        attribute_limit_result.cost != 16200U ||
        events != std::vector<std::string>{"attribute-limit", "attribute-label",
                                           "attribute-name:3", "heading:4",
                                           "grid:4", "tail"}) {
        return 4;
    }

    events.clear();
    FieldActionQuantityOfferState missing{};
    missing.master_persona_level = 5U;
    if (render_field_action_quantity_offer(missing, callbacks).kind !=
            FieldActionQuantityOfferKind::missing_source_data ||
        !events.empty()) {
        return 5;
    }

    FieldActionQuantityOfferState invalid{};
    invalid.category = 5U;
    if (render_field_action_quantity_offer(invalid, callbacks).kind !=
            FieldActionQuantityOfferKind::invalid_category) {
        return 6;
    }
    return 0;
}
