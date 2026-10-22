#include "world_content_tsv_loader.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

bool field_string_is(lua_State* state, const int table_index,
                     const char* field, const std::string_view expected) {
    lua_getfield(state, table_index, field);
    std::size_t length = 0U;
    const auto* value = lua_tolstring(state, -1, &length);
    const bool matches =
        value != nullptr && std::string_view(value, length) == expected;
    lua_pop(state, 1);
    return matches;
}

bool field_integer_is(lua_State* state, const int table_index,
                      const char* field, const lua_Integer expected) {
    lua_getfield(state, table_index, field);
    const bool matches =
        lua_isinteger(state, -1) != 0 &&
        lua_tointeger(state, -1) == expected;
    lua_pop(state, 1);
    return matches;
}

bool array_has_id(lua_State* state, const int array_index,
                  const char* id) {
    const auto array = lua_absindex(state, array_index);
    const auto count = lua_rawlen(state, array);
    for (std::size_t index = 1U; index <= count; ++index) {
        lua_rawgeti(state, array, static_cast<lua_Integer>(index));
        const bool matches = field_string_is(state, -1, "id", id);
        lua_pop(state, 1);
        if (matches) {
            return true;
        }
    }
    return false;
}

bool scene_has_instance(lua_State* state, const int array_index,
                        const char* instance_id,
                        const lua_Integer expected_x,
                        const lua_Integer expected_y) {
    const auto array = lua_absindex(state, array_index);
    const auto count = lua_rawlen(state, array);
    for (std::size_t index = 1U; index <= count; ++index) {
        lua_rawgeti(state, array, static_cast<lua_Integer>(index));
        const bool matches =
            field_string_is(state, -1, "instance_id", instance_id) &&
            field_integer_is(state, -1, "origin_x", expected_x) &&
            field_integer_is(state, -1, "origin_y", expected_y);
        lua_pop(state, 1);
        if (matches) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    lua_State* state = luaL_newstate();
    if (!expect(state != nullptr, "could not create Lua state")) {
        return 1;
    }
    lua_newtable(state);
    const int modules_reference = luaL_ref(state, LUA_REGISTRYINDEX);
    std::string failure;
    if (!shadowrun::ghidra_conversion::scripting::detail::
            load_world_content_tsv(
                state, modules_reference, SHADOWRUN_GAMEPLAY_WORLD_ROOT,
                failure)) {
        std::cerr << failure << '\n';
        lua_close(state);
        return 1;
    }

    lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
    lua_getfield(state, -1, "world_content_data");
    lua_remove(state, -2);
    const int data_table = lua_absindex(state, -1);

    lua_getfield(state, data_table, "entities");
    const int entities = lua_absindex(state, -1);
    lua_getfield(state, entities, "stoker_coffin_motel");
    const int stoker = lua_absindex(state, -1);
    bool valid =
        expect(field_string_is(state, stoker, "name",
                               "Stoker's Coffin Motel"),
               "entity.tsv was not loaded into the runtime data table");

    lua_getfield(state, entities, "puyallup_site_04_crime_mall");
    const int crime_mall = lua_absindex(state, -1);
    valid = expect(field_string_is(state, crime_mall, "name", "Crime Mall"),
                   "Puyallup marker 4 must resolve to the Crime Mall") &&
            valid;
    lua_pop(state, 1);

    lua_getfield(state, entities, "puyallup_site_05_underground_93");
    const int underground_93 = lua_absindex(state, -1);
    valid = expect(field_string_is(state, underground_93, "name",
                                   "Underground 93"),
                   "Puyallup marker 5 must resolve to Underground 93") &&
            valid;
    lua_pop(state, 1);

    lua_getfield(state, entities, "scene15_unnamed_old_man_contact");
    const int scene15_contact = lua_absindex(state, -1);
    valid = expect(field_string_is(state, scene15_contact, "speaker_id",
                                   "jackals_lantern_old_man"),
                   "Scene 15 event 30 lost its Lua speaker profile") &&
            valid;
    lua_getfield(state, scene15_contact, "interactables");
    const int scene15_interactables = lua_absindex(state, -1);
    lua_rawgeti(state, scene15_interactables, 1);
    const int scene15_interactable = lua_absindex(state, -1);
    valid = expect(field_string_is(state, scene15_interactable,
                                   "source_event", "event_30") &&
                       field_integer_is(state, scene15_interactable,
                                        "width", 32) &&
                       field_integer_is(state, scene15_interactable,
                                        "height", 32),
                   "Scene 15 interaction bounds did not load as event 30") &&
            valid;
    lua_pop(state, 2);
    lua_getfield(state, scene15_contact, "links");
    valid = expect(lua_isnil(state, -1),
                   "route data remains in the legacy TSV world loader") &&
            valid;
    lua_pop(state, 1);

    lua_getfield(state, stoker, "parts");
    const int stoker_parts = lua_absindex(state, -1);
    valid = expect(array_has_id(state, stoker_parts, "left_pipe_attachment") &&
                       array_has_id(state, stoker_parts, "sign_face") &&
                       array_has_id(state, stoker_parts, "sign_shadow"),
                   "parts.tsv did not preserve separate pipe and sign parts") &&
            valid;

    lua_getfield(state, stoker, "interactables");
    const int stoker_interactables = lua_absindex(state, -1);
    lua_rawgeti(state, stoker_interactables, 1);
    const int stoker_door = lua_absindex(state, -1);
    valid = expect(field_integer_is(state, stoker_door, "local_x", 2) &&
                       field_integer_is(state, stoker_door, "local_y", -3),
                   "interactables.tsv lost the source-aligned motel hitbox") &&
            valid;

    lua_getfield(state, data_table, "areas");
    lua_rawgeti(state, -1, 2);
    const int scene_two = lua_absindex(state, -1);
    valid = expect(
                scene_has_instance(state, scene_two,
                                   "scene02_stokers_coffin_motel", 958, 643),
                "scene-02.tsv placement was not loaded from its area file") &&
            valid;

    lua_rawgeti(state, -2, 1);
    const int scene_one = lua_absindex(state, -1);
    valid = expect(
                scene_has_instance(state, scene_one,
                                   "scene01_puyallup_site_04_crime_mall",
                                   96, 544) &&
                    scene_has_instance(state, scene_one,
                                       "scene01_puyallup_site_05_underground_93",
                                       800, 512),
                "Puyallup site names were not bound to the ROM-aligned markers") &&
            valid;
    lua_pop(state, 1);

    lua_rawgeti(state, -2, 15);
    const int scene_fifteen = lua_absindex(state, -1);
    valid = expect(scene_has_instance(state, scene_fifteen,
                                      "scene15_unnamed_old_man_contact",
                                      288, 32),
                   "scene-15.tsv did not place the event-30 contact region") &&
            valid;
    lua_pop(state, 1);

    lua_getfield(state, data_table, "map_events");
    valid = expect(lua_rawlen(state, -1) == 1U,
                   "scene trigger TSV did not produce the runtime event") &&
            valid;

    lua_close(state);
    return valid ? 0 : 1;
}
