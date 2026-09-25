#include "shadowrun_ghidra/field_exit_orchestrator_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    FieldExitOrchestratorState state{0U, true, 7};
    const FieldExitOrchestratorCallbacks callbacks{
        [&] { order.push_back(0); },
        [&](FieldExitOrchestratorState&) {
            order.push_back(1);
            return false;
        },
        [&](FieldExitOrchestratorState&) {
            order.push_back(2);
            return false;
        },
        [&](FieldExitOrchestratorState&) {
            order.push_back(3);
            return false;
        },
        [&](FieldExitOrchestratorState&) {
            order.push_back(4);
            return false;
        },
        [&](FieldExitOrchestratorState&) {
            order.push_back(5);
            return false;
        },
        [&](FieldExitOrchestratorState&) {
            order.push_back(6);
            return false;
        },
        [&](FieldExitOrchestratorState&) {
            order.push_back(7);
            return false;
        }};
    if (expect(run_field_exit_orchestrator(state, callbacks) ==
                   FieldExitOrchestratorAction::completed &&
               order == std::vector<int>{1, 2, 3, 4, 5, 6, 7} &&
               !state.prompt_display_prepared && state.service_marker == 0) != 0) {
        return 1;
    }

    state.controller_flags = 0x40U;
    order.clear();
    if (expect(run_field_exit_orchestrator(state, callbacks) ==
                   FieldExitOrchestratorAction::graphics_service_routed &&
               order == std::vector<int>{0}) != 0) {
        return 2;
    }

    state.controller_flags = 0;
    order.clear();
    const auto abort_action = run_field_exit_orchestrator(
        state,
        { {},
          [&](FieldExitOrchestratorState&) {
              order.push_back(1);
              return false;
          },
          [&](FieldExitOrchestratorState&) {
              order.push_back(2);
              return false;
          },
          [&](FieldExitOrchestratorState&) {
              order.push_back(3);
              return true;
          },
          [&](FieldExitOrchestratorState&) {
              order.push_back(4);
              return false;
          },
          [&](FieldExitOrchestratorState&) {
              order.push_back(5);
              return false;
          },
          [&](FieldExitOrchestratorState&) {
              order.push_back(6);
              return false;
          },
          [&](FieldExitOrchestratorState&) {
              order.push_back(7);
              return false;
          } });
    if (expect(abort_action ==
                   FieldExitOrchestratorAction::aborted_after_indexed_services &&
               order == std::vector<int>{1, 2, 3} && state.service_marker == 0) != 0) {
        return 3;
    }

    order.clear();
    const auto marker_abort = run_field_exit_orchestrator(
        state,
        { {},
          {},
          [&](FieldExitOrchestratorState& callback_state) {
              order.push_back(2);
              callback_state.service_marker = -1;
              return false;
          },
          [&](FieldExitOrchestratorState&) {
              order.push_back(3);
              return false;
          },
          {},
          {},
          {},
          {} });
    if (expect(marker_abort ==
                   FieldExitOrchestratorAction::aborted_after_world_effect &&
               order == std::vector<int>{2} && state.service_marker == 0) != 0) {
        return 4;
    }

    return 0;
}
