package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

-- ROM finalizer 0x1FDBE, Scene 7/event 0: three roots, all returning to the
-- field (D6 = -1 at 0x1FE80).
local Lodge = require("salish_medicine_lodge")
local Json = require("json")

local function load_json(path)
    local file = assert(io.open(path, "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local texts = load_json("decompilation/assets/dialogue/script-text.json").texts
local speaker = load_json("decompilation/gameplay/dialogue/speakers.json")
    .profiles.salish_medicine_lodge_shaman
local menus = load_json("decompilation/gameplay/dialogue/menus.json")
    .profiles["salish.medicine_lodge"]

local operations
local party

local function reset(facts)
    operations = {}
    party = {
        [0] = { physical = 2, stun = 0,
                runner_facts = { lifecycle = { dead = false, downed = true } } },
        [1] = { physical = 0, stun = 0,
                runner_facts = { lifecycle = { dead = true, downed = false } } },
    }
    game = {
        state = facts,
        location = { type = "building", id = "salish_medicine_lodge" },
        field = {},
        ui = {
            say = function(event_id)
                operations[#operations + 1] = "say:" .. event_id
            end,
            menu = function(menu_id, choices)
                assert(#choices == 1, "lodge replies are single-label records")
                operations[#operations + 1] = "menu:" .. menu_id .. ":" .. choices[1]
                return choices[1]
            end,
        },
        party = function() return party end,
        commit_party = function(updated) party = updated end,
    }
end

local function text_of_event(event_id)
    local event = assert(speaker.events[event_id], "missing event " .. event_id)
    return assert(texts[event.title_key], "missing text " .. event.title_key)
end

local function text_of_menu(menu_id)
    local menu = assert(menus[menu_id], "missing menu " .. menu_id)
    return assert(texts[menu.title_key], "missing text " .. menu.title_key)
end

-- First visit: FFFC1E bit 24 clear -> set it, play root 0x11B714.
reset({ medicine_lodge_turned_away = false, hospital_visited = false })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(game.state.medicine_lodge_turned_away == true,
    "0x1FDEA did not set FFFC1E bit 24")
assert(game.state.field_mode_effect_flag_bit_18 == true,
    "command 0x42 (0x21F80) did not set FFFC34 bit 18")
local expected = {
    "menu:turned_away:neighborhood", "say:holo_commotion", "say:holo_youth",
    "say:holo_shaman", "menu:holo_michael:where_holo", "say:shaman_tension",
    "say:follow_me", "menu:stark_alcove:call_stark", "say:stark_found",
    "say:stark_magic", "say:stark_fetch_help",
}
assert(#operations == #expected, "0x11B714 chain must have eleven records")
for index, operation in ipairs(expected) do
    assert(operations[index] == operation,
        "record " .. index .. " was " .. tostring(operations[index]))
end
-- ROM text 0x11B804 / 0x11C014 bracket the chain.
assert(text_of_menu("turned_away"):sub(1, 54) ==
    "<You enter a large, dim building of AmerIndian design.")
assert(text_of_event("stark_fetch_help"):find(
    "If you could fetch help he may yet live.", 1, true))
assert(speaker.events.stark_fetch_help.continuation_key == "",
    "0x11B7DC ends the chain (-1)")
assert(speaker.events.holo_commotion.continuation_key == "shared.more")

-- Revisit before admission: 0x11B7E4 -> text 0x11C0EA, no heal.
reset({ medicine_lodge_turned_away = true, hospital_visited = false })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#operations == 1 and operations[1] == "say:stark_time_short")
assert(text_of_event("stark_time_short"):sub(1, 52) ==
    "The time for Stark to join his ancestors draws nigh.")
assert(party[0].runner_facts.lifecycle.downed == true, "revisit must not heal")

-- After admission: 0x59BCC heal, then 0x11B7EC -> text 0x11C14C.
reset({ medicine_lodge_turned_away = true, hospital_visited = true })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#operations == 1 and operations[1] == "say:stark_saved_heal")
for slot = 0, 1 do
    assert(party[slot].physical == 10 and party[slot].stun == 10 and
           not party[slot].runner_facts.lifecycle.dead and
           not party[slot].runner_facts.lifecycle.downed,
           "0x59BCC did not restore slot " .. slot)
end

print("salish_medicine_lodge_tests: ok")
