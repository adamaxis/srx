#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"
#include "shadowrun_ghidra/scripting/combat_inventory.hpp"
#include "shadowrun_ghidra/scripting/cyberdeck_menu_bindings.hpp"
#include "shadowrun_ghidra/scripting/run_location_catalog.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"
#include "shadowrun_ghidra/world/world_activity_routine_conversion.hpp"
#include "enums.hpp"
#include "world_content_tsv_loader.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::scripting {
namespace {

// E.ActorStatus.DOWN (0x10): actor record +0x10 bit 4, set at 0xB356 [C].
constexpr std::uint8_t kActorStatusDown = 0x10U;

// Runner menus receive the meaning of +0x10 as named fields. Keep these
// source masks inside the native adapter, where record storage is translated.
// [C] +0x10.0 is active; +0x10.1 is the temporary clip remainder marker;
// Command checks +0x10.4/+0x10.6 at 0x10D36/0x10D40.
constexpr std::uint8_t kRunnerActive = 0x01U;
constexpr std::uint8_t kRunnerClipRemainder = 0x02U;
constexpr std::uint8_t kRunnerDowned = 0x10U;
constexpr std::uint8_t kRunnerDead = 0x40U;
constexpr double kDegreesPerTurn = 360.0;
constexpr double kSourceHeadingUnitsPerTurn = 65536.0;
constexpr double kWorldCoordinatePeriod = 65536.0;
// ROM 0xB77E preserves a negative terrain result when entity flag bit 1 is set [C].
constexpr std::uint8_t kMovementRecoveryEnabledFlag = 0x02U;
struct CompassOffset {
    std::int8_t x = 0;
    std::int8_t y = 0;
};
constexpr std::array<std::string_view, 8> kCompassDirections{
    "north", "north_east", "east", "south_east",
    "south", "south_west", "west", "north_west"};
// Host compass geometry; source coordinate wrapping stays at the native edge [C: 0xB77E, C002].
constexpr std::array<CompassOffset, 8> kCompassOffsets{{
    {0, -1}, {1, -1}, {1, 0}, {1, 1},
    {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}}};
// ROM 0xB7B8 consumes these encodings. Keep them in the native adapter;
// field_events.lua receives only the resulting event facts and rewrite intent.
constexpr std::uint8_t kTerrainMarkerSuppressionByte = 0xDAU;
constexpr std::uint16_t kEvent25RewrittenCollisionCode = 0x00A8U;
constexpr std::int8_t kEvent25DirectionProbe = -0x58;

// ROM turn units stay inside the native adapter. Lua receives a bearing in
// degrees clockwise from north and returns that same semantic quantity.
double bearing_degrees_from_source(const std::uint16_t heading) {
    return static_cast<double>(heading) * kDegreesPerTurn /
           kSourceHeadingUnitsPerTurn;
}

std::optional<std::uint16_t> source_heading_from_bearing_degrees(
    const double degrees) {
    if (!std::isfinite(degrees) || degrees < 0.0 ||
        degrees >= kDegreesPerTurn) {
        return std::nullopt;
    }
    const auto source_units = std::llround(
        degrees * kSourceHeadingUnitsPerTurn / kDegreesPerTurn);
    return static_cast<std::uint16_t>(source_units & 0xFFFFLL);
}

bool read_optional_source_heading_from_bearing_field(
    lua_State* state, const int table_index, const char* field_name,
    std::optional<std::uint16_t>& output) {
    lua_getfield(state, table_index, field_name);
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        output.reset();
        return true;
    }
    if (lua_type(state, -1) != LUA_TNUMBER) {
        lua_pop(state, 1);
        return false;
    }
    const auto degrees = static_cast<double>(lua_tonumber(state, -1));
    lua_pop(state, 1);
    const auto source_heading = source_heading_from_bearing_degrees(degrees);
    if (!source_heading.has_value()) {
        return false;
    }
    output = *source_heading;
    return true;
}

bool world_coordinate_cell(const double coordinate,
                           std::uint16_t& output) {
    if (!std::isfinite(coordinate) || coordinate < 0.0 ||
        coordinate >= kWorldCoordinatePeriod) {
        return false;
    }
    output = static_cast<std::uint16_t>(std::floor(coordinate));
    return true;
}

bool read_number(lua_State* state, const int index, double& value) {
    if (lua_type(state, index) != LUA_TNUMBER) {
        return false;
    }
    value = static_cast<double>(lua_tonumber(state, index));
    return std::isfinite(value);
}

const char* formation_position_id(
    const ScriptFormationPosition position) noexcept {
    switch (position) {
    case ScriptFormationPosition::primary:
        return "primary";
    case ScriptFormationPosition::secondary:
        return "secondary";
    case ScriptFormationPosition::other:
        return "other";
    }
    return "unknown";
}

void push_formation_position(
    lua_State* state, const char* field,
    const std::optional<ScriptFormationPosition>& position) {
    if (position.has_value()) {
        lua_pushstring(state, formation_position_id(*position));
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, field);
}

const char* party_member_role_id(
    const ScriptPartyMemberRole role) noexcept {
    switch (role) {
    case ScriptPartyMemberRole::leader:
        return "leader";
    case ScriptPartyMemberRole::companion_one:
        return "companion_one";
    case ScriptPartyMemberRole::companion_two:
        return "companion_two";
    }
    return "unknown";
}

const char* target_selection_policy_id(
    const ScriptTargetSelectionPolicy policy) noexcept {
    switch (policy) {
    case ScriptTargetSelectionPolicy::standard:
        return "standard";
    case ScriptTargetSelectionPolicy::marker:
        return "marker";
    }
    return "standard";
}

const char* field_scene_mode_id(const ScriptFieldSceneMode mode) noexcept {
    switch (mode) {
    case ScriptFieldSceneMode::none:
        return "none";
    case ScriptFieldSceneMode::street:
        return "street";
    case ScriptFieldSceneMode::building:
        return "building";
    case ScriptFieldSceneMode::cave:
        return "cave";
    case ScriptFieldSceneMode::unknown:
        return "unknown";
    }
    return "unknown";
}

const char* scene_activity_state_id(
    const ScriptSceneActivityState state) noexcept {
    switch (state) {
    case ScriptSceneActivityState::idle:
        return "idle";
    case ScriptSceneActivityState::actors_being_installed:
        return "actors_being_installed";
    case ScriptSceneActivityState::story_interaction_active:
        return "story_interaction_active";
    case ScriptSceneActivityState::other_activity_active:
        return "other_activity_active";
    case ScriptSceneActivityState::queued_activity:
        return "queued_activity";
    }
    return "idle";
}

const char* movement_direction_id(
    const MovementDirection direction) noexcept {
    switch (direction) {
    case MovementDirection::north:
        return "north";
    case MovementDirection::north_east:
        return "north_east";
    case MovementDirection::east:
        return "east";
    case MovementDirection::south_east:
        return "south_east";
    case MovementDirection::south:
        return "south";
    case MovementDirection::south_west:
        return "south_west";
    case MovementDirection::west:
        return "west";
    case MovementDirection::north_west:
        return "north_west";
    }
    return "unknown";
}

std::optional<MovementDirection> movement_direction_from_id(
    const std::string_view id) noexcept {
    if (id == "north") return MovementDirection::north;
    if (id == "north_east") return MovementDirection::north_east;
    if (id == "east") return MovementDirection::east;
    if (id == "south_east") return MovementDirection::south_east;
    if (id == "south") return MovementDirection::south;
    if (id == "south_west") return MovementDirection::south_west;
    if (id == "west") return MovementDirection::west;
    if (id == "north_west") return MovementDirection::north_west;
    return std::nullopt;
}

std::optional<ScriptCardinalDirection> cardinal_direction_from_id(
    const std::string_view id) noexcept {
    if (id == "north") return ScriptCardinalDirection::north;
    if (id == "east") return ScriptCardinalDirection::east;
    if (id == "south") return ScriptCardinalDirection::south;
    if (id == "west") return ScriptCardinalDirection::west;
    return std::nullopt;
}

const char* actor_engagement_policy_id(
    const ScriptActorEngagementPolicy policy) noexcept {
    switch (policy) {
    case ScriptActorEngagementPolicy::patrol:
        return "patrol";
    case ScriptActorEngagementPolicy::watch_nearby_runner:
        return "watch_nearby_runner";
    case ScriptActorEngagementPolicy::engage_within_range:
        return "engage_within_range";
    case ScriptActorEngagementPolicy::engage:
        return "engage";
    }
    return "engage";
}

const char* actor_approach_band_id(
    const ScriptActorApproachBand band) noexcept {
    switch (band) {
    case ScriptActorApproachBand::far:
        return "far";
    case ScriptActorApproachBand::medium:
        return "medium";
    case ScriptActorApproachBand::close:
        return "close";
    }
    return "unknown";
}

std::optional<std::string_view> script_spell_id_from_action_code(
    const std::uint8_t action_code) {
    if (action_code == 0U) {
        return std::nullopt;
    }
    const auto* spell = find_us_magic_spell(
        static_cast<std::uint8_t>(action_code - 1U));
    return spell == nullptr
        ? std::nullopt
        : std::optional<std::string_view>{spell->id};
}

std::optional<std::uint8_t> script_spell_action_code_from_id(
    const std::string_view spell_id) {
    const auto* spell = find_us_magic_spell(spell_id);
    if (spell == nullptr || spell->spell_code >= kUsMagicSpellCount) {
        return std::nullopt;
    }
    // Live action records store the catalog row plus one; zero is no action.
    return static_cast<std::uint8_t>(spell->spell_code + 1U);
}

std::optional<std::uint8_t> script_action_code_from_semantic_id(
    const std::string_view action_kind, const std::string_view action_id) {
    if (action_kind == "none") {
        return action_id.empty()
            ? std::optional<std::uint8_t>{0U}
            : std::nullopt;
    }
    if (action_kind == "item") {
        return script_item_source_code_from_id(action_id);
    }
    if (action_kind == "spell") {
        return script_spell_action_code_from_id(action_id);
    }
    return std::nullopt;
}

void push_runner_menu_facts(lua_State* lua,
                           const std::uint8_t source_status) {
    lua_createtable(lua, 0, 2);
    const auto runner_index = lua_absindex(lua, -1);
    lua_createtable(lua, 0, 3);
    lua_pushboolean(lua, (source_status & kRunnerActive) != 0U);
    lua_setfield(lua, -2, "active");
    lua_pushboolean(lua, (source_status & kRunnerDowned) != 0U);
    lua_setfield(lua, -2, "downed");
    lua_pushboolean(lua, (source_status & kRunnerDead) != 0U);
    lua_setfield(lua, -2, "dead");
    lua_setfield(lua, runner_index, "lifecycle");
    lua_pushboolean(lua, (source_status & kRunnerClipRemainder) != 0U);
    lua_setfield(lua, runner_index, "clip_distribution_remainder");
}

void merge_runner_menu_facts(lua_State* lua, const int member_index,
                             const std::uint8_t original_status,
                             std::uint8_t& status) {
    status = original_status;
    const auto apply_boolean = [lua, &status](const int table_index,
                                               const char* field,
                                               const std::uint8_t mask) {
        lua_getfield(lua, table_index, field);
        if (lua_isboolean(lua, -1)) {
            if (lua_toboolean(lua, -1) != 0) {
                status = static_cast<std::uint8_t>(status | mask);
            } else {
                status = static_cast<std::uint8_t>(
                    status & static_cast<std::uint8_t>(~mask));
            }
        }
        lua_pop(lua, 1);
    };

    lua_getfield(lua, member_index, "runner_facts");
    if (lua_istable(lua, -1)) {
        const auto facts_index = lua_absindex(lua, -1);
        lua_getfield(lua, facts_index, "lifecycle");
        if (lua_istable(lua, -1)) {
            const auto lifecycle_index = lua_absindex(lua, -1);
            apply_boolean(lifecycle_index, "active", kRunnerActive);
            apply_boolean(lifecycle_index, "downed", kRunnerDowned);
            apply_boolean(lifecycle_index, "dead", kRunnerDead);
        }
        lua_pop(lua, 1);
        apply_boolean(facts_index, "clip_distribution_remainder",
                      kRunnerClipRemainder);
    }
    lua_pop(lua, 1);
}

// Older saves stored the byte selected from ROM's F3EBC offer row under
// source_initializer/source_offer_type. Decode it here and pass only the
// named gameplay variant into Lua. [C] The row selector indexes the callback
// table at 0xF3DC6.
std::optional<std::string_view> legacy_run_offer_variant_id(
    const std::int64_t selector) noexcept {
    static constexpr std::array<std::string_view, 13> kVariants{
        "ghoul_bounty", "bodyguard_escort", "package_delivery",
        "gang_contract", "package_recovery", "employee_recruitment",
        "matrix_simple", "matrix_moderate", "matrix_expert",
        "package_recovery_two_site_pool", "package_recovery_renraku",
        "employee_recruitment_two_site_pool",
        "employee_recruitment_renraku"};
    if (selector < 0 || selector >=
            static_cast<std::int64_t>(kVariants.size())) {
        return std::nullopt;
    }
    return kVariants[static_cast<std::size_t>(selector)];
}

bool valid_run_contact_id(std::string_view value);
bool valid_run_client_id(std::string_view value);

std::optional<std::string_view> legacy_run_id(
    const std::int64_t source_type) noexcept {
    static constexpr std::array<std::string_view, 7> kRunIds{
        "ghoul_bounty", "bodyguard_escort", "package_delivery",
        "gang_contract", "package_recovery", "employee_recruitment",
        "matrix_objective"};
    if (source_type < 0 || source_type >=
            static_cast<std::int64_t>(kRunIds.size())) {
        return std::nullopt;
    }
    return kRunIds[static_cast<std::size_t>(source_type)];
}

std::optional<std::string_view> legacy_run_contact_id(
    const std::int64_t source_context) noexcept {
    static constexpr std::array<std::string_view, 5> kContactIds{
        "jump_house", "matchstick", "big_rhino", "wanderer",
        "icarus_descending"};
    if (source_context < 0 || source_context >=
            static_cast<std::int64_t>(kContactIds.size())) {
        return std::nullopt;
    }
    return kContactIds[static_cast<std::size_t>(source_context)];
}

std::optional<std::string_view> legacy_run_client_id(
    const std::int64_t source_index) noexcept {
    static constexpr std::array<std::string_view, 12> kClientIds{
        "mike_nana", "john_worley", "ellis_goodson", "john_van_anthony",
        "dok_whitson", "jase_weese", "heinrich_michaels", "billy_gibson",
        "nero_manser", "alan_turing", "chris_stout", "jorge_kish"};
    if (source_index < 0 || source_index >=
            static_cast<std::int64_t>(kClientIds.size())) {
        return std::nullopt;
    }
    return kClientIds[static_cast<std::size_t>(source_index)];
}

bool normalize_legacy_run_identity(
    ScriptPersistentValue::Object& offer, std::string& failure) {
    const auto run_type = offer.find("source_run_type");
    if (run_type != offer.end()) {
        const auto* value = std::get_if<std::int64_t>(&run_type->second.value);
        const auto decoded = value == nullptr
            ? std::optional<std::string_view>{}
            : legacy_run_id(*value);
        const auto run_id = offer.find("id");
        const auto* saved_run_id = run_id == offer.end()
            ? nullptr
            : std::get_if<std::string>(&run_id->second.value);
        if (!decoded.has_value() || saved_run_id == nullptr ||
            *saved_run_id != *decoded) {
            failure = "saved run identity conflicts with its legacy type";
            return false;
        }
    }

    std::optional<std::string_view> decoded_contact;
    for (const auto field_name : {"source_offer_context", "context"}) {
        const auto field = offer.find(field_name);
        if (field == offer.end()) continue;
        if (const auto* value =
                std::get_if<std::int64_t>(&field->second.value)) {
            const auto decoded = legacy_run_contact_id(*value);
            if (!decoded.has_value() ||
                (decoded_contact.has_value() &&
                 *decoded_contact != *decoded)) {
                failure = "saved contact conflicts with its legacy context";
                return false;
            }
            decoded_contact = decoded;
        } else if (const auto* value =
                       std::get_if<std::string>(&field->second.value)) {
            if (!valid_run_contact_id(*value) ||
                (decoded_contact.has_value() &&
                 *decoded_contact != *value)) {
                failure = "saved contact has an invalid legacy value";
                return false;
            }
            decoded_contact = *value;
        } else {
            failure = "saved contact has an invalid legacy type";
            return false;
        }
    }

    if (decoded_contact.has_value()) {
        const auto contact = offer.find("contact_id");
        if (contact != offer.end()) {
            const auto* saved_contact =
                std::get_if<std::string>(&contact->second.value);
            if (saved_contact == nullptr ||
                *saved_contact != *decoded_contact) {
                failure = "saved contact conflicts with its semantic ID";
                return false;
            }
        } else {
            ScriptPersistentValue contact_value{};
            contact_value.value = std::string(*decoded_contact);
            offer.emplace("contact_id", std::move(contact_value));
        }
    }

    offer.erase("source_run_type");
    offer.erase("source_offer_context");
    offer.erase("context");
    return true;
}

bool normalize_legacy_run_client_ids(
    ScriptPersistentValue::Object& offer, std::string& failure) {
    const auto decode_client = [&failure](
                                  const ScriptPersistentValue& value)
        -> std::optional<std::string_view> {
        if (const auto* index = std::get_if<std::int64_t>(&value.value)) {
            const auto decoded = legacy_run_client_id(*index);
            if (!decoded.has_value()) {
                failure = "saved client index is outside its legacy table";
            }
            return decoded;
        }
        if (const auto* id = std::get_if<std::string>(&value.value)) {
            if (!valid_run_client_id(*id)) {
                failure = "saved client ID is unknown";
                return std::nullopt;
            }
            return *id;
        }
        failure = "saved client identity has an invalid type";
        return std::nullopt;
    };

    const auto route_field = offer.find("route");
    if (route_field != offer.end()) {
        auto* route = std::get_if<ScriptPersistentValue::Object>(
            &route_field->second.value);
        if (route == nullptr) {
            failure = "saved run route is not an object";
            return false;
        }
        const auto client = route->find("client_id");
        if (client != route->end()) {
            const auto decoded = decode_client(client->second);
            if (!decoded.has_value()) return false;
            ScriptPersistentValue semantic_client{};
            semantic_client.value = std::string(*decoded);
            client->second = std::move(semantic_client);
        }
    }

    const auto offer_id_field = offer.find("id");
    const auto* offer_id = offer_id_field == offer.end()
        ? nullptr
        : std::get_if<std::string>(&offer_id_field->second.value);
    const auto legacy_client = offer.find("client_id");
    const auto employee_id = offer.find("employee_id");
    if (offer_id != nullptr && *offer_id == "employee_recruitment") {
        std::optional<std::string_view> decoded;
        if (legacy_client != offer.end()) {
            decoded = decode_client(legacy_client->second);
            if (!decoded.has_value()) return false;
        }
        if (employee_id != offer.end()) {
            const auto* saved_id =
                std::get_if<std::string>(&employee_id->second.value);
            if (saved_id == nullptr || !valid_run_client_id(*saved_id) ||
                (decoded.has_value() && *saved_id != *decoded)) {
                failure = "saved employee conflicts with its client ID";
                return false;
            }
        } else if (decoded.has_value()) {
            ScriptPersistentValue semantic_employee{};
            semantic_employee.value = std::string(*decoded);
            offer.emplace("employee_id", std::move(semantic_employee));
        }
        offer.erase("client_id");
    } else if (employee_id != offer.end()) {
        failure = "non-recruitment run has an employee ID";
        return false;
    } else if (legacy_client != offer.end()) {
        failure = "run has an unexpected top-level client identity";
        return false;
    }

    // Older offers duplicated the selected employee's display name. The
    // native migration keeps the stable identity and derives its display name.
    offer.erase("employee_name");
    return true;
}

bool normalize_saved_gang_contract_variant(
    ScriptPersistentValue::Object& offer, std::string& failure) {
    static constexpr std::array<std::string_view, 4> kVariantIds{
        "building_vandalism", "renraku_information_leak",
        "local_terrorism", "insurance_premiums"};
    const auto id_field = offer.find("id");
    const auto* run_id = id_field == offer.end()
        ? nullptr
        : std::get_if<std::string>(&id_field->second.value);
    const auto legacy = offer.find("variant");
    const auto semantic = offer.find("contract_variant_id");
    if (run_id == nullptr || *run_id != "gang_contract") {
        if (legacy != offer.end() || semantic != offer.end()) {
            failure = "non-gang run has a gang contract variant";
            return false;
        }
        return true;
    }

    std::optional<std::string_view> variant_id;
    if (legacy != offer.end()) {
        const auto* index = std::get_if<std::int64_t>(&legacy->second.value);
        if (index == nullptr || *index < 0 ||
            *index >= static_cast<std::int64_t>(kVariantIds.size())) {
            failure = "saved gang variant is outside its legacy table";
            return false;
        }
        variant_id = kVariantIds[static_cast<std::size_t>(*index)];
    }
    if (semantic != offer.end()) {
        const auto* value = std::get_if<std::string>(&semantic->second.value);
        if (value == nullptr ||
            std::find(kVariantIds.begin(), kVariantIds.end(), *value) ==
                kVariantIds.end() ||
            (variant_id.has_value() && *variant_id != *value)) {
            failure = "saved gang contract variant ID is invalid";
            return false;
        }
        variant_id = *value;
    }
    if (!variant_id.has_value()) {
        failure = "saved gang run has no contract variant identity";
        return false;
    }
    ScriptPersistentValue value{};
    value.value = std::string(*variant_id);
    if (semantic == offer.end()) {
        offer.emplace("contract_variant_id", std::move(value));
    } else {
        semantic->second = std::move(value);
    }
    offer.erase("variant");
    return true;
}

bool normalize_legacy_matrix_run_state(
    ScriptPersistentValue::Object& offer, std::string& failure) {
    const auto id_field = offer.find("id");
    const auto* run_id = id_field == offer.end()
        ? nullptr
        : std::get_if<std::string>(&id_field->second.value);
    if (run_id == nullptr || *run_id != "matrix_objective") {
        return true;
    }

    const auto state_field = offer.find("matrix_run_state");
    if (state_field == offer.end()) return true;
    auto* run_state = std::get_if<ScriptPersistentValue::Object>(
        &state_field->second.value);
    if (run_state == nullptr || run_state->find("status") != run_state->end()) {
        return true;
    }

    // Previous save versions stored packed Matrix record fields. Decode once
    // here; run_registry.lua only receives the named state used by gameplay.
    const auto read_integer = [run_state](const char* name)
        -> std::optional<std::int64_t> {
        const auto field = run_state->find(name);
        if (field == run_state->end()) return std::nullopt;
        const auto* value = std::get_if<std::int64_t>(&field->second.value);
        return value == nullptr ? std::nullopt
                                : std::optional<std::int64_t>{*value};
    };
    const auto legacy_status = read_integer("state");
    const auto legacy_class = read_integer("objective");
    const auto legacy_value = read_integer("value");
    const auto legacy_size = read_integer("size");
    const auto completion_flags = read_integer("fc28");
    const auto run_flags = read_integer("fc1c");
    const auto in_range = [](const std::optional<std::int64_t>& value,
                             const std::int64_t minimum,
                             const std::int64_t maximum) {
        return value.has_value() && *value >= minimum && *value <= maximum;
    };
    if (!in_range(legacy_status, -128, 127) ||
        !in_range(legacy_class, -128, 127) ||
        !in_range(legacy_value, -128, 127) ||
        !in_range(legacy_size, 0, 255) ||
        !in_range(completion_flags, 0, 255) ||
        !in_range(run_flags, 0, 255)) {
        failure = "saved Matrix run has invalid legacy record values";
        return false;
    }

    static constexpr std::array<std::string_view, 47> kFileClassLabels{
        "Outdated Files", "System Files", "Mngmnt Files", "Project Files",
        "Security Files", "Dead Projects", "Competition", "Marketing Data",
        "Simsense Files", "Financial Data", "Confidntl Data", "Legal Files",
        "Prisoner Files", "Case Files", "Unlisted", "Seattle Gneral",
        "Renraku", "Fuchi", "Mitsuhama", "Hlywd Corr Fac", "Aztechnology",
        "UCAS Fed. Gov.", "Gates Undrsnd.", "Ares", "Lone Star", "City Hall",
        "Club Penumbra", "Building Maint", "Security", "Executive Area",
        "Research", "Marketing", "Retail Control", "Data Routing",
        "Office Mangmnt", "Shipping", "Simsense Rec.", "Terminal", "Matrix Jack",
        "Security Cntrl", "Cameras", "Maglocks", "Alert Control",
        "Automtd. Equip", "Elevator Cntrl", "Undrwatr Equip", "HVAC Systems",
    };

    const auto completed = ((*run_flags & 0x80) != 0) || *legacy_status > 0;
    const auto status = completed ? "completed"
        : *legacy_status < 0 ? "active" : "inactive";
    ScriptPersistentValue::Object objective;
    const auto add_string = [](ScriptPersistentValue::Object& object,
                               const char* key, std::string value) {
        ScriptPersistentValue field{};
        field.value = std::move(value);
        object.emplace(key, std::move(field));
    };
    const auto add_integer = [](ScriptPersistentValue::Object& object,
                                const char* key, std::int64_t value) {
        ScriptPersistentValue field{};
        field.value = value;
        object.emplace(key, std::move(field));
    };
    add_string(objective, "action", "crash_system");
    if (*legacy_class < 0) {
        add_string(objective, "node", "cpu");
    } else {
        if (*legacy_class >=
            static_cast<std::int64_t>(kFileClassLabels.size())) {
            failure = "saved Matrix file class is outside its legacy table";
            return false;
        }
        const auto action = *legacy_value < 0 ? "upload_file"
            : *legacy_value == 0 ? "delete_file" : "download_file";
        if (*legacy_value >= 0 && *legacy_value > 69) {
            failure = "saved Matrix file selection is outside its legacy range";
            return false;
        }
        objective.clear();
        add_string(objective, "node", "datastore");
        add_string(objective, "action", action);
        add_string(objective, "file_class",
                   std::string(kFileClassLabels[
                       static_cast<std::size_t>(*legacy_class)]));
        if (*legacy_value >= 0) {
            add_integer(objective, "file_id", *legacy_value);
        }
    }

    std::optional<ScriptPersistentValue> target_system_id;
    if (const auto target = run_state->find("target_system_id");
        target != run_state->end()) {
        target_system_id = target->second;
    }
    run_state->clear();
    add_string(*run_state, "status", status);
    ScriptPersistentValue objective_value{};
    objective_value.value = std::move(objective);
    run_state->emplace("objective", std::move(objective_value));
    add_integer(*run_state, "file_size", *legacy_size);
    ScriptPersistentValue completion_marker{};
    completion_marker.value = ((*completion_flags & 1) != 0);
    run_state->emplace("completion_marker_present",
                       std::move(completion_marker));
    ScriptPersistentValue completion_recorded{};
    completion_recorded.value = completed;
    run_state->emplace("completion_recorded", std::move(completion_recorded));
    if (target_system_id.has_value()) {
        run_state->emplace("target_system_id", std::move(*target_system_id));
    }
    return true;
}

bool normalize_saved_run_locations(
    ScriptPersistentValue::Object& offer, std::string& failure) {
    const auto run_id_field = offer.find("id");
    const auto* run_id = run_id_field == offer.end()
        ? nullptr
        : std::get_if<std::string>(&run_id_field->second.value);
    const auto route_field = offer.find("route");
    if (route_field != offer.end()) {
        auto* route = std::get_if<ScriptPersistentValue::Object>(
            &route_field->second.value);
        if (route == nullptr) {
            failure = "saved run route is not an object";
            return false;
        }
        const auto normalize_point = [&failure](
                                         ScriptPersistentValue& value) {
            auto* point = std::get_if<ScriptPersistentValue::Object>(
                &value.value);
            if (point == nullptr) {
                failure = "saved run place is not an object";
                return false;
            }
            const auto location_field = point->find("location_id");
            const auto region_field = point->find("area_id");
            std::optional<std::string_view> semantic_location;
            if (location_field != point->end()) {
                if (const auto* value = std::get_if<std::string>(
                        &location_field->second.value)) {
                    semantic_location = run_region_id_for_location(*value)
                        .has_value()
                        ? std::optional<std::string_view>{*value}
                        : std::nullopt;
                } else if (const auto* selector = std::get_if<std::int64_t>(
                               &location_field->second.value)) {
                    const auto* region_selector = region_field == point->end()
                        ? nullptr
                        : std::get_if<std::int64_t>(
                              &region_field->second.value);
                    if (region_selector != nullptr) {
                        semantic_location =
                            run_location_id_for_source_selector(
                                *region_selector, *selector);
                    }
                }
            }
            if (!semantic_location.has_value()) {
                failure = "saved run place has an unknown semantic location";
                return false;
            }
            ScriptPersistentValue semantic_value{};
            semantic_value.value = std::string(*semantic_location);
            if (location_field == point->end()) {
                point->emplace("location_id", std::move(semantic_value));
            } else {
                location_field->second = std::move(semantic_value);
            }
            if (region_field != point->end()) {
                const auto* legacy_region = std::get_if<std::int64_t>(
                    &region_field->second.value);
                const auto expected_region = run_region_id_for_location(
                    *semantic_location);
                if (legacy_region == nullptr || !expected_region.has_value() ||
                    run_region_id_for_source_selector(*legacy_region) !=
                        expected_region) {
                    failure = "saved run place conflicts with its region";
                    return false;
                }
                point->erase(region_field);
            }
            point->erase("location_name");
            point->erase("area_suffix");
            return true;
        };
        for (const auto* point_name : {"origin", "destination", "target"}) {
            const auto point = route->find(point_name);
            if (point != route->end() && !normalize_point(point->second)) {
                return false;
            }
        }
    }

    const auto legacy_region = offer.find("area_id");
    if (legacy_region != offer.end()) {
        if (run_id == nullptr || *run_id != "ghoul_bounty") {
            failure = "run has an unexpected legacy area selector";
            return false;
        }
        const auto* selector = std::get_if<std::int64_t>(
            &legacy_region->second.value);
        const auto region_id = selector == nullptr
            ? std::optional<std::string_view>{}
            : run_region_id_for_source_selector(*selector);
        if (!region_id.has_value() || *selector > 3) {
            failure = "saved ghoul region is outside its legacy table";
            return false;
        }
        const auto current_region = offer.find("region_id");
        if (current_region != offer.end()) {
            const auto* semantic_region = std::get_if<std::string>(
                &current_region->second.value);
            if (semantic_region == nullptr || *semantic_region != *region_id) {
                failure = "saved ghoul region conflicts with its semantic ID";
                return false;
            }
        } else {
            ScriptPersistentValue semantic_region{};
            semantic_region.value = std::string(*region_id);
            offer.emplace("region_id", std::move(semantic_region));
        }
        offer.erase(legacy_region);
    }
    const auto region = offer.find("region_id");
    if (region != offer.end()) {
        const auto* region_id = std::get_if<std::string>(&region->second.value);
        if (run_id == nullptr || *run_id != "ghoul_bounty" ||
            region_id == nullptr || !is_run_region_id(*region_id)) {
            failure = "saved run has an invalid region identity";
            return false;
        }
    }
    return true;
}

bool normalize_saved_offer_variant(
    ScriptPersistentValue& value, std::string& failure) {
    auto* offer = std::get_if<ScriptPersistentValue::Object>(&value.value);
    if (offer == nullptr) return true;
    if (!normalize_legacy_run_identity(*offer, failure)) return false;
    if (!normalize_legacy_run_client_ids(*offer, failure)) return false;
    if (!normalize_saved_gang_contract_variant(*offer, failure)) return false;
    if (!normalize_legacy_matrix_run_state(*offer, failure)) return false;
    if (!normalize_saved_run_locations(*offer, failure)) return false;
    if (const auto* run_id = [&]() -> const std::string* {
            const auto id = offer->find("id");
            return id == offer->end()
                ? nullptr
                : std::get_if<std::string>(&id->second.value);
        }();
        run_id != nullptr && *run_id == "package_recovery") {
        offer->erase("presentation_variant");
    }

    const auto source_initializer = offer->find("source_initializer");
    const auto source_offer_type = offer->find("source_offer_type");
    const bool has_legacy_selector = source_initializer != offer->end() ||
                                     source_offer_type != offer->end();
    std::optional<std::int64_t> selector;
    for (const auto* field : {&source_initializer, &source_offer_type}) {
        if (*field == offer->end()) continue;
        const auto* integer =
            std::get_if<std::int64_t>(&(*field)->second.value);
        if (integer == nullptr ||
            (selector.has_value() && *selector != *integer)) {
            failure = "saved offer has inconsistent legacy source selectors";
            return false;
        }
        selector = *integer;
    }

    const auto variant_field = offer->find("offer_variant_id");
    if (variant_field != offer->end()) {
        const auto* variant =
            std::get_if<std::string>(&variant_field->second.value);
        if (variant == nullptr || variant->empty()) {
            failure = "saved offer has an invalid semantic variant";
            return false;
        }
        if (selector.has_value()) {
            const auto decoded = legacy_run_offer_variant_id(*selector);
            if (!decoded.has_value() || *decoded != *variant) {
                failure = "saved offer variant conflicts with legacy source data";
                return false;
            }
        }
    } else if (has_legacy_selector) {
        if (!selector.has_value()) {
            failure = "saved offer has no usable legacy source selector";
            return false;
        }
        const auto decoded = legacy_run_offer_variant_id(*selector);
        if (!decoded.has_value()) {
            failure = "saved offer source selector is outside the imported table";
            return false;
        }
        ScriptPersistentValue variant{};
        variant.value = std::string(*decoded);
        offer->emplace("offer_variant_id", std::move(variant));
    }

    offer->erase("source_initializer");
    offer->erase("source_offer_type");
    return true;
}

bool normalize_saved_run_offer_variants(
    const ScriptPersistentValue& saved_state,
    ScriptPersistentValue& normalized, std::string& failure) {
    normalized = saved_state;
    auto* registry =
        std::get_if<ScriptPersistentValue::Object>(&normalized.value);
    if (registry == nullptr) return true;
    for (const auto field_name : {"current_offer", "active_offer",
                                  "last_completed_offer"}) {
        const auto offer = registry->find(field_name);
        if (offer == registry->end() ||
            std::holds_alternative<std::monostate>(offer->second.value)) {
            continue;
        }
        if (!normalize_saved_offer_variant(offer->second, failure)) {
            return false;
        }
    }
    return true;
}

// The field host translates its legacy records before these facts reach Lua.
using ActorLifecycleFacts = ScriptActorLifecycleFacts;
using ActorInteractionFacts = ScriptActorInteractionFacts;
using ActorBehaviorFacts = ScriptActorBehaviorFacts;
using ActorRoleFacts = ScriptActorRoleFacts;
using ActorFacts = ScriptActorFacts;

ActorRoleFacts actor_roles_from_source(const std::uint8_t roles) {
    return {
        (roles & 0x01U) != 0U,
        (roles & 0x02U) != 0U,
        (roles & 0x04U) != 0U,
        (roles & 0x08U) != 0U,
        (roles & 0x10U) != 0U,
        (roles & 0x20U) != 0U,
        (roles & 0x40U) != 0U,
    };
}

ActorFacts actor_facts_from_source_flags(
    const std::uint8_t lifecycle, const std::uint8_t interaction,
    const std::uint8_t behavior, const std::uint8_t roles) {
    return {
        { (lifecycle & 0x01U) != 0U, (lifecycle & 0x02U) != 0U,
          (lifecycle & 0x08U) != 0U, (lifecycle & 0x10U) != 0U,
          (lifecycle & 0x20U) != 0U, (lifecycle & 0x40U) != 0U },
        { (interaction & 0x01U) != 0U, (interaction & 0x02U) != 0U,
          (interaction & 0x04U) != 0U, (interaction & 0x08U) != 0U,
          (interaction & 0x10U) != 0U, (interaction & 0x20U) != 0U,
          (interaction & 0x40U) != 0U, (interaction & 0x80U) != 0U,
          (interaction & 0x80U) != 0U },
        { (behavior & 0x01U) != 0U, (behavior & 0x02U) != 0U,
          (behavior & 0x04U) != 0U, (behavior & 0x08U) != 0U,
          (behavior & 0x10U) != 0U, (behavior & 0x20U) != 0U,
          (behavior & 0x40U) != 0U, (behavior & 0x80U) != 0U },
        actor_roles_from_source(roles)
    };
}

template <typename Facts>
void push_named_boolean_group(
    lua_State* state, const Facts& facts,
    const std::initializer_list<std::pair<const char*, bool Facts::*>>& fields) {
    lua_createtable(state, 0, static_cast<int>(fields.size()));
    const auto table_index = lua_absindex(state, -1);
    for (const auto& [name, member] : fields) {
        lua_pushboolean(state, facts.*member);
        lua_setfield(state, table_index, name);
    }
}

void push_actor_facts(lua_State* state, const ActorFacts& facts) {
    lua_createtable(state, 0, 4);
    const auto table_index = lua_absindex(state, -1);
    push_named_boolean_group(state, facts.lifecycle,
        {{"active", &ActorLifecycleFacts::active},
         {"thinking_enabled", &ActorLifecycleFacts::thinking_enabled},
         {"ai_runner", &ActorLifecycleFacts::ai_runner},
         {"downed", &ActorLifecycleFacts::downed},
         {"group_leader", &ActorLifecycleFacts::group_leader},
         {"dead", &ActorLifecycleFacts::dead}});
    lua_setfield(state, table_index, "lifecycle");
    push_named_boolean_group(state, facts.interaction,
        {{"projectile", &ActorInteractionFacts::projectile},
         {"proximity_interaction_pending",
          &ActorInteractionFacts::proximity_interaction_pending},
         {"use_collision_heading",
          &ActorInteractionFacts::use_collision_heading},
         {"collision_recovery_active",
          &ActorInteractionFacts::collision_recovery_active},
         {"action_hook_pending", &ActorInteractionFacts::action_hook_pending},
         {"defense_difficulty_reduction",
          &ActorInteractionFacts::defense_difficulty_reduction},
         {"breathing_out", &ActorInteractionFacts::breathing_out},
         {"unseen", &ActorInteractionFacts::unseen},
         {"no_coverage_difficulty_bonus",
          &ActorInteractionFacts::no_coverage_difficulty_bonus}});
    lua_setfield(state, table_index, "interaction");
    push_named_boolean_group(state, facts.behavior,
        {{"route_clear", &ActorBehaviorFacts::route_clear},
         {"path_active", &ActorBehaviorFacts::path_active},
         {"restart_steering", &ActorBehaviorFacts::restart_steering},
         {"wall_following", &ActorBehaviorFacts::wall_following},
         {"in_target_band", &ActorBehaviorFacts::in_target_band},
         {"route_transition", &ActorBehaviorFacts::route_transition},
         {"fleeing", &ActorBehaviorFacts::fleeing},
         {"escort", &ActorBehaviorFacts::escort}});
    lua_setfield(state, table_index, "behavior");
    push_named_boolean_group(state, facts.roles,
        {{"spotted", &ActorRoleFacts::spotted},
         {"reinforcement", &ActorRoleFacts::reinforcement},
         {"despawn_offscreen", &ActorRoleFacts::despawn_offscreen},
         {"quest", &ActorRoleFacts::quest}, {"runner", &ActorRoleFacts::runner},
         {"pedestrian", &ActorRoleFacts::pedestrian},
         {"hostile", &ActorRoleFacts::hostile}});
    lua_setfield(state, table_index, "roles");
}

void push_actor_behavior_facts(lua_State* state,
                               const std::uint8_t behavior) {
    const ActorBehaviorFacts facts{
        (behavior & 0x01U) != 0U, (behavior & 0x02U) != 0U,
        (behavior & 0x04U) != 0U, (behavior & 0x08U) != 0U,
        (behavior & 0x10U) != 0U, (behavior & 0x20U) != 0U,
        (behavior & 0x40U) != 0U, (behavior & 0x80U) != 0U};
    push_named_boolean_group(state, facts,
        {{"route_clear", &ActorBehaviorFacts::route_clear},
         {"path_active", &ActorBehaviorFacts::path_active},
         {"restart_steering", &ActorBehaviorFacts::restart_steering},
         {"wall_following", &ActorBehaviorFacts::wall_following},
         {"in_target_band", &ActorBehaviorFacts::in_target_band},
         {"route_transition", &ActorBehaviorFacts::route_transition},
         {"fleeing", &ActorBehaviorFacts::fleeing},
         {"escort", &ActorBehaviorFacts::escort}});
}

bool read_actor_behavior_facts(lua_State* state, const int table_index,
                               const char* field_name,
                               ActorBehaviorFacts& facts) {
    lua_getfield(state, table_index, field_name);
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto facts_index = lua_absindex(state, -1);
    bool valid = true;
    for (const auto& [name, member] : {
             std::pair<const char*, bool ActorBehaviorFacts::*> {
                 "route_clear", &ActorBehaviorFacts::route_clear},
             {"path_active", &ActorBehaviorFacts::path_active},
             {"restart_steering", &ActorBehaviorFacts::restart_steering},
             {"wall_following", &ActorBehaviorFacts::wall_following},
             {"in_target_band", &ActorBehaviorFacts::in_target_band},
             {"route_transition", &ActorBehaviorFacts::route_transition},
             {"fleeing", &ActorBehaviorFacts::fleeing},
             {"escort", &ActorBehaviorFacts::escort}}) {
        lua_getfield(state, facts_index, name);
        if (!lua_isboolean(state, -1)) {
            lua_pop(state, 1);
            valid = false;
            break;
        }
        facts.*member = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    return valid;
}

template <typename Facts>
bool read_named_boolean_group(
    lua_State* state, const int table_index, const char* group_name,
    Facts& facts,
    const std::initializer_list<std::pair<const char*, bool Facts::*>>& fields) {
    lua_getfield(state, table_index, group_name);
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto group_index = lua_absindex(state, -1);
    for (const auto& [name, member] : fields) {
        lua_getfield(state, group_index, name);
        if (!lua_isboolean(state, -1)) {
            lua_pop(state, 2);
            return false;
        }
        facts.*member = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    return true;
}

bool read_actor_facts(lua_State* state, const int table_index,
                      ActorFacts& facts) {
    lua_getfield(state, table_index, "actor_facts");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto facts_index = lua_absindex(state, -1);
    const bool valid =
        read_named_boolean_group(state, facts_index, "lifecycle", facts.lifecycle,
            {{"active", &ActorLifecycleFacts::active},
             {"thinking_enabled", &ActorLifecycleFacts::thinking_enabled},
             {"ai_runner", &ActorLifecycleFacts::ai_runner},
             {"downed", &ActorLifecycleFacts::downed},
             {"group_leader", &ActorLifecycleFacts::group_leader},
             {"dead", &ActorLifecycleFacts::dead}}) &&
        read_named_boolean_group(state, facts_index, "interaction", facts.interaction,
            {{"projectile", &ActorInteractionFacts::projectile},
             {"proximity_interaction_pending",
              &ActorInteractionFacts::proximity_interaction_pending},
             {"use_collision_heading",
              &ActorInteractionFacts::use_collision_heading},
             {"collision_recovery_active",
              &ActorInteractionFacts::collision_recovery_active},
             {"action_hook_pending",
              &ActorInteractionFacts::action_hook_pending},
             {"defense_difficulty_reduction",
              &ActorInteractionFacts::defense_difficulty_reduction},
             {"breathing_out", &ActorInteractionFacts::breathing_out},
             {"unseen", &ActorInteractionFacts::unseen},
             {"no_coverage_difficulty_bonus",
              &ActorInteractionFacts::no_coverage_difficulty_bonus}}) &&
        read_named_boolean_group(state, facts_index, "behavior", facts.behavior,
            {{"route_clear", &ActorBehaviorFacts::route_clear},
             {"path_active", &ActorBehaviorFacts::path_active},
             {"restart_steering", &ActorBehaviorFacts::restart_steering},
             {"wall_following", &ActorBehaviorFacts::wall_following},
             {"in_target_band", &ActorBehaviorFacts::in_target_band},
             {"route_transition", &ActorBehaviorFacts::route_transition},
             {"fleeing", &ActorBehaviorFacts::fleeing},
             {"escort", &ActorBehaviorFacts::escort}}) &&
        read_named_boolean_group(state, facts_index, "roles", facts.roles,
            {{"spotted", &ActorRoleFacts::spotted},
             {"reinforcement", &ActorRoleFacts::reinforcement},
             {"despawn_offscreen", &ActorRoleFacts::despawn_offscreen},
             {"quest", &ActorRoleFacts::quest}, {"runner", &ActorRoleFacts::runner},
             {"pedestrian", &ActorRoleFacts::pedestrian},
             {"hostile", &ActorRoleFacts::hostile}});
    lua_pop(state, 1);
    return valid;
}

bool read_actor_role_facts(lua_State* state, const int table_index,
                           ActorRoleFacts& facts) {
    lua_getfield(state, table_index, "actor_facts");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto facts_index = lua_absindex(state, -1);
    const bool valid = read_named_boolean_group(
        state, facts_index, "roles", facts,
        {{"spotted", &ActorRoleFacts::spotted},
         {"reinforcement", &ActorRoleFacts::reinforcement},
         {"despawn_offscreen", &ActorRoleFacts::despawn_offscreen},
         {"quest", &ActorRoleFacts::quest}, {"runner", &ActorRoleFacts::runner},
         {"pedestrian", &ActorRoleFacts::pedestrian},
         {"hostile", &ActorRoleFacts::hostile}});
    lua_pop(state, 1);
    return valid;
}

bool read_actor_interaction_facts(lua_State* state, const int table_index,
                                  ActorInteractionFacts& facts) {
    lua_getfield(state, table_index, "actor_facts");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto facts_index = lua_absindex(state, -1);
    const bool valid = read_named_boolean_group(
        state, facts_index, "interaction", facts,
        {{"projectile", &ActorInteractionFacts::projectile},
         {"proximity_interaction_pending",
          &ActorInteractionFacts::proximity_interaction_pending},
         {"use_collision_heading",
          &ActorInteractionFacts::use_collision_heading},
         {"collision_recovery_active",
          &ActorInteractionFacts::collision_recovery_active},
         {"action_hook_pending", &ActorInteractionFacts::action_hook_pending},
         {"defense_difficulty_reduction",
          &ActorInteractionFacts::defense_difficulty_reduction},
         {"breathing_out", &ActorInteractionFacts::breathing_out},
         {"unseen", &ActorInteractionFacts::unseen},
         {"no_coverage_difficulty_bonus",
          &ActorInteractionFacts::no_coverage_difficulty_bonus}});
    lua_pop(state, 1);
    return valid;
}

template <typename Facts>
std::uint8_t merge_known_actor_bits(
    const std::uint8_t original, const std::uint8_t known_mask,
    const Facts& facts,
    const std::initializer_list<std::pair<bool Facts::*, std::uint8_t>>& fields) {
    auto result = static_cast<std::uint8_t>(original &
        static_cast<std::uint8_t>(~known_mask));
    for (const auto& [member, bit] : fields) {
        if (facts.*member) {
            result = static_cast<std::uint8_t>(result | bit);
        }
    }
    return result;
}

std::uint8_t lifecycle_to_source(const ActorLifecycleFacts& facts,
                                const std::uint8_t original) {
    return merge_known_actor_bits(original, 0x7BU, facts,
        {{&ActorLifecycleFacts::active, 0x01U},
         {&ActorLifecycleFacts::thinking_enabled, 0x02U},
         {&ActorLifecycleFacts::ai_runner, 0x08U},
         {&ActorLifecycleFacts::downed, 0x10U},
         {&ActorLifecycleFacts::group_leader, 0x20U},
         {&ActorLifecycleFacts::dead, 0x40U}});
}

std::uint8_t behavior_to_source(const ActorBehaviorFacts& facts,
                                const std::uint8_t original) {
    return merge_known_actor_bits(original, 0xFFU, facts,
        {{&ActorBehaviorFacts::route_clear, 0x01U},
         {&ActorBehaviorFacts::path_active, 0x02U},
         {&ActorBehaviorFacts::restart_steering, 0x04U},
         {&ActorBehaviorFacts::wall_following, 0x08U},
         {&ActorBehaviorFacts::in_target_band, 0x10U},
         {&ActorBehaviorFacts::route_transition, 0x20U},
         {&ActorBehaviorFacts::fleeing, 0x40U},
         {&ActorBehaviorFacts::escort, 0x80U}});
}

std::uint8_t roles_to_source(const ActorRoleFacts& facts,
                             const std::uint8_t original) {
    return merge_known_actor_bits(original, 0x7FU, facts,
        {{&ActorRoleFacts::spotted, 0x01U},
         {&ActorRoleFacts::reinforcement, 0x02U},
         {&ActorRoleFacts::despawn_offscreen, 0x04U},
         {&ActorRoleFacts::quest, 0x08U}, {&ActorRoleFacts::runner, 0x10U},
         {&ActorRoleFacts::pedestrian, 0x20U},
         {&ActorRoleFacts::hostile, 0x40U}});
}

std::uint8_t interaction_to_source(const ActorInteractionFacts& facts) {
    std::uint8_t result = 0U;
    const auto set = [&result](const bool enabled, const std::uint8_t bit) {
        if (enabled) {
            result = static_cast<std::uint8_t>(result | bit);
        }
    };
    set(facts.projectile, 0x01U);
    set(facts.proximity_interaction_pending, 0x02U);
    set(facts.use_collision_heading, 0x04U);
    set(facts.collision_recovery_active, 0x08U);
    set(facts.action_hook_pending, 0x10U);
    set(facts.defense_difficulty_reduction, 0x20U);
    set(facts.breathing_out, 0x40U);
    set(facts.unseen || facts.no_coverage_difficulty_bonus, 0x80U);
    return result;
}

constexpr std::size_t kMaximumScriptModules = 128U;
constexpr std::size_t kMaximumMapEventRules = 512U;
// The command overview has seven source rows; Pocket Secretary's extended
// caller can expose six. Keep the generic script-menu bound at the larger
// source-defined menu size.
constexpr std::size_t kMaximumMenuChoices = 8U;
// Static script catalogs may describe source tables such as all 13 Magic
// spells or 32 shop headings; live dialogue menus keep the seven-row cap.
constexpr std::size_t kMaximumMenuCatalogChoices = 32U;
constexpr std::size_t kMaximumStringMapEntries = 128U;
constexpr std::size_t kMaximumShopGroups = 8U;
constexpr std::size_t kMaximumShopOffers = 8U;
constexpr std::size_t kMaximumItemCatalogEntries = 50U;
constexpr std::size_t kMaximumItemDetailTextEntries = 32U;
constexpr std::size_t kMaximumIdentifierLength = 96U;
constexpr std::size_t kMaximumStoryFacts = 4096U;
// The direct ROM text catalog already exceeds the old 1024-entry ceiling as
// more location and menu records are imported. Keep the parser bound above
// the complete host asset catalog instead of turning valid extracted text
// into a gameplay bootstrap failure.
constexpr std::size_t kMaximumDialogueTextEntries = 4096U;
constexpr std::size_t kMaximumDialogueTextBytes = 8192U;
constexpr std::size_t kMaximumDialogueJsonBytes = 4U * 1024U * 1024U;
constexpr std::size_t kMaximumGameplayJsonDocuments = 256U;
constexpr std::size_t kMaximumGameplayJsonBytes = 16U * 1024U * 1024U;
constexpr int kScriptInstructionBudget = 100000;
constexpr int kDialogueJsonInstructionBudget = 20000000;

std::string stack_error(lua_State* state) {
    const auto* message = lua_tostring(state, -1);
    return message == nullptr ? "Lua reported a non-string error"
                              : std::string(message);
}

void instruction_budget_hook(lua_State* state, lua_Debug*) {
    luaL_error(state, "gameplay script exceeded its instruction budget");
}

class ScopedInstructionBudget final {
public:
    explicit ScopedInstructionBudget(
        lua_State* state,
        const int instruction_budget = kScriptInstructionBudget)
        : state_(state), previous_hook_(lua_gethook(state)),
          previous_mask_(lua_gethookmask(state)),
          previous_count_(lua_gethookcount(state)) {
        lua_sethook(state_, instruction_budget_hook, LUA_MASKCOUNT,
                    instruction_budget);
    }
    ~ScopedInstructionBudget() {
        lua_sethook(state_, previous_hook_, previous_mask_, previous_count_);
    }

    ScopedInstructionBudget(const ScopedInstructionBudget&) = delete;
    ScopedInstructionBudget& operator=(const ScopedInstructionBudget&) = delete;

private:
    lua_State* state_;
    lua_Hook previous_hook_;
    int previous_mask_;
    int previous_count_;
};

bool read_string(lua_State* state, const int index, std::string& value) {
    if (lua_type(state, index) != LUA_TSTRING) {
        return false;
    }
    std::size_t length = 0U;
    const auto* text = lua_tolstring(state, index, &length);
    if (text == nullptr || length == 0U ||
        length > kMaximumIdentifierLength) {
        return false;
    }
    value.assign(text, length);
    return true;
}

bool read_string_allow_empty(lua_State* state, const int index,
                             std::string& value) {
    if (lua_type(state, index) != LUA_TSTRING) {
        return false;
    }
    std::size_t length = 0U;
    const auto* text = lua_tolstring(state, index, &length);
    if (text == nullptr || length > kMaximumIdentifierLength) {
        return false;
    }
    value.assign(text, length);
    return true;
}

bool read_integer(lua_State* state, const int index,
                  const std::uint32_t maximum, std::uint32_t& value) {
    if (!lua_isinteger(state, index)) {
        return false;
    }
    const auto number = lua_tointeger(state, index);
    if (number < 0 ||
        static_cast<lua_Unsigned>(number) > maximum) {
        return false;
    }
    value = static_cast<std::uint32_t>(number);
    return true;
}

bool read_field_string(lua_State* state, const int table_index,
                       const char* field, std::string& value) {
    lua_getfield(state, table_index, field);
    const bool valid = read_string(state, -1, value);
    lua_pop(state, 1);
    return valid;
}

bool read_field_string_allow_empty(lua_State* state, const int table_index,
                                   const char* field, std::string& value) {
    lua_getfield(state, table_index, field);
    const bool valid = read_string_allow_empty(state, -1, value);
    lua_pop(state, 1);
    return valid;
}

bool read_field_text(lua_State* state, const int table_index,
                     const char* field, std::string& value,
                     const std::size_t maximum_length =
                         kMaximumDialogueTextBytes) {
    lua_getfield(state, table_index, field);
    if (lua_type(state, -1) != LUA_TSTRING) {
        lua_pop(state, 1);
        return false;
    }
    std::size_t length = 0U;
    const auto* text = lua_tolstring(state, -1, &length);
    const bool valid = text != nullptr && length <= maximum_length;
    if (valid) {
        value.assign(text, length);
    }
    lua_pop(state, 1);
    return valid;
}

bool read_optional_field_string(lua_State* state, const int table_index,
                                const char* field, std::string& value) {
    lua_getfield(state, table_index, field);
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        return true;
    }
    const bool valid = read_string(state, -1, value);
    lua_pop(state, 1);
    return valid;
}

std::optional<menus::CommandSection> command_section_from_id(
    const std::string_view section) noexcept {
    using menus::CommandSection;
    if (section == "inventory_left") return CommandSection::inventory_left;
    if (section == "inventory_right") return CommandSection::inventory_right;
    if (section == "clips") return CommandSection::clips;
    if (section == "attributes_and_skills") {
        return CommandSection::attributes_and_skills;
    }
    if (section == "cyberware") return CommandSection::cyberware;
    if (section == "magic") return CommandSection::magic;
    if (section == "pocket_secretary") {
        return CommandSection::pocket_secretary;
    }
    return std::nullopt;
}

std::optional<menus::CommandSection> read_optional_command_section(
    lua_State* state, const int table_index, const char* field) {
    std::string section;
    if (!read_optional_field_string(state, table_index, field, section) ||
        section.empty()) {
        return std::nullopt;
    }
    return command_section_from_id(section);
}

bool read_field_integer(lua_State* state, const int table_index,
                        const char* field, const std::uint32_t maximum,
                        std::uint32_t& value) {
    lua_getfield(state, table_index, field);
    const bool valid = read_integer(state, -1, maximum, value);
    lua_pop(state, 1);
    return valid;
}

void push_inventory_cell(lua_State* state, const int table_index,
                         const char* field, const std::size_t position) {
    lua_createtable(state, 0, 2);
    const auto cell_index = lua_absindex(state, -1);
    lua_pushinteger(state, static_cast<lua_Integer>(position % 2U));
    lua_setfield(state, cell_index, "column");
    lua_pushinteger(state, static_cast<lua_Integer>(position / 2U));
    lua_setfield(state, cell_index, "row");
    lua_setfield(state, table_index, field);
}

bool read_inventory_cell(lua_State* state, const int table_index,
                         const char* field, std::uint32_t& column,
                         std::uint32_t& row) {
    lua_getfield(state, table_index, field);
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto cell_index = lua_absindex(state, -1);
    const bool valid = read_field_integer(state, cell_index, "column", 1U,
                                          column) &&
                       read_field_integer(state, cell_index, "row", 3U, row);
    lua_pop(state, 1);
    return valid;
}

bool read_field_number(lua_State* state, const int table_index,
                       const char* field, double& value) {
    lua_getfield(state, table_index, field);
    const bool valid = read_number(state, -1, value);
    lua_pop(state, 1);
    return valid;
}

bool read_source_posture_from_lua_percent(
    lua_State* state, const int table_index, const char* field,
    std::uint8_t& source_posture) {
    std::uint32_t reduction_percent = 0U;
    if (!read_field_integer(state, table_index, field, 6375U,
                            reduction_percent)) {
        return false;
    }
    const auto parsed = script_source_posture_from_reduction_percent(
        static_cast<std::uint16_t>(reduction_percent));
    if (!parsed.has_value()) {
        return false;
    }
    source_posture = *parsed;
    return true;
}

bool read_optional_field_u32(lua_State* state, const int table_index,
                             const char* field, const std::uint32_t maximum,
                             std::optional<std::uint32_t>& value) {
    lua_getfield(state, table_index, field);
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        value.reset();
        return true;
    }
    std::uint32_t parsed = 0U;
    const bool valid = read_integer(state, -1, maximum, parsed);
    lua_pop(state, 1);
    if (valid) {
        value = parsed;
    }
    return valid;
}

bool read_optional_field_integer(
    lua_State* state, const int table_index, const char* field,
    const std::uint32_t maximum,
    std::optional<std::uint8_t>& value) {
    lua_getfield(state, table_index, field);
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        value.reset();
        return true;
    }
    std::uint32_t parsed = 0U;
    const bool valid = read_integer(state, -1, maximum, parsed);
    lua_pop(state, 1);
    if (valid) {
        value = static_cast<std::uint8_t>(parsed);
    }
    return valid;
}

bool push_combat_inventory(
    lua_State* state, const std::array<std::uint8_t, 8>& item_codes,
    const std::array<std::uint8_t, 8>& item_values) {
    lua_createtable(state, static_cast<int>(item_codes.size()), 0);
    const auto inventory_index = lua_absindex(state, -1);
    lua_Integer inventory_entry = 1;
    for (std::size_t index = 0U; index < item_codes.size(); ++index) {
        if (item_codes[index] == 0U) {
            continue;
        }
        const auto item = script_combat_inventory_item(
            static_cast<std::uint8_t>(index + 1U), item_codes[index],
            item_values[index]);
        if (!item.has_value()) {
            lua_pop(state, 1);
            return false;
        }
        lua_createtable(state, 0, item->value_property.empty() ? 2 : 3);
        const auto item_index = lua_absindex(state, -1);
        lua_pushlstring(state, item->item_id.data(), item->item_id.size());
        lua_setfield(state, item_index, "id");
        lua_pushinteger(state, item->slot);
        lua_setfield(state, item_index, "slot");
        if (!item->value_property.empty()) {
            lua_pushinteger(state, item->value);
            lua_setfield(state, item_index,
                         std::string(item->value_property).c_str());
        }
        lua_rawseti(state, inventory_index, inventory_entry++);
    }
    return true;
}

bool push_menu_equipment(
    lua_State* state, const std::array<std::uint8_t, 8>& item_codes,
    const std::array<std::uint8_t, 8>& item_values) {
    lua_createtable(state, static_cast<int>(item_codes.size()), 0);
    const auto inventory_index = lua_absindex(state, -1);
    lua_Integer inventory_entry = 1;
    for (std::size_t index = 0U; index < item_codes.size(); ++index) {
        if (item_codes[index] == 0U) {
            continue;
        }
        const auto item = script_combat_inventory_item(
            static_cast<std::uint8_t>(index + 1U), item_codes[index],
            item_values[index]);
        if (!item.has_value()) {
            lua_pop(state, 1);
            return false;
        }
        lua_createtable(state, 0, item->value_property.empty() ? 2 : 3);
        const auto item_index = lua_absindex(state, -1);
        lua_pushlstring(state, item->item_id.data(), item->item_id.size());
        lua_setfield(state, item_index, "id");
        push_inventory_cell(state, item_index, "cell", index);
        // Keep the cell marker semantic in Lua and convert its armor ID back
        // to the source phase only at the typed host boundary.
        if (script_armor_source_index_from_item_id(item->item_id)
                .has_value()) {
            const auto armor_marker_id =
                script_armor_item_id_from_source_index(item_values[index]);
            if (item_values[index] != 0U && !armor_marker_id.has_value()) {
                lua_pop(state, 2);
                return false;
            }
            if (armor_marker_id.has_value()) {
                lua_pushlstring(state, armor_marker_id->data(),
                                armor_marker_id->size());
            } else {
                lua_pushnil(state);
            }
            lua_setfield(state, item_index, "armor_marker_id");
        }
        if (!item->value_property.empty()) {
            lua_pushinteger(state, item->value);
            lua_setfield(state, item_index,
                         std::string(item->value_property).c_str());
        }
        lua_rawseti(state, inventory_index, inventory_entry++);
    }
    return true;
}

bool read_menu_equipment(
    lua_State* state, const int combatant_index,
    std::array<std::uint8_t, 8>& item_codes,
    std::array<std::uint8_t, 8>& item_values,
    const std::array<std::uint8_t, 8>& original_item_codes,
    const std::array<std::uint8_t, 8>& original_item_values) {
    lua_getfield(state, combatant_index, "equipment");
    if (!lua_istable(state, -1) || lua_rawlen(state, -1) > item_codes.size()) {
        lua_pop(state, 1);
        return false;
    }
    item_codes.fill(0U);
    item_values = original_item_values;
    std::array<bool, 8> used_slots{};
    const auto inventory_index = lua_absindex(state, -1);
    const auto inventory_size = lua_rawlen(state, inventory_index);
    for (std::size_t index = 0U; index < inventory_size; ++index) {
        lua_rawgeti(state, inventory_index,
                    static_cast<lua_Integer>(index + 1U));
        if (!lua_istable(state, -1)) {
            lua_pop(state, 2);
            return false;
        }
        const auto item_index = lua_absindex(state, -1);
        std::string item_id;
        std::uint32_t column = 0U;
        std::uint32_t row = 0U;
        if (!read_field_string(state, item_index, "id", item_id) ||
            !read_inventory_cell(state, item_index, "cell", column, row)) {
            lua_pop(state, 2);
            return false;
        }
        const auto position = row * 2U + column;
        if (position >= used_slots.size() || used_slots[position]) {
            lua_pop(state, 2);
            return false;
        }
        const auto source_code = script_item_source_code_from_id(item_id);
        if (!source_code.has_value()) {
            lua_pop(state, 2);
            return false;
        }
        const auto item = script_combat_inventory_item(
            static_cast<std::uint8_t>(position + 1U), *source_code, 0U);
        if (!item.has_value()) {
            lua_pop(state, 2);
            return false;
        }
        item_codes[position] = *source_code;
        used_slots[position] = true;
        if (!item->value_property.empty()) {
            std::uint32_t value = 0U;
            if (!read_field_integer(state, item_index,
                                    std::string(item->value_property).c_str(),
                                    0xFFU, value)) {
                lua_pop(state, 2);
                return false;
            }
            item_values[position] = static_cast<std::uint8_t>(value);
        } else if (original_item_codes[position] != *source_code) {
            item_values[position] = 0U;
        }
        // ROM 0x121A8/0x121B8 changes only the selected inventory cell's
        // +0x6E marker; the Lua item carries that marker as an armor ID [C].
        if (script_armor_source_index_from_item_id(item_id).has_value()) {
            lua_getfield(state, item_index, "armor_marker_id");
            const auto marker_type = lua_type(state, -1);
            if (marker_type == LUA_TNIL) {
                item_values[position] = 0U;
            } else if (marker_type == LUA_TSTRING) {
                std::string marker_id;
                const auto valid_marker =
                    read_string(state, -1, marker_id);
                const auto marker_index = valid_marker
                    ? script_armor_source_index_from_item_id(marker_id)
                    : std::nullopt;
                if (!marker_index.has_value()) {
                    lua_pop(state, 3);
                    return false;
                }
                item_values[position] = *marker_index;
            } else {
                lua_pop(state, 3);
                return false;
            }
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
    }
    for (std::size_t slot = 0U; slot < item_codes.size(); ++slot) {
        if (item_codes[slot] == 0U) {
            item_values[slot] = 0U;
        }
    }
    lua_pop(state, 1);
    return true;
}

bool read_combat_inventory(
    lua_State* state, const int combatant_index,
    std::array<std::uint8_t, 8>& item_codes,
    std::array<std::uint8_t, 8>& item_values) {
    lua_getfield(state, combatant_index, "inventory");
    if (!lua_istable(state, -1) || lua_rawlen(state, -1) > item_codes.size()) {
        lua_pop(state, 1);
        return false;
    }
    item_codes.fill(0U);
    item_values.fill(0U);
    std::array<bool, 8> used_slots{};
    const auto inventory_index = lua_absindex(state, -1);
    const auto inventory_size = lua_rawlen(state, inventory_index);
    for (std::size_t index = 0U; index < inventory_size; ++index) {
        lua_rawgeti(state, inventory_index,
                    static_cast<lua_Integer>(index + 1U));
        if (!lua_istable(state, -1)) {
            lua_pop(state, 2);
            return false;
        }
        const auto item_index = lua_absindex(state, -1);
        std::string item_id;
        std::uint32_t slot = 0U;
        if (!read_field_string(state, item_index, "id", item_id) ||
            !read_field_integer(state, item_index, "slot", 8U, slot) ||
            slot == 0U || used_slots[slot - 1U]) {
            lua_pop(state, 2);
            return false;
        }
        const auto source_code = script_item_source_code_from_id(item_id);
        if (!source_code.has_value()) {
            lua_pop(state, 2);
            return false;
        }
        const auto item = script_combat_inventory_item(
            static_cast<std::uint8_t>(slot), *source_code, 0U);
        if (!item.has_value()) {
            lua_pop(state, 2);
            return false;
        }
        item_codes[slot - 1U] = *source_code;
        used_slots[slot - 1U] = true;
        if (!item->value_property.empty()) {
            std::uint32_t value = 0U;
            if (!read_field_integer(state, item_index,
                                    std::string(item->value_property).c_str(),
                                    0xFFU, value)) {
                lua_pop(state, 2);
                return false;
            }
            item_values[slot - 1U] = static_cast<std::uint8_t>(value);
        }
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    return true;
}

bool read_field_boolean(lua_State* state, const int table_index,
                        const char* field, bool& value) {
    lua_getfield(state, table_index, field);
    const bool valid = lua_isboolean(state, -1) != 0;
    if (valid) {
        value = lua_toboolean(state, -1) != 0;
    }
    lua_pop(state, 1);
    return valid;
}

bool read_combat_cyberware(lua_State* state, const int combatant_index,
                           ScriptCombatCyberware& cyberware) {
    lua_getfield(state, combatant_index, "cyberware");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto cyberware_index = lua_absindex(state, -1);
    std::uint32_t melee_strength_bonus = 0U;
    std::uint32_t melee_defense_bonus = 0U;
    const bool valid =
        read_field_boolean(state, cyberware_index, "cybereyes",
                           cyberware.cybereyes) &&
        read_field_boolean(state, cyberware_index, "hand_razors",
                           cyberware.hand_razors) &&
        read_field_boolean(state, cyberware_index, "spurs",
                           cyberware.spurs) &&
        read_field_boolean(state, cyberware_index, "smartlink",
                           cyberware.smartlink) &&
        read_field_integer(state, cyberware_index, "melee_strength_bonus",
                           4U, melee_strength_bonus) &&
        read_field_integer(state, cyberware_index, "melee_defense_bonus",
                           3U, melee_defense_bonus);
    lua_pop(state, 1);
    if (!valid) {
        return false;
    }
    cyberware.melee_strength_bonus =
        static_cast<std::uint8_t>(melee_strength_bonus);
    cyberware.melee_defense_bonus =
        static_cast<std::uint8_t>(melee_defense_bonus);
    return true;
}

bool read_combat_traits(lua_State* state, const int combatant_index,
                        ScriptCombatTraits& traits) {
    lua_getfield(state, combatant_index, "combat_traits");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto traits_index = lua_absindex(state, -1);
    const bool valid =
        read_field_boolean(state, traits_index,
                           "ignores_cover_condition_penalty",
                           traits.ignores_cover_condition_penalty) &&
        read_field_boolean(state, traits_index,
                           "adds_two_condition_difficulty",
                           traits.adds_two_condition_difficulty) &&
        read_field_boolean(state, traits_index, "is_special_spell_target",
                           traits.is_special_spell_target) &&
        read_field_boolean(state, traits_index, "takes_fixed_meter_damage",
                           traits.takes_fixed_meter_damage) &&
        read_field_boolean(state, traits_index,
                           "qualifies_for_party_support_bonus",
                           traits.qualifies_for_party_support_bonus) &&
        read_field_boolean(state, traits_index, "is_ghoul_bounty_target",
                           traits.is_ghoul_bounty_target);
    lua_pop(state, 1);
    return valid;
}

bool read_combat_action_profile_options(
    lua_State* state, const int combatant_index,
    ScriptCombatActionProfileOptions& options) {
    lua_getfield(state, combatant_index, "action_profile_options");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto options_index = lua_absindex(state, -1);
    const bool valid =
        read_field_boolean(state, options_index, "primary", options.primary) &&
        read_field_boolean(state, options_index, "secondary",
                           options.secondary);
    lua_pop(state, 1);
    return valid;
}

bool read_field_signed_integer(lua_State* state, const int table_index,
                               const char* field, const int minimum,
                               const int maximum, int& value) {
    lua_getfield(state, table_index, field);
    const bool valid = lua_isinteger(state, -1) != 0 &&
                       lua_tointeger(state, -1) >= minimum &&
                       lua_tointeger(state, -1) <= maximum;
    if (valid) {
        value = static_cast<int>(lua_tointeger(state, -1));
    }
    lua_pop(state, 1);
    return valid;
}

bool read_weapon_action_policies(
    lua_State* state, const int combatant_index,
    std::array<ScriptWeaponActionPolicy, 12>& policies) {
    lua_getfield(state, combatant_index, "weapon_action_policies");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto table_index = lua_absindex(state, -1);
    bool valid = true;
    for (std::size_t index = 0U; index < policies.size(); ++index) {
        const auto action_id = kScriptWeaponActionIds[index];
        const auto action_name = script_weapon_action_name(action_id);
        lua_getfield(state, table_index, action_name.data());
        if (!lua_istable(state, -1)) {
            lua_pop(state, 1);
            valid = false;
            break;
        }
        const auto action_index = lua_absindex(state, -1);
        std::string skill_group;
        int difficulty_adjustment = 0;
        bool targeting_assist = false;
        valid = read_field_string(state, action_index, "skill_group",
                                  skill_group) &&
            skill_group == script_weapon_skill_group_name(
                               script_weapon_skill_group(action_id)) &&
            read_field_signed_integer(state, action_index,
                                      "difficulty_adjustment", -3, 3,
                                      difficulty_adjustment) &&
            read_field_boolean(state, action_index,
                               "benefits_from_targeting_assist",
                               targeting_assist);
        lua_pop(state, 1);
        if (!valid) {
            break;
        }
        policies[index] = ScriptWeaponActionPolicy{
            script_weapon_skill_group(action_id),
            static_cast<std::int8_t>(difficulty_adjustment),
            targeting_assist};
    }
    lua_pop(state, 1);
    return valid;
}

bool valid_identifier(std::string_view value);

bool read_shop_menu_snapshot(lua_State* state, const int table_index,
                             ScriptShopMenuSnapshot& snapshot) {
    std::uint32_t menu_mode = 0U;
    std::uint32_t action_cursor = 0U;
    std::uint32_t action_panel = 0U;
    std::uint32_t selected_action = 0U;
    int menu_result = 0;
    if (!read_field_boolean(state, table_index, "open", snapshot.open) ||
        !read_field_string(state, table_index, "shop_id", snapshot.shop_id) ||
        !valid_identifier(snapshot.shop_id) ||
        !read_field_integer(state, table_index, "menu_mode", 0xFFU,
                            menu_mode) ||
        !read_field_signed_integer(state, table_index, "menu_result",
                                   -0x80, 0x7F, menu_result) ||
        !read_field_integer(state, table_index, "action_cursor", 7U,
                            action_cursor) ||
        !read_field_integer(state, table_index, "action_panel", 1U,
                            action_panel) ||
        !read_field_integer(state, table_index, "selected_action", 7U,
                            selected_action) ||
        !read_field_string(state, table_index, "event", snapshot.event) ||
        !valid_identifier(snapshot.event)) {
        return false;
    }
    snapshot.menu_mode = static_cast<std::uint8_t>(menu_mode);
    snapshot.menu_result = static_cast<std::int8_t>(menu_result);
    snapshot.action_cursor = static_cast<std::uint8_t>(action_cursor);
    snapshot.action_panel = static_cast<std::uint8_t>(action_panel);
    snapshot.selected_action = static_cast<std::uint8_t>(selected_action);
    return true;
}

std::string collision_event_name(const FieldCollisionEventCode event_code) {
    if (event_code == FieldCollisionEventCode::no_event) {
        return "no_event";
    }
    return "event_" + std::to_string(
                           field_collision_event_value(event_code));
}

// [C] ROM 0xB7B8 selects behavior from the collision identity. This adapter
// reduces that source identity to the gameplay concept Lua needs; the numeric
// event never crosses the scripting boundary.
std::string_view field_collision_kind_name(
    const FieldCollisionEventCode event_code,
    const bool lockable_door_context) noexcept {
    switch (event_code) {
    case FieldCollisionEventCode::event_0:
        return "empty_world_interaction";
    case FieldCollisionEventCode::event_2:
    case FieldCollisionEventCode::event_3:
    case FieldCollisionEventCode::event_4:
    case FieldCollisionEventCode::event_5:
        return lockable_door_context ? "lockable_door"
                                     : "world_interaction";
    case FieldCollisionEventCode::event_24:
        return "security_terminal";
    case FieldCollisionEventCode::event_25:
        return "contextual_activity";
    case FieldCollisionEventCode::event_26:
        return "terrain_marker";
    case FieldCollisionEventCode::event_27:
        return "world_activity";
    case FieldCollisionEventCode::event_28:
    case FieldCollisionEventCode::event_29:
        return "activity_transition";
    case FieldCollisionEventCode::event_30:
        return "profiled_contact";
    case FieldCollisionEventCode::event_31:
        return "field_activity";
    default:
        return "world_interaction";
    }
}

bool parse_collision_event_name(const std::string_view name,
                                FieldCollisionEventCode& event_code) {
    if (name == "no_event") {
        event_code = FieldCollisionEventCode::no_event;
        return true;
    }
    for (std::uint8_t value = 0U; value < 32U; ++value) {
        const auto candidate = "event_" + std::to_string(value);
        if (name == candidate) {
            event_code = static_cast<FieldCollisionEventCode>(value);
            return true;
        }
    }
    return false;
}

bool field_map_event_action(const std::string_view name,
                            const FieldCollisionEventCode source_event_code,
                            FieldMapEventAction& action) {
    if (name == "project_world_interaction") {
        action = FieldMapEventAction::project_world_interaction;
    } else if (name == "skip_world_interaction") {
        action = FieldMapEventAction::skip_world_interaction;
    } else if (name == "begin_world_activity") {
        // Keep source-specific continuation selection in the native adapter.
        switch (source_event_code) {
        case FieldCollisionEventCode::event_31:
            action = FieldMapEventAction::begin_event_31_activity;
            break;
        case FieldCollisionEventCode::event_30:
            action = FieldMapEventAction::begin_event_30_activity;
            break;
        case FieldCollisionEventCode::event_29:
            action = FieldMapEventAction::begin_event_29_activity;
            break;
        case FieldCollisionEventCode::event_28:
            action = FieldMapEventAction::begin_event_28_activity;
            break;
        case FieldCollisionEventCode::event_27:
            action = FieldMapEventAction::begin_event_27_activity;
            break;
        default:
            return false;
        }
    } else if (name == "mark_projected_terrain") {
        action = FieldMapEventAction::mark_projected_terrain;
    } else if (name == "begin_maglock_interaction") {
        action = FieldMapEventAction::begin_maglock_interaction;
    } else if (name == "begin_security_terminal") {
        action = FieldMapEventAction::begin_security_terminal;
    } else if (name == "begin_special_activity" &&
               source_event_code == FieldCollisionEventCode::event_25) {
        action = FieldMapEventAction::begin_event_25_activity;
    } else if (name == "begin_field_mode_exit") {
        action = FieldMapEventAction::begin_field_mode_exit;
    } else {
        return false;
    }
    return true;
}

bool valid_identifier(const std::string_view value) {
    return !value.empty() && value.size() <= kMaximumIdentifierLength &&
           std::all_of(value.begin(), value.end(), [](const char character) {
               return (character >= 'a' && character <= 'z') ||
                      (character >= 'A' && character <= 'Z') ||
                      (character >= '0' && character <= '9') ||
                      character == '_' || character == '-' ||
                      character == '.';
           });
}

bool valid_run_contact_id(const std::string_view value) {
    return value == "jump_house" || value == "matchstick" ||
           value == "big_rhino" || value == "wanderer" ||
           value == "icarus_descending";
}

bool valid_run_client_id(const std::string_view value) {
    return value == "mike_nana" || value == "john_worley" ||
           value == "ellis_goodson" || value == "john_van_anthony" ||
           value == "dok_whitson" || value == "jase_weese" ||
           value == "heinrich_michaels" || value == "billy_gibson" ||
           value == "nero_manser" || value == "alan_turing" ||
           value == "chris_stout" || value == "jorge_kish";
}

bool outcome_has_only_fields(
    lua_State* state, const int table_index,
    const std::initializer_list<std::string_view> allowed_fields) {
    const auto absolute_index = lua_absindex(state, table_index);
    lua_pushnil(state);
    while (lua_next(state, absolute_index) != 0) {
        if (lua_type(state, -2) != LUA_TSTRING) {
            lua_pop(state, 2);
            return false;
        }
        std::size_t length = 0U;
        const auto* key = lua_tolstring(state, -2, &length);
        const std::string_view name = key == nullptr
                                          ? std::string_view{}
                                          : std::string_view(key, length);
        const bool allowed = std::find(allowed_fields.begin(),
                                       allowed_fields.end(), name) !=
                             allowed_fields.end();
        lua_pop(state, 1);
        if (!allowed) {
            lua_pop(state, 1);
            return false;
        }
    }
    return true;
}

std::optional<DialogueOutcome> read_dialogue_outcome(
    lua_State* state, const int table_index) {
    if (!lua_istable(state, table_index)) {
        return std::nullopt;
    }
    const auto absolute_index = lua_absindex(state, table_index);
    std::string type_name;
    if (!read_field_string(state, absolute_index, "type", type_name)) {
        return std::nullopt;
    }
    const auto type = dialogue_outcome_type_from_name(type_name);
    if (!type.has_value()) {
        return std::nullopt;
    }

    std::uint32_t scene_id = 0U;
    switch (*type) {
    case DialogueOutcomeType::continue_in_place:
    case DialogueOutcomeType::return_to_caller:
    case DialogueOutcomeType::resume_source_route:
        if (!outcome_has_only_fields(state, absolute_index, {"type"})) {
            return std::nullopt;
        }
        if (*type == DialogueOutcomeType::continue_in_place) {
            return ContinueInPlaceOutcome{};
        }
        if (*type == DialogueOutcomeType::return_to_caller) {
            return ReturnToCallerOutcome{};
        }
        return ResumeSourceRouteOutcome{};
    case DialogueOutcomeType::return_to_origin: {
        std::string return_point_id;
        std::string return_policy;
        if (!outcome_has_only_fields(state, absolute_index,
                                     {"type", "returnPoint",
                                      "returnPolicy"}) ||
            !read_field_string(state, absolute_index, "returnPoint",
                               return_point_id) ||
            !read_optional_field_string(state, absolute_index,
                                        "returnPolicy",
                                        return_policy)) {
            return std::nullopt;
        }
        std::optional<std::string> optional_return_policy;
        if (!return_policy.empty()) {
            optional_return_policy = std::move(return_policy);
        }
        return ReturnToOriginOutcome{
            std::move(return_point_id), std::move(optional_return_policy)};
    }
    case DialogueOutcomeType::enter_field_scene: {
        std::string arrival_id;
        if (!outcome_has_only_fields(state, absolute_index,
                                     {"type", "sceneId", "arrivalId"}) ||
            !read_field_integer(state, absolute_index, "sceneId", 55U,
                                scene_id) ||
            !read_field_string(state, absolute_index, "arrivalId",
                               arrival_id)) {
            return std::nullopt;
        }
        return EnterFieldSceneOutcome{static_cast<std::uint8_t>(scene_id),
                                      std::move(arrival_id)};
    }
    case DialogueOutcomeType::enter_activity: {
        std::string activity_id;
        if (!outcome_has_only_fields(state, absolute_index,
                                     {"type", "activityId"}) ||
            !read_field_string(state, absolute_index, "activityId",
                               activity_id)) {
            return std::nullopt;
        }
        return EnterActivityOutcome{std::move(activity_id)};
    }
    case DialogueOutcomeType::open_service: {
        std::string service_id;
        std::string return_policy;
        if (!outcome_has_only_fields(state, absolute_index,
                                     {"type", "serviceId",
                                      "returnPolicy"}) ||
            !read_field_string(state, absolute_index, "serviceId",
                               service_id) ||
            !read_field_string(state, absolute_index, "returnPolicy",
                               return_policy)) {
            return std::nullopt;
        }
        return OpenServiceOutcome{std::move(service_id),
                                  std::move(return_policy)};
    }
    case DialogueOutcomeType::play_cinematic: {
        std::string cinematic_id;
        std::string return_policy;
        if (!outcome_has_only_fields(state, absolute_index,
                                     {"type", "cinematicId",
                                      "returnPolicy"}) ||
            !read_field_string(state, absolute_index, "cinematicId",
                               cinematic_id) ||
            !read_field_string(state, absolute_index, "returnPolicy",
                               return_policy)) {
            return std::nullopt;
        }
        return PlayCinematicOutcome{std::move(cinematic_id),
                                    std::move(return_policy)};
    }
    case DialogueOutcomeType::source_boundary: {
        std::string reason;
        if (!outcome_has_only_fields(state, absolute_index,
                                     {"type", "reason"}) ||
            !read_field_text(state, absolute_index, "reason", reason, 512U) ||
            reason.empty()) {
            return std::nullopt;
        }
        return SourceBoundaryOutcome{std::move(reason)};
    }
    }
    return std::nullopt;
}

bool valid_lua_module_name(const std::string_view value) {
    if (value.empty() || value.size() > kMaximumIdentifierLength) {
        return false;
    }
    bool component_start = true;
    for (const char character : value) {
        if (character == '.') {
            if (component_start) {
                return false;
            }
            component_start = true;
            continue;
        }
        const bool letter = (character >= 'a' && character <= 'z') ||
                            (character >= 'A' && character <= 'Z');
        const bool digit = character >= '0' && character <= '9';
        if (component_start) {
            if (!letter && character != '_') {
                return false;
            }
        } else if (!letter && character != '_' && !digit) {
            return false;
        }
        component_start = false;
    }
    return !component_start;
}

bool path_is_within_root(const std::filesystem::path& root,
                         const std::filesystem::path& candidate) {
    const auto relative = candidate.lexically_relative(root);
    if (relative.empty() || relative.is_absolute()) {
        return false;
    }
    for (const auto& component : relative) {
        if (component == "..") {
            return false;
        }
    }
    return true;
}

void open_safe_libraries(lua_State* state) {
    luaL_requiref(state, LUA_GNAME, luaopen_base, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    lua_pop(state, 1);

    // Scripts are event handlers and menu descriptions, not plug-ins with
    // filesystem, process, or dynamic module access.
    for (const char* name : {"dofile", "loadfile", "load",
                             "collectgarbage", "getmetatable"}) {
        lua_pushnil(state);
        lua_setglobal(state, name);
    }
}

bool field_flag(const std::string_view name, std::uint32_t& flag) {
    if (name == "new_game_opening_completed") {
        flag = field_map_event_flag(
            FieldMapEventStateFlag::new_game_opening_completed);
        return true;
    }
    return false;
}

bool field_trigger(const std::string_view name,
                   FieldMapEventTrigger& trigger) {
    if (name == "scene_entry") {
        trigger = FieldMapEventTrigger::scene_entry;
        return true;
    }
    if (name == "actor_within_scope") {
        trigger = FieldMapEventTrigger::actor_within_scope;
        return true;
    }
    return false;
}

bool field_repeat_policy(const std::string_view name,
                         FieldMapEventRepeatPolicy& policy) {
    if (name == "every_match") {
        policy = FieldMapEventRepeatPolicy::every_match;
        return true;
    }
    if (name == "once_per_scene_visit") {
        policy = FieldMapEventRepeatPolicy::once_per_scene_visit;
        return true;
    }
    if (name == "once_per_session") {
        policy = FieldMapEventRepeatPolicy::once_per_session;
        return true;
    }
    return false;
}

bool field_action(const std::string_view name,
                  FieldMapEventScriptAction& action) {
    if (name == "run_lua_handler") {
        action = FieldMapEventScriptAction::run_lua_handler;
        return true;
    }
    if (name == "dispatch_native_script") {
        action = FieldMapEventScriptAction::dispatch_native_script;
        return true;
    }
    return false;
}

bool parse_coordinate_scope(lua_State* state, const int rule_index,
                            std::optional<FieldMapEventCoordinateScope>& scope) {
    lua_getfield(state, rule_index, "scope");
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        scope.reset();
        return true;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto scope_index = lua_absindex(state, -1);
    FieldMapEventCoordinateScope parsed{};
    std::uint32_t min_x = 0U;
    std::uint32_t max_x = 0U;
    std::uint32_t min_y = 0U;
    std::uint32_t max_y = 0U;
    const bool valid =
        read_field_integer(state, scope_index, "min_x", 0xFFFFU,
                           min_x) &&
        read_field_integer(state, scope_index, "max_x", 0xFFFFU,
                           max_x) &&
        read_field_integer(state, scope_index, "min_y", 0xFFFFU,
                           min_y) &&
        read_field_integer(state, scope_index, "max_y", 0xFFFFU,
                           max_y) &&
        min_x <= max_x && min_y <= max_y;
    lua_pop(state, 1);
    if (valid) {
        parsed.min_x = static_cast<std::uint16_t>(min_x);
        parsed.max_x = static_cast<std::uint16_t>(max_x);
        parsed.min_y = static_cast<std::uint16_t>(min_y);
        parsed.max_y = static_cast<std::uint16_t>(max_y);
        scope = parsed;
    }
    return valid;
}

bool parse_required_flags(lua_State* state, const int rule_index,
                          std::uint32_t& flags) {
    lua_getfield(state, rule_index, "requires");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto flags_index = lua_absindex(state, -1);
    const auto count = lua_rawlen(state, flags_index);
    bool valid = count <= 32U;
    for (std::size_t index = 1U; valid && index <= count; ++index) {
        lua_rawgeti(state, flags_index, static_cast<lua_Integer>(index));
        std::string name;
        std::uint32_t flag = 0U;
        valid = read_string(state, -1, name) && field_flag(name, flag);
        if (valid) {
            flags |= flag;
        }
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    return valid;
}

bool parse_event_rule(lua_State* state, const int index,
                      FieldMapEventRule& rule) {
    if (!lua_istable(state, index)) {
        return false;
    }
    std::uint32_t scene_id = 0U;
    std::string trigger_name;
    std::string repeat_name;
    std::string action_name;
    std::string handler_name;
    if (!read_field_integer(state, index, "id", 0xFFFFFFFFU, rule.id) ||
        rule.id == 0U ||
        !read_field_integer(state, index, "scene", 0xFFFFU, scene_id) ||
        !read_field_string(state, index, "trigger", trigger_name) ||
        !field_trigger(trigger_name, rule.trigger) ||
        !read_field_string(state, index, "repeat", repeat_name) ||
        !field_repeat_policy(repeat_name, rule.repeat_policy) ||
        !read_field_string(state, index, "action", action_name) ||
        !field_action(action_name, rule.action) ||
        !parse_coordinate_scope(state, index, rule.scope) ||
        !parse_required_flags(state, index, rule.required_state_flags)) {
        return false;
    }
    if (rule.action == FieldMapEventScriptAction::run_lua_handler &&
        (!read_field_string(state, index, "handler", handler_name) ||
         !valid_identifier(handler_name))) {
        return false;
    }
    rule.scene_id = static_cast<std::uint16_t>(scene_id);
    rule.handler = std::move(handler_name);
    return true;
}

bool parse_menu_choices(lua_State* state, const int index,
                        const std::string_view menu_id,
                        const std::string_view text_profile,
                        const std::string_view speaker_id,
                        ScriptMenuDefinition& menu,
                        const std::size_t maximum_choices =
                            kMaximumMenuChoices) {
    if (!lua_istable(state, index) || !valid_identifier(menu_id) ||
        !valid_identifier(text_profile)) {
        return false;
    }
    const auto count = lua_rawlen(state, index);
    if (count > maximum_choices) {
        return false;
    }
    menu.id = std::string(menu_id);
    menu.text_profile = std::string(text_profile);
    menu.speaker_id = std::string(speaker_id);
    menu.choices.reserve(count);
    std::set<std::string> choice_ids;
    for (std::size_t choice_index = 1U; choice_index <= count;
         ++choice_index) {
        lua_rawgeti(state, index, static_cast<lua_Integer>(choice_index));
        ScriptMenuChoice choice;
        bool valid = false;
        if (lua_istable(state, -1)) {
            const auto choice_record = lua_absindex(state, -1);
            const bool label_key_valid =
                read_optional_field_string(state, choice_record,
                                           "label_key", choice.label_key) &&
                (choice.label_key.empty() ||
                 valid_identifier(choice.label_key));
            valid = read_field_string(state, choice_record, "id", choice.id) &&
                    read_field_string(state, choice_record, "label",
                                      choice.inline_label) &&
                    label_key_valid &&
                    !choice.inline_label.empty() &&
                    choice.inline_label.size() <= 160U &&
                    std::none_of(
                        choice.inline_label.begin(),
                        choice.inline_label.end(), [](const unsigned char ch) {
                            return ch < 0x20U || ch == 0x7FU;
                        });
        } else {
            valid = read_string(state, -1, choice.id);
        }
        valid = valid && valid_identifier(choice.id);
        lua_pop(state, 1);
        if (!valid || !choice_ids.insert(choice.id).second) {
            return false;
        }
        menu.choices.push_back(std::move(choice));
    }
    return true;
}

bool parse_explicit_menu_text_keys(lua_State* state,
                                   const int title_key_index,
                                   const int label_keys_index,
                                   ScriptMenuDefinition& menu) {
    if (!lua_isnil(state, title_key_index)) {
        if (!read_string(state, title_key_index, menu.title_key) ||
            !valid_identifier(menu.title_key)) {
            return false;
        }
    }
    if (!lua_istable(state, label_keys_index) ||
        lua_rawlen(state, label_keys_index) != menu.choices.size()) {
        return false;
    }
    for (std::size_t index = 0; index < menu.choices.size(); ++index) {
        auto& label_key = menu.choices[index].label_key;
        lua_rawgeti(state, label_keys_index,
                    static_cast<lua_Integer>(index + 1U));
        const bool valid = read_string(state, -1, label_key) &&
                           valid_identifier(label_key);
        lua_pop(state, 1);
        if (!valid) {
            return false;
        }
    }
    return true;
}

constexpr std::size_t kMaximumPersistentStateNodes = 10000U;
constexpr std::size_t kMaximumPersistentStateDepth = 32U;
constexpr std::size_t kMaximumPersistentStateStringBytes = 65536U;

struct PersistentReadContext {
    std::size_t nodes = 0U;
    std::set<const void*> active_tables;
    std::string error;
};

bool read_persistent_value(lua_State* state, const int index,
                          ScriptPersistentValue& output,
                          PersistentReadContext& context,
                          const std::size_t depth) {
    if (depth > kMaximumPersistentStateDepth ||
        ++context.nodes > kMaximumPersistentStateNodes) {
        context.error = "script save state exceeds its size limit";
        return false;
    }
    switch (lua_type(state, index)) {
    case LUA_TNIL:
        output.value = std::monostate{};
        return true;
    case LUA_TBOOLEAN:
        output.value = lua_toboolean(state, index) != 0;
        return true;
    case LUA_TNUMBER:
        if (lua_isinteger(state, index)) {
            output.value = static_cast<std::int64_t>(lua_tointeger(state, index));
            return true;
        } else {
            const auto value = static_cast<double>(lua_tonumber(state, index));
            if (!std::isfinite(value)) {
                context.error = "script save state contains a non-finite number";
                return false;
            }
            output.value = value;
            return true;
        }
    case LUA_TSTRING: {
        std::size_t size = 0U;
        const auto* value = lua_tolstring(state, index, &size);
        if (value == nullptr || size > kMaximumPersistentStateStringBytes) {
            context.error = "script save state contains an oversized string";
            return false;
        }
        output.value = std::string(value, size);
        return true;
    }
    case LUA_TTABLE:
        break;
    default:
        context.error = "script save state contains a non-data Lua value";
        return false;
    }

    const auto table_index = lua_absindex(state, index);
    const auto* identity = lua_topointer(state, table_index);
    if (identity == nullptr || !context.active_tables.insert(identity).second) {
        context.error = "script save state contains a cyclic table";
        return false;
    }
    ScriptPersistentValue::Object object;
    std::map<lua_Integer, ScriptPersistentValue> indexed;
    bool has_string_keys = false;
    bool has_integer_keys = false;
    lua_pushnil(state);
    while (lua_next(state, table_index) != 0) {
        ScriptPersistentValue child;
        if (!read_persistent_value(state, -1, child, context, depth + 1U)) {
            lua_pop(state, 1);
            context.active_tables.erase(identity);
            return false;
        }
        if (lua_type(state, -2) == LUA_TSTRING) {
            std::size_t key_size = 0U;
            const auto* key = lua_tolstring(state, -2, &key_size);
            if (key == nullptr || key_size == 0U || key_size > 256U) {
                context.error = "script save state has an invalid object key";
                lua_pop(state, 1);
                context.active_tables.erase(identity);
                return false;
            }
            has_string_keys = true;
            object.emplace(std::string(key, key_size), std::move(child));
        } else if (lua_isinteger(state, -2)) {
            const auto key = lua_tointeger(state, -2);
            if (key < 1 || static_cast<lua_Unsigned>(key) >
                               kMaximumPersistentStateNodes) {
                context.error = "script save state has an invalid array index";
                lua_pop(state, 1);
                context.active_tables.erase(identity);
                return false;
            }
            has_integer_keys = true;
            indexed.emplace(key, std::move(child));
        } else {
            context.error = "script save state has an unsupported table key";
            lua_pop(state, 1);
            context.active_tables.erase(identity);
            return false;
        }
        lua_pop(state, 1);
    }
    context.active_tables.erase(identity);
    if (has_string_keys && has_integer_keys) {
        context.error = "script save state mixes array and object keys";
        return false;
    }
    if (has_integer_keys) {
        ScriptPersistentValue::Array array;
        array.reserve(indexed.size());
        lua_Integer expected = 1;
        for (auto& [key, value] : indexed) {
            if (key != expected++ ||
                std::holds_alternative<std::monostate>(value.value)) {
                context.error = "script save state has a sparse or null array";
                return false;
            }
            array.push_back(std::move(value));
        }
        output.value = std::move(array);
    } else {
        output.value = std::move(object);
    }
    return true;
}

bool push_persistent_value(lua_State* state,
                           const ScriptPersistentValue& value,
                           std::size_t& nodes, const std::size_t depth) {
    if (depth > kMaximumPersistentStateDepth ||
        ++nodes > kMaximumPersistentStateNodes) {
        return false;
    }
    if (std::holds_alternative<std::monostate>(value.value)) {
        lua_pushnil(state);
    } else if (const auto* boolean = std::get_if<bool>(&value.value)) {
        lua_pushboolean(state, *boolean);
    } else if (const auto* integer = std::get_if<std::int64_t>(&value.value)) {
        if (*integer < static_cast<std::int64_t>(LUA_MININTEGER) ||
            *integer > static_cast<std::int64_t>(LUA_MAXINTEGER)) {
            return false;
        }
        lua_pushinteger(state, static_cast<lua_Integer>(*integer));
    } else if (const auto* number = std::get_if<double>(&value.value)) {
        if (!std::isfinite(*number)) return false;
        lua_pushnumber(state, static_cast<lua_Number>(*number));
    } else if (const auto* string = std::get_if<std::string>(&value.value)) {
        if (string->size() > kMaximumPersistentStateStringBytes) return false;
        lua_pushlstring(state, string->data(), string->size());
    } else if (const auto* array =
                   std::get_if<ScriptPersistentValue::Array>(&value.value)) {
        lua_createtable(state, static_cast<int>(array->size()), 0);
        const auto table_index = lua_absindex(state, -1);
        for (std::size_t index = 0; index < array->size(); ++index) {
            if (std::holds_alternative<std::monostate>((*array)[index].value) ||
                !push_persistent_value(state, (*array)[index], nodes,
                                       depth + 1U)) {
                return false;
            }
            lua_rawseti(state, table_index,
                        static_cast<lua_Integer>(index + 1U));
        }
    } else if (const auto* object =
                   std::get_if<ScriptPersistentValue::Object>(&value.value)) {
        lua_createtable(state, 0, static_cast<int>(object->size()));
        const auto table_index = lua_absindex(state, -1);
        for (const auto& [key, child] : *object) {
            lua_pushlstring(state, key.data(), key.size());
            if (std::holds_alternative<std::monostate>(child.value)) {
                lua_pushnil(state);
            } else if (!push_persistent_value(state, child, nodes,
                                              depth + 1U)) {
                return false;
            }
            lua_settable(state, table_index);
        }
    }
    return true;
}

} // namespace

std::uint16_t source_movement_substate_for_cardinal_direction(
    const ScriptCardinalDirection direction) noexcept {
    // ROM 0xBADE records the cardinal fallback in the source movement
    // substate. Keep that encoding at the native state boundary.
    switch (direction) {
    case ScriptCardinalDirection::north:
        return 0U;
    case ScriptCardinalDirection::east:
        return 2U;
    case ScriptCardinalDirection::south:
        return 4U;
    case ScriptCardinalDirection::west:
        return 6U;
    }
    return 0U;
}

std::int16_t source_route_status_after_script_update(
    const std::int16_t current_status,
    const ScriptMovementRouteUpdate& update) noexcept {
    // The route field is retained only by the native record. Lua reports the
    // gameplay outcome; this adapter preserves or encodes the live field.
    switch (update.kind) {
    case ScriptMovementRouteUpdate::Kind::unchanged:
        return current_status;
    case ScriptMovementRouteUpdate::Kind::reset:
        return -1;
    case ScriptMovementRouteUpdate::Kind::blocked_by_direction:
        return static_cast<std::int16_t>(
            source_movement_substate_for_cardinal_direction(
                update.direction));
    }
    return current_status;
}

struct GameScriptRuntime::Impl {
    enum class MenuBinding : lua_Integer {
        play_sound = 1,
        stop_sound,
        redraw,
        blank,
        mask,
        set_window,
        set_screen,
        menu_closed,
        legacy_open,
        legacy_advance,
        reselect_leader,
        leader_posture,
        set_leader_posture,
        leader_class,
        party,
        spell_table,
        set_spell_table,
        active_slot,
        magazine_capacity,
        set_runner_ammo,
        slot_active_set,
        commit_party,
        roll_successes,
        pocket_extra_row,
        in_combat,
        is_outside_area,
        save_game,
        load_game,
        save_status,
        source_random_bound,
        matrix_alert_random_bound,
        dismiss_runner,
        group_item_ids,
        notebook_facts,
        cyberdeck_available,
        notebook_boundary,
        consume_debug_unlock,
        nuyen,
        set_nuyen,
        begin_taxi_cabbie,
        seed_debug_contacts,
        set_walk_fast,
        install_test_deck,
        deck_enter,
        deck_leave,
        set_deck_mode,
        matrix_online,
        matrix_node,
        alert,
        world_effect_gate_negative,
        matrix_node_discovered,
        matrix_world_state,
        commit_matrix_world_state,
        deck_record,
        deck_slots,
        deck_commit,
        program_unloaded,
        menu_b_policy,
    };

    lua_State* state = nullptr;
    lua_State* task_state = nullptr;
    int modules_reference = LUA_NOREF;
    int task_reference = LUA_NOREF;
    int menu_route_reference = LUA_NOREF;
    bool task_waiting = false;
    std::filesystem::path script_root;
    std::set<std::string> modules_loading;
    std::size_t loaded_module_count = 0U;
    std::optional<ScriptNativeOperation> pending_task_operation;
    std::optional<ScriptNativeOperation> active_native_operation;
    std::optional<ScriptDialogueOpenDescriptor> active_dialogue_open;
    StoryFacts story_facts;
    std::vector<ScriptConditionalActorCandidate> conditional_actor_catalog;
    std::vector<ScriptActivitySelectorBinding> activity_selector_bindings;
    std::vector<ScriptWorldActivityGroupBinding> activity_group_bindings;
    std::unordered_map<std::string, std::uint16_t>
        action_profile_index_by_actor_profile_id;
    StoryFactReader story_fact_reader;
    StoryFactWriter story_fact_writer;
    std::unordered_map<std::string, std::string> dialogue_texts;
    std::string location_type;
    std::string location_id;
    std::string location_text_profile;
    std::string location_speaker_id;
    std::string location_event_id;
    std::optional<std::uint8_t> location_scene_id;
    ScriptFieldModeContext field_mode_context{};
    ScriptMenuRouteBindings menu_route_bindings{};
    ScriptMenuRouteSnapshot menu_route_state{};
    ScriptActorHeartbeatObserver actor_heartbeat_observer;
    const ScriptActorBehaviorContext* active_actor_context = nullptr;
    struct PendingMovementRecoveryProbe {
        std::uint16_t x = 0U;
        std::uint16_t y = 0U;
        std::int16_t collision_code = 0;
        std::uint8_t direction_ordinal = 0U;
    };
    std::optional<PendingMovementRecoveryProbe>
        last_actor_movement_recovery_probe;
    std::optional<std::uint16_t> last_actor_terrain_collision_code;
    ScriptActorRandomBound active_conditional_actor_random_bound;
    std::string bridge_failure;
    std::string failure;

    static Impl* from_upvalue(lua_State* lua) {
        return static_cast<Impl*>(lua_touserdata(lua, lua_upvalueindex(1)));
    }

    static int menu_binding_dispatch(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr) {
            lua_pushnil(lua);
            return 1;
        }
        const auto binding = static_cast<MenuBinding>(
            lua_tointeger(lua, lua_upvalueindex(2)));
        const auto integer_argument = [lua](const int index) {
            int is_number = 0;
            const auto value = lua_tointegerx(lua, index, &is_number);
            return is_number ? static_cast<int>(value) : 0;
        };
        switch (binding) {
        case MenuBinding::play_sound:
            if (self->menu_route_bindings.host.play_sound) {
                self->menu_route_bindings.host.play_sound(
                    static_cast<std::uint16_t>(integer_argument(1)));
            }
            return 0;
        case MenuBinding::stop_sound:
            if (self->menu_route_bindings.host.stop_sound) {
                self->menu_route_bindings.host.stop_sound(
                    static_cast<std::uint16_t>(integer_argument(1)));
            }
            return 0;
        case MenuBinding::redraw:
            if (self->menu_route_bindings.host.redraw) {
                self->menu_route_bindings.host.redraw(
                    static_cast<std::uint32_t>(integer_argument(1)));
            }
            return 0;
        case MenuBinding::blank:
            if (self->menu_route_bindings.host.blank) {
                self->menu_route_bindings.host.blank(
                    static_cast<std::uint16_t>(integer_argument(1)),
                    static_cast<std::uint16_t>(integer_argument(2)),
                    static_cast<std::uint16_t>(integer_argument(3)));
            }
            return 0;
        case MenuBinding::mask:
            if (self->menu_route_bindings.host.mask) {
                self->menu_route_bindings.host.mask(
                    static_cast<std::uint16_t>(integer_argument(1)),
                    static_cast<std::uint16_t>(integer_argument(2)),
                    static_cast<std::int16_t>(integer_argument(3)),
                    static_cast<std::int16_t>(integer_argument(4)),
                    static_cast<std::int16_t>(integer_argument(5)),
                    static_cast<std::int16_t>(integer_argument(6)));
            }
            return 0;
        case MenuBinding::set_window:
            if (self->menu_route_bindings.host.set_window) {
                self->menu_route_bindings.host.set_window(
                    static_cast<std::uint16_t>(integer_argument(1)));
            }
            return 0;
        case MenuBinding::set_screen:
            if (self->menu_route_bindings.host.set_screen) {
                self->menu_route_bindings.host.set_screen(
                    static_cast<std::uint16_t>(integer_argument(1)));
            }
            return 0;
        case MenuBinding::menu_closed:
            if (self->menu_route_bindings.host.menu_closed) {
                self->menu_route_bindings.host.menu_closed(
                    lua_toboolean(lua, 1) != 0,
                    lua_toboolean(lua, 2) != 0);
            }
            return 0;
        case MenuBinding::legacy_open: {
            ScriptMenuRouteArgs args;
            if (lua_istable(lua, 2)) {
                args.command_section = read_optional_command_section(
                    lua, 2, "command_section");
                const auto read_optional = [lua](const char* name)
                    -> std::optional<int> {
                    lua_getfield(lua, 2, name);
                    int is_number = 0;
                    const auto value = lua_tointegerx(lua, -1, &is_number);
                    lua_pop(lua, 1);
                    return is_number ? std::optional<int>(
                                           static_cast<int>(value))
                                     : std::nullopt;
                };
                args.page = read_optional("page");
                args.row = read_optional("row");
                args.column = read_optional("column");
            }
            if (self->menu_route_bindings.host.legacy_open) {
                self->menu_route_bindings.host.legacy_open(
                    static_cast<std::uint16_t>(integer_argument(1)), args);
            }
            return 0;
        }
        case MenuBinding::legacy_advance: {
            std::pair<bool, bool> result{false, false};
            if (self->menu_route_bindings.host.legacy_advance) {
                result = self->menu_route_bindings.host.legacy_advance(
                    static_cast<std::uint16_t>(integer_argument(1)),
                    static_cast<std::uint16_t>(integer_argument(2)));
            }
            lua_pushboolean(lua, result.first ? 1 : 0);
            lua_pushboolean(lua, result.second ? 1 : 0);
            return 2;
        }
        case MenuBinding::notebook_boundary: {
            const auto value = integer_argument(1);
            if (value >= 0 &&
                value <= static_cast<int>(
                             ScriptNotebookBoundary::mr_johnsons_unresolved) &&
                self->menu_route_bindings.host.notebook_boundary) {
                self->menu_route_bindings.host.notebook_boundary(
                    static_cast<ScriptNotebookBoundary>(value));
            }
            return 0;
        }
        case MenuBinding::reselect_leader:
            if (self->menu_route_bindings.game.reselect_leader) {
                self->menu_route_bindings.game.reselect_leader();
            }
            return 0;
        case MenuBinding::leader_posture:
            lua_pushinteger(
                lua, self->menu_route_bindings.game.leader_posture
                         ? self->menu_route_bindings.game.leader_posture()
                         : 0);
            return 1;
        case MenuBinding::set_leader_posture:
            if (self->menu_route_bindings.game.set_leader_posture) {
                self->menu_route_bindings.game.set_leader_posture(
                    integer_argument(1));
            }
            return 0;
        case MenuBinding::leader_class:
            {
                const auto class_name = runner_class_name(
                    self->menu_route_bindings.game.leader_class
                        ? self->menu_route_bindings.game.leader_class()
                        : ScriptRunnerClass::unknown);
                lua_pushlstring(lua, class_name.data(), class_name.size());
            }
            return 1;
        case MenuBinding::party: {
            const auto party = self->menu_route_bindings.game.party
                                   ? self->menu_route_bindings.game.party()
                                   : std::array<ScriptMenuPartyMember, 3>{};
            lua_createtable(lua, static_cast<int>(party.size()), 0);
            for (std::size_t index = 0; index < party.size(); ++index) {
                const auto& member = party[index];
                lua_createtable(lua, 0, 24);
                const auto member_index = lua_absindex(lua, -1);
                const auto set_integer = [lua, member_index](
                    const char* name, const lua_Integer value) {
                    lua_pushinteger(lua, value);
                    lua_setfield(lua, member_index, name);
                };
                const auto set_string = [lua, member_index](
                    const char* name, const std::string_view value) {
                    lua_pushlstring(lua, value.data(), value.size());
                    lua_setfield(lua, member_index, name);
                };
                push_runner_menu_facts(lua, member.status);
                lua_setfield(lua, member_index, "runner_facts");
                set_integer("clips", member.clips);
                set_integer("rounds", member.rounds);
                if (member.weapon_slot >= 0 &&
                    member.weapon_slot <
                        static_cast<int>(member.inventory.size())) {
                    const auto item_id = script_item_id_from_source_code(
                        member.weapon_code);
                    if (item_id.has_value()) {
                        set_string("equipped_weapon_id", *item_id);
                        push_inventory_cell(
                            lua, member_index, "equipped_weapon_cell",
                            static_cast<std::size_t>(member.weapon_slot));
                    } else {
                        lua_pushnil(lua);
                        lua_setfield(lua, member_index,
                                     "equipped_weapon_id");
                        lua_pushnil(lua);
                        lua_setfield(lua, member_index,
                                     "equipped_weapon_cell");
                    }
                    lua_pushnil(lua);
                    lua_setfield(lua, member_index, "selected_spell_id");
                } else {
                    lua_pushnil(lua);
                    lua_setfield(lua, member_index, "equipped_weapon_id");
                    lua_pushnil(lua);
                    lua_setfield(lua, member_index,
                                 "equipped_weapon_cell");
                    const auto spell_id =
                        script_spell_id_from_action_code(member.weapon_code);
                    if (spell_id.has_value()) {
                        set_string("selected_spell_id", *spell_id);
                    } else {
                        lua_pushnil(lua);
                        lua_setfield(lua, member_index,
                                     "selected_spell_id");
                    }
                }
                const auto armor_id = script_armor_item_id_from_source_index(
                    member.armor);
                if (armor_id.has_value()) {
                    set_string("equipped_armor_id", *armor_id);
                } else {
                    lua_pushnil(lua);
                    lua_setfield(lua, member_index, "equipped_armor_id");
                }
                set_integer("physical", member.physical);
                set_integer("stun", member.stun);
                set_integer("magic", member.magic_resource);
                set_string("race", runner_race_name(member.race));
                set_string("character_class",
                           runner_class_name(member.character_class));
                set_integer("karma", member.karma);
                set_integer("force", member.magic_force);
                set_integer("posture", member.posture);
                const auto continuation = member.state_function == 0xB45CU
                    ? std::string_view{"idle"}
                    : std::string_view{"unknown"};
                set_string("first_aid_continuation", continuation);
                if (push_menu_equipment(lua, member.inventory,
                                        member.phase)) {
                    lua_setfield(lua, member_index, "equipment");
                } else {
                    lua_pushnil(lua);
                    lua_setfield(lua, member_index, "equipment");
                }
                lua_createtable(lua, 0, 6);
                lua_pushinteger(lua, member.attributes.body);
                lua_setfield(lua, -2, "body");
                lua_pushinteger(lua, member.attributes.quickness);
                lua_setfield(lua, -2, "quickness");
                lua_pushinteger(lua, member.attributes.strength);
                lua_setfield(lua, -2, "strength");
                lua_pushinteger(lua, member.attributes.charisma);
                lua_setfield(lua, -2, "charisma");
                lua_pushinteger(lua, member.attributes.intelligence);
                lua_setfield(lua, -2, "intelligence");
                lua_pushinteger(lua, member.attributes.willpower);
                lua_setfield(lua, -2, "willpower");
                lua_setfield(lua, member_index, "attributes");
                lua_createtable(lua, 0, 12);
                lua_pushinteger(lua, member.skills.sorcery);
                lua_setfield(lua, -2, "sorcery");
                lua_pushinteger(lua, member.skills.firearms);
                lua_setfield(lua, -2, "firearms");
                lua_pushinteger(lua, member.skills.pistols);
                lua_setfield(lua, -2, "pistols");
                lua_pushinteger(lua, member.skills.smgs);
                lua_setfield(lua, -2, "smgs");
                lua_pushinteger(lua, member.skills.shotguns);
                lua_setfield(lua, -2, "shotguns");
                lua_pushinteger(lua, member.skills.melee_combat);
                lua_setfield(lua, -2, "melee_combat");
                lua_pushinteger(lua, member.skills.throwing);
                lua_setfield(lua, -2, "throwing");
                lua_pushinteger(lua, member.skills.computer);
                lua_setfield(lua, -2, "computer");
                lua_pushinteger(lua, member.skills.biotech);
                lua_setfield(lua, -2, "biotech");
                lua_pushinteger(lua, member.skills.electronics);
                lua_setfield(lua, -2, "electronics");
                lua_pushinteger(lua, member.skills.reputation);
                lua_setfield(lua, -2, "reputation");
                lua_pushinteger(lua, member.skills.negotiation);
                lua_setfield(lua, -2, "negotiation");
                lua_setfield(lua, member_index, "skills");
                lua_rawseti(lua, -2, static_cast<lua_Integer>(index));
            }
            return 1;
        }
        case MenuBinding::spell_table: {
            const auto table = self->menu_route_bindings.game.spell_table
                                   ? self->menu_route_bindings.game.spell_table()
                                   : std::nullopt;
            if (!table.has_value()) {
                lua_pushnil(lua);
                return 1;
            }
            lua_createtable(lua,
                            static_cast<int>(table->entry_count), 0);
            for (std::size_t index = 0; index < table->entry_count;
                 ++index) {
                const auto& entry = table->entries[index];
                lua_createtable(lua, 0, 5);
                const auto entry_index = lua_absindex(lua, -1);
                lua_pushlstring(lua, entry.spell_id.data(),
                                entry.spell_id.size());
                lua_setfield(lua, entry_index, "id");
                lua_pushinteger(lua, entry.force_limit);
                lua_setfield(lua, entry_index, "force_limit");
                lua_pushinteger(lua, entry.availability);
                lua_setfield(lua, entry_index, "availability");
                lua_pushinteger(lua, entry.difficulty_modifier);
                lua_setfield(lua, entry_index, "difficulty_modifier");
                lua_pushinteger(lua, entry.base_effect);
                lua_setfield(lua, entry_index, "base_effect");
                lua_rawseti(lua, -2, static_cast<lua_Integer>(index + 1U));
            }
            return 1;
        }
        case MenuBinding::set_spell_table: {
            const auto callback = self->menu_route_bindings.game.set_spell_table;
            if (!callback || !lua_istable(lua, 1)) {
                return 0;
            }
            const auto entries_index = lua_absindex(lua, 1);
            const auto entry_count = lua_rawlen(lua, entries_index);
            if (entry_count > 14U) {
                return 0;
            }
            ScriptMenuSpellTable parsed;
            parsed.entry_count = static_cast<std::uint8_t>(entry_count);
            for (std::size_t index = 0; index < parsed.entry_count; ++index) {
                lua_rawgeti(lua, entries_index,
                            static_cast<lua_Integer>(index + 1U));
                if (!lua_istable(lua, -1)) {
                    lua_pop(lua, 2);
                    return 0;
                }
                const auto entry_index = lua_absindex(lua, -1);
                std::string spell_id;
                std::uint32_t force = 0U;
                const bool valid_entry =
                    read_field_string(lua, entry_index, "id", spell_id) &&
                    read_field_integer(lua, entry_index, "force_limit",
                                       0xFFU, force);
                lua_pop(lua, 1);
                const auto action_code = valid_entry
                    ? script_spell_action_code_from_id(spell_id)
                    : std::nullopt;
                if (!action_code.has_value()) {
                    lua_pop(lua, 1);
                    return 0;
                }
                for (std::size_t prior = 0U; prior < index; ++prior) {
                    if (parsed.entries[prior].spell_id == spell_id) {
                        lua_pop(lua, 1);
                        return 0;
                    }
                }
                parsed.entries[index].spell_id = std::move(spell_id);
                parsed.entries[index].force_limit =
                    static_cast<std::uint8_t>(force);
            }
            callback(parsed);
            return 0;
        }
        case MenuBinding::active_slot:
            lua_pushinteger(
                lua, self->menu_route_bindings.game.active_slot
                         ? self->menu_route_bindings.game.active_slot()
                         : 0);
            return 1;
        case MenuBinding::magazine_capacity:
            {
                std::string item_id;
                const auto item_code = read_string(lua, 1, item_id)
                    ? script_item_source_code_from_id(item_id)
                    : std::nullopt;
                lua_pushinteger(
                    lua, item_code.has_value() &&
                                 self->menu_route_bindings.game
                                     .magazine_capacity
                             ? self->menu_route_bindings.game
                                   .magazine_capacity(*item_code)
                             : 0);
            }
            return 1;
        case MenuBinding::set_runner_ammo:
            if (self->menu_route_bindings.game.set_runner_ammo) {
                self->menu_route_bindings.game.set_runner_ammo(
                    integer_argument(1), integer_argument(2),
                    integer_argument(3));
            }
            return 0;
        case MenuBinding::slot_active_set: {
            const auto active = self->menu_route_bindings.game.slot_active_set
                                    ? self->menu_route_bindings.game
                                          .slot_active_set()
                                    : std::array<bool, 3>{};
            lua_createtable(lua, static_cast<int>(active.size()), 0);
            for (std::size_t index = 0; index < active.size(); ++index) {
                lua_pushboolean(lua, active[index] ? 1 : 0);
                lua_rawseti(lua, -2, static_cast<lua_Integer>(index));
            }
            return 1;
        }
        case MenuBinding::commit_party: {
            if (!self->menu_route_bindings.game.commit_party ||
                !lua_istable(lua, 1)) {
                return 0;
            }
            const auto original_party = self->menu_route_bindings.game.party
                ? self->menu_route_bindings.game.party()
                : std::array<ScriptMenuPartyMember, 3>{};
            std::array<ScriptMenuPartyMember, 3> party{};
            const auto read_integer = [lua](const int table_index,
                                            const char* name,
                                            const lua_Integer fallback) {
                lua_getfield(lua, table_index, name);
                int is_number = 0;
                const auto value = lua_tointegerx(lua, -1, &is_number);
                lua_pop(lua, 1);
                return is_number ? value : fallback;
            };
            const auto read_stat = [&read_integer](const int table_index,
                                                   const char* name,
                                                   const int fallback) {
                return static_cast<int>(std::clamp<lua_Integer>(
                    read_integer(table_index, name, fallback), 0, 0xFF));
            };
            const auto party_index = lua_absindex(lua, 1);
            for (std::size_t index = 0; index < party.size(); ++index) {
                lua_rawgeti(lua, party_index,
                            static_cast<lua_Integer>(index));
                if (lua_istable(lua, -1)) {
                    const auto member_index = lua_absindex(lua, -1);
                    auto& member = party[index];
                    const auto bounded_byte = [&read_integer](
                                                  const int table_index,
                                                  const char* name) {
                        return static_cast<std::uint8_t>(
                            std::clamp<lua_Integer>(
                                read_integer(table_index, name, 0), 0, 0xFF));
                    };
                    merge_runner_menu_facts(
                        lua, member_index, original_party[index].status,
                        member.status);
                    // Race and class are host-owned profile facts. Lua rules
                    // can inspect their named values but cannot mutate them.
                    member.race = original_party[index].race;
                    member.character_class =
                        original_party[index].character_class;
                    member.clips = bounded_byte(member_index, "clips");
                    member.rounds = bounded_byte(member_index, "rounds");
                    member.weapon_slot = original_party[index].weapon_slot;
                    member.weapon_code = original_party[index].weapon_code;
                    lua_getfield(lua, member_index, "equipped_weapon_id");
                    const auto weapon_id_type = lua_type(lua, -1);
                    std::string weapon_item_id;
                    const bool has_weapon_item_id =
                        weapon_id_type == LUA_TSTRING &&
                        read_string(lua, -1, weapon_item_id);
                    lua_pop(lua, 1);
                    if (has_weapon_item_id) {
                        std::uint32_t weapon_column = 0U;
                        std::uint32_t weapon_row = 0U;
                        const bool has_weapon_cell = read_inventory_cell(
                            lua, member_index, "equipped_weapon_cell",
                            weapon_column, weapon_row);
                        const auto weapon_code =
                            script_item_source_code_from_id(weapon_item_id);
                        if (weapon_code.has_value() && has_weapon_cell) {
                            member.weapon_slot = static_cast<int>(
                                weapon_row * 2U + weapon_column);
                            member.weapon_code = *weapon_code;
                        }
                    } else if (weapon_id_type == LUA_TNIL) {
                        lua_getfield(lua, member_index, "selected_spell_id");
                        const auto spell_id_type = lua_type(lua, -1);
                        std::string spell_id;
                        const bool has_spell_id =
                            spell_id_type == LUA_TSTRING &&
                            read_string(lua, -1, spell_id);
                        lua_pop(lua, 1);
                        if (has_spell_id) {
                            const auto action_code =
                                script_spell_action_code_from_id(spell_id);
                            if (action_code.has_value()) {
                                member.weapon_slot = -1;
                                member.weapon_code = *action_code;
                            }
                        } else if (spell_id_type == LUA_TNIL) {
                            // E.WeaponSelectionState.EMPTY makes an explicit
                            // unequip distinct from a source record whose
                            // weapon code has no semantic item ID. Keep the
                            // latter's +0x56/+0x57 bytes unchanged on a
                            // round-trip through Lua.
                            lua_getfield(lua, member_index,
                                         "equipped_weapon_state");
                            std::string weapon_state;
                            const bool explicitly_empty =
                                lua_type(lua, -1) == LUA_TSTRING &&
                                read_string(lua, -1, weapon_state) &&
                                weapon_state == "empty";
                            lua_pop(lua, 1);
                            if (explicitly_empty) {
                                member.weapon_slot = -1;
                                member.weapon_code = 0U;
                            }
                        }
                    }
                    member.armor = original_party[index].armor;
                    lua_getfield(lua, member_index, "equipped_armor_id");
                    const auto armor_id_type = lua_type(lua, -1);
                    std::string armor_item_id;
                    const bool has_armor_item_id =
                        armor_id_type == LUA_TSTRING &&
                        read_string(lua, -1, armor_item_id);
                    lua_pop(lua, 1);
                    if (has_armor_item_id) {
                        const auto armor_index =
                            script_armor_source_index_from_item_id(
                                armor_item_id);
                        if (armor_index.has_value()) {
                            member.armor = *armor_index;
                        }
                    } else if (armor_id_type == LUA_TNIL) {
                        member.armor = 0U;
                    }
                    member.physical =
                        bounded_byte(member_index, "physical");
                    member.stun = bounded_byte(member_index, "stun");
                    member.magic_resource =
                        bounded_byte(member_index, "magic");
                    member.karma = bounded_byte(member_index, "karma");
                    member.magic_force =
                        bounded_byte(member_index, "force");
                    member.posture =
                        bounded_byte(member_index, "posture");
                    member.magic_table_index =
                        original_party[index].magic_table_index;
                    member.state_function = original_party[index].state_function;
                    std::string continuation;
                    lua_getfield(lua, member_index,
                                 "first_aid_continuation");
                    const bool has_continuation =
                        lua_type(lua, -1) == LUA_TSTRING &&
                        read_string(lua, -1, continuation);
                    lua_pop(lua, 1);
                    if (has_continuation && continuation == "idle") {
                        member.state_function = 0xB45CU;
                    }
                    auto equipment_codes = original_party[index].inventory;
                    auto equipment_values = original_party[index].phase;
                    if (read_menu_equipment(
                            lua, member_index, equipment_codes,
                            equipment_values,
                            original_party[index].inventory,
                            original_party[index].phase)) {
                        member.inventory = equipment_codes;
                        member.phase = equipment_values;
                    } else {
                        member.inventory = original_party[index].inventory;
                        member.phase = original_party[index].phase;
                    }
                    member.attributes = original_party[index].attributes;
                    lua_getfield(lua, member_index, "attributes");
                    if (lua_istable(lua, -1)) {
                        const auto attributes_index = lua_absindex(lua, -1);
                        member.attributes.body = read_stat(
                            attributes_index, "body", member.attributes.body);
                        member.attributes.quickness = read_stat(
                            attributes_index, "quickness",
                            member.attributes.quickness);
                        member.attributes.strength = read_stat(
                            attributes_index, "strength",
                            member.attributes.strength);
                        member.attributes.charisma = read_stat(
                            attributes_index, "charisma",
                            member.attributes.charisma);
                        member.attributes.intelligence = read_stat(
                            attributes_index, "intelligence",
                            member.attributes.intelligence);
                        member.attributes.willpower = read_stat(
                            attributes_index, "willpower",
                            member.attributes.willpower);
                    }
                    lua_pop(lua, 1);
                    member.skills = original_party[index].skills;
                    lua_getfield(lua, member_index, "skills");
                    if (lua_istable(lua, -1)) {
                        const auto skills_index = lua_absindex(lua, -1);
                        member.skills.sorcery = read_stat(
                            skills_index, "sorcery", member.skills.sorcery);
                        member.skills.firearms = read_stat(
                            skills_index, "firearms", member.skills.firearms);
                        member.skills.pistols = read_stat(
                            skills_index, "pistols", member.skills.pistols);
                        member.skills.smgs = read_stat(
                            skills_index, "smgs", member.skills.smgs);
                        member.skills.shotguns = read_stat(
                            skills_index, "shotguns", member.skills.shotguns);
                        member.skills.melee_combat = read_stat(
                            skills_index, "melee_combat",
                            member.skills.melee_combat);
                        member.skills.throwing = read_stat(
                            skills_index, "throwing", member.skills.throwing);
                        member.skills.computer = read_stat(
                            skills_index, "computer", member.skills.computer);
                        member.skills.biotech = read_stat(
                            skills_index, "biotech", member.skills.biotech);
                        member.skills.electronics = read_stat(
                            skills_index, "electronics",
                            member.skills.electronics);
                        member.skills.reputation = read_stat(
                            skills_index, "reputation",
                            member.skills.reputation);
                        member.skills.negotiation = read_stat(
                            skills_index, "negotiation",
                            member.skills.negotiation);
                    }
                    lua_pop(lua, 1);
                }
                lua_pop(lua, 1);
            }
            self->menu_route_bindings.game.commit_party(party);
            return 0;
        }
        case MenuBinding::roll_successes: {
            const auto successes = self->menu_route_bindings.game.roll_successes
                                       ? self->menu_route_bindings.game
                                             .roll_successes(
                                                 integer_argument(1),
                                                 integer_argument(2))
                                       : 0;
            lua_pushinteger(lua, successes);
            return 1;
        }
        case MenuBinding::pocket_extra_row:
            lua_pushboolean(
                lua, self->menu_route_bindings.game.pocket_extra_row &&
                             self->menu_route_bindings.game.pocket_extra_row()
                         ? 1
                         : 0);
            return 1;
        case MenuBinding::in_combat:
            lua_pushboolean(
                lua, self->menu_route_bindings.game.in_combat &&
                             self->menu_route_bindings.game.in_combat()
                         ? 1
                         : 0);
            return 1;
        case MenuBinding::is_outside_area:
            lua_pushboolean(
                lua, !self->menu_route_bindings.game.is_outside_area ||
                         self->menu_route_bindings.game.is_outside_area());
            return 1;
        case MenuBinding::save_game:
            lua_pushboolean(
                lua, self->menu_route_bindings.game.save_game &&
                             self->menu_route_bindings.game.save_game(
                                 integer_argument(1))
                         ? 1
                         : 0);
            return 1;
        case MenuBinding::load_game:
            lua_pushboolean(
                lua, self->menu_route_bindings.game.load_game &&
                             self->menu_route_bindings.game.load_game(
                                 integer_argument(1))
                         ? 1
                         : 0);
            return 1;
        case MenuBinding::save_status:
            lua_pushinteger(
                lua, self->menu_route_bindings.game.save_status
                         ? self->menu_route_bindings.game.save_status(
                               integer_argument(1))
                         : 0);
            return 1;
        case MenuBinding::source_random_bound: {
            const auto bound = std::clamp(integer_argument(1), 0, 0xFFFF);
            const auto value =
                self->menu_route_bindings.game.source_random_bound
                    ? self->menu_route_bindings.game.source_random_bound(bound)
                    : 0;
            lua_pushinteger(lua, std::clamp(value, 0, 0xFFFF));
            return 1;
        }
        case MenuBinding::matrix_alert_random_bound: {
            const auto bound = std::clamp(integer_argument(1), 0, 0xFFFF);
            const auto value =
                self->menu_route_bindings.game.matrix_alert_random_bound
                    ? self->menu_route_bindings.game.matrix_alert_random_bound(
                          bound)
                    : 0;
            lua_pushinteger(lua, std::clamp(value, 0, 0xFFFF));
            return 1;
        }
        case MenuBinding::dismiss_runner:
            if (self->menu_route_bindings.game.dismiss_runner) {
                self->menu_route_bindings.game.dismiss_runner(
                    integer_argument(1));
            }
            return 0;
        case MenuBinding::group_item_ids: {
            const auto ids = self->menu_route_bindings.game.group_item_ids
                                 ? self->menu_route_bindings.game
                                       .group_item_ids()
                                 : std::nullopt;
            if (!ids.has_value()) {
                lua_pushnil(lua);
                return 1;
            }
            lua_createtable(lua, static_cast<int>(ids->size()), 0);
            for (std::size_t index = 0; index < ids->size(); ++index) {
                lua_pushinteger(lua, (*ids)[index]);
                lua_rawseti(lua, -2, static_cast<lua_Integer>(index + 1U));
            }
            return 1;
        }
        case MenuBinding::notebook_facts: {
            const auto facts = self->menu_route_bindings.game.notebook_facts
                                   ? self->menu_route_bindings.game
                                         .notebook_facts()
                                   : std::nullopt;
            if (!facts.has_value()) {
                lua_pushnil(lua);
                return 1;
            }
            lua_createtable(lua, 0, 8);
            lua_pushboolean(lua, facts->current_run_available ? 1 : 0);
            lua_setfield(lua, -2, "current_run");
            if (facts->event_gates.has_value()) {
                lua_createtable(lua,
                                static_cast<int>(facts->event_gates->size()),
                                0);
                for (std::size_t index = 0;
                     index < facts->event_gates->size(); ++index) {
                    lua_pushboolean(lua, (*facts->event_gates)[index] ? 1 : 0);
                    lua_rawseti(lua, -2,
                                static_cast<lua_Integer>(index + 1U));
                }
                lua_setfield(lua, -2, "gates");
            } else {
                lua_pushnil(lua);
                lua_setfield(lua, -2, "gates");
            }
            const auto push_optional_ids = [lua](
                const char* field,
                const std::optional<std::vector<int>>& values) {
                if (!values.has_value()) {
                    lua_pushnil(lua);
                    lua_setfield(lua, -2, field);
                    return;
                }
                lua_createtable(lua, static_cast<int>(values->size()), 0);
                for (std::size_t index = 0; index < values->size(); ++index) {
                    lua_pushinteger(lua,
                                    static_cast<lua_Integer>((*values)[index]));
                    lua_rawseti(lua, -2,
                                static_cast<lua_Integer>(index + 1U));
                }
                lua_setfield(lua, -2, field);
            };
            push_optional_ids("tip_ids", facts->tip_ids);
            push_optional_ids("referral_ids", facts->referral_ids);
            push_optional_ids("contact_ids", facts->contact_ids);
            push_optional_ids("shadowrunner_ids", facts->shadowrunner_ids);
            return 1;
        }
        case MenuBinding::cyberdeck_available: {
            const auto available =
                self->menu_route_bindings.game.cyberdeck_available
                    ? self->menu_route_bindings.game.cyberdeck_available()
                    : std::nullopt;
            if (available.has_value()) {
                lua_pushboolean(lua, *available ? 1 : 0);
            } else {
                lua_pushnil(lua);
            }
            return 1;
        }
        case MenuBinding::deck_enter:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::enter);
        case MenuBinding::deck_leave:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::leave);
        case MenuBinding::set_deck_mode:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::set_mode);
        case MenuBinding::matrix_online:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::matrix_online);
        case MenuBinding::matrix_node:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::matrix_node);
        case MenuBinding::alert:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::alert);
        case MenuBinding::world_effect_gate_negative:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::world_effect_gate_negative);
        case MenuBinding::matrix_node_discovered:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::matrix_node_discovered);
        case MenuBinding::matrix_world_state:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::matrix_world_state);
        case MenuBinding::commit_matrix_world_state:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::commit_matrix_world_state);
        case MenuBinding::deck_record:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::record);
        case MenuBinding::deck_slots:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::program_slots);
        case MenuBinding::deck_commit:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::commit);
        case MenuBinding::program_unloaded:
            return dispatch_cyberdeck_menu_binding(
                lua, self->menu_route_bindings.game,
                CyberdeckMenuBinding::program_unloaded);
        case MenuBinding::menu_b_policy: {
            const auto policy = self->menu_route_bindings.game.menu_b_policy
                ? self->menu_route_bindings.game.menu_b_policy()
                : std::string("rom");
            lua_pushlstring(lua, policy.data(), policy.size());
            return 1;
        }
        case MenuBinding::nuyen: {
            const auto funds = self->menu_route_bindings.game.nuyen
                                   ? self->menu_route_bindings.game.nuyen()
                                   : std::nullopt;
            if (funds.has_value()) {
                lua_pushinteger(lua, static_cast<lua_Integer>(*funds));
            } else {
                lua_pushnil(lua);
            }
            return 1;
        }
        case MenuBinding::set_nuyen:
            if (self->menu_route_bindings.game.set_nuyen) {
                self->menu_route_bindings.game.set_nuyen(
                    static_cast<std::uint32_t>(
                        std::max(integer_argument(1), 0)));
            }
            return 0;
        case MenuBinding::begin_taxi_cabbie: {
            std::string region_id;
            if (!read_string(lua, 1, region_id)) {
                return luaL_error(
                    lua, "game.begin_taxi_cabbie expects a named region");
            }
            const auto region = script_field_region_from_id(region_id);
            if (!region.has_value()) {
                return luaL_error(
                    lua, "game.begin_taxi_cabbie received an unknown region");
            }
            const bool accepted =
                self->menu_route_bindings.game.begin_taxi_cabbie &&
                self->menu_route_bindings.game.begin_taxi_cabbie(
                    *region);
            lua_pushboolean(lua, accepted ? 1 : 0);
            return 1;
        }
        case MenuBinding::consume_debug_unlock:
            if (self->menu_route_bindings.game.consume_debug_unlock) {
                self->menu_route_bindings.game.consume_debug_unlock();
            }
            return 0;
        case MenuBinding::seed_debug_contacts:
            if (self->menu_route_bindings.game.seed_debug_contacts) {
                self->menu_route_bindings.game.seed_debug_contacts();
            }
            return 0;
        case MenuBinding::set_walk_fast:
            if (self->menu_route_bindings.game.set_walk_fast) {
                self->menu_route_bindings.game.set_walk_fast();
            }
            return 0;
        case MenuBinding::install_test_deck:
            if (self->menu_route_bindings.game.install_test_deck) {
                self->menu_route_bindings.game.install_test_deck();
            }
            return 0;
        }
        lua_pushnil(lua);
        return 1;
    }

    void add_menu_binding(lua_State* lua, const int table_index,
                          const char* name, const MenuBinding binding) {
        lua_pushlightuserdata(lua, this);
        lua_pushinteger(lua, static_cast<lua_Integer>(binding));
        lua_pushcclosure(lua, menu_binding_dispatch, 2);
        lua_setfield(lua, table_index, name);
    }

    void push_menu_host() {
        lua_newtable(state);
        const auto table_index = lua_absindex(state, -1);
        add_menu_binding(state, table_index, "play_sound",
                         MenuBinding::play_sound);
        add_menu_binding(state, table_index, "stop_sound",
                         MenuBinding::stop_sound);
        add_menu_binding(state, table_index, "redraw", MenuBinding::redraw);
        add_menu_binding(state, table_index, "blank", MenuBinding::blank);
        add_menu_binding(state, table_index, "mask", MenuBinding::mask);
        add_menu_binding(state, table_index, "set_window",
                         MenuBinding::set_window);
        add_menu_binding(state, table_index, "set_screen",
                         MenuBinding::set_screen);
        add_menu_binding(state, table_index, "menu_closed",
                         MenuBinding::menu_closed);
        add_menu_binding(state, table_index, "legacy_open",
                         MenuBinding::legacy_open);
        add_menu_binding(state, table_index, "legacy_advance",
                         MenuBinding::legacy_advance);
        add_menu_binding(state, table_index, "notebook_boundary",
                         MenuBinding::notebook_boundary);
    }

    void push_menu_game() {
        lua_newtable(state);
        const auto table_index = lua_absindex(state, -1);
        add_menu_binding(state, table_index, "reselect_leader",
                         MenuBinding::reselect_leader);
        add_menu_binding(state, table_index, "leader_posture",
                         MenuBinding::leader_posture);
        add_menu_binding(state, table_index, "set_leader_posture",
                         MenuBinding::set_leader_posture);
        add_menu_binding(state, table_index, "leader_class",
                         MenuBinding::leader_class);
        add_menu_binding(state, table_index, "party", MenuBinding::party);
        add_menu_binding(state, table_index, "spell_table",
                         MenuBinding::spell_table);
        add_menu_binding(state, table_index, "set_spell_table",
                         MenuBinding::set_spell_table);
        add_menu_binding(state, table_index, "active_slot",
                         MenuBinding::active_slot);
        add_menu_binding(state, table_index, "magazine_capacity",
                         MenuBinding::magazine_capacity);
        add_menu_binding(state, table_index, "set_runner_ammo",
                         MenuBinding::set_runner_ammo);
        add_menu_binding(state, table_index, "slot_active_set",
                         MenuBinding::slot_active_set);
        add_menu_binding(state, table_index, "commit_party",
                         MenuBinding::commit_party);
        add_menu_binding(state, table_index, "roll_successes",
                         MenuBinding::roll_successes);
        add_menu_binding(state, table_index, "pocket_extra_row",
                         MenuBinding::pocket_extra_row);
        add_menu_binding(state, table_index, "in_combat",
                         MenuBinding::in_combat);
        add_menu_binding(state, table_index, "is_outside_area",
                         MenuBinding::is_outside_area);
        add_menu_binding(state, table_index, "save_game",
                         MenuBinding::save_game);
        add_menu_binding(state, table_index, "load_game",
                         MenuBinding::load_game);
        add_menu_binding(state, table_index, "save_status",
                         MenuBinding::save_status);
        add_menu_binding(state, table_index, "source_random_bound",
                         MenuBinding::source_random_bound);
        add_menu_binding(state, table_index, "matrix_alert_random_bound",
                         MenuBinding::matrix_alert_random_bound);
        add_menu_binding(state, table_index, "dismiss_runner",
                         MenuBinding::dismiss_runner);
        add_menu_binding(state, table_index, "group_item_ids",
                         MenuBinding::group_item_ids);
        add_menu_binding(state, table_index, "notebook",
                         MenuBinding::notebook_facts);
        add_menu_binding(state, table_index, "cyberdeck_available",
                         MenuBinding::cyberdeck_available);
        add_menu_binding(state, table_index, "nuyen", MenuBinding::nuyen);
        add_menu_binding(state, table_index, "set_nuyen",
                         MenuBinding::set_nuyen);
        add_menu_binding(state, table_index, "begin_taxi_cabbie",
                         MenuBinding::begin_taxi_cabbie);
        add_menu_binding(state, table_index, "consume_debug_unlock",
                         MenuBinding::consume_debug_unlock);
        add_menu_binding(state, table_index, "seed_debug_contacts",
                         MenuBinding::seed_debug_contacts);
        add_menu_binding(state, table_index, "set_walk_fast",
                         MenuBinding::set_walk_fast);
        add_menu_binding(state, table_index, "install_test_deck",
                         MenuBinding::install_test_deck);
        add_menu_binding(state, table_index, "deck_enter",
                         MenuBinding::deck_enter);
        add_menu_binding(state, table_index, "deck_leave",
                         MenuBinding::deck_leave);
        add_menu_binding(state, table_index, "set_deck_mode",
                         MenuBinding::set_deck_mode);
        add_menu_binding(state, table_index, "matrix_online",
                         MenuBinding::matrix_online);
        add_menu_binding(state, table_index, "matrix_node",
                         MenuBinding::matrix_node);
        add_menu_binding(state, table_index, "alert", MenuBinding::alert);
        add_menu_binding(state, table_index,
                         "world_effect_gate_negative",
                         MenuBinding::world_effect_gate_negative);
        add_menu_binding(state, table_index, "matrix_node_discovered",
                         MenuBinding::matrix_node_discovered);
        add_menu_binding(state, table_index, "matrix_world_state",
                         MenuBinding::matrix_world_state);
        add_menu_binding(state, table_index, "commit_matrix_world_state",
                         MenuBinding::commit_matrix_world_state);
        add_menu_binding(state, table_index, "deck_record",
                         MenuBinding::deck_record);
        add_menu_binding(state, table_index, "deck_slots",
                         MenuBinding::deck_slots);
        add_menu_binding(state, table_index, "deck_commit",
                         MenuBinding::deck_commit);
        add_menu_binding(state, table_index, "program_unloaded",
                         MenuBinding::program_unloaded);
        add_menu_binding(state, table_index, "menu_b_policy",
                         MenuBinding::menu_b_policy);
    }

    bool refresh_menu_route_snapshot() {
        menu_route_state = ScriptMenuRouteSnapshot{};
        if (state == nullptr || menu_route_reference == LUA_NOREF) {
            return false;
        }
        const auto base = lua_gettop(state);
        lua_rawgeti(state, LUA_REGISTRYINDEX, menu_route_reference);
        const auto route_index = lua_absindex(state, -1);
        lua_getfield(state, route_index, "is_open");
        lua_pushvalue(state, route_index);
        if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
            failure = "menu route state query failed: " + stack_error(state);
            lua_settop(state, base);
            return false;
        }
        menu_route_state.open = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        lua_getfield(state, route_index, "field_return_transition_snapshot");
        if (lua_isfunction(state, -1)) {
            lua_pushvalue(state, route_index);
            if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
                failure = "menu route field return transition query failed: " +
                          stack_error(state);
                lua_settop(state, base);
                return false;
            }
            if (lua_istable(state, -1)) {
                const auto transition_index = lua_absindex(state, -1);
                lua_getfield(state, transition_index, "active");
                menu_route_state.field_return_transition_active =
                    lua_toboolean(state, -1) != 0;
                lua_pop(state, 1);
                auto* lua_context = state;
                const auto read_transition_integer =
                    [lua_context, transition_index](const char* field)
                    -> std::optional<lua_Integer> {
                    lua_getfield(lua_context, transition_index, field);
                    int is_integer = 0;
                    const auto value =
                        lua_tointegerx(lua_context, -1, &is_integer);
                    lua_pop(lua_context, 1);
                    return is_integer ? std::optional<lua_Integer>(value)
                                      : std::nullopt;
                };
                const auto elapsed =
                    read_transition_integer("elapsed_frames");
                if (elapsed.has_value() && *elapsed >= 0 &&
                    *elapsed <= static_cast<lua_Integer>(
                                    std::numeric_limits<std::uint16_t>::max())) {
                    menu_route_state.field_return_transition_elapsed_frames =
                        static_cast<std::uint16_t>(*elapsed);
                }
                const auto palette_step =
                    read_transition_integer("palette_step");
                if (palette_step.has_value() && *palette_step >= 0 &&
                    *palette_step <= static_cast<lua_Integer>(
                                         std::numeric_limits<std::uint8_t>::max())) {
                    menu_route_state.field_return_transition_palette_step =
                        static_cast<std::uint8_t>(*palette_step);
                }
                const auto phase = read_transition_integer("phase");
                if (phase.has_value() && *phase >= static_cast<lua_Integer>(
                                             ScriptFieldReturnTransitionPhase::none) &&
                    *phase <= static_cast<lua_Integer>(
                                  ScriptFieldReturnTransitionPhase::complete)) {
                    menu_route_state.field_return_transition_phase =
                        static_cast<ScriptFieldReturnTransitionPhase>(*phase);
                }
            }
            lua_pop(state, 1);
        } else {
            lua_pop(state, 1);
        }
        if (!menu_route_state.open) {
            lua_settop(state, base);
            return true;
        }
        auto* const lua = state;
        const auto read_optional = [lua](const int table_index,
                                         const char* name)
            -> std::optional<int> {
            lua_getfield(lua, table_index, name);
            int is_number = 0;
            const auto value = lua_tointegerx(lua, -1, &is_number);
            lua_pop(lua, 1);
            return is_number
                       ? std::optional<int>(static_cast<int>(value))
                       : std::nullopt;
        };
        const auto read_optional_alias = [&read_optional](
                                             const int table_index,
                                             const char* primary,
                                             const char* alias)
            -> std::optional<int> {
            const auto value = read_optional(table_index, primary);
            return value.has_value() ? value
                                     : read_optional(table_index, alias);
        };
        const auto read_integer_array = [lua](
            const int table_index, const char* name,
            const std::size_t maximum_count, const int minimum,
            const int maximum) {
            std::vector<int> values;
            lua_getfield(lua, table_index, name);
            if (lua_istable(lua, -1)) {
                const auto array_index = lua_absindex(lua, -1);
                const auto count = std::min<std::size_t>(
                    static_cast<std::size_t>(lua_rawlen(lua, array_index)),
                    maximum_count);
                values.reserve(count);
                for (std::size_t index = 0; index < count; ++index) {
                    lua_rawgeti(lua, array_index,
                                static_cast<lua_Integer>(index + 1U));
                    if (lua_isinteger(lua, -1)) {
                        const auto value = lua_tointeger(lua, -1);
                        if (value >= minimum && value <= maximum) {
                            values.push_back(static_cast<int>(value));
                        }
                    }
                    lua_pop(lua, 1);
                }
            }
            lua_pop(lua, 1);
            return values;
        };
        const auto read_string_array = [lua](
            const int table_index, const char* name,
            const std::size_t maximum_count) {
            std::vector<std::string> values;
            lua_getfield(lua, table_index, name);
            if (lua_istable(lua, -1)) {
                const auto array_index = lua_absindex(lua, -1);
                const auto count = std::min<std::size_t>(
                    static_cast<std::size_t>(lua_rawlen(lua, array_index)),
                    maximum_count);
                values.reserve(count);
                for (std::size_t index = 0U; index < count; ++index) {
                    lua_rawgeti(lua, array_index,
                                static_cast<lua_Integer>(index + 1U));
                    std::string value;
                    if (read_string(lua, -1, value)) {
                        values.push_back(std::move(value));
                    }
                    lua_pop(lua, 1);
                }
            }
            lua_pop(lua, 1);
            return values;
        };
        const auto read_text_array = [lua](
            const int table_index, const char* name,
            const std::size_t maximum_count)
            -> std::optional<std::vector<std::string>> {
            std::vector<std::string> values;
            lua_getfield(lua, table_index, name);
            if (!lua_istable(lua, -1)) {
                lua_pop(lua, 1);
                return std::nullopt;
            }
            const auto array_index = lua_absindex(lua, -1);
            const auto count = static_cast<std::size_t>(
                lua_rawlen(lua, array_index));
            if (count == 0U || count > maximum_count) {
                lua_pop(lua, 1);
                return std::nullopt;
            }
            values.reserve(count);
            for (std::size_t index = 0U; index < count; ++index) {
                lua_rawgeti(lua, array_index,
                            static_cast<lua_Integer>(index + 1U));
                std::string value;
                const bool valid = read_string_allow_empty(lua, -1, value);
                lua_pop(lua, 1);
                if (!valid) {
                    lua_pop(lua, 1);
                    return std::nullopt;
                }
                values.push_back(std::move(value));
            }
            lua_pop(lua, 1);
            return values;
        };
        lua_getfield(state, route_index, "top");
        lua_pushvalue(state, route_index);
        if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
            failure = "menu route top query failed: " + stack_error(state);
            lua_settop(state, base);
            menu_route_state = ScriptMenuRouteSnapshot{};
            return false;
        }
        if (lua_istable(state, -1)) {
            const auto entry_index = lua_absindex(state, -1);
            lua_getfield(state, entry_index, "debug_enter_callback_present");
            if (lua_isboolean(state, -1)) {
                menu_route_state.top_enter_callback_present =
                    lua_toboolean(state, -1) != 0;
            }
            lua_pop(state, 1);
            lua_getfield(state, entry_index, "debug_enter_called");
            if (lua_isboolean(state, -1)) {
                menu_route_state.top_enter_called =
                    lua_toboolean(state, -1) != 0;
            }
            lua_pop(state, 1);
            menu_route_state.top_enter_transition =
                read_optional(entry_index, "debug_enter_transition");
            lua_getfield(state, entry_index, "screen");
            if (lua_isinteger(state, -1)) {
                menu_route_state.top_screen = static_cast<std::uint16_t>(
                    lua_tointeger(state, -1));
            }
            lua_pop(state, 1);
            lua_getfield(state, entry_index, "state");
            if (lua_istable(state, -1)) {
                const auto screen_state_index = lua_absindex(state, -1);
                menu_route_state.posture_bar_selection =
                    read_optional(screen_state_index,
                                  "posture_bar_selection");
                lua_getfield(state, screen_state_index, "title");
                (void)read_string(state, -1, menu_route_state.top_title);
                lua_pop(state, 1);
                (void)read_field_text(state, screen_state_index, "text",
                                      menu_route_state.top_text);
                (void)read_field_text(state, screen_state_index,
                                      "footer_text",
                                      menu_route_state.top_footer_text);
                (void)read_field_text(
                    state, screen_state_index, "description",
                    menu_route_state.top_matrix_deck_description);
                lua_getfield(state, screen_state_index, "layout");
                if (lua_istable(state, -1)) {
                    const auto layout_index = lua_absindex(state, -1);
                    ScriptMenuRouteLayout layout;
                    layout.title_x = read_optional(layout_index, "title_x");
                    layout.title_y = read_optional(layout_index, "title_y");
                    layout.separator_x =
                        read_optional(layout_index, "separator_x");
                    layout.separator_y =
                        read_optional(layout_index, "separator_y");
                    layout.label_x = read_optional(layout_index, "label_x");
                    layout.label_y = read_optional(layout_index, "label_y");
                    layout.cursor_x = read_optional(layout_index, "cursor_x");
                    layout.cursor_y = read_optional(layout_index, "cursor_y");
                    layout.row_spacing =
                        read_optional(layout_index, "row_spacing");
                    menu_route_state.top_layout = std::move(layout);
                }
                lua_pop(state, 1);
                if (menu_route_state.top_screen == 41U) {
                    lua_getfield(state, screen_state_index, "view");
                    if (lua_istable(state, -1)) {
                        const auto view_index = lua_absindex(state, -1);
                        ScriptNotebookCurrentRunView view;
                        const bool variant_valid = read_field_string(
                            state, view_index, "offer_variant_id",
                            view.offer_variant_id) &&
                            valid_identifier(view.offer_variant_id);
                        const bool run_identity_valid =
                            read_field_string(state, view_index, "contact_id",
                                              view.contact_id) &&
                            valid_run_contact_id(view.contact_id) &&
                            read_field_string(state, view_index, "run_id",
                                              view.run_id) &&
                            valid_identifier(view.run_id);
                        const auto payment = read_optional(
                            view_index, "payment_amount");
                        const auto body_lines = read_text_array(
                            view_index, "body_lines", 16U);
                        const bool labels_valid =
                            read_field_string(state, view_index,
                                              "johnson_label",
                                              view.johnson_label) &&
                            read_field_string(state, view_index,
                                              "johnson_name",
                                              view.johnson_name) &&
                            read_field_string(state, view_index,
                                              "run_type_label",
                                              view.run_type_label) &&
                            read_field_string(state, view_index,
                                              "run_type_name",
                                              view.run_type_name) &&
                            read_field_string(state, view_index,
                                              "payment_label",
                                              view.payment_label) &&
                            read_field_string(state, view_index,
                                              "currency_symbol",
                                              view.currency_symbol);
                        if (variant_valid &&
                            run_identity_valid &&
                            payment.has_value() && *payment >= 0 &&
                            *payment <= 0xFFFF && labels_valid &&
                            body_lines.has_value() &&
                            !body_lines->empty()) {
                            view.payment_amount = *payment;
                            view.body_lines = *body_lines;
                            menu_route_state.top_notebook_current_run =
                                std::move(view);
                        }
                    }
                    lua_pop(state, 1);
                }
                const auto detail_labels = read_string_array(
                    screen_state_index, "program_detail_labels", 3U);
                if (detail_labels.size() ==
                    menu_route_state.top_matrix_deck_detail_labels.size()) {
                    std::copy(detail_labels.begin(), detail_labels.end(),
                              menu_route_state.top_matrix_deck_detail_labels.begin());
                    menu_route_state.top_matrix_deck_detail_labels_present =
                        true;
                }
                const auto detail_values = read_string_array(
                    screen_state_index, "program_detail_values", 3U);
                if (detail_values.size() ==
                    menu_route_state.top_matrix_deck_program_detail_values
                        .size()) {
                    std::copy(
                        detail_values.begin(), detail_values.end(),
                        menu_route_state
                            .top_matrix_deck_program_detail_values.begin());
                    menu_route_state
                        .top_matrix_deck_program_detail_values_present = true;
                }
                const auto runner_detail_labels = read_string_array(
                    screen_state_index, "runner_detail_labels", 2U);
                if (runner_detail_labels.size() ==
                    menu_route_state.top_matrix_deck_runner_detail_labels
                        .size()) {
                    std::copy(
                        runner_detail_labels.begin(),
                        runner_detail_labels.end(),
                        menu_route_state
                            .top_matrix_deck_runner_detail_labels.begin());
                    menu_route_state
                        .top_matrix_deck_runner_detail_labels_present = true;
                }
                const auto runner_details = read_string_array(
                    screen_state_index, "runner_detail_lines", 2U);
                if (runner_details.size() ==
                    menu_route_state.top_matrix_deck_runner_details.size()) {
                    std::copy(runner_details.begin(), runner_details.end(),
                              menu_route_state.top_matrix_deck_runner_details.begin());
                    menu_route_state.top_matrix_deck_runner_details_present =
                        true;
                }
                lua_getfield(state, screen_state_index, "labels");
                if (lua_istable(state, -1)) {
                    const auto labels_index = lua_absindex(state, -1);
                    const auto label_count = std::min<std::size_t>(
                        static_cast<std::size_t>(
                            lua_rawlen(state, labels_index)),
                        16U);
                    for (std::size_t index = 0; index < label_count;
                         ++index) {
                        lua_rawgeti(state, labels_index,
                                    static_cast<lua_Integer>(index + 1U));
                        std::string label;
                        if (read_string(state, -1, label)) {
                            menu_route_state.top_option_labels.push_back(
                                std::move(label));
                        }
                        lua_pop(state, 1);
                    }
                }
                lua_pop(state, 1);
                menu_route_state.top_column =
                    read_optional_alias(screen_state_index, "col", "column");
                menu_route_state.top_row =
                    read_optional(screen_state_index, "row");
                menu_route_state.top_matrix_deck_parent_column =
                    read_optional(screen_state_index, "deck_parent_col");
                menu_route_state.top_matrix_deck_parent_row =
                    read_optional(screen_state_index, "deck_parent_row");
                lua_getfield(state, screen_state_index,
                             "deck_parent_program_name");
                menu_route_state
                    .top_matrix_deck_parent_program_name_present =
                    read_string(
                        state, -1,
                        menu_route_state.top_matrix_deck_parent_program_name);
                lua_pop(state, 1);
                const auto storage_panel_lines = read_string_array(
                    screen_state_index, "storage_panel_lines", 6U);
                if (storage_panel_lines.size() ==
                    menu_route_state.top_matrix_storage_panel_lines.size()) {
                    std::copy(
                        storage_panel_lines.begin(), storage_panel_lines.end(),
                        menu_route_state.top_matrix_storage_panel_lines.begin());
                    menu_route_state.top_matrix_storage_panel_lines_present =
                        true;
                }
                menu_route_state.top_matrix_storage_total =
                    read_optional(screen_state_index, "storage_total");
                menu_route_state.top_matrix_storage_available =
                    read_optional(screen_state_index, "storage_available");
                (void)read_field_string(
                    state, screen_state_index, "phase",
                    menu_route_state.top_matrix_storage_phase);
                (void)read_field_string(
                    state, screen_state_index, "storage_separator",
                    menu_route_state.top_matrix_storage_separator);
                menu_route_state.top_matrix_storage_file_names =
                    read_string_array(screen_state_index,
                                      "storage_file_names", 5U);
                menu_route_state.top_matrix_storage_file_sizes =
                    read_integer_array(screen_state_index,
                                       "storage_file_sizes", 5U, 0,
                                       0xFFFF);
                lua_getfield(state, screen_state_index,
                             "storage_erase_name");
                menu_route_state.top_matrix_storage_erase_name_present =
                    read_string(
                        state, -1,
                        menu_route_state.top_matrix_storage_erase_name);
                lua_pop(state, 1);
                menu_route_state.top_matrix_storage_erase_size =
                    read_optional(screen_state_index,
                                  "storage_erase_size");
                menu_route_state.top_matrix_storage_empty_message_lines =
                    read_string_array(screen_state_index,
                                      "storage_empty_message_lines", 2U);
                const auto deck_palette_phase =
                    read_optional(screen_state_index,
                                  "deck_palette_phase");
                if (deck_palette_phase.has_value() &&
                    *deck_palette_phase >= 0 && *deck_palette_phase < 4) {
                    menu_route_state.top_matrix_deck_palette_phase =
                        deck_palette_phase;
                }
                lua_getfield(state, screen_state_index, "program_ratings");
                if (lua_istable(state, -1)) {
                    const auto ratings_index = lua_absindex(state, -1);
                    bool complete_ratings = true;
                    for (std::size_t index = 0U;
                         index < menu_route_state
                                     .top_matrix_deck_program_ratings.size();
                         ++index) {
                        lua_rawgeti(state, ratings_index,
                                    static_cast<lua_Integer>(index));
                        if (!lua_isinteger(state, -1)) {
                            complete_ratings = false;
                        } else {
                            const auto rating = lua_tointeger(state, -1);
                            if (rating < 0 || rating > 0xFF) {
                                complete_ratings = false;
                            } else {
                                menu_route_state
                                    .top_matrix_deck_program_ratings[index] =
                                    static_cast<std::uint8_t>(rating);
                            }
                        }
                        lua_pop(state, 1);
                    }
                    menu_route_state
                        .top_matrix_deck_program_ratings_present =
                        complete_ratings;
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index,
                             "active_programs");
                if (lua_istable(state, -1)) {
                    const auto active_index = lua_absindex(state, -1);
                    bool complete_active = true;
                    for (std::size_t index = 0U;
                         index < menu_route_state
                                     .top_matrix_deck_active_programs.size();
                         ++index) {
                        lua_rawgeti(state, active_index,
                                    static_cast<lua_Integer>(index));
                        if (!lua_isinteger(state, -1)) {
                            complete_active = false;
                        } else {
                            const auto program = lua_tointeger(state, -1);
                            if (program < -1 || program > 11) {
                                complete_active = false;
                            } else {
                                menu_route_state
                                    .top_matrix_deck_active_programs[index] =
                                    static_cast<std::int8_t>(program);
                            }
                        }
                        lua_pop(state, 1);
                    }
                    menu_route_state
                        .top_matrix_deck_active_programs_present =
                        complete_active;
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index,
                             "program_slot_status");
                if (lua_istable(state, -1)) {
                    const auto status_index = lua_absindex(state, -1);
                    bool complete_status = true;
                    for (std::size_t index = 0U;
                         index < menu_route_state
                                     .top_matrix_deck_program_slot_status.size();
                         ++index) {
                        lua_rawgeti(state, status_index,
                                    static_cast<lua_Integer>(index));
                        if (!lua_isinteger(state, -1)) {
                            complete_status = false;
                        } else {
                            const auto status = lua_tointeger(state, -1);
                            if (status < 0 || status > 6) {
                                complete_status = false;
                            } else {
                                menu_route_state
                                    .top_matrix_deck_program_slot_status[index] =
                                    static_cast<std::uint8_t>(status);
                            }
                        }
                        lua_pop(state, 1);
                    }
                    menu_route_state
                        .top_matrix_deck_program_slot_status_present =
                        complete_status;
                }
                lua_pop(state, 1);
                menu_route_state.top_page =
                    read_optional(screen_state_index, "page");
                menu_route_state.top_selection =
                    read_optional_alias(screen_state_index, "selection",
                                        "selector");
                (void)read_optional_field_string(
                    state, screen_state_index, "selection",
                    menu_route_state.top_selection_id);
                (void)read_optional_field_string(
                    state, screen_state_index, "selected_action",
                    menu_route_state.top_action_id);
                menu_route_state.top_program_index =
                    read_optional(screen_state_index, "p");
                menu_route_state.top_program_slot =
                    read_optional(screen_state_index, "slot");
                menu_route_state.top_program_choice =
                    read_optional(screen_state_index, "choice");
                lua_getfield(state, screen_state_index, "caption_visible");
                if (lua_isboolean(state, -1)) {
                    menu_route_state.top_caption_visible =
                        lua_toboolean(state, -1) != 0;
                }
                lua_pop(state, 1);
                std::string caption_presentation;
                if (read_optional_field_string(
                        state, screen_state_index, "caption_presentation",
                        caption_presentation) &&
                    caption_presentation == "white_on_black") {
                    menu_route_state.top_caption_presentation =
                        ScriptTaxiCaptionPresentation::white_on_black;
                }
                menu_route_state.top_command =
                    read_optional(screen_state_index, "command");
                menu_route_state.top_quantity =
                    read_optional(screen_state_index, "quantity");
                menu_route_state.top_message =
                    read_optional(screen_state_index, "message");
                menu_route_state.top_source =
                    read_optional(screen_state_index, "source");
                menu_route_state.top_recipient =
                    read_optional(screen_state_index, "recipient");
                std::uint32_t destination_column = 0U;
                std::uint32_t destination_row = 0U;
                if (read_inventory_cell(state, screen_state_index, "dst_cell",
                                        destination_column,
                                        destination_row)) {
                    menu_route_state.top_destination_slot =
                        static_cast<int>(destination_row * 2U +
                                         destination_column);
                }
                menu_route_state.top_target =
                    read_optional(screen_state_index, "target");
                menu_route_state.top_grid_effect_frames =
                    read_optional(screen_state_index, "grid_effect_frames");
                lua_getfield(state, screen_state_index, "matrix_state");
                if (lua_istable(state, -1)) {
                    const auto matrix_state_index = lua_absindex(state, -1);
                    menu_route_state.top_matrix_state_present = true;
                    menu_route_state.top_matrix_system_id =
                        read_optional(matrix_state_index, "system_id");
                    menu_route_state.top_matrix_node =
                        read_optional(matrix_state_index, "current_node");
                    menu_route_state.top_matrix_previous_node =
                        read_optional(matrix_state_index, "previous_node");
                    menu_route_state.top_matrix_alert =
                        read_optional(matrix_state_index, "alert");
                    lua_getfield(state, matrix_state_index, "phase");
                    (void)read_string_allow_empty(
                        state, -1, menu_route_state.top_matrix_phase);
                    lua_pop(state, 1);
                    lua_getfield(state, matrix_state_index,
                                 "pending_action");
                    (void)read_string_allow_empty(
                        state, -1,
                        menu_route_state.top_matrix_pending_action);
                    lua_pop(state, 1);
                    lua_getfield(state, matrix_state_index,
                                 "world_effect_gate_negative");
                    if (lua_isboolean(state, -1)) {
                        menu_route_state
                            .top_matrix_world_effect_gate_negative =
                            lua_toboolean(state, -1) != 0;
                    }
                    lua_pop(state, 1);
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index,
                             "matrix_node_status");
                if (lua_istable(state, -1)) {
                    const auto status_index = lua_absindex(state, -1);
                    ScriptMatrixNodeStatusView status;
                    lua_getfield(state, status_index, "file_class_label");
                    const bool has_file_class =
                        read_string_allow_empty(
                            state, -1, status.file_class_label);
                    lua_pop(state, 1);
                    lua_getfield(state, status_index, "alert_band_label");
                    const bool has_alert_band =
                        read_string(state, -1, status.alert_band_label);
                    lua_pop(state, 1);
                    const auto action_difficulty =
                        read_optional(status_index, "action_difficulty");
                    const auto background_profile =
                        read_optional(status_index, "background_profile");
                    const auto palette_theme =
                        read_optional(status_index, "palette_theme");
                    const auto world_model_id =
                        read_optional(status_index, "world_model_id");
                    if (has_file_class && has_alert_band &&
                        action_difficulty.has_value() &&
                        // Lua leaves the class label empty and uses -1 while
                        // node details are hidden below the ROM knowledge
                        // threshold. Preserve its palette/model fields so
                        // traversal can still render the world view.
                        *action_difficulty >= -1 &&
                        *action_difficulty <= 255 &&
                        background_profile.has_value() &&
                        *background_profile >= 0 &&
                        *background_profile <= 4 &&
                        palette_theme.has_value() &&
                        *palette_theme >= 1 && *palette_theme <= 4 &&
                        world_model_id.has_value() &&
                        *world_model_id >= 0 && *world_model_id <= 5) {
                        status.action_difficulty = *action_difficulty;
                        status.background_profile = *background_profile;
                        status.palette_theme = *palette_theme;
                        status.world_model_id =
                            static_cast<ScriptMatrixWorldModelId>(
                                *world_model_id);
                        menu_route_state.top_matrix_node_status =
                            std::move(status);
                    }
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index,
                             "matrix_traversal_view");
                if (lua_istable(state, -1)) {
                    const auto view_index = lua_absindex(state, -1);
                    ScriptMatrixTraversalView view;
                    const auto world_model_id =
                        read_optional(view_index, "world_model_id");
                    const auto palette_theme =
                        read_optional(view_index, "palette_theme");
                    const auto world_effect_mode =
                        read_optional(view_index, "world_effect_mode");
                    const auto world_effect_phase =
                        read_optional(view_index, "world_effect_phase");
                    const auto action_mode =
                        read_optional(view_index, "action_mode");
                    const auto analyze_effect_phase =
                        read_optional(view_index, "analyze_effect_phase");
                    const auto analyze_runner_phase =
                        read_optional(view_index, "analyze_runner_phase");
                    const auto world_entry_phase =
                        read_optional(view_index, "world_entry_phase");
                    const auto world_entry_setup_vblanks =
                        read_optional(view_index,
                                      "world_entry_setup_vblanks");
                    const auto world_entry_sequence_vblanks =
                        read_optional(view_index,
                                      "world_entry_sequence_vblanks");
                    const auto world_entry_sequence_duration =
                        read_optional(view_index,
                                      "world_entry_sequence_duration");
                    const auto world_entry_published_frame =
                        read_optional(view_index,
                                      "world_entry_published_frame");
                    const auto model_canvas_x =
                        read_optional(view_index, "model_canvas_x");
                    const auto model_canvas_y =
                        read_optional(view_index, "model_canvas_y");
                    const auto model_canvas_width =
                        read_optional(view_index, "model_canvas_width");
                    const auto model_canvas_height =
                        read_optional(view_index, "model_canvas_height");
                    const auto analysis_level =
                        read_optional(view_index, "analysis_level");
                    const auto action_difficulty =
                        read_optional(view_index, "action_difficulty");
                    const auto ice_rating_column =
                        read_optional(view_index, "ice_rating_column");
                    const auto second_ice_rating_column =
                        read_optional(view_index,
                                      "second_ice_rating_column");
                    lua_getfield(state, view_index, "function_label");
                    const bool has_function_label =
                        read_string_allow_empty(state, -1,
                                                view.function_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index, "file_class_label");
                    const bool has_file_class =
                        read_string_allow_empty(state, -1,
                                                view.file_class_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index, "alert_band_label");
                    const bool has_alert_band =
                        read_string_allow_empty(state, -1,
                                                view.alert_band_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index, "ice_label");
                    const bool has_ice_label =
                        read_string_allow_empty(state, -1, view.ice_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index, "ice_rating_label");
                    const bool has_ice_rating_label =
                        read_string_allow_empty(state, -1,
                                                view.ice_rating_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index, "second_ice_label");
                    const bool has_second_ice_label =
                        read_string_allow_empty(state, -1,
                                                view.second_ice_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index,
                                 "second_ice_rating_label");
                    const bool has_second_ice_rating_label =
                        read_string_allow_empty(
                        state, -1, view.second_ice_rating_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index,
                                 "action_difficulty_label");
                    const bool has_action_difficulty_label =
                        read_string_allow_empty(
                        state, -1, view.action_difficulty_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index, "alert_label");
                    const bool has_alert_label =
                        read_string_allow_empty(state, -1,
                                                view.alert_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index,
                                 "active_program_label");
                    const bool has_active_program_label =
                        read_string_allow_empty(state, -1,
                                                view.active_program_label);
                    lua_pop(state, 1);
                    lua_getfield(state, view_index,
                                 "active_program_status_label");
                    const bool has_active_program_status_label =
                        read_string_allow_empty(
                            state, -1,
                            view.active_program_status_label);
                    lua_pop(state, 1);
                    if (world_model_id.has_value() &&
                        *world_model_id >= 0 && *world_model_id <= 5 &&
                        palette_theme.has_value() &&
                        *palette_theme >= 0 && *palette_theme <= 4 &&
                        world_effect_mode.has_value() &&
                        (*world_effect_mode == -1 ||
                         (*world_effect_mode >= 0 &&
                          *world_effect_mode <= 8) ||
                         *world_effect_mode == 0x45) &&
                        world_effect_phase.has_value() &&
                        *world_effect_phase >= 0 &&
                        *world_effect_phase <= 27 &&
                        action_mode.has_value() && *action_mode >= -1 &&
                        *action_mode <= 1 &&
                        analyze_effect_phase.has_value() &&
                        *analyze_effect_phase >= -1 &&
                        *analyze_effect_phase <= 3 &&
                        analyze_runner_phase.has_value() &&
                        *analyze_runner_phase >= -1 &&
                        *analyze_runner_phase <= 2 &&
                        world_entry_phase.has_value() &&
                        *world_entry_phase >= 0 && *world_entry_phase <= 2 &&
                        world_entry_setup_vblanks.has_value() &&
                        *world_entry_setup_vblanks >= 0 &&
                        *world_entry_setup_vblanks <= 61 &&
                        world_entry_sequence_vblanks.has_value() &&
                        *world_entry_sequence_vblanks >= 0 &&
                        *world_entry_sequence_vblanks <= 239 &&
                        world_entry_sequence_duration.has_value() &&
                        (*world_entry_sequence_duration == 0 ||
                         *world_entry_sequence_duration == 179 ||
                         *world_entry_sequence_duration == 239) &&
                        world_entry_published_frame.has_value() &&
                        *world_entry_published_frame >= 0 &&
                        *world_entry_published_frame <= 56 &&
                        model_canvas_x.has_value() &&
                        *model_canvas_x >= 0 && *model_canvas_x < 320 &&
                        model_canvas_y.has_value() &&
                        *model_canvas_y >= 0 && *model_canvas_y < 224 &&
                        model_canvas_width.has_value() &&
                        *model_canvas_width >= 0 &&
                        *model_canvas_width <= 320 &&
                        model_canvas_height.has_value() &&
                        *model_canvas_height >= 0 &&
                        *model_canvas_height <= 224 &&
                        ((*model_canvas_width == 0 &&
                          *model_canvas_height == 0) ||
                         (*model_canvas_width > 0 &&
                          *model_canvas_height > 0 &&
                          *model_canvas_x + *model_canvas_width <= 320 &&
                          *model_canvas_y + *model_canvas_height <= 224)) &&
                        analysis_level.has_value() &&
                        *analysis_level >= 0 && *analysis_level <= 7 &&
                        action_difficulty.has_value() &&
                        *action_difficulty >= -1 &&
                        *action_difficulty <= 255 &&
                        ice_rating_column.has_value() &&
                        *ice_rating_column >= 0 &&
                        *ice_rating_column <= 15 &&
                        second_ice_rating_column.has_value() &&
                        *second_ice_rating_column >= 0 &&
                        *second_ice_rating_column <= 15 &&
                        has_function_label && has_file_class &&
                        has_alert_band && has_ice_label &&
                        has_ice_rating_label && has_second_ice_label &&
                        has_second_ice_rating_label &&
                        has_action_difficulty_label && has_alert_label &&
                        has_active_program_label &&
                        has_active_program_status_label) {
                        view.world_model_id =
                            static_cast<ScriptMatrixWorldModelId>(
                                *world_model_id);
                        view.palette_theme = *palette_theme;
                        view.world_effect_mode = *world_effect_mode;
                        view.world_effect_phase = *world_effect_phase;
                        view.action_mode = *action_mode;
                        view.analyze_effect_phase = *analyze_effect_phase;
                        view.analyze_runner_phase = *analyze_runner_phase;
                        view.world_entry_phase = *world_entry_phase;
                        view.world_entry_setup_vblanks =
                            *world_entry_setup_vblanks;
                        view.world_entry_sequence_vblanks =
                            *world_entry_sequence_vblanks;
                        view.world_entry_sequence_duration =
                            *world_entry_sequence_duration;
                        view.world_entry_published_frame =
                            *world_entry_published_frame;
                        view.model_canvas_x = *model_canvas_x;
                        view.model_canvas_y = *model_canvas_y;
                        view.model_canvas_width = *model_canvas_width;
                        view.model_canvas_height = *model_canvas_height;
                        view.analysis_level = *analysis_level;
                        view.action_difficulty = *action_difficulty;
                        view.ice_rating_column = *ice_rating_column;
                        view.second_ice_rating_column =
                            *second_ice_rating_column;
                        menu_route_state.top_matrix_traversal_view =
                            std::move(view);
                    }
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index,
                             "matrix_run_info");
                if (lua_istable(state, -1)) {
                    const auto info_index = lua_absindex(state, -1);
                    std::string kind;
                    lua_getfield(state, info_index, "kind");
                    const bool has_kind = read_string(state, -1, kind);
                    lua_pop(state, 1);

                    ScriptMatrixRunInfoView info;
                    if (has_kind && kind == "no_run") {
                        info.kind = ScriptMatrixRunInfoKind::no_run;
                        lua_getfield(state, info_index, "message");
                        if (read_string(state, -1, info.message)) {
                            menu_route_state.top_matrix_run_info =
                                std::move(info);
                        }
                        lua_pop(state, 1);
                    } else if (has_kind && kind == "complete") {
                        info.kind = ScriptMatrixRunInfoKind::complete;
                        lua_getfield(state, info_index, "message");
                        if (read_string(state, -1, info.message)) {
                            menu_route_state.top_matrix_run_info =
                                std::move(info);
                        }
                        lua_pop(state, 1);
                    } else if (has_kind && kind == "active") {
                        info.kind = ScriptMatrixRunInfoKind::active;
                        lua_getfield(state, info_index, "node_label");
                        const bool has_node_label =
                            read_string(state, -1, info.node_label);
                        lua_pop(state, 1);
                        lua_getfield(state, info_index, "node");
                        const bool has_node =
                            read_string(state, -1, info.node);
                        lua_pop(state, 1);
                        lua_getfield(state, info_index, "function_label");
                        const bool has_function_label =
                            read_string(state, -1, info.function_label);
                        lua_pop(state, 1);
                        lua_getfield(state, info_index, "node_function");
                        const bool has_node_function =
                            read_string(state, -1, info.node_function);
                        lua_pop(state, 1);
                        lua_getfield(state, info_index, "action_label");
                        const bool has_action_label =
                            read_string(state, -1, info.action_label);
                        lua_pop(state, 1);
                        lua_getfield(state, info_index, "action");
                        const bool has_action =
                            read_string(state, -1, info.action);
                        lua_pop(state, 1);
                        if (has_node_label && has_node &&
                            has_function_label && has_node_function &&
                            has_action_label && has_action) {
                            menu_route_state.top_matrix_run_info =
                                std::move(info);
                        }
                    }
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index,
                             "matrix_deck_stats");
                if (lua_istable(state, -1)) {
                    const auto stats_index = lua_absindex(state, -1);
                    ScriptMatrixDeckStatsView stats;
                    lua_getfield(state, stats_index, "manufacturer");
                    const bool has_manufacturer =
                        read_string(state, -1, stats.manufacturer);
                    lua_pop(state, 1);
                    lua_getfield(state, stats_index, "model");
                    const bool has_model = read_string(state, -1, stats.model);
                    lua_pop(state, 1);

                    bool has_labels = true;
                    lua_getfield(state, stats_index, "labels");
                    if (lua_istable(state, -1)) {
                        const auto labels_index = lua_absindex(state, -1);
                        for (std::size_t index = 0;
                             index < stats.labels.size(); ++index) {
                            lua_rawgeti(state, labels_index,
                                        static_cast<lua_Integer>(index + 1U));
                            has_labels = read_string(
                                             state, -1, stats.labels[index]) &&
                                         has_labels;
                            lua_pop(state, 1);
                        }
                    } else {
                        has_labels = false;
                    }
                    lua_pop(state, 1);

                    bool has_values = true;
                    lua_getfield(state, stats_index, "values");
                    if (lua_istable(state, -1)) {
                        const auto values_index = lua_absindex(state, -1);
                        for (std::size_t index = 0;
                             index < stats.values.size(); ++index) {
                            lua_rawgeti(state, values_index,
                                        static_cast<lua_Integer>(index + 1U));
                            int is_integer = 0;
                            const auto value = lua_tointegerx(
                                state, -1, &is_integer);
                            const bool valid =
                                is_integer != 0 && value >= 0 &&
                                value <= 0xFFFF;
                            has_values = valid && has_values;
                            if (valid) {
                                stats.values[index] =
                                    static_cast<unsigned int>(value);
                            }
                            lua_pop(state, 1);
                        }
                    } else {
                        has_values = false;
                    }
                    lua_pop(state, 1);

                    if (has_manufacturer && has_model && has_labels &&
                        has_values) {
                        menu_route_state.top_matrix_deck_stats =
                            std::move(stats);
                    }
                }
                lua_pop(state, 1);
                menu_route_state.top_matrix_countdown =
                    read_optional(screen_state_index, "matrix_countdown");
                lua_getfield(state, screen_state_index, "matrix_transition");
                if (lua_istable(state, -1)) {
                    const auto transition_index = lua_absindex(state, -1);
                    const auto fade_step =
                        read_optional(transition_index, "fade_step");
                    if (fade_step.has_value() && *fade_step >= 0 &&
                        *fade_step <= 5) {
                        menu_route_state.top_matrix_transition_fade_step =
                            *fade_step;
                    }
                    lua_getfield(state, transition_index, "visual");
                    if (lua_istable(state, -1)) {
                        const auto visual_index = lua_absindex(state, -1);
                        std::string phase;
                        lua_getfield(state, visual_index, "phase");
                        const bool has_phase =
                            read_string_allow_empty(state, -1, phase);
                        lua_pop(state, 1);

                        ScriptMatrixLogoutVisual visual;
                        bool valid_visual = has_phase;
                        if (phase == "fade") {
                            visual.phase =
                                ScriptMatrixLogoutVisualPhase::FADE;
                        } else if (phase == "cross") {
                            visual.phase =
                                ScriptMatrixLogoutVisualPhase::CROSS;
                            visual.horizontal_left =
                                read_optional(visual_index, "horizontal_left")
                                    .value_or(-1);
                            visual.horizontal_right =
                                read_optional(visual_index, "horizontal_right")
                                    .value_or(-1);
                            visual.vertical_top =
                                read_optional(visual_index, "vertical_top")
                                    .value_or(-1);
                            visual.vertical_bottom =
                                read_optional(visual_index, "vertical_bottom")
                                    .value_or(-1);
                            valid_visual =
                                valid_visual &&
                                visual.horizontal_left >= 0 &&
                                visual.horizontal_left <=
                                    visual.horizontal_right &&
                                visual.horizontal_right <= 320 &&
                                visual.vertical_top >= 0 &&
                                visual.vertical_top <=
                                    visual.vertical_bottom &&
                                visual.vertical_bottom <= 224;
                        } else if (phase == "dot") {
                            visual.phase =
                                ScriptMatrixLogoutVisualPhase::DOT;
                        } else if (phase == "sparkle") {
                            visual.phase =
                                ScriptMatrixLogoutVisualPhase::SPARKLE;
                        } else if (phase == "black") {
                            visual.phase =
                                ScriptMatrixLogoutVisualPhase::BLACK;
                        } else {
                            valid_visual = false;
                        }
                        if (valid_visual) {
                            menu_route_state.top_matrix_logout_visual = visual;
                        }
                    }
                    lua_pop(state, 1);
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "magic_view");
                if (lua_istable(state, -1)) {
                    const auto view_index = lua_absindex(state, -1);
                    ScriptMenuRouteSnapshot::MagicView view;
                    view.present = true;
                    view.row = read_optional(view_index, "row").value_or(0);
                    view.column =
                        read_optional(view_index, "column").value_or(0);
                    view.table_entry_index =
                        read_optional(view_index, "table_entry_index")
                            .value_or(0);
                    lua_getfield(state, view_index, "spell_id");
                    read_string(state, -1, view.spell_id);
                    lua_pop(state, 1);
                    view.title_icon_x =
                        read_optional(view_index, "title_icon_x").value_or(80);
                    view.title_icon_y =
                        read_optional(view_index, "title_icon_y").value_or(24);
                    view.title_text_x =
                        read_optional(view_index, "title_text_x").value_or(128);
                    view.title_text_y =
                        read_optional(view_index, "title_text_y").value_or(32);
                    view.spell_grid_x =
                        read_optional(view_index, "spell_grid_x").value_or(32);
                    view.spell_grid_y =
                        read_optional(view_index, "spell_grid_y").value_or(168);
                    view.spell_grid_column_step =
                        read_optional(view_index, "spell_grid_column_step")
                            .value_or(32);
                    view.spell_grid_row_step =
                        read_optional(view_index, "spell_grid_row_step")
                            .value_or(24);
                    view.spell_grid_icon_width =
                        read_optional(view_index, "spell_grid_icon_width")
                            .value_or(24);
                    view.spell_grid_icon_height =
                        read_optional(view_index, "spell_grid_icon_height")
                            .value_or(16);
                    view.cursor_offset_x =
                        read_optional(view_index, "cursor_offset_x").value_or(0);
                    view.cursor_offset_y =
                        read_optional(view_index, "cursor_offset_y").value_or(0);
                    view.posture_bar_x =
                        read_optional(view_index, "posture_bar_x").value_or(160);
                    view.posture_bar_y =
                        read_optional(view_index, "posture_bar_y").value_or(136);
                    view.posture_text_x =
                        read_optional(view_index, "posture_text_x").value_or(160);
                    view.posture_text_y =
                        read_optional(view_index, "posture_text_y").value_or(144);
                    view.spell_ids = read_string_array(
                        view_index, "spell_ids", 14U);
                    view.force =
                        read_optional(view_index, "force").value_or(1);
                    view.maximum_force =
                        read_optional(view_index, "maximum_force").value_or(1);
                    view.success_value =
                        read_optional(view_index, "success_value").value_or(0);
                    view.success_row =
                        read_optional(view_index, "success_row").value_or(-1);
                    view.effect_value =
                        read_optional(view_index, "effect_value").value_or(0);
                    view.effect_row =
                        read_optional(view_index, "effect_row").value_or(-1);
                    view.availability =
                        read_optional(view_index, "availability").value_or(-1);
                    view.physical_defense_value =
                        read_optional(view_index, "physical_defense_value")
                            .value_or(0);
                    view.physical_defense_row =
                        read_optional(view_index, "physical_defense_row")
                            .value_or(-1);
                    view.mana_defense_value =
                        read_optional(view_index, "mana_defense_value")
                            .value_or(0);
                    view.mana_defense_row =
                        read_optional(view_index, "mana_defense_row")
                            .value_or(-1);
                    view.posture =
                        read_optional(view_index, "posture").value_or(0);
                    view.force_bar_selection =
                        read_optional(view_index, "force_bar_selection")
                            .value_or(0);
                    view.posture_bar_selection =
                        read_optional(view_index, "posture_bar_selection")
                            .value_or(0);
                    view.cursor_phase =
                        read_optional(view_index, "cursor_phase").value_or(0);
                    view.cursor_sat_phase =
                        read_optional(view_index, "cursor_sat_phase")
                            .value_or(-1);
                    menu_route_state.magic_view = view;
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "panel_view");
                if (lua_istable(state, -1)) {
                    const auto panel_index = lua_absindex(state, -1);
                    ScriptMenuRouteSnapshot::PanelView panel;
                    panel.present = true;
                    panel.background =
                        static_cast<ScriptMenuRouteSnapshot::PanelBackground>(
                            read_optional(panel_index, "background")
                                .value_or(0));
                    panel.posture =
                        static_cast<ScriptMenuRouteSnapshot::PanelPosture>(
                            read_optional(panel_index, "posture")
                                .value_or(0));
                    panel.background_x =
                        read_optional(panel_index, "background_x").value_or(0);
                    panel.background_y =
                        read_optional(panel_index, "background_y").value_or(0);
                    panel.background_width =
                        read_optional(panel_index, "background_width")
                            .value_or(0);
                    panel.background_height =
                        read_optional(panel_index, "background_height")
                            .value_or(0);
                    lua_getfield(state, panel_index, "elements");
                    if (lua_istable(state, -1)) {
                        constexpr std::size_t kMaximumPanelElements = 8U;
                        const auto elements_index = lua_absindex(state, -1);
                        const auto element_count =
                            lua_rawlen(state, elements_index);
                        if (element_count <= kMaximumPanelElements) {
                            panel.elements.reserve(element_count);
                            for (std::size_t element_index = 0U;
                                 element_index < element_count;
                                 ++element_index) {
                                lua_rawgeti(
                                    state, elements_index,
                                    static_cast<lua_Integer>(element_index + 1U));
                                if (lua_istable(state, -1)) {
                                    const auto value_index =
                                        lua_absindex(state, -1);
                                    ScriptMenuRouteSnapshot::PanelElement
                                        element;
                                    element.kind = static_cast<
                                        ScriptMenuRouteSnapshot::PanelElementKind>(
                                        read_optional(value_index, "kind")
                                            .value_or(0));
                                    element.text_key = static_cast<
                                        ScriptMenuRouteSnapshot::PanelTextKey>(
                                        read_optional(value_index, "text")
                                            .value_or(0));
                                    element.value =
                                        read_optional(value_index, "value")
                                            .value_or(0);
                                    element.x =
                                        read_optional(value_index, "x")
                                            .value_or(0);
                                    element.y =
                                        read_optional(value_index, "y")
                                            .value_or(0);
                                    panel.elements.push_back(element);
                                }
                                lua_pop(state, 1);
                            }
                            menu_route_state.panel_view = std::move(panel);
                        }
                    }
                    lua_pop(state, 1);
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "matrix_map_cells");
                if (lua_istable(state, -1)) {
                    const auto cells_index = lua_absindex(state, -1);
                    constexpr std::size_t kMatrixMapCellCount = 140U;
                    if (lua_rawlen(state, cells_index) >=
                        kMatrixMapCellCount) {
                        bool valid = true;
                        for (std::size_t cell = 0U;
                             cell < kMatrixMapCellCount; ++cell) {
                            lua_rawgeti(
                                state, cells_index,
                                static_cast<lua_Integer>(cell + 1U));
                            if (!lua_istable(state, -1)) {
                                valid = false;
                                lua_pop(state, 1);
                                break;
                            }
                            const auto visual_index = lua_absindex(state, -1);
                            lua_getfield(state, visual_index, "tiles");
                            if (!lua_istable(state, -1) ||
                                lua_rawlen(state, -1) < 4U) {
                                valid = false;
                                lua_pop(state, 2);
                                break;
                            }
                            const auto tiles_index = lua_absindex(state, -1);
                            for (std::size_t tile = 0U; tile < 4U; ++tile) {
                                lua_rawgeti(
                                    state, tiles_index,
                                    static_cast<lua_Integer>(tile + 1U));
                                int is_integer = 0;
                                const auto value =
                                    lua_tointegerx(state, -1, &is_integer);
                                if (is_integer == 0 || value < 0 ||
                                    value > 0xFFFF) {
                                    valid = false;
                                } else {
                                    menu_route_state.top_matrix_map_cells[cell]
                                        .tile_words[tile] =
                                            static_cast<std::uint16_t>(value);
                                }
                                lua_pop(state, 1);
                            }
                            lua_pop(state, 2);
                            if (!valid) {
                                break;
                            }
                        }
                        menu_route_state.top_matrix_map_cells_present = valid;
                    }
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "matrix_map_labels");
                if (lua_istable(state, -1)) {
                    const auto labels_index = lua_absindex(state, -1);
                    constexpr std::size_t kMatrixMapLabelCount = 6U;
                    if (lua_rawlen(state, labels_index) >=
                        kMatrixMapLabelCount) {
                        bool valid = true;
                        for (std::size_t label = 0U;
                             label < kMatrixMapLabelCount; ++label) {
                            lua_rawgeti(
                                state, labels_index,
                                static_cast<lua_Integer>(label + 1U));
                            std::size_t length = 0U;
                            const auto* value =
                                lua_type(state, -1) == LUA_TSTRING
                                    ? lua_tolstring(state, -1, &length)
                                    : nullptr;
                            if (value == nullptr || length == 0U || length > 3U) {
                                valid = false;
                            } else {
                                menu_route_state.top_matrix_map_labels[label]
                                    .assign(value, length);
                            }
                            lua_pop(state, 1);
                            if (!valid) {
                                break;
                            }
                        }
                        menu_route_state.top_matrix_map_labels_present = valid;
                    }
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "grid_cells");
                if (lua_istable(state, -1)) {
                    menu_route_state.top_grid_cells_present = true;
                    menu_route_state.top_grid_cells.fill(-1);
                    const auto grid_index = lua_absindex(state, -1);
                    for (std::size_t cell = 0;
                         cell < menu_route_state.top_grid_cells.size(); ++cell) {
                        lua_rawgeti(state, grid_index,
                                    static_cast<lua_Integer>(cell + 1U));
                        int is_number = 0;
                        const auto value = lua_tointegerx(state, -1, &is_number);
                        if (is_number && value >= -1 && value <= 0x7F) {
                            menu_route_state.top_grid_cells[cell] =
                                static_cast<std::int8_t>(value);
                        }
                        lua_pop(state, 1);
                    }
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "visible_item_ids");
                if (lua_istable(state, -1)) {
                    const auto item_ids_index = lua_absindex(state, -1);
                    const auto item_count = std::min<std::size_t>(
                        static_cast<std::size_t>(lua_rawlen(state, item_ids_index)),
                        15U);
                    for (std::size_t index = 0; index < item_count; ++index) {
                        lua_rawgeti(state, item_ids_index,
                                    static_cast<lua_Integer>(index + 1U));
                        int is_number = 0;
                        const auto value = lua_tointegerx(state, -1, &is_number);
                        if (is_number && value >= 0 && value <= 14) {
                            menu_route_state.top_visible_item_ids.push_back(
                                static_cast<int>(value));
                        }
                        lua_pop(state, 1);
                    }
                }
                lua_pop(state, 1);
                menu_route_state.top_visible_option_ids =
                    read_integer_array(screen_state_index, "options", 8U,
                                       0, 7);
                menu_route_state.top_visible_entry_ids = read_integer_array(
                    screen_state_index, "visible_entry_ids", 96U, 0, 95);
                lua_getfield(state, screen_state_index, "yes");
                if (lua_isboolean(state, -1)) {
                    menu_route_state.top_yes = lua_toboolean(state, -1) != 0;
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "full");
                if (lua_isboolean(state, -1)) {
                    menu_route_state.top_recipient_full =
                        lua_toboolean(state, -1) != 0;
                }
                lua_pop(state, 1);
                lua_getfield(state, screen_state_index, "data");
                if (lua_istable(state, -1)) {
                    const auto data_index = lua_absindex(state, -1);
                    menu_route_state.top_detail_entry_id =
                        read_optional(data_index, "entry_id");
                    menu_route_state.top_detail_row =
                        read_optional(data_index, "row");
                    if (menu_route_state.top_visible_entry_ids.empty()) {
                        menu_route_state.top_visible_entry_ids =
                            read_integer_array(data_index,
                                               "visible_entry_ids", 96U,
                                               0, 95);
                    }
                }
                lua_pop(state, 1);

                const auto read_item = [this, lua](
                                           const int item_index) {
                    if (!lua_istable(lua, item_index)) {
                        return;
                    }
                    const auto read_item_integer = [lua, item_index](
                                                       const char* name)
                        -> std::optional<int> {
                        lua_getfield(lua, item_index, name);
                        int is_number = 0;
                        const auto value = lua_tointegerx(lua, -1, &is_number);
                        lua_pop(lua, 1);
                        return is_number
                                   ? std::optional<int>(static_cast<int>(value))
                                   : std::nullopt;
                    };
                    std::uint32_t column = 0U;
                    std::uint32_t row = 0U;
                    const bool has_cell = read_inventory_cell(
                        lua, item_index, "cell", column, row);
                    const int position = static_cast<int>(row * 2U + column + 1U);
                    menu_route_state.top_item_slot = has_cell
                        ? std::optional<int>(position - 1)
                        : std::nullopt;
                    std::string item_id;
                    lua_getfield(lua, item_index, "id");
                    const bool has_item_id = read_string(lua, -1, item_id);
                    lua_pop(lua, 1);
                    if (!has_cell || !has_item_id) {
                        menu_route_state.top_item_code.reset();
                        menu_route_state.top_item_phase.reset();
                        return;
                    }
                    const auto source_code =
                        script_item_source_code_from_id(item_id);
                    if (!source_code.has_value()) {
                        menu_route_state.top_item_code.reset();
                        menu_route_state.top_item_phase.reset();
                        return;
                    }
                    menu_route_state.top_item_code = *source_code;
                    const auto source_item = script_combat_inventory_item(
                        static_cast<std::uint8_t>(position), *source_code, 0U);
                    if (source_item.has_value() &&
                        !source_item->value_property.empty()) {
                        menu_route_state.top_item_phase = read_item_integer(
                            std::string(source_item->value_property).c_str());
                    } else {
                        menu_route_state.top_item_phase = 0;
                    }
                };
                lua_getfield(state, screen_state_index, "sel");
                read_item(lua_absindex(state, -1));
                lua_pop(state, 1);
                if (!menu_route_state.top_item_slot.has_value()) {
                    lua_getfield(state, screen_state_index, "data");
                    if (lua_istable(state, -1)) {
                        lua_getfield(state, -1, "sel");
                        read_item(lua_absindex(state, -1));
                        lua_pop(state, 1);
                    }
                    lua_pop(state, 1);
                }

                lua_getfield(state, screen_state_index, "targets");
                if (lua_istable(state, -1)) {
                    const auto targets_index = lua_absindex(state, -1);
                    const auto target_count = std::min<std::size_t>(
                        static_cast<std::size_t>(lua_rawlen(state, targets_index)),
                        3U);
                    for (std::size_t index = 0; index < target_count; ++index) {
                        lua_rawgeti(state, targets_index,
                                    static_cast<lua_Integer>(index + 1U));
                        if (lua_isinteger(state, -1)) {
                            menu_route_state.top_targets.push_back(
                                static_cast<int>(lua_tointeger(state, -1)));
                        }
                        lua_pop(state, 1);
                    }
                }
                lua_pop(state, 1);
                if (!menu_route_state.top_target.has_value() ||
                    menu_route_state.top_targets.empty()) {
                    lua_getfield(state, screen_state_index, "data");
                    if (lua_istable(state, -1)) {
                        const auto data_index = lua_absindex(state, -1);
                        if (!menu_route_state.top_target.has_value()) {
                            menu_route_state.top_target = read_optional(
                                data_index, "target");
                        }
                        if (menu_route_state.top_targets.empty()) {
                            lua_getfield(state, data_index, "targets");
                            if (lua_istable(state, -1)) {
                                const auto targets_index = lua_absindex(state, -1);
                                const auto target_count = std::min<std::size_t>(
                                    static_cast<std::size_t>(lua_rawlen(
                                        state, targets_index)),
                                    3U);
                                for (std::size_t index = 0;
                                     index < target_count; ++index) {
                                    lua_rawgeti(state, targets_index,
                                                static_cast<lua_Integer>(index + 1U));
                                    if (lua_isinteger(state, -1)) {
                                        menu_route_state.top_targets.push_back(
                                            static_cast<int>(lua_tointeger(state, -1)));
                                    }
                                    lua_pop(state, 1);
                                }
                            }
                            lua_pop(state, 1);
                        }
                    }
                    lua_pop(state, 1);
                }

                // Clip Trade keeps its recipient list in the Lua screen
                // state and advances an index through that filtered list.
                // Snapshot the selected stable party slot for the Qt painter;
                // the list and its behavior remain Lua-owned.
                lua_getfield(state, screen_state_index, "recipients");
                const auto recipients_index = lua_absindex(state, -1);
                lua_getfield(state, screen_state_index, "index");
                if (lua_istable(state, recipients_index) &&
                    lua_isinteger(state, -1)) {
                    const auto recipient_index = lua_tointeger(state, -1);
                    if (recipient_index > 0) {
                        lua_rawgeti(state, recipients_index, recipient_index);
                        if (lua_isinteger(state, -1)) {
                            menu_route_state.top_recipient =
                                static_cast<int>(lua_tointeger(state, -1));
                        }
                        lua_pop(state, 1);
                    }
                }
                lua_pop(state, 2);
            }
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
        lua_getfield(state, route_index, "root");
        lua_pushvalue(state, route_index);
        if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
            failure = "menu route root query failed: " + stack_error(state);
            lua_settop(state, base);
            menu_route_state = ScriptMenuRouteSnapshot{};
            return false;
        }
        if (lua_istable(state, -1)) {
            lua_getfield(state, -1, "state");
            if (lua_istable(state, -1)) {
                const auto state_index = lua_absindex(state, -1);
                menu_route_state.command_section =
                    read_optional_command_section(state, state_index,
                                                  "section");
                menu_route_state.selected_inventory_row =
                    read_optional(state_index, "selected_inventory_row");
                menu_route_state.page = read_optional(state_index, "page");
                menu_route_state.row = read_optional(state_index, "row");
                lua_getfield(state, state_index, "inventory_detail_pending");
                if (lua_isboolean(state, -1)) {
                    menu_route_state.root_inventory_detail_pending =
                        lua_toboolean(state, -1) != 0;
                }
                lua_pop(state, 1);
                lua_getfield(state, state_index, "inventory_picture_partial");
                if (lua_isboolean(state, -1)) {
                    menu_route_state.root_inventory_picture_partial =
                        lua_toboolean(state, -1) != 0;
                }
                lua_pop(state, 1);
                lua_getfield(state, state_index, "inventory_cursor_hidden");
                if (lua_isboolean(state, -1)) {
                    menu_route_state.root_inventory_cursor_hidden =
                        lua_toboolean(state, -1) != 0;
                }
                lua_pop(state, 1);
                menu_route_state.root_inventory_ammo_count =
                    read_optional(state_index, "inventory_ammo_count");
            }
        }
        lua_settop(state, base);
        return true;
    }

    bool load_module(lua_State* target_state, const std::string_view name,
                     std::string& error) {
        if (target_state == nullptr || modules_reference == LUA_NOREF ||
            !valid_lua_module_name(name)) {
            error = "invalid gameplay module name";
            return false;
        }

        // `require` can run inside a suspended gameplay task coroutine. Lua
        // registry values are shared across threads, but stack operations
        // must stay on the calling thread or require returns a value from the
        // unrelated main-state stack.
        lua_rawgeti(target_state, LUA_REGISTRYINDEX, modules_reference);
        const int modules_index = lua_absindex(target_state, -1);
        lua_pushlstring(target_state, name.data(), name.size());
        lua_gettable(target_state, modules_index);
        if (lua_istable(target_state, -1)) {
            lua_remove(target_state, modules_index);
            return true;
        }
        if (!lua_isnil(target_state, -1)) {
            lua_settop(target_state, modules_index - 1);
            error = "gameplay module cache contains a non-table value";
            return false;
        }
        lua_pop(target_state, 1);

        const std::string module_name(name);
        if (modules_loading.find(module_name) != modules_loading.end()) {
            lua_settop(target_state, modules_index - 1);
            error = "cyclic gameplay module dependency: " + module_name;
            return false;
        }
        constexpr std::size_t kMaximumLoadedModules = 256U;
        if (loaded_module_count >= kMaximumLoadedModules) {
            lua_settop(target_state, modules_index - 1);
            error = "gameplay module package exceeds module limit";
            return false;
        }

        modules_loading.insert(module_name);
        struct RemoveLoadingModule {
            std::set<std::string>& modules;
            const std::string& name;
            ~RemoveLoadingModule() { modules.erase(name); }
        } remove_loading_module{modules_loading, module_name};

        std::filesystem::path relative_path;
        std::string component;
        for (const char character : name) {
            if (character == '.') {
                relative_path /= component;
                component.clear();
            } else {
                component.push_back(character);
            }
        }
        relative_path /= component + ".lua";

        std::error_code path_error;
        const auto module_path =
            std::filesystem::weakly_canonical(script_root / relative_path,
                                              path_error);
        if (path_error || !path_is_within_root(script_root, module_path) ||
            !std::filesystem::is_regular_file(module_path, path_error) ||
            path_error) {
            lua_settop(target_state, modules_index - 1);
            error = "gameplay module is missing or outside script root: " +
                    module_name;
            return false;
        }

        int status = LUA_OK;
        {
            ScopedInstructionBudget instruction_budget(target_state);
            status = luaL_loadfilex(target_state,
                                    module_path.string().c_str(), "t");
            if (status == LUA_OK) {
                status = lua_pcall(target_state, 0, 1, 0);
            }
        }
        if (status != LUA_OK) {
            error = module_name + ": " + stack_error(target_state);
            lua_settop(target_state, modules_index - 1);
            return false;
        }
        if (!lua_istable(target_state, -1)) {
            lua_settop(target_state, modules_index - 1);
            error = module_name + " must return a definition table";
            return false;
        }

        const int definition_index = lua_absindex(target_state, -1);
        lua_pushlstring(target_state, name.data(), name.size());
        lua_pushvalue(target_state, definition_index);
        lua_settable(target_state, modules_index);
        lua_remove(target_state, modules_index);
        ++loaded_module_count;
        return true;
    }

    static int require_module(lua_State* lua) {
        auto* self = from_upvalue(lua);
        bool loaded = false;
        {
            std::string module_name;
            std::string error;
            if (self != nullptr && read_string(lua, 1, module_name)) {
                loaded = self->load_module(lua, module_name, error);
            } else {
                error = "require expects a dotted gameplay module name";
            }
            if (!loaded) {
                lua_pushlstring(lua, error.data(), error.size());
            }
        }
        return loaded ? 1 : lua_error(lua);
    }

    static int state_index(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || lua_type(lua, 2) != LUA_TSTRING) {
            lua_pushnil(lua);
            return 1;
        }
        std::string name;
        if (!read_string(lua, 2, name) || !valid_identifier(name)) {
            lua_pushnil(lua);
            return 1;
        }
        const auto fact = self->story_fact_reader
                              ? self->story_fact_reader(name)
                              : [&]() -> std::optional<bool> {
                                    const auto stored =
                                        self->story_facts.find(name);
                                    if (stored == self->story_facts.end()) {
                                        return std::nullopt;
                                    }
                                    return stored->second;
                                }();
        if (!fact.has_value()) {
            lua_pushnil(lua);
        } else {
            lua_pushboolean(lua, *fact ? 1 : 0);
        }
        return 1;
    }

    static int state_newindex(lua_State* lua) {
        auto* self = from_upvalue(lua);
        const char* error = nullptr;
        if (self == nullptr) {
            error = "game.state has no native story-fact store";
        } else {
            {
                std::string name;
                if (lua_type(lua, 2) != LUA_TSTRING ||
                    !read_string(lua, 2, name) ||
                    !valid_identifier(name) ||
                    lua_type(lua, 3) != LUA_TBOOLEAN) {
                    error =
                        "game.state accepts named boolean story facts only";
                } else if (self->story_fact_writer) {
                    const bool value = lua_toboolean(lua, 3) != 0;
                    if (self->story_fact_writer(name, value)) {
                        return 0;
                    }
                    error = "game.state property is not writable";
                } else if (
                    self->story_facts.find(name) ==
                        self->story_facts.end() &&
                    self->story_facts.size() >= kMaximumStoryFacts) {
                    error = "game.state has reached its fact limit";
                } else {
                    self->story_facts[std::move(name)] =
                        lua_toboolean(lua, 3) != 0;
                    return 0;
                }
            }
        }
        return luaL_error(lua, "%s", error);
    }

    static int location_index(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || lua_type(lua, 2) != LUA_TSTRING) {
            lua_pushnil(lua);
            return 1;
        }
        std::string field;
        if (!read_string(lua, 2, field)) {
            lua_pushnil(lua);
            return 1;
        }
        if (field == "scene_id") {
            if (self->location_scene_id.has_value()) {
                lua_pushinteger(lua, *self->location_scene_id);
            } else {
                lua_pushnil(lua);
            }
            return 1;
        }
        std::string_view value;
        if (field == "type") {
            value = self->location_type;
        } else if (field == "id") {
            value = self->location_id;
        } else if (field == "text_profile") {
            value = self->location_text_profile;
        } else if (field == "speaker_id") {
            value = self->location_speaker_id;
        } else if (field == "event_id") {
            value = self->location_event_id;
        } else {
            lua_pushnil(lua);
            return 1;
        }
        if (value.empty()) {
            lua_pushnil(lua);
        } else {
            lua_pushlstring(lua, value.data(), value.size());
        }
        return 1;
    }

    static int location_newindex(lua_State* lua) {
        return luaL_error(lua, "game.location is native read-only context");
    }

    static int field_context_index(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || lua_type(lua, 2) != LUA_TSTRING) {
            lua_pushnil(lua);
            return 1;
        }
        std::string field;
        if (!read_string(lua, 2, field)) {
            lua_pushnil(lua);
            return 1;
        }
        if (field == "prompt_id") {
            const auto prompt_id = script_field_mode_prompt_id(
                script_field_mode_prompt_id_from_source_context(
                    self->field_mode_context.source_mode,
                    self->field_mode_context.source_event_code));
            lua_pushlstring(lua, prompt_id.data(), prompt_id.size());
        } else if (field == "service_id") {
            lua_pushlstring(lua,
                            self->field_mode_context.service_id.data(),
                            self->field_mode_context.service_id.size());
        } else if (field == "prompt_variant") {
            lua_pushlstring(
                lua, self->field_mode_context.prompt_variant.data(),
                self->field_mode_context.prompt_variant.size());
        } else if (field == "package_run_status") {
            const auto status_id = script_package_run_status_id(
                script_package_run_status_from_source_result(
                    self->field_mode_context.source_signed_result));
            lua_pushlstring(lua, status_id.data(), status_id.size());
        } else if (field == "has_alternate_profile") {
            lua_pushboolean(
                lua, self->field_mode_context.source_alternate_profile
                         .has_value() &&
                         *self->field_mode_context.source_alternate_profile !=
                             0U);
        } else if (field == "has_encounter_profile_context") {
            // ROM 0x53F1A/0x53F32 selects the Licourtrix conversation only
            // in its encounter profile context (source mode 8) [C].
            lua_pushboolean(
                lua, self->field_mode_context.source_mode == 8U);
        } else if (field == "serpent_lair_transition_active") {
            // Source transition selector 19 gates the Scene-47 serpent-lair
            // continuation at ROM 0x53EFA/0x1D54B0 and the neighboring mode
            // table prompt at 0x53EE2 [C]. Lua consumes the named world fact,
            // not that selector value.
            lua_pushboolean(
                lua, self->field_mode_context.source_transition_context ==
                         19U);
        } else if (field == "taxi_current_region") {
            if (self->field_mode_context.taxi_current_region.has_value()) {
                const auto region_id = script_field_region_id(
                    *self->field_mode_context.taxi_current_region);
                lua_pushlstring(lua, region_id.data(), region_id.size());
            } else {
                lua_pushnil(lua);
            }
        } else if (field == "taxi_destination_region") {
            if (self->field_mode_context.taxi_destination_region.has_value()) {
                const auto region_id = script_field_region_id(
                    *self->field_mode_context.taxi_destination_region);
                lua_pushlstring(lua, region_id.data(), region_id.size());
            } else {
                lua_pushnil(lua);
            }
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }

    static int field_context_newindex(lua_State* lua) {
        return luaL_error(lua, "game.field context is read-only");
    }

    static int movement_collision(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        std::uint32_t step_distance = 0U;
        const auto argument_count = lua_gettop(lua);
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->movement_collision ||
            (argument_count != 2 && argument_count != 4) ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y)) {
            return luaL_error(
                lua,
                "game.movement.probe expects a world position with an optional direction and distance during an actor heartbeat");
        }
        std::optional<std::uint8_t> direction_ordinal;
        if (argument_count == 4) {
            if (lua_type(lua, 3) != LUA_TSTRING) {
                return luaL_error(
                    lua, "game.movement.probe direction must be a compass name");
            }
            if (!read_integer(lua, 4, 0xFFFFU, step_distance)) {
                return luaL_error(
                    lua, "game.movement.probe distance must be a nonnegative whole number");
            }
            std::size_t direction_length = 0U;
            const char* direction_text = lua_tolstring(lua, 3, &direction_length);
            const std::string_view direction(direction_text, direction_length);
            const auto direction_it = std::find(kCompassDirections.begin(),
                                                kCompassDirections.end(),
                                                direction);
            if (direction_it == kCompassDirections.end()) {
                return luaL_error(
                    lua, "game.movement.probe expects a compass direction");
            }
            direction_ordinal = static_cast<std::uint8_t>(
                std::distance(kCompassDirections.begin(), direction_it));
        }
        const auto& offset = direction_ordinal.has_value()
            ? kCompassOffsets[*direction_ordinal]
            : kCompassOffsets[0U];
        const auto candidate_x = static_cast<std::uint16_t>(
            static_cast<std::int32_t>(x) +
            (direction_ordinal.has_value()
                 ? static_cast<std::int32_t>(offset.x) *
                       static_cast<std::int32_t>(step_distance)
                 : 0));
        const auto candidate_y = static_cast<std::uint16_t>(
            static_cast<std::int32_t>(y) +
            (direction_ordinal.has_value()
                 ? static_cast<std::int32_t>(offset.y) *
                       static_cast<std::int32_t>(step_distance)
                 : 0));
        try {
            self->last_actor_movement_recovery_probe.reset();
            const auto result =
                self->active_actor_context->movement_collision(
                    candidate_x, candidate_y);
            self->last_actor_terrain_collision_code =
                result.terrain_collision_code;

            // ROM 0xB77E preserves a negative collision result only when the
            // actor's recovery flag is set. Keep that source encoding here;
            // Lua receives a named movement outcome [C].
            auto probe_code = result.collision_code;
            if (probe_code < 0 &&
                (self->active_actor_context->movement_entity_flags &
                 kMovementRecoveryEnabledFlag) == 0U) {
                probe_code = 0x7F;
            }

            const auto& context = *self->active_actor_context;
            const auto route_code = apply_movement_result_policy(
                probe_code,
                MovementResultPolicy{context.movement_transition_active,
                                     context.movement_scene_id,
                                     context.movement_collision_mode});
            const char* kind = "blocked";
            // ROM 0xBADE handles the initial 0xB77E result before invoking
            // 0xB732 for a later route-fallback probe. Preserve that order:
            // Lua needs the raw B77E outcome here, while the separate
            // blocks_route_fallback fact carries B732's policy result [C].
            if (probe_code == 0) {
                kind = "clear";
            } else if (probe_code < 0 &&
                       (route_code < 0 ||
                        static_cast<bool>(context.movement_mode_transition))) {
                kind = "requires_recovery";
                if (direction_ordinal.has_value()) {
                    self->last_actor_movement_recovery_probe =
                        PendingMovementRecoveryProbe{
                            candidate_x, candidate_y, probe_code,
                            *direction_ordinal};
                }
            }

            // The B732 policy decides whether this probe blocks route
            // fallback. Its low-bit event encoding never crosses into Lua [C].
            lua_createtable(lua, 0, 5);
            lua_pushstring(lua, kind);
            lua_setfield(lua, -2, "kind");
            lua_pushinteger(lua, candidate_x);
            lua_setfield(lua, -2, "candidate_x");
            lua_pushinteger(lua, candidate_y);
            lua_setfield(lua, -2, "candidate_y");
            lua_pushboolean(lua, route_code > 0);
            lua_setfield(lua, -2, "blocks_route_fallback");
            lua_pushboolean(lua, result.terrain_collision_code == 0U);
            lua_setfield(lua, -2, "terrain_is_clear");
            return 1;
        } catch (...) {
            return luaL_error(lua,
                              "native movement collision query failed");
        }
    }

    static int actor_random_bound(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t bound = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->ai_random_bound ||
            !read_integer(lua, 1, 0xFFFFU, bound) || bound == 0U) {
            return luaL_error(
                lua,
                "actor target selection needs a nonzero source random bound");
        }
        try {
            const auto result =
                self->active_actor_context->ai_random_bound(
                    static_cast<std::uint16_t>(bound));
            if (!result.has_value() || *result >= bound) {
                return luaL_error(
                    lua,
                    "source random service returned an invalid target index");
            }
            lua_pushinteger(lua, *result);
            return 1;
        } catch (...) {
            return luaL_error(lua, "source actor random service failed");
        }
    }

    static int conditional_actor_random_bound(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t bound = 0U;
        if (self == nullptr || !self->active_conditional_actor_random_bound ||
            !read_integer(lua, 1, 0xFFFFU, bound) || bound == 0U) {
            return luaL_error(
                lua,
                "conditional actor selection needs a nonzero source random bound");
        }
        try {
            const auto result = self->active_conditional_actor_random_bound(
                static_cast<std::uint16_t>(bound));
            if (!result.has_value() || *result >= bound) {
                return luaL_error(
                    lua,
                    "conditional actor random service returned an invalid index");
            }
            lua_pushinteger(lua, *result);
            return 1;
        } catch (...) {
            return luaL_error(
                lua, "conditional actor random service failed");
        }
    }

    static int actor_action_line_clear(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->action_line_clear ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y)) {
            return luaL_error(
                lua, "spell line probe expects target world coordinates");
        }
        try {
            const auto clear =
                self->active_actor_context->action_line_clear(
                    static_cast<std::uint16_t>(x),
                    static_cast<std::uint16_t>(y));
            lua_pushboolean(lua, clear ? 1 : 0);
            return 1;
        } catch (...) {
            return luaL_error(lua, "spell line probe failed");
        }
    }

    static int actor_measure_destination(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->measure_destination ||
            !read_integer(lua, 1,
                          std::numeric_limits<std::uint16_t>::max(), x) ||
            !read_integer(lua, 2,
                          std::numeric_limits<std::uint16_t>::max(), y)) {
            return luaL_error(
                lua, "actor destination measure expects world coordinates");
        }
        try {
            const auto measured =
                self->active_actor_context->measure_destination(
                    static_cast<std::uint16_t>(x),
                    static_cast<std::uint16_t>(y));
            if (!measured.has_value()) {
                lua_pushnil(lua);
                return 1;
            }
            lua_createtable(lua, 0, 3);
            lua_pushnumber(lua, bearing_degrees_from_source(measured->heading));
            lua_setfield(lua, -2, "bearing_degrees");
            lua_pushinteger(lua, measured->distance);
            lua_setfield(lua, -2, "distance");
            lua_pushinteger(lua, measured->step_cost);
            lua_setfield(lua, -2, "step_cost");
            return 1;
        } catch (...) {
            return luaL_error(lua, "actor destination measure failed");
        }
    }

    static int movement_route(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->movement_route_lookup ||
            lua_type(lua, 1) != LUA_TSTRING) {
            return luaL_error(
                lua,
                "game.movement.route expects a cardinal direction during an actor heartbeat");
        }
        std::size_t direction_length = 0U;
        const char* direction_text = lua_tolstring(lua, 1, &direction_length);
        const std::string_view direction(direction_text, direction_length);
        std::uint16_t route_key = 0U;
        if (direction == "north") {
            route_key = 0U;
        } else if (direction == "east") {
            route_key = 4U;
        } else if (direction == "south") {
            route_key = 8U;
        } else if (direction == "west") {
            route_key = 12U;
        } else {
            return luaL_error(
                lua,
                "game.movement.route expects north, east, south, or west");
        }
        try {
            const auto route =
                self->active_actor_context->movement_route_lookup(
                    route_key);
            lua_createtable(lua, 0, 4);
            lua_pushboolean(lua, route.primary_marker > 0);
            lua_setfield(lua, -2, "primary_route_marker_present");
            lua_pushboolean(lua, route.secondary_marker > 0);
            lua_setfield(lua, -2, "secondary_route_marker_present");
            const auto primary_direction = kCompassDirections[
                route.primary_fallback_slot & 0x07U];
            lua_pushlstring(lua, primary_direction.data(),
                            primary_direction.size());
            lua_setfield(lua, -2, "primary_fallback_direction");
            const auto secondary_direction = kCompassDirections[
                route.secondary_fallback_slot & 0x07U];
            lua_pushlstring(lua, secondary_direction.data(),
                            secondary_direction.size());
            lua_setfield(lua, -2, "secondary_fallback_direction");
            return 1;
        } catch (...) {
            return luaL_error(lua, "native movement route lookup failed");
        }
    }

    static int movement_follow_route_step_clear(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->movement_collision ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y)) {
            return luaL_error(
                lua,
                "game.movement.follow_route_step_clear expects map coordinates during an actor heartbeat");
        }
        try {
            self->last_actor_movement_recovery_probe.reset();
            const auto result =
                self->active_actor_context->movement_collision(
                    static_cast<std::uint16_t>(x),
                    static_cast<std::uint16_t>(y));
            self->last_actor_terrain_collision_code =
                result.terrain_collision_code;
            // ROM 0x14550/0x1455A treats zero and negative results as
            // traversable for a follower's formation step [C].
            lua_pushboolean(lua, result.collision_code <= 0);
            return 1;
        } catch (...) {
            return luaL_error(
                lua, "native follow-route collision query failed");
        }
    }

    static int movement_recover_collision(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            (!self->active_actor_context->movement_collision_recovery &&
             !self->active_actor_context->movement_mode_transition) ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y) ||
            lua_type(lua, 3) != LUA_TSTRING ||
            !self->last_actor_movement_recovery_probe.has_value() ||
            self->last_actor_movement_recovery_probe->x != x ||
            self->last_actor_movement_recovery_probe->y != y) {
            return luaL_error(
                lua,
                "game.movement.recover_collision expects the candidate and direction from the latest recoverable probe");
        }
        std::size_t direction_length = 0U;
        const char* direction_text = lua_tolstring(lua, 3, &direction_length);
        const std::string_view direction(direction_text, direction_length);
        const auto direction_it = std::find(kCompassDirections.begin(),
                                            kCompassDirections.end(),
                                            direction);
        if (direction_it == kCompassDirections.end()) {
            return luaL_error(
                lua, "game.movement.recover_collision expects a compass direction");
        }
        const auto direction_ordinal = static_cast<std::uint8_t>(
            std::distance(kCompassDirections.begin(), direction_it));
        if (self->last_actor_movement_recovery_probe->direction_ordinal !=
            direction_ordinal) {
            return luaL_error(
                lua, "game.movement.recover_collision direction does not match the latest probe");
        }
        try {
            const auto pending_probe =
                *self->last_actor_movement_recovery_probe;
            self->last_actor_movement_recovery_probe.reset();
            // ROM 0xB77E writes twice the compass ordinal to its vector-table
            // slot. Convert the semantic direction only at this native edge [C].
            constexpr std::uint8_t kSourceDirectionSlotScale = 2U;
            const auto direction_slot = static_cast<std::uint8_t>(
                pending_probe.direction_ordinal *
                kSourceDirectionSlotScale);
            const auto& context = *self->active_actor_context;
            const auto& transition = context.movement_mode_transition
                ? context.movement_mode_transition
                : context.movement_collision_recovery;
            const auto recovery = transition(
                static_cast<std::uint16_t>(x),
                static_cast<std::uint16_t>(y),
                pending_probe.collision_code, direction_slot);
            lua_pushboolean(lua, recovery.movement_recovered);
            lua_pushboolean(lua, (recovery.route_flags & 0x20U) != 0U);
            return 2;
        } catch (...) {
            return luaL_error(
                lua, "native movement collision recovery failed");
        }
    }

    static int movement_projectile_terrain(lua_State* lua) {
        auto* self = from_upvalue(lua);
        double map_x = 0.0;
        double map_y = 0.0;
        std::uint16_t x = 0U;
        std::uint16_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->projectile_movement.has_value() ||
            !self->active_actor_context->projectile_terrain_probe ||
            !read_number(lua, 1, map_x) ||
            !read_number(lua, 2, map_y) ||
            !world_coordinate_cell(map_x, x) ||
            !world_coordinate_cell(map_y, y)) {
            return luaL_error(
                lua,
                "game.movement.projectile_terrain expects map positions during a projectile behavior heartbeat");
        }
        try {
            const auto sample =
                self->active_actor_context->projectile_terrain_probe(
                    x, y);
            lua_createtable(lua, 0, 3);
            lua_pushboolean(lua, sample.blocked);
            lua_setfield(lua, -2, "blocked");
            lua_pushboolean(lua, sample.boundary_active);
            lua_setfield(lua, -2, "boundary_active");
            lua_pushboolean(lua, sample.outside_map);
            lua_setfield(lua, -2, "outside_map");
            return 1;
        } catch (...) {
            return luaL_error(
                lua, "native projectile terrain query failed");
        }
    }

    static int movement_projectile_collision(lua_State* lua) {
        auto* self = from_upvalue(lua);
        double map_x = 0.0;
        double map_y = 0.0;
        std::uint16_t x = 0U;
        std::uint16_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->projectile_movement.has_value() ||
            !self->active_actor_context->projectile_collision_search ||
            !read_number(lua, 1, map_x) ||
            !read_number(lua, 2, map_y) ||
            !world_coordinate_cell(map_x, x) ||
            !world_coordinate_cell(map_y, y)) {
            return luaL_error(
                lua,
                "game.movement.projectile_collision expects map positions during a projectile behavior heartbeat");
        }
        try {
            const auto result =
                self->active_actor_context->projectile_collision_search(
                    x, y);
            lua_createtable(lua, 0, 3);
            const char* kind =
                result.kind == ScriptProjectileContactKind::target_hit
                    ? "target_hit"
                    : result.kind ==
                              ScriptProjectileContactKind::linked_followup
                          ? "linked_followup"
                          : "none";
            lua_pushstring(lua, kind);
            lua_setfield(lua, -2, "kind");
            if (result.target_position.has_value()) {
                lua_createtable(lua, 0, 2);
                lua_pushnumber(lua, result.target_position->x);
                lua_setfield(lua, -2, "x");
                lua_pushnumber(lua, result.target_position->y);
                lua_setfield(lua, -2, "y");
            } else {
                lua_pushnil(lua);
            }
            lua_setfield(lua, -2, "target_position");
            if (result.hit_target_id.has_value()) {
                lua_pushlstring(lua, result.hit_target_id->data(),
                                result.hit_target_id->size());
            } else {
                lua_pushnil(lua);
            }
            lua_setfield(lua, -2, "target_id");
            return 1;
        } catch (...) {
            return luaL_error(
                lua, "native projectile collision search failed");
        }
    }

    static int native_operation_continuation(
        lua_State*, int, lua_KContext) {
        // The host resumes this continuation with the native service result.
        return 1;
    }

    static int actor_walk_direction(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.actor.walk_direction must run inside an active task";
            }
            lua_pushboolean(lua, 0);
            return 1;
        }

        std::size_t actor_length = 0U;
        const auto* actor = lua_type(lua, 1) == LUA_TSTRING
                                ? lua_tolstring(lua, 1, &actor_length)
                                : nullptr;
        const bool valid_actor = actor != nullptr && actor_length == 6U &&
                                 std::string_view(actor, actor_length) ==
                                     "player";
        std::size_t direction_length = 0U;
        const auto* direction_text = lua_type(lua, 2) == LUA_TSTRING
                                         ? lua_tolstring(
                                               lua, 2, &direction_length)
                                         : nullptr;
        const auto direction = direction_text == nullptr
            ? std::nullopt
            : movement_direction_from_id(
                  std::string_view(direction_text, direction_length));
        if (!valid_actor || !direction.has_value()) {
            self->bridge_failure =
                "game.actor.walk_direction expects the player and a named field direction";
            lua_pushboolean(lua, 0);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::actor_walk_direction;
        operation.actor_id = "player";
        operation.direction = *direction;
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 0, native_operation_continuation);
    }

    static int ui_menu(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.menu must run inside an active task";
            }
            lua_pushnil(lua);
            return 1;
        }

        std::string menu_id;
        std::string speaker_id = self->location_speaker_id;
        std::string text_profile = self->location_text_profile;
        ScriptMenuDefinition menu{};
        if (lua_gettop(lua) >= 3 &&
            (!read_string_allow_empty(lua, 3, speaker_id) ||
             (!speaker_id.empty() && !valid_identifier(speaker_id)))) {
            self->bridge_failure =
                "game.ui.menu optional speaker id must be an identifier";
            lua_pushnil(lua);
            return 1;
        }
        if (lua_gettop(lua) >= 4 &&
            (!read_string(lua, 4, text_profile) ||
             !valid_identifier(text_profile))) {
            self->bridge_failure =
                "game.ui.menu optional text profile must be an identifier";
            lua_pushnil(lua);
            return 1;
        }
        if (!read_string(lua, 1, menu_id) ||
            !valid_identifier(menu_id) ||
            !valid_identifier(text_profile) ||
            !parse_menu_choices(lua, 2, menu_id, text_profile, speaker_id,
                                menu,
                                kMaximumMenuChoices)) {
            self->bridge_failure =
                "game.ui.menu expects a menu name and unique stable choice ids in a profiled location";
            lua_pushnil(lua);
            return 1;
        }
        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_menu;
        operation.menu = std::move(menu);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, native_operation_continuation);
    }

    static int ui_prompt_text(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.prompt_text must run inside an active task";
            }
            lua_pushnil(lua);
            return 1;
        }

        std::string menu_id;
        std::size_t text_length = 0U;
        const auto* text = lua_type(lua, 2) == LUA_TSTRING
                               ? lua_tolstring(lua, 2, &text_length)
                               : nullptr;
        ScriptMenuDefinition menu{};
        if (!read_string(lua, 1, menu_id) ||
            !valid_identifier(menu_id) || text == nullptr ||
            text_length > kMaximumDialogueTextBytes ||
            !valid_identifier(self->location_text_profile) ||
            !parse_menu_choices(lua, 3, menu_id,
                                self->location_text_profile,
                                self->location_speaker_id, menu,
                                kMaximumMenuChoices)) {
            self->bridge_failure =
                "game.ui.prompt_text expects a menu name, bounded prompt, and unique stable choice ids in a profiled speaker location";
            lua_pushnil(lua);
            return 1;
        }

        menu.inline_text.assign(text, text_length);
        menu.inline_prompt = true;
        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_menu;
        operation.menu = std::move(menu);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, native_operation_continuation);
    }

    static int ui_prompt_text_key(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.prompt_text_key must run inside an active task";
            }
            lua_pushnil(lua);
            return 1;
        }

        std::string menu_id;
        std::string text_key;
        std::string speaker_id = self->location_speaker_id;
        std::string text_profile = self->location_text_profile;
        ScriptMenuDefinition menu{};
        if (lua_gettop(lua) >= 4 &&
            (!read_string_allow_empty(lua, 4, speaker_id) ||
             (!speaker_id.empty() && !valid_identifier(speaker_id)))) {
            self->bridge_failure =
                "game.ui.prompt_text_key optional speaker id must be an identifier";
            lua_pushnil(lua);
            return 1;
        }
        if (lua_gettop(lua) >= 5 &&
            (!read_string(lua, 5, text_profile) ||
             !valid_identifier(text_profile))) {
            self->bridge_failure =
                "game.ui.prompt_text_key optional text profile must be an identifier";
            lua_pushnil(lua);
            return 1;
        }
        if (!read_string(lua, 1, menu_id) ||
            !valid_identifier(menu_id) ||
            !read_string(lua, 2, text_key) ||
            !valid_identifier(text_key) ||
            !valid_identifier(text_profile) ||
            !parse_menu_choices(lua, 3, menu_id, text_profile,
                                speaker_id, menu,
                                kMaximumMenuChoices)) {
            self->bridge_failure =
                "game.ui.prompt_text_key expects a menu name, ROM text key, and unique stable choice ids in a profiled speaker location";
            lua_pushnil(lua);
            return 1;
        }
        const auto text = self->dialogue_texts.find(text_key);
        if (text == self->dialogue_texts.end() || text->second.empty() ||
            text->second.size() > kMaximumDialogueTextBytes) {
            self->bridge_failure =
                "game.ui.prompt_text_key could not resolve a bounded ROM dialogue text key";
            lua_pushnil(lua);
            return 1;
        }

        menu.inline_text = text->second;
        menu.inline_prompt = true;
        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_menu;
        operation.menu = std::move(menu);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, native_operation_continuation);
    }

    static int ui_cursor_menu(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.cursor_menu must run inside an active task";
            }
            lua_pushnil(lua);
            return 1;
        }

        std::string menu_id;
        std::string speaker_id = self->location_speaker_id;
        std::string text_profile = self->location_text_profile;
        std::string inline_text;
        ScriptMenuDefinition menu{};
        const auto choice_count = lua_istable(lua, 2)
                                      ? lua_rawlen(lua, 2)
                                      : std::size_t{0U};
        const auto requested_selection = lua_isinteger(lua, 3) != 0
                                             ? lua_tointeger(lua, 3)
                                             : static_cast<lua_Integer>(-1);
        const bool valid_selection =
            choice_count == 0U
                ? requested_selection == 0
                : requested_selection >= 1 &&
                      requested_selection <= static_cast<lua_Integer>(
                                                  kMaximumMenuChoices);
        const bool valid_speaker = lua_gettop(lua) < 4 ||
            (read_string_allow_empty(lua, 4, speaker_id) &&
             (speaker_id.empty() || valid_identifier(speaker_id)));
        const bool valid_profile = lua_gettop(lua) < 5 ||
            (read_string(lua, 5, text_profile) &&
             valid_identifier(text_profile));
        const bool valid_inline_text = lua_gettop(lua) < 6 ||
            (read_string_allow_empty(lua, 6, inline_text) &&
             inline_text.size() <= 1024U &&
             std::none_of(inline_text.begin(), inline_text.end(),
                          [](const unsigned char ch) {
                              return ch < 0x20U || ch == 0x7FU;
                          }));
        if (!read_string(lua, 1, menu_id) ||
            !valid_identifier(menu_id) || !valid_selection ||
            !valid_speaker || !valid_profile || !valid_inline_text ||
            !valid_identifier(text_profile) ||
            !parse_menu_choices(lua, 2, menu_id, text_profile, speaker_id,
                                menu,
                                kMaximumMenuChoices) ||
            (choice_count != 0U &&
             requested_selection >
                 static_cast<lua_Integer>(menu.choices.size()))) {
            self->bridge_failure =
                "game.ui.cursor_menu expects a menu name, choices, and a valid one-based selection in a profiled location";
            lua_pushnil(lua);
            return 1;
        }

        menu.input_mode = ScriptMenuInputMode::cursor;
        menu.selected_index = choice_count == 0U
                                  ? 0U
                                  : static_cast<std::size_t>(
                                        requested_selection - 1);
        menu.inline_prompt = !inline_text.empty();
        menu.inline_text = std::move(inline_text);
        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_menu;
        operation.menu = std::move(menu);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, native_operation_continuation);
    }

    static int ui_say(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.say must run inside an active task";
            }
            lua_pushboolean(lua, 0);
            return 1;
        }

        std::string event_id;
        std::string speaker_id = self->location_speaker_id;
        std::string text_profile = self->location_text_profile;
        std::string presentation_name = "field_npc";
        std::uint32_t input_delay_frames = 0U;
        const bool valid_speaker = lua_gettop(lua) < 2 ||
            (read_string(lua, 2, speaker_id) &&
             valid_identifier(speaker_id));
        const bool valid_profile = lua_gettop(lua) < 3 ||
            (read_string(lua, 3, text_profile) &&
             valid_identifier(text_profile));
        const bool valid_presentation = lua_gettop(lua) < 4 ||
            (read_string(lua, 4, presentation_name) &&
             (presentation_name == "field_npc" ||
              presentation_name == "field_mode_text"));
        const bool valid_input_delay = lua_gettop(lua) < 5 ||
            read_integer(lua, 5, 255U, input_delay_frames);
        if (!read_string(lua, 1, event_id) ||
            !valid_identifier(event_id) || !valid_speaker ||
            !valid_profile || !valid_identifier(text_profile) ||
            !valid_presentation || !valid_input_delay ||
            (presentation_name == "field_mode_text" &&
             input_delay_frames == 0U)) {
            self->bridge_failure =
                "game.ui.say expects an event, optional speaker/profile, and a valid presentation/input-delay contract";
            lua_pushboolean(lua, 0);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_say;
        ScriptDialogueRequest dialogue{};
        dialogue.speaker_id = std::move(speaker_id);
        dialogue.text_profile = std::move(text_profile);
        dialogue.event_id = std::move(event_id);
        dialogue.presentation = presentation_name == "field_mode_text"
            ? ScriptDialoguePresentation::field_mode_text
            : ScriptDialoguePresentation::field_npc;
        dialogue.input_delay_frames =
            static_cast<std::uint8_t>(input_delay_frames);
        operation.dialogue = std::move(dialogue);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, native_operation_continuation);
    }

    static int ui_say_text(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.say_text must run inside an active task";
            }
            lua_pushboolean(lua, 0);
            return 1;
        }

        std::size_t text_length = 0U;
        const auto* text = lua_type(lua, 1) == LUA_TSTRING
                               ? lua_tolstring(lua, 1, &text_length)
                               : nullptr;
        const bool valid_continuation =
            lua_isnoneornil(lua, 2) || lua_type(lua, 2) == LUA_TBOOLEAN;
        if (text == nullptr || text_length == 0U ||
            text_length > kMaximumDialogueTextBytes ||
            !valid_continuation ||
            !valid_identifier(self->location_text_profile) ||
            !valid_identifier(self->location_speaker_id)) {
            self->bridge_failure =
                "game.ui.say_text expects bounded text and an optional continuation flag in a profiled speaker location";
            lua_pushboolean(lua, 0);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_say;
        ScriptDialogueRequest dialogue{};
        dialogue.speaker_id = self->location_speaker_id;
        dialogue.text_profile = self->location_text_profile;
        dialogue.inline_text.assign(text, text_length);
        if (!lua_isnoneornil(lua, 2) && lua_toboolean(lua, 2) != 0) {
            dialogue.continuation_key = "shared.more";
        }
        operation.dialogue = std::move(dialogue);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, native_operation_continuation);
    }

    static int ui_story_page(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.story_page must run inside an active task";
            }
            lua_pushboolean(lua, 0);
            return 1;
        }

        std::string asset_alias;
        std::string text_key;
        std::uint32_t origin_x = 0U;
        std::uint32_t origin_y = 0U;
        std::uint32_t columns = 0U;
        std::uint32_t rows = 0U;
        std::array<std::uint32_t, 3> ink_rgb{};
        const bool valid =
            lua_gettop(lua) == 9 &&
            read_string(lua, 1, asset_alias) &&
            valid_identifier(asset_alias) &&
            read_string(lua, 2, text_key) && valid_identifier(text_key) &&
            read_integer(lua, 3, 319U, origin_x) &&
            read_integer(lua, 4, 223U, origin_y) &&
            read_integer(lua, 5, 40U, columns) && columns != 0U &&
            read_integer(lua, 6, 28U, rows) && rows != 0U &&
            read_integer(lua, 7, 255U, ink_rgb[0]) &&
            read_integer(lua, 8, 255U, ink_rgb[1]) &&
            read_integer(lua, 9, 255U, ink_rgb[2]);
        if (!valid) {
            self->bridge_failure =
                "game.ui.story_page expects an asset alias, text key, bounded text layout, and RGB8 ink";
            lua_pushboolean(lua, 0);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_story_page;
        ScriptNativeOperation::StoryPageRequest request{};
        request.asset_alias = std::move(asset_alias);
        request.text_key = std::move(text_key);
        request.origin_x = static_cast<std::uint16_t>(origin_x);
        request.origin_y = static_cast<std::uint16_t>(origin_y);
        request.columns = static_cast<std::uint8_t>(columns);
        request.rows = static_cast<std::uint8_t>(rows);
        for (std::size_t index = 0U; index < request.ink_rgb.size(); ++index) {
            request.ink_rgb[index] =
                static_cast<std::uint8_t>(ink_rgb[index]);
        }
        operation.story_page = std::move(request);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 0, native_operation_continuation);
    }

    static int ui_credits_page(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.credits_page must run inside an active task";
            }
            lua_pushboolean(lua, 0);
            return 1;
        }

        std::string asset_alias;
        std::string page_id;
        if (lua_gettop(lua) != 2 ||
            !read_string(lua, 1, asset_alias) ||
            !valid_identifier(asset_alias) ||
            !read_string(lua, 2, page_id) || !valid_identifier(page_id)) {
            self->bridge_failure =
                "game.ui.credits_page expects an asset alias and page id";
            lua_pushboolean(lua, 0);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_credits_page;
        ScriptNativeOperation::CreditsPageRequest request{};
        request.asset_alias = std::move(asset_alias);
        request.page_id = std::move(page_id);
        operation.credits_page = std::move(request);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 0, native_operation_continuation);
    }

    static int services_call(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.services.call must run inside an active task";
            }
            lua_pushnil(lua);
            return 1;
        }

        std::string domain;
        std::string action;
        if (!read_string(lua, 1, domain) || !valid_identifier(domain) ||
            !read_string(lua, 2, action) || !valid_identifier(action)) {
            self->bridge_failure =
                "game.services.call expects a service domain and action id";
            lua_pushnil(lua);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::service_call;
        operation.service_id = domain + "." + action;
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, native_operation_continuation);
    }

    void release_task() {
        task_waiting = false;
        pending_task_operation.reset();
        active_native_operation.reset();
        active_dialogue_open.reset();
        bridge_failure.clear();
        if (task_state != nullptr) {
            lua_settop(task_state, 0);
        }
        if (state != nullptr && task_reference != LUA_NOREF) {
            luaL_unref(state, LUA_REGISTRYINDEX, task_reference);
        }
        task_state = nullptr;
        task_reference = LUA_NOREF;
    }

    ScriptTaskStep failed_task(std::string message) {
        failure = std::move(message);
        release_task();
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }

    std::optional<ScriptDialogueOpenDescriptor>
    read_active_dialogue_open() const {
        if (task_state == nullptr) return std::nullopt;
        lua_getglobal(task_state, "game");
        if (!lua_istable(task_state, -1)) {
            lua_pop(task_state, 1);
            return std::nullopt;
        }
        lua_getfield(task_state, -1, "dialogue");
        if (!lua_istable(task_state, -1)) {
            lua_pop(task_state, 2);
            return std::nullopt;
        }
        lua_getfield(task_state, -1, "active");
        if (!lua_istable(task_state, -1)) {
            lua_pop(task_state, 3);
            return std::nullopt;
        }
        const auto descriptor_index = lua_absindex(task_state, -1);
        ScriptDialogueOpenDescriptor descriptor{};
        std::uint32_t transition_frames = 0U;
        std::uint32_t text_delay_frames = 0U;
        std::uint32_t input_lock_frames = 0U;
        const auto read_optional = [this, descriptor_index](
            const char* field, std::string& value) {
            lua_getfield(task_state, descriptor_index, field);
            const bool valid = lua_isnil(task_state, -1) ||
                read_string(task_state, -1, value);
            lua_pop(task_state, 1);
            return valid;
        };
        const bool valid =
            read_field_string(task_state, descriptor_index, "source",
                              descriptor.source) &&
            read_field_string(task_state, descriptor_index, "route_id",
                              descriptor.route_id) &&
            read_field_string(task_state, descriptor_index, "scene_outcome",
                              descriptor.scene_outcome) &&
            read_field_string(task_state, descriptor_index, "exit_behavior",
                              descriptor.exit_behavior) &&
            read_field_string(task_state, descriptor_index, "transition",
                              descriptor.transition) &&
            read_field_integer(task_state, descriptor_index,
                               "transition_frames", 255U,
                               transition_frames) &&
            read_optional("return_anchor", descriptor.return_anchor) &&
            read_optional("return_facing", descriptor.return_facing) &&
            read_optional("music_return_policy",
                          descriptor.music_return_policy) &&
            read_optional("input_lock", descriptor.input_lock) &&
            read_optional("scene_change_result_id",
                          descriptor.scene_change_result_id) &&
            read_field_integer(task_state, descriptor_index,
                               "text_delay_frames", 255U,
                               text_delay_frames) &&
            read_field_integer(task_state, descriptor_index,
                               "input_lock_frames", 255U,
                               input_lock_frames);
        lua_pop(task_state, 3);
        if (!valid) return std::nullopt;
        descriptor.transition_frames =
            static_cast<std::uint8_t>(transition_frames);
        descriptor.text_delay_frames =
            static_cast<std::uint8_t>(text_delay_frames);
        descriptor.input_lock_frames =
            static_cast<std::uint8_t>(input_lock_frames);
        return descriptor;
    }

    ScriptTaskStep start_task(
        const std::string_view module, const std::string_view function,
        const std::vector<std::string_view>& arguments) {
        if (state == nullptr) {
            failure = "gameplay Lua state is not available";
            return {ScriptTaskStep::Status::failed, std::nullopt};
        }
        if (task_state != nullptr) {
            failure = "a gameplay script task is already active";
            return {ScriptTaskStep::Status::failed, std::nullopt};
        }
        if (!valid_identifier(function)) {
            failure = "invalid gameplay script task id";
            return {ScriptTaskStep::Status::failed, std::nullopt};
        }
        if (!push_module(module)) {
            failure = "gameplay script module is missing: " +
                      std::string(module);
            return {ScriptTaskStep::Status::failed, std::nullopt};
        }
        lua_getfield(state, -1, std::string(function).c_str());
        if (!lua_isfunction(state, -1)) {
            lua_pop(state, 2);
            failure = "script task handler is missing: " +
                      std::string(module) + "." + std::string(function);
            return {ScriptTaskStep::Status::failed, std::nullopt};
        }
        lua_remove(state, -2);
        task_state = lua_newthread(state);
        task_reference = luaL_ref(state, LUA_REGISTRYINDEX);
        lua_xmove(state, task_state, 1);
        for (const auto argument : arguments) {
            lua_pushlstring(state, argument.data(), argument.size());
            lua_xmove(state, task_state, 1);
        }
        active_dialogue_open.reset();
        return resume_task(static_cast<int>(arguments.size()));
    }

    ScriptTaskStep resume_task(const int argument_count) {
        if (state == nullptr || task_state == nullptr) {
            return failed_task("no active gameplay script task");
        }
        bridge_failure.clear();
        int result_count = 0;
        int status = LUA_ERRRUN;
        {
            ScopedInstructionBudget instruction_budget(task_state);
            status = lua_resume(task_state, state, argument_count,
                                &result_count);
        }
        const auto active_dialogue = read_active_dialogue_open();
        if (active_dialogue.has_value()) {
            active_dialogue_open = active_dialogue;
        }
        if (!bridge_failure.empty()) {
            return failed_task(bridge_failure);
        }
        if (status == LUA_YIELD) {
            if (!task_waiting || !pending_task_operation.has_value()) {
                return failed_task(
                    "gameplay script yielded without a native operation");
            }
            ScriptTaskStep step{};
            step.status =
                ScriptTaskStep::Status::awaiting_native_operation;
            active_native_operation = std::move(pending_task_operation);
            pending_task_operation.reset();
            step.operation = active_native_operation;
            step.dialogue_open = active_dialogue_open;
            failure.clear();
            return step;
        }
        if (status != LUA_OK) {
            return failed_task(stack_error(task_state));
        }
        ScriptTaskStep step{};
        step.status = ScriptTaskStep::Status::completed;
        step.dialogue_open = active_dialogue_open;
        if (result_count == 1 && lua_type(task_state, -1) == LUA_TSTRING) {
            std::string result_id;
            if (!read_string(task_state, -1, result_id) ||
                !valid_identifier(result_id)) {
                return failed_task(
                    "gameplay script returned an invalid choice id");
            }
            step.result_id = std::move(result_id);
        } else if (result_count == 1 && lua_istable(task_state, -1)) {
            step.dialogue_outcome = read_dialogue_outcome(task_state, -1);
            if (!step.dialogue_outcome.has_value()) {
                return failed_task(
                    "gameplay task returned an invalid typed dialogue outcome");
            }
        } else if (result_count > 1 ||
                   (result_count == 1 &&
                    lua_type(task_state, -1) != LUA_TBOOLEAN &&
                    lua_type(task_state, -1) != LUA_TNIL)) {
            return failed_task(
                "gameplay task must return no value, a boolean, or a choice id");
        }
        lua_settop(task_state, 0);
        release_task();
        failure.clear();
        return step;
    }

    ~Impl() {
        if (state != nullptr) {
            lua_close(state);
        }
    }

    void reset() {
        if (state != nullptr) {
            lua_close(state);
        }
        state = luaL_newstate();
        task_state = nullptr;
        modules_reference = LUA_NOREF;
        task_reference = LUA_NOREF;
        menu_route_reference = LUA_NOREF;
        task_waiting = false;
        script_root.clear();
        modules_loading.clear();
        loaded_module_count = 0U;
        pending_task_operation.reset();
        active_native_operation.reset();
        story_facts.clear();
        conditional_actor_catalog.clear();
        activity_selector_bindings.clear();
        activity_group_bindings.clear();
        action_profile_index_by_actor_profile_id.clear();
        dialogue_texts.clear();
        location_type.clear();
        location_id.clear();
        location_text_profile.clear();
        location_speaker_id.clear();
        location_scene_id.reset();
        field_mode_context = ScriptFieldModeContext{};
        menu_route_bindings = ScriptMenuRouteBindings{};
        menu_route_state = ScriptMenuRouteSnapshot{};
        active_actor_context = nullptr;
        active_conditional_actor_random_bound = {};
        bridge_failure.clear();
        failure.clear();
        if (state == nullptr) {
            failure = "could not create Lua state";
            return;
        }
        open_safe_libraries(state);
        lua_newtable(state);
        modules_reference = luaL_ref(state, LUA_REGISTRYINDEX);

        lua_newtable(state);
        const auto gameplay_game_index = lua_absindex(state, -1);
        // Gameplay tasks and the field-menu route share these typed runner
        // and wallet host services. Lua owns treatment choices and updates;
        // the callbacks only read or commit the authoritative field records.
        add_menu_binding(state, gameplay_game_index, "party",
                         MenuBinding::party);
        add_menu_binding(state, gameplay_game_index, "active_slot",
                         MenuBinding::active_slot);
        add_menu_binding(state, gameplay_game_index, "commit_party",
                         MenuBinding::commit_party);
        add_menu_binding(state, gameplay_game_index, "nuyen",
                         MenuBinding::nuyen);
        add_menu_binding(state, gameplay_game_index, "set_nuyen",
                         MenuBinding::set_nuyen);
        lua_newtable(state); // Immutable-by-convention authored JSON content.
        lua_setfield(state, -2, "content");
        lua_newtable(state); // game.actors registry; Lua owns actor behavior state
        lua_setfield(state, -2, "actors");
        lua_newtable(state); // Independently-authored run scripts register here.
        lua_setfield(state, -2, "runs");
        lua_newtable(state); // game.state proxy
        lua_newtable(state); // game.state metatable
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, state_index, 1);
        lua_setfield(state, -2, "__index");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, state_newindex, 1);
        lua_setfield(state, -2, "__newindex");
        lua_setmetatable(state, -2);
        lua_setfield(state, -2, "state");
        lua_newtable(state); // game.location proxy
        lua_newtable(state); // game.location metatable
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, location_index, 1);
        lua_setfield(state, -2, "__index");
        lua_pushcfunction(state, location_newindex);
        lua_setfield(state, -2, "__newindex");
        lua_setmetatable(state, -2);
        lua_setfield(state, -2, "location");
        lua_newtable(state); // game.field proxy
        lua_newtable(state); // game.field metatable
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, field_context_index, 1);
        lua_setfield(state, -2, "__index");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, field_context_newindex, 1);
        lua_setfield(state, -2, "__newindex");
        lua_setmetatable(state, -2);
        lua_setfield(state, -2, "field");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, actor_walk_direction, 1);
        lua_setfield(state, -2, "walk_direction");
        lua_setfield(state, -2, "actor");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_menu, 1);
        lua_setfield(state, -2, "menu");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_prompt_text, 1);
        lua_setfield(state, -2, "prompt_text");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_prompt_text_key, 1);
        lua_setfield(state, -2, "prompt_text_key");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_cursor_menu, 1);
        lua_setfield(state, -2, "cursor_menu");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_say, 1);
        lua_setfield(state, -2, "say");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_say_text, 1);
        lua_setfield(state, -2, "say_text");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_story_page, 1);
        lua_setfield(state, -2, "story_page");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_credits_page, 1);
        lua_setfield(state, -2, "credits_page");
        lua_setfield(state, -2, "ui");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, services_call, 1);
        lua_setfield(state, -2, "call");
        lua_setfield(state, -2, "services");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_collision, 1);
        lua_setfield(state, -2, "probe");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_follow_route_step_clear, 1);
        lua_setfield(state, -2, "follow_route_step_clear");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_route, 1);
        lua_setfield(state, -2, "route");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_recover_collision, 1);
        lua_setfield(state, -2, "recover_collision");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_projectile_terrain, 1);
        lua_setfield(state, -2, "projectile_terrain");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_projectile_collision, 1);
        lua_setfield(state, -2, "projectile_collision");
        lua_setfield(state, -2, "movement");
        lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
        lua_setfield(state, -2, "modules");
        lua_setglobal(state, "game");

        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, require_module, 1);
        lua_setglobal(state, "require");
    }

    bool push_module(const std::string_view name) {
        if (state == nullptr || modules_reference == LUA_NOREF ||
            !valid_identifier(name)) {
            return false;
        }
        lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
        lua_getfield(state, -1, std::string(name).c_str());
        lua_remove(state, -2);
        if (!lua_istable(state, -1)) {
            lua_pop(state, 1);
            return false;
        }
        return true;
    }

    bool decode_json_document(const std::filesystem::path& path,
                              const std::size_t maximum_bytes,
                              std::string& error) {
        std::error_code file_error;
        const auto size = std::filesystem::file_size(path, file_error);
        if (file_error || size > maximum_bytes) {
            error = file_error
                        ? "could not read JSON file size: " +
                              file_error.message()
                        : "JSON file exceeds the byte limit";
            return false;
        }
        const auto base = lua_gettop(state);
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            error = "could not open JSON file";
            return false;
        }
        std::string source(static_cast<std::size_t>(size), '\0');
        if (!source.empty()) {
            input.read(source.data(),
                       static_cast<std::streamsize>(source.size()));
        }
        if (!input || input.gcount() !=
                          static_cast<std::streamsize>(source.size())) {
            error = "could not read complete JSON file";
            return false;
        }
        std::string module_error;
        if (!load_module(state, "json", module_error)) {
            lua_settop(state, base);
            error = module_error.empty()
                        ? "gameplay JSON decoder module is missing"
                        : std::move(module_error);
            return false;
        }
        const auto module_index = lua_absindex(state, -1);
        lua_getfield(state, module_index, "decode");
        if (!lua_isfunction(state, -1)) {
            lua_settop(state, base);
            error = "gameplay JSON decoder has no decode function";
            return false;
        }
        lua_remove(state, module_index);
        lua_pushlstring(state, source.data(), source.size());
        {
            ScopedInstructionBudget instruction_budget(
                state, kDialogueJsonInstructionBudget);
            if (lua_pcall(state, 1, 2, 0) != LUA_OK) {
                error = stack_error(state);
                lua_settop(state, base);
                return false;
            }
        }
        if (!lua_istable(state, -2)) {
            error = "invalid JSON document";
            if (lua_type(state, -1) == LUA_TSTRING) {
                std::size_t length = 0U;
                const auto* message = lua_tolstring(state, -1, &length);
                if (message != nullptr) {
                    error.assign(message, length);
                }
            }
            lua_settop(state, base);
            return false;
        }
        lua_pop(state, 1); // decoder error is nil on success
        const auto document_index = lua_absindex(state, -1);
        lua_getfield(state, document_index, "schemaVersion");
        const bool supported_schema =
            lua_isinteger(state, -1) && lua_tointeger(state, -1) == 1;
        lua_pop(state, 1);
        if (!supported_schema) {
            lua_settop(state, base);
            error = "JSON document must declare schemaVersion 1";
            return false;
        }
        return true;
    }

    bool load_json_content(const std::filesystem::path& content_root,
                           std::string& error,
                           const std::string_view namespace_prefix = {}) {
        std::error_code path_error;
        if (!std::filesystem::exists(content_root, path_error)) {
            if (!path_error) {
                return true;
            }
            error = "could not inspect gameplay JSON content directory: " +
                    path_error.message();
            return false;
        }
        const auto root =
            std::filesystem::weakly_canonical(content_root, path_error);
        if (path_error || !std::filesystem::is_directory(root, path_error) ||
            path_error) {
            error = "gameplay JSON content path is not a directory: " +
                    content_root.string();
            return false;
        }

        std::vector<std::filesystem::path> documents;
        for (std::filesystem::recursive_directory_iterator iterator(
                 root, path_error),
             end;
             !path_error && iterator != end; iterator.increment(path_error)) {
            if (!iterator->is_regular_file(path_error) || path_error ||
                iterator->path().extension() != ".json") {
                continue;
            }
            const auto document =
                std::filesystem::weakly_canonical(iterator->path(),
                                                  path_error);
            if (path_error || !path_is_within_root(root, document)) {
                error = "gameplay JSON file resolves outside content root: " +
                        iterator->path().string();
                return false;
            }
            documents.push_back(document);
            if (documents.size() > kMaximumGameplayJsonDocuments) {
                error = "gameplay JSON content exceeds the document limit";
                return false;
            }
        }
        if (path_error) {
            error = "could not enumerate gameplay JSON content: " +
                    path_error.message();
            return false;
        }
        std::sort(documents.begin(), documents.end());

        std::size_t total_bytes = 0U;
        for (const auto& path : documents) {
            std::error_code file_error;
            const auto size = std::filesystem::file_size(path, file_error);
            if (file_error || size > kMaximumDialogueJsonBytes ||
                size > kMaximumGameplayJsonBytes - total_bytes) {
                error = file_error
                            ? "could not read gameplay JSON file size: " +
                                  file_error.message()
                            : "gameplay JSON content exceeds its byte limit";
                return false;
            }
            total_bytes += static_cast<std::size_t>(size);

            const auto relative = path.lexically_relative(root);
            std::vector<std::string> keys;
            if (!namespace_prefix.empty()) {
                if (!valid_identifier(namespace_prefix)) {
                    error = "gameplay JSON namespace is invalid";
                    return false;
                }
                keys.emplace_back(namespace_prefix);
            }
            for (auto component = relative.begin();
                 component != relative.end(); ++component) {
                auto key = component->string();
                auto next_component = component;
                ++next_component;
                if (next_component == relative.end()) {
                    if (std::filesystem::path(key).extension() != ".json") {
                        error = "gameplay JSON filename has no .json suffix: " +
                                path.string();
                        return false;
                    }
                    key = std::filesystem::path(key).stem().string();
                }
                if (!valid_identifier(key)) {
                    error = "gameplay JSON path contains an invalid content id: " +
                            path.string();
                    return false;
                }
                keys.push_back(std::move(key));
            }
            if (keys.empty()) {
                error = "gameplay JSON path has no content id: " +
                        path.string();
                return false;
            }

            const auto base = lua_gettop(state);
            std::string decode_error;
            if (!decode_json_document(path, kMaximumDialogueJsonBytes,
                                      decode_error)) {
                error = path.string() + ": " + decode_error;
                lua_settop(state, base);
                return false;
            }
            const auto document_index = lua_absindex(state, -1);
            lua_getglobal(state, "game");
            lua_getfield(state, -1, "content");
            if (!lua_istable(state, -1)) {
                lua_settop(state, base);
                error = "game.content JSON registry is missing";
                return false;
            }
            lua_remove(state, -2);
            auto parent_index = lua_absindex(state, -1);
            for (std::size_t index = 0U; index + 1U < keys.size(); ++index) {
                const auto& key = keys[index];
                lua_getfield(state, parent_index, key.c_str());
                if (lua_isnil(state, -1)) {
                    lua_pop(state, 1);
                    lua_newtable(state);
                    lua_pushlstring(state, key.data(), key.size());
                    lua_pushvalue(state, -2);
                    lua_settable(state, parent_index);
                } else if (!lua_istable(state, -1)) {
                    lua_settop(state, base);
                    error = "gameplay JSON paths collide at content id: " + key;
                    return false;
                }
                parent_index = lua_absindex(state, -1);
            }
            const auto& content_id = keys.back();
            lua_getfield(state, parent_index, content_id.c_str());
            const bool duplicate = !lua_isnil(state, -1);
            lua_pop(state, 1);
            if (duplicate) {
                lua_settop(state, base);
                error = "duplicate gameplay JSON content id: " +
                        relative.generic_string();
                return false;
            }
            lua_pushlstring(state, content_id.data(), content_id.size());
            lua_pushvalue(state, document_index);
            lua_settable(state, parent_index);
            lua_settop(state, base);
        }
        return true;
    }
};

GameScriptRuntime::GameScriptRuntime() : impl_(std::make_unique<Impl>()) {}
GameScriptRuntime::~GameScriptRuntime() = default;
GameScriptRuntime::GameScriptRuntime(GameScriptRuntime&&) noexcept = default;
GameScriptRuntime& GameScriptRuntime::operator=(GameScriptRuntime&&) noexcept = default;

bool GameScriptRuntime::load_package(
    const std::filesystem::path& script_directory,
    const std::filesystem::path& dialogue_text_file,
    const std::filesystem::path& json_content_directory) {
    impl_->reset();
    if (impl_->state == nullptr) {
        return false;
    }
    std::error_code error;
    if (!std::filesystem::is_directory(script_directory, error) || error) {
        impl_->failure = "gameplay script directory is missing: " +
                         script_directory.string();
        return false;
    }
    impl_->script_root =
        std::filesystem::weakly_canonical(script_directory, error);
    if (error) {
        impl_->failure = "could not resolve gameplay script directory: " +
                         error.message();
        return false;
    }

    const auto world_directory = script_directory.parent_path() / "world";
    const auto field_asset_directory =
        script_directory.parent_path().parent_path() / "assets" / "field";
    std::vector<std::pair<std::string, std::uint16_t>> action_profile_bindings;
    std::vector<ScriptWorldActivityGroupBinding> activity_group_bindings;
    if (!detail::load_world_content_tsv(
            impl_->state, impl_->modules_reference, world_directory,
            impl_->failure, field_asset_directory,
            &impl_->conditional_actor_catalog,
            &impl_->activity_selector_bindings, &activity_group_bindings,
            &action_profile_bindings)) {
        return false;
    }
    impl_->activity_group_bindings = std::move(activity_group_bindings);
    for (const auto& [profile_id, source_profile_index] :
         action_profile_bindings) {
        const auto [entry, inserted] =
            impl_->action_profile_index_by_actor_profile_id.emplace(
                profile_id, source_profile_index);
        if (!inserted && entry->second != source_profile_index) {
            impl_->failure =
                "actor profile has conflicting action profile bindings: " +
                profile_id;
            return false;
        }
    }
    const auto gameplay_dialogue_directory = json_content_directory.empty()
        ? script_directory.parent_path() / "dialogue"
        : json_content_directory;
    if (!impl_->load_json_content(gameplay_dialogue_directory,
                                  impl_->failure)) {
        return false;
    }
    if (!impl_->load_json_content(world_directory / "entities",
                                  impl_->failure, "entities")) {
        return false;
    }

    std::vector<std::filesystem::path> files;
    for (std::filesystem::directory_iterator iterator(script_directory, error),
         end;
         !error && iterator != end; iterator.increment(error)) {
        if (iterator->is_regular_file(error) &&
            iterator->path().extension() == ".lua") {
            files.push_back(iterator->path());
        }
    }
    if (error) {
        impl_->failure = "could not enumerate gameplay scripts: " +
                         error.message();
        return false;
    }
    std::sort(files.begin(), files.end());
    if (files.empty() || files.size() > kMaximumScriptModules) {
        impl_->failure = files.empty()
                             ? "gameplay script package has no .lua modules"
                             : "gameplay script package exceeds module limit";
        return false;
    }

    auto* state = impl_->state;
    bool has_run_modules = false;
    for (const auto& file : files) {
        const auto module = file.stem().string();
        if (!valid_identifier(module)) {
            impl_->failure = "invalid gameplay script module name: " + module;
            return false;
        }
        // This data module is populated directly from gameplay/world TSV
        // files above. Never let a stale or copied Lua snapshot replace it.
        if (module == "world_content_data") {
            continue;
        }
        std::string module_error;
        if (!impl_->load_module(state, module, module_error)) {
            impl_->failure = file.filename().string() + ": " +
                             module_error;
            return false;
        }
        const auto definition_index = lua_absindex(state, -1);

        // Run content is a Lua plug-in family. Adding run_<id>.lua makes its
        // returned table available to other scripts through game.runs[id];
        // no native per-run switch or registration edit is needed.
        constexpr std::string_view kRunModulePrefix = "run_";
        if (module.rfind(kRunModulePrefix, 0U) == 0U &&
            module.size() > kRunModulePrefix.size() &&
            lua_istable(state, definition_index)) {
            lua_getfield(state, definition_index, "prepare_offer");
            const bool has_prepare_offer = lua_isfunction(state, -1);
            lua_pop(state, 1);
            lua_getfield(state, definition_index, "present_offer");
            const bool has_present_offer = lua_isfunction(state, -1);
            lua_pop(state, 1);
            if (!has_prepare_offer || !has_present_offer) {
                lua_pop(state, 1);
                continue;
            }

            const auto run_id = std::string_view(module).substr(
                kRunModulePrefix.size());
            if (!valid_identifier(run_id)) {
                impl_->failure = "invalid Lua run id in module name: " +
                                 module;
                lua_pop(state, 1);
                return false;
            }
            lua_getglobal(state, "game");
            lua_getfield(state, -1, "runs");
            lua_pushlstring(state, run_id.data(), run_id.size());
            lua_pushvalue(state, definition_index);
            lua_settable(state, -3);
            lua_pop(state, 2);
            has_run_modules = true;
        }
        lua_pop(state, 1);
    }

    // Run modules are registered above as game.runs[run_id]. Publish the
    // shared registry alongside them so gameplay scripts can read and update
    // the active offer through the same Lua-owned game facade.
    if (has_run_modules) {
        const auto stack_base = lua_gettop(state);
        if (!impl_->push_module("run_registry") || !lua_istable(state, -1)) {
            lua_settop(state, stack_base);
            impl_->failure = "run_registry.lua module is missing";
            return false;
        }
        const auto registry_index = lua_absindex(state, -1);
        lua_getglobal(state, "game");
        lua_getfield(state, -1, "runs");
        if (!lua_istable(state, -1)) {
            lua_settop(state, stack_base);
            impl_->failure = "game.runs registry table is missing";
            return false;
        }
        lua_pushvalue(state, registry_index);
        lua_setfield(state, -2, "registry");
        lua_settop(state, stack_base);
    }

    if (!dialogue_text_file.empty()) {
        error.clear();
        const auto fail_dialogue = [this, state](const std::string& message) {
            impl_->failure = message;
            lua_settop(state, 0);
            return false;
        };
        if (!std::filesystem::is_regular_file(dialogue_text_file, error) ||
            error) {
            return fail_dialogue(
                "gameplay dialogue JSON is missing: " +
                dialogue_text_file.string());
        }
        const auto dialogue_size =
            std::filesystem::file_size(dialogue_text_file, error);
        if (error || dialogue_size > kMaximumDialogueJsonBytes) {
            return fail_dialogue(
                error ? "could not read gameplay dialogue JSON size: " +
                            error.message()
                      : "gameplay dialogue JSON exceeds the byte limit");
        }
        std::ifstream dialogue_input(dialogue_text_file, std::ios::binary);
        if (!dialogue_input) {
            return fail_dialogue(
                "could not open gameplay dialogue JSON: " +
                dialogue_text_file.string());
        }
        std::string dialogue_json(
            static_cast<std::size_t>(dialogue_size), '\0');
        if (!dialogue_json.empty()) {
            dialogue_input.read(
                dialogue_json.data(),
                static_cast<std::streamsize>(dialogue_json.size()));
        }
        if (!dialogue_input ||
            dialogue_input.gcount() !=
                static_cast<std::streamsize>(dialogue_json.size())) {
            return fail_dialogue(
                "could not read complete gameplay dialogue JSON: " +
                dialogue_text_file.string());
        }
        if (!impl_->push_module("json")) {
            return fail_dialogue(
                "gameplay JSON decoder module is missing");
        }
        const auto json_module = lua_absindex(state, -1);
        lua_getfield(state, json_module, "decode");
        if (!lua_isfunction(state, -1)) {
            return fail_dialogue(
                "gameplay JSON decoder has no decode function");
        }
        lua_remove(state, json_module);
        lua_pushlstring(state, dialogue_json.data(), dialogue_json.size());
        {
            ScopedInstructionBudget instruction_budget(
                state, kDialogueJsonInstructionBudget);
            if (lua_pcall(state, 1, 2, 0) != LUA_OK) {
                return fail_dialogue(
                    dialogue_text_file.filename().string() +
                    " could not be decoded: " +
                    stack_error(state));
            }
        }
        if (!lua_istable(state, -2)) {
            std::string decode_error = "invalid JSON document";
            if (lua_type(state, -1) == LUA_TSTRING) {
                std::size_t length = 0U;
                const auto* message = lua_tolstring(state, -1, &length);
                if (message != nullptr) {
                    decode_error.assign(message, length);
                }
            }
            return fail_dialogue(
                dialogue_text_file.filename().string() +
                " is invalid: " + decode_error);
        }
        lua_pop(state, 1); // decode error is nil on success
        const auto document_index = lua_absindex(state, -1);
        lua_getfield(state, document_index, "schemaVersion");
        const bool supported_schema =
            lua_isinteger(state, -1) && lua_tointeger(state, -1) == 1;
        lua_pop(state, 1);
        if (!supported_schema) {
            return fail_dialogue(
                "dialogue JSON must declare schemaVersion 1");
        }
        lua_getfield(state, document_index, "texts");
        if (!lua_istable(state, -1)) {
            return fail_dialogue(
                "dialogue JSON must contain a texts object");
        }
        const auto texts_table = lua_absindex(state, -1);
        std::unordered_map<std::string, std::string> parsed_texts;
        lua_pushnil(state);
        while (lua_next(state, texts_table) != 0) {
            std::string key;
            const bool valid_key = read_string(state, -2, key);
            const bool string_value = lua_type(state, -1) == LUA_TSTRING;
            std::size_t text_length = 0U;
            const auto* text = string_value
                                   ? lua_tolstring(state, -1, &text_length)
                                   : nullptr;
            // Empty Lua strings represent source item categories that are
            // intentionally blank; missing keys remain distinguishable.
            const bool valid_text = string_value && text != nullptr &&
                                    text_length <= kMaximumDialogueTextBytes;
            if (!valid_key || !valid_identifier(key) || !valid_text ||
                parsed_texts.size() >= kMaximumDialogueTextEntries) {
                lua_pop(state, 2);
                std::string message =
                    "dialogue text script contains an invalid entry";
                if (valid_key) {
                    message += " at key '" + key + "'";
                } else {
                    message += ": key is not a string";
                }
                if (valid_key && !valid_identifier(key)) {
                    message += ": key is not a valid identifier";
                } else if (!valid_text) {
                    message += ": value is not a bounded string";
                } else if (parsed_texts.size() >=
                           kMaximumDialogueTextEntries) {
                    message += ": entry limit exceeded";
                }
                return fail_dialogue(message);
            }
            if (!parsed_texts.emplace(
                    std::move(key), std::string(text, text_length)).second) {
                lua_pop(state, 2);
                return fail_dialogue(
                    "dialogue text script contains a duplicate key");
            }
            lua_pop(state, 1);
        }
        if (parsed_texts.empty()) {
            return fail_dialogue("dialogue text script has no entries");
        }
        impl_->dialogue_texts = std::move(parsed_texts);
        lua_settop(state, 0);
    }
    impl_->failure.clear();
    return true;
}

const std::string* GameScriptRuntime::dialogue_text(
    const std::string_view key) const {
    const auto found = impl_->dialogue_texts.find(std::string(key));
    return found == impl_->dialogue_texts.end() ? nullptr : &found->second;
}

const std::unordered_map<std::string, std::string>&
GameScriptRuntime::dialogue_text_entries() const noexcept {
    return impl_->dialogue_texts;
}

std::optional<FieldScriptRoute>
GameScriptRuntime::resolve_field_location_route(
    const FieldScriptRouteContext& context) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for field location routing";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<FieldScriptRoute> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("building")) {
        return fail("building.lua module is missing");
    }
    lua_getfield(state, -1, "resolve_field_location_route");
    if (!lua_isfunction(state, -1)) {
        return fail("building.lua route function is missing");
    }

    lua_createtable(state, 0, 5);
    const auto input_index = lua_absindex(state, -1);
    lua_pushinteger(state, context.scene_id);
    lua_setfield(state, input_index, "scene_id");
    lua_pushboolean(
        state, context.field_mode == field_exit_mode_value(
                                         FieldExitMode::building_interaction));
    lua_setfield(state, input_index, "building_interaction_mode");
    const auto event_code_value =
        field_collision_event_value(context.event_code);
    const auto event_code_name = collision_event_name(context.event_code);
    lua_pushlstring(state, event_code_name.c_str(), event_code_name.size());
    lua_setfield(state, input_index, "event_code");
    lua_createtable(state, 0, 2);
    const auto transition_index = lua_absindex(state, -1);
    std::string_view transition_kind = "no_destination";
    // [C] The profile selected through 0x3232 stores 32 scene destinations
    // at profile+0x4E. Resolve the selected host record here and expose only
    // the resulting world transition to gameplay Lua.
    if (context.event_table_available &&
        event_code_value < context.event_table.size()) {
        transition_kind = "enter_scene";
    } else if (!context.event_table_available && event_code_value == 0U) {
        transition_kind = "return_to_field";
    }
    lua_pushlstring(state, transition_kind.data(), transition_kind.size());
    lua_setfield(state, transition_index, "kind");
    if (transition_kind == "enter_scene") {
        lua_pushinteger(state, context.event_table[event_code_value]);
        lua_setfield(state, transition_index, "destination_scene_id");
    }
    lua_setfield(state, input_index, "transition");

    // Route predicates receive semantic field facts. The source event
    // identity remains in the route input above; raw mode/event selectors do
    // not enter Lua's condition context.
    lua_createtable(state, 0, 2);
    const auto field_index = lua_absindex(state, -1);
    lua_pushboolean(
        state, context.field_mode == field_exit_mode_value(
                                         FieldExitMode::building_interaction));
    lua_setfield(state, field_index, "building_interaction_mode");
    const auto package_run_status = script_package_run_status_id(
        script_package_run_status_from_source_result(
            context.alternate_result));
    lua_pushlstring(state, package_run_status.data(),
                    package_run_status.size());
    lua_setfield(state, field_index, "package_run_status");
    lua_setfield(state, input_index, "field");

    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        std::size_t message_length = 0U;
        const auto* message = lua_tolstring(state, -1, &message_length);
        return fail(message == nullptr
                        ? "building.lua field location route failed"
                        : "building.lua field location route failed: " +
                              std::string(message, message_length));
    }
    if (!lua_istable(state, -1)) {
        return fail("building.lua field location route returned no decision");
    }
    const auto result_index = lua_absindex(state, -1);
    std::string kind_name;
    std::string script_name;
    std::string resolved_event_code_name;
    FieldCollisionEventCode event_code = FieldCollisionEventCode::event_0;
    if (!read_field_string(state, result_index, "kind", kind_name) ||
        !read_field_string(state, result_index, "script", script_name) ||
        !read_field_string(state, result_index, "event_code",
                           resolved_event_code_name) ||
        !parse_collision_event_name(resolved_event_code_name, event_code)) {
        return fail("building.lua field location route has invalid fields");
    }

    std::optional<std::uint8_t> target_scene;
    lua_getfield(state, result_index, "area_event_target");
    if (!lua_isnil(state, -1)) {
        std::uint32_t value = 0U;
        const bool valid = read_integer(state, -1, 0xFFU, value);
        lua_pop(state, 1);
        if (!valid) {
            return fail("building.lua route has an invalid area event target");
        }
        target_scene = static_cast<std::uint8_t>(value);
    } else {
        lua_pop(state, 1);
    }

    std::optional<WorldActivityMode> destination_mode;
    lua_getfield(state, result_index, "area_event_destination_mode");
    if (!lua_isnil(state, -1)) {
        std::string mode_name;
        const bool valid = read_string(state, -1, mode_name);
        lua_pop(state, 1);
        if (!valid) {
            return fail("building.lua route has an invalid destination mode");
        }
        if (mode_name == "field") {
            destination_mode = WorldActivityMode::field;
        } else if (mode_name == "resource_activity") {
            destination_mode = WorldActivityMode::resource_activity;
        } else if (mode_name == "scripted_activity") {
            destination_mode = WorldActivityMode::scripted_activity;
        } else if (mode_name == "halted") {
            destination_mode = WorldActivityMode::halted;
        } else {
            return fail("building.lua route returned an unknown destination mode");
        }
    } else {
        lua_pop(state, 1);
    }

    auto return_anchor = FieldLocationReturnAnchor::none;
    lua_getfield(state, result_index, "return_anchor");
    if (!lua_isnil(state, -1)) {
        std::string anchor_name;
        const bool valid = read_string(state, -1, anchor_name);
        lua_pop(state, 1);
        if (!valid) {
            return fail("building.lua route has an invalid return anchor");
        }
        if (anchor_name == "" || anchor_name == "none") {
            return_anchor = FieldLocationReturnAnchor::none;
        } else if (anchor_name == "collision_probe_lead") {
            return_anchor =
                FieldLocationReturnAnchor::collision_probe_lead;
        } else {
            return fail("building.lua route returned an unknown return anchor");
        }
    } else {
        lua_pop(state, 1);
    }

    auto return_facing =
        FieldLocationReturnFacing::preserve_entry_heading;
    lua_getfield(state, result_index, "return_facing");
    if (!lua_isnil(state, -1)) {
        std::string facing_name;
        const bool valid = read_string(state, -1, facing_name);
        lua_pop(state, 1);
        if (!valid) {
            return fail("building.lua route has an invalid return facing");
        }
        if (facing_name == "" || facing_name == "preserve_entry_heading" ||
            facing_name == "preserve_entry") {
            return_facing =
                FieldLocationReturnFacing::preserve_entry_heading;
        } else if (facing_name == "face_away_from_entry") {
            return_facing = FieldLocationReturnFacing::face_away_from_entry;
        } else {
            return fail("building.lua route returned an unknown return facing");
        }
    } else {
        lua_pop(state, 1);
    }

    auto return_music_policy = FieldLocationMusicReturnPolicy::none;
    lua_getfield(state, result_index, "return_music_policy");
    if (!lua_isnil(state, -1)) {
        std::string policy_name;
        const bool valid = read_string(state, -1, policy_name);
        lua_pop(state, 1);
        if (!valid) {
            return fail("building.lua route has an invalid return music policy");
        }
        if (policy_name == "" || policy_name == "none") {
            return_music_policy = FieldLocationMusicReturnPolicy::none;
        } else if (policy_name == "resume_scene_profile") {
            return_music_policy =
                FieldLocationMusicReturnPolicy::resume_scene_profile;
        } else {
            return fail("building.lua route returned an unknown return music policy");
        }
    } else {
        lua_pop(state, 1);
    }

    auto scene_outcome = FieldLocationSceneOutcome::scene_change;
    lua_getfield(state, result_index, "scene_outcome");
    if (!lua_isnil(state, -1)) {
        std::string outcome_name;
        const bool valid = read_string(state, -1, outcome_name);
        lua_pop(state, 1);
        if (!valid) {
            return fail("building.lua route has an invalid scene outcome");
        }
        if (outcome_name == "same_scene") {
            scene_outcome = FieldLocationSceneOutcome::same_scene;
        } else if (outcome_name == "source_door_return") {
            scene_outcome = FieldLocationSceneOutcome::source_door_return;
        } else if (outcome_name == "scene_change") {
            scene_outcome = FieldLocationSceneOutcome::scene_change;
        } else {
            return fail("building.lua route returned an unknown scene outcome");
        }
    } else {
        lua_pop(state, 1);
    }

    auto scene_transition_policy = FieldSceneTransitionPolicy::none;
    lua_getfield(state, result_index, "scene_transition_policy");
    if (!lua_isnil(state, -1)) {
        std::string policy_name;
        const bool valid = read_string(state, -1, policy_name);
        lua_pop(state, 1);
        if (!valid) {
            return fail(
                "building.lua route has an invalid scene transition policy");
        }
        if (policy_name == "none") {
            scene_transition_policy = FieldSceneTransitionPolicy::none;
        } else if (policy_name == "building_fade") {
            scene_transition_policy =
                FieldSceneTransitionPolicy::building_fade;
        } else {
            return fail(
                "building.lua route returned an unknown scene transition policy");
        }
    } else {
        lua_pop(state, 1);
    }

    std::uint8_t scene_transition_vblanks = 0U;
    lua_getfield(state, result_index, "scene_transition_vblanks");
    if (!lua_isnil(state, -1)) {
        if (!lua_isinteger(state, -1)) {
            lua_pop(state, 1);
            return fail(
                "building.lua route has an invalid scene transition duration");
        }
        const auto vblanks = lua_tointeger(state, -1);
        lua_pop(state, 1);
        if (vblanks < 0 || vblanks > 0xFF ||
            (scene_transition_policy ==
                 FieldSceneTransitionPolicy::none &&
             vblanks != 0) ||
            (scene_transition_policy ==
                 FieldSceneTransitionPolicy::building_fade &&
             (vblanks != kRomBuildingTransitionVBlanks ||
              scene_outcome !=
                  FieldLocationSceneOutcome::scene_change))) {
            return fail(
                "building.lua route returned an invalid scene transition duration");
        }
        scene_transition_vblanks = static_cast<std::uint8_t>(vblanks);
    } else {
        lua_pop(state, 1);
    }
    if (scene_transition_policy ==
            FieldSceneTransitionPolicy::building_fade &&
        scene_transition_vblanks != kRomBuildingTransitionVBlanks) {
        return fail(
            "building.lua route omitted the ROM scene transition duration");
    }

    const auto read_optional_string =
        [state, result_index](const char* name, std::string& value) {
            lua_getfield(state, result_index, name);
            if (lua_isnil(state, -1)) {
                lua_pop(state, 1);
                value.clear();
                return true;
            }
            if (lua_type(state, -1) != LUA_TSTRING) {
                lua_pop(state, 1);
                return false;
            }
            std::size_t length = 0U;
            const auto* bytes = lua_tolstring(state, -1, &length);
            value.assign(bytes, length);
            lua_pop(state, 1);
            return true;
        };

    const auto read_location_task =
        [state, result_index](const char* name,
                             std::optional<FieldLocationScriptTask>& task) {
            lua_getfield(state, result_index, name);
            if (lua_isnil(state, -1)) {
                lua_pop(state, 1);
                task.reset();
                return true;
            }
            if (!lua_istable(state, -1)) {
                lua_pop(state, 1);
                return false;
            }
            const auto table_index = lua_absindex(state, -1);
            const auto read_required =
                [state, table_index](const char* field, std::string& value) {
                    lua_getfield(state, table_index, field);
                    if (lua_type(state, -1) != LUA_TSTRING) {
                        lua_pop(state, 1);
                        return false;
                    }
                    std::size_t length = 0U;
                    const auto* bytes = lua_tolstring(state, -1, &length);
                    value.assign(bytes, length);
                    lua_pop(state, 1);
                    return !value.empty();
                };
            FieldLocationScriptTask parsed{};
            const bool valid =
                read_required("module", parsed.module) &&
                read_required("function_name", parsed.function) &&
                read_required("location_type", parsed.location_type) &&
                read_required("location_id", parsed.location_id) &&
                read_required("text_profile", parsed.text_profile) &&
                read_required("speaker_id", parsed.speaker_id);
            lua_pop(state, 1);
            if (!valid) {
                return false;
            }
            task = std::move(parsed);
            return true;
        };

    const auto read_resource_scene_entry_placement =
        [state, result_index](
            std::optional<ResourceSceneEntryPlacement>& placement) {
            lua_getfield(state, result_index,
                         "resource_scene_entry_placement");
            if (lua_isnil(state, -1)) {
                lua_pop(state, 1);
                placement.reset();
                return true;
            }
            if (!lua_istable(state, -1)) {
                lua_pop(state, 1);
                return false;
            }
            const auto placement_index = lua_absindex(state, -1);
            const auto read_word = [state](const int table_index,
                                           const char* field,
                                           std::uint16_t& value) {
                lua_getfield(state, table_index, field);
                std::uint32_t parsed = 0U;
                const bool valid = read_integer(state, -1, 0xFFFFU, parsed);
                lua_pop(state, 1);
                if (valid) {
                    value = static_cast<std::uint16_t>(parsed);
                }
                return valid;
            };
            ResourceSceneEntryPlacement parsed{};
            bool valid = read_word(placement_index, "camera_x",
                                   parsed.camera_x) &&
                         read_word(placement_index, "camera_y",
                                   parsed.camera_y);
            lua_getfield(state, placement_index, "runner_positions");
            if (!lua_istable(state, -1) || lua_rawlen(state, -1) != 3U) {
                valid = false;
            } else {
                const auto positions_index = lua_absindex(state, -1);
                for (std::size_t slot = 0U;
                     slot < parsed.runner_positions.size(); ++slot) {
                    lua_rawgeti(state, positions_index,
                                static_cast<lua_Integer>(slot + 1U));
                    if (!lua_istable(state, -1)) {
                        valid = false;
                        lua_pop(state, 1);
                        continue;
                    }
                    const auto position_index = lua_absindex(state, -1);
                    valid = read_word(position_index, "x",
                                      parsed.runner_positions[slot].x) && valid;
                    valid = read_word(position_index, "y",
                                      parsed.runner_positions[slot].y) && valid;
                    lua_pop(state, 1);
                }
            }
            lua_pop(state, 1);
            lua_pop(state, 1);
            if (valid) {
                placement = parsed;
            }
            return valid;
        };

    std::optional<ResourceSceneEntryPlacement> resource_scene_entry_placement;
    if (!read_resource_scene_entry_placement(resource_scene_entry_placement)) {
        return fail(
            "building.lua route has an invalid resource-scene entry placement");
    }

    FieldScriptRoute route{};
    route.event_code = field_collision_event_value(event_code);
    route.area_event_target = target_scene;
    route.area_event_destination_mode = destination_mode;
    route.return_anchor = return_anchor;
    route.return_facing = return_facing;
    route.return_music_policy = return_music_policy;
    route.scene_outcome = scene_outcome;
    route.scene_transition_policy = scene_transition_policy;
    route.scene_transition_vblanks = scene_transition_vblanks;
    route.resource_scene_entry_placement = resource_scene_entry_placement;
    std::string resource_override_name;
    std::string entry_continue_outcome_type_name;
    if (!read_optional_string("entry_dialogue_module",
                              route.entry_dialogue_module) ||
        !read_optional_string("entry_dialogue_function",
                              route.entry_dialogue_function) ||
        !read_optional_string("entry_location_type",
                              route.entry_location_type) ||
        !read_optional_string("entry_location_id",
                              route.entry_location_id) ||
        !read_optional_string("entry_text_profile",
                              route.entry_text_profile) ||
        !read_optional_string("entry_speaker_id",
                              route.entry_speaker_id) ||
        !read_optional_string("entry_event_id",
                              route.entry_event_id) ||
        !read_optional_string("entry_continue_result_id",
                              route.entry_continue_result_id) ||
        !read_optional_string("entry_continue_outcome_type",
                              entry_continue_outcome_type_name) ||
        !read_optional_string("entry_resource_override",
                              resource_override_name) ||
        !read_optional_string("boundary_reason", route.boundary_reason) ||
        !read_optional_string("field_service_id",
                              route.field_service_id) ||
        !read_location_task("route_preparation",
                            route.route_preparation) ||
        !read_location_task("indexed_title_service",
                            route.indexed_title_service) ||
        !read_location_task("field_exit_prompt", route.field_exit_prompt)) {
        return fail("building.lua route has invalid entry dialogue fields");
    }
    if (!entry_continue_outcome_type_name.empty()) {
        route.entry_continue_outcome_type = dialogue_outcome_type_from_name(
            entry_continue_outcome_type_name);
        if (!route.entry_continue_outcome_type.has_value() ||
            !route.entry_continue_result_id.empty()) {
            return fail(
                "building.lua route has an invalid or conflicting entry outcome");
        }
    }
    if (resource_override_name.empty() || resource_override_name == "none" ||
        resource_override_name == "-") {
        route.entry_resource_override =
            FieldLocationResourceEntryOverride::none;
    } else if (resource_override_name == "index_five_marker_zero") {
        route.entry_resource_override =
            FieldLocationResourceEntryOverride::index_five_marker_zero;
    } else if (resource_override_name == "index_two_marker_three") {
        route.entry_resource_override =
            FieldLocationResourceEntryOverride::index_two_marker_three;
    } else if (resource_override_name == "index_four") {
        route.entry_resource_override =
            FieldLocationResourceEntryOverride::index_four;
    } else {
        return fail("building.lua route has an unknown resource entry override");
    }
    const bool no_target = !target_scene.has_value();
    if (kind_name == "no_route" && script_name == "none" && no_target) {
        route.kind = FieldScriptRouteKind::no_route;
        route.script = FieldScriptId::none;
    } else if (kind_name == "dialogue" &&
               script_name == "lua_dialogue" && no_target &&
               !route.entry_dialogue_module.empty() &&
               !route.entry_dialogue_function.empty()) {
        route.kind = FieldScriptRouteKind::dialogue;
        route.script = FieldScriptId::lua_dialogue;
    } else if (kind_name == "field_service" &&
               script_name == "host_field_service" && no_target &&
               !route.field_service_id.empty()) {
        route.kind = FieldScriptRouteKind::field_service;
        route.script = FieldScriptId::host_field_service;
    } else if (kind_name == "field_service" &&
               script_name == "lua_field_service" && no_target &&
               route.indexed_title_service.has_value()) {
        route.kind = FieldScriptRouteKind::field_service;
        route.script = FieldScriptId::lua_field_service;
    } else if (kind_name == "area_event" &&
               script_name == "area_event_table" && target_scene &&
               destination_mode.has_value()) {
        route.kind = FieldScriptRouteKind::area_event;
        route.script = FieldScriptId::area_event_table;
    } else if (kind_name == "source_activity_return" &&
               script_name == "source_activity_return" && no_target &&
               !destination_mode.has_value() &&
               ((context.scene_id >= 21U && context.scene_id <= 43U) ||
                context.scene_id == field_scene_id_value(
                    FieldSceneId::jump_house) ||
                context.scene_id == field_scene_id_value(
                    FieldSceneId::wanderer)) &&
               context.event_code == FieldCollisionEventCode::event_0 &&
               !context.event_table_available) {
        route.kind = FieldScriptRouteKind::source_activity_return;
        route.script = FieldScriptId::source_activity_return;
    } else if (kind_name == "source_boundary" &&
               script_name == "source_boundary" && no_target &&
               !route.boundary_reason.empty()) {
        route.kind = FieldScriptRouteKind::source_boundary;
        route.script = FieldScriptId::source_boundary;
    } else {
        return fail("building.lua route returned an unknown action");
    }

    const bool has_entry_dialogue =
        !route.entry_dialogue_module.empty() ||
        !route.entry_dialogue_function.empty() ||
        !route.entry_location_type.empty() ||
        !route.entry_location_id.empty() ||
        !route.entry_text_profile.empty() ||
        !route.entry_speaker_id.empty() ||
        !route.entry_event_id.empty() ||
        !route.entry_continue_result_id.empty() ||
        route.entry_continue_outcome_type.has_value();
    if (has_entry_dialogue &&
        ((route.kind != FieldScriptRouteKind::area_event &&
          route.kind != FieldScriptRouteKind::dialogue) ||
         (route.kind == FieldScriptRouteKind::area_event && no_target) ||
         (route.kind == FieldScriptRouteKind::dialogue && !no_target) ||
         route.entry_dialogue_module.empty() ||
         route.entry_dialogue_function.empty() ||
         route.entry_location_type.empty() ||
         route.entry_location_id.empty())) {
        return fail("building.lua route has incomplete entry dialogue data");
    }

    if (!route.boundary_reason.empty() &&
        route.kind != FieldScriptRouteKind::source_boundary) {
        return fail("building.lua route misplaced its source boundary reason");
    }

    if (route.entry_resource_override !=
            FieldLocationResourceEntryOverride::none &&
        (route.kind != FieldScriptRouteKind::area_event ||
         destination_mode != WorldActivityMode::resource_activity ||
         route.entry_dialogue_module.empty() ||
         (route.entry_continue_result_id.empty() &&
          !route.entry_continue_outcome_type.has_value()))) {
        return fail(
            "resource entry override needs an accepted resource-activity dialogue");
    }

    if (route.resource_scene_entry_placement.has_value() &&
        (route.kind != FieldScriptRouteKind::area_event ||
         !target_scene.has_value() || !destination_mode.has_value() ||
         *destination_mode != WorldActivityMode::resource_activity ||
         *target_scene < field_scene_id_value(
                             FieldSceneId::first_resource_activity) ||
         *target_scene > field_scene_id_value(
                             FieldSceneId::last_resource_activity))) {
        return fail(
            "resource-scene entry placement is outside an ordinary resource route");
    }

    if ((route.indexed_title_service.has_value() &&
         (route.kind != FieldScriptRouteKind::field_service ||
          route.script != FieldScriptId::lua_field_service)) ||
        (route.route_preparation.has_value() &&
         route.kind != FieldScriptRouteKind::area_event) ||
        (route.field_exit_prompt.has_value() &&
         route.kind != FieldScriptRouteKind::area_event) ||
        (!route.field_service_id.empty() &&
         (route.kind != FieldScriptRouteKind::field_service ||
          route.script != FieldScriptId::host_field_service))) {
        return fail("building.lua route has a misplaced location script task");
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return route;
}

ScriptFieldSceneMode script_field_scene_mode_from_source_word(
    const std::uint16_t source_mode) noexcept {
    // ROM 0x167AE writes FFDF40 before the field actor-selection pass. Keep
    // that word at the boundary and pass the selector a named scene mode.
    switch (source_mode) {
    case 0U:
        return ScriptFieldSceneMode::none;
    case 1U:
        return ScriptFieldSceneMode::street;
    case 2U:
        return ScriptFieldSceneMode::building;
    case 3U:
        return ScriptFieldSceneMode::cave;
    default:
        return ScriptFieldSceneMode::unknown;
    }
}

ScriptActorBehaviorProfile
script_actor_behavior_profile_from_source_posture(
    const std::uint8_t source_posture) noexcept {
    // [C] ROM 0x13EF4 sends 0-2 to patrol, 0x13F52 sends 3 through the
    // runner-watch branch, 0x13EF4 checks the close threshold for 4, and
    // 0x13F36 sends values above 4 to combat. ROM 0x13D84 indexes the range
    // table at 0x13E58: 0-2 far, 3-5 medium, 6-8 close. Values outside that
    // table keep their think policy but have no inferred movement band.
    ScriptActorBehaviorProfile profile{};
    if (source_posture <= 2U) {
        profile.engagement_policy = ScriptActorEngagementPolicy::patrol;
        profile.approach_band = ScriptActorApproachBand::far;
    } else if (source_posture == 3U) {
        profile.engagement_policy =
            ScriptActorEngagementPolicy::watch_nearby_runner;
        profile.approach_band = ScriptActorApproachBand::medium;
    } else if (source_posture == 4U) {
        profile.engagement_policy =
            ScriptActorEngagementPolicy::engage_within_range;
        profile.approach_band = ScriptActorApproachBand::medium;
    } else {
        profile.engagement_policy = ScriptActorEngagementPolicy::engage;
        if (source_posture <= 5U) {
            profile.approach_band = ScriptActorApproachBand::medium;
        } else if (source_posture <= 8U) {
            profile.approach_band = ScriptActorApproachBand::close;
        }
    }
    return profile;
}

ScriptCombatCyberware script_combat_cyberware_from_source_flags(
    const std::uint16_t source_flags) noexcept {
    // ROM 0x55718/0x55740 checks +0x90 bits 4/1; 0x55D2C..0x55D86 selects
    // the first bit in 5..8 as a melee-strength bonus; 0x55D66..0x55D86
    // selects the first bit in 9..11 as a melee-defense bonus; and
    // 0x55DA2..0x55DBC checks bits 3/2 for spurs/razors. Decode the packed
    // source word here so Lua receives only named capabilities and effects.
    // [C]
    ScriptCombatCyberware cyberware{};
    cyberware.cybereyes = (source_flags & 0x0002U) != 0U;
    cyberware.hand_razors = (source_flags & 0x0004U) != 0U;
    cyberware.spurs = (source_flags & 0x0008U) != 0U;
    cyberware.smartlink = (source_flags & 0x0010U) != 0U;
    for (std::uint8_t offset = 0U; offset < 4U; ++offset) {
        if ((source_flags & (std::uint16_t{1U} << (5U + offset))) != 0U) {
            cyberware.melee_strength_bonus =
                static_cast<std::uint8_t>(offset + 1U);
            break;
        }
    }
    for (std::uint8_t offset = 0U; offset < 3U; ++offset) {
        if ((source_flags & (std::uint16_t{1U} << (9U + offset))) != 0U) {
            cyberware.melee_defense_bonus =
                static_cast<std::uint8_t>(offset + 1U);
            break;
        }
    }
    return cyberware;
}

ScriptCombatTraits script_combat_traits_from_source_values(
    const std::int8_t race_or_state,
    const std::uint8_t class_code) noexcept {
    // These predicates retain the observed source branches while keeping the
    // selector values out of Lua: +0x9B == 2/4 bypasses the coverage penalty
    // at 0x55740; class 0x16 adds two there; +0x9B == 0x17 takes the special
    // spell path at 0x55B78; signed-negative +0x9B takes the one-point meter
    // path in 0x559BC; signed class < 2 qualifies for the party scan at
    // 0x1047E. [C values and branch outcomes]
    const auto signed_class = class_code < 0x80U
                                  ? static_cast<int>(class_code)
                                  : static_cast<int>(class_code) - 0x100;
    return {
        race_or_state == 0x02 || race_or_state == 0x04,
        class_code == 0x16U,
        race_or_state == 0x17,
        race_or_state < 0,
        signed_class <= 1,
        race_or_state == 0x42,
    };
}

bool script_sprite_type_blocks_spell_death_pose(
    const std::uint16_t source_sprite_type) noexcept {
    constexpr std::uint16_t kThonSpriteType = 0x000FU;
    return source_sprite_type == kThonSpriteType;
}

std::optional<std::string_view>
script_actor_visual_profile_id_from_source_type(
    const std::uint16_t source_sprite_type) noexcept {
    // The US actor template's first word is copied to live +0x0E by 0x44BC;
    // the animation service uses that selector to choose 0x62AC lists. Resolve
    // it once here to stable content IDs, so Lua never indexes by that word.
    switch (source_sprite_type) {
    case 1U: return "hit_spark";
    case 3U: return "field_runner";
    case 4U: return "ork_bandersnatch";
    case 5U: return "human_guard";
    case 6U: return "elven_mage";
    case 7U: return "ghoul_eye_fiver";
    case 8U: return "yakuza_weenr";
    case 9U: return "vampire_gator_shaman";
    case 10U: return "lone_star_mage";
    case 11U: return "pedestrian_rat_shaman";
    case 12U: return "hell_hound";
    case 13U: return "gargoyle";
    case 14U: return "wendigo";
    case 15U: return "thon";
    default: return std::nullopt;
    }
}

std::optional<std::uint16_t>
script_source_type_from_actor_visual_profile_id(
    const std::string_view visual_profile_id) noexcept {
    for (std::uint16_t source_type = 1U; source_type <= 15U;
         ++source_type) {
        const auto profile =
            script_actor_visual_profile_id_from_source_type(source_type);
        if (profile.has_value() && *profile == visual_profile_id) {
            return source_type;
        }
    }
    return std::nullopt;
}

ScriptCombatActionProfileOptions
script_combat_action_profile_options_from_source_flags(
    const std::uint8_t source_flags) noexcept {
    // The source decision reads +0xDF bit 0 for the primary profile, and bit 1
    // for the secondary profile. Decode those choices before Lua sees them.
    // The flag values are independently named in combat.lua's prior policy.
    return {(source_flags & 0x01U) != 0U,
            (source_flags & 0x02U) != 0U};
}

std::uint16_t script_posture_reduction_percent_from_source_value(
    const std::uint8_t source_posture) noexcept {
    // [C] ROM 0x55F20 applies 25 percent per posture step with integer
    // division. Send the resulting gameplay percentage to Lua, not +0x4D.
    return static_cast<std::uint16_t>(source_posture) * 25U;
}

std::optional<std::uint8_t>
script_source_posture_from_reduction_percent(
    const std::uint16_t posture_reduction_percent) noexcept {
    if (posture_reduction_percent % 25U != 0U) {
        return std::nullopt;
    }
    const auto source_posture = posture_reduction_percent / 25U;
    if (source_posture > 0xFFU) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(source_posture);
}

ScriptSceneTenActivityVariant
script_scene_ten_activity_variant_from_source_event(
    const std::uint8_t source_event) noexcept {
    // [C] ROM 0x1686C-0x16880 selects the alternate Scene-10 actor list only
    // when FFFBF9 equals 6; all other event values take the standard list.
    return source_event == 6U ? ScriptSceneTenActivityVariant::alternate
                              : ScriptSceneTenActivityVariant::standard;
}

std::vector<std::string>
script_mode_one_activity_ids_present_from_source_markers(
    const std::uint8_t primary_marker,
    const std::uint8_t secondary_marker) {
    // [C] ROM 0x168EE-0x16900 suppresses the scene's lone actor when its
    // 0xEEFE-derived party marker matches either compact party marker. Give
    // Lua the actor-list identities associated with those markers instead.
    constexpr std::array<std::pair<std::uint8_t, std::string_view>, 10U>
        kMarkerActivityIds{{
            {3U, "scene_10_event_6_script"},
            {5U, "scene_10_default_script"},
            {2U, "scene_11_script"},
            {7U, "scene_13_ilene_gate_script"},
            {4U, "scene_14_script"},
            {10U, "scene_15_script"},
            {6U, "scene_16_script"},
            {8U, "scene_17_script"},
            {9U, "scene_18_script"},
            {1U, "scene_19_script"},
        }};
    std::vector<std::string> result;
    result.reserve(2U);
    const auto append_activity_id =
        [&result, &kMarkerActivityIds](const std::uint8_t marker) {
            const auto found = std::find_if(
                kMarkerActivityIds.begin(), kMarkerActivityIds.end(),
                [marker](const auto& row) { return row.first == marker; });
            if (found != kMarkerActivityIds.end() &&
                std::find(result.begin(), result.end(), found->second) ==
                    result.end()) {
                result.emplace_back(found->second);
            }
        };
    append_activity_id(primary_marker);
    append_activity_id(secondary_marker);
    return result;
}

std::optional<std::string> GameScriptRuntime::active_run_id() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure =
            "Lua runtime is unavailable for the active run identity";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::string> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "active_run_id");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry active run identity is missing");
    }
    lua_remove(state, -2);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return fail("run_registry active run identity failed: " +
                    stack_error(state));
    }
    std::string run_id;
    if (!read_string(state, -1, run_id)) {
        return fail("active run identity is not a nonempty string");
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return run_id == "none"
        ? std::optional<std::string>{}
        : std::optional<std::string>{std::move(run_id)};
}

std::optional<std::string>
GameScriptRuntime::active_run_offer_variant_id() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure =
            "Lua runtime is unavailable for the active run variant";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::string> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "active_offer_variant_id");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry active offer variant is missing");
    }
    lua_remove(state, -2);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return fail("run_registry active offer variant failed: " +
                    stack_error(state));
    }
    if (lua_isnil(state, -1)) {
        lua_settop(state, stack_base);
        impl_->failure.clear();
        return std::nullopt;
    }
    std::string variant_id;
    if (!read_string(state, -1, variant_id) ||
        !valid_identifier(variant_id)) {
        return fail("active source offer variant is invalid");
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return variant_id;
}

std::optional<ScriptRunFieldEventDecision>
GameScriptRuntime::resolve_active_run_field_event(
    const std::string_view location_id) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for run field events";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptRunFieldEventDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "resolve_field_event");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry field-event resolver is missing");
    }
    lua_remove(state, -2);
    lua_pushlstring(state, location_id.data(), location_id.size());
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("run_registry field-event resolver failed: " +
                    stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("run_registry field-event resolver returned no decision");
    }

    const auto result_index = lua_absindex(state, -1);
    std::string action_name;
    std::string run_id;
    std::string dialogue_module;
    std::string dialogue_function;
    std::uint32_t reward_nuyen = 0U;
    std::uint32_t progress_delta = 0U;
    std::string actor_group_id;
    if (!read_field_string(state, result_index, "action", action_name) ||
        !read_field_string(state, result_index, "run_id", run_id) ||
        !valid_identifier(run_id) ||
        !read_field_integer(state, result_index, "reward_nuyen",
                            0xFFFFFFFFU, reward_nuyen) ||
        !read_field_integer(state, result_index, "runner_progress_delta",
                            0xFFU, progress_delta)) {
        return fail("run field-event decision has invalid fields");
    }

    ScriptRunFieldEventDecision decision{};
    if (action_name == "no_action") {
        decision.action = ScriptRunFieldEventAction::no_action;
    } else if (action_name == "objective_advanced") {
        decision.action = ScriptRunFieldEventAction::objective_advanced;
    } else if (action_name == "completed") {
        decision.action = ScriptRunFieldEventAction::completed;
    } else if (action_name == "entry_dialogue") {
        decision.action = ScriptRunFieldEventAction::entry_dialogue;
        if (!read_field_string(state, result_index, "dialogue_module",
                               dialogue_module) ||
            !valid_identifier(dialogue_module) ||
            !read_field_string(state, result_index, "dialogue_function",
                               dialogue_function) ||
            !valid_identifier(dialogue_function)) {
            return fail("run field-event dialogue task is invalid");
        }
        if (!read_optional_field_string(state, result_index, "actor_group_id",
                                        actor_group_id) ||
            (!actor_group_id.empty() &&
             !is_run_target_actor_group_id(actor_group_id))) {
            return fail("run field-event actor group identity is invalid");
        }
    } else {
        return fail("run field-event decision has an unknown action");
    }
    if (decision.action == ScriptRunFieldEventAction::completed &&
        run_id == "none") {
        return fail("completed run field-event decision has no run id");
    }
    decision.run_id = std::move(run_id);
    decision.reward_nuyen = reward_nuyen;
    decision.runner_progress_delta =
        static_cast<std::uint8_t>(progress_delta);
    decision.dialogue_module = std::move(dialogue_module);
    decision.dialogue_function = std::move(dialogue_function);
    decision.actor_group_id = std::move(actor_group_id);

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return decision;
}

std::optional<ScriptRunActorDefeatDecision>
GameScriptRuntime::resolve_active_run_actor_defeat(
    const ScriptActorDefeatFacts& facts) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for run actor defeats";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptRunActorDefeatDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "resolve_actor_defeat");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry actor-defeat resolver is missing");
    }
    lua_remove(state, -2);
    lua_createtable(state, 0, 1);
    lua_pushboolean(state, facts.is_ghoul_bounty_target);
    lua_setfield(state, -2, "is_ghoul_bounty_target");
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("run_registry actor-defeat resolver failed: " +
                    stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("run_registry actor-defeat resolver returned no decision");
    }

    const auto result_index = lua_absindex(state, -1);
    std::string action_name;
    std::string run_id;
    std::uint32_t objective_progress = 0U;
    if (!read_field_string(state, result_index, "action", action_name) ||
        !read_field_string(state, result_index, "run_id", run_id) ||
        !valid_identifier(run_id) ||
        !read_field_integer(state, result_index, "objective_progress",
                            0xFFU, objective_progress)) {
        return fail("run actor-defeat decision has invalid fields");
    }

    ScriptRunActorDefeatDecision decision{};
    if (action_name == "no_action") {
        decision.action = ScriptRunActorDefeatAction::no_action;
    } else if (action_name == "objective_advanced") {
        decision.action = ScriptRunActorDefeatAction::objective_advanced;
    } else {
        return fail("run actor-defeat decision has an unknown action");
    }
    if (decision.action == ScriptRunActorDefeatAction::objective_advanced &&
        run_id == "none") {
        return fail("advanced actor-defeat decision has no run id");
    }
    decision.run_id = std::move(run_id);
    decision.objective_progress =
        static_cast<std::uint8_t>(objective_progress);

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return decision;
}

std::optional<ScriptRunJohnsonReturnDecision>
GameScriptRuntime::resolve_active_run_johnson_return(
    const std::string_view contact_id) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_run_contact_id(contact_id)) {
        impl_->failure = "Lua runtime is unavailable for Johnson run returns";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptRunJohnsonReturnDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "resolve_johnson_return");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry Johnson-return resolver is missing");
    }
    lua_remove(state, -2);
    lua_pushlstring(state, contact_id.data(), contact_id.size());
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("run_registry Johnson-return resolver failed: " +
                    stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("run_registry Johnson-return resolver returned no decision");
    }

    const auto result_index = lua_absindex(state, -1);
    std::string action_name;
    std::string run_id;
    std::uint32_t reward_nuyen = 0U;
    std::uint32_t progress_delta = 0U;
    std::uint32_t objective_progress = 0U;
    if (!read_field_string(state, result_index, "action", action_name) ||
        !read_field_string(state, result_index, "run_id", run_id) ||
        !valid_identifier(run_id) ||
        !read_field_integer(state, result_index, "reward_nuyen",
                            0xFFFFFFFFU, reward_nuyen) ||
        !read_field_integer(state, result_index, "runner_progress_delta",
                            0xFFU, progress_delta) ||
        !read_field_integer(state, result_index, "objective_progress",
                            0xFFU, objective_progress)) {
        return fail("Johnson-return decision has invalid fields");
    }

    ScriptRunJohnsonReturnDecision decision{};
    if (action_name == "no_action") {
        decision.action = ScriptRunJohnsonReturnAction::no_action;
    } else if (action_name == "completed") {
        decision.action = ScriptRunJohnsonReturnAction::completed;
    } else {
        return fail("Johnson-return decision has an unknown action");
    }
    if (decision.action == ScriptRunJohnsonReturnAction::completed &&
        run_id == "none") {
        return fail("completed Johnson-return decision has no run id");
    }
    decision.run_id = std::move(run_id);
    decision.reward_nuyen = reward_nuyen;
    decision.runner_progress_delta =
        static_cast<std::uint8_t>(progress_delta);
    decision.objective_progress =
        static_cast<std::uint8_t>(objective_progress);

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return decision;
}

std::optional<std::string>
GameScriptRuntime::johnson_interaction_contact_id(
    const std::string_view text_profile) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(text_profile)) {
        impl_->failure = "Johnson interaction profile is unavailable";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::string> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("dialogue")) {
        return fail("dialogue.lua module is missing");
    }
    lua_getfield(state, -1, "johnson_interaction_profiles");
    if (!lua_istable(state, -1)) {
        return fail("dialogue Johnson interaction profiles are missing");
    }
    lua_getfield(state, -1, std::string(text_profile).c_str());
    if (!lua_istable(state, -1)) {
        return fail("location has no Johnson interaction profile");
    }
    lua_getfield(state, -1, "run_contact_id");
    std::string contact_id;
    if (!read_string(state, -1, contact_id) ||
        !valid_run_contact_id(contact_id)) {
        return fail("Johnson interaction profile has no valid run contact");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return contact_id;
}

std::optional<ScriptPersistentValue>
GameScriptRuntime::save_run_registry_state() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for run-state saves";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string& reason)
        -> std::optional<ScriptPersistentValue> {
        lua_settop(state, stack_base);
        impl_->failure = reason;
        return std::nullopt;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "save_state");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry save_state is missing");
    }
    lua_remove(state, -2);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return fail("run_registry save_state failed: " + stack_error(state));
    }
    ScriptPersistentValue result;
    PersistentReadContext context;
    if (!read_persistent_value(state, -1, result, context, 0U)) {
        return fail(context.error);
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

bool GameScriptRuntime::validate_run_registry_state(
    const ScriptPersistentValue& saved_state) {
    ScriptPersistentValue normalized_state;
    std::string normalization_failure;
    if (!normalize_saved_run_offer_variants(
            saved_state, normalized_state, normalization_failure)) {
        impl_->failure = std::move(normalization_failure);
        return false;
    }
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for run-state restores";
        return false;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string& reason) {
        lua_settop(state, stack_base);
        impl_->failure = reason;
        return false;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "validate_saved_state");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry validate_saved_state is missing");
    }
    lua_remove(state, -2);
    std::size_t nodes = 0U;
    if (!push_persistent_value(state, normalized_state, nodes, 0U)) {
        return fail("saved run state is not JSON-compatible");
    }
    if (lua_pcall(state, 1, 2, 0) != LUA_OK) {
        return fail("run_registry state validation failed: " +
                    stack_error(state));
    }
    const bool valid = lua_isboolean(state, -2) &&
                       lua_toboolean(state, -2) != 0;
    std::string reason = "saved run state failed Lua validation";
    if (!valid && lua_type(state, -1) == LUA_TSTRING) {
        std::size_t size = 0U;
        const auto* message = lua_tolstring(state, -1, &size);
        if (message != nullptr) reason.assign(message, size);
    }
    lua_settop(state, stack_base);
    if (!valid) {
        impl_->failure = std::move(reason);
        return false;
    }
    impl_->failure.clear();
    return true;
}

bool GameScriptRuntime::restore_run_registry_state(
    const ScriptPersistentValue& saved_state) {
    ScriptPersistentValue normalized_state;
    std::string normalization_failure;
    if (!normalize_saved_run_offer_variants(
            saved_state, normalized_state, normalization_failure)) {
        impl_->failure = std::move(normalization_failure);
        return false;
    }
    if (!validate_run_registry_state(normalized_state)) return false;
    auto* state = impl_->state;
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string& reason) {
        lua_settop(state, stack_base);
        impl_->failure = reason;
        return false;
    };
    if (!impl_->push_module("run_registry")) {
        return fail("run_registry.lua module is missing");
    }
    lua_getfield(state, -1, "restore_saved_state");
    if (!lua_isfunction(state, -1)) {
        return fail("run_registry restore_saved_state is missing");
    }
    lua_remove(state, -2);
    std::size_t nodes = 0U;
    if (!push_persistent_value(state, normalized_state, nodes, 0U)) {
        return fail("saved run state is not JSON-compatible");
    }
    if (lua_pcall(state, 1, 0, 0) != LUA_OK) {
        return fail("run_registry restore failed: " + stack_error(state));
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return true;
}

std::optional<ScriptLocationEntryDecision>
GameScriptRuntime::resolve_field_location_entry(
    const std::uint16_t scene_id,
    const FieldCollisionEventCode event_code,
    const std::vector<std::vector<std::uint16_t>>& source_elevator_groups) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for location entry";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptLocationEntryDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("building")) {
        return fail("building.lua module is missing");
    }
    lua_getfield(state, -1, "resolve_field_location_entry");
    if (!lua_isfunction(state, -1)) {
        return fail("building.lua location-entry function is missing");
    }

    lua_createtable(state, 0, 2);
    const auto input_index = lua_absindex(state, -1);
    lua_pushinteger(state, scene_id);
    lua_setfield(state, input_index, "scene_id");
    const auto event_name = collision_event_name(event_code);
    lua_pushlstring(state, event_name.c_str(), event_name.size());
    lua_setfield(state, input_index, "event_code");
    lua_createtable(state,
                    static_cast<int>(source_elevator_groups.size()), 0);
    const auto groups_index = lua_absindex(state, -1);
    for (std::size_t group_index = 0U;
         group_index < source_elevator_groups.size(); ++group_index) {
        const auto& source_group = source_elevator_groups[group_index];
        lua_createtable(state, static_cast<int>(source_group.size()), 0);
        const auto group_table_index = lua_absindex(state, -1);
        for (std::size_t floor = 0U; floor < source_group.size(); ++floor) {
            lua_pushinteger(state, source_group[floor]);
            lua_rawseti(state, group_table_index,
                        static_cast<lua_Integer>(floor + 1U));
        }
        lua_rawseti(state, groups_index,
                    static_cast<lua_Integer>(group_index + 1U));
    }
    lua_setfield(state, input_index, "source_elevator_groups");

    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("building.lua location-entry resolution failed");
    }
    if (!lua_istable(state, -1)) {
        return fail("building.lua location-entry returned no decision");
    }

    const auto result_index = lua_absindex(state, -1);
    std::string action_name;
    if (!read_field_string(state, result_index, "action", action_name)) {
        return fail("building.lua location-entry action is missing");
    }

    ScriptLocationEntryDecision decision{};
    if (action_name == "no_action") {
        lua_settop(state, stack_base);
        impl_->failure.clear();
        return decision;
    }
    if (action_name != "open_elevator_selector") {
        return fail("building.lua location-entry action is unknown");
    }

    lua_getfield(state, result_index, "floor_scenes");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return fail("building.lua elevator entry has no floor list");
    }
    const auto floor_table_index = lua_absindex(state, -1);
    const auto floor_count = lua_rawlen(state, floor_table_index);
    if (floor_count < 2U || floor_count > 6U) {
        lua_pop(state, 1);
        return fail("building.lua elevator entry has an invalid floor count");
    }
    decision.elevator_group.scene_ids.reserve(floor_count);
    for (std::size_t floor = 0U; floor < floor_count; ++floor) {
        lua_rawgeti(state, floor_table_index,
                    static_cast<lua_Integer>(floor + 1U));
        std::uint32_t scene_reference = 0U;
        const bool valid = read_integer(state, -1, 255U, scene_reference);
        lua_pop(state, 1);
        if (!valid) {
            lua_pop(state, 1);
            return fail("building.lua elevator entry has an invalid scene");
        }
        decision.elevator_group.scene_ids.push_back(
            static_cast<std::uint16_t>(scene_reference));
    }
    lua_pop(state, 1);

    std::uint32_t entry_floor = 0U;
    if (!read_field_integer(state, result_index, "entry_floor",
                            static_cast<std::uint32_t>(floor_count - 1U),
                            entry_floor)) {
        return fail("building.lua elevator entry has an invalid floor");
    }
    decision.action = ScriptLocationEntryAction::open_elevator_selector;
    decision.elevator_group.entry_floor =
        static_cast<std::uint8_t>(entry_floor);
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return decision;
}

std::optional<std::uint16_t> GameScriptRuntime::resolve_elevator_destination(
    const ScriptElevatorFloorGroup& group,
    const std::uint8_t floor_index) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for elevator routing";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::uint16_t> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (group.scene_ids.size() < 2U || group.scene_ids.size() > 6U ||
        floor_index >= group.scene_ids.size()) {
        return fail("elevator destination request has an invalid floor");
    }
    if (!impl_->push_module("building")) {
        return fail("building.lua module is missing");
    }
    lua_getfield(state, -1, "resolve_elevator_destination");
    if (!lua_isfunction(state, -1)) {
        return fail("building.lua elevator-destination function is missing");
    }

    lua_createtable(state, 0, 5);
    const auto input_index = lua_absindex(state, -1);
    lua_createtable(state, static_cast<int>(group.scene_ids.size()), 0);
    const auto floors_index = lua_absindex(state, -1);
    for (std::size_t floor = 0U; floor < group.scene_ids.size(); ++floor) {
        lua_pushinteger(state, group.scene_ids[floor]);
        lua_rawseti(state, floors_index,
                    static_cast<lua_Integer>(floor + 1U));
    }
    lua_setfield(state, input_index, "floor_scenes");
    lua_pushinteger(state, floor_index);
    lua_setfield(state, input_index, "floor_index");

    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("building.lua elevator-destination resolution failed");
    }
    std::uint32_t destination = 0U;
    if (!read_integer(state, -1, 255U, destination)) {
        return fail("building.lua returned an invalid elevator destination");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return static_cast<std::uint16_t>(destination);
}

std::optional<FieldMapEventPolicyDecision>
GameScriptRuntime::resolve_field_collision_event(
    const FieldMapCollisionEventInput& input) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for field collision policy";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<FieldMapEventPolicyDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("field_events")) {
        return fail("field_events.lua module is missing");
    }
    lua_getfield(state, -1, "resolve_collision");
    if (!lua_isfunction(state, -1)) {
        return fail("field_events.resolve_collision is missing");
    }

    lua_createtable(state, 0, 11);
    const auto input_index = lua_absindex(state, -1);
    const auto source_event_code =
        field_collision_event_code(input.raw_collision_code);
    const bool lockable_door_context =
        input.scene_id == field_scene_id_value(FieldSceneId::scene_20) ||
        input.world_mode == static_cast<std::uint16_t>(
                                WorldActivityMode::resource_activity);
    const auto collision_kind =
        field_collision_kind_name(source_event_code, lockable_door_context);
    lua_pushlstring(state, collision_kind.data(), collision_kind.size());
    lua_setfield(state, input_index, "collision_kind");
    lua_pushboolean(
        state, input.world_mode == static_cast<std::uint16_t>(
                                       WorldActivityMode::resource_activity));
    lua_setfield(state, input_index, "resource_site");
    lua_pushboolean(
        state, input.scene_id == field_scene_id_value(FieldSceneId::scene_20));
    lua_setfield(state, input_index, "correctional_facility");
    lua_pushboolean(
        state, input.field_transition_context ==
                   static_cast<std::uint16_t>(
                       FieldTransitionContext::event_25_special));
    lua_setfield(state, input_index, "special_activity_context");
    lua_pushboolean(state,
                    field_mode_exit_requested(input.field_global_flags));
    lua_setfield(state, input_index, "field_mode_exit_requested");
    lua_pushboolean(state, input.interaction_gate != 0U);
    lua_setfield(state, input_index, "interaction_gate_active");
    lua_pushboolean(state, input.activity_sequence_active);
    lua_setfield(state, input_index, "activity_sequence_active");
    lua_pushboolean(state, input.terrain_event_suppressed);
    lua_setfield(state, input_index, "terrain_event_suppressed");
    const bool terrain_marker_publish_allowed =
        (input.raw_collision_code & 0x00FFU) !=
            kTerrainMarkerSuppressionByte &&
        !input.world_effect_control_bit;
    lua_pushboolean(state, terrain_marker_publish_allowed);
    lua_setfield(state, input_index, "terrain_marker_publish_allowed");
    if (input.maglocks_disabled.has_value()) {
        lua_pushboolean(state, *input.maglocks_disabled);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, input_index, "maglocks_disabled");

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("field_events.resolve_collision failed");
    }
    if (!lua_istable(state, -1)) {
        return fail("field_events.resolve_collision returned no decision");
    }
    const auto result_index = lua_absindex(state, -1);
    std::string action_name;
    std::string collision_rewrite;
    FieldMapEventAction action{};
    bool publishes_world_interaction = false;
    bool projected_terrain_event_published = false;
    const bool valid =
        read_field_string(state, result_index, "action", action_name) &&
        field_map_event_action(action_name, source_event_code, action) &&
        read_field_string(state, result_index, "collision_rewrite",
                          collision_rewrite) &&
        read_field_boolean(state, result_index,
                           "publishes_world_interaction",
                           publishes_world_interaction) &&
        read_field_boolean(state, result_index,
                           "projected_terrain_event_published",
                           projected_terrain_event_published);
    if (!valid) {
        return fail("field_events.resolve_collision returned invalid fields");
    }

    const bool rewrites_contextual_activity =
        collision_rewrite == "ordinary_interaction";
    if ((!rewrites_contextual_activity && collision_rewrite != "none") ||
        (rewrites_contextual_activity &&
         (source_event_code != FieldCollisionEventCode::event_25 ||
          (action != FieldMapEventAction::project_world_interaction &&
           action != FieldMapEventAction::begin_field_mode_exit))) ||
        (action == FieldMapEventAction::begin_event_25_activity &&
         source_event_code != FieldCollisionEventCode::event_25)) {
        return fail("field_events.resolve_collision returned an invalid collision rewrite");
    }

    FieldMapEventPolicyDecision decision{};
    decision.action = action;
    decision.source_event_code = source_event_code;
    decision.event_code = rewrites_contextual_activity
                              ? FieldCollisionEventCode::event_8
                              : source_event_code;
    decision.raw_collision_code = input.raw_collision_code;
    decision.mode_event_code_published = publishes_world_interaction;
    decision.projected_terrain_event_published =
        projected_terrain_event_published;
    if (rewrites_contextual_activity) {
        decision.raw_collision_code = kEvent25RewrittenCollisionCode;
        decision.rewrite_direction_probe = true;
        decision.direction_probe_value = kEvent25DirectionProbe;
        decision.rewrite_quantized_delta = true;
        decision.quantized_delta = kEvent25RewrittenCollisionCode;
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return decision;
}

std::optional<ScriptFieldSafeDecision>
GameScriptRuntime::resolve_field_safe_action(
    const std::uint16_t scene_id,
    const bool projected_terrain_event_published,
    const bool fuchi_cyberheart_safe_armed) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for field safe policy";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptFieldSafeDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("field_safes")) {
        return fail("field_safes.lua module is missing");
    }
    lua_getfield(state, -1, "resolve");
    if (!lua_isfunction(state, -1)) {
        return fail("field_safes.resolve is missing");
    }

    lua_createtable(state, 0, 3);
    const auto input_index = lua_absindex(state, -1);
    lua_pushinteger(state, scene_id);
    lua_setfield(state, input_index, "scene_id");
    lua_pushboolean(state, projected_terrain_event_published);
    lua_setfield(state, input_index, "projected_terrain_event_published");
    lua_pushboolean(state, fuchi_cyberheart_safe_armed);
    lua_setfield(state, input_index, "fuchi_cyberheart_safe_armed");

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("field_safes.resolve failed");
    }
    if (!lua_istable(state, -1)) {
        return fail("field_safes.resolve returned no decision");
    }
    const auto result_index = lua_absindex(state, -1);
    std::string action_name;
    bool raise_alarm = false;
    bool reset_strike_team_stage = false;
    bool defer_alarm_transition = false;
    if (!read_field_string(state, result_index, "action", action_name) ||
        !read_field_boolean(state, result_index, "raise_alarm", raise_alarm) ||
        !read_field_boolean(state, result_index,
                            "reset_strike_team_stage",
                            reset_strike_team_stage) ||
        !read_field_boolean(state, result_index, "defer_alarm_transition",
                            defer_alarm_transition)) {
        return fail("field_safes.resolve returned invalid fields");
    }

    ScriptFieldSafeDecision decision{};
    if (action_name == "none") {
        decision.action = ScriptFieldSafeAction::none;
    } else if (action_name == "renraku_map_overlay") {
        decision.action = ScriptFieldSafeAction::renraku_map_overlay;
    } else if (action_name == "fuchi_prototype_cyberheart") {
        decision.action =
            ScriptFieldSafeAction::fuchi_prototype_cyberheart;
    } else {
        return fail("field_safes.resolve returned an unknown action");
    }
    decision.raise_alarm = raise_alarm;
    decision.reset_strike_team_stage = reset_strike_team_stage;
    decision.defer_alarm_transition = defer_alarm_transition;

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return decision;
}

std::optional<ScriptFieldEncounterOutcome>
GameScriptRuntime::resolve_field_encounter(
    const std::uint16_t scene_id, const double weighted_selection) {
    return resolve_field_encounter_impl(scene_id, weighted_selection);
}

std::optional<ScriptFieldEncounterOutcome>
GameScriptRuntime::resolve_field_actor_sequence(
    const std::uint16_t scene_id, const std::string_view sequence_id) {
    if (scene_id > 55U || !valid_identifier(sequence_id)) {
        impl_->failure = "field actor sequence id is invalid";
        return std::nullopt;
    }
    return resolve_field_encounter_impl(scene_id, 0.5, sequence_id);
}

std::optional<std::vector<ScriptWorldActivityActor>>
GameScriptRuntime::world_activity_actors_for_script(
    const std::uint16_t scene_id, const std::uint16_t script_index) {
    if (scene_id > 55U || script_index > 0xFFU) {
        impl_->failure = "world-activity script selector is outside source bounds";
        return std::nullopt;
    }
    const auto binding = std::find_if(
        impl_->activity_selector_bindings.begin(),
        impl_->activity_selector_bindings.end(),
        [scene_id, script_index](const ScriptActivitySelectorBinding& row) {
            return row.scene_id == scene_id && row.selector == script_index;
        });
    if (binding == impl_->activity_selector_bindings.end()) {
        impl_->failure.clear();
        return std::vector<ScriptWorldActivityActor>{};
    }
    return world_activity_actors_for_activity_id_impl(
        scene_id, binding->activity_id);
}

std::optional<std::vector<ScriptWorldActivityActor>>
GameScriptRuntime::world_activity_actors_for_activity_id(
    const std::uint16_t scene_id, const std::string_view activity_id) {
    if (activity_id.empty() || activity_id.size() > 128U) {
        impl_->failure = "world-activity id is empty or too long";
        return std::nullopt;
    }
    return world_activity_actors_for_activity_id_impl(scene_id, activity_id);
}

std::optional<std::vector<ScriptWorldActivityActor>>
GameScriptRuntime::world_activity_actors_for_activity_id(
    const std::string_view activity_id) {
    if (activity_id.empty() || activity_id.size() > 128U ||
        !valid_identifier(activity_id)) {
        impl_->failure = "world-activity id is invalid";
        return std::nullopt;
    }
    return world_activity_actors_for_activity_id_impl(std::nullopt,
                                                        activity_id);
}

std::optional<std::string>
GameScriptRuntime::activity_id_for_source_group(
    const std::uint16_t scene_id,
    const std::uint32_t source_group_offset) const {
    const auto found = std::find_if(
        impl_->activity_group_bindings.begin(),
        impl_->activity_group_bindings.end(),
        [scene_id, source_group_offset](
            const ScriptWorldActivityGroupBinding& binding) {
            return binding.scene_id == scene_id &&
                   binding.source_group_offset == source_group_offset;
        });
    if (found == impl_->activity_group_bindings.end()) {
        return std::nullopt;
    }
    return found->activity_id;
}

std::optional<ScriptFieldActorContactAction>
GameScriptRuntime::field_actor_contact_task(
    const std::uint16_t scene_id,
    const std::string_view actor_instance_id,
    const std::string_view activity_id,
    const ScriptFieldActorContactHandler handler) {
    auto* state = impl_->state;
    const bool activity_id_valid = activity_id.size() <= 128U &&
        std::all_of(activity_id.begin(), activity_id.end(),
                    [](const char character) {
                        return (character >= 'a' && character <= 'z') ||
                               (character >= 'A' && character <= 'Z') ||
                               (character >= '0' && character <= '9') ||
                               character == '_' || character == '-' ||
                               character == '.' || character == ':';
                    });
    if (state == nullptr || scene_id > 55U ||
        !valid_identifier(actor_instance_id) || !activity_id_valid) {
        impl_->failure = "field actor contact facts are invalid";
        return std::nullopt;
    }

    std::string_view handler_id;
    switch (handler) {
    case ScriptFieldActorContactHandler::ordinary:
        handler_id = "ordinary";
        break;
    case ScriptFieldActorContactHandler::world_effect:
        handler_id = "world_effect";
        break;
    case ScriptFieldActorContactHandler::pending_world_effect:
        handler_id = "pending_world_effect";
        break;
    default:
        impl_->failure = "field actor contact handler is unknown";
        return std::nullopt;
    }

    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptFieldActorContactAction> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("world_content")) {
        return fail("gameplay script module is missing: world_content");
    }
    lua_getfield(state, -1, "field_actor_contact_task");
    if (!lua_isfunction(state, -1)) {
        return fail("world_content.field_actor_contact_task is missing");
    }
    lua_remove(state, -2);
    lua_pushinteger(state, static_cast<lua_Integer>(scene_id));
    lua_pushlstring(state, actor_instance_id.data(), actor_instance_id.size());
    lua_pushlstring(state, activity_id.data(), activity_id.size());
    lua_pushlstring(state, handler_id.data(), handler_id.size());
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 4, 1, 0) != LUA_OK) {
        return fail("world_content.field_actor_contact_task: " +
                    stack_error(state));
    }

    std::string action_id;
    if (!read_string(state, -1, action_id)) {
        return fail("field actor contact task must return a semantic action");
    }
    ScriptFieldActorContactAction action{};
    if (action_id == "start_activity") {
        action = ScriptFieldActorContactAction::start_activity;
    } else if (action_id == "aragorn_contact") {
        action = ScriptFieldActorContactAction::aragorn_contact;
    } else if (action_id == "ilene_activity") {
        action = ScriptFieldActorContactAction::ilene_activity;
    } else if (action_id == "random_pedestrian_contact") {
        action = ScriptFieldActorContactAction::random_pedestrian_contact;
    } else if (action_id == "generic_contact_dialogue") {
        action = ScriptFieldActorContactAction::generic_contact_dialogue;
    } else {
        return fail("field actor contact task returned an unknown action");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return action;
}

std::optional<std::string>
GameScriptRuntime::generic_field_contact_dialogue(
    const std::uint16_t scene_id, const std::string_view activity_id,
    const std::uint8_t row_variant,
    const std::optional<std::int16_t> signed_selector) {
    auto* state = impl_->state;
    if (state == nullptr || scene_id > 55U || activity_id.empty() ||
        activity_id.size() > 128U || !valid_identifier(activity_id)) {
        impl_->failure = "generic field contact facts are invalid";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::string> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("world_content")) {
        return fail("gameplay script module is missing: world_content");
    }
    lua_getfield(state, -1, "generic_field_contact_dialogue");
    if (!lua_isfunction(state, -1)) {
        return fail(
            "world_content.generic_field_contact_dialogue is missing");
    }
    lua_remove(state, -2);
    lua_pushinteger(state, static_cast<lua_Integer>(scene_id));
    lua_pushlstring(state, activity_id.data(), activity_id.size());
    lua_pushinteger(state, static_cast<lua_Integer>(row_variant));
    if (signed_selector.has_value()) {
        lua_pushinteger(state,
                        static_cast<lua_Integer>(*signed_selector));
    } else {
        lua_pushnil(state);
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 4, 1, 0) != LUA_OK) {
        return fail("world_content.generic_field_contact_dialogue: " +
                    stack_error(state));
    }
    if (lua_isnil(state, -1)) {
        lua_settop(state, stack_base);
        impl_->failure.clear();
        return std::nullopt;
    }
    std::string response_id;
    if (!read_string(state, -1, response_id) ||
        !valid_identifier(response_id)) {
        return fail(
            "generic field contact dialogue must return a stable response id");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return response_id;
}

std::optional<std::vector<ScriptConditionalActorCandidate>>
GameScriptRuntime::conditional_actor_candidates_for_scene(
    const std::uint16_t scene_id) {
    if (impl_->state == nullptr || scene_id > 55U) {
        impl_->failure =
            "conditional actor-candidate scene is invalid";
        return std::nullopt;
    }
    std::vector<ScriptConditionalActorCandidate> candidates;
    for (const auto& candidate : impl_->conditional_actor_catalog) {
        if (candidate.scene_id == scene_id) {
            candidates.push_back(candidate);
        }
    }
    if (candidates.size() > 3U) {
        impl_->failure =
            "conditional actor catalog exceeds the selector's source table";
        return std::nullopt;
    }
    impl_->failure.clear();
    return candidates;
}

bool GameScriptRuntime::reset_conditional_actor_selection() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for A800 state reset";
        return false;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason) {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return false;
    };
    if (!impl_->push_module("world_content")) {
        return fail("gameplay script module is missing: world_content");
    }
    lua_getfield(state, -1, "reset_conditional_actor_selection");
    if (!lua_isfunction(state, -1)) {
        return fail(
            "world_content.reset_conditional_actor_selection is missing");
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 0, 0) != LUA_OK) {
        return fail("world_content.reset_conditional_actor_selection: " +
                    stack_error(state));
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return true;
}

std::optional<ScriptConditionalActorVariant>
GameScriptRuntime::consume_conditional_actor_selection() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure =
            "Lua runtime is unavailable for conditional actor selection consumption";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptConditionalActorVariant> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("world_content")) {
        return fail("gameplay script module is missing: world_content");
    }
    lua_getfield(state, -1, "consume_conditional_actor_selection");
    if (!lua_isfunction(state, -1)) {
        return fail(
            "world_content.consume_conditional_actor_selection is missing");
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return fail(
            "world_content.consume_conditional_actor_selection: " +
            stack_error(state));
    }
    std::string variant;
    if (!read_string_allow_empty(state, -1, variant) ||
        (variant != "none" && variant != "first" && variant != "second")) {
        return fail(
            "conditional actor selection returned an unknown variant");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    if (variant == "first") {
        return ScriptConditionalActorVariant::first;
    }
    if (variant == "second") {
        return ScriptConditionalActorVariant::second;
    }
    return ScriptConditionalActorVariant::none;
}

std::optional<ScriptConditionalActorSelection>
GameScriptRuntime::resolve_conditional_actor_selection(
    const std::uint16_t scene_id,
    const ScriptConditionalActorSelectionContext& context,
    const ScriptActorRandomBound& random_bound) {
    auto* state = impl_->state;
    if (state == nullptr || scene_id > 55U) {
        impl_->failure =
            "conditional actor selection input is outside source bounds";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptConditionalActorSelection> {
        lua_settop(state, stack_base);
        impl_->active_conditional_actor_random_bound = {};
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("world_content")) {
        return fail("gameplay script module is missing: world_content");
    }
    lua_getfield(state, -1, "resolve_conditional_actor_selection");
    if (!lua_isfunction(state, -1)) {
        return fail(
            "world_content.resolve_conditional_actor_selection is missing");
    }
    lua_remove(state, -2);
    lua_pushinteger(state, static_cast<lua_Integer>(scene_id));
    lua_createtable(state, 0, 10);
    lua_pushstring(state, scene_activity_state_id(context.activity_state));
    lua_setfield(state, -2, "activity_state");
    if (context.active_offer_id.has_value()) {
        lua_pushlstring(state, context.active_offer_id->data(),
                        context.active_offer_id->size());
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "active_offer_id");
    lua_pushstring(state, field_scene_mode_id(context.scene_mode));
    lua_setfield(state, -2, "scene_mode");
    const char* scene_ten_variant =
        context.scene_ten_activity_variant ==
                ScriptSceneTenActivityVariant::alternate
            ? "alternate"
            : "standard";
    lua_pushstring(state, scene_ten_variant);
    lua_setfield(state, -2, "scene_ten_activity_variant");
    lua_createtable(
        state, 0, static_cast<int>(context.completed_story_facts.size()));
    for (const auto& fact : context.completed_story_facts) {
        lua_pushboolean(state, 1);
        lua_setfield(state, -2, fact.c_str());
    }
    lua_setfield(state, -2, "completed_story_facts");
    lua_createtable(state, 0,
                    static_cast<int>(context.party_present_activity_ids.size()));
    for (const auto& activity_id : context.party_present_activity_ids) {
        lua_pushboolean(state, 1);
        lua_setfield(state, -2, activity_id.c_str());
    }
    lua_setfield(state, -2, "party_present_activity_ids");
    lua_pushboolean(state, context.recruitment_encounter_ready);
    lua_setfield(state, -2, "recruitment_encounter_ready");

    impl_->active_conditional_actor_random_bound = random_bound;
    lua_pushlightuserdata(state, impl_.get());
    lua_pushcclosure(state, Impl::conditional_actor_random_bound, 1);
    ScopedInstructionBudget instruction_budget(state);
    const auto call_status = lua_pcall(state, 3, 1, 0);
    impl_->active_conditional_actor_random_bound = {};
    if (call_status != LUA_OK) {
        return fail("world_content.resolve_conditional_actor_selection: " +
                    stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("conditional actor selector must return a decision table");
    }
    const auto decision_index = lua_absindex(state, -1);
    std::string kind;
    std::string selection_variant;
    std::string selection_id;
    if (!read_field_text(state, decision_index, "kind", kind) ||
        !read_field_text(state, decision_index, "selection_variant",
                         selection_variant, 32U) ||
        !read_field_text(state, decision_index, "selection_id",
                         selection_id, 128U)) {
        return fail("conditional actor selector returned invalid decision fields");
    }
    if (selection_variant != "none" && selection_variant != "first" &&
        selection_variant != "second") {
        return fail("conditional actor selector returned an unknown variant");
    }
    lua_getfield(state, decision_index, "choice_changed");
    if (!lua_isboolean(state, -1)) {
        return fail(
            "conditional actor selector omitted its choice-change status");
    }
    const bool choice_changed = lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);

    ScriptConditionalActorSelection decision{};
    if (kind == "none") {
        decision.kind = ScriptConditionalActorSelectionKind::none;
        if (!selection_id.empty()) {
            return fail("empty conditional actor decision names an activity");
        }
    } else if (kind == "story_contact") {
        decision.kind = ScriptConditionalActorSelectionKind::story_contact;
    } else if (kind == "pending_world_activity") {
        decision.kind =
            ScriptConditionalActorSelectionKind::pending_world_activity;
    } else if (kind == "random_pedestrian") {
        decision.kind = ScriptConditionalActorSelectionKind::random_pedestrian;
    } else if (kind == "mode_one_world_activity") {
        decision.kind =
            ScriptConditionalActorSelectionKind::mode_one_world_activity;
    } else {
        return fail("conditional actor selector returned an unknown decision");
    }
    if (decision.kind != ScriptConditionalActorSelectionKind::none &&
        selection_id.empty()) {
        return fail("selected conditional actor decision has no activity id");
    }
    if (selection_variant == "first") {
        decision.selection_variant = ScriptConditionalActorVariant::first;
    } else if (selection_variant == "second") {
        decision.selection_variant = ScriptConditionalActorVariant::second;
    } else {
        decision.selection_variant = ScriptConditionalActorVariant::none;
    }
    decision.choice_changed = choice_changed;
    decision.selection_id = std::move(selection_id);
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return decision;
}

std::optional<std::string>
GameScriptRuntime::pending_world_activity_id_for_scene(
    const std::uint16_t scene_id, const bool world_effect_active,
    const bool pending_world_effect) {
    auto* state = impl_->state;
    if (state == nullptr || scene_id > 55U) {
        impl_->failure =
            "pending world-activity scene is outside source bounds";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::string> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("world_content")) {
        return fail("gameplay script module is missing: world_content");
    }
    lua_getfield(state, -1,
                 "pending_world_activity_id_for_scene");
    if (!lua_isfunction(state, -1)) {
        return fail(
            "world_content.pending_world_activity_id_for_scene is missing");
    }
    lua_remove(state, -2);
    lua_pushinteger(state, static_cast<lua_Integer>(scene_id));
    lua_pushboolean(state, world_effect_active);
    lua_pushboolean(state, pending_world_effect);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 3, 1, 0) != LUA_OK) {
        return fail(
            "world_content.pending_world_activity_id_for_scene: " +
            stack_error(state));
    }
    if (lua_isnil(state, -1)) {
        lua_settop(state, stack_base);
        impl_->failure.clear();
        return std::nullopt;
    }
    std::string list_id;
    if (!read_string(state, -1, list_id) || !valid_identifier(list_id)) {
        return fail(
            "world-content pending actor-list selector returned an invalid id");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return list_id;
}

std::optional<std::vector<ScriptWorldActivityActor>>
GameScriptRuntime::world_activity_actors_for_activity_id_impl(
    const std::optional<std::uint16_t> scene_id,
    const std::string_view activity_id) {
    auto* state = impl_->state;
    if (state == nullptr || (scene_id.has_value() && *scene_id > 55U) ||
        activity_id.empty() || activity_id.size() > 128U ||
        !valid_identifier(activity_id)) {
        impl_->failure = "world-activity actor lookup has an invalid activity id";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::vector<ScriptWorldActivityActor>> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("world_content")) {
        return fail("gameplay script module is missing: world_content");
    }
    lua_getfield(state, -1, "world_activity_actors_for_activity_id");
    if (!lua_isfunction(state, -1)) {
        return fail("world_content.world_activity_actors_for_activity_id is missing");
    }
    lua_remove(state, -2);
    lua_pushlstring(state, activity_id.data(), activity_id.size());
    int argument_count = 1;
    if (scene_id.has_value()) {
        lua_pushinteger(state, static_cast<lua_Integer>(*scene_id));
        ++argument_count;
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, argument_count, 1, 0) != LUA_OK) {
        return fail("world_content.world_activity_actors_for_activity_id: " +
                    stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("world-content actor lookup must return an array");
    }
    const auto list_index = lua_absindex(state, -1);
    const auto actor_count = lua_rawlen(state, list_index);

    std::vector<ScriptWorldActivityActor> actors;
    actors.reserve(actor_count);
    std::set<std::string> actor_instance_ids;
    for (std::size_t index = 0U; index < actor_count; ++index) {
        lua_rawgeti(state, list_index, static_cast<lua_Integer>(index + 1U));
        if (!lua_istable(state, -1)) {
            return fail("world-content actor list contains a bad row");
        }
        const auto row_index = lua_absindex(state, -1);
        ScriptWorldActivityActor actor{};
        std::string visual_profile_id;
        std::uint32_t palette_line = 0U;
        std::string action_id;
        std::uint32_t world_x = 0U;
        std::uint32_t world_y = 0U;
        std::uint32_t heading = 0U;
        ActorRoleFacts actor_roles;
        ActorInteractionFacts actor_interaction;
        std::uint32_t physical_meter = 0U;
        std::uint32_t action_delay_modifier = 0U;
        std::uint32_t row_scene = 0U;
        bool activity_probe_enabled = false;
        std::string action_kind;
        std::string activity_kind;
        std::string row_instance_id;
        std::string row_activity_id;
        std::string invalid_fact;
        const auto valid_fact = [&invalid_fact](
                                    const bool valid,
                                    const std::string_view name) {
            if (!valid && invalid_fact.empty()) {
                invalid_fact = name;
            }
            return valid;
        };
        const bool valid =
            valid_fact(read_field_integer(state, row_index, "scene", 55U,
                                          row_scene), "scene id") &&
            valid_fact(!scene_id.has_value() || row_scene == *scene_id,
                       "scene selection") &&
            valid_fact(read_field_string(state, row_index, "id",
                                         row_instance_id), "actor identity") &&
            valid_fact(valid_identifier(row_instance_id), "actor identity") &&
            valid_fact(index <= 0xFFFFU, "activity member index") &&
            valid_fact(actor_instance_ids.insert(row_instance_id).second,
                       "actor identity uniqueness") &&
            valid_fact(read_field_string(state, row_index, "activity_id",
                                         row_activity_id), "activity id") &&
            valid_fact(valid_identifier(row_activity_id), "activity id") &&
            valid_fact(row_activity_id == activity_id,
                       "activity selection") &&
            valid_fact(read_field_string(state, row_index, "activity_kind",
                                         activity_kind), "activity kind") &&
            valid_fact(read_field_boolean(state, row_index,
                                          "activity_probe_enabled",
                                          activity_probe_enabled),
                       "activity probe fact") &&
            valid_fact(read_field_text(state, row_index, "name",
                                       actor.actor_name, 128U),
                       "actor name") &&
            valid_fact(read_field_string(state, row_index, "profile_id",
                                         actor.profile_id), "actor profile") &&
            valid_fact(valid_identifier(actor.profile_id), "actor profile") &&
            valid_fact(read_field_string(state, row_index,
                                         "visual_profile_id",
                                         visual_profile_id) &&
                           valid_identifier(visual_profile_id),
                       "visual profile") &&
            valid_fact(read_field_string(state, row_index, "sprite",
                                         actor.sprite_stem), "sprite id") &&
            valid_fact(valid_identifier(actor.sprite_stem), "sprite id") &&
            valid_fact(read_field_string(state, row_index, "action_kind",
                                         action_kind), "action kind") &&
            valid_fact(read_optional_field_string(state, row_index,
                                                  "action_id", action_id),
                       "action selection") &&
            valid_fact(read_field_integer(state, row_index, "palette_line",
                                          3U, palette_line), "palette line") &&
            valid_fact(read_field_integer(state, row_index, "x", 0xFFFFU,
                                          world_x), "world position x") &&
            valid_fact(read_field_integer(state, row_index, "y", 0xFFFFU,
                                          world_y), "world position y") &&
            valid_fact(read_field_integer(state, row_index, "heading",
                                          0xFFFFU, heading), "heading") &&
            valid_fact(read_actor_role_facts(state, row_index, actor_roles),
                       "actor roles") &&
            valid_fact(read_actor_interaction_facts(
                           state, row_index, actor_interaction),
                       "interaction facts") &&
            valid_fact(read_field_integer(state, row_index, "physical_meter",
                                          0xFFU, physical_meter),
                       "physical vitality") &&
            valid_fact(read_field_integer(state, row_index,
                                          "action_delay_modifier", 12U,
                                          action_delay_modifier) &&
                       action_delay_modifier % 4U == 0U,
                       "action-delay modifier");
        if (!valid) {
            return fail("world-content actor descriptor has invalid " +
                        invalid_fact);
        }
        const auto group = std::find_if(
            impl_->activity_group_bindings.begin(),
            impl_->activity_group_bindings.end(),
            [row_scene, &row_activity_id](
                const ScriptWorldActivityGroupBinding& binding) {
                return binding.scene_id == row_scene &&
                       binding.activity_id == row_activity_id;
            });
        if (group == impl_->activity_group_bindings.end()) {
            return fail("world-content actor has no native source-group mapping");
        }
        const auto source_sprite_type =
            script_source_type_from_actor_visual_profile_id(
                visual_profile_id);
        if (!source_sprite_type.has_value()) {
            return fail(
                "world-content actor descriptor has an unknown visual profile");
        }
        if (action_kind == "none") {
            actor.action_kind = ScriptActorActionKind::none;
        } else if (action_kind == "item") {
            actor.action_kind = ScriptActorActionKind::item;
        } else if (action_kind == "spell") {
            actor.action_kind = ScriptActorActionKind::spell;
        } else {
            return fail("world-content script actor action kind is unknown");
        }
        if (activity_kind == "scene_script") {
            actor.activity_kind = ScriptWorldActivityKind::scene_script;
        } else if (activity_kind == "event_driven") {
            actor.activity_kind = ScriptWorldActivityKind::event_driven;
        } else {
            return fail("world-content actor activity kind is unknown");
        }
        const auto source_action_code =
            script_action_code_from_semantic_id(action_kind, action_id);
        if (!source_action_code.has_value()) {
            return fail("world-content script actor action fields disagree");
        }
        actor.scene_id = static_cast<std::uint16_t>(row_scene);
        actor.instance_id = std::move(row_instance_id);
        actor.activity_id = std::move(row_activity_id);
        actor.source_order = static_cast<std::uint16_t>(index);
        actor.activity_probe_enabled = activity_probe_enabled;
        actor.entity_kind = *source_sprite_type;
        actor.combatant.blocks_spell_death_pose =
            script_sprite_type_blocks_spell_death_pose(actor.entity_kind);
        actor.action_code = *source_action_code;
        actor.palette_line = static_cast<std::uint8_t>(palette_line);
        actor.world_x = static_cast<std::uint16_t>(world_x);
        actor.world_y = static_cast<std::uint16_t>(world_y);
        actor.heading = static_cast<std::uint16_t>(heading);
        actor.source_spawn_state_flags = roles_to_source(actor_roles, 0U);
        actor.source_interaction_flags =
            interaction_to_source(actor_interaction);
        actor.physical_meter = static_cast<std::uint8_t>(physical_meter);
        actor.action_delay_modifier =
            static_cast<std::uint8_t>(action_delay_modifier);

        lua_getfield(state, row_index, "combatant");
        if (!lua_istable(state, -1)) {
            return fail("world-content actor combat facts must be a table");
        }
        const auto combatant_index = lua_absindex(state, -1);
        const auto read_combatant_byte =
            [state, combatant_index](const char* field,
                                     std::uint8_t& destination) {
                std::uint32_t value = 0U;
                if (!read_field_integer(state, combatant_index, field, 0xFFU,
                                        value)) {
                    return false;
                }
                destination = static_cast<std::uint8_t>(value);
                return true;
            };
        const auto read_combatant_fact =
            [state, combatant_index](const char* group, const char* field,
                                     std::uint8_t& destination) {
                lua_getfield(state, combatant_index, group);
                if (!lua_istable(state, -1)) {
                    lua_pop(state, 1);
                    return false;
                }
                const auto group_index = lua_absindex(state, -1);
                std::uint32_t value = 0U;
                const bool valid =
                    read_field_integer(state, group_index, field, 0xFFU,
                                       value);
                lua_pop(state, 1);
                if (!valid) {
                    return false;
                }
                destination = static_cast<std::uint8_t>(value);
                return true;
            };
        auto& combatant = actor.combatant;
        ActorInteractionFacts combatant_interaction;
        const bool combatant_scalars_valid =
            read_actor_interaction_facts(state, combatant_index,
                                         combatant_interaction) &&
            read_combatant_byte("physical_meter", combatant.physical_meter) &&
            read_combatant_byte("mental_meter", combatant.mental_meter) &&
            read_combat_cyberware(state, combatant_index,
                                  combatant.cyberware) &&
            read_combat_traits(state, combatant_index, combatant.traits) &&
            read_source_posture_from_lua_percent(
                state, combatant_index, "posture_reduction_percent",
                combatant.source_posture) &&
            read_combatant_fact("attributes", "body",
                                combatant.attributes.body) &&
            read_combatant_fact("attributes", "quickness",
                                combatant.attributes.quickness) &&
            read_combatant_fact("attributes", "strength",
                                combatant.attributes.strength) &&
            read_combatant_fact("attributes", "intelligence",
                                combatant.attributes.intelligence) &&
            read_combatant_fact("attributes", "willpower",
                                combatant.attributes.willpower) &&
            read_combatant_fact("skills", "sorcery",
                                combatant.skills.sorcery) &&
            read_combatant_fact("skills", "firearms",
                                combatant.skills.firearms) &&
            read_combatant_fact("skills", "pistols",
                                combatant.skills.pistols) &&
            read_combatant_fact("skills", "smgs",
                                combatant.skills.smgs) &&
            read_combatant_fact("skills", "shotguns",
                                combatant.skills.shotguns) &&
            read_combatant_fact("skills", "melee_combat",
                                combatant.skills.melee_combat) &&
            read_optional_field_string(
                state, combatant_index, "equipped_armor_id",
                combatant.equipped_armor_id) &&
            (combatant.equipped_armor_id.empty() ||
             script_armor_source_index_from_item_id(
                 combatant.equipped_armor_id).has_value()) &&
            read_combat_action_profile_options(
                state, combatant_index, combatant.action_profile_options) &&
            read_combatant_byte("force", combatant.force) &&
            read_combatant_byte("spell_trial_cap",
                                combatant.spell_trial_cap);
        if (!combatant_scalars_valid) {
            return fail("world-content actor combat facts are invalid");
        }
        if (!read_combat_inventory(state, combatant_index,
                                   combatant.item_codes,
                                   combatant.item_phases) ||
            !read_weapon_action_policies(state, combatant_index,
                                         combatant.weapon_action_policies)) {
            return fail("world-content actor combat facts are invalid");
        }
        combatant.interaction_flags =
            interaction_to_source(combatant_interaction);
        combatant.equipped_action_code = actor.action_code;
        actors.push_back(std::move(actor));
        lua_pop(state, 1);
        lua_pop(state, 1);
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return actors;
}

std::optional<ScriptFieldEncounterOutcome>
GameScriptRuntime::resolve_field_encounter_impl(
    const std::uint16_t scene_id, const double weighted_selection,
    const std::optional<std::string_view> source_sequence_id) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for field encounters";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptFieldEncounterOutcome> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    const bool source_sequence = source_sequence_id.has_value();
    const bool run_target_actor_group = source_sequence &&
        is_run_target_actor_group_id(*source_sequence_id);
    if ((!source_sequence &&
         (scene_id > 55U || !std::isfinite(weighted_selection) ||
          weighted_selection < 0.0 || weighted_selection >= 1.0)) ||
        (source_sequence &&
         (scene_id > 55U || !valid_identifier(*source_sequence_id)))) {
        return fail("field encounter request has invalid scene or weighted selection");
    }
    if (!impl_->push_module("field_encounters")) {
        return fail("field_encounters.lua module is missing");
    }
    const auto* resolver_name = source_sequence
        ? "resolve_sequence" : "resolve";
    lua_getfield(state, -1, resolver_name);
    if (!lua_isfunction(state, -1)) {
        return fail(std::string("field_encounters.") + resolver_name +
                    " is missing");
    }
    lua_remove(state, -2);
    if (source_sequence) {
        lua_pushinteger(state, static_cast<lua_Integer>(scene_id));
        lua_pushlstring(state, source_sequence_id->data(),
                        source_sequence_id->size());
    } else {
        lua_pushinteger(state, scene_id);
        lua_pushnumber(state, weighted_selection);
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 2, 1, 0) != LUA_OK) {
        return fail(std::string("field_encounters.") + resolver_name +
                    " failed");
    }
    if (lua_isnil(state, -1)) {
        lua_settop(state, stack_base);
        impl_->failure.clear();
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        return fail(std::string("field_encounters.") + resolver_name +
                    " returned a non-record result");
    }

    const auto result_index = lua_absindex(state, -1);
    ScriptFieldEncounterOutcome outcome{};
    std::string kind_name;
    std::uint32_t parsed_scene = 0U;
    if (!read_field_string(state, result_index, "kind", kind_name) ||
        !read_field_integer(state, result_index, "scene_id", 55U,
                            parsed_scene) ||
        parsed_scene != scene_id ||
        !read_field_string(state, result_index, "outcome_id",
                           outcome.outcome_id) ||
        !valid_identifier(outcome.outcome_id)) {
        return fail(std::string("field_encounters.") + resolver_name +
                    " returned invalid semantic outcome facts");
    }
    if (kind_name == "dialogue_event") {
        outcome.kind = ScriptFieldEncounterOutcomeKind::dialogue_event;
    } else if (kind_name == "dynamic_world_effect_sequence") {
        outcome.kind =
            ScriptFieldEncounterOutcomeKind::dynamic_world_effect_sequence;
        std::string placement_policy;
        if (!read_field_string(state, result_index, "placement_policy",
                               placement_policy)) {
            return fail(
                "dynamic world encounter has no placement policy");
        }
        if (placement_policy == "explicit_actor_positions") {
            outcome.placement_policy =
                ScriptFieldEncounterPlacementPolicy::explicit_actor_positions;
        } else if (placement_policy == "screen_edge_candidates") {
            outcome.placement_policy =
                ScriptFieldEncounterPlacementPolicy::
                    screen_edge_candidates;
        } else if (placement_policy == "map_derived_candidates") {
            outcome.placement_policy =
                ScriptFieldEncounterPlacementPolicy::
                    map_derived_candidates;
        } else {
            return fail(
                "dynamic world encounter has an unknown placement policy");
        }
    } else {
        return fail(std::string("field_encounters.") + resolver_name +
                    " returned an unknown outcome kind");
    }
    if (outcome.kind ==
        ScriptFieldEncounterOutcomeKind::dynamic_world_effect_sequence) {
        if (!read_field_string(state, result_index, "sequence_id",
                               outcome.sequence_id) ||
            !valid_identifier(outcome.sequence_id) ||
            (source_sequence &&
             outcome.sequence_id != *source_sequence_id)) {
            return fail(std::string("field_encounters.") + resolver_name +
                        " returned an invalid semantic sequence id");
        }
    }
    outcome.scene_id = static_cast<std::uint16_t>(parsed_scene);

    if (outcome.kind == ScriptFieldEncounterOutcomeKind::dialogue_event) {
        if (!read_field_string(state, result_index, "event_id",
                               outcome.event_id) ||
            !valid_identifier(outcome.event_id) ||
            !read_field_text(state, result_index, "opening_text",
                             outcome.opening_text)) {
            return fail(
                "field_encounters.resolve returned invalid event identity or text");
        }
        lua_getfield(state, result_index, "choices");
        if (!lua_istable(state, -1)) {
            lua_pop(state, 1);
            return fail(
                "field_encounters.resolve returned an invalid choice list");
        }
        const auto choices_index = lua_absindex(state, -1);
        const auto choice_count = lua_rawlen(state, choices_index);
        constexpr std::size_t kMaximumEncounterChoiceCount = 64U;
        if (choice_count > kMaximumEncounterChoiceCount) {
            lua_pop(state, 1);
            return fail(
                "field_encounters.resolve returned too many choices");
        }
        outcome.choices.reserve(choice_count);
        for (std::size_t index = 0U; index < choice_count; ++index) {
            lua_rawgeti(state, choices_index,
                        static_cast<lua_Integer>(index + 1U));
            if (!lua_istable(state, -1)) {
                lua_pop(state, 2);
                return fail(
                    "field_encounters.resolve returned a non-record choice");
            }
            const auto choice_index = lua_absindex(state, -1);
            ScriptFieldEncounterChoice choice{};
            const bool choice_valid =
                read_field_string_allow_empty(state, choice_index, "id",
                                              choice.id) &&
                read_field_text(state, choice_index, "label", choice.label,
                                512U) &&
                (choice.id.empty() == choice.label.empty()) &&
                (choice.id.empty() || valid_identifier(choice.id));
            lua_pop(state, 1);
            if (!choice_valid) {
                lua_pop(state, 1);
                return fail(
                    "field_encounters.resolve returned an invalid semantic choice");
            }
            outcome.choices.push_back(std::move(choice));
        }
        lua_pop(state, 1);
    }

    if (outcome.kind ==
        ScriptFieldEncounterOutcomeKind::dynamic_world_effect_sequence) {
        lua_getfield(state, result_index, "actors");
        if (!lua_istable(state, -1)) {
            return fail("dynamic world encounter has no actor list");
        }
        const auto actors_index = lua_absindex(state, -1);
        const auto actor_count = lua_rawlen(state, actors_index);
        if (actor_count == 0U || actor_count > 16U) {
            return fail("dynamic world encounter actor count is invalid");
        }
        outcome.actors.reserve(actor_count);
        for (std::size_t index = 0U; index < actor_count; ++index) {
            lua_rawgeti(state, actors_index,
                        static_cast<lua_Integer>(index + 1U));
            if (!lua_istable(state, -1)) {
                return fail("dynamic world encounter contains a bad actor row");
            }
            const auto actor_index = lua_absindex(state, -1);
            ScriptFieldEncounterActor actor{};
            std::uint32_t source_order = 0U;
            std::string visual_profile_id;
            std::string action_id;
            ActorRoleFacts actor_roles;
            ActorInteractionFacts actor_interaction;
            std::uint32_t physical_meter = 0U;
            std::uint32_t palette_line = 0U;
            bool activity_probe_enabled = false;
            std::uint32_t action_delay_modifier = 0U;
            std::optional<std::uint32_t> run_target_spawn_x;
            std::optional<std::uint32_t> run_target_spawn_y;
            std::string action_kind;
            bool valid_actor =
                read_field_integer(state, actor_index, "sequence_order", 31U,
                                   source_order) &&
                read_field_string(state, actor_index, "sequence_id",
                                  actor.sequence_id) &&
                valid_identifier(actor.sequence_id) &&
                (!source_sequence ||
                 actor.sequence_id == *source_sequence_id) &&
                read_field_text(state, actor_index, "actor_name",
                                actor.actor_name, 128U) &&
                read_field_string(state, actor_index, "profile_id",
                                  actor.profile_id) &&
                valid_identifier(actor.profile_id) &&
                read_field_string(state, actor_index, "visual_profile_id",
                                  visual_profile_id) &&
                valid_identifier(visual_profile_id) &&
                read_field_string(state, actor_index, "action_kind",
                                  action_kind) &&
                read_optional_field_string(state, actor_index, "action_id",
                                           action_id) &&
                read_actor_role_facts(state, actor_index, actor_roles) &&
                read_actor_interaction_facts(state, actor_index,
                                             actor_interaction) &&
                read_field_integer(state, actor_index, "physical_meter",
                                   255U, physical_meter) &&
                read_field_integer(state, actor_index, "palette_line", 3U,
                                   palette_line) &&
                read_field_string(state, actor_index, "sprite_stem",
                                  actor.sprite_stem) &&
                valid_identifier(actor.sprite_stem) &&
                read_field_boolean(state, actor_index,
                                   "activity_probe_enabled",
                                   activity_probe_enabled) &&
                read_field_integer(state, actor_index,
                                   "action_delay_modifier", 12U,
                                   action_delay_modifier) &&
                action_delay_modifier % 4U == 0U &&
                read_optional_field_u32(state, actor_index, "spawn_x",
                                        0xFFFFU, run_target_spawn_x) &&
                read_optional_field_u32(state, actor_index, "spawn_y",
                                        0xFFFFU, run_target_spawn_y);
            const auto source_action_code =
                script_action_code_from_semantic_id(action_kind, action_id);
            if (!source_action_code.has_value()) {
                valid_actor = false;
            }
            lua_getfield(state, actor_index, "combatant");
            if (!lua_istable(state, -1)) {
                valid_actor = false;
            } else {
                const auto combat_index = lua_absindex(state, -1);
                std::uint32_t combat_value = 0U;
                const auto read_combat_byte = [&](const char* field,
                                                  std::uint8_t& value) {
                    if (!read_field_integer(state, combat_index, field,
                                            0xFFU, combat_value)) {
                        return false;
                    }
                    value = static_cast<std::uint8_t>(combat_value);
                    return true;
                };
                const auto read_combat_fact = [&](const char* group,
                                                  const char* field,
                                                  std::uint8_t& value) {
                    lua_getfield(state, combat_index, group);
                    if (!lua_istable(state, -1)) {
                        lua_pop(state, 1);
                        return false;
                    }
                    const auto group_index = lua_absindex(state, -1);
                    const bool valid = read_field_integer(
                        state, group_index, field, 0xFFU, combat_value);
                    lua_pop(state, 1);
                    if (!valid) {
                        return false;
                    }
                    value = static_cast<std::uint8_t>(combat_value);
                    return true;
                };
                const auto source_profile_binding =
                    impl_->action_profile_index_by_actor_profile_id.find(
                        actor.profile_id);
                ActorInteractionFacts combat_interaction;
                bool valid_combatant =
                    read_actor_interaction_facts(state, combat_index,
                                                 combat_interaction) &&
                    read_combat_cyberware(state, combat_index,
                                          actor.combatant.cyberware) &&
                    read_combat_traits(state, combat_index,
                                       actor.combatant.traits) &&
                    read_combat_byte("physical_meter",
                                     actor.combatant.physical_meter) &&
                    read_combat_byte("mental_meter",
                                     actor.combatant.mental_meter) &&
                    read_source_posture_from_lua_percent(
                        state, combat_index, "posture_reduction_percent",
                        actor.combatant.source_posture) &&
                    read_combat_fact("attributes", "body",
                                     actor.combatant.attributes.body) &&
                    read_combat_fact("attributes", "quickness",
                                     actor.combatant.attributes.quickness) &&
                    read_combat_fact("attributes", "strength",
                                     actor.combatant.attributes.strength) &&
                    read_combat_fact("attributes", "intelligence",
                                     actor.combatant.attributes.intelligence) &&
                    read_combat_fact("attributes", "willpower",
                                     actor.combatant.attributes.willpower) &&
                    read_combat_fact("skills", "sorcery",
                                     actor.combatant.skills.sorcery) &&
                    read_combat_fact("skills", "firearms",
                                     actor.combatant.skills.firearms) &&
                    read_combat_fact("skills", "pistols",
                                     actor.combatant.skills.pistols) &&
                    read_combat_fact("skills", "smgs",
                                     actor.combatant.skills.smgs) &&
                    read_combat_fact("skills", "shotguns",
                                     actor.combatant.skills.shotguns) &&
                    read_combat_fact("skills", "melee_combat",
                                     actor.combatant.skills.melee_combat) &&
                    read_combat_byte("force", actor.combatant.force) &&
                    read_combat_byte("spell_trial_cap",
                                     actor.combatant.spell_trial_cap) &&
                    read_optional_field_string(
                        state, combat_index, "equipped_armor_id",
                        actor.combatant.equipped_armor_id) &&
                    (actor.combatant.equipped_armor_id.empty() ||
                     script_armor_source_index_from_item_id(
                         actor.combatant.equipped_armor_id).has_value()) &&
                    read_combat_action_profile_options(
                        state, combat_index,
                        actor.combatant.action_profile_options) &&
                    source_profile_binding !=
                        impl_->action_profile_index_by_actor_profile_id.end();
                if (valid_combatant && source_action_code.has_value()) {
                    actor.combatant.interaction_flags =
                        interaction_to_source(combat_interaction);
                    actor.combatant.equipped_action_code =
                        *source_action_code;
                    actor.combatant.linked_action_profile_index =
                        source_profile_binding->second;
                    valid_combatant =
                        read_combat_inventory(
                            state, combat_index,
                            actor.combatant.item_codes,
                            actor.combatant.item_phases) &&
                        read_weapon_action_policies(
                            state, combat_index,
                            actor.combatant.weapon_action_policies);
                }
                valid_actor = valid_actor && valid_combatant;
            }
            lua_pop(state, 1);
            const bool has_run_target_spawn =
                run_target_spawn_x.has_value() &&
                run_target_spawn_y.has_value();
            if (!valid_actor || source_order != index ||
                (run_target_spawn_x.has_value() !=
                 run_target_spawn_y.has_value()) ||
                (run_target_actor_group && !has_run_target_spawn)) {
                return fail(
                    "dynamic world encounter actor record is incomplete or unordered "
                    "(valid=" + std::to_string(valid_actor) +
                    ", source_order=" + std::to_string(source_order) +
                    ", expected_order=" + std::to_string(index) +
                    ", spawn_x=" + (run_target_spawn_x.has_value()
                                         ? std::to_string(*run_target_spawn_x)
                                         : std::string("nil")) +
                    ", spawn_y=" + (run_target_spawn_y.has_value()
                                         ? std::to_string(*run_target_spawn_y)
                                         : std::string("nil")) + ")");
            }
            if (action_kind == "none") {
                actor.action_kind = ScriptActorActionKind::none;
            } else if (action_kind == "item") {
                actor.action_kind = ScriptActorActionKind::item;
            } else if (action_kind == "spell") {
                actor.action_kind = ScriptActorActionKind::spell;
            } else {
                return fail("dynamic world encounter has an unknown action kind");
            }
            actor.source_order = static_cast<std::uint16_t>(source_order);
            if (has_run_target_spawn) {
                actor.run_target_spawn_x = static_cast<std::uint16_t>(
                    *run_target_spawn_x);
                actor.run_target_spawn_y = static_cast<std::uint16_t>(
                    *run_target_spawn_y);
            }
            const auto source_sprite_type =
                script_source_type_from_actor_visual_profile_id(
                    visual_profile_id);
            if (!source_sprite_type.has_value()) {
                return fail(
                    "dynamic world encounter has an unknown visual profile");
            }
            actor.entity_kind = *source_sprite_type;
            actor.combatant.blocks_spell_death_pose =
                script_sprite_type_blocks_spell_death_pose(actor.entity_kind);
            actor.action_code = *source_action_code;
            actor.source_spawn_state_flags = roles_to_source(actor_roles, 0U);
            actor.source_interaction_flags =
                interaction_to_source(actor_interaction);
            actor.physical_meter =
                static_cast<std::uint8_t>(physical_meter);
            actor.palette_line = static_cast<std::uint8_t>(palette_line);
            actor.activity_probe_enabled = activity_probe_enabled;
            actor.action_delay_modifier =
                static_cast<std::uint8_t>(action_delay_modifier);
            if ((actor.action_kind == ScriptActorActionKind::none) !=
                (actor.action_code == 0U)) {
                return fail("dynamic world encounter action facts disagree");
            }
            outcome.actors.push_back(std::move(actor));
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return outcome;
}

std::optional<ScriptFieldEncounterOutcome>
GameScriptRuntime::resolve_run_target_actor_group(
    const std::string_view actor_group_id) {
    if (!is_run_target_actor_group_id(actor_group_id)) {
        impl_->failure = "run actor group identity is unknown";
        return std::nullopt;
    }
    return resolve_field_encounter_impl(15U, 0.5, actor_group_id);
}

std::optional<std::uint16_t> GameScriptRuntime::scene_music(
    const std::uint16_t scene_id, const std::uint16_t profile_music) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for scene music";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::uint16_t> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("audio")) {
        return fail("audio.lua module is missing");
    }
    lua_getfield(state, -1, "scene_music");
    if (!lua_isfunction(state, -1)) {
        return fail("audio.scene_music is missing");
    }
    lua_remove(state, -2);
    lua_createtable(state, 0, 2);
    lua_pushinteger(state, scene_id);
    lua_setfield(state, -2, "scene_id");
    lua_pushinteger(state, profile_music);
    lua_setfield(state, -2, "profile_music");
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("audio.scene_music failed");
    }
    std::optional<std::uint16_t> result;
    if (lua_isinteger(state, -1)) {
        const auto value = lua_tointeger(state, -1);
        if (value < 0 || value > 0xFF) {
            return fail("audio.scene_music returned an id outside 0..0xFF");
        }
        result = static_cast<std::uint16_t>(value);
    } else if (!lua_isnil(state, -1)) {
        return fail("audio.scene_music returned a non-integer");
    }
    lua_settop(state, stack_base);
    return result;
}

std::optional<ScriptFieldEncounterTriggerAction>
GameScriptRuntime::evaluate_field_encounter_trigger(
    const ScriptFieldEncounterTriggerInput& input) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for field encounter triggers";
        return std::nullopt;
    }
    if (input.scene_id > static_cast<std::uint16_t>(srx::SceneId::FINAL)) {
        impl_->failure = "field encounter trigger scene is outside game content";
        return std::nullopt;
    }

    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptFieldEncounterTriggerAction> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("field_encounters")) {
        return fail("field_encounters.lua module is missing");
    }
    lua_getfield(state, -1, "trigger_attempt");
    if (!lua_isfunction(state, -1)) {
        return fail("field_encounters.trigger_attempt is missing");
    }
    lua_remove(state, -2);
    lua_createtable(state, 0, 10);
    const auto set_integer = [state](const char* name,
                                     const std::uint16_t value) {
        lua_pushinteger(state, value);
        lua_setfield(state, -2, name);
    };
    const auto set_boolean = [state](const char* name, const bool value) {
        lua_pushboolean(state, value);
        lua_setfield(state, -2, name);
    };
    const auto set_number = [state](const char* name, const double value) {
        lua_pushnumber(state, value);
        lua_setfield(state, -2, name);
    };
    set_integer("scene_id", input.scene_id);
    set_boolean("encounter_check_due", input.encounter_check_due);
    set_boolean("display_busy", input.display_busy);
    set_boolean("leader_unseen", input.leader_unseen);
    set_boolean("world_activity_locked", input.world_activity_locked);
    set_boolean("alarm_active", input.alarm_active);
    set_boolean("combat_active", input.combat_active);
    set_boolean("encounter_in_progress", input.encounter_in_progress);
    if (input.probability_sample.has_value()) {
        set_number("probability_sample", *input.probability_sample);
    } else {
        lua_pushnil(state);
        lua_setfield(state, -2, "probability_sample");
    }
    set_number("encounter_probability", input.encounter_probability);

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("field_encounters.trigger_attempt failed");
    }
    if (!lua_isstring(state, -1)) {
        return fail("field_encounters.trigger_attempt returned invalid action");
    }
    std::size_t action_length = 0U;
    const auto* action_text = lua_tolstring(state, -1, &action_length);
    const std::string_view action(action_text, action_length);
    std::optional<ScriptFieldEncounterTriggerAction> result;
    if (action == "blocked") {
        result = ScriptFieldEncounterTriggerAction::blocked;
    } else if (action == "not_due") {
        result = ScriptFieldEncounterTriggerAction::not_due;
    } else if (action == "sample_probability") {
        result = ScriptFieldEncounterTriggerAction::sample_probability;
    } else if (action == "random_miss") {
        result = ScriptFieldEncounterTriggerAction::random_miss;
    } else if (action == "select_alarm_sequence") {
        result = ScriptFieldEncounterTriggerAction::select_alarm_sequence;
    } else if (action == "select_scene_outcome") {
        result = ScriptFieldEncounterTriggerAction::select_scene_outcome;
    }
    if (!result.has_value()) {
        return fail("field_encounters.trigger_attempt returned unknown action");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<ScriptFieldAlarmProfile>
GameScriptRuntime::field_alarm_profile(const std::uint16_t scene_id) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for field alarm profiles";
        return std::nullopt;
    }
    if (scene_id > static_cast<std::uint16_t>(srx::SceneId::FINAL)) {
        impl_->failure = "field alarm profile scene is outside the ROM table";
        return std::nullopt;
    }

    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptFieldAlarmProfile> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("world_content_data")) {
        return fail("world-content TSV data module is missing");
    }
    const auto content_table = lua_absindex(state, -1);
    lua_getfield(state, content_table, "field_alarm_profiles");
    if (!lua_istable(state, -1)) {
        return fail("world_content_data.field_alarm_profiles is missing");
    }
    lua_rawgeti(state, -1, static_cast<lua_Integer>(scene_id));
    if (lua_isnil(state, -1)) {
        lua_settop(state, stack_base);
        impl_->failure.clear();
        return ScriptFieldAlarmProfile{};
    }
    if (!lua_istable(state, -1)) {
        return fail("field alarm profile row is not a table");
    }
    const auto profile_table = lua_absindex(state, -1);
    ScriptFieldAlarmProfile profile;
    std::string activity_id;
    if (!read_optional_field_string(state, profile_table,
                                    "reinforcement_activity_id",
                                    activity_id) ||
        (!activity_id.empty() && !valid_identifier(activity_id))) {
        return fail("field alarm profile has an invalid activity id");
    }
    if (!activity_id.empty()) {
        profile.reinforcement_activity_id = std::move(activity_id);
    }
    std::optional<std::uint32_t> expected_region_count;
    if (!read_optional_field_u32(state, profile_table, "region_count", 256U,
                                 expected_region_count) ||
        !expected_region_count.has_value()) {
        return fail("field alarm profile has an invalid region count");
    }

    lua_getfield(state, content_table, "field_alarm_regions");
    if (!lua_istable(state, -1)) {
        return fail("world_content_data.field_alarm_regions is missing");
    }
    lua_rawgeti(state, -1, static_cast<lua_Integer>(scene_id));
    if (!lua_istable(state, -1)) {
        if (lua_isnil(state, -1) && *expected_region_count == 0U) {
            lua_settop(state, stack_base);
            impl_->failure.clear();
            return profile;
        }
        return fail("field alarm region rows are missing or invalid");
    }
    const auto regions_table = lua_absindex(state, -1);
    const auto region_count = lua_rawlen(state, regions_table);
    if (region_count > 256U || region_count != *expected_region_count) {
        return fail("field alarm region rows disagree with the profile count");
    }
    profile.alarm_regions.reserve(region_count);
    for (std::size_t index = 0U; index < region_count; ++index) {
        lua_rawgeti(state, regions_table,
                    static_cast<lua_Integer>(index + 1U));
        if (!lua_istable(state, -1)) {
            return fail("field alarm region row is not a table");
        }
        const auto region_table = lua_absindex(state, -1);
        std::uint32_t x_min = 0U;
        std::uint32_t x_max = 0U;
        std::uint32_t y_min = 0U;
        std::uint32_t y_max = 0U;
        const auto coordinate_max = static_cast<std::uint32_t>(
            srx::FieldAlarm::WORD_MAX);
        if (!read_field_integer(state, region_table, "x_min",
                                coordinate_max, x_min) ||
            !read_field_integer(state, region_table, "x_max",
                                coordinate_max, x_max) ||
            !read_field_integer(state, region_table, "y_min",
                                coordinate_max, y_min) ||
            !read_field_integer(state, region_table, "y_max",
                                coordinate_max, y_max)) {
            return fail("field alarm region has invalid coordinates");
        }
        profile.alarm_regions.push_back(ScriptFieldAlarmRegion{
            x_min, x_max, y_min, y_max});
        lua_pop(state, 1);
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return profile;
}

std::optional<ScriptFieldAlarmResult>
GameScriptRuntime::evaluate_field_alarm(const ScriptFieldAlarmInput& input) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for field alarm policy";
        return std::nullopt;
    }
    if (input.alarm_regions.size() > 256U ||
        input.actors.size() > 16U ||
        (input.reinforcement_activity_id.has_value() &&
         !valid_identifier(*input.reinforcement_activity_id))) {
        impl_->failure = "field alarm input exceeds its source bounds";
        return std::nullopt;
    }

    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptFieldAlarmResult> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("field_alarm")) {
        return fail("field_alarm.lua module is missing");
    }
    lua_getfield(state, -1, "tick");
    if (!lua_isfunction(state, -1)) {
        return fail("field_alarm.tick is missing");
    }
    lua_remove(state, -2);
    lua_createtable(state, 0, 17);
    const auto input_table = lua_absindex(state, -1);
    const auto set_integer = [state, input_table](const char* name,
                                                   const lua_Integer value) {
        lua_pushinteger(state, value);
        lua_setfield(state, input_table, name);
    };
    const auto set_boolean = [state, input_table](const char* name,
                                                   const bool value) {
        lua_pushboolean(state, value);
        lua_setfield(state, input_table, name);
    };
    set_boolean("alarm_check_due", input.alarm_check_due);
    set_boolean("building_scene", input.building_scene);
    set_boolean("alarm_timeout_scene", input.alarm_timeout_scene);
    set_integer("player_tile_x", input.player_tile_x);
    set_integer("player_tile_y", input.player_tile_y);
    set_boolean("security_disabled", input.security_disabled);
    set_boolean("alarm_active", input.alarm_active);
    set_integer("alarm_time_remaining_frames",
                input.alarm_time_remaining_frames);
    set_boolean("direct_raise", input.direct_raise);
    set_boolean("clear_alarm", input.clear_alarm);
    set_boolean("disable_alarm", input.disable_alarm);
    set_boolean("reset", input.reset);
    set_boolean("scene_exit", input.scene_exit);
    if (input.probability_sample.has_value()) {
        lua_pushnumber(state, *input.probability_sample);
        lua_setfield(state, input_table, "probability_sample");
    } else {
        lua_pushnil(state);
        lua_setfield(state, input_table, "probability_sample");
    }
    if (input.reinforcement_activity_id.has_value()) {
        lua_pushlstring(state, input.reinforcement_activity_id->data(),
                        input.reinforcement_activity_id->size());
        lua_setfield(state, input_table, "reinforcement_activity_id");
    } else {
        lua_pushnil(state);
        lua_setfield(state, input_table, "reinforcement_activity_id");
    }
    lua_createtable(state, static_cast<int>(input.alarm_regions.size()), 0);
    for (std::size_t index = 0U;
         index < input.alarm_regions.size(); ++index) {
        const auto& region = input.alarm_regions[index];
        lua_createtable(state, 0, 4);
        const auto region_table = lua_absindex(state, -1);
        lua_pushinteger(state, region.min_tile_x);
        lua_setfield(state, region_table, "min_tile_x");
        lua_pushinteger(state, region.max_tile_x);
        lua_setfield(state, region_table, "max_tile_x");
        lua_pushinteger(state, region.min_tile_y);
        lua_setfield(state, region_table, "min_tile_y");
        lua_pushinteger(state, region.max_tile_y);
        lua_setfield(state, region_table, "max_tile_y");
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, input_table, "alarm_regions");
    lua_createtable(state, static_cast<int>(input.actors.size()), 0);
    for (std::size_t index = 0U; index < input.actors.size(); ++index) {
        const auto& actor = input.actors[index];
        lua_createtable(state, 0, 2);
        const auto actor_table = lua_absindex(state, -1);
        lua_pushlstring(state, actor.actor_id.data(), actor.actor_id.size());
        lua_setfield(state, actor_table, "actor_id");
        lua_createtable(state, 0, 1);
        const auto facts_table = lua_absindex(state, -1);
        push_named_boolean_group(state, actor.roles,
            {{"spotted", &ActorRoleFacts::spotted},
             {"reinforcement", &ActorRoleFacts::reinforcement},
             {"despawn_offscreen", &ActorRoleFacts::despawn_offscreen},
             {"quest", &ActorRoleFacts::quest},
             {"runner", &ActorRoleFacts::runner},
             {"pedestrian", &ActorRoleFacts::pedestrian},
             {"hostile", &ActorRoleFacts::hostile}});
        lua_setfield(state, facts_table, "roles");
        lua_setfield(state, actor_table, "actor_facts");
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, input_table, "actors");

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("field_alarm.tick failed");
    }
    if (!lua_istable(state, -1)) {
        return fail("field_alarm.tick returned a non-table result");
    }
    const auto result_table = lua_absindex(state, -1);
    ScriptFieldAlarmResult result;
    if (!read_field_boolean(state, result_table, "alarm_active",
                            result.alarm_active) ||
        !read_field_boolean(state, result_table, "security_disabled",
                            result.security_disabled)) {
        return fail("field_alarm.tick omitted alarm state");
    }
    int timer = 0;
    if (!read_field_signed_integer(
            state, result_table, "alarm_time_remaining_frames", -1,
            static_cast<int>(srx::AiTuning::ALARM_FRAMES), timer)) {
        return fail("field_alarm.tick returned invalid remaining duration");
    }
    result.alarm_time_remaining_frames = timer;
    const auto read_optional_boolean = [state, result_table](
        const char* field, bool& value) {
        lua_getfield(state, result_table, field);
        if (lua_isnil(state, -1)) {
            lua_pop(state, 1);
            return true;
        }
        const bool valid = lua_isboolean(state, -1) != 0;
        if (valid) {
            value = lua_toboolean(state, -1) != 0;
        }
        lua_pop(state, 1);
        return valid;
    };
    if (!read_optional_boolean("request_probability_sample",
                               result.request_probability_sample) ||
        !read_optional_boolean("schedule_timeout", result.schedule_timeout) ||
        !read_optional_boolean("spawn_reinforcements",
                               result.spawn_reinforcements)) {
        return fail("field_alarm.tick returned an invalid action flag");
    }
    std::optional<std::uint8_t> sound_id;
    if (!read_optional_field_integer(state, result_table, "sound_start",
                                    static_cast<std::uint32_t>(
                                        srx::SoundId::ALARM), sound_id)) {
        return fail("field_alarm.tick returned an invalid start sound");
    }
    if (sound_id.has_value()) {
        result.sound_start = *sound_id;
    }
    if (!read_optional_field_integer(state, result_table, "sound_stop",
                                    static_cast<std::uint32_t>(
                                        srx::SoundId::ALARM), sound_id)) {
        return fail("field_alarm.tick returned an invalid stop sound");
    }
    if (sound_id.has_value()) {
        result.sound_stop = *sound_id;
    }
    std::string flag_action;
    if (!read_optional_field_string(state, result_table, "clear_flags",
                                    flag_action)) {
        return fail("field_alarm.tick returned an invalid flag action");
    }
    if (flag_action == "clear_spotted") {
        result.flag_action = ScriptFieldAlarmFlagAction::clear_spotted;
    } else if (flag_action == "promote_reinforcement") {
        result.flag_action =
            ScriptFieldAlarmFlagAction::promote_reinforcement;
    } else if (!flag_action.empty()) {
        return fail("field_alarm.tick returned an unknown flag action");
    }
    std::string reinforcement_activity_id;
    if (!read_optional_field_string(state, result_table,
                                    "reinforcement_activity_id",
                                    reinforcement_activity_id) ||
        (!reinforcement_activity_id.empty() &&
         !valid_identifier(reinforcement_activity_id))) {
        return fail("field_alarm.tick returned an invalid actor list");
    }
    if (!reinforcement_activity_id.empty()) {
        result.reinforcement_activity_id =
            std::move(reinforcement_activity_id);
    }
    lua_getfield(state, result_table, "actors");
    if (!lua_istable(state, -1) || lua_rawlen(state, -1) != input.actors.size()) {
        return fail("field_alarm.tick returned a mismatched actor list");
    }
    const auto actors_table = lua_absindex(state, -1);
    result.actors.reserve(input.actors.size());
    for (std::size_t index = 0U; index < input.actors.size(); ++index) {
        lua_rawgeti(state, actors_table,
                    static_cast<lua_Integer>(index + 1U));
        if (!lua_istable(state, -1)) {
            return fail("field_alarm.tick returned an invalid actor row");
        }
        const auto actor_table = lua_absindex(state, -1);
        std::string actor_id;
        ActorRoleFacts roles;
        const bool actor_id_valid =
            read_field_string(state, actor_table, "actor_id", actor_id);
        const bool actor_roles_valid =
            read_actor_role_facts(state, actor_table, roles);
        if (!actor_id_valid || !actor_roles_valid ||
            actor_id != input.actors[index].actor_id) {
            return fail(
                "field_alarm.tick returned invalid actor facts at row " +
                std::to_string(index) + ": expected actor_id='" +
                input.actors[index].actor_id + "', actual actor_id='" +
                (actor_id_valid ? actor_id : std::string("<invalid>")) +
                "', roles_valid=" +
                (actor_roles_valid ? std::string("true") :
                                     std::string("false")));
        }
        result.actors.push_back(ScriptFieldAlarmActorFacts{
            std::move(actor_id), roles});
        lua_pop(state, 1);
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<std::vector<FieldMapEventRule>>
GameScriptRuntime::map_event_rules() {
    auto* state = impl_->state;
    if (!impl_->push_module("world_content_data")) {
        impl_->failure = "world-content TSV data module is missing";
        return std::nullopt;
    }
    lua_getfield(state, -1, "map_events");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "world_content_data.map_events must be an array";
        return std::nullopt;
    }
    const auto rules_table = lua_absindex(state, -1);
    if (!impl_->push_module("field_events")) {
        lua_pop(state, 2);
        impl_->failure = "field_events.lua module is missing";
        return std::nullopt;
    }
    const auto module_table = lua_absindex(state, -1);
    const auto count = lua_rawlen(state, rules_table);
    if (count == 0U || count > kMaximumMapEventRules) {
        lua_pop(state, 3);
        impl_->failure = "world_content_data.map_events has invalid rule count";
        return std::nullopt;
    }
    std::vector<FieldMapEventRule> rules;
    rules.reserve(count);
    std::set<std::uint32_t> identifiers;
    for (std::size_t index = 1U; index <= count; ++index) {
        lua_rawgeti(state, rules_table, static_cast<lua_Integer>(index));
        FieldMapEventRule rule{};
        bool valid = parse_event_rule(state, -1, rule) &&
                     identifiers.insert(rule.id).second;
        if (valid &&
            rule.action == FieldMapEventScriptAction::run_lua_handler) {
            lua_getfield(state, module_table, rule.handler.c_str());
            valid = lua_isfunction(state, -1) != 0;
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
        if (!valid) {
            lua_pop(state, 3);
            impl_->failure = "world_content_data.map_events contains an invalid or duplicate rule";
            return std::nullopt;
        }
        rules.push_back(std::move(rule));
    }
    lua_pop(state, 3);
    impl_->failure.clear();
    return rules;
}

std::optional<ScriptShopDefinition> GameScriptRuntime::shop_definition(
    const std::string_view shop_id) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for shop data";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptShopDefinition> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!valid_identifier(shop_id) || !impl_->push_module("shop")) {
        return fail("shop.lua module or shop identifier is invalid");
    }

    lua_getfield(state, -1, "catalog");
    if (!lua_istable(state, -1)) {
        return fail("shop.catalog must be a table");
    }
    lua_getfield(state, -1, std::string(shop_id).c_str());
    if (!lua_istable(state, -1)) {
        return fail("shop catalog entry is missing");
    }
    const auto definition_index = lua_absindex(state, -1);
    ScriptShopDefinition definition;
    definition.id = std::string(shop_id);
    std::uint32_t menu_mode = 0U;
    int menu_result = 0;
    if (!read_field_string(state, definition_index, "title_key",
                           definition.title_key) ||
        !valid_identifier(definition.title_key) ||
        !read_field_integer(state, definition_index, "menu_mode", 0xFFU,
                            menu_mode) ||
        !read_field_signed_integer(state, definition_index, "menu_result",
                                   -0x80, 0x7F, menu_result)) {
        return fail("shop catalog title or source mode/result is invalid");
    }
    definition.menu_mode = static_cast<std::uint8_t>(menu_mode);
    definition.menu_result = static_cast<std::int8_t>(menu_result);
    lua_getfield(state, definition_index, "text_keys");
    if (!lua_istable(state, -1)) {
        return fail("shop catalog text_keys must be a table");
    }
    const auto text_keys_index = lua_absindex(state, -1);
    const auto read_text_key = [state, text_keys_index](
                                   const char* field, std::string& value) {
        return read_field_string(state, text_keys_index, field, value) &&
               valid_identifier(value);
    };
    const bool text_keys_valid =
        read_text_key("universal_clips_title",
                      definition.text_keys.universal_clips_title) &&
        read_text_key("universal_clips_description",
                      definition.text_keys.universal_clips_description) &&
        read_text_key("transaction_prompt",
                      definition.text_keys.transaction_prompt) &&
        read_text_key("no_inventory_slot",
                      definition.text_keys.no_inventory_slot) &&
        read_text_key("insufficient_funds",
                      definition.text_keys.insufficient_funds) &&
        read_text_key("special_action_limit",
                      definition.text_keys.special_action_limit) &&
        read_text_key("protected_sale",
                      definition.text_keys.protected_sale) &&
        read_text_key("sale_quote_heading",
                      definition.text_keys.sale_quote_heading) &&
        read_text_key("purchase_price_heading",
                      definition.text_keys.purchase_price_heading) &&
        read_text_key("progression_training_unavailable",
                      definition.text_keys.progression_training_unavailable) &&
        read_text_key("progression_maximum_level",
                      definition.text_keys.progression_maximum_level) &&
        read_text_key("progression_cost_heading",
                      definition.text_keys.progression_cost_heading) &&
        read_text_key("progression_level_label",
                      definition.text_keys.progression_level_label) &&
        read_text_key("cyberware_already_owned",
                      definition.text_keys.cyberware_already_owned) &&
        read_text_key("cyberware_spurs_conflict",
                      definition.text_keys.cyberware_spurs_conflict) &&
        read_text_key("cyberware_insufficient_essence",
                      definition.text_keys.cyberware_insufficient_essence) &&
        read_text_key("cyberware_purchase_heading",
                      definition.text_keys.cyberware_purchase_heading) &&
        read_text_key("cyberware_essence_heading",
                      definition.text_keys.cyberware_essence_heading) &&
        read_text_key("cyberware_essence_cost_heading",
                      definition.text_keys.cyberware_essence_cost_heading);
    lua_pop(state, 1);
    if (!text_keys_valid) {
        return fail("shop catalog transaction text keys are incomplete");
    }

    lua_getfield(state, definition_index,
                 "equipment_upgrade_meter_thresholds");
    if (lua_istable(state, -1)) {
        const auto thresholds_index = lua_absindex(state, -1);
        const auto threshold_count = lua_rawlen(state, thresholds_index);
        if (threshold_count != 16U) {
            return fail("equipment upgrade threshold table must contain 16 rows");
        }
        definition.equipment_upgrade_meter_thresholds.reserve(threshold_count);
        for (std::size_t index = 1U; index <= threshold_count; ++index) {
            lua_rawgeti(state, thresholds_index,
                        static_cast<lua_Integer>(index));
            std::uint32_t threshold = 0U;
            const bool valid = lua_isinteger(state, -1) &&
                lua_tointeger(state, -1) >= 0 &&
                lua_tointeger(state, -1) <= 0xFF;
            if (valid) {
                threshold = static_cast<std::uint32_t>(
                    lua_tointeger(state, -1));
            }
            lua_pop(state, 1);
            if (!valid) {
                return fail("equipment upgrade threshold is outside a byte");
            }
            definition.equipment_upgrade_meter_thresholds.push_back(
                static_cast<std::uint8_t>(threshold));
        }
    } else if (!lua_isnil(state, -1)) {
        return fail("equipment upgrade thresholds must be an array");
    }
    lua_pop(state, 1);
    lua_getfield(state, definition_index, "groups");
    if (!lua_istable(state, -1)) {
        return fail("shop catalog groups must be an array");
    }
    const auto groups_index = lua_absindex(state, -1);
    const auto group_count = lua_rawlen(state, groups_index);
    if (group_count == 0U || group_count > kMaximumShopGroups) {
        return fail("shop catalog has an invalid group count");
    }

    std::set<std::string> group_ids;
    definition.groups.reserve(group_count);
    for (std::size_t group_index = 1U; group_index <= group_count;
         ++group_index) {
        lua_rawgeti(state, groups_index,
                    static_cast<lua_Integer>(group_index));
        if (!lua_istable(state, -1)) {
            return fail("shop catalog contains a non-table group");
        }
        const auto group_table = lua_absindex(state, -1);
        ScriptShopOfferGroup group;
        if (!read_field_string(state, group_table, "id", group.id) ||
            !valid_identifier(group.id) ||
            !group_ids.insert(group.id).second) {
            return fail("shop catalog contains an invalid or duplicate group id");
        }
        lua_getfield(state, group_table, "title_keys");
        if (!lua_istable(state, -1) || lua_rawlen(state, -1) != 2U) {
            return fail("shop offer group must contain two title keys");
        }
        const auto title_keys_index = lua_absindex(state, -1);
        bool title_keys_valid = true;
        for (std::size_t title_index = 0U;
             title_index < group.title_keys.size(); ++title_index) {
            lua_rawgeti(state, title_keys_index,
                        static_cast<lua_Integer>(title_index + 1U));
            title_keys_valid = read_string(state, -1,
                                           group.title_keys[title_index]) &&
                               valid_identifier(
                                   group.title_keys[title_index]);
            lua_pop(state, 1);
            if (!title_keys_valid) {
                break;
            }
        }
        lua_pop(state, 1);
        if (!title_keys_valid) {
            return fail("shop offer group contains an invalid title key");
        }
        lua_getfield(state, group_table, "offers");
        if (!lua_istable(state, -1)) {
            return fail("shop offer group must contain an offers array");
        }
        const auto offers_index = lua_absindex(state, -1);
        const auto offer_count = lua_rawlen(state, offers_index);
        if (offer_count > kMaximumShopOffers) {
            return fail("shop offer group exceeds the eight-slot panel");
        }
        group.offers.reserve(offer_count);
        for (std::size_t offer_index = 1U; offer_index <= offer_count;
             ++offer_index) {
            lua_rawgeti(state, offers_index,
                        static_cast<lua_Integer>(offer_index));
            if (!lua_istable(state, -1)) {
                return fail("shop offer is not a record table");
            }
            const auto offer_table = lua_absindex(state, -1);
            std::uint32_t type = 0U;
            std::uint32_t value = 0U;
            std::uint32_t quantity = 0U;
            const bool valid =
                read_field_integer(state, offer_table, "type", 0xFFU, type) &&
                read_field_integer(state, offer_table, "value", 0xFFU, value) &&
                read_field_integer(state, offer_table, "quantity", 0xFFU,
                                   quantity);
            lua_pop(state, 1);
            if (!valid) {
                return fail("shop offer has invalid type, value, or quantity");
            }
            group.offers.push_back(ScriptShopOffer{
                static_cast<std::uint8_t>(type),
                static_cast<std::uint8_t>(value),
                static_cast<std::uint8_t>(quantity)});
        }
        lua_pop(state, 1);

        lua_getfield(state, group_table, "progression_offers");
        if (!lua_istable(state, -1)) {
            return fail(
                "shop offer group must contain a progression_offers array");
        }
        const auto progression_offers_index = lua_absindex(state, -1);
        const auto progression_offer_count =
            lua_rawlen(state, progression_offers_index);
        if (progression_offer_count > kMaximumShopOffers) {
            return fail(
                "shop offer group exceeds the eight-slot progression panel");
        }
        group.progression_offers.reserve(progression_offer_count);
        for (std::size_t offer_index = 1U;
             offer_index <= progression_offer_count; ++offer_index) {
            lua_rawgeti(state, progression_offers_index,
                        static_cast<lua_Integer>(offer_index));
            if (!lua_istable(state, -1)) {
                return fail("progression offer is not a record table");
            }
            const auto offer_table = lua_absindex(state, -1);
            std::uint32_t action_code = 0U;
            std::uint32_t source_metadata = 0U;
            std::uint32_t base_cost = 0U;
            const bool valid =
                read_field_integer(state, offer_table, "action_code", 0xFFU,
                                   action_code) &&
                read_field_integer(state, offer_table, "source_metadata",
                                   0xFFU, source_metadata) &&
                read_field_integer(state, offer_table, "base_cost", 0xFFFFU,
                                   base_cost);
            lua_pop(state, 1);
            if (!valid) {
                return fail("progression offer has invalid source fields");
            }
            group.progression_offers.push_back(ScriptShopProgressionOffer{
                static_cast<std::uint8_t>(action_code),
                static_cast<std::uint8_t>(source_metadata),
                static_cast<std::uint16_t>(base_cost)});
        }
        lua_pop(state, 1);

        lua_getfield(state, group_table, "equipment_upgrade_offers");
        if (!lua_istable(state, -1)) {
            return fail(
                "shop offer group must contain an equipment_upgrade_offers array");
        }
        const auto upgrade_offers_index = lua_absindex(state, -1);
        const auto upgrade_offer_count = lua_rawlen(state, upgrade_offers_index);
        if (upgrade_offer_count > kMaximumShopOffers) {
            return fail(
                "shop offer group exceeds the eight-slot equipment upgrade panel");
        }
        group.equipment_upgrade_offers.reserve(upgrade_offer_count);
        for (std::size_t offer_index = 1U;
             offer_index <= upgrade_offer_count; ++offer_index) {
            lua_rawgeti(state, upgrade_offers_index,
                        static_cast<lua_Integer>(offer_index));
            if (!lua_istable(state, -1)) {
                return fail("equipment upgrade offer is not a record table");
            }
            const auto offer_table = lua_absindex(state, -1);
            std::uint32_t category = 0U;
            std::uint32_t base_cost = 0U;
            std::string label_key;
            std::string selected_name_key;
            std::string description_key;
            const bool valid =
                read_field_integer(state, offer_table, "category", 7U,
                                   category) &&
                read_field_integer(state, offer_table, "base_cost",
                                   0xFFFFFFFFU, base_cost) &&
                read_field_string(state, offer_table, "label_key", label_key) &&
                valid_identifier(label_key) &&
                read_field_string(state, offer_table, "selected_name_key",
                                  selected_name_key) &&
                valid_identifier(selected_name_key) &&
                read_field_string(state, offer_table, "description_key",
                                  description_key) &&
                valid_identifier(description_key);
            lua_pop(state, 1);
            if (!valid) {
                return fail("equipment upgrade offer has invalid source fields");
            }
            group.equipment_upgrade_offers.push_back(
                ScriptShopEquipmentUpgradeOffer{
                    static_cast<std::uint8_t>(category), base_cost,
                    std::move(label_key), std::move(selected_name_key),
                    std::move(description_key)});
        }
        lua_pop(state, 1);
        lua_pop(state, 1);
        definition.groups.push_back(std::move(group));
    }

    if (definition.id == "wire_masters" &&
        (definition.menu_mode != 7U || definition.menu_result != 1 ||
         definition.equipment_upgrade_meter_thresholds.size() != 16U ||
         definition.groups.size() != 1U ||
         definition.groups.front().equipment_upgrade_offers.size() != 8U)) {
        return fail("Wire-Masters has an invalid source upgrade shape");
    }
    if (!definition.equipment_upgrade_meter_thresholds.empty() &&
        (definition.menu_result != 1 ||
         definition.equipment_upgrade_meter_thresholds.size() != 16U)) {
        return fail("equipment upgrade thresholds have an invalid menu shape");
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return definition;
}

std::optional<ScriptShopMenuSnapshot> GameScriptRuntime::open_shop_menu(
    const std::string_view shop_id) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for shop menus";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptShopMenuSnapshot> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!valid_identifier(shop_id) || !impl_->push_module("shop")) {
        return fail("shop.lua module or shop identifier is invalid");
    }
    lua_getfield(state, -1, "open_menu");
    if (!lua_isfunction(state, -1)) {
        return fail("shop.lua menu opener is missing");
    }
    lua_remove(state, -2);
    lua_pushlstring(state, shop_id.data(), shop_id.size());
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("shop.lua menu opener failed: " + stack_error(state));
    }
    ScriptShopMenuSnapshot snapshot;
    if (!lua_istable(state, -1) ||
        !read_shop_menu_snapshot(state, -1, snapshot) || !snapshot.open ||
        snapshot.shop_id != shop_id || snapshot.event != "opened") {
        return fail("shop.lua menu opener returned invalid state");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return snapshot;
}

std::optional<ScriptShopMenuSnapshot> GameScriptRuntime::advance_shop_menu(
    const std::uint16_t controller_events,
    const bool transaction_active) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for shop menus";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptShopMenuSnapshot> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("shop")) {
        return fail("shop.lua module is unavailable for menu input");
    }
    lua_getfield(state, -1, "advance_menu");
    if (!lua_isfunction(state, -1)) {
        return fail("shop.lua menu input handler is missing");
    }
    lua_remove(state, -2);
    lua_pushinteger(state, controller_events);
    if (impl_->menu_route_bindings.game.menu_b_policy) {
        const auto policy = impl_->menu_route_bindings.game.menu_b_policy();
        lua_pushlstring(state, policy.data(), policy.size());
    } else {
        lua_pushnil(state);
    }
    lua_pushboolean(state, transaction_active);
    if (lua_pcall(state, 3, 1, 0) != LUA_OK) {
        return fail("shop.lua menu input handler failed: " +
                    stack_error(state));
    }
    ScriptShopMenuSnapshot snapshot;
    if (!lua_istable(state, -1) ||
        !read_shop_menu_snapshot(state, -1, snapshot)) {
        return fail("shop.lua menu input handler returned invalid state");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return snapshot;
}

std::optional<ScriptShopMenuSnapshot> GameScriptRuntime::reset_shop_menu() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for shop menus";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptShopMenuSnapshot> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("shop")) {
        return fail("shop.lua module is unavailable for menu reset");
    }
    lua_getfield(state, -1, "reset_menu");
    if (!lua_isfunction(state, -1)) {
        return fail("shop.lua menu reset is missing");
    }
    lua_remove(state, -2);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return fail("shop.lua menu reset failed: " + stack_error(state));
    }
    ScriptShopMenuSnapshot snapshot;
    if (!lua_istable(state, -1) ||
        !read_shop_menu_snapshot(state, -1, snapshot) || !snapshot.open) {
        return fail("shop.lua menu reset returned invalid state");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return snapshot;
}

bool GameScriptRuntime::close_shop_menu() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for shop menus";
        return false;
    }
    const auto stack_base = lua_gettop(state);
    if (!impl_->push_module("shop")) {
        impl_->failure = "shop.lua module is unavailable for menu close";
        return false;
    }
    lua_getfield(state, -1, "close_menu");
    if (!lua_isfunction(state, -1)) {
        lua_settop(state, stack_base);
        impl_->failure = "shop.lua menu close is missing";
        return false;
    }
    lua_remove(state, -2);
    if (lua_pcall(state, 0, 0, 0) != LUA_OK) {
        const auto message = stack_error(state);
        lua_settop(state, stack_base);
        impl_->failure = "shop.lua menu close failed: " + message;
        return false;
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return true;
}

std::optional<ScriptItemCatalog> GameScriptRuntime::item_catalog() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for item data";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptItemCatalog> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("items")) {
        return fail("items.lua module is unavailable");
    }
    const auto module_index = lua_absindex(state, -1);
    ScriptItemCatalog catalog;

    lua_getfield(state, module_index, "catalog");
    if (!lua_istable(state, -1)) {
        return fail("items.catalog must be an array");
    }
    const auto items_index = lua_absindex(state, -1);
    const auto item_count = lua_rawlen(state, items_index);
    if (item_count != kMaximumItemCatalogEntries) {
        return fail("items.catalog does not match the 50 source item IDs");
    }
    catalog.items.reserve(item_count);
    std::set<std::uint8_t> item_codes;
    std::set<std::string> item_ids;
    std::set<std::uint8_t> armor_codes;
    for (std::size_t index = 1U; index <= item_count; ++index) {
        lua_rawgeti(state, items_index, static_cast<lua_Integer>(index));
        if (!lua_istable(state, -1)) {
            return fail("items.catalog contains a non-record value");
        }
        const auto item_index = lua_absindex(state, -1);
        ScriptItemDefinition item;
        const bool base_fields_valid =
            read_field_string(state, item_index, "id", item.id) &&
            valid_identifier(item.id) &&
            read_field_string(state, item_index, "kind", item.kind) &&
            valid_identifier(item.kind) &&
            read_field_string(state, item_index, "name_key", item.name_key) &&
            valid_identifier(item.name_key) &&
            read_field_string(state, item_index, "category_key",
                              item.category_key) &&
            valid_identifier(item.category_key) &&
            read_field_boolean(state, item_index, "has_detail_art",
                               item.has_detail_art);
        if (!base_fields_valid) {
            return fail("items.catalog contains an incomplete item record");
        }
        const auto source_item_code = script_item_source_code_from_id(item.id);
        if (!source_item_code.has_value()) {
            return fail("items.catalog contains an unknown semantic item ID");
        }
        item.code = *source_item_code;

        lua_getfield(state, item_index, "target_motion_range");
        if (!lua_istable(state, -1)) {
            return fail("item target motion range must be a distance interval");
        }
        const auto target_motion_range_index = lua_absindex(state, -1);
        std::uint32_t minimum_distance = 0U;
        std::uint32_t maximum_distance = 0U;
        const bool target_motion_range_valid =
            read_field_integer(state, target_motion_range_index,
                               "minimum_distance", 65535U,
                               minimum_distance) &&
            read_field_integer(state, target_motion_range_index,
                               "maximum_distance", 65535U,
                               maximum_distance) &&
            minimum_distance <= maximum_distance;
        lua_pop(state, 1);
        if (!target_motion_range_valid) {
            return fail("item target motion range is invalid");
        }
        item.target_motion_range = ScriptTargetMotionRange{
            static_cast<std::uint16_t>(minimum_distance),
            static_cast<std::uint16_t>(maximum_distance)};

        lua_getfield(state, item_index, "pricing");
        if (lua_istable(state, -1)) {
            const auto pricing_index = lua_absindex(state, -1);
            std::uint32_t purchase_base = 0U;
            std::uint32_t resale_base = 0U;
            if (!read_field_integer(state, pricing_index, "purchase_base",
                                    65535U, purchase_base) ||
                !read_field_integer(state, pricing_index, "resale_base",
                                    65535U, resale_base) ||
                resale_base != purchase_base / 2U) {
                return fail("item pricing is incomplete");
            }
            item.pricing = ScriptItemDefinition::Pricing{
                static_cast<std::uint16_t>(purchase_base),
                static_cast<std::uint16_t>(resale_base)};
        } else if (item.kind != "totem") {
            return fail("item pricing must be a record");
        }
        lua_pop(state, 1);

        lua_getfield(state, item_index, "presentation");
        if (!lua_istable(state, -1)) {
            return fail("item presentation must be a record");
        }
        const auto presentation_index = lua_absindex(state, -1);
        if (!read_field_string(state, presentation_index,
                               "icon_resource_key",
                               item.presentation.icon_resource_key) ||
            !valid_identifier(item.presentation.icon_resource_key) ||
            !read_field_string(state, presentation_index,
                               "detail_resource_key",
                               item.presentation.detail_resource_key) ||
            !valid_identifier(item.presentation.detail_resource_key)) {
            return fail("item presentation is incomplete");
        }
        lua_pop(state, 1);

        lua_getfield(state, item_index, "detail");
        if (!lua_isnil(state, -1)) {
            if (!lua_istable(state, -1)) {
                return fail("item detail must be a record");
            }
            const auto detail_index = lua_absindex(state, -1);
            std::uint32_t value = 0U;
            ScriptItemDefinition::Detail detail;
            if (!read_field_string(state, detail_index, "kind", detail.kind) ||
                !valid_identifier(detail.kind) || detail.kind != "uses" ||
                !read_field_integer(state, detail_index, "value", 255U,
                                    value)) {
                return fail("item detail has an unsupported kind or value");
            }
            detail.value = static_cast<std::uint8_t>(value);
            item.detail = std::move(detail);
        }
        lua_pop(state, 1);

        lua_getfield(state, item_index, "weapon");
        if (!lua_isnil(state, -1)) {
            if (!lua_istable(state, -1)) {
                return fail("item weapon template must be a record");
            }
            const auto weapon_index = lua_absindex(state, -1);
            std::uint32_t magazine_capacity = 0U;
            std::uint32_t damage_effect = 0U;
            std::uint32_t rounds_per_action = 0U;
            std::uint32_t source_effect_value = 0U;
            std::uint32_t source_difficulty = 0U;
            ScriptWeaponTemplate weapon;
            weapon.id = item.id;
            weapon.item_code = item.code;
            if (!read_field_integer(state, weapon_index, "magazine_capacity",
                                    255U, magazine_capacity) ||
                magazine_capacity == 0U ||
                !read_field_integer(state, weapon_index, "damage_effect",
                                    10U, damage_effect) ||
                damage_effect == 0U ||
                !read_field_string(state, weapon_index, "power_profile",
                                   weapon.power_profile) ||
                !valid_identifier(weapon.power_profile) ||
                !read_field_string(state, weapon_index, "fire_mode",
                                   weapon.fire_mode) ||
                !valid_identifier(weapon.fire_mode) ||
                !read_field_string(state, weapon_index, "trigger_behavior",
                                   weapon.trigger_behavior) ||
                weapon.trigger_behavior != "repeat_while_held" ||
                !read_field_integer(state, weapon_index, "rounds_per_action",
                                    3U, rounds_per_action) ||
                rounds_per_action == 0U ||
                !read_field_string(state, weapon_index, "attack_cadence",
                                   weapon.attack_cadence) ||
                weapon.attack_cadence != "runner_attribute" ||
                !read_field_string(state, weapon_index,
                                   "attack_delay_scale",
                                   weapon.attack_delay_scale) ||
                (weapon.attack_delay_scale != "half" &&
                 weapon.attack_delay_scale != "normal") ||
                !read_field_string(state, weapon_index,
                                   "reload_delay_scale",
                                   weapon.reload_delay_scale) ||
                (weapon.reload_delay_scale != "normal" &&
                 weapon.reload_delay_scale != "double") ||
                (weapon.fire_mode == "single_round" &&
                 rounds_per_action != 1U) ||
                (weapon.fire_mode == "three_round_burst" &&
                 rounds_per_action != 3U) ||
                (weapon.fire_mode != "single_round" &&
                 weapon.fire_mode != "three_round_burst")) {
                return fail("item weapon template is incomplete");
            }
            weapon.magazine_capacity =
                static_cast<std::uint8_t>(magazine_capacity);
            weapon.damage_effect =
                static_cast<std::uint8_t>(damage_effect);
            weapon.rounds_per_action =
                static_cast<std::uint8_t>(rounds_per_action);
            lua_getfield(state, weapon_index, "source_action");
            if (!lua_istable(state, -1)) {
                lua_pop(state, 1);
                return fail("item weapon source action is missing");
            }
            const auto source_action_index = lua_absindex(state, -1);
            const bool source_action_valid =
                read_field_integer(state, source_action_index,
                                   "effect_value", 10U,
                                   source_effect_value) &&
                source_effect_value != 0U &&
                read_field_integer(state, source_action_index,
                                   "difficulty", 255U,
                                   source_difficulty) &&
                source_difficulty != 0U;
            lua_pop(state, 1);
            const auto expected_effect_index = item.code <= 4U
                ? 0U : item.code <= 10U ? 1U : 2U;
            const auto expected_effect_value = expected_effect_index == 0U
                ? 1U : expected_effect_index == 1U ? 3U : 6U;
            if (!source_action_valid ||
                damage_effect != expected_effect_value ||
                source_effect_value != damage_effect) {
                return fail("item weapon attack profile disagrees with D4540/166AD4");
            }
            weapon.damage_bar_index =
                static_cast<std::uint8_t>(expected_effect_index);
            weapon.source_action = ScriptWeaponTemplate::SourceAction{
                static_cast<std::uint8_t>(expected_effect_index),
                static_cast<std::uint8_t>(source_effect_value),
                static_cast<std::uint8_t>(source_difficulty)};
            item.weapon = std::move(weapon);
        }
        lua_pop(state, 1);

        lua_getfield(state, item_index, "armor");
        if (!lua_isnil(state, -1)) {
            if (!lua_istable(state, -1)) {
                return fail("item armor template must be a record");
            }
            const auto armor_index = lua_absindex(state, -1);
            std::uint32_t primary_modifier = 0U;
            std::uint32_t secondary_modifier = 0U;
            ScriptArmorTemplate armor;
            armor.item_id = item.id;
            armor.item_code = item.code;
            if (!read_field_integer(state, armor_index, "primary_modifier",
                                    255U, primary_modifier) ||
                !read_field_integer(state, armor_index, "secondary_modifier",
                                    255U, secondary_modifier)) {
                return fail("item armor template is incomplete");
            }
            armor.primary_modifier =
                static_cast<std::uint8_t>(primary_modifier);
            armor.secondary_modifier =
                static_cast<std::uint8_t>(secondary_modifier);
            item.armor = std::move(armor);
        }
        lua_pop(state, 1);
        lua_pop(state, 1);

        const bool firearm = item.kind == "firearm";
        const bool armor_item = item.kind == "armor";
        // ROM 0x12A8C reads live actor detail for the extended item-action
        // bands. Static Lua definitions must not freeze those counts here.
        const bool content_shape_valid =
            (item.code <= 12U) == firearm &&
            (item.code >= 44U && item.code <= 50U) == armor_item &&
            !item.detail.has_value() &&
            (firearm == item.weapon.has_value()) &&
            (armor_item == item.armor.has_value()) &&
            (!item.weapon.has_value() || !item.armor.has_value());
        if (!content_shape_valid ||
            !item_codes.insert(item.code).second ||
            !item_ids.insert(item.id).second ||
            (item.armor.has_value() &&
             !armor_codes.insert(item.code).second)) {
            return fail("items.catalog contains duplicate or mismatched item data");
        }
        catalog.items.push_back(std::move(item));
    }
    lua_pop(state, 1);

    lua_getfield(state, module_index, "detail_text_keys");
    if (!lua_istable(state, -1) ||
        lua_rawlen(state, -1) != kMaximumItemDetailTextEntries) {
        return fail("items.detail_text_keys must contain 32 source entries");
    }
    const auto details_index = lua_absindex(state, -1);
    catalog.detail_text_keys.reserve(kMaximumItemDetailTextEntries);
    for (std::size_t index = 1U;
         index <= kMaximumItemDetailTextEntries; ++index) {
        lua_rawgeti(state, details_index, static_cast<lua_Integer>(index));
        std::string key;
        const bool valid = read_string(state, -1, key) &&
                           valid_identifier(key);
        lua_pop(state, 1);
        if (!valid) {
            return fail("items.detail_text_keys contains an invalid key");
        }
        catalog.detail_text_keys.push_back(std::move(key));
    }
    lua_pop(state, 1);

    lua_getfield(state, module_index, "detail_label_keys");
    if (!lua_istable(state, -1) ||
        lua_rawlen(state, -1) != catalog.detail_label_keys.size()) {
        return fail("items.detail_label_keys must contain three source labels");
    }
    const auto labels_index = lua_absindex(state, -1);
    for (std::size_t index = 0; index < catalog.detail_label_keys.size();
         ++index) {
        lua_rawgeti(state, labels_index, static_cast<lua_Integer>(index + 1U));
        const bool valid = read_string(state, -1,
                                       catalog.detail_label_keys[index]) &&
                           valid_identifier(catalog.detail_label_keys[index]);
        lua_pop(state, 1);
        if (!valid) {
            return fail("items.detail_label_keys contains an invalid key");
        }
    }
    lua_pop(state, 1);
    lua_pop(state, 1);

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return catalog;
}

void GameScriptRuntime::set_story_state(const StoryFacts& facts) {
    impl_->story_facts.clear();
    for (const auto& [name, value] : facts) {
        if (!valid_identifier(name) ||
            impl_->story_facts.size() >= kMaximumStoryFacts) {
            continue;
        }
        impl_->story_facts.emplace(name, value);
    }
}

void GameScriptRuntime::set_story_fact_accessors(
    StoryFactReader read, StoryFactWriter write) {
    impl_->story_fact_reader = std::move(read);
    impl_->story_fact_writer = std::move(write);
}

StoryFacts GameScriptRuntime::story_state() const {
    return impl_->story_facts;
}

void GameScriptRuntime::set_location_context(
    const std::string_view type, const std::string_view id,
    const std::string_view text_profile,
    const std::string_view speaker_id,
    const std::string_view event_id,
    const std::optional<std::uint8_t> scene_id) {
    if (impl_->state == nullptr || !valid_identifier(type) ||
        !valid_identifier(id) ||
        (!text_profile.empty() && !valid_identifier(text_profile)) ||
        (!speaker_id.empty() && !valid_identifier(speaker_id)) ||
        (!event_id.empty() && !valid_identifier(event_id))) {
        impl_->failure = "invalid gameplay location context";
        return;
    }
    impl_->location_type = std::string(type);
    impl_->location_id = std::string(id);
    impl_->location_text_profile = std::string(text_profile);
    impl_->location_speaker_id = std::string(speaker_id);
    impl_->location_event_id = std::string(event_id);
    impl_->location_scene_id = scene_id;
}

void GameScriptRuntime::set_field_mode_context(
    const ScriptFieldModeContext context) noexcept {
    impl_->field_mode_context = context;
}

void GameScriptRuntime::set_actor_heartbeat_observer(
    ScriptActorHeartbeatObserver observer) {
    impl_->actor_heartbeat_observer = std::move(observer);
}

std::optional<ScriptActorTargetListRebuildDecision>
GameScriptRuntime::should_rebuild_actor_target_lists(
    const ScriptActorTargetListRebuildFacts& facts) {
    auto* const state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "gameplay Lua actor targeting state is unavailable";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptActorTargetListRebuildDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("actor_ai")) {
        return fail("gameplay Lua actor AI module is missing");
    }
    lua_getfield(state, -1, "should_rebuild_target_lists");
    if (!lua_isfunction(state, -1)) {
        return fail("actor_ai.should_rebuild_target_lists must be a Lua function");
    }
    lua_remove(state, -2);
    lua_pushinteger(state, facts.frame_counter);
    lua_pushinteger(state, facts.display_service_word);
    if (lua_pcall(state, 2, 2, 0) != LUA_OK) {
        return fail("actor_ai.should_rebuild_target_lists: " +
                    stack_error(state));
    }
    if (!lua_isboolean(state, -2) || !lua_isboolean(state, -1)) {
        return fail(
            "actor_ai.should_rebuild_target_lists returned invalid decisions");
    }
    const ScriptActorTargetListRebuildDecision decision{
        lua_toboolean(state, -2) != 0,
        lua_toboolean(state, -1) != 0};
    lua_settop(state, stack_base);
    return decision;
}

std::optional<ScriptActorBehaviorDecision>
GameScriptRuntime::actor_heartbeat(
    const ScriptActorBehaviorContext& context) {
    auto* state = impl_->state;
    const auto actor_id = context.instance_id.empty()
        ? context.profile_id
        : context.instance_id;
    if (state == nullptr || !valid_identifier(context.profile_id) ||
        !valid_identifier(actor_id)) {
        impl_->failure = "gameplay Lua actor AI state is unavailable";
        return std::nullopt;
    }
    if ((context.action_kind == ScriptActorActionKind::none) !=
        (context.action_code == 0U)) {
        impl_->failure = "gameplay Lua actor action source is inconsistent";
        return std::nullopt;
    }
    const auto action_item_id = context.action_kind ==
            ScriptActorActionKind::item
        ? script_item_id_from_source_code(context.action_code)
        : std::nullopt;
    if (context.action_kind == ScriptActorActionKind::item &&
        !action_item_id.has_value()) {
        impl_->failure = "gameplay Lua actor item identity is unavailable";
        return std::nullopt;
    }
    const auto action_spell_id = context.action_kind ==
            ScriptActorActionKind::spell
        ? script_spell_id_from_action_code(context.action_code)
        : std::nullopt;
    if (context.action_kind == ScriptActorActionKind::spell &&
        !action_spell_id.has_value()) {
        impl_->failure = "gameplay Lua actor spell identity is unavailable";
        return std::nullopt;
    }
    const auto player_spell_id = context.player_spell_available
        ? script_spell_id_from_action_code(context.player_spell_code)
        : std::nullopt;
    if (context.player_spell_available && !player_spell_id.has_value()) {
        impl_->failure = "gameplay Lua player spell identity is unavailable";
        return std::nullopt;
    }
    if (context.projectile_movement.has_value() &&
        (context.profile_id != "weapon_projectile" ||
         context.phase != ScriptActorHeartbeatPhase::behavior ||
         !context.projectile_terrain_probe ||
         !context.projectile_collision_search)) {
        impl_->failure =
            "gameplay Lua projectile behavior services are unavailable";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptActorBehaviorDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("actor_ai")) {
        return fail("gameplay Lua actor AI module is missing");
    }
    lua_getfield(state, -1, "heartbeat");
    if (!lua_isfunction(state, -1)) {
        return fail("actor_ai.heartbeat must be a Lua function");
    }
    lua_remove(state, -2);
    std::string_view action_kind;
    switch (context.action_kind) {
    case ScriptActorActionKind::none:
        action_kind = "none";
        break;
    case ScriptActorActionKind::item:
        action_kind = "item";
        break;
    case ScriptActorActionKind::spell:
        action_kind = "spell";
        break;
    default:
        return fail("gameplay Lua actor action kind is invalid");
    }
    const std::string_view heartbeat_phase =
        context.phase == ScriptActorHeartbeatPhase::behavior
            ? "behavior"
            : "render";
    std::string_view animation_mode;
    switch (context.animation_mode) {
    case ScriptActorAnimationMode::idle:
        animation_mode = "idle";
        break;
    case ScriptActorAnimationMode::walking:
        animation_mode = "walking";
        break;
    case ScriptActorAnimationMode::attack:
        animation_mode = "attack";
        break;
    case ScriptActorAnimationMode::strike:
        animation_mode = "strike";
        break;
    case ScriptActorAnimationMode::cast:
        animation_mode = "cast";
        break;
    case ScriptActorAnimationMode::runner_down:
        animation_mode = "runner_down";
        break;
    case ScriptActorAnimationMode::runner_hurt:
        animation_mode = "runner_hurt";
        break;
    default:
        return fail("gameplay Lua actor animation mode is invalid");
    }
    lua_createtable(state, 0, 53);
    lua_pushlstring(state, actor_id.data(), actor_id.size());
    lua_setfield(state, -2, "actor_id");
    lua_pushlstring(state, context.profile_id.data(),
                    context.profile_id.size());
    lua_setfield(state, -2, "profile_id");
    const auto visual_profile_id = context.profile_id == "weapon_projectile"
        ? std::optional<std::string_view>{"weapon_projectile"}
        : script_actor_visual_profile_id_from_source_type(
              context.entity_kind);
    if (visual_profile_id.has_value()) {
        lua_pushlstring(state, visual_profile_id->data(),
                        visual_profile_id->size());
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "visual_profile_id");
    lua_pushboolean(state, context.source_actor_has_waypoint_data);
    lua_setfield(state, -2, "source_actor_has_waypoint_data");
    lua_pushlstring(state, action_kind.data(), action_kind.size());
    lua_setfield(state, -2, "action_kind");
    if (action_item_id.has_value()) {
        lua_pushlstring(state, action_item_id->data(),
                        action_item_id->size());
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "action_item_id");
    if (action_spell_id.has_value()) {
        lua_pushlstring(state, action_spell_id->data(),
                        action_spell_id->size());
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "action_spell_id");
    const auto profile_separator = context.profile_id.rfind('_');
    const auto profile_suffix = profile_separator == std::string_view::npos
        ? std::string_view{}
        : context.profile_id.substr(profile_separator + 1U);
    const bool has_profile_prefix =
        profile_suffix == "none00" ||
        (profile_suffix.size() == 6U &&
         profile_suffix.substr(0U, 4U) == "item") ||
        (profile_suffix.size() == 7U &&
         profile_suffix.substr(0U, 5U) == "spell");
    if (has_profile_prefix) {
        lua_pushlstring(state, context.profile_id.data(),
                        profile_separator);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "profile_prefix");
    if (context.projectile_action.has_value()) {
        const auto action_name =
            script_weapon_action_name(*context.projectile_action);
        lua_pushlstring(state, action_name.data(), action_name.size());
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "projectile_action_id");
    lua_pushinteger(state, context.entry_delay);
    lua_setfield(state, -2, "entry_delay");
    lua_pushboolean(state, context.entry_started);
    lua_setfield(state, -2, "entry_started");
    lua_pushboolean(state, context.entry_complete);
    lua_setfield(state, -2, "entry_complete");
    lua_pushnumber(state, bearing_degrees_from_source(context.initial_heading));
    lua_setfield(state, -2, "initial_bearing_degrees");
    lua_pushboolean(state, context.spawn);
    lua_setfield(state, -2, "spawn");
    lua_pushboolean(state, context.movement_allowed);
    lua_setfield(state, -2, "movement_allowed");
    lua_pushboolean(state, context.combat_confirm_held);
    lua_setfield(state, -2, "combat_confirm_held");
    lua_pushboolean(state, context.combat_action_ready);
    lua_setfield(state, -2, "combat_action_ready");
    lua_pushboolean(state, context.combat_action_available);
    lua_setfield(state, -2, "combat_action_available");
    lua_pushboolean(state, context.player_weapon_action_will_fire);
    lua_setfield(state, -2, "player_weapon_action_will_fire");
    if (context.controlled_party_member_id.empty()) {
        lua_pushnil(state);
    } else {
        lua_pushlstring(state, context.controlled_party_member_id.data(),
                        context.controlled_party_member_id.size());
    }
    lua_setfield(state, -2, "controlled_party_member_id");
    lua_pushboolean(state, context.is_controlled_party_actor);
    lua_setfield(state, -2, "is_controlled_party_actor");
    lua_pushboolean(state, context.player_spell_available);
    lua_setfield(state, -2, "player_spell_available");
    if (player_spell_id.has_value()) {
        lua_pushlstring(state, player_spell_id->data(),
                        player_spell_id->size());
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "player_spell_id");
    lua_pushboolean(state, context.player_melee_available);
    lua_setfield(state, -2, "player_melee_available");
    lua_pushboolean(state, context.combat_input_blocked);
    lua_setfield(state, -2, "combat_input_blocked");
    if (context.movement_input_direction.has_value()) {
        lua_pushstring(
            state, movement_direction_id(*context.movement_input_direction));
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "movement_input_direction");
    if (context.movement_direction_override.has_value()) {
        lua_pushstring(
            state, movement_direction_id(*context.movement_direction_override));
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "movement_direction_override");
    lua_pushboolean(state, context.dynamic_world_encounter_actor);
    lua_setfield(state, -2, "dynamic_world_encounter_actor");
    lua_pushboolean(state, context.source_default_item_action_profile);
    lua_setfield(state, -2, "source_default_item_action_profile");
    lua_pushboolean(state, context.world_activity_script_actor);
    lua_setfield(state, -2, "world_activity_script_actor");
    lua_pushboolean(state, context.has_live_actor_record ||
                               context.live_field_actor ||
                               context.source_record_slot.has_value());
    lua_setfield(state, -2, "has_live_actor_record");
    push_formation_position(state, "formation_position",
                            context.formation_position);
    lua_pushboolean(state, context.live_field_actor ||
                               context.source_actor_slot.has_value());
    lua_setfield(state, -2, "live_field_actor");
    if (context.world_position.has_value()) {
        lua_createtable(state, 0, 2);
        lua_pushinteger(state, context.world_position->x);
        lua_setfield(state, -2, "x");
        lua_pushinteger(state, context.world_position->y);
        lua_setfield(state, -2, "y");
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "world_position");
    push_actor_facts(state, context.actor_facts);
    lua_setfield(state, -2, "actor_facts");
    if (context.physical_condition.has_value()) {
        lua_pushinteger(state, *context.physical_condition);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "physical_condition");
    if (context.actor_behavior_profile.has_value()) {
        const auto& profile = *context.actor_behavior_profile;
        lua_createtable(state, 0, 2);
        lua_pushstring(
            state,
            actor_engagement_policy_id(profile.engagement_policy));
        lua_setfield(state, -2, "engagement_policy");
        if (profile.approach_band.has_value()) {
            lua_pushstring(
                state,
                actor_approach_band_id(*profile.approach_band));
        } else {
            lua_pushnil(state);
        }
        lua_setfield(state, -2, "approach_band");
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "actor_behavior_profile");
    lua_pushinteger(state, context.ammunition_remaining);
    lua_setfield(state, -2, "ammunition_remaining");
    lua_pushboolean(state, context.hold_for_ending_activity);
    lua_setfield(state, -2, "hold_for_ending_activity");
    lua_pushboolean(state, context.animation_blocks_behavior);
    lua_setfield(state, -2, "animation_blocks_behavior");
    lua_pushboolean(state, context.can_start_item_action);
    lua_setfield(state, -2, "can_start_item_action");
    if (context.source_item_spawn_result.has_value()) {
        lua_pushstring(
            state,
            *context.source_item_spawn_result ==
                    ScriptActorItemSpawnResult::spawned
                ? "spawned"
                : "blocked");
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "source_item_spawn_result");
    if (context.source_think_suppressed.has_value()) {
        lua_pushboolean(state, *context.source_think_suppressed ? 1 : 0);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "source_think_suppressed");
    if (context.thon_action_cooldown_remaining.has_value()) {
        lua_pushinteger(state, *context.thon_action_cooldown_remaining);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "thon_action_cooldown_remaining");
    if (context.source_alarm_active.has_value()) {
        lua_pushboolean(state, *context.source_alarm_active ? 1 : 0);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "source_alarm_active");
    const auto push_optional_actor_integer = [state](
        const char* field, const std::optional<std::uint16_t>& value) {
        if (value.has_value()) {
            lua_pushinteger(state, *value);
        } else {
            lua_pushnil(state);
        }
        lua_setfield(state, -2, field);
    };
    if (context.scene_mode.has_value()) {
        lua_pushstring(state, field_scene_mode_id(*context.scene_mode));
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "scene_mode");
    push_optional_actor_integer("source_camera_x", context.source_camera_x);
    push_optional_actor_integer("source_camera_y", context.source_camera_y);
    push_optional_actor_integer("source_camera_limit_x",
                                context.source_camera_limit_x);
    push_optional_actor_integer("source_camera_limit_y",
                                context.source_camera_limit_y);
    lua_pushboolean(state, context.world_activity_active);
    lua_setfield(state, -2, "world_activity_active");
    // FFE186 stays inside the native field host. Lua receives the behavior
    // facts used by ROM 0x13996/0x13FFE and 0xAE6E instead of its state byte.
    lua_pushboolean(state, context.world_scene_state != 0U);
    lua_setfield(state, -2, "field_sequence_active");
    lua_pushboolean(state, context.world_scene_state == 3U);
    lua_setfield(state, -2, "final_battle_active");
    lua_pushinteger(state, context.ai_animation_frame);
    lua_setfield(state, -2, "ai_animation_frame");
    lua_pushboolean(state, context.target_heading_audio_alternate);
    lua_setfield(state, -2, "target_heading_audio_alternate");
    lua_pushinteger(state, context.initial_action_delay);
    lua_setfield(state, -2, "initial_action_delay");
    lua_createtable(state, 0, 3);
    lua_pushinteger(state, context.action_timing.quickness);
    lua_setfield(state, -2, "quickness");
    lua_pushinteger(state, context.action_timing.intelligence);
    lua_setfield(state, -2, "intelligence");
    lua_pushinteger(state, context.action_timing.action_delay_modifier);
    lua_setfield(state, -2, "action_delay_modifier");
    lua_setfield(state, -2, "action_timing");
    lua_createtable(state, static_cast<int>(
                               context.world_effect_slot_occupancy.size()), 0);
    for (std::size_t index = 0U;
         index < context.world_effect_slot_occupancy.size(); ++index) {
        lua_pushboolean(state, context.world_effect_slot_occupancy[index]);
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, -2, "world_effect_slot_occupancy");
    bool semantic_inventory_valid = true;
    const auto push_combatant = [state, &semantic_inventory_valid, &context](
        const ScriptCombatantFacts& combatant) {
        lua_createtable(state, 0, 22);
        const auto table_index = lua_absindex(state, -1);
        const auto set_integer = [state, table_index](
                                     const char* field,
                                     const lua_Integer value) {
            lua_pushinteger(state, value);
            lua_setfield(state, table_index, field);
        };
        lua_pushboolean(state, combatant.record_type < 3U);
        lua_setfield(state, table_index, "is_party_member");
        lua_pushboolean(state, combatant.blocks_spell_death_pose);
        lua_setfield(state, table_index, "blocks_spell_death_pose");
        push_actor_facts(state, actor_facts_from_source_flags(
            combatant.source_flags, combatant.interaction_flags, 0U, 0U));
        lua_setfield(state, table_index, "actor_facts");
        lua_createtable(state, 0, 6);
        const auto cyberware_index = lua_absindex(state, -1);
        const auto set_cyberware_flag = [state, cyberware_index](
                                            const char* field,
                                            const bool enabled) {
            lua_pushboolean(state, enabled);
            lua_setfield(state, cyberware_index, field);
        };
        set_cyberware_flag("cybereyes", combatant.cyberware.cybereyes);
        set_cyberware_flag("hand_razors",
                           combatant.cyberware.hand_razors);
        set_cyberware_flag("spurs", combatant.cyberware.spurs);
        set_cyberware_flag("smartlink", combatant.cyberware.smartlink);
        lua_pushinteger(state,
                        combatant.cyberware.melee_strength_bonus);
        lua_setfield(state, cyberware_index, "melee_strength_bonus");
        lua_pushinteger(state,
                        combatant.cyberware.melee_defense_bonus);
        lua_setfield(state, cyberware_index, "melee_defense_bonus");
        lua_setfield(state, table_index, "cyberware");
        set_integer("physical_meter", combatant.physical_meter);
        set_integer("mental_meter", combatant.mental_meter);
        set_integer(
            "posture_reduction_percent",
            script_posture_reduction_percent_from_source_value(
                combatant.source_posture));
        lua_createtable(state, 0, 6);
        const auto attributes_index = lua_absindex(state, -1);
        const auto set_attribute = [state, attributes_index](
                                       const char* field,
                                       const std::uint8_t value) {
            lua_pushinteger(state, value);
            lua_setfield(state, attributes_index, field);
        };
        set_attribute("body", combatant.attributes.body);
        set_attribute("quickness", combatant.attributes.quickness);
        set_attribute("strength", combatant.attributes.strength);
        set_attribute("intelligence", combatant.attributes.intelligence);
        set_attribute("willpower", combatant.attributes.willpower);
        lua_setfield(state, table_index, "attributes");
        lua_createtable(state, 0, 6);
        const auto skills_index = lua_absindex(state, -1);
        const auto set_skill = [state, skills_index](
                                   const char* field,
                                   const std::uint8_t value) {
            lua_pushinteger(state, value);
            lua_setfield(state, skills_index, field);
        };
        set_skill("sorcery", combatant.skills.sorcery);
        set_skill("firearms", combatant.skills.firearms);
        set_skill("pistols", combatant.skills.pistols);
        set_skill("smgs", combatant.skills.smgs);
        set_skill("shotguns", combatant.skills.shotguns);
        set_skill("melee_combat", combatant.skills.melee_combat);
        lua_setfield(state, table_index, "skills");
        set_integer("force", combatant.force);
        set_integer("last_action_successes",
                    combatant.last_action_successes);
        set_integer("spell_trial_cap", combatant.spell_trial_cap);
        if (combatant.equipped_armor_id.empty()) {
            lua_pushnil(state);
        } else {
            lua_pushlstring(state, combatant.equipped_armor_id.data(),
                            combatant.equipped_armor_id.size());
        }
        lua_setfield(state, table_index, "equipped_armor_id");
        lua_createtable(state, 0, 6);
        const auto traits_index = lua_absindex(state, -1);
        const auto set_trait = [state, traits_index](const char* field,
                                                    const bool value) {
            lua_pushboolean(state, value);
            lua_setfield(state, traits_index, field);
        };
        set_trait("ignores_cover_condition_penalty",
                  combatant.traits.ignores_cover_condition_penalty);
        set_trait("adds_two_condition_difficulty",
                  combatant.traits.adds_two_condition_difficulty);
        set_trait("is_special_spell_target",
                  combatant.traits.is_special_spell_target);
        set_trait("takes_fixed_meter_damage",
                  combatant.traits.takes_fixed_meter_damage);
        set_trait("qualifies_for_party_support_bonus",
                  combatant.traits.qualifies_for_party_support_bonus);
        set_trait("is_ghoul_bounty_target",
                  combatant.traits.is_ghoul_bounty_target);
        lua_setfield(state, table_index, "combat_traits");
        const auto equipped_action_id = script_item_id_from_source_code(
            combatant.equipped_action_code);
        if (equipped_action_id.has_value()) {
            lua_pushlstring(state, equipped_action_id->data(),
                            equipped_action_id->size());
        } else {
            lua_pushnil(state);
        }
        lua_setfield(state, table_index, "equipped_action_id");
        lua_createtable(state, 0, 2);
        const auto profile_options_index = lua_absindex(state, -1);
        lua_pushboolean(state, combatant.action_profile_options.primary);
        lua_setfield(state, profile_options_index, "primary");
        lua_pushboolean(state, combatant.action_profile_options.secondary);
        lua_setfield(state, profile_options_index, "secondary");
        lua_setfield(state, table_index, "action_profile_options");
        // Resolve the source-linked profile on the host. Lua receives the
        // selected roll rules and attribute meaning, never the +0xA0 row index.
        const auto profile_index = static_cast<std::size_t>(
            combatant.linked_action_profile_index);
        if (profile_index < context.action_profile_modifiers.size()) {
            const auto& profile =
                context.action_profile_modifiers[profile_index];
            lua_createtable(state, 0, 2);
            const auto profiles_index = lua_absindex(state, -1);
            const auto push_profile = [state, profiles_index](
                                          const char* name,
                                          const std::uint8_t roll_count,
                                          const std::uint8_t difficulty,
                                          const std::string_view attribute) {
                if (attribute != "body" && attribute != "willpower") {
                    return;
                }
                lua_createtable(state, 0, 3);
                lua_pushinteger(state, roll_count);
                lua_setfield(state, -2, "roll_count");
                lua_pushinteger(state, difficulty);
                lua_setfield(state, -2, "difficulty");
                lua_pushlstring(state, attribute.data(), attribute.size());
                lua_setfield(state, -2, "attribute");
                lua_setfield(state, profiles_index, name);
            };
            push_profile("primary", profile.primary_roll_count,
                         profile.primary_difficulty,
                         context.action_profile_attributes[0]);
            push_profile("secondary", profile.secondary_roll_count,
                         profile.secondary_difficulty,
                         context.action_profile_attributes[1]);
            lua_setfield(state, table_index, "dynamic_action_profile");
        }
        if (!push_combat_inventory(state, combatant.item_codes,
                                   combatant.item_phases)) {
            semantic_inventory_valid = false;
            lua_pushnil(state);
        }
        lua_setfield(state, table_index, "inventory");
        lua_createtable(state, 0,
                        static_cast<int>(combatant.weapon_action_policies.size()));
        const auto policies_index = lua_absindex(state, -1);
        for (std::size_t index = 0U;
             index < combatant.weapon_action_policies.size(); ++index) {
            const auto action_id = kScriptWeaponActionIds[index];
            const auto& policy = combatant.weapon_action_policies[index];
            lua_createtable(state, 0, 3);
            const auto skill_group =
                script_weapon_skill_group_name(policy.skill_group);
            lua_pushlstring(state, skill_group.data(), skill_group.size());
            lua_setfield(state, -2, "skill_group");
            lua_pushinteger(state, policy.difficulty_adjustment);
            lua_setfield(state, -2, "difficulty_adjustment");
            lua_pushboolean(state, policy.benefits_from_targeting_assist);
            lua_setfield(state, -2, "benefits_from_targeting_assist");
            const auto action_name = script_weapon_action_name(action_id);
            lua_setfield(state, policies_index, action_name.data());
        }
        lua_setfield(state, table_index, "weapon_action_policies");
    };
    if (context.combatant.has_value()) {
        push_combatant(*context.combatant);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "combatant");
    lua_pushboolean(state, context.combat_meter_service_blocked);
    lua_setfield(state, -2, "combat_meter_service_blocked");
    lua_pushboolean(state, context.target_selection_latched);
    lua_setfield(state, -2, "target_selection_latched");
    lua_pushboolean(state, context.target_marker_reads_stun_meter);
    lua_setfield(state, -2, "target_marker_reads_stun_meter");
    lua_pushboolean(state, context.target_selection_requested);
    lua_setfield(state, -2, "target_selection_requested");
    lua_pushstring(state,
                   target_selection_policy_id(context.target_selection_policy));
    lua_setfield(state, -2, "target_selection_policy");
    const auto push_actor_candidates = [state, &push_combatant](
        const char* field,
        const std::vector<ScriptActorTargetCandidate>& candidates) {
        lua_createtable(state, static_cast<int>(candidates.size()), 0);
        for (std::size_t index = 0; index < candidates.size(); ++index) {
            const auto& candidate = candidates[index];
            lua_createtable(state, 0, 20);
            lua_pushlstring(state, candidate.id.data(), candidate.id.size());
            lua_setfield(state, -2, "id");
            lua_pushinteger(state, candidate.world_x);
            lua_setfield(state, -2, "world_x");
            lua_pushinteger(state, candidate.world_y);
            lua_setfield(state, -2, "world_y");
            lua_pushnumber(state, bearing_degrees_from_source(candidate.heading));
            lua_setfield(state, -2, "bearing_degrees");
            if (candidate.source_record_heading.has_value()) {
                lua_pushnumber(
                    state,
                    bearing_degrees_from_source(
                        *candidate.source_record_heading));
            } else {
                lua_pushnil(state);
            }
            lua_setfield(state, -2, "facing_degrees");
            lua_pushinteger(state, candidate.distance);
            lua_setfield(state, -2, "distance");
            push_actor_facts(state, actor_facts_from_source_flags(
                candidate.service_flags, candidate.interaction_flags,
                candidate.behavior_flags, candidate.spawn_state_flags));
            lua_setfield(state, -2, "actor_facts");
            lua_pushboolean(state, candidate.projected_y_nonzero);
            lua_setfield(state, -2, "projected_y_nonzero");
            lua_pushboolean(state, candidate.world_activity_actor);
            lua_setfield(state, -2, "world_activity_actor");
            lua_pushinteger(state, candidate.direct_route_step_cost);
            lua_setfield(state, -2, "direct_route_step_cost");
            lua_pushinteger(state,
                            candidate.perpendicular_route_step_cost);
            lua_setfield(state, -2, "perpendicular_route_step_cost");
            lua_pushinteger(state, candidate.direct_route_distance);
            lua_setfield(state, -2, "direct_route_distance");
            lua_pushinteger(state, candidate.perpendicular_route_distance);
            lua_setfield(state, -2, "perpendicular_route_distance");
            push_formation_position(state, "formation_position",
                                    candidate.formation_position);
            if (candidate.party_member_role.has_value()) {
                lua_pushstring(
                    state,
                    party_member_role_id(*candidate.party_member_role));
            } else {
                lua_pushnil(state);
            }
            lua_setfield(state, -2, "party_member_role");
            if (candidate.combatant.has_value()) {
                push_combatant(*candidate.combatant);
            } else {
                lua_pushnil(state);
            }
            lua_setfield(state, -2, "combatant");
            lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
        }
        lua_setfield(state, -2, field);
    };
    push_actor_candidates("ai_target_candidates",
                          context.ai_target_candidates);
    push_actor_candidates("world_target_selection_candidates",
                          context.world_target_selection_candidates);
    lua_createtable(
        state, static_cast<int>(context.projectile_combat_targets.size()), 0);
    for (std::size_t index = 0U;
         index < context.projectile_combat_targets.size(); ++index) {
        const auto& target = context.projectile_combat_targets[index];
        lua_createtable(state, 0, 3);
        lua_pushlstring(state, target.id.data(), target.id.size());
        lua_setfield(state, -2, "id");
        push_combatant(target.combatant);
        lua_setfield(state, -2, "combatant");
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, -2, "projectile_combat_targets");
    if (!semantic_inventory_valid) {
        return fail(
            "actor combat facts contain an item without a semantic identity");
    }
    lua_pushlightuserdata(state, impl_.get());
    lua_pushcclosure(state, Impl::actor_random_bound, 1);
    lua_setfield(state, -2, "ai_random_bound");
    if (context.source_projection_active.has_value()) {
        lua_pushboolean(state, *context.source_projection_active ? 1 : 0);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "source_projection_active");
    if (context.action_line_clear) {
        lua_pushlightuserdata(state, impl_.get());
        lua_pushcclosure(state, Impl::actor_action_line_clear, 1);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "action_line_clear");
    if (context.measure_destination) {
        lua_pushlightuserdata(state, impl_.get());
        lua_pushcclosure(state, Impl::actor_measure_destination, 1);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "measure_destination");
    lua_pushlstring(state, heartbeat_phase.data(), heartbeat_phase.size());
    lua_setfield(state, -2, "phase");
    lua_pushlstring(state, animation_mode.data(), animation_mode.size());
    lua_setfield(state, -2, "animation_mode");
    if (context.animation_direction_override.has_value()) {
        lua_pushinteger(state, *context.animation_direction_override);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "animation_direction_override");
    lua_pushboolean(state, context.animation_clock_reset_requested);
    lua_setfield(state, -2, "animation_clock_reset_requested");
    if (context.projectile_animation.has_value()) {
        const auto& animation = *context.projectile_animation;
        lua_createtable(state, 0, 1);
        lua_pushinteger(state,
                        static_cast<lua_Integer>(animation.frame_count));
        lua_setfield(state, -2, "frame_count");
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "projectile_animation");
    if (context.projectile_movement.has_value()) {
        const auto& movement = *context.projectile_movement;
        lua_createtable(state, 0, 16);
        const auto role = movement.role == ScriptActorProjectileRole::primary
                              ? "primary"
                              : "secondary";
        lua_pushstring(state, role);
        lua_setfield(state, -2, "role");
        const char* phase = movement.phase ==
                                    ScriptActorProjectilePhase::inactive
            ? "inactive"
            : movement.phase ==
                      ScriptActorProjectilePhase::awaiting_animation
                  ? "awaiting_animation"
                  : "flying";
        lua_pushstring(state, phase);
        lua_setfield(state, -2, "phase");
        lua_pushboolean(state, movement.impact_occurred);
        lua_setfield(state, -2, "impact_occurred");
        lua_pushboolean(state, movement.animation_finished);
        lua_setfield(state, -2, "animation_finished");
        lua_createtable(state, 0, 2);
        lua_pushnumber(state, movement.position.x);
        lua_setfield(state, -2, "x");
        lua_pushnumber(state, movement.position.y);
        lua_setfield(state, -2, "y");
        lua_setfield(state, -2, "position");
        lua_createtable(state, 0, 2);
        lua_pushnumber(state, movement.velocity.x);
        lua_setfield(state, -2, "x");
        lua_pushnumber(state, movement.velocity.y);
        lua_setfield(state, -2, "y");
        lua_setfield(state, -2, "velocity");
        lua_pushnumber(state, movement.remaining_flight_range);
        lua_setfield(state, -2, "remaining_flight_range");
        lua_pushinteger(state, movement.shots_remaining);
        lua_setfield(state, -2, "shots_remaining");
        lua_pushnumber(state, movement.bearing_degrees);
        lua_setfield(state, -2, "bearing_degrees");
        lua_createtable(state, 0, 2);
        lua_pushnumber(state, movement.burst_origin.x);
        lua_setfield(state, -2, "x");
        lua_pushnumber(state, movement.burst_origin.y);
        lua_setfield(state, -2, "y");
        lua_setfield(state, -2, "burst_origin");
        lua_pushinteger(state, movement.attack_successes);
        lua_setfield(state, -2, "attack_successes");
        const char* animation_action = "unchanged";
        if (movement.animation_action ==
            ScriptActorProjectileAnimationAction::stop) {
            animation_action = "stop";
        } else if (movement.animation_action ==
                   ScriptActorProjectileAnimationAction::restart) {
            animation_action = "restart";
        }
        lua_pushstring(state, animation_action);
        lua_setfield(state, -2, "animation_action");
        if (movement.hit_target_id.empty()) {
            lua_pushnil(state);
        } else {
            lua_pushlstring(state, movement.hit_target_id.data(),
                            movement.hit_target_id.size());
        }
        lua_setfield(state, -2, "hit_target_id");
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "projectile_movement");
    lua_pushboolean(state, context.projectile_attack_seed_pending);
    lua_setfield(state, -2, "projectile_attack_seed_pending");
    lua_pushboolean(
        state,
        context.movement_services_available &&
            static_cast<bool>(context.movement_collision) &&
            static_cast<bool>(context.movement_route_lookup) &&
            (static_cast<bool>(context.movement_collision_recovery) ||
             static_cast<bool>(context.movement_mode_transition)));
    lua_setfield(state, -2, "movement_services_available");
    lua_pushinteger(state, context.movement_default_step_distance);
    lua_setfield(state, -2, "movement_default_step_distance");
    lua_createtable(state, 0, 13);
    lua_pushinteger(state, context.movement_state.position_x);
    lua_setfield(state, -2, "position_x");
    lua_pushinteger(state, context.movement_state.position_y);
    lua_setfield(state, -2, "position_y");
    lua_pushnumber(
        state, bearing_degrees_from_source(context.movement_state.heading));
    lua_setfield(state, -2, "bearing_degrees");
    lua_pushnumber(
        state, bearing_degrees_from_source(
                   context.movement_state.animation_heading));
    lua_setfield(state, -2, "animation_bearing_degrees");
    const char* substate_update = "unchanged";
    if (context.movement_state.substate_update ==
        ScriptMovementSubstateUpdate::direct_translation) {
        substate_update = "direct_translation";
    } else if (context.movement_state.substate_update ==
               ScriptMovementSubstateUpdate::cardinal_fallback) {
        substate_update = "cardinal_fallback";
    }
    lua_pushstring(state, substate_update);
    lua_setfield(state, -2, "substate_update");
    lua_createtable(state, 0, 1);
    lua_pushstring(state, "unchanged");
    lua_setfield(state, -2, "kind");
    lua_setfield(state, -2, "route_update");
    push_actor_behavior_facts(state, context.movement_state.route_flags);
    lua_setfield(state, -2, "route_facts");
    lua_pushboolean(state, context.movement_state.mark_reset_after_block);
    lua_setfield(state, -2, "mark_reset_after_block");
    lua_pushboolean(state, context.movement_state.mark_directional_fallback);
    lua_setfield(state, -2, "mark_directional_fallback");
    lua_pushboolean(state, context.movement_state.clear_collision_recovery);
    lua_setfield(state, -2, "clear_collision_recovery");
    lua_pushboolean(state, context.movement_state.clear_pending_event);
    lua_setfield(state, -2, "clear_pending_event");
    lua_pushinteger(state, context.movement_state.collision_extent);
    lua_setfield(state, -2, "collision_extent");
    lua_pushboolean(state,
                    context.movement_state.terrain_collision_code == 0U);
    lua_setfield(state, -2, "terrain_is_clear");
    lua_setfield(state, -2, "movement_state");
    push_actor_facts(state, actor_facts_from_source_flags(
        context.movement_entity_flags, 0U, 0U, 0U));
    lua_setfield(state, -2, "movement_actor_facts");
    lua_pushboolean(state, context.walk_fast_enabled);
    lua_setfield(state, -2, "walk_fast_enabled");
    lua_pushinteger(state, context.movement_scene_id);
    lua_setfield(state, -2, "scene_id");
    ScopedInstructionBudget instruction_budget(state);
    impl_->last_actor_movement_recovery_probe.reset();
    impl_->last_actor_terrain_collision_code.reset();
    impl_->active_actor_context = &context;
    const auto actor_status = lua_pcall(state, 1, 1, 0);
    impl_->active_actor_context = nullptr;
    if (actor_status != LUA_OK) {
        return fail("actor_ai.heartbeat: " + stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        std::string reason =
            "actor_ai.heartbeat did not return a snapshot: actor=" +
            std::string(context.instance_id) + " profile=" +
            std::string(context.profile_id) + " phase=" +
            (context.phase == ScriptActorHeartbeatPhase::behavior
                 ? std::string("behavior")
                 : std::string("render")) +
            " spawn=" + (context.spawn ? "true" : "false");
        if (context.projectile_movement.has_value()) {
            const auto& movement = *context.projectile_movement;
            reason += " projectile={role=" +
                (movement.role == ScriptActorProjectileRole::primary
                     ? std::string("primary")
                     : std::string("secondary")) +
                ",phase=" +
                std::to_string(static_cast<unsigned>(movement.phase)) +
                ",impact_occurred=" +
                (movement.impact_occurred ? "true" : "false") +
                ",animation_finished=" +
                (movement.animation_finished ? "true" : "false") +
                ",flight_range=" +
                std::to_string(movement.remaining_flight_range) +
                ",shots_remaining=" +
                std::to_string(movement.shots_remaining) +
                "}";
        }
        return fail(std::move(reason));
    }

    const auto decision_table = lua_absindex(state, -1);
    ScriptActorBehaviorDecision decision;
    std::string action;
    std::string returned_profile_id;
    std::string returned_actor_id;
    std::string returned_action_kind;
    std::string returned_action_id;
    std::uint32_t returned_value = 0U;
    if (!read_field_string(state, decision_table, "actor_id",
                           returned_actor_id) ||
        !read_field_string(state, decision_table, "package_id",
                           decision.package_id) ||
        !valid_identifier(decision.package_id) ||
        !read_field_string(state, decision_table, "profile_id",
                           returned_profile_id) ||
        !read_field_string(state, decision_table, "action_kind",
                           returned_action_kind) ||
        !read_optional_field_string(state, decision_table, "action_id",
                                    returned_action_id)) {
        return fail("actor_ai.heartbeat returned an incomplete snapshot");
    }
    const auto expected_action_id = action_item_id.has_value()
        ? *action_item_id
        : action_spell_id.has_value() ? *action_spell_id
                                      : std::string_view{};
    if (returned_actor_id != actor_id ||
        returned_profile_id != context.profile_id ||
        returned_action_kind != action_kind ||
        returned_action_id != expected_action_id) {
        return fail("actor_ai.heartbeat changed source actor identity");
    }
    if (!read_field_string(state, decision_table, "action", action) ||
        !read_field_string(state, decision_table, "animation",
                           decision.animation_id) ||
        !read_field_integer(state, decision_table, "direction", 7U,
                            returned_value)) {
        return fail("actor_ai.heartbeat returned an incomplete snapshot");
    }
    decision.direction = static_cast<std::uint8_t>(returned_value);
    if (!read_field_integer(state, decision_table, "sprite_flip", 1U,
                            returned_value)) {
        return fail("actor_ai.heartbeat returned an invalid sprite flip");
    }
    decision.animation_flip_x = returned_value == 1U;
    if (!read_field_integer(state, decision_table, "sprite_priority", 1U,
                            returned_value)) {
        return fail("actor_ai.heartbeat returned an invalid sprite priority");
    }
    decision.sprite_priority = returned_value == 0U
        ? ScriptFieldSpritePriority::low
        : ScriptFieldSpritePriority::high;
    ActorFacts returned_actor_facts;
    if (!read_actor_facts(state, decision_table, returned_actor_facts)) {
        return fail("actor_ai.heartbeat returned invalid named actor facts");
    }
    decision.actor_facts = returned_actor_facts;
    if (!read_field_boolean(state, decision_table,
                            "combat_engagement_started",
                            decision.combat_engagement_started)) {
        return fail("actor_ai.heartbeat returned an invalid combat event");
    }
    if (!read_optional_field_u32(
            state, decision_table, "field_event_request",
            std::numeric_limits<std::uint32_t>::max(),
            decision.field_event_request)) {
        return fail("actor_ai.heartbeat returned an invalid field event request");
    }
    if (!read_field_boolean(state, decision_table, "think_token_claimed",
                            decision.think_token_claimed)) {
        return fail("actor_ai.heartbeat returned an invalid think-token result");
    }
    if (!read_optional_field_u32(
            state, decision_table, "thon_action_cooldown_remaining",
            std::numeric_limits<std::uint32_t>::max(),
            decision.thon_action_cooldown_remaining)) {
        return fail("actor_ai.heartbeat returned an invalid Thon cooldown");
    }
    if (!read_field_integer(state, decision_table, "frame", 15U,
                            returned_value)) {
        return fail("actor_ai.heartbeat returned an invalid animation frame");
    }
    decision.animation_frame = static_cast<std::uint8_t>(returned_value);
    if (!read_field_boolean(state, decision_table, "animation_complete",
                            decision.animation_complete) ||
        !read_field_string_allow_empty(
            state, decision_table, "animation_variant",
            decision.animation_variant) ||
        (decision.animation_variant != "" &&
         decision.animation_variant != "spell_death_e0" &&
         decision.animation_variant != "spell_death_e4")) {
        return fail("actor_ai.heartbeat returned invalid semantic animation facts");
    }
    lua_getfield(state, decision_table, "hit_spark_retired");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_isboolean(state, -1)) {
            lua_pop(state, 1);
            return fail("actor_ai.heartbeat returned an invalid hit-spark retirement state");
        }
        decision.hit_spark_retired = lua_toboolean(state, -1) != 0;
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "target_marker_visible");
    if (!lua_isboolean(state, -1)) {
        return fail("actor_ai.heartbeat returned an invalid target-marker visibility");
    }
    decision.target_marker_visible = lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "target_marker_animating");
    if (!lua_isboolean(state, -1)) {
        return fail("actor_ai.heartbeat returned an invalid target-marker animation state");
    }
    decision.target_marker_animating = lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);
    std::uint32_t target_marker_value = 0U;
    if (!read_field_integer(state, decision_table, "target_marker_style",
                            3U, target_marker_value)) {
        return fail("actor_ai.heartbeat returned an invalid target-marker style");
    }
    decision.target_marker_style =
        static_cast<std::uint8_t>(target_marker_value);
    if (!read_field_integer(state, decision_table, "target_marker_radius",
                            64U, target_marker_value)) {
        return fail("actor_ai.heartbeat returned an invalid target-marker radius");
    }
    decision.target_marker_radius =
        static_cast<std::uint8_t>(target_marker_value);
    int signed_actor_value = 0;
    if (!read_field_signed_integer(state, decision_table, "action_delay",
                                   -0x8000, 0x7FFF, signed_actor_value)) {
        return fail("actor_ai.heartbeat returned an invalid action delay");
    }
    decision.action_delay = static_cast<std::int16_t>(signed_actor_value);
    if (!read_optional_field_integer(
            state, decision_table, "target_motion_effect_slot_request",
            17U, decision.target_motion_effect_slot_request)) {
        return fail("actor_ai.heartbeat returned an invalid creature effect slot");
    }
    if (!read_field_boolean(state, decision_table,
                            "target_motion_effect_visible",
                            decision.target_motion_effect_visible)) {
        return fail("actor_ai.heartbeat returned an invalid creature effect visibility");
    }
    std::uint32_t creature_effect_value = 0U;
    if (!read_field_integer(state, decision_table,
                            "target_motion_effect_direction", 7U,
                            creature_effect_value)) {
        return fail("actor_ai.heartbeat returned an invalid creature effect direction");
    }
    decision.target_motion_effect_direction =
        static_cast<std::uint8_t>(creature_effect_value);
    if (!read_field_integer(state, decision_table,
                            "target_motion_effect_phase", 4U,
                            creature_effect_value)) {
        return fail("actor_ai.heartbeat returned an invalid creature effect phase");
    }
    decision.target_motion_effect_phase =
        static_cast<std::uint8_t>(creature_effect_value);
    if (!read_field_boolean(state, decision_table,
                            "target_motion_effect_slot_released",
                            decision.target_motion_effect_slot_released)) {
        return fail("actor_ai.heartbeat returned an invalid creature slot release");
    }
    if (!read_field_boolean(state, decision_table, "spell_effect_visible",
                            decision.spell_effect_visible)) {
        return fail("actor_ai.heartbeat returned invalid spell effect visibility");
    }
    std::uint32_t spell_effect_phase_value = 0U;
    if (!read_field_integer(state, decision_table, "spell_effect_phase", 16U,
                            spell_effect_phase_value)) {
        return fail("actor_ai.heartbeat returned an invalid spell effect phase");
    }
    decision.spell_effect_phase =
        static_cast<std::uint8_t>(spell_effect_phase_value);
    if (!read_optional_field_string(state, decision_table,
                                    "spell_effect_id",
                                    decision.spell_effect_id) ||
        (!decision.spell_effect_id.empty() &&
         find_us_magic_spell(decision.spell_effect_id) == nullptr)) {
        return fail("actor_ai.heartbeat returned an invalid spell effect id");
    }
    std::optional<std::uint32_t> released_spell_slot;
    if (!read_optional_field_u32(state, decision_table,
                                 "spell_effect_slot_released", 17U,
                                 released_spell_slot)) {
        return fail("actor_ai.heartbeat returned an invalid spell effect slot release");
    }
    if (released_spell_slot.has_value()) {
        if (context.phase != ScriptActorHeartbeatPhase::render ||
            context.instance_id.empty() || !context.combatant.has_value()) {
            return fail("actor_ai.heartbeat released a spell effect outside its render pass");
        }
        decision.spell_effect_slot_released =
            static_cast<std::uint8_t>(*released_spell_slot);
    }
    std::optional<std::uint32_t> field_barrier_remaining;
    if (!read_optional_field_u32(state, decision_table,
                                 "field_barrier_remaining", 0xFFFFU,
                                 field_barrier_remaining)) {
        return fail("actor_ai.heartbeat returned an invalid field barrier lifetime");
    }
    if (field_barrier_remaining.has_value()) {
        if (context.phase != ScriptActorHeartbeatPhase::render ||
            context.instance_id != "field_player_runner" ||
            *field_barrier_remaining == 0U) {
            return fail("actor_ai.heartbeat returned field barrier lifetime outside its render pass");
        }
        decision.field_barrier_remaining = static_cast<std::uint16_t>(
            *field_barrier_remaining);
    }
    if (!read_field_boolean(state, decision_table, "spell_effect_completed",
                            decision.spell_effect_completed)) {
        return fail("actor_ai.heartbeat returned an invalid spell effect completion state");
    }
    const bool hound_encounter = context.dynamic_world_encounter_actor &&
        context.entity_kind == 12U &&
        context.profile_id == "hell_hound_k0c_none00";
    if ((!hound_encounter &&
         (decision.target_motion_effect_slot_request.has_value() ||
          decision.target_motion_effect_visible ||
          decision.target_motion_effect_slot_released)) ||
        (decision.target_motion_effect_slot_request.has_value() &&
         (!hound_encounter ||
          context.phase != ScriptActorHeartbeatPhase::behavior ||
          context.world_effect_slot_occupancy[
              *decision.target_motion_effect_slot_request])) ||
        (decision.target_motion_effect_visible &&
         (!hound_encounter ||
          context.phase != ScriptActorHeartbeatPhase::render ||
          decision.target_motion_effect_phase < 1U ||
          decision.target_motion_effect_phase > 3U)) ||
        (decision.target_motion_effect_slot_released &&
         (!hound_encounter ||
          context.phase != ScriptActorHeartbeatPhase::render ||
          decision.target_motion_effect_visible ||
          decision.target_motion_effect_phase != 4U))) {
        return fail("actor_ai.heartbeat returned an inconsistent creature effect state");
    }
    const auto spell_phase_count = [](const std::string_view spell_id) {
        if (spell_id == "flame_dart") {
            return std::uint8_t{4U};
        }
        if (spell_id == "flame_bolt" || spell_id == "mana_blast") {
            return std::uint8_t{6U};
        }
        if (spell_id == "mana_zap") {
            return std::uint8_t{3U};
        }
        if (spell_id == "sleep") {
            return std::uint8_t{9U};
        }
        if (spell_id == "heal_wounds") {
            return std::uint8_t{16U};
        }
        // ROM 0x49A2 uses a timed field record and one terminal render phase.
        if (spell_id == "super_barrier" || spell_id == "barrier") {
            return std::uint8_t{1U};
        }
        return std::uint8_t{0U};
    };
    const bool spell_effect_reported = decision.spell_effect_visible ||
        decision.spell_effect_slot_released.has_value() ||
        decision.spell_effect_completed;
    const bool spell_effect_target_context =
        context.phase == ScriptActorHeartbeatPhase::render &&
        !context.instance_id.empty() && context.combatant.has_value();
    const auto expected_spell_phases =
        spell_phase_count(decision.spell_effect_id);
    const bool invalid_spell_completion = decision.spell_effect_completed &&
        (decision.spell_effect_visible ||
         decision.spell_effect_phase != expected_spell_phases ||
         (decision.spell_effect_id != "sleep" &&
          !decision.spell_effect_slot_released.has_value()));
    if ((spell_effect_reported &&
         (!spell_effect_target_context || expected_spell_phases == 0U ||
          decision.spell_effect_id.empty())) ||
        (!spell_effect_reported &&
         !decision.spell_effect_id.empty()) ||
        (decision.spell_effect_visible &&
         (decision.spell_effect_phase >= expected_spell_phases ||
          decision.spell_effect_slot_released.has_value())) ||
        (decision.spell_effect_slot_released.has_value() &&
         (decision.spell_effect_visible ||
          decision.spell_effect_phase != expected_spell_phases)) ||
        invalid_spell_completion) {
        return fail("actor_ai.heartbeat returned an inconsistent spell effect state");
    }
    if ((!decision.target_marker_visible &&
         (decision.target_marker_animating ||
          decision.target_marker_style != 0U ||
          decision.target_marker_radius != 0U)) ||
        (!decision.target_marker_animating &&
         decision.target_marker_radius != 0U) ||
        (decision.target_marker_animating &&
         decision.target_marker_radius % 8U != 0U)) {
        return fail("actor_ai.heartbeat returned inconsistent target-marker state");
    }
    lua_getfield(state, decision_table, "target_selection_processed");
    if (!lua_isboolean(state, -1)) {
        return fail("actor_ai.heartbeat returned invalid target selection state");
    }
    decision.target_selection_processed = lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);
    const bool target_selection_expected =
        context.target_selection_requested &&
        context.phase == ScriptActorHeartbeatPhase::behavior;
    if (decision.target_selection_processed != target_selection_expected ||
        (decision.target_selection_processed &&
         context.profile_id != "field_runner_k03_none00")) {
        return fail("actor_ai.heartbeat returned an unexpected target selection");
    }
    std::string selected_world_actor_id;
    if (!read_optional_field_string(state, decision_table,
                                    "selected_world_actor_id",
                                    selected_world_actor_id)) {
        return fail("actor_ai.heartbeat returned an invalid selected actor id");
    }
    if (!selected_world_actor_id.empty()) {
        const auto selected_candidate = std::find_if(
            context.world_target_selection_candidates.begin(),
            context.world_target_selection_candidates.end(),
            [&selected_world_actor_id](const auto& candidate) {
                return candidate.id == selected_world_actor_id &&
                       (candidate.service_flags & 0x01U) != 0U;
            });
        if (!decision.target_selection_processed ||
            selected_candidate ==
                context.world_target_selection_candidates.end()) {
            return fail("actor_ai.heartbeat selected a missing world actor");
        }
        decision.selected_world_actor_id =
            std::move(selected_world_actor_id);
    }
    if (!decision.target_selection_processed &&
        decision.selected_world_actor_id.has_value()) {
        return fail("actor_ai.heartbeat retained a stale selected actor");
    }
    struct ResolvedCombatTarget {
        std::uint8_t record_slot = 0;
        const ScriptCombatantFacts* combatant = nullptr;
        bool world_activity_actor = false;
    };
    const auto resolve_combat_target =
        [&context, &actor_id](const std::string_view target_id,
                              const bool allow_caster)
        -> std::optional<ResolvedCombatTarget> {
        if (allow_caster && target_id == actor_id &&
            context.source_record_slot.has_value() &&
            context.combatant.has_value()) {
            return ResolvedCombatTarget{
                *context.source_record_slot, &*context.combatant, false};
        }
        if (context.projectile_movement.has_value()) {
            const auto target = std::find_if(
                context.projectile_combat_targets.begin(),
                context.projectile_combat_targets.end(),
                [target_id](const auto& candidate) {
                    return candidate.id == target_id;
                });
            if (target != context.projectile_combat_targets.end()) {
                return ResolvedCombatTarget{
                    target->source_record_slot, &target->combatant, false};
            }
            return std::nullopt;
        }
        const auto target = std::find_if(
            context.ai_target_candidates.begin(),
            context.ai_target_candidates.end(),
            [target_id](const auto& candidate) {
                return candidate.id == target_id &&
                       candidate.source_record_slot.has_value() &&
                       candidate.combatant.has_value();
            });
        if (target == context.ai_target_candidates.end()) {
            return std::nullopt;
        }
        return ResolvedCombatTarget{
            *target->source_record_slot, &*target->combatant,
            target->world_activity_actor};
    };
    lua_getfield(state, decision_table, "combat_meter_update");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_istable(state, -1)) {
            return fail("actor_ai.heartbeat returned an invalid combat meter update");
        }
        const auto update_table = lua_absindex(state, -1);
        ScriptCombatMeterUpdate update{};
        std::uint32_t combat_value = 0U;
        if (!read_field_string(state, update_table, "target_id",
                               update.target_id)) {
            return fail("actor_ai.heartbeat returned incomplete combat target facts");
        }
        if (!read_field_integer(state, update_table, "physical_meter",
                                0xFFU, combat_value)) {
            return fail("actor_ai.heartbeat returned an invalid physical meter");
        }
        update.physical_meter = static_cast<std::uint8_t>(combat_value);
        if (!read_field_integer(state, update_table, "mental_meter",
                                0xFFU, combat_value)) {
            return fail("actor_ai.heartbeat returned an invalid mental meter");
        }
        update.mental_meter = static_cast<std::uint8_t>(combat_value);
        if (!read_field_boolean(state, update_table, "physical",
                                update.physical) ||
            !read_field_boolean(state, update_table, "changed",
                                update.changed)) {
            return fail("actor_ai.heartbeat returned invalid combat meter flags");
        }
        const auto target = resolve_combat_target(update.target_id, true);
        if (!target.has_value() || target->combatant == nullptr) {
            return fail("actor_ai.heartbeat returned an unlinked combat target");
        }
        update.record_type = target->combatant->record_type;
        update.source_record_slot = target->record_slot;
        const auto* target_combatant = target->combatant;
        const bool target_is_party_member =
            target_combatant->record_type < 3U;
        bool target_linked = false;
        if (context.projectile_movement.has_value()) {
            target_linked = target_is_party_member
                ? update.source_record_slot < 3U
                : update.source_record_slot >= 3U;
            if (context.profile_id != "weapon_projectile" ||
                context.phase != ScriptActorHeartbeatPhase::behavior ||
                target_combatant == nullptr || !target_linked) {
                return fail("weapon projectile returned an unlinked combat target");
            }
        } else {
            target_linked =
                (context.dynamic_world_encounter_actor ||
                 context.player_melee_available) &&
                context.phase == ScriptActorHeartbeatPhase::behavior &&
                target_is_party_member != target->world_activity_actor &&
                ((target_is_party_member &&
                  update.source_record_slot < 3U) ||
                 (!target_is_party_member &&
                  update.source_record_slot >= 3U));
            if (!target_linked) {
                return fail("actor_ai.heartbeat returned an unlinked combat target");
            }
        }
        const bool meters_changed =
            update.physical_meter != target_combatant->physical_meter ||
            update.mental_meter != target_combatant->mental_meter;
        if (meters_changed != update.changed) {
            return fail("actor_ai.heartbeat returned an inconsistent combat meter update");
        }
        decision.combat_meter_update = std::move(update);
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "spell_action_resolution");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_istable(state, -1)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell action resolution");
        }
        const auto resolution_table = lua_absindex(state, -1);
        ScriptActorSpellActionResolution resolution{};
        std::uint32_t resolution_value = 0U;
        std::uint32_t field_effect_kind_value = 0U;
        int signed_resolution_value = 0;
        if (!read_field_string(state, resolution_table, "spell_id",
                               resolution.spell_id)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell identity");
        }
        const auto source_action_code =
            script_spell_action_code_from_id(resolution.spell_id);
        if (!source_action_code.has_value()) {
            return fail(
                "actor_ai.heartbeat returned an unknown spell identity");
        }
        resolution.source_action_code = *source_action_code;
        if (!read_field_string_allow_empty(state, resolution_table,
                                           "target_id", resolution.target_id) ||
            !read_field_boolean(state, resolution_table, "untargeted",
                                resolution.untargeted) ||
            !read_field_integer(state, resolution_table,
                                "field_effect_kind", 1U,
                                field_effect_kind_value)) {
            return fail(
                "actor_ai.heartbeat returned incomplete spell target facts");
        }
        resolution.field_effect_kind =
            static_cast<ScriptActorSpellFieldEffectKind>(
                field_effect_kind_value);
        const auto action_target = resolve_combat_target(
            resolution.untargeted ? std::string_view(actor_id)
                                  : std::string_view(resolution.target_id),
            true);
        if (!action_target.has_value() || action_target->combatant == nullptr) {
            return fail(
                "actor_ai.heartbeat returned an unlinked spell target");
        }
        resolution.target_record_type =
            action_target->combatant->record_type;
        resolution.target_record_slot = action_target->record_slot;
        if (!read_field_integer(state, resolution_table, "successes",
                                0x7FU, resolution_value) ||
            !read_field_boolean(state, resolution_table, "roll_attempted",
                                resolution.roll_attempted) ||
            !read_field_string(state, resolution_table, "outcome",
                               resolution.outcome) ||
            !read_field_boolean(state, resolution_table, "effect_requested",
                                resolution.effect_requested) ||
            !read_field_boolean(state, resolution_table, "effect_started",
                                resolution.effect_started) ||
            !read_field_signed_integer(state, resolution_table,
                                       "residual_effect", -0x8000, 0x7FFF,
                                       signed_resolution_value)) {
            return fail(
                "actor_ai.heartbeat returned incomplete spell result facts");
        }
        resolution.successes =
            static_cast<std::uint16_t>(resolution_value);
        resolution.residual_effect =
            static_cast<std::int16_t>(signed_resolution_value);
        if (!read_field_integer(state, resolution_table,
                                "behavior_meter_delta", 0xFFU,
                                resolution_value)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell meter delta");
        }
        resolution.behavior_meter_delta =
            static_cast<std::uint8_t>(resolution_value);
        if (!read_field_boolean(state, resolution_table, "inventory_changed",
                                resolution.inventory_changed) ||
            !read_field_boolean(state, resolution_table,
                                "special_result_published",
                                resolution.special_result_published) ||
            !read_field_integer(state, resolution_table,
                                "special_result_successes", 0xFFU,
                                resolution_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid spell side effects");
        }
        resolution.special_result_successes =
            static_cast<std::uint8_t>(resolution_value);
        if (!read_field_integer(state, resolution_table,
                                "special_result_bonus", 0xFFU,
                                resolution_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid spell side effects");
        }
        resolution.special_result_bonus =
            static_cast<std::uint8_t>(resolution_value);

        const auto read_optional_byte = [&](const char* field,
                                            std::optional<std::uint8_t>& value) {
            lua_getfield(state, resolution_table, field);
            if (lua_type(state, -1) == LUA_TNIL) {
                lua_pop(state, 1);
                value.reset();
                return true;
            }
            std::uint32_t parsed = 0U;
            const bool valid = read_integer(state, -1, 0xFFU, parsed);
            lua_pop(state, 1);
            if (valid) {
                value = static_cast<std::uint8_t>(parsed);
            }
            return valid;
        };
        if (!read_optional_byte("initial_trial_count",
                                resolution.initial_trial_count) ||
            !read_optional_byte("difficulty", resolution.difficulty) ||
            !read_optional_byte("spell_effect_slot",
                                resolution.spell_effect_slot)) {
            return fail(
                "actor_ai.heartbeat returned invalid spell roll facts");
        }
        lua_getfield(state, resolution_table, "next_action_kind");
        if (lua_type(state, -1) != LUA_TNIL) {
            std::string next_action_kind;
            if (!read_string(state, -1, next_action_kind)) {
                return fail(
                    "actor_ai.heartbeat returned an invalid next spell action");
            }
            if (next_action_kind == "none") {
                resolution.next_action_kind = ScriptActorActionKind::none;
            } else if (next_action_kind == "item") {
                resolution.next_action_kind = ScriptActorActionKind::item;
            } else {
                return fail(
                    "actor_ai.heartbeat returned an unsupported next spell action");
            }
        }
        lua_pop(state, 1);
        lua_getfield(state, resolution_table, "next_action_item_id");
        if (lua_type(state, -1) != LUA_TNIL) {
            std::string item_id;
            if (!read_string(state, -1, item_id)) {
                return fail(
                    "actor_ai.heartbeat returned an invalid next inventory item");
            }
            const auto source_code =
                script_item_source_code_from_id(item_id);
            if (!source_code.has_value()) {
                return fail(
                    "actor_ai.heartbeat returned an unknown next inventory item");
            }
            resolution.next_action_code = *source_code;
        } else if (resolution.next_action_kind ==
                   ScriptActorActionKind::none) {
            resolution.next_action_code = 0U;
        }
        lua_pop(state, 1);
        std::string next_profile_prefix;
        if (!read_optional_field_string(state, resolution_table,
                                        "next_profile_prefix",
                                        next_profile_prefix)) {
            return fail(
                "actor_ai.heartbeat returned an invalid next actor profile");
        }
        if (!next_profile_prefix.empty()) {
            if (!valid_identifier(next_profile_prefix) ||
                !resolution.next_action_kind.has_value() ||
                !resolution.next_action_code.has_value()) {
                return fail(
                    "actor_ai.heartbeat returned an incomplete next actor identity");
            }
            std::string next_profile_id = next_profile_prefix + "_";
            if (resolution.next_action_kind == ScriptActorActionKind::none) {
                next_profile_id += "none00";
            } else {
                constexpr char hex_digits[] = "0123456789abcdef";
                const auto item_code = *resolution.next_action_code;
                next_profile_id += "item";
                next_profile_id += hex_digits[(item_code >> 4U) & 0x0FU];
                next_profile_id += hex_digits[item_code & 0x0FU];
            }
            if (!valid_identifier(next_profile_id)) {
                return fail(
                    "actor_ai.heartbeat returned an invalid next actor profile");
            }
            resolution.next_profile_id = std::move(next_profile_id);
        }

        lua_getfield(state, resolution_table, "combatant");
        if (!lua_istable(state, -1)) {
            return fail(
                "actor_ai.heartbeat omitted the updated spell caster state");
        }
        const auto updated_combatant_table = lua_absindex(state, -1);
        if (!read_field_integer(state, updated_combatant_table,
                                "last_action_successes", 0x7FU,
                                resolution_value)) {
            return fail(
                "actor_ai.heartbeat returned an invalid caster success count");
        }
        const auto returned_successes =
            static_cast<std::uint16_t>(resolution_value);
        if ((resolution.roll_attempted &&
             returned_successes != resolution.successes) ||
            !read_field_integer(state, updated_combatant_table,
                                "mental_meter", 0xFFU, resolution_value)) {
            return fail(
                "actor_ai.heartbeat returned inconsistent spell caster state");
        }
        resolution.mental_meter =
            static_cast<std::uint8_t>(resolution_value);
        if (context.combatant.has_value()) {
            resolution.item_codes = context.combatant->item_codes;
            resolution.item_phases = context.combatant->item_phases;
        }
        if (resolution.inventory_changed) {
            std::array<std::uint8_t, 8> updated_item_codes{};
            std::array<std::uint8_t, 8> updated_item_values{};
            if (!context.combatant.has_value() ||
                !read_combat_inventory(state, updated_combatant_table,
                                       updated_item_codes,
                                       updated_item_values)) {
                return fail(
                    "actor_ai.heartbeat returned invalid semantic spell inventory");
            }
            const auto fetish_code = script_item_source_code_from_id("fetish");
            if (!fetish_code.has_value()) {
                return fail("semantic inventory has no fetish item identity");
            }
            for (std::size_t slot = 0U;
                 slot < resolution.item_codes.size(); ++slot) {
                if (resolution.item_codes[slot] == *fetish_code) {
                    resolution.item_codes[slot] = updated_item_codes[slot];
                    resolution.item_phases[slot] = updated_item_values[slot];
                }
            }
        }
        lua_pop(state, 1);

        static const std::array<std::string_view, 8> spell_outcomes{
            "target_state_gate", "roll_failed", "source_data_unavailable",
            "profile_data_unavailable", "random_service_unavailable",
            "effect_dispatched", "effect_residual_empty", "fetish_consumed"};
        const bool known_outcome =
            std::find(spell_outcomes.begin(), spell_outcomes.end(),
                      resolution.outcome) != spell_outcomes.end() ||
            resolution.outcome == "fetish_exhausted";
        const bool effect_outcome =
            resolution.outcome == "effect_dispatched" ||
            resolution.outcome == "effect_residual_empty" ||
            resolution.outcome == "fetish_consumed" ||
            resolution.outcome == "fetish_exhausted";
        const auto target = std::find_if(
            context.ai_target_candidates.begin(),
            context.ai_target_candidates.end(),
            [&resolution](const auto& candidate) {
                return candidate.id == resolution.target_id &&
                       candidate.source_record_slot.has_value() &&
                       *candidate.source_record_slot ==
                           resolution.target_record_slot &&
                       candidate.combatant.has_value() &&
                       candidate.combatant->record_type ==
                           resolution.target_record_type;
            });
        const bool linked_target =
            target != context.ai_target_candidates.end() &&
            (context.dynamic_world_encounter_actor ||
             context.player_spell_available) &&
            context.phase == ScriptActorHeartbeatPhase::behavior &&
            target->world_activity_actor !=
                (resolution.target_record_type < 3U) &&
            ((resolution.target_record_type < 3U &&
              resolution.target_record_slot < 3U) ||
             (resolution.target_record_type >= 3U &&
              resolution.target_record_slot >= 3U));
        const bool untargeted_player_cast =
            resolution.untargeted && resolution.target_id.empty() &&
            context.player_spell_available &&
            !context.dynamic_world_encounter_actor &&
            context.phase == ScriptActorHeartbeatPhase::behavior &&
            context.source_record_slot.has_value() &&
            *context.source_record_slot == resolution.target_record_slot &&
            context.combatant.has_value() &&
            context.combatant->record_type ==
                resolution.target_record_type;
        const bool field_barrier = resolution.field_effect_kind ==
            ScriptActorSpellFieldEffectKind::field_barrier;
        const bool field_effect_kind_valid =
            resolution.field_effect_kind ==
                ScriptActorSpellFieldEffectKind::none ||
            (field_barrier && untargeted_player_cast &&
             (resolution.spell_id == "super_barrier" ||
              resolution.spell_id == "barrier") &&
             resolution.effect_requested);
        // The field player's cast code is offered separately; its profile
        // action code stays "none" for movement and weapon services.
        const auto caster_action_code =
            context.player_spell_available &&
                    !context.dynamic_world_encounter_actor
                ? context.player_spell_code
                : context.action_code;
        const auto expected_spell_id =
            script_spell_id_from_action_code(caster_action_code);
        if (!known_outcome ||
            resolution.source_action_code != caster_action_code ||
            !expected_spell_id.has_value() ||
            *expected_spell_id != resolution.spell_id ||
            !context.combatant.has_value() ||
            (!linked_target && !untargeted_player_cast) ||
            (resolution.untargeted && !untargeted_player_cast) ||
            (!resolution.untargeted && field_barrier) ||
            !field_effect_kind_valid ||
            resolution.effect_requested != effect_outcome ||
            (resolution.effect_started &&
             (!resolution.effect_requested ||
              !resolution.spell_effect_slot.has_value())) ||
            (resolution.spell_effect_slot.has_value() !=
             resolution.effect_started) ||
            (resolution.effect_started &&
             (*resolution.spell_effect_slot >=
                  context.world_effect_slot_occupancy.size() ||
              context.world_effect_slot_occupancy[
                  *resolution.spell_effect_slot])) ||
            (resolution.roll_attempted &&
             ((resolution.successes > 0U) !=
              resolution.effect_requested)) ||
            (!resolution.roll_attempted &&
             (resolution.successes != 0U ||
              resolution.effect_requested)) ||
            (resolution.behavior_meter_delta != 0U &&
             resolution.behavior_meter_delta != 1U) ||
            (resolution.inventory_changed && !resolution.effect_requested) ||
            (resolution.next_action_kind.has_value() !=
             resolution.next_action_code.has_value()) ||
            (resolution.next_action_kind.has_value() !=
             resolution.next_profile_id.has_value()) ||
            (resolution.next_action_kind.has_value() &&
             ((resolution.next_action_kind == ScriptActorActionKind::none) !=
              (*resolution.next_action_code == 0U))) ||
            (resolution.next_action_kind.has_value() &&
             resolution.outcome != "target_state_gate") ||
            (resolution.special_result_published &&
             (resolution.spell_id != "stink" &&
              resolution.spell_id != "confusion"))) {
            return fail(
                "actor_ai.heartbeat returned an inconsistent spell action resolution");
        }
        decision.spell_action_resolution = std::move(resolution);
    }
    lua_pop(state, 1);

    lua_getfield(state, decision_table, "spell_effect_resolution");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_istable(state, -1)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell effect resolution");
        }
        const auto resolution_table = lua_absindex(state, -1);
        ScriptActorSpellEffectResolution resolution{};
        std::uint32_t value = 0U;
        int signed_value = 0;
        if (!read_field_string(state, resolution_table, "spell_id",
                               resolution.spell_id)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell effect id");
        }
        const auto source_action_code =
            script_spell_action_code_from_id(resolution.spell_id);
        if (!source_action_code.has_value()) {
            return fail(
                "actor_ai.heartbeat returned an unknown spell effect id");
        }
        resolution.source_action_code = *source_action_code;
        if (!read_field_string(state, resolution_table, "target_id",
                               resolution.target_id)) {
            return fail(
                "actor_ai.heartbeat returned incomplete spell effect target facts");
        }
        const auto effect_target =
            resolve_combat_target(resolution.target_id, true);
        if (!effect_target.has_value() || effect_target->combatant == nullptr) {
            return fail(
                "actor_ai.heartbeat returned an unlinked spell effect target");
        }
        resolution.target_record_type =
            effect_target->combatant->record_type;
        resolution.target_record_slot = effect_target->record_slot;
        if (!read_field_integer(state, resolution_table, "target_trial_count",
                                0xFFU, value)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell target roll count");
        }
        resolution.target_trial_count = static_cast<std::uint8_t>(value);
        if (!read_field_integer(state, resolution_table, "difficulty",
                                0xFFU, value)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell target difficulty");
        }
        resolution.difficulty = static_cast<std::uint8_t>(value);
        if (!read_field_integer(state, resolution_table,
                                "target_roll_successes", 0xFFU, value)) {
            return fail(
                "actor_ai.heartbeat returned an invalid spell target result");
        }
        resolution.target_roll_successes = static_cast<std::uint8_t>(value);
        if (!read_field_signed_integer(state, resolution_table, "effect_value",
                                       -128, 127, signed_value) ||
            !read_field_string(state, resolution_table, "outcome",
                               resolution.outcome) ||
            !read_field_boolean(state, resolution_table, "physical",
                                resolution.physical) ||
            !read_field_integer(state, resolution_table, "physical_meter",
                                0xFFU, value)) {
            return fail(
                "actor_ai.heartbeat returned incomplete spell effect results");
        }
        resolution.effect_value = static_cast<std::int16_t>(signed_value);
        resolution.physical_meter = static_cast<std::uint8_t>(value);
        if (!read_field_integer(state, resolution_table, "mental_meter",
                                0xFFU, value)) {
            return fail(
                "actor_ai.heartbeat returned incomplete spell effect meters");
        }
        resolution.mental_meter = static_cast<std::uint8_t>(value);
        ActorFacts result_actor_facts;
        if (!read_actor_facts(state, resolution_table, result_actor_facts) ||
            !read_field_boolean(state, resolution_table,
                                "actor_lifecycle_changed",
                                resolution.source_record_flags_changed) ||
            !read_field_boolean(state, resolution_table, "changed",
                                resolution.changed)) {
            return fail(
                "actor_ai.heartbeat returned incomplete spell effect state");
        }
        resolution.source_record_flags = lifecycle_to_source(
            result_actor_facts.lifecycle,
            context.combatant.has_value()
                ? context.combatant->source_flags
                : static_cast<std::uint8_t>(0U));

        const bool target_identity_matches =
            context.instance_id == resolution.target_id ||
            (!context.controlled_party_member_id.empty() &&
             context.controlled_party_member_id == resolution.target_id);
        const bool target_context_matches = context.combatant.has_value() &&
            target_identity_matches &&
            context.source_record_slot.has_value() &&
            *context.source_record_slot == resolution.target_record_slot &&
            context.combatant->record_type == resolution.target_record_type &&
            (context.dynamic_world_encounter_actor !=
             (resolution.target_record_type < 3U)) &&
            ((resolution.target_record_type < 3U &&
              resolution.target_record_slot < 3U) ||
             (resolution.target_record_type >= 3U &&
              resolution.target_record_slot >= 3U &&
              resolution.target_record_slot <= 9U));
        const bool linked_target = target_context_matches &&
            spell_effect_target_context &&
            decision.spell_effect_id == resolution.spell_id;
        const bool meters_changed = target_context_matches &&
            (resolution.physical_meter !=
                 context.combatant->physical_meter ||
             resolution.mental_meter != context.combatant->mental_meter);
        const bool source_flags_changed = target_context_matches &&
            resolution.source_record_flags !=
                context.combatant->source_flags;
        const bool any_state_changed =
            meters_changed || source_flags_changed;
        static constexpr std::array<std::string_view, 5> kSpellEffectOutcomes{
            "source_special_target_path", "random_service_unavailable",
            "target_resisted", "effect_applied", "effect_no_change"};
        const bool known_outcome =
            std::find(kSpellEffectOutcomes.begin(), kSpellEffectOutcomes.end(),
                      resolution.outcome) != kSpellEffectOutcomes.end();
        if (!known_outcome || !linked_target ||
            resolution.physical != (resolution.spell_id != "sleep") ||
            resolution.effect_value < 0 || resolution.effect_value > 10 ||
            source_flags_changed != resolution.source_record_flags_changed ||
            any_state_changed != resolution.changed ||
            (resolution.outcome == "effect_applied" &&
             (!resolution.changed || resolution.effect_value == 0)) ||
            (resolution.outcome == "effect_no_change" &&
             (resolution.changed || resolution.effect_value == 0)) ||
            ((resolution.outcome == "target_resisted" ||
              resolution.outcome == "source_special_target_path" ||
              resolution.outcome == "random_service_unavailable") &&
             (resolution.changed || resolution.effect_value != 0))) {
            return fail(
                "actor_ai.heartbeat returned an inconsistent spell effect result");
        }
        decision.spell_effect_resolution = resolution;
        if (resolution.changed) {
            ScriptCombatMeterUpdate update{};
            update.target_id = resolution.target_id;
            update.source_record_slot = resolution.target_record_slot;
            update.record_type = resolution.target_record_type;
            update.physical_meter = resolution.physical_meter;
            update.mental_meter = resolution.mental_meter;
            update.source_record_flags = resolution.source_record_flags;
            update.source_record_flags_updated =
                resolution.source_record_flags_changed;
            update.physical = resolution.physical;
            update.changed = true;
            decision.combat_meter_update = std::move(update);
        }
    }
    lua_pop(state, 1);

    const auto read_boolean_field = [state, decision_table](
                                        const char* field,
                                        bool& value) {
        lua_getfield(state, decision_table, field);
        const bool valid = lua_isboolean(state, -1);
        if (valid) {
            value = lua_toboolean(state, -1) != 0;
        }
        lua_pop(state, 1);
        return valid;
    };
    if (!read_boolean_field("transition_started",
                            decision.transition_started) ||
        !read_boolean_field("transition_complete",
                            decision.transition_complete) ||
        !read_boolean_field("combat_action_requested",
                            decision.combat_action_requested)) {
        return fail("actor_ai.heartbeat returned invalid transition state");
    }
    if (decision.combat_action_requested &&
        (context.profile_id != "field_runner_k03_none00" ||
         context.phase != ScriptActorHeartbeatPhase::behavior ||
         !context.combat_action_available ||
         !context.combat_confirm_held || !context.combat_action_ready ||
         context.combat_input_blocked)) {
        return fail("actor_ai.heartbeat requested an unavailable player action");
    }
    if (action == "hold" &&
        (decision.animation_id == "idle" ||
         decision.animation_id == "entry" ||
         decision.animation_id == "walk" ||
         decision.animation_id == "action" ||
         decision.animation_id == "attack" ||
         decision.animation_id == "strike" ||
         decision.animation_id == "cast" ||
         decision.animation_id == "runner_down" ||
         decision.animation_id == "runner_hurt" ||
         decision.animation_id == "projectile" ||
         decision.animation_id == "hit_spark" ||
         decision.animation_id == "hurt" ||
         decision.animation_id == "down" ||
         decision.animation_id == "spell" ||
         decision.animation_id == "weapon")) {
        decision.action = ScriptActorBehaviorDecision::Action::hold;
    } else if (action == "play_entry" &&
               decision.animation_id == "entry") {
        decision.action =
            ScriptActorBehaviorDecision::Action::play_entry_animation;
    } else {
        return fail("actor_ai.heartbeat returned an unsupported action");
    }
    // Every phase returns the actor's full state snapshot. A behavior
    // heartbeat may therefore report the walk/attack pose last selected by
    // the render heartbeat without advancing that animation itself.
    const bool dynamic_creature_walk =
        (context.dynamic_world_encounter_actor ||
         context.source_actor_slot.has_value()) &&
        decision.animation_id == "walk";
    const bool dynamic_creature_action =
        context.dynamic_world_encounter_actor &&
        decision.animation_id == "action";
    // ROM 0xAFF2 -> 0xB330 / 0xA93A service every live world record in
    // slots 3..9, including the static default-actor list (playtest #9).
    const bool world_combat_record =
        context.dynamic_world_encounter_actor ||
        (context.combatant.has_value() &&
         context.combatant->record_type >= 3U);
    const bool dynamic_creature_hurt =
        world_combat_record && decision.animation_id == "hurt";
    const bool dynamic_creature_down =
        world_combat_record && decision.animation_id == "down";
    const bool source_item_action =
        (context.dynamic_world_encounter_actor ||
         context.source_default_item_action_profile) &&
        context.action_kind == ScriptActorActionKind::item &&
        decision.animation_id == "weapon";
    if ((decision.animation_id == "walk" ||
         decision.animation_id == "attack" ||
         decision.animation_id == "strike" ||
         decision.animation_id == "cast" ||
         decision.animation_id == "runner_down" ||
         decision.animation_id == "runner_hurt") &&
        context.profile_id != "field_runner_k03_none00" &&
        !dynamic_creature_walk) {
        return fail("actor_ai.heartbeat returned an unsupported actor animation");
    }
    if (decision.animation_id == "runner_down" &&
        decision.animation_frame > 7U) {
        return fail("actor_ai.heartbeat returned an unsupported C0B2 down frame");
    }
    if (decision.animation_id == "runner_hurt" &&
        decision.animation_frame > 1U) {
        return fail(
            "actor_ai.heartbeat returned an unsupported C072 hurt pose");
    }
    if (decision.animation_id == "strike" && decision.animation_frame > 1U) {
        return fail("actor_ai.heartbeat returned an unsupported C0A2 strike frame");
    }
    if (decision.animation_id == "action" &&
        !dynamic_creature_action) {
        return fail("actor_ai.heartbeat returned an unsupported target action pose");
    }
    if (decision.animation_id == "hurt" &&
        !dynamic_creature_hurt) {
        return fail("actor_ai.heartbeat returned an unsupported hit reaction pose");
    }
    if (decision.animation_id == "down") {
        if (!dynamic_creature_down) {
            return fail(
                "actor_ai.heartbeat returned an unsupported exhausted pose "
                "for a non-encounter actor");
        }
        if (!decision.actor_facts.lifecycle.downed) {
            return fail(
                "actor_ai.heartbeat returned an exhausted pose without the "
                "downed actor fact");
        }
    }
    if (decision.animation_id == "weapon" &&
        (!source_item_action ||
         decision.animation_variant != "")) {
        return fail("actor_ai.heartbeat returned an unsupported item-action animation");
    }

    lua_getfield(state, decision_table, "audio_event");
    if (lua_type(state, -1) != LUA_TNIL) {
        std::uint32_t event = 0U;
        if (!read_integer(state, -1, 0xFFFFU, event)) {
            return fail("actor_ai.heartbeat returned an invalid audio event");
        }
        decision.audio_event = static_cast<std::uint16_t>(event);
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "target_heading_audio_event");
    if (lua_type(state, -1) != LUA_TNIL) {
        std::uint32_t event = 0U;
        if (!read_integer(state, -1, 0xFFFFU, event) ||
            (event != 0x004DU && event != 0x004EU) ||
            (!(context.player_melee_available &&
               !context.dynamic_world_encounter_actor &&
               context.phase == ScriptActorHeartbeatPhase::behavior) &&
             (!context.dynamic_world_encounter_actor ||
              decision.animation_id != "action"))) {
            return fail(
                "actor_ai.heartbeat returned an invalid target-heading audio event");
        }
        decision.target_heading_audio_event =
            static_cast<std::uint16_t>(event);
    }
    lua_pop(state, 1);
    std::optional<std::uint16_t> strike_source_heading;
    if (!read_optional_source_heading_from_bearing_field(
            state, decision_table, "strike_bearing_degrees",
            strike_source_heading)) {
        return fail("actor_ai.heartbeat returned an invalid strike bearing");
    }
    if (strike_source_heading.has_value()) {
        if (!context.player_melee_available ||
            context.dynamic_world_encounter_actor ||
            context.phase != ScriptActorHeartbeatPhase::behavior) {
            return fail("actor_ai.heartbeat returned a disallowed strike bearing");
        }
        decision.strike_heading = *strike_source_heading;
    }
    lua_getfield(state, decision_table, "weapon_item_id");
    if (lua_type(state, -1) != LUA_TNIL) {
        std::string item_id;
        if (!read_string(state, -1, item_id)) {
            return fail("actor_ai.heartbeat returned an invalid weapon item");
        }
        const auto item_code = script_item_source_code_from_id(item_id);
        if (!item_code.has_value() ||
            context.action_kind != ScriptActorActionKind::item ||
            *item_code != context.action_code) {
            return fail("actor_ai.heartbeat returned an invalid weapon item");
        }
        decision.weapon_item_id = *item_code;
    }
    lua_pop(state, 1);
    std::optional<std::uint16_t> current_source_heading;
    if (!read_optional_source_heading_from_bearing_field(
            state, decision_table, "movement_current_bearing_degrees",
            current_source_heading)) {
        return fail(
            "actor_ai.heartbeat returned an invalid current bearing");
    }
    if (current_source_heading.has_value()) {
        const bool movement_actor =
            context.profile_id == "field_runner_k03_none00" ||
            context.dynamic_world_encounter_actor ||
            context.source_actor_slot.has_value();
        if (!movement_actor) {
            return fail(
                "actor_ai.heartbeat returned a current bearing for a non-mover");
        }
        decision.movement_current_heading = *current_source_heading;
    }
    std::string ai_target_id;
    if (!read_optional_field_string(state, decision_table, "ai_target_id",
                                    ai_target_id)) {
        return fail("actor_ai.heartbeat returned an invalid AI target id");
    }
    if (!ai_target_id.empty()) {
        decision.ai_target_id = std::move(ai_target_id);
    }
    std::optional<std::uint16_t> ai_target_source_heading;
    std::optional<std::uint32_t> ai_target_distance;
    if (!read_optional_source_heading_from_bearing_field(
            state, decision_table, "ai_target_bearing_degrees",
            ai_target_source_heading) ||
        !read_optional_field_u32(state, decision_table, "ai_target_distance",
                                 0xFFFFU, ai_target_distance)) {
        return fail("actor_ai.heartbeat returned invalid AI target geometry");
    }
    if (ai_target_source_heading.has_value()) {
        decision.ai_target_heading = *ai_target_source_heading;
    }
    if (ai_target_distance.has_value()) {
        decision.ai_target_distance = static_cast<std::uint16_t>(
            *ai_target_distance);
    }
    if (decision.ai_target_id.has_value() !=
            decision.ai_target_heading.has_value() ||
        decision.ai_target_id.has_value() !=
            decision.ai_target_distance.has_value()) {
        return fail("actor_ai.heartbeat returned incomplete AI target state");
    }
    if (!read_boolean_field("ai_target_engaged",
                            decision.ai_target_engaged)) {
        return fail("actor_ai.heartbeat returned invalid target engagement state");
    }
    std::uint32_t target_motion_delay = 0U;
    if (!read_field_integer(state, decision_table, "target_motion_delay",
                            6U, target_motion_delay)) {
        return fail("actor_ai.heartbeat returned invalid target motion delay");
    }
    decision.target_motion_delay =
        static_cast<std::uint8_t>(target_motion_delay);
    std::optional<std::uint16_t> movement_source_heading;
    if (!read_optional_source_heading_from_bearing_field(
            state, decision_table, "movement_bearing_degrees",
            movement_source_heading)) {
        return fail("actor_ai.heartbeat returned an invalid movement bearing");
    }
    if (movement_source_heading.has_value()) {
        const bool movement_actor =
            context.profile_id == "field_runner_k03_none00" ||
            context.dynamic_world_encounter_actor ||
            context.source_actor_slot.has_value();
        // Lua snapshots retain the selected destination bearing through the
        // separate render heartbeat; behavior passes apply it as movement.
        if (!movement_actor ||
            (context.phase == ScriptActorHeartbeatPhase::behavior &&
             !context.movement_allowed &&
             !context.source_actor_slot.has_value())) {
            return fail(
                "actor_ai.heartbeat returned an invalid movement intent");
        }
        decision.movement_heading = *movement_source_heading;
    }
    std::optional<std::uint32_t> movement_route_distance;
    if (!read_optional_field_u32(state, decision_table,
                                 "movement_route_distance", 0xFFFFU,
                                 movement_route_distance)) {
        return fail(
            "actor_ai.heartbeat returned an invalid movement route distance");
    }
    if (movement_route_distance.has_value()) {
        if ((!context.dynamic_world_encounter_actor &&
             !context.source_actor_slot.has_value()) ||
            !decision.movement_heading.has_value()) {
            return fail(
                "actor_ai.heartbeat returned route distance without a source actor route");
        }
        decision.movement_route_distance = static_cast<std::uint16_t>(
            *movement_route_distance);
    }
    lua_getfield(state, decision_table, "movement_step");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_istable(state, -1) ||
            (context.profile_id != "field_runner_k03_none00" &&
             !context.dynamic_world_encounter_actor &&
             !context.source_actor_slot.has_value()) ||
            context.phase != ScriptActorHeartbeatPhase::behavior ||
            !context.movement_allowed ||
            !context.movement_services_available) {
            return fail("actor_ai.heartbeat returned an invalid movement step");
        }
        const auto movement_table = lua_absindex(state, -1);
        ScriptActorMovementStep movement_step{};
        movement_step.state = context.movement_state;
        std::string outcome;
        if (!read_field_string(state, movement_table, "outcome", outcome) ||
            (outcome != "accepted" && outcome != "reset")) {
            return fail("actor_ai.heartbeat returned an invalid movement outcome");
        }
        movement_step.outcome = outcome == "reset"
                                    ? ScriptActorMovementStep::Outcome::reset
                                    : ScriptActorMovementStep::Outcome::accepted;
        lua_getfield(state, movement_table, "state");
        if (!lua_istable(state, -1)) {
            return fail("actor_ai.heartbeat omitted movement state");
        }
        const auto movement_state_table = lua_absindex(state, -1);
        std::uint32_t movement_value = 0U;
        const auto read_movement = [&](const char* field,
                                       const std::uint32_t maximum,
                                       auto& destination) {
            if (!read_field_integer(state, movement_state_table, field,
                                    maximum, movement_value)) {
                return false;
            }
            destination = static_cast<std::remove_reference_t<decltype(destination)>>(
                movement_value);
            return true;
        };
        std::optional<std::uint16_t> movement_source_bearing;
        std::optional<std::uint16_t> animation_source_bearing;
        if (!read_movement("position_x", 0xFFFFU,
                           movement_step.state.position_x) ||
            !read_movement("position_y", 0xFFFFU,
                           movement_step.state.position_y) ||
            !read_optional_source_heading_from_bearing_field(
                state, movement_state_table, "bearing_degrees",
                movement_source_bearing) ||
            !movement_source_bearing.has_value() ||
            !read_optional_source_heading_from_bearing_field(
                state, movement_state_table, "animation_bearing_degrees",
                animation_source_bearing) ||
            !animation_source_bearing.has_value()) {
            return fail("actor_ai.heartbeat returned incomplete movement state");
        }
        movement_step.state.heading = *movement_source_bearing;
        movement_step.state.animation_heading = *animation_source_bearing;
        std::string substate_update;
        if (!read_field_string(state, movement_state_table,
                               "substate_update", substate_update)) {
            return fail("actor_ai.heartbeat returned invalid movement substate action");
        }
        if (substate_update == "unchanged") {
            movement_step.state.substate_update =
                ScriptMovementSubstateUpdate::unchanged;
        } else if (substate_update == "direct_translation") {
            movement_step.state.substate_update =
                ScriptMovementSubstateUpdate::direct_translation;
        } else if (substate_update == "cardinal_fallback") {
            movement_step.state.substate_update =
                ScriptMovementSubstateUpdate::cardinal_fallback;
        } else {
            return fail("actor_ai.heartbeat returned unsupported movement substate action");
        }
        if (movement_step.state.substate_update ==
            ScriptMovementSubstateUpdate::cardinal_fallback) {
            std::string direction_id;
            if (!read_field_string(state, movement_state_table,
                                   "cardinal_fallback_direction",
                                   direction_id)) {
                return fail(
                    "actor_ai.heartbeat omitted cardinal fallback direction");
            }
            movement_step.state.cardinal_fallback_direction =
                cardinal_direction_from_id(direction_id);
            if (!movement_step.state.cardinal_fallback_direction.has_value()) {
                return fail(
                    "actor_ai.heartbeat returned an invalid cardinal fallback direction");
            }
        } else {
            lua_getfield(state, movement_state_table,
                         "cardinal_fallback_direction");
            const bool absent = lua_isnil(state, -1);
            lua_pop(state, 1);
            if (!absent) {
                return fail(
                    "actor_ai.heartbeat returned a direction without cardinal fallback");
            }
            movement_step.state.cardinal_fallback_direction.reset();
        }
        const auto read_movement_action = [&](const char* field, bool& value) {
            lua_getfield(state, movement_state_table, field);
            const bool valid = lua_isboolean(state, -1);
            if (valid) {
                value = lua_toboolean(state, -1) != 0;
            }
            lua_pop(state, 1);
            return valid;
        };
        if (!read_movement_action("mark_reset_after_block",
                                  movement_step.state.mark_reset_after_block) ||
            !read_movement_action("mark_directional_fallback",
                                  movement_step.state.mark_directional_fallback) ||
            !read_movement_action("clear_collision_recovery",
                                  movement_step.state.clear_collision_recovery) ||
            !read_movement_action("clear_pending_event",
                                  movement_step.state.clear_pending_event)) {
            return fail("actor_ai.heartbeat returned incomplete movement actions");
        }
        ActorBehaviorFacts route_facts;
        if (!read_actor_behavior_facts(state, movement_state_table,
                                       "route_facts", route_facts)) {
            return fail("actor_ai.heartbeat returned invalid movement route facts");
        }
        movement_step.state.route_flags = behavior_to_source(
            route_facts, context.movement_state.route_flags);
        lua_getfield(state, movement_state_table, "route_update");
        if (!lua_istable(state, -1)) {
            return fail("actor_ai.heartbeat returned invalid movement route update");
        }
        const auto route_update_table = lua_absindex(state, -1);
        std::string route_update_kind;
        if (!read_field_string(state, route_update_table, "kind",
                               route_update_kind)) {
            return fail("actor_ai.heartbeat omitted movement route update kind");
        }
        if (route_update_kind == "unchanged") {
            movement_step.state.route_update.kind =
                ScriptMovementRouteUpdate::Kind::unchanged;
        } else if (route_update_kind == "reset") {
            movement_step.state.route_update.kind =
                ScriptMovementRouteUpdate::Kind::reset;
        } else if (route_update_kind == "blocked_by_direction") {
            std::string direction_id;
            if (!read_field_string(state, route_update_table, "direction",
                                   direction_id)) {
                return fail(
                    "actor_ai.heartbeat omitted blocked route direction");
            }
            const auto direction = cardinal_direction_from_id(direction_id);
            if (!direction.has_value()) {
                return fail(
                    "actor_ai.heartbeat returned invalid blocked route direction");
            }
            movement_step.state.route_update.direction = *direction;
            movement_step.state.route_update.kind =
                ScriptMovementRouteUpdate::Kind::blocked_by_direction;
        } else {
            return fail("actor_ai.heartbeat returned unsupported movement route update");
        }
        lua_pop(state, 1);
        lua_getfield(state, movement_state_table, "collision_extent");
        if (!lua_isinteger(state, -1) || lua_tointeger(state, -1) < -32768 ||
            lua_tointeger(state, -1) > 32767) {
            return fail("actor_ai.heartbeat returned invalid collision extent");
        }
        movement_step.state.collision_extent =
            static_cast<std::int16_t>(lua_tointeger(state, -1));
        lua_pop(state, 1);
        if (impl_->last_actor_terrain_collision_code.has_value()) {
            movement_step.state.terrain_collision_code =
                *impl_->last_actor_terrain_collision_code;
        }
        lua_pop(state, 1);
        decision.movement_step = movement_step;
    }
    lua_pop(state, 1);
    if (context.movement_services_available &&
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        context.movement_allowed && decision.movement_heading.has_value() &&
        !decision.movement_step.has_value() &&
        !decision.ai_target_engaged) {
        return fail("actor_ai.heartbeat omitted the Lua movement step");
    }
    lua_getfield(state, decision_table, "projectile_movement");
    if (context.projectile_movement.has_value()) {
        if (!lua_istable(state, -1) ||
            context.profile_id != "weapon_projectile" ||
            context.phase != ScriptActorHeartbeatPhase::behavior) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile movement step");
        }
        const auto movement_table = lua_absindex(state, -1);
        ScriptActorProjectileMovementDecision movement_decision{};
        movement_decision.state = *context.projectile_movement;
        const auto read_boolean = [state, movement_table](
                                      const char* field, bool& value) {
            lua_getfield(state, movement_table, field);
            const bool valid = lua_isboolean(state, -1);
            if (valid) {
                value = lua_toboolean(state, -1) != 0;
            }
            lua_pop(state, 1);
            return valid;
        };
        const auto read_signed = [state, movement_table](
                                     const char* field, const lua_Integer min,
                                     const lua_Integer max,
                                     lua_Integer& value) {
            lua_getfield(state, movement_table, field);
            const bool valid = lua_isinteger(state, -1) &&
                               lua_tointeger(state, -1) >= min &&
                               lua_tointeger(state, -1) <= max;
            if (valid) {
                value = lua_tointeger(state, -1);
            }
            lua_pop(state, 1);
            return valid;
        };
        std::string role;
        std::string phase;
        std::uint32_t unsigned_value = 0U;
        lua_Integer signed_value = 0;
        if (!read_field_string(state, movement_table, "role", role) ||
            role != (context.projectile_movement->role ==
                             ScriptActorProjectileRole::primary
                         ? "primary"
                         : "secondary") ||
            !read_field_string(state, movement_table, "phase", phase) ||
            (phase != "inactive" && phase != "flying" &&
             phase != "awaiting_animation") ||
            !read_boolean("impact_occurred",
                          movement_decision.state.impact_occurred) ||
            !read_boolean("animation_finished",
                          movement_decision.state.animation_finished)) {
            return fail(
                "actor_ai.heartbeat returned an incomplete projectile state");
        }
        movement_decision.state.phase =
            phase == "inactive"
                ? ScriptActorProjectilePhase::inactive
                : phase == "awaiting_animation"
                    ? ScriptActorProjectilePhase::awaiting_animation
                    : ScriptActorProjectilePhase::flying;
        double position_x = 0.0;
        double position_y = 0.0;
        lua_getfield(state, movement_table, "position");
        const bool position_table_valid = lua_istable(state, -1);
        const auto position_table = position_table_valid
            ? lua_absindex(state, -1)
            : 0;
        const bool position_valid = position_table_valid &&
            read_field_number(state, position_table, "x", position_x) &&
            read_field_number(state, position_table, "y", position_y) &&
            position_x >= 0.0 && position_x < kWorldCoordinatePeriod &&
            position_y >= 0.0 && position_y < kWorldCoordinatePeriod;
        lua_pop(state, 1);
        if (!position_valid) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile map position");
        }
        movement_decision.state.position = {position_x, position_y};
        double velocity_x = 0.0;
        double velocity_y = 0.0;
        lua_getfield(state, movement_table, "velocity");
        const bool velocity_table_valid = lua_istable(state, -1);
        const auto velocity_table = velocity_table_valid
            ? lua_absindex(state, -1)
            : 0;
        const bool velocity_valid = velocity_table_valid &&
            read_field_number(state, velocity_table, "x", velocity_x) &&
            read_field_number(state, velocity_table, "y", velocity_y);
        lua_pop(state, 1);
        if (!velocity_valid) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile step vector");
        }
        movement_decision.state.velocity = {velocity_x, velocity_y};
        if (!read_field_number(state, movement_table, "remaining_flight_range",
                               movement_decision.state.remaining_flight_range) ||
            movement_decision.state.remaining_flight_range <
                96.0 ||
            movement_decision.state.remaining_flight_range >
                128.0 * 96.0) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile travel budget");
        }
        if (!read_field_integer(state, movement_table, "shots_remaining", 2U,
                                unsigned_value)) {
            return fail(
                "actor_ai.heartbeat returned an invalid burst continuation count");
        }
        movement_decision.state.shots_remaining =
            static_cast<std::uint8_t>(unsigned_value);
        if (!read_field_number(state, movement_table, "bearing_degrees",
                               movement_decision.state.bearing_degrees) ||
            movement_decision.state.bearing_degrees < 0.0 ||
            movement_decision.state.bearing_degrees >= kDegreesPerTurn) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile bearing");
        }
        double origin_x = 0.0;
        double origin_y = 0.0;
        lua_getfield(state, movement_table, "burst_origin");
        const bool origin_table_valid = lua_istable(state, -1);
        const auto origin_table = origin_table_valid
            ? lua_absindex(state, -1)
            : 0;
        const bool origin_valid = origin_table_valid &&
            read_field_number(state, origin_table, "x", origin_x) &&
            read_field_number(state, origin_table, "y", origin_y) &&
            origin_x >= 0.0 && origin_x < kWorldCoordinatePeriod &&
            origin_y >= 0.0 && origin_y < kWorldCoordinatePeriod;
        lua_pop(state, 1);
        if (!origin_valid) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile burst origin");
        }
        movement_decision.state.burst_origin = {origin_x, origin_y};
        if (!read_field_integer(state, movement_table, "attack_successes",
                                127U, unsigned_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile success count");
        }
        movement_decision.state.attack_successes =
            static_cast<std::uint16_t>(unsigned_value);
        std::string animation_action;
        if (!read_field_string(state, movement_table, "animation_action",
                               animation_action)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile animation action");
        }
        if (animation_action == "unchanged") {
            movement_decision.state.animation_action =
                ScriptActorProjectileAnimationAction::unchanged;
        } else if (animation_action == "stop") {
            movement_decision.state.animation_action =
                ScriptActorProjectileAnimationAction::stop;
        } else if (animation_action == "restart") {
            movement_decision.state.animation_action =
                ScriptActorProjectileAnimationAction::restart;
        } else {
            return fail(
                "actor_ai.heartbeat returned an unsupported projectile animation action");
        }
        lua_getfield(state, movement_table, "hit_target_id");
        if (lua_type(state, -1) == LUA_TSTRING) {
            std::size_t target_id_size = 0U;
            const auto* target_id = lua_tolstring(state, -1, &target_id_size);
            if (target_id == nullptr || target_id_size == 0U ||
                target_id_size > 96U) {
                lua_pop(state, 1);
                return fail(
                    "actor_ai.heartbeat returned an invalid projectile hit target");
            }
            movement_decision.state.hit_target_id.assign(
                target_id, target_id_size);
        } else if (lua_type(state, -1) != LUA_TNIL) {
            lua_pop(state, 1);
            return fail(
                "actor_ai.heartbeat returned an invalid projectile hit target");
        }
        lua_pop(state, 1);
        std::string outcome;
        if (!read_field_string(state, movement_table, "outcome", outcome)) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile outcome");
        }
        if (outcome == "none") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::none;
        } else if (outcome == "stopped") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::stopped;
        } else if (outcome == "retired") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::retired;
        } else if (outcome == "hit_resolution_pending") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::hit_resolution_pending;
        } else if (outcome == "linked_followup_pending") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::linked_followup_pending;
        } else if (outcome == "burst_followup_restarted") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::burst_followup_restarted;
        } else {
            return fail(
                "actor_ai.heartbeat returned an unsupported projectile outcome");
        }
        decision.projectile_movement = movement_decision;
    } else if (lua_type(state, -1) != LUA_TNIL) {
        return fail(
            "actor_ai.heartbeat returned unexpected projectile movement state");
    }
    lua_pop(state, 1);
    if (context.action_kind == ScriptActorActionKind::item &&
        !decision.weapon_item_id.has_value()) {
        return fail("actor_ai.heartbeat omitted the source item action");
    }

    std::string source_action_dispatch;
    if (!read_optional_field_string(state, decision_table,
                                    "source_action_dispatch",
                                    source_action_dispatch)) {
        return fail("actor_ai.heartbeat returned an invalid action dispatch");
    }
    if (!source_action_dispatch.empty()) {
        const bool spell_context =
            context.action_kind == ScriptActorActionKind::spell;
        const bool item_context =
            context.action_kind == ScriptActorActionKind::item;
        const bool shared_label =
            source_action_dispatch == "waiting" ||
            source_action_dispatch == "blocked" ||
            source_action_dispatch == "offscreen";
        const bool spell_label =
            source_action_dispatch == "spell" ||
            source_action_dispatch == "suppressed";
        const bool item_label =
            source_action_dispatch == "item_effect" ||
            source_action_dispatch == "item_gated" ||
            source_action_dispatch == "item_empty" ||
            source_action_dispatch == "item_pose_gated" ||
            source_action_dispatch == "item_target_heading_unresolved" ||
            source_action_dispatch == "item_spawn_requested";
        const bool label_matches =
            (shared_label && (spell_context || item_context)) ||
            (spell_label && spell_context) ||
            (item_label && item_context);
        const bool player_spell_label =
            source_action_dispatch == "player_spell" ||
            source_action_dispatch == "player_spell_blocked" ||
            source_action_dispatch == "player_spell_no_target" ||
            source_action_dispatch == "player_spell_suppressed";
        const bool player_melee_label =
            source_action_dispatch == "player_melee" ||
            source_action_dispatch == "player_melee_hit";
        const bool player_label_matches =
            ((player_spell_label && context.player_spell_available) ||
             (player_melee_label && context.player_melee_available)) &&
            !context.dynamic_world_encounter_actor &&
            context.phase == ScriptActorHeartbeatPhase::behavior;
        const bool default_item_action_label_matches =
            context.source_default_item_action_profile && item_context &&
            context.phase == ScriptActorHeartbeatPhase::behavior;
        if (player_label_matches) {
            // Field-player 0xAE6E dispatch; validated below.
        } else if (!label_matches ||
            (!context.dynamic_world_encounter_actor &&
             !default_item_action_label_matches) ||
            context.phase != ScriptActorHeartbeatPhase::behavior) {
            return fail("actor_ai.heartbeat returned an invalid action dispatch");
        }
        if (source_action_dispatch == "item_spawn_requested") {
            const auto linked_target =
                decision.ai_target_id.has_value()
                    ? std::find_if(
                          context.ai_target_candidates.begin(),
                          context.ai_target_candidates.end(),
                          [&decision](const auto& candidate) {
                              return candidate.id == *decision.ai_target_id;
                          })
                    : context.ai_target_candidates.end();
            const bool target_has_source_pose =
                linked_target != context.ai_target_candidates.end() &&
                linked_target->source_record_slot.has_value() &&
                linked_target->source_record_heading.has_value();
            const bool invalid_item_action =
                context.action_kind != ScriptActorActionKind::item ||
                context.action_code == 0U || context.action_code > 12U ||
                context.ammunition_remaining == 0U ||
                !context.can_start_item_action ||
                (context.world_scene_state == 0U &&
                 !decision.actor_facts.behavior.in_target_band) ||
                context.source_projection_active != true ||
                !context.action_line_clear || !target_has_source_pose;
            if (invalid_item_action) {
                return fail(
                    "actor_ai.heartbeat requested an item outside its ROM action window "
                    "(kind=" + std::to_string(static_cast<unsigned>(
                                      context.action_kind)) +
                    ",code=" + std::to_string(context.action_code) +
                    ",ammo=" + std::to_string(context.ammunition_remaining) +
                    ",can_start_item_action=" +
                        (context.can_start_item_action ? "yes" : "no") +
                    ",scene=" + std::to_string(context.world_scene_state) +
                    ",target_band=" +
                        (decision.actor_facts.behavior.in_target_band
                             ? "yes" : "no") +
                    ",projection=" +
                        (context.source_projection_active ? "active" : "inactive") +
                    ",line_probe=" + (context.action_line_clear ? "available" : "missing") +
                    ",target=" + (decision.ai_target_id.has_value()
                                       ? *decision.ai_target_id
                                       : "<missing>") +
                    ",target_source_pose=" +
                        (target_has_source_pose ? "yes" : "no") + ")");
            }
        }
        decision.source_action_dispatch = std::move(source_action_dispatch);
    }
    lua_getfield(state, decision_table, "hit_spark_spawn_request");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_istable(state, -1)) {
            return fail("actor_ai.heartbeat returned an invalid hit-spark spawn request");
        }
        const auto request_table = lua_absindex(state, -1);
        ScriptActorHitSparkSpawnRequest request{};
        std::uint32_t world_x = 0U;
        std::uint32_t world_y = 0U;
        if (!read_field_string(state, request_table, "target_id",
                               request.target_id) ||
            !valid_identifier(request.target_id) ||
            !read_field_integer(state, request_table, "world_x", 0xFFFFU,
                                world_x) ||
            !read_field_integer(state, request_table, "world_y", 0xFFFFU,
                                world_y)) {
            return fail("actor_ai.heartbeat returned incomplete hit-spark target facts");
        }
        request.world_x = static_cast<std::uint16_t>(world_x);
        request.world_y = static_cast<std::uint16_t>(world_y);
        decision.hit_spark_spawn_request = std::move(request);
    }
    lua_pop(state, 1);
    const bool spell_pose = decision.animation_id == "spell";
    static constexpr std::array<std::uint16_t, 14> kSpellActionAudioEvents{
        0U, 0x0032U, 0x0034U, 0x0034U, 0x0031U, 0x0037U, 0x0038U,
        0U, 0x0033U, 0x0035U, 0x0035U, 0x0039U, 0x003AU, 0x0036U};
    bool spell_action_audio_matches = false;
    if (decision.spell_action_resolution.has_value()) {
        const auto& resolution = *decision.spell_action_resolution;
        const auto table_audio =
            resolution.source_action_code < kSpellActionAudioEvents.size()
                ? kSpellActionAudioEvents[resolution.source_action_code]
                : 0U;
        // ROM 0x49A2 emits event 0x3B only after 0x1582A allocates the
        // directional field-barrier record; a full pool stays silent [C].
        const auto expected_audio =
            resolution.field_effect_kind ==
                    ScriptActorSpellFieldEffectKind::field_barrier
                ? (resolution.effect_started ? 0x003BU : 0U)
                : table_audio;
        spell_action_audio_matches =
            (!resolution.effect_started && !decision.audio_event.has_value()) ||
            (resolution.effect_started &&
             ((expected_audio == 0U && !decision.audio_event.has_value()) ||
             (expected_audio != 0U && decision.audio_event.has_value() &&
              *decision.audio_event == expected_audio)));
    }
    if (spell_pose &&
        (!context.dynamic_world_encounter_actor ||
         context.action_kind != ScriptActorActionKind::spell ||
         decision.animation_frame != 0U ||
         decision.target_heading_audio_event.has_value() ||
         (decision.combat_meter_update.has_value() &&
          !decision.spell_effect_resolution.has_value()) ||
         decision.combat_action_requested ||
         decision.ai_target_engaged ||
         decision.movement_step.has_value() ||
         decision.movement_heading.has_value() ||
         (decision.spell_action_resolution.has_value() &&
          !spell_action_audio_matches))) {
        return fail("actor_ai.heartbeat returned an unsupported spell pose");
    }
    if (decision.source_action_dispatch.has_value() &&
        *decision.source_action_dispatch == "spell" &&
        (!spell_pose || decision.action_delay < 0x0F)) {
        return fail("actor_ai.heartbeat returned an unsupported spell dispatch");
    }
    const bool player_spell_cast =
        context.player_spell_available &&
        !context.dynamic_world_encounter_actor &&
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        decision.source_action_dispatch.has_value() &&
        *decision.source_action_dispatch == "player_spell" &&
        decision.spell_action_resolution.has_value() &&
        spell_action_audio_matches && !spell_pose;
    if (decision.spell_action_resolution.has_value() && !player_spell_cast &&
        (!spell_pose || !decision.source_action_dispatch.has_value() ||
         *decision.source_action_dispatch != "spell" ||
         !spell_action_audio_matches)) {
        return fail(
            "actor_ai.heartbeat returned a spell result outside its cast dispatch");
    }
    if (decision.source_action_dispatch.has_value() &&
        (*decision.source_action_dispatch == "item_effect" ||
         *decision.source_action_dispatch == "item_gated" ||
         *decision.source_action_dispatch == "item_empty" ||
         *decision.source_action_dispatch == "item_pose_gated" ||
         *decision.source_action_dispatch ==
             "item_target_heading_unresolved" ||
         *decision.source_action_dispatch == "item_spawn_requested") &&
        (decision.animation_id == "action" ||
         decision.animation_id == "spell" ||
         decision.combat_meter_update.has_value() ||
         decision.combat_action_requested ||
         decision.ai_target_engaged)) {
        return fail("actor_ai.heartbeat returned an unsupported item dispatch");
    }
    if (decision.source_action_dispatch.has_value() &&
        *decision.source_action_dispatch == "item_spawn_requested" &&
        decision.animation_id == "weapon") {
        return fail("actor_ai.heartbeat spawned from an active C042 pose");
    }
    lua_pop(state, 1);

    const bool begins_entry = decision.action ==
        ScriptActorBehaviorDecision::Action::play_entry_animation;
    const bool projectile_behavior = context.projectile_movement.has_value();
    const bool begins_dynamic_target_action =
        decision.animation_id == "action" &&
        decision.audio_event.has_value() &&
        context.dynamic_world_encounter_actor;
    const bool dynamic_actor_behavior_audio =
        decision.audio_event.has_value() &&
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        (context.dynamic_world_encounter_actor ||
         (context.combatant.has_value() &&
          context.combatant->record_type >= 3U));
    const bool spell_action_audio =
        (spell_pose && decision.spell_action_resolution.has_value() &&
         spell_action_audio_matches) ||
        player_spell_cast;
    // ROM 0xAFF2 selects the kind-3 C072 pose for a nonzero physical meter;
    // its 0xB07E continuation emits the 0xB03E (0x40) cue after eight actor
    // services. Animation can return to walk/idle before that delayed cue, so
    // identify it by its runner behavior service and source audio event.
    const bool player_hurt_audio =
        context.instance_id == "field_player_runner" &&
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        decision.audio_event == std::optional<std::uint16_t>{0x0040U};
    // ROM 0xB1DC: the player strike always requests 0x4C/0x55; only an
    // in-range strike adds the 0x4D/0x4E heading cue.
    const bool player_melee_dispatch =
        context.player_melee_available &&
        !context.dynamic_world_encounter_actor &&
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        decision.source_action_dispatch.has_value() &&
        (*decision.source_action_dispatch == "player_melee" ||
         *decision.source_action_dispatch == "player_melee_hit");
    const bool player_melee_hit = player_melee_dispatch &&
        *decision.source_action_dispatch == "player_melee_hit";
    if (player_melee_dispatch &&
        (!decision.audio_event.has_value() ||
         (*decision.audio_event != 0x004CU &&
          *decision.audio_event != 0x0055U) ||
         (decision.combat_meter_update.has_value() && !player_melee_hit))) {
        return fail("actor_ai.heartbeat returned an invalid player strike");
    }
    // ROM 0xB1DC is also entered from the 0x141D8 enemy accept path, so a
    // changed dynamic-actor melee reaches the same 0xB29E child init [C].
    const bool actor_melee_hit = context.dynamic_world_encounter_actor &&
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        decision.animation_id == "action" &&
        decision.combat_meter_update.has_value() &&
        decision.combat_meter_update->changed;
    // ROM 0x13FFE -> 0xB11C: the Hell Hound close effect resolves 0x55B16 and
    // allocates the 0xB146 child (same init) only on changed meters [C].
    const bool actor_close_effect_hit =
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        decision.target_motion_effect_slot_request.has_value() &&
        decision.combat_meter_update.has_value() &&
        decision.combat_meter_update->changed;
    const bool hit_spark_expected = actor_melee_hit || actor_close_effect_hit ||
        (player_melee_hit &&
         decision.combat_meter_update.has_value() &&
         decision.combat_meter_update->changed);
    if (hit_spark_expected != decision.hit_spark_spawn_request.has_value()) {
        return fail("actor_ai.heartbeat returned a hit-spark request inconsistent with the changed strike");
    }
    if (decision.hit_spark_spawn_request.has_value()) {
        const auto& request = *decision.hit_spark_spawn_request;
        const auto target = std::find_if(
            context.ai_target_candidates.begin(),
            context.ai_target_candidates.end(),
            [&request](const auto& candidate) {
                return candidate.id == request.target_id &&
                       candidate.world_x == request.world_x &&
                       candidate.world_y == request.world_y;
            });
        if (!hit_spark_expected || !decision.combat_meter_update.has_value() ||
            !decision.combat_meter_update->changed ||
            decision.combat_meter_update->target_id != request.target_id ||
            target == context.ai_target_candidates.end()) {
            return fail("actor_ai.heartbeat requested a hit spark outside the resolved strike target");
        }
    }
    const bool hit_spark_actor = context.profile_id == "field_hit_spark";
    if (decision.hit_spark_retired &&
        (!hit_spark_actor ||
         context.phase != ScriptActorHeartbeatPhase::behavior)) {
        return fail("actor_ai.heartbeat retired a hit spark outside its behavior pass");
    }
    if (hit_spark_actor) {
        if (context.phase == ScriptActorHeartbeatPhase::render &&
            decision.animation_id != "hit_spark") {
            return fail("actor_ai.heartbeat returned an invalid hit-spark frame");
        }
        if (context.phase == ScriptActorHeartbeatPhase::behavior &&
            decision.hit_spark_retired != decision.animation_complete) {
            return fail("actor_ai.heartbeat retirement did not follow hit-spark completion");
        }
    } else if (decision.animation_id == "hit_spark") {
        return fail("actor_ai.heartbeat used hit-spark state outside its source profile");
    }
    if (((begins_entry != decision.audio_event.has_value()) &&
         !projectile_behavior && !begins_dynamic_target_action &&
         !dynamic_actor_behavior_audio && !spell_action_audio &&
         !player_melee_dispatch && !player_hurt_audio) ||
        ((begins_dynamic_target_action || player_melee_hit) !=
         decision.target_heading_audio_event.has_value()) ||
        (decision.animation_id == "action" &&
         decision.audio_event.has_value() &&
         !begins_dynamic_target_action) ||
        (projectile_behavior && decision.audio_event.has_value() &&
         *decision.audio_event != 0x003CU) ||
        (decision.projectile_movement.has_value() &&
         decision.projectile_movement->state.role ==
             ScriptActorProjectileRole::secondary &&
         (decision.projectile_movement->outcome ==
              ScriptActorProjectileOutcome::linked_followup_pending ||
          decision.projectile_movement->outcome ==
              ScriptActorProjectileOutcome::hit_resolution_pending ||
          decision.projectile_movement->outcome ==
              ScriptActorProjectileOutcome::burst_followup_restarted)) ||
        (decision.transition_complete && !decision.transition_started) ||
        (decision.animation_id == "entry" &&
         !decision.transition_started) ||
        (decision.animation_id == "idle" &&
         decision.animation_frame != 0U) ||
        (begins_entry && !decision.transition_started)) {
        const auto reason =
            "actor_ai.heartbeat returned inconsistent actor state: actor=" +
            std::string(context.instance_id) + " profile=" +
            std::string(context.profile_id) + " phase=" +
            (context.phase == ScriptActorHeartbeatPhase::behavior
                 ? std::string("behavior")
                 : std::string("render")) +
            " animation=" + decision.animation_id +
            " animation_frame=" +
            std::to_string(decision.animation_frame) +
            " transition_started=" +
            (decision.transition_started ? "true" : "false") +
            " transition_complete=" +
            (decision.transition_complete ? "true" : "false") +
            " action=" +
            (begins_entry ? "entry" : "hold") + " audio=" +
            (decision.audio_event.has_value() ? "present" : "none");
        return fail(reason);
    }
    impl_->failure.clear();
    if (impl_->actor_heartbeat_observer) {
        impl_->actor_heartbeat_observer(actor_id, context.profile_id,
                                        context.phase);
    }
    return decision;
}

std::optional<bool> GameScriptRuntime::has_world_scene_effect_target(
    const std::vector<std::string>& actor_ids) {
    auto* state = impl_->state;
    const auto stack_base = state == nullptr ? 0 : lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<bool> {
        if (state != nullptr) {
            lua_settop(state, stack_base);
        }
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (state == nullptr) {
        return fail("gameplay Lua actor state is unavailable");
    }
    if (!impl_->push_module("actor_ai")) {
        return fail("gameplay Lua actor AI module is missing");
    }
    lua_getfield(state, -1, "has_world_scene_effect_target");
    if (!lua_isfunction(state, -1)) {
        return fail("actor_ai.has_world_scene_effect_target is missing");
    }
    lua_remove(state, -2);
    lua_createtable(state, static_cast<int>(actor_ids.size()), 0);
    for (std::size_t index = 0U; index < actor_ids.size(); ++index) {
        const auto& actor_id = actor_ids[index];
        if (!actor_id.empty() && !valid_identifier(actor_id)) {
            return fail("world scene effect actor ID is invalid");
        }
        lua_pushlstring(state, actor_id.data(), actor_id.size());
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("actor_ai.has_world_scene_effect_target: " +
                    stack_error(state));
    }
    if (!lua_isboolean(state, -1)) {
        return fail(
            "actor_ai.has_world_scene_effect_target returned invalid actor state");
    }
    const bool result = lua_toboolean(state, -1) != 0;
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<ScriptPartyRunnerDownDecision>
GameScriptRuntime::party_runner_down(
    const ScriptPartyRunnerDownContext& context) {
    auto* state = impl_->state;
    const auto stack_base = state == nullptr ? 0 : lua_gettop(state);
    const auto fail = [this, state, stack_base](
                          const std::string_view reason)
        -> std::optional<ScriptPartyRunnerDownDecision> {
        if (state != nullptr) {
            lua_settop(state, stack_base);
        }
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (state == nullptr) {
        return fail("gameplay Lua party state is unavailable");
    }

    static constexpr std::array<std::string_view, 3> kRoleIds{
        "leader", "companion_one", "companion_two"};
    const auto target_index = static_cast<std::size_t>(context.target);
    if (target_index >= kRoleIds.size()) {
        return fail("party runner-down target role is invalid");
    }
    if (!impl_->push_module("party_down")) {
        return fail("gameplay Lua party_down module is missing");
    }
    lua_getfield(state, -1, "enter_runner_down_state");
    if (!lua_isfunction(state, -1)) {
        return fail("party_down.enter_runner_down_state is missing");
    }
    lua_remove(state, -2);

    lua_createtable(state, 0, 3);
    lua_pushlstring(state, kRoleIds[target_index].data(),
                    kRoleIds[target_index].size());
    lua_setfield(state, -2, "target_role");
    lua_createtable(state, 0, 2);
    lua_pushinteger(state, context.physical_meter);
    lua_setfield(state, -2, "physical");
    lua_pushinteger(state, context.mental_meter);
    lua_setfield(state, -2, "mental");
    lua_setfield(state, -2, "target_meters");
    lua_createtable(state, 0, static_cast<int>(context.party.size()));
    for (std::size_t index = 0U; index < context.party.size(); ++index) {
        lua_createtable(state, 0, 3);
        lua_pushboolean(state, context.party[index].active);
        lua_setfield(state, -2, "active");
        lua_pushboolean(state, context.party[index].downed);
        lua_setfield(state, -2, "downed");
        lua_pushboolean(state, context.party[index].dead);
        lua_setfield(state, -2, "dead");
        lua_setfield(state, -2, kRoleIds[index].data());
    }
    lua_setfield(state, -2, "party");

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("party_down.enter_runner_down_state: " +
                    stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("party_down.enter_runner_down_state returned no decision");
    }

    const auto read_boolean_field = [state](const int table_index,
                                             const char* field,
                                             bool& value) {
        lua_getfield(state, table_index, field);
        const bool valid = lua_isboolean(state, -1) != 0;
        if (valid) value = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        return valid;
    };
    ScriptPartyRunnerDownDecision result{};
    lua_getfield(state, -1, "lifecycle");
    if (!lua_istable(state, -1) ||
        !read_boolean_field(-1, "downed", result.downed)) {
        return fail("party runner-down lifecycle result is invalid");
    }
    lua_pop(state, 1);
    lua_getfield(state, -1, "recovery");
    if (!lua_istable(state, -1) ||
        !read_boolean_field(-1, "party_wipe_imminent",
                            result.party_wipe_imminent)) {
        return fail("party runner-down recovery result is invalid");
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<ScriptPartyWipeRecoveryDecision>
GameScriptRuntime::party_wipe_recovery(
    const ScriptPartyWipeRecoveryContext& context) {
    auto* state = impl_->state;
    const auto stack_base = state == nullptr ? 0 : lua_gettop(state);
    const auto fail = [this, state, stack_base](
                          const std::string_view reason)
        -> std::optional<ScriptPartyWipeRecoveryDecision> {
        if (state != nullptr) {
            lua_settop(state, stack_base);
        }
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (state == nullptr) {
        return fail("gameplay Lua party recovery state is unavailable");
    }

    static constexpr std::array<std::string_view, 3> kRoleIds{
        "leader", "companion_one", "companion_two"};
    if (!impl_->push_module("party_down")) {
        return fail("gameplay Lua party_down module is missing");
    }
    lua_getfield(state, -1, "resolve_party_wipe");
    if (!lua_isfunction(state, -1)) {
        return fail("party_down.resolve_party_wipe is missing");
    }
    lua_remove(state, -2);

    lua_createtable(state, 0, 3);
    lua_createtable(state, 0, static_cast<int>(context.party.size()));
    for (std::size_t index = 0U; index < context.party.size(); ++index) {
        lua_createtable(state, 0, 2);
        lua_createtable(state, 0, 3);
        lua_pushboolean(state, context.party[index].lifecycle.active);
        lua_setfield(state, -2, "active");
        lua_pushboolean(state, context.party[index].lifecycle.downed);
        lua_setfield(state, -2, "downed");
        lua_pushboolean(state, context.party[index].lifecycle.dead);
        lua_setfield(state, -2, "dead");
        lua_setfield(state, -2, "lifecycle");
        lua_createtable(state, 0, 2);
        lua_pushinteger(state, context.party[index].physical_meter);
        lua_setfield(state, -2, "physical");
        lua_pushinteger(state, context.party[index].mental_meter);
        lua_setfield(state, -2, "mental");
        lua_setfield(state, -2, "meters");
        lua_setfield(state, -2, kRoleIds[index].data());
    }
    lua_setfield(state, -2, "party");
    lua_createtable(state, 0, 1);
    lua_pushinteger(state, context.nuyen);
    lua_setfield(state, -2, "nuyen");
    lua_setfield(state, -2, "wallet");
    lua_pushboolean(state, context.motel_bill_paid);
    lua_setfield(state, -2, "motel_bill_paid");

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("party_down.resolve_party_wipe: " + stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("party_down.resolve_party_wipe returned no decision");
    }
    const auto root_index = lua_absindex(state, -1);
    const auto read_boolean_field = [state](const int table_index,
                                             const char* field,
                                             bool& value) {
        lua_getfield(state, table_index, field);
        const bool valid = lua_isboolean(state, -1) != 0;
        if (valid) value = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        return valid;
    };
    const auto read_meter_field = [state](const int table_index,
                                           const char* field,
                                           std::uint8_t& value) {
        lua_getfield(state, table_index, field);
        std::uint32_t parsed = 0U;
        const bool valid = read_integer(state, -1, 0xFFU, parsed);
        lua_pop(state, 1);
        if (valid) value = static_cast<std::uint8_t>(parsed);
        return valid;
    };

    ScriptPartyWipeRecoveryDecision result{};
    lua_getfield(state, root_index, "leader");
    if (!lua_istable(state, -1)) {
        return fail("party wipe leader result is invalid");
    }
    const auto leader_index = lua_absindex(state, -1);
    lua_getfield(state, leader_index, "lifecycle");
    if (!lua_istable(state, -1)) {
        return fail("party wipe leader lifecycle is invalid");
    }
    const auto leader_lifecycle_index = lua_absindex(state, -1);
    if (!read_boolean_field(leader_lifecycle_index, "active",
                            result.leader_active) ||
        !read_boolean_field(leader_lifecycle_index, "downed",
                            result.leader_downed) ||
        !read_boolean_field(leader_lifecycle_index, "dead",
                            result.leader_dead)) {
        return fail("party wipe leader lifecycle facts are invalid");
    }
    lua_getfield(state, leader_index, "meters");
    if (!lua_istable(state, -1)) {
        return fail("party wipe leader meters are invalid");
    }
    const auto leader_meters_index = lua_absindex(state, -1);
    if (!read_meter_field(leader_meters_index, "physical",
                          result.leader_physical_meter) ||
        !read_meter_field(leader_meters_index, "mental",
                          result.leader_mental_meter)) {
        return fail("party wipe leader meter values are invalid");
    }

    lua_getfield(state, root_index, "companions");
    if (!lua_istable(state, -1)) {
        return fail("party wipe companion results are invalid");
    }
    const auto companions_index = lua_absindex(state, -1);
    for (std::size_t index = 0U; index < result.dismiss_companions.size();
         ++index) {
        const auto role_id = kRoleIds[index + 1U];
        lua_getfield(state, companions_index, role_id.data());
        if (!lua_istable(state, -1)) {
            return fail("party wipe companion result is invalid");
        }
        if (!read_boolean_field(-1, "dismissed",
                                result.dismiss_companions[index]) ||
            !read_meter_field(-1, "physical_meter",
                              result.companion_physical_meters[index])) {
            return fail("party wipe companion decision is invalid");
        }
        lua_pop(state, 1);
    }

    lua_getfield(state, root_index, "wallet");
    if (!lua_istable(state, -1)) {
        return fail("party wipe wallet result is invalid");
    }
    lua_getfield(state, -1, "nuyen");
    std::uint32_t remaining_nuyen = 0U;
    const bool valid_nuyen = read_integer(
        state, -1, std::numeric_limits<std::uint32_t>::max(),
        remaining_nuyen);
    lua_pop(state, 1);
    if (!valid_nuyen) {
        return fail("party wipe wallet amount is invalid");
    }
    result.remaining_nuyen = remaining_nuyen;

    lua_getfield(state, root_index, "wake_destination");
    std::string wake_destination;
    if (!read_string(state, -1, wake_destination)) {
        return fail("party wipe destination is invalid");
    }
    if (wake_destination == "docwagon") {
        result.wake_destination = ScriptPartyWakeDestination::docwagon;
    } else if (wake_destination == "little_chiba") {
        result.wake_destination = ScriptPartyWakeDestination::little_chiba;
    } else {
        return fail("party wipe destination is unknown");
    }

    if (!result.leader_active || result.leader_downed || result.leader_dead) {
        return fail("party wipe did not restore the leader lifecycle");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<ScriptPartyRunnerRefreshDecision>
GameScriptRuntime::party_runner_refresh(
    const ScriptPartyRunnerRefreshContext& context) {
    auto* state = impl_->state;
    const auto stack_base = state == nullptr ? 0 : lua_gettop(state);
    const auto fail = [this, state, stack_base](
                          const std::string_view reason)
        -> std::optional<ScriptPartyRunnerRefreshDecision> {
        if (state != nullptr) {
            lua_settop(state, stack_base);
        }
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (state == nullptr) {
        return fail("gameplay Lua party roster state is unavailable");
    }

    static constexpr std::array<std::string_view, 3> kRoleIds{
        "leader", "companion_one", "companion_two"};
    const auto previous_index =
        static_cast<std::size_t>(context.previous_role);
    if (previous_index >= kRoleIds.size()) {
        return fail("party roster previous role is invalid");
    }
    if (!impl_->push_module("party_down")) {
        return fail("gameplay Lua party_down module is missing");
    }
    lua_getfield(state, -1, "refresh_runner_selection");
    if (!lua_isfunction(state, -1)) {
        return fail("party_down.refresh_runner_selection is missing");
    }
    lua_remove(state, -2);

    lua_createtable(state, 0, 3);
    lua_pushlstring(state, kRoleIds[previous_index].data(),
                    kRoleIds[previous_index].size());
    lua_setfield(state, -2, "previous_role");
    lua_pushboolean(state, context.suppression_override);
    lua_setfield(state, -2, "suppression_override");
    lua_createtable(state, 0, static_cast<int>(context.party.size()));
    for (std::size_t index = 0U; index < context.party.size(); ++index) {
        lua_createtable(state, 0, 3);
        lua_createtable(state, 0, 3);
        lua_pushboolean(state, context.party[index].lifecycle.active);
        lua_setfield(state, -2, "active");
        lua_pushboolean(state, context.party[index].lifecycle.downed);
        lua_setfield(state, -2, "downed");
        lua_pushboolean(state, context.party[index].lifecycle.dead);
        lua_setfield(state, -2, "dead");
        lua_setfield(state, -2, "lifecycle");
        lua_pushboolean(state, context.party[index].selection_pending);
        lua_setfield(state, -2, "selection_pending");
        lua_pushboolean(state, context.party[index].ends_roster);
        lua_setfield(state, -2, "ends_roster");
        lua_setfield(state, -2, kRoleIds[index].data());
    }
    lua_setfield(state, -2, "party");

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("party_down.refresh_runner_selection: " +
                    stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("party_down.refresh_runner_selection returned no decision");
    }
    const auto root_index = lua_absindex(state, -1);
    const auto read_boolean_field = [state](const int table_index,
                                             const char* field,
                                             bool& value) {
        lua_getfield(state, table_index, field);
        const bool valid = lua_isboolean(state, -1) != 0;
        if (valid) value = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        return valid;
    };

    std::string active_role;
    lua_getfield(state, root_index, "active_role");
    if (!read_string(state, -1, active_role)) {
        return fail("party roster selected role is invalid");
    }
    std::optional<std::size_t> selected_index;
    for (std::size_t index = 0U; index < kRoleIds.size(); ++index) {
        if (active_role == kRoleIds[index]) {
            selected_index = index;
            break;
        }
    }
    if (!selected_index.has_value()) {
        return fail("party roster selected an unknown role");
    }

    ScriptPartyRunnerRefreshDecision result{};
    result.active_role =
        static_cast<ScriptPartyMemberRole>(*selected_index);
    if (!read_boolean_field(root_index, "active_role_changed",
                            result.active_role_changed)) {
        return fail("party roster active-role change result is invalid");
    }
    lua_getfield(state, root_index, "selection_pending");
    if (!lua_istable(state, -1)) {
        return fail("party roster selection indicators are invalid");
    }
    const auto markers_index = lua_absindex(state, -1);
    for (std::size_t index = 0U; index < result.selection_pending.size();
         ++index) {
        if (!read_boolean_field(markers_index, kRoleIds[index].data(),
                                result.selection_pending[index])) {
            return fail("party roster selection indicator is invalid");
        }
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<ScriptPartyWipeRunFailureDecision>
GameScriptRuntime::prepare_party_wipe_run_failure(
    const ScriptPartyWipeRunFailureContext& context) {
    auto* state = impl_->state;
    const auto stack_base = state == nullptr ? 0 : lua_gettop(state);
    const auto fail = [this, state, stack_base](
                          const std::string_view reason)
        -> std::optional<ScriptPartyWipeRunFailureDecision> {
        if (state != nullptr) {
            lua_settop(state, stack_base);
        }
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (state == nullptr) {
        return fail("gameplay Lua party run-failure state is unavailable");
    }
    if (!impl_->push_module("party_down")) {
        return fail("gameplay Lua party_down module is missing");
    }
    lua_getfield(state, -1, "prepare_run_failure");
    if (!lua_isfunction(state, -1)) {
        return fail("party_down.prepare_run_failure is missing");
    }
    lua_remove(state, -2);

    lua_createtable(state, 0, 1);
    lua_pushboolean(state, context.world_activity_scene_reset);
    lua_setfield(state, -2, "world_activity_scene_reset");

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        return fail("party_down.prepare_run_failure: " + stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("party_down.prepare_run_failure returned no decision");
    }
    const auto root_index = lua_absindex(state, -1);
    const auto read_boolean_field = [state](const int table_index,
                                             const char* field,
                                             bool& value) {
        lua_getfield(state, table_index, field);
        const bool valid = lua_isboolean(state, -1) != 0;
        if (valid) value = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        return valid;
    };
    ScriptPartyWipeRunFailureDecision result{};
    if (!read_boolean_field(root_index, "page_pending", result.page_pending) ||
        !read_boolean_field(root_index, "clear_high_effects",
                             result.clear_high_effects) ||
        !read_boolean_field(root_index, "clear_world_activity_request",
                             result.clear_world_activity_request) ||
        !read_boolean_field(root_index, "clear_player_spawn_request",
                             result.clear_player_spawn_request)) {
        return fail("party run-failure boolean result is invalid");
    }
    lua_getfield(state, root_index, "objective_result");
    std::string objective_result;
    if (!read_string(state, -1, objective_result)) {
        return fail("party run-failure objective result is invalid");
    }
    if (objective_result == "unchanged") {
        result.objective_result = ScriptPartyWipeObjectiveResult::unchanged;
    } else if (objective_result == "cleared") {
        result.objective_result = ScriptPartyWipeObjectiveResult::cleared;
    } else if (objective_result == "unavailable") {
        result.objective_result = ScriptPartyWipeObjectiveResult::unavailable;
    } else {
        return fail("party run-failure objective result is unknown");
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<std::int16_t> GameScriptRuntime::player_weapon_action_delay(
    const std::uint8_t primary_attribute,
    const std::uint8_t secondary_attribute,
    const std::uint16_t capability_flags, const std::uint8_t weapon_code,
    const ScriptPlayerDelayPurpose purpose) {
    auto* state = impl_->state;
    const auto stack_base = state == nullptr ? 0 : lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<std::int16_t> {
        if (state != nullptr) {
            lua_settop(state, stack_base);
        }
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (state == nullptr) {
        return fail("gameplay Lua player delay state is unavailable");
    }

    std::string_view use;
    switch (purpose) {
    case ScriptPlayerDelayPurpose::weapon_attack:
        use = "attack";
        break;
    case ScriptPlayerDelayPurpose::reload_action:
        use = "reload_action";
        break;
    case ScriptPlayerDelayPurpose::shared_action:
        use = "shared_action";
        break;
    default:
        return fail("player weapon delay purpose is invalid");
    }

    if (!impl_->push_module("actor.delay")) {
        return fail("gameplay Lua actor.delay module is missing");
    }
    lua_getfield(state, -1, "player_weapon_action_delay");
    if (!lua_isfunction(state, -1)) {
        return fail("actor.delay.player_weapon_action_delay is missing");
    }
    lua_remove(state, -2);
    lua_pushinteger(state, primary_attribute);
    lua_pushinteger(state, secondary_attribute);
    lua_pushinteger(state, capability_flags);
    const auto weapon_id = script_item_id_from_source_code(weapon_code);
    if (weapon_id.has_value()) {
        lua_pushlstring(state, weapon_id->data(), weapon_id->size());
    } else if (purpose == ScriptPlayerDelayPurpose::shared_action) {
        lua_pushliteral(state, "");
    } else {
        return fail("player weapon delay item ID is unknown");
    }
    lua_pushlstring(state, use.data(), use.size());

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 5, 1, 0) != LUA_OK) {
        return fail("actor.delay.player_weapon_action_delay: " +
                    stack_error(state));
    }
    std::uint32_t delay = 0U;
    if (!read_integer(
            state, -1,
            static_cast<std::uint32_t>(
                std::numeric_limits<std::int16_t>::max()),
            delay)) {
        return fail("actor.delay.player_weapon_action_delay returned no delay");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return static_cast<std::int16_t>(delay);
}

std::optional<ScriptTarislarTombOutroStep>
GameScriptRuntime::advance_tarislar_tomb_outro(
    const bool active,
    const std::vector<std::string>& actor_ids, const bool arm) {
    auto* state = impl_->state;
    const auto stack_base = state == nullptr ? 0 : lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptTarislarTombOutroStep> {
        if (state != nullptr) {
            lua_settop(state, stack_base);
        }
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (state == nullptr) {
        return fail("gameplay Lua actor state is unavailable");
    }
    if (!impl_->push_module("tarislar_tomb_outro")) {
        return fail("gameplay Lua Scene-55 outro module is missing");
    }
    lua_getfield(state, -1, "advance");
    if (!lua_isfunction(state, -1)) {
        return fail("tarislar_tomb_outro.advance is missing");
    }
    lua_remove(state, -2);
    lua_pushboolean(state, active);
    lua_createtable(state, static_cast<int>(actor_ids.size()), 0);
    for (std::size_t index = 0U; index < actor_ids.size(); ++index) {
        const auto& actor_id = actor_ids[index];
        if (!actor_id.empty() && !valid_identifier(actor_id)) {
            return fail("Scene-55 outro actor ID is invalid");
        }
        lua_pushlstring(state, actor_id.data(), actor_id.size());
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_pushboolean(state, arm);

    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 3, 1, 0) != LUA_OK) {
        return fail("tarislar_tomb_outro.advance: " + stack_error(state));
    }
    std::size_t step_length = 0U;
    const auto* step_data = lua_tolstring(state, -1, &step_length);
    if (step_data == nullptr) {
        return fail("tarislar_tomb_outro.advance returned invalid state");
    }
    const std::string_view step(step_data, step_length);
    std::optional<ScriptTarislarTombOutroStep> result;
    if (step == "inactive") {
        result = ScriptTarislarTombOutroStep::inactive;
    } else if (step == "armed") {
        result = ScriptTarislarTombOutroStep::armed;
    } else if (step == "progressed") {
        result = ScriptTarislarTombOutroStep::progressed;
    } else if (step == "actor_target_present") {
        result = ScriptTarislarTombOutroStep::actor_target_present;
    } else if (step == "completion_ready") {
        result = ScriptTarislarTombOutroStep::completion_ready;
    } else if (step == "unresolved") {
        result = ScriptTarislarTombOutroStep::unresolved;
    } else {
        return fail("tarislar_tomb_outro.advance returned unknown state");
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<ScriptActorBehaviorDecision>
GameScriptRuntime::actor_behavior_decision(
    const ScriptActorBehaviorContext& context) {
    return actor_heartbeat(context);
}

std::optional<std::string> GameScriptRuntime::call_string(
    const std::string_view module, const std::string_view function) {
    auto* state = impl_->state;
    if (!valid_identifier(function)) {
        impl_->failure = "invalid gameplay script callback id";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "script callback is missing: " +
                         std::string(module) + "." + std::string(function);
        return std::nullopt;
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    std::string result;
    if (!read_string(state, -1, result) || !valid_identifier(result)) {
        lua_pop(state, 1);
        impl_->failure = "script callback must return a known string id";
        return std::nullopt;
    }
    lua_pop(state, 1);
    impl_->failure.clear();
    return result;
}

std::optional<ScriptPersistentValue> GameScriptRuntime::call_value(
    const std::string_view module, const std::string_view function,
    const std::vector<ScriptPersistentValue>& arguments) {
    constexpr std::size_t kMaximumArguments = 64U;
    auto* state = impl_->state;
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](std::string reason)
        -> std::optional<ScriptPersistentValue> {
        lua_settop(state, stack_base);
        impl_->failure = std::move(reason);
        return std::nullopt;
    };
    if (!valid_identifier(function) || arguments.size() > kMaximumArguments) {
        return fail("invalid gameplay script structured call");
    }
    if (!impl_->push_module(module)) {
        return fail("gameplay script module is missing: " +
                    std::string(module));
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        return fail("script callback is missing: " + std::string(module) +
                    "." + std::string(function));
    }
    lua_remove(state, -2);
    std::size_t argument_nodes = 0U;
    for (const auto& argument : arguments) {
        if (!push_persistent_value(state, argument, argument_nodes, 0U)) {
            return fail("gameplay script call arguments exceed their data limit");
        }
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, static_cast<int>(arguments.size()), 1, 0) != LUA_OK) {
        return fail(std::string(module) + "." + std::string(function) +
                    ": " + stack_error(state));
    }
    ScriptPersistentValue result;
    PersistentReadContext result_nodes;
    if (!read_persistent_value(state, -1, result, result_nodes, 0U)) {
        return fail(std::string(module) + "." + std::string(function) +
                    " returned invalid structured data: " +
                    result_nodes.error);
    }
    lua_settop(state, stack_base);
    impl_->failure.clear();
    return result;
}

std::optional<std::vector<std::int64_t>> GameScriptRuntime::call_integers(
    const std::string_view module, const std::string_view function,
    const std::vector<std::int64_t>& arguments) {
    constexpr std::size_t kMaximumIntegers = 64U;
    auto* state = impl_->state;
    if (!valid_identifier(function) || arguments.size() > kMaximumIntegers) {
        impl_->failure = "invalid gameplay script integer call";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "script callback is missing: " +
                         std::string(module) + "." + std::string(function);
        return std::nullopt;
    }
    lua_remove(state, -2);
    for (const auto value : arguments) {
        lua_pushinteger(state, static_cast<lua_Integer>(value));
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, static_cast<int>(arguments.size()), 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    std::vector<std::int64_t> result;
    bool valid = lua_istable(state, -1);
    if (valid) {
        const auto count = static_cast<std::size_t>(lua_rawlen(state, -1));
        valid = count <= kMaximumIntegers;
        for (std::size_t index = 1U; valid && index <= count; ++index) {
            lua_rawgeti(state, -1, static_cast<lua_Integer>(index));
            valid = lua_isinteger(state, -1) != 0;
            if (valid) {
                result.push_back(static_cast<std::int64_t>(
                    lua_tointeger(state, -1)));
            }
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 1);
    if (!valid) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return an integer array";
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

std::optional<std::vector<std::string>> GameScriptRuntime::string_list(
    const std::string_view module, const std::string_view function) {
    auto* state = impl_->state;
    if (!valid_identifier(function)) {
        impl_->failure = "invalid gameplay script list function";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "script list function is missing: " +
                         std::string(module) + "." + std::string(function);
        return std::nullopt;
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return an ordered ID list";
        return std::nullopt;
    }

    const auto count = lua_rawlen(state, -1);
    if (count > kMaximumMenuCatalogChoices) {
        lua_pop(state, 1);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " returned too many IDs";
        return std::nullopt;
    }
    std::vector<std::string> result;
    result.reserve(count);
    std::set<std::string> unique_ids;
    bool valid = true;
    for (std::size_t index = 1U; index <= count; ++index) {
        lua_rawgeti(state, -1, static_cast<lua_Integer>(index));
        std::string id;
        valid = read_string(state, -1, id) && valid_identifier(id) &&
                unique_ids.insert(id).second;
        lua_pop(state, 1);
        if (!valid) {
            break;
        }
        result.push_back(std::move(id));
    }
    lua_pop(state, 1);
    if (!valid) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " returned an invalid, duplicate, or empty ID";
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

ScriptTaskStep GameScriptRuntime::start_task(
    const std::string_view module, const std::string_view function) {
    return impl_->start_task(module, function, {});
}

ScriptTaskStep GameScriptRuntime::start_dialogue_task(
    const ScriptDialogueSource source, const std::string_view module,
    const std::string_view function,
    const ScriptDialogueOpenRouteData& route_data) {
    const auto source_id = script_dialogue_source_id(source);
    if (source_id.empty()) {
        impl_->failure = "invalid typed dialogue source";
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }
    const auto input_lock_frames = route_data.input_lock_frames == 0U
        ? std::string{}
        : std::to_string(route_data.input_lock_frames);
    const auto transition_frames = route_data.transition_frames == 0U
        ? std::string{}
        : std::to_string(route_data.transition_frames);
    const auto text_delay_frames = std::string_view{};
    const std::array<std::string_view, 13> arguments{
        source_id, module, function, route_data.scene_outcome,
        route_data.exit_behavior, route_data.transition,
        route_data.return_anchor, route_data.return_facing,
        route_data.music_return_policy, route_data.input_lock,
        input_lock_frames, route_data.scene_change_result_id,
        transition_frames};
    return impl_->start_task("dialogue_framework", "open", {
        arguments[0], arguments[1], arguments[2], arguments[3],
        arguments[4], arguments[5], arguments[6], arguments[7],
        arguments[8], arguments[9], text_delay_frames, arguments[10],
        arguments[11], arguments[12]});
}

ScriptTaskStep GameScriptRuntime::complete_native_operation(
    const bool succeeded) {
    auto* state = impl_->state;
    auto* task = impl_->task_state;
    if (state == nullptr || task == nullptr || !impl_->task_waiting ||
        impl_->pending_task_operation.has_value() ||
        !impl_->active_native_operation.has_value() ||
        (impl_->active_native_operation->type !=
             ScriptNativeOperation::Type::actor_walk_direction &&
         impl_->active_native_operation->type !=
             ScriptNativeOperation::Type::ui_say &&
         impl_->active_native_operation->type !=
             ScriptNativeOperation::Type::ui_story_page &&
         impl_->active_native_operation->type !=
             ScriptNativeOperation::Type::ui_credits_page)) {
        return impl_->failed_task(
            "gameplay script task is not waiting for a resumable operation");
    }
    impl_->task_waiting = false;
    impl_->active_native_operation.reset();
    lua_pushboolean(task, succeeded ? 1 : 0);
    return impl_->resume_task(1);
}

ScriptTaskStep GameScriptRuntime::complete_native_menu_choice(
    const std::string_view choice_id) {
    auto* state = impl_->state;
    auto* task = impl_->task_state;
    const auto& operation = impl_->active_native_operation;
    if (state == nullptr || task == nullptr || !impl_->task_waiting ||
        impl_->pending_task_operation.has_value() || !operation.has_value() ||
        operation->type != ScriptNativeOperation::Type::ui_menu ||
        !operation->menu.has_value()) {
        return impl_->failed_task(
            "gameplay script task is not waiting for a menu choice");
    }
    const auto selected = std::find_if(
        operation->menu->choices.begin(), operation->menu->choices.end(),
        [choice_id](const ScriptMenuChoice& choice) {
            return choice.id == choice_id;
        });
    if (selected == operation->menu->choices.end()) {
        return impl_->failed_task(
            "native menu returned a choice absent from the Lua menu");
    }
    impl_->task_waiting = false;
    impl_->active_native_operation.reset();
    lua_pushlstring(task, choice_id.data(), choice_id.size());
    return impl_->resume_task(1);
}

ScriptTaskStep GameScriptRuntime::complete_native_menu_input(
    const std::string_view input_id) {
    auto* state = impl_->state;
    auto* task = impl_->task_state;
    const auto& operation = impl_->active_native_operation;
    const bool generic_menu_cancel = input_id == "menu_cancel";
    const bool recognized_input = generic_menu_cancel ||
        input_id == "up" || input_id == "down" || input_id == "confirm" ||
        input_id == "cancel" || input_id == "back" || input_id == "page";
    if (state == nullptr || task == nullptr || !impl_->task_waiting ||
        impl_->pending_task_operation.has_value() || !operation.has_value() ||
        operation->type != ScriptNativeOperation::Type::ui_menu ||
        !operation->menu.has_value() ||
        (!generic_menu_cancel && operation->menu->input_mode !=
                                     ScriptMenuInputMode::cursor) ||
        !recognized_input) {
        return impl_->failed_task(
            "gameplay script task is not waiting for a valid menu input");
    }
    impl_->task_waiting = false;
    impl_->active_native_operation.reset();
    lua_pushlstring(task, input_id.data(), input_id.size());
    return impl_->resume_task(1);
}

ScriptTaskStep GameScriptRuntime::complete_native_service_result(
    const std::string_view result_id) {
    auto* state = impl_->state;
    auto* task = impl_->task_state;
    const auto& operation = impl_->active_native_operation;
    if (state == nullptr || task == nullptr || !impl_->task_waiting ||
        impl_->pending_task_operation.has_value() || !operation.has_value() ||
        operation->type != ScriptNativeOperation::Type::service_call ||
        result_id.empty() || !valid_identifier(result_id)) {
        return impl_->failed_task(
            "gameplay script task is not waiting for a valid service result");
    }
    impl_->task_waiting = false;
    impl_->active_native_operation.reset();
    lua_pushlstring(task, result_id.data(), result_id.size());
    return impl_->resume_task(1);
}

bool GameScriptRuntime::task_waiting_for_native_operation() const noexcept {
    return impl_->task_state != nullptr && impl_->task_waiting &&
           !impl_->pending_task_operation.has_value() &&
           impl_->active_native_operation.has_value();
}

void GameScriptRuntime::cancel_task() noexcept {
    impl_->release_task();
    impl_->failure.clear();
}

std::optional<ScriptStringMap> GameScriptRuntime::string_map(
    const std::string_view module, const std::string_view function) {
    return string_map(module, function, std::nullopt);
}

std::optional<ScriptStringMap> GameScriptRuntime::string_map(
    const std::string_view module, const std::string_view function,
    const std::optional<std::string_view> argument) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(function) ||
        (argument.has_value() && !valid_identifier(*argument))) {
        impl_->failure = "invalid gameplay script string-map callback";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    int argument_count = 0;
    if (argument.has_value()) {
        lua_pushlstring(state, argument->data(), argument->size());
        argument_count = 1;
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, argument_count, 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }

    ScriptStringMap result;
    bool valid = lua_istable(state, -1);
    const auto table_index = lua_absindex(state, -1);
    if (valid) {
        lua_pushnil(state);
        while (lua_next(state, table_index) != 0) {
            std::string key;
            std::string value;
            if (!read_string(state, -2, key) || !valid_identifier(key) ||
                !read_string(state, -1, value) ||
                !valid_identifier(value) ||
                result.size() >= kMaximumStringMapEntries ||
                !result.emplace(std::move(key), std::move(value)).second) {
                lua_pop(state, 2);
                valid = false;
                break;
            }
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 1);
    if (!valid || result.empty()) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return a bounded map of string IDs";
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

std::optional<ScriptSpeakerProfile> GameScriptRuntime::speaker_profile(
    const std::string_view speaker_id) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(speaker_id)) {
        impl_->failure = "invalid gameplay script speaker id";
        return std::nullopt;
    }
    constexpr std::string_view kModule = "dialogue";
    constexpr std::string_view kFunction = "speaker_profile";
    if (!impl_->push_module(kModule)) {
        impl_->failure = "gameplay script module is missing: dialogue";
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(kFunction).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "dialogue.speaker_profile must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    lua_pushlstring(state, speaker_id.data(), speaker_id.size());
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        impl_->failure = "dialogue.speaker_profile: " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = "dialogue speaker profile is unavailable: " +
                         std::string(speaker_id);
        return std::nullopt;
    }

    const auto table_index = lua_absindex(state, -1);
    ScriptSpeakerProfile result;
    result.id = speaker_id;
    const auto read_profile_field = [state, table_index](
                                        const char* field,
                                        std::string& value) {
        lua_getfield(state, table_index, field);
        const bool valid = read_string(state, -1, value);
        lua_pop(state, 1);
        return valid;
    };
    const bool fields_valid =
        read_profile_field("name", result.name) &&
        read_profile_field("text_profile", result.text_profile) &&
        read_profile_field("portrait_alias", result.portrait_alias);
    const bool name_valid =
        !result.name.empty() && result.name.size() <= 80U &&
        std::all_of(result.name.begin(), result.name.end(),
                    [](const char character) {
                        return character >= 0x20 && character <= 0x7E;
                    });
    const bool profile_valid =
        valid_identifier(result.text_profile) &&
        valid_identifier(result.portrait_alias);
    lua_pop(state, 1);
    if (!fields_valid || !name_valid || !profile_valid) {
        impl_->failure = "dialogue speaker profile is malformed: " +
                         std::string(speaker_id);
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

std::optional<ScriptDialogueTextKeys>
GameScriptRuntime::dialogue_event_text_keys(
    const std::string_view speaker_id, const std::string_view text_profile,
    const std::string_view event_id) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(speaker_id) ||
        !valid_identifier(text_profile) || !valid_identifier(event_id)) {
        impl_->failure = "invalid gameplay script dialogue event context";
        return std::nullopt;
    }
    constexpr std::string_view kModule = "dialogue";
    constexpr std::string_view kFunction = "event_text_keys";
    if (!impl_->push_module(kModule)) {
        impl_->failure = "gameplay script module is missing: dialogue";
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(kFunction).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "dialogue.event_text_keys must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    const auto push_string = [state](const std::string_view value) {
        lua_pushlstring(state, value.data(), value.size());
    };
    push_string(speaker_id);
    push_string(text_profile);
    push_string(event_id);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 3, 1, 0) != LUA_OK) {
        impl_->failure = "dialogue.event_text_keys: " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = "Lua dialogue event is not bound: " +
                         std::string(speaker_id) + "." +
                         std::string(event_id);
        return std::nullopt;
    }

    const auto table_index = lua_absindex(state, -1);
    ScriptDialogueTextKeys result;
    lua_getfield(state, table_index, "title_key");
    const bool title_valid = read_string(state, -1, result.title_key) &&
                             valid_identifier(result.title_key);
    lua_pop(state, 1);
    lua_getfield(state, table_index, "continuation_key");
    std::size_t continuation_length = 0U;
    const auto* continuation =
        lua_type(state, -1) == LUA_TSTRING
            ? lua_tolstring(state, -1, &continuation_length)
            : nullptr;
    const bool continuation_valid =
        continuation != nullptr &&
        continuation_length <= kMaximumIdentifierLength &&
        (continuation_length == 0U ||
         valid_identifier(std::string_view(continuation,
                                           continuation_length)));
    if (continuation_valid) {
        result.continuation_key.assign(continuation, continuation_length);
    }
    lua_pop(state, 1);
    lua_pop(state, 1);
    if (!title_valid || !continuation_valid) {
        impl_->failure = "Lua dialogue event keys are malformed: " +
                         std::string(speaker_id) + "." +
                         std::string(event_id);
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

std::optional<ScriptMenuDefinition> GameScriptRuntime::resolve_dialogue_menu(
    ScriptMenuDefinition menu) {
    auto* state = impl_->state;
    if (state == nullptr ||
        (!menu.speaker_id.empty() && !valid_identifier(menu.speaker_id)) ||
        !valid_identifier(menu.text_profile) || !valid_identifier(menu.id) ||
        menu.choices.size() > kMaximumMenuChoices) {
        impl_->failure = "invalid gameplay script dialogue menu context";
        return std::nullopt;
    }
    for (const auto& choice : menu.choices) {
        if (!valid_identifier(choice.id)) {
            impl_->failure =
                "dialogue menu contains an invalid stable choice id";
            return std::nullopt;
        }
    }

    // A Lua `prompt_text` operation already carries both its prompt and
    // visible labels. It should not need a second JSON menu-label table.
    // Catalog-backed prompts (choice IDs only) still resolve through
    // dialogue.menu_text_keys below.
    const bool has_authored_inline_labels = menu.inline_prompt &&
        !menu.choices.empty() &&
        std::all_of(menu.choices.begin(), menu.choices.end(),
                    [](const ScriptMenuChoice& choice) {
                        return choice.label_key.empty() &&
                               !choice.inline_label.empty();
                    });
    if (has_authored_inline_labels) {
        impl_->failure.clear();
        return menu;
    }

    constexpr std::string_view kModule = "dialogue";
    constexpr std::string_view kFunction = "menu_text_keys";
    if (!impl_->push_module(kModule)) {
        impl_->failure = "gameplay script module is missing: dialogue";
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(kFunction).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "dialogue.menu_text_keys must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    const auto push_string = [state](const std::string_view value) {
        lua_pushlstring(state, value.data(), value.size());
    };
    push_string(menu.speaker_id);
    push_string(menu.text_profile);
    push_string(menu.id);
    lua_createtable(state, static_cast<int>(menu.choices.size()), 0);
    for (std::size_t index = 0; index < menu.choices.size(); ++index) {
        const auto& choice_id = menu.choices[index].id;
        lua_pushlstring(state, choice_id.data(), choice_id.size());
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_pushboolean(state, menu.inline_prompt ? 1 : 0);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 5, 1, 0) != LUA_OK) {
        impl_->failure = "dialogue.menu_text_keys: " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = "Lua dialogue menu is not bound: " +
                         std::string(menu.speaker_id) + "." + menu.id;
        return std::nullopt;
    }

    const auto table_index = lua_absindex(state, -1);
    lua_getfield(state, table_index, "layout");
    bool layout_valid = true;
    if (lua_isnil(state, -1)) {
        menu.layout = ScriptMenuLayout::standard;
    } else {
        std::string layout;
        layout_valid = read_string(state, -1, layout);
        if (layout_valid && layout == "standard") {
            menu.layout = ScriptMenuLayout::standard;
        } else if (layout_valid && layout == "compact_list") {
            menu.layout = ScriptMenuLayout::compact_list;
        } else {
            layout_valid = false;
        }
    }
    lua_pop(state, 1);
    lua_getfield(state, table_index, "title_key");
    bool title_valid = false;
    if (!menu.inline_prompt) {
        const bool read_title =
            menu.layout == ScriptMenuLayout::compact_list
                ? read_string_allow_empty(state, -1, menu.title_key)
                : read_string(state, -1, menu.title_key);
        title_valid = read_title &&
                      (valid_identifier(menu.title_key) ||
                       (menu.layout == ScriptMenuLayout::compact_list &&
                        menu.title_key.empty()));
    } else {
        title_valid = lua_isnil(state, -1) != 0;
    }
    lua_pop(state, 1);
    lua_getfield(state, table_index, "choice_label_keys");
    bool labels_valid = lua_istable(state, -1) &&
                       lua_rawlen(state, -1) == menu.choices.size();
    if (labels_valid) {
        const auto labels_index = lua_absindex(state, -1);
        for (std::size_t index = 0; index < menu.choices.size(); ++index) {
            auto& label_key = menu.choices[index].label_key;
            lua_rawgeti(state, labels_index,
                        static_cast<lua_Integer>(index + 1U));
            labels_valid = read_string(state, -1, label_key) &&
                           valid_identifier(label_key);
            lua_pop(state, 1);
            if (!labels_valid) {
                break;
            }
        }
    }
    lua_pop(state, 2);
    if (!layout_valid || !title_valid || !labels_valid) {
        impl_->failure = "Lua dialogue menu keys do not match its choices: " +
                         std::string(menu.speaker_id) + "." + menu.id;
        return std::nullopt;
    }
    impl_->failure.clear();
    return menu;
}

std::optional<ScriptMenuDefinition> GameScriptRuntime::menu(
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile, const std::string_view menu_id) {
    auto* state = impl_->state;
    if (!valid_identifier(function) || !valid_identifier(text_profile) ||
        !valid_identifier(menu_id)) {
        impl_->failure = "invalid gameplay script menu function or profile";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 3, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }

    ScriptMenuDefinition definition;
    if (!parse_menu_choices(state, -3, menu_id, text_profile,
                            impl_->location_speaker_id, definition,
                            kMaximumMenuCatalogChoices)) {
        lua_pop(state, 3);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return unique choice ids";
        return std::nullopt;
    }
    if (!parse_explicit_menu_text_keys(state, -2, -1, definition)) {
        lua_pop(state, 3);
        impl_->failure = std::string(module) + "." +
                         std::string(function) +
                         " must return explicit text keys matching its choices";
        return std::nullopt;
    }
    lua_pop(state, 3);
    impl_->failure.clear();
    return definition;
}

void GameScriptRuntime::set_menu_route_bindings(
    ScriptMenuRouteBindings bindings) {
    impl_->menu_route_bindings = std::move(bindings);
    impl_->failure.clear();
}

bool GameScriptRuntime::open_menu_route(
    const std::uint16_t root_screen,
    const ScriptMenuRouteStartArguments start_arguments) {
    auto* state = impl_->state;
    if (state == nullptr || impl_->modules_reference == LUA_NOREF) {
        impl_->failure = "gameplay Lua package is not loaded for menu route";
        return false;
    }
    if (impl_->menu_route_reference != LUA_NOREF &&
        impl_->menu_route_state.open) {
        impl_->failure = "menu route is already open";
        return false;
    }
    if (impl_->menu_route_reference != LUA_NOREF) {
        luaL_unref(state, LUA_REGISTRYINDEX, impl_->menu_route_reference);
        impl_->menu_route_reference = LUA_NOREF;
    }
    impl_->failure.clear();

    const auto base = lua_gettop(state);
    std::string module_error;
    if (!impl_->load_module(state, "menu.field_init", module_error)) {
        impl_->failure = "could not load field menu route: " + module_error;
        lua_settop(state, base);
        return false;
    }
    const auto module_index = lua_absindex(state, -1);
    lua_getfield(state, module_index, "new");
    if (!lua_isfunction(state, -1)) {
        impl_->failure = "menu.field_init must return the route module";
        lua_settop(state, base);
        return false;
    }
    lua_remove(state, module_index);
    impl_->push_menu_host();
    impl_->push_menu_game();
    {
        ScopedInstructionBudget instruction_budget(state);
        if (lua_pcall(state, 2, 1, 0) != LUA_OK) {
            impl_->failure = "menu route construction failed: " +
                             stack_error(state);
            lua_settop(state, base);
            return false;
        }
    }
    if (!lua_istable(state, -1)) {
        impl_->failure = "menu route constructor returned no route";
        lua_settop(state, base);
        return false;
    }
    impl_->menu_route_reference = luaL_ref(state, LUA_REGISTRYINDEX);

    lua_rawgeti(state, LUA_REGISTRYINDEX, impl_->menu_route_reference);
    const auto route_index = lua_absindex(state, -1);
    lua_getfield(state, route_index, "open");
    if (!lua_isfunction(state, -1)) {
        impl_->failure = "menu route has no open function";
        lua_settop(state, base);
        luaL_unref(state, LUA_REGISTRYINDEX,
                   impl_->menu_route_reference);
        impl_->menu_route_reference = LUA_NOREF;
        return false;
    }
    lua_pushvalue(state, route_index);
    lua_pushinteger(state, static_cast<lua_Integer>(root_screen));
    lua_createtable(state, 0, 6);
    if (start_arguments.matrix_system_id.has_value()) {
        lua_pushinteger(state, *start_arguments.matrix_system_id);
        lua_setfield(state, -2, "system_id");
    }
    if (start_arguments.matrix_node_id.has_value()) {
        lua_pushinteger(state, *start_arguments.matrix_node_id);
        lua_setfield(state, -2, "node_id");
    }
    if (start_arguments.matrix_node_ic_condition.has_value()) {
        lua_pushinteger(state,
                        *start_arguments.matrix_node_ic_condition);
        lua_setfield(state, -2, "node_ic_condition");
    }
    if (start_arguments.matrix_node_analysis_level.has_value()) {
        lua_pushinteger(state,
                        *start_arguments.matrix_node_analysis_level);
        lua_setfield(state, -2, "node_analysis_level");
    }
    if (start_arguments.taxi_current_region.has_value()) {
        const auto region_id = script_field_region_id(
            *start_arguments.taxi_current_region);
        lua_pushlstring(state, region_id.data(), region_id.size());
        lua_setfield(state, -2, "current_region");
    }
    lua_pushboolean(state, start_arguments.return_to_field ? 1 : 0);
    lua_setfield(state, -2, "return_to_field");
    {
        ScopedInstructionBudget instruction_budget(state);
        if (lua_pcall(state, 3, 0, 0) != LUA_OK) {
            impl_->failure = "menu route open failed: " + stack_error(state);
            lua_settop(state, base);
            luaL_unref(state, LUA_REGISTRYINDEX,
                       impl_->menu_route_reference);
            impl_->menu_route_reference = LUA_NOREF;
            impl_->menu_route_state = ScriptMenuRouteSnapshot{};
            return false;
        }
    }
    lua_settop(state, base);
    if (!impl_->refresh_menu_route_snapshot()) {
        luaL_unref(state, LUA_REGISTRYINDEX, impl_->menu_route_reference);
        impl_->menu_route_reference = LUA_NOREF;
        impl_->menu_route_state = ScriptMenuRouteSnapshot{};
        return false;
    }
    if (!impl_->menu_route_state.open) {
        if (impl_->failure.empty()) {
            impl_->failure = "menu route did not open its root screen";
        }
        return false;
    }
    impl_->failure.clear();
    return true;
}

void GameScriptRuntime::advance_menu_route(const std::uint16_t events) {
    auto* state = impl_->state;
    if (state == nullptr || impl_->menu_route_reference == LUA_NOREF ||
        (!impl_->menu_route_state.open &&
         !impl_->menu_route_state.field_return_transition_active)) {
        return;
    }
    impl_->failure.clear();
    const auto base = lua_gettop(state);
    lua_rawgeti(state, LUA_REGISTRYINDEX, impl_->menu_route_reference);
    const auto route_index = lua_absindex(state, -1);
    lua_getfield(state, route_index, "frame");
    if (!lua_isfunction(state, -1)) {
        impl_->failure = "menu route has no frame function";
        lua_settop(state, base);
        return;
    }
    lua_pushvalue(state, route_index);
    lua_pushinteger(state, static_cast<lua_Integer>(events));
    {
        ScopedInstructionBudget instruction_budget(state);
        if (lua_pcall(state, 2, 0, 0) != LUA_OK) {
            impl_->failure = "menu route frame failed: " + stack_error(state);
            lua_settop(state, base);
            return;
        }
    }
    lua_settop(state, base);
    if (!impl_->refresh_menu_route_snapshot()) {
        return;
    }
    impl_->failure.clear();
}

void GameScriptRuntime::close_menu_route() {
    auto* state = impl_->state;
    if (state == nullptr || impl_->menu_route_reference == LUA_NOREF) {
        impl_->menu_route_state = ScriptMenuRouteSnapshot{};
        return;
    }
    if (!impl_->menu_route_state.open) {
        return;
    }
    impl_->failure.clear();
    const auto base = lua_gettop(state);
    lua_rawgeti(state, LUA_REGISTRYINDEX, impl_->menu_route_reference);
    const auto route_index = lua_absindex(state, -1);
    lua_getfield(state, route_index, "close");
    if (!lua_isfunction(state, -1)) {
        impl_->failure = "menu route has no close function";
        lua_settop(state, base);
        return;
    }
    lua_pushvalue(state, route_index);
    {
        ScopedInstructionBudget instruction_budget(state);
        if (lua_pcall(state, 1, 0, 0) != LUA_OK) {
            impl_->failure = "menu route close failed: " + stack_error(state);
            lua_settop(state, base);
            return;
        }
    }
    lua_settop(state, base);
    if (!impl_->refresh_menu_route_snapshot()) {
        return;
    }
    impl_->failure.clear();
}

ScriptMenuRouteSnapshot GameScriptRuntime::menu_route_snapshot() const noexcept {
    return impl_->menu_route_state;
}

const std::string& GameScriptRuntime::failure() const noexcept {
    return impl_->failure;
}

} // namespace shadowrun::ghidra_conversion::scripting
