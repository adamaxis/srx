#!/bin/sh
# Pure-Lua suites, no Qt, no display. Run from the repository root.
# Exits nonzero when any suite exits nonzero (each suite's own exit status, not the pipe's).
set -u
export LUA_PATH="decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;decompilation/gameplay/scripts/tools/?.lua;decompilation/gameplay/tests_lua/?.lua;;"
rc=0
out=$(mktemp) || exit 1
trap 'rm -f "$out"' EXIT
lua_bin=${LUA_BIN:-}
if [ -z "$lua_bin" ]; then
  for candidate in lua5.4 lua54 lua; do
    if command -v "$candidate" >/dev/null 2>&1; then
      lua_bin=$candidate
      break
    fi
  done
fi
if [ -z "$lua_bin" ]; then
  echo "Lua 5.4 interpreter not found (set LUA_BIN to its executable)" >&2
  exit 127
fi
lua_version=$("$lua_bin" -v 2>&1) || exit 127
case "$lua_version" in
  *"Lua 5.4"*) ;;
  *) echo "Lua 5.4 is required; $lua_bin reports: $lua_version" >&2; exit 127 ;;
esac
run() {
  name=$1; shift
  printf '%s: ' "$name"
  if timeout 600 "$lua_bin" "$@" </dev/null >"$out" 2>&1; then tail -1 "$out"; else st=$?; tail -1 "$out"; echo "  -> exit $st"; rc=1; fi
}
for t in menu_tests submenu_tests matrix_actions_tests story_flags_tests equiv_units run_target_event_tests run_offer_payment_profile_tests run_current_run_view_tests run_registry_save_validation_tests condition_evaluator_tests dialogue_graph_tests building_route_tests capt_quinton_graph_tests alesandro_graph_tests sharkey_graph_tests kipp_graph_tests wilma_graph_tests macklemann_graph_tests gregory_graph_tests henry_graph_tests chillicut_graph_tests max_graph_tests professor_jefferson_graph_tests vid_phone_affiliation_graph_tests michael_package_tests tarislar_city_inn_tests wylie_gala_inn_tests tarislar_apartments_tests salish_lodge_tests salish_spirit_eyes_tests renraku_offices_tests fuchi_offices_tests gates_undersound_tests boris_greenhouse_tests council_medicine_lodge_tests world_content_tests spell_death_tests animator_tests; do
  run "$t" "decompilation/gameplay/tests_lua/$t.lua"
done
# Informational only: the pre-split reference predates the ROM combat fixes (17130f9c onward),
# so whole-heartbeat mismatches are expected; see reference/INTENDED_DIVERGENCES.md.
printf 'equiv_heartbeat (info): '
timeout 600 "$lua_bin" decompilation/gameplay/tests_lua/equiv_heartbeat.lua 200 1 30 </dev/null 2>/dev/null | tail -1
exit $rc
