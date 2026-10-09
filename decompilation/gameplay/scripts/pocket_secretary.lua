-- Stable root choices for the field Pocket Secretary menu.
-- The native presenter owns selection cadence, page drawing, and handlers.
local PocketSecretary = {}

function PocketSecretary.root()
    return {
        "notebook",
        "group_items",
        "cyberdeck",
        "dismiss_runner",
        "save_load",
    }
end

function PocketSecretary.root_text_entries()
    return { "separator" }
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
    }
end

-- Child-page text IDs remain in the ROM's source order. Native state decides
-- whether these messages or rows are currently visible.
function PocketSecretary.cyberdeck_unavailable_lines()
    return { "no_deck_first", "no_deck_second", "look_for_deck" }
end

function PocketSecretary.group_item_names()
    return {
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
    }
end

function PocketSecretary.group_items_empty_lines()
    return { "first_line", "second_line" }
end

function PocketSecretary.dismiss_options()
    return { "dismiss", "who_question", "abort" }
end

function PocketSecretary.dismiss_separator_text()
    return { "separator" }
end

function PocketSecretary.notebook_no_run_lines()
    return { "first_line", "second_line", "last_line" }
end

function PocketSecretary.mr_johnson_contacts()
    return {
        "gunderson",
        "julius_strouther",
        "mortimer_reed",
        "vigore_and_jarl",
        "caleb_brightmore",
    }
end

function PocketSecretary.mr_johnson_details()
    return {
        "gunderson",
        "julius_strouther",
        "mortimer_reed",
        "vigore_and_jarl",
        "caleb_brightmore",
    }
end

return PocketSecretary
