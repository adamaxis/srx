-- ROM 0x53B78/0x53BCC/0x53CEA title services for scene 6/event 0.
-- The scripts express the prompt graph in F4B68/F4B84/F4BA0 as Lua pages;
-- the field host only supplies the location, portraits, and input operations.
local E = require("enums")
local StoryFlags = require("story_flags")

local Owlfeather = {}

local LOCATION_ID = "salish_owlfeather_ranger_lodge"
local land_information
local choose_topics
local council_information
local sinsearach_information
local village_information
local ranger_information

local function at_lodge()
    return game.location.type == "building" and
           game.location.id == LOCATION_ID and
           game.field.mode == 6 and game.field.event_code == 0
end

local function say(text, continuation)
    game.ui.say_text(text, continuation == true)
end

local function more(menu_id, text)
    return game.ui.prompt_text(menu_id, text, { "more" }) == "more"
end

local function choose(menu_id, text, options)
    return game.ui.prompt_text(menu_id, text, options)
end

local function farewell()
    say("Sure thing.||You just keep your nose, and these lands, clean.", true)
    say("Farewell, and may the spirits guide you.", false)
    return "field_return"
end

council_information = function()
    if not more("council_more", table.concat({
        "The Council Island was established by the Salish-Shidhe Council to put a buffer between those of us who prefer the beauty of nature to the perverse ugliness of technology.",
    }, "||")) then
        return "field_return"
    end
    return choose_topics("council_topics", table.concat({
        "There we have established a Passport Lodge, a Medicine Lodge dedicated to the shamanic tradition of magic, a hotel, a hospital, and a restaurant.",
        "There is also an Ork Embassy in the southwest corner of the island.",
    }, "||"))
end

sinsearach_information = function()
    local choice = choose("sinsearach", table.concat({
        "The Sinsearach are a tribe which consists mostly of xenophobic elves.",
        "They've staked a claim on the lands south of here and have erected walls and patrols to keep others out.",
    }, "||"), { "lands", "council", "thanks" })
    if choice == "lands" then return land_information() end
    if choice == "council" then return council_information() end
    return farewell()
end

village_information = function()
    local choice = choose("villages", table.concat({
        "Neg on that.  Most of our people like to wander the land.",
        "This AmerIndian village was built close to Council Island for trade purposes.",
        "All the tribes, with the exception of the Sinsearach, do trade through this area.",
    }, "||"), { "sinsearach", "thanks" })
    if choice == "sinsearach" then return sinsearach_information() end
    return farewell()
end

ranger_information = function()
    local choice = choose("rangers", table.concat({
        "The Salish-Shidhe Rangers were formed to patrol the wilderness in an attempt to restore and maintain its beauty.",
        "The rangers also serve as a kind of police force for the region.",
    }, "||"), { "villages", "thanks" })
    if choice == "villages" then return village_information() end
    return farewell()
end

land_information = function()
    local choice = choose("lands", table.concat({
        "Much of the north-western tribal lands were reclaimed by AmerIndians from many differing tribes.",
        "The Salish-Shidhe Council was formed to control the land that encompasses the blight of Seattle.",
    }, "||"), { "rangers", "thanks" })
    if choice == "rangers" then return ranger_information() end
    return farewell()
end

choose_topics = function(menu_id, text)
    local choice = choose(menu_id, text, { "lands", "council", "thanks" })
    if choice == "lands" then return land_information() end
    if choice == "council" then return council_information() end
    return farewell()
end

local function brother_information()
    if not more("brother_more", table.concat({
        "You pull his holopix from your pocket and give a brief synopsis of the events leading up to Tabatha's murder.",
        "Owlfeather seems very disturbed by your story.",
    }, "||")) then
        return "field_return"
    end

    local learned = choose("brother_status", table.concat({
        "I am sorry to hear of another senseless death.",
        "I fear I was of little help in her quest to find Michael's killers.",
        "Perhaps if I had been able to learn more, she would still be alive today.",
    }, "||"), { "learned" })
    if learned ~= "learned" then return "field_return" end

    if not more("status_more", table.concat({
        "I have not made much progress.",
        "It seems a great amount of interest has been generated in our land lately, and many strangers have been visiting here.",
        "Your brother was one of them.",
    }, "||")) then
        return "field_return"
    end

    local like_who = choose("status_topic", table.concat({
        "After his death, Tabatha gave me many leads, all to no avail.",
        "Something has the people of this region scared into silence.",
        "Perhaps we need the help of one who knows more.",
    }, "||"), { "like_who" })
    if like_who ~= "like_who" then return "field_return" end

    local task = choose("simple_task", table.concat({
        "If you require his help, I must first ask for yours.",
        "If you are willing to perform a simple task, I will give you the name of someone who may further help you.",
    }, "||"), { "task" })
    if task ~= "task" then return "field_return" end

    -- Dialogue command 0x3C at 0x21ED0 sets FFFC30 bit 10 and opens F50C8.
    game.state.field_mode_effect_flag_bit_10 = true
    return choose_topics("task_result", table.concat({
        "Ironically, my own brother, Aragorn, left our land two years ago and has never returned.",
        "We have not heard from him since he entered the sprawl to seek his fortune.",
        "Return Aragorn to me and I will help you.",
    }, "||"))
end

function Owlfeather.present_first_contact()
    if not at_lodge() or
       StoryFlags.is_set(E.StoryFlag.TARISLAR_ASKED_SHALE) ~= true or
       StoryFlags.is_set(E.StoryFlag.OWLFEATHER_ASKED) ~= false then
        return "denied"
    end

    -- 0x53B78 requires story bit 1, clear bit 2, scene 6/event 0. It clears
    -- FFFC30 bits 8/9 before opening F4B68 with profile 0x121E14.
    StoryFlags.begin_trigger(E.StoryFlag.OWLFEATHER_ASKED)
    game.state.field_mode_effect_flag_bit_8 = false
    game.state.field_mode_effect_flag_bit_9 = false
    local choice = choose("opening", table.concat({
        "As you enter the tribal council building a Ranger approaches you.",
        "His ID tag reads:",
        '  "D. Owlfeather".',
        "How can I help you this fine day?",
    }, "||"), { "lands", "council", "brother" })
    if choice == "lands" then return land_information() end
    if choice == "council" then return council_information() end
    if choice ~= "brother" then return "field_return" end

    -- Dialogue command 0x17 first calls 0x59C34 with D0=1, then marks the
    -- active service bit and opens F5064.
    if game.services.call("runner", "adjust_source_byte_triplet_1") ~=
       "applied" then
        return "field_return"
    end
    StoryFlags.apply_dialogue_command(0x17)
    return brother_information()
end

function Owlfeather.present_search_followup()
    if not at_lodge() or
       StoryFlags.is_set(E.StoryFlag.OWLFEATHER_ASKED) ~= true or
       StoryFlags.is_set(E.StoryFlag.ARAGORN_FOUND) ~= false then
        return "denied"
    end

    -- Scene 6/event 0 branch at 0x53C50 opens F4B84 and clears FFFC30 bit 9.
    StoryFlags.begin_trigger(E.StoryFlag.ARAGORN_FOUND)
    game.state.field_mode_effect_flag_bit_9 = false
    local choice = choose("search_followup", table.concat({
        "Still no luck, eh?",
        "Well, I must insist you keep searching.",
        "I just know that Aragorn is alive and in need of help.",
    }, "||"), { "lands", "council", "later" })
    if choice == "lands" then return land_information() end
    if choice == "council" then return council_information() end
    return farewell()
end

local function returned_brother_sequence()
    if not more("return_more", table.concat({
        "<The station doors swing wide and Aragorn swaggers through, head held high.  He speaks with arrogant dignity.>",
        "I have returned, brother!  The evil stain of Seattle was no match for me.",
        "<His last few words quaver in his throat as he recognizes you.>",
    }, "||")) then
        return "field_return"
    end
    -- Command 0x41 opens the remaining F50F8 -> F5134 Owlfeather pages.
    if not more("return_more", table.concat({
        "<Owlfeather quickly clasps Aragorn in a huge embrace, winking knowingly at you over his brother's shoulder.>",
    }, "||")) then
        return "field_return"
    end
    if not more("return_more", table.concat({
        "Brother, welcome back to our land!  Make yourself comfortable in the back so I can finish biz here.",
        "<Aragorn strolls to the back rooms, pausing long enough to give you a warning glare over his shoulder.>",
    }, "||")) then
        return "field_return"
    end
    if not more("return_more", table.concat({
        "You must forgive him.  He is much too proud to admit defeat.",
        "I, however, am grateful for his return.",
        "The best man to aid you is my honored elder Spirit Eyes.",
        "He is wise in the way of things unseen.",
    }, "||")) then
        return "field_return"
    end
    return choose_topics("task_result", table.concat({
        "Go to the Medicine Lodge, and talk to the shaman there.",
        "Tell him you wish to speak with our most wise one.",
        "Farewell, Joshua, and Godspeed on your quest.",
    }, "||"))
end

function Owlfeather.present_return_followup()
    if not at_lodge() or
       StoryFlags.is_set(E.StoryFlag.ARAGORN_FOUND) ~= true or
       StoryFlags.is_set(E.StoryFlag.ARAGORN_FREED_TOLD) ~= false then
        return "denied"
    end

    StoryFlags.begin_trigger(E.StoryFlag.ARAGORN_FREED_TOLD)
    if game.services.call("runner", "adjust_source_byte_triplet_2") ~=
       "applied" then
        return "field_return"
    end
    for _, bit in ipairs({ 8, 9, 10, 13 }) do
        game.state["field_mode_effect_flag_bit_" .. bit] = false
    end
    game.state.field_mode_effect_flag_bit_14 = true

    local choice = choose("return_followup", table.concat({
        "Still no luck, eh?",
        "Well, I must insist you keep searching.",
        "I just know that Aragorn is alive and in need of help.",
    }, "||"), { "freed" })
    if choice == "freed" then
        StoryFlags.apply_dialogue_command(0x19)
        returned_brother_sequence()
    end
    -- Handler 4 calls 0x2196C at 0x53D4E after the conversation even when
    -- command 0x19 was not selected.
    StoryFlags.finish_trigger(E.StoryFlag.ARAGORN_FREED_TOLD)
    return "field_return"
end

return Owlfeather
