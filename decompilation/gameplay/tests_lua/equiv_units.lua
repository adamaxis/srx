-- Function-level differential test: calls each internal actions/anim/targeting/movement function of the original
-- actor_ai.lua (found through heartbeat's upvalue graph) and of the split (actor.registry) with identical random
-- state/context/arguments, and compares returns, state, context, actor table and service log.
-- Usage: LUA_PATH=... lua5.4 equiv_units.lua [trials_per_function] [seed]
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
local TRIALS, SEED = tonumber(arg[1]) or 300, tonumber(arg[2]) or 1
local ser, deepcopy = H.ser, H.deepcopy

local logA, logB = {}, {}
local gameA, gameB = H.make_game(logA, true), H.make_game(logB, false)
_G.game = gameA; local A = dofile(H.ORIG)
_G.game = gameB; require("actor.init"); local REG = require("actor.registry")
local E = require("enums")
local MovementBearingDegrees = E.MovementBearingDegrees
local Items = require("items")

-- Translate between semantic gameplay bearings and the frozen reference's
-- source-word fields. This adapter exists only in the differential test.
local source_bearing_fields = {
  initial_heading = "initial_bearing_degrees",
  heading = "bearing_degrees",
  source_record_heading = "facing_degrees",
  movement_heading = "movement_bearing_degrees",
  movement_current_heading = "movement_current_bearing_degrees",
  ai_target_heading = "ai_target_bearing_degrees",
  strike_heading = "strike_bearing_degrees",
  watch_heading = "watch_bearing_degrees",
  animation_heading = "animation_bearing_degrees",
}
local semantic_bearing_fields = {}
for source_name, semantic_name in pairs(source_bearing_fields) do
  semantic_bearing_fields[semantic_name] = source_name
end
local function source_units_to_degrees(value)
  return (value % 0x10000) * 360 / 0x10000
end
local function degrees_to_source_units(value)
  return math.floor((value % 360) * 0x10000 / 360 + 0.5) % 0x10000
end
local function source_fixed_to_map(value)
  local bits = value % 0x100000000
  return (bits >> 16) + (bits & 0xffff) / 0x10000
end
local function map_to_source_fixed(value)
  return math.floor(value * 0x10000 + 0.5) % 0x100000000
end
local function map_step_to_source(value)
  local word = math.floor(value * 0x100) % 0x10000
  return word >= 0x8000 and word - 0x10000 or word
end
local function legacy_projectile_coordinates(value)
  if type(value) ~= "table" or
     type(value.position) ~= "table" or
     type(value.velocity) ~= "table" or
     type(value.burst_origin) ~= "table" then
    return
  end
  local vector = require("entity_motion").projectile_step(
    value.bearing_degrees)
  value.world_x = map_to_source_fixed(value.position.x)
  value.world_y = map_to_source_fixed(value.position.y)
  value.tile_offset_x = map_step_to_source(vector.x)
  value.tile_offset_y = map_step_to_source(vector.y)
  value.saved_position_x = value.burst_origin.x
  value.saved_position_y = value.burst_origin.y
  value.active = value.phase ~= "inactive"
  value.waiting_for_animation_end = value.phase == "awaiting_animation"
  value.remaining_clear_steps = math.floor(
    value.remaining_flight_range / 96 + 0.5) - 1
  value.additional_shots_remaining = value.shots_remaining
  value.heading = degrees_to_source_units(value.bearing_degrees)
  value.action_result = value.attack_successes
  value.position = nil
  value.velocity = nil
  value.burst_origin = nil
  value.phase = nil
  value.remaining_flight_range = nil
  value.shots_remaining = nil
  value.bearing_degrees = nil
  value.attack_successes = nil
end
local function semanticize_projectile_coordinates(value)
  if type(value) ~= "table" or
     type(value.world_x) ~= "number" or
     type(value.world_y) ~= "number" or
     type(value.tile_offset_x) ~= "number" or
     type(value.tile_offset_y) ~= "number" then
    return
  end
  value.position = {
    x = source_fixed_to_map(value.world_x),
    y = source_fixed_to_map(value.world_y),
  }
  value.velocity = {
    x = value.tile_offset_x / 0x100,
    y = value.tile_offset_y / 0x100,
  }
  value.burst_origin = {
    x = value.saved_position_x,
    y = value.saved_position_y,
  }
  value.remaining_flight_range =
    ((value.remaining_clear_steps or 0) + 1) * 96
  value.shots_remaining = value.additional_shots_remaining or 0
  value.phase = not value.active and "inactive" or
    value.waiting_for_animation_end and "awaiting_animation" or "flying"
  value.bearing_degrees = source_units_to_degrees(value.heading or 0)
  value.attack_successes = value.action_result
  value.remaining_clear_steps = nil
  value.additional_shots_remaining = nil
  value.waiting_for_animation_end = nil
  value.active = nil
  value.heading = nil
  value.action_result = nil
  value.world_x = nil
  value.world_y = nil
  value.tile_offset_x = nil
  value.tile_offset_y = nil
  value.saved_position_x = nil
  value.saved_position_y = nil
end
local function convert_bearings(value, to_semantic, seen)
  if type(value) ~= "table" then return value end
  seen = seen or {}
  if seen[value] then return value end
  seen[value] = true
  local renames = {}
  for key, item in pairs(value) do
    convert_bearings(item, to_semantic, seen)
    local new_key
    if to_semantic then
      new_key = source_bearing_fields[key]
    else
      new_key = semantic_bearing_fields[key]
    end
    if new_key ~= nil then
      if type(item) == "number" then
        item = to_semantic and source_units_to_degrees(item) or
          degrees_to_source_units(item)
      end
      renames[#renames + 1] = { key, new_key, item }
    end
  end
  for _, rename in ipairs(renames) do
    value[rename[2]] = rename[3]
    value[rename[1]] = nil
  end
  return value
end
local function reference_context(context)
  local result = deepcopy(context)
  legacy_projectile_coordinates(result.projectile_movement)
  convert_bearings(result, false)
  local direction = context.movement_direction_override
  if direction ~= nil and MovementBearingDegrees[direction] ~= nil then
    result.movement_heading_override = degrees_to_source_units(
      MovementBearingDegrees[direction])
  end
  result.movement_direction_override = nil
  return result
end
local function semantic_reference_context(context)
  local source_heading = context.movement_heading_override
  if type(source_heading) == "number" then
    local degrees = source_units_to_degrees(source_heading)
    for direction, value in pairs(MovementBearingDegrees) do
      if value == degrees then
        context.movement_direction_override = direction
        break
      end
    end
  end
  context.movement_heading_override = nil
  semanticize_projectile_coordinates(context.projectile_movement)
  return convert_bearings(context, true)
end
-- The frozen differential reference still models the old source selector.
local LEGACY_ITEM_POSE_SELECTOR = 0x40
local SpellIdByActionCode = {
  [1] = "flame_dart", [2] = "flame_bolt", [3] = "hellblast",
  [4] = "invisibility", [5] = "sleep", [6] = "stink",
  [7] = "super_barrier", [8] = "mana_zap", [9] = "mana_blast",
  [10] = "mana_storm", [11] = "rockskin", [12] = "heal_wounds",
  [13] = "confusion", [14] = "barrier",
}
local function actor_facts(active, ai_runner)
  return {
    lifecycle = {
      active = active == true, thinking_enabled = false,
      ai_runner = ai_runner == true, downed = false,
      group_leader = false, dead = false,
    },
    interaction = {},
    behavior = { in_target_band = false, escort = false },
    roles = { pedestrian = false, quest = false },
  }
end
local function shape_delay_inputs(st, ctx)
  local timing = ctx.action_timing
  if type(timing) ~= "table" then
    timing = { quickness = 0, intelligence = 0, modifier = 0 }
    ctx.action_timing = timing
  end
  timing.action_delay_modifier = timing.modifier or 0
  st.delay_primary_attribute = timing.quickness or 0
  st.delay_secondary_attribute = timing.intelligence or 0
  local modifier_bit = ({ [0] = 0, [4] = 12, [8] = 13, [12] = 14 })[
    timing.action_delay_modifier]
  st.delay_capability_flags = modifier_bit == 0 and 0 or
    modifier_bit and (1 << modifier_bit) or 0
end
local function target_candidate(slot, active, world_activity_actor)
  return {
    id = "a" .. (slot % 3 + 1),
    source_record_slot = slot,
    service_flags = active and 0x01 or 0,
    behavior_flags = 0,
    spawn_state_flags = 0,
    has_action = true,
    projected_y_nonzero = true,
    world_activity_actor = world_activity_actor == true,
    actor_facts = actor_facts(active, false),
  }
end
gameA.modules.items = Items
gameB.modules.items = Items

-- Keep projectile comparisons on one stable open-path service fixture. The
-- two actor APIs encode terrain/collision responses differently at the Lua
-- boundary; collision outcomes are covered by the focused projectile tests.
gameA.movement.projectile_terrain = function() return 0, false, false end
gameB.movement.projectile_terrain = function()
  return { blocked = false, boundary_active = false, outside_map = false }
end
gameA.movement.projectile_collision = function()
  return false, false, 0, 0, nil
end
gameB.movement.projectile_collision = function()
  return { kind = "none" }
end
gameA.modules.combat.resolve_weapon_attack_seed = function() return 13 end
gameB.modules.combat.resolve_weapon_attack_seed = function() return 13 end
gameA.modules.entity_motion.heading_offsets = function(source_heading)
  local step = require("entity_motion").projectile_step(
    (source_heading % 0x10000) * 360 / 0x10000)
  return {
    horizontal = math.floor(step.x * 0x100),
    vertical = math.floor(step.y * 0x100),
  }
end

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
  local successes = H.num()
  return { action_code = R(0, 13), action_result = successes, successes = successes, ai_target_candidates = H.list(H.cand, 3), caster_actor_id = "a" .. R(3),
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
  local phase = R(6) == 1 and "inactive" or
    R(2) == 1 and "awaiting_animation" or "flying"
  -- Cardinal headings isolate lifecycle comparison with identical source and
  -- semantic offsets; entity_motion_tests covers the shared vector contract.
  local bearing_degrees = R(0, 3) * 90
  local velocity = require("entity_motion").projectile_step(bearing_degrees)
  return { role = R(5) == 1 and "secondary" or "primary", phase = phase,
    impact_occurred = R(2) == 1,
    animation_finished = R(2) == 1, animation_action = "unchanged",
    position = { x = source_fixed_to_map(R(0, 0xffffff)),
      y = source_fixed_to_map(R(0, 0xffffff)) },
    velocity = velocity,
    remaining_flight_range = (R(0, 4) + 1) * 96,
    shots_remaining = R(0, 2),
    bearing_degrees = bearing_degrees,
    burst_origin = { x = R(0, 0xffff), y = R(0, 0xffff) },
    attack_successes = R(0, 255) }
end

-- Keep the frozen pre-split reference in its original source-shaped format.
-- Only this test adapter translates the new gameplay state to/from the old
-- counters; production Lua receives neither ROM field nor byte arithmetic.
local function prepare_reference_projectile(v)
  if type(v) ~= "table" then return end
  if type(v.additional_shots_remaining) == "number" and
     type(v.remaining_clear_steps) == "number" then
    v.flags = v.active and 0x01 or 0
    v.record_flags = (v.waiting_for_animation_end and 0x08 or 0) |
      (v.impact_occurred and 0x01 or 0)
    v.animation_selector = v.animation_finished and 0x8000 or 0
    v.retry_counter = v.remaining_clear_steps
    v.linked_action_timer = v.additional_shots_remaining
    v.remaining_clear_steps = nil
    v.additional_shots_remaining = nil
    v.waiting_for_animation_end = nil
    v.impact_occurred = nil
    v.animation_finished = nil
    v.animation_action = nil
    v.phase = nil
  end
end
local function normalize_reference_projectile(v, seen)
  if type(v) ~= "table" then return end
  seen = seen or {}
  if seen[v] then return end
  seen[v] = true
  if type(v.retry_counter) == "number" or
     type(v.linked_action_timer) == "number" or
     type(v.flags) == "number" or type(v.record_flags) == "number" or
     type(v.animation_selector) == "number" then
    if type(v.flags) == "number" then
      v.active = (v.flags & 0x01) ~= 0
      v.flags = nil
    end
    if type(v.record_flags) == "number" then
      v.waiting_for_animation_end = (v.record_flags & 0x08) ~= 0
      v.impact_occurred = (v.record_flags & 0x01) ~= 0
      v.record_flags = nil
    end
    if type(v.animation_selector) == "number" then
      v.animation_finished = (v.animation_selector & 0x8000) ~= 0
      v.animation_selector = nil
    end
    if type(v.retry_counter) == "number" then
      v.remaining_clear_steps = v.retry_counter >= 0x80 and 0 or v.retry_counter
      v.retry_counter = nil
    end
    if type(v.linked_action_timer) == "number" then
      v.additional_shots_remaining = math.max(0, v.linked_action_timer)
      v.linked_action_timer = nil
    end
    if v.outcome == "burst_followup_restarted" then
      v.animation_action = "restart"
    elseif v.role == "primary" and
        (v.outcome == "stopped" or
         v.outcome == "hit_resolution_pending" or
         v.outcome == "linked_followup_pending" or
         (v.outcome == "retired" and v.animation_finished == false)) then
      v.animation_action = "stop"
    elseif v.animation_action == nil then
      v.animation_action = "unchanged"
    end
  end
  semanticize_projectile_coordinates(v)
  for _, child in pairs(v) do normalize_reference_projectile(child, seen) end
end
local function canonical_animation_state(st, pose)
  st.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD
  st.direction = 0
  return REG.Animator.list(st.visual_profile_id, pose, st.direction)
end
local SHAPE = {
  advance_dynamic_actor_walk_animation = function(st, ctx)
    local list = canonical_animation_state(st, REG.Animator.Clip.WALK)
    ctx.walk_animation_frame_count = list.frame_count
    st.walk_animation_frame_count = list.frame_count
    st.animation_cadence_profile = "moving_actor_walk"
    st.movement_distance_per_update = 9
    st.animation_frame = 0
    st.animation_progress = 0
  end,
  advance_dynamic_actor_action_animation = function(st)
    local list = canonical_animation_state(st, REG.Animator.Clip.ACTION)
    st.action_animation_frame_count = list.frame_count
    st.action_animation_increment = 0x200
    st.action_animation_threshold = 0x1000
    st.animation_frame = 0
    st.animation_accumulator = 0
    st.action_animation_complete = false
  end,
  advance_dynamic_actor_item_action_animation = function(st)
    local list = canonical_animation_state(st, REG.Animator.Clip.ITEM)
    st.item_action_frame_count = list.frame_count
    st.item_action_increment = 0x80
    st.item_action_threshold = 0x1000
    st.source_animation_selector = LEGACY_ITEM_POSE_SELECTOR
    st.animation_frame = 0
    st.animation_accumulator = 0
    st.item_action_complete = false
  end,
  begin_dynamic_target_action = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true; st.action_delay = -R(1, 10); st.target_motion_effect_active = false
    ctx.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD; st.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD
    st.movement_current_bearing_degrees = 22.5
    ctx.initial_bearing_degrees = 22.5
  end,
  projectile_movement_snapshot = function(st, ctx)
    ctx.projectile_movement = projectile_source()
    st.action_result = ctx.projectile_movement.attack_successes
    st.last_action_successes = ctx.projectile_movement.attack_successes
  end,
  projectile_heartbeat = function(st, ctx)
    ctx.projectile_movement = projectile_source()
    ctx.phase = "behavior"
    ctx.projectile_attack_seed_pending = false
    st.animation_id = "projectile"
    st.direction = R(0, 7)
    st.action_result = ctx.projectile_movement.attack_successes
    st.last_action_successes = ctx.projectile_movement.attack_successes
    st.attack_seed_resolved = R(2) == 1
    st.spell_effect_action_code = 5
    st.spell_effect_id = "sleep"
  end,
  snapshot = function(st)
    st.action_kind = "none"; st.action_code = 0; st.action_id = nil
    st.animation_id = "spell"
    st.direction = R(0, 7)
    st.animation_frame = R(0, 5)
    st.spell_effect_action_code = 5
    st.spell_effect_id = "sleep"
  end,
  begin_received_hit_reaction = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true
    ctx.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD; st.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD
    st.incoming_hit = { action_result = R(0, 255), damage = R(0, 5), source = "a1",
      physical_meter = R(0, 5), mental_meter = R(0, 5), down = R(3) == 1 }
  end,
  begin_dynamic_item_action = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true; ctx.action_kind = "item"; ctx.action_code = R(1, 15); ctx.action_item_id = Items.catalog[ctx.action_code].id; st.action_delay = -R(1, 5)
    ctx.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD; st.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD
    ctx.source_item_spawn_result = "spawned"; ctx.action_line_clear = R(4) ~= 1
    ctx.source_projection_active = true; ctx.can_start_item_action = false
    st.source_animation_selector = LEGACY_ITEM_POSE_SELECTOR
  end,
  begin_spell_cast = function(st, ctx)
    ctx.dynamic_world_encounter_actor = true; ctx.action_kind = "spell"; ctx.action_code = R(0, 13); ctx.action_spell_id = SpellIdByActionCode[ctx.action_code]; st.action_delay = -R(1, 5)
    ctx.actor_facts = actor_facts(true, false)
    st.spell_effect_action_code = 0
    st.spell_effect_id = nil
    shape_delay_inputs(st, ctx)
  end,
  advance_spell_effect = function(st, ctx)
    local effect = pending()
    effect.action_code = 5
    effect.spell_id = SpellIdByActionCode[effect.action_code]
    effect.target_id = ctx.actor_id
    effect.source_record_slot = ctx.source_record_slot
    effect.scene_id = ctx.scene_id
    effect.ai_target_candidates = {}
    effect.caster_combatant = {}
    effect.countdown = R(0, 2)
    effect.phase_count = R(1, 4)
    st.spell_effect_pending = effect
    st.spell_effect_id = nil
    st.spell_effect_action_code = 0
    ctx.phase = "render"
    ctx.scene_id = R(0, 5)
    ctx.movement_scene_id = ctx.scene_id
    ctx.source_record_slot = R(0, 7)
    effect.source_record_slot = ctx.source_record_slot
    effect.scene_id = ctx.scene_id
    ctx.ai_target_candidates = {}
  end,
  transfer_pending_spell_effect = function(st, ctx) st.pending_spell_effect = pending() end,
  advance_world_target_selection = function(st, ctx)
    ctx.phase = "behavior"
    ctx.target_selection_requested = true
    ctx.target_selection_special_marker = false
    ctx.target_selection_policy = nil
    ctx.target_selection_current_slot = nil
    ctx.target_selection_linked_slot = nil
    ctx.world_target_selection_candidates = {}
    local count = R(1, 3)
    for i = 1, count do
      ctx.world_target_selection_candidates[i] =
        target_candidate(2 + i, R(3) ~= 1, false)
    end
  end,
}
for _, n in ipairs({ "classify_spell_dispatch", "classify_item_dispatch" }) do SHAPE[n] = SHAPE.begin_spell_cast end
SHAPE.classify_item_dispatch = SHAPE.begin_dynamic_item_action
-- functions whose first parameter is not the state: return replacement leading args
local ARGS = {
  probe_movement = function(st, ctx)
    local directions = { "north", "north_east", "east", "south_east",
      "south", "south_west", "west", "north_west" }
    return { ctx, H.mstate(), pick(directions), R(0, 40) }
  end,
  initial_state = function(st, ctx)
    ctx.action_kind = "none"; ctx.action_code = 0
    ctx.action_item_id = nil; ctx.action_spell_id = nil
    ctx.actor_facts = actor_facts(true, false)
    ctx.source_record_flags = 1
    local profile = {
      package_id = "equiv_units.neutral_actor",
      visual_profile_id = ctx.visual_profile_id,
      controls_movement = false,
      projectile_animation = false,
      hit_spark_animation = false,
      animation_cadence_profile = "moving_actor_walk",
    }
    return { ctx, profile }
  end,
  source_motion_bounds = function(st, ctx)
    local world_actor = R(2) == 1
    ctx.action_kind = "none"; ctx.action_code = 0
    ctx.action_item_id = nil; ctx.action_spell_id = nil
    ctx.source_record_flags = world_actor and 0x08 or 0
    ctx.actor_facts = actor_facts(true, world_actor)
    ctx.source_actor_posture = R(0, 8)
    local posture = ctx.source_actor_posture
    ctx.actor_behavior_profile = {
      approach_band = posture < 3 and "far" or
        posture < 6 and "medium" or "close",
    }
    st.target_motion_delay = R(0, 6)
    return { ctx, st }
  end,
  projectile_movement_snapshot = function(st, ctx)
    return { ctx, st }
  end,
  begin_spell_cast = function(st, ctx)
    local target = H.cand(1)
    target.bearing_degrees = 22.5
    target.source_record_slot = R(0, 7)
    local resolution = {
      effect_started = false,
      successes = 0,
      action_result = 0,
    }
    return { st, ctx, target, resolution }
  end,
  classify_item_dispatch = function(st, ctx)
    local target = H.cand(1)
    target.bearing_degrees = 22.5
    target.facing_degrees = 22.5
    target.source_record_heading = 0x1000
    return { st, ctx, target }
  end,
}
local ALWAYS_SHAPED = {
  initial_state = true,
  source_motion_bounds = true,
  projectile_movement_snapshot = true,
  projectile_heartbeat = true,
  begin_spell_cast = true,
  classify_item_dispatch = true,
  advance_spell_effect = true,
  advance_world_target_selection = true,
  snapshot = true,
}
math.randomseed(SEED)
local targets = {}
-- Functions whose behavior was deliberately changed to match the ROM after the split; the
-- pre-split reference is wrong for them by design. See reference/INTENDED_DIVERGENCES.md.
local INTENDED = {
  advance_target_marker = "ROM 0x5C7E tiers the target's meter (a5a74ea3)",
  request_target_motion_effect = "Hell Hound 0xB11C resolves 0x55B16 and emits the 0xB146 child request on changed defender meters",
  advance_player_animation = "runner idle and walk now store normalized animation progress; animator_tests checks the source frame cadence",
  advance_dynamic_actor_walk_animation = "dynamic actor locomotion now stores normalized animation progress; animator_tests checks the source frame cadence",
  advance_projectile_animation = "weapon-effect lists now use explicit per-frame durations and a semantic completion flag; animator_tests and runtime QA check both list lengths",
  begin_dynamic_target_action = "0xC0A2 pose, rate, selector, and cue now come from generated ROM animation data",
  begin_dynamic_item_action = "0xC042 pose and rate now come from generated ROM animation data",
  begin_received_hit_reaction = "0xC072/C0B2 pose data and B03E/B05E cues now come from generated ROM animation data",
  source_action_delay = "0xABFC now signed-halves every nonzero D0 other than 2 [C]",
  think = "merged R.think API replaces the former advance_think/apply_think_path pair",
  probe_movement = "0xB77E collision encodings are converted to named native probe facts before Lua movement policy",
  movement_probe_is_blocked = "route fallback consumes the native semantic blocking fact; ROM 0xB732 encodings stay host-side",
  advance_entity_movement = "movement consumes named native collision and route facts; focused runtime QA covers direct, recovery, fallback, scene, mode, and transition branches",
  commit_movement_candidate = "candidate collision state uses named terrain and route facts instead of source record bytes",
  finish_entity_movement = "terrain-clear state is translated by the native collision adapter",
  reset_blocked_movement = "the movement result uses typed route facts and named collision outcomes",
}
-- Fields added by actor/think.lua (0x13904 gate outputs). Stripped before comparing so the
-- rest of the result/state must still match the pre-split reference exactly.
local THINK_FIELDS = { "combat_engagement_started", "field_event_request", "source_spawn_state_flags",
  "think_token_claimed", "source_thon_period_counter",
  "thon_action_cooldown_remaining",
  "ai_think_path", "ai_think_gap", "ai_deactivate_requested",
  "ai_route_target_x", "ai_route_target_y", "ai_route_target",
  "ai_waypoint_index" }
local function strip_think(v, seen)
  if type(v) ~= "table" then return v end
  seen = seen or {}; if seen[v] then return v end; seen[v] = true
  for _, k in ipairs(THINK_FIELDS) do v[k] = nil end
  for _, x in pairs(v) do strip_think(x, seen) end
  return v
end
for name, f in pairs(REG) do if type(f) == "function" and origf[name] and not INTENDED[name] then targets[#targets + 1] = name end end
table.sort(targets)
local total, mism, perfn = 0, 0, {}
for _, name in ipairs(targets) do
  local fa, fb = origf[name], REG[name]
  local bad = 0
  for t = 1, TRIALS do
    local ctx = H.context(pick({ "field_player_runner", "a1", "a2" }))
    local st = convert_bearings(random_state(ctx), true)
    local canonical_animation = name == "advance_dynamic_actor_action_animation" or
      name == "advance_dynamic_actor_item_action_animation" or
      name == "advance_dynamic_actor_walk_animation"
    if SHAPE[name] and
       (canonical_animation or t % 4 ~= 0 or ALWAYS_SHAPED[name]) then
      SHAPE[name](st, ctx)
    end
    local extra = { pick({ H.cand(1), H.num(), nil, 1, 2 }), pick({ { action_result = H.num(), effect_started = H.bool(), spell_effect_slot = R(0, 7), combatant = H.bool() }, H.num(), nil, 0x1000 }), pick({ H.num(), nil, 0x1000 }) }
    local othersA, othersB = {}, {}
    for i = 1, 2 do
      local id = "a" .. i
      othersA[id] = random_state(ctx)
      othersB[id] = convert_bearings(deepcopy(othersA[id]), true)
    end
    for k in pairs(gameA.actors) do gameA.actors[k] = nil; gameB.actors[k] = nil end
    for k, v in pairs(othersA) do
      gameA.actors[k] = deepcopy(v)
      gameB.actors[k] = deepcopy(othersB[k])
    end
    local sB, cB = deepcopy(st), deepcopy(ctx)
    cB.ai_random_bound = ctx.ai_random_bound
    local xB = deepcopy(extra)
    for i = #logA, 1, -1 do logA[i] = nil end; for i = #logB, 1, -1 do logB[i] = nil end
    local argsA, argsB
    if ARGS[name] and (t % 4 ~= 0 or ALWAYS_SHAPED[name]) then
      math.randomseed(SEED * 100003 + t); local b1 = ARGS[name](sB, cB)
      argsB = b1
    else
      argsB = { sB, cB, xB[1], xB[2], xB[3] }
    end
    local sA = convert_bearings(deepcopy(sB), false)
    local cA = reference_context(cB)
    local xA = convert_bearings(deepcopy(xB), false)
    if ARGS[name] and (t % 4 ~= 0 or ALWAYS_SHAPED[name]) then
      argsA = {}
      for i, value in ipairs(argsB) do
        if value == sB then
          argsA[i] = sA
        elseif value == cB then
          argsA[i] = cA
        elseif value == xB then
          argsA[i] = xA
        else
          argsA[i] = convert_bearings(deepcopy(value), false)
        end
      end
    else
      argsA = { sA, cA, xA[1], xA[2], xA[3] }
    end
    prepare_reference_projectile(cA.projectile_movement)
    _G.game = gameA; local ra = table.pack(pcall(fa, table.unpack(argsA, 1, 5)))
    _G.game = gameB; local rb = table.pack(pcall(fb, table.unpack(argsB, 1, 5)))
    normalize_reference_projectile(ra)
    normalize_reference_projectile(cA)
    convert_bearings(ra, true)
    convert_bearings(sA, true)
    semantic_reference_context(cA)
    convert_bearings(gameA.actors, true)
    convert_bearings(xA, true)
    if name == "source_motion_bounds" and type(ra[2]) == "table" then
      ra[2] = {
        minimum_distance = ra[2].lower,
        maximum_distance = ra[2].upper,
      }
    end
    if name == "begin_player_melee" then
      -- ROM 0xB29E..0xB2EA adds this player-only child request after a changed
      -- 0x55C76 meter result. The frozen bfa3908 helper predates that source
      -- fact; player_hit_spark_tests.lua asserts the new field directly. Keep
      -- the differential comparison for every preexisting state field.
      sA.hit_spark_spawn_request = nil
      sB.hit_spark_spawn_request = nil
    end
    local function normalize_clip_clock(value, seen_clock)
      if type(value) ~= "table" then return end
      seen_clock = seen_clock or {}
      if seen_clock[value] then return end
      seen_clock[value] = true
      if type(value.animation_id) == "string" then
        value.animation_selector = nil
        value.source_animation_selector = nil
      end
      local semantic_finite_clip =
        value.animation_id == "action" or value.animation_id == "weapon" or
        value.animation_id == "spell" or value.animation_id == "hurt" or value.animation_id == "down" or
        value.animation_id == "entry" or value.animation_id == "hit_spark" or
        value.animation_id == "projectile"
      if semantic_finite_clip then
        -- The port stores elapsed animation updates; the frozen reference
        -- stores the source fixed-point accumulator. Compare visible clip,
        -- frame, selector, and completion state while ignoring that encoding.
        value.animation_elapsed_updates = nil
        value.animation_accumulator = nil
        value.animation_increment = nil
        value.animation_period = nil
        value.animation_frame_count = nil
        value.animation_progress = nil
        value.hit_spark_animation_complete = nil
      end
      for _, child in pairs(value) do normalize_clip_clock(child, seen_clock) end
    end
    local function normalize_projectile_precision(value, seen_projectile)
      if type(value) ~= "table" then return end
      seen_projectile = seen_projectile or {}
      if seen_projectile[value] then return end
      seen_projectile[value] = true
      if (value.role == "primary" or value.role == "secondary") and
         type(value.bearing_degrees) == "number" then
        value.bearing_degrees = value.bearing_degrees + 0.0
        if type(value.velocity) == "table" then
          for _, axis in ipairs({ "x", "y" }) do
            if type(value.velocity[axis]) == "number" then
              value.velocity[axis] = math.floor(
                value.velocity[axis] * 0x100) / 0x100
            end
          end
        end
        if type(value.position) == "table" then
          for _, axis in ipairs({ "x", "y" }) do
            if type(value.position[axis]) == "number" then
              value.position[axis] = math.floor(
                value.position[axis] * 0x10000 + 0.5) / 0x10000
            end
          end
        end
      end
      for _, child in pairs(value) do
        normalize_projectile_precision(child, seen_projectile)
      end
    end
    local function normalize_spell_pending(pending_value)
      if type(pending_value) ~= "table" then return end
      if type(pending_value.action_code) == "number" then
        pending_value.spell_id = pending_value.spell_id or
          SpellIdByActionCode[pending_value.action_code]
        pending_value.action_code = nil
      end
      if pending_value.action_result ~= nil and
         pending_value.successes ~= nil then
        pending_value.action_result = nil
      end
    end
    local function normalize_spell_effect_state(value)
      if type(value) ~= "table" then return end
      if type(value.spell_effect_action_code) == "number" then
        if value.spell_effect_id == nil then
          value.spell_effect_id = SpellIdByActionCode[
            value.spell_effect_action_code]
        end
        value.spell_effect_action_code = nil
      end
      normalize_spell_pending(value.spell_effect_pending)
    end
    local INITIAL_STATE_FIELDS = {
      "actor_id", "profile_id", "action_kind", "package_id",
      "weapon_item_id", "direction", "entry_delay", "behavior_timer",
      "animation_id", "animation_frame", "action_animation_complete",
      "item_action_complete", "damage_reaction_active",
      "damage_reaction_animation_complete", "hurt_behavior_timer",
      "hurt_audio_event", "hurt_audio_emitted",
      "movement_current_bearing_degrees", "movement_route_distance",
      "movement_active", "transition_started", "transition_complete",
      "combat_action_requested", "ai_target_id", "ai_target_bearing_degrees",
      "ai_target_distance", "ai_target_engaged", "target_motion_delay",
      "target_selection_latched", "target_marker_visible",
      "target_marker_animating", "target_marker_style", "target_marker_radius",
      "action_delay", "spell_action_resolution", "spell_effect_completed",
      "spell_effect_slot_released", "spell_effect_resolution",
      "spell_effect_visible", "spell_effect_phase", "pending_source_action",
      "target_motion_effect_active", "target_motion_effect_slot",
      "target_motion_effect_slot_request", "target_motion_effect_slot_released",
      "target_motion_effect_countdown", "target_motion_effect_phase",
      "target_motion_effect_direction", "target_motion_effect_visible",
      "selected_world_actor_id", "target_selection_processed",
    }
    local function canonical_initial_state(result)
      if type(result) ~= "table" then return result end
      local out = {}
      for _, key in ipairs(INITIAL_STATE_FIELDS) do
        local value = result[key]
        out[key] = { present = value ~= nil, value = value }
      end
      local successes = result.last_action_successes or result.action_result
      out.action_successes = { present = successes ~= nil, value = successes }
      return out
    end
    local SNAPSHOT_FIELDS = {
      "actor_id", "profile_id", "package_id", "action_kind",
      "action", "animation", "direction", "frame", "transition_started",
      "transition_complete", "combat_action_requested", "audio_event",
      "target_heading_audio_event", "weapon_item_id",
      "movement_bearing_degrees", "movement_current_bearing_degrees",
      "movement_route_distance", "ai_target_id", "ai_target_bearing_degrees",
      "ai_target_distance", "ai_target_engaged", "target_motion_delay",
      "action_delay", "target_motion_effect_slot_request",
      "spell_effect_slot_released", "spell_effect_completed",
      "spell_effect_visible", "spell_effect_phase", "spell_effect_id",
      "target_motion_effect_visible", "target_motion_effect_direction",
      "target_motion_effect_phase", "target_motion_effect_slot_released",
      "target_marker_visible", "target_marker_animating", "target_marker_style",
      "target_marker_radius", "target_selection_processed",
      "selected_world_actor_id", "combat_meter_update",
      "spell_action_resolution", "spell_effect_resolution",
      "source_action_dispatch", "movement_step", "projectile_movement",
    }
    local function canonical_snapshot(result, legacy)
      if type(result) ~= "table" then return result end
      local out = {}
      for _, key in ipairs(SNAPSHOT_FIELDS) do
        local value
        if key == "spell_effect_id" and legacy then
          value = SpellIdByActionCode[result.spell_effect_action_code]
        else
          value = result[key]
        end
        out[key] = { present = value ~= nil, value = value }
      end
      return out
    end
    local function sig(r, s, c, g, log, x)
      if name == "initial_state" and r[1] and type(r[2]) == "table" then
        r[2] = canonical_initial_state(r[2])
      elseif (name == "snapshot" or name == "projectile_heartbeat") and
             r[1] and type(r[2]) == "table" then
        r[2] = canonical_snapshot(r[2], r == ra)
      end
      normalize_clip_clock(s)
      normalize_clip_clock(c)
      normalize_clip_clock(g.actors)
      if name == "projectile_heartbeat" then
        -- The old record exposed the finite-list selector; the actor module
        -- exposes completion as a semantic flag. The movement snapshot below
        -- carries the behavior-relevant stop/restart result in both versions.
        s.animation_selector = nil
        s.animation_complete = nil
        s.projectile_animation_started = nil
        if r == ra then
          s.last_action_successes = s.action_result
        end
        s.action_result = nil
      end
      normalize_projectile_precision(r)
      normalize_projectile_precision(s)
      normalize_projectile_precision(c)
      normalize_projectile_precision(g.actors)
      normalize_projectile_precision(x)
      if name == "advance_spell_effect" or name == "begin_spell_cast" then
        normalize_spell_effect_state(s)
        normalize_spell_effect_state(g.actors)
      end
      strip_think(r); strip_think(s); strip_think(g.actors)
      local service_log = table.concat(log, ";")
      if name == "projectile_movement_snapshot" or
         name == "projectile_heartbeat" then
        -- The legacy collision mock included the source actor flags as its
        -- third argument; the typed service now receives only map position.
        service_log = service_log:gsub(
          "(movement%.projectile_collision{[^}]-),3=[^,}]+", "%1")
      end
      return table.concat({ tostring(r[1]), r[1] and ser({ table.unpack(r, 2, r.n) }) or H.norm_err(r[2]), ser(s), ser(c), ser(g.actors), ser(x), service_log }, "|")
    end
    total = total + 1
    local signature_a = sig(ra, sA, cA, gameA, logA, xA)
    local signature_b = sig(rb, sB, cB, gameB, logB, xB)
    if signature_a ~= signature_b then
      bad = bad + 1; mism = mism + 1
      if bad == 1 then
        local first = 1
        while first <= #signature_a and signature_a:byte(first) == signature_b:byte(first) do
          first = first + 1
        end
        io.stderr:write(("MISMATCH %s at %d\n A=%s\n B=%s\n"):format(
          name, first,
          signature_a:sub(math.max(1, first - 100), first + 180),
          signature_b:sub(math.max(1, first - 100), first + 180)))
      end
    end
  end
  perfn[#perfn + 1] = name .. (bad > 0 and ("!" .. bad) or "")
end
print(("functions=%d calls=%d mismatches=%d"):format(#targets, total, mism))
if mism > 0 then print(table.concat(perfn, " ")) end
if not COVERAGE then os.exit(mism == 0 and 0 or 1) end
