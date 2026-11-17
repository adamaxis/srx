-- Roscoe's scene-0/event-7 site handler. The source event-table target is
-- cancelled by the shared field return; see out/DIAG.md.
local E = require("enums")
local DeckRules = require("menu.rules.deck")
local Sites = require("data.roscoe_sites")

local Roscoe = {}
local ENTITY_ID = "downtown_site_09_roscoe"
local FILE_SLOT_COUNT = DeckRules.FILES
local DATAFILE_PRICE_FACTOR = 5

local SELL_LABEL = "I've got some data I'd like to sell."
local LATER_LABEL = "I'll talk to you later, chum."
local YES_LABEL = "Yes."
local NO_LABEL = "No."

-- ROM 0x114754's question mark is the dynamic amount drawn at 0xC4AE.
-- The source also draws the selected file name at 0xC1A6 over the blank
-- line after "This file here...".
local OFFER_TEMPLATE = "This file here...|||I might be able ta' find" ..
    "|a use for the info.||How's about I offer ya'|" ..
    "             $?"

local function source_boundary(reason)
    return { type = "source_boundary", reason = reason }
end

local function site_context()
    local site = game.location.type == "building" and
        Sites[game.location.id] or nil
    assert(site ~= nil and game.location.id == ENTITY_ID,
        "Roscoe prompt ran outside its linked building")
    return site
end

local function say(site, event_id)
    game.ui.say(event_id, site.speaker, site.text_profile)
end

local function package_result(site, status)
    if status == E.PackageRunStatus.HANDOFF_DUE then
        say(site, "package_handoff")
        return "field_return"
    elseif status == E.PackageRunStatus.PAYMENT_DUE then
        say(site, "package_payment")
        return "field_return"
    elseif status ~= E.PackageRunStatus.NONE then
        return source_boundary("Roscoe package result is unresolved")
    end
end

local function commit_files(record)
    if type(game.deck_slots) ~= "function" or
       type(game.deck_commit) ~= "function" then
        return false
    end
    game.deck_commit(record, game.deck_slots())
    return true
end

local function offer_text(file_name, price)
    local text = OFFER_TEMPLATE:gsub("|||", "||" .. file_name .. "|", 1)
    return text:gsub("%?", tostring(price), 1)
end

local function sell_datafiles(site)
    if type(game.cyberdeck_available) ~= "function" then
        return source_boundary("Roscoe needs the typed cyberdeck availability fact")
    end
    local available = game.cyberdeck_available()
    if type(available) ~= "boolean" then
        return source_boundary("Roscoe cyberdeck availability is unresolved")
    end
    if not available then
        say(site, "no_deck")
        return "field_return"
    end

    if type(game.deck_record) ~= "function" then
        return source_boundary("Roscoe needs the typed cyberdeck record")
    end
    local record = game.deck_record()
    if type(record) ~= "table" or type(record.files) ~= "table" then
        return source_boundary("Roscoe datafile records are unresolved")
    end

    -- Cmd 0xAF sets the source counter to five; cmd 0xB0 counts down and
    -- visits the five three-byte records in reverse slot order.
    for slot = FILE_SLOT_COUNT - 1, 0, -1 do
        local file = record.files[slot]
        if type(file) == "table" and file.id >= 0 then
            local data_value = file.run_objective and -1 or file.data_value
            if type(data_value) ~= "number" then
                return source_boundary("Roscoe datafile value is unresolved")
            end

            if data_value < 0 then
                say(site, "encrypted")
            elseif data_value == 0 then
                DeckRules.erase_file(record, slot)
                if not commit_files(record) then
                    return source_boundary(
                        "Roscoe could not persist a deleted datafile")
                end
                say(site, "useless")
            else
                local file_name = DeckRules.FILE_NAMES[file.id]
                if type(file_name) ~= "string" then
                    return source_boundary("Roscoe datafile name is unresolved")
                end
                local price = file.size * DATAFILE_PRICE_FACTOR * data_value
                local choice = game.ui.prompt_text(
                    "roscoe_datafile_price",
                    offer_text(file_name, price),
                    {
                        { id = "yes", label = YES_LABEL },
                        { id = "no", label = NO_LABEL },
                    },
                    site.speaker, site.text_profile)
                if choice == "yes" then
                    local funds = game.nuyen()
                    if type(funds) ~= "number" then
                        return source_boundary(
                            "Roscoe datafile sale needs the current nuyen balance")
                    end
                    -- Cmd 0xAE adds the stored price before clearing the slot.
                    game.set_nuyen(funds + price)
                    DeckRules.erase_file(record, slot)
                    if not commit_files(record) then
                        return source_boundary(
                            "Roscoe could not persist the sold datafile")
                    end
                elseif choice ~= "no" then
                    return source_boundary("Roscoe sale choice is unresolved")
                end
            end
        end
    end

    say(site, "end_of_file")
    return "field_return"
end

function Roscoe.present_field_exit_prompt()
    local site = site_context()
    local package_status = game.field.package_run_status
    if package_status == E.PackageRunStatus.HANDOFF_DUE or
       package_status == E.PackageRunStatus.PAYMENT_DUE then
        return package_result(site, package_status)
    elseif package_status ~= E.PackageRunStatus.NONE then
        return source_boundary("Roscoe package result is unresolved")
    end

    local choice = game.ui.prompt_text_key(
        "roscoe_opening",
        "downtown.roscoe.opening",
        {
            { id = "sell", label = SELL_LABEL },
            { id = "later", label = LATER_LABEL },
        },
        site.speaker, site.text_profile)
    if choice == "later" then
        say(site, "farewell")
        return "field_return"
    elseif choice == "sell" then
        say(site, "connect_deck")
        return sell_datafiles(site)
    end
    return source_boundary("Roscoe opening choice is unresolved")
end

return Roscoe
