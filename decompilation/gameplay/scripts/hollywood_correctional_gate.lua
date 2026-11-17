-- ROM scene-2/event-0: pages 0x1113D2/0x1113E6, commands 0x64/0x65.
local E = require("enums")
local Sites = require("data.hollywood_sites")
local Combat = require("combat")
local StoryState = require("story_state")

local Gate = {}

local function site_context()
    local location = game.location
    local site = type(location) == "table" and
        location.type == "building" and Sites[location.id] or nil
    assert(site ~= nil,
        "Hollywood gate dialogue ran outside a linked facility entrance")
    return site
end

local function speaker_for(site, event_id)
    if event_id == site.events.sarge_pass then
        return site.sarge_speaker
    end
    return site.speaker
end

local function say(site, event_id)
    game.ui.say(event_id, speaker_for(site, event_id), site.text_profile)
end

local function access_flag(bit)
    return "field_route_flag_bit_" .. bit
end

local function present_choices(site, menu_id, prompt_key)
    return game.ui.prompt_text_key(
        menu_id, prompt_key,
        { "attack", "story", "leave" }, site.speaker, site.text_profile)
end

local function random_die(bound)
    local response = game.services.call("random", "bounded_" .. bound)
    local digits = type(response) == "string" and
        response:match("^roll_(%d+)$") or nil
    local value = digits and tonumber(digits) or nil
    if value == nil or value % 1 ~= 0 or value < 0 or value >= bound then
        return nil
    end
    return value
end

local function attack_guard_post(site)
    local party = type(game.party) == "function" and game.party() or nil
    local slot = type(game.active_slot) == "function" and
        game.active_slot() or nil
    local runner = type(party) == "table" and type(slot) == "number" and
        party[slot] or nil
    local successes = runner ~= nil and
        Combat.roll_equipped_weapon_skill_successes(
            runner, E.HollywoodGateAttack.ROLL_DIFFICULTY, random_die) or nil

    if successes == nil or
       successes < E.HollywoodGateAttack.MINIMUM_ENTRY_SUCCESSES then
        -- ROM 0x2277A calls 0x1E1FC with D0=1 for list 0x1D6FEC. A zero
        -- placement result falls through to 0x227A8 and the entry page.
        local placement = StoryState.place_world_effect(
            E.Scene.HOLLYWOOD_CORRECTIONAL, {
                actors = E.WorldEffectSequenceActorCount
                    .HOLLYWOOD_CORRECTIONAL_FAILED_SHOT_GUARDS,
                sequence_id = E.WorldEffectSequence
                    .HOLLYWOOD_CORRECTIONAL_FAILED_SHOT_GUARDS,
            })
        if placement == "no_placement" then
            game.state[access_flag(E.FieldRouteFlagBit.SCENE_20_ENTRY)] = true
            say(site, site.events.attack_success)
            return "enter_scene_20"
        elseif placement == "spawned" then
            -- The typed FFE182 owner is Ilene's pending world-effect fact.
            game.state.pending_world_effect = false
            say(site, site.events.attack_missed)
            return "left"
        end
        return "left"
    end

    -- ROM 0x227CA..0x227D4 sets only FFFC2C bit 26 after a successful roll.
    game.state[access_flag(E.FieldRouteFlagBit.SCENE_20_ENTRY)] = true
    if successes >= E.HollywoodGateAttack.EASY_AMBUSH_SUCCESSES then
        say(site, site.events.attack_ambush)
    else
        say(site, site.events.attack_success)
    end
    return "enter_scene_20"
end

function Gate.present_entry()
    local site = site_context()
    say(site, site.events.entrance)

    -- Command 0x64 selects package pages before checking either access bit.
    local package_status = game.field.package_run_status
    if package_status == E.PackageRunStatus.PAYMENT_DUE then
        say(site, site.events.package_payment)
        return "left"
    elseif package_status == E.PackageRunStatus.HANDOFF_DUE then
        say(site, site.events.package_handoff)
        return "left"
    elseif package_status ~= E.PackageRunStatus.NONE then
        return "left"
    end

    local badge_bit = E.FieldRouteFlagBit.HOLLYWOOD_CLEARANCE_BADGE
    if game.state[access_flag(badge_bit)] == true then
        game.state[access_flag(badge_bit)] = false
        say(site, site.events.sarge_pass)
        return "enter_scene_20"
    end

    local false_name_bit = E.FieldRouteFlagBit.HOLLYWOOD_FALSE_NAME
    if game.state[access_flag(false_name_bit)] == true then
        game.state[access_flag(false_name_bit)] = false
        say(site, site.events.false_name_pass)
        return "enter_scene_20"
    end

    local menu_id = site.menus.guard
    local prompt_key = site.prompts.guard
    while true do
        local choice = present_choices(site, menu_id, prompt_key)
        if choice == "attack" then
            return attack_guard_post(site)
        elseif choice == "story" then
            say(site, site.events.story_first)
            menu_id = site.menus.story
            prompt_key = site.prompts.story
        elseif choice == "leave" then
            say(site, site.events.farewell)
            return "left"
        else
            return "left"
        end
    end
end

return Gate
