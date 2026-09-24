#include "shadowrun_ghidra/mode_exit_resources_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeExitResourceState ordinary{};
    ordinary.optional_resource_pending = true;
    std::vector<int> ordinary_order;
    ModeExitResourceCallbacks ordinary_callbacks{};
    ordinary_callbacks.prepare_optional_resource = [&] { ordinary_order.push_back(1); };
    ordinary_callbacks.refresh_mode_resource_metadata = [&] { ordinary_order.push_back(2); };
    ordinary_callbacks.decode_and_upload_source_stream = [&] { ordinary_order.push_back(3); };
    ordinary_callbacks.render_hud_sprite_stream = [&] { ordinary_order.push_back(4); };
    prepare_mode_exit_resources(ordinary, ordinary_callbacks);
    if (ordinary_order != std::vector<int>{1, 2, 3, 4} ||
        ordinary.optional_resource_pending || !ordinary.optional_resource_cleared ||
        !ordinary.hud_rendered) {
        return 1;
    }

    ModeExitResourceState alternate{};
    alternate.asset_mode = -1;
    alternate.resource_cursor = -1;
    std::vector<int> alternate_order;
    ModeExitResourceCallbacks alternate_callbacks{};
    alternate_callbacks.refresh_mode_resource_metadata = [&] { alternate_order.push_back(1); };
    alternate_callbacks.decode_and_upload_source_stream = [&] { alternate_order.push_back(2); };
    alternate_callbacks.prepare_mode_resource_variant_a = [&] { alternate_order.push_back(3); };
    alternate_callbacks.prepare_mode_resource_variant_b = [&] { alternate_order.push_back(4); };
    alternate_callbacks.render_hud_sprite_stream = [&] { alternate_order.push_back(5); };
    prepare_mode_exit_resources(alternate, alternate_callbacks);
    if (alternate_order != std::vector<int>{1, 2, 3, 4} ||
        alternate.resource_cursor != 0 || alternate.asset_mode != 0 ||
        alternate.hud_rendered) {
        return 2;
    }
    return 0;
}
