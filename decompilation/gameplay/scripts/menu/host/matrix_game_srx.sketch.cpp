// matrix_game_srx.sketch.cpp: SKETCH (not built). Wires MatrixGame / MenuRouteApi (menu_binding.hpp) onto the srx
// cyberspace conversions at bfa3908 (/tmp/srx_full/decompilation/ghidra_conversion). [C] = file:line verified.
//
// srx boundary (header unless noted)                                   -> our side
// cyberspace/matrix_node_action_loop_conversion.hpp:165 MatrixNodeActionLoopCallbacks
//   :177 advance_random_matrix_alert  (cpp:72-98, ROM 0x54A8E)          -> Lua node_actions.random_alert + MX.raise_alert
//   :194 calculate_matrix_node_action_gate_inputs / :197 count_..._rolls -> Lua node_actions.gate (fixes R6 max->min);
//        srx cpp:205-246 gate body is bypassed: the callback returns the Lua verdict as 1/0 "rolls"
//   :206 dispatch_matrix_node_action (unresolved in srx)                -> MenuRouteApi::node_action(cursor) -> Lua
//        handlers (0x1664D8 table: cancel_alert / crash_system / turn_off_node / transfer / erase / lockout)
//   :181 render_matrix_text                                             -> MatrixGame::print(text_addr)
//   :201 wait_for_controller_event                                      -> MatrixGame::wait_button
//   :200 redraw_matrix_node_action_grid                                 -> MatrixGame::redraw_node (0x549F6)
//   :214 enter_matrix_navigation (FFF0B1 abort)                         -> MatrixGame::logout result
// cyberspace/matrix_program_action_conversion.hpp:34-36 dispatch_program_action (0xE2E0A unresolved)
//                                                                       -> MenuRouteApi::run_program(p) -> Lua matrix_programs
// cyberspace/matrix_interaction_conversion.cpp:23 run_matrix_interaction_loop (0x1822C per frame)
//                                                                       -> MenuRouteApi::matrix_frame each VBlank
//        (trace_frame, rebound/shroud ticks, dump_shake 60 frames, then MatrixGame::logout)
// cyberspace/matrix_entry_conversion.cpp:35 enter_matrix_screen          -> unchanged (display setup stays C++)
// qt_runtime: no caller of run_matrix_* exists (rg over qt_runtime/src) -> the host below is new code.
#include "../menu_binding.hpp"
#include "../../debugger/srxdbg/source_rng.hpp"
#include "shadowrun_ghidra/cyberspace/matrix_node_action_loop_conversion.hpp"

namespace srx::menu {
class SrxMatrixGame final : public MatrixGame {
public:
    SrxMatrixGame(srxdbg::SourceRng& rng, /* FieldSceneRuntime state */ void* ram) : rng_(rng), ram_(ram) {}
    int rand(int n) override { return rng_.bounded(static_cast<std::uint16_t>(n), site_); }          // 0xD3A
    int roll(int d, int tn) override { return rng_.dice(static_cast<std::int16_t>(d), static_cast<std::int16_t>(tn), site_); }
    void commit() override { /* write Lua proxies back to FF1100/FFF0xx/FFE1xx/FFFBxx/FFFCxx in ram_ */ }
    void print(std::uint32_t a) override { /* srx text renderer at window 0xC416 (render_matrix_text role lookup) */ (void)a; }
    void print_story(int) override {}
    void wait_button() override {}           // route-resumable: Lua yields; srx wait_for_controller_event
    void wait_frames(int) override {}
    void stop_sfx(std::uint16_t) override {}
    void redraw_node() override {}           // srx redraw_matrix_node_action_grid_service (cpp:34)
    void fade_out() override {}
    void logout() override {}                // 0x26A00 / 0x1B1B8
    void program_anim(int, int) override {}
    void program_fail(int) override {}
    void unload_program(int) override {}     // srx matrix_menu_service_conversion.cpp:17 clear_matrix_program_slot
    void trace_bar(int, int, int) override {}
    int relocate_pos() const override { return 0; }   // FFE056
    void dump_shake(int) override { for (int i = 0; i < 0x58; ++i) rng_.raw("matrix.dump"); }  // keep stream in step
    const char* site_ = "matrix";            // set from the Lua caller per call (docs/24 §3.4)
private:
    srxdbg::SourceRng& rng_; void* ram_;
};

// Callback wiring (sketch): only the four boundaries above change; everything else stays srx.
inline void wire(shadowrun::ghidra_conversion::MatrixNodeActionLoopCallbacks& cb, MenuRouteApi& lua, bool& gate_ok) {
    cb.dispatch_matrix_node_action = [&lua](auto, std::int8_t cursor) { lua.node_action(cursor); };
    cb.count_matrix_node_action_rolls = [&gate_ok](std::uint16_t, std::uint8_t) -> std::int8_t { return gate_ok ? 1 : 0; };
}
}  // namespace srx::menu
