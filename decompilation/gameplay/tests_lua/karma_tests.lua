-- Hotel Karma upgrade rules from ROM 0xD82A/0xD3EA [C].
local E = require("enums")
local K = require("karma")
local AttributeScreen = require("menu.screens.attributes")
local MenuRoute = require("menu.route")
local Combat = require("combat")
local Items = require("items")
local Magic = require("magic")
local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end
local function runner(race, character_class, overrides)
    local profile = {
        race = race,
        character_class = character_class,
        attributes = {
            body = 3, quickness = 0, strength = 0, charisma = 0,
            intelligence = 0, willpower = 5,
        },
        skills = {
            sorcery = 0, firearms = 0, pistols = 0, smgs = 0,
            shotguns = 0, melee_combat = 0, throwing = 0,
            computer = 0, biotech = 0, electronics = 0,
            reputation = 0, negotiation = 0,
        },
    }
    for name, value in pairs(overrides or {}) do
        local section, field = name:match("^([^.]+)%.(.+)$")
        profile[section][field] = value
    end
    return profile
end
local function p(profile, column, row)
    return K.upgrade_preview(profile, column, row)
end

MenuRoute.register(E.MenuScreen.ATTRIBUTES_SKILLS,
                   AttributeScreen.Attributes)
MenuRoute.register(E.MenuScreen.UPGRADE_PROMPT,
                   AttributeScreen.Prompt)

local human = runner(E.RunnerRace.HUMAN, E.RunnerClass.SAMURAI)
local result = p(human, 0, 0)
check(result.status == E.KarmaUpgrade.AVAILABLE and
      result.section == "attribute" and result.field == E.RunnerAttribute.BODY and
      result.next_value == 4 and result.karma_cost == 4,
      "Body upgrade returns named field and next-value cost")
human.attributes.body = 6
check(p(human, 0, 0).status == E.KarmaUpgrade.AT_LIMIT,
      "Human Body maximum is six")

local troll = runner(E.RunnerRace.TROLL, E.RunnerClass.SAMURAI)
troll.attributes.body = 10
result = p(troll, 0, 0)
check(result.status == E.KarmaUpgrade.AVAILABLE and
      result.next_value == 11 and result.karma_cost == 11,
      "Troll Body maximum is eleven")
troll.attributes.body = 11
check(p(troll, 0, 0).status == E.KarmaUpgrade.AT_LIMIT,
      "Troll Body reaches its race-specific ceiling")

local elf = runner(E.RunnerRace.ELF, E.RunnerClass.DECKER)
elf.attributes.quickness = 6
result = p(elf, 0, 1)
check(result.status == E.KarmaUpgrade.AVAILABLE and
      result.field == E.RunnerAttribute.QUICKNESS and
      result.next_value == 7 and result.karma_cost == 7,
      "Elf Quickness can rise to seven")

local dwarf = runner(E.RunnerRace.DWARF, E.RunnerClass.SHAMAN)
dwarf.attributes.willpower = 5
result = p(dwarf, 1, 1)
check(result.status == E.KarmaUpgrade.AVAILABLE and
      result.field == E.RunnerAttribute.WILLPOWER and
      result.next_value == 6,
      "right-column Willpower selects the named field")
check(p(human, 1, 2).status == E.KarmaUpgrade.NOT_UPGRADEABLE,
      "Essence is visible and fixed")
check(p(human, 1, 3).status == E.KarmaUpgrade.NOT_UPGRADEABLE,
      "Magic is visible and fixed")
check(p(runner("unknown", E.RunnerClass.MAGE), 0, 0).status ==
      E.KarmaUpgrade.UNKNOWN_RACE,
      "unmapped race has no upgrade ceiling")

local mage = runner(E.RunnerRace.HUMAN, E.RunnerClass.MAGE,
                    { ["skills.sorcery"] = 2 })
result = p(mage, 0, 4)
check(result.status == E.KarmaUpgrade.AVAILABLE and
      result.next_value == 3 and result.karma_cost == 6,
      "Mage Sorcery uses ordinary doubled cost")
local shaman = runner(E.RunnerRace.HUMAN, E.RunnerClass.SHAMAN)
check(p(shaman, 0, 4).status == E.KarmaUpgrade.AVAILABLE,
      "Shaman can upgrade Sorcery")
for _, character_class in ipairs({ E.RunnerClass.SAMURAI,
                                   E.RunnerClass.DECKER }) do
    check(p(runner(E.RunnerRace.HUMAN, character_class), 0, 4).status ==
          E.KarmaUpgrade.CLASS_RESTRICTED,
          character_class .. " cannot upgrade Sorcery")
end

local trained = runner(E.RunnerRace.HUMAN, E.RunnerClass.SAMURAI, {
    ["skills.firearms"] = 3,
})
result = p(trained, 0, 6)
check(result.status == E.KarmaUpgrade.AVAILABLE and
      result.field == E.RunnerSkill.PISTOLS and
      result.next_value == 4 and result.karma_cost == 6,
      "untrained Pistols inherit Firearms and use rounded cost")
trained.skills.smgs = 2
result = p(trained, 0, 7)
check(result.next_value == 3 and result.karma_cost == 5,
      "trained SMGs ignore Firearms")
trained.skills.melee_combat = 4
check(p(trained, 0, 9).karma_cost == 10,
      "Melee Combat costs twice its next rank")
trained.skills.throwing = 4
result = p(trained, 1, 4)
check(result.field == E.RunnerSkill.THROWING and result.karma_cost == 10,
      "right-column Throwing uses the named skill")
trained.skills.negotiation = 12
check(p(trained, 1, 9).status == E.KarmaUpgrade.AT_LIMIT,
      "skill rank twelve is the cap")
trained.skills.firearms = 12
result = p(trained, 0, 8)
check(result.status == E.KarmaUpgrade.AVAILABLE and
      result.next_value == 13 and result.karma_cost == 20,
      "untrained Shotguns use Firearms twelve for preview")
check(p(trained, 2, 0).status == E.KarmaUpgrade.INVALID,
      "out-of-grid selection is invalid")

-- Independent GPGX runtime oracle for the exact Human/Shaman record shown at
-- frame 9030. The fixture records FFF0B6/FFF0B8 preview RAM plus the committed
-- record and +0x8D Karma after real D-pad/A input. Exercise both production
-- Lua rule modules and the shared menu.route.lua input/prompt path.
local function route_host()
    return {
        blank = function() end,
        mask = function() end,
        redraw = function() end,
        set_window = function() end,
        set_screen = function() end,
        menu_closed = function() end,
        play_sound = function() end,
    }
end

local function source_runner()
    local profile = runner(E.RunnerRace.HUMAN, E.RunnerClass.SHAMAN, {
        ["attributes.body"] = 2,
        ["attributes.quickness"] = 4,
        ["attributes.strength"] = 1,
        ["attributes.charisma"] = 2,
        ["attributes.intelligence"] = 3,
        ["attributes.willpower"] = 4,
        ["skills.sorcery"] = 3,
        ["skills.firearms"] = 2,
        ["skills.pistols"] = 0,
        ["skills.smgs"] = 0,
        ["skills.shotguns"] = 0,
        ["skills.melee_combat"] = 1,
        ["skills.throwing"] = 0,
        ["skills.computer"] = 0,
        ["skills.biotech"] = 0,
        ["skills.electronics"] = 0,
        ["skills.reputation"] = 1,
        ["skills.negotiation"] = 3,
    })
    profile.karma = 20
    return profile
end

local capped_body = source_runner()
capped_body.attributes.body = 6
local cap_route = MenuRoute.new(route_host(), {
    party = function() return { [1] = capped_body } end,
    active_slot = function() return 1 end,
})
cap_route:open(E.MenuScreen.ATTRIBUTES_SKILLS, { karma = true })
check(cap_route:top().screen == E.MenuScreen.ATTRIBUTES_SKILLS and
      cap_route:top().state.message == E.MenuMessage.MAX_LEVEL,
      "ROM maximum-level feedback stays in the Attributes page status area")
cap_route:frame(0)
check(cap_route:top().screen == E.MenuScreen.ATTRIBUTES_SKILLS,
      "maximum-level feedback does not consume the next grid input as a modal")
cap_route:frame(E.Pad.DOWN)
check(cap_route:top().screen == E.MenuScreen.ATTRIBUTES_SKILLS and
      cap_route:top().state.row == 1 and
      cap_route:top().state.message == nil,
      "moving off a capped stat clears its inline maximum-level feedback")

local restricted_profile = source_runner()
restricted_profile.character_class = E.RunnerClass.SAMURAI
local restricted_route = MenuRoute.new(route_host(), {
    party = function() return { [1] = restricted_profile } end,
    active_slot = function() return 1 end,
})
restricted_route:open(E.MenuScreen.ATTRIBUTES_SKILLS, { karma = true })
for _ = 1, 4 do restricted_route:frame(E.Pad.DOWN) end
restricted_route:frame(E.Pad.A)
check(restricted_route:top().screen == E.MenuScreen.ATTRIBUTES_SKILLS and
      restricted_route:top().state.message == E.MenuMessage.MAGE_ONLY,
      "Mage Only feedback stays inline and keeps the grid as route owner")
restricted_route:frame(E.Pad.DOWN)
check(restricted_route:top().state.row == 5 and
      restricted_route:top().state.message == nil,
      "a d-pad event moves directly after inline Mage Only feedback")

local poor_profile = source_runner()
poor_profile.karma = 0
local poor_route = MenuRoute.new(route_host(), {
    party = function() return { [1] = poor_profile } end,
    active_slot = function() return 1 end,
})
poor_route:open(E.MenuScreen.ATTRIBUTES_SKILLS, { karma = true })
poor_route:frame(E.Pad.A)
check(poor_route:top().screen == E.MenuScreen.ATTRIBUTES_SKILLS and
      poor_route:top().state.message == E.MenuMessage.INSUFFICIENT_KARMA,
      "insufficient Karma feedback stays inline without a pushed message page")

local fixture_path = (os.getenv("SHADOWRUN_ROOT") or ".") ..
    "/decompilation/gameplay/tests_lua/fixtures/" ..
    "karma_upgrade_gpgx_frame_9030.tsv"
local fixture = assert(io.open(fixture_path, "r"),
                       "missing GPGX Karma-upgrade fixture")
for line in fixture:lines() do
    if not line:match("^%s*#") and line ~= "" then
        local col, row, section, field, current, next_value, cost,
            committed, karma_before, karma_after = line:match(
                "^(%d+)\t(%d+)\t([^\t]+)\t([^\t]+)\t(%d+)\t(%d+)\t" ..
                "(%d+)\t(%d+)\t(%d+)\t(%d+)$")
        assert(col ~= nil, "malformed GPGX Karma fixture row: " .. line)
        col, row = tonumber(col), tonumber(row)
        current, next_value, cost = tonumber(current), tonumber(next_value),
            tonumber(cost)
        committed, karma_before, karma_after = tonumber(committed),
            tonumber(karma_before), tonumber(karma_after)

        local profile = source_runner()
        profile.karma = karma_before
        local record_section = section == "attribute"
            and "attributes" or "skills"
        local field_profile_value = profile[record_section][field]
        local result = K.upgrade_preview(profile, col, row)
        check(field_profile_value == current and
              result.status == E.KarmaUpgrade.AVAILABLE and
              result.section == section and result.field == field and
              result.next_value == next_value and
              result.karma_cost == cost and
              result.text_tone == E.MenuTextTone.FOCUSED,
              string.format("GPGX preview mismatch at (%d,%d)", col, row))

        local screen_status, screen_next, screen_cost =
            AttributeScreen.Attributes.preview(profile, col, row)
        check(screen_status == E.KarmaUpgrade.AVAILABLE and
              screen_next == next_value and screen_cost == cost,
              string.format("Attributes screen rule mismatch at (%d,%d)",
                            col, row))

        local route_profile = source_runner()
        route_profile.karma = karma_before
        local game_state = {
            party = function()
                local copy = {}
                for key, value in pairs(route_profile) do
                    if key == "attributes" or key == "skills" then
                        local nested = {}
                        for field_name, field_value in pairs(value) do
                            nested[field_name] = field_value
                        end
                        copy[key] = nested
                    else
                        copy[key] = value
                    end
                end
                return { [1] = copy }
            end,
            active_slot = function() return 1 end,
            commit_party = function(party)
                route_profile = party[1]
            end,
            reselect_leader = function() end,
        }
        local route = MenuRoute.new(route_host(), game_state)
        route:open(E.MenuScreen.ATTRIBUTES_SKILLS, { karma = true })
        if col == 1 then route:frame(E.Pad.RIGHT) end
        for _ = 1, row do route:frame(E.Pad.DOWN) end
        local top = route:top()
        local correct_cursor = top ~= nil and top.state.col == col and
            top.state.row == row
        route:frame(E.Pad.A)
        local prompt_open = route:top() ~= nil and
            route:top().screen == E.MenuScreen.UPGRADE_PROMPT
        route:frame(E.Pad.A)
        check(correct_cursor and prompt_open and
              route:top() ~= nil and
              route:top().screen == E.MenuScreen.ATTRIBUTES_SKILLS and
              route_profile[record_section][field] == committed and
              route_profile.karma == karma_after and
              karma_before - route_profile.karma == cost,
              string.format("route.lua GPGX transaction mismatch at (%d,%d)",
                            col, row))
    end
end
fixture:close()

game = { actors = {}, modules = {
    combat = Combat, items = Items, magic = Magic,
} }

local function source_rng(seed)
    return function(bound)
        -- ROM 0x0D44 advances the source seed; 0x0D3A scales to bound.
        seed = (seed * 0x20E5 + 1) & 0xFFFF
        return (seed * bound) >> 16
    end
end

local function source_rolls(count, difficulty, random_bound)
    local successes = 0
    for _ = 1, math.max(count, 1) do
        local accumulated = 0
        while true do
            local face = random_bound(6) + 1
            if face == 6 then
                accumulated = accumulated + face
            else
                if accumulated + face >= difficulty then
                    successes = successes + 1
                end
                break
            end
        end
    end
    return successes
end

local function combat_record(profile)
    return {
        is_party_member = true,
        physical_meter = 10,
        mental_meter = 10,
        posture_reduction_percent = 50,
        attributes = {
            body = profile.attributes.body,
            quickness = 4,
            strength = 4,
            intelligence = 4,
            willpower = 4,
        },
        skills = {
            firearms = profile.skills.firearms,
            pistols = profile.skills.pistols,
            smgs = profile.skills.smgs,
            shotguns = profile.skills.shotguns,
            melee_combat = profile.skills.melee_combat,
            sorcery = profile.skills.sorcery,
        },
        cyberware = {},
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false,
        },
        actor_facts = {
            lifecycle = { active = true, thinking_enabled = true,
                ai_runner = true, downed = false, group_leader = false,
                dead = false },
            interaction = { no_coverage_difficulty_bonus = false,
                unseen = false, defense_difficulty_reduction = false },
        },
        action_profile_options = { primary = false, secondary = false },
        inventory = {},
        weapon_action_policies = {
            american_l36 = {
                skill_group = "pistols",
                difficulty_adjustment = 0,
                benefits_from_targeting_assist = false,
            },
        },
    }
end

local function attack_seed(profile, seed)
    return Combat.resolve_weapon_attack_seed({
        combatant = combat_record(profile),
        projectile_action_id = "american_l36",
        ai_target_candidates = {},
        ai_random_bound = source_rng(seed),
    })
end

-- ROM 0x12C74 chooses max(Firearms, Pistols) then adds the smaller of that
-- value and the posture share from 0x55ED6/0x55F20. Human Pistols 3->4 costs
-- 6 karma at 0xD920; with Q/I/W 4/4/4 and Neutral posture the source pool is
-- 6->7. Seed 0x0000 gives 2->3 successes at difficulty four.
local pistols_profile = runner(E.RunnerRace.HUMAN, E.RunnerClass.SAMURAI, {
    ["skills.firearms"] = 3,
})
local pistols_upgrade = p(pistols_profile, 0, 6)
local pistols_trained = runner(E.RunnerRace.HUMAN, E.RunnerClass.SAMURAI, {
    ["skills.firearms"] = 3,
    ["skills.pistols"] = pistols_upgrade.next_value,
})
local base_pistols_pool = 3 + math.min(3, 3)
local trained_pistols_pool = math.max(3, pistols_upgrade.next_value) +
    math.min(pistols_upgrade.next_value, 3)
check(pistols_upgrade.status == E.KarmaUpgrade.AVAILABLE and
      pistols_upgrade.next_value == 4 and pistols_upgrade.karma_cost == 6 and
      base_pistols_pool == 6 and trained_pistols_pool == 7 and
      attack_seed(pistols_profile, 0x0000) ==
          source_rolls(base_pistols_pool, 4, source_rng(0x0000)) and
      attack_seed(pistols_trained, 0x0000) ==
          source_rolls(trained_pistols_pool, 4, source_rng(0x0000)) and
      attack_seed(pistols_profile, 0x0000) == 2 and
      attack_seed(pistols_trained, 0x0000) == 3,
      "Pistols karma upgrade did not change the seeded 0x12C74 action pool")

local function body_hit(profile, seed)
    local target = {
        id = "karma-body-" .. profile.attributes.body,
        combatant = combat_record(profile),
    }
    local source_successes = source_rolls(
        6 - 3 + profile.attributes.body, 6, source_rng(seed))
    local expected_damage = math.max(0, 1 + 5 - source_successes)
    local result = Combat.resolve_weapon_hit({
        projectile_action_id = "american_l36",
        ai_random_bound = source_rng(seed),
    }, { attack_successes = 5 }, target)
    return result, expected_damage, source_successes
end

-- ROM 0xD82A/0xD3EA sets Human Body 3->4 at karma cost 4. The weapon
-- defender pool is command-average minus posture-share plus Body at 0x55996;
-- seed 0x0003 produces 1->2 defense successes, reducing damage by one.
local body_profile = runner(E.RunnerRace.HUMAN, E.RunnerClass.SAMURAI)
local body_upgrade = p(body_profile, 0, 0)
local body_upgraded = runner(E.RunnerRace.HUMAN, E.RunnerClass.SAMURAI, {
    ["attributes.body"] = body_upgrade.next_value,
})
local base_body_hit, base_body_damage, base_body_successes =
    body_hit(body_profile, 0x0003)
local upgraded_body_hit, upgraded_body_damage, upgraded_body_successes =
    body_hit(body_upgraded, 0x0003)
check(body_upgrade.status == E.KarmaUpgrade.AVAILABLE and
      body_upgrade.next_value == 4 and body_upgrade.karma_cost == 4 and
      base_body_successes == 1 and upgraded_body_successes == 2 and
      base_body_damage == 5 and upgraded_body_damage == 4 and
      base_body_hit.physical_meter == 5 and
      upgraded_body_hit.physical_meter == 6 and
      base_body_hit.mental_meter == 10 and upgraded_body_hit.mental_meter == 10,
      "Body karma upgrade did not change the seeded 0x55948 defense result")

print(string.format("karma: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
