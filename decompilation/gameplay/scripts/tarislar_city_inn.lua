-- Scene 1/event 1: Tarislar City Inn.
-- ROM 0x53B16 owns the paid-stay/Tabatha service, while 0x1F65A owns the
-- ordinary, run-result, and Frosty prompts. The link data selects each entry
-- path from source-backed story facts before this module presents it.
local E = require("enums")
local StoryFacts = require("story_facts")
local DialogueGraph = require("dialogue_graph")

local Inn = {}

local LOCATION_ID = "puyallup_site_03_tarislar_city_inn"

local TABATHA_FIELD_MODE_FLAG_WRITES = {
    { bit = E.FieldModeFlagBit.BIT_2, value = false },
    { bit = E.FieldModeFlagBit.BIT_4, value = false },
    { bit = E.FieldModeFlagBit.BIT_5, value = true },
    { bit = E.FieldModeFlagBit.BIT_6, value = true },
    { bit = E.FieldModeFlagBit.BIT_8, value = true },
}

-- The ROM has one FFFC30 longword. This checkout still keeps two host copies
-- of it: `field_mode_flag_bit_N` (field mode word) and
-- `notebook_tip_flag_bit_N` (Notebook tips 0-31, read by the pocket
-- secretary and saved separately). Every FFFC30 write goes to both so the
-- Notebook and field readers agree, as the motel payment path already does.
local FFFC30_HOST_KEYS = { "field_mode_flag_bit_", "notebook_tip_flag_bit_" }

local function fffc30_resolved(bit)
    for _, prefix in ipairs(FFFC30_HOST_KEYS) do
        if game.state[prefix .. bit] == nil then return false end
    end
    return true
end

local function write_fffc30(bit, value)
    for _, prefix in ipairs(FFFC30_HOST_KEYS) do
        game.state[prefix .. bit] = value
    end
end

local function at_inn()
    return game.location.type == "building" and
           game.location.id == LOCATION_ID
end

local function more(menu_id, text)
    return game.ui.prompt_text(menu_id, text, { "more" }) == "more"
end

local function say(text, continuation)
    game.ui.say_text(text, continuation == true)
end

local function leave()
    say("I'll be seeing you, chum.", false)
    return "field_return"
end

local function frosty_information()
    -- [C] The inn branch selects record 0xF4990 at 0x1F70A; its command word
    -- at 0xF49B8 is `00 00 00 E6`. Handler 0x24982 clears FFFC30 bit 23
    -- (`08 81 00 17`); 0x24992 sets only FFFC34 bit 4 (`08 C1 00 04`), and
    -- 0x2499C selects the existing 0x1180D2 Sinsearach-land response.
    write_fffc30(E.FieldModeFlagBit.BIT_23, false)
    game.state["field_mode_effect_flag_bit_" ..
        E.FieldModeEffectFlagBit.BIT_4] = true
    say(table.concat({
        "I haven't seen her in a couple of days, but that's not too unusual.",
        "If you're in a hurry, you might try the Sinsearach lands south of the AmerIndian Village.",
        "Story is she went through some harrowing ritual to become a tribal member.",
    }, "||"), false)
    return "field_return"
end

local function apply_tabatha_acceptance()
    -- [C] Command 0x14 at 0x21862 awards Karma; bytes at 0x21872..0x21886
    -- (`08 81 00 02`, `08 81 00 04`, `08 C1 00 05/06/08`) clear FFFC30
    -- bits 2/4 and set 5/6/8. Bytes at 0x21892..0x2189A
    -- (`08 C1 00 04`, `08 C1 00 02`) set the Caleb/Mortimer facts in FFFC27.
    -- This command has no FFFC34 write. Check every guarded store before the
    -- party service awards Karma, so acceptance cannot apply partial state.
    for _, write in ipairs(TABATHA_FIELD_MODE_FLAG_WRITES) do
        if not fffc30_resolved(write.bit) then
            return "service_unavailable"
        end
    end
    if game.state[E.JohnsonFact.MORTIMER_KNOWN] == nil or
       game.state[E.JohnsonFact.CALEB_KNOWN] == nil then
        return "service_unavailable"
    end

    if game.services.call("party", "award_karma_1") ~=
       "applied" then
        return "service_unavailable"
    end
    for _, write in ipairs(TABATHA_FIELD_MODE_FLAG_WRITES) do
        write_fffc30(write.bit, write.value)
    end
    game.state[E.JohnsonFact.MORTIMER_KNOWN] = true
    game.state[E.JohnsonFact.CALEB_KNOWN] = true
    return "applied"
end

function Inn.present_indexed_title_service()
    if not at_inn() or
       StoryFacts.is_set(E.StoryFact.MOTEL_BILL_PAID) ~= true or
       StoryFacts.is_set(E.StoryFact.TARISLAR_ASKED_SHALE) ~= false or
       game.field.package_run_status ~= E.PackageRunStatus.NONE then
        return "denied"
    end

    -- ROM 0x53A78 publishes indexed title service 1 in FFDD64 before calling
    -- 0x53B16. The intro, manager options, and paid-stay branch are represented
    -- by the reusable JSON graph. Its page and choice permutations follow the
    -- ROM prompt records; the award updates each runner's Karma through the
    -- party service, and the post-attack encounter remains a boundary.
    return DialogueGraph.run(
        "tarislar_city_inn.paid_stay_and_shale_service", {
            ["tarislar_city_inn.apply_tabatha_acceptance"] = function()
                return apply_tabatha_acceptance()
            end,
        })
end

function Inn.present_field_exit_prompt()
    if not at_inn() then
        return "denied"
    end

    if game.field.package_run_status == E.PackageRunStatus.PAYMENT_DUE then
        say(table.concat({
            "Oh, is that the package we've been waiting for?",
            "You hand him the package and he updates your credstick.",
        }, "||"), false)
        return "field_return"
    elseif game.field.package_run_status == E.PackageRunStatus.HANDOFF_DUE then
        say(table.concat({
            "You tell the proprietor that you are here for a package.",
            "He roots around, looking, and finds it.",
            "Handing it over, he wishes you a great day.",
        }, "||"), false)
        return "field_return"
    end

    if not more("shale_more", table.concat({
        "The Tarislar City Inn caters mostly to elves, as the word Tarislar is elvish for Remembrance.",
        "Despite its run-down appearance, this apartment has a great rep for being the safest place to stay in the Barrens.",
    }, "||")) then
        return "field_return"
    end
    local choices = { "crash" }
    if StoryFacts.is_set(E.StoryFact.MICHAEL_MISSION_TOLD) == true and
       StoryFacts.is_set(E.StoryFact.FROSTY_MET) == false then
        choices[#choices + 1] = "frosty"
    end
    choices[#choices + 1] = "leave"
    local choice = game.ui.prompt_text(
        "inn_service",
        table.concat({
            "You need a place to stay?",
            "Well you've come to the right place.",
            "This is the safest hotel in the city, for the price.",
            "Of course, payment is required in advance.",
        }, "||"),
        choices)
    if choice == "crash" then
        -- The ordinary 0x1F65A field prompt shares the same 30-nuyen room
        -- transaction as the indexed service, but starts at the payment
        -- choice because the manager's offer was already presented above.
        return DialogueGraph.run(
            "tarislar_city_inn.field_exit_room_offer")
    end
    if choice == "frosty" then return frosty_information() end
    return leave()
end

return Inn
