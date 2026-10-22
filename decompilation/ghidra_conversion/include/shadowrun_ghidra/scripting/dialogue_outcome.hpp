#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace shadowrun::ghidra_conversion::scripting {

// JSON keeps the portable, human-readable tagged representation. The host
// bridge converts it to this closed set so a service target cannot accidentally
// carry a scene ID or a cinematic policy.
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

struct ContinueInPlaceOutcome final {};
struct ResumeSourceRouteOutcome final {};

struct ReturnToOriginOutcome final {
    std::string return_point_id;
    std::optional<std::string> return_policy;
};

struct EnterFieldSceneOutcome final {
    std::uint8_t scene_id = 0U;
    std::string arrival_id;
};

struct EnterActivityOutcome final {
    std::string activity_id;
};

struct OpenServiceOutcome final {
    std::string service_id;
    std::string return_policy;
};

struct PlayCinematicOutcome final {
    std::string cinematic_id;
    std::string return_policy;
};

// A source boundary is a diagnostic result, not a successful graph terminal.
struct SourceBoundaryOutcome final {
    std::string reason;
};

template <typename Outcome>
struct DialogueOutcomeTraits;

template <>
struct DialogueOutcomeTraits<ContinueInPlaceOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::continue_in_place;
    static constexpr bool requires_target = false;
};

template <>
struct DialogueOutcomeTraits<ResumeSourceRouteOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::resume_source_route;
    static constexpr bool requires_target = false;
};

template <>
struct DialogueOutcomeTraits<ReturnToOriginOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::return_to_origin;
    static constexpr bool requires_target = true;
};

template <>
struct DialogueOutcomeTraits<EnterFieldSceneOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::enter_field_scene;
    static constexpr bool requires_target = false;
};

template <>
struct DialogueOutcomeTraits<EnterActivityOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::enter_activity;
    static constexpr bool requires_target = true;
};

template <>
struct DialogueOutcomeTraits<OpenServiceOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::open_service;
    static constexpr bool requires_target = true;
};

template <>
struct DialogueOutcomeTraits<PlayCinematicOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::play_cinematic;
    static constexpr bool requires_target = true;
};

template <>
struct DialogueOutcomeTraits<SourceBoundaryOutcome> final {
    static constexpr DialogueOutcomeType type =
        DialogueOutcomeType::source_boundary;
    static constexpr bool requires_target = false;
};

using DialogueOutcome = std::variant<
    ContinueInPlaceOutcome,
    ResumeSourceRouteOutcome,
    ReturnToOriginOutcome,
    EnterFieldSceneOutcome,
    EnterActivityOutcome,
    OpenServiceOutcome,
    PlayCinematicOutcome,
    SourceBoundaryOutcome>;

inline DialogueOutcomeType dialogue_outcome_type(
    const DialogueOutcome& outcome) noexcept {
    return std::visit(
        [](const auto& typed_outcome) noexcept {
            using Outcome = std::decay_t<decltype(typed_outcome)>;
            return DialogueOutcomeTraits<Outcome>::type;
        },
        outcome);
}

inline bool dialogue_outcome_requires_target(
    const DialogueOutcome& outcome) noexcept {
    return std::visit(
        [](const auto& typed_outcome) noexcept {
            using Outcome = std::decay_t<decltype(typed_outcome)>;
            return DialogueOutcomeTraits<Outcome>::requires_target;
        },
        outcome);
}

inline std::string_view dialogue_outcome_target_id(
    const DialogueOutcome& outcome) noexcept {
    return std::visit(
        [](const auto& typed_outcome) noexcept -> std::string_view {
            using Outcome = std::decay_t<decltype(typed_outcome)>;
            if constexpr (std::is_same_v<Outcome,
                                         ReturnToOriginOutcome>) {
                return typed_outcome.return_point_id;
            } else if constexpr (std::is_same_v<Outcome,
                                                EnterActivityOutcome>) {
                return typed_outcome.activity_id;
            } else if constexpr (std::is_same_v<Outcome,
                                                OpenServiceOutcome>) {
                return typed_outcome.service_id;
            } else if constexpr (std::is_same_v<Outcome,
                                                PlayCinematicOutcome>) {
                return typed_outcome.cinematic_id;
            } else {
                return {};
            }
        },
        outcome);
}

} // namespace shadowrun::ghidra_conversion::scripting
