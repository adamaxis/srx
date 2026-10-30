-- Hotel Karma upgrade rules from ROM 0xD82A/0xD3EA [C].
local E = require("enums")
local K = require("karma")
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

print(string.format("karma: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
