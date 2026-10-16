#include "shadowrun_ghidra/scripting/cyberdeck_menu_bindings.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

namespace shadowrun::ghidra_conversion::scripting {
namespace {

int integer_argument(lua_State* lua, const int index) {
    int is_number = 0;
    const auto value = lua_tointegerx(lua, index, &is_number);
    return is_number ? static_cast<int>(value) : 0;
}

void push_record(lua_State* lua, const ScriptCyberdeckRecord& record) {
    lua_createtable(lua, 0, 12);
    const auto record_index = lua_absindex(lua, -1);
    lua_pushinteger(lua, record.mpcp);
    lua_setfield(lua, record_index, "mpcp");
    lua_pushinteger(lua, record.hardening);
    lua_setfield(lua, record_index, "hardening");
    lua_pushinteger(lua, record.memory);
    lua_setfield(lua, record_index, "memory");
    lua_pushinteger(lua, record.storage);
    lua_setfield(lua, record_index, "storage");
    lua_pushinteger(lua, record.load_io);
    lua_setfield(lua, record_index, "load_io");
    lua_pushinteger(lua, record.response);
    lua_setfield(lua, record_index, "response");
    lua_pushinteger(lua, record.persona_condition);
    lua_setfield(lua, record_index, "persona_condition");
    lua_pushinteger(lua, record.model);
    lua_setfield(lua, record_index, "model");

    const auto push_array = [lua, record_index](const char* name,
                                                 const auto& values) {
        lua_createtable(lua, static_cast<int>(values.size()), 0);
        const auto array_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < values.size(); ++index) {
            lua_pushinteger(lua, values[index]);
            lua_rawseti(lua, array_index, static_cast<lua_Integer>(index));
        }
        lua_setfield(lua, record_index, name);
    };
    push_array("persona", record.persona);
    push_array("ratings", record.ratings);
    push_array("active", record.active_programs);

    lua_createtable(lua, static_cast<int>(record.files.size()), 0);
    const auto files_index = lua_absindex(lua, -1);
    for (std::size_t index = 0; index < record.files.size(); ++index) {
        lua_createtable(lua, 0, 2);
        lua_pushinteger(lua, record.files[index].id);
        lua_setfield(lua, -2, "id");
        lua_pushinteger(lua, record.files[index].size);
        lua_setfield(lua, -2, "size");
        lua_rawseti(lua, files_index, static_cast<lua_Integer>(index));
    }
    lua_setfield(lua, record_index, "files");
}

int read_integer(lua_State* lua, const int table_index, const char* name,
                 const lua_Integer fallback, const lua_Integer minimum,
                 const lua_Integer maximum) {
    lua_getfield(lua, table_index, name);
    int is_number = 0;
    const auto value = lua_tointegerx(lua, -1, &is_number);
    lua_pop(lua, 1);
    return static_cast<int>(std::clamp<lua_Integer>(
        is_number ? value : fallback, minimum, maximum));
}

template <typename Values>
void read_array(lua_State* lua, const int table_index, const char* name,
                Values& values, const int minimum, const int maximum) {
    lua_getfield(lua, table_index, name);
    if (lua_istable(lua, -1)) {
        const auto array_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < values.size(); ++index) {
            lua_rawgeti(lua, array_index, static_cast<lua_Integer>(index));
            int is_number = 0;
            const auto value = lua_tointegerx(lua, -1, &is_number);
            if (is_number) {
                values[index] = static_cast<typename Values::value_type>(
                    std::clamp<lua_Integer>(value, minimum, maximum));
            }
            lua_pop(lua, 1);
        }
    }
    lua_pop(lua, 1);
}

ScriptCyberdeckRecord read_record(lua_State* lua, const int table_index) {
    ScriptCyberdeckRecord record;
    record.mpcp = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "mpcp", 0, 0, 0xFF));
    record.hardening = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "hardening", 0, 0, 0xFF));
    record.memory = static_cast<std::uint16_t>(
        read_integer(lua, table_index, "memory", 0, 0, 0xFFFF));
    record.storage = static_cast<std::uint16_t>(
        read_integer(lua, table_index, "storage", 0, 0, 0xFFFF));
    record.load_io = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "load_io", 0, 0, 0xFF));
    record.response = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "response", 0, 0, 0xFF));
    record.persona_condition = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "persona_condition", 0, 0, 0xFF));
    record.model = static_cast<std::int8_t>(
        read_integer(lua, table_index, "model", -1, -128, 127));
    read_array(lua, table_index, "persona", record.persona, 0, 0xFF);
    read_array(lua, table_index, "ratings", record.ratings, 0, 0xFF);
    read_array(lua, table_index, "active", record.active_programs, -1, 11);

    lua_getfield(lua, table_index, "files");
    if (lua_istable(lua, -1)) {
        const auto files_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < record.files.size(); ++index) {
            lua_rawgeti(lua, files_index, static_cast<lua_Integer>(index));
            if (lua_istable(lua, -1)) {
                const auto file_index = lua_absindex(lua, -1);
                record.files[index].id = static_cast<std::int8_t>(
                    read_integer(lua, file_index, "id", -1, -1, 127));
                record.files[index].size = static_cast<std::uint16_t>(
                    read_integer(lua, file_index, "size", 0, 0, 0xFFFF));
            }
            lua_pop(lua, 1);
        }
    }
    lua_pop(lua, 1);
    return record;
}

std::array<ScriptCyberdeckProgramSlot, 5> read_slots(
    lua_State* lua, const int table_index) {
    std::array<ScriptCyberdeckProgramSlot, 5> slots{};
    for (std::size_t index = 0; index < slots.size(); ++index) {
        lua_rawgeti(lua, table_index, static_cast<lua_Integer>(index));
        if (lua_istable(lua, -1)) {
            const auto slot_index = lua_absindex(lua, -1);
            slots[index].status = static_cast<std::uint8_t>(
                read_integer(lua, slot_index, "status", 0, 0, 0xFF));
            slots[index].timer = static_cast<std::uint16_t>(
                read_integer(lua, slot_index, "timer", 0, 0, 0xFFFF));
            slots[index].phase = static_cast<std::uint8_t>(
                read_integer(lua, slot_index, "phase", 0, 0, 0xFF));
        }
        lua_pop(lua, 1);
    }
    return slots;
}

} // namespace

int dispatch_cyberdeck_menu_binding(
    lua_State* lua, ScriptMenuRouteGame& game,
    const CyberdeckMenuBinding binding) {
    switch (binding) {
    case CyberdeckMenuBinding::enter:
        lua_pushinteger(lua, game.deck_enter ? game.deck_enter() : 0);
        return 1;
    case CyberdeckMenuBinding::leave:
        if (game.deck_leave) game.deck_leave(integer_argument(lua, 1));
        return 0;
    case CyberdeckMenuBinding::tick:
        if (game.deck_tick) game.deck_tick();
        return 0;
    case CyberdeckMenuBinding::set_mode:
        if (game.set_deck_mode) {
            const int source_mode = integer_argument(lua, 1);
            // FFF0B7 is a byte in the ROM: attack is 0xFF, balanced is 0,
            // and defense is 1. The host API uses signed -1/0/1 values.
            const int host_mode = source_mode == 0xFF
                                      ? -1
                                      : std::clamp(source_mode, 0, 1);
            game.set_deck_mode(host_mode);
        }
        return 0;
    case CyberdeckMenuBinding::matrix_online:
        lua_pushboolean(lua, game.matrix_online && game.matrix_online());
        return 1;
    case CyberdeckMenuBinding::matrix_node: {
        const auto node = game.matrix_node ? game.matrix_node() : std::nullopt;
        if (!node.has_value()) {
            lua_pushnil(lua);
            return 1;
        }
        lua_createtable(lua, 0, 4);
        lua_pushinteger(lua, node->ic_condition);
        lua_setfield(lua, -2, "ic_condition");
        lua_pushinteger(lua, node->ic_code);
        lua_setfield(lua, -2, "ic_code");
        lua_pushinteger(lua, node->ic_rating);
        lua_setfield(lua, -2, "ic_rating");
        lua_pushinteger(lua, node->ic_rating2);
        lua_setfield(lua, -2, "ic_rating2");
        return 1;
    }
    case CyberdeckMenuBinding::alert:
        lua_pushinteger(lua, game.alert ? game.alert() : -1);
        return 1;
    case CyberdeckMenuBinding::in_alert:
        lua_pushboolean(lua, game.in_alert && game.in_alert());
        return 1;
    case CyberdeckMenuBinding::map_seen:
        lua_pushinteger(lua, game.map_seen ? game.map_seen() : 0);
        return 1;
    case CyberdeckMenuBinding::record:
        push_record(lua, game.deck_record ? game.deck_record()
                                          : ScriptCyberdeckRecord{});
        return 1;
    case CyberdeckMenuBinding::slots: {
        const auto slots = game.deck_slots
                               ? game.deck_slots()
                               : std::array<ScriptCyberdeckProgramSlot, 5>{};
        lua_createtable(lua, static_cast<int>(slots.size()), 0);
        const auto slots_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < slots.size(); ++index) {
            lua_createtable(lua, 0, 3);
            lua_pushinteger(lua, slots[index].status);
            lua_setfield(lua, -2, "status");
            lua_pushinteger(lua, slots[index].timer);
            lua_setfield(lua, -2, "timer");
            lua_pushinteger(lua, slots[index].phase);
            lua_setfield(lua, -2, "phase");
            lua_rawseti(lua, slots_index, static_cast<lua_Integer>(index));
        }
        return 1;
    }
    case CyberdeckMenuBinding::commit:
        if (game.deck_commit && lua_istable(lua, 1) && lua_istable(lua, 2)) {
            game.deck_commit(read_record(lua, lua_absindex(lua, 1)),
                             read_slots(lua, lua_absindex(lua, 2)));
        }
        return 0;
    case CyberdeckMenuBinding::program_unloaded:
        if (game.program_unloaded) {
            game.program_unloaded(integer_argument(lua, 1));
        }
        return 0;
    }
    lua_pushnil(lua);
    return 1;
}

} // namespace shadowrun::ghidra_conversion::scripting
