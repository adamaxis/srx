#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> calls;
    const FieldSceneRuntimePreparationCallbacks callbacks{
        [&] { calls.push_back(802); },
        [&] { calls.push_back(64); },
    };

    const auto refresh = prepare_field_scene_runtime(6U, callbacks);
    if (!expect(refresh.path ==
                    FieldSceneRuntimePreparationPath::
                        refresh_runner_state_and_assets &&
                refresh.callback_dispatched && calls == std::vector<int>{802})) {
        return 1;
    }

    calls.clear();
    const auto finalize = prepare_field_scene_runtime(0U, callbacks);
    if (!expect(finalize.path ==
                    FieldSceneRuntimePreparationPath::
                        finalize_field_mode_exit_transition &&
                finalize.callback_dispatched && calls == std::vector<int>{64})) {
        return 2;
    }

    calls.clear();
    const auto unresolved = prepare_field_scene_runtime(
        1U, FieldSceneRuntimePreparationCallbacks{});
    if (!expect(unresolved.path ==
                    FieldSceneRuntimePreparationPath::
                        refresh_runner_state_and_assets &&
                !unresolved.callback_dispatched && calls.empty())) {
        return 3;
    }

    return 0;
}
