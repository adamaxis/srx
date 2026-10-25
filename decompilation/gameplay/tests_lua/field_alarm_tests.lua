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
        scene_id = E.FieldAlarmRomLayout.FIRST_LIST_SCENE,
        frame_counter = E.FieldAlarm.FRAME_VALUE,
        scene_mode = E.SceneMode.BUILDING,
        player_tile_x = 33,
        player_tile_y = 385,
        disabled = false,
        alarm = false,
        timer = 0,
        rng = nil,
        rects = {
            -- Source rectangle at 0x1D7498; lower edges exclusive, upper
            -- edges inclusive [C]. Coordinates here are synthetic probes [H].
            { x_min = 32, x_max = 127, y_min = 384, y_max = 479 },
        },
        actors = {
            { flags9e = E.ActorFlag.SPOTTED | E.ActorFlag.REINFORCEMENT },
            { flags9e = E.ActorFlag.SPOTTED },
        },
        reinforcement_list_address =
            E.FieldAlarmRomLayout.SCENE_21_REINFORCEMENT_LIST,
    }
    for key, item in pairs(overrides or {}) do value[key] = item end
    return value
end

-- 0x1DFC4 gates its random draw to FFDF88 low six bits == 13 [C].
local not_due = Alarm.tick(facts({ frame_counter = E.FieldAlarm.FRAME_VALUE - 1 }))
eq(not_due.sample_rng, nil, "nonmatching frame does not ask for RNG")
eq(not_due.alarm, false, "nonmatching frame leaves alarm clear")
local due = Alarm.tick(facts())
eq(due.sample_rng, true, "matching frame requests the source RNG")

-- The random value must be below 0x8000 before the profile rectangles are read [C].
local random_miss = Alarm.tick(facts({ rng = E.FieldAlarm.RANDOM_REJECT_FROM }))
eq(random_miss.alarm, false, "high unsigned RNG value is rejected")
local random_hit = Alarm.tick(facts({ rng = E.FieldAlarm.RANDOM_REJECT_FROM - 1 }))
eq(random_hit.alarm, true, "low unsigned RNG value reaches rectangle test")

-- Region record 0x1D7498 uses x/y lower-exclusive and upper-inclusive bounds [C].
eq(Alarm.tick(facts({ player_tile_x = 32,
                      rng = E.FieldAlarm.RANDOM_REJECT_FROM - 1 })).alarm,
   false, "exclusive lower x edge does not match")
eq(Alarm.tick(facts({ player_tile_y = 384,
                      rng = E.FieldAlarm.RANDOM_REJECT_FROM - 1 })).alarm,
   false, "exclusive lower y edge does not match")
eq(Alarm.tick(facts({ player_tile_x = 127, player_tile_y = 479,
                      rng = E.FieldAlarm.RANDOM_REJECT_FROM - 1 })).alarm,
   true, "inclusive upper x/y edges match")
eq(Alarm.tick(facts({ player_tile_x = 128,
                      rng = E.FieldAlarm.RANDOM_REJECT_FROM - 1 })).alarm,
   false, "coordinate outside upper x edge does not match")

-- A matched non-alarm-combat building with a clear latch starts the alarm [C].
local raised = Alarm.tick(facts({ rng = 0 }))
eq(raised.timer, E.AiTuning.ALARM_FRAMES,
   "fresh alarm loads source countdown")
eq(raised.sound_start, E.SoundId.ALARM, "fresh alarm starts sound 0x30")
eq(raised.clear_flags, "clear_spotted", "fresh alarm clears spotted flags")
eq(raised.actors[1].flags9e,
   E.ActorFlag.REINFORCEMENT,
   "fresh alarm clears +0x9E bit zero")
eq(raised.spawn_reinforcements, true,
   "fresh alarm requests the building actor list")
eq(raised.reinforcement_flag, E.ActorFlag.REINFORCEMENT,
   "spawn result carries the source reinforcement flag")

-- Timer refresh precedes the already-alarmed latch check at 0x1E04A [C].
local refreshed = Alarm.tick(facts({
    alarm = true, timer = 1, rng = 0,
}))
eq(refreshed.timer, E.AiTuning.ALARM_FRAMES,
   "matched region refreshes an active countdown")
eq(refreshed.sound_start, nil, "active alarm does not replay its sound")
eq(refreshed.spawn_reinforcements, nil,
   "active alarm does not respawn reinforcements")

-- Countdown expiration uses the same clear branch and promotes +0x9E bit one [C].
local expired = Alarm.tick(facts({
    alarm = true,
    timer = 0,
    disabled = true,
    frame_counter = E.FieldAlarm.FRAME_VALUE - 1,
    actors = {
        { flags9e = E.ActorFlag.REINFORCEMENT },
        { flags9e = E.ActorFlag.SPOTTED },
    },
}))
eq(expired.alarm, false, "negative timer clears the alarm")
eq(expired.sound_stop, E.SoundId.ALARM, "alarm clear stops sound 0x30")
eq(expired.clear_flags, "promote_reinforcement",
   "alarm clear promotes reinforcement flags")
eq(expired.actors[1].flags9e,
   E.ActorFlag.REINFORCEMENT | E.ActorFlag.DESPAWN_OFFSCREEN,
   "clear sets +0x9E bit two only for reinforcements")
eq(expired.actors[2].flags9e, E.ActorFlag.SPOTTED,
   "clear preserves unrelated actor flags")

local countdown_step = Alarm.tick(facts({
    alarm = true, timer = 1, frame_counter = E.FieldAlarm.FRAME_VALUE - 1,
}))
eq(countdown_step.timer, 0, "positive timer decrements without clearing")
eq(countdown_step.alarm, true, "zero timer remains active for this tick")
local countdown_end = Alarm.tick(facts({
    alarm = true, timer = countdown_step.timer,
    frame_counter = E.FieldAlarm.FRAME_VALUE - 1,
}))
eq(countdown_end.alarm, false, "next negative timer tick clears")

local explicit_clear = Alarm.tick(facts({
    alarm = true, timer = 77, clear_alarm = true,
}))
eq(explicit_clear.alarm, false, "security success uses shared clear")
eq(explicit_clear.disabled, false,
   "ordinary security success does not set FFE170")
eq(explicit_clear.sound_stop, E.SoundId.ALARM,
   "ordinary security success stops the alarm sound")

-- ROM 0x167AE resets FFE16F/FFE170 at scene entry. Its alarm cleanup also
-- takes 0x1E09A/0x1E0B8 side effects so a scene exit stops sound 0x30 and
-- promotes published reinforcements before 0x1E166 evaluates 0x1E20A again.
local scene_exit = Alarm.tick(facts({
    alarm = true,
    disabled = true,
    timer = E.AiTuning.ALARM_FRAMES,
    scene_exit = true,
    actors = {
        { flags9e = E.ActorFlag.REINFORCEMENT },
        { flags9e = E.ActorFlag.SPOTTED },
    },
}))
eq(scene_exit.alarm, false, "scene entry clears FFE16F")
eq(scene_exit.disabled, false, "scene entry clears FFE170")
eq(scene_exit.sound_stop, E.SoundId.ALARM,
   "scene exit stops the alarm sound resource")
eq(scene_exit.actors[1].flags9e,
   E.ActorFlag.REINFORCEMENT | E.ActorFlag.DESPAWN_OFFSCREEN,
   "scene exit marks alarm responders for despawn")
eq(scene_exit.actors[2].flags9e, E.ActorFlag.SPOTTED,
   "scene exit preserves unrelated actor flags")
local quiet_exit = Alarm.tick(facts({ alarm = false, scene_exit = true }))
eq(quiet_exit.sound_stop, nil,
   "scene exit with no active alarm sends no second 0x30 stop")
eq(Encounters.trigger_attempt({
       scene_id = E.FieldAlarmRomLayout.FIRST_LIST_SCENE,
       frame_counter = E.FieldEncounter.FRAME_VALUE,
       display_service_word = 0,
       leader_protected = false,
       field_activity_lock_active = false,
       field_alarm_active = scene_exit.alarm,
       combat_active = false,
       encounter_actor_count = 0,
       probability_roll = 0,
       probability_threshold = E.FieldAlarm.WORD_MAX,
   }), "select_scene_outcome",
   "scene exit restores the ordinary table instead of alarm list 0x1E20A")

local reset = Alarm.tick(facts({ alarm = true, disabled = true, reset = true }))
eq(reset.alarm, false, "projected-cell reset clears FFE16F")
eq(reset.disabled, false, "projected-cell reset clears FFE170")
eq(reset.sound_stop, nil, "projected-cell reset does not stop sound")
eq(reset.actors[1].flags9e,
   E.ActorFlag.SPOTTED | E.ActorFlag.REINFORCEMENT,
   "projected-cell reset does not promote actor flags")

-- FFE170 suppresses new region alarms after timer servicing [C].
local disabled = Alarm.tick(facts({ disabled = true, rng = 0 }))
eq(disabled.alarm, false, "disabled security blocks a new alarm")
eq(disabled.sample_rng, nil, "disabled security prevents the random draw")
local disabled_clear = Alarm.tick(facts({
    alarm = true, timer = E.AiTuning.ALARM_FRAMES, disable_alarm = true,
}))
eq(disabled_clear.disabled, true, "terminal disable sets FFE170 state")
eq(disabled_clear.alarm, false, "terminal disable uses shared clear")
eq(disabled_clear.sound_stop, E.SoundId.ALARM,
   "terminal disable returns the shared clear sound stop")

-- Scene 0x14's explicit begin path schedules the special timeout [C].
local alarm_combat = Alarm.tick(facts({
    scene_id = E.SceneId.ALARM_COMBAT,
    direct_raise = true,
}))
eq(alarm_combat.schedule_timeout, true,
   "alarm-combat scene selects the timeout path")
eq(alarm_combat.alarm, false,
   "alarm-combat timeout path does not set FFE16F")

-- Direct 0x1E04A calls bypass the frame/RNG/rectangle trigger gates [C].
local direct = Alarm.tick(facts({
    frame_counter = E.FieldAlarm.FRAME_VALUE - 1,
    direct_raise = true,
}))
eq(direct.alarm, true, "direct source event raises the alarm")
eq(direct.sound_start, E.SoundId.ALARM,
   "direct alarm raise requests sound 0x30")

print(("field alarm tests passed (%d checks)"):format(checks))
