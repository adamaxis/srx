-- ROM title service 10 (0x53F8C), Scene 2/event 6.
-- The handler loads profile 0x121F58 and runs the indexed dialogue at
-- 0x1F246. Its D6=-1 return suppresses event-table Scene 26, so every
-- completed branch remains on the Redmond Barrens map.
local Boris = {}

local LOCATION_ID = "boris_greenhouse_frontage"
local TEXT_PROFILE = "boris.greenhouse"

local function at_greenhouse()
    return game.location.type == "building" and
           game.location.id == LOCATION_ID and
           game.field.event_code == 6
end

local function more(text_key)
    return game.ui.prompt_text_key("more", text_key, { "more" }) == "more"
end

local function page(menu_id, text_key, choices)
    return game.ui.prompt_text_key(menu_id, text_key, choices)
end

local function state_fact(name)
    return game.state[name]
end

local function dialogue_flag(bit)
    return state_fact("field_dialogue_flag_bit_" .. tostring(bit))
end

local function dialogue_flags_ready()
    for bit = 0, 3 do
        if dialogue_flag(bit) == nil then return false end
    end
    return true
end

local function topic_facts_ready()
    local facts = {
        "story_flag_bit_1", "story_flag_bit_3", "story_flag_bit_9",
        "story_flag_bit_18", "story_flag_bit_19", "group_item_flag_bit_5",
    }
    for _, fact in ipairs(facts) do
        if state_fact(fact) == nil then return false end
    end
    return true
end

local function insufficient_funds()
    game.ui.say("insufficient_funds")
    return "field_return"
end

local function purchase(amount, dialogue_flag_bit)
    local result = game.services.call("building", "charge_" .. amount)
    if result == "insufficient" then return insufficient_funds() end
    if result ~= "paid" then return "boris_payment_unavailable" end
    game.state["field_dialogue_flag_bit_" .. dialogue_flag_bit] = true
    return "paid"
end

local function main_menu(text_key, menu_id)
    local choice = page(menu_id or "opening", text_key, {
        "story", "information", "later",
    })
    if choice == "story" then return Boris.present_story_offer() end
    if choice == "information" then return Boris.present_information_offer(false) end
    return "field_return"
end

local function topic_choices()
    if not topic_facts_ready() then return nil end
    local choices = { "michael" }
    if state_fact("story_flag_bit_1") == true and
       state_fact("story_flag_bit_3") == false then
        choices[#choices + 1] = "david"
    elseif state_fact("story_flag_bit_18") == true and
           state_fact("group_item_flag_bit_5") == false and
           state_fact("story_flag_bit_19") == false then
        choices[#choices + 1] = "ilene"
    elseif state_fact("story_flag_bit_9") == true then
        choices[#choices + 1] = "thon"
    end
    choices[#choices + 1] = "later"
    return choices
end

local function present_topics(first_payment)
    local choices = topic_choices()
    if choices == nil then return "boris_dialogue_state_unavailable" end
    local text_key = first_payment
        and "boris.greenhouse.page.paid_information"
        or "boris.greenhouse.page.paid_information_repeat"
    local choice = page("information_topics", text_key, choices)

    if choice == "michael" then
        if state_fact("story_flag_bit_1") == false then
            return main_menu("boris.greenhouse.page.michael_unknown")
        elseif state_fact("story_flag_bit_10") == true then
            return main_menu("boris.greenhouse.page.michael_repeat")
        end

        -- Command 0x2B at 0x21B82 learns Michael's alias and prepares the
        -- source field-mode state for the next same-map service visit.
        game.state.story_flag_bit_10 = true
        game.state.field_dialogue_flag_bit_1 = true
        game.state.field_mode_flag_bit_6 = false
        game.state.field_mode_flag_bit_7 = false
        game.state.field_mode_flag_bit_20 = true
        more("boris.greenhouse.page.michael_learned")
        return "field_return"
    elseif choice == "david" then
        -- Command 0x3B at 0x21EB0 updates the same FFFC30 mode bits. The
        -- following Brother? choice reaches command 0x2C at 0x21BE4, whose
        -- Strike Team/world effect has not yet been converted.
        game.state.field_mode_flag_bit_6 = false
        game.state.field_mode_flag_bit_7 = false
        game.state.field_mode_flag_bit_20 = true
        page("david_followup", "boris.greenhouse.page.david_information",
             { "brother" })
        return "boris_strike_team_unconverted"
    elseif choice == "ilene" then
        -- Command 0x2A at 0x21B6A sets FFFC34 bit 0 before this root.
        game.state.field_mode_effect_flag_bit_0 = true
        return main_menu("boris.greenhouse.page.ilene_information")
    elseif choice == "thon" then
        more("boris.greenhouse.page.thon_information")
    end
    return "field_return"
end

function Boris.present_story_offer()
    if dialogue_flag(0) == true then
        more("boris.greenhouse.page.story_answer")
        return "field_return"
    end
    local choice = page("story_payment", "boris.greenhouse.page.story_prompt",
                        { "accept", "later" })
    if choice ~= "accept" then return "field_return" end
    local result = purchase(100, 0)
    if result ~= "paid" then return result end
    more("boris.greenhouse.page.story_answer")
    return "field_return"
end

function Boris.present_information_offer(already_paid)
    if already_paid or dialogue_flag(2) == true then
        return present_topics(false)
    end
    local choice = page("information_payment",
                        "boris.greenhouse.page.information_prompt",
                        { "accept", "later" })
    if choice ~= "accept" then return "field_return" end
    local result = purchase(500, 2)
    if result ~= "paid" then return result end
    return present_topics(true)
end

function Boris.present_indexed_title_service()
    if not at_greenhouse() then return "denied" end
    if not dialogue_flags_ready() then
        return "boris_dialogue_state_unavailable"
    end

    -- FFF0D5 selects the package request/payment records before the ordinary
    -- opening dialogue. Those records return directly to the same map.
    if game.field.alternate_result > 0 then
        game.ui.say("package_request")
        return "field_return"
    elseif game.field.alternate_result < 0 then
        game.ui.say("package_payment")
        more("boris.greenhouse.page.followup")
        return "field_return"
    end

    if dialogue_flag(1) == true then
        game.state.field_dialogue_flag_bit_1 = false
        more("boris.greenhouse.page.alternate_0")
        return main_menu("boris.greenhouse.page.after_michael")
    end

    if not more("boris.greenhouse.page.default") then
        return "field_return"
    end
    if dialogue_flag(3) == true then
        game.state.field_dialogue_flag_bit_3 = false
        return main_menu("boris.greenhouse.page.repeat", "repeat_opening")
    end
    return main_menu("boris.greenhouse.page.opening")
end

return Boris
