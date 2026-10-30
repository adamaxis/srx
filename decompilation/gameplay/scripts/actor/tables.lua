-- actor/tables.lua: shared actor data and movement tuning (no functions).
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local ActorProfileCatalog = require("data.actor_profiles")
local u8, u16, s16 = I.u8, I.u16, I.s16
local SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning

local projectile = {
    role = { primary = "primary", secondary = "secondary" },
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

-- Semantic names for the source-template facts consumed by actor packages.
-- Numeric values remain here as data; package definitions use game concepts.
local ActionKind = {
    none = "none",
    item = "item",
    spell = "spell",
}

local E = require("enums")
local VisualProfile = E.ActorVisualProfile
-- Named approach bands from the world-actor route table. ROM 0x13D84
-- selects these intervals through the host-derived band; Lua does not index
-- the source's nine-entry table by a record value. Distances are world units.
local ActorApproachDistanceBounds = {
    far = { minimum_distance = 1600, maximum_distance = 1920 },
    medium = { minimum_distance = 1120, maximum_distance = 1440 },
    close = { minimum_distance = 480, maximum_distance = 960 },
}
local target_motion_reacquire_updates = 6
local source_close_range_threshold = 0x01e0
local profiles = {
    ["corp_guard_k05_item06"] = {
        package_id = "corp_security.max_power_heavy_pistol",
        visual_profile_id = VisualProfile.HUMAN_GUARD,
        action_kind = ActionKind.item,
    },
    ["pedestrian_k0b_none00"] = {
        package_id = "pedestrian.no_active_action",
        visual_profile_id = VisualProfile.PEDESTRIAN_RAT_SHAMAN,
        action_kind = ActionKind.none,
    },
    ["field_runner_k03_none00"] = {
        package_id = "runner.field_controlled",
        visual_profile_id = VisualProfile.FIELD_RUNNER,
        action_kind = ActionKind.none,
        controls_movement = true,
    },
    ["hell_hound_k0c_none00"] = {
        package_id = "creature.hell_hound.source_target_motion",
        visual_profile_id = VisualProfile.HELL_HOUND,
        action_kind = ActionKind.none,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["gargoyle_k0d_none00"] = {
        package_id = "creature.gargoyle.source_target_motion",
        visual_profile_id = VisualProfile.GARGOYLE,
        action_kind = ActionKind.none,
        controls_movement = true,
        animation_cadence_profile = "gargoyle_walk",
        dynamic_encounter_only = true,
    },
    ["wendigo_k0e_none00"] = {
        package_id = "creature.wendigo.source_target_motion",
        visual_profile_id = VisualProfile.WENDIGO,
        action_kind = ActionKind.none,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["weapon_projectile"] = {
        package_id = "combat.weapon_projectile",
        action_kind = ActionKind.none,
        projectile_animation = true,
    },
    ["field_hit_spark"] = {
        package_id = "combat.player_hit_spark",
        action_kind = ActionKind.none,
        hit_spark_animation = true,
    },
}

-- The extracted profile records supply ROM identity and equipment facts;
-- this table retains only policy structure that is unique to the host AI.
for profile_id, profile in pairs(profiles) do
    local source_profile = ActorProfileCatalog.by_id[profile_id]
    assert(source_profile ~= nil,
           "actor profile has no source record: " .. profile_id)
    assert(source_profile.package_id == profile.package_id and
               source_profile.action_kind == profile.action_kind,
           "actor profile differs from source record: " .. profile_id)
    profile.source_profile = source_profile
    profile.visual_profile_id = source_profile.visual_profile_id
    profile.weapon_item_id = source_profile.weapon
    profile.controls_movement = source_profile.controls_movement
    profile.projectile_animation = source_profile.projectile
    profile.hit_spark_animation = profile_id == "field_hit_spark"
    profile.dynamic_encounter_only = source_profile.dynamic_encounter_only
end


-- register this module's definitions
R.projectile = projectile
R.ActionKind = ActionKind
R.VisualProfile = VisualProfile
R.ActorApproachDistanceBounds = ActorApproachDistanceBounds
R.target_motion_reacquire_updates = target_motion_reacquire_updates
R.source_close_range_threshold = source_close_range_threshold
R.profiles = profiles
return R
