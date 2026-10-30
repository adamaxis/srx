package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local Alarm = require("field_alarm")
local Encounters = require("field_encounters")

local checks = 0
local function eq(actual, expected, label)
    assert(actual == expected,
        ("%s: expected %s, got %s"):format(label, tostring(expected),
                                             tostring(actual)))
    checks = checks + 1
end

local function facts(overrides)
    local value = {
        alarm_check_due = true,
        building_scene = true,
        alarm_timeout_scene = false,
        player_tile_x = 33,
        player_tile_y = 385,
        security_disabled = false,
        alarm_active = false,
        alarm_time_remaining_frames = 0,
        probability_sample = nil,
        alarm_regions = {
            -- Source rectangle at 0x1D7498; lower edges exclusive, upper
            -- edges inclusive [C]. Coordinates here are synthetic probes [H].
            { min_tile_x = 32, max_tile_x = 127,
              min_tile_y = 384, max_tile_y = 479 },
        },
        actors = {
            { actor_facts = { roles = {
                spotted = true, reinforcement = true,
                despawn_offscreen = false, quest = false, runner = false,
                pedestrian = false, hostile = false,
            } } },
            { actor_facts = { roles = {
                spotted = true, reinforcement = false,
                despawn_offscreen = false, quest = false, runner = false,
                pedestrian = false, hostile = false,
            } } },
        },
        reinforcement_activity_id = "field_alarm_scene_21",
    }
    for key, item in pairs(overrides or {}) do value[key] = item end
    return value
end

-- The native field clock supplies the source 13-in-64-frame opportunity [C].
local not_due = Alarm.tick(facts({ alarm_check_due = false }))
eq(not_due.request_probability_sample, nil,
   "nonmatching field pass does not request a random sample")
eq(not_due.alarm_active, false,
   "nonmatching field pass leaves alarm clear")
local due = Alarm.tick(facts())
eq(due.request_probability_sample, true,
   "eligible field pass requests a probability sample")

-- The source accepts values below one half; equality at one half is rejected [C].
local random_miss = Alarm.tick(facts({ probability_sample = 0.5 }))
eq(random_miss.alarm_active, false,
   "sample at the half-chance boundary is rejected")
local random_hit = Alarm.tick(facts({
    probability_sample = 0.5 - 1 / 65536,
}))
eq(random_hit.alarm_active, true,
   "sample below the half-chance boundary reaches the rectangle test")

-- Region record 0x1D7498 uses x/y lower-exclusive and upper-inclusive bounds [C].
eq(Alarm.tick(facts({ player_tile_x = 32,
                      probability_sample = 0 })).alarm_active,
   false, "exclusive lower x edge does not match")
eq(Alarm.tick(facts({ player_tile_y = 384,
                      probability_sample = 0 })).alarm_active,
   false, "exclusive lower y edge does not match")
eq(Alarm.tick(facts({ player_tile_x = 127, player_tile_y = 479,
                      probability_sample = 0 })).alarm_active,
   true, "inclusive upper x/y edges match")
eq(Alarm.tick(facts({ player_tile_x = 128,
                      probability_sample = 0 })).alarm_active,
   false, "coordinate outside upper x edge does not match")

-- A matched non-alarm-combat building with a clear latch starts the alarm [C].
local raised = Alarm.tick(facts({ probability_sample = 0 }))
eq(raised.alarm_time_remaining_frames, E.AiTuning.ALARM_FRAMES,
   "fresh alarm loads its duration")
eq(raised.sound_start, E.SoundId.ALARM, "fresh alarm starts sound 0x30")
eq(raised.clear_flags, "clear_spotted", "fresh alarm clears spotted flags")
eq(raised.actors[1].actor_facts.roles.spotted, false,
   "fresh alarm clears the spotted role")
eq(raised.actors[1].actor_facts.roles.reinforcement, true,
   "fresh alarm preserves the reinforcement role")
eq(raised.spawn_reinforcements, true,
   "fresh alarm requests the building actor list")
eq(raised.reinforcement_activity_id, "field_alarm_scene_21",
   "spawn result carries the semantic reinforcement activity identity")
eq(raised.reinforcement_list_address, nil,
   "gameplay policy does not return the ROM list address")

-- Timer refresh precedes the already-alarmed latch check at 0x1E04A [C].
local refreshed = Alarm.tick(facts({
    alarm_active = true, alarm_time_remaining_frames = 1,
    probability_sample = 0,
}))
eq(refreshed.alarm_time_remaining_frames, E.AiTuning.ALARM_FRAMES,
   "matched region refreshes an active countdown")
eq(refreshed.sound_start, nil, "active alarm does not replay its sound")
eq(refreshed.spawn_reinforcements, nil,
   "active alarm does not respawn reinforcements")

-- Countdown expiration uses the same clear branch and promotes +0x9E bit one [C].
local expired = Alarm.tick(facts({
    alarm_active = true,
    alarm_time_remaining_frames = 0,
    security_disabled = true,
    alarm_check_due = false,
    actors = {
        { actor_facts = { roles = {
            spotted = false, reinforcement = true,
            despawn_offscreen = false, quest = false, runner = false,
            pedestrian = false, hostile = false,
        } } },
        { actor_facts = { roles = {
            spotted = true, reinforcement = false,
            despawn_offscreen = false, quest = false, runner = false,
            pedestrian = false, hostile = false,
        } } },
    },
}))
eq(expired.alarm_active, false, "negative duration clears the alarm")
eq(expired.sound_stop, E.SoundId.ALARM, "alarm clear stops sound 0x30")
eq(expired.clear_flags, "promote_reinforcement",
   "alarm clear promotes reinforcement flags")
eq(expired.actors[1].actor_facts.roles.despawn_offscreen, true,
   "clear marks reinforcements for offscreen despawn")
eq(expired.actors[2].actor_facts.roles.spotted, true,
   "clear preserves unrelated spotted roles")
eq(expired.actors[2].actor_facts.roles.despawn_offscreen, false,
   "clear leaves non-reinforcements eligible for ordinary despawn policy")

local countdown_step = Alarm.tick(facts({
    alarm_active = true, alarm_time_remaining_frames = 1,
    alarm_check_due = false,
}))
eq(countdown_step.alarm_time_remaining_frames, 0,
   "positive duration decrements without clearing")
eq(countdown_step.alarm_active, true,
   "zero duration remains active for this tick")
local countdown_end = Alarm.tick(facts({
    alarm_active = true,
    alarm_time_remaining_frames = countdown_step.alarm_time_remaining_frames,
    alarm_check_due = false,
}))
eq(countdown_end.alarm_active, false, "next negative duration tick clears")

local explicit_clear = Alarm.tick(facts({
    alarm_active = true, alarm_time_remaining_frames = 77, clear_alarm = true,
}))
eq(explicit_clear.alarm_active, false, "security success uses shared clear")
eq(explicit_clear.security_disabled, false,
   "ordinary security success does not set the disable state")
eq(explicit_clear.sound_stop, E.SoundId.ALARM,
   "ordinary security success stops the alarm sound")

-- Scene exit clears both states and retires active alarm responders [C].
local scene_exit = Alarm.tick(facts({
    alarm_active = true,
    security_disabled = true,
    alarm_time_remaining_frames = E.AiTuning.ALARM_FRAMES,
    scene_exit = true,
    actors = {
        { actor_facts = { roles = {
            spotted = false, reinforcement = true,
            despawn_offscreen = false, quest = false, runner = false,
            pedestrian = false, hostile = false,
        } } },
        { actor_facts = { roles = {
            spotted = true, reinforcement = false,
            despawn_offscreen = false, quest = false, runner = false,
            pedestrian = false, hostile = false,
        } } },
    },
}))
eq(scene_exit.alarm_active, false, "scene entry clears the alarm state")
eq(scene_exit.security_disabled, false,
   "scene entry clears the security-disable state")
eq(scene_exit.sound_stop, E.SoundId.ALARM,
   "scene exit stops the alarm sound resource")
eq(scene_exit.actors[1].actor_facts.roles.despawn_offscreen, true,
   "scene exit marks alarm responders for despawn")
eq(scene_exit.actors[2].actor_facts.roles.spotted, true,
   "scene exit preserves unrelated actor roles")
local quiet_exit = Alarm.tick(facts({ scene_exit = true }))
eq(quiet_exit.sound_stop, nil,
   "scene exit with no active alarm sends no second 0x30 stop")
eq(Encounters.trigger_attempt({
       scene_id = E.Scene.RESOURCE_ACTIVITY_21,
       encounter_check_due = true,
       display_busy = false,
       leader_unseen = false,
       world_activity_locked = false,
       alarm_active = scene_exit.alarm_active,
       combat_active = false,
       encounter_in_progress = false,
       probability_sample = 0,
       encounter_probability = 1,
   }), "select_scene_outcome",
   "scene exit restores the ordinary field event set")

local reset = Alarm.tick(facts({
    alarm_active = true, security_disabled = true, reset = true,
}))
eq(reset.alarm_active, false, "projected-cell reset clears the alarm state")
eq(reset.security_disabled, false,
   "projected-cell reset clears the security-disable state")
eq(reset.sound_stop, nil, "projected-cell reset does not stop sound")
eq(reset.actors[1].actor_facts.roles.spotted, true,
   "projected-cell reset leaves actor roles unchanged")
eq(reset.actors[1].actor_facts.roles.despawn_offscreen, false,
   "projected-cell reset does not promote reinforcements")

local disabled = Alarm.tick(facts({ security_disabled = true }))
eq(disabled.alarm_active, false,
   "disabled security blocks a new alarm")
eq(disabled.request_probability_sample, nil,
   "disabled security prevents the random draw")
local disabled_clear = Alarm.tick(facts({
    alarm_active = true,
    alarm_time_remaining_frames = E.AiTuning.ALARM_FRAMES,
    disable_alarm = true,
}))
eq(disabled_clear.security_disabled, true,
   "terminal disable sets security state")
eq(disabled_clear.alarm_active, false,
   "terminal disable uses shared clear")
eq(disabled_clear.sound_stop, E.SoundId.ALARM,
   "terminal disable returns the shared clear sound stop")

-- Scene 0x14's explicit begin path schedules the special timeout [C].
local alarm_combat = Alarm.tick(facts({
    alarm_timeout_scene = true,
    direct_raise = true,
}))
eq(alarm_combat.schedule_timeout, true,
   "alarm-combat scene selects the timeout path")
eq(alarm_combat.alarm_active, false,
   "alarm-combat timeout path does not set the alarm state")

-- Direct 0x1E04A calls bypass cadence, chance, and region trigger gates [C].
local direct = Alarm.tick(facts({
    alarm_check_due = false,
    direct_raise = true,
}))
eq(direct.alarm_active, true, "direct source event raises the alarm")
eq(direct.sound_start, E.SoundId.ALARM,
   "direct alarm raise requests sound 0x30")

print(("field alarm tests passed (%d checks)"):format(checks))
