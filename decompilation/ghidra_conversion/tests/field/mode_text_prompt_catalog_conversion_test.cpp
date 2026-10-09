#include "shadowrun_ghidra/field/mode_text_prompt_catalog_conversion.hpp"

#include <cstdint>
#include <string_view>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto intro = select_field_mode_text_prompt(
        FieldModeTextPromptContext{2U, 11U, 0});
    const auto handoff = select_field_mode_text_prompt(
        FieldModeTextPromptContext{2U, 11U, -1});
    const auto payment = select_field_mode_text_prompt(
        FieldModeTextPromptContext{2U, 11U, 1});
    if (!intro.has_value() || !handoff.has_value() || !payment.has_value() ||
        intro->title_text != static_cast<std::uint32_t>(
                                 ModeTextPromptTextKey::ares_store_intro) ||
        handoff->title_text != static_cast<std::uint32_t>(
                                   ModeTextPromptTextKey::ares_package_handoff) ||
        payment->title_text != static_cast<std::uint32_t>(
                                   ModeTextPromptTextKey::ares_package_payment)) {
        return 1;
    }

    for (const auto* prompt : {&*intro, &*handoff, &*payment}) {
        if (!expect(prompt->first_option_is_exit_sentinel) ||
            !expect(!prompt->first_option.present) ||
            !expect(!prompt->second_option.present) ||
            !expect(!prompt->third_option.present)) {
            return 2;
        }
    }

    const auto intro_key = field_mode_text_prompt_key(intro->title_text);
    const auto handoff_key = field_mode_text_prompt_key(handoff->title_text);
    const auto payment_key = field_mode_text_prompt_key(payment->title_text);
    if (!intro_key.has_value() || !handoff_key.has_value() ||
        !payment_key.has_value() ||
        *intro_key != "field.mode.ares.store_intro" ||
        *handoff_key != "field.mode.ares.package_handoff" ||
        *payment_key != "field.mode.ares.package_payment" ||
        field_mode_text_prompt_key(0xFFFFFFFFU).has_value()) {
        return 3;
    }

    // The event key distinguishes the Ares mode-text record from Stoker's
    // separate mode-2/event-2 NPC panel route.
    if (select_field_mode_text_prompt(FieldModeTextPromptContext{2U, 2U, 0}) ||
        select_field_mode_text_prompt(FieldModeTextPromptContext{1U, 11U, 0}) ||
        select_field_mode_text_prompt(FieldModeTextPromptContext{2U, 10U, 0})) {
        return 4;
    }

    return 0;
}
