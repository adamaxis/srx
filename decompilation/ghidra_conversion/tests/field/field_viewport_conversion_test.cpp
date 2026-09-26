#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldViewportState state{0x1111U, 0x2222U, 0x3333U};
    std::vector<std::uint16_t> masks;
    render_normal_field_viewport(state, [&state, &masks] {
        masks.push_back(state.render_mask);
    });
    if (!expect(state.normal_render_pending == 0U &&
                state.alternate_render_pending == 0x2222U &&
                state.render_mask == 0x3333U &&
                masks == std::vector<std::uint16_t>{0x3333U})) {
        return 1;
    }

    masks.clear();
    render_alternate_field_viewport(state, [&state, &masks] {
        masks.push_back(state.render_mask);
    });
    if (!expect(state.alternate_render_pending == 0U &&
                state.render_mask == 0U &&
                masks == std::vector<std::uint16_t>{0x8000U})) {
        return 2;
    }

    render_alternate_field_viewport(state, {});
    if (!expect(state.render_mask == 0U)) {
        return 3;
    }
    return 0;
}
