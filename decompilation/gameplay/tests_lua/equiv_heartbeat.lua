-- Differential test: original repo actor_ai.lua vs refactor/actor/* (split + enum renames).
-- Run from the repository root; set SHADOWRUN_ROOT if the working directory differs.
-- Usage: LUA_PATH="decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;;" lua5.4 decompilation/gameplay/tests_lua/equiv_heartbeat.lua [trials] [seed]
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
-- The original monolith is the pre-split reference (bfa3908); the live actor_ai.lua is a shim to actor.init.
local ORIG = os.getenv("SHADOWRUN_ORIG_ACTOR_AI") or (ROOT .. "/decompilation/gameplay/tests_lua/reference/actor_ai_bfa3908.lua")
local TRIALS = tonumber(arg[1]) or 2000
local SEED = tonumber(arg[2]) or 1

local function ser(v, seen, depth)
  seen = seen or {}; depth = depth or 0
  local t = type(v)
  if t == "string" then return string.format("%q", v) end
  if t ~= "table" then return t == "function" and "fn" or tostring(v) end
  if seen[v] then return "<cycle>" end
  if depth > 12 then return "<deep>" end
  seen[v] = true
  local keys = {}
  for k in pairs(v) do keys[#keys + 1] = k end
  table.sort(keys, function(a, b) return tostring(a) < tostring(b) end)
  local out = {}
  for _, k in ipairs(keys) do out[#out + 1] = tostring(k) .. "=" .. ser(v[k], seen, depth + 1) end
  seen[v] = nil
  return "{" .. table.concat(out, ",") .. "}"
end

-- deterministic hash -> value for mock returns
local function hash(s)
  local h = 2166136261
  for i = 1, #s do h = ((h ~ s:byte(i)) * 16777619) & 0xffffffff end
  return h
end


-- typed mock results (shapes the actor code expects); 1 in 7 calls still returns a random shape
local function b(h, n) return ((h >> n) & 1) == 1 end
TYPED = {
  ["movement.collision"] = function(h) return ((h >> 3) % 0xA0) - 0x20, (h >> 9) % 8 end,
  ["movement.projectile_terrain"] = function(h) return (h >> 3) % 4, b(h, 8), b(h, 9) end,
  ["movement.projectile_collision"] = function(h)
    return b(h, 2), b(h, 3), (h >> 4) % 0x10000, (h >> 12) % 0x10000, b(h, 20) and ("a" .. (h % 3)) or nil end,
  ["movement.recover_collision"] = function(h) return b(h, 2), (h >> 4) % 0x100 end,
  ["movement.route"] = function(h) return { primary_marker = (h % 3) - 1, secondary_marker = ((h >> 2) % 3) - 1,
      primary_fallback_slot = (h >> 4) % 16, secondary_fallback_slot = (h >> 8) % 16 } end,
  ["entity_motion.heading_offsets"] = function(h) return { horizontal = (h % 64) - 32, vertical = ((h >> 6) % 64) - 32 } end,
  ["combat.resolve_weapon_attack_seed"] = function(h) return (h >> 3) % 256 end,
  ["combat.resolve_reaction"] = function(h) return { damage = h % 5, stun = (h >> 3) % 3 } end,
  ["combat.resolve_weapon_hit"] = function(h) return { damage = h % 5 } end,
  ["combat.resolve_spell_action"] = function(h) return { action_result = (h >> 3) % 256,
      next_action_kind = ({ "none", "item", "spell" })[h % 3 + 1] } end,
  ["magic.actor_effect_profile"] = function(h) return { duration = (h >> 3) % 64, kind = h % 4 } end,
}
local function make_game(log)
  local count = 0
  local function svc(path)
    return setmetatable({}, { __index = function(_, name)
      local full = path .. "." .. name
      return function(...)
        count = count + 1
        local a = ser({ ... })
        log[#log + 1] = full .. a
        local h = hash(full .. a .. count)
        local T = TYPED[full]
        if T and h % 7 ~= 0 then return T(h) end
        local k = h % 5
        if k == 0 then return nil
        elseif k == 1 then return h % 3 == 0
        elseif k == 2 then return (h >> 4) % 0x10000
        else
          return { blocked = (h & 1) == 1, hit = (h & 2) == 2, position_x = (h >> 3) % 0x10000,
                   position_y = (h >> 7) % 0x10000, terrain_collision_code = (h >> 11) % 4,
                   heading = (h >> 13) % 0x10000, status = (h >> 2) % 3, seed = (h >> 5) % 256,
                   damage = (h >> 9) % 10, next_profile_id = nil, collision_code = (h >> 6) % 4 }
        end
      end
    end })
  end
  return { actors = {}, movement = svc("movement"),
           modules = { combat = svc("combat"), entity_motion = svc("entity_motion"),
                       items = svc("items"), magic = svc("magic") } }
end

-- load implementations, each with its own game
local logA, logB = {}, {}
local gameA, gameB = make_game(logA), make_game(logB)
_G.game = gameA
local A = dofile(ORIG)
if os.getenv("SRX_HEARTBEAT_RECORD") then A = require("heartbeat_recorder").wrap(A, os.getenv("SRX_HEARTBEAT_RECORD")) end
_G.game = gameB
local B = require("actor.init")

-- profile ids from the source
local src = io.open(ORIG):read("a")
local pids = {}
for id in src:gmatch('%["([%w_]+)"%] = {') do pids[#pids + 1] = id end
local kinds = {}
for id in src:gmatch('"([%w_]+_k%x%x_[a-z]+%x%x)"') do kinds[id] = true end

math.randomseed(SEED)
local R = math.random
local function pick(t) return t[R(#t)] end
local function num()
  local k = R(6)
  if k == 1 then return 0 elseif k == 2 then return R(0, 15) elseif k == 3 then return R(0, 0xff)
  elseif k == 4 then return R(0, 0xffff) elseif k == 5 then return R(1, 8) else return pick({ 0x28, 15, 0x10, 0x20, 0x80 }) end
end
local function maybe(f) if R(5) == 1 then return nil end return f() end
local function bool() return R(2) == 1 end
local function cand(i)
  return { id = "a" .. R(4), combatant = bool(), has_action = bool(), service_flags = num(), behavior_flags = num(),
           spawn_state_flags = num(), source_record_slot = R(0, 7), world_activity_actor = bool(),
           direct_route_step_cost = num(), direct_route_distance = num(), perpendicular_route_distance = num(),
           perpendicular_route_step_cost = num(), distance = num(), heading = num(), world_x = num(), world_y = num(),
           source_record_heading = num() }
end
local function list(f, n) local t = {} for i = 1, R(0, n) do t[i] = f(i) end return t end
local function mstate()
  return { animation_heading = num(), collision_context = num(), collision_extent = num(), collision_flags = num(),
           event_flags = num(), heading = num(), movement_flags = num(), movement_substate = num(),
           position_x = num(), position_y = num(), route_flags = num(), route_status = num(), terrain_collision_code = num() }
end
local function offsets() local t = {} for i = 1, 8 do t[i] = { R(-32, 32), R(-32, 32) } end return t end

local kinds_list = {} for k in pairs(kinds) do kinds_list[#kinds_list + 1] = k end table.sort(kinds_list)
local function context(actor)
  local pid = pick(pids)
  local dyn = R(4) == 1
  if dyn then pid = (#kinds_list > 0 and R(2) == 1) and pick(kinds_list) or string.format("x_k%02x_%s%02x", R(4, 14), pick({ "item", "spell", "none" }), R(0, 15)) end
  local ek, ak, ac = pid:match("_k(%x%x)_([a-z]+)(%x%x)$")
  return {
    actor_id = actor, profile_id = pid, dynamic_world_encounter_actor = dyn,
    entity_kind = ek and tonumber(ek, 16) or num(), action_kind = ak or "none", action_code = ac and tonumber(ac, 16) or 0,
    phase = pick({ "behavior", "render", "behavior" }), spawn = R(6) == 1,
    initial_heading = num(), entry_delay = R(0, 16), entry_animation_frame_count = R(1, 8),
    source_record_flags = R(4) == 1 and 0 or 1, source_record_slot = R(0, 7), combatant = bool(),
    ai_target_candidates = maybe(function() return list(cand, 4) end),
    world_target_selection_candidates = maybe(function() return list(cand, 3) end),
    projectile_combat_targets = maybe(function() return list(cand, 3) end),
    movement_state = mstate(), movement_offsets = offsets(), movement_idle_selectors = list(function() return R(0, 7) end, 4),
    controller_heading_table = (function() local t = {} for i = 1, 16 do t[i] = num() end return t end)(),
    world_effect_slot_occupancy = list(bool, 8),
    movement_scene_id = num(), movement_allowed = bool(), controller_nibble = R(0, 15),
    action_animation_threshold = num(), action_animation_increment = num(), target_selection_mode = R(0, 3),
    source_projection_active = bool(), source_behavior_flags = num(), movement_entity_flags = num(),
    ai_random_bound = function(n) return (n and n > 0) and ((#actor * 7) % n) or 0 end,
    ai_animation_frame = num(), action_animation_frame_count = R(1, 8), action_animation_audio_event = num(),
    world_scene_state = { }, walk_animation_frame_count = R(1, 8), target_selection_linked_slot = R(0, 7),
    target_selection_current_slot = R(0, 7), target_selection_advances_cursor = bool(),
    source_animation_selector = num(), source_action_slot = R(0, 7), source_action_counter = num(), source_action_ammo = R(0, 30),
    projectile_attack_seed_pending = bool(), projectile_animation = bool(), projectile_movement = maybe(mstate),
    movement_heading_override = maybe(num), item_action_threshold = num(), item_action_increment = num(),
    item_action_frame_count = R(1, 8), hurt_audio_event = num(), hurt_animation_threshold = num(),
    hurt_animation_increment = num(), hurt_animation_frame_count = R(1, 8), down_audio_event = num(),
    delay_secondary_attribute = R(0, 10), delay_primary_attribute = R(0, 10), delay_capability_flags = num(),
    combat_input_blocked = bool(), combat_confirm_held = bool(), combat_action_ready = bool(), combat_action_available = bool(),
    animation_mode = pick({ "attack", "walking", "idle" }), animation_direction_override = maybe(function() return R(0, 7) end),
    animation_accumulator_override = maybe(num), world_activity_active = bool(), target_selection_special_marker = bool(),
    target_selection_requested = bool(), target_selection_owner_slot = R(0, 7), target_selection_latched = bool(),
    target_heading_audio_alternate = bool(), source_spawn_state_flags = num(),
    source_item_spawn_result = pick({ "spawned", "failed" }), source_interaction_flags = num(), source_actor_posture = R(0, 7),
    projectile_combat_targets_n = nil, movement_transition_active = bool(), movement_services_available = bool(),
    movement_collision_mode = R(0, 3), initial_action_delay = R(0, 15), combat_meter_service_blocked = bool(),
    action_line_clear = bool(),
  }
end

local function deepcopy(v, seen)
  if type(v) ~= "table" then return v end
  seen = seen or {}
  if seen[v] then return seen[v] end
  local c = {} seen[v] = c
  for k, x in pairs(v) do c[deepcopy(k, seen)] = deepcopy(x, seen) end
  return c
end
local function serialize_actor_states_for_parity(actors)
  -- The split runtime caches the raw record slot on each persistent actor so
  -- typed host slots can be checked against ROM records. The bfa3908 baseline
  -- consumes source_record_slot from each heartbeat but does not cache it at
  -- the actor root. Ignore only that root-level metadata extension here;
  -- returned states, nested state, contexts, and service traces still compare.
  local comparable = deepcopy(actors)
  for _, state in pairs(comparable) do
    if type(state) == "table" then state.source_record_slot = nil end
  end
  return ser(comparable)
end
local function norm_err(e) return (tostring(e):gsub("[^%s:]+%.lua:%d+:", "<src>:")) end

local calls, nonnil, errs, mism = 0, 0, 0, 0
local FRAMES = tonumber(arg[3]) or 120
local function first_difference(a, b)
  local offset = 1
  while offset <= #a and offset <= #b and a:byte(offset) == b:byte(offset) do
    offset = offset + 1
  end
  return offset
end
for trial = 1, TRIALS do
  for k in pairs(gameA.actors) do gameA.actors[k] = nil end
  for k in pairs(gameB.actors) do gameB.actors[k] = nil end
  -- a few persistent actors per trial; each keeps its identity fields and re-rolls volatile ones
  local bases = {}
  for i, id in ipairs({ "field_player_runner", "a1", "a2" }) do
    local b = context(id); b.spawn = true; bases[i] = b
  end
  for frame = 1, FRAMES do
    local bi = R(#bases)
    local base = bases[bi]
    local fresh = context(base.actor_id)
    local ctx = {}
    for k, v in pairs(base) do ctx[k] = v end
    local fk = {} for k in pairs(fresh) do fk[#fk + 1] = k end table.sort(fk)
    for _, k in ipairs(fk) do local v = fresh[k]
      if k ~= "profile_id" and k ~= "entity_kind" and k ~= "action_kind" and k ~= "action_code" and
         k ~= "dynamic_world_encounter_actor" and k ~= "actor_id" and R(100) <= 25 then ctx[k] = v end
    end
    ctx.spawn = base.spawn or R(200) == 1
    base.spawn = false
    ctx.phase = (frame % 2 == 0) and "render" or "behavior"
    if R(10) == 1 then ctx.phase = fresh.phase end
    local cA, cB = deepcopy(ctx), deepcopy(ctx)
    cB.ai_random_bound, cA.ai_random_bound = ctx.ai_random_bound, ctx.ai_random_bound
    for i = #logA, 1, -1 do logA[i] = nil end
    for i = #logB, 1, -1 do logB[i] = nil end
    _G.game = gameA; local okA, rA = pcall(A.heartbeat, cA)
    _G.game = gameB; local okB, rB = pcall(B.heartbeat, cB)
    calls = calls + 1
    if okA and rA ~= nil then nonnil = nonnil + 1 end
    if not okA then errs = errs + 1 end
    local partsA = {
      tostring(okA), okA and ser(rA) or norm_err(rA), ser(cA),
      serialize_actor_states_for_parity(gameA.actors), table.concat(logA, ";"),
    }
    local partsB = {
      tostring(okB), okB and ser(rB) or norm_err(rB), ser(cB),
      serialize_actor_states_for_parity(gameB.actors), table.concat(logB, ";"),
    }
    local sA, sB = table.concat(partsA, "|"), table.concat(partsB, "|")
    if sA ~= sB then
      mism = mism + 1
      if mism <= 3 then
        local labels = { "success", "result", "context", "actors", "services" }
        io.stderr:write(("MISMATCH trial %d frame %d\n"):format(trial, frame))
        for index, label in ipairs(labels) do
          if partsA[index] ~= partsB[index] then
            local offset = first_difference(partsA[index], partsB[index])
            io.stderr:write((" %s differs at byte %d\n A: %s\n B: %s\n"):
              format(label, offset,
                partsA[index]:sub(math.max(1, offset - 80), offset + 140),
                partsB[index]:sub(math.max(1, offset - 80), offset + 140)))
          end
        end
      end
    end
  end
end
print(("calls=%d non_nil_results=%d errors(both)=%d mismatches=%d"):format(calls, nonnil, errs, mism))
if not COVERAGE then os.exit(mism == 0 and 0 or 1) end
