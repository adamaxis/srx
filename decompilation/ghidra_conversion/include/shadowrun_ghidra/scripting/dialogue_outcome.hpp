#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace shadowrun::ghidra_conversion::scripting {

// Completed dialogue graphs return one source-level transition request.
// Qt, another host, or a test dispatcher can apply the same portable value.
enum class DialogueOutcomeType : std::uint8_t {
    continue_in_place,
    resume_source_route,
    return_to_origin,
    enter_field_scene,
    enter_activity,
    open_service,
    play_cinematic,
    source_boundary,
};

constexpr std::optional<DialogueOutcomeType> dialogue_outcome_type_from_name(
    const std::string_view name) noexcept {
    if (name == "continue_in_place") {
        return DialogueOutcomeType::continue_in_place;
    }
    if (name == "resume_source_route") {
        return DialogueOutcomeType::resume_source_route;
    }
    if (name == "return_to_origin") {
        return DialogueOutcomeType::return_to_origin;
    }
    if (name == "enter_field_scene") {
        return DialogueOutcomeType::enter_field_scene;
    }
    if (name == "enter_activity") {
        return DialogueOutcomeType::enter_activity;
    }
    if (name == "open_service") {
        return DialogueOutcomeType::open_service;
    }
    if (name == "play_cinematic") {
        return DialogueOutcomeType::play_cinematic;
    }
    if (name == "source_boundary") {
        return DialogueOutcomeType::source_boundary;
    }
    return std::nullopt;
}

constexpr std::string_view dialogue_outcome_type_name(
    const DialogueOutcomeType type) noexcept {
    switch (type) {
    case DialogueOutcomeType::continue_in_place:
        return "continue_in_place";
    case DialogueOutcomeType::resume_source_route:
        return "resume_source_route";
    case DialogueOutcomeType::return_to_origin:
        return "return_to_origin";
    case DialogueOutcomeType::enter_field_scene:
        return "enter_field_scene";
    case DialogueOutcomeType::enter_activity:
        return "enter_activity";
    case DialogueOutcomeType::open_service:
        return "open_service";
    case DialogueOutcomeType::play_cinematic:
        return "play_cinematic";
    case DialogueOutcomeType::source_boundary:
        return "source_boundary";
    }
    return "source_boundary";
}

struct DialogueOutcome {
    DialogueOutcomeType type = DialogueOutcomeType::source_boundary;
    std::optional<std::uint8_t> scene_id;
    // returnPoint, activityId, serviceId, or cinematicId, by outcome type.
    std::string destination_id;
    std::string arrival_id;
    std::string return_policy;
    std::string reason;
};

} // namespace shadowrun::ghidra_conversion::scripting
