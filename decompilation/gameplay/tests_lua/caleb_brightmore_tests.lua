package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    contacts = load_json("dialogue/contacts"),
}
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

game = {
    content = content,
    state = {},
    location = {
        type = "building",
        id = "icarus_descending",
        text_profile = "icarus-descending.caleb_brightmore",
    },
    field = { package_run_status = "none" },
    modules = {},
    services = {},
    runs = { registry = {} },
    ui = {},
}

local Dialogue = require("dialogue")
game.modules.dialogue = Dialogue
local Johnson = require("johnson")
local E = require("enums")

local profile_id = "icarus-descending.caleb_brightmore"
local profile = Dialogue.johnson_interaction_profiles[profile_id]
assert(profile ~= nil, "Caleb Johnson profile must be registered")

game.runs.registry.take_pending_johnson_return = function() return nil end
game.runs.registry.present_offer_for_contact = function()
    return "not_accepted"
end
game.services.call = function() return "applied" end

local scenario
local pages, menus = {}, {}
local wallet = 0
local CAPTURED_DYNAMIC_MENU = "captured Caleb dynamic menu"

local function copy_array(values)
    local result = {}
    for i, value in ipairs(values) do result[i] = value end
    return result
end

local function seed_fffc30()
    for bit = 0, 31 do
        local value = bit % 2 == 0
        game.state["field_mode_flag_bit_" .. bit] = value
        game.state["notebook_tip_flag_bit_" .. bit] = value
    end
end

local function seed_fffc34()
    for bit = 0, 31 do
        game.state["field_mode_effect_flag_bit_" .. bit] = bit % 2 == 0
    end
end

local function reset(options)
    options = options or {}
    scenario = options
    pages, menus = {}, {}
    game.state = {}
    for _, fact in pairs(E.StoryFact) do game.state[fact] = false end
    game.state[profile.first_visit_fact] = not options.first_visit
    seed_fffc30()
    seed_fffc34()
    for index = 0, 12 do
        game.state["vid_phone_contact_" .. index] = false
    end
    for fact, value in pairs(options.story or {}) do
        game.state[fact] = value
    end
    for bit, value in pairs(options.field_mode or {}) do
        game.state["field_mode_flag_bit_" .. bit] = value
        game.state["notebook_tip_flag_bit_" .. bit] = value
    end
    for bit, value in pairs(options.field_mode_effect or {}) do
        game.state["field_mode_effect_flag_bit_" .. bit] = value
    end
    for index, value in pairs(options.phone_contacts or {}) do
        game.state["vid_phone_contact_" .. index] = value
    end

    wallet = options.nuyen or 0
    game.nuyen = function() return wallet end
    game.set_nuyen = function(value) wallet = value end

    game.ui.say = function(event_id)
        local speaker = content.speakers.profiles.caleb_brightmore
        local event = speaker.events[event_id]
        assert(event ~= nil, "missing Caleb speaker event " .. tostring(event_id))
        assert(script_text[event.title_key] ~= nil,
            "missing Caleb event text " .. tostring(event.title_key))
        if event.continuation_key ~= "" then
            assert(script_text[event.continuation_key] ~= nil,
                "missing Caleb continuation text " ..
                tostring(event.continuation_key))
        end
        pages[#pages + 1] = event_id
        return true
    end

    game.ui.menu = function(menu_id, choices)
        local profile_menus = content.menus.profiles[profile_id]
        local definition = profile_menus and profile_menus[menu_id]
        assert(definition ~= nil, "missing Caleb menu " .. tostring(menu_id))
        assert(script_text[definition.title_key] ~= nil,
            "missing Caleb menu title " .. tostring(definition.title_key))
        for _, choice_id in ipairs(choices) do
            local key = definition.choice_label_keys[choice_id]
            assert(key ~= nil and script_text[key] ~= nil,
                "missing Caleb menu label " .. menu_id .. "." ..
                tostring(choice_id))
        end
        menus[#menus + 1] = { id = menu_id, choices = copy_array(choices) }

        if scenario.capture_dynamic and
           (menu_id == "information_questions" or
            menu_id == "information_questions_empty") then
            scenario.captured = menus[#menus]
            error(CAPTURED_DYNAMIC_MENU, 0)
        end

        local queued = scenario.choices and scenario.choices[menu_id]
        local selected = queued and table.remove(queued, 1) or nil
        assert(selected ~= nil,
            "unexpected Caleb menu " .. menu_id)
        for _, choice_id in ipairs(choices) do
            if choice_id == selected then return selected end
        end
        error("Caleb choice " .. tostring(selected) ..
              " is unavailable in " .. menu_id)
    end
end

game.runs.registry.take_pending_johnson_return = function() return nil end

local function assert_pages(expected, label)
    assert(#pages == #expected,
        (label or "page sequence") .. ": expected " .. #expected ..
        " pages, got " .. #pages .. " (" .. table.concat(pages, ", ") .. ")")
    for index, event_id in ipairs(expected) do
        assert(pages[index] == event_id,
            (label or "page sequence") .. ": page " .. index ..
            " expected " .. event_id .. ", got " .. tostring(pages[index]))
    end
end

local function assert_equal_array(actual, expected, label)
    assert(#actual == #expected,
        label .. ": expected " .. table.concat(expected, ",") ..
        ", got " .. table.concat(actual, ","))
    for index, value in ipairs(expected) do
        assert(actual[index] == value,
            label .. ": entry " .. index .. " expected " .. value ..
            ", got " .. tostring(actual[index]))
    end
end

local function assert_fffc30_changes(before, allowed)
    for bit = 0, 31 do
        for _, prefix in ipairs({ "field_mode_flag_bit_",
                                  "notebook_tip_flag_bit_" }) do
            local key = prefix .. bit
            local expected = allowed[bit]
            if expected == nil then expected = before[key] end
            assert(game.state[key] == expected,
                key .. " changed unexpectedly: expected " ..
                tostring(expected) .. ", got " .. tostring(game.state[key]))
        end
    end
end

local function snapshot_prefix(prefix, count)
    local values = {}
    for bit = 0, count - 1 do
        values[bit] = game.state[prefix .. bit]
    end
    return values
end

local function assert_only_prefix_changes(prefix, before, allowed, count)
    for bit = 0, count - 1 do
        local expected = allowed[bit]
        if expected == nil then expected = before[bit] end
        assert(game.state[prefix .. bit] == expected,
            prefix .. bit .. " changed unexpectedly")
    end
end

local function snapshot_fffc30()
    local values = {}
    for bit = 0, 31 do
        values["field_mode_flag_bit_" .. bit] =
            game.state["field_mode_flag_bit_" .. bit]
        values["notebook_tip_flag_bit_" .. bit] =
            game.state["notebook_tip_flag_bit_" .. bit]
    end
    return values
end

-- First meeting: 0x1056AC <MORE> -> 0x1056F8, whose only label 0x103CFC
-- ("Tell me about Michael's last Shadowrun.") targets command 0x1C. Command
-- 0x1C selects 0x105780; the five-page chain ends at 0x1057D0, whose
-- information label targets 0x10570C.
local function test_first_meeting_michael_chain_and_command_1c_writes()
    for _, wrapup_choice in ipairs({ "leave", "information" }) do
        reset({
            first_visit = true,
            choices = {
                first_meeting = { "michael_last_run" },
                michael_wrapup = { wrapup_choice },
                information_offer = { "different_subject" },
                information_return = { "leave" },
            },
            field_mode = {
                [E.FieldModeFlagBit.BIT_5] = true,
                [E.FieldModeFlagBit.BIT_23] = false,
                [E.FieldModeFlagBit.BIT_4] = true,
            },
        })
        game.state[E.StoryFact.MICHAEL_MISSION_TOLD] = false
        local fffc30_before = snapshot_fffc30()

        local outcome = Johnson.interaction()
        assert(outcome == "leave", "Caleb interaction ended as " ..
            tostring(outcome))
        assert(profile.information_flow == "caleb_brightmore",
            "Caleb must use the ROM-backed information flow")
        assert(menus[1].id == "first_meeting",
            "first meeting must open record 0x1056F8")
        assert_equal_array(menus[1].choices, { "michael_last_run" },
            "record 0x1056F8 has one label (0x103CFC -> command 0x1C)")
        local expected = {
            "first_meeting", "first_meeting_continuation",
            "michael_information_1", "michael_information_2",
            "michael_information_3", "michael_information_4",
            "michael_wrapup",
        }
        if wrapup_choice == "information" then
            expected[#expected + 1] = "information_contacts_offer"
            expected[#expected + 1] = "information_different_subject"
            assert(menus[3].id == "information_offer",
                "0x1057D0 information label must open record 0x10570C")
        end
        expected[#expected + 1] = "leave"
        assert_pages(expected, "command 1C Michael chain (" ..
            wrapup_choice .. ")")
        assert(menus[2].id == "michael_wrapup")
        assert_equal_array(menus[2].choices,
            { "information", "work", "leave" }, "record 0x1057D0 choices")
        assert(game.state[profile.first_visit_fact] == true)
        assert(game.state[E.StoryFact.MICHAEL_MISSION_TOLD] == true,
            "command 1C must set the Michael mission fact")
        assert_fffc30_changes(fffc30_before, {
            [E.FieldModeFlagBit.BIT_5] = false,
            [E.FieldModeFlagBit.BIT_23] = true,
        })
        assert(game.state.field_mode_flag_bit_4 == true and
               game.state.notebook_tip_flag_bit_4 == true,
            "command 1C must preserve unrelated FFFC30 state")
        assert(game.state[E.StoryFact.TARISLAR_ASKED_SHALE] == false,
            "command 1C must preserve unrelated FFFC1E facts")
    end
end

-- Repeat root 0x1056C0: label 0x104051 targets page 0x10570C directly, so
-- asking for information must not replay command 0x1C or touch its flags.
local function test_repeat_information_opens_10570c_directly()
    reset({
        choices = {
            opening = { "information" },
            information_offer = { "different_subject" },
            information_return = { "leave" },
        },
        field_mode = {
            [E.FieldModeFlagBit.BIT_5] = true,
            [E.FieldModeFlagBit.BIT_23] = false,
        },
    })
    game.state[E.StoryFact.MICHAEL_MISSION_TOLD] = false
    local fffc30_before = snapshot_fffc30()
    assert(Johnson.interaction() == "leave")
    assert_pages({
        "information_contacts_offer", "information_different_subject",
        "leave",
    }, "repeat information root")
    assert(menus[2].id == "information_offer")
    assert_equal_array(menus[2].choices,
        { "questions", "contacts", "different_subject" },
        "record 0x10570C choices")
    assert(menus[3].id == "information_return")
    assert_equal_array(menus[3].choices,
        { "information", "work", "leave" }, "record 0x1056DC choices")
    assert(game.state[E.StoryFact.MICHAEL_MISSION_TOLD] == false,
        "repeat information must not run command 0x1C")
    assert_fffc30_changes(fffc30_before, {})
end

local function test_command_90_all_gate_combinations()
    local gates = {
        E.StoryFact.RED_BUFFALO_WOMAN,
        E.StoryFact.ILENE_MET,
        E.StoryFact.SHAMAN_TASKS_DONE,
        E.StoryFact.MAKO_DEFEATED,
        E.StoryFact.RENRAKU_MAP_OVERLAY,
        E.StoryFact.GILLIAN_INITIATION,
        E.StoryFact.GARDEN_APTS_ENTERED,
    }
    local combinations = 0
    for mask = 0, 127 do
        local story = {}
        for index, fact in ipairs(gates) do
            story[fact] = (mask & (1 << (index - 1))) ~= 0
        end
        reset({
            choices = {
                opening = { "information" },
                information_offer = { "questions" },
            },
            story = story,
            capture_dynamic = true,
        })

        local ok, err = pcall(Johnson.interaction)
        assert(not ok and err == CAPTURED_DYNAMIC_MENU,
            "Caleb's dynamic topic menu was not built for bit combination " ..
            mask .. ": " .. tostring(err))
        local expected = {}
        if story[E.StoryFact.RED_BUFFALO_WOMAN] and
           not story[E.StoryFact.ILENE_MET] then
            expected[#expected + 1] = "ilene"
        end
        if story[E.StoryFact.SHAMAN_TASKS_DONE] or
           story[E.StoryFact.MAKO_DEFEATED] then
            expected[#expected + 1] = "thon"
        end
        if story[E.StoryFact.RENRAKU_MAP_OVERLAY] or
           story[E.StoryFact.GILLIAN_INITIATION] or
           story[E.StoryFact.GARDEN_APTS_ENTERED] then
            expected[#expected + 1] = "ito_ogami"
        end

        local captured = scenario.captured
        assert(captured ~= nil, "Caleb's topic menu was not captured")
        if #expected == 0 then
            assert(captured.id == "information_questions_empty",
                "no-topic bit combination must use the ROM fallback page")
            expected = { "more" }
            assert(pages[#pages] == "information_questions_empty",
                "no-topic bit combination must show the default answer")
        else
            assert(captured.id == "information_questions",
                "available topics must use the dynamic topic menu")
        end
        assert_equal_array(captured.choices, expected,
            "Caleb topic gates " .. mask)
        combinations = combinations + 1
    end
    assert(combinations == 128)
end

local function test_thon_answer_and_return()
    reset({
        choices = {
            opening = { "information" },
            information_offer = { "questions" },
            information_questions = { "thon" },
            thon_followup = { "different_subject" },
            information_return = { "leave" },
        },
        story = {
            [E.StoryFact.RED_BUFFALO_WOMAN] = true,
            [E.StoryFact.SHAMAN_TASKS_DONE] = true,
            [E.StoryFact.RENRAKU_MAP_OVERLAY] = true,
        },
    })

    assert(Johnson.interaction() == "leave")
    assert_pages({
        "information_contacts_offer",
        "thon_information", "information_different_subject", "leave",
    }, "Thon answer and return")
    assert_equal_array(menus[3].choices,
        { "ilene", "thon", "ito_ogami" },
        "Thon menu with all eligible topics")
    assert_equal_array(menus[4].choices,
        { "another_question", "different_subject" },
        "Thon answer targets")
end

local function test_command_91_and_92_writes()
    reset({
        choices = {
            opening = { "information" },
            information_offer = { "questions" },
            information_questions = { "ilene" },
            ilene_followup = { "different_subject" },
            information_return = { "leave" },
        },
        story = {
            [E.StoryFact.RED_BUFFALO_WOMAN] = true,
            [E.StoryFact.ILENE_MET] = false,
        },
        field_mode = {
            [E.FieldModeFlagBit.BIT_5] = true,
            [E.FieldModeFlagBit.BIT_23] = false,
            [E.FieldModeFlagBit.BIT_4] = true,
        },
        field_mode_effect = { [0] = false, [4] = true, [18] = false },
    })
    local fffc30_before = {}
    for bit = 0, 31 do
        fffc30_before["field_mode_flag_bit_" .. bit] =
            game.state["field_mode_flag_bit_" .. bit]
        fffc30_before["notebook_tip_flag_bit_" .. bit] =
            game.state["notebook_tip_flag_bit_" .. bit]
    end
    local effect_before = snapshot_prefix("field_mode_effect_flag_bit_", 32)
    assert(Johnson.interaction() == "leave")
    assert(table.concat(pages, ","):find("ilene_information", 1, true) ~= nil,
        "Ilene command must show its ROM page")
    assert(game.state.field_mode_effect_flag_bit_0 == true,
        "Ilene command must set only its ROM effect flag")
    assert_only_prefix_changes("field_mode_effect_flag_bit_", effect_before,
        { [0] = true }, 32)
    assert_fffc30_changes(fffc30_before, {})

    reset({
        choices = {
            opening = { "information" },
            information_offer = { "questions" },
            information_questions = { "ito_ogami" },
            ito_ogami_followup = { "different_subject" },
            information_return = { "leave" },
        },
        story = { [E.StoryFact.RENRAKU_MAP_OVERLAY] = true },
        field_mode = {
            [E.FieldModeFlagBit.BIT_5] = true,
            [E.FieldModeFlagBit.BIT_23] = false,
            [E.FieldModeFlagBit.BIT_4] = true,
            [E.FieldModeFlagBit.RENRAKU_HEADHUNTER_TOLD] = false,
        },
    })
    local mode_before = {}
    for bit = 0, 31 do
        mode_before["field_mode_flag_bit_" .. bit] =
            game.state["field_mode_flag_bit_" .. bit]
        mode_before["notebook_tip_flag_bit_" .. bit] =
            game.state["notebook_tip_flag_bit_" .. bit]
    end
    assert(Johnson.interaction() == "leave")
    assert(table.concat(pages, ","):find("ito_ogami_information", 1, true) ~= nil,
        "Ogami command must show its ROM page")
    assert_fffc30_changes(mode_before, {
        [E.FieldModeFlagBit.RENRAKU_HEADHUNTER_TOLD] = true,
    })
end

local function test_contacts(cost, selected_contact, contact_index)
    local contacts = {}
    for index = 0, 12 do contacts[index] = true end
    contacts[contact_index] = false
    local insufficient = cost < E.JohnsonContactFee.CALEB_INFORMATION
    local result_event = insufficient and "contacts_purchase_insufficient" or
        "contacts_purchase_success"
    local result_menu = result_event
    reset({
        choices = {
            opening = { "information" },
            information_offer = { "contacts" },
            contacts_choice = { selected_contact },
            [result_menu] = { "leave" },
        },
        nuyen = cost,
        phone_contacts = contacts,
    })
    local fffc30_before = {}
    for bit = 0, 31 do
        fffc30_before["field_mode_flag_bit_" .. bit] =
            game.state["field_mode_flag_bit_" .. bit]
        fffc30_before["notebook_tip_flag_bit_" .. bit] =
            game.state["notebook_tip_flag_bit_" .. bit]
    end

    assert(Johnson.interaction() == "leave")
    local expected_balance = insufficient and cost or
        cost - E.JohnsonContactFee.CALEB_INFORMATION
    assert(game.nuyen() == expected_balance,
        "contact purchase debit does not match the ROM balance check")
    assert(game.state["vid_phone_contact_" .. contact_index] ==
           (not insufficient),
        "contact purchase wrote the wrong Vid-Phone availability fact")
    for index = 0, 12 do
        if index ~= contact_index then
            assert(game.state["vid_phone_contact_" .. index] == true,
                "contact purchase changed unrelated FFFC2C bit " .. index)
        end
    end
    assert_fffc30_changes(fffc30_before, {})
    assert(pages[#pages - 1] == result_event,
        "contact purchase must show the matching result page")
    assert(pages[#pages] == "leave")
    assert(menus[3].id == "contacts_choice",
        "the contacts choices must follow the introduction pages")
    assert_equal_array(menus[3].choices,
        { "chillicut", "kipp_david", "different_subject" },
        "Caleb contacts choices")
end

local function test_51_52_cost_boundary_and_contact_flags()
    test_contacts(1999, "chillicut", E.VidPhoneContact.CHILLICUT)
    test_contacts(2000, "chillicut", E.VidPhoneContact.CHILLICUT)
    test_contacts(1999, "kipp_david", E.VidPhoneContact.KIPP_DAVID)
    test_contacts(2000, "kipp_david", E.VidPhoneContact.KIPP_DAVID)
end

test_first_meeting_michael_chain_and_command_1c_writes()
test_repeat_information_opens_10570c_directly()
test_command_90_all_gate_combinations()
test_thon_answer_and_return()
test_command_91_and_92_writes()
test_51_52_cost_boundary_and_contact_flags()

print("Caleb Brightmore ROM information flow permutations passed")
