-- Speaker properties are supplied by gameplay content and resolved from the
-- speaker id carried by the active map/location context.
local Dialogue = {}

local speakers = {
    stoker = {
        name = "Stoker",
        text_profile = "stoker.motel",
        portrait_alias = "dialogue.portraits.stoker.motel",
    },
}

function Dialogue.speaker_profile(speaker_id)
    return speakers[speaker_id]
end

return Dialogue
