package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local E = require("enums")

local function load_json(path)
    local file = assert(io.open(path, "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local entity = load_json(
    "decompilation/gameplay/world/entities/council_passport_lodge/interactions.json")
local interaction
for _, candidate in ipairs(entity.interactions) do
    if candidate.source.sceneId == 4 and
       candidate.source.eventCode == "event_2" then
        interaction = candidate
        break
    end
end
assert(interaction ~= nil, "Council Passport Lodge event 2 is not registered")
local task = interaction.tasks and interaction.tasks.fieldExitPrompt
assert(task ~= nil,
    "Council Passport Lodge event 2 must bind its field-exit prompt task")
assert(task.module == "council_passport_lodge" and
       task["function"] == "present_field_exit_prompt")
assert(interaction.locationContext.type == "building" and
       interaction.locationContext.id == "council_passport_lodge" and
       interaction.locationContext.speakerId ==
           "council_passport_lodge_clerk")

local speakers = load_json("decompilation/gameplay/dialogue/speakers.json")
local clerk = assert(speakers.profiles.council_passport_lodge_clerk)
assert(clerk.portrait_alias ==
       "dialogue.portraits.vid.phone.professor.jefferson" and
       clerk.text_profile == "council_medicine_lodge",
       "Lodge clerk must use the source portrait and site text profile")

local Lodge = require(task.module)
local host
local queued_choices
local prompts
local speeches
local timeline

local function reset(package_status, funds, heat, choices)
    host = {
        package_status = package_status,
        funds = funds,
        heat = heat,
        heat_reads = 0,
    }
    queued_choices = choices or {}
    prompts = {}
    speeches = {}
    timeline = {}
    local state = {}
    for _, item_id in pairs(E.GroupItemId) do
        state["group_item." .. item_id] =
            item_id ~= E.GroupItemId.INDIAN_LANDS_VISA
    end
    game = {
        location = {
            type = interaction.locationContext.type,
            id = interaction.locationContext.id,
        },
        field = { package_run_status = package_status },
        state = state,
        nuyen = function() return host.funds end,
        set_nuyen = function(value) host.funds = value end,
        services = {
            call = function(domain, operation)
                assert(domain == "story" and operation == "get_heat_level",
                    "unexpected Passport Lodge service: " ..
                    tostring(domain) .. "." .. tostring(operation))
                host.heat_reads = host.heat_reads + 1
                return host.heat and ("level_" .. host.heat) or nil
            end,
        },
        ui = {
            say_text = function(text, continuation, speaker_id, text_profile)
                assert(speaker_id == "council_passport_lodge_clerk")
                assert(text_profile == "council_medicine_lodge")
                speeches[#speeches + 1] = {
                    text = text,
                    continuation = continuation,
                }
                timeline[#timeline + 1] = {
                    kind = "say",
                    text = text,
                }
                return true
            end,
            prompt_text = function(menu_id, text, choices_for_prompt,
                                   speaker_id, text_profile)
                assert(speaker_id == "council_passport_lodge_clerk")
                assert(text_profile == "council_medicine_lodge")
                prompts[#prompts + 1] = {
                    menu_id = menu_id,
                    text = text,
                    choices = choices_for_prompt,
                }
                timeline[#timeline + 1] = {
                    kind = "prompt",
                    text = text,
                }
                local selected = table.remove(queued_choices, 1)
                assert(selected ~= nil,
                    "unexpected Passport Lodge prompt: " .. menu_id)
                for _, choice in ipairs(choices_for_prompt) do
                    assert(type(choice) == "table" and
                           type(choice.label) == "string")
                    if choice.id == selected then return selected end
                end
                error("choice " .. selected .. " is unavailable in " ..
                      menu_id)
            end,
        },
    }
end

local function assert_choices(prompt, expected)
    assert(#prompt.choices == #expected)
    for index, choice in ipairs(prompt.choices) do
        assert(choice.id == expected[index][1] and
               choice.label == expected[index][2],
               "Passport Lodge choice order/text differs at " .. index)
    end
end

local function assert_timeline(expected)
    assert(#timeline == #expected, "unexpected Passport Lodge page count")
    for index, page in ipairs(expected) do
        assert(timeline[index].kind == page[1] and
               timeline[index].text == page[2],
               "Passport Lodge page order differs at " .. index)
    end
end

local function assert_visa_unchanged(expected)
    for _, item_id in pairs(E.GroupItemId) do
        assert(game.state["group_item." .. item_id] == expected[item_id],
            "unexpected group-item change: " .. item_id)
    end
end

local initial_group_items = {}
for _, item_id in pairs(E.GroupItemId) do
    initial_group_items[item_id] =
        item_id ~= E.GroupItemId.INDIAN_LANDS_VISA
end

local entry_text = "<The Passport Lodge on Council Island serves as a " ..
    "visiting center for tourists.||All sorts of cheap items are for sale " ..
    "here, but nothing of value.||An AmerIndian clerk in shirt-sleeves " ..
    "and tie notices you and comes over.>"
local price_text = "<The clerk eyes you like something the cat dragged in.>||" ..
    "A passport costs 500$, which includes a security check to prove you " ..
    "are not fleeing justice."
local contract_text = "Be aware, you are signing a binding contract to abide " ..
    "by tribal law while in the Salish-Shidhe.||Any deviance from the " ..
    "tribal law will revoke all privileges.||If you hand me your credstick " ..
    "I can give you the forms to fill out."
local pay_label = "Easy money, chummer.  I'll pay."
local leave_label = "I'll talk to you later, chum."
local goodbye_text = "Enjoy the beauty of the island, good sir."
local low_funds_text = "<You slot your credstick, but the account balance " ..
    "is under 500$.||The clerk gives you a disapproving look and moves on.>"
local forms_text = "<You fill out the forms, and hand them back.||The clerk " ..
    "feeds them into the computer, and soon the forms come back.||The clerk " ..
    "looks them over, and his eyes widen.>"
local warrant_text = "I'm sorry sir, but there appears to be a warrant out " ..
    "for your arrest.||As this is neutral territory, I suggest you leave " ..
    "before I am forced to call security."
local success_text = "<The forms are filled out and go through without a " ..
    "hitch.>||Remember, this passport is only good if you obey the tribal " ..
    "laws.||Any slip-ups and you will be escorted out and Lone Star " ..
    "apprised of your misconduct."
local success_end_text = "Enjoy your visit to the Salish-Shidhe."
local pickup_text = "<You explain you are here to pick up a package.||He gives " ..
    "it to you and you are on your way.>"
local delivery_text = "<You explain you have a package for him.||He accepts it " ..
    "and pays you.>"

-- Page order through the no-run entry, price page, purchase choice, and leave.
reset("none", 500, 2, { "purchase", "leave" })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 2 and #speeches == 2)
assert(prompts[1].menu_id == "passport_lodge_entry" and
       prompts[1].text == entry_text)
assert_choices(prompts[1], {
    { "purchase", "I would like to purchase a passport to the AmerIndian Lands." },
    { "leave", leave_label },
})
assert(speeches[1].text == price_text and
       speeches[1].continuation == true)
assert(prompts[2].menu_id == "passport_lodge_purchase" and
       prompts[2].text == contract_text)
assert_choices(prompts[2], {
    { "pay", pay_label },
    { "leave", leave_label },
})
assert(speeches[2].text == goodbye_text and
       speeches[2].continuation == false)
assert_timeline({
    { "prompt", entry_text },
    { "say", price_text },
    { "prompt", contract_text },
    { "say", goodbye_text },
})
assert(host.funds == 500 and host.heat_reads == 0)
assert_visa_unchanged(initial_group_items)

-- Leaving from the first choice also selects the same closing page.
reset("none", 500, 2, { "leave" })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 1 and #speeches == 1)
assert(prompts[1].text == entry_text)
assert(speeches[1].text == goodbye_text and
       speeches[1].continuation == false)
assert_timeline({
    { "prompt", entry_text },
    { "say", goodbye_text },
})
assert(host.funds == 500 and host.heat_reads == 0)
assert_visa_unchanged(initial_group_items)

-- Command 0x5E's wallet check occurs before its heat check. Short funds do not
-- debit or set the visa.
reset("none", 499, 2, { "purchase", "pay" })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 2 and #speeches == 2)
assert(speeches[1].text == price_text and speeches[1].continuation == true)
assert(speeches[2].text == low_funds_text and
       speeches[2].continuation == false)
assert_timeline({
    { "prompt", entry_text },
    { "say", price_text },
    { "prompt", contract_text },
    { "say", low_funds_text },
})
assert(host.funds == 499 and host.heat_reads == 0)
assert_visa_unchanged(initial_group_items)

-- At the signed-byte threshold, command 0x5E returns the two warrant pages
-- without debiting the wallet or setting the visa.
reset("none", 500, 3, { "purchase", "pay" })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 2 and #speeches == 3)
assert(speeches[1].text == price_text and
       speeches[1].continuation == true)
assert(speeches[2].text == forms_text and
       speeches[2].continuation == true)
assert(speeches[3].text == warrant_text and
       speeches[3].continuation == false)
assert_timeline({
    { "prompt", entry_text },
    { "say", price_text },
    { "prompt", contract_text },
    { "say", forms_text },
    { "say", warrant_text },
})
assert(host.funds == 500 and host.heat_reads == 1)
assert_visa_unchanged(initial_group_items)

-- Below the threshold, the purchase sets only the named visa item and charges
-- 500; the other group-item states remain intact.
reset("none", 500, 2, { "purchase", "pay" })
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 2 and #speeches == 3)
assert(speeches[1].text == price_text and
       speeches[1].continuation == true)
assert(speeches[2].text == success_text and
       speeches[2].continuation == true)
assert(speeches[3].text == success_end_text and
       speeches[3].continuation == false)
assert_timeline({
    { "prompt", entry_text },
    { "say", price_text },
    { "prompt", contract_text },
    { "say", success_text },
    { "say", success_end_text },
})
assert(host.funds == 0 and host.heat_reads == 1)
assert(game.state["group_item." .. E.GroupItemId.INDIAN_LANDS_VISA] == true)
for _, item_id in pairs(E.GroupItemId) do
    local expected = item_id == E.GroupItemId.INDIAN_LANDS_VISA or
                     initial_group_items[item_id]
    assert(game.state["group_item." .. item_id] == expected,
        "purchase changed another group item: " .. item_id)
end

-- Preserve the signed FFF0D5 package-result branches and their text.
reset("handoff_due", 0, nil, {})
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 0 and #speeches == 1 and
       speeches[1].text == pickup_text and
       speeches[1].continuation == false)
assert_timeline({ { "say", pickup_text } })
reset("payment_due", 0, nil, {})
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 0 and #speeches == 1 and
       speeches[1].text == delivery_text and
       speeches[1].continuation == false)
assert_timeline({ { "say", delivery_text } })

print("council_passport_lodge tests passed: entry, leave, purchase branches, and package results")
