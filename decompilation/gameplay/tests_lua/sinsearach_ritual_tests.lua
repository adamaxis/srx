package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local value = Json.decode(file:read("*a"))
    file:close()
    return value
end

local E = require("enums")
local Gate = require("salish_sinsearach_gate")
local RedBuffalo = require("salish_red_buffalo")
local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    conditions = load_json("dialogue/conditions"),
    cinematics = load_json("dialogue/cinematics"),
    conversations = {
        salish_sinsearach_gate = load_json(
            "dialogue/conversations/salish_sinsearach_gate"),
    },
}
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

local failures, checks = 0, 0
local function check(ok, label)
    checks = checks + 1
    if not ok then
        failures = failures + 1
        print("FAIL " .. label)
    end
end

local function copy(values)
    local out = {}
    for key, value in pairs(values or {}) do out[key] = value end
    return out
end

local function new_game(overrides, choices, stop_on_event, stop_on_prompt)
    local state = {
        michael_mission_told = true,
        ritual_of_binding = true,
        red_buffalo_woman = true,
        ilene_met = true,
        gillian_initiation = true,
        frosty_met = false,
        shaman_tasks_done = false,
        ["group_item.gift_red_buffalo_woman"] = false,
        ["group_item.gift_lady_gillian"] = false,
        field_dialogue_flag_bit_4 = false,
        field_mode_flag_bit_23 = true,
        field_mode_flag_bit_28 = false,
        field_mode_flag_bit_29 = false,
        field_mode_flag_bit_30 = false,
        field_mode_flag_bit_31 = false,
        field_mode_effect_flag_bit_1 = true,
        field_mode_effect_flag_bit_2 = false,
        field_mode_effect_flag_bit_3 = false,
        field_mode_effect_flag_bit_29 = true,
        field_mode_effect_flag_bit_30 = false,
        notebook_tip_flag_bit_5 = true,
        notebook_tip_flag_bit_6 = false,
        notebook_tip_flag_bit_82 = true,
        notebook_tip_flag_bit_84 = true,
    }
    for key, value in pairs(overrides or {}) do state[key] = value end
    local instance = {
        content = content,
        state = state,
        field = {},
        location = { type = "site", id = "salish_sinsearach_village_gate" },
        wallet = 5000,
        choices = copy(choices),
        pages = {},
        menus = {},
        runner_karma = { 10, 20, 30 },
        stop_on_event = stop_on_event,
        stop_on_prompt = stop_on_prompt,
    }
    local function take_choice(prompt_id, available)
        local selected = table.remove(instance.choices, 1)
        if selected == nil then
            check(false, "choice supplied for " .. prompt_id)
            return available[1]
        end
        for _, choice in ipairs(available) do
            if choice == selected then return selected end
        end
        check(false, "choice " .. selected .. " available for " .. prompt_id)
        table.insert(instance.choices, 1, selected)
        return available[1]
    end
    local function validate_menu(menu_id, choice_ids, speaker_id, text_profile)
        local profile = content.menus.profiles[text_profile]
        local menu = profile and profile[menu_id]
        check(menu ~= nil, "menu " .. tostring(menu_id) .. " exists")
        if menu == nil then return end
        for _, choice_id in ipairs(choice_ids) do
            check(menu.choice_label_keys[choice_id] ~= nil and
                  script_text[menu.choice_label_keys[choice_id]] ~= nil,
                  "label exists for " .. tostring(choice_id))
        end
    end
    instance.ui = {
        say = function(event_id, speaker_id, text_profile)
            local profile = content.speakers.profiles[speaker_id]
            local event = profile and profile.events[event_id]
            check(event ~= nil, "dialogue event " .. tostring(event_id) .. " exists")
            local key = event and event.title_key or ""
            check(script_text[key] ~= nil, "ROM page text exists for " .. key)
            instance.pages[#instance.pages + 1] = { text_key = key, event_id = event_id }
            if event_id == instance.stop_on_event or
               event_id == "elders_escort" then return false end
            if event and event.continuation_key == "shared.more" then
                take_choice("more:" .. key, { "more" })
            end
            return true
        end,
        prompt_text_key = function(menu_id, text_key, choice_ids,
                                   speaker_id, text_profile)
            check(script_text[text_key] ~= nil,
                  "ROM prompt text exists for " .. text_key)
            validate_menu(menu_id, choice_ids, speaker_id, text_profile)
            instance.pages[#instance.pages + 1] = {
                text_key = text_key, menu_id = menu_id, choice_ids = choice_ids,
            }
            instance.menus[#instance.menus + 1] = menu_id
            if text_key == instance.stop_on_prompt then return nil end
            return take_choice(menu_id .. ":" .. text_key, choice_ids)
        end,
        menu = function(menu_id, choice_ids, speaker_id, text_profile)
            validate_menu(menu_id, choice_ids, speaker_id, text_profile)
            instance.menus[#instance.menus + 1] = menu_id
            return take_choice(menu_id, choice_ids)
        end,
        prompt_text = function(menu_id, text, choice_ids)
            return take_choice(menu_id .. ":" .. text, choice_ids)
        end,
        say_text = function(text) instance.pages[#instance.pages + 1] = { text = text } end,
    }
    instance.services = {
        call = function(domain, operation)
            if domain == "party" then
                local amount = tonumber(operation:match("^award_karma_(%d+)$"))
                if amount == nil then return "service_unavailable" end
                for slot = 1, #instance.runner_karma do
                    instance.runner_karma[slot] =
                        instance.runner_karma[slot] + amount
                end
                return "applied"
            end
            return "service_unavailable"
        end,
    }
    instance.nuyen = function() return instance.wallet end
    instance.set_nuyen = function(value) instance.wallet = value end
    game = instance
    return instance
end

local function has_page(instance, key)
    for _, page in ipairs(instance.pages) do
        if page.text_key == key then return true end
    end
    return false
end

local function run_case(label, flags, expected_page, choices, stop_on_event,
                        stop_on_prompt)
    if stop_on_event == nil then
        stop_on_event = expected_page:match("%.page%.([%w_]+)$")
    end
    local instance = new_game(flags, choices, stop_on_event, stop_on_prompt)
    local result = Gate.present()
    check(has_page(instance, expected_page), label .. " opens " .. expected_page)
    return instance, result
end

-- Command 0x34's ordered selector has eight branches; the ritual-start
-- branch has two distinct roots, both tested here.
run_case("0x21CE4 bit13 clear", {
    michael_mission_told = false,
}, "salish.sinsearach.page.elders_escort",
   { "elders", "thanks" })

local reentry = run_case("0x21CE4 bit17 clear with FFFC26 bit4 set", {
    ritual_of_binding = false,
    field_dialogue_flag_bit_4 = true,
}, "salish.sinsearach.page.gillian_reentry_intro",
   { "elders", "thanks", "more" },
   "gillian_reentry_intro")
check(reentry.state.field_dialogue_flag_bit_4 == false,
      "re-entry clears FFFC26 bit4 before its first page")

local first_audience = run_case("0x21CE4 bit17 clear with FFFC26 bit4 clear", {
    ritual_of_binding = false,
}, "salish.sinsearach.page.gillian_first_audience",
   { "elders", "thanks" })
check(first_audience.state.field_mode_flag_bit_28 == true and
      first_audience.state.field_dialogue_flag_bit_4 == false and
      first_audience.state.ritual_of_binding == false and
      first_audience.state["group_item.gift_red_buffalo_woman"] == false,
      "first-audience route sets only FFFC30 bit28 before the Yes/No record")
local first_audience_no = run_case("first audience No command 0x35", {
    ritual_of_binding = false,
}, "salish.sinsearach.page.gillian_first_audience",
   { "elders", "thanks", "more", "no", "more" }, "ritual_no_response")
check(first_audience_no.state.field_mode_flag_bit_28 == true and
      first_audience_no.state.field_dialogue_flag_bit_4 == true and
      first_audience_no.state.ritual_of_binding == false and
      first_audience_no.state["group_item.gift_red_buffalo_woman"] == false,
      "0x21DCC No sets FFFC30 bit28 and FFFC26 bit4 without starting the ritual")

run_case("0x21CE4 bit18 clear", {
    red_buffalo_woman = false,
}, "salish.sinsearach.page.ritual_friendship_not_done",
   { "elders", "thanks" })

local offering = run_case("0x21CE4 Lady Gillian gift held", {
    ["group_item.gift_lady_gillian"] = true,
}, "salish.sinsearach.page.gillian_return_offering",
   { "elders", "thanks" })
check(offering.state["group_item.gift_lady_gillian"] == false and
      offering.runner_karma[1] == 11 and
      offering.runner_karma[2] == 21 and
      offering.runner_karma[3] == 31 and
      offering.state.field_mode_flag_bit_30 == false and
      offering.state.field_mode_flag_bit_31 == true and
      offering.state.field_mode_effect_flag_bit_29 == true and
      offering.state.field_mode_effect_flag_bit_30 == false,
      "0x21D60 consumes Lady Gillian's gift, awards +1 each, and sets FFFC30 bit31")

run_case("0x21CE4 bit19 clear", {
    ilene_met = false,
}, "salish.sinsearach.page.ritual_fellowship_not_done",
   { "elders", "thanks" })

local fellowship = run_case("0x21CE4 bit20 clear", {
    gillian_initiation = false,
}, "salish.sinsearach.page.gillian_trial_fellowship",
   { "elders", "thanks" })
local banished = new_game({ gillian_initiation = false }, {
    "elders", "thanks", "more", "no", "more",
}, "ritual_banish")
Gate.present()
check(fellowship.state.field_mode_effect_flag_bit_1 == true and
      fellowship.state.field_mode_effect_flag_bit_3 == false,
      "bit20-clear outcome leaves FFFC34 unchanged before a choice")
check(has_page(banished, "salish.sinsearach.page.gillian_trial_fellowship") and
      banished.state.field_mode_effect_flag_bit_1 == false and
      banished.state.field_mode_effect_flag_bit_3 == true,
      "0x21DEC No clears FFFC34 bit1 and sets bit3")

run_case("0x21CE4 bit30 clear", {
    frosty_met = false,
}, "salish.sinsearach.page.lady_gillian_greeting",
   { "elders", "thanks" }, nil,
   "salish.sinsearach.page.lady_gillian_greeting")
run_case("0x21CE4 all gates passed and bit30 set", {
    frosty_met = true,
}, "salish.sinsearach.page.gillian_busy",
   { "elders", "thanks" })

-- Production path: first audience -> command 0x1B -> Red Buffalo exchange ->
-- command 0x34 gift delivery -> Trial of Fellowship -> command 0x37.
local journey = new_game({
    michael_mission_told = true,
    ritual_of_binding = false,
    red_buffalo_woman = false,
    ilene_met = false,
    gillian_initiation = false,
    frosty_met = false,
}, { "elders", "thanks", "more", "yes", "more", "more", "more",
     "no", "more" })
local first_result = Gate.present()
local first_audience_ok = first_result.type == "continue_in_place" and
      has_page(journey, "salish.sinsearach.page.gillian_first_audience") and
      journey.state.ritual_of_binding == true and
      journey.state["group_item.gift_red_buffalo_woman"] == true and
      journey.state.field_mode_flag_bit_28 == true and
      journey.state.field_mode_flag_bit_29 == true
check(first_audience_ok,
      "first audience Yes executes 0x1B and gives the bundle")

if first_audience_ok then
journey.location = { type = "building", id = "salish_red_buffalo_lodge" }
journey.choices = { "more", "more" }
RedBuffalo.present_indexed_title_service()
check(journey.state["group_item.gift_red_buffalo_woman"] == false and
      journey.state["group_item.gift_lady_gillian"] == true and
      journey.state.field_mode_flag_bit_29 == false and
      journey.state.field_mode_flag_bit_30 == true and
      journey.state.notebook_tip_flag_bit_5 == true and
      journey.state.notebook_tip_flag_bit_6 == false and
      journey.state.field_mode_effect_flag_bit_29 == true and
      journey.state.field_mode_effect_flag_bit_30 == false,
      "Red Buffalo exchange moves the group gift and changes only FFFC30")

journey.state.ilene_met = true -- independent prerequisite from the Ilene activity.
journey.location = { type = "site", id = "salish_sinsearach_village_gate" }
journey.choices = { "elders", "thanks", "more", "more", "more", "no", "more" }
Gate.present()
check(journey.state["group_item.gift_lady_gillian"] == false and
      journey.runner_karma[1] == 11 and
      journey.runner_karma[2] == 21 and
      journey.runner_karma[3] == 31 and
      journey.state.field_mode_flag_bit_30 == false and
      journey.state.field_mode_flag_bit_31 == true,
      "Lady Gillian consumes the gift once and awards +1 to every runner")

journey.choices = { "elders", "thanks", "more", "yes", "more", "more",
                    "more", "more", "ellisia", "thon", "more", "frosty",
                    "more", "more", "no", "more" }
local before_trial = { journey.runner_karma[1], journey.runner_karma[2],
                       journey.runner_karma[3] }
Gate.present()
check(journey.state.gillian_initiation == true and
      journey.runner_karma[1] == before_trial[1] + 2 and
      journey.runner_karma[2] == before_trial[2] + 2 and
      journey.runner_karma[3] == before_trial[3] + 2 and
      journey.state.field_mode_flag_bit_23 == false and
      journey.state.field_mode_flag_bit_28 == false and
      journey.state.field_mode_effect_flag_bit_1 == false and
      journey.state.field_mode_effect_flag_bit_2 == true and
      journey.state.field_mode_effect_flag_bit_3 == false and
      journey.state.notebook_tip_flag_bit_82 == false and
      journey.state.notebook_tip_flag_bit_84 == false,
      "command 0x37 awards +2 and writes FFFC1E/30/34/38 in the initiation chain")
check(journey.runner_karma[1] == 13 and journey.runner_karma[2] == 23 and
      journey.runner_karma[3] == 33,
      "end-to-end ritual reward totals +3 Karma per runner")
check(has_page(journey, "salish.sinsearach.page.ritual_frosty_not_ready"),
      "command 0xF8 bit9 clear opens 0xF57A8")
end

-- Exercise command 0xF8's bit9-set route and command 0x44 without entering the
-- later Harlequin rewards; the harness stops after the arrival page is selected.
local frosty = new_game({
    michael_mission_told = true,
    ritual_of_binding = true,
    red_buffalo_woman = true,
    ilene_met = true,
    gillian_initiation = false,
    frosty_met = false,
    shaman_tasks_done = true,
}, { "elders", "thanks", "more", "yes", "more", "more", "more", "more",
     "ellisia", "thon", "more", "frosty", "more", "more" }, "frosty_arrival")
local frosty_result = Gate.present()
check(frosty_result.type == "source_boundary" and
      has_page(frosty, "salish.sinsearach.page.ritual_frosty_expected") and
      frosty.state.frosty_met == true,
      "command 0xF8 bit9 set opens 0xF5794 then command44 marks Frosty met")

if failures > 0 then
    print(string.format("Sinsearach ritual tests: %d/%d checks failed", failures, checks))
    os.exit(1)
end
print(string.format("Sinsearach ritual tests passed: %d checks", checks))
