-- Stable IDs for the ROM-ordered Cyberware page copy.
-- The native field presenter owns inventory state, focus, and rendering.
local Cyberware = {}

function Cyberware.text_entries()
    local ids = {
        "title",
        "separator",
        "empty",
        "datajack",
        "cyber_eyes",
        "hand_razors",
        "spurs",
        "smartlink",
        "muscle_replace_1",
        "muscle_replace_2",
        "muscle_replace_3",
        "muscle_replace_4",
        "dermal_plating_1",
        "dermal_plating_2",
        "dermal_plating_3",
        "wired_reflexes_1",
        "wired_reflexes_2",
        "wired_reflexes_3",
    }
    local label_keys = {}
    for index, id in ipairs(ids) do
        label_keys[index] = "cyberware.text.option." .. id
    end
    return ids, nil, label_keys
end

return Cyberware
