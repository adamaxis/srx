-- Council Island event 2 site service. The field dispatcher loads portrait
-- 0x121F34 at ROM 0x200B2; the speaker profile reuses that portrait asset.
local E = require("enums")
local GroupItems = require("group_items")
local StoryState = require("story_state")

local Lodge = {}

local PRICE_NUYEN = 500
local WARRANT_THRESHOLD = 3
local VISA_ITEM = E.GroupItemId.INDIAN_LANDS_VISA
local SPEAKER_ID = "council_passport_lodge_clerk"
local TEXT_PROFILE = "council_medicine_lodge"

local INTRODUCTION = "<The Passport Lodge on Council Island serves as a " ..
    "visiting center for tourists.||All sorts of cheap items are for sale " ..
    "here, but nothing of value.||An AmerIndian clerk in shirt-sleeves " ..
    "and tie notices you and comes over.>"
local PURCHASE_LABEL =
    "I would like to purchase a passport to the AmerIndian Lands."
local LEAVE_LABEL = "I'll talk to you later, chum."
local PRICE_PAGE = "<The clerk eyes you like something the cat dragged in.>||" ..
    "A passport costs 500$, which includes a security check to prove you " ..
    "are not fleeing justice."
local CONTRACT_PAGE = "Be aware, you are signing a binding contract to abide " ..
    "by tribal law while in the Salish-Shidhe.||Any deviance from the " ..
    "tribal law will revoke all privileges.||If you hand me your credstick " ..
    "I can give you the forms to fill out."
local PAY_LABEL = "Easy money, chummer.  I'll pay."
local LOW_FUNDS_PAGE = "<You slot your credstick, but the account balance " ..
    "is under 500$.||The clerk gives you a disapproving look and moves on.>"
local FORMS_PAGE = "<You fill out the forms, and hand them back.||The clerk " ..
    "feeds them into the computer, and soon the forms come back.||The clerk " ..
    "looks them over, and his eyes widen.>"
local WARRANT_PAGE = "I'm sorry sir, but there appears to be a warrant out " ..
    "for your arrest.||As this is neutral territory, I suggest you leave " ..
    "before I am forced to call security."
local SUCCESS_PAGE = "<The forms are filled out and go through without a " ..
    "hitch.>||Remember, this passport is only good if you obey the tribal " ..
    "laws.||Any slip-ups and you will be escorted out and Lone Star " ..
    "apprised of your misconduct."
local SUCCESS_CLOSING = "Enjoy your visit to the Salish-Shidhe."
local FAREWELL = "Enjoy the beauty of the island, good sir."
local PACKAGE_PICKUP = "<You explain you are here to pick up a package.||He " ..
    "gives it to you and you are on your way.>"
local PACKAGE_DELIVERY = "<You explain you have a package for him.||He " ..
    "accepts it and pays you.>"

local function assert_lodge_context()
    assert(game.location.type == "building" and
           game.location.id == "council_passport_lodge",
           "Passport Lodge prompt ran outside its linked building")
end

local function say(text, has_more)
    assert(game.ui.say_text(text, has_more, SPEAKER_ID, TEXT_PROFILE),
        "Passport Lodge page could not be presented")
end

local function prompt(menu_id, text, choices)
    return game.ui.prompt_text(menu_id, text, choices,
                               SPEAKER_ID, TEXT_PROFILE)
end

local function present_purchase()
    -- ROM 0x1114EA (record bytes 00 11 A3 9C 00 0F E5 BF ... 00 11 14 FE)
    -- is the price page followed by MORE; 0x1114FE's command choice is 0x5E.
    say(PRICE_PAGE, true)
    local choice = prompt("passport_lodge_purchase", CONTRACT_PAGE, {
        { id = "pay", label = PAY_LABEL },
        { id = "leave", label = LEAVE_LABEL },
    })
    if choice ~= "pay" then
        say(FAREWELL, false) -- ROM 0x11154E.
        return "field_return"
    end

    local funds = game.nuyen()
    if type(funds) ~= "number" then
        return {
            type = "source_boundary",
            reason = "Passport Lodge purchase needs the current nuyen balance",
        }
    end
    -- ROM 0x2256A checks funds before it reads the signed threshold at
    -- 0xFFFBFA. The low-funds branch returns 0x111532 without either write.
    if funds < PRICE_NUYEN then
        say(LOW_FUNDS_PAGE, false)
        return "field_return"
    end

    local heat = StoryState.heat_level()
    if type(heat) ~= "number" then
        return {
            type = "source_boundary",
            reason = "Passport Lodge purchase needs the current heat level",
        }
    end
    if heat >= WARRANT_THRESHOLD then
        -- ROM 0x22580 returns 0x111516 for values at/above three; its MORE
        -- target 0x11152A contains the warrant message. This path does not pay.
        say(FORMS_PAGE, true)
        say(WARRANT_PAGE, false)
        return "field_return"
    end

    -- ROM 0x22594 sets FFFC22 bit 1, then 0x225A4 debits 500 nuyen.
    GroupItems.set(VISA_ITEM, true)
    game.set_nuyen(funds - PRICE_NUYEN)
    say(SUCCESS_PAGE, true)
    say(SUCCESS_CLOSING, false)
    return "field_return"
end

function Lodge.present_field_exit_prompt()
    assert_lodge_context()

    local package_status = game.field.package_run_status
    if package_status == E.PackageRunStatus.HANDOFF_DUE then
        say(PACKAGE_PICKUP, false) -- ROM 0x200D0 selects root 0x110D36.
        return "field_return"
    elseif package_status == E.PackageRunStatus.PAYMENT_DUE then
        say(PACKAGE_DELIVERY, false) -- ROM 0x200C6 selects root 0x110D3E.
        return "field_return"
    elseif package_status ~= E.PackageRunStatus.NONE then
        return {
            type = "source_boundary",
            reason = "Passport Lodge prompt has no package-run status",
        }
    end

    -- ROM 0x200DA selects root 0x110D46 when FFF0D5 is zero; the two labels
    -- lead to 0x1114EA and 0x11154E, respectively.
    local choice = prompt("passport_lodge_entry", INTRODUCTION, {
        { id = "purchase", label = PURCHASE_LABEL },
        { id = "leave", label = LEAVE_LABEL },
    })
    if choice == "purchase" then
        return present_purchase()
    end

    say(FAREWELL, false)
    return "field_return"
end

return Lodge
