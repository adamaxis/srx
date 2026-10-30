-- ROM 0x1F7CE intercepts Scene 4/event 0 before the event table's Scene 13
-- route. The A0 roots at 0xF4A60/0xF4A78 provide the greeting, 40-nuyen
-- offer, and choices; 0x21458 debits FFFBFE or returns the insufficient-funds
-- page. A successful debit changes FFF10D to -1 and 0x1F81A enters 0xCF38.
local E = require("enums")
local Inn = {}

local INTRODUCTION =
    "Salutations, weary traveler!||I offer my services to you this fine day.  Are you in need of a hot meal and a soft bed?"
local ROOM_OFFER =
    "Please, look no further.||I ask for only 40 nuyen, payable in advance, of course."
local PACKAGE_PAYMENT =
    "<A well-dressed elf enters behind you.>||Do you have my package?||<You hand him the package, and he gives you the payment.>||My employer thanks you."
local PACKAGE_HANDOFF =
    "<You tell the proprietor that you are here for a package.||He roots around, looking, and finds it.||Handing it over, he wishes you a great day.>"

local function outcome(kind)
    return { type = kind }
end

local function source_boundary(reason)
    return { type = "source_boundary", reason = reason }
end

local function correct_location()
    return game.location.type == "building" and
        game.location.id == "council_island_inn"
end

local function known_package_run_status(status)
    return status == E.PackageRunStatus.NONE or
        status == E.PackageRunStatus.HANDOFF_DUE or
        status == E.PackageRunStatus.PAYMENT_DUE
end

function Inn.present_field_exit_prompt()
    if not correct_location() then
        return source_boundary(
            "Council Island Inn prompt requires its linked building location")
    end

    local package_run_status = game.field.package_run_status
    if not known_package_run_status(package_run_status) then
        return source_boundary(
            "Council Island Inn prompt has no package-run status")
    end

    -- ROM 0x1F7E8/0x1F7F2 select payment/handoff roots by sign; the field
    -- host translates that branch into a named package-run status.
    if package_run_status == E.PackageRunStatus.PAYMENT_DUE then
        game.ui.say_text(PACKAGE_PAYMENT, false)
        return outcome("continue_in_place")
    elseif package_run_status == E.PackageRunStatus.HANDOFF_DUE then
        game.ui.say_text(PACKAGE_HANDOFF, false)
        return outcome("continue_in_place")
    end

    local choice = game.ui.prompt_text(
        "inn_service", INTRODUCTION, { "crash", "leave" })
    if choice ~= "crash" then
        return outcome("continue_in_place")
    end

    choice = game.ui.prompt_text(
        "room_payment", ROOM_OFFER, { "pay", "leave" })
    if choice ~= "pay" then
        return outcome("continue_in_place")
    end

    -- The source payment helper reads the live wallet at 0x21458. It leaves
    -- FFF10D positive when funds are short and writes -1 after a 40-nuyen
    -- debit; the caller then branches at 0x1F81A.
    local payment = game.services.call("building", "charge_40")
    if payment == "paid" then
        game.ui.say(
            "room_paid", "council_island_inn_narrator",
            "council_island_inn")
        return {
            type = "open_service",
            serviceId = "hotel.options",
            returnPolicy = "origin",
        }
    elseif payment == "insufficient" then
        game.ui.say(
            "room_insufficient", "council_island_inn_narrator",
            "council_island_inn")
        return outcome("continue_in_place")
    end

    return source_boundary(
        "ROM 0x21458 room payment returned an unresolved wallet result")
end

return Inn
