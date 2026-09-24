#pragma once

#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleResourceLoadCallbacks {
    // The source invokes the same decoded-resource boundary three times. The
    // host callback selects the named asset for each invocation.
    std::function<void()> upload_resource_stream;
    // ROM 0x1CFC resets the decoded stream cursor between its first and
    // second resource uploads. This is a host asset-selection boundary, not
    // a runtime ROM pointer.
    std::function<void()> reset_resource_stream;
    std::function<void()> prepare_sprite_list;
};

// ROM 0x1CFC: load the three title resource streams in source order, then
// build the title sprite list.
void load_title_resources(const TitleResourceLoadCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
