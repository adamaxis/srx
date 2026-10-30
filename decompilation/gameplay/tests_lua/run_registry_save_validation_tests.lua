package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local Registry = require("run_registry")
local Support = require("run_support")

local run_modules = { registry = Registry }
for _, resolved in pairs(OfferData.run_by_variant_id) do
    run_modules[resolved.id] = {
        prepare_offer = function() end,
        resolve_field_event = function() return { action = "no_action" } end,
    }
end
game = { runs = run_modules, modules = { run_support = Support } }

local function matrix_state()
    return {
        status = E.MatrixRunStatus.PENDING,
        objective = {
            node = "datastore",
            action = E.MatrixRunAction.DOWNLOAD_FILE,
            file_class = "Outdated Files",
            file_id = 1,
        },
        file_size = 10,
        completion_marker_present = false,
        completion_recorded = false,
        target_system_id = 0,
    }
end

local function source_offer(contact_id, row, skill, roll)
    local resolved = OfferData.run_by_variant_id[row.variant_id]
    local bound = row.negotiation_scale * skill + 1
    local offer = {
        id = resolved.id,
        category = "audit",
        objective = "source_offer",
        completion_condition = "source_completion",
        runner_progress_delta = row.runner_progress_delta,
        offer_variant_id = row.variant_id,
        contact_id = contact_id,
        payment = {
            base_amount = row.base_payment,
            negotiated_amount = row.base_payment + roll * 5,
            negotiation_skill = skill,
            negotiation_bound = bound,
            negotiation_roll = roll,
        },
    }
    if resolved.id == E.RunId.BODYGUARD_ESCORT then
        offer.route = {
            origin = { location_id = "jump_house" },
            destination = { location_id = "jackals_lantern" },
            client_id = "mike_nana",
        }
    elseif resolved.id == E.RunId.PACKAGE_DELIVERY then
        offer.route = {
            origin = { location_id = "ares_weapon_emporium" },
            destination = { location_id = "jackals_lantern" },
        }
    elseif resolved.id == E.RunId.GANG_CONTRACT then
        offer.route = { target = { location_id = "ork_gang_hideout" } }
        offer.contract_variant_id = "building_vandalism"
    elseif resolved.id == E.RunId.PACKAGE_RECOVERY then
        offer.route = { target = { location_id = "ares_corporate_offices" } }
    elseif resolved.id == E.RunId.EMPLOYEE_RECRUITMENT then
        offer.employee_id = "mike_nana"
        offer.route = { target = { location_id = "renraku_offices" } }
    elseif resolved.id == E.RunId.GHOUL_BOUNTY then
        offer.region_id = "redmond_barrens"
    end
    if resolved.id == E.RunId.MATRIX_OBJECTIVE then
        offer.matrix_run_state = matrix_state()
    end
    return offer
end

local function saved(offer)
    return {
        schema_version = 1,
        current_offer = nil,
        active_offer = offer,
        last_completed_offer = nil,
        last_offer_result = "accepted",
    }
end

local function valid(offer)
    return Registry.validate_saved_state(saved(offer))
end

local function copy_offer(original)
    local copy = {}
    for key, value in pairs(original) do
        if type(value) == "table" then
            local nested = {}
            for nested_key, nested_value in pairs(value) do
                nested[nested_key] = nested_value
            end
            copy[key] = nested
        else
            copy[key] = value
        end
    end
    return copy
end

local contacts = {
    E.RunContact.JUMP_HOUSE,
    E.RunContact.MATCHSTICK,
    E.RunContact.BIG_RHINO,
    E.RunContact.WANDERER,
    E.RunContact.ICARUS_DESCENDING,
}
local checked = 0
local courier
for _, contact_id in ipairs(contacts) do
    for _, row in ipairs(OfferData.contacts[contact_id]) do
        for skill = 0, 12 do
            local bound = row.negotiation_scale * skill + 1
            for _, roll in ipairs({ 0, bound - 1 }) do
                local offer = source_offer(contact_id, row, skill, roll)
                assert(valid(offer), string.format(
                    "valid source save rejected: contact=%s variant=%s " ..
                    "skill=%d roll=%d",
                    contact_id, row.variant_id, skill, roll))
                checked = checked + 1
                if contact_id == E.RunContact.JUMP_HOUSE and
                   row.variant_id == "package_delivery" and
                   row.base_payment == 40 and skill == 3 and roll == 3 then
                    courier = offer
                end
            end
        end
    end
end
assert(checked == 1040,
    "source save matrix must cover 40 rows, 13 ranks, and two roll edges")
assert(courier ~= nil and courier.payment.negotiated_amount == 55,
    "Section 03 Courier offer must resolve to the saved 40 -> 55 amount")
courier.route = {
    origin = { location_id = "ares_weapon_emporium" },
    destination = { location_id = "jackals_lantern" },
}

local source_shaped_route = copy_offer(courier)
source_shaped_route.route.origin = {
    area_id = 2, location_id = 11,
}
assert(not valid(source_shaped_route),
    "Lua save validation must reject raw region/event selectors")

local mismatched_contact = copy_offer(courier)
mismatched_contact.contact_id = E.RunContact.MATCHSTICK
assert(not valid(mismatched_contact),
    "a package delivery cannot be restored under another Johnson contact")

local mismatched_variant = copy_offer(courier)
mismatched_variant.offer_variant_id = "ghoul_bounty"
assert(not valid(mismatched_variant),
    "saved offer variant must agree with the selected contact row")

local missing_contact = copy_offer(courier)
missing_contact.contact_id = nil
assert(not valid(missing_contact),
    "source-backed save must retain its Johnson contact")

local mismatched_run = copy_offer(courier)
mismatched_run.id = "ghoul_bounty"
assert(not valid(mismatched_run),
    "offer variant must dispatch to its canonical Lua run")

local wrong_progress = copy_offer(courier)
wrong_progress.runner_progress_delta = 2
assert(not valid(wrong_progress),
    "saved progress delta must match the selected ROM offer row")

local missing_payment = copy_offer(courier)
missing_payment.payment = nil
assert(not valid(missing_payment),
    "source-backed accepted run must retain the negotiated payment record")

local invalid_client
for _, row in ipairs(OfferData.contacts[E.RunContact.JUMP_HOUSE]) do
    if row.variant_id == "bodyguard_escort" then
        invalid_client = source_offer(
            E.RunContact.JUMP_HOUSE, row, 3, 0)
        break
    end
end
assert(invalid_client ~= nil)
invalid_client.route.client_id = 7
assert(not valid(invalid_client),
    "saved Bodyguard routes must store a stable client ID")

local wrong_skill = copy_offer(courier)
wrong_skill.payment.negotiation_skill = 13
assert(not valid(wrong_skill),
    "saved Negotiation cannot exceed the Attributes/Skills cap of 12")

local wrong_bound = copy_offer(courier)
wrong_bound.payment.negotiation_bound = 5
assert(not valid(wrong_bound),
    "saved payment bound must equal source scale times skill plus one")

local out_of_range_roll = copy_offer(courier)
out_of_range_roll.payment.negotiation_roll = 4
out_of_range_roll.payment.negotiated_amount = 60
assert(not valid(out_of_range_roll),
    "a maximum-bound roll cannot be restored")

local wrong_amount = copy_offer(courier)
wrong_amount.payment.negotiated_amount = 54
assert(not valid(wrong_amount),
    "saved negotiated payment must equal base plus five per roll")

Registry.current_offer = copy_offer(courier)
Registry.active_offer = nil
assert(Registry.resolve_current_offer("yes") == "accepted" and
       Registry.active_offer.payment.negotiated_amount == 55,
    "acceptance must move the negotiated offer into active-run state")
local accepted_save = Registry.save_state()
assert(Registry.validate_saved_state(accepted_save),
    "accepted Courier run must pass source save validation")
Registry.active_offer = nil
Registry.restore_saved_state(accepted_save)
assert(Registry.active_offer.contact_id ==
           E.RunContact.JUMP_HOUSE and
       Registry.active_offer.offer_variant_id == "package_delivery" and
       Registry.active_offer.payment.base_amount == 40 and
       Registry.active_offer.payment.negotiated_amount == 55 and
       Support.current_run_view(Registry.active_offer).payment_amount == 55,
    "save restore must preserve the accepted Current Run contact and amount")

local matrix_offer
for _, row in ipairs(OfferData.contacts[E.RunContact.BIG_RHINO]) do
    if row.variant_id == "matrix_simple" then
        matrix_offer = source_offer(E.RunContact.BIG_RHINO, row, 0, 0)
        break
    end
end
assert(matrix_offer ~= nil)
matrix_offer.matrix_run_state = {
    state = -1,
    objective = 4,
    value = 20,
    size = 30,
    fc28 = 1,
    fc1c = 0,
    target_system_id = 7,
}
local legacy_matrix_save = saved(matrix_offer)
assert(Registry.validate_saved_state(legacy_matrix_save),
    "the save boundary accepts a prior Matrix run record for migration")
Registry.restore_saved_state(legacy_matrix_save)
local migrated_matrix = Registry.active_matrix_run_state()
assert(migrated_matrix.status == E.MatrixRunStatus.ACTIVE and
       migrated_matrix.objective.node == "datastore" and
       migrated_matrix.objective.action == E.MatrixRunAction.DOWNLOAD_FILE and
       migrated_matrix.objective.file_class == "Security Files" and
       migrated_matrix.objective.file_id == 20 and
       migrated_matrix.file_size == 30 and
       migrated_matrix.completion_marker_present and
       not migrated_matrix.completion_recorded and
       migrated_matrix.target_system_id == 7 and
       migrated_matrix.state == nil and migrated_matrix.fc28 == nil,
    "legacy Matrix save fields normalize once into named objective facts")

local invalid_matrix_offer = copy_offer(matrix_offer)
invalid_matrix_offer.matrix_run_state = {
    status = E.MatrixRunStatus.ACTIVE,
    objective = {
        node = "datastore",
        action = E.MatrixRunAction.DELETE_FILE,
        file_class = "Security Files",
    },
    file_size = 30,
    completion_marker_present = true,
    completion_recorded = false,
    target_system_id = 7,
}
assert(not Registry.validate_saved_state(saved(invalid_matrix_offer)),
    "a delete action without a selected file is not a valid Matrix objective")

print("run registry save validation passed: " .. tostring(checked) ..
    " source row/rank/roll states; accepted Courier 40 -> 55 persisted; " ..
    "corrupt contact, offer variant, progression, and payment rejected")
