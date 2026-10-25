-- actor/tables.lua: data: ROM-derived constants, flag tables, actor profiles (no functions).
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local ActorProfileCatalog = require("data.actor_profiles")
local u8, u16, s16 = I.u8, I.u16, I.s16
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning

-- The directional weapon-effect records at ROM 0x630C use this descriptor
-- cadence in their finite 0x8003/0x800B lists.
local weapon_projectile_descriptor_increment = 0x270f
local projectile = {
    role = { primary = "primary", secondary = "secondary" },
    flags = {
        active_record = 0x01,
        stopped_record = 0x08,
        terminal_record = 0x09,
        animation_list_finished = 0x8000,
    },
    outcome = {
        none = "none",
        stopped = "stopped",
        retired = "retired",
        hit_resolution_pending = "hit_resolution_pending",
        linked_followup_pending = "linked_followup_pending",
        burst_followup_restarted = "burst_followup_restarted",
    },
    movement_probes_per_update = 4,
    impact_sound_event = E.SoundId.PROJECTILE_IMPACT,   -- 0x3C
}
local byte_mask = 0xff
local signed_byte_threshold = 0x80
local byte_modulus = 0x100
local fixed_long_mask = 0xffffffff
local fixed_integer_shift = 16
local fixed_fraction_scale = 0x100

-- Semantic names for the source-template facts consumed by actor packages.
-- Numeric values remain here as data; package definitions use game concepts.
local ActionKind = {
    none = "none",
    item = "item",
    spell = "spell",
}

local E = require("enums")
-- Flag tables now come from the shared enums (names corrected per docs/20_actors_ai.md; lua_review.md #1-#2):
--   ActorServiceFlag.{active,targets_world_actors,action_candidate_reserved,effect_locked} -> ActorStatus.{ACTIVE,AI_RUNNER,DOWN,DEAD} (+0x10)
--   ActorSpawnFlag.{transitioning,effect_route} -> ActorFlag.{PEDESTRIAN,QUEST} (+0x9E)
--   ActorBehaviorFlag.{action_window_open,high_effect} -> BehaviorFlag.{IN_BAND,ESCORT} (+0xB0)
--   EntityKind.* -> SpriteType.* (+0x0E)
local ActorStatus, ActorFlag, BehaviorFlag, SpriteType = E.ActorStatus, E.ActorFlag, E.BehaviorFlag, E.SpriteType
-- ROM 0x13E7C has four finite bounds pairs selected by item/spell template
-- byte +0x27. Coordinates are typed 16-bit scene units.
local ActionMotionDistanceBounds = {
    { lower = 0x0500, upper = 0x0640 },
    { lower = 0x05a0, upper = 0x0780 },
    { lower = 0x0640, upper = 0x08c0 },
    { lower = 0x08c0, upper = 0x0c80 },
}
local EntityMotionDistanceBounds = {
    { lower = 0x0640, upper = 0x0780 },
    { lower = 0x0640, upper = 0x0780 },
    { lower = 0x0640, upper = 0x0780 },
    { lower = 0x0460, upper = 0x05a0 },
    { lower = 0x0460, upper = 0x05a0 },
    { lower = 0x0460, upper = 0x05a0 },
    { lower = 0x01e0, upper = 0x03c0 },
    { lower = 0x01e0, upper = 0x03c0 },
    { lower = 0x01e0, upper = 0x03c0 },
}
local target_motion_reacquire_updates = 6
local source_close_range_threshold = 0x01e0
local item_action_pose_selectors = {
    0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C,
}
local idle_pose_selectors = {
    0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C,
}
local walk_pose_selectors = {
    0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C,
}
local ItemAction = {
    max_power_heavy_pistol = 6,
}
local profiles = {
    ["corp_guard_k05_item06"] = {
        package_id = "corp_security.max_power_heavy_pistol",
        entity_kind = SpriteType.CORP_GUARD,
        action_kind = ActionKind.item,
        action_code = ItemAction.max_power_heavy_pistol,
        weapon_item_id = ItemAction.max_power_heavy_pistol,
    },
    ["pedestrian_k0b_none00"] = {
        package_id = "pedestrian.no_active_action",
        entity_kind = SpriteType.PEDESTRIAN,
        action_kind = ActionKind.none,
        action_code = 0,
    },
    ["field_runner_k03_none00"] = {
        package_id = "runner.field_controlled",
        entity_kind = SpriteType.FIELD_RUNNER,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
    },
    ["hell_hound_k0c_none00"] = {
        package_id = "creature.hell_hound.source_target_motion",
        entity_kind = SpriteType.HELL_HOUND,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["gargoyle_k0d_none00"] = {
        package_id = "creature.gargoyle.source_target_motion",
        entity_kind = SpriteType.GARGOYLE,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["wendigo_k0e_none00"] = {
        package_id = "creature.wendigo.source_target_motion",
        entity_kind = SpriteType.WENDIGO,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["weapon_projectile"] = {
        package_id = "combat.weapon_projectile",
        action_kind = ActionKind.none,
        action_code = 0,
        projectile_animation = true,
    },
}

-- The extracted profile records supply ROM identity and equipment facts;
-- this table retains only policy structure that is unique to the host AI.
for profile_id, profile in pairs(profiles) do
    local source_profile = ActorProfileCatalog.by_id[profile_id]
    assert(source_profile ~= nil,
           "actor profile has no source record: " .. profile_id)
    assert(source_profile.package_id == profile.package_id and
               source_profile.action_kind == profile.action_kind and
               source_profile.action_code == profile.action_code,
           "actor profile differs from source record: " .. profile_id)
    profile.source_profile = source_profile
    profile.entity_kind = source_profile.sprite_type
    profile.weapon_item_id = source_profile.weapon
    profile.controls_movement = source_profile.controls_movement
    profile.projectile_animation = source_profile.projectile
    profile.dynamic_encounter_only = source_profile.dynamic_encounter_only
end


-- register this module's definitions
R.weapon_projectile_descriptor_increment = weapon_projectile_descriptor_increment
R.projectile = projectile
R.byte_mask = byte_mask
R.signed_byte_threshold = signed_byte_threshold
R.byte_modulus = byte_modulus
R.fixed_long_mask = fixed_long_mask
R.fixed_integer_shift = fixed_integer_shift
R.fixed_fraction_scale = fixed_fraction_scale
R.ActionKind = ActionKind
R.ActorStatus = ActorStatus
R.ActorFlag = ActorFlag
R.BehaviorFlag = BehaviorFlag
R.SpriteType = SpriteType
R.ActionMotionDistanceBounds = ActionMotionDistanceBounds
R.EntityMotionDistanceBounds = EntityMotionDistanceBounds
R.target_motion_reacquire_updates = target_motion_reacquire_updates
R.source_close_range_threshold = source_close_range_threshold
R.item_action_pose_selectors = item_action_pose_selectors
R.idle_pose_selectors = idle_pose_selectors
R.walk_pose_selectors = walk_pose_selectors
R.ItemAction = ItemAction
R.profiles = profiles
return R
