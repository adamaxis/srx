-- Stable root choices for the field Pocket Secretary menu.
-- The native presenter owns selection cadence, page drawing, and handlers.
local PocketSecretary = {}

local function text_entries(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return ids, nil, keys
end

function PocketSecretary.root()
    return {
        "notebook",
        "group_items",
        "cyberdeck",
        "dismiss_runner",
        "save_load",
    }, "pocket_secretary.root.title", {
        "pocket_secretary.option.notebook",
        "pocket_secretary.option.group_items",
        "pocket_secretary.option.cyberdeck",
        "pocket_secretary.option.dismiss_runner",
        "pocket_secretary.option.save_load",
    }
end

function PocketSecretary.root_text_entries()
    return { "separator" }, nil, {
        "pocket_secretary.root.text.option.separator",
    }
end

-- All possible Notebook rows in ROM order. The native model applies the
-- Contacts, Mr. Johnsons, and Shadowrunners story-flag gates at runtime.
function PocketSecretary.notebook()
    return {
        "current_run",
        "tips_and_clues",
        "contacts",
        "mr_johnsons",
        "shadowrunners",
    }, "pocket_secretary.notebook.options.title", {
        "pocket_secretary.notebook.option.current_run",
        "pocket_secretary.notebook.option.tips_and_clues",
        "pocket_secretary.notebook.option.contacts",
        "pocket_secretary.notebook.option.mr_johnsons",
        "pocket_secretary.notebook.option.shadowrunners",
    }
end

-- Child-page text IDs remain in the ROM's source order. Native state decides
-- whether these messages or rows are currently visible.
function PocketSecretary.cyberdeck_unavailable_lines()
    return text_entries(
        { "no_deck_first", "no_deck_second", "look_for_deck" },
        "pocket_secretary.cyberdeck.option.")
end

function PocketSecretary.group_item_names()
    return text_entries({
            "prototype_cyberheart",
            "indian_lands_visa",
            "feathered_scale",
            "hell_hound_pelt",
            "gargoyle_horn",
            "gift_lady_gillian",
            "gift_red_buffalo_woman",
            "landmark_map",
            "map_overlay",
            "current_run_package",
            "security_pass",
            "weapon_permit",
            "eye_fiver_ear_cuff",
            "halloweener_patch",
            "ork_armband",
        }, "pocket_secretary.group_item.option.")
end

function PocketSecretary.group_items_empty_lines()
    return text_entries(
        { "first_line", "second_line" },
        "pocket_secretary.group_items.empty.option.")
end

function PocketSecretary.dismiss_options()
    return text_entries(
        { "dismiss", "who_question", "abort" },
        "pocket_secretary.dismiss.option.")
end

function PocketSecretary.dismiss_separator_text()
    return text_entries(
        { "separator" }, "pocket_secretary.dismiss.text.option.")
end

function PocketSecretary.notebook_no_run_lines()
    return text_entries(
        { "first_line", "second_line", "last_line" },
        "pocket_secretary.notebook.no_run.option.")
end

function PocketSecretary.mr_johnson_contacts()
    return text_entries({
            "gunderson",
            "julius_strouther",
            "mortimer_reed",
            "vigore_and_jarl",
            "caleb_brightmore",
        }, "pocket_secretary.notebook.mr_johnson.name.option.")
end

function PocketSecretary.mr_johnson_details()
    return text_entries({
            "gunderson",
            "julius_strouther",
            "mortimer_reed",
            "vigore_and_jarl",
            "caleb_brightmore",
        }, "pocket_secretary.notebook.mr_johnson.detail.option.")
end

function PocketSecretary.notebook_tips_metadata()
    return text_entries(
        { "title", "separator", "page_count", "page_number_clear" },
        "pocket_secretary.notebook.tips.option.")
end

local function notebook_tip_page(first_tip)
    local ids = {}
    for tip_number = first_tip, first_tip + 31 do
        ids[#ids + 1] = "tip_" .. tip_number
    end
    return text_entries(ids, "pocket_secretary.notebook.tips.option.")
end

-- Keep each source-order page within the script menu catalog's 32-row limit.
function PocketSecretary.notebook_tip_page_1()
    return notebook_tip_page(1)
end

function PocketSecretary.notebook_tip_page_2()
    return notebook_tip_page(33)
end

function PocketSecretary.notebook_tip_page_3()
    return notebook_tip_page(65)
end

return PocketSecretary
