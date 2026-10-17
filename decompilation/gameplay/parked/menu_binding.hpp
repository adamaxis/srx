// menu_binding.hpp: the ONLY interface between the Lua menu rules (refactor/menu) and Qt/C++.
// C++ = presenter (paint regions from model), input timing (one frame() per VBlank sample), sound output,
// VDP-equivalent display blank / window plane. No menu rule (cursor, page wrap, transitions) lives in C++.
#pragma once
#include <cstdint>
#include <functional>
#include "../enums.hpp"
namespace srx::menu {

// Lua -> C++ (exposed as table `host` to Route.new)
struct MenuHost {
    virtual ~MenuHost() = default;
    virtual void play_sound(std::uint16_t id) = 0;              // -> 0x530DE equivalent, same frame
    virtual void redraw(std::uint32_t region_mask) = 0;         // MenuRegion bits; painted THIS frame [C docs/09 s1]
    virtual void blank(DisplayBlank mode) = 0;                  // OFF/ON around a full build (black frame, no fade)
    virtual void set_window(WindowPlane plane) = 0;             // SIDEBAR ($9191) / OFF ($9100)
    virtual void set_screen(MenuScreen top, int lua_state_ref) = 0; // presenter picks its painter for the top screen
    virtual void menu_closed() = 0;                             // route empty: resume field, clear display_service_word
    // transitional, for screens still in C++ (legacy.lua); returns {still_open, exit_menu}
    virtual void legacy_open(MenuScreen s, int lua_args_ref) = 0;
    virtual std::pair<bool, bool> legacy_advance(MenuScreen s, std::uint16_t events) = 0;
};

// Rule-side game services Lua may call (table `game`); thin accessors over actor records, no logic.
struct MenuGame {
    virtual ~MenuGame() = default;
    virtual bool leader_incapacitated() const = 0;              // leader +0x10 & (DEAD|DOWN)
    virtual void reselect_leader() = 0;                         // 0x391A service (sidebar frame + sfx 0x25)
    virtual int leader_posture() const = 0;                     // +0x4D
    virtual void set_leader_posture(int v) = 0;
    virtual int leader_archetype() const = 0;                  // +0x9B
    virtual bool inventory_item(int column, int row) const = 0; // 0x12148 slot occupied
    // Submenu services (all data access or RNG/host work; rules stay in Lua):
    // party() / slot_active_set(): the runner records as Lua tables (0-based slots, fields in rules/*.lua)
    virtual int active_slot() const = 0;
    virtual int magazine_capacity(int weapon_code) const = 0;   // item table lookup
    virtual int roll_successes(int dice, int target) = 0;       // RNG lives in C++
    virtual bool pocket_extra_row() const = 0;                  // FFF101 != 0
    virtual void secretary_effect_tick() = 0;                   // 0xE660 per frame
    virtual int group_item_flags() const = 0;
    virtual bool cyberdeck_available() const = 0;
    virtual void dismiss_runner(int slot) = 0;
    // Debug menu 0x599DA (docs/12): RAM writes only; the rule choices live in screens/debug_menu.lua
    virtual void clear_debug_unlock() = 0;                     // FFF101 = 0
    virtual void set_nuyen(std::uint32_t v) = 0;               // FFFBFE
    virtual void install_test_deck() = 0;                      // FF018F b0, FFFC28, FFFBBE record (0x59B7E)
    virtual void set_contacts(std::uint16_t mask) = 0;         // FFFC2C
    virtual void set_walk_fast() = 0;                          // FFE168 = FF
    virtual bool jack_in(int system) = 0;                      // FFFBBC = system; 0xC24(20) + Matrix 0x181F8; returns FFF0B1 != 0 (and clears it)
    virtual void field_resume() = 0;                           // 0x173F2
    virtual void kill_leader() = 0;                            // 0xB330
    // Save/Load 0xEFB4 (screens/save_load.lua)
    virtual int scene_mode() const = 0;                        // FFDF40 (0 = outside area)
    virtual bool in_combat() const = 0;                        // FFDF84 != 0 (also the Matrix IC-alert gate)
    virtual void save_game(int slot) = 0;                      // 0x596C4
    virtual bool load_game(int slot) = 0;                      // 0x59360; false = carry (empty); true also sets FFE108 = FFFF
    // Cyberdeck 0x18150 / deck loop 0x1822C (screens/cyberdeck.lua)
    virtual int deck_enter() = 0;                              // 9100, FFF0B7 = 0, 0xE2E3A, 0x177CC, deck +0x0C = 10; saves FFF0B7/FFDD08, clr FFF0B1
    virtual void deck_leave(int saved) = 0;                    // restore FFF0B7/FFDD08; 0xD29D6, 0x566D8/0x5670E, 0x334C, 0x39F2, 0x10A7E
    virtual void deck_tick() = 0;                              // 0x196F2
    virtual void set_deck_mode(int v) = 0;                     // FFF0B7
    // deck_record() / deck_slots(): Lua tables proxied onto FFFBBE and FFF0E8/FFF0ED/FFF0F2 (rules/deck.lua header)
    virtual void deck_commit() = 0;                            // flush the proxies + 0x17BC0/0x1792E slot redraw
    virtual void program_unloaded(int program) = 0;            // 0x1962A RAM clears for programs 3/5/6/7
    virtual void deck_paint_stats() = 0;                       // 0x18B5C (labels 0xE3902)
    virtual int run_state() const = 0;                         // FFF0E3
    virtual int run_node_kind() const = 0;                     // FFFBF2 (signed)
    virtual int run_action() const = 0;                        // FFFBF6 (signed)
    virtual void deck_paint_run(int action_text) = 0;          // 0x18C78..0x18D18, 0xE44C8[action_text]
    virtual void show_map() = 0;                               // 0x532BC
    virtual void show_retreat() = 0;                           // 0x532BC + 0x53A24(FF1100)
    virtual void deck_restore_screen() = 0;                    // 0x19138: 0xE2E3A, 0xFFBF40 <- 0xD86E0, 0x17708
    virtual int previous_node() const = 0;                     // FFF11A (signed)
    virtual void retreat() = 0;                                // swap FFF118 / FFF11A
    virtual bool matrix_online() const = 0;                    // FFF0B0
    // matrix_node(): Lua table {ic_condition +0x0F, ic_rating +0x11, ic_rating2 +0x12, ic_code +0x13, x +3, y +4} of FF1100+FFF118*0x14
    virtual int alert() const = 0;                             // FFF0A0 signed (-1 none, 0 passive, 1 active); rules/matrix.lua
    virtual bool in_alert() const = 0;                         // FFDF84 < 0
    virtual int map_seen() const = 0;                          // FFFB30[FFF102] >> 4 (0x17C3C)
    virtual void apply_physical(int delta) = 0;                // 0x17D90
    virtual void matrix_logout() = 0;                          // FFF0DC = FF, 0x18776, 0x26A00 (sfx 0x29, FFF0B1 = FF)
    // set_leader_spells(list): spell block FFF9FA + (leader +0xA0)*0x22 (0x59B34), via the Lua binding
    // notebook(), spell_table(runner): read-only tables built by the binding (magic value maths are Lua: rules/magic.lua)
};

// Matrix node actions / late programs (rules/node_actions.lua, matrix_programs.lua, datastore_special.lua).
// Table `matrix`: storage + RNG + presenter only. Every rule (tests, effects, run completion) is Lua.
// Lua tables proxied by the binding: run() {state FFF0E3, objective FFFBF2, value FFFBF6, size FFFBF7, fc28 FFFC28,
// fc1c FFFC1C}; field() {alarm FFE16F, security_off FFE170, maglocks_off FFE17F}; story() {system FFFBBC, fc38, fc34,
// fc1e, fc22, fbfa, fbfb, fc26, fc27, nuyen FFFBFE}; matrix_state() adds rebound FFF0D9, sleazed FFF0A9, sleaze FFE16B,
// trace_rewind FFE0C8, fff0ae/fff096/fff098; actors() = FF0400..FF0A00 {flags9e +0x9E}; fffb30() map nibbles.
struct MatrixGame {
    virtual ~MatrixGame() = default;
    // rand/roll (and MenuGame::roll_successes) MUST forward to the one srxdbg::SourceRng (docs/24 s3): ROM has one stream.
    virtual int rand(int n) = 0;                               // 0xD3A: 0..n-1
    virtual int roll(int dice, int target) = 0;                // 0xDBA successes (FFF0DD)
    virtual void commit() = 0;                                 // flush run/field/story/matrix_state/actors/fffb30 proxies
    virtual void print(std::uint32_t text_addr) = 0;           // 0xC0DA at window 0xC416
    virtual void print_story(int system) = 0;                  // datastore_special text branch (0x5557E..)
    virtual void wait_button() = 0;                            // 0xD94 (Lua route resumes on the next A/B/C edge)
    virtual void wait_frames(int frames) = 0;                  // presenter delay; Lua schedules countdown/dots
    virtual void stop_sfx(std::uint16_t id) = 0;               // e.g. 0x30 after alarm_off
    virtual void redraw_node() = 0;                            // 0x549F6
    virtual void fade_out() = 0;                               // 0x18776
    virtual void logout() = 0;                                 // 0x26A00 (crash system) / 0x1B1B8 (dump)
    virtual void program_anim(int program, int result) = 0;    // 0x17C84(d0 = program, d1 = result) + 0x17C52
    virtual void program_fail(int program) = 0;                // 0x19B34
    virtual void unload_program(int program) = 0;              // 0x1962A (+ deck slot clear)
    // trace bar presenter: bar 0 = FFE0B6 (type 7), 1 = FFE0B8 (type 6); pos units as in matrix_programs.trace_frame
    virtual void trace_bar(int bar, int state, int pos) = 0;
    virtual int relocate_pos() const = 0;                      // FFE056
    virtual void dump_shake(int frame) = 0;                    // 0x1B302 scroll jitter (frame 0..0x3B), 0x1B5A6 reset at end
};

// C++ -> Lua
struct MenuRouteApi {
    std::function<void(MenuScreen root)> open;                  // Start edge in field
    std::function<void(std::uint16_t ffdd22_events)> frame;     // once per VBlank, after 0x195C-equivalent sampling
    std::function<bool()> is_open;
    std::function<void(int action)> node_action;                // node-menu pick (0x54AEA..0x5561E dispatch)
    std::function<void(int program)> run_program;               // 0xE2E0A dispatch (Lua picks the rule)
    std::function<void()> matrix_frame;                         // per VBlank: trace bar, rebound/shroud timers, dump
};
}  // namespace srx::menu
