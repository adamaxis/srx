-- Compatibility text lookup derived from the ROM-imported rows. Keep authored
-- strings out of this file; tools/enumerate_field_contacts.py owns the data.
local imported = require("data.field_contact_dialogue_rows")
local dialogue_text = {}

for content_id, response in pairs(imported.responses or {}) do
    if type(response.opening_text) == "string" and
       response.opening_text ~= "" then
        dialogue_text[content_id] = response.opening_text
    end
end

return dialogue_text
