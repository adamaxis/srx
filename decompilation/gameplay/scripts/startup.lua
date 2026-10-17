-- Stable choice identifiers for the three title-screen cursor menus.
-- The native startup controller owns selection, input timing, and transitions.
local Startup = {}

function Startup.options()
    return { "new_game", "load_game" }, "startup.options.title", {
        "startup.option.new_game",
        "startup.option.load_game",
    }
end

function Startup.save_slots()
    return { "slot_one", "slot_two" }, nil, {
        "startup.option.slot_one",
        "startup.option.slot_two",
    }
end

function Startup.archetypes()
    return { "samurai", "decker", "gator_shaman" }, nil, {
        "startup.option.samurai",
        "startup.option.decker",
        "startup.option.gator_shaman",
    }
end

-- Portraits are presentation content attached to stable archetype IDs. The
-- selected archetype's gameplay record remains the extracted ROM template.
function Startup.archetype_portraits()
    return {
        samurai = "menu.field.runner.portrait.0",
        decker = "menu.field.runner.portrait.1",
        gator_shaman = "menu.field.runner.portrait.2",
    }
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

-- Opening-intro presentation holds, keyed by the conversion's source
-- boundary names. Each value is "<kind>.<frames>": the host keeps the
-- current picture ("freeze") or shows black ("black") for that many VBlanks
-- before the cutscene (or the field handoff) advances again. These are the
-- ROM's blocking calls that run no intro logic: the 0x53126 audio fade
-- (6 x 3 VBlanks), 0x131C/0x12F6 passes over an already black screen
-- (17 x 2 VBlanks) and CPU-bound decode/load frames that never reach
-- 0x0BD2. Counts come from the ROM call structure plus a per-frame
-- emulator trace (wait_for_vblank call counter FFDF88); see
-- docs/ghidra/visual-audits/opening-intro-timeline.md.
local OPENING_HOLD = {
    AUDIO_FADE_0x53126 = 18,     -- 0x53126: six steps, wait_for_vblank_frames(2)
    PALETTE_PASSES_0x131C = 34,  -- 0x131C / 0x12F6: 17 passes x 2 VBlanks
}
Startup.OPENING_HOLD = OPENING_HOLD

function Startup.opening_intro_timeline()
    local H = OPENING_HOLD
    return {
        -- 0x5AF62 -> 0x1406: 0x131C over the faded menu, then 22 decode
        -- frames before the Seattle plate (emulator: song 0x10 onset ->
        -- plate = 56 frames). [C emulator trace]
        ["0x5AF62.load_title_font"] = "black." .. (H.PALETTE_PASSES_0x131C + 22),
        -- The Seattle schedule's still tail before 0x53126 starts (emulator:
        -- last visual change -> song 0x10 still at full level for 10 more
        -- frames). Includes the one deferral frame. [H: tail record timing]
        ["0x5AF62.schedule_complete"] = "freeze.9",
        -- 0x53126 with the last Seattle frame up; song 0x11 starts when it
        -- ends (emulator: 0x11 onset, then 0x131C two frames later).
        ["0x59D7C.begin_stage2"] = "freeze." .. H.AUDIO_FADE_0x53126,
        -- 0x1406 decode of the wilderness resources (30 lag frames).
        ["0x59D9C.install_wilderness_map_layers"] = "black.30",
        -- After the ambush loop: 0x53126 with the last wilderness frame up.
        ["0x5AA96.begin_trivid_orchestrator"] = "freeze." .. H.AUDIO_FADE_0x53126,
        -- 0x5AA96 clears the planes and decodes the tri-vid (22 lag frames).
        ["0x5AA96.decode_trivid_source_streams"] = "black.22",
        -- 0x5AD1A -> 0x1406 decode after its 0x131C (23 lag frames).
        ["0x5AD1A.prepare_airport_resources"] = "black.23",
        -- 0x59D12: 0x1406 (0x131C on black + 1 lag), plane clears, 0x12F6 on
        -- the cleared planes, then 9 lag frames of 0x134D2 setup.
        ["0x59D12.enter_post_intro_mode"] =
            "black." .. (H.PALETTE_PASSES_0x131C + 1 + H.PALETTE_PASSES_0x131C + 9),
        -- 0x59CFE: 0x53126, then the 0x07F0 field setup until its first
        -- picture (91 frames in the trace, 37 of them lag frames).
        ["0x59CFE.prepare_mode_entry"] = "black." .. (H.AUDIO_FADE_0x53126 + 91),
        -- 0x07F0 field entry: the first field picture is up while 0x12F6
        -- runs before the main loop starts Jake's opening walk (emulator:
        -- field visible 30 frames before the first step). [C]
        ["field.opening_auto_walk.armed"] = "freeze.30",
        -- Motel-sign trigger: after the field fade, 0x53126 and the dialogue
        -- resource load keep the screen black (emulator: 18 + 20 frames). [C]
        ["field.opening_auto_walk.consumed"] = "black." .. (H.AUDIO_FADE_0x53126 + 20),
    }
end

-- Audio setup gaps the host runs in one frame: boundary -> frames between
-- the end of the preceding 0x53126 and the next audio command.
function Startup.opening_audio_waits()
    return {
        -- New Game 0x07F0 field setup runs 8 frames after its 0x53126
        -- before 0x2900 play_music (emulator: fade end 3554, song 0x0F
        -- 3562). [C]
        ["field.opening_auto_walk.scene_music"] = "8",
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
