#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(compute_diagonal_step_cost(0, 0) == 0U)) {
        return 1;
    }
    if (!expect(compute_diagonal_step_cost(10, 4) == 12U)) {
        return 2;
    }
    if (!expect(compute_diagonal_step_cost(4, 10) == 12U)) {
        return 3;
    }
    if (!expect(compute_diagonal_step_cost(-10, 3) == 12U)) {
        return 4;
    }
    if (!expect(compute_diagonal_step_cost(-5, -5) == 8U)) {
        return 5;
    }
    if (!expect(compute_diagonal_step_cost(7, 2) == 8U)) {
        return 6;
    }
    if (!expect(compute_diagonal_step_cost(0x1000AU, 4) == 12U &&
                compute_diagonal_step_cost(-65526, 4) == 12U)) {
        return 18;
    }
    // NEG.W cannot represent the positive magnitude of 0x8000.  The source
    // therefore ASR.Ws the wrapped word and ADD.Ws the result back into D0.
    if (!expect(compute_diagonal_step_cost(-32768, -32768) == 0x4000U)) {
        return 19;
    }
    if (!expect(compute_record_diagonal_step_cost(10U, 4U, 0U, 0U) == 12U &&
                compute_record_diagonal_step_cost(0U, 0U, 10U, 4U) == 12U)) {
        return 20;
    }

    const auto negative_vertical = measure_heading_and_distance(0, -100);
    if (!expect(negative_vertical.heading == 0x0000U &&
                negative_vertical.distance == 100U)) {
        return 7;
    }
    const auto positive_horizontal = measure_heading_and_distance(100, 0);
    if (!expect(positive_horizontal.heading == 0x4000U &&
                positive_horizontal.distance == 100U)) {
        return 8;
    }
    const auto southwest = measure_heading_and_distance(-100, 100);
    if (!expect(southwest.heading == 0xA000U && southwest.distance == 150U)) {
        return 9;
    }
    const auto zero = measure_heading_and_distance(0, 0);
    if (!expect(zero.heading == 0U && zero.distance == 0U)) {
        return 10;
    }
    const auto relative = measure_relative_heading_and_distance(10, 20, 10, 120);
    if (!expect(relative.heading == 0x8000U && relative.distance == 100U)) {
        return 11;
    }
    const auto entity_target = measure_entity_target_heading_and_distance(
        0x0100U, 0x0200U, 0x0100U, 0x0264U);
    if (!expect(entity_target.heading == 0x8000U &&
                entity_target.distance == 100U)) {
        return 17;
    }
    // Scene 0x23/order 1 at ROM route 0x1DC3D6 turns from
    // (0x6080,0x4C80) toward (0x4C00,0x1A80). ROM 0x15D3C returns F000;
    // B574 must truncate this just below the north-west/north boundary.
    const auto scene_35_guard_route = measure_relative_heading_and_distance(
        0x6080, 0x4C80, 0x4C00, 0x1A80);
    if (!expect(scene_35_guard_route.heading == 0xF000U &&
                scene_35_guard_route.distance == 0x3C40U)) {
        return 23;
    }
    // These additional samples come from the extracted default waypoint
    // streams at 0xD556C, 0x17D6B6, 0x172D58, 0x18A76E, and 0x1DC3D6. Across
    // the 379 route edges, they cover the other measured headings whose
    // rounded BABC and truncated B574 octants differ.
    struct RouteHeadingSample {
        std::int32_t origin_x;
        std::int32_t origin_y;
        std::int32_t target_x;
        std::int32_t target_y;
        std::uint16_t expected_heading;
    };
    constexpr std::array<RouteHeadingSample, 9> route_heading_samples{{
        {0x4E00, 0x3900, 0x5180, 0x3200, 0x1300U},
        {0x2F00, 0x2280, 0x3680, 0x2600, 0x5200U},
        {0x6280, 0x0B80, 0x7300, 0x1380, 0x5300U},
        {0x1F80, 0x4700, 0x2F00, 0x5080, 0x5600U},
        {0x4C00, 0x1A80, 0x6080, 0x4C80, 0x7000U},
        {0x3C00, 0x3600, 0x3C80, 0x3900, 0x7900U},
        {0x2F00, 0x5080, 0x2100, 0x5480, 0xB500U},
        {0x7A00, 0x1800, 0x6C80, 0x1980, 0xBB00U},
        {0x2100, 0x5480, 0x1F80, 0x3E80, 0xFE00U},
    }};
    for (const auto& sample : route_heading_samples) {
        if (!expect(measure_relative_heading_and_distance(
                        sample.origin_x, sample.origin_y,
                        sample.target_x, sample.target_y)
                        .heading == sample.expected_heading)) {
            return 24;
        }
    }

    // Audit the complete profile-selected default route manifest against the
    // source metric, not just the hand-picked route samples above.
    const auto tsv_fields = [](const std::string& row) {
        std::vector<std::string_view> fields;
        std::size_t start = 0U;
        while (start <= row.size()) {
            const auto separator = row.find('\t', start);
            const auto end = separator == std::string::npos
                ? row.size()
                : separator;
            fields.emplace_back(row.data() + start, end - start);
            if (separator == std::string::npos) {
                break;
            }
            start = separator + 1U;
        }
        return fields;
    };
    const auto column = [](const std::vector<std::string_view>& header,
                           const std::string_view name) {
        for (std::size_t index = 0U; index < header.size(); ++index) {
            if (header[index] == name) {
                return index;
            }
        }
        return header.size();
    };
    const auto number = [](const std::string_view field) {
        return static_cast<std::uint32_t>(
            std::stoul(std::string(field), nullptr, 0));
    };
    using RouteKey = std::pair<std::uint32_t, std::uint32_t>;
    using RoutePoint = std::pair<std::uint16_t, std::uint16_t>;
    const std::filesystem::path asset_root{
        SHADOWRUN_GHIDRA_ASSET_ROOT};
    std::ifstream actor_input(
        asset_root / "field/entities/default-actors.tsv");
    std::ifstream waypoint_input(
        asset_root / "field/entities/default-actor-waypoints.tsv");
    if (!expect(actor_input.is_open() && waypoint_input.is_open())) {
        return 25;
    }

    std::string row;
    if (!std::getline(actor_input, row)) {
        return 26;
    }
    const auto actor_header = tsv_fields(row);
    const auto actor_scene = column(actor_header, "scene");
    const auto actor_order = column(actor_header, "source_order");
    const auto actor_x = column(actor_header, "world_x");
    const auto actor_y = column(actor_header, "world_y");
    if (!expect(actor_scene < actor_header.size() &&
                actor_order < actor_header.size() &&
                actor_x < actor_header.size() &&
                actor_y < actor_header.size())) {
        return 27;
    }
    std::map<RouteKey, RoutePoint> route_initials;
    while (std::getline(actor_input, row)) {
        if (row.empty()) {
            continue;
        }
        const auto fields = tsv_fields(row);
        if (!expect(fields.size() >
                    std::max({actor_scene, actor_order, actor_x, actor_y}))) {
            return 28;
        }
        const RouteKey key{number(fields[actor_scene]),
                           number(fields[actor_order])};
        route_initials[key] = RoutePoint{
            static_cast<std::uint16_t>(number(fields[actor_x])),
            static_cast<std::uint16_t>(number(fields[actor_y]))};
    }

    if (!std::getline(waypoint_input, row)) {
        return 29;
    }
    const auto waypoint_header = tsv_fields(row);
    const auto waypoint_scene = column(waypoint_header, "scene");
    const auto waypoint_actor_order = column(waypoint_header, "source_order");
    const auto waypoint_order = column(waypoint_header, "waypoint_order");
    const auto waypoint_x = column(waypoint_header, "world_x");
    const auto waypoint_y = column(waypoint_header, "world_y");
    if (!expect(waypoint_scene < waypoint_header.size() &&
                waypoint_actor_order < waypoint_header.size() &&
                waypoint_order < waypoint_header.size() &&
                waypoint_x < waypoint_header.size() &&
                waypoint_y < waypoint_header.size())) {
        return 30;
    }
    std::map<RouteKey, std::map<std::uint32_t, RoutePoint>> route_waypoints;
    while (std::getline(waypoint_input, row)) {
        if (row.empty()) {
            continue;
        }
        const auto fields = tsv_fields(row);
        if (!expect(fields.size() > std::max(
                        {waypoint_scene, waypoint_actor_order,
                         waypoint_order, waypoint_x, waypoint_y}))) {
            return 31;
        }
        const RouteKey key{number(fields[waypoint_scene]),
                           number(fields[waypoint_actor_order])};
        const auto order = number(fields[waypoint_order]);
        route_waypoints[key][order] = RoutePoint{
            static_cast<std::uint16_t>(number(fields[waypoint_x])),
            static_cast<std::uint16_t>(number(fields[waypoint_y]))};
    }

    std::size_t route_edge_count = 0U;
    std::size_t rounded_truncation_difference_count = 0U;
    std::set<std::uint16_t> differing_route_headings;
    for (const auto& [key, initial] : route_initials) {
        std::vector<RoutePoint> points{initial};
        const auto route = route_waypoints.find(key);
        if (route != route_waypoints.end()) {
            for (const auto& [order, point] : route->second) {
                (void)order;
                points.push_back(point);
            }
        }
        for (std::size_t index = 1U; index < points.size(); ++index) {
            const auto& origin = points[index - 1U];
            const auto& target = points[index];
            const auto heading = measure_relative_heading_and_distance(
                origin.first, origin.second, target.first, target.second)
                                     .heading;
            const auto truncation = static_cast<std::uint8_t>(
                (heading >> 13U) & 0x07U);
            auto rotated = static_cast<std::uint16_t>(
                (heading << 3U) | (heading >> 13U));
            if ((rotated & 0x8000U) != 0U) {
                rotated = static_cast<std::uint16_t>(rotated + 1U);
            }
            const auto rounded = static_cast<std::uint8_t>(rotated & 0x07U);
            ++route_edge_count;
            if (rounded != truncation) {
                ++rounded_truncation_difference_count;
                differing_route_headings.insert(heading);
            }
        }
    }
    if (!expect(route_edge_count == 379U &&
                rounded_truncation_difference_count == 19U &&
                differing_route_headings.size() == 10U)) {
        return 32;
    }

    // The six regional pedestrian profiles use the same 0x15D3C distance
    // result to choose the 0xB6E4 movement table. Waypoint TSV order zero is
    // the repeated spawn pair; skip it, then include the zero-X wrap back to
    // each route's initial coordinate. [C]
    std::array<std::size_t, 5> regional_pedestrian_distance_bands{};
    std::size_t regional_pedestrian_route_legs = 0U;
    for (const auto& [key, initial] : route_initials) {
        if (key.first > 5U) {
            continue;
        }
        const auto route = route_waypoints.find(key);
        if (!expect(route != route_waypoints.end())) {
            return 33;
        }
        auto previous = initial;
        for (const auto& [order, point] : route->second) {
            if (order == 0U) {
                if (!expect(point == initial)) {
                    return 34;
                }
                continue;
            }
            const auto distance = measure_relative_heading_and_distance(
                previous.first, previous.second, point.first, point.second)
                                      .distance;
            const auto band = distance < 0x0100U ? 0U
                : distance < 0x0200U ? 1U
                : distance < 0x0360U ? 2U
                : distance < 0x0460U ? 3U
                                     : 4U;
            ++regional_pedestrian_distance_bands[band];
            ++regional_pedestrian_route_legs;
            previous = point;
        }
        const auto wrap_distance = measure_relative_heading_and_distance(
            previous.first, previous.second, initial.first, initial.second)
                                      .distance;
        const auto wrap_band = wrap_distance < 0x0100U ? 0U
            : wrap_distance < 0x0200U ? 1U
            : wrap_distance < 0x0360U ? 2U
            : wrap_distance < 0x0460U ? 3U
                                      : 4U;
        ++regional_pedestrian_distance_bands[wrap_band];
        ++regional_pedestrian_route_legs;
    }
    if (!expect(regional_pedestrian_route_legs == 168U &&
                regional_pedestrian_distance_bands ==
                    std::array<std::size_t, 5>{0U, 0U, 8U, 0U, 160U})) {
        return 35;
    }
    const auto quantized = measure_heading_and_distance(-64, -63);
    if (!expect(quantized.heading == 0xDF00U && quantized.distance == 95U)) {
        return 12;
    }

    // Cardinal outputs must agree with the ROM's eight movement ordinals:
    // up, northeast, right, southeast, down, southwest, left, northwest.
    const std::array<std::array<std::int32_t, 3>, 8> cardinal_headings{{
        {{0, -100, 0x0000}}, {{100, -100, 0x2000}},
        {{100, 0, 0x4000}}, {{100, 100, 0x6000}},
        {{0, 100, 0x8000}}, {{-100, 100, 0xA000}},
        {{-100, 0, 0xC000}}, {{-100, -100, 0xE000}},
    }};
    for (const auto& sample : cardinal_headings) {
        if (!expect(measure_heading_and_distance(sample[0], sample[1]).heading ==
                    static_cast<std::uint16_t>(sample[2]))) {
            return 22;
        }
    }

    // D0 is a word result in 0x15D6C, so the host representation must wrap
    // the distance before exposing it to callers.
    if (!expect(measure_heading_and_distance(0x7FFF, 0x7FFF).distance ==
                0xBFFEU)) {
        return 21;
    }

    if (!expect(is_entity_target_within_range(
                    0x01U, 0, 0, 100, 0))) {
        return 13;
    }
    if (!expect(!is_entity_target_within_range(
                    0x41U, 0, 0, 100, 0) &&
                !is_entity_target_within_range(
                    0x00U, 0, 0, 100, 0) &&
                !is_entity_target_within_range(
                    0x01U, 0, 0, 0x400, 0))) {
        return 14;
    }
    const auto updated_heading = update_entity_heading_from_linked_target(
        true, 0x01U, 0xA000U, 0, 0, 100, 0);
    if (!expect(updated_heading.has_value() &&
                updated_heading.value() == 0xA000U)) {
        return 15;
    }
    if (!expect(!update_entity_heading_from_linked_target(
                     false, 0x01U, 0xA000U, 0, 0, 100, 0)
                     .has_value())) {
        return 16;
    }

    return 0;
}
