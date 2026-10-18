-- ROM commands 0x6E/0x6F: fare and world-mode destination records for
-- Sharkey's two passage offers. C++ receives only the selected typed request.
local M = {
    salish_shidhe = { destination_scene = 6, fare = 3000 },
    sinsearach = { destination_scene = 9, fare = 5000 },
}

function M.service_action(passage_id)
    local passage = M[passage_id]
    if type(passage) ~= "table" or
       type(passage.destination_scene) ~= "number" or
       passage.destination_scene % 1 ~= 0 or
       passage.destination_scene < 0 or passage.destination_scene > 255 or
       type(passage.fare) ~= "number" or passage.fare % 1 ~= 0 or
       passage.fare < 0 or passage.fare > 0xFFFFFFFF then
        return nil
    end
    return "passage_to_scene_" .. tostring(passage.destination_scene) ..
           ".fare_" .. tostring(passage.fare)
end

return M
