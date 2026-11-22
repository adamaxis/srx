-- A reviewable production checkpoint for the Salish b2 milestone. The full
-- W3 route remains in route_w3.lua and retains its later gates unchanged.
local route = require("walkthrough.route_w3")
local steps = {}
for index = 1, 5 do
    steps[index] = route.steps[index]
end

return {
    resume_from_fixture_step = route.resume_from_fixture_step,
    steps = steps,
}
