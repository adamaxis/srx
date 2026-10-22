package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" ..
    "decompilation/gameplay/scripts/tools/?.lua;" ..
    "decompilation/gameplay/tests_lua/?.lua;" .. package.path

local Json = require("json")
local Harness = {}
Harness.__index = Harness

local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local function load_content()
    return {
        speakers = load_json("dialogue/speakers"),
        menus = load_json("dialogue/menus"),
        contacts = load_json("dialogue/contacts"),
        conditions = load_json("dialogue/conditions"),
        cinematics = load_json("dialogue/cinematics"),
        conversations = {
            vid_phone_services = load_json(
                "dialogue/conversations/vid_phone_services"),
        },
    }
end

function Harness.new(contact_id)
    local self = setmetatable({
        contact_id = contact_id,
        speaker_id = "vid_phone_" .. contact_id,
        menus = {},
        pages = {},
        operations = {},
        choices = {},
        service_results = {},
        active_slot = 0,
        reputation = 5,
    }, Harness)
    self.content = load_content()
    local text_file = assert(io.open(
        "decompilation/assets/dialogue/script-text.json", "rb"))
    self.script_text = Json.decode(text_file:read("*a")).texts
    text_file:close()

    game = {
        content = self.content,
        state = {},
        location = { type = "field", id = "scene_0_terminal" },
        field = { mode = 0, event_code = 31, alternate_result = 0 },
        ui = {},
        services = {},
    }
    game.party = function()
        return { [self.active_slot] = {
            skills = self.reputation == nil and {} or {
                [10] = self.reputation,
            },
        } }
    end
    game.active_slot = function() return self.active_slot end
    game.ui.cursor_menu = function(menu_id, choices, selected_index,
                                   speaker_id, text_profile)
        assert(menu_id == "contacts" and selected_index == 1)
        assert(speaker_id == "" and text_profile == "field.vid_phone")
        assert(#choices == 1 and choices[1] == self.contact_id,
            "test contact must be the only available Vid-Phone contact")
        return "confirm"
    end
    game.ui.menu = function(menu_id, choice_ids, speaker_id, text_profile)
        assert(speaker_id == self.speaker_id,
            "Vid-Phone menu used the wrong contact speaker: " .. menu_id)
        assert(text_profile == "field.vid_phone")
        local profile = self.content.menus.profiles[text_profile][menu_id]
        assert(profile ~= nil, "missing Vid-Phone menu " .. menu_id)
        assert(self.script_text[profile.title_key] ~= nil,
            "missing Vid-Phone menu title " .. tostring(profile.title_key))
        for _, choice_id in ipairs(choice_ids) do
            local label_key = profile.choice_label_keys[choice_id]
            assert(label_key ~= nil and self.script_text[label_key] ~= nil,
                "missing Vid-Phone menu label " .. menu_id .. "." .. choice_id)
        end
        self.menus[#self.menus + 1] = {
            id = menu_id,
            choices = choice_ids,
        }
        local selected = table.remove(self.choices, 1)
        assert(selected ~= nil, "unexpected Vid-Phone menu " .. menu_id)
        for _, choice_id in ipairs(choice_ids) do
            if choice_id == selected then return selected end
        end
        error("choice " .. selected .. " is not available in " .. menu_id)
    end
    game.ui.say = function(event_id, speaker_id, text_profile)
        assert(speaker_id == self.speaker_id,
            "Vid-Phone page used the wrong speaker: " .. event_id)
        assert(text_profile == "field.vid_phone")
        local event = self.content.speakers.profiles[speaker_id].events[event_id]
        assert(event ~= nil, "missing Vid-Phone event " .. event_id)
        assert(self.script_text[event.title_key] ~= nil,
            "missing Vid-Phone text " .. tostring(event.title_key))
        self.pages[#self.pages + 1] = event_id
        return true
    end
    game.services.call = function(domain, operation)
        assert(domain == "vid_phone")
        self.operations[#self.operations + 1] = operation
        return self.service_results[operation]
    end

    self.activities = require("field_activities")
    return self
end

function Harness:reset(called, choices, service_results, reputation,
                       affiliations)
    self.menus = {}
    self.pages = {}
    self.operations = {}
    self.choices = choices or {}
    self.service_results = service_results or {}
    if reputation == "unknown" then
        self.reputation = nil
    else
        self.reputation = reputation == nil and 5 or reputation
    end
    affiliations = affiliations or {}
    local contact
    for _, candidate in ipairs(self.content.contacts.contacts) do
        if candidate.id == self.contact_id then contact = candidate break end
    end
    assert(contact ~= nil, "unknown Vid-Phone test contact " .. self.contact_id)
    game.state = {
        vid_phone_contacts_resolved = true,
        [contact.availabilityFact] = true,
        [contact.callHistoryFact] = called,
        vid_phone_affiliation_mafia = affiliations.mafia or false,
        vid_phone_affiliation_yakuza = affiliations.yakuza or false,
    }
end

function Harness:run()
    return self.activities.vid_phone()
end

function Harness:expect_finished()
    assert(self:run() == nil,
        "Vid-Phone graph did not return to the field terminal")
    assert(#self.choices == 0, "Vid-Phone graph left unused choices")
end

function Harness:expect_boundary()
    local outcome = self:run()
    assert(type(outcome) == "table" and outcome.type == "source_boundary",
        "unresolved Vid-Phone result must remain a source boundary")
    assert(#self.choices == 0, "Vid-Phone boundary left unused choices")
    return outcome
end

function Harness:expect_menu(menu_id)
    for _, menu in ipairs(self.menus) do
        if menu.id == menu_id then return end
    end
    error("Vid-Phone graph did not present menu " .. menu_id)
end

function Harness:expect_page(event_id)
    for _, page in ipairs(self.pages) do
        if page == event_id then return end
    end
    error("Vid-Phone graph did not present page " .. event_id)
end

function Harness:expect_transaction(operation)
    assert(#self.operations == 1 and self.operations[1] == operation,
        "Vid-Phone graph requested the wrong transaction")
end

return Harness
