#pragma once

#include "shadowrun_ghidra/field/mode_text_prompt_conversion.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion {

enum class ModeTextPromptTextKey : std::uint32_t {
    ares_store_intro = 1U,
    ares_package_handoff = 2U,
    ares_package_payment = 3U,
};

struct FieldModeTextPromptContext {
    std::uint8_t field_mode = 0; // FFFBF8
    std::uint8_t event_code = 0; // FFFBF9
    std::int8_t alternate_result = 0; // FFF0D5
};

// Resolve the source-selected text record before the generic mode-prompt
// controller or presenter sees it. The key is a native catalog ID; ROM text
// pointers are provenance only.
std::optional<ModeTextPromptDescriptor> select_field_mode_text_prompt(
    const FieldModeTextPromptContext& context) noexcept;

std::optional<std::string_view> field_mode_text_prompt_key(
    std::uint32_t text_key) noexcept;

} // namespace shadowrun::ghidra_conversion
