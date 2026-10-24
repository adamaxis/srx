-- Shared source data and helpers for independently-authored run modules.
local Support = {}
local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local MatrixCatalog = require("data.matrix_run_catalog")

Support.area_suffixes = {
    -- ROM F1E58 strings used by the contract location formatter.
    [0] = "in Downtown Seattle.",
    [1] = "in the Puyallup Barrens.",
    [2] = "in the Redmond Barrens.",
    [3] = "in the Penumbra District.",
    [4] = "on Council Island.",
    [5] = "in the Renraku Arcology.",
    [6] = "in the AmerIndian Lands.",
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
    "They have been defacing and damaging buildings in that area.  I find this appalling!||Show them how I feel about vandals and return to me for your payment.",
    "They have been feeding Renraku a lot of information.||Renraku has been using this info to find and eliminate Johnsons.  Needless to say, this makes me uneasy.||\"Discuss\" this matter with them and return here to collect your fee.",
    "Their recent antics have been most heinous!||I want you to tell them to quit terrorizing the locals.  And make it stick!||Payment will be rendered when the gang is no longer a problem.",
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

local run_context_names = {
    [E.RunOfferContext.JUMP_HOUSE] = "Mr. Gunderson",
    [E.RunOfferContext.SCENE_18] = "Julius Strouther",
    [E.RunOfferContext.BIG_RHINO] = "Mortimer Reed",
    [E.RunOfferContext.WANDERER] = "Vigore and Jarl",
    [E.RunOfferContext.ICARUS_DESCENDING] = "Caleb Brightmore",
}

local run_type_names = {
    [E.RunType.GHOUL_BOUNTY] = "Ghoul Bounty",
    [E.RunType.BODYGUARD_ESCORT] = "Bodyguard",
    [E.RunType.PACKAGE_DELIVERY] = "Courier",
    [E.RunType.GANG_CONTRACT] = "Enforcement",
    [E.RunType.PACKAGE_RECOVERY] = "Acquisition",
    [E.RunType.EMPLOYEE_RECRUITMENT] = "Extraction",
    [E.RunType.MATRIX_OBJECTIVE] = "Matrix Run",
}

local function source_location(pair)
    if type(pair) ~= "table" then return nil end
    local area_id = pair.area_id
    local location_id = pair.location_id
    if type(area_id) ~= "number" or area_id % 1 ~= 0 or
       type(location_id) ~= "number" or location_id % 1 ~= 0 then
        return nil
    end
    local area = Support.area_suffixes[area_id]
    local locations = Support.location_names[area_id]
    local location = type(locations) == "table" and
        locations[location_id] or nil
    if area == nil or location == nil then return nil end
    return location, area
end

local function source_client(client_id)
    if type(client_id) ~= "number" or client_id % 1 ~= 0 or
       client_id < 0 or client_id >= #Support.client_names then
        return nil
    end
    return Support.client_names[client_id + 1]
end

local function source_run_body(offer, run_type)
    local body = {}
    local function copy(text, spaces)
        if type(text) ~= "string" then return false end
        body[#body + 1] = text
        body[#body + 1] = string.rep(" ", spaces)
        return true
    end
    local function route_pair(pair)
        local location, area = source_location(pair)
        if location == nil then return nil end
        return location, area
    end

    if run_type == E.RunType.GHOUL_BOUNTY then
        local area = Support.area_suffixes[offer.area_id]
        if area == nil or offer.area_id < 0 or offer.area_id > 3 then
            return nil
        end
        copy("There is a community of Ghouls somewhere", 1) -- F2520, D1=0.
        copy(area, 2)                                        -- F1E58, D1=1.
        copy("They have been preying on the citizens there." ..
             "\n\nKill as many as possible and return here for payment.",
             1)                                              -- F2549, D1=0.
    elseif run_type == E.RunType.BODYGUARD_ESCORT or
           run_type == E.RunType.PACKAGE_DELIVERY then
        local origin, origin_area = route_pair(
            offer.route and offer.route.origin)
        local destination, destination_area = route_pair(
            offer.route and offer.route.destination)
        if origin == nil or destination == nil then return nil end
        copy("Go to", 1)                                    -- F23B2, D1=0.
        copy(origin, 1)
        copy(origin_area, 2)
        if run_type == E.RunType.BODYGUARD_ESCORT then
            local client = source_client(offer.route.client_id)
            if client == nil then return nil end
            copy("There you will meet my client,", 1)       -- F34C8.
            copy(client, 2)                                 -- F35A6, D1=1.
            copy("\n\nEscort my client to", 1)             -- F34E7, D1=0.
            copy(destination, 1)
            copy(destination_area, 2)
            copy("You will be paid by my client upon safe arrival at the specified destination.",
                 0)                                         -- F34FD, D1=-1.
        else
            copy("A contact there will give you a package.\n\nTake it to", 1)
                                                            -- F23B8, D1=0.
            copy(destination, 1)
            copy(destination_area, 2)
            copy("You will be paid upon safe delivery of the package.", 0)
                                                            -- F23ED, D1=-1.
        end
    elseif run_type == E.RunType.GANG_CONTRACT or
           run_type == E.RunType.PACKAGE_RECOVERY then
        local target, area = route_pair(offer.route and offer.route.target)
        if target == nil then return nil end
        if run_type == E.RunType.GANG_CONTRACT then
            if type(offer.variant) ~= "number" or
               offer.variant % 1 ~= 0 or offer.variant < 0 or
               offer.variant >= #Support.gang_contract_texts then
                return nil
            end
            -- 0x1ECA4 removes "to" from F23B2 because each gang target
            -- begins with "see", yielding "Go see <gang>".
            copy("Go ", 0)
            copy(target, 1)
            copy(area, 2)
            copy(Support.gang_contract_texts[offer.variant + 1]:gsub("|", "\n"), 0)
        else
            copy("Go to", 1)                                -- F23B2, D1=0.
            copy(target, 1)
            copy(area, 2)
            copy("A package was taken from a courier of mine." ..
                 "\n\nGet the package and bring it back to me." ..
                 "\n\nI will then pay you for your services.", 0)
                                                            -- F367C, D1=-1.
        end
    elseif run_type == E.RunType.EMPLOYEE_RECRUITMENT then
        local target, area = route_pair(offer.route and offer.route.target)
        local client = source_client(offer.client_id)
        if target == nil or area == nil or client == nil then return nil end
        area = area:gsub("%.$", ",", 1)                     -- 0x1E91A.
        copy("An employee of", 1)                          -- F39BE, D1=0.
        copy(target, 1)
        copy(area, 0)
        body[#body + 1] = " "                               -- 0x1E91A.
        copy(client, 0)
        body[#body + 1] = ", "                              -- 0x1E91A.
        copy("wishes for a change in career.", 2)           -- F39CD, D1=1.
        copy("Unfortunately, his current employer frowns upon this." ..
             "\n\nAssist my client with his professional growth and return here " ..
             "to receive full compensation for your services.", 0)
                                                            -- F39EC, D1=-1.
    elseif run_type == E.RunType.MATRIX_OBJECTIVE then
        local objective_index = offer.objective_index
        if type(objective_index) ~= "number" or
           objective_index % 1 ~= 0 or objective_index < 0 or
           objective_index >= #MatrixCatalog.objectives then
            return nil
        end
        local objective = MatrixCatalog.objectives[objective_index + 1]
        if objective_index < 17 then
            local target_system_id = offer.target_system_id
            local system_name = type(target_system_id) == "number" and
                MatrixCatalog.systems[target_system_id] or nil
            if system_name == nil then return nil end
            copy(system_name, 1)                             -- C19C6, D1=0.
        end
        copy(objective.description, 0)                      -- F25AE, D1=-1.
    else
        return nil
    end
    return table.concat(body)
end

function Support.wrap_current_run_body(body)
    assert(type(body) == "string")
    local lines = {}
    local source_scan_width = 27 -- C168 receives D1=0x1A (27 scan cells).
    local function wrap_line(line)
        if line == "" then
            lines[#lines + 1] = ""
            return
        end
        while #line > 0 do
            local scanned = 0
            local last_fitting_space = 0
            for index = 1, #line do
                scanned = index
                if line:sub(index, index) == " " then
                    if scanned < source_scan_width then
                        last_fitting_space = scanned
                    else
                        break
                    end
                end
            end
            local consumed = scanned
            if scanned > source_scan_width then
                -- C168 retains the last space whose scan count is below
                -- width. The separator belongs to the consumed span but is
                -- not visible at the end of the native text row.
                consumed = last_fitting_space > 0
                    and last_fitting_space or source_scan_width
            end
            if consumed <= 0 then return end
            lines[#lines + 1] = line:sub(1, consumed):gsub(" +$", "")
            line = line:sub(consumed + 1)
        end
    end
    local start = 1
    for index = 1, #body + 1 do
        if index > #body or body:sub(index, index) == "\n" then
            wrap_line(body:sub(start, index - 1))
            start = index + 1
        end
    end
    return lines
end

-- The Notebook Current Run page displays the accepted source record's stored
-- payment and source-selected route fields. It does not reroll or recompute
-- the Johnson offer. Text fragments and trailing spaces follow the shared
-- Current Run handlers at 0x1E72C/0x1EE64 and their C134 copy parameters.
function Support.current_run_view(offer)
    if type(offer) ~= "table" then return nil end
    local initializer = offer.source_offer_type or offer.source_initializer
    local resolved = type(initializer) == "number" and
        OfferData.run_by_initializer[initializer] or nil
    if resolved == nil or offer.source_run_type ~= resolved.type then
        return nil
    end
    local context = offer.source_offer_context or offer.context
    local johnson_name = run_context_names[context]
    local run_type_name = run_type_names[resolved.type]
    local payment = type(offer.payment) == "table" and
        offer.payment.negotiated_amount or nil
    if johnson_name == nil or run_type_name == nil or
       type(payment) ~= "number" or payment % 1 ~= 0 or
       payment < 0 or payment > 0xFFFF then
        return nil
    end
    local body = source_run_body(offer, resolved.type)
    if body == nil or body == "" then return nil end
    return {
        source_initializer = initializer,
        context = context,
        run_type = resolved.type,
        johnson_label = "JOHNSON:",
        johnson_name = johnson_name,
        run_type_label = "TYPE:",
        run_type_name = run_type_name,
        payment_label = "PAY:",
        payment_amount = payment,
        currency_symbol = "$",
        body = body,
        body_lines = Support.wrap_current_run_body(body),
    }
end

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
