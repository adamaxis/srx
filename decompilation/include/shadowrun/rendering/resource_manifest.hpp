#pragma once

#include <string>
#include <string_view>

namespace shadowrun::resources {

inline constexpr std::string_view kManifestFile = "resources.tsv";

// Stable aliases are derived once from the relative asset path.  Directory
// separators and filename separators become dots, so for example
// menu/field/action-focus-2.bmp becomes menu.field.action.focus.2.
inline std::string alias_for_path(std::string_view relative_path) {
    std::string alias;
    const auto extension = relative_path.find_last_of('.');
    const auto stem = relative_path.substr(
        0, extension == std::string_view::npos ? relative_path.size() : extension);
    alias.reserve(stem.size());
    bool previous_was_dot = false;
    for (const auto character : stem) {
        const auto is_separator = character == '/' || character == '\\' ||
                                  character == '-';
        if (is_separator) {
            if (!previous_was_dot) {
                alias.push_back('.');
            }
            previous_was_dot = true;
        } else {
            alias.push_back(character);
            previous_was_dot = false;
        }
    }
    while (!alias.empty() && alias.back() == '.') {
        alias.pop_back();
    }
    return alias;
}

} // namespace shadowrun::resources
