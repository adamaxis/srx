// menu_game_fsr.sketch.cpp: SKETCH (not built) of srx::menu::MenuGame + MenuHost over srx FieldSceneRuntime,
// so the Lua screens in refactor/menu run in-game. Line refs are srx @ bfa3908
// (decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.{hpp,cpp}, "hpp:N" / "cpp:N").
// Rule: every method is data access, RNG, or host I/O. No menu rule may live here (rules are in Lua).
// Headless: construct only after srxdbg::headless::apply() (docs/23 §9); play_sound records, never opens audio.
#include "menu/menu_binding.hpp"
#include "field/field_scene_runtime.hpp"

namespace srx::menu {

class FsrMenuGame final : public MenuGame {
public:
    explicit FsrMenuGame(FieldSceneRuntime& f) : f_(f) {}   // needs `friend class FsrMenuGame;` in FSR (private members below)

    // ---- leader / posture ---------------------------------------------------------------------------------
    bool leader_incapacitated() const override {              // flags & 0x50 (DEAD|DOWN): same test as cpp:11763/11974
        return (f_.active_runner_record().flags & 0x50U) != 0U; }          // hpp:649 active_runner_record()
    void reselect_leader() override {                           // 0x391A: cpp:5289 handle_mode_action_event (sfx 0x25 + select)
        f_.field_runtime_callbacks_.handle_mode_action_event(); }
    int leader_posture() const override { return f_.posture_dialog_value(); }               // hpp:907 (+0x4D)
    void set_leader_posture(int v) override { f_.active_runner_record().posture = std::uint8_t(v); } // hpp:620
    int leader_archetype() const override { return f_.active_runner_record().archetype_index; }      // +0x9B
    bool inventory_item(int c, int r) const override {
        return f_.active_runner_record().inventory_item_codes[r * 2 + c] != 0U; }           // slot = row*2+col (0x12148)

    // ---- party ---------------------------------------------------------------------------------------------
    int active_slot() const override { return int(f_.field_active_party_slot_for_dump()); } // hpp:646
    // party(): Lua table per slot built from f_.field_party_records_for_dump() (hpp:661) and written back after
    // each frame. Field map (srx runner_record_access_conversion.hpp:39):
    //   status=flags>>8, clips=ammo_clips, rounds=ammo_rounds, weapon_slot=equipped_inventory_slot,
    //   weapon_code=equipped_item_code, armor=equipped_armor_index, force=magic_force, posture,
    //   inventory/phase=inventory_item_codes/inventory_phase_bytes, attributes, skills,
    //   species=species_index (+0x9A), archetype=archetype_index (+0x9B), magic=attributes[aux 0x7F] [H index],
    //   karma=+0x8D aux byte [H: no named field in RunnerRecordFields; locate in attributes[] tail].
    // Write-back path: mutable records via field_party_roster_ (private) -> needs a friend or a setter.
    // slot_active_set(): f_.field_party_slot_active_for_dump(i) (hpp:643) for i = 0..2.
    int magazine_capacity(int code) const override {          // hpp:1144 / used at cpp:6051, 6228
        return f_.weapon_magazine_capacity_for_item(std::uint8_t(code)).value_or(0); }
    int roll_successes(int dice, int target) override {       // cpp:~6676 roll_service of first aid
        return count_thresholded_random_rolls(dice, std::int16_t(target),
            [this](std::uint16_t b) { return sample_source_random_bound(f_.field_random_seed_, b); }); }

    // ---- pocket secretary --------------------------------------------------------------------------------
    bool pocket_extra_row() const override { return false; }   // cpp:5577 srx hard-codes the 5-entry FFF101 form [H]
    void secretary_effect_tick() override {                    // 0xE660, cpp:5634 (called from cpp:10694-10697)
        f_.advance_pocket_secretary_dismiss_runner_effect(); }
    int group_item_flags() const override { return int(f_.group_item_flags_for_dump().value_or(0) & 0x7FFF); } // hpp:865
    bool cyberdeck_available() const override { return f_.cyberdeck_available_.value_or(false); }  // hpp:1563, cpp:10785
    void dismiss_runner(int slot) override { f_.finish_pocket_secretary_dismiss_runner(std::size_t(slot)); } // cpp:5664
    // notebook(): gates = f_.notebook_event_gates() (hpp:400); tips = notebook_tip_flags_[0..2] (hpp:392);
    //   referrals = f_.notebook_referral_flags_for_dump() (hpp:397); current_run = notebook_current_run_available_ (hpp:1569)
    // spell_table(r): find_us_magic_spell_table(r.magic_table_index) (cpp:5754) -> {entries={code,force_limit}}
    // magic values: now Lua (rules/magic.lua, D22); FSR refresh_magic_menu_success_or_defense_values (cpp:13182/13205) becomes dead
private:
    FieldSceneRuntime& f_;
};

// MenuHost over FSR presentation (all existing srx paint/boundary code):
//   play_sound(id)     -> f_.source_audio_event_callback_(id)          (cpp:463, 1453; headless: callback only records)
//   redraw(mask)       -> per-region paint requests; today one repaint of paint_command_overview (hpp:1140)
//   blank(mode)        -> source_boundary("...display_off/on") + VDP-model display bit
//   set_window(plane)  -> FSR window-plane flag ($9100 OFF for attributes)
//   legacy_open/advance(SAVE_LOAD|CYBERDECK) -> existing FSR loops: save_load at cpp:10811 (still "pending" in srx [H]),
//                         cyberdeck route at cpp:10785; return exit_menu when FFF0CD is set.
//   menu_closed()      -> command_overview_ close (inverse of open_command_overview cpp:5389)
// Call order per VBlank: after controller sample (cpp:1421-1429) -> MenuRouteApi.frame(events).
// FSR code that becomes dead once wired: command-overview child chain cpp:10498-10960, clips 6049-6150,
// item 6333-6720, magic 5745-6048, pocket secretary 10694-10840.
}  // namespace srx::menu

// ---- Save/Load + Cyberdeck services (screens/save_load.lua, cyberdeck.lua, deck_screens.lua) ----
// srx bfa3908. FSR = decompilation/ghidra_conversion/qt_runtime/src/field/; conversions = decompilation/ghidra_conversion/src/.
// FSR has no Matrix or Save/Load runtime yet: both rows end in source_boundary() ("matrix_path_pending" field_scene_runtime.cpp:10796,
// "save_load_pending" field_scene_runtime.cpp:10811). The services below hook there and call the existing conversions.
//   scene_mode()          FFDF40 -> field_movement_transition_state_.world_mode            field_scene_runtime.cpp:7362
//   in_combat()           FFDF84 -> world_frame_logic_state_.world_active                  field_scene_runtime.cpp:3924 (sync point)
//   save_game(slot)       0x596C4  -> no srx writer; format/checksum/repair are Lua (rules/save.lua, doc 07 s7); C++ = SRAM odd-byte IO only
//   load_game(slot)       0x59360  -> src/menus/startup_menu_conversion.cpp:114 (try_load_save_slot callback; unresolved in srx)
//   cyberdeck_available() FF018F b0 -> cyberdeck_available_                                 field_scene_runtime.cpp:541, :6768
//   deck_enter/deck_leave 0x18150 / 0x1822C -> src/cyberspace/matrix_entry_conversion.cpp:35 enter_matrix_screen,
//                         src/cyberspace/matrix_loop_conversion.cpp:271 (FFF0B7/FFDD08 save)
//   deck_tick()           0x196F2  -> src/cyberspace/matrix_menu_service_conversion.cpp:53 advance_matrix_program_loading
//   set_deck_mode(v)      FFF0B7   -> src/cyberspace/matrix_loop_conversion.cpp:215 apply_matrix_action_input
//   grid (Lua)            0x1822C  -> src/cyberspace/matrix_interaction_conversion.cpp:23 run_matrix_interaction_loop (reference)
//   deck_record/slots     FFFBBE.. -> src/cyberspace/matrix_storage_conversion.cpp:33/:56 (same maths as rules/deck.lua)
//   deck_commit()         0x17BC0/0x1792E -> src/cyberspace/matrix_program_timing_conversion.cpp:41 refresh_matrix_program_slot
//   program_unloaded(p)   0x1962A  -> src/cyberspace/matrix_menu_service_conversion.cpp:17 clear_matrix_program_slot (flag clears)
//   Program screen (Lua)  0x19238  -> src/cyberspace/matrix_program_slot_screen_conversion.cpp:26, matrix_program_slot_conversion.cpp:32
//   deck_paint_stats/run  0x18B5C / 0x18C28 -> src/cyberspace/matrix_cyber_info_screen_conversion.cpp:20, matrix_cyber_info_conversion.cpp:34
//   run_state/node/action FFF0E3/FFFBF2/FFFBF6 -> src/cyberspace/matrix_cyber_info_conversion.cpp:23 classify_matrix_run_availability
//   Storage list (Lua)    0x18F5E  -> src/cyberspace/matrix_storage_conversion.cpp:75 collect_matrix_storage_datafiles
//   Map/Retreat (Lua)     0x19018  -> src/cyberspace/matrix_utility_conversion.cpp:17; retreat() -> src/cyberspace/matrix_retreat_conversion.cpp:5
//   show_map/show_retreat/deck_restore_screen: VDP only; reveal + cell style are Lua (rules/matrix.lua reveal/cell_style), doc 05 s11
//   matrix_online() FFF0B0 (jacked-in, 0x17410/0x17422); matrix_node/alert/in_alert/map_seen RAM reads -> IC rules in Lua;
//   matrix_logout() 0x26A00 renderer, 10 + 54 frames (rules/matrix.lua LOGOUT_*). Still no srx conversion for these.
//   apply_physical(d)     0x17D90  -> FSR set_default_actor_physical_meter_for_dump  field_scene_runtime.hpp:313 (nearest)
//   roll_successes        0xDBA    -> sample_source_random_bound(field_random_seed_)       field_scene_runtime.cpp:474
