#!/bin/sh
# Pure-Lua suites, no Qt, no display. Run from the repository root.
# Exits nonzero when any suite exits nonzero (each suite's own exit status, not the pipe's).
set -u
export LUA_PATH="decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;decompilation/gameplay/scripts/tools/?.lua;;"
rc=0
out=$(mktemp) || exit 1
trap 'rm -f "$out"' EXIT
run() {
  name=$1; shift
  printf '%s: ' "$name"
  if timeout 600 lua5.4 "$@" </dev/null >"$out" 2>&1; then tail -1 "$out"; else st=$?; tail -1 "$out"; echo "  -> exit $st"; rc=1; fi
}
for t in menu_tests submenu_tests matrix_actions_tests story_flags_tests equiv_units; do
  run "$t" "decompilation/gameplay/tests_lua/$t.lua"
done
run equiv_heartbeat decompilation/gameplay/tests_lua/equiv_heartbeat.lua 200 1 30
exit $rc
