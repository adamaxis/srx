#include "attributes_skills_menu_runtime.hpp"

#include "shadowrun_ghidra/core/selection_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion::qt_runtime {

void AttributesSkillsMenuController::open(
    const RunnerRecordFields& active_record,
    const AttributesSkillsMenuMode mode) {
    mode_ = mode;
    upgrade_prompt_open_ = false;
    upgrade_prompt_selection_ = 0U;
    message_ = AttributesSkillsMenuMessage::none;
    AttributesSkillsMenuInitializationCallbacks callbacks{};
    callbacks.render_attributes_and_skills = [this, &active_record] {
        return render_attributes_and_skills(
            active_record, state_.cursor, allows_karma_upgrades());
    };
    initialize_attributes_and_skills_menu(state_, callbacks);
    refresh_selection_state(active_record);
}

void AttributesSkillsMenuController::close_to_parent() noexcept {
    state_.open = false;
    upgrade_prompt_open_ = false;
    upgrade_prompt_selection_ = 0U;
    message_ = AttributesSkillsMenuMessage::none;
}

AttributeSkillGrid AttributesSkillsMenuController::grid(
    const RunnerRecordFields& record) const {
    return read_attributes_and_skills_grid(record, true);
}

AttributeSkillUpgradePreview AttributesSkillsMenuController::preview(
    const RunnerRecordFields& record) const {
    return preview_attribute_skill_upgrade(record, state_.cursor);
}

void AttributesSkillsMenuController::refresh(
    const RunnerRecordFields& active_record) {
    if (state_.open) {
        refresh_selection_state(active_record);
    }
}

void AttributesSkillsMenuController::synchronize_cursor(
    const int column, const int row,
    const RunnerRecordFields& active_record) {
    if (!state_.open) {
        return;
    }
    state_.cursor.column = static_cast<std::uint8_t>(std::clamp(
        column, 0, static_cast<int>(kAttributesSkillsColumnCount) - 1));
    state_.cursor.row = static_cast<std::uint8_t>(std::clamp(
        row, 0, static_cast<int>(kAttributesSkillsRowCount) - 1));
    refresh_selection_state(active_record);
}

void AttributesSkillsMenuController::refresh_selection_state(
    const RunnerRecordFields& active_record) {
    const auto result = render_attributes_and_skills(
        active_record, state_.cursor, allows_karma_upgrades());
    state_.selection_data_available =
        result.outcome !=
        AttributeSkillRenderOutcome::selection_data_unavailable;
    state_.upgrade_error_marker =
        result.outcome ==
        AttributeSkillRenderOutcome::invalid_upgrade_selection;
    message_ = AttributesSkillsMenuMessage::none;
    if (allows_karma_upgrades()) {
        const auto selection_preview = preview(active_record);
        if (selection_preview.status ==
            AttributeSkillUpgradeStatus::at_limit) {
            message_ = AttributesSkillsMenuMessage::maximum_level;
        }
    }
}

AttributesSkillsRuntimeEvent AttributesSkillsMenuController::advance(
    const std::uint16_t controller_events,
    RunnerRecordFields& active_record,
    const std::function<void()>& advance_active_runner_selection) {
    if (!state_.open) {
        return AttributesSkillsRuntimeEvent::no_change;
    }

    if (upgrade_prompt_open_) {
        // ROM 0xD53C gives Up and Down priority over the confirm/cancel group.
        if ((controller_events & 0x0100U) != 0U) {
            upgrade_prompt_selection_ = move_wrapped_menu_cursor(
                upgrade_prompt_selection_, -1, 1U);
            return AttributesSkillsRuntimeEvent::upgrade_prompt_moved;
        }
        if ((controller_events & 0x0200U) != 0U) {
            upgrade_prompt_selection_ = move_wrapped_menu_cursor(
                upgrade_prompt_selection_, 1, 1U);
            return AttributesSkillsRuntimeEvent::upgrade_prompt_moved;
        }
        if ((controller_events & 0x7000U) == 0U) {
            return AttributesSkillsRuntimeEvent::no_change;
        }

        upgrade_prompt_open_ = false;
        if (upgrade_prompt_selection_ != 0U) {
            upgrade_prompt_selection_ = 0U;
            message_ = AttributesSkillsMenuMessage::none;
            return AttributesSkillsRuntimeEvent::upgrade_declined;
        }

        const auto result = apply_attribute_skill_upgrade(
            active_record, state_.cursor, true);
        upgrade_prompt_selection_ = 0U;
        refresh_selection_state(active_record);
        if (result == AttributeSkillUpgradeResult::upgraded) {
            return AttributesSkillsRuntimeEvent::upgrade_applied;
        }
        if (result == AttributeSkillUpgradeResult::insufficient_karma) {
            message_ = AttributesSkillsMenuMessage::insufficient_karma;
            return AttributesSkillsRuntimeEvent::insufficient_karma;
        }
        return AttributesSkillsRuntimeEvent::upgrade_blocked;
    }

    const AttributesSkillsMenuInputFrame input{
        (controller_events & 0x0400U) != 0U,
        (controller_events & 0x0800U) != 0U,
        (controller_events & 0x0100U) != 0U,
        (controller_events & 0x0200U) != 0U,
        (controller_events & 0x5000U) != 0U,
        (controller_events & 0x2000U) != 0U,
        (controller_events & 0x8000U) != 0U,
    };
    const auto event = apply_attributes_skills_menu_input(
        state_, active_record, input);
    switch (event) {
    case AttributesSkillsMenuInputEvent::waiting:
        return AttributesSkillsRuntimeEvent::no_change;
    case AttributesSkillsMenuInputEvent::selection_moved:
        refresh_selection_state(active_record);
        return AttributesSkillsRuntimeEvent::selection_moved;
    case AttributesSkillsMenuInputEvent::upgrade_requested: {
        const auto result = apply_attribute_skill_upgrade(
            active_record, state_.cursor, false);
        if (result == AttributeSkillUpgradeResult::confirmation_required) {
            upgrade_prompt_open_ = true;
            upgrade_prompt_selection_ = 0U;
            return AttributesSkillsRuntimeEvent::upgrade_prompt_opened;
        }
        if (result == AttributeSkillUpgradeResult::insufficient_karma) {
            message_ = AttributesSkillsMenuMessage::insufficient_karma;
            return AttributesSkillsRuntimeEvent::insufficient_karma;
        }
        return AttributesSkillsRuntimeEvent::upgrade_blocked;
    }
    case AttributesSkillsMenuInputEvent::upgrade_blocked:
        refresh_selection_state(active_record);
        if (preview(active_record).status ==
            AttributeSkillUpgradeStatus::class_restricted) {
            message_ = AttributesSkillsMenuMessage::mage_only_skill;
        }
        return AttributesSkillsRuntimeEvent::upgrade_blocked;
    case AttributesSkillsMenuInputEvent::active_runner_selection_requested:
        if (advance_active_runner_selection) {
            advance_active_runner_selection();
            refresh_selection_state(active_record);
        }
        return AttributesSkillsRuntimeEvent::
            active_runner_selection_requested;
    case AttributesSkillsMenuInputEvent::exited_to_field:
        upgrade_prompt_open_ = false;
        return AttributesSkillsRuntimeEvent::returned_to_parent;
    }
    return AttributesSkillsRuntimeEvent::no_change;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
