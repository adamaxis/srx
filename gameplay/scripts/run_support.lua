-- Shared source data and helpers for independently-authored run modules.
local Support = {}
local E = require("enums")

Support.area_suffixes = {
    -- ROM F1E58 strings used by the contract location formatter.
    [0] = "in Downtown Seattle.",
    [1] = "in the Puyallup Barrens.",
    [2] = "in the Redmond Barrens.",
    [3] = "in the Penumbra District.",
    [4] = "on Council Island.",
    [5] = "in the Renraku Arcology.",
}

Support.location_names = {
    -- ROM F1F1A's per-area location strings, kept as named Lua data.
    [0] = {
        [0] = "Seattle General Hospital",
        [1] = "Icarus Descending",
        [2] = "Matchsticks",
        [3] = "the Gates Undersound Hotel",
        [4] = "Mitsuhama Computer Technologies",
        [5] = "Fuchi's local office",
        [6] = "the Space Needle",
        [7] = "see Roscoe the Fixer",
        [8] = "OOGA BOOGA BOOGA",
        [9] = "OOGA BOOGA BOOGA",
    },
    [1] = {
        [0] = "the Wanderer",
        [1] = "the Tarislar City Inn",
        [2] = "Ares' Corporate Offices",
        [3] = "Riannon's",
        [4] = "Underground 93",
        [5] = "the Crime Mall",
        [6] = "Dr. Bob's Quickstitch",
        [7] = "the Tarislar Gardens Apartment Complex",
        [8] = "OOGA BOOGA BOOGA",
        [9] = "see the Ork Gang",
    },
    [2] = {
        [0] = "the Hollywood Correctional Facilities",
        [1] = "Little Chiba",
        [2] = "Stoker's Coffin Motel",
        [3] = "the Shiawase Nuke Plant",
        [4] = "the Jump House",
        [5] = "the Jackal's Lantern",
        [6] = "Boris' Greenhouse",
        [7] = "the Rat's Nest",
        [9] = "see the Halloweeners",
        [11] = "Ares Weapon Emporium",
    },
    [3] = {
        [0] = "Club Penumbra",
        [1] = "Wylie's Gala Inn",
        [2] = "Fuchi Industrial Electronics",
        [3] = "Mitsuhama's local office",
        [4] = "see the Eye Fivers",
        [5] = "the Big Rhino",
        [6] = "Lone Star Security",
    },
    [4] = {
        [0] = "the Council Island Inn",
        [1] = "the Friendship Restaurant",
        [2] = "the Passport Lodge",
        [3] = "the Medicine Lodge Hollow",
        [4] = "the Council Island Hospital",
        [5] = "the Ork Embassy",
    },
    [5] = {
        [1] = "the Frag Grenade",
        [2] = "Wire-Masters",
        [3] = "Merlin's Lore",
        [4] = "Microtronics",
        [5] = "Weapons World",
        [6] = "the Renraku Offices",
    },
}

-- ROM F3E80's ten-entry location-index selection for each contract area.
Support.location_rolls_by_area = {
    [0] = { 7, 0, 1, 2, 3, 4, 5, 6, 7, 0 },
    [1] = { 5, 0, 1, 2, 3, 4, 5, 6, 7, 9 },
    [2] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 11 },
    [3] = { 0, 1, 2, 3, 4, 5, 6, 0, 4, 6 },
    [4] = { 0, 1, 2, 3, 4, 5, 2, 3, 4, 5 },
    [5] = { 6, 1, 2, 3, 4, 5, 6, 1, 2, 6 },
}

Support.client_names = {
    -- ROM F35A6 pointer table, selected with the source bound of twelve.
    "Mike Nana", "John Worley", "Ellis Goodson", "John Van Anthony",
    "Dok Whitson", "Jase Weese", "Heinrich Michaels", "Billy Gibson",
    "Nero Manser", "Alan Turing", "Chris Stout", "Jorge Kish",
}

-- ROM 0xF36FA contains the four gang-contract variations shown after the
-- selected target location. These strings are kept as content, separate from
-- route selection and shared payment rules.
Support.gang_contract_texts = {
    "They have been defacing and damaging buildings in that area. I find this appalling!||Show them how I feel about vandals and return to me for your payment.",
    "They have been feeding Renraku a lot of information.||Renraku has been using this info to find and eliminate Johnsons. Needless to say, this makes me uneasy.||\"Discuss\" this matter with them and return here to collect your fee.",
    "Their recent antics have been most heinous!||I want you to tell them to quit terrorizing the locals. And make it stick!||Payment will be rendered when the gang is no longer a problem.",
    "Their payments are late on an insurance policy they have with me.||You will be payed when you return with their premiums.",
}

Support.gang_contract_targets = {
    { area_id = 1, location_id = 9 },
    { area_id = 2, location_id = 9 },
    { area_id = 3, location_id = 4 },
}

-- ROM F3E6E (four-site pool), F3E76 (two-site pool), and the fixed 5/6
-- Renraku tuple used by initializers 10 and 12.
Support.run_target_pools = {
    four_site = {
        { area_id = 0, location_id = 5 },
        { area_id = 0, location_id = 4 },
        { area_id = 3, location_id = 2 },
        { area_id = 3, location_id = 3 },
    },
    two_site = {
        { area_id = 1, location_id = 2 },
        { area_id = 3, location_id = 6 },
    },
    renraku = { area_id = 5, location_id = 6 },
}

function Support.bounded_random(bound)
    local result = game.services.call(
        "random", "bounded_" .. tostring(bound))
    local digits = result:match("^roll_(%d+)$")
    assert(digits ~= nil, "random service returned an invalid bounded roll")
    local roll = tonumber(digits)
    assert(roll ~= nil and roll >= 0 and roll < bound,
        "random service returned a roll outside its bound")
    return roll
end

function Support.negotiate_payment(source_record)
    assert(type(source_record) == "table")
    local base = source_record.base_payment
    local scale = source_record.negotiation_scale
    assert(type(base) == "number" and base >= 0 and base % 1 == 0)
    assert(type(scale) == "number" and scale >= 0 and scale % 1 == 0)

    -- ROM 0x1E69C reads the first live runner's Negotiation skill, draws
    -- below (source row scale * skill) + 1, then adds five per result.
    local result = game.services.call("runner", "negotiation_skill")
    local skill_text = type(result) == "string" and
        result:match("^skill_(%d+)$") or nil
    local skill = skill_text ~= nil and tonumber(skill_text) or nil
    assert(skill ~= nil and skill >= 0 and skill % 1 == 0,
        "runner service returned an invalid Negotiation skill")
    local bound = scale * skill + 1
    local roll = Support.bounded_random(bound)
    return {
        base_amount = base,
        negotiated_amount = base + roll * 5,
        negotiation_skill = skill,
        negotiation_bound = bound,
        negotiation_roll = roll,
    }
end

function Support.amount_in_reserved_field(amount, width)
    assert(type(amount) == "number" and amount >= 0 and amount % 1 == 0)
    assert(type(width) == "number" and width > 0 and width % 1 == 0)
    local digits = tostring(amount)
    assert(#digits <= width,
        "run amount exceeds the ROM payment text field")
    return string.rep(" ", width - #digits) .. digits
end

local function choose_location(area_id)
    local roll = Support.bounded_random(10)
    local local_index = Support.location_rolls_by_area[area_id][roll + 1]
    local name = Support.location_names[area_id][local_index]
    assert(name ~= nil, "source location table entry has no imported name")
    return local_index, name
end

function Support.choose_contract_route(context)
    -- ROM 0x1E9C0 chooses origin area/location, client, then destination
    -- area/location. The opening context fixes both areas to Redmond and
    -- retries the destination location until it differs from the origin.
    local origin_area
    if context == E.RunOfferContext.JUMP_HOUSE then
        origin_area = 2
    else
        origin_area = Support.bounded_random(6)
    end

    local origin_index, origin_name = choose_location(origin_area)
    local client_index = Support.bounded_random(#Support.client_names)
    local client_name = Support.client_names[client_index + 1]

    local destination_area
    if context == E.RunOfferContext.JUMP_HOUSE then
        destination_area = 2
    else
        repeat
            destination_area = Support.bounded_random(6)
        until destination_area ~= origin_area
    end

    local destination_index, destination_name = choose_location(destination_area)
    while context == E.RunOfferContext.JUMP_HOUSE and
          destination_index == origin_index do
        destination_index, destination_name = choose_location(destination_area)
    end

    return {
        origin = {
            area_id = origin_area,
            area_suffix = Support.area_suffixes[origin_area],
            location_id = origin_index,
            location_name = origin_name,
        },
        destination = {
            area_id = destination_area,
            area_suffix = Support.area_suffixes[destination_area],
            location_id = destination_index,
            location_name = destination_name,
        },
        client_id = client_index,
        client_name = client_name,
    }
end

function Support.choose_ghoul_area(context)
    -- ROM 0x1E744 fixes the opening contract to area 2; other contexts use
    -- the four-area random pool from that initializer.
    if context == E.RunOfferContext.JUMP_HOUSE then return 2 end
    return Support.bounded_random(4)
end

local function named_target(target)
    local name = Support.location_names[target.area_id] and
        Support.location_names[target.area_id][target.location_id]
    assert(name ~= nil, "source run target has no imported location name")
    return {
        area_id = target.area_id,
        area_suffix = Support.area_suffixes[target.area_id],
        location_id = target.location_id,
        location_name = name,
    }
end

function Support.choose_gang_contract_target()
    local index = Support.bounded_random(#Support.gang_contract_targets)
    return named_target(Support.gang_contract_targets[index + 1])
end

local function choose_target(pool)
    local index = Support.bounded_random(#pool)
    return named_target(pool[index + 1])
end

function Support.choose_facility_target(source_initializer)
    local I = E.RunOfferInitializer
    if source_initializer == I.PACKAGE_RECOVERY_RENRAKU or
       source_initializer == I.EMPLOYEE_RECRUITMENT_RENRAKU then
        return named_target(Support.run_target_pools.renraku)
    elseif source_initializer == I.PACKAGE_RECOVERY_TWO_SITE_POOL or
           source_initializer == I.EMPLOYEE_RECRUITMENT_TWO_SITE_POOL then
        return choose_target(Support.run_target_pools.two_site)
    end
    return choose_target(Support.run_target_pools.four_site)
end

function Support.present_run_acceptance(offer, label)
    local payment = offer.payment
    local prompt =
        "<You are offered" ..
        Support.amount_in_reserved_field(payment.base_amount, 6) ..
        "$|" .. label .. "|After negotiations, you|come to an agreement.>|" ..
        "Payment for this run is|" ..
        Support.amount_in_reserved_field(payment.negotiated_amount, 5) ..
        "$|Whatta' you say, friend?"
    local choice = game.ui.prompt_text(
        "run_acceptance", prompt, { "yes", "no" })
    return Support.resolve_offer_decision(choice)
end

function Support.new_offer(id, category, objective, completion_condition,
                           route, payment, runner_progress_delta)
    assert(type(id) == "string" and id ~= "")
    assert(type(category) == "string" and category ~= "")
    assert(type(objective) == "string" and objective ~= "")
    assert(type(completion_condition) == "string" and
        completion_condition ~= "")

    local offer = {
        id = id,
        category = category,
        objective = objective,
        completion_condition = completion_condition,
        runner_progress_delta = runner_progress_delta or 0,
    }
    if route ~= nil then
        offer.route = route
    end
    if payment ~= nil then
        offer.payment = payment
    end
    return offer
end

function Support.resolve_offer_decision(choice)
    local result = game.runs.registry.resolve_current_offer(choice)
    if result == "accepted" then
        -- ROM text 0x105F81/0x105FDD/0x105FF1 follows an accepted offer.
        local acceptance_text =
            "<\"Mr. Johnson\" logs the run on his pocket secretary and " ..
            "downloads the information to you.>|Thanks, my friend!|" ..
            "You keep comin' to me and I'll keep ya' runnin'."
        game.ui.say_text(
            acceptance_text, false)
    end
    return result
end

return Support
