#pragma once

#include "../../src/field/field_scene_runtime.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime::qa {

struct MapCoverageRow {
    std::string row_id;
    std::string scene;
    std::string category;
    std::string rom_addr;
    std::string trigger;
    std::string gate;
    std::string dest_scene;
    std::string arrival_xy;
    std::string arrival_mode;
    std::string port_ref;
};

struct MapCoverageRect {
    std::uint16_t x0{};
    std::uint16_t x1{};
    std::uint16_t y0{};
    std::uint16_t y1{};
};

struct MapCoverageAttempt {
    bool setup{};
    bool trigger_seen{};
    bool transition_fired{};
    std::uint32_t frames_used{};
    std::uint16_t observed_scene{};
    std::uint16_t observed_x{};
    std::uint16_t observed_y{};
    std::uint16_t backup_x{};
    std::uint16_t backup_y{};
    bool backup_captured{};
    std::uint16_t observed_mode{};
    std::uint8_t observed_event{};
    std::string note;
};

inline std::vector<std::string> map_coverage_split_tsv(
    const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    for (;;) {
        const auto end = line.find('\t', start);
        fields.emplace_back(line.substr(
            start, end == std::string::npos ? end : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    return fields;
}

inline std::vector<MapCoverageRow> map_coverage_read_rows(
    const std::filesystem::path& path) {
    std::ifstream input(path);
    std::string line;
    std::vector<MapCoverageRow> rows;
    if (!std::getline(input, line)) {
        return rows;
    }
    while (std::getline(input, line)) {
        const auto fields = map_coverage_split_tsv(line);
        if (fields.size() != 10U) {
            return {};
        }
        rows.push_back(MapCoverageRow{
            fields[0], fields[1], fields[2], fields[3], fields[4],
            fields[5], fields[6], fields[7], fields[8], fields[9]});
    }
    return rows;
}

inline std::optional<std::uint32_t> map_coverage_number(
    std::string_view value) {
    if (value.empty()) {
        return std::nullopt;
    }
    std::string copy(value);
    char* end = nullptr;
    const auto number = std::strtoul(copy.c_str(), &end, 0);
    if (end == copy.c_str() || *end != '\0') {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(number);
}

inline std::optional<MapCoverageRect> map_coverage_rect(
    std::string_view trigger) {
    const auto marker = trigger.find("rect:x=");
    if (marker == std::string_view::npos) {
        return std::nullopt;
    }
    const auto x_start = marker + 7U;
    const auto x_end = trigger.find(",y=", x_start);
    const auto y_start = x_end == std::string_view::npos ? x_end : x_end + 3U;
    const auto y_end = trigger.find(';', y_start);
    if (x_end == std::string_view::npos || y_start == std::string_view::npos) {
        return std::nullopt;
    }
    const auto x_range = trigger.substr(x_start, x_end - x_start);
    const auto y_range = trigger.substr(
        y_start, y_end == std::string_view::npos ? y_end : y_end - y_start);
    const auto x_split = x_range.find("..");
    const auto y_split = y_range.find("..");
    if (x_split == std::string_view::npos || y_split == std::string_view::npos) {
        return std::nullopt;
    }
    const auto x0 = map_coverage_number(x_range.substr(0, x_split));
    const auto x1 = map_coverage_number(x_range.substr(x_split + 2U));
    const auto y0 = map_coverage_number(y_range.substr(0, y_split));
    const auto y1 = map_coverage_number(y_range.substr(y_split + 2U));
    if (!x0 || !x1 || !y0 || !y1 || *x1 > 0xFFFFU || *y1 > 0xFFFFU) {
        return std::nullopt;
    }
    return MapCoverageRect{
        static_cast<std::uint16_t>(*x0), static_cast<std::uint16_t>(*x1),
        static_cast<std::uint16_t>(*y0), static_cast<std::uint16_t>(*y1)};
}

inline std::optional<std::uint32_t> map_coverage_event(
    std::string_view trigger) {
    const auto marker = trigger.find("event=0x");
    if (marker == std::string_view::npos) {
        return std::nullopt;
    }
    const auto start = marker + 8U;
    const auto end = trigger.find_first_not_of("0123456789abcdefABCDEF", start);
    return map_coverage_number(std::string("0x") + std::string(
        trigger.substr(start, end == std::string_view::npos ? end : end - start)));
}

inline std::optional<std::uint16_t> map_coverage_scene(
    std::string_view scene) {
    const auto value = map_coverage_number(scene);
    if (!value || *value >= 56U) {
        return std::nullopt;
    }
    return static_cast<std::uint16_t>(*value);
}

inline std::optional<std::uint32_t> map_coverage_record_address(
    std::string_view rom_addr) {
    if (rom_addr.rfind("0x", 0U) != 0U) {
        return std::nullopt;
    }
    const auto end = rom_addr.find(';');
    return map_coverage_number(rom_addr.substr(
        0U, end == std::string_view::npos ? end : end));
}

inline std::optional<std::uint16_t> map_coverage_mode(
    std::string_view gate) {
    const auto marker = gate.find("FFDF40=");
    if (marker == std::string_view::npos) {
        return std::nullopt;
    }
    const auto start = marker + 7U;
    const auto end = gate.find_first_not_of("0123456789", start);
    const auto value = map_coverage_number(gate.substr(
        start, end == std::string_view::npos ? end : end - start));
    if (!value || *value > 3U) {
        return std::nullopt;
    }
    return static_cast<std::uint16_t>(*value);
}

inline std::optional<std::array<std::uint16_t, 2>> map_coverage_xy(
    std::string_view text) {
    if (text.size() < 3U || text[0] != '0' || text[1] != 'x') {
        return std::nullopt;
    }
    const auto comma = text.find(',');
    if (comma == std::string_view::npos) {
        return std::nullopt;
    }
    const auto x = map_coverage_number(text.substr(0, comma));
    const auto y = map_coverage_number(text.substr(comma + 1U));
    if (!x || !y || *x > 0xFFFFU || *y > 0xFFFFU) {
        return std::nullopt;
    }
    return std::array<std::uint16_t, 2>{
        static_cast<std::uint16_t>(*x), static_cast<std::uint16_t>(*y)};
}

inline std::string map_coverage_hex(const std::uint32_t value) {
    std::ostringstream output;
    output << "0x" << std::hex << std::uppercase << value;
    return output.str();
}

inline void map_coverage_tap(
    FieldSceneRuntime& runtime, const std::uint8_t port_byte) {
    runtime.set_controller_port_byte(port_byte);
    runtime.advance_frame();
    runtime.set_controller_port_byte(0U);
    runtime.advance_frame();
}

inline std::uint8_t map_coverage_direction(
    FieldSceneRuntime& runtime, const MapCoverageRect& rect) {
    const auto position = runtime.field_entity_world_position();
    const std::uint32_t cx = (static_cast<std::uint32_t>(rect.x0) + rect.x1) / 2U;
    const std::uint32_t cy = (static_cast<std::uint32_t>(rect.y0) + rect.y1) / 2U;
    struct Candidate { std::uint32_t x; std::uint32_t y; std::uint8_t input; };
    const std::array<Candidate, 4> candidates{{
        {rect.x0 > 0x80U ? rect.x0 - 0x80U : 0U, cy, 0x08U},
        {std::min<std::uint32_t>(0xFFFFU, rect.x1 + 0x80U), cy, 0x04U},
        {cx, rect.y0 > 0x80U ? rect.y0 - 0x80U : 0U, 0x02U},
        {cx, std::min<std::uint32_t>(0xFFFFU, rect.y1 + 0x80U), 0x01U}}};
    const Candidate* best = &candidates.front();
    auto best_distance = std::numeric_limits<double>::max();
    for (const auto& candidate : candidates) {
        const auto dx = static_cast<double>(position.x) - candidate.x;
        const auto dy = static_cast<double>(position.y) - candidate.y;
        const auto distance = dx * dx + dy * dy;
        if (distance < best_distance) {
            best = &candidate;
            best_distance = distance;
        }
    }
    runtime.set_field_entity_world_position_for_dump(
        static_cast<std::uint16_t>(best->x),
        static_cast<std::uint16_t>(best->y));
    return best->input;
}

inline bool map_coverage_near_rect(
    const std::uint16_t x, const std::uint16_t y,
    const MapCoverageRect& rect) {
    const auto x_gap = x < rect.x0 ? rect.x0 - x
        : x > rect.x1 ? x - rect.x1 : 0U;
    const auto y_gap = y < rect.y0 ? rect.y0 - y
        : y > rect.y1 ? y - rect.y1 : 0U;
    return x_gap <= 0x180U && y_gap <= 0x180U;
}

template <typename StartupState>
inline MapCoverageAttempt map_coverage_drive_rect(
    FieldSceneRuntime& runtime, const StartupState&,
    const std::uint16_t source_scene, const MapCoverageRect& rect,
    const std::optional<std::uint32_t> expected_event,
    const std::uint32_t frame_budget, const std::string_view row_category,
    const std::string_view trigger) {
    MapCoverageAttempt result;
    result.setup = true;
    const auto starting_scene = runtime.field_scene_reference_for_dump();
    if (trigger.find("camX>=0x1400") != std::string_view::npos) {
        runtime.set_field_camera_world_position_for_dump(0x1400U, 0U);
    } else if (trigger.find("camX<0x1400") != std::string_view::npos) {
        runtime.set_field_camera_world_position_for_dump(0x1300U, 0U);
    }
    const auto direction = map_coverage_direction(runtime, rect);
    const auto target_floor = trigger.find("floor=");
    const auto target_cursor = trigger.find("input_cursor=");
    const auto selector = target_floor != std::string_view::npos
        ? map_coverage_number(trigger.substr(target_floor + 6U))
        : (target_cursor != std::string_view::npos
            ? map_coverage_number(trigger.substr(target_cursor + 13U))
            : std::optional<std::uint32_t>{});
    bool selector_driven = false;
    bool selector_done = false;
    std::uint32_t near_rect_frames = 0U;
    std::uint32_t frames_used = 0U;
    std::uint16_t last_scene = starting_scene;
    while (frames_used < frame_budget) {
        const auto position = runtime.field_entity_world_position();
        const bool in_rect = position.x >= rect.x0 && position.x <= rect.x1 &&
                             position.y >= rect.y0 && position.y <= rect.y1;
        const bool near_rect = map_coverage_near_rect(position.x, position.y, rect);
        if (near_rect) {
            ++near_rect_frames;
        }
        const auto before_event = runtime.field_mode_event_code_for_dump();
        const auto port = !result.trigger_seen && near_rect &&
                                  (near_rect_frames % 8U == 1U)
            ? 0x40U
            : (result.trigger_seen ? 0U : direction);
        runtime.set_controller_port_byte(port);
        runtime.advance_frame();
        ++frames_used;
        const auto event = runtime.field_mode_event_code_for_dump();
        if (event != 0U) {
            result.observed_event = event;
        }
        if (expected_event && *expected_event == 0U && in_rect) {
            result.trigger_seen = true;
        } else if (expected_event && event == *expected_event) {
            result.trigger_seen = true;
        }
        if ((row_category == "elevator" &&
             runtime.elevator_selector_open_for_dump()) ||
            (row_category == "travel" &&
             (runtime.field_interaction_open() || runtime.dialogue_open() ||
              runtime.field_mode_text_open_for_dump()))) {
            result.trigger_seen = true;
        }
        if (before_event != 0U && expected_event &&
            *expected_event != 0U && before_event == *expected_event) {
            result.trigger_seen = true;
        }
        const auto scene = runtime.field_scene_reference_for_dump();
        const bool transition_pending = runtime.field_transition_pending_for_dump();
        if (!result.backup_captured &&
            (scene != starting_scene || transition_pending)) {
            result.backup_x = position.x;
            result.backup_y = position.y;
            result.backup_captured = true;
        }
        if (scene != source_scene || scene != last_scene ||
            transition_pending) {
            result.transition_fired = true;
        }
        last_scene = scene;
        if (scene != starting_scene) {
            break;
        }
        if (result.trigger_seen && !selector_driven &&
            ((row_category == "elevator" && selector.has_value()) ||
             trigger.find("Call Taxi menu") != std::string_view::npos ||
             trigger.find("menu=Vid-Phone") != std::string_view::npos)) {
            selector_driven = true;
            const bool taxi_route =
                trigger.find("Call Taxi menu") != std::string_view::npos;
            const bool vid_phone =
                trigger.find("menu=Vid-Phone") != std::string_view::npos;
            if (taxi_route && frames_used + 2U <= frame_budget) {
                map_coverage_tap(runtime, 0x40U);
                frames_used += 2U;
                for (std::uint32_t wait = 0U;
                     wait < 4U && frames_used < frame_budget; ++wait) {
                    runtime.advance_frame();
                    ++frames_used;
                }
            }
            const auto steps = row_category == "elevator" && selector
                ? *selector : (vid_phone ? 1U : (selector ? *selector : 0U));
            for (std::uint32_t tap = 0U;
                 tap < steps && frames_used + 2U <= frame_budget; ++tap) {
                map_coverage_tap(runtime, 0x02U);
                frames_used += 2U;
            }
            if (frames_used + 2U <= frame_budget) {
                map_coverage_tap(runtime, 0x40U);
                frames_used += 2U;
                selector_done = true;
            }
            if (vid_phone && frames_used + 2U <= frame_budget) {
                map_coverage_tap(runtime, 0x10U);
                frames_used += 2U;
            }
        }
    }
    runtime.set_controller_port_byte(0U);
    for (std::uint32_t tail = 0U; tail < 60U && frames_used < frame_budget; ++tail) {
        runtime.advance_frame();
        ++frames_used;
        const auto event = runtime.field_mode_event_code_for_dump();
        if (event != 0U) {
            result.observed_event = event;
        }
        const auto scene = runtime.field_scene_reference_for_dump();
        result.transition_fired = result.transition_fired ||
            scene != starting_scene || runtime.field_transition_pending_for_dump();
        if (scene != starting_scene) {
            break;
        }
    }
    result.frames_used = frames_used;
    result.observed_scene = runtime.field_scene_reference_for_dump();
    const auto position = runtime.field_entity_world_position();
    result.observed_x = position.x;
    result.observed_y = position.y;
    result.observed_mode = runtime.field_interaction_mode_for_dump();
    if (!result.trigger_seen) {
        result.note = "controller input did not produce the ROM event";
    } else if (selector.has_value() && !selector_done) {
        result.note = "menu selector did not reach its ROM option";
    }
    return result;
}

inline std::string map_coverage_sanitize(std::string value) {
    std::replace(value.begin(), value.end(), '\t', ' ');
    std::replace(value.begin(), value.end(), '\r', ' ');
    std::replace(value.begin(), value.end(), '\n', ' ');
    return value;
}

template <typename StartupState>
inline int run_map_coverage_qa(
    const StartupState& startup_state, const std::filesystem::path& rows_path,
    const std::filesystem::path& results_path) {
    const auto rows = map_coverage_read_rows(rows_path);
    if (rows.empty()) {
        std::cerr << "map coverage: row input is empty or malformed\n";
        return 4;
    }
    std::ofstream output(results_path);
    if (!output) {
        std::cerr << "map coverage: cannot open result output\n";
        return 4;
    }
    output << "row_id\tcategory\tattempted\tstatus\tobserved_event"
              "\ttransition_fired\tdest_scene\tarrival_xy\tarrival_mode\tdiff\n";
    std::size_t tested = 0U;
    std::size_t total_attempted = 0U;
    for (const auto& row : rows) {
        bool attempted = row.category == "edge" || row.category == "door" ||
                         row.category == "bexit" || row.category == "elevator" ||
                         row.category == "cave" || row.category == "travel";
        MapCoverageAttempt attempt;
        std::string status;
        std::string diff;
        std::string observed_event = "-";
        std::string observed_scene = "-";
        std::string observed_xy = "-";
        std::string observed_mode = "-";
        const bool dynamic_static = row.category == "spawn" ||
                                    row.category == "revent" ||
                                    row.category == "trigger";
        if (dynamic_static) {
            status = row.port_ref == "MISSING" ? "MISSING" : "PRESENT-UNTESTED";
            diff = row.port_ref == "MISSING"
                ? "no static port reference" : "static port presence only";
            attempted = false;
        } else if (!attempted) {
            status = "MISSING";
            diff = "unsupported matrix category";
        } else {
            ++total_attempted;
            const auto source_scene = map_coverage_scene(row.scene);
            auto physical_trigger = row.trigger;
            auto rect = map_coverage_rect(physical_trigger);
            std::optional<std::uint32_t> expected_event =
                map_coverage_event(physical_trigger);
            if (!rect && row.category == "travel" &&
                physical_trigger.find("Call Taxi menu") != std::string::npos &&
                source_scene) {
                const auto terminal = std::find_if(
                    rows.begin(), rows.end(), [&](const MapCoverageRow& candidate) {
                        return candidate.category == "travel" &&
                               candidate.scene == row.scene &&
                               candidate.row_id.find("travel-terminal-") == 0U;
                    });
                if (terminal != rows.end()) {
                    physical_trigger = terminal->trigger;
                    rect = map_coverage_rect(physical_trigger);
                    expected_event = map_coverage_event(physical_trigger);
                }
            }
            if (!source_scene) {
                attempt.note = "ROM continuation has no fixed source scene";
                status = "MISSING";
                diff = attempt.note;
            } else {
                std::optional<MapCoverageRow> paired_entry;
                auto bootstrap_scene = *source_scene;
                const auto bexit_entry = row.category == "bexit"
                    ? row.rom_addr.find("entry=0x")
                    : std::string::npos;
                if (bexit_entry != std::string::npos) {
                    const auto address_start = bexit_entry + 6U;
                    const auto address_end = row.rom_addr.find(';', address_start);
                    const auto entry_address = map_coverage_number(
                        std::string_view(row.rom_addr).substr(
                            address_start,
                            address_end == std::string::npos
                                ? address_end : address_end - address_start));
                    const auto entry = std::find_if(
                        rows.begin(), rows.end(), [&](const MapCoverageRow& candidate) {
                            return candidate.category == "door" && entry_address &&
                                   map_coverage_record_address(candidate.rom_addr) ==
                                       entry_address;
                        });
                    if (entry != rows.end()) {
                        paired_entry = *entry;
                        if (const auto outside_scene = map_coverage_scene(entry->scene)) {
                            bootstrap_scene = *outside_scene;
                        }
                    }
                }
                FieldSceneRuntime runtime([](std::string_view) {});
                if (runtime.bootstrap_for_scene_dump(startup_state, bootstrap_scene)) {
                    attempt.setup = true;
                    bool source_ready = true;
                    if (bexit_entry != std::string::npos) {
                        if (!paired_entry) {
                            source_ready = false;
                            attempt.note = "paired ROM entrance row is unavailable";
                        } else {
                            const auto outside_scene = map_coverage_scene(paired_entry->scene);
                            const auto outside_rect = map_coverage_rect(paired_entry->trigger);
                            if (!outside_scene || !outside_rect ||
                                runtime.field_scene_reference_for_dump() != *outside_scene) {
                                source_ready = false;
                                attempt.note = "paired entrance scene could not be bootstrapped";
                            } else {
                                const auto entered = map_coverage_drive_rect(
                                    runtime, startup_state, *outside_scene,
                                    *outside_rect, map_coverage_event(paired_entry->trigger),
                                    600U, "door", paired_entry->trigger);
                                attempt = entered;
                                if (!entered.transition_fired ||
                                    runtime.field_scene_reference_for_dump() != *source_scene) {
                                    source_ready = false;
                                    attempt.note = "paired entrance did not produce the ROM source scene";
                                } else if (const auto mode = map_coverage_mode(row.gate)) {
                                    if (runtime.field_interaction_mode_for_dump() != *mode) {
                                        source_ready = false;
                                        attempt.note = "paired entrance mode differs from ROM gate";
                                    }
                                }
                            }
                        }
                    } else if (const auto mode = map_coverage_mode(row.gate)) {
                        runtime.set_field_interaction_mode_for_dump(*mode);
                    } else if (row.category == "cave") {
                        runtime.set_field_interaction_mode_for_dump(3U);
                    } else if (row.category == "travel" && *source_scene <= 6U) {
                        runtime.set_field_interaction_mode_for_dump(0U);
                    }
                    if (!source_ready) {
                        status = "MISSING";
                        diff = attempt.note;
                    } else if (!rect) {
                        std::uint8_t input = 0U;
                        if (row.category == "edge" &&
                            row.row_id.find("edge-rule-") == 0U) {
                            if (physical_trigger.find("camX>=") != std::string::npos) {
                                input = 0x08U;
                                runtime.set_field_camera_world_position_for_dump(
                                    0x1400U, 0U);
                                runtime.set_field_entity_world_position_for_dump(
                                    0x1500U, 0x2000U);
                            } else if (physical_trigger.find("camX<") != std::string::npos) {
                                input = 0x04U;
                                runtime.set_field_camera_world_position_for_dump(
                                    0x1300U, 0U);
                                runtime.set_field_entity_world_position_for_dump(
                                    0x1200U, 0x2000U);
                            } else {
                                input = 0x0FU;
                            }
                        } else if (row.category == "travel" &&
                                   row.trigger.find("party-down") != std::string::npos) {
                            input = 0x01U;
                        }
                        for (std::uint32_t frame = 0U; frame < 600U; ++frame) {
                            runtime.set_controller_port_byte(input);
                            runtime.advance_frame();
                            const auto event = runtime.field_mode_event_code_for_dump();
                            if (event != 0U) {
                                attempt.observed_event = event;
                                attempt.trigger_seen = true;
                            }
                            attempt.transition_fired =
                                attempt.transition_fired ||
                                runtime.field_scene_reference_for_dump() != bootstrap_scene ||
                                runtime.field_transition_pending_for_dump();
                            if (runtime.field_scene_reference_for_dump() != bootstrap_scene) {
                                break;
                            }
                        }
                        runtime.set_controller_port_byte(0U);
                        attempt.note = row.category == "travel" &&
                                       row.trigger.find("party-down") != std::string::npos
                            ? "party-down prerequisite not reached by map input"
                            : row.category == "edge"
                                ? "edge input did not fire the ROM edge transition"
                                : "ROM row has no concrete position trigger";
                        const auto position = runtime.field_entity_world_position();
                        attempt.observed_scene = runtime.field_scene_reference_for_dump();
                        attempt.observed_x = position.x;
                        attempt.observed_y = position.y;
                        attempt.observed_mode = runtime.field_interaction_mode_for_dump();
                        attempt.frames_used = 600U;
                    } else {
                        const auto remaining_frames = row.category == "bexit"
                            ? (attempt.frames_used < 600U
                                  ? 600U - attempt.frames_used : 0U)
                            : 600U;
                        const auto driven = map_coverage_drive_rect(
                            runtime, startup_state, *source_scene, *rect,
                            expected_event, remaining_frames, row.category, row.trigger);
                        auto combined = driven;
                        combined.setup = true;
                        combined.frames_used += attempt.frames_used;
                        if (row.category == "bexit") {
                            combined.backup_x = attempt.backup_x;
                            combined.backup_y = attempt.backup_y;
                            combined.backup_captured = attempt.backup_captured;
                        }
                        attempt = std::move(combined);
                        if (row.category == "cave" &&
                            row.gate.find("FFFC10=") != std::string::npos) {
                            attempt.note = "cave room gate not established by a natural incoming route";
                            attempt.trigger_seen = false;
                        }
                    }
                    const bool death_wake_row = row.category == "travel" &&
                        row.trigger.find("party-down") != std::string::npos;
                    if (status.empty() && death_wake_row) {
                        status = "MISSING";
                        diff = attempt.note.empty()
                            ? "party-down prerequisite was not reached"
                            : attempt.note;
                    } else if (status.empty() && row.port_ref == "MISSING") {
                        status = "MISSING";
                        diff = "no port reference; " + attempt.note;
                    } else if (status.empty() && !attempt.transition_fired &&
                               row.row_id.find("travel-terminal-") == 0U &&
                               attempt.trigger_seen) {
                        status = "PASS";
                        diff = "terminal event observed; no fixed map destination";
                    } else if (status.empty() && !attempt.transition_fired &&
                               row.row_id.find("travel-vidphone-") == 0U &&
                               attempt.trigger_seen) {
                        status = "PASS";
                        diff = "Vid-Phone event observed; ROM handler returns to source scene";
                    } else if (status.empty() && !attempt.transition_fired) {
                        status = "MISSING";
                        diff = attempt.note.empty()
                            ? "no map transition within 600 input frames"
                            : attempt.note;
                    } else if (status.empty()) {
                        bool matches = true;
                        const auto expected_destination =
                            map_coverage_number(row.dest_scene);
                        if (expected_destination &&
                            attempt.observed_scene != *expected_destination) {
                            matches = false;
                            diff += "dest expected=" + std::to_string(*expected_destination) +
                                    " observed=" + std::to_string(attempt.observed_scene) + ";";
                        }
                        const auto expected_mode = map_coverage_number(row.arrival_mode);
                        if (expected_mode && attempt.observed_mode != *expected_mode) {
                            matches = false;
                            diff += "mode expected=" + std::to_string(*expected_mode) +
                                    " observed=" + std::to_string(attempt.observed_mode) + ";";
                        }
                        const auto expected_xy = map_coverage_xy(row.arrival_xy);
                        if (expected_xy &&
                            (attempt.observed_x != (*expected_xy)[0] ||
                             attempt.observed_y != (*expected_xy)[1])) {
                            matches = false;
                            diff += "xy expected=" + map_coverage_hex((*expected_xy)[0]) +
                                    "," + map_coverage_hex((*expected_xy)[1]) +
                                    " observed=" + map_coverage_hex(attempt.observed_x) +
                                    "," + map_coverage_hex(attempt.observed_y) + ";";
                        }
                        if (row.category == "bexit") {
                            if (!attempt.backup_captured ||
                                attempt.observed_x != attempt.backup_x ||
                                attempt.observed_y != attempt.backup_y) {
                                matches = false;
                                diff += "restored FFDE26 expected=" +
                                    (attempt.backup_captured
                                        ? map_coverage_hex(attempt.backup_x) + "," +
                                              map_coverage_hex(attempt.backup_y)
                                        : std::string("unobserved")) +
                                    " observed=" + map_coverage_hex(attempt.observed_x) +
                                    "," + map_coverage_hex(attempt.observed_y) + ";";
                            }
                        }
                        if (row.dest_scene == "D0" ||
                            row.dest_scene == "menu-selected" ||
                            row.arrival_mode == "selected route") {
                            matches = false;
                            diff += "ROM destination or mode remains dynamic;";
                        }
                        if (matches) {
                            status = "PASS";
                            diff = "observed route matches all resolved ROM outputs";
                        } else {
                            status = "WRONG";
                        }
                    }
                } else {
                    status = "MISSING";
                    diff = "fresh scene bootstrap failed";
                }
            }
            observed_event = attempt.observed_event == 0U
                ? (expected_event && *expected_event == 0U && attempt.trigger_seen
                       ? "0x0" : "-")
                : map_coverage_hex(attempt.observed_event);
            if (attempt.setup) {
                observed_scene = std::to_string(attempt.observed_scene);
                observed_xy = map_coverage_hex(attempt.observed_x) + "," +
                              map_coverage_hex(attempt.observed_y);
                observed_mode = std::to_string(attempt.observed_mode);
            }
            ++tested;
        }
        output << map_coverage_sanitize(row.row_id) << '\t'
               << map_coverage_sanitize(row.category) << '\t'
               << (attempted ? "1" : "0") << '\t'
               << status << '\t' << observed_event << '\t'
               << (attempt.transition_fired ? "1" : "0") << '\t'
               << observed_scene << '\t' << observed_xy << '\t'
               << observed_mode << '\t' << map_coverage_sanitize(diff) << '\n';
    }
    output.close();
    std::unordered_map<std::string, std::string> expected_categories;
    for (const auto& row : rows) {
        expected_categories.emplace(row.row_id, row.category);
    }
    std::ifstream check(results_path);
    std::string line;
    std::set<std::string> seen;
    std::size_t output_count = 0U;
    bool integrity_ok = static_cast<bool>(output) &&
                        static_cast<bool>(std::getline(check, line));
    while (integrity_ok && std::getline(check, line)) {
        const auto fields = map_coverage_split_tsv(line);
        if (fields.size() != 10U ||
            expected_categories.find(fields[0]) == expected_categories.end() ||
            !seen.insert(fields[0]).second) {
            integrity_ok = false;
            break;
        }
        const auto& category = expected_categories.at(fields[0]);
        const bool runtime_category = category == "edge" || category == "door" ||
            category == "bexit" || category == "elevator" || category == "cave" ||
            category == "travel";
        if ((runtime_category && fields[2] != "1") ||
            (!runtime_category && fields[2] != "0") || fields[1] != category) {
            integrity_ok = false;
            break;
        }
        ++output_count;
    }
    if (!integrity_ok || seen.size() != rows.size() || output_count != rows.size() ||
        tested != total_attempted) {
        std::cerr << "map coverage: output integrity mismatch; expected_rows="
                  << rows.size() << " output_rows=" << output_count
                  << " runtime_attempts=" << total_attempted << '\n';
        return 4;
    }
    std::cout << "map_coverage_rows=" << rows.size()
              << " runtime_attempts=" << total_attempted
              << " output=" << results_path.string() << '\n';
    return 0;
}

}  // namespace shadowrun::ghidra_conversion::qt_runtime::qa
