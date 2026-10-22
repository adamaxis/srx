#pragma once

#include "shadowrun_ghidra/scripting/dialogue_outcome.hpp"

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::scripting {

// Hosts register the transitions they actually implement. Named targets such
// as service, activity, and cinematic IDs resolve without growing a switch in
// the dialogue presenter. A missing target stays an explicit source boundary.
class DialogueOutcomeDispatcher final {
public:
    using Handler = std::function<bool(const DialogueOutcome&)>;

    enum class Status {
        handled,
        target_not_registered,
        handler_rejected,
    };

    struct Result {
        Status status = Status::target_not_registered;

        bool handled() const noexcept { return status == Status::handled; }
    };

    bool register_type_handler(DialogueOutcomeType type, Handler handler) {
        if (!handler) {
            return false;
        }
        return type_handlers_.emplace(type, std::move(handler)).second;
    }

    bool register_target_handler(DialogueOutcomeType type,
                                 std::string_view target_id,
                                 Handler handler) {
        if (target_id.empty() || !handler) {
            return false;
        }
        return target_handlers_
            .emplace(TargetKey{type, std::string(target_id)},
                     std::move(handler))
            .second;
    }

    Result dispatch(const DialogueOutcome& outcome) const {
        const auto target = outcome_target_id(outcome);
        if (!target.empty()) {
            const auto found =
                target_handlers_.find(
                    TargetKey{outcome.type, std::string(target)});
            if (found != target_handlers_.end()) {
                return invoke(found->second, outcome);
            }
        }

        const auto found = type_handlers_.find(outcome.type);
        if (found == type_handlers_.end()) {
            return {Status::target_not_registered};
        }
        return invoke(found->second, outcome);
    }

private:
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

    static std::string_view outcome_target_id(
        const DialogueOutcome& outcome) noexcept {
        switch (outcome.type) {
        case DialogueOutcomeType::return_to_origin:
        case DialogueOutcomeType::enter_activity:
        case DialogueOutcomeType::open_service:
        case DialogueOutcomeType::play_cinematic:
            return outcome.destination_id;
        case DialogueOutcomeType::continue_in_place:
        case DialogueOutcomeType::resume_source_route:
        case DialogueOutcomeType::enter_field_scene:
        case DialogueOutcomeType::source_boundary:
            return {};
        }
        return {};
    }

    static Result invoke(const Handler& handler,
                         const DialogueOutcome& outcome) {
        return {handler(outcome) ? Status::handled
                                 : Status::handler_rejected};
    }

    std::map<DialogueOutcomeType, Handler> type_handlers_;
    std::map<TargetKey, Handler> target_handlers_;
};

} // namespace shadowrun::ghidra_conversion::scripting
