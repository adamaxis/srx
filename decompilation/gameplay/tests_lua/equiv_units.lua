-- Function-level differential test: calls each internal actions/anim/targeting/movement function of the original
-- actor_ai.lua (found through heartbeat's upvalue graph) and of the split (actor.registry) with identical random
-- state/context/arguments, and compares returns, state, context, actor table and service log.
-- Usage: LUA_PATH=... lua5.4 equiv_units.lua [trials_per_function] [seed]
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
local TRIALS, SEED = tonumber(arg[1]) or 300, tonumber(arg[2]) or 1
local ser, deepcopy = H.ser, H.deepcopy

local logA, logB = {}, {}
local gameA, gameB = H.make_game(logA), H.make_game(logB)
_G.game = gameA; local A = dofile(H.ORIG)
_G.game = gameB; require("actor.init"); local REG = require("actor.registry")

-- collect the original's local functions by name via upvalues
local origf, seen = {}, {}
local function walk(f)
  if seen[f] then return end; seen[f] = true
  local i = 1
  while true do
    local name, v = debug.getupvalue(f, i)
    if not name then break end
    if type(v) == "function" then origf[name] = origf[name] or v; walk(v) end
    i = i + 1
  end
end
walk(A.heartbeat)

local fields = {}
-- Every `state.<field>` the original reads or writes (was a /tmp scratch file).
do
  local src, seen_field = assert(io.open(H.ORIG)):read("a"), {}
  for w in src:gmatch("%f[%w_]state%.([%a_][%w_]*)") do
    if not seen_field[w] then seen_field[w] = true; fields[#fields + 1] = w end
  end
  table.sort(fields)
end
local R = math.random
local function pick(t) return t[R(#t)] end
local ANIM = { "action", "attack", "down", "entry", "hurt", "idle", "projectile", "spell", "walk", "weapon" }
local function pending()
  return { action_code = R(0, 13), action_result = H.num(), ai_target_candidates = H.list(H.cand, 3), caster_actor_id = "a" .. R(3),
    caster_combatant = H.bool(), caster_source_flags = H.num(), countdown = R(0, 4), effect_slot = R(0, 7), initial_countdown = R(0, 4),
    owns_effect_slot = H.bool(), phase = R(0, 4), phase_count = R(1, 4), repeat_countdown = R(0, 3), scene_id = R(0, 5),
    source_record_slot = R(0, 7), target_id = "a" .. R(3), profile_id = H.pids[R(#H.pids)] }
end
local function value_for(name)
  if name:find("animation_id") then return pick(ANIM) end
  if name:find("pending") then return R(3) == 1 and pending() or nil end
  if name:find("incoming_hit") then return R(2) == 1 and { action_result = H.num(), damage = R(0, 5), source = "a1" } or nil end
  if name:find("_id$") then return R(3) == 1 and nil or ("a" .. R(3)) end
  if name:find("movement_step") or name:find("movement_state") then return R(2) == 1 and { state = H.mstate(), outcome = pick({ "moved", "blocked", "reset" }) } or nil end
  local k = R(10)
  if k <= 2 then return H.bool() elseif k == 3 then return nil elseif k == 4 then return 0 else return H.num() end
end
local function random_state(ctx)
  local st = {}
  for _, f in ipairs(fields) do st[f] = value_for(f) end
  st.profile_id = ctx.profile_id
  return st
end


-- Targeted generators: shape context/state so specific deep paths are reachable (coverage for spell/item chains,
-- dynamic target actions, received hits and projectile impacts).
local function projectile_source()
  return { role = R(5) == 1 and "secondary" or "primary", active = R(6) ~= 1, flags = pick({ 0x01, 0x09, 0x00, R(0, 0xff) }),
    record_flags = pick({ 0x01, 0x08, 0x09, 0x00, R(0, 0xff) }), world_x = R(0, 0xffffff), world_y = R(0, 0xffffff),
    tile_offset_x = R(-2, 2), tile_offset_y = R(-2, 2), retry_counter = R(0, 4), linked_action_timer = R(-2, 3),
    heading = R(0, 0xffff), saved_position_x = R(0, 0xffff), saved_position_y = R(0, 0xffff),
    animation_selector = pick({ 0x8000, 0, R(0, 0xffff) }), action_result = R(0, 255) }
end
local SHAPE = {
  begin_dynamic_target_action = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true; st.action_delay = -R(1, 10); st.target_motion_effect_active = false
    ctx.action_animation_frame_count = R(1, 8); ctx.action_animation_increment = R(1, 0x400)
    ctx.action_animation_threshold = R(1, 0x1000); ctx.action_animation_audio_event = R(0, 0x60)
  end,
  projectile_movement_snapshot = function(st, ctx) ctx.projectile_movement = projectile_source() end,
  projectile_heartbeat = function(st, ctx) ctx.projectile_movement = projectile_source(); st.attack_seed_resolved = R(2) == 1 end,
  begin_received_hit_reaction = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true
    st.incoming_hit = { action_result = R(0, 255), damage = R(0, 5), source = "a1", down = R(3) == 1 }
    ctx.hurt_animation_frame_count = R(1, 8); ctx.hurt_animation_increment = R(1, 0x400); ctx.hurt_animation_threshold = R(1, 0x1000)
  end,
  begin_dynamic_item_action = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true; ctx.action_kind = "item"; ctx.action_code = R(1, 15); st.action_delay = -R(1, 5)
    ctx.item_action_frame_count = R(1, 8); ctx.item_action_increment = R(1, 0x400); ctx.item_action_threshold = R(1, 0x1000)
    ctx.source_item_spawn_result = "spawned"; ctx.action_line_clear = R(4) ~= 1
  end,
  begin_spell_cast = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true; ctx.action_kind = "spell"; ctx.action_code = R(0, 13); st.action_delay = -R(1, 5)
  end,
  advance_spell_effect = function(st, ctx) st.pending_spell_effect = pending(); st.pending_spell_effect.countdown = R(0, 2) end,
  transfer_pending_spell_effect = function(st, ctx) st.pending_spell_effect = pending() end,
}
for _, n in ipairs({ "classify_spell_dispatch", "classify_item_dispatch" }) do SHAPE[n] = SHAPE.begin_spell_cast end
SHAPE.classify_item_dispatch = SHAPE.begin_dynamic_item_action
-- functions whose first parameter is not the state: return replacement leading args
local ARGS = {
  movement_collision_code = function(st, ctx)
    ctx.movement_transition_active = R(3) == 1; ctx.movement_collision_mode = R(0, 3); ctx.movement_scene_id = pick({ 0x14, R(0, 0x37) })
    return { -R(1, 0x40), ctx }
  end,
  probe_movement = function(st, ctx)
    ctx.movement_entity_flags = pick({ 0, 2, R(0, 255) })
    local offs = {} for i = 1, 8 do offs[i] = { R(-32, 32), R(-32, 32) } end
    return { ctx, H.mstate(), R(0, 15), offs }
  end,
}
math.randomseed(SEED)
local targets = {}
-- Functions whose behavior was deliberately changed to match the ROM after the split; the
-- pre-split reference is wrong for them by design. See reference/INTENDED_DIVERGENCES.md.
local INTENDED = {
  advance_target_marker = "ROM 0x5C7E tiers the target's meter (a5a74ea3)",
  advance_player_animation = "actor/animator.lua 0x600E wrap/0x5FDC signed word; equal on frame 0..3/acc 0..0xFFF (animator_tests)",
}
for name, f in pairs(REG) do if type(f) == "function" and origf[name] and not INTENDED[name] then targets[#targets + 1] = name end end
table.sort(targets)
local total, mism, perfn = 0, 0, {}
for _, name in ipairs(targets) do
  local fa, fb = origf[name], REG[name]
  local bad = 0
  for t = 1, TRIALS do
    local ctx = H.context(pick({ "field_player_runner", "a1", "a2" }))
    local st = random_state(ctx)
    if SHAPE[name] and t % 4 ~= 0 then SHAPE[name](st, ctx) end
    local extra = { pick({ H.cand(1), H.num(), nil, 1, 2 }), pick({ { action_result = H.num(), effect_started = H.bool(), spell_effect_slot = R(0, 7), combatant = H.bool() }, H.num(), nil, 0x1000 }), pick({ H.num(), nil, 0x1000 }) }
    local others = {}
    for i = 1, 2 do others["a" .. i] = random_state(ctx) end
    for k in pairs(gameA.actors) do gameA.actors[k] = nil; gameB.actors[k] = nil end
    for k, v in pairs(others) do gameA.actors[k] = deepcopy(v); gameB.actors[k] = deepcopy(v) end
    local sA, sB, cA, cB = deepcopy(st), deepcopy(st), deepcopy(ctx), deepcopy(ctx)
    cB.ai_random_bound = ctx.ai_random_bound; cA.ai_random_bound = ctx.ai_random_bound
    local xA, xB = deepcopy(extra), deepcopy(extra)
    for i = #logA, 1, -1 do logA[i] = nil end; for i = #logB, 1, -1 do logB[i] = nil end
    local argsA, argsB
    if ARGS[name] and t % 4 ~= 0 then
      math.randomseed(SEED * 100003 + t); local a1 = ARGS[name](sA, cA)
      math.randomseed(SEED * 100003 + t); local b1 = ARGS[name](sB, cB)
      argsA, argsB = a1, b1
    else
      argsA, argsB = { sA, cA, xA[1], xA[2], xA[3] }, { sB, cB, xB[1], xB[2], xB[3] }
    end
    _G.game = gameA; local ra = table.pack(pcall(fa, table.unpack(argsA, 1, 5)))
    _G.game = gameB; local rb = table.pack(pcall(fb, table.unpack(argsB, 1, 5)))
    local function sig(r, s, c, g, log, x)
      return table.concat({ tostring(r[1]), r[1] and ser({ table.unpack(r, 2, r.n) }) or H.norm_err(r[2]), ser(s), ser(c), ser(g.actors), ser(x), table.concat(log, ";") }, "|")
    end
    total = total + 1
    if sig(ra, sA, cA, gameA, logA, xA) ~= sig(rb, sB, cB, gameB, logB, xB) then
      bad = bad + 1; mism = mism + 1
      if bad == 1 then io.stderr:write(("MISMATCH %s\n A=%s\n B=%s\n"):format(name, sig(ra, sA, cA, gameA, logA, xA):sub(1, 300), sig(rb, sB, cB, gameB, logB, xB):sub(1, 300))) end
    end
  end
  perfn[#perfn + 1] = name .. (bad > 0 and ("!" .. bad) or "")
end
print(("functions=%d calls=%d mismatches=%d"):format(#targets, total, mism))
if mism > 0 then print(table.concat(perfn, " ")) end
if not COVERAGE then os.exit(mism == 0 and 0 or 1) end
