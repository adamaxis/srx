#include "shadowrun_ghidra/field_mode_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeInitializationState state{
        FieldModeProfile::profile_b, {1U, 2U, 3U, 4U, 5U, 6U, 7U}, true};
    std::vector<int> order;
    FieldModeProfile selected = FieldModeProfile::profile_b;
    initialize_field_mode(
        state, FieldModeProfile::profile_a,
        {
            [&order, &selected](const FieldModeProfile profile) {
                order.push_back(1);
                selected = profile;
            },
            [&order] { order.push_back(2); },
            [&order] { order.push_back(3); },
        });
    if (!expect(state.profile == FieldModeProfile::profile_a &&
                selected == FieldModeProfile::profile_a &&
                state.transient_fields ==
                    std::array<std::uint8_t, 7>{0U, 0U, 0U, 0U, 0U, 0U,
                                                0U} &&
                !state.render_pending && order == std::vector<int>{1, 2, 3})) {
        return 1;
    }

    initialize_field_mode(state, FieldModeProfile::profile_b, {});
    if (!expect(state.profile == FieldModeProfile::profile_b &&
                state.transient_fields[0] == 0U && !state.render_pending)) {
        return 2;
    }

    return 0;
}
