local E = require("enums")
local producer = require("walkthrough.route_beat_w7_b16_ito_producer")

local steps = {}
for _, step in ipairs(producer.steps) do
    steps[#steps + 1] = step
end

local ito_won_mask = 1 << E.StoryFlagBit.ITO_FIGHT_WON
local baseline
local transitions_total = 0
local callbacks_total = 0
local shots_total = 0
local hits_total = 0

local function hold(host, button, frames)
    local ok, problem = host.hold_pad(button, frames)
    assert(ok, problem or "Ito live combat pad input failed")
end

local function combat_window(host, initial)
    if initial then
        -- Release the entry modal, then begin the Mako live-combat pattern.
        hold(host, "NEUTRAL", 240)
    end
    hold(host, "B", 1)
    hold(host, "A", 1)
    local after = host.snapshot()
    for _ = 1, 18 do
        hold(host, "A", 179)
        hold(host, "A+B", 1)
        after = host.snapshot()
        if (after.story & ito_won_mask) ~= 0 then break end
    end
    hold(host, "NEUTRAL", 1)
    after = host.snapshot()
    transitions_total = transitions_total + host.count_source_events(
        "field.world_activity.matchstick_ito_actor_transition_completed")
    callbacks_total = callbacks_total + host.count_source_events(
        "field.world_activity.matchstick_ito_callback_dispatched")
    shots_total = shots_total + host.count_source_events(
        "field.combat.player_weapon_spawned")
    hits_total = hits_total + host.count_source_events(
        "field.combat.weapon_hit_resolved:")
    return after
end

steps[#steps + 1] = {
    id = "ito_live_combat_window_one",
    milestone = 16,
    expected = { scene = E.Scene.MATCHSTICK },
    setup = function(host)
        -- Template 0x1E5D6 uses the bodyguard at (0x1280,0x0480); place in
        -- this room's firing lane at x-0x0100 before live pad combat.
        local placed, placement_error = host.place_within_scene(
            E.Scene.MATCHSTICK, 4480, 1152)
        assert(placed, placement_error or
            "Ito combat setup position was rejected in Matchstick")
        -- ROM item table 0xD4540: grant/equip Warhawk with test ammunition.
        local armed, arm_error = host.prepare_ito_combat_for_setup()
        assert(armed, arm_error or "Warhawk combat setup failed")
        -- Start at heat 7 so the callback must apply its +2 cap at 8.
        local heat_set, heat_error = host.set_heat_level_for_setup(7)
        assert(heat_set, heat_error or "Ito heat setup failed")
    end,
    run = function(host)
        baseline = host.snapshot()
        assert((baseline.story & ito_won_mask) == 0,
            "Ito fight result was already set before combat")
        assert(baseline.nuyen ~= nil and baseline.heat_level == 7 and
               baseline.strike_team_stage >= 0,
            "Ito callback inputs were unavailable before combat")
        local after = combat_window(host, true)
        assert((after.story & ito_won_mask) ~= 0 or
               after.activity_transitions_remaining < 4,
            "Ito live combat window one made no actor transition; shots=" ..
            tostring(shots_total) .. ", hits=" .. tostring(hits_total))
        return "first live-pad combat window: transitions=" ..
            tostring(transitions_total) .. ", shots=" .. tostring(shots_total) ..
            ", hits=" .. tostring(hits_total)
    end,
}

steps[#steps + 1] = {
    id = "ito_live_combat_window_two_and_callback",
    milestone = 16,
    expected = { scene = E.Scene.MATCHSTICK },
    run = function(host)
        local after = host.snapshot()
        if (after.story & ito_won_mask) == 0 then
            after = combat_window(host, false)
        end
        assert((after.story & ito_won_mask) ~= 0,
            "Ito live pad combat did not dispatch the callback after two windows; transitions=" ..
            tostring(after.activity_transitions_remaining) ..
            ", completed=" .. tostring(transitions_total) ..
            ", shots=" .. tostring(shots_total) .. ", hits=" ..
            tostring(hits_total) .. ", actors=" ..
            tostring(after.script_actor_summaries))
        assert(after.nuyen == baseline.nuyen + 10000 and
               after.heat_level == 8 and
               after.strike_team_stage == math.min(
                   8, baseline.strike_team_stage + 2),
            "Ito callback effects differ from ROM 0x54286 (+10000 nuyen, " ..
            "+2 capped heat, +2 strike-team stage)")
        assert(after.activity_transitions_remaining == -1 and
               not after.activity_callback_pending,
            "Ito callback did not drain its four-transition gate")
        assert(transitions_total == 4 and callbacks_total == 1,
            "Ito callback dispatch count mismatch: transitions=" ..
            tostring(transitions_total) .. ", callbacks=" ..
            tostring(callbacks_total))
        return "held A and pulsed B until all four actors transitioned; ROM 0x54286 callback effects verified"
    end,
}

return { steps = steps }
