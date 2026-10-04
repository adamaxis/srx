#include "shadowrun_ghidra/field/mode_text_prompt_catalog_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::uint8_t kAresFieldMode = 2U;
constexpr std::uint8_t kAresEventCode = 11U;

constexpr std::string_view kAresStoreIntro =
    "<A large man with the Ares logo on his huge coveralls grins at you over "
    "the counter.>\n\n"
    "You'll love this place, sir.  None'a that wimpy stuff here, no way!\n\n"
    "You just point out what you want.";

constexpr std::string_view kAresPackageHandoff =
    "<The receptionist hands a package to you.>\n\n"
    "Please sign for it and it's all yours.\n\n"
    "<You scrawl a fake name and move on.>";

constexpr std::string_view kAresPackagePayment =
    "<The receptionist takes the package and looks it over.\n\n"
    "Happy, she transfers nuyen to your credstick account and wishes you a "
    "good day.>";

ModeTextPromptDescriptor narrative_prompt(
    const ModeTextPromptTextKey text_key) noexcept {
    ModeTextPromptDescriptor descriptor{};
    descriptor.title_text = static_cast<std::uint32_t>(text_key);
    descriptor.first_option = ModeTextPromptOption{0U, false};
    descriptor.second_option = ModeTextPromptOption{0U, false};
    descriptor.third_option = ModeTextPromptOption{0U, false};
    // Source data places the negative sentinel in the first-option slot. The
    // generic prompt waits 30 frames and then returns 0xFB on any button.
    descriptor.first_option_is_exit_sentinel = true;
    return descriptor;
}

} // namespace

std::optional<ModeTextPromptDescriptor> select_field_mode_text_prompt(
    const FieldModeTextPromptContext& context) noexcept {
    if (context.field_mode != kAresFieldMode ||
        context.event_code != kAresEventCode) {
        return std::nullopt;
    }

    if (context.alternate_result > 0) {
        return narrative_prompt(ModeTextPromptTextKey::ares_package_payment);
    }
    if (context.alternate_result < 0) {
        return narrative_prompt(ModeTextPromptTextKey::ares_package_handoff);
    }
    return narrative_prompt(ModeTextPromptTextKey::ares_store_intro);
}

std::optional<std::string_view> field_mode_text_prompt_text(
    const std::uint32_t text_key) noexcept {
    switch (static_cast<ModeTextPromptTextKey>(text_key)) {
    case ModeTextPromptTextKey::ares_store_intro:
        return kAresStoreIntro;
    case ModeTextPromptTextKey::ares_package_handoff:
        return kAresPackageHandoff;
    case ModeTextPromptTextKey::ares_package_payment:
        return kAresPackagePayment;
    }
    return std::nullopt;
}

} // namespace shadowrun::ghidra_conversion
