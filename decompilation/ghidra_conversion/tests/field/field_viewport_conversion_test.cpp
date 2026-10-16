#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldViewportState state{};
    state.normal_render_pending = 0x1111U;
    state.alternate_render_pending = 0x2222U;
    state.render_mask = 0x3333U;
    state.plane_a_offset = 0x00C0U;
    state.plane_b_offset = 0x00E0U;
    state.plane_a_destination = 0x00FFDDDEU;
    state.plane_b_destination = 0x00FFDDE6U;
    std::vector<std::uint16_t> masks;
    render_normal_field_viewport(state, [&state, &masks] {
        masks.push_back(state.render_mask);
    });
    if (!expect(state.normal_render_pending == 0U &&
                state.alternate_render_pending == 0x2222U &&
                state.render_mask == 0x3333U &&
                state.selected_plane_offset == 0x00C0U &&
                state.selected_destination == 0x00FFDDDEU &&
                state.selected_tile_table == kFieldNormalTileTable &&
                masks == std::vector<std::uint16_t>{0x3333U})) {
        return 1;
    }

    masks.clear();
    render_alternate_field_viewport(state, [&state, &masks] {
        masks.push_back(state.render_mask);
    });
    if (!expect(state.alternate_render_pending == 0U &&
                state.render_mask == 0U &&
                state.selected_plane_offset == 0x00E0U &&
                state.selected_destination == 0x00FFDDE6U &&
                state.selected_tile_table == kFieldAlternateTileTable &&
                masks == std::vector<std::uint16_t>{0x8000U})) {
        return 2;
    }

    render_alternate_field_viewport(state, {});
    if (!expect(state.render_mask == 0U &&
                state.selected_tile_table == kFieldAlternateTileTable)) {
        return 3;
    }
    return 0;
}
