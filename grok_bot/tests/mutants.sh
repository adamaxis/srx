#!/bin/bash
# Mutation sanity check: each mutant must be caught (mismatches > 0) by equiv_heartbeat.lua.
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
REF="$ROOT/grok_bot/refactor"
TESTS="$ROOT/grok_bot/tests"
cd "$REF/actor"
run() { (cd "$TESTS" && SHADOWRUN_ROOT="$ROOT" LUA_PATH="$REF/?.lua;$REF/tools/?.lua;;" lua5.4 equiv_heartbeat.lua 30 11 2>/dev/null | tail -1; SHADOWRUN_ROOT="$ROOT" LUA_PATH="$REF/?.lua;$REF/tools/?.lua;;" lua5.4 equiv_units.lua 80 2 2>/dev/null | head -1); }
mut() { f=$1; a=$2; b=$3; cp $f /tmp/mut.bak; python3 -c "import sys;p='$f';s=open(p).read();assert sys.argv[1] in s,sys.argv[1];open(p,'w').write(s.replace(sys.argv[1],sys.argv[2],1))" "$a" "$b" && echo "$f: $a -> $b :: $(run)"; cp /tmp/mut.bak $f; }
mut actions.lua "and SoundId.TARGET_HEADING_ALT or SoundId.TARGET_HEADING" "and SoundId.TARGET_HEADING or SoundId.TARGET_HEADING_ALT"
mut movement.lua "return Collision.BLOCKED" "return 0x7e"
mut movement.lua "collision_code = Collision.BLOCKED" "collision_code = 0x7e"
mut movement.lua "u8(movement_state.route_flags & ~BehaviorFlag.PATH)" "movement_state.route_flags"
mut targeting.lua "local signed_delta = s16(delta)" "local signed_delta = delta"
mut heartbeat.lua "EntryDelayMax.STATIC)" "14)"
mut tables.lua "impact_sound_event = E.SoundId.PROJECTILE_IMPACT," "impact_sound_event = 0x3d,"
mut targeting.lua "u8(state.source_behavior_flags & ~BehaviorFlag.IN_BAND)" "state.source_behavior_flags"
