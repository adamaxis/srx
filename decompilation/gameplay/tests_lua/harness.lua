-- harness.lua: shared serializer, typed service mocks and context generator (extracted from equiv_heartbeat.lua)
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local Enums = require("enums")
local VisualProfile = Enums.ActorVisualProfile
local MovementDirection = Enums.MovementDirection
-- The original monolith is the pre-split reference (bfa3908); the live actor_ai.lua is a shim to actor.init.
local ORIG = os.getenv("SHADOWRUN_ORIG_ACTOR_AI") or (ROOT .. "/decompilation/gameplay/tests_lua/reference/actor_ai_bfa3908.lua")
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
  ["movement.probe"] = function(h)
    local kind = h % 5 == 0 and "requires_recovery" or
      h % 3 == 0 and "clear" or "blocked"
    return { kind = kind,
      blocks_route_fallback = kind == "blocked" or
        (kind == "requires_recovery" and h % 2 == 0),
      terrain_is_clear = h % 7 == 0 }
  end,
  ["movement.follow_route_step_clear"] = function(h) return h % 2 == 0 end,
  ["movement.projectile_terrain"] = function(h)
    return { blocked = (h >> 3) % 4 ~= 0,
      boundary_active = b(h, 8), outside_map = b(h, 9) }
  end,
  ["movement.projectile_collision"] = function(h)
    local hit = b(h, 2)
    local has_position = hit and b(h, 3)
    return { kind = not hit and "none" or has_position and "target_hit" or
        "linked_followup",
      target_position = has_position and {
        x = (h >> 4) % 0x10000, y = (h >> 12) % 0x10000 } or nil,
      target_id = hit and b(h, 20) and ("a" .. (h % 3)) or nil }
  end,
  ["movement.recover_collision"] = function(h) return b(h, 2), b(h, 4) end,
  ["movement.route"] = function(h)
    local directions = { "north", "north_east", "east", "south_east",
      "south", "south_west", "west", "north_west" }
    return { primary_route_marker_present = h % 3 == 0,
      secondary_route_marker_present = (h >> 2) % 3 == 0,
      primary_fallback_direction = directions[(h >> 4) % 8 + 1],
      secondary_fallback_direction = directions[(h >> 8) % 8 + 1] }
  end,
  ["entity_motion.heading_offsets"] = function(source_heading)
    local step = require("entity_motion").projectile_step(
      source_heading)
    return {
      horizontal = math.floor(step.x * 256),
      vertical = math.floor(step.y * 256),
    }
  end,
  ["combat.resolve_weapon_attack_seed"] = function(h) return (h >> 3) % 256 end,
  ["combat.resolve_reaction"] = function(h) return { damage = h % 5, stun = (h >> 3) % 3 } end,
  ["combat.resolve_weapon_hit"] = function(h) return { damage = h % 5 } end,
  ["combat.resolve_spell_action"] = function(h)
    local successes = (h >> 3) % 256
    return { action_result = successes, successes = successes,
      next_action_kind = ({ "none", "item", "spell" })[h % 3 + 1] }
  end,
  ["magic.actor_effect_profile"] = function(h) return { duration = (h >> 3) % 64, kind = h % 4 } end,
}
local function make_game(log, legacy_services)
  local count = 0
  local function svc(path)
    return setmetatable({}, { __index = function(_, name)
      local full = path .. "." .. name
      return function(...)
        count = count + 1
        local arguments = { ... }
        local logged_full = full
        if full == "entity_motion.heading_offsets" and
           type(arguments[1]) == "number" then
          logged_full = "movement.projectile_step_offsets"
          arguments[1] = (arguments[1] % 0x10000) * 360 / 0x10000
        end
        if full == "movement.projectile_terrain" or
           full == "movement.projectile_collision" then
          for index = 1, 2 do
            if type(arguments[index]) == "number" then
              arguments[index] = math.floor(arguments[index]) % 0x10000
            end
          end
        end
        local a = ser(arguments)
        if full == "movement.collision" or full == "movement.probe" or
           full == "movement.follow_route_step_clear" then
          logged_full = "movement.collision"
        end
        -- The old module requested a pure vector calculation from a host
        -- service. The port now calculates the vector in Lua; keep comparing
        -- resulting gameplay state without treating that internal call as an
        -- externally visible host action.
        if full ~= "entity_motion.heading_offsets" then
          log[#log + 1] = logged_full .. a
        end
        -- Keep the frozen reference and modern port on the same open-path
        -- fixture. ROM collision encodings are covered by embedded runtime QA.
        if full == "movement.collision" then return 0, 0 end
        if full == "movement.probe" then
          return { kind = "clear", blocks_route_fallback = false,
            terrain_is_clear = true }
        end
        if full == "movement.follow_route_step_clear" then return true end
        if full == "movement.recover_collision" then
          return b(hash(logged_full .. a .. count), 2),
            legacy_services and 0 or false
        end
        local h = hash(logged_full .. a .. count)
        local T = TYPED[full]
        if T and h % 7 ~= 0 then
          if full == "movement.route" and legacy_services then
            return { primary_marker = (h % 3) - 1,
              secondary_marker = ((h >> 2) % 3) - 1,
              primary_fallback_slot = (h >> 4) % 8,
              secondary_fallback_slot = (h >> 8) % 8 }
          end
          if full == "entity_motion.heading_offsets" then
            return T(arguments[1])
          end
          local result = T(h)
          if legacy_services and full == "movement.projectile_terrain" then
            return result.blocked and 1 or 0,
              result.boundary_active, result.outside_map
          end
          if legacy_services and full == "movement.projectile_collision" then
            local position = result.target_position or {}
            return result.kind ~= "none", result.kind == "target_hit",
              position.x or 0, position.y or 0, result.target_id
          end
          return result
        end
        local k = h % 5
        if k == 0 then return nil
        elseif k == 1 then return h % 3 == 0
        elseif k == 2 then return (h >> 4) % 0x10000
        else
          return { blocked = (h & 1) == 1, hit = (h & 2) == 2, position_x = (h >> 3) % 0x10000,
                   position_y = (h >> 7) % 0x10000, terrain_collision_code = (h >> 11) % 4,
                   bearing_degrees = ((h >> 13) % 0x10000) * 360 / 0x10000,
                   status = (h >> 2) % 3, seed = (h >> 5) % 256,
                   damage = (h >> 9) % 10, next_profile_id = nil, collision_code = (h >> 6) % 4 }
        end
      end
    end })
  end
  return { actors = {}, movement = svc("movement"),
           modules = { combat = svc("combat"), entity_motion = svc("entity_motion"),
                       items = svc("items"), magic = svc("magic") } }
end

local R = math.random
local function pick(t) return t[R(#t)] end
local function actor_behavior_profile()
  local profiles = {
    { engagement_policy = "patrol", approach_band = "far" },
    { engagement_policy = "watch_nearby_runner", approach_band = "medium" },
    { engagement_policy = "engage_within_range", approach_band = "medium" },
    { engagement_policy = "engage", approach_band = "medium" },
    { engagement_policy = "engage", approach_band = "close" },
    { engagement_policy = "engage" },
  }
  local selected = pick(profiles)
  local result = {}
  for name, value in pairs(selected) do result[name] = value end
  return result
end
-- profile ids from the source
local src = io.open(ORIG):read("a")
local pids = {}
for id in src:gmatch('%["([%w_]+)"%] = {') do pids[#pids + 1] = id end
local kinds = {}
for id in src:gmatch('"([%w_]+_k%x%x_[a-z]+%x%x)"') do kinds[id] = true end

local function num()
  local k = R(6)
  if k == 1 then return 0 elseif k == 2 then return R(0, 15) elseif k == 3 then return R(0, 0xff)
  elseif k == 4 then return R(0, 0xffff) elseif k == 5 then return R(1, 8) else return pick({ 0x28, 15, 0x10, 0x20, 0x80 }) end
end
local function bearing()
  return R(0, 0xffff) * 360 / 0x10000
end
local function maybe(f) if R(5) == 1 then return nil end return f() end
local function bool() return R(2) == 1 end
local function cand(i)
  local projected_y_nonzero = bool()
  return { id = "a" .. R(4), combatant = bool(), has_action = projected_y_nonzero,
           projected_y_nonzero = projected_y_nonzero, service_flags = num(), behavior_flags = num(),
           spawn_state_flags = num(), source_record_slot = R(0, 7), world_activity_actor = bool(),
           direct_route_step_cost = num(), direct_route_distance = num(), perpendicular_route_distance = num(),
           perpendicular_route_step_cost = num(), distance = num(),
           bearing_degrees = bearing(), world_x = num(), world_y = num(),
           facing_degrees = bearing() }
end
local function list(f, n) local t = {} for i = 1, R(0, n) do t[i] = f(i) end return t end
local function mstate()
  return { animation_bearing_degrees = bearing(), collision_context = num(), collision_extent = num(),
           bearing_degrees = bearing(), position_x = num(), position_y = num(), route_status = num(),
           terrain_collision_code = num(), substate_update = "unchanged",
           cardinal_direction = R(0, 3), mark_reset_after_block = bool(),
           mark_directional_fallback = bool(), clear_collision_recovery = bool(),
           clear_pending_event = bool(), route_facts = {
             route_clear = bool(), path_active = bool(), restart_steering = bool(),
             wall_following = bool(), in_target_band = bool(), route_transition = bool(),
             fleeing = bool(), escort = bool() } }
end
local function offsets() local t = {} for i = 1, 8 do t[i] = { R(-32, 32), R(-32, 32) } end return t end

local kinds_list = {} for k in pairs(kinds) do kinds_list[#kinds_list + 1] = k end table.sort(kinds_list)
local SpellIdByActionCode = {
  [1] = "flame_dart", [2] = "flame_bolt", [3] = "hellblast",
  [4] = "invisibility", [5] = "sleep", [6] = "stink",
  [7] = "super_barrier", [8] = "mana_zap", [9] = "mana_blast",
  [10] = "mana_storm", [11] = "rockskin", [12] = "heal_wounds",
  [13] = "confusion", [14] = "barrier",
}
local VisualProfileBySourceKind = {
  [1] = VisualProfile.HIT_SPARK,
  [3] = VisualProfile.FIELD_RUNNER,
  [4] = VisualProfile.ORK_BANDERSNATCH,
  [5] = VisualProfile.HUMAN_GUARD,
  [6] = VisualProfile.ELVEN_MAGE,
  [7] = VisualProfile.GHOUL_EYE_FIVER,
  [8] = VisualProfile.YAKUZA_WEENR,
  [9] = VisualProfile.VAMPIRE_GATOR_SHAMAN,
  [10] = VisualProfile.LONE_STAR_MAGE,
  [11] = VisualProfile.PEDESTRIAN_RAT_SHAMAN,
  [12] = VisualProfile.HELL_HOUND,
  [13] = VisualProfile.GARGOYLE,
  [14] = VisualProfile.WENDIGO,
  [15] = VisualProfile.THON,
}
local VisualProfileIds = {
  VisualProfile.FIELD_RUNNER, VisualProfile.ORK_BANDERSNATCH,
  VisualProfile.HUMAN_GUARD, VisualProfile.ELVEN_MAGE,
  VisualProfile.GHOUL_EYE_FIVER, VisualProfile.YAKUZA_WEENR,
  VisualProfile.VAMPIRE_GATOR_SHAMAN, VisualProfile.LONE_STAR_MAGE,
  VisualProfile.PEDESTRIAN_RAT_SHAMAN, VisualProfile.HELL_HOUND,
  VisualProfile.GARGOYLE, VisualProfile.WENDIGO, VisualProfile.THON,
}
local function context(actor)
  local ammunition_remaining = R(0, 30)
  local pid = pick(pids)
  local dyn = R(4) == 1
  if dyn then pid = (#kinds_list > 0 and R(2) == 1) and pick(kinds_list) or string.format("x_k%02x_%s%02x", R(4, 14), pick({ "item", "spell", "none" }), R(0, 15)) end
  local ek, ak, ac = pid:match("_k(%x%x)_([a-z]+)(%x%x)$")
  local action_code = ac and tonumber(ac, 16) or 0
  return {
    actor_id = actor, profile_id = pid, dynamic_world_encounter_actor = dyn,
    visual_profile_id = ek and VisualProfileBySourceKind[tonumber(ek, 16)]
        or pick(VisualProfileIds), action_kind = ak or "none", action_code = action_code,
    action_spell_id = ak == "spell" and SpellIdByActionCode[action_code] or nil,
    phase = pick({ "behavior", "render", "behavior" }), spawn = R(6) == 1,
    initial_bearing_degrees = bearing(), entry_delay = R(0, 16), entry_animation_frame_count = R(1, 8),
    source_record_flags = R(4) == 1 and 0 or 1, source_record_slot = R(0, 7), combatant = bool(),
    action_timing = { quickness = R(0, 10), intelligence = R(0, 10),
                      modifier = pick({ 0, 4, 8, 12 }) },
    ai_target_candidates = maybe(function() return list(cand, 4) end),
    world_target_selection_candidates = maybe(function() return list(cand, 3) end),
    projectile_combat_targets = maybe(function() return list(cand, 3) end),
    movement_state = mstate(), movement_offsets = offsets(),
    movement_input_direction = maybe(function()
      return pick({ MovementDirection.NORTH, MovementDirection.NORTH_EAST,
                    MovementDirection.EAST, MovementDirection.SOUTH_EAST,
                    MovementDirection.SOUTH, MovementDirection.SOUTH_WEST,
                    MovementDirection.WEST, MovementDirection.NORTH_WEST })
    end),
    world_effect_slot_occupancy = list(bool, 8),
    scene_id = num(), movement_scene_id = num(), movement_allowed = bool(),
    action_animation_threshold = num(), action_animation_increment = num(), target_selection_mode = R(0, 3),
    source_projection_active = bool(), source_behavior_flags = num(), movement_entity_flags = num(),
    ai_random_bound = function(n) return (n and n > 0) and ((#actor * 7) % n) or 0 end,
    ai_animation_frame = num(), action_animation_frame_count = R(1, 8), action_animation_audio_event = num(),
    field_sequence_active = bool(), final_battle_active = bool(),
    walk_animation_frame_count = R(1, 8), target_selection_linked_slot = R(0, 7),
    target_selection_current_slot = R(0, 7), target_selection_advances_cursor = bool(),
    ammunition_remaining = ammunition_remaining,
    hold_for_ending_activity = false,
    projectile_attack_seed_pending = bool(), projectile_animation = bool(), projectile_movement = maybe(mstate),
    movement_direction_override = maybe(function()
      return pick({ MovementDirection.NORTH, MovementDirection.NORTH_EAST,
                    MovementDirection.EAST, MovementDirection.SOUTH_EAST,
                    MovementDirection.SOUTH, MovementDirection.SOUTH_WEST,
                    MovementDirection.WEST, MovementDirection.NORTH_WEST })
    end), item_action_threshold = num(), item_action_increment = num(),
    item_action_frame_count = R(1, 8), hurt_audio_event = num(), hurt_animation_threshold = num(),
    hurt_animation_increment = num(), hurt_animation_frame_count = R(1, 8), down_audio_event = num(),
    combat_input_blocked = bool(), combat_confirm_held = bool(), combat_action_ready = bool(), combat_action_available = bool(),
    animation_mode = pick({ "attack", "walking", "idle" }), animation_direction_override = maybe(function() return R(0, 7) end),
    animation_accumulator_override = maybe(num), world_activity_active = bool(), target_selection_special_marker = bool(),
    target_selection_requested = bool(), target_selection_owner_slot = R(0, 7), target_selection_latched = bool(),
    target_heading_audio_alternate = bool(), source_spawn_state_flags = num(),
    source_item_spawn_result = pick({ "spawned", "failed" }), source_interaction_flags = num(),
    actor_behavior_profile = actor_behavior_profile(),
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
local function norm_err(e)
  -- strip file:line and local-variable names (lib.int68k helpers name their parameter differently)
  return (tostring(e):gsub("[^%s:]+%.lua:%d+:", "<src>:"):gsub(" %(local '[%w_]+'%)", ""):gsub(" %(upvalue '[%w_]+'%)", ""))
end
return { ORIG = ORIG, ser = ser, make_game = make_game, context = context, cand = cand, mstate = mstate, num = num, bearing = bearing, bool = bool, list = list, deepcopy = deepcopy, norm_err = norm_err, pids = pids }
