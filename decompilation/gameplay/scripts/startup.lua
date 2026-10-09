-- Stable choice identifiers for the three title-screen cursor menus.
-- The native startup controller owns selection, input timing, and transitions.
local Startup = {}

function Startup.options()
    return { "new_game", "load_game" }
end

function Startup.save_slots()
    return { "slot_one", "slot_two" }
end

function Startup.archetypes()
    return { "samurai", "decker", "gator_shaman" }
end

-- ROM-derived title, archetype, and save/load copy lives in the shared text
-- resource. Keep this module as the screen's semantic binding layer.
function Startup.text_keys()
    return {
        options_title = "startup.options.title",
        options_separator = "startup.options.separator",
        archetype_heading_choose = "startup.archetype.heading.choose",
        archetype_heading_archetype = "startup.archetype.heading.archetype",
        archetype_separator = "startup.archetype.separator",
        archetype_gator_suffix = "startup.archetype.gator_shaman.suffix",
        no_save_0 = "startup.message.no_save.0",
        no_save_1 = "startup.message.no_save.1",
        no_save_2 = "startup.message.no_save.2",
        no_save_3 = "startup.message.no_save.3",
        save_success_0 = "startup.message.save_success.0",
        save_success_1 = "startup.message.save_success.1",
        save_success_2 = "startup.message.save_success.2",
    }
end

-- Opening-story copy is source text; the native cutscene owns when and where
-- each typed story record is drawn.
function Startup.opening_text_keys()
    return {
        time = "opening.story.time",
        place = "opening.story.place",
        team_cover = "opening.story.team_cover",
        team_runners = "opening.story.team_runners",
        team_wilderness = "opening.story.team_wilderness",
        without_warning = "opening.story.without_warning",
        ambushed = "opening.story.ambushed",
        massacre = "opening.story.massacre",
        captured = "opening.story.captured",
        slain = "opening.story.slain",
        tri_vid_recovered = "opening.story.tri_vid_recovered",
        national_news = "opening.story.national_news",
        brother_last_die = "opening.story.brother_last_die",
        spending = "opening.story.spending",
        vow = "opening.story.vow",
        brother_death = "opening.story.brother_death",
        landing = "opening.story.landing",
        flag_cab = "opening.story.flag_cab",
        last_credstick = "opening.story.last_credstick",
        transaction = "opening.story.transaction",
        motel = "opening.story.motel",
        redmond = "opening.story.redmond",
    }
end

return Startup
