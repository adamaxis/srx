-- Source-backed handlers for the weighted field-event selector.
-- ROM 0x21004 opens the hidden-camera prompt; its MORE command at 0x244D2
-- rolls the party's average Quickness against a party-size target. Source
-- addresses stay here as provenance; Lua works with named runner facts.
local RandomEvents = {}
local E = require("enums")
local StoryState = require("story_state")
local Encounters = require("field_encounters")

local function source_boundary(reason)
    return { type = "source_boundary", reason = reason }
end

local ALARM_RESULT =
    "<As the alarm goes off, you realize you must have just walked in front " ..
    "of a hidden security camera.>"
local NERVES_RESULT =
    "<Nothing happens, though. Maybe it's just your nerves.>"
-- ROM record 0x120F90 (text 0x11D128) carries a MORE label at 0xFE5BF whose
-- command is record 0x120FA4 (text 0x11D1C9), the terminal second page.
local FACTION_NEUTRAL_RESULT =
    "<Impressive figures pass through the parting crowd.||" ..
    "Your first impression is that they are veteran shadowrunners.||" ..
    "Then you see they are wearing gang colors!?>"
local FACTION_NEUTRAL_CONTINUATION =
    "<They continue on down the street in an air of confidence, not " ..
    "bothering to harrass those they pass.||" ..
    "Gangs going pro?  You doubt it!!||" ..
    "Shrugging, you continue on your way.>"
local FACTION_MAFIA_RESULT =
    "<A gang of Eye-Fivers jump out of the crowd.>||" ..
    "So, you chose to serve the Yakuza?||" ..
    "Then you deserve to die like the other enemies of the Mafia!"
local FACTION_HALLOWEENER_RESULT =
    "<Some Halloweeners appear out of the crowd.>||" ..
    "Hey, Mafia boy!  You chose the wrong side to ally with!||" ..
    "Yakuza rules this town!"
local FACTION_LIST_FAILURE_RESULT =
    "<You glimpse someone taking your picture with a holocam.>||" ..
    "The person quickly disappears into the crowd."
local HAGGARD_HELP_RESULT =
    "<You grab the man and hold him while the henchmen straight-jacket " ..
    "him and drag him off.>||Here's fifty nuyen. Now get lost."
local LONE_STAR_ATTACK_RESULT =
    "<The Lone Star forces draw their weapons with well-trained efficiency.>"
local LONE_STAR_ATTACK_LIST_FAILURE =
    "<As you are about to jump them, they get a call on the radio and " ..
    "hastily leave.>"
local HAGGARD_CREDENTIALS_RESULT =
    "<They explain that they work for an insane asylum and that this is one " ..
    "of their patients.||After showing the proper credentials they give you " ..
    "50$ for your help, take their patient, and leave.>"
local HAGGARD_THREATEN_RESULT =
    "<The doctor turns to his two assistants.>||Let's go.||" ..
    "This one's about dry, anyway."
local HAGGARD_ZERO_BODY_ATTACK_RESULT =
    "Who do you think you're scaring?||Kill them all!!"
local HAGGARD_ONE_BODY_ATTACK_RESULT =
    "<The doctor looks about nervously, then addresses his henchmen.>||" ..
    "You take care of the brash one as well as our friend.||" ..
    "I have other matters to attend to."
local HAGGARD_INTRO_TYPICAL =
    "<The man looks like a typical doctor to you.>"
local HAGGARD_INTRO_UNSURE =
    "<You're not sure what to make of this strange group.>"
local HAGGARD_INTRO_MALIGNANT =
    "<The well-dressed man has a malignant air about him.||" ..
    "The ragged victim seems terrified.>"
local HAGGARD_CHOICES = {
    { id = "attack", label = "Attack!!" }, -- ROM text 0x103A9A.
    { id = "threaten", label = "Leave him alone or you'll be sorry!" }, -- 0x103BFE.
    { id = "help", label = "Help them." }, -- ROM text 0x103ADE.
} -- Longplay section 11, frame 930; source dispatch 0x23BB4 [C].
local SECURITY_GUARD_NO_ID_CHOICES = {
    { id = "attack", label = "<Attack!!>" }, -- 0x121278 / command 0xD2.
    { id = "talk", label = "<Talk to him.>" }, -- 0x121278 / command 0xD0.
}
local SECURITY_GUARD_NO_ID_PROMPT =
    "<Since you don't have a badge, you'll have to come up with something else.>" -- 0x121278 / 0x11F168 [C].
local SECURITY_GUARD_ATTACK_RESULT =
    "<You quickly pull your weapons and attack!>" -- 0x121290 [C].
local SECURITY_GUARD_TALK_ONE_SUCCESS_RESULT =
    "<You barely convince him you have misplaced it.\n\n" ..
    "Suspicous, he decides to check in with dispatch...>" -- 0x1212A0 -> 0x11F313 [C].
local SECURITY_GUARD_TALK_ZERO_SUCCESS_RESULT =
    "<You try a lame story.\n\n" ..
    "You realize this isn't going to work about the time he attacks.>" -- 0x121298 -> 0x11F248 [C].
local SECURITY_GUARD_TALK_SUCCESS_RESULT =
    "<You manage to convince him you misplaced it.\n\n" ..
    "He seems satisfied and moves on.>" -- 0x1212B4 -> 0x11F378 [C].
local SECURITY_GUARD_TALK_DISPATCH_SUCCESS_RESULT =
    "<Dispatch verifies his check and he waves you by.>" -- 0x121268 -> 0x11F1E0 [C].
local SECURITY_GUARD_TALK_DISPATCH_ALERT_RESULT =
    "<Dispatch comes back negative and sounds the alert.>" -- 0x121270 -> 0x11F213 [C].
local SECURITY_GUARD_BADGE_PASS_RESULT =
    "<You show the ID, and he waves you past.>" -- 0x12124C -> 0x11F098 [C].
local SECURITY_GUARD_BADGE_VERIFY_RESULT =
    "<He checks your ID and calls dispatch to verify it.>" -- 0x121254 -> 0x11F133 [C].
-- The event artwork in section 11 frames 000618/000930/000990 matches this
-- extracted image; give this route its own binding because field_narrator has
-- no portrait.
local HAGGARD_SPEAKER = "haggard_man_event"
local HAGGARD_TEXT_PROFILE = "field.world_event"
local COMPANY_MAN_CONFRONTATION_SPEAKER = "company_man_confrontation"
local COMPANY_MAN_CONFRONTATION_TEXT_PROFILE = "field.world_event"
local COMPANY_MAN_SPEAKER = "company_man_random_event"
local COMPANY_MAN_TEXT_PROFILE = "field.world_event"
local WATCHER_BODY_TARGET = 6 -- ROM 0x2381A [C].
local WATCHER_REWARD_NUYEN = 200 -- ROM 0x23834 [C].
local WATCHER_ACCEPT_CHOICE = "accept_watcher_request"
local WATCHER_INSULT_CHOICE = "insult"
local WATCHER_REFUSAL_RESULT =
    "This is horrible!  Where's your compassion?!||" ..
    "<The watcher vanishes.>" -- ROM text 0x11D893 [C].
local WATCHER_ACCEPT_RESULT =
    "<The watcher leads you through some side streets.||" ..
    "He points ahead, and you see an old man being brutally beaten " ..
    "by a group of Eye-Fivers.>" -- ROM text 0x11D8D9 [C].
local WATCHER_ZERO_SUCCESS_RESULT =
    "<The mage looks up.||" ..
    "The Eye-Fivers turn around, following his gaze.>||" ..
    "Look, some new meat to tenderize!  Get him, boys!" -- ROM text 0x11D965 [C].
local WATCHER_ONE_SUCCESS_RESULT =
    "<The mage looks at you pleadingly.||" ..
    "The Eye-Fivers turn and size you up.||" ..
    "A couple of them run away, but the rest stay behind.>||" ..
    "You want to play, too, do you?" -- ROM text 0x11D9DE [C].
local WATCHER_WAVE_RESULT =
    "<As the gangers come at you, you catch the mage giving you a wave " ..
    "of thanks.||He then shimmers and fades to nothingness.>" -- ROM text 0x11DA7E [C].
local WATCHER_REWARD_RESULT =
    "<The Eye-Fivers see you and scatter immediately.||" ..
    "The mage addresses you.>||Thank you for your help.||" ..
    "This, I'm afraid, is all I can offer in return.||" ..
    "<He hands you 200 nuyen.>" -- ROM text 0x11DAF8 [C].
local COMPANY_MAN_TARGET = 7 -- ROM 0x24846 and 0x2489A [C].
local COMPANY_MAN_CONFRONTATION_TARGET = 5 -- ROM 0x23920 [C].
local COMPANY_MAN_CONFRONTATION_ATTACK_FLEE_RESULT =
    "<As you move to attack, the man turns white and runs away.>" -- ROM 0x120FC8 / text 0x11DC13 [C].
local COMPANY_MAN_CONFRONTATION_LEAVE_RESULT =
    "<The man follows you for a while, yelling at you.||" ..
    "He eventually moves on.>" -- ROM 0x120FD0 / text 0x11DC4F [C].
local COMPANY_MAN_CONFRONTATION_TALK_RESULT =
    "Wait... You aren't Mr. Johnson, are you?||" ..
    "I am very sorry to have bothered you.||" ..
    "<The confused man scurries off.>" -- ROM 0x120FD8 / text 0x11DD5E [C].
local COMPANY_MAN_ATTACK_FLEE_RESULT =
    "<As you pull your guns, the Company Man flees and raises the alarm.>"
    -- ROM record 0x121510 -> text 0x11FE92 [C].
local COMPANY_MAN_ATTACK_WOUND_RESULT =
    "<You wound him as he escapes down the hall.  " ..
    "The alarm soon comes to life..>"
    -- ROM record 0x121518 -> text 0x11FED7 [C].
local COMPANY_MAN_ATTACK_DEATH_RESULT =
    "<He does the staccato death-dance to the beat of your gun.||" ..
    "He'll not mock you again!>" -- ROM record 0x121520 -> 0x11FF24 [C].
local COMPANY_MAN_INSULT_SECURITY_RESULT =
    "<He stops and looks you over carefully.  " ..
    "He then calls security.>" -- ROM record 0x121528 -> 0x11FF7B [C].
local COMPANY_MAN_INSULT_LAUGH_RESULT =
    "<He laughs as he walks away.>" -- ROM record 0x121530 -> 0x11FFBD [C].
local COMPANY_MAN_STARE_DOWN_RESULT =
    "<He stops and stares you down. He then smiles.>" -- ROM 0x11FFDB [C].
local COMPANY_MAN_RESOURCE_CLUES = {
    first = "He says to check the first floor.>",
    second = "He says to check the second floor.>",
    third = "He says to check the third floor.>",
    fourth = "He says to check the fourth floor.>",
    fifth = "He says to check the fifth floor.>",
    sixth = "He says to check the sixth floor.>",
} -- ROM pointer table 0x121540 indexed by FFE12C [C].
local COMPANY_MAN_LEAVE_RESULT =
    "<He saunters away.>" -- ROM record 0x121538 -> text 0x12013E [C].
local COMPANY_MAN_RESOURCE_VARIANT_SERVICE = "resource_variant"
local WATCHER_ATTACK_OUTCOMES = {
    [0] = {
        sequence_id = E.WorldEffectSequence.PHANTASMAL_WATCHER_EYE_FIVERS_FOUR,
        actors = 4,
        result_text = WATCHER_ZERO_SUCCESS_RESULT,
    },
    [1] = {
        sequence_id = E.WorldEffectSequence.PHANTASMAL_WATCHER_EYE_FIVERS_TWO,
        actors = 2,
        result_text = WATCHER_ONE_SUCCESS_RESULT,
    },
} -- ROM 0x23846 selects list 0x1D6DAE or 0x1D6DB6 [C].
local HAGGARD_ATTACK_OUTCOMES = {
    [0] = {
        sequence_id =
            E.WorldEffectSequence.HAGGARD_GATOR_SHAMAN_AND_BODYGUARDS,
        actors = 3,
        result_text = HAGGARD_ZERO_BODY_ATTACK_RESULT,
    },
    [1] = {
        sequence_id = E.WorldEffectSequence.HAGGARD_TWO_BODYGUARDS,
        actors = 2,
        result_text = HAGGARD_ONE_BODY_ATTACK_RESULT,
    },
} -- ROM 0x23C52 selects 0x1D704C or 0x1D7050 after a target-6 Body roll [C]

local function event_opening(event_id)
    local data = game.modules.world_content_data
    local content = type(data) == "table" and
        data.world_activity_event_content or nil
    local choices = type(data) == "table" and
        data.world_activity_event_choices or nil
    if type(content) ~= "table" or type(choices) ~= "table" then
        return nil
    end

    local opening
    for _, row in ipairs(content) do
        if row.event_id == event_id then
            opening = row.opening_text
            break
        end
    end
    local choice_count = 0
    for _, row in ipairs(choices) do
        if row.event_id == event_id then
            choice_count = choice_count + 1
            if row.choice_order ~= 1 or row.choice_id ~= "continue" then
                return nil
            end
        end
    end
    if type(opening) ~= "string" or opening == "" or choice_count ~= 1 then
        return nil
    end
    return opening
end

local function event_prompt_data(event_id, expected_choice_ids)
    local data = game.modules.world_content_data
    local content = type(data) == "table" and
        data.world_activity_event_content or nil
    local choices = type(data) == "table" and
        data.world_activity_event_choices or nil
    if type(content) ~= "table" or type(choices) ~= "table" then
        return nil, nil
    end

    local opening
    for _, row in ipairs(content) do
        if row.event_id == event_id then
            opening = row.opening_text
            break
        end
    end
    local options = {}
    for _, row in ipairs(choices) do
        if row.event_id == event_id then
            if type(row.choice_order) ~= "number" or
               row.choice_order % 1 ~= 0 or
               type(row.choice_id) ~= "string" or row.choice_id == "" or
               type(row.label) ~= "string" or row.label == "" then
                return nil, nil
            end
            options[#options + 1] = {
                order = row.choice_order,
                id = row.choice_id,
                label = row.label,
            }
        end
    end
    table.sort(options, function(left, right)
        return left.order < right.order
    end)
    if type(opening) ~= "string" or opening == "" or
       type(expected_choice_ids) ~= "table" or
       #options ~= #expected_choice_ids then
        return nil, nil
    end
    local prompt_options = {}
    for index, expected_id in ipairs(expected_choice_ids) do
        if options[index].order ~= index or
           options[index].id ~= expected_id then
            return nil, nil
        end
        prompt_options[index] = {
            id = options[index].id,
            label = options[index].label,
        }
    end
    return opening, prompt_options
end

local function party_check()
    if type(game.party) ~= "function" then return nil end
    local party = game.party()
    if type(party) ~= "table" then return nil end

    local quickness_total = 0
    local active_count = 0
    for _, member in pairs(party) do
        local lifecycle = type(member) == "table" and
            type(member.runner_facts) == "table" and
            member.runner_facts.lifecycle or nil
        if type(lifecycle) == "table" and lifecycle.active == true then
            local quickness = type(member.attributes) == "table" and
                member.attributes.quickness or nil
            if type(quickness) ~= "number" or quickness % 1 ~= 0 or
               quickness < 0 or quickness > 0xFF then
                return nil
            end
            quickness_total = quickness_total + quickness
            active_count = active_count + 1
        end
    end
    if active_count == 0 or active_count > 3 then return nil end

    return math.floor(quickness_total / active_count), 4 + active_count - 1
end

local function random_die(bound, service_id)
    local response = game.services.call(
        "random", service_id or ("bounded_" .. bound))
    local digits = type(response) == "string" and
        response:match("^roll_(%d+)$") or nil
    local value = digits and tonumber(digits) or nil
    if value == nil or value % 1 ~= 0 or value < 0 or value >= bound then
        return nil
    end
    return value
end

local function first_runner_attribute(attribute)
    if type(game.party) ~= "function" then return nil end
    local party = game.party()
    local runner = type(party) == "table" and party[0] or nil
    local attributes = type(runner) == "table" and runner.attributes or nil
    local value = type(attributes) == "table" and attributes[attribute] or nil
    if type(value) ~= "number" or value % 1 ~= 0 or value < 0 or
       value > 0xFF then
        return nil
    end
    return value
end

local function haggard_intro_text(successes, selector_roll)
    if selector_roll < 50 then
        if successes == 0 then return HAGGARD_INTRO_TYPICAL end
        if successes == 1 then return HAGGARD_INTRO_UNSURE end
        return HAGGARD_INTRO_MALIGNANT
    end
    if successes == 0 then return HAGGARD_INTRO_MALIGNANT end
    if successes == 1 then return HAGGARD_INTRO_UNSURE end
    return HAGGARD_INTRO_TYPICAL
end

local function haggard_reward(amount, result_text, missing_funds_reason)
    local funds = type(game.nuyen) == "function" and game.nuyen() or nil
    if type(funds) ~= "number" or funds % 1 ~= 0 or funds < 0 then
        return source_boundary(missing_funds_reason)
    end
    game.set_nuyen(funds + amount)
    game.ui.say_text(result_text, false, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)
    return "resolved"
end

function RandomEvents.present_hidden_camera()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "hidden_camera" then
        return nil
    end

    local opening = event_opening("hidden_camera")
    local dice_pool, target = party_check()
    if opening == nil or dice_pool == nil then return nil end

    -- ROM prompt 0x12130C has one MORE command. The generic dialogue host
    -- renders the source text and forwards its single continuation input.
    game.ui.say_text(opening, true)

    local combat = game.modules.combat
    if type(combat) ~= "table" or
       type(combat.roll_thresholded_successes) ~= "function" then
        return nil
    end
    local successes = combat.roll_thresholded_successes(
        dice_pool, target, random_die)
    if successes == nil then return nil end

    if successes == 0 then
        -- ROM 0x244D2 calls the shared alarm routine 0x1E04A before its
        -- result record. Scene 20's timeout and map-specific reinforcements
        -- stay inside the existing typed field alarm service.
        if game.services.call("field", "raise_alarm") ~= "applied" then
            return nil
        end
        game.ui.say_text(ALARM_RESULT, false)
    else
        game.ui.say_text(NERVES_RESULT, false)
    end
    return "resolved"
end

local function watcher_reward()
    local funds = type(game.nuyen) == "function" and game.nuyen() or nil
    if type(funds) ~= "number" or funds % 1 ~= 0 or funds < 0 or
       type(game.set_nuyen) ~= "function" then
        return source_boundary("watcher reward needs the current nuyen value")
    end
    game.set_nuyen(funds + WATCHER_REWARD_NUYEN)
    game.ui.say_text(WATCHER_REWARD_RESULT, false, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)
    return "resolved"
end

function RandomEvents.present_phantasmal_watcher()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "phantasmal_watcher" then
        return nil
    end

    local opening, choices = event_prompt_data(
        "phantasmal_watcher",
        { WATCHER_ACCEPT_CHOICE, WATCHER_INSULT_CHOICE })
    if opening == nil or choices == nil then
        return source_boundary("watcher prompt data is unavailable")
    end
    local choice = game.ui.prompt_text(
        "phantasmal_watcher", opening, choices, HAGGARD_SPEAKER,
        HAGGARD_TEXT_PROFILE)
    if choice == WATCHER_INSULT_CHOICE then
        -- Record 0x120F18 ends after text 0x11D893 without changing state [C].
        game.ui.say_text(WATCHER_REFUSAL_RESULT, false, HAGGARD_SPEAKER,
                         HAGGARD_TEXT_PROFILE)
        return "resolved"
    end
    if choice ~= WATCHER_ACCEPT_CHOICE then
        return source_boundary("watcher choice is unavailable")
    end

    -- Record 0x120F20 has the shared MORE label at +4, then dispatches 0x9A.
    game.ui.say_text(WATCHER_ACCEPT_RESULT, true, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)
    local body = first_runner_attribute("body")
    local modules = type(game.modules) == "table" and game.modules or nil
    local combat = type(modules) == "table" and modules.combat or nil
    if body == nil or type(combat) ~= "table" or
       type(combat.roll_thresholded_successes) ~= "function" then
        return source_boundary("watcher Body check needs its runner and dice service")
    end
    local successes = combat.roll_thresholded_successes(
        body, WATCHER_BODY_TARGET, random_die)
    if successes == nil then
        return source_boundary("watcher Body roll is unavailable")
    end
    if successes >= 2 then
        return watcher_reward()
    end

    local outcome = WATCHER_ATTACK_OUTCOMES[successes]
    local scene = StoryState.scene()
    if outcome == nil or scene == nil then
        return source_boundary("watcher Eye-Fiver sequence is unavailable")
    end
    -- The source passes D0=1 to 0x1E1FC; the host resolves this map-derived
    -- list after the dialogue returns, like the haggard-man sequence.
    if not StoryState.queue_world_effect(scene, outcome) then
        -- ROM 0x1E1FC returns zero when the selected list cannot start.
        return watcher_reward()
    end
    game.ui.say_text(outcome.result_text, true, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)
    -- Records 0x120F34/0x120F48 continue to 0x120F5C, the thanks-and-fade page.
    game.ui.say_text(WATCHER_WAVE_RESULT, false, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)
    return "resolved"
end

local function company_man_roll(dice_pool, boundary_reason)
    local modules = type(game.modules) == "table" and game.modules or nil
    local combat = type(modules) == "table" and modules.combat or nil
    if type(combat) ~= "table" or
       type(combat.roll_thresholded_successes) ~= "function" then
        return nil, source_boundary(boundary_reason ..
                                   " needs the shared threshold-roll service")
    end
    local successes = combat.roll_thresholded_successes(
        dice_pool, COMPANY_MAN_TARGET, random_die)
    if successes == nil then
        return nil, source_boundary(boundary_reason ..
                                   " threshold roll is unavailable")
    end
    return successes
end

local function company_man_confrontation_roll(dice_pool)
    local modules = type(game.modules) == "table" and game.modules or nil
    local combat = type(modules) == "table" and modules.combat or nil
    if type(combat) ~= "table" or
       type(combat.roll_thresholded_successes) ~= "function" then
        return nil, source_boundary(
            "Company Man Talk needs the shared threshold-roll service")
    end
    local successes = combat.roll_thresholded_successes(
        dice_pool, COMPANY_MAN_CONFRONTATION_TARGET, random_die)
    if successes == nil then
        return nil, source_boundary(
            "Company Man Talk threshold roll is unavailable")
    end
    return successes
end

local function present_lone_star_patrol_stop_prompt()
    local opening, choices = event_prompt_data(
        "lone_star_patrol_stop", { "attack", "talk", "run_away" })
    if opening == nil or choices == nil then
        return nil, source_boundary(
            "ROM 0x120FE8 Lone Star prompt data is unavailable")
    end
    return game.ui.prompt_text(
        "lone_star_patrol_stop", opening, choices)
end

local function resolve_lone_star_patrol_attack()
    local scene = StoryState.scene()
    if scene == nil then
        return source_boundary(
            "ROM 0x23412 needs the active field scene for list 0x1D6FDC")
    end
    local sequence_id = E.WorldEffectSequence.LONE_STAR_PATROL_ATTACK
    local outcome = Encounters.resolve_sequence(scene, sequence_id)
    local expected_profiles = {
        "lone11_k05_item03",
        "lone12_k05_item07",
        "lone_detectiv_k05_item07",
    }
    if outcome == nil or #outcome.actors ~= #expected_profiles then
        return source_boundary(
            "ROM 0x23412 patrol actor list 0x1D6FDC is unavailable")
    end
    for order, profile_id in ipairs(expected_profiles) do
        if outcome.actors[order].profile_id ~= profile_id then
            return source_boundary(
                "ROM 0x23412 patrol actor list 0x1D6FDC has an unexpected template")
        end
    end

    local heat = StoryState.heat_level()
    if heat == nil then
        return source_boundary("ROM 0x23412 needs the current Lone Star heat")
    end
    local effect = { actors = #expected_profiles, sequence_id = sequence_id }
    if not StoryState.queue_world_effect(scene, effect) then
        game.ui.say_text(LONE_STAR_ATTACK_LIST_FAILURE, true)
        return "resolved"
    end
    if not StoryState.set_heat_level(math.min(
           StoryState.MAX_HEAT_LEVEL, heat + 2)) then
        return source_boundary(
            "ROM 0x23412 queued the patrol but could not add two heat")
    end
    game.ui.say_text(LONE_STAR_ATTACK_RESULT, true)
    return "resolved"
end

local function unresolved_lone_star_patrol_choice(choice)
    if choice == "attack" then
        return resolve_lone_star_patrol_attack()
    end
    if choice == "talk" then
        return source_boundary(
            "ROM 0x96 -> 0x23452 shared patrol comply is not implemented")
    end
    if choice == "run_away" then
        return source_boundary(
            "ROM 0x99 -> 0x23782 shared patrol chase is not implemented")
    end
    return source_boundary(
        "ROM 0x120FE8 returned an unavailable Lone Star choice")
end

function RandomEvents.present_lone_star_patrol_stop()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "lone_star_patrol_stop" then
        return nil
    end
    local choice, failure = present_lone_star_patrol_stop_prompt()
    if choice == nil then return failure end
    return unresolved_lone_star_patrol_choice(choice)
end

function RandomEvents.present_company_man_confrontation()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "company_man_confrontation" then
        return nil
    end

    local opening, choices = event_prompt_data(
        "company_man_confrontation", { "attack", "talk", "leave" })
    if opening == nil or choices == nil then
        return source_boundary(
            "Company Man confrontation prompt data is unavailable")
    end
    -- ROM 0x20F5A selects this separate three-choice record. Profile 0x121F40
    -- decodes to dialogue.portraits.club.penumbra.doorman [C]; the toothpick
    -- event uses its distinct profile at 0x121E2C.
    local choice = game.ui.prompt_text(
        "company_man_confrontation", opening, choices,
        COMPANY_MAN_CONFRONTATION_SPEAKER,
        COMPANY_MAN_CONFRONTATION_TEXT_PROFILE)

    if choice == "attack" then
        -- ROM 0x238EA selects 0x120FC8 for rolls 0-49. Rolls 50-99 reach
        -- actor list 0x1D7074 and the unresolved gun-draw handoff.
        local roll = random_die(100)
        if roll == nil then
            return source_boundary(
                "ROM 0x238EA requires a bounded-100 random result")
        end
        if roll < 50 then
            game.ui.say_text(
                COMPANY_MAN_CONFRONTATION_ATTACK_FLEE_RESULT, false,
                COMPANY_MAN_CONFRONTATION_SPEAKER,
                COMPANY_MAN_CONFRONTATION_TEXT_PROFILE)
            return "resolved"
        end
        return source_boundary(
            "ROM 0x238EA actor list 0x1D7074 gun-draw handoff is unresolved")
    end

    if choice == "talk" then
        -- ROM 0x23920 reads slot 0 +0x7A Charisma and rolls against target 5.
        local dice_pool = first_runner_attribute("charisma")
        if dice_pool == nil then
            return source_boundary(
                "Company Man Talk needs the first runner's Charisma")
        end
        local successes, failure = company_man_confrontation_roll(dice_pool)
        if successes == nil then return failure end
        if successes == 0 then
            local patrol_choice, failure =
                present_lone_star_patrol_stop_prompt()
            if patrol_choice == nil then return failure end
            return unresolved_lone_star_patrol_choice(patrol_choice)
        end
        if successes == 1 then
            return source_boundary(
                "ROM 0x23920 actor list 0x1D7074 gun-draw handoff is unresolved")
        end
        game.ui.say_text(
            COMPANY_MAN_CONFRONTATION_TALK_RESULT, false,
            COMPANY_MAN_CONFRONTATION_SPEAKER,
            COMPANY_MAN_CONFRONTATION_TEXT_PROFILE)
        return "resolved"
    end

    if choice == "leave" then
        -- ROM 0x23978: 0-39 returns 0x120FD0; 40-79 attempts list 0x1D7074;
        -- 80-99 opens the separate Lone Star menu at 0x120FE8.
        local roll = random_die(100)
        if roll == nil then
            return source_boundary(
                "ROM 0x23978 requires a bounded-100 random result")
        end
        if roll < 40 then
            game.ui.say_text(
                COMPANY_MAN_CONFRONTATION_LEAVE_RESULT, false,
                COMPANY_MAN_CONFRONTATION_SPEAKER,
                COMPANY_MAN_CONFRONTATION_TEXT_PROFILE)
            return "resolved"
        end
        if roll < 80 then
            return source_boundary(
                "ROM 0x23978 actor list 0x1D7074 gun-draw handoff is unresolved")
        end
        local patrol_choice, failure = present_lone_star_patrol_stop_prompt()
        if patrol_choice == nil then return failure end
        return unresolved_lone_star_patrol_choice(patrol_choice)
    end

    return source_boundary(
        "ROM 0x120FAC returned an unavailable Company Man choice")
end

function RandomEvents.present_company_man_toothpick()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "company_man_toothpick" then
        return nil
    end

    local opening, choices = event_prompt_data(
        "company_man_toothpick", { "attack", "insult", "leave" })
    if opening == nil or choices == nil then
        return source_boundary("Company Man prompt data is unavailable")
    end
    local choice = game.ui.prompt_text(
        "company_man_toothpick", opening, choices, COMPANY_MAN_SPEAKER,
        COMPANY_MAN_TEXT_PROFILE)
    if choice == "leave" then
        game.ui.say_text(COMPANY_MAN_LEAVE_RESULT, false,
                         COMPANY_MAN_SPEAKER, COMPANY_MAN_TEXT_PROFILE)
        return "resolved"
    end

    if choice == "attack" then
        -- ROM 0x23750 averages +0x78 across active party records; this is the
        -- same typed average Quickness fact used by the hidden-camera check.
        local dice_pool = party_check()
        if dice_pool == nil then
            return source_boundary("Company Man attack needs an active party")
        end
        local successes, failure = company_man_roll(
            dice_pool, "Company Man Attack")
        if successes == nil then return failure end
        if successes > 1 then
            if game.services.call("party", "award_karma_1") ~= "applied" then
                return source_boundary(
                    "Company Man death result needs the party Karma service")
            end
            game.ui.say_text(COMPANY_MAN_ATTACK_DEATH_RESULT, false,
                             COMPANY_MAN_SPEAKER,
                             COMPANY_MAN_TEXT_PROFILE)
            return "resolved"
        end
        if game.services.call("field", "raise_alarm") ~= "applied" then
            return source_boundary(
                "Company Man escape result needs the field alarm service")
        end
        game.ui.say_text(
            successes == 0 and COMPANY_MAN_ATTACK_FLEE_RESULT or
                COMPANY_MAN_ATTACK_WOUND_RESULT,
            false, COMPANY_MAN_SPEAKER, COMPANY_MAN_TEXT_PROFILE)
        return "resolved"
    end

    if choice == "insult" then
        -- ROM 0x2489A reads slot 0's +0x7A value directly; no party average.
        local dice_pool = first_runner_attribute("charisma")
        if dice_pool == nil then
            return source_boundary(
                "Company Man insult needs the first runner's Charisma")
        end
        local successes, failure = company_man_roll(
            dice_pool, "Company Man Insult")
        if successes == nil then return failure end
        if successes == 0 then
            if game.services.call("field", "raise_alarm") ~= "applied" then
                return source_boundary(
                    "Company Man Insult result needs the field alarm service")
            end
            game.ui.say_text(COMPANY_MAN_INSULT_SECURITY_RESULT, false,
                             COMPANY_MAN_SPEAKER,
                             COMPANY_MAN_TEXT_PROFILE)
            return "resolved"
        end
        if successes == 1 then
            game.ui.say_text(COMPANY_MAN_INSULT_LAUGH_RESULT, false,
                             COMPANY_MAN_SPEAKER,
                             COMPANY_MAN_TEXT_PROFILE)
            return "resolved"
        end

        local resource_variant = game.services.call(
            "field", COMPANY_MAN_RESOURCE_VARIANT_SERVICE)
        local clue = COMPANY_MAN_RESOURCE_CLUES[resource_variant]
        if clue == nil then
            return source_boundary(
                "Company Man floor clue needs the selected resource variant")
        end
        -- The source-selected label is a resource variant, not proof of the
        -- runner's physical floor. The shared 0x11CD92 page is terminal.
        game.ui.say_text(COMPANY_MAN_STARE_DOWN_RESULT .. "||" .. clue,
                         false, COMPANY_MAN_SPEAKER,
                         COMPANY_MAN_TEXT_PROFILE)
        return "resolved"
    end

    return source_boundary("Company Man choice is unavailable")
end

local function faction_actor_sequence_failed(reason)
    -- ROM 0x238C8 selects one of 20 optional resource bundles before it
    -- returns text record 0x120E20 (0x11CF74). The native shared text page
    -- represents that no-combat fallback; keep the source RNG draw here.
    if random_die(20) == nil then
        return source_boundary(
            reason .. "; ROM 0x238C8 fallback resource roll is unavailable")
    end
    game.ui.say_text(FACTION_LIST_FAILURE_RESULT, false)
    return "resolved"
end

local function security_guard_dispatch_result()
    -- The MORE/CE continuation at 0x121254 and the one-success Talk route
    -- both reach ROM 0x24392.
    local dispatch_roll = random_die(100, "security_guard_dispatch")
    if dispatch_roll == nil then
        return source_boundary(
            "ROM 0x24392 requires the shared bounded-100 random service")
    end
    if dispatch_roll >= 40 then
        game.ui.say_text(SECURITY_GUARD_TALK_DISPATCH_SUCCESS_RESULT, false)
        return "resolved"
    end

    local scene = StoryState.scene()
    if scene == nil or scene % 1 ~= 0 or scene < 0 or
       scene > E.Scene.LAST_ID then
        return source_boundary(
            "ROM 0x24392 needs the active scene to queue list 0x1D705C")
    end
    local sequence_id = E.WorldEffectSequence.SECURITY_GUARD_STOP_ATTACK
    if not StoryState.queue_world_effect(scene, {
           actors = 1,
           sequence_id = sequence_id,
       }) then
        return source_boundary(
            "ROM 0x24392 could not queue one guard from list 0x1D705C")
    end
    game.ui.say_text(SECURITY_GUARD_TALK_DISPATCH_ALERT_RESULT, false)
    return "resolved"
end

function RandomEvents.present_faction_disturbance()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "faction_disturbance" then
        return nil
    end

    local opening = event_opening("faction_disturbance")
    if opening == nil then
        return source_boundary(
            "ROM 0x20F42 prompt data is unavailable")
    end

    -- The source prompt at 0x120F6C contains one MORE command. Its handler
    -- 0x23876 checks FF018F bit 1 first, then bit 2. The no-flag branch at
    -- 0x238C2 is a text-only result and does not start combat.
    game.ui.say_text(opening, true)
    local mafia_route = game.services.call(
        "story", "faction_disturbance_mafia_route_enabled")
    if mafia_route ~= "enabled" and mafia_route ~= "disabled" then
        return source_boundary(
            "ROM 0x23876 faction route flags are unavailable")
    end
    if mafia_route == "enabled" then
        local scene = StoryState.scene()
        local sequence_id = E.WorldEffectSequence.FACTION_MAFIA_EYE_FIVERS
        local outcome = scene ~= nil and
            Encounters.resolve_sequence(scene, sequence_id) or nil
        if outcome == nil or #outcome.actors ~= 4 then
            return faction_actor_sequence_failed(
                "ROM 0x23876 four-actor Mafia sequence is unavailable")
        end
        if not StoryState.queue_world_effect(scene, {
               sequence_id = outcome.sequence_id,
               actors = #outcome.actors,
           }) then
            return faction_actor_sequence_failed(
                "ROM 0x23876 Mafia actor sequence could not be queued")
        end
        -- 0x23876 chooses the Eye-Fiver record only after attempting the
        -- ordered four-actor map placement through 0x1E1FC.
        game.ui.say_text(FACTION_MAFIA_RESULT, false)
        return "resolved"
    end

    local halloweener_route = game.services.call(
        "story", "faction_disturbance_halloweener_route_enabled")
    if halloweener_route ~= "enabled" and
       halloweener_route ~= "disabled" then
        return source_boundary(
            "ROM 0x23876 faction route flags are unavailable")
    end
    if halloweener_route == "enabled" then
        local scene = StoryState.scene()
        local sequence_id = E.WorldEffectSequence.FACTION_HALLOWEENERS
        local outcome = scene ~= nil and
            Encounters.resolve_sequence(scene, sequence_id) or nil
        if outcome == nil or #outcome.actors ~= 4 then
            return faction_actor_sequence_failed(
                "ROM 0x23876 four-actor Halloweener sequence is unavailable")
        end
        if not StoryState.queue_world_effect(scene, {
               sequence_id = outcome.sequence_id,
               actors = #outcome.actors,
           }) then
            return faction_actor_sequence_failed(
                "ROM 0x23876 Halloweener actor sequence could not be queued")
        end
        -- Bit 2 is reached only when bit 1 is clear. The matching four-entry
        -- actor list and result record are selected at 0x1D7038/0x11D019.
        game.ui.say_text(FACTION_HALLOWEENER_RESULT, false)
        return "resolved"
    end

    game.ui.say_text(FACTION_NEUTRAL_RESULT, true)
    game.ui.say_text(FACTION_NEUTRAL_CONTINUATION, false)
    return "resolved"
end

function RandomEvents.present_haggard_man()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "haggard_man_collapse" then
        return nil
    end

    local opening = event_opening("haggard_man_collapse")
    if opening == nil then
        return source_boundary(
            "ROM 0x20FA2 doctor opening data is unavailable")
    end

    game.ui.say_text(opening, true, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)

    local intelligence = first_runner_attribute("intelligence")
    local modules = type(game.modules) == "table" and game.modules or nil
    local combat = type(modules) == "table" and modules.combat or nil
    if intelligence == nil or type(combat) ~= "table" or
       type(combat.roll_thresholded_successes) ~= "function" then
        return source_boundary(
            "ROM 0x23BB4 needs the first runner's Intelligence and dice service")
    end
    -- FF017B is the first runner record's Intelligence (+0x7B). The ROM
    -- checks that pool against 6 before its separate 0-99 prompt roll.
    local successes = combat.roll_thresholded_successes(
        intelligence, 6, random_die)
    if successes == nil then
        return source_boundary("ROM 0x23BB4 Intelligence roll is unavailable")
    end
    local selector_roll = random_die(100)
    if selector_roll == nil then
        return source_boundary("ROM 0x23BB4 prompt roll is unavailable")
    end
    local prompt_variant = selector_roll < 50 and "alternate" or "standard"
    local choice = game.ui.prompt_text(
        "haggard_man_action", haggard_intro_text(successes, selector_roll),
        HAGGARD_CHOICES, HAGGARD_SPEAKER, HAGGARD_TEXT_PROFILE)

    if prompt_variant == "standard" then
        if choice == "attack" then
            -- The first command at 0x12111C returns data pointer 0x121180;
            -- the record's next interaction is still an explicit boundary.
            return source_boundary(
                "ROM 0x12111C Attack returns through prompt data 0x121180")
        end
        if choice == "threaten" then
            -- Command 0xA9 reaches 0x23C38. Its 0-49 result pays 50 via
            -- 0x23CE0; 50-99 enters the police prompt at 0x12115C.
            local roll = random_die(100)
            if roll == nil then
                return source_boundary(
                    "ROM 0x23C38 needs a valid 0-99 random result")
            end
            if roll >= 50 then
                return source_boundary(
                    "ROM 0x23C38 patrol prompt at 0x12115C is not implemented")
            end
            return haggard_reward(
                50, HAGGARD_CREDENTIALS_RESULT,
                "ROM 0x23CE0 reward needs the current nuyen value")
        end
        if choice == "help" then
            -- Command 0xAC directly reaches 0x23CE0 in this prompt variant.
            return haggard_reward(
                50, HAGGARD_CREDENTIALS_RESULT,
                "ROM 0x23CE0 reward needs the current nuyen value")
        end
    elseif choice == "attack" then
        -- Command 0xAA reaches 0x23C52 and checks first-runner Body at
        -- FF0177 against target 6. The <=1-success branches queue their
        -- source actor lists; the >1 path pays 100 and uses 0x1211A0.
        local body = first_runner_attribute("body")
        if body == nil then
            return source_boundary(
                "ROM 0x23C52 needs the first runner's Body")
        end
        local attack_successes = combat.roll_thresholded_successes(
            body, 6, random_die)
        if attack_successes == nil then
            return source_boundary("ROM 0x23C52 Body roll is unavailable")
        end
        if attack_successes <= 1 then
            local outcome = HAGGARD_ATTACK_OUTCOMES[attack_successes]
            local scene = StoryState.scene()
            if outcome == nil or scene == nil then
                return source_boundary(
                    "ROM 0x23C52 combat sequence or field scene is unavailable")
            end
            -- ROM 0x23C52 calls 0x1E1FC with D0=1, which places the selected
            -- Gator Shaman/bodyguard list at map-derived candidates before
            -- returning the corresponding 0x121190 or 0x121198 page.
            if not StoryState.queue_world_effect(scene, outcome) then
                return source_boundary(
                    "ROM 0x23C52 map-derived actor list could not be queued")
            end
            game.ui.say_text(outcome.result_text, false, HAGGARD_SPEAKER,
                             HAGGARD_TEXT_PROFILE)
            return "resolved"
        end
        return haggard_reward(
            100, HAGGARD_THREATEN_RESULT,
            "ROM 0x23C74 reward needs the current nuyen value")
    elseif choice == "threaten" then
        -- Command 0xAB reaches 0x23CB4. Its 0-49 result adds 0x64 to
        -- FFFBFE and displays 0x1211A0; 50-99 enters the patrol prompt
        -- at 0x12115C, which remains an explicit boundary.
        local roll = random_die(100)
        if roll == nil then
            return source_boundary(
                "ROM 0x23CB4 needs a valid 0-99 random result")
        end
        if roll >= 50 then
            return source_boundary(
                "ROM 0x23CB4 patrol prompt at 0x12115C is not implemented")
        end

        return haggard_reward(
            100, HAGGARD_THREATEN_RESULT,
            "ROM 0x23CCE reward needs the current nuyen value")
    elseif choice == "help" then
        -- Command 0xAD reaches 0x23CF2 and pays 50 nuyen.
        return haggard_reward(
            50, HAGGARD_HELP_RESULT,
            "ROM 0x23CF2 reward needs the current nuyen value")
    end

    return source_boundary(
        "ROM 0x23BB4 choice continuation is not implemented: " ..
        tostring(choice))
end

function RandomEvents.present_security_guard_stop()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "security_guard_stop" then
        return nil
    end

    -- ROM 0x20FEC supplies the guard message and its MORE command to the
    -- common 0x1F246 text service. The event content table owns the text;
    -- FieldDialogueRuntime owns page input and the same-scene return.
    local opening = event_opening("security_guard_stop")
    if opening == nil then
        return source_boundary(
            "ROM 0x20FEC guard message content is unavailable")
    end
    game.ui.say_text(opening, true)

    -- With FFFC22 bit 10 clear, ROM 0x242FA returns the no-ID record
    -- 0x121278. Its D2/D0 commands enter 0x24402/0x24432 respectively.
    local state = game.state
    local has_security_pass
    if type(state) == "table" then
        has_security_pass = state["group_item.security_pass"]
    end
    if type(has_security_pass) ~= "boolean" then
        return source_boundary(
            "ROM 0x242FA requires FFFC22 bit 10; security-pass state is unavailable")
    end
    if has_security_pass then
        -- ROM 0x2430C draws in [0, 100); rolls below 20 return 0x121254.
        local badge_roll = random_die(100, "security_guard_badge")
        if badge_roll == nil then
            return source_boundary(
                "ROM 0x242FA requires the shared bounded-100 random service")
        end
        if badge_roll >= 20 then
            game.ui.say_text(SECURITY_GUARD_BADGE_PASS_RESULT, false)
            return "resolved"
        end
        -- 0x121254 has MORE followed by command CE, which invokes 0x24392.
        game.ui.say_text(SECURITY_GUARD_BADGE_VERIFY_RESULT, true)
        return security_guard_dispatch_result()
    end
    local choice = game.ui.prompt_text(
        "security_guard_no_id", SECURITY_GUARD_NO_ID_PROMPT,
        SECURITY_GUARD_NO_ID_CHOICES)
    if choice == "talk" then
        -- ROM 0x24432 reads slot 0 +0x7A. The source attribute sequence
        -- begins Body at +0x77 and ends Willpower at +0x7C (0xD1AA); +0x7A
        -- is Charisma, exposed by the named runner fact [C].
        local party = type(game.party) == "function" and game.party() or nil
        local first_runner = type(party) == "table" and party[0] or nil
        local attributes = type(first_runner) == "table" and
            first_runner.attributes or nil
        local charisma = type(attributes) == "table" and
            attributes.charisma or nil
        if type(charisma) ~= "number" or charisma % 1 ~= 0 or
           charisma < 0 or charisma > 0xFF then
            return source_boundary(
                "ROM 0x24432 requires slot 0 +0x7A Charisma")
        end

        local combat = game.modules and game.modules.combat or nil
        if type(combat) ~= "table" or
           type(combat.roll_thresholded_successes) ~= "function" then
            return source_boundary(
                "ROM 0x24432 requires the shared threshold-roll service")
        end
        local successes = combat.roll_thresholded_successes(
            charisma, 6, random_die)
        if successes == nil then
            return source_boundary(
                "ROM 0x24432 Charisma threshold roll is unavailable")
        end
        if successes == 0 then
            local scene = StoryState.scene()
            if scene == nil or scene % 1 ~= 0 or scene < 0 or
               scene > E.Scene.LAST_ID then
                return source_boundary(
                    "ROM 0x24432 needs the active scene to queue list 0x1D705C")
            end
            local sequence_id = E.WorldEffectSequence.SECURITY_GUARD_STOP_ATTACK
            if not StoryState.queue_world_effect(scene, {
                   actors = 1,
                   sequence_id = sequence_id,
               }) then
                return source_boundary(
                    "ROM 0x24432 could not queue one guard from list 0x1D705C")
            end
            game.ui.say_text(SECURITY_GUARD_TALK_ZERO_SUCCESS_RESULT, false)
            return "resolved"
        end
        if successes > 1 then
            -- 0x1212B4 is exactly the terminal text record for 0x11F378;
            -- the next, independent mage record starts at 0x1212BC.
            game.ui.say_text(SECURITY_GUARD_TALK_SUCCESS_RESULT, false)
            return "resolved"
        end
        if successes == 1 then
            -- Record 0x1212A0 supplies a MORE label at 0xFE5BF. The shared
            -- text operation yields here until CE; only after resumption does
            -- command 0xCE invoke ROM 0x24392's bounded-100 check.
            game.ui.say_text(SECURITY_GUARD_TALK_ONE_SUCCESS_RESULT, true)
            return security_guard_dispatch_result()
        end
        return source_boundary(
            "ROM 0x24432 returned an unexpected negative Talk success count")
    end
    if choice ~= "attack" then
        return source_boundary(
            "ROM 0x121278 selected an unsupported no-ID command: " ..
            tostring(choice))
    end

    local scene_reference = game.services.call("field", "scene_reference")
    local scene_id_text = type(scene_reference) == "string" and
        scene_reference:match("^scene_(%d+)$") or nil
    local scene_id = scene_id_text and tonumber(scene_id_text) or nil
    if scene_id == nil or scene_id % 1 ~= 0 or scene_id < 0 or
       scene_id > E.Scene.LAST_ID then
        return source_boundary(
            "ROM 0x24402 needs the active field scene for list 0x1D705C")
    end

    local sequence_id = E.WorldEffectSequence.SECURITY_GUARD_STOP_ATTACK
    local queued = game.services.call(
        "story", "queue_world_effect_" .. scene_id .. "_1_" .. sequence_id)
    if queued ~= "queued" then
        return source_boundary(
            "ROM 0x24402 list 0x1D705C could not enter the shared actor queue")
    end

    -- ROM 0x24402 returns 0x121290 after forwarding the single template.
    -- Combat entry/exit remains a separate source boundary.
    game.ui.say_text(SECURITY_GUARD_ATTACK_RESULT, false)
    return source_boundary(
        "ROM 0x24402 queued the 0x1D6A1C guard and returned 0x121290; combat handoff remains unresolved")
end

return RandomEvents
