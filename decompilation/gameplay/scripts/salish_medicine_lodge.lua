-- Scene 7, event 0: the Salish medicine lodge where Stark lies dying.
-- ROM finalizer 0x1FDBE (FFFBF8 = 7, FFFBF9 = 0) selects dialogue profile
-- 0x121E08 and one of three 0x1F246 roots, then sets D6 = -1 at 0x1FE80:
-- the event-table scene 13 is never loaded and the prompt returns to Scene 7.
local E = require("enums")
local StoryFacts = require("story_facts")
local DialogueActions = require("dialogue_actions")

local EFFECTS = DialogueActions.new_default_registry()

local MedicineLodge = {}

local LOCATION_ID = "salish_medicine_lodge"

local function assert_route_context()
    assert(game.location.type == "building" and
           game.location.id == LOCATION_ID,
        "Salish medicine lodge dialogue ran outside its linked building")
end

local function reply(menu_id, choice_id)
    local choice = game.ui.menu(menu_id, { choice_id })
    assert(choice == choice_id,
        "medicine lodge reply " .. menu_id .. " returned " .. tostring(choice))
end

-- Root 0x11B714: eleven 0x1F246 records. Single-label records are the
-- player's reply; 0x0FE5BF records are <MORE> continuations.
local function present_first_visit()
    reply("turned_away", "neighborhood")        -- 0x11B714 / 0x103F9F
    game.ui.say("holo_commotion")               -- 0x11B728
    game.ui.say("holo_youth")                   -- 0x11B73C
    game.ui.say("holo_shaman")                  -- 0x11B750
    reply("holo_michael", "where_holo")         -- 0x11B764 / 0x11BB77
    game.ui.say("shaman_tension")               -- 0x11B778
    game.ui.say("follow_me")                    -- 0x11B78C
    -- Command 0x42 (table 0x12CCB4 -> 0x21F80) sets FFFC34 bit 18, the
    -- Notebook "Stark is dying" tip, then switches to profile 0x121DE4 for
    -- record 0x11B7A0. That profile's portrait tiles (0x1219FC) are not
    -- extracted, so the alcove page keeps the lodge speaker.
    local ok, result, failure = EFFECTS:execute(
        "field.set_mode_effect_flag",
        { flagBit = E.FieldModeEffectFlagBit.STARK_DYING_TIP, value = true })
    assert(ok and result == nil,
        failure or "medicine lodge command 0x42 flag write failed")
    reply("stark_alcove", "call_stark")         -- 0x11B7A0 / 0x11BE41
    -- Command 0x43 (0x21FB8) restores profile 0x121E08 for 0x11B7B4.
    game.ui.say("stark_found")                  -- 0x11B7B4
    game.ui.say("stark_magic")                  -- 0x11B7C8
    game.ui.say("stark_fetch_help")             -- 0x11B7DC (ends: -1)
end

-- ROM 0x59BCC clears the downed/dead conditions and refills physical and
-- stun for all three live-record slots.
local function heal_party()
    assert(type(game.party) == "function" and
           type(game.commit_party) == "function",
           "medicine lodge heal needs the party service")
    local party = game.party()
    for slot = 0, 2 do
        local runner = party[slot]
        if runner then
            runner.runner_facts.lifecycle.dead = false
            runner.runner_facts.lifecycle.downed = false
            runner.physical = 10
            runner.stun = 10
        end
    end
    game.commit_party(party)
end

function MedicineLodge.present_field_exit_prompt()
    assert_route_context()
    if StoryFacts.is_set(E.StoryFact.MEDICINE_LODGE_TURNED_AWAY) ~= true then
        -- 0x1FDE2..0x1FDF4: set FFFC1E bit 24, then root 0x11B714.
        StoryFacts.set(E.StoryFact.MEDICINE_LODGE_TURNED_AWAY)
        present_first_visit()
    elseif StoryFacts.is_set(E.StoryFact.HOSPITAL_VISITED) ~= true then
        -- 0x1FDFE..0x1FE0C: bit 25 clear selects 0x11B7E4 (text 0x11C0EA).
        game.ui.say("stark_time_short")
    else
        -- 0x1FE10..0x1FE16: heal via 0x59BCC, then 0x11B7EC (0x11C14C).
        heal_party()
        game.ui.say("stark_saved_heal")
    end
    return "field_return"
end

return MedicineLodge
