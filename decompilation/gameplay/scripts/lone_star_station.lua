-- Scene-3 event 6 is a same-map Lone Star desk interaction. ROM page roots
-- and the 0x66/0x67 command trace are recorded in out/DIAG.md.
local E = require("enums")
local Sites = require("data.lone_star_sites")
local StoryState = require("story_state")

local LoneStar = {}
local ARREST_THRESHOLD = 3
local FINE_PER_HEAT_LEVEL = 2500
local LOCATION_ID = "penumbra_site_05_lone_star"

local function source_boundary(reason)
    return { type = "source_boundary", reason = reason }
end

local function site_context()
    local site = game.location.type == "building" and
        Sites[game.location.id] or nil
    assert(site ~= nil and game.location.id == LOCATION_ID,
        "Lone Star station prompt ran outside its linked building")
    return site
end

local function say(site, event_id)
    game.ui.say(event_id, site.narrator, site.text_profile)
end

local function package_result(site, status)
    if status == E.PackageRunStatus.HANDOFF_DUE then
        say(site, site.package_handoff_event)
        return "field_return"
    elseif status == E.PackageRunStatus.PAYMENT_DUE then
        say(site, site.package_payment_event)
        return "field_return"
    end
end

local function apply_arrest_fine(site, heat)
    local fine = heat * FINE_PER_HEAT_LEVEL
    local displayed_amount = fine
    if not StoryState.set_heat_level(0) then
        return source_boundary("Lone Star arrest could not clear heat")
    end

    local funds = game.nuyen()
    if type(funds) ~= "number" or funds % 1 ~= 0 or funds < 0 then
        return source_boundary("Lone Star arrest needs the current wallet")
    end

    if fine > funds then
        if funds == 0 then
            -- ROM 0x2281E returns the release page before displaying a fine.
            say(site, site.release_event)
            return "field_return"
        end
        game.set_nuyen(0)
        -- 0x22814 loads the short wallet into D1; command 0x67 displays D1.
        displayed_amount = funds
    else
        game.set_nuyen(funds - fine)
    end

    -- ROM 0x1158B7 is `Your fine comes to`, a line break, six spaces, `$!`,
    -- a blank line, then `A few days later, you are|released.`. The 0xC0DA
    -- renderer draws the whole string (0x80 advances a line, 0xFF ends), and
    -- command 0x67 fills the selected amount at 0xC1A0.
    game.ui.say_text(string.format(
        "Your fine comes to|      $%d!||A few days later, you are|released.",
        displayed_amount), false)
    return "field_return"
end

function LoneStar.present_field_exit_prompt()
    local site = site_context()
    local package_status = game.field.package_run_status
    local package_outcome = package_result(site, package_status)
    if package_outcome ~= nil then return package_outcome end
    if package_status ~= E.PackageRunStatus.NONE then
        return source_boundary("Lone Star package result is unresolved")
    end

    local heat = StoryState.heat_level()
    if heat == nil then
        return source_boundary("Lone Star heat level is unresolved")
    end

    -- ROM page 0x110F66 carries < MORE > to command 0x66.
    say(site, site.entry_event)
    if heat < ARREST_THRESHOLD then
        -- Command 0x66 selects page 0x1118F2 below the arrest threshold.
        say(site, site.citizen_event)
        return "field_return"
    end

    -- Command 0x66 selects 0x1118FA, whose < MORE > runs command 0x67.
    say(site, site.arrest_event)
    return apply_arrest_fine(site, heat)
end

return LoneStar
