-- Small, dependency-free JSON decoder for authored gameplay content.
-- Objects and arrays carry distinct metatables so callers can validate schema
-- shape even when either container is empty.
local Json = {}

local OBJECT_METATABLE = {}
local ARRAY_METATABLE = {}
local MAX_DOCUMENT_BYTES = 4 * 1024 * 1024
local MAX_NESTING_DEPTH = 64
local MAX_CONTAINER_ITEMS = 65536

Json.null = setmetatable({}, { __tostring = function() return "null" end })

function Json.is_object(value)
    return type(value) == "table" and
           getmetatable(value) == OBJECT_METATABLE
end

function Json.is_array(value)
    return type(value) == "table" and
           getmetatable(value) == ARRAY_METATABLE
end

local function append_codepoint(output, codepoint)
    if codepoint <= 0x7F then
        output[#output + 1] = string.char(codepoint)
    elseif codepoint <= 0x7FF then
        output[#output + 1] = string.char(
            0xC0 + math.floor(codepoint / 0x40),
            0x80 + (codepoint % 0x40))
    elseif codepoint <= 0xFFFF then
        output[#output + 1] = string.char(
            0xE0 + math.floor(codepoint / 0x1000),
            0x80 + (math.floor(codepoint / 0x40) % 0x40),
            0x80 + (codepoint % 0x40))
    else
        output[#output + 1] = string.char(
            0xF0 + math.floor(codepoint / 0x40000),
            0x80 + (math.floor(codepoint / 0x1000) % 0x40),
            0x80 + (math.floor(codepoint / 0x40) % 0x40),
            0x80 + (codepoint % 0x40))
    end
end

local function valid_utf8(source)
    local index = 1
    local length = #source
    while index <= length do
        local first = string.byte(source, index)
        if first <= 0x7F then
            index = index + 1
        else
            local needed, minimum
            if first >= 0xC2 and first <= 0xDF then
                needed, minimum = 1, 0x80
            elseif first >= 0xE0 and first <= 0xEF then
                needed, minimum = 2, 0x800
            elseif first >= 0xF0 and first <= 0xF4 then
                needed, minimum = 3, 0x10000
            else
                return false
            end
            if index + needed > length then
                return false
            end
            local codepoint = first % (2 ^ (6 - needed))
            for offset = 1, needed do
                local continuation = string.byte(source, index + offset)
                if continuation < 0x80 or continuation > 0xBF then
                    return false
                end
                codepoint = codepoint * 0x40 + (continuation - 0x80)
            end
            if codepoint < minimum or codepoint > 0x10FFFF or
               (codepoint >= 0xD800 and codepoint <= 0xDFFF) then
                return false
            end
            index = index + needed + 1
        end
    end
    return true
end

local function decode_document(source)
    local position = 1
    local length = #source

    local function fail(message)
        error(message .. " at byte " .. tostring(position), 0)
    end

    local function skip_whitespace()
        while position <= length do
            local byte = string.byte(source, position)
            if byte ~= 0x20 and byte ~= 0x09 and
               byte ~= 0x0A and byte ~= 0x0D then
                return
            end
            position = position + 1
        end
    end

    local function expect(character)
        if string.sub(source, position, position) ~= character then
            fail("expected '" .. character .. "'")
        end
        position = position + 1
    end

    local function read_hex_quad()
        local digits = string.sub(source, position, position + 3)
        if #digits ~= 4 or not string.match(digits, "^%x%x%x%x$") then
            fail("invalid Unicode escape")
        end
        position = position + 4
        return tonumber(digits, 16)
    end

    local function parse_string()
        expect('"')
        local output = {}
        local chunk_start = position
        while position <= length do
            local byte = string.byte(source, position)
            if byte == 0x22 then
                if position > chunk_start then
                    output[#output + 1] =
                        string.sub(source, chunk_start, position - 1)
                end
                position = position + 1
                return table.concat(output)
            elseif byte == 0x5C then
                if position > chunk_start then
                    output[#output + 1] =
                        string.sub(source, chunk_start, position - 1)
                end
                position = position + 1
                local escape = string.sub(source, position, position)
                if escape == '"' or escape == "\\" or escape == "/" then
                    output[#output + 1] = escape
                    position = position + 1
                elseif escape == "b" then
                    output[#output + 1] = "\b"
                    position = position + 1
                elseif escape == "f" then
                    output[#output + 1] = "\f"
                    position = position + 1
                elseif escape == "n" then
                    output[#output + 1] = "\n"
                    position = position + 1
                elseif escape == "r" then
                    output[#output + 1] = "\r"
                    position = position + 1
                elseif escape == "t" then
                    output[#output + 1] = "\t"
                    position = position + 1
                elseif escape == "u" then
                    position = position + 1
                    local codepoint = read_hex_quad()
                    if codepoint >= 0xD800 and codepoint <= 0xDBFF then
                        if string.sub(source, position, position + 1) ~= "\\u" then
                            fail("high surrogate has no low surrogate")
                        end
                        position = position + 2
                        local low = read_hex_quad()
                        if low < 0xDC00 or low > 0xDFFF then
                            fail("invalid low surrogate")
                        end
                        codepoint = 0x10000 +
                                    (codepoint - 0xD800) * 0x400 +
                                    (low - 0xDC00)
                    elseif codepoint >= 0xDC00 and codepoint <= 0xDFFF then
                        fail("unexpected low surrogate")
                    end
                    append_codepoint(output, codepoint)
                else
                    fail("invalid string escape")
                end
                chunk_start = position
            elseif byte < 0x20 then
                fail("unescaped control byte in string")
            else
                position = position + 1
            end
        end
        fail("unterminated string")
    end

    local function parse_number()
        local start = position
        if string.sub(source, position, position) == "-" then
            position = position + 1
        end
        local first = string.sub(source, position, position)
        if first == "0" then
            position = position + 1
            local next_digit = string.sub(source, position, position)
            if next_digit >= "0" and next_digit <= "9" then
                fail("leading zero in number")
            end
        elseif first >= "1" and first <= "9" then
            repeat
                position = position + 1
                first = string.sub(source, position, position)
            until first < "0" or first > "9"
        else
            fail("invalid number")
        end
        if string.sub(source, position, position) == "." then
            position = position + 1
            first = string.sub(source, position, position)
            if first < "0" or first > "9" then
                fail("fraction has no digits")
            end
            repeat
                position = position + 1
                first = string.sub(source, position, position)
            until first < "0" or first > "9"
        end
        first = string.sub(source, position, position)
        if first == "e" or first == "E" then
            position = position + 1
            first = string.sub(source, position, position)
            if first == "+" or first == "-" then
                position = position + 1
                first = string.sub(source, position, position)
            end
            if first < "0" or first > "9" then
                fail("exponent has no digits")
            end
            repeat
                position = position + 1
                first = string.sub(source, position, position)
            until first < "0" or first > "9"
        end
        local value = tonumber(string.sub(source, start, position - 1))
        if value == nil or value ~= value or
           value == math.huge or value == -math.huge then
            fail("number is outside the supported range")
        end
        return value
    end

    local parse_value
    parse_value = function(depth)
        if depth > MAX_NESTING_DEPTH then
            fail("maximum nesting depth exceeded")
        end
        skip_whitespace()
        local character = string.sub(source, position, position)
        if character == '"' then
            return parse_string()
        elseif character == "{" then
            position = position + 1
            skip_whitespace()
            local object = setmetatable({}, OBJECT_METATABLE)
            local seen = {}
            if string.sub(source, position, position) == "}" then
                position = position + 1
                return object
            end
            local count = 0
            while true do
                skip_whitespace()
                if string.sub(source, position, position) ~= '"' then
                    fail("object key must be a string")
                end
                local key = parse_string()
                if seen[key] then
                    fail("duplicate object key")
                end
                seen[key] = true
                skip_whitespace()
                expect(":")
                object[key] = parse_value(depth + 1)
                count = count + 1
                if count > MAX_CONTAINER_ITEMS then
                    fail("object member limit exceeded")
                end
                skip_whitespace()
                local separator = string.sub(source, position, position)
                if separator == "}" then
                    position = position + 1
                    return object
                end
                expect(",")
            end
        elseif character == "[" then
            position = position + 1
            skip_whitespace()
            local array = setmetatable({}, ARRAY_METATABLE)
            if string.sub(source, position, position) == "]" then
                position = position + 1
                return array
            end
            local count = 0
            while true do
                count = count + 1
                if count > MAX_CONTAINER_ITEMS then
                    fail("array item limit exceeded")
                end
                array[count] = parse_value(depth + 1)
                skip_whitespace()
                local separator = string.sub(source, position, position)
                if separator == "]" then
                    position = position + 1
                    return array
                end
                expect(",")
            end
        elseif string.sub(source, position, position + 3) == "true" then
            position = position + 4
            return true
        elseif string.sub(source, position, position + 4) == "false" then
            position = position + 5
            return false
        elseif string.sub(source, position, position + 3) == "null" then
            position = position + 4
            return Json.null
        elseif character == "-" or
               (character >= "0" and character <= "9") then
            return parse_number()
        end
        fail("unexpected value")
    end

    local value = parse_value(0)
    skip_whitespace()
    if position <= length then
        fail("unexpected trailing data")
    end
    return value
end

function Json.decode(source)
    if type(source) ~= "string" then
        return nil, "JSON input must be a string"
    end
    if #source > MAX_DOCUMENT_BYTES then
        return nil, "JSON document exceeds the byte limit"
    end
    if not valid_utf8(source) then
        return nil, "JSON document is not valid UTF-8"
    end
    local ok, value = pcall(decode_document, source)
    if not ok then
        return nil, value
    end
    return value
end

return Json
