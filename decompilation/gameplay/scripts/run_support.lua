-- Shared source data and helpers for independently-authored run modules.
local Support = {}

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
    if context == 0 then
        origin_area = 2
    else
        origin_area = Support.bounded_random(6)
    end

    local origin_index, origin_name = choose_location(origin_area)
    local client_index = Support.bounded_random(#Support.client_names)
    local client_name = Support.client_names[client_index + 1]

    local destination_area
    if context == 0 then
        destination_area = 2
    else
        repeat
            destination_area = Support.bounded_random(6)
        until destination_area ~= origin_area
    end

    local destination_index, destination_name = choose_location(destination_area)
    while context == 0 and destination_index == origin_index do
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

function Support.new_offer(id, category, objective, completion_condition,
                           route)
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
    }
    if route ~= nil then
        offer.route = route
    end
    return offer
end

return Support
