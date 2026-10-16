#!/usr/bin/env python3
"""Mechanical first cut of the actor_ai.lua split (refactor draft). Reads the read-only srx clone, writes refactor/actor/*.lua.
Behaviour is unchanged: each module imports its cross-module names through actor/registry.lua."""
import re, subprocess, os
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / 'gameplay/scripts/actor_ai.lua'
OUT = ROOT / 'grok_bot/refactor/actor'
L = SRC.read_text().split('\n')
MODULES = [  # (module, role, [(first,last) 1-based inclusive])
 ('tables',    'data: ROM-derived constants, flag tables, actor profiles (no functions)', [(9, 180)]),
 ('targeting', 'logic: profile lookup, target eligibility and selection, motion bounds, initial state', [(181, 586)]),
 ('anim',      'presentation: snapshots, pose selectors, entry/hit/walk/action animations', [(587, 659), (901, 948), (1226, 1488), (1953, 1990)]),
 ('actions',   'logic: action delays, target markers, world target selection, spell/item dispatch', [(660, 900), (1489, 1952)]),
 ('movement',  'ENGINE (move to C++): projectile and entity movement integration and probes', [(949, 1225), (1991, 2427)]),
 ('heartbeat', 'logic: ActorAI.heartbeat facade (public entry point, unchanged signature)', [(2428, L.index('return ActorAI'))]),
]
BUILTIN = set('assert error ipairs math next pairs print rawequal rawget rawset require select setmetatable getmetatable string table tonumber tostring type utf8 pcall xpcall os game _G'.split())
def defs(text):
    fn = re.findall(r'^local function ([A-Za-z_]\w*)', text, re.M)
    val = [n for n in re.findall(r'^local ([A-Za-z_]\w*)\s*=', text, re.M)]
    return fn, val
seg = {}
owner, kind = {}, {}
for name, role, ranges in MODULES:
    body = '\n'.join('\n'.join(L[a-1:b]) for a, b in ranges)
    seg[name] = body
    fn, val = defs(body)
    for n in fn: owner[n] = name; kind[n] = 'fn'
    for n in val: owner[n] = name; kind[n] = 'val'
# names that lived at file top (lines 1-8) belong to the heartbeat module state
top = '\n'.join(L[:8])
def free_names(body):
    tmp = '/tmp/_seg.lua'
    open(tmp, 'w').write('local ActorAI = {}\n' + body)
    out = subprocess.run(['luac5.4', '-l', '-l', '-p', tmp], capture_output=True, text=True)
    if out.returncode: raise SystemExit(out.stderr)
    return sorted(set(re.findall(r'_ENV "([A-Za-z_]\w*)"', out.stdout)) - BUILTIN)
os.makedirs(OUT, exist_ok=True)
open(f'{OUT}/registry.lua', 'w').write('-- actor/registry.lua: shared name table for the split actor_ai modules (draft).\nreturn {}\n')
report = []
for name, role, ranges in MODULES:
    body = seg[name]
    free = free_names(body)
    hdr = [f'-- actor/{name}.lua: {role}.',
           f'-- Mechanically split from gameplay/scripts/actor_ai.lua lines ' + ', '.join(f'{a}-{b}' for a, b in ranges) + ' (see refactor/MIGRATION.md).',
           'local R = require("actor.registry")']
    if name == 'heartbeat':
        hdr += ['local ActorAI = {}', 'local states = game.actors',
                'local pending_spell_effects_by_source_slot = R.pending_spell_effects_by_source_slot',
                'local spell_effect_scans_by_source_slot = R.spell_effect_scans_by_source_slot']
    if name == 'actions':
        hdr += ['R.pending_spell_effects_by_source_slot = R.pending_spell_effects_by_source_slot or {}',
                'R.spell_effect_scans_by_source_slot = R.spell_effect_scans_by_source_slot or {}',
                'local pending_spell_effects_by_source_slot = R.pending_spell_effects_by_source_slot',
                'local spell_effect_scans_by_source_slot = R.spell_effect_scans_by_source_slot']
    unresolved = []
    for n in free:
        if n in ('pending_spell_effects_by_source_slot', 'spell_effect_scans_by_source_slot', 'ActorAI', 'states'): continue
        if n not in owner: unresolved.append(n); continue
        if kind[n] == 'fn': hdr.append(f'local function {n}(...) return R.{n}(...) end   -- from actor/{owner[n]}.lua')
        else: hdr.append(f'local {n} = R.{n}   -- from actor/{owner[n]}.lua')
    fn, val = defs(body)
    tail = ['', '-- register this module\'s definitions'] + [f'R.{n} = {n}' for n in fn + val]
    if name == 'heartbeat': tail.append('return ActorAI')
    else: tail.append('return R')
    open(f'{OUT}/{name}.lua', 'w').write('\n'.join(hdr) + '\n\n' + body + '\n' + '\n'.join(tail) + '\n')
    report.append((name, len(body.split('\n')), len(free), unresolved))
loader = ['-- actor/init.lua: loads the split modules in dependency order and returns the ActorAI table (same API as before).']
for name, _, _ in MODULES[:-1]: loader.append(f'require("actor.{name}")')
loader.append('return require("actor.heartbeat")')
open(f'{OUT}/init.lua', 'w').write('\n'.join(loader) + '\n')
for r in report: print(r)
