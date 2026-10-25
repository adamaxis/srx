#pragma once

#include "shadowrun_ghidra/menus/attributes_skills_menu_conversion.hpp"

#include <cstdint>
#include <functional>
#include <utility>

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

// Shared state and input controller for two distinct screens that reuse the
// ROM's Attributes/Skills grid body. Route-facing Qt code should use one of
// the fixed-mode screen types below so the command view cannot enter the
// hotel's Karma-spending flow.
class AttributesSkillsMenuController final {
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
    void synchronize_cursor(int column, int row,
                            const RunnerRecordFields& active_record);
    // Upgrade rule source (0xD82A). Unset keeps the converted C++ rule.
    using UpgradeRule = std::function<AttributeSkillUpgradePreview(
        const RunnerRecordFields&, AttributeSkillCursor)>;
    void set_upgrade_rule(UpgradeRule rule) { upgrade_rule_ = std::move(rule); }

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
    UpgradeRule upgrade_rule_{};
};

template <AttributesSkillsMenuMode Mode>
class AttributesSkillsMenuScreenRuntime final {
public:
    static constexpr bool kAllowsKarmaUpgrades =
        Mode == AttributesSkillsMenuMode::hotel_karma_upgrade;

    void open(const RunnerRecordFields& active_record) {
        controller_.open(active_record, Mode);
    }

    void close_to_parent() noexcept { controller_.close_to_parent(); }
    bool is_open() const noexcept { return controller_.is_open(); }
    bool allows_karma_upgrades() const noexcept {
        return kAllowsKarmaUpgrades;
    }
    bool upgrade_prompt_open() const noexcept {
        return controller_.upgrade_prompt_open();
    }
    std::uint8_t upgrade_prompt_selection() const noexcept {
        return controller_.upgrade_prompt_selection();
    }
    const AttributesSkillsMenuState& state() const noexcept {
        return controller_.state();
    }
    AttributesSkillsMenuMessage message() const noexcept {
        return controller_.message();
    }
    AttributeSkillGrid grid(const RunnerRecordFields& record) const {
        return controller_.grid(record);
    }
    AttributeSkillUpgradePreview preview(
        const RunnerRecordFields& record) const {
        return controller_.preview(record);
    }
    void refresh(const RunnerRecordFields& active_record) {
        controller_.refresh(active_record);
    }
    void set_upgrade_rule(AttributesSkillsMenuController::UpgradeRule rule) {
        controller_.set_upgrade_rule(std::move(rule));
    }
    void synchronize_cursor(int column, int row,
                            const RunnerRecordFields& active_record) {
        controller_.synchronize_cursor(column, row, active_record);
    }
    AttributesSkillsRuntimeEvent advance(
        std::uint16_t controller_events, RunnerRecordFields& active_record,
        const std::function<void()>& advance_active_runner_selection = {}) {
        return controller_.advance(controller_events, active_record,
                                   advance_active_runner_selection);
    }

    const AttributesSkillsMenuController& controller() const noexcept {
        return controller_;
    }

private:
    AttributesSkillsMenuController controller_{};
};

using CommandMenuKarmaViewRuntime = AttributesSkillsMenuScreenRuntime<
    AttributesSkillsMenuMode::command_view_only>;
using HotelKarmaUpgradeRuntime = AttributesSkillsMenuScreenRuntime<
    AttributesSkillsMenuMode::hotel_karma_upgrade>;

} // namespace shadowrun::ghidra_conversion::qt_runtime
