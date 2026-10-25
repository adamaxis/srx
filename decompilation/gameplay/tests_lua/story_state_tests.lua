-- story_state.lua and the Club Penumbra Mako callback against a host stub
-- that mirrors FieldSceneRuntime's story.* / field.* primitives.
local E = require("enums")

local host
local function reset(scene, heat, strike, busy)
    host = { scene = scene, bytes = { [E.StoryByte.HEAT] = heat,
             [E.StoryByte.STRIKE_TEAM] = strike }, busy = busy or false,
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
        local addr = op:match("^byte_get_(%x+)$")
        if addr then
            local v = host.bytes[tonumber(addr, 16)]
            return v and ("value_" .. v) or nil
        end
        local a, v = op:match("^byte_set_(%x+)_(%d+)$")
        if a then host.bytes[tonumber(a, 16)] = tonumber(v); return "applied" end
        if op == "world_effect_idle" then return host.busy and "busy" or "idle" end
        local s, l, n, name = op:match("^queue_world_effect_(%d+)_(%x+)_(%d+)_(.+)$")
        if s then
            host.queued[#host.queued + 1] = { tonumber(s), tonumber(l, 16), tonumber(n), name }
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
check(S.advance_strike_team() == "applied" and host.bytes[E.StoryByte.STRIKE_TEAM] == 8, "advance to 8")
check(S.advance_strike_team() == "applied" and host.bytes[E.StoryByte.STRIKE_TEAM] == 8, "cap 8")
check(#host.queued == 0, "no follow-up outside scene 2")
-- Unknown byte: no change.
reset(5, 0, nil)
check(S.advance_strike_team() == nil, "unknown strike byte fails")
-- Scene 2 (ROM 0x1F1E0): queues Boris list 0x1D6FD0 with 2 actors.
reset(2, 0, 1)
check(S.advance_strike_team() == "applied" and host.bytes[E.StoryByte.STRIKE_TEAM] == 2, "scene 2 advance")
check(#host.queued == 1 and host.queued[1][1] == 2 and host.queued[1][2] == 0x1D6FD0 and
      host.queued[1][3] == 2 and host.queued[1][4] == "boris_strike_team", "Boris list queued")
-- Scene 2 while a world effect is pending: no change.
reset(2, 0, 1, true)
check(S.advance_strike_team() == nil and host.bytes[E.StoryByte.STRIKE_TEAM] == 1, "busy blocks")

local Club = require("club_penumbra")
reset(5, 7, 0)
check(Club.apply_mako_callback() == "applied", "mako applied")
check(host.bytes[E.StoryByte.HEAT] == 8 and host.karma == 2 and
      host.bytes[E.StoryByte.STRIKE_TEAM] == 1 and host.facts.story_flag_bit_11 == true and
      host.facts.field_mode_flag_bit_20 == false and host.facts.field_mode_flag_bit_24 == true and
      host.facts.notebook_tip_flag_bit_49 == false, "mako effects")
-- Missing roster: nothing changes.
reset(5, 3, 0); host.roster = false
check(Club.apply_mako_callback() == "service_unavailable" and host.bytes[E.StoryByte.HEAT] == 3 and
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
