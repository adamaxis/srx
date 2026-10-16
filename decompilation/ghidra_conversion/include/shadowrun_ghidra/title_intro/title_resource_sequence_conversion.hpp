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

// ROM 0x1D58: keep the sprite-list construction as an explicit presentation
// boundary instead of baking its source renderer into the host runtime.
void prepare_title_sprite_list(const std::function<void()>& prepare_sprites);

} // namespace shadowrun::ghidra_conversion
