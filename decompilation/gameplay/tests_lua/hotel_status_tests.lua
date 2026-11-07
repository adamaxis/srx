-- Hotel recovery policy from ROM 0xEF08; meter side effects are applied by
-- the shared native HUD meter services at 0x3674 and 0x37EC [C].
local Hotel = require("hotel")

local pass, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL " .. label) end
end

local function decision(active, physical_meter)
    local result = Hotel.service_party_status({
        runners = {{ active = active, physical_meter = physical_meter }},
    })
    return result.runners[1]
end

for meter = 0, 255 do
    local result = decision(true, meter)
    if meter < 3 then
        check(result.medical_attention and
              result.physical_meter_delta == 0 and
              result.mental_meter_delta == 10 and
              not result.clear_recovery_request,
              "active runner below recovery threshold " .. meter)
    elseif meter < 5 then
        check(not result.medical_attention and
              result.physical_meter_delta == 1 and
              result.mental_meter_delta == 10 and
              result.clear_recovery_request,
              "active runner receives one-point recovery at " .. meter)
    else
        check(not result.medical_attention and
              result.physical_meter_delta == 2 and
              result.mental_meter_delta == 10 and
              result.clear_recovery_request,
              "active runner receives two-point recovery at " .. meter)
    end

    local inactive = decision(false, meter)
    check(not inactive.medical_attention and
          inactive.physical_meter_delta == 0 and
          inactive.mental_meter_delta == 0 and
          not inactive.clear_recovery_request,
          "inactive runner remains unchanged at " .. meter)
end

local party = Hotel.service_party_status({
    runners = {
        { active = true, physical_meter = 2 },
        { active = false, physical_meter = 4 },
        { active = true, physical_meter = 4 },
    },
})
check(#party.runners == 3 and
      party.runners[1].medical_attention and
      not party.runners[2].medical_attention and
      party.runners[2].mental_meter_delta == 0 and
      party.runners[3].physical_meter_delta == 1,
      "party decisions retain source runner order and active state")

for _, invalid in ipairs({-1, 1.5, 256, "3"}) do
    local ok = pcall(decision, true, invalid)
    check(not ok, "rejects invalid physical meter " .. tostring(invalid))
end
check(not pcall(Hotel.service_party_status, nil),
      "rejects missing party state")
check(pcall(Hotel.service_party_status, { runners = {} }),
      "accepts an empty ordered party list")
check(not pcall(Hotel.service_party_status, {
    runners = {{ active = 1, physical_meter = 2 }},
}), "rejects non-boolean active state")
check(not pcall(Hotel.service_party_status, {
    runners = {{ active = true }},
}), "rejects missing physical meter")

print(string.format("%d/%d Hotel status checks passed", pass, total))
if pass ~= total then os.exit(1) end
