-- Shared source data and helpers for independently-authored run modules.
local Support = {}
local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local MatrixCatalog = require("data.matrix_run_catalog")

Support.regions = {
    downtown_seattle = { suffix = "in Downtown Seattle." },
    puyallup_barrens = { suffix = "in the Puyallup Barrens." },
    redmond_barrens = { suffix = "in the Redmond Barrens." },
    penumbra_district = { suffix = "in the Penumbra District." },
    council_island = { suffix = "on Council Island." },
    renraku_arcology = { suffix = "in the Renraku Arcology." },
    amerindian_lands = { suffix = "in the AmerIndian Lands." },
}

Support.contract_region_ids = {
    "downtown_seattle", "puyallup_barrens", "redmond_barrens",
    "penumbra_district", "council_island", "renraku_arcology",
}

Support.locations = {
    seattle_general_hospital = {
        region_id = "downtown_seattle", name = "Seattle General Hospital",
    },
    icarus_descending = {
        region_id = "downtown_seattle", name = "Icarus Descending",
    },
    matchsticks = {
        region_id = "downtown_seattle", name = "Matchsticks",
    },
    gates_undersound_hotel = {
        region_id = "downtown_seattle", name = "the Gates Undersound Hotel",
    },
    mitsuhama_computer_technologies = {
        region_id = "downtown_seattle",
        name = "Mitsuhama Computer Technologies",
    },
    fuchi_local_office = {
        region_id = "downtown_seattle", name = "Fuchi's local office",
    },
    space_needle = {
        region_id = "downtown_seattle", name = "the Space Needle",
    },
    roscoe_the_fixer = {
        region_id = "downtown_seattle", name = "see Roscoe the Fixer",
    },
    wanderer = { region_id = "puyallup_barrens", name = "the Wanderer" },
    tarislar_city_inn = {
        region_id = "puyallup_barrens", name = "the Tarislar City Inn",
    },
    ares_corporate_offices = {
        region_id = "puyallup_barrens", name = "Ares' Corporate Offices",
    },
    riannons = { region_id = "puyallup_barrens", name = "Riannon's" },
    underground_93 = {
        region_id = "puyallup_barrens", name = "Underground 93",
    },
    crime_mall = { region_id = "puyallup_barrens", name = "the Crime Mall" },
    dr_bobs_quickstitch = {
        region_id = "puyallup_barrens", name = "Dr. Bob's Quickstitch",
    },
    tarislar_gardens_apartments = {
        region_id = "puyallup_barrens",
        name = "the Tarislar Gardens Apartment Complex",
    },
    ork_gang_hideout = {
        region_id = "puyallup_barrens", name = "see the Ork Gang",
    },
    hollywood_correctional_facilities = {
        region_id = "redmond_barrens",
        name = "the Hollywood Correctional Facilities",
    },
    little_chiba = { region_id = "redmond_barrens", name = "Little Chiba" },
    stokers_coffin_motel = {
        region_id = "redmond_barrens", name = "Stoker's Coffin Motel",
    },
    shiawase_nuke_plant = {
        region_id = "redmond_barrens", name = "the Shiawase Nuke Plant",
    },
    jump_house = { region_id = "redmond_barrens", name = "the Jump House" },
    jackals_lantern = {
        region_id = "redmond_barrens", name = "the Jackal's Lantern",
    },
    boris_greenhouse = {
        region_id = "redmond_barrens", name = "Boris' Greenhouse",
    },
    rats_nest = { region_id = "redmond_barrens", name = "the Rat's Nest" },
    halloweeners_hideout = {
        region_id = "redmond_barrens", name = "see the Halloweeners",
    },
    ares_weapon_emporium = {
        region_id = "redmond_barrens", name = "Ares Weapon Emporium",
    },
    club_penumbra = {
        region_id = "penumbra_district", name = "Club Penumbra",
    },
    wylies_gala_inn = {
        region_id = "penumbra_district", name = "Wylie's Gala Inn",
    },
    fuchi_industrial_electronics = {
        region_id = "penumbra_district",
        name = "Fuchi Industrial Electronics",
    },
    mitsuhama_local_office = {
        region_id = "penumbra_district", name = "Mitsuhama's local office",
    },
    eye_fivers_hideout = {
        region_id = "penumbra_district", name = "see the Eye Fivers",
    },
    big_rhino = { region_id = "penumbra_district", name = "the Big Rhino" },
    lone_star_security = {
        region_id = "penumbra_district", name = "Lone Star Security",
    },
    council_island_inn = {
        region_id = "council_island", name = "the Council Island Inn",
    },
    friendship_restaurant = {
        region_id = "council_island", name = "the Friendship Restaurant",
    },
    passport_lodge = {
        region_id = "council_island", name = "the Passport Lodge",
    },
    medicine_lodge_hollow = {
        region_id = "council_island", name = "the Medicine Lodge Hollow",
    },
    council_island_hospital = {
        region_id = "council_island", name = "the Council Island Hospital",
    },
    ork_embassy = { region_id = "council_island", name = "the Ork Embassy" },
    frag_grenade = {
        region_id = "renraku_arcology", name = "the Frag Grenade",
    },
    wire_masters = { region_id = "renraku_arcology", name = "Wire-Masters" },
    merlins_lore = { region_id = "renraku_arcology", name = "Merlin's Lore" },
    microtronics = { region_id = "renraku_arcology", name = "Microtronics" },
    weapons_world = { region_id = "renraku_arcology", name = "Weapons World" },
    renraku_offices = {
        region_id = "renraku_arcology", name = "the Renraku Offices",
    },
}

-- Preserve each source table's ten equally likely choices as named locations.
Support.location_choice_ids_by_region = {
    downtown_seattle = {
        "roscoe_the_fixer", "seattle_general_hospital", "icarus_descending",
        "matchsticks", "gates_undersound_hotel",
        "mitsuhama_computer_technologies", "fuchi_local_office",
        "space_needle", "roscoe_the_fixer", "seattle_general_hospital",
    },
    puyallup_barrens = {
        "crime_mall", "wanderer", "tarislar_city_inn",
        "ares_corporate_offices", "riannons", "underground_93",
        "crime_mall", "dr_bobs_quickstitch", "tarislar_gardens_apartments",
        "ork_gang_hideout",
    },
    redmond_barrens = {
        "hollywood_correctional_facilities", "little_chiba",
        "stokers_coffin_motel", "shiawase_nuke_plant", "jump_house",
        "jackals_lantern", "boris_greenhouse", "rats_nest",
        "halloweeners_hideout", "ares_weapon_emporium",
    },
    penumbra_district = {
        "club_penumbra", "wylies_gala_inn", "fuchi_industrial_electronics",
        "mitsuhama_local_office", "eye_fivers_hideout", "big_rhino",
        "lone_star_security", "club_penumbra", "eye_fivers_hideout",
        "lone_star_security",
    },
    council_island = {
        "council_island_inn", "friendship_restaurant", "passport_lodge",
        "medicine_lodge_hollow", "council_island_hospital", "ork_embassy",
        "passport_lodge", "medicine_lodge_hollow", "council_island_hospital",
        "ork_embassy",
    },
    renraku_arcology = {
        "renraku_offices", "frag_grenade", "wire_masters", "merlins_lore",
        "microtronics", "weapons_world", "renraku_offices", "frag_grenade",
        "wire_masters", "renraku_offices",
    },
}

Support.clients = {
    mike_nana = { name = "Mike Nana" },
    john_worley = { name = "John Worley" },
    ellis_goodson = { name = "Ellis Goodson" },
    john_van_anthony = { name = "John Van Anthony" },
    dok_whitson = { name = "Dok Whitson" },
    jase_weese = { name = "Jase Weese" },
    heinrich_michaels = { name = "Heinrich Michaels" },
    billy_gibson = { name = "Billy Gibson" },
    nero_manser = { name = "Nero Manser" },
    alan_turing = { name = "Alan Turing" },
    chris_stout = { name = "Chris Stout" },
    jorge_kish = { name = "Jorge Kish" },
}

-- Offer selection preserves the source table's twelve-entry distribution;
-- live and saved state carries the selected person's stable identity.
Support.client_ids = {
    "mike_nana", "john_worley", "ellis_goodson", "john_van_anthony",
    "dok_whitson", "jase_weese", "heinrich_michaels", "billy_gibson",
    "nero_manser", "alan_turing", "chris_stout", "jorge_kish",
}

function Support.client_name(client_id)
    local client = type(client_id) == "string" and
        Support.clients[client_id] or nil
    return type(client) == "table" and client.name or nil
end

function Support.choose_client_id()
    local choice = Support.bounded_random(#Support.client_ids)
    return Support.client_ids[choice + 1]
end

-- ROM 0xF36FA contains the four gang-contract variations shown after the
-- selected target location. These strings are kept as content, separate from
-- route selection and shared payment rules.
Support.gang_contract_variants = {
    {
        id = "building_vandalism",
        text = "They have been defacing and damaging buildings in that area.  I find this appalling!||Show them how I feel about vandals and return to me for your payment.",
    },
    {
        id = "renraku_information_leak",
        text = "They have been feeding Renraku a lot of information.||Renraku has been using this info to find and eliminate Johnsons.  Needless to say, this makes me uneasy.||\"Discuss\" this matter with them and return here to collect your fee.",
    },
    {
        id = "local_terrorism",
        text = "Their recent antics have been most heinous!||I want you to tell them to quit terrorizing the locals.  And make it stick!||Payment will be rendered when the gang is no longer a problem.",
    },
    {
        id = "insurance_premiums",
        text = "Their payments are late on an insurance policy they have with me.||You will be payed when you return with their premiums.",
    },
}

function Support.choose_gang_contract_variant_id()
    local index = Support.bounded_random(#Support.gang_contract_variants)
    return Support.gang_contract_variants[index + 1].id
end

function Support.gang_contract_variant_text(variant_id)
    for _, variant in ipairs(Support.gang_contract_variants) do
        if variant.id == variant_id then return variant.text end
    end
    return nil
end

Support.gang_contract_targets = {
    "ork_gang_hideout",
    "halloweeners_hideout",
    "eye_fivers_hideout",
}

Support.gang_actor_group_ids = {
    ork_gang_hideout = "gang_contract.ork_gang",
    halloweeners_hideout = "gang_contract.halloweeners",
    eye_fivers_hideout = "gang_contract.eye_fivers",
}

-- ROM F3E6E (four-site pool), F3E76 (two-site pool), and the fixed 5/6
-- Renraku tuple imported for the two named Renraku variants.
Support.run_target_pools = {
    four_site = {
        "fuchi_local_office",
        "mitsuhama_computer_technologies",
        "fuchi_industrial_electronics",
        "mitsuhama_local_office",
    },
    two_site = {
        "ares_corporate_offices",
        "lone_star_security",
    },
    renraku = "renraku_offices",
}

local contact_names = {
    [E.RunContact.JUMP_HOUSE] = "Mr. Gunderson",
    [E.RunContact.MATCHSTICK] = "Julius Strouther",
    [E.RunContact.BIG_RHINO] = "Mortimer Reed",
    [E.RunContact.WANDERER] = "Vigore and Jarl",
    [E.RunContact.ICARUS_DESCENDING] = "Caleb Brightmore",
}

local run_names = {
    [E.RunId.GHOUL_BOUNTY] = "Ghoul Bounty",
    [E.RunId.BODYGUARD_ESCORT] = "Bodyguard",
    [E.RunId.PACKAGE_DELIVERY] = "Courier",
    [E.RunId.GANG_CONTRACT] = "Enforcement",
    [E.RunId.PACKAGE_RECOVERY] = "Acquisition",
    [E.RunId.EMPLOYEE_RECRUITMENT] = "Extraction",
    [E.RunId.MATRIX_OBJECTIVE] = "Matrix Run",
}

function Support.location_details(location_id)
    local location = type(location_id) == "string" and
        Support.locations[location_id] or nil
    if type(location) ~= "table" then return nil end
    local region = Support.regions[location.region_id]
    if type(region) ~= "table" then return nil end
    return {
        location_id = location_id,
        name = location.name,
        region_id = location.region_id,
        region_suffix = region.suffix,
    }
end

function Support.region_suffix(region_id)
    local region = type(region_id) == "string" and
        Support.regions[region_id] or nil
    return type(region) == "table" and region.suffix or nil
end

function Support.gang_actor_group_id(location_id)
    return type(location_id) == "string" and
        Support.gang_actor_group_ids[location_id] or nil
end

local function source_run_body(offer, run_id)
    local body = {}
    local function copy(text, spaces)
        if type(text) ~= "string" then return false end
        body[#body + 1] = text
        body[#body + 1] = string.rep(" ", spaces)
        return true
    end
    local function route_location(route_point)
        if type(route_point) ~= "table" then return nil end
        local details = Support.location_details(route_point.location_id)
        if details == nil then return nil end
        return details.name, details.region_suffix
    end

    if run_id == E.RunId.GHOUL_BOUNTY then
        local region_suffix = Support.region_suffix(offer.region_id)
        if region_suffix == nil or
           offer.region_id == "council_island" or
           offer.region_id == "renraku_arcology" or
           offer.region_id == "amerindian_lands" then return nil end
        copy("There is a community of Ghouls somewhere", 1) -- F2520, D1=0.
        copy(region_suffix, 2)                              -- F1E58, D1=1.
        copy("They have been preying on the citizens there." ..
             "\n\nKill as many as possible and return here for payment.",
             1)                                              -- F2549, D1=0.
    elseif run_id == E.RunId.BODYGUARD_ESCORT or
           run_id == E.RunId.PACKAGE_DELIVERY then
        local origin, origin_area = route_location(
            offer.route and offer.route.origin)
        local destination, destination_area = route_location(
            offer.route and offer.route.destination)
        if origin == nil or destination == nil then return nil end
        copy("Go to", 1)                                    -- F23B2, D1=0.
        copy(origin, 1)
        copy(origin_area, 2)
        if run_id == E.RunId.BODYGUARD_ESCORT then
            local client = Support.client_name(offer.route.client_id)
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
    elseif run_id == E.RunId.GANG_CONTRACT or
           run_id == E.RunId.PACKAGE_RECOVERY then
        local target, area = route_location(offer.route and offer.route.target)
        if target == nil then return nil end
        if run_id == E.RunId.GANG_CONTRACT then
            local variant_text = Support.gang_contract_variant_text(
                offer.contract_variant_id)
            if variant_text == nil then return nil end
            -- 0x1ECA4 removes "to" from F23B2 because each gang target
            -- begins with "see", yielding "Go see <gang>".
            copy("Go ", 0)
            copy(target, 1)
            copy(area, 2)
            copy(variant_text:gsub("|", "\n"), 0)
        else
            copy("Go to", 1)                                -- F23B2, D1=0.
            copy(target, 1)
            copy(area, 2)
            copy("A package was taken from a courier of mine." ..
                 "\n\nGet the package and bring it back to me." ..
                 "\n\nI will then pay you for your services.", 0)
                                                            -- F367C, D1=-1.
        end
    elseif run_id == E.RunId.EMPLOYEE_RECRUITMENT then
        local target, area = route_location(offer.route and offer.route.target)
        local client = Support.client_name(offer.employee_id)
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
    elseif run_id == E.RunId.MATRIX_OBJECTIVE then
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
    local variant_id = offer.offer_variant_id
    local resolved = type(variant_id) == "string" and
        OfferData.run_by_variant_id[variant_id] or nil
    if resolved == nil or offer.id ~= resolved.id or
       type(offer.contact_id) ~= "string" then
        return nil
    end
    local contact_id = offer.contact_id
    local johnson_name = contact_names[contact_id]
    local run_name = run_names[resolved.id]
    local payment = type(offer.payment) == "table" and
        offer.payment.negotiated_amount or nil
    if johnson_name == nil or run_name == nil or
       type(payment) ~= "number" or payment % 1 ~= 0 or
       payment < 0 or payment > 0xFFFF then
        return nil
    end
    local body = source_run_body(offer, resolved.id)
    if body == nil or body == "" then return nil end
    return {
        offer_variant_id = variant_id,
        contact_id = contact_id,
        run_id = resolved.id,
        johnson_label = "JOHNSON:",
        johnson_name = johnson_name,
        run_type_label = "TYPE:",
        run_type_name = run_name,
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

local function choose_location(region_id)
    local roll = Support.bounded_random(10)
    local choices = Support.location_choice_ids_by_region[region_id]
    assert(type(choices) == "table",
        "contract offer selected an unknown region")
    local location_id = choices[roll + 1]
    assert(Support.location_details(location_id) ~= nil,
        "contract location choice has no semantic place")
    return location_id
end

function Support.choose_contract_route(contact_id)
    -- ROM 0x1E9C0 chooses origin area/location, client, then destination
    -- area/location. The opening context fixes both areas to Redmond and
    -- retries the destination location until it differs from the origin.
    local origin_region_id
    if contact_id == E.RunContact.JUMP_HOUSE then
        origin_region_id = "redmond_barrens"
    else
        local region_index = Support.bounded_random(
            #Support.contract_region_ids)
        origin_region_id = Support.contract_region_ids[region_index + 1]
    end

    local origin_location_id = choose_location(origin_region_id)
    local client_id = Support.choose_client_id()

    local destination_region_id
    if contact_id == E.RunContact.JUMP_HOUSE then
        destination_region_id = "redmond_barrens"
    else
        repeat
            local region_index = Support.bounded_random(
                #Support.contract_region_ids)
            destination_region_id =
                Support.contract_region_ids[region_index + 1]
        until destination_region_id ~= origin_region_id
    end

    local destination_location_id = choose_location(destination_region_id)
    while contact_id == E.RunContact.JUMP_HOUSE and
          destination_location_id == origin_location_id do
        destination_location_id = choose_location(destination_region_id)
    end

    return {
        origin = { location_id = origin_location_id },
        destination = { location_id = destination_location_id },
        client_id = client_id,
    }
end

function Support.choose_ghoul_region(contact_id)
    -- ROM 0x1E744 fixes the opening contract to area 2; other contexts use
    -- the four-area random pool from that handler.
    if contact_id == E.RunContact.JUMP_HOUSE then
        return "redmond_barrens"
    end
    local region_index = Support.bounded_random(4)
    return Support.contract_region_ids[region_index + 1]
end

local function named_target(location_id)
    assert(Support.location_details(location_id) ~= nil,
        "run target has no semantic place")
    return { location_id = location_id }
end

function Support.choose_gang_contract_target()
    local index = Support.bounded_random(#Support.gang_contract_targets)
    return named_target(Support.gang_contract_targets[index + 1])
end

local function choose_target(pool)
    local index = Support.bounded_random(#pool)
    return named_target(pool[index + 1])
end

function Support.choose_facility_target(target_pool_id)
    assert(type(target_pool_id) == "string",
        "facility offer needs a semantic target pool")
    local pool = Support.run_target_pools[target_pool_id]
    assert(pool ~= nil,
        "facility offer selected an unknown target pool")
    if target_pool_id == "renraku" then
        return named_target(pool)
    end
    assert(#pool > 0, "facility offer selected an empty target pool")
    return choose_target(pool)
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
