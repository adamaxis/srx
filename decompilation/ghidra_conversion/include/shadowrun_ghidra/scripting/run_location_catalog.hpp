#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion::scripting {

struct RunLocationSourceEntry {
    std::uint8_t region_selector;
    std::uint8_t location_selector;
    std::string_view region_id;
    std::string_view location_id;
};

// The old offer handler compares the current field location with its selected
// region/event pair. Keep that source crosswalk at the native boundary; Lua
// offers and saved state use the stable place IDs below. [C] ROM 0x1EF2C,
// 0x1F046, and 0x1F086 consume these selectors; names are recovered from
// 0xF1E58/0xF1F1A.
inline constexpr std::array<RunLocationSourceEntry, 46>
    kRunLocationSourceEntries{{
        {0U, 0U, "downtown_seattle", "seattle_general_hospital"},
        {0U, 1U, "downtown_seattle", "icarus_descending"},
        {0U, 2U, "downtown_seattle", "matchsticks"},
        {0U, 3U, "downtown_seattle", "gates_undersound_hotel"},
        {0U, 4U, "downtown_seattle", "mitsuhama_computer_technologies"},
        {0U, 5U, "downtown_seattle", "fuchi_local_office"},
        {0U, 6U, "downtown_seattle", "space_needle"},
        {0U, 7U, "downtown_seattle", "roscoe_the_fixer"},
        {1U, 0U, "puyallup_barrens", "wanderer"},
        {1U, 1U, "puyallup_barrens", "tarislar_city_inn"},
        {1U, 2U, "puyallup_barrens", "ares_corporate_offices"},
        {1U, 3U, "puyallup_barrens", "riannons"},
        {1U, 4U, "puyallup_barrens", "underground_93"},
        {1U, 5U, "puyallup_barrens", "crime_mall"},
        {1U, 6U, "puyallup_barrens", "dr_bobs_quickstitch"},
        {1U, 7U, "puyallup_barrens", "tarislar_gardens_apartments"},
        {1U, 9U, "puyallup_barrens", "ork_gang_hideout"},
        {2U, 0U, "redmond_barrens", "hollywood_correctional_facilities"},
        {2U, 1U, "redmond_barrens", "little_chiba"},
        {2U, 2U, "redmond_barrens", "stokers_coffin_motel"},
        {2U, 3U, "redmond_barrens", "shiawase_nuke_plant"},
        {2U, 4U, "redmond_barrens", "jump_house"},
        {2U, 5U, "redmond_barrens", "jackals_lantern"},
        {2U, 6U, "redmond_barrens", "boris_greenhouse"},
        {2U, 7U, "redmond_barrens", "rats_nest"},
        {2U, 9U, "redmond_barrens", "halloweeners_hideout"},
        {2U, 11U, "redmond_barrens", "ares_weapon_emporium"},
        {3U, 0U, "penumbra_district", "club_penumbra"},
        {3U, 1U, "penumbra_district", "wylies_gala_inn"},
        {3U, 2U, "penumbra_district", "fuchi_industrial_electronics"},
        {3U, 3U, "penumbra_district", "mitsuhama_local_office"},
        {3U, 4U, "penumbra_district", "eye_fivers_hideout"},
        {3U, 5U, "penumbra_district", "big_rhino"},
        {3U, 6U, "penumbra_district", "lone_star_security"},
        {4U, 0U, "council_island", "council_island_inn"},
        {4U, 1U, "council_island", "friendship_restaurant"},
        {4U, 2U, "council_island", "passport_lodge"},
        {4U, 3U, "council_island", "medicine_lodge_hollow"},
        {4U, 4U, "council_island", "council_island_hospital"},
        {4U, 5U, "council_island", "ork_embassy"},
        {5U, 1U, "renraku_arcology", "frag_grenade"},
        {5U, 2U, "renraku_arcology", "wire_masters"},
        {5U, 3U, "renraku_arcology", "merlins_lore"},
        {5U, 4U, "renraku_arcology", "microtronics"},
        {5U, 5U, "renraku_arcology", "weapons_world"},
        {5U, 6U, "renraku_arcology", "renraku_offices"},
    }};

inline std::optional<std::string_view> run_location_id_for_source_selector(
    const std::int64_t region_selector,
    const std::int64_t location_selector) noexcept {
    for (const auto& entry : kRunLocationSourceEntries) {
        if (entry.region_selector == region_selector &&
            entry.location_selector == location_selector) {
            return entry.location_id;
        }
    }
    return std::nullopt;
}

inline std::optional<std::string_view> run_region_id_for_source_selector(
    const std::int64_t region_selector) noexcept {
    for (const auto& entry : kRunLocationSourceEntries) {
        if (entry.region_selector == region_selector) {
            return entry.region_id;
        }
    }
    return std::nullopt;
}

inline std::optional<std::string_view> run_region_id_for_location(
    const std::string_view location_id) noexcept {
    for (const auto& entry : kRunLocationSourceEntries) {
        if (entry.location_id == location_id) {
            return entry.region_id;
        }
    }
    return std::nullopt;
}

inline bool is_run_region_id(const std::string_view region_id) noexcept {
    return region_id == "downtown_seattle" ||
           region_id == "puyallup_barrens" ||
           region_id == "redmond_barrens" ||
           region_id == "penumbra_district" ||
           region_id == "council_island" ||
           region_id == "renraku_arcology" ||
           region_id == "amerindian_lands";
}

inline std::optional<std::string_view>
run_target_actor_group_id_for_source_selector(
    const std::int64_t region_selector,
    const std::int64_t location_selector) noexcept {
    if (region_selector == 1 && location_selector == 9) {
        return "gang_contract.ork_gang";
    }
    if (region_selector == 2 && location_selector == 9) {
        return "gang_contract.halloweeners";
    }
    if (region_selector == 3 && location_selector == 4) {
        return "gang_contract.eye_fivers";
    }
    return std::nullopt;
}

inline std::optional<std::string_view>
run_target_actor_group_id_for_location(
    const std::string_view location_id) noexcept {
    if (location_id == "ork_gang_hideout") {
        return "gang_contract.ork_gang";
    }
    if (location_id == "halloweeners_hideout") {
        return "gang_contract.halloweeners";
    }
    if (location_id == "eye_fivers_hideout") {
        return "gang_contract.eye_fivers";
    }
    return std::nullopt;
}

inline bool is_run_target_actor_group_id(
    const std::string_view group_id) noexcept {
    return group_id == "gang_contract.ork_gang" ||
           group_id == "gang_contract.halloweeners" ||
           group_id == "gang_contract.eye_fivers";
}

}  // namespace shadowrun::ghidra_conversion::scripting
