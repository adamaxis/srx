-- Native story-file rules for Matrix datastores. The host adapter turns save
-- records into named collection facts; this module never reads or edits flag
-- words. ROM 0x55058 selects these outcomes before ordinary search detection.
local E = require("enums")
local StoryFacts = require("story_facts")

local M = {}

-- Normalize the ROM's displayed file-category labels at the menu boundary.
-- Rule tables below use stable names that do not inherit source spelling.
local CATEGORY_BY_LABEL = {
    ["Outdated Files"] = "outdated_files",
    ["System Files"] = "system_files",
    ["Mngmnt Files"] = "management_files",
    ["Project Files"] = "project_files",
    ["Security Files"] = "security_files",
    ["Marketing Data"] = "marketing_data",
    ["Financial Data"] = "financial_data",
    ["Confidntl Data"] = "confidential_data",
    ["Prisoner Files"] = "prisoner_files",
    ["Case Files"] = "case_files",
}

local function category_key(label)
    if type(label) ~= "string" then return nil end
    local normalized = label:match("^%s*(.-)%s*$")
    return CATEGORY_BY_LABEL[normalized]
end

local function outcome(name, when, wallet_reward)
    return { fact = name, when = when, wallet_reward = wallet_reward }
end

local function story_flag_is(flag, expected)
    local value = StoryFacts.is_set(flag)
    if value == nil then return nil end
    return value == expected
end

local function fact_is(facts, name, expected)
    local value = facts[name]
    if value == nil then return nil end
    return value == expected
end

local function has_story_flag(flag)
    return function()
        return story_flag_is(flag, true)
    end
end

local function lacks_story_flag(flag)
    return function()
        return story_flag_is(flag, false)
    end
end

local function either_story_flag(first, second)
    local first_value = StoryFacts.is_set(first)
    local second_value = StoryFacts.is_set(second)
    if first_value == true or second_value == true then return true end
    if first_value == nil or second_value == nil then return nil end
    return false
end

-- Ordered outcomes preserve the source handler's fall-through order when a
-- system offers more than one report in a category.
local RULES = {
    -- [C] ROM 0x550F4..0x55134: Gates financial and outdated files.
    gates_undersound = {
        financial_data = {
            outcome("gates_financial_file_collected",
                has_story_flag(E.StoryFact.MAKO_DEFEATED)),
        },
        outdated_files = {
            outcome("gates_outdated_file_collected"),
        },
    },
    -- [C] ROM 0x55138..0x55158: Aztechnology confidential files.
    aztechnology = {
        confidential_data = {
            outcome("aztechnology_confidential_file_collected"),
        },
    },
    -- [C] ROM 0x5515C..0x551BC: Fuchi management, security, and follow-up.
    fuchi = {
        management_files = {
            outcome("fuchi_management_file_collected",
                lacks_story_flag(E.StoryFact.HEART_DELIVERED)),
        },
        security_files = {
            outcome("fuchi_security_file_collected",
                lacks_story_flag(E.StoryFact.HEART_DELIVERED)),
            outcome("fuchi_security_followup_collected", function(r)
                local heart_delivered = StoryFacts.is_set(
                    E.StoryFact.HEART_DELIVERED)
                if heart_delivered == true then return true end
                if heart_delivered == nil then return nil end
                return fact_is(r, "fuchi_security_file_collected", true)
            end),
        },
    },
    -- [C] ROM 0x551C0..0x55220: Mitsuhama confidential/security files.
    mitsuhama = {
        confidential_data = {
            outcome("mitsuhama_confidential_file_collected"),
        },
        security_files = {
            outcome("mitsuhama_security_file_collected",
                lacks_story_flag(E.StoryFact.ARAGORN_FOUND)),
        },
    },
    -- [C] ROM 0x55224..0x55302: Lone Star case, prisoner, and financial files.
    lone_star = {
        prisoner_files = {
            outcome("lone_star_prisoner_file_collected", function(r)
                return fact_is(r, "johnson_contact_recorded", true)
            end),
        },
        financial_data = {
            outcome("lone_star_financial_file_collected",
                lacks_story_flag(E.StoryFact.HEART_DELIVERED)),
        },
    },
    -- [C] ROM 0x55302..0x55346: Ares confidential file and Ilene follow-up.
    ares = {
        confidential_data = {
            outcome("ares_confidential_file_collected"),
            outcome("ares_confidential_followup_collected", function(r)
                local primary = fact_is(
                    r, "ares_confidential_file_collected", true)
                if primary ~= true then return primary end
                return story_flag_is(E.StoryFact.ILENE_MET, false)
            end),
        },
    },
    -- [C] ROM 0x55346..0x553B2: Hollywood prisoner and outdated files.
    hollywood = {
        prisoner_files = {
            outcome("hollywood_prisoner_file_collected", function()
                return story_flag_is(E.StoryFact.ILENE_MET, false)
            end),
        },
        outdated_files = {
            outcome("hollywood_outdated_file_collected", function(r)
                return fact_is(r, "johnson_contact_recorded", true)
            end),
        },
    },
    -- [C] ROM 0x553B2..0x55526: Renraku report sequence and prerequisites.
    renraku = {
        confidential_data = {
            outcome("renraku_confidential_file_collected"),
            outcome("renraku_confidential_followup_collected", function(r)
                local primary = fact_is(
                    r, "renraku_confidential_file_collected", true)
                if primary ~= true then return primary end
                return either_story_flag(E.StoryFact.MAKO_DEFEATED,
                                         E.StoryFact.MICHAEL_MISSION_TOLD)
            end),
        },
        project_files = {
            outcome("renraku_shaman_project_report_collected", function()
                local pelt_given = StoryFacts.is_set(
                    E.StoryFact.SHAMAN_PELT_GIVEN)
                if pelt_given == nil then return nil end
                if pelt_given ~= true then return false end
                return story_flag_is(E.StoryFact.SERPENT_SCALE_TAKEN, false)
            end),
            outcome("renraku_mako_project_report_collected",
                has_story_flag(E.StoryFact.MAKO_DEFEATED)),
            outcome("renraku_michael_project_report_collected", function()
                return either_story_flag(E.StoryFact.MICHAEL_ALIAS_LEARNED,
                                         E.StoryFact.MICHAEL_MISSION_TOLD)
            end),
            outcome("renraku_map_project_report_collected", function(r)
                return fact_is(r, "renraku_map_overlay_collected", true)
            end),
        },
        management_files = {
            outcome("renraku_management_file_collected",
                has_story_flag(E.StoryFact.MICHAEL_ALIAS_LEARNED)),
        },
    },
    -- [C] ROM 0x5507C..0x550F0: Ito's four categories use the second page.
    ito_passcode = {
        project_files = {
            outcome("ito_project_file_collected"),
        },
        confidential_data = {
            outcome("ito_confidential_file_collected"),
        },
        financial_data = {
            outcome("ito_financial_file_collected"),
        },
        management_files = {
            outcome("ito_management_file_collected"),
        },
    },
    -- [C] ROM 0x55526..0x55578: UCAS research report and wallet reward.
    ucas_federal_government = {
        system_files = {
            outcome("ucas_virology_report_collected", function(r)
                local research_complete = r.ucas_virology_research_complete
                if research_complete == nil then return nil end
                if research_complete ~= true then return false end
                return type(r.nuyen) == "number"
            end, 50000),
        },
    },
}

local function selected_outcome(facts, file_class)
    if type(facts) ~= "table" then return nil end
    local system_rules = RULES[facts.system_key]
    local category_rules = system_rules and
        system_rules[category_key(file_class)]
    if category_rules == nil then return nil end

    for _, candidate in ipairs(category_rules) do
        local eligible = true
        if candidate.when ~= nil then eligible = candidate.when(facts) end
        if eligible == nil then return nil end
        if eligible == true then
            local collected = facts[candidate.fact]
            if collected == nil then return nil end
            if collected == false and
               (candidate.wallet_reward == nil or
                type(facts.nuyen) == "number") then
                return candidate
            end
        end
    end
    return nil
end

function M.can_match(facts, file_class)
    return selected_outcome(facts, file_class) ~= nil
end

-- The ROM adds the UCAS reward to a 32-bit wallet at 0x5536E. Keep that
-- currency rule in one named operation rather than exposing bit-mask math to
-- the story handler.
local function credit_wallet(balance, amount)
    return (balance + amount) % 4294967296
end

function M.matches(facts, file_class)
    local selected = selected_outcome(facts, file_class)
    if selected == nil then return false end
    facts[selected.fact] = true
    if selected.wallet_reward ~= nil then
        facts.nuyen = credit_wallet(facts.nuyen, selected.wallet_reward)
    end
    return true
end

return { can_match = M.can_match, matches = M.matches }
