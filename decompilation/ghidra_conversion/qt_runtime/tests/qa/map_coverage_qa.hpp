#pragma once

#include "../../src/field/field_scene_runtime.hpp"

#include <QImage>
#include <QPainter>

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
    std::string script;
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
    std::uint32_t trigger_frame{};
    std::uint32_t transition_pending_frame{};
    std::uint32_t first_new_scene_frame{};
    std::uint16_t observed_scene{};
    std::uint16_t observed_x{};
    std::uint16_t observed_y{};
    std::uint16_t backup_x{};
    std::uint16_t backup_y{};
    bool backup_captured{};
    std::uint16_t lead_x{};
    std::uint16_t lead_y{};
    std::uint8_t door_facing{};
    std::uint8_t door_bank{0xFFU};
    bool lead_captured{};
    std::uint16_t observed_mode{};
    std::uint8_t observed_event{};
    std::uint8_t observed_fade_steps{};
    std::uint32_t playfield_lit_pixels{};
    std::string approach_side;
    std::uint32_t prompt_inputs{};
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
        if (fields.size() != 11U) {
            return {};
        }
        rows.push_back(MapCoverageRow{
            fields[0], fields[1], fields[2], fields[3], fields[4],
            fields[5], fields[6], fields[7], fields[8], fields[9], fields[10]});
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

inline std::uint32_t map_coverage_playfield_lit_pixels(const QImage& image) {
    std::uint32_t lit = 0U;
    const auto width = std::min(image.width(), 272);
    const auto height = std::min(image.height(), 224);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto pixel = image.pixel(x, y);
            if (qRed(pixel) != 0 || qGreen(pixel) != 0 || qBlue(pixel) != 0) {
                ++lit;
            }
        }
    }
    return lit;
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

inline std::optional<std::uint32_t> map_coverage_trigger_number(
    std::string_view trigger, const std::string_view name) {
    std::string marker(name);
    marker.push_back('=');
    const auto marker_start = trigger.find(marker);
    if (marker_start == std::string_view::npos) {
        return std::nullopt;
    }
    const auto start = marker_start + marker.size();
    const auto end = trigger.find(';', start);
    return map_coverage_number(trigger.substr(
        start, end == std::string_view::npos ? end : end - start));
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

inline std::optional<std::string> map_coverage_return_side(
    std::string_view trigger) {
    const auto marker = trigger.find("return_side=");
    if (marker == std::string_view::npos) {
        return std::nullopt;
    }
    const auto start = marker + 12U;
    const auto end = trigger.find(';', start);
    const auto side = trigger.substr(
        start, end == std::string_view::npos ? end : end - start);
    if (side == "north" || side == "east" || side == "south" || side == "west") {
        return std::string(side);
    }
    return std::nullopt;
}

inline std::vector<std::string> map_coverage_approach_sides(
    std::string_view trigger) {
    const std::array<std::string_view, 4> all{{
        "north", "east", "south", "west"}};
    std::vector<std::string> sides;
    if (const auto preferred = map_coverage_return_side(trigger)) {
        sides.push_back(*preferred);
    }
    for (const auto side : all) {
        if (std::find(sides.begin(), sides.end(), side) == sides.end()) {
            sides.emplace_back(side);
        }
    }
    return sides;
}

inline bool map_coverage_side_possible(
    const MapCoverageRect& rect, const std::string_view side) {
    if (side == "north") return rect.y0 >= 0x80U;
    if (side == "east") return rect.x1 <= 0xFF7FU;
    if (side == "south") return rect.y1 <= 0xFF7FU;
    if (side == "west") return rect.x0 >= 0x80U;
    return false;
}

inline std::uint8_t map_coverage_position_on_side(
    FieldSceneRuntime& runtime, const MapCoverageRect& rect,
    const std::string_view side) {
    const std::uint32_t cx = (static_cast<std::uint32_t>(rect.x0) + rect.x1) / 2U;
    const std::uint32_t cy = (static_cast<std::uint32_t>(rect.y0) + rect.y1) / 2U;
    std::uint32_t x = cx;
    std::uint32_t y = cy;
    std::uint8_t input = 0U;
    if (side == "north" && rect.y0 >= 0x80U) {
        y = rect.y0 - 0x80U;
        input = 0x02U;
    } else if (side == "east" && rect.x1 <= 0xFF7FU) {
        x = rect.x1 + 0x80U;
        input = 0x04U;
    } else if (side == "south" && rect.y1 <= 0xFF7FU) {
        y = rect.y1 + 0x80U;
        input = 0x01U;
    } else if (side == "west" && rect.x0 >= 0x80U) {
        x = rect.x0 - 0x80U;
        input = 0x08U;
    }
    runtime.set_field_entity_world_position_for_dump(
        static_cast<std::uint16_t>(x), static_cast<std::uint16_t>(y));
    return input;
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

// ROM 0xBC36/0xBC66: the return bank is bits 5-6 of the first door-cell
// collision byte (FFEF10..12, written by 0xE8A0 from the door record code),
// not the party facing. The lead word of each 12-byte 0xBD34 entry is added
// to the lead position held when the door probe hit (before that step).
inline std::optional<std::array<std::int16_t, 2>>
map_coverage_return_offset(const std::uint8_t bank) {
    switch (bank) {
    case 0U: return std::array<std::int16_t, 2>{0, -0x0140};   // 0xBD34
    case 1U: return std::array<std::int16_t, 2>{0x01E0, 0};    // 0xBD40
    case 2U: return std::array<std::int16_t, 2>{0, 0x0140};    // 0xBD4C
    case 3U: return std::array<std::int16_t, 2>{-0x01E0, 0};   // 0xBD58
    default: return std::nullopt;
    }
}

inline std::optional<std::uint8_t> map_coverage_rom_bank(
    std::string_view trigger) {
    const auto marker = trigger.find(";bank=");
    if (marker == std::string_view::npos ||
        marker + 6U >= trigger.size()) {
        return std::nullopt;
    }
    const char digit = trigger[marker + 6U];
    if (digit < '0' || digit > '3') {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(digit - '0');
}

inline std::optional<std::string> map_coverage_side_hint(
    std::string_view trigger, const std::string_view name) {
    std::string marker = ";";
    marker.append(name);
    marker.push_back('=');
    const auto start_marker = trigger.find(marker);
    if (start_marker == std::string_view::npos) {
        return std::nullopt;
    }
    const auto start = start_marker + marker.size();
    const auto end = trigger.find(';', start);
    const auto side = trigger.substr(
        start, end == std::string_view::npos ? end : end - start);
    if (side == "north" || side == "east" || side == "south" ||
        side == "west") {
        return std::string(side);
    }
    return std::nullopt;
}

inline std::uint8_t map_coverage_script_prompt_button(
    std::string_view script) {
    if (script == "0x053F98" || script == "0x020484") {
        return 0x40U; // A accepts the ROM prompt branch.
    }
    if (script == "0x05432E") {
        // Title service 18 (0x15FD5C+0x48): scene 6 event 1 shows text and
        // both branches set d6=-1 (0x5438A, 0x5439C); A dismisses it.
        return 0x40U;
    }
    if (script == "0x05448A") {
        return 0x10U; // B exercises the alternate ROM prompt branch.
    }
    return 0U;
}

template <typename StartupState>
inline MapCoverageAttempt map_coverage_drive_rect(
    FieldSceneRuntime& runtime, const StartupState&,
    const std::uint16_t source_scene, const MapCoverageRect& rect,
    const std::optional<std::uint32_t> expected_event,
    const std::uint32_t frame_budget, const std::string_view row_category,
    const std::string_view trigger, const std::string_view approach_side = {},
    const std::string_view script = {},
    std::uint32_t* frame_clock = nullptr) {
    MapCoverageAttempt result;
    result.setup = true;
    const auto starting_scene = runtime.field_scene_reference_for_dump();
    const auto starting_route_flags =
        runtime.field_movement_route_flags_for_dump();
    const auto starting_collision_dispatches =
        runtime.field_map_event_system_for_dump().collision_dispatch_count();
    std::uint32_t frames_used = 0U;
    bool elevator_cooldown_ready = true;
    if (row_category == "elevator") {
        while (runtime.world_activity_request_timeout_for_dump() != 0U &&
               frames_used < 60U) {
            runtime.set_controller_nibble(0U);
            runtime.advance_frame();
            ++frames_used;
        }
        elevator_cooldown_ready =
            runtime.world_activity_request_timeout_for_dump() == 0U;
        if (!elevator_cooldown_ready) {
            result.note = "prior world activity did not release within the source 60-frame cooldown";
        }
    }
    if (row_category == "elevator" && source_scene == 26U) {
        // Match the scene-26 elevator QA's post-entry walk checkpoint after
        // its real scene-2 event-1 route has loaded the destination.
        runtime.set_field_entity_world_position_for_dump(0x0A00U, 0x0500U);
        runtime.set_field_camera_world_position_for_dump(0x0200U, 0U);
        runtime.set_field_entity_heading_for_dump(0U);
    }
    if (trigger.find("camX>=0x1400") != std::string_view::npos) {
        runtime.set_field_camera_world_position_for_dump(0x1400U, 0U);
    } else if (trigger.find("camX<0x1400") != std::string_view::npos) {
        runtime.set_field_camera_world_position_for_dump(0x1300U, 0U);
    }
    const auto direction = row_category == "elevator"
        ? static_cast<std::uint8_t>(0x01U)
        : approach_side.empty()
            ? map_coverage_direction(runtime, rect)
            : map_coverage_position_on_side(runtime, rect, approach_side);
    result.approach_side = std::string(approach_side);
    const auto target_floor = map_coverage_trigger_number(trigger, "floor");
    const auto target_cursor = map_coverage_trigger_number(trigger, "input_cursor");
    const auto selector = target_floor ? target_floor : target_cursor;
    bool selector_driven = false;
    bool selector_done = false;
    std::uint32_t near_rect_frames = 0U;
    std::uint32_t prompt_inputs_used = 0U;
    const auto prompt_button = map_coverage_script_prompt_button(script);
    const bool script_stays_on_scene = prompt_button != 0U;
    std::uint16_t last_scene = starting_scene;
    if (row_category == "elevator" && source_scene >= 31U &&
        source_scene <= 34U) {
        for (std::uint8_t align = 0U; align < 6U; ++align) {
            runtime.set_controller_nibble(0x08U);
            runtime.advance_frame();
            ++frames_used;
        }
        runtime.set_controller_nibble(0U);
    }
    while (frames_used < frame_budget && elevator_cooldown_ready) {
        const auto position = runtime.field_entity_world_position();
        const bool in_rect = position.x >= rect.x0 && position.x <= rect.x1 &&
                             position.y >= rect.y0 && position.y <= rect.y1;
        const bool near_rect = map_coverage_near_rect(position.x, position.y, rect);
        if (near_rect) {
            ++near_rect_frames;
        }
        const auto before_event = runtime.field_mode_event_code_for_dump();
        const auto port = row_category == "elevator"
            ? (selector_driven ? 0U : direction)
            : row_category != "cave" && !result.trigger_seen && near_rect &&
                                  (near_rect_frames % 8U == 1U)
                ? 0x40U
                : (result.trigger_seen ? 0U : direction);
        const auto probe_before = runtime.field_collision_probe_state_for_dump();
        const bool door_already_fired = result.transition_fired;
        const bool trigger_was_seen = result.trigger_seen;
        const auto collision_dispatches_before = runtime
            .field_map_event_system_for_dump().collision_dispatch_count();
        runtime.set_controller_port_byte(port);
        if (frame_clock != nullptr) {
            ++*frame_clock;
        }
        runtime.advance_frame();
        ++frames_used;
        const auto event = runtime.field_mode_event_code_for_dump();
        if (event != 0U) {
            result.observed_event = event;
        }
        const auto& last_collision_dispatch = runtime
            .field_map_event_system_for_dump().last_collision_dispatch();
        const bool fresh_cave_collision = row_category == "cave" &&
            expected_event.has_value() &&
            runtime.field_map_event_system_for_dump()
                    .collision_dispatch_count() > collision_dispatches_before &&
            last_collision_dispatch.has_value() &&
            static_cast<std::uint32_t>(
                last_collision_dispatch->decision.source_event_code) ==
                *expected_event;
        if (expected_event && *expected_event == 0U && in_rect) {
            result.trigger_seen = true;
        } else if (fresh_cave_collision) {
            // FFFBF9 survives an incoming cave-room transition. Require a
            // new matching collision dispatch, but use its source event
            // rather than bounding the probe coordinate by the actor's exit
            // rectangle: the ROM collision probe may lead the actor across
            // that rectangle edge. The route check below validates the full
            // destination scene, room and spawn before accepting the exit.
            result.trigger_seen = true;
        } else if (row_category != "cave" && expected_event &&
                   event == *expected_event) {
            result.trigger_seen = true;
        }
        if ((row_category == "elevator" &&
             runtime.elevator_selector_open_for_dump()) ||
            (row_category == "travel" &&
             (runtime.field_interaction_open() || runtime.dialogue_open() ||
              runtime.field_mode_text_open_for_dump()))) {
            result.trigger_seen = true;
        }
        if (row_category != "cave" && before_event != 0U && expected_event &&
            *expected_event != 0U && before_event == *expected_event) {
            result.trigger_seen = true;
        }
        if (!trigger_was_seen && result.trigger_seen) {
            result.trigger_frame = frame_clock != nullptr
                ? *frame_clock
                : frames_used;
        }
        const auto scene = runtime.field_scene_reference_for_dump();
        if (scene != starting_scene && result.first_new_scene_frame == 0U) {
            result.first_new_scene_frame = frame_clock != nullptr
                ? *frame_clock
                : frames_used;
        }
        const bool transition_pending = runtime.field_transition_pending_for_dump();
        if (transition_pending && result.transition_pending_frame == 0U) {
            result.transition_pending_frame = frame_clock != nullptr
                ? *frame_clock
                : frames_used;
        }
        // The door contact is the frame on which the port records its probe
        // triplet; the ROM saves from the lead position held before that
        // frame's step (0xB834 -> 0xBC36 runs before the move is applied).
        // Blocked-move recoveries can also record a triplet, so the last
        // recording before the door transition fires is the one kept.
        const auto probe_after = runtime.field_collision_probe_state_for_dump();
        const bool door_contact_recorded = row_category == "door" &&
            (probe_after.initialized != probe_before.initialized ||
             probe_after.points[0].x != probe_before.points[0].x ||
             probe_after.points[0].y != probe_before.points[0].y);
        if (!door_already_fired && door_contact_recorded) {
            result.lead_x = position.x;
            result.lead_y = position.y;
            result.door_facing = runtime.field_entity_direction_for_dump();
            result.lead_captured = true;
            const auto bank = map_coverage_rom_bank(trigger);
            const auto offset = bank ? map_coverage_return_offset(*bank)
                                     : std::nullopt;
            if (offset) {
                result.door_bank = *bank;
                result.backup_x = static_cast<std::uint16_t>(
                    static_cast<std::int32_t>(position.x) + (*offset)[0]);
                result.backup_y = static_cast<std::uint16_t>(
                    static_cast<std::int32_t>(position.y) + (*offset)[1]);
                result.backup_captured = true;
            }
        }
        if (prompt_button != 0U && prompt_inputs_used < 1U &&
            (runtime.dialogue_open() || runtime.field_mode_text_open_for_dump() ||
             runtime.field_interaction_open()) &&
            frames_used + 2U <= frame_budget) {
            map_coverage_tap(runtime, prompt_button);
            frames_used += 2U;
            ++prompt_inputs_used;
            result.prompt_inputs = prompt_inputs_used;
        }
        if (scene != source_scene || scene != last_scene ||
            (transition_pending && !script_stays_on_scene)) {
            result.transition_fired = true;
        }
        last_scene = scene;
        if (scene != starting_scene) {
            break;
        }
        if (result.trigger_seen && !selector_driven &&
            row_category == "elevator" && selector.has_value() &&
            runtime.elevator_selector_open_for_dump()) {
            selector_driven = true;
            runtime.set_controller_port_byte(0U);
            const auto wait_frames = [&](const std::uint32_t count) {
                if (count > frame_budget - frames_used) {
                    return false;
                }
                for (std::uint32_t wait = 0U; wait < count; ++wait) {
                    runtime.advance_frame();
                    ++frames_used;
                    if (runtime.field_scene_reference_for_dump() !=
                        starting_scene) {
                        return false;
                    }
                }
                return true;
            };
            // The source route releases the opening input, lets the selector
            // settle, then drives the selected floor through the same inputs
            // as the scene-26/group QA routes.
            bool selector_sequence_ready = wait_frames(31U);
            const auto current_floor = map_coverage_trigger_number(
                trigger, "current_floor");
            const auto actual_current_floor = static_cast<std::uint32_t>(
                runtime.elevator_selector_current_floor_for_dump());
            const auto target_floor = *selector;
            if (!current_floor || actual_current_floor != *current_floor ||
                target_floor >= 6U || !runtime.elevator_selector_open_for_dump()) {
                selector_sequence_ready = false;
                result.note = "selector opened at a floor different from its ROM group";
            }
            if (selector_sequence_ready) {
                if (frames_used + 2U > frame_budget) {
                    selector_sequence_ready = false;
                } else {
                    map_coverage_tap(runtime, 0x20U); // source B selector input
                    frames_used += 2U;
                }
            }
            if (selector_sequence_ready) {
                auto selected_floor = actual_current_floor;
                while (selected_floor != target_floor) {
                    const bool moving_up = selected_floor < target_floor;
                    const auto next_floor = moving_up
                        ? selected_floor + 1U : selected_floor - 1U;
                    if (frames_used + 2U > frame_budget) {
                        selector_sequence_ready = false;
                        break;
                    }
                    map_coverage_tap(
                        runtime, moving_up ? 0x01U : 0x02U);
                    frames_used += 2U;
                    if (runtime.elevator_selector_selected_floor_for_dump() !=
                        next_floor ||
                        !wait_frames(8U)) {
                        selector_sequence_ready = false;
                        result.note = "selector D-pad input did not select the next floor";
                        break;
                    }
                    selected_floor = next_floor;
                }
            }
            if (selector_sequence_ready &&
                runtime.elevator_selector_selected_floor_for_dump() ==
                    target_floor && frames_used + 2U <= frame_budget) {
                const auto floor_distance = target_floor > actual_current_floor
                    ? target_floor - actual_current_floor
                    : actual_current_floor - target_floor;
                map_coverage_tap(runtime, 0x40U); // source A starts the lift
                frames_used += 2U;
                const auto lift_wait = floor_distance * 17U * 8U - 1U;
                if (wait_frames(lift_wait) &&
                    !runtime.elevator_selector_animating_for_dump() &&
                    runtime.elevator_selector_current_floor_for_dump() ==
                        target_floor &&
                    runtime.elevator_selector_selected_floor_for_dump() ==
                        target_floor && frames_used + 2U <= frame_budget) {
                    map_coverage_tap(runtime, 0x80U); // source Start commits
                    frames_used += 2U;
                    selector_done = true;
                } else {
                    result.note = "selector lift did not settle at the ROM floor";
                }
            } else if (selector_sequence_ready) {
                result.note = "selector floor could not be confirmed within the frame budget";
            }
        } else if (result.trigger_seen && !selector_driven &&
            (trigger.find("Call Taxi menu") != std::string_view::npos ||
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
            const auto steps = vid_phone ? 1U : (selector ? *selector : 0U);
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
    const auto tail_limit = row_category == "edge" ? 36U : 60U;
    for (std::uint32_t tail = 0U;
         tail < tail_limit && frames_used < frame_budget; ++tail) {
        if (frame_clock != nullptr) {
            ++*frame_clock;
        }
        runtime.advance_frame();
        ++frames_used;
        const auto event = runtime.field_mode_event_code_for_dump();
        if (event != 0U) {
            result.observed_event = event;
        }
        const auto scene = runtime.field_scene_reference_for_dump();
        const bool transition_pending =
            runtime.field_transition_pending_for_dump();
        if (transition_pending && result.transition_pending_frame == 0U) {
            result.transition_pending_frame = frame_clock != nullptr
                ? *frame_clock
                : frames_used;
        }
        if (scene != starting_scene && result.first_new_scene_frame == 0U) {
            result.first_new_scene_frame = frame_clock != nullptr
                ? *frame_clock
                : frames_used;
        }
        result.transition_fired = result.transition_fired ||
            scene != starting_scene ||
            (transition_pending && !script_stays_on_scene);
        if (scene != starting_scene && row_category != "edge") {
            break;
        }
    }
    result.frames_used = frames_used;
    result.observed_scene = runtime.field_scene_reference_for_dump();
    const auto position = runtime.field_entity_world_position();
    result.observed_x = position.x;
    result.observed_y = position.y;
    result.observed_mode = runtime.field_interaction_mode_for_dump();
    if (row_category == "edge" && result.transition_fired &&
        result.observed_scene != starting_scene) {
        result.observed_fade_steps =
            runtime.field_palette_fade_steps_for_dump();
        QImage image(320, 224, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::black);
        QPainter painter(&image);
        runtime.paint(painter);
        painter.end();
        result.playfield_lit_pixels =
            map_coverage_playfield_lit_pixels(image);
    }
    if (approach_side.empty() &&
        (row_category == "edge" || row_category == "door" ||
         row_category == "bexit")) {
        result.approach_side = "nearest";
    }
    if (result.note.empty() && !result.trigger_seen &&
        row_category == "elevator") {
        const auto movement = runtime.last_field_movement_transition_for_dump();
        const auto position = runtime.field_entity_world_position();
        result.note = "elevator selector absent;position=" +
            map_coverage_hex(position.x) + "," + map_coverage_hex(position.y) +
            ";mode_event=" + map_coverage_hex(
                runtime.field_mode_event_code_for_dump()) +
            ";collision=" + map_coverage_hex(
                runtime.field_terrain_collision_code_for_dump()) +
            ";movement_event=" + map_coverage_hex(
                static_cast<std::uint8_t>(movement.event_code)) +
            ";activity_timeout=" + std::to_string(
                runtime.world_activity_request_timeout_for_dump());
    } else if (result.note.empty() && row_category == "elevator" && selector_done &&
               !result.transition_fired) {
        result.note = "selector confirmed without a scene handoff;current=" +
            std::to_string(runtime.elevator_selector_current_floor_for_dump()) +
            ";selected=" + std::to_string(
                runtime.elevator_selector_selected_floor_for_dump()) +
            ";selector_open=" + std::to_string(
                runtime.elevator_selector_open_for_dump()) +
            ";transition_phase=" + std::to_string(static_cast<unsigned>(
                runtime.elevator_scene_transition_phase_for_dump())) +
            ";pending=" + std::to_string(
                runtime.field_transition_pending_for_dump());
    } else if (result.note.empty() && row_category == "cave" &&
               !result.trigger_seen) {
        const auto movement = runtime.last_field_movement_transition_for_dump();
        const auto& signs = runtime.field_directional_probe_signs_for_dump();
        result.note = "controller input did not produce the ROM event;position=" +
            map_coverage_hex(position.x) + "," + map_coverage_hex(position.y) +
            ";mode=" + std::to_string(result.observed_mode) +
            ";mode_event=" + map_coverage_hex(
                runtime.field_mode_event_code_for_dump()) +
            ";collision=" + map_coverage_hex(
                runtime.field_terrain_collision_code_for_dump()) +
            ";movement_event=" + map_coverage_hex(
                static_cast<std::uint8_t>(movement.event_code)) +
            ";movement_kind=" + std::to_string(
                static_cast<unsigned>(movement.kind)) +
            ";route_flags=" + map_coverage_hex(
                runtime.field_movement_route_flags_for_dump()) +
            ";entry_route_flags=" + map_coverage_hex(starting_route_flags) +
            ";interaction_transition=" + std::to_string(
                runtime.field_interaction_transition_active_for_dump()) +
            ";field_pending=" + std::to_string(
                runtime.field_transition_pending_for_dump()) +
            ";activity_timeout=" + std::to_string(
                runtime.world_activity_request_timeout_for_dump()) +
            ";negative_probes=" + std::to_string(movement.negative_probe_count) +
            ";probe_signs=" + std::to_string(signs[0]) + "," +
                std::to_string(signs[1]) + "," + std::to_string(signs[2]) +
                "," + std::to_string(signs[3]);
    } else if (result.note.empty() && !result.trigger_seen) {
        result.note = "controller input did not produce the ROM event";
    } else if (result.note.empty() && selector.has_value() && !selector_done) {
        result.note = "menu selector did not reach its ROM option";
    }
    if (row_category == "cave") {
        const auto& dispatch = runtime.field_map_event_system_for_dump()
            .last_collision_dispatch();
        result.note += ";collision_dispatch_delta=" + std::to_string(
            runtime.field_map_event_system_for_dump().collision_dispatch_count() -
            starting_collision_dispatches);
        if (dispatch.has_value()) {
            result.note += ";last_dispatch=scene:" + std::to_string(
                dispatch->input.scene_id) + ",mode:" + std::to_string(
                    dispatch->input.world_mode) + ",room:" + std::to_string(
                    dispatch->input.field_transition_context) + ",gate:" +
                map_coverage_hex(
                    dispatch->input.interaction_gate) + ",xy:" +
                map_coverage_hex(dispatch->input.world_x) + "," +
                map_coverage_hex(dispatch->input.world_y) + ",collision:" +
                map_coverage_hex(dispatch->input.raw_collision_code) +
                ",flags:" + map_coverage_hex(
                    dispatch->input.field_global_flags) + ",event:" +
                std::to_string(static_cast<unsigned>(
                    dispatch->decision.source_event_code)) + "->" +
                std::to_string(static_cast<unsigned>(
                    dispatch->decision.event_code)) + ",action:" +
                std::to_string(static_cast<unsigned>(
                    dispatch->decision.action));
        } else {
            result.note += ";last_dispatch=none";
        }
    }
    return result;
}

struct MapCoverageApproachProbe {
    bool event_found{};
    bool entry_passed{};
    std::string entry_side;
    std::string approach_side;
    std::string tried_sides;
};

inline void map_coverage_set_mode(
    FieldSceneRuntime& runtime, const std::string_view category,
    const std::uint16_t scene, const std::string_view gate) {
    if (const auto mode = map_coverage_mode(gate)) {
        runtime.set_field_interaction_mode_for_dump(*mode);
    } else if (category == "cave") {
        runtime.set_field_interaction_mode_for_dump(3U);
    } else if (category == "travel" && scene <= 6U) {
        runtime.set_field_interaction_mode_for_dump(0U);
    }
}

struct MapCoverageCaveRouteStep {
    std::string_view row_id;
    std::uint16_t source_room;
    std::uint16_t destination_room;
};

struct MapCoverageCaveCollisionRoute {
    std::vector<std::uint16_t> waypoint_cells;
    std::uint16_t event_cell = 0U;
    std::uint8_t event_input = 0U;
    MapCoverageRect trigger{};
    std::optional<std::uint32_t> expected_event;
    std::string trigger_description;
};

inline std::optional<MapCoverageCaveCollisionRoute>
map_coverage_cave_collision_path(
    FieldSceneRuntime& runtime, const MapCoverageRect& trigger,
    const std::optional<std::uint32_t> expected_event,
    const std::string_view trigger_description,
    std::string* route_analysis = nullptr,
    const std::array<bool, 16384>* blocked_cells = nullptr) {
    if (!expected_event.has_value()) {
        return std::nullopt;
    }
    constexpr std::string_view kDirectionMarker = ";direction=";
    const auto direction_marker = trigger_description.rfind(kDirectionMarker);
    if (direction_marker == std::string_view::npos) {
        if (route_analysis != nullptr) {
            *route_analysis = "ROM exit direction missing";
        }
        return std::nullopt;
    }
    const auto direction_start = direction_marker + kDirectionMarker.size();
    const auto direction_end = trigger_description.find(';', direction_start);
    const auto direction = trigger_description.substr(
        direction_start,
        direction_end == std::string_view::npos
            ? direction_end : direction_end - direction_start);
    std::uint8_t event_input = 0U;
    if (direction == "north") {
        event_input = 0x01U;
    } else if (direction == "east") {
        event_input = 0x08U;
    } else if (direction == "south") {
        event_input = 0x02U;
    } else if (direction == "west") {
        event_input = 0x04U;
    } else {
        if (route_analysis != nullptr) {
            *route_analysis = "ROM exit direction is unrecognized";
        }
        return std::nullopt;
    }
    constexpr std::uint16_t kCollisionMapSide = 64U;
    constexpr std::size_t kCollisionMapCellCount =
        kCollisionMapSide * kCollisionMapSide;
    constexpr std::uint16_t kRouteGridSide = 128U;
    constexpr std::size_t kRouteGridCellCount =
        kRouteGridSide * kRouteGridSide;
    const auto map_cell = runtime.source_terrain_collision_map_for_dump();
    const auto active_actor_positions =
        runtime.active_default_actor_positions_for_dump();
    const auto start_position = runtime.field_entity_world_position();
    const auto start_x = static_cast<std::uint32_t>(start_position.x) >> 8U;
    const auto start_y = static_cast<std::uint32_t>(start_position.y) >> 8U;
    if (start_x >= kRouteGridSide || start_y >= kRouteGridSide) {
        return std::nullopt;
    }

    std::array<bool, kCollisionMapCellCount> goals{};
    const auto first_x = static_cast<std::uint32_t>(trigger.x0) >> 9U;
    const auto last_x = static_cast<std::uint32_t>(trigger.x1) >> 9U;
    const auto first_y = static_cast<std::uint32_t>(trigger.y0) >> 9U;
    const auto last_y = static_cast<std::uint32_t>(trigger.y1) >> 9U;
    const auto route_context = ";world_rect=" +
        map_coverage_hex(trigger.x0) + ".." + map_coverage_hex(trigger.x1) +
        "," + map_coverage_hex(trigger.y0) + ".." +
        map_coverage_hex(trigger.y1) + ";grid_rect=" +
        std::to_string(first_x) + ".." + std::to_string(last_x) + "," +
        std::to_string(first_y) + ".." + std::to_string(last_y) +
        ";direction=" + std::string(direction);
    for (auto y = first_y; y <= last_y && y < kCollisionMapSide; ++y) {
        for (auto x = first_x; x <= last_x && x < kCollisionMapSide; ++x) {
            const auto index = static_cast<std::size_t>(
                y * kCollisionMapSide + x);
            const auto collision = map_cell[index];
            if (collision != 0U &&
                (collision & 0x1FU) == *expected_event) {
                goals[index] = true;
            }
        }
    }
    if (std::none_of(goals.begin(), goals.end(), [](const bool goal) {
            return goal;
        })) {
        if (route_analysis != nullptr) {
            *route_analysis = "event_cells=0" + route_context;
        }
        return std::nullopt;
    }
    const auto event_cell_count = static_cast<std::size_t>(std::count(
        goals.begin(), goals.end(), true));

    // The byte map omits blocking geometry represented by terrain-object
    // records. Classify each candidate waypoint with the runtime's ordinary
    // terrain collision query so the QA route does not plan through those
    // records and then mistake the resulting wall contact for a game exit bug.
    std::array<bool, kRouteGridCellCount> passable_cells{};
    std::size_t passable_event_cell_count = 0U;
    constexpr std::int64_t kActorClearance = 0x0000;
    constexpr std::int64_t kActorClearanceSquared =
        kActorClearance * kActorClearance;
    for (std::size_t index = 0U; index < passable_cells.size(); ++index) {
        if (blocked_cells != nullptr && (*blocked_cells)[index]) {
            continue;
        }
        const auto route_x = static_cast<std::uint32_t>(
            index % kRouteGridSide);
        const auto route_y = static_cast<std::uint32_t>(
            index / kRouteGridSide);
        const auto map_index = static_cast<std::size_t>(
            (route_y / 2U) * kCollisionMapSide + route_x / 2U);
        if (map_cell[map_index] != 0U && !goals[map_index]) {
            continue;
        }
        const auto x = static_cast<std::uint16_t>(
            route_x * 0x0100U + 0x0080U);
        const auto y = static_cast<std::uint16_t>(
            route_y * 0x0100U + 0x0080U);
        const bool clear_of_active_actors = std::none_of(
            active_actor_positions.begin(), active_actor_positions.end(),
            [x, y](const SceneRunnerPosition& actor_position) {
                const auto dx = static_cast<std::int64_t>(x) -
                    actor_position.x;
                const auto dy = static_cast<std::int64_t>(y) -
                    actor_position.y;
                return dx * dx + dy * dy <= kActorClearanceSquared;
            });
        passable_cells[index] = clear_of_active_actors &&
            runtime.probe_field_collision_for_dump(x, y) == 0;
        if (goals[map_index] && passable_cells[index]) {
            ++passable_event_cell_count;
        }
    }

    constexpr std::int32_t kUnvisited = -2;
    constexpr std::int32_t kPathStart = -1;
    std::array<std::int32_t, kRouteGridCellCount> parent{};
    parent.fill(kUnvisited);
    std::array<std::int32_t, kRouteGridCellCount> event_for_approach{};
    event_for_approach.fill(kUnvisited);
    std::array<std::uint8_t, kRouteGridCellCount> input_for_approach{};
    std::size_t clear_event_approach_count = 0U;
    struct DirectionalApproach {
        std::int8_t x;
        std::int8_t y;
        std::uint8_t input;
    };
    constexpr std::array<DirectionalApproach, 4> kApproaches{{
        {0, -1, 0x01U}, {1, 0, 0x08U},
        {0, 1, 0x02U}, {-1, 0, 0x04U}}};
    std::size_t preferred_direction_index = 0U;
    if (event_input == 0x08U) {
        preferred_direction_index = 1U;
    } else if (event_input == 0x02U) {
        preferred_direction_index = 2U;
    } else if (event_input == 0x04U) {
        preferred_direction_index = 3U;
    }
    for (std::size_t event_cell = 0U; event_cell < goals.size(); ++event_cell) {
        if (!goals[event_cell]) {
            continue;
        }
        const auto event_x = static_cast<std::int32_t>(
            event_cell % kCollisionMapSide);
        const auto event_y = static_cast<std::int32_t>(
            event_cell / kCollisionMapSide);
        // The ROM direction identifies the exit-table slot. Physical approach
        // still has to respect the live collision geometry, so try that side
        // first and retain any other clear neighbor around the event cell.
        for (std::size_t rank = 0U; rank < kApproaches.size(); ++rank) {
            const auto direction = kApproaches[
                (preferred_direction_index + rank) % kApproaches.size()];
            const auto approach_x = event_x - direction.x;
            const auto approach_y = event_y - direction.y;
            if (approach_x < 0 || approach_x >= kCollisionMapSide ||
                approach_y < 0 || approach_y >= kCollisionMapSide) {
                continue;
            }
            for (std::int32_t sub_y = 0; sub_y < 2; ++sub_y) {
                for (std::int32_t sub_x = 0; sub_x < 2; ++sub_x) {
                    const auto approach = static_cast<std::uint16_t>(
                        (approach_y * 2 + sub_y) * kRouteGridSide +
                        approach_x * 2 + sub_x);
                    if (!passable_cells[approach]) {
                        continue;
                    }
                    if (event_for_approach[approach] == kUnvisited) {
                        ++clear_event_approach_count;
                        event_for_approach[approach] =
                            static_cast<std::int32_t>(event_cell);
                        input_for_approach[approach] = direction.input;
                    }
                }
            }
        }
    }
    std::array<std::uint16_t, kRouteGridCellCount> pending{};
    std::size_t pending_read = 0U;
    std::size_t pending_write = 0U;
    const auto start_index = static_cast<std::uint16_t>(
        start_y * kRouteGridSide + start_x);
    parent[start_index] = kPathStart;
    pending[pending_write++] = start_index;
    const auto passable = [&passable_cells](const std::uint16_t index) {
        return passable_cells[index];
    };
    constexpr std::array<std::pair<std::int8_t, std::int8_t>, 8> kNeighbors{{
        {0, -1}, {1, 0}, {0, 1}, {-1, 0},
        {1, -1}, {1, 1}, {-1, 1}, {-1, -1}}};
    std::optional<std::uint16_t> approach_index;
    std::optional<std::uint16_t> event_index;
    while (pending_read < pending_write) {
        const auto current = pending[pending_read++];
        if (event_for_approach[current] != kUnvisited) {
            approach_index = current;
            event_index = static_cast<std::uint16_t>(
                event_for_approach[current]);
            event_input = input_for_approach[current];
            break;
        }
        const auto x = static_cast<std::int32_t>(
            current % kRouteGridSide);
        const auto y = static_cast<std::int32_t>(
            current / kRouteGridSide);
        for (const auto [dx, dy] : kNeighbors) {
            const auto next_x = x + dx;
            const auto next_y = y + dy;
            if (next_x < 0 || next_x >= kRouteGridSide ||
                next_y < 0 || next_y >= kRouteGridSide) {
                continue;
            }
            const auto next = static_cast<std::uint16_t>(
                next_y * kRouteGridSide + next_x);
            if (parent[next] != kUnvisited || !passable(next)) {
                continue;
            }
            if (dx != 0 && dy != 0) {
                const auto horizontal = static_cast<std::uint16_t>(
                    y * kRouteGridSide + next_x);
                const auto vertical = static_cast<std::uint16_t>(
                    next_y * kRouteGridSide + x);
                if (!passable(horizontal) || !passable(vertical)) {
                    continue;
                }
            }
            parent[next] = current;
            pending[pending_write++] = next;
        }
    }
    if (!approach_index.has_value() || !event_index.has_value()) {
        if (route_analysis != nullptr) {
            *route_analysis = "event_cells=" +
                std::to_string(event_cell_count) +
                ";collision_clear_event_cells=" +
                std::to_string(passable_event_cell_count) +
                ";clear_directional_approaches=" +
                std::to_string(clear_event_approach_count) +
                ";collision_reachable_cells=" +
                std::to_string(pending_read) + route_context;
            const auto first_event = static_cast<std::size_t>(
                std::find(goals.begin(), goals.end(), true) - goals.begin());
            const auto event_x = static_cast<std::int32_t>(
                first_event % kCollisionMapSide);
            const auto event_y = static_cast<std::int32_t>(
                first_event / kCollisionMapSide);
            *route_analysis += ";event_neighborhood=";
            for (std::int32_t y = event_y - 1; y <= event_y + 1; ++y) {
                for (std::int32_t x = event_x - 1; x <= event_x + 1; ++x) {
                    if (x < 0 || x >= kCollisionMapSide || y < 0 ||
                        y >= kCollisionMapSide) {
                        continue;
                    }
                    const auto cell = static_cast<std::size_t>(
                        y * kCollisionMapSide + x);
                    const auto world_x = static_cast<std::uint16_t>(
                        x * 0x0200U + 0x0100U);
                    const auto world_y = static_cast<std::uint16_t>(
                        y * 0x0200U + 0x0100U);
                    if (route_analysis->back() != '=') {
                        route_analysis->push_back(',');
                    }
                    *route_analysis += std::to_string(x) + ":" +
                        std::to_string(y) + "=" +
                        map_coverage_hex(map_cell[cell]) + "/" +
                        map_coverage_hex(static_cast<std::uint16_t>(
                            runtime.probe_field_collision_for_dump(
                                world_x, world_y)));
                }
            }
        }
        return std::nullopt;
    }

    std::vector<std::uint16_t> path;
    for (auto cell = static_cast<std::int32_t>(*approach_index);
         cell != kPathStart; cell = parent[static_cast<std::size_t>(cell)]) {
        if (cell < 0) {
            return std::nullopt;
        }
        path.push_back(static_cast<std::uint16_t>(cell));
    }
    std::reverse(path.begin(), path.end());
    MapCoverageCaveCollisionRoute route{};
    route.waypoint_cells = std::move(path);
    route.event_cell = *event_index;
    route.event_input = event_input;
    route.trigger = trigger;
    route.expected_event = expected_event;
    route.trigger_description = std::string(trigger_description);
    if (route_analysis != nullptr) {
        *route_analysis = "event_cells=" +
            std::to_string(event_cell_count) +
            ";collision_clear_event_cells=" +
            std::to_string(passable_event_cell_count) +
            ";clear_directional_approaches=" +
            std::to_string(clear_event_approach_count) +
            ";path_cells=" + std::to_string(route.waypoint_cells.size());
    }
    return route;
}

inline std::uint8_t map_coverage_cave_path_input(
    const std::uint16_t from, const std::uint16_t to) {
    const auto from_x = static_cast<std::int32_t>(from % 128U);
    const auto from_y = static_cast<std::int32_t>(from / 128U);
    const auto to_x = static_cast<std::int32_t>(to % 128U);
    const auto to_y = static_cast<std::int32_t>(to / 128U);
    std::uint8_t input = 0U;
    if (to_x < from_x) input |= 0x04U;
    if (to_x > from_x) input |= 0x08U;
    if (to_y < from_y) input |= 0x01U;
    if (to_y > from_y) input |= 0x02U;
    return input;
}

inline MapCoverageAttempt map_coverage_drive_cave_collision_path(
    FieldSceneRuntime& runtime, const std::uint16_t source_scene,
    MapCoverageCaveCollisionRoute route,
    const std::optional<std::uint32_t> expected_event) {
    constexpr std::uint32_t kFrameBudget = 4800U;
    constexpr std::uint32_t kFramesPerCell = 48U;
    constexpr std::size_t kMaximumReplans = 8U;
    std::array<bool, 16384> blocked_cells{};
    std::size_t replan_count = 0U;
    std::string last_replan_analysis;
    MapCoverageAttempt result;
    result.setup = true;
    result.approach_side = "collision-map-route";
    const auto starting_collision_dispatches =
        runtime.field_map_event_system_for_dump().collision_dispatch_count();
    std::uint16_t active_target_cell = 0U;
    std::uint8_t active_input = 0U;
    const auto record_frame = [&] {
        const auto event = runtime.field_mode_event_code_for_dump();
        if (event != 0U) {
            result.observed_event = event;
        }
        if (expected_event.has_value()) {
            const auto& dispatch = runtime.field_map_event_system_for_dump()
                .last_collision_dispatch();
            if (runtime.field_map_event_system_for_dump()
                    .collision_dispatch_count() >
                    starting_collision_dispatches &&
                dispatch.has_value() &&
                static_cast<std::uint32_t>(
                    dispatch->decision.source_event_code) == *expected_event) {
                result.trigger_seen = true;
            }
        }
        result.transition_fired =
            runtime.field_scene_reference_for_dump() != source_scene;
        const auto position = runtime.field_entity_world_position();
        result.observed_scene = runtime.field_scene_reference_for_dump();
        result.observed_x = position.x;
        result.observed_y = position.y;
        result.observed_mode = runtime.field_interaction_mode_for_dump();
    };
    const auto advance_with_input = [&](const std::uint8_t input) {
        const auto before_scene = runtime.field_scene_reference_for_dump();
        const auto before_position = runtime.field_entity_world_position();
        const auto before_event = runtime.field_mode_event_code_for_dump();
        const auto source_tile = static_cast<std::uint16_t>(
            (static_cast<std::uint32_t>(before_position.y) >> 9U) * 64U +
            (static_cast<std::uint32_t>(before_position.x) >> 9U));
        const auto source_tile_byte =
            runtime.terrain_collision_map_byte_for_dump(source_tile);
        const auto source_default_actor_count =
            runtime.default_actor_count_for_dump();
        const auto source_dynamic_actor_count =
            runtime.dynamic_world_actor_count_for_dump();
        const auto source_script_actor_count =
            runtime.world_activity_script_actor_count_for_dump();
        const auto source_activity_countdown =
            runtime.world_frame_logic_for_dump().activity_countdown;
        const auto source_activity_timeout =
            runtime.world_activity_timeout_for_dump().timeout_frames;
        const auto source_request_timeout =
            runtime.world_activity_request_timeout_for_dump();
        const auto source_probe = static_cast<std::uint16_t>(
            runtime.probe_field_collision_for_dump(
                before_position.x, before_position.y));
        std::string nearby_actors;
        for (std::uint16_t source_order = 0U; source_order < 32U;
             ++source_order) {
            const auto actor_position =
                runtime.default_actor_world_position_for_dump(source_order);
            if (!actor_position.has_value()) continue;
            const auto dx = std::abs(
                static_cast<std::int32_t>(actor_position->x) -
                static_cast<std::int32_t>(before_position.x));
            const auto dy = std::abs(
                static_cast<std::int32_t>(actor_position->y) -
                static_cast<std::int32_t>(before_position.y));
            if (dx <= 0x0800 && dy <= 0x0800) {
                if (!nearby_actors.empty()) nearby_actors.push_back(',');
                const auto actor_id =
                    runtime.default_actor_instance_id_for_dump(source_order);
                const auto actor_profile =
                    runtime.default_actor_script_profile_for_dump(source_order);
                const auto ai_package =
                    runtime.default_actor_ai_package_for_dump(source_order);
                const auto target_distance =
                    runtime.default_actor_ai_target_distance_for_dump(
                        source_order);
                nearby_actors += std::to_string(source_order) + ":" +
                    actor_id.value_or("?") + ":" +
                    actor_profile.value_or("?") + ":" +
                    ai_package.value_or("?") + ":" +
                    (target_distance.has_value()
                         ? std::to_string(*target_distance) : "?") + "@" +
                    map_coverage_hex(actor_position->x) + "," +
                    map_coverage_hex(actor_position->y);
            }
        }
        runtime.set_controller_port_byte(input);
        runtime.advance_frame();
        ++result.frames_used;
        record_frame();
        if (runtime.field_scene_reference_for_dump() != before_scene) {
            result.note += ";transition_from_scene=" +
                std::to_string(before_scene) + ";transition_from_xy=" +
                map_coverage_hex(before_position.x) + "," +
                map_coverage_hex(before_position.y) +
                ";transition_frame_event=" + map_coverage_hex(before_event) +
                ";transition_source_tile=" + map_coverage_hex(source_tile_byte) +
                ";transition_source_probe=" + map_coverage_hex(source_probe) +
                ";transition_target=" + std::to_string(
                    active_target_cell % 128U) + "," +
                    std::to_string(active_target_cell / 128U) +
                ";transition_input=" + map_coverage_hex(active_input) +
                ";source_actor_counts=" + std::to_string(
                    source_default_actor_count) + "," +
                    std::to_string(source_dynamic_actor_count) + "," +
                    std::to_string(source_script_actor_count) +
                ";source_activity_timers=" + std::to_string(
                    source_activity_countdown) + "," +
                    std::to_string(source_activity_timeout) + "," +
                    std::to_string(source_request_timeout) +
                ";nearby_actors=" +
                    (nearby_actors.empty() ? "none" : nearby_actors);
        }
    };

    std::size_t waypoint_index = 1U;
    bool route_failed = false;
    while (waypoint_index < route.waypoint_cells.size() &&
           result.frames_used < kFrameBudget) {
        const auto target_cell = route.waypoint_cells[waypoint_index];
        active_target_cell = target_cell;
        const auto target_x = static_cast<std::uint16_t>(
            (target_cell % 128U) * 0x0100U + 0x0080U);
        const auto target_y = static_cast<std::uint16_t>(
            (target_cell / 128U) * 0x0100U + 0x0080U);
        const auto segment_limit = std::min(
            kFramesPerCell, kFrameBudget - result.frames_used);
        bool reached_target_cell = false;
        for (std::uint32_t frame = 0U; frame < segment_limit; ++frame) {
            const auto position = runtime.field_entity_world_position();
            const auto position_cell = static_cast<std::uint16_t>(
                ((static_cast<std::uint32_t>(position.y) >> 8U) * 128U) +
                (static_cast<std::uint32_t>(position.x) >> 8U));
            const auto difference_x = static_cast<std::int32_t>(target_x) -
                static_cast<std::int32_t>(position.x);
            const auto difference_y = static_cast<std::int32_t>(target_y) -
                static_cast<std::int32_t>(position.y);
            const bool centered_on_waypoint =
                std::abs(difference_x) <= 0x80 &&
                std::abs(difference_y) <= 0x80;
            if (position_cell == target_cell || centered_on_waypoint) {
                reached_target_cell = true;
                break;
            }
            std::uint8_t input = 0U;
            if (difference_x < -0x80) input |= 0x04U;
            if (difference_x > 0x80) input |= 0x08U;
            if (difference_y < -0x80) input |= 0x01U;
            if (difference_y > 0x80) input |= 0x02U;
            active_input = input;
            advance_with_input(input);
            if (result.trigger_seen || result.transition_fired) {
                break;
            }
        }
        if (result.trigger_seen || result.transition_fired) {
            break;
        }
        if (!reached_target_cell) {
            const auto position = runtime.field_entity_world_position();
            result.note = "collision-map route stalled before cell=" +
                std::to_string(target_cell % 128U) + "," +
                std::to_string(target_cell / 128U) + ";position=" +
                map_coverage_hex(position.x) + "," +
                map_coverage_hex(position.y);
            if (replan_count >= kMaximumReplans) {
                route_failed = true;
                break;
            }
            blocked_cells[target_cell] = true;
            ++replan_count;
            const auto replanned = map_coverage_cave_collision_path(
                runtime, route.trigger, route.expected_event,
                route.trigger_description, &last_replan_analysis,
                &blocked_cells);
            if (!replanned.has_value()) {
                result.note += ";replan_failed=" + last_replan_analysis;
                route_failed = true;
                break;
            }
            route = *replanned;
            waypoint_index = 1U;
            result.note.clear();
            continue;
        }
        ++waypoint_index;
    }

    if (result.transition_fired &&
        runtime.field_scene_reference_for_dump() != source_scene) {
        const auto interrupted_scene =
            runtime.field_scene_reference_for_dump();
        for (std::uint32_t frame = 0U;
             frame < 600U && result.frames_used < kFrameBudget &&
             runtime.field_scene_reference_for_dump() != source_scene;
             ++frame) {
            advance_with_input(0U);
        }
        if (runtime.field_scene_reference_for_dump() == source_scene) {
            result.transition_fired = false;
            result.trigger_seen = false;
            waypoint_index = 1U;
            if (const auto resumed = map_coverage_cave_collision_path(
                    runtime, route.trigger, route.expected_event,
                    route.trigger_description, &last_replan_analysis,
                    &blocked_cells);
                resumed.has_value()) {
                route = *resumed;
                route_failed = false;
            } else {
                route_failed = true;
                result.note += ";route_after_activity_failed=" +
                    last_replan_analysis;
            }
        } else {
            result.note += ";activity_wait_did_not_return=" +
                std::to_string(interrupted_scene) + "->" +
                std::to_string(runtime.field_scene_reference_for_dump());
        }
    }

    runtime.set_controller_port_byte(0U);
    const bool reached_approach =
        !route_failed && waypoint_index >= route.waypoint_cells.size();
    if (reached_approach) {
        for (std::uint32_t frame = 0U;
             frame < 120U && result.frames_used < kFrameBudget; ++frame) {
            advance_with_input(route.event_input);
            if (result.trigger_seen || result.transition_fired) {
                break;
            }
        }
    }
    runtime.set_controller_port_byte(0U);
    if (result.trigger_seen && !result.transition_fired) {
        for (std::uint32_t tail = 0U;
             tail < 60U && result.frames_used < kFrameBudget; ++tail) {
            advance_with_input(0U);
            if (result.transition_fired) {
                break;
            }
        }
    }
    runtime.set_controller_port_byte(0U);
    if (result.note.empty() && !result.trigger_seen) {
        result.note = "directional collision-map approach ended without a fresh event";
    }
    result.note += ";path_cells=" +
        std::to_string(route.waypoint_cells.size()) +
        ";event_cell=" + std::to_string(route.event_cell) +
        ";replans=" + std::to_string(replan_count) +
        ";frames=" + std::to_string(result.frames_used);
    if (!last_replan_analysis.empty()) {
        result.note += ";last_replan=" + last_replan_analysis;
    }
    return result;
}

template <typename StartupState>
inline MapCoverageAttempt map_coverage_drive_cave_exit(
    FieldSceneRuntime& runtime, const StartupState& startup_state,
    const std::uint16_t source_scene, const MapCoverageRow& row,
    const MapCoverageRect& rect,
    const std::optional<std::uint32_t> expected_event) {
    // Prefer actual movement from the reached room spawn whenever the source
    // collision map connects that spawn to this event cell. The side probes
    // below remain only for rows whose source collision map has no route.
    std::string collision_route_analysis;
    if (const auto path = map_coverage_cave_collision_path(
            runtime, rect, expected_event, row.trigger,
            &collision_route_analysis);
        path.has_value()) {
        auto attempt = map_coverage_drive_cave_collision_path(
            runtime, source_scene, *path, expected_event);
        attempt.note += ";collision_route=" + collision_route_analysis;
        return attempt;
    }

    // ROM 0xA12 dispatches FFFBF9=0x08..0x0B while FFFC0E is set. Try each
    // geometrically possible approach with directional input; room context
    // remains the value produced by the production scene/exit route.
    constexpr std::uint32_t kCaveExitFrameBudget = 600U;
    MapCoverageAttempt last_attempt;
    const auto sides = map_coverage_approach_sides(row.trigger);
    const auto per_side_budget = std::max<std::uint32_t>(
        1U, kCaveExitFrameBudget /
                static_cast<std::uint32_t>(sides.size()));
    for (const auto& side : sides) {
        if (!map_coverage_side_possible(rect, side)) {
            continue;
        }
        last_attempt = map_coverage_drive_rect(
            runtime, startup_state, source_scene, rect, expected_event,
            per_side_budget, "cave", row.trigger, side, row.script);
        // transition_fired also records a pending world-activity request in
        // the generic driver. That request can occur without resolving a
        // cave exit (0x02CE0); keep trying this ROM exit until its fresh
        // collision is observed or the active scene actually changes.
        if (last_attempt.trigger_seen ||
            runtime.field_scene_reference_for_dump() != source_scene) {
            return last_attempt;
        }
    }
    if (!collision_route_analysis.empty()) {
        last_attempt.note += ";collision_route=" + collision_route_analysis;
    }
    return last_attempt;
}

inline std::optional<std::uint16_t> map_coverage_cave_target_room(
    const MapCoverageRow& row) {
    if (row.category != "cave") {
        return std::nullopt;
    }
    constexpr std::string_view kRoomMarker = "FFFC10=";
    const auto marker = row.gate.find(kRoomMarker);
    if (marker == std::string::npos) {
        return std::nullopt;
    }
    const auto start = marker + kRoomMarker.size();
    const auto end = row.gate.find(';', start);
    const auto room = map_coverage_number(std::string_view(row.gate).substr(
        start, end == std::string::npos ? end : end - start));
    if (!room || (*room > 3U && *room != 7U)) {
        return std::nullopt;
    }
    return static_cast<std::uint16_t>(*room);
}

inline std::vector<MapCoverageCaveRouteStep> map_coverage_cave_shortest_route(
    const std::uint16_t target_room) {
    // These are shortest directed paths in the ROM-backed cave exit graph.
    // The row IDs resolve each step to its real event-8..0x0B rectangle.
    switch (target_room) {
    case 0U:
        return {};
    case 1U:
        return {{"cave-room00-south-r14c0fe", 0U, 7U},
                {"cave-room07-south-r1d4e5e", 7U, 1U}};
    case 2U:
        return {{"cave-room00-north-r14c0ea", 0U, 5U},
                {"cave-room05-south-r1d4e5e", 5U, 2U}};
    case 3U:
        return {{"cave-room00-south-r14c0fe", 0U, 7U},
                {"cave-room07-south-r1d4e5e", 7U, 1U},
                {"cave-room01-east-r1b45a6", 1U, 11U},
                {"cave-room0b-south-r1d4e5e", 11U, 12U},
                {"cave-room0c-east-r1d57a2", 12U, 13U},
                {"cave-room0d-east-r1d4b94", 13U, 3U}};
    case 7U:
        return {{"cave-room00-south-r14c0fe", 0U, 7U}};
    default:
        return {};
    }
}

template <typename StartupState>
inline bool map_coverage_drive_cave_route_step(
    FieldSceneRuntime& runtime, const StartupState& startup_state,
    const std::vector<MapCoverageRow>& rows,
    const MapCoverageCaveRouteStep& step, std::string& evidence) {
    const auto row = std::find_if(rows.begin(), rows.end(), [&](const auto& candidate) {
        return candidate.row_id == step.row_id;
    });
    if (row == rows.end()) {
        evidence = "route row missing: " + std::string(step.row_id);
        return false;
    }
    const auto source_scene = map_coverage_scene(row->scene);
    const auto rect = map_coverage_rect(row->trigger);
    const auto expected_event = map_coverage_event(row->trigger);
    const auto expected_destination = map_coverage_scene(row->dest_scene);
    const auto expected_xy = map_coverage_xy(row->arrival_xy);
    // Cave handler 0x2CE0 selects the destination room/scene and spawn but
    // leaves FFDF40 untouched. Compare its output mode with the naturally
    // reached source mode; the table's legacy fixed mode 3 is not an input.
    const auto expected_mode = runtime.field_interaction_mode_for_dump();
    if (!source_scene || !rect || !expected_event || !expected_destination ||
        !expected_xy ||
        runtime.field_scene_reference_for_dump() != *source_scene ||
        runtime.field_transition_context_for_dump() != step.source_room) {
        evidence = "route source mismatch at " + std::string(step.row_id) +
            ";scene=" + std::to_string(
                runtime.field_scene_reference_for_dump()) +
            ";room=" + std::to_string(
                runtime.field_transition_context_for_dump());
        return false;
    }

    const auto attempt = map_coverage_drive_cave_exit(
        runtime, startup_state, *source_scene, *row, *rect, expected_event);
    const auto position = runtime.field_entity_world_position();
    // A completed transition to the ROM-selected scene/room/spawn is the
    // authoritative proof that this cave exit fired. The collision dispatch
    // may be sampled on the transition frame after the source room has
    // already been replaced, so trigger_seen can remain false even though
    // all destination fields match the source route exactly.
    if (!attempt.transition_fired ||
        runtime.field_scene_reference_for_dump() != *expected_destination ||
        runtime.field_transition_context_for_dump() != step.destination_room ||
        runtime.field_interaction_mode_for_dump() != expected_mode ||
        position.x != (*expected_xy)[0] || position.y != (*expected_xy)[1]) {
        evidence = "route step failed at " + std::string(step.row_id) +
            ";event=" + map_coverage_hex(attempt.observed_event) +
            ";transition_fired=" +
                (attempt.transition_fired ? "1" : "0") +
            ";scene=" + std::to_string(
                runtime.field_scene_reference_for_dump()) +
            ";expected_scene=" + std::to_string(*expected_destination) +
            ";room=" + std::to_string(
                runtime.field_transition_context_for_dump()) +
            ";expected_room=" + std::to_string(step.destination_room) +
            ";xy=" + map_coverage_hex(position.x) + "," +
                map_coverage_hex(position.y) +
            ";expected_xy=" + map_coverage_hex((*expected_xy)[0]) + "," +
                map_coverage_hex((*expected_xy)[1]) +
            ";mode=" + std::to_string(
                runtime.field_interaction_mode_for_dump()) +
            ";expected_mode=" + std::to_string(expected_mode) +
            ";probe=" + attempt.note;
        return false;
    }
    if (!evidence.empty()) {
        evidence.push_back(',');
    }
    evidence.append(step.row_id);
    evidence += "[mode=" + std::to_string(
        runtime.field_interaction_mode_for_dump()) +
        "/source=" + std::to_string(expected_mode) + "]";
    if (runtime.field_interaction_mode_for_dump() != expected_mode) {
        evidence += "(mode_mismatch)";
    }
    return true;
}

template <typename StartupState>
inline bool map_coverage_prepare_cave_target_room(
    FieldSceneRuntime& runtime, const StartupState& startup_state,
    const std::vector<MapCoverageRow>& rows, const std::uint16_t target_room,
    const std::vector<std::string>& source_boundaries,
    std::string& evidence) {
    constexpr std::uint16_t kCaveEntryScene = 6U;
    constexpr std::uint16_t kCaveEntryRoom = 0U;
    constexpr std::uint16_t kCaveEntryFieldMode = 0U;
    constexpr std::string_view kNaturalEntryBoundary =
        "field.dialogue.scene_entry.entry_room_seeded";
    if (runtime.field_scene_reference_for_dump() != kCaveEntryScene ||
        runtime.field_transition_context_for_dump() != kCaveEntryRoom ||
        runtime.field_interaction_mode_for_dump() != kCaveEntryFieldMode ||
        std::find(source_boundaries.begin(), source_boundaries.end(),
                  kNaturalEntryBoundary) == source_boundaries.end()) {
        evidence = "Scene 6 production entry did not seed room 0 and field mode 0";
        return false;
    }

    // ROM 0x136DE clears FFDF40 at scene entry. Keep that natural source mode
    // and the entry-seeded room context; the test never writes either value.
    evidence = "entry=Scene9-production-dialogue-transition->Scene6;room-route=0x00->";
    for (const auto& step : map_coverage_cave_shortest_route(target_room)) {
        const auto boundary_start = source_boundaries.size();
        if (!map_coverage_drive_cave_route_step(
                runtime, startup_state, rows, step, evidence)) {
            // Keep a compact transition-boundary trace beside a failed
            // natural route so the report distinguishes an absent mode
            // transition from an unresolved destination or scene commit.
            evidence += ";boundaries=";
            bool first = true;
            for (auto index = boundary_start;
                 index < source_boundaries.size(); ++index) {
                const auto& boundary = source_boundaries[index];
                if (boundary.find("mode_transition") == std::string::npos &&
                    boundary.find("field.movement") == std::string::npos &&
                    boundary.find("field.entry") == std::string::npos &&
                    boundary.find("field.exit.") == std::string::npos &&
                    boundary.find("field.world_activity") == std::string::npos &&
                    boundary.find("field.runtime.field_transition_requested") ==
                        std::string::npos) {
                    continue;
                }
                if (!first) evidence.push_back(',');
                first = false;
                evidence += boundary;
            }
            return false;
        }
        evidence += "->0x";
        std::ostringstream room;
        room << std::hex << std::uppercase << step.destination_room;
        evidence += room.str();
    }
    const auto row_scene = std::find_if(rows.begin(), rows.end(),
        [target_room](const MapCoverageRow& candidate) {
            return candidate.category == "cave" &&
                   map_coverage_cave_target_room(candidate) == target_room;
        });
    const auto expected_scene = row_scene == rows.end()
        ? std::nullopt : map_coverage_scene(row_scene->scene);
    if (!expected_scene ||
        runtime.field_scene_reference_for_dump() != *expected_scene ||
        runtime.field_transition_context_for_dump() != target_room) {
        evidence += ";target room was not reached";
        return false;
    }
    evidence += ";target=0x";
    std::ostringstream room;
    room << std::hex << std::uppercase << target_room;
    evidence += room.str();
    return true;
}

template <typename StartupState>
inline MapCoverageApproachProbe map_coverage_probe_approach_sides(
    const StartupState& startup_state, const MapCoverageRow& row,
    const std::uint16_t source_scene, const MapCoverageRect& rect,
    const std::optional<std::uint32_t> expected_event) {
    MapCoverageApproachProbe probe;
    for (const auto& side : map_coverage_approach_sides(row.trigger)) {
        if (!map_coverage_side_possible(rect, side)) {
            continue;
        }
        if (!probe.tried_sides.empty()) probe.tried_sides += ",";
        probe.tried_sides += side;
        FieldSceneRuntime runtime([](std::string_view) {});
        if (!runtime.bootstrap_for_scene_dump(startup_state, source_scene)) {
            continue;
        }
        map_coverage_set_mode(
            runtime, row.category, source_scene, row.gate);
        const auto attempt = map_coverage_drive_rect(
            runtime, startup_state, source_scene, rect, expected_event,
            600U, row.category, row.trigger, side, row.script);
        if (attempt.trigger_seen || attempt.observed_scene != source_scene) {
            probe.event_found = true;
            probe.approach_side = side;
            return probe;
        }
    }
    return probe;
}

template <typename StartupState>
inline MapCoverageApproachProbe map_coverage_probe_bexit_sides(
    const StartupState& startup_state, const MapCoverageRow& entry,
    const MapCoverageRow& exit_row, const std::uint16_t interior_scene,
    const MapCoverageRect& entry_rect, const MapCoverageRect& exit_rect) {
    MapCoverageApproachProbe probe;
    const auto entry_scene = map_coverage_scene(entry.scene);
    if (!entry_scene) {
        return probe;
    }
    const auto entry_sides = map_coverage_approach_sides(entry.trigger);
    const auto exit_sides = map_coverage_approach_sides(exit_row.trigger);
    for (const auto& exit_side : exit_sides) {
        if (!map_coverage_side_possible(exit_rect, exit_side)) {
            continue;
        }
        for (const auto& entry_side : entry_sides) {
            if (!map_coverage_side_possible(entry_rect, entry_side)) {
                continue;
            }
            if (!probe.tried_sides.empty()) probe.tried_sides += ",";
            probe.tried_sides += entry_side + ">" + exit_side;
            FieldSceneRuntime runtime([](std::string_view) {});
            if (!runtime.bootstrap_for_scene_dump(startup_state, *entry_scene)) {
                continue;
            }
            map_coverage_set_mode(runtime, entry.category, *entry_scene, entry.gate);
            const auto entered = map_coverage_drive_rect(
                runtime, startup_state, *entry_scene, entry_rect,
                map_coverage_event(entry.trigger), 600U, "door", entry.trigger,
                entry_side, entry.script);
            if (!entered.transition_fired ||
                runtime.field_scene_reference_for_dump() != interior_scene) {
                continue;
            }
            probe.entry_passed = true;
            if (probe.entry_side.empty()) probe.entry_side = entry_side;
            const auto exited = map_coverage_drive_rect(
                runtime, startup_state, interior_scene, exit_rect,
                map_coverage_event(exit_row.trigger), 600U, "bexit",
                exit_row.trigger, exit_side);
            if (exited.trigger_seen || exited.transition_fired) {
                probe.event_found = true;
                probe.entry_side = entry_side;
                probe.approach_side = exit_side;
                return probe;
            }
        }
    }
    return probe;
}

template <typename StartupState>
inline int run_map_coverage_bexit_timing_qa(
    const StartupState& startup_state, const std::filesystem::path& rows_path,
    const std::filesystem::path& results_path) {
    constexpr std::string_view kEntryRowId = "door-s00-r0d5796";
    constexpr std::string_view kExitRowId =
        "bexit-s0a-entry0d5796-r1d6178";
    constexpr std::uint32_t kRomBuildingFadePalettePasses = 17U;
    constexpr std::uint32_t kRomDoorInteractionDelayFrames =
        0x10U; // FFDF3C initialized at ROM 0x0BA8E.
    constexpr std::uint32_t kRomSceneSetupWaitFrames = 1U; // 0x0920
    constexpr std::uint32_t kRomFieldLoopWaitFrames = 1U; // 0x0952
    const auto rows = map_coverage_read_rows(rows_path);
    const auto find_row = [&rows](const std::string_view id)
        -> const MapCoverageRow* {
        const auto found = std::find_if(rows.begin(), rows.end(),
            [id](const MapCoverageRow& row) { return row.row_id == id; });
        return found == rows.end() ? nullptr : &*found;
    };
    const auto* entry = find_row(kEntryRowId);
    const auto* exit_row = find_row(kExitRowId);
    if (entry == nullptr || exit_row == nullptr) {
        std::cerr << "building-exit timing QA: paired baseline PASS rows are missing\n";
        return 3;
    }
    const auto outside_scene = map_coverage_scene(entry->scene);
    const auto interior_scene = map_coverage_scene(exit_row->scene);
    const auto outside_rect = map_coverage_rect(entry->trigger);
    const auto exit_rect = map_coverage_rect(exit_row->trigger);
    const auto exit_destination = map_coverage_scene(exit_row->dest_scene);
    if (!outside_scene || !interior_scene || !outside_rect || !exit_rect ||
        !exit_destination) {
        std::cerr << "building-exit timing QA: selected row fields are unresolved\n";
        return 3;
    }

    std::uint32_t exit_frame_clock = 0U;
    std::uint32_t exit_trigger_frame = 0U;
    std::uint32_t fade_out_started_frame = 0U;
    std::uint32_t first_new_scene_frame = 0U;
    std::uint32_t fade_in_started_frame = 0U;
    std::uint32_t transition_completed_frame = 0U;
    std::uint32_t fade_out_palette_steps = 0U;
    std::uint32_t fade_in_palette_steps = 0U;
    bool measuring_exit = false;
    bool entry_transition_started = false;
    bool entry_transition_completed = false;
    FieldSceneRuntime runtime([&](const std::string_view boundary) {
        if (!measuring_exit) {
            if (boundary == "field.building_transition.fade_out_started") {
                entry_transition_started = true;
            }
            if (boundary == "field.building_transition.completed") {
                entry_transition_completed = true;
            }
            return;
        }
        if (exit_trigger_frame == 0U &&
            boundary.rfind("field.exit.", 0U) == 0U) {
            exit_trigger_frame = exit_frame_clock;
        }
        if (boundary == "field.building_transition.palette_step") {
            if (fade_in_started_frame == 0U) {
                ++fade_out_palette_steps;
            } else {
                ++fade_in_palette_steps;
            }
        } else if (boundary ==
                   "field.building_transition.fade_out_started") {
            fade_out_started_frame = exit_frame_clock;
        } else if (boundary ==
                   "field.scene.transition.display_service_released") {
            first_new_scene_frame = exit_frame_clock;
        } else if (boundary ==
                   "field.building_transition.fade_in_started") {
            fade_in_started_frame = exit_frame_clock;
        } else if (boundary == "field.building_transition.completed") {
            transition_completed_frame = exit_frame_clock;
        }
    });
    if (!runtime.bootstrap_for_scene_dump(startup_state, *outside_scene)) {
        std::cerr << "building-exit timing QA: outside scene bootstrap failed: "
                  << runtime.failure() << '\n';
        return 3;
    }
    map_coverage_set_mode(runtime, entry->category, *outside_scene,
                          entry->gate);
    const auto entered = map_coverage_drive_rect(
        runtime, startup_state, *outside_scene, *outside_rect,
        map_coverage_event(entry->trigger), 600U, "door", entry->trigger,
        "south", entry->script);
    if (!entered.transition_fired ||
        runtime.field_scene_reference_for_dump() != *interior_scene) {
        std::cerr << "building-exit timing QA: paired entry did not reach scene "
                  << *interior_scene << '\n';
        return 3;
    }
    runtime.set_controller_port_byte(0U);
    for (std::uint32_t frame = 0U;
         entry_transition_started && frame < 100U &&
         !entry_transition_completed; ++frame) {
        runtime.advance_frame();
    }
    if (entry_transition_started && !entry_transition_completed) {
        std::cerr << "building-exit timing QA: paired entry fade-in did not "
                     "complete before the exit walk\n";
        return 3;
    }

    measuring_exit = true;
    const auto exited = map_coverage_drive_rect(
        runtime, startup_state, *interior_scene, *exit_rect,
        map_coverage_event(exit_row->trigger), 600U, "bexit",
        exit_row->trigger, "north", {}, &exit_frame_clock);
    if (exited.transition_pending_frame != 0U) {
        exit_trigger_frame = exited.transition_pending_frame;
    } else if (exit_trigger_frame == 0U) {
        exit_trigger_frame = exited.trigger_frame;
    }
    if (!exited.transition_fired ||
        runtime.field_scene_reference_for_dump() != *exit_destination ||
        exited.first_new_scene_frame == 0U) {
        std::cerr << "building-exit timing QA: ordinary exit did not reach scene "
                  << *exit_destination << '\n';
        return 3;
    }

    runtime.set_controller_port_byte(0U);
    while (transition_completed_frame == 0U && exit_frame_clock < 180U) {
        ++exit_frame_clock;
        runtime.advance_frame();
    }
    std::uint32_t control_return_frame = exited.first_new_scene_frame;
    if (transition_completed_frame != 0U) {
        ++exit_frame_clock;
        runtime.advance_frame();
        control_return_frame = exit_frame_clock;
    }

    const auto measured_new_scene_frame = first_new_scene_frame != 0U
        ? first_new_scene_frame : exited.first_new_scene_frame;
    const auto measured_fade_out_start_frame = fade_out_started_frame != 0U
        ? fade_out_started_frame : exit_trigger_frame;
    const auto new_scene_frames =
        measured_new_scene_frame - measured_fade_out_start_frame;
    const auto control_return_frames =
        control_return_frame - measured_fade_out_start_frame;
    const auto request_to_control_return_frames =
        control_return_frame - exit_trigger_frame;
    const auto request_to_fade_out_start_frames =
        fade_out_started_frame == 0U
            ? 0U : fade_out_started_frame - exit_trigger_frame;
    const auto request_to_new_scene_frames =
        measured_new_scene_frame - exit_trigger_frame;
    const auto contact_to_new_scene_frames =
        measured_new_scene_frame - exited.trigger_frame;
    const auto expected_control_return_frames =
        2U * static_cast<std::uint32_t>(kRomBuildingTransitionVBlanks) +
        kRomSceneSetupWaitFrames + kRomFieldLoopWaitFrames;
    const auto contact_to_request_frames =
        exit_trigger_frame - exited.trigger_frame;
    const auto contact_to_control_return_frames =
        control_return_frame - exited.trigger_frame;
    const auto new_scene_to_fade_in_frames =
        fade_in_started_frame == 0U
            ? 0U : fade_in_started_frame - measured_new_scene_frame;
    const bool passes =
        fade_out_started_frame != 0U && first_new_scene_frame != 0U &&
        new_scene_frames == kRomBuildingTransitionVBlanks &&
        control_return_frames == expected_control_return_frames &&
        contact_to_request_frames == kRomDoorInteractionDelayFrames &&
        request_to_fade_out_start_frames == 1U &&
        request_to_new_scene_frames ==
            1U + kRomBuildingTransitionVBlanks &&
        contact_to_new_scene_frames ==
            kRomDoorInteractionDelayFrames + 1U +
                kRomBuildingTransitionVBlanks &&
        contact_to_control_return_frames ==
            kRomDoorInteractionDelayFrames + 1U +
                expected_control_return_frames &&
        new_scene_to_fade_in_frames == kRomSceneSetupWaitFrames &&
        fade_out_palette_steps == kRomBuildingFadePalettePasses &&
        fade_in_palette_steps == kRomBuildingFadePalettePasses;

    std::ofstream results(results_path, std::ios::trunc);
    if (!results) {
        std::cerr << "building-exit timing QA: could not open results file\n";
        return 3;
    }
    results << "row\tcontact_to_request_vblanks\t"
               "request_to_fade_out_start_vblanks\t"
               "fade_out_start_to_new_scene_vblanks\t"
               "request_to_new_scene_vblanks\tcontact_to_new_scene_vblanks\t"
               "fade_out_start_to_control_return_vblanks\t"
               "request_to_control_return_vblanks\t"
               "contact_to_control_return_vblanks\t"
               "new_scene_to_fade_in_start_vblanks\tfade_out_palette_steps\t"
               "fade_in_palette_steps\tstatus\n"
            << exit_row->row_id << '\t' << contact_to_request_frames << '\t'
            << request_to_fade_out_start_frames << '\t' << new_scene_frames
            << '\t' << request_to_new_scene_frames << '\t'
            << contact_to_new_scene_frames << '\t'
            << control_return_frames << '\t'
            << request_to_control_return_frames << '\t'
            << contact_to_control_return_frames << '\t'
            << new_scene_to_fade_in_frames << '\t'
            << fade_out_palette_steps << '\t'
            << fade_in_palette_steps << '\t' << (passes ? "PASS" : "FAIL")
            << '\n';
    std::cout << "building_exit_timing row=" << exit_row->row_id
              << " rect_contact_frame=" << exited.trigger_frame
              << " transition_pending_frame="
              << exited.transition_pending_frame
              << " contact_to_request_vblanks="
              << contact_to_request_frames
              << " request_to_fade_out_start_vblanks="
              << request_to_fade_out_start_frames
              << " fade_out_start_to_new_scene_vblanks=" << new_scene_frames
              << " request_to_new_scene_vblanks="
              << request_to_new_scene_frames
              << " contact_to_new_scene_vblanks="
              << contact_to_new_scene_frames
              << " fade_out_start_to_control_return_vblanks="
              << control_return_frames
              << " request_to_control_return_vblanks="
              << request_to_control_return_frames
              << " contact_to_control_return_vblanks="
              << contact_to_control_return_frames
              << " new_scene_to_fade_in_start_vblanks="
              << new_scene_to_fade_in_frames
              << " fade_out_palette_steps=" << fade_out_palette_steps
              << " fade_in_palette_steps=" << fade_in_palette_steps
              << " expected_vblanks="
              << static_cast<unsigned>(kRomBuildingTransitionVBlanks)
              << "/"
              << expected_control_return_frames
              << " contact_expected=16/51/87 hold=1"
              << " artifacts=" << results_path.string() << '\n';
    if (!passes) {
        std::cerr << "FAIL: ordinary building exit timing must match the ROM's "
                     "16-frame door gate, 34-VBlank fade-out, 1-VBlank "
                     "scene wait, 34-VBlank fade-in, and field-loop wait\n";
        return 1;
    }
    return 0;
}

inline std::string map_coverage_sanitize(std::string value) {
    std::replace(value.begin(), value.end(), '\t', ' ');
    std::replace(value.begin(), value.end(), '\r', ' ');
    std::replace(value.begin(), value.end(), '\n', ' ');
    return value;
}

template <typename StartupState>
inline bool map_coverage_bootstrap_scene(
    const StartupState& startup_state, const std::uint16_t scene,
    const std::string_view category, FieldSceneRuntime& runtime) {
    if (category != "elevator") {
        return runtime.bootstrap_for_scene_dump(startup_state, scene);
    }
    if (!startup_state.field_session_source_assets.has_value() ||
        !startup_state.joshua_selection.has_value()) {
        return false;
    }
    const auto scene_input = integration::make_field_scene_input(
        *startup_state.field_session_source_assets, scene);
    if (!scene_input.has_value()) {
        return false;
    }

    // The elevator route QAs bootstrap from the scene's restored descriptor
    // position and camera. Reuse that typed setup so the physical event is
    // reached through ordinary movement rather than by placing the runner at
    // the collision rectangle.
    auto scene_startup_state = startup_state;
    shadowrun::ghidra_conversion::SaveFieldRestoreState descriptor_spawn{};
    descriptor_spawn.world_mode = scene;
    descriptor_spawn.camera_x = static_cast<std::uint16_t>(
        scene_input->source_metadata.initial_camera_x_fixed);
    descriptor_spawn.camera_y = static_cast<std::uint16_t>(
        scene_input->source_metadata.initial_camera_y_fixed);
    for (std::size_t slot = 0U;
         slot < descriptor_spawn.runner_positions.size(); ++slot) {
        const auto& spawn = scene_input->source_metadata.initial_field_records[slot];
        descriptor_spawn.runner_positions[slot] =
            shadowrun::ghidra_conversion::RestoredRunnerPosition{
                spawn.x, spawn.y};
    }
    descriptor_spawn.roster.default_record.bytes =
        startup_state.joshua_selection->live_joshua_record;
    descriptor_spawn.active_runner_slot = 0U;
    scene_startup_state.restored_field_state = std::move(descriptor_spawn);
    return runtime.bootstrap_for_scene_dump(scene_startup_state, scene);
}

template <typename StartupState, typename BootstrapRow>
inline int run_map_coverage_qa(
    const StartupState& startup_state, const std::filesystem::path& rows_path,
    const std::filesystem::path& results_path,
    const std::filesystem::path& route_rows_path,
    BootstrapRow&& bootstrap_row) {
    const auto rows = map_coverage_read_rows(rows_path);
    const auto route_rows = map_coverage_read_rows(route_rows_path);
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
        std::optional<std::uint32_t> cave_source_mode;
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
            MapCoverageApproachProbe approach_probe;
            std::string selected_approach_side;
            std::string selected_entry_side;
            bool bexit_entrance_passed = row.category != "bexit";
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
                const auto cave_target_room =
                    map_coverage_cave_target_room(row);
                auto bootstrap_scene = cave_target_room.has_value()
                    ? std::uint16_t{9U} : *source_scene;
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
                const bool side_sensitive_row = row.category == "edge" ||
                    row.category == "door" || row.category == "bexit" ||
                    (row.category == "travel" && expected_event &&
                     *expected_event == 0x1FU);
                if (side_sensitive_row && rect) {
                    const auto baseline_approach_side = map_coverage_side_hint(
                        row.trigger, "baseline_approach_side");
                    const auto baseline_entry_side = map_coverage_side_hint(
                        row.trigger, "baseline_entry_side");
                    if (row.category == "bexit" && paired_entry) {
                        const auto entry_rect = map_coverage_rect(paired_entry->trigger);
                        if (entry_rect) {
                            if (baseline_entry_side && baseline_approach_side &&
                                map_coverage_side_possible(
                                    *entry_rect, *baseline_entry_side) &&
                                map_coverage_side_possible(
                                    *rect, *baseline_approach_side)) {
                                // A recorded passing approach already fixed
                                // both sides; the actual row run below still
                                // repeats the entrance and exit checks.
                                approach_probe.entry_passed = true;
                                approach_probe.entry_side =
                                    *baseline_entry_side;
                                approach_probe.approach_side =
                                    *baseline_approach_side;
                            } else {
                                approach_probe = map_coverage_probe_bexit_sides(
                                    startup_state, *paired_entry, row,
                                    *source_scene, *entry_rect, *rect);
                            }
                            bexit_entrance_passed = approach_probe.entry_passed;
                            selected_entry_side = approach_probe.entry_side;
                        }
                    } else if (row.category != "bexit") {
                        if (baseline_approach_side &&
                            map_coverage_side_possible(
                                *rect, *baseline_approach_side)) {
                            approach_probe.approach_side =
                                *baseline_approach_side;
                        } else {
                            auto approach_row = row;
                            approach_row.trigger = physical_trigger;
                            approach_probe = map_coverage_probe_approach_sides(
                                startup_state, approach_row, *source_scene,
                                *rect, expected_event);
                        }
                    }
                    selected_approach_side = approach_probe.approach_side;
                    if (selected_approach_side.empty()) {
                        for (const auto& side : map_coverage_approach_sides(
                                 row.category == "bexit" ? row.trigger : physical_trigger)) {
                            if (map_coverage_side_possible(*rect, side)) {
                                selected_approach_side = side;
                            }
                        }
                    }
                    if (selected_entry_side.empty() && paired_entry) {
                        const auto entry_rect = map_coverage_rect(paired_entry->trigger);
                        if (entry_rect) {
                            for (const auto& side : map_coverage_approach_sides(
                                     paired_entry->trigger)) {
                                if (map_coverage_side_possible(*entry_rect, side)) {
                                    selected_entry_side = side;
                                    break;
                                }
                            }
                        }
                    }
                }
                std::vector<std::uint16_t> field_audio_events;
                std::vector<std::string> source_boundaries;
                FieldSceneRuntime runtime(
                    [&source_boundaries](const std::string_view boundary) {
                        source_boundaries.emplace_back(boundary);
                    },
                    [&field_audio_events](const std::uint16_t event) {
                        field_audio_events.push_back(event);
                    });
                if (bootstrap_row(
                        startup_state, bootstrap_scene, row.category, runtime,
                        results_path.parent_path(), field_audio_events,
                        source_boundaries)) {
                    attempt.setup = true;
                    bool source_ready = true;
                    std::string cave_route_evidence;
                    if (bexit_entry != std::string::npos) {
                        if (!bexit_entrance_passed) {
                            source_ready = false;
                            attempt.note = "paired entrance did not PASS; exit left untested";
                        } else if (!paired_entry) {
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
                                map_coverage_set_mode(
                                    runtime, paired_entry->category, *outside_scene,
                                    paired_entry->gate);
                                const auto entered = map_coverage_drive_rect(
                                    runtime, startup_state, *outside_scene,
                                    *outside_rect, map_coverage_event(paired_entry->trigger),
                                    600U, "door", paired_entry->trigger,
                                    selected_entry_side, paired_entry->script);
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
                    } else if (cave_target_room.has_value()) {
                        source_ready = map_coverage_prepare_cave_target_room(
                            runtime, startup_state,
                            route_rows.empty() ? rows : route_rows,
                            *cave_target_room,
                            source_boundaries, cave_route_evidence);
                        if (!source_ready) {
                            attempt.note = cave_route_evidence;
                        }
                    } else {
                        map_coverage_set_mode(
                            runtime, row.category, *source_scene, row.gate);
                    }
                    if (!source_ready) {
                        status = row.category == "bexit" &&
                                 !bexit_entrance_passed
                            ? "PRESENT-UNTESTED" : "MISSING";
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
                        std::uint32_t frame_budget = 600U;
                        if (row.category == "elevator") {
                            const auto current_floor =
                                map_coverage_trigger_number(
                                    physical_trigger, "current_floor");
                            const auto target_floor =
                                map_coverage_trigger_number(
                                    physical_trigger, "floor");
                            if (current_floor && target_floor) {
                                const auto distance = *current_floor > *target_floor
                                    ? *current_floor - *target_floor
                                    : *target_floor - *current_floor;
                                frame_budget += distance * 17U * 8U;
                            }
                        }
                        const auto remaining_frames = row.category == "bexit"
                            ? (attempt.frames_used < frame_budget
                                  ? frame_budget - attempt.frames_used : 0U)
                            : frame_budget;
                        if (row.category == "cave") {
                            // ROM 0x2CE0 publishes the destination and spawn
                            // pair but leaves FFDF40 untouched. The cave row's
                            // arrival_mode field is not a transition result.
                            cave_source_mode =
                                runtime.field_interaction_mode_for_dump();
                        }
                        const auto driven = row.category == "cave"
                            ? map_coverage_drive_cave_exit(
                                  runtime, startup_state, *source_scene, row,
                                  *rect, expected_event)
                            : map_coverage_drive_rect(
                                  runtime, startup_state, *source_scene, *rect,
                                  expected_event, remaining_frames, row.category,
                                  physical_trigger, selected_approach_side,
                                  row.script);
                        auto combined = driven;
                        combined.setup = true;
                        combined.frames_used += attempt.frames_used;
                        if (row.category == "bexit") {
                            combined.backup_x = attempt.backup_x;
                            combined.backup_y = attempt.backup_y;
                            combined.backup_captured = attempt.backup_captured;
                            combined.lead_x = attempt.lead_x;
                            combined.lead_y = attempt.lead_y;
                            combined.door_facing = attempt.door_facing;
                            combined.door_bank = attempt.door_bank;
                            combined.lead_captured = attempt.lead_captured;
                            combined.prompt_inputs += attempt.prompt_inputs;
                        }
                        attempt = std::move(combined);
                        if (row.category == "cave" &&
                            row.gate.find("FFFC10=") != std::string::npos &&
                            !cave_target_room.has_value()) {
                            attempt.note = "cave room gate not established by a natural incoming route";
                            attempt.trigger_seen = false;
                        }
                        if (row.category == "cave" &&
                            !attempt.approach_side.empty()) {
                            selected_approach_side = attempt.approach_side;
                        }
                    }
                    const bool death_wake_row = row.category == "travel" &&
                        row.trigger.find("party-down") != std::string::npos;
                    const bool scripted_no_door = row.script == "0x053F98" ||
                        row.script == "0x020484" || row.script == "0x05448A" ||
                        row.script == "0x05432E";
                    if (status.empty() && death_wake_row) {
                        status = "MISSING";
                        diff = attempt.note.empty()
                            ? "party-down prerequisite was not reached"
                            : attempt.note;
                    } else if (status.empty() && scripted_no_door &&
                               attempt.trigger_seen &&
                               attempt.prompt_inputs != 0U &&
                               attempt.observed_scene == *source_scene) {
                        // The ROM script always shows its text (0x1F246)
                        // before deciding d6, so a PASS needs the port's
                        // dialogue to have opened, not just no transition.
                        status = "PASS";
                        diff = "ROM script " + row.script +
                            " handled the event without generic door transition";
                    } else if (status.empty() && scripted_no_door &&
                               attempt.observed_scene != *source_scene) {
                        status = "WRONG";
                        diff = "ROM script " + row.script +
                            " expected no generic door transition; observed scene=" +
                            std::to_string(attempt.observed_scene);
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
                        const auto expected_mode = row.category == "cave"
                            ? cave_source_mode
                            : map_coverage_number(row.arrival_mode);
                        if (expected_mode && attempt.observed_mode != *expected_mode) {
                            matches = false;
                            diff += "mode expected=" + std::to_string(*expected_mode) +
                                    " observed=" + std::to_string(attempt.observed_mode) + ";";
                        }
                        if (row.category == "edge" &&
                            attempt.observed_fade_steps != 0U) {
                            matches = false;
                            diff += "fade overlay remained at " +
                                    std::to_string(attempt.observed_fade_steps) +
                                    " steps;";
                        }
                        if (row.category == "edge" &&
                            attempt.playfield_lit_pixels < 100U) {
                            matches = false;
                            diff += "playfield remained black;lit_pixels=" +
                                    std::to_string(attempt.playfield_lit_pixels) + ";";
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
                            diff = cave_route_evidence.empty()
                                ? "observed route matches all resolved ROM outputs"
                                : cave_route_evidence +
                                    ";observed route matches all resolved ROM outputs";
                            if (row.category == "edge") {
                                diff += ";fade_steps=" + std::to_string(
                                    attempt.observed_fade_steps) +
                                    ";lit_pixels=" + std::to_string(
                                        attempt.playfield_lit_pixels);
                            }
                        } else {
                            status = "WRONG";
                            if (!cave_route_evidence.empty()) {
                                diff = cave_route_evidence + ";" + diff;
                            }
                        }
                    }
                } else {
                    status = row.category == "bexit" &&
                             !bexit_entrance_passed
                        ? "PRESENT-UNTESTED" : "MISSING";
                    diff = "fresh scene bootstrap failed";
                }
                if (row.category == "cave" && status == "PASS") {
                    runtime.set_controller_port_byte(0U);
                    const auto& capture_camera =
                        runtime.field_camera_follow_for_dump();
                    const auto capture_position =
                        runtime.field_entity_world_position();
                    diff += ";capture_position=" +
                        map_coverage_hex(capture_position.x) + "," +
                        map_coverage_hex(capture_position.y) +
                        ";capture_camera=" +
                        map_coverage_hex(capture_camera.camera_x) + "," +
                        map_coverage_hex(capture_camera.camera_y) +
                        ";capture_camera_max=" +
                        map_coverage_hex(capture_camera.maximum_x) + "," +
                        map_coverage_hex(capture_camera.maximum_y);
                    QImage image(320, 224, QImage::Format_ARGB32_Premultiplied);
                    image.fill(Qt::black);
                    QPainter painter(&image);
                    runtime.paint(painter);
                    painter.end();
                    const auto lit_pixels =
                        map_coverage_playfield_lit_pixels(image);
                    diff += ";capture_playfield_lit_pixels=" +
                        std::to_string(lit_pixels);
                    if (lit_pixels < 1000U) {
                        status = "MISSING";
                        diff += ";destination map dump is black";
                    }
                    const auto dump_path =
                        results_path.parent_path() / "actual.bmp";
                    if (!image.save(
                            QString::fromStdString(dump_path.string()))) {
                        status = "MISSING";
                        diff += ";failed to write cave scene actual.bmp";
                    }
                }
            }
            if (!approach_probe.tried_sides.empty()) {
                diff += ";approach_sides_tried=" + approach_probe.tried_sides;
            }
            if (!selected_approach_side.empty()) {
                diff += ";approach_side=" + selected_approach_side;
            }
            if (row.category == "bexit" && !selected_entry_side.empty()) {
                diff += ";entry_side=" + selected_entry_side;
            }
            if (row.script != "-") {
                diff += ";script=" + row.script;
                if (attempt.prompt_inputs != 0U) {
                    const auto button = map_coverage_script_prompt_button(row.script);
                    diff += ";prompt=" + std::string(
                        button == 0x10U ? "B" : "A") +
                        ";prompt_inputs=" + std::to_string(attempt.prompt_inputs);
                }
            }
            if (attempt.lead_captured) {
                const auto facing = attempt.door_facing == 0U ? "N" :
                    attempt.door_facing == 2U ? "E" :
                    attempt.door_facing == 4U ? "S" :
                    attempt.door_facing == 6U ? "W" : "?";
                diff += ";door_lead=" + map_coverage_hex(attempt.lead_x) +
                    "," + map_coverage_hex(attempt.lead_y) +
                    ";facing=" + facing;
                if (attempt.door_bank <= 3U) {
                    diff += ";rom_bank=" + std::to_string(attempt.door_bank);
                }
                if (attempt.backup_captured) {
                    diff += ";FFDE26=" + map_coverage_hex(attempt.backup_x) +
                        "," + map_coverage_hex(attempt.backup_y);
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
