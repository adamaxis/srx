#pragma once

#include "shadowrun_ghidra/scripting/dialogue_outcome.hpp"

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace shadowrun::ghidra_conversion::scripting {

// Hosts register the transitions they actually implement. A handler receives
// only the payload type associated with its registration, while named targets
// resolve without route-specific cases in the presenter.
class DialogueOutcomeDispatcher final {
public:
    enum class Status {
        handled,
        target_not_registered,
        handler_rejected,
    };

    struct Result {
        Status status = Status::target_not_registered;

        bool handled() const noexcept { return status == Status::handled; }
    };

    template <typename Outcome, typename Callable>
    bool register_type_handler(Callable&& callable) {
        static_assert(is_terminal_outcome<Outcome>() &&
                          !DialogueOutcomeTraits<Outcome>::requires_target,
                      "targeted outcomes need a registered target handler");
        TypedHandler<Outcome> typed_handler(std::forward<Callable>(callable));
        if (!typed_handler) {
            return false;
        }
        return type_handlers_.emplace(
            DialogueOutcomeTraits<Outcome>::type,
            [handler = std::move(typed_handler)](
                const DialogueOutcome& outcome) {
                const auto* typed = std::get_if<Outcome>(&outcome);
                return typed != nullptr && handler(*typed);
            }).second;
    }

    template <typename Outcome, typename Callable>
    bool register_target_handler(std::string_view target_id,
                                 Callable&& callable) {
        static_assert(is_terminal_outcome<Outcome>() &&
                          DialogueOutcomeTraits<Outcome>::requires_target,
                      "only named-target outcomes use target handlers");
        if (target_id.empty()) {
            return false;
        }
        TypedHandler<Outcome> typed_handler(std::forward<Callable>(callable));
        if (!typed_handler) {
            return false;
        }
        return target_handlers_.emplace(
            TargetKey{DialogueOutcomeTraits<Outcome>::type,
                      std::string(target_id)},
            [handler = std::move(typed_handler)](
                const DialogueOutcome& outcome) {
                const auto* typed = std::get_if<Outcome>(&outcome);
                return typed != nullptr && handler(*typed);
            }).second;
    }

    Result dispatch(const DialogueOutcome& outcome) const {
        if (std::holds_alternative<SourceBoundaryOutcome>(outcome)) {
            return {Status::handler_rejected};
        }

        const auto type = dialogue_outcome_type(outcome);
        const auto target = dialogue_outcome_target_id(outcome);
        if (dialogue_outcome_requires_target(outcome)) {
            if (target.empty()) {
                return {Status::target_not_registered};
            }
            const auto found =
                target_handlers_.find(
                    TargetKey{type, std::string(target)});
            if (found != target_handlers_.end()) {
                return invoke(found->second, outcome);
            }
            return {Status::target_not_registered};
        }

        const auto found = type_handlers_.find(type);
        if (found == type_handlers_.end()) {
            return {Status::target_not_registered};
        }
        return invoke(found->second, outcome);
    }

private:
    using Handler = std::function<bool(const DialogueOutcome&)>;

    template <typename Outcome>
    using TypedHandler = std::function<bool(const Outcome&)>;

    template <typename Outcome>
    static constexpr bool is_terminal_outcome() noexcept {
        return !std::is_same_v<Outcome, SourceBoundaryOutcome>;
    }

    struct TargetKey {
        DialogueOutcomeType type;
        std::string target_id;

        friend bool operator<(const TargetKey& left,
                              const TargetKey& right) noexcept {
            if (left.type != right.type) {
                return left.type < right.type;
            }
            return left.target_id < right.target_id;
        }
    };

    static Result invoke(const Handler& handler,
                         const DialogueOutcome& outcome) {
        return {handler(outcome) ? Status::handled
                                 : Status::handler_rejected};
    }

    std::map<DialogueOutcomeType, Handler> type_handlers_;
    std::map<TargetKey, Handler> target_handlers_;
};

} // namespace shadowrun::ghidra_conversion::scripting
