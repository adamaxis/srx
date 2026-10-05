#pragma once

#include "shadowrun_ghidra/menus/attributes_skills_menu_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class AttributesSkillsRuntimeEvent {
    no_change,
    selection_moved,
    upgrade_prompt_opened,
    upgrade_prompt_moved,
    upgrade_applied,
    upgrade_declined,
    upgrade_blocked,
    insufficient_karma,
    active_runner_selection_requested,
    returned_to_parent,
};

enum class AttributesSkillsMenuMessage {
    none,
    maximum_level,
    mage_only_skill,
    insufficient_karma,
};

enum class AttributesSkillsMenuMode {
    command_view_only,
    hotel_karma_upgrade,
};

// Non-blocking Qt frame-loop adapter for two distinct screens that share the
// ROM's Attributes/Skills grid body. ROM 0xD01C sets FFF116=-1; that command
// screen displays current Karma only. Hotel dispatch enters at 0xD024 and
// displays upgrade costs and enables Karma spending.
class AttributesSkillsMenuRuntime final {
public:
    void open(const RunnerRecordFields& active_record,
              AttributesSkillsMenuMode mode);
    void close_to_parent() noexcept;

    bool is_open() const noexcept { return state_.open; }
    bool allows_karma_upgrades() const noexcept {
        return mode_ == AttributesSkillsMenuMode::hotel_karma_upgrade;
    }
    bool upgrade_prompt_open() const noexcept { return upgrade_prompt_open_; }
    std::uint8_t upgrade_prompt_selection() const noexcept {
        return upgrade_prompt_selection_;
    }
    const AttributesSkillsMenuState& state() const noexcept { return state_; }
    AttributesSkillsMenuMessage message() const noexcept { return message_; }
    AttributeSkillGrid grid(const RunnerRecordFields& record) const;
    AttributeSkillUpgradePreview preview(
        const RunnerRecordFields& record) const;
    void refresh(const RunnerRecordFields& active_record);

    AttributesSkillsRuntimeEvent advance(
        std::uint16_t controller_events, RunnerRecordFields& active_record,
        const std::function<void()>& advance_active_runner_selection = {});

private:
    void refresh_selection_state(const RunnerRecordFields& active_record);

    AttributesSkillsMenuState state_{};
    AttributesSkillsMenuMode mode_ =
        AttributesSkillsMenuMode::command_view_only;
    bool upgrade_prompt_open_ = false;
    std::uint8_t upgrade_prompt_selection_ = 0U;
    AttributesSkillsMenuMessage message_ = AttributesSkillsMenuMessage::none;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
