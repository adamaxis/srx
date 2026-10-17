#!/bin/sh
# Pure-Lua suites, no Qt, no display. Run from the repository root.
set -u
export LUA_PATH="decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;decompilation/gameplay/scripts/tools/?.lua;;"
rc=0
for t in menu_tests submenu_tests matrix_actions_tests equiv_units; do
  printf '%s: ' "$t"; timeout 600 lua5.4 decompilation/gameplay/tests_lua/$t.lua </dev/null 2>&1 | tail -1 || rc=1
done
printf 'equiv_heartbeat: '; timeout 600 lua5.4 decompilation/gameplay/tests_lua/equiv_heartbeat.lua 200 1 30 </dev/null 2>&1 | tail -1
exit $rc
