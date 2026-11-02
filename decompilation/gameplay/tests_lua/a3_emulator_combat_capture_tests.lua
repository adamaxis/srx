-- Emulator-backed assertions for the seeded firearm action capture.
-- ROM source: 0x0D3A/0x0D44, 0x0DBA..0x0DFA, 0x55640, 0x556C6,
-- 0x55740, 0x55948..0x559BC, 0x55E92, and 0x55A90.
local Combat = require("combat")
local E = require("enums")
local Items = require("items")
local Magic = require("magic")
game = { actors = {}, modules = { combat = Combat, items = Items, magic = Magic } }
require("actor.init")
local PartyDown = require("party_down")
local ActorAnim = require("actor.anim")

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then
        passed = passed + 1
    else
        print("FAIL " .. label)
    end
end

local function split_tabs(line)
    local fields, start = {}, 1
    while true do
        local first, last = line:find("\t", start, true)
        if first == nil then
            fields[#fields + 1] = line:sub(start)
            break
        end
        fields[#fields + 1] = line:sub(start, first - 1)
        start = last + 1
    end
    return fields
end

local function crc32_update(crc, text)
    for index = 1, #text do
        crc = crc ~ text:byte(index)
        for _ = 1, 8 do
            if (crc & 1) ~= 0 then
                crc = (crc >> 1) ~ 0xEDB88320
            else
                crc = crc >> 1
            end
            crc = crc & 0xFFFFFFFF
        end
    end
    return crc
end

local function faces_successes(faces, difficulty)
    local successes, accumulated = 0, 0
    for index = 1, #faces do
        local face = tonumber(faces:sub(index, index))
        if face == 6 then
            accumulated = accumulated + face
        else
            if accumulated + face >= difficulty then
                successes = successes + 1
            end
            accumulated = 0
        end
    end
    return successes, accumulated == 0
end

local function source_lifecycle_downed(target, physical_meter, mental_meter)
    if target.combatant.is_party_member then
        local down_result = PartyDown.enter_runner_down_state({
            target_role = "leader",
            target_meters = {
                [E.RunnerMeter.PHYSICAL] = physical_meter,
                [E.RunnerMeter.MENTAL] = mental_meter,
            },
            party = {
                leader = { active = true, dead = false },
                companion_one = { active = true, dead = false },
                companion_two = { active = true, dead = false },
            },
        })
        return down_result.lifecycle.downed, false
    end

    local visual_profile = E.ActorVisualProfile.HUMAN_GUARD
    local actor_state = {
        visual_profile_id = visual_profile,
        direction = 0,
        actor_facts = { lifecycle = {
            active = true, ai_runner = false, downed = false, dead = false,
        } },
        incoming_hit = {
            physical_meter = physical_meter,
            mental_meter = mental_meter,
        },
    }
    ActorAnim.begin_received_hit_reaction(actor_state, {
        visual_profile_id = visual_profile,
    })
    return actor_state.actor_facts.lifecycle.downed,
           actor_state.actor_facts.lifecycle.dead == true
end

local PROFILES = {
    decker = {
        rom_record = 0x72A1A, action = "american_l36",
        item_code = E.ItemId.AMERICAN_L36, skill_group = "pistols",
        difficulty_adjustment = 0, difficulty = 6,
        party = true, armor = "armor_clothing", body = 2,
        quickness = 5, strength = 2, intelligence = 4, willpower = 3,
        firearms = 2, pistols = 0, smgs = 0, shotguns = 0,
        cybereyes = false, datajack = true, hand_razors = false,
    },
    weenr2 = {
        rom_record = 0x1D66A4, action = "streetline",
        item_code = E.ItemId.STREETLINE_SPECIAL, skill_group = "pistols",
        difficulty_adjustment = 0, difficulty = 4,
        party = false, armor = "armor_vest", body = 5,
        quickness = 4, strength = 4, intelligence = 2, willpower = 2,
        firearms = 2, pistols = 0, smgs = 0, shotguns = 0,
        cybereyes = false, datajack = false, hand_razors = true,
    },
    yakuza = {
        rom_record = 0x1D66EE, action = "mach_22",
        item_code = E.ItemId.MACH22, skill_group = "smgs",
        -- ROM 0x556C6 automatic-action policy adds three; 0x55740
        -- then applies Yakuza Weenr's cybereyes reduction in Lua.
        difficulty_adjustment = 3, difficulty = 6,
        party = false, armor = "armor_jacket", body = 5,
        quickness = 5, strength = 4, intelligence = 4, willpower = 3,
        firearms = 5, pistols = 0, smgs = 0, shotguns = 0,
        cybereyes = true, datajack = false, hand_razors = true,
    },
}

local function make_actor(profile_name, slot, physical, mental)
    local profile = PROFILES[profile_name]
    local combatant = {
        is_party_member = profile.party,
        physical_meter = physical,
        mental_meter = mental,
        posture_reduction_percent = 50,
        equipped_action_id = profile.action,
        equipped_armor_id = profile.armor,
        force = 0,
        spell_trial_cap = 0,
        attributes = {
            body = profile.body,
            quickness = profile.quickness,
            strength = profile.strength,
            intelligence = profile.intelligence,
            willpower = profile.willpower,
        },
        skills = {
            firearms = profile.firearms,
            pistols = profile.pistols,
            smgs = profile.smgs,
            shotguns = profile.shotguns,
            melee_combat = 0,
            sorcery = 0,
        },
        cyberware = {
            datajack = profile.datajack,
            cybereyes = profile.cybereyes,
            hand_razors = profile.hand_razors,
            spurs = false,
            smartlink = false,
            melee_strength_bonus = 0,
            melee_defense_bonus = 0,
        },
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false,
        },
        actor_facts = {
            lifecycle = {
                active = true,
                thinking_enabled = true,
                ai_runner = profile.party,
                downed = false,
                group_leader = false,
                dead = false,
            },
            interaction = {
                no_coverage_difficulty_bonus = false,
                unseen = false,
                defense_difficulty_reduction = false,
            },
        },
        action_profile_options = { primary = false, secondary = false },
        inventory = {},
        weapon_action_policies = {
            [profile.action] = {
                skill_group = profile.skill_group,
                difficulty_adjustment = profile.difficulty_adjustment,
                benefits_from_targeting_assist = false,
            },
        },
    }
    return {
        id = profile_name .. "_slot_" .. slot,
        slot = slot,
        profile_name = profile_name,
        profile = profile,
        combatant = combatant,
    }
end

local PAIRS = {
    P = {
        names = { "decker", "weenr2" },
        slots = { 0, 6 },
        -- [C] 0xA8EA services the active record array in ascending slot order.
    },
    E = {
        names = { "weenr2", "yakuza" },
        slots = { 6, 7 },
    },
}

local fixture_path =
    "decompilation/gameplay/tests_lua/fixtures/a3_emulator_fight_summary.tsv"
local fixture = assert(io.open(fixture_path, "r"),
                       "missing A3 emulator fixture: " .. fixture_path)
local expected = { P = {}, E = {} }
for line in fixture:lines() do
    if line ~= "" and line:sub(1, 1) ~= "#" and
       line:sub(1, 4) ~= "mode" then
        local fields = split_tabs(line)
        expected[fields[1]][tonumber(fields[2], 16)] = {
            actions = tonumber(fields[3]),
            winner = fields[4],
            a_physical = tonumber(fields[5]),
            b_physical = tonumber(fields[6]),
            final_seed = tonumber(fields[7], 16),
            action_crc32 = tonumber(fields[8], 16),
        }
    end
end
fixture:close()

local function source_rng(seed)
    local rolls = {}
    local function random_bound(bound)
        seed = (seed * 0x20E5 + 1) & 0xFFFF
        local value = (seed * bound) >> 16
        rolls[#rolls + 1] = value + 1
        return value
    end
    return random_bound, function() return seed end, rolls
end

local function simulate_pair(mode, initial_seed)
    game.actors = {}
    game.player_hurt_pending_by_actor_id = {}
    local pair = PAIRS[mode]
    local actor_a = make_actor(pair.names[1], pair.slots[1], 10, 10)
    local actor_b = make_actor(pair.names[2], pair.slots[2], 10, 10)
    local by_slot = { [pair.slots[1]] = actor_a, [pair.slots[2]] = actor_b }
    local random_bound, get_seed, rolls = source_rng(initial_seed)
    local action_count, action_crc = 0, 0xFFFFFFFF
    local anchors = {}
    local party_hurt_pending = false

    -- This controlled capture gives one action per active slot in each pass.
    -- It tests the 0xA8EA ascending service order; it does not model AI or the
    -- action-delay frames between natural combat actions.
    for _ = 1, 40 do
        for _, source_slot in ipairs(pair.slots) do
            local from = by_slot[source_slot]
            local target_slot = source_slot == pair.slots[1] and
                pair.slots[2] or pair.slots[1]
            local target = by_slot[target_slot]
            if from.combatant.physical_meter > 0 and
               target.combatant.physical_meter > 0 then
                action_count = action_count + 1
                local profile = from.profile
                local seed_before = get_seed()
                local physical_before = target.combatant.physical_meter
                local mental_before = target.combatant.mental_meter
                local context = {
                    combatant = from.combatant,
                    projectile_action_id = profile.action,
                    ai_target_candidates = {},
                    ai_random_bound = random_bound,
                }
                local attack_roll_start = #rolls + 1
                local attack_successes =
                    Combat.resolve_weapon_attack_seed(context)
                local attack_faces = table.concat(
                    rolls, "", attack_roll_start, #rolls)
                local defense_roll_start = #rolls + 1
                local hit = Combat.resolve_weapon_hit(
                    context, { attack_successes = attack_successes }, target)
                if hit == nil then
                    check(false, string.format(
                        "%s seed %04X action %d was rejected by production resolver",
                        mode, initial_seed, action_count))
                    return nil
                end
                if hit.changed and game.actors[target.id] ~= nil then
                    target.combatant = game.actors[target.id].combatant
                end
                local physical_after = hit.physical_meter
                local mental_after = hit.mental_meter
                local damage = physical_before - physical_after
                check(physical_after >= 0 and mental_after >= 0,
                    string.format("0x55948 made a negative meter (%s seed %04X action %d)",
                        mode, initial_seed, action_count))
                check(mental_after == mental_before,
                    string.format("0x55948/0x55A90 firearm hit changed mental +0x9C (%s seed %04X action %d)",
                        mode, initial_seed, action_count))
                local downed, dead = source_lifecycle_downed(
                    target, physical_after, mental_after)
                check(downed == (physical_after == 0) and
                      (physical_after ~= 0 or not dead),
                    string.format("0xAFF2/0xB330 down gate disagrees with physical zero (%s seed %04X action %d)",
                        mode, initial_seed, action_count))
                if target.combatant.is_party_member then
                    if physical_after == 0 then
                        party_hurt_pending = false
                    elseif physical_after < physical_before then
                        party_hurt_pending = true
                    end
                    local pending = game.player_hurt_pending_by_actor_id
                    check((type(pending) == "table" and
                           pending[target.id] == true) == party_hurt_pending,
                        string.format("0x55A1E party meter branch disagrees with hurt state (%s seed %04X action %d)",
                            mode, initial_seed, action_count))
                end
                local defense_faces = table.concat(
                    rolls, "", defense_roll_start, #rolls)
                local defense_successes, complete_dice = faces_successes(
                    defense_faces, profile.difficulty)
                check(complete_dice, string.format(
                    "%s seed %04X action %d has an incomplete exploding-d6 stream",
                    mode, initial_seed, action_count))
                local row = table.concat({
                    mode,
                    tostring(initial_seed),
                    tostring(action_count),
                    tostring(source_slot) .. ">" .. tostring(target_slot),
                    string.format("%02X", profile.item_code),
                    string.format("%04X", seed_before),
                    attack_faces,
                    tostring(attack_successes),
                    defense_faces,
                    tostring(defense_successes),
                    tostring(damage),
                    tostring(physical_before),
                    tostring(physical_after),
                    tostring(mental_before),
                    tostring(mental_after),
                    string.format("%04X", get_seed()),
                }, "\t") .. "\n"
                action_crc = crc32_update(action_crc, row)
                anchors[mode .. ":" .. initial_seed .. ":" .. action_count] = {
                    attack = attack_successes,
                    defense = defense_successes,
                    damage = damage,
                    physical_before = physical_before,
                    physical_after = physical_after,
                    mental_before = mental_before,
                    mental_after = mental_after,
                }
            end
        end
        if actor_a.combatant.physical_meter == 0 or
           actor_b.combatant.physical_meter == 0 then
            break
        end
    end

    local first_physical = actor_a.combatant.physical_meter
    local second_physical = actor_b.combatant.physical_meter
    local winner = first_physical > 0 and second_physical == 0 and "A" or
        (first_physical == 0 and second_physical > 0 and "B" or "D")
    return {
        actions = action_count,
        winner = winner,
        a_physical = first_physical,
        b_physical = second_physical,
        final_seed = get_seed(),
        action_crc32 = (~action_crc) & 0xFFFFFFFF,
        anchors = anchors,
        actors = { actor_a, actor_b },
    }
end

local total_actions = { P = 0, E = 0 }
local outcome_counts = {
    P = { A = 0, B = 0, D = 0 },
    E = { A = 0, B = 0, D = 0 },
}
local matched_fights = 0
local anchor_party, anchor_enemy

for _, mode in ipairs({ "P", "E" }) do
    local slots = PAIRS[mode].slots
    check((mode == "P" and slots[1] == 0 and slots[2] == 6) or
          (mode == "E" and slots[1] == 6 and slots[2] == 7),
          mode .. " capture service order must follow ascending 0xA8EA slots")
    check(PAIRS[mode].slots[1] < PAIRS[mode].slots[2],
          mode .. " actor service slots are not ascending per 0xA8EA")
    local seed_rows = expected[mode]
    local seed_count = 0
    for seed = 1, 64 do
        seed_count = seed_count + 1
        local expected_row = seed_rows[seed]
        check(expected_row ~= nil,
              string.format("fixture missing %s seed %04X", mode, seed))
        local measured = simulate_pair(mode, seed)
        if expected_row ~= nil and measured ~= nil then
            local matches = measured.actions == expected_row.actions and
                measured.winner == expected_row.winner and
                measured.a_physical == expected_row.a_physical and
                measured.b_physical == expected_row.b_physical and
                measured.final_seed == expected_row.final_seed and
                measured.action_crc32 == expected_row.action_crc32
            check(matches, string.format(
                "%s seed %04X differs from ROM capture: actions %d/%d winner %s/%s CRC %08X/%08X",
                mode, seed, measured.actions, expected_row.actions,
                measured.winner, expected_row.winner,
                measured.action_crc32, expected_row.action_crc32))
            if matches then
                matched_fights = matched_fights + 1
            end
            total_actions[mode] = total_actions[mode] + measured.actions
            outcome_counts[mode][measured.winner] =
                outcome_counts[mode][measured.winner] + 1
            if mode == "P" and seed == 1 then
                anchor_party = measured.anchors["P:1:1"]
            elseif mode == "E" and seed == 1 then
                anchor_enemy = measured.anchors["E:1:4"]
            end
        end
    end
    check(seed_count == 64, mode .. " capture must cover 64 seeds")
end

check(matched_fights == 128 and total_actions.P == 3504 and
      outcome_counts.P.A == 30 and outcome_counts.P.B == 16 and
      outcome_counts.P.D == 18,
      string.format("Decker/Weenr2 capture is %d fights, %d actions, A/B/D %d/%d/%d",
          matched_fights, total_actions.P, outcome_counts.P.A,
          outcome_counts.P.B, outcome_counts.P.D))
check(total_actions.E == 1028 and outcome_counts.E.A == 0 and
      outcome_counts.E.B == 64 and outcome_counts.E.D == 0,
      string.format("Weenr2/Yakuza capture is %d actions, A/B/D %d/%d/%d",
          total_actions.E, outcome_counts.E.A,
          outcome_counts.E.B, outcome_counts.E.D))
check(anchor_party ~= nil and anchor_party.attack == 2 and
      anchor_party.defense == 0 and anchor_party.damage == 1 and
      anchor_party.physical_before == 10 and
      anchor_party.physical_after == 9 and anchor_party.mental_after == 10,
      "seed 0001 Decker -> Weenr2 must match captured 2/0/1, physical 10->9")
check(anchor_enemy ~= nil and anchor_enemy.attack == 2 and
      anchor_enemy.defense == 2 and anchor_enemy.damage == 1 and
      anchor_enemy.physical_before == 3 and anchor_enemy.physical_after == 2,
      "seed 0001 Yakuza action 4 must match captured 2/2/1, physical 3->2")

print(string.format(
    "a3_emulator_combat_capture_tests: %d/%d passed; production Lua matched %d/4532 actions",
    passed, total, total_actions.P + total_actions.E))
if passed ~= total then
    os.exit(1)
end
