-- story_state.lua and the Club Penumbra Mako callback against a host stub
-- that mirrors FieldSceneRuntime's story.* / field.* primitives.
local host
local function reset(scene, heat, strike, busy)
    host = { scene = scene, heat_level = heat,
             strike_team_stage = strike, busy = busy or false,
             queued = {}, karma = 0, roster = true,
             facts = { notebook_tip_flag_bit_49 = true,
                       field_mode_flag_bit_20 = true } }
end
game = {
    state = setmetatable({}, {
        __index = function(_, name) return host.facts[name] end,
        __newindex = function(_, name, value) host.facts[name] = value end,
    }),
    services = { call = function(domain, op)
        if domain == "field" and op == "scene_reference" then
            return "scene_" .. host.scene
        end
        if domain == "party" and op == "award_karma_2" then
            if not host.roster then return nil end
            host.karma = host.karma + 2
            return "applied"
        end
        assert(domain == "story", "unexpected service " .. domain .. "." .. op)
        if op == "get_heat_level" then
            return host.heat_level and ("level_" .. host.heat_level) or nil
        end
        if op == "get_strike_team_stage" then
            return host.strike_team_stage and
                ("stage_" .. host.strike_team_stage) or nil
        end
        local heat = op:match("^set_heat_level_(%d+)$")
        if heat then host.heat_level = tonumber(heat); return "applied" end
        local stage = op:match("^set_strike_team_stage_(%d+)$")
        if stage then host.strike_team_stage = tonumber(stage); return "applied" end
        if op == "world_effect_idle" then return host.busy and "busy" or "idle" end
        local s, n, sequence_id = op:match("^queue_world_effect_(%d+)_(%d+)_(.+)$")
        if s then
            host.queued[#host.queued + 1] = { tonumber(s), tonumber(n), sequence_id }
            return "queued"
        end
        error("unexpected story op " .. op)
    end },
}

local S = require("story_state")
local checks = 0
local function check(cond, msg) assert(cond, msg); checks = checks + 1 end

-- Plain scene: +1, no follow-up; cap at 8.
reset(5, 0, 7)
check(S.advance_strike_team() == "applied" and host.strike_team_stage == 8, "advance to 8")
check(S.advance_strike_team() == "applied" and host.strike_team_stage == 8, "cap 8")
check(#host.queued == 0, "no follow-up outside scene 2")
-- Unknown byte: no change.
reset(5, 0, nil)
check(S.advance_strike_team() == nil, "unknown strike byte fails")
-- Scene 2 (ROM 0x1F1E0): queues the Boris sequence by semantic identity.
reset(2, 0, 1)
check(S.advance_strike_team() == "applied" and host.strike_team_stage == 2, "scene 2 advance")
check(#host.queued == 1 and host.queued[1][1] == 2 and
      host.queued[1][2] == 2 and
      host.queued[1][3] == "boris_strike_team", "Boris sequence queued")
-- Scene 2 while a world effect is pending: no change.
reset(2, 0, 1, true)
check(S.advance_strike_team() == nil and host.strike_team_stage == 1, "busy blocks")

local Club = require("club_penumbra")
reset(5, 7, 0)
check(Club.apply_mako_callback() == "applied", "mako applied")
check(host.heat_level == 8 and host.karma == 2 and
      host.strike_team_stage == 1 and host.facts.story_flag_bit_11 == true and
      host.facts.field_mode_flag_bit_20 == false and host.facts.field_mode_flag_bit_24 == true and
      host.facts.notebook_tip_flag_bit_49 == false, "mako effects")
-- Missing roster: nothing changes.
reset(5, 3, 0); host.roster = false
check(Club.apply_mako_callback() == "service_unavailable" and host.heat_level == 3 and
      host.facts.story_flag_bit_11 == nil, "no roster leaves state")
-- Unresolved facts: unavailable.
reset(5, 3, 0); host.facts.notebook_tip_flag_bit_49 = nil
check(Club.apply_mako_callback() == "service_unavailable" and host.karma == 0, "unresolved facts")

print(("story_state tests passed: %d checks"):format(checks))

-- Route actor lists: queue id carries the data-table descriptor.
local seen
game.services.call = function(domain, op) seen = domain .. "." .. op; return "queued" end
local Routes = require("route_actor_lists")
assert(Routes.queue("club_penumbra_mako") == "queued" and
       seen == "field.queue_route_actor_list_17_47_1e628_4_club_penumbra_mako", seen)
assert(type(Routes.complete_club_penumbra_mako) == "function")
print("route_actor_lists tests passed")
