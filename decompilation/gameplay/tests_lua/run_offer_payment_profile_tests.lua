package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local Support = require("run_support")

local raw_profile_negotiation = {
    { "Joshua Samurai", 1 },
    { "Joshua Decker", 2 },
    { "Joshua Gator Shaman", 3 },
    { "Rat Shaman", 1 },
    { "Troll Samurai", 0 },
    { "Human Mage", 2 },
    { "Dwarf Decker", 0 },
    { "Bear Shaman", 0 },
    { "Elf Decker 1", 0 },
    { "Human Samurai", 4 },
    { "Elf Mage", 4 },
    { "Elf Decker 2", 0 },
    { "Human Samurai 2", 0 },
}

local current_skill = 0
local current_bound = 1
local current_roll = 0
local random_calls = 0

game = {
    services = {
        call = function(domain, service)
            if domain == "runner" and service == "negotiation_skill" then
                return "skill_" .. tostring(current_skill)
            end
            assert(domain == "random", "payment used an unexpected service")
            local bound_text = service:match("^bounded_(%d+)$")
            assert(bound_text ~= nil,
                "payment requested a malformed bounded random service")
            assert(tonumber(bound_text) == current_bound,
                "payment used the wrong ROM negotiation bound")
            assert(current_roll >= 0 and current_roll < current_bound,
                "test roll is outside the selected ROM bound")
            random_calls = random_calls + 1
            return "roll_" .. tostring(current_roll)
        end,
    },
}

local function check_payment(record, skill, use_maximum_roll)
    current_skill = skill
    current_bound = record.negotiation_scale * skill + 1
    current_roll = use_maximum_roll and current_bound - 1 or 0

    local payment = Support.negotiate_payment(record)
    local expected_amount = record.base_payment + current_roll * 5
    assert(payment.base_amount == record.base_payment)
    assert(payment.negotiation_skill == skill)
    assert(payment.negotiation_bound == current_bound)
    assert(payment.negotiation_roll == current_roll)
    assert(payment.negotiated_amount == expected_amount,
        "displayed/payout amount diverged from base + five per roll")
    return payment
end

local source_row_count = 0
local minimum_source_skill = 0x7FFF
local maximum_source_skill = 0
for _, profile in ipairs(raw_profile_negotiation) do
    local skill = profile[2]
    assert(skill >= 0 and skill <= 12,
        profile[1] .. " has an invalid ROM Negotiation rank")
    minimum_source_skill = math.min(minimum_source_skill, skill)
    maximum_source_skill = math.max(maximum_source_skill, skill)
end

local contexts = {
    E.RunOfferContext.JUMP_HOUSE,
    E.RunOfferContext.SCENE_18,
    E.RunOfferContext.BIG_RHINO,
    E.RunOfferContext.WANDERER,
    E.RunOfferContext.ICARUS_DESCENDING,
}
local maximum_payment = 0
local maximum_payment_row = nil
local maximum_preset_payment = 0
local maximum_preset_payment_row = nil
local longplay_offer_payment = nil

for _, context in ipairs(contexts) do
    local rows = OfferData.contexts[context]
    assert(rows ~= nil and #rows == 8,
        "source Johnson context must contain eight ROM offer rows")
    for row_index, record in ipairs(rows) do
        source_row_count = source_row_count + 1
        assert(type(record.source_initializer) == "number")
        assert(type(record.runner_progress_delta) == "number")

        -- All thirteen ROM runner presets have Negotiation ranks 0-4;
        -- the Attributes/Skills screen permits ranks 0-12. Exercise every
        -- valid rank and both endpoints of its bounded roll for every row.
        for skill = 0, 12 do
            check_payment(record, skill, false)
            local maximum_roll_payment = check_payment(record, skill, true)
            if context == E.RunOfferContext.JUMP_HOUSE and row_index == 5 and
               skill == raw_profile_negotiation[3][2] then
                longplay_offer_payment = maximum_roll_payment
            end
        end

        local max_for_row =
            record.base_payment + 5 * record.negotiation_scale * 12
        if max_for_row > maximum_payment then
            maximum_payment = max_for_row
            maximum_payment_row = record
        end
        local max_for_profile =
            record.base_payment + 5 * record.negotiation_scale * 4
        if max_for_profile > maximum_preset_payment then
            maximum_preset_payment = max_for_profile
            maximum_preset_payment_row = record
        end
    end
end

assert(source_row_count == 40,
    "payment audit must cover all five eight-row ROM offer contexts")
local longplay_offer_row =
    OfferData.contexts[E.RunOfferContext.JUMP_HOUSE][5]
assert(longplay_offer_row.base_payment == 40 and
       longplay_offer_row.negotiation_scale == 1 and
       raw_profile_negotiation[3][1] == "Joshua Gator Shaman" and
       raw_profile_negotiation[3][2] == 3 and
       longplay_offer_payment ~= nil and
       longplay_offer_payment.negotiated_amount == 55 and
       longplay_offer_payment.negotiation_bound == 4 and
       longplay_offer_payment.negotiation_roll == 3,
    "independent New Game offer anchor (40 -> 55) changed")
assert(random_calls == source_row_count * 13 * 2,
    "payment audit missed a skill/roll endpoint case")
assert(minimum_source_skill == 0 and maximum_source_skill == 4,
    "ROM preset Negotiation range changed; refresh profile provenance")
assert(maximum_payment == 6700 and maximum_payment_row.base_payment == 6100 and
       maximum_payment_row.negotiation_scale == 10,
    "the source-row maximum payment at Negotiation rank 12 changed")
assert(maximum_preset_payment == 6450 and
       maximum_preset_payment_row.base_payment == 6350 and
       maximum_preset_payment_row.negotiation_scale == 5,
    "the maximum payment at the highest ROM preset Negotiation rank changed")

print("run offer payment tests passed: 40 rows x 13 skill ranks x both roll bounds; " ..
    "13 ROM preset values span 0-4; max rank-12 payment is " ..
    tostring(maximum_payment) .. " nuyen; preset-rank-4 maximum is " ..
    tostring(maximum_preset_payment) .. " nuyen; longplay anchor is 40 -> 55 " ..
    "at Joshua Gator Shaman Negotiation 3")
