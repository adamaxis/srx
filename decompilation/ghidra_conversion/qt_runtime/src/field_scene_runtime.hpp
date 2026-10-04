#pragma once

#include "shadowrun_ghidra/core/controller_wait_conversion.hpp"
#include "shadowrun_ghidra/menus/pocket_secretary_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/first_aid_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_state_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_camera_conversion.hpp"
#include "shadowrun_ghidra/field/field_hud_conversion.hpp"
#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"
#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_route_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_step_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_mode_transition_conversion.hpp"
#include "shadowrun_ghidra/field/terrain_probe_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_exit_transition_finalizer_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_runner_state_asset_refresh_conversion.hpp"
#include "shadowrun_ghidra/integration/field_runtime_coordinator_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_bootstrap_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/inventory_action_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_confirmation_conversion.hpp"
#include "shadowrun_ghidra/session/runner_switch_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"
#include "shadowrun_ghidra/world/world_entity_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"
#include "shadowrun_ghidra/world/world_frame_logic_conversion.hpp"
#include "shadowrun_ghidra/world/world_scene_entity_conversion.hpp"

#include <QImage>

#include "field_command_overview_bridge.hpp"
#include "field_clips_menu_runtime.hpp"
#include "field_party_roster_runtime.hpp"
#include "field_dialogue_runtime.hpp"
#include "hotel_menu_runtime.hpp"
#include "field_mode_text_runtime.hpp"
#include "field_interaction_runtime.hpp"
#include "field_menu_text_catalog.hpp"
#include "shop_menu_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class CommandOverviewMagicState {
    inactive,
    magical_defense_wait,
    spell_panel,
    spell_force_selection,
    spell_posture_selection,
};

enum class CommandOverviewFirstAidState {
    inactive,
    target_selection,
    result_wait,
};

enum class CommandOverviewAutomaticUseState {
    inactive,
    result_wait,
};

enum class CommandOverviewActionMessage {
    none,
    cannot_discard,
    cannot_trade,
};

struct MagicDefenseStripView {
    std::uint16_t value = 0;
    std::uint8_t bar_row = 0;
};

// Composition-only adapter for the separate Ghidra-conversion Qt executable.
// It owns no ROM view and does not duplicate converted field state. The
// conversion bootstrap remains authoritative; this class supplies host
// presentation callbacks and paints the direct source assets it receives.
class FieldSceneRuntime final {
public:
    explicit FieldSceneRuntime(
        std::function<void(std::string_view)> source_boundary,
        std::function<void(std::uint16_t)> source_audio_event = {});

    bool bootstrap(
        const integration::StartupIntegrationState& startup_state);
    void advance_frame();
    void set_controller_nibble(std::uint8_t dpad_nibble) noexcept;
    void set_controller_port_byte(std::uint8_t port_byte) noexcept;
    void paint(QPainter& painter) const;
    bool open_command_overview_for_dump();

    bool command_overview_open() const noexcept {
        return command_overview_.open();
    }
    bool dialogue_open() const noexcept { return field_dialogue_.open(); }
    const DialoguePanelView& dialogue_panel() const noexcept {
        return field_dialogue_.panel();
    }
    bool open_stoker_dialogue_for_dump(
        StokerMotelPromptId prompt) {
        return field_dialogue_.open_stoker_prompt_for_dump(prompt);
    }
    bool open_stoker_arrival_result_for_dump(std::uint8_t random_roll) {
        return field_dialogue_.open_stoker_arrival_result_for_dump(
            random_roll);
    }
    bool open_stoker_room_payment_for_dump(std::uint32_t funds);
    FieldDialogueAdvanceResult advance_stoker_dialogue_for_dump(
        std::uint16_t event_word);
    bool hotel_menu_open() const noexcept { return hotel_menu_.open(); }
    HotelMenuPhase hotel_menu_phase() const noexcept {
        return hotel_menu_.phase();
    }
    std::uint8_t hotel_option_selection() const noexcept {
        return static_cast<std::uint8_t>(hotel_menu_.option_selection());
    }
    std::uint8_t hotel_checkout_selection() const noexcept {
        return static_cast<std::uint8_t>(hotel_menu_.checkout_selection());
    }
    std::uint32_t hotel_animation_frames() const noexcept {
        return hotel_menu_.animation_frames();
    }
    HotelMenuEvent advance_hotel_menu_for_dump(std::uint16_t events);
    std::optional<std::uint32_t> notebook_story_flags_for_dump() const noexcept {
        return notebook_tip_flags_resolved_
                   ? std::optional<std::uint32_t>{notebook_tip_flags_[0]}
                   : std::nullopt;
    }
    std::optional<std::uint8_t> notebook_referral_flags_for_dump() const noexcept {
        return notebook_referral_flags_;
    }
    std::optional<std::array<bool, 3>> notebook_event_gates() const noexcept {
        if (!notebook_contacts_flags_.has_value() ||
            !notebook_referral_flags_.has_value() ||
            !notebook_shadowrunner_flags_.has_value()) {
            return std::nullopt;
        }
        return std::array<bool, 3>{
            *notebook_contacts_flags_ != 0U,
            *notebook_referral_flags_ != 0U,
            *notebook_shadowrunner_flags_ != 0U};
    }
    bool open_ares_mode_text_for_dump(std::int8_t alternate_result) {
        return field_mode_text_.open(
            FieldModeTextPromptContext{2U, 11U, alternate_result});
    }
    bool field_interaction_open() const noexcept {
        return field_interaction_.open();
    }
    const DialoguePanelView& field_interaction_panel() const noexcept {
        return field_interaction_.panel();
    }
    bool dispatch_field_interaction_through_coordinator_for_dump(
        FieldInteractionPrompt prompt, std::uint8_t interaction_selection = 2U,
        bool interaction_suppressed = false,
        std::optional<std::uint8_t> security_floor_index = std::nullopt);
    FieldInteractionAdvanceResult advance_field_interaction_for_dump(
        std::uint16_t event_word) {
        return advance_field_interaction(event_word);
    }
    bool begin_ares_shop_event_for_dump();
    bool shop_menu_open() const noexcept { return field_shop_menu_.is_open(); }
    ShopActionGroup shop_menu_group() const noexcept {
        return field_shop_menu_.selected_group();
    }
    std::uint8_t shop_menu_cursor() const noexcept {
        return field_shop_menu_.selected_cursor();
    }
    bool shop_menu_offers_active() const noexcept {
        return field_shop_menu_.offers_panel_active();
    }
    bool shop_transaction_confirmation_open() const noexcept {
        return field_shop_menu_.transaction_confirmation_open();
    }
    bool shop_transaction_waiting_for_event() const noexcept {
        return field_shop_menu_.transaction_waiting_for_event();
    }
    const MenuConfirmationState& shop_transaction_confirmation() const noexcept {
        return field_shop_menu_.transaction_confirmation();
    }
    ShopTransactionMessage shop_transaction_message() const noexcept {
        return field_shop_menu_.transaction_message();
    }
    FieldActionApplicationResult shop_last_purchase_result() const noexcept {
        return field_shop_menu_.last_purchase_result();
    }
    const ShopSaleOutcome& shop_last_sale_outcome() const noexcept {
        return field_shop_menu_.last_sale_outcome();
    }
    std::optional<std::uint32_t> field_funds_for_dump() const noexcept {
        return field_funds_;
    }
    void set_field_funds_for_dump(const std::uint32_t funds) noexcept {
        field_funds_ = funds;
    }
    const FieldCommandOverviewView& command_overview_view() const noexcept {
        return command_overview_.view();
    }
    RunnerRecordFields& active_runner_record() noexcept {
        return field_party_roster_.active_record();
    }
    void install_party_member_for_dump(std::size_t slot,
                                       RunnerRecordFields record,
                                       std::string name) {
        field_party_roster_.install_party_member_for_dump(
            slot, std::move(record), std::move(name));
    }
    void set_active_party_slot_for_dump(std::size_t slot) {
        field_party_roster_.set_active_slot(slot);
        field_entity_selection_state_.active_index =
            field_party_roster_.active_slot();
        field_entity_behavior_flags_ = static_cast<std::uint8_t>(
            active_runner_record().flags >> 8U);
        field_movement_behavior_record_.behavior_flags =
            field_entity_behavior_flags_;
        refresh_field_hud_runner_slot();
    }
    bool field_party_slot_active_for_dump(std::size_t slot) const noexcept {
        return field_party_roster_.slot_active(slot);
    }
    std::size_t field_active_party_slot_for_dump() const noexcept {
        return field_party_roster_.active_slot();
    }
    const RunnerRecordFields& active_runner_record() const noexcept {
        return field_party_roster_.active_record();
    }
    const std::array<RunnerRecordFields,
                     FieldPartyRosterRuntime::kSlotCount>&
    field_party_records_for_dump() const noexcept {
        return field_party_roster_.records();
    }
    const EquipmentCombatBarResult& command_overview_combat_bars() const
        noexcept {
        return command_overview_combat_bars_;
    }
    bool inventory_action_open() const noexcept {
        return command_overview_inventory_action_active_;
    }
    std::uint8_t inventory_action_selection() const noexcept {
        return command_overview_inventory_action_.action_selector;
    }
    bool first_aid_open() const noexcept {
        return command_overview_first_aid_state_ !=
               CommandOverviewFirstAidState::inactive;
    }
    bool first_aid_result_waiting() const noexcept {
        return command_overview_first_aid_state_ ==
               CommandOverviewFirstAidState::result_wait;
    }
    std::size_t first_aid_target_cursor() const noexcept {
        return command_overview_first_aid_cursor_;
    }
    const FirstAidTargetList& first_aid_targets() const noexcept {
        return command_overview_first_aid_targets_;
    }
    FirstAidMessage first_aid_message() const noexcept {
        return command_overview_first_aid_message_;
    }
    bool automatic_use_result_waiting() const noexcept {
        return command_overview_automatic_use_state_ ==
               CommandOverviewAutomaticUseState::result_wait;
    }
    bool discard_confirmation_open() const noexcept {
        return command_overview_discard_confirmation_active_;
    }
    bool discard_confirmation_yes() const noexcept {
        return command_overview_discard_confirmation_.selected_option == 0U;
    }
    bool inventory_trade_open() const noexcept {
        return command_overview_inventory_trade_.open;
    }
    std::size_t inventory_trade_recipient() const noexcept {
        return command_overview_inventory_trade_.recipient_slot;
    }
    std::size_t inventory_trade_recipient_inventory_slot() const noexcept {
        return command_overview_inventory_trade_.recipient_inventory_slot;
    }
    bool inventory_trade_recipient_full() const noexcept {
        return command_overview_inventory_trade_.recipient_inventory_full;
    }
    bool inventory_trade_yes_selected() const noexcept {
        return command_overview_inventory_trade_.yes_selected;
    }
    CommandOverviewActionMessage inventory_action_message() const noexcept {
        return command_overview_action_message_;
    }
    bool clips_menu_open() const noexcept {
        return command_overview_clips_.is_open();
    }
    bool cyberware_menu_open() const noexcept {
        return command_overview_cyberware_active_;
    }
    bool magic_defense_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::magical_defense_wait;
    }
    bool magic_spell_panel_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_panel;
    }
    bool magic_force_selection_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_force_selection;
    }
    bool magic_posture_selection_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_posture_selection;
    }
    bool magic_spell_interface_open() const noexcept {
        return magic_spell_panel_open() || magic_force_selection_open() ||
               magic_posture_selection_open();
    }
    std::uint16_t magic_current_force() const noexcept {
        return command_overview_magic_force_.current_force;
    }
    std::uint16_t magic_maximum_force() const noexcept {
        return command_overview_magic_force_.maximum_force;
    }
    MagicMenuSelection magic_spell_selection() const noexcept {
        return command_overview_magic_loop_.selection;
    }
    const std::optional<MagicSpellSelection>& magic_selected_spell() const
        noexcept {
        return command_overview_magic_selected_spell_;
    }
    std::uint8_t magic_cursor_phase() const noexcept {
        return command_overview_magic_cursor_phase_;
    }
    const MagicDefenseStripView& magic_physical_defense_view() const noexcept {
        return command_overview_magic_physical_defense_;
    }
    const MagicDefenseStripView& magic_mana_defense_view() const noexcept {
        return command_overview_magic_mana_defense_;
    }
    const CyberwareMenuModel& cyberware_menu_model() const noexcept {
        return command_overview_cyberware_;
    }
    std::uint8_t clips_menu_selection() const noexcept {
        return command_overview_clips_.selection();
    }
    bool clips_reload_confirmation_open() const noexcept {
        return command_overview_clips_.reload_confirmation_open();
    }
    bool clips_reload_confirmation_yes() const noexcept {
        return command_overview_clips_.reload_confirmation_yes();
    }
    ClipsMenuMessage clips_menu_message() const noexcept {
        return command_overview_clips_.message();
    }

    bool ready() const noexcept { return ready_; }
    // Navigate the converted command loop to a specific primary page so QA
    // dumps can exercise pages other than the entry selector (6).
    bool seek_command_overview_page(std::uint8_t page);
    bool pocket_secretary_open() const noexcept {
        return pocket_secretary_.open;
    }
    bool pocket_secretary_cyberdeck_unavailable_waiting() const noexcept {
        return pocket_secretary_.cyberdeck_unavailable_wait;
    }
    bool pocket_secretary_group_items_waiting() const noexcept {
        return pocket_secretary_.group_items_wait;
    }
    bool pocket_secretary_notebook_open() const noexcept {
        return pocket_secretary_.notebook.open;
    }
    bool pocket_secretary_notebook_no_run_waiting() const noexcept {
        return pocket_secretary_.notebook.no_current_run_wait;
    }
    std::uint8_t pocket_secretary_notebook_selection() const noexcept {
        return pocket_secretary_.notebook.selection;
    }
    std::uint8_t pocket_secretary_notebook_option_count() const noexcept {
        return pocket_secretary_.notebook.option_count;
    }
    bool pocket_secretary_notebook_tips_open() const noexcept {
        return pocket_secretary_.notebook.tips.open;
    }
    std::uint8_t pocket_secretary_notebook_tips_selection() const noexcept {
        return pocket_secretary_.notebook.tips.selection;
    }
    std::uint8_t pocket_secretary_notebook_visible_tip_count() const noexcept {
        return pocket_secretary_.notebook.tips.visible_count;
    }
    std::uint8_t pocket_secretary_notebook_visible_tip_source_index(
        std::size_t page) const noexcept {
        const auto& tips = pocket_secretary_.notebook.tips;
        return page < tips.visible_count
                   ? tips.visible_source_indices[page]
                   : 0xFFU;
    }
    bool notebook_tip_flags_resolved() const noexcept {
        return notebook_tip_flags_resolved_;
    }
    // Story event writers use the same typed source bit order as F9A4. This
    // adapter currently seeds fresh New Game; save restoration must supply
    // the full three-longword state before mutating individual tips.
    bool set_notebook_tip_unlocked(std::size_t source_index,
                                   bool unlocked) noexcept;
    // QA seam for the source FFFC22 catalog branch. Normal play obtains this
    // word from quest-item event handling, which is not connected yet.
    void set_group_item_flags_for_dump(std::uint32_t flags) noexcept {
        group_item_flags_ = flags & 0x7FFFU;
    }
    std::uint8_t pocket_secretary_selection() const noexcept {
        return pocket_secretary_.selection;
    }
    bool pocket_secretary_dismiss_runner_open() const noexcept {
        return dismiss_runner_.open;
    }
    std::uint8_t dismiss_runner_selection() const noexcept {
        return dismiss_runner_.selection;
    }
    std::uint8_t dismiss_runner_option_count() const noexcept {
        return static_cast<std::uint8_t>(dismiss_runner_.candidate_count + 1U);
    }
    std::optional<std::size_t> dismiss_runner_preview_slot() const noexcept {
        if (!dismiss_runner_.open || dismiss_runner_.selection == 0U ||
            dismiss_runner_.selection > dismiss_runner_.candidate_count) {
            return std::nullopt;
        }
        return dismiss_runner_.candidate_slots[
            static_cast<std::size_t>(dismiss_runner_.selection - 1U)];
    }
    std::uint32_t dismiss_runner_effect_frames() const noexcept {
        return dismiss_runner_effect_frames_;
    }
    bool posture_dialog_open() const noexcept {
        return posture_dialog_active_;
    }
    std::uint8_t posture_dialog_value() const noexcept {
        return active_runner_record().posture;
    }
    // Presentation seam for QA dumps: enter the 0x119CE posture adjustment
    // dialog the way the converted command loop arms it.
    void open_posture_dialog_for_dump() noexcept { posture_dialog_active_ = true; }
    bool field_entity_visible() const noexcept {
        return field_entity_.visible;
    }
    const std::string& failure() const noexcept { return failure_; }
    const integration::FieldSessionBootstrapState& state() const noexcept {
        return state_;
    }
    const SceneRunnerPosition& field_entity_world_position() const noexcept {
        return field_entity_.world_position;
    }

private:
    FieldInteractionAdvanceResult advance_field_interaction(
        std::uint16_t event_word);
    void configure_callbacks();
    void configure_runtime_coordinator_callbacks();
    void advance_field_movement();
    void initialize_field_entity_selection_state();
    void refresh_field_runner_state_and_assets();
    void handle_field_mode_action_event();
    void run_field_mode_exit_transition();
    bool run_field_exit_orchestrator(
        integration::FieldRuntimeCoordinatorState& runtime_state);
    bool project_field_world_interaction();
    void update_source_field_camera();
    void apply_field_entity_animation(bool walking);
    bool recover_blocked_field_movement(
        const MovementProbeResult& initial_probe);
    void advance_field_scene_transition_after_modal();
    std::optional<std::size_t> projected_terrain_cell() const;
    void record_field_activity_start(
        MovementModeTransitionContinuation continuation);
    std::int16_t evaluate_field_collision(std::uint16_t candidate_x,
                                          std::uint16_t candidate_y);
    void source_boundary(std::string_view boundary) const;
    bool load_image(const ConversionAssetCatalog& catalog,
                    std::string_view alias,
                    QImage& destination);
    bool load_sprite_image(const ConversionAssetCatalog& catalog,
                           std::string_view alias,
                           QImage& destination,
                           int& origin_x,
                           int& origin_y);
    void paint_field_entity(QPainter& painter) const;
    void paint_field_hud_overlays(QPainter& painter) const;
    void paint_command_overview_magic(QPainter& painter) const;
    void paint_command_overview(QPainter& painter) const;
    void paint_command_overview_clips(QPainter& painter) const;
    void paint_field_shop(QPainter& painter) const;
    void begin_hotel_menu();
    HotelMenuEvent process_hotel_menu_events(std::uint16_t events);
    void draw_command_overview_text(QPainter& painter, int x, int y,
                                    std::string_view text) const;
    void draw_hotel_menu_text(QPainter& painter, int x, int y,
                              std::string_view text,
                              HotelMenuPaletteLine palette_line) const;
    void refresh_command_overview_combat_bars();
    void refresh_command_overview_item_detail();
    void refresh_field_hud_runner_slot();
    void initialize_field_party_roster(
        const integration::StartupIntegrationState& startup_state);
    void advance_command_overview_inventory_action(
        std::uint16_t controller_events);
    void begin_command_overview_first_aid();
    void advance_command_overview_first_aid(
        std::uint16_t controller_events);
    void finish_command_overview_first_aid();
    void advance_command_overview_automatic_use(
        std::uint16_t controller_events);
    void finish_command_overview_automatic_use();
    void advance_command_overview_clips(std::uint16_t controller_events);
    void begin_command_overview_inventory_action();
    void begin_command_overview_cyberware();
    void begin_command_overview_magic();
    void advance_command_overview_magic(std::uint16_t controller_events);
    void begin_command_overview_magic_spell_action();
    void finish_command_overview_magic_spell_action();
    void begin_command_overview_clips();
    void begin_pocket_secretary();
    void begin_pocket_secretary_dismiss_runner();
    void finish_pocket_secretary_dismiss_runner(std::size_t slot);
    void advance_pocket_secretary_dismiss_runner_effect();
    void dispatch_command_overview_inventory_action(
        const InventoryActionResult& result);
    void advance_command_overview_discard_confirmation(
        std::uint16_t controller_events);
    void advance_command_overview_inventory_trade(
        std::uint16_t controller_events);
    void open_command_overview();
    bool load_command_overview_assets(const ConversionAssetCatalog& catalog);
    bool load_field_hud_overlays(const ConversionAssetCatalog& catalog);

    struct FieldEntityPresentation {
        bool active = false;
        SceneRunnerPosition world_position{};
        EntityProjectionState projection{};
        std::uint8_t direction = 0;
        bool visible = false;
    };

    std::function<void(std::string_view)> source_boundary_callback_;
    std::function<void(std::uint16_t)> source_audio_event_callback_;
    integration::FieldSessionBootstrapInput input_;
    integration::FieldSessionBootstrapState state_;
    integration::FieldSessionBootstrapCallbacks callbacks_;
    integration::FieldRuntimeCoordinatorState field_runtime_state_;
    integration::FieldRuntimeCoordinatorCallbacks field_runtime_callbacks_;
    ControllerSampleState field_controller_sample_state_{};
    ControllerLatchState field_controller_latch_state_{};
    WorldFrameState world_frame_state_;
    WorldFrameCallbacks world_frame_callbacks_;
    WorldFrameLogicState world_frame_logic_state_;
    WorldFrameLogicCallbacks world_frame_logic_callbacks_;
    FieldViewportState viewport_state_;
    FieldCommandOverviewBridge command_overview_;
    FieldDialogueRuntime field_dialogue_;
    HotelMenuRuntime hotel_menu_{};
    FieldModeTextRuntime field_mode_text_{};
    FieldInteractionRuntime field_interaction_;
    ShopMenuController field_shop_menu_{};
    FieldDialogueEventResolver field_dialogue_event_resolver_;
    FieldPartyRosterRuntime field_party_roster_{};
    DismissRunnerMenuState dismiss_runner_{};
    RandomGridEffectState dismiss_runner_random_grid_effect_{};
    std::uint32_t dismiss_runner_effect_frames_ = 0U;
    std::array<std::int8_t, 12> dismiss_runner_random_grid_cells_{
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    std::size_t field_joshua_archetype_selection_ = 0U;
    QImage field_map_;
    QImage field_hud_;
    QImage field_service_background_;
    QImage field_service_universal_clips_icon_;
    std::array<QImage, 6> field_service_tab_images_{};
    QImage command_overview_background_;
    QImage command_overview_inventory_boxes_;
    QImage command_overview_font_;
    QImage hotel_checkout_font_;
    QImage command_overview_cursor_;
    FieldMenuTextCatalog field_menu_text_{};
    std::array<QImage, 3> command_overview_portraits_{};
    std::array<QImage, 5> command_overview_focus_{};
    std::array<QImage, 5> command_overview_status_bars_{};
    std::array<QImage, 2> command_overview_selected_cells_{};
    QImage command_overview_action_background_;
    std::array<QImage, 4> command_overview_action_focus_{};
    QImage command_overview_action_confirmation_background_;
    std::array<QImage, 2> command_overview_action_confirmation_focus_{};
    std::array<QImage, 4> command_overview_clips_focus_{};
    std::array<QImage, 4> command_overview_clips_trade_focus_{};
    QImage command_overview_magic_panel_background_;
    std::array<QImage, kMagicSpellTextCount> command_overview_magic_spell_icons_{};
    std::array<QImage, 4> command_overview_magic_cursor_frames_{};
    std::array<QImage, 5> command_overview_magic_posture_bars_{};
    std::array<QImage, 5> command_overview_magic_force_meter_tiles_{};
    std::array<QImage, 4> command_overview_magic_availability_bars_{};
    std::array<QImage, 10> command_overview_magic_damage_bars_{};
    std::array<std::array<QImage, 2>, 50> command_overview_inventory_icons_{};
    std::array<QImage, 50> command_overview_item_details_{};
    std::array<QImage, 14> command_overview_damage_bars_{};
    std::array<QImage, 14> command_overview_drain_bars_{};
    std::array<QImage, 256> command_overview_detail_values_{};
    std::array<QImage, 23> command_overview_attack_bars_{};
    std::array<QImage, 23> command_overview_defense_bars_{};
    ResourceVariantState command_overview_resource_variant_{};
    EquipmentCombatBarResult command_overview_combat_bars_{};
    ItemActionDetailResult command_overview_item_detail_{};
    InventoryActionMenuState command_overview_inventory_action_{};
    CommandOverviewFirstAidState command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::inactive;
    FirstAidTargetList command_overview_first_aid_targets_{};
    std::size_t command_overview_first_aid_cursor_ = 0U;
    FirstAidMessage command_overview_first_aid_message_ =
        FirstAidMessage::none;
    CommandOverviewAutomaticUseState command_overview_automatic_use_state_ =
        CommandOverviewAutomaticUseState::inactive;
    std::uint16_t field_random_seed_ = 0U;
    std::optional<SelectedInventoryItem>
        command_overview_selected_inventory_item_;
    MenuConfirmationState command_overview_discard_confirmation_{};
    InventoryTradeState command_overview_inventory_trade_{};
    FieldClipsMenuRuntime command_overview_clips_{};
    CyberwareMenuModel command_overview_cyberware_{};
    bool command_overview_cyberware_active_ = false;
    CommandOverviewMagicState command_overview_magic_state_ =
        CommandOverviewMagicState::inactive;
    MagicDefenseStripView command_overview_magic_physical_defense_{};
    MagicDefenseStripView command_overview_magic_mana_defense_{};
    MagicMenuLoopState command_overview_magic_loop_{};
    MagicForceState command_overview_magic_force_{};
    std::optional<MagicSpellSelection> command_overview_magic_selected_spell_;
    std::uint8_t command_overview_magic_cursor_phase_ = 0U;
    std::uint8_t command_overview_magic_cursor_timer_ = 0U;
    CommandOverviewActionMessage command_overview_action_message_ =
        CommandOverviewActionMessage::none;
    bool command_overview_inventory_action_active_ = false;
    bool command_overview_inventory_selector_suppressed_ = false;
    bool command_overview_discard_confirmation_active_ = false;
    std::array<QImage, kFieldHudSlotCount> field_hud_faces_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_items_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_unarmed_items_{};
    std::array<QImage, kFieldHudMeterLevelCount>
        field_hud_delay_meter_counters_{};
    std::array<QImage,
               kFieldHudMeterLevelCount * kFieldHudBehaviorMeterProfileCount>
        field_hud_behavior_meter_counters_{};
    std::array<QImage, kFieldHudAmmoCounterCount> field_hud_ammo_counters_{};
    std::array<FieldHudSlotState, kFieldHudSlotCount> field_hud_slots_{};
    PocketSecretaryMenuState pocket_secretary_{};
    // FF018F bit 0 is session state. Fresh New Game starts clear; restored
    // saves remain unknown until that global is included in the save adapter.
    std::optional<bool> cyberdeck_available_ = false;
    // FFFC22 is not present in the current save adapter. New Game initializes
    // it to zero; restored saves remain an explicit source-state boundary.
    std::optional<std::uint32_t> group_item_flags_ = 0U;
    // A fresh New Game has no active run. Save restoration does not yet
    // publish the source FFFBEC run state to this presentation adapter.
    bool notebook_current_run_available_ = false;
    std::array<std::uint32_t, 3> notebook_tip_flags_{{1U, 0U, 0U}};
    bool notebook_tip_flags_resolved_ = true;
    // FFFC2C, FFFC27, and FFFC2A independently gate Contacts, Mr. Johnsons,
    // and Shadowrunners in F33E. Fresh New Game seeds Contacts to 0x1C7F at
    // 0x59C48 and clears the other two; a loaded save does not restore them.
    std::optional<std::uint32_t> notebook_contacts_flags_ = 0x1C7FU;
    std::optional<std::uint8_t> notebook_referral_flags_ = 0U;
    std::optional<std::uint16_t> notebook_shadowrunner_flags_ = 0U;
    QImage pocket_secretary_background_;
    QImage pocket_secretary_grid_;
    QImage hotel_options_cursor_;
    QImage hotel_checkout_cursor_;
    std::unordered_map<std::string, QImage> dismiss_runner_preview_panels_;
    std::array<QImage, 7> dismiss_runner_random_grid_patterns_{};
    QImage command_overview_attributes_background_;
    QImage posture_focus_;
    bool posture_dialog_active_ = false;
    std::array<QImage, 8> field_entity_idle_images_{};
    std::array<int, 8> field_entity_idle_origin_x_{};
    std::array<int, 8> field_entity_idle_origin_y_{};
    // The 0xC0B2 walking lists carry an 0x8008 marker (eight frames per
    // direction); the hurt lists have two and the attack pose one. Keep
    // each direction's own frame count instead of forcing a fixed width.
    std::array<std::vector<QImage>, 8> field_entity_walk_images_{};
    std::array<std::vector<int>, 8> field_entity_walk_origin_x_{};
    std::array<std::vector<int>, 8> field_entity_walk_origin_y_{};
    EntityAnimationState field_entity_animation_{};
    std::uint16_t field_entity_animation_selector_ = 0;
    bool field_entity_walking_ = false;
    std::array<MovementOffset, integration::kFieldMovementDirectionCount>
        field_movement_direction_offsets_{};
    CollisionProbeOffsetBank field_collision_probe_offset_banks_{};
    std::array<std::uint16_t,
               integration::kFieldControllerHeadingTableCount>
        field_controller_heading_table_{};
    std::array<DirectionalMovementRoute,
               integration::kFieldMovementRouteCount>
        field_movement_routes_{};
    std::array<integration::FieldMovementRouteSourceRecord,
               integration::kFieldMovementRouteCount>
        field_movement_route_sources_{};
    EntityMovementStepState field_movement_state_{};
    EntityCollisionRecord field_collision_record_{};
    CollisionProbeState field_collision_probe_state_{};
    EntityBehaviorFrameRecord field_movement_behavior_record_{};
    RunnerSwitchState field_entity_selection_state_{};
    FieldRunnerRefreshState field_runner_refresh_state_{};
    FieldModeExitTransitionState field_mode_exit_transition_state_{};
    std::array<std::uint16_t, 10> field_entity_table_occupancy_{};
    std::array<AuxiliaryWorldEntity, 10> field_entity_table_{};
    EntityMovementModeTransitionState field_movement_transition_state_{};
    WorldActivityState field_world_activity_state_{};
    MovementModeTransitionContinuation field_activity_continuation_ =
        MovementModeTransitionContinuation::none;
    // FFE12C selects a floor label on the successful Security Terminal file
    // branch. Its scene initialization source has not been connected yet.
    std::optional<std::uint8_t> field_security_floor_index_;
    std::uint8_t field_projected_terrain_mark_count_ = 0;
    bool field_terrain_render_dirty_ = false;
    std::array<std::uint8_t, 4> field_terrain_sample_state_{};
    // FFDFE4. Index of the FF1700 collision byte selected by the last 0xE8A0
    // sample. 0x1651E clears that byte; 0xE9EE may leak A2, but the index is
    // only defined for the collision table.
    std::uint16_t field_collision_marker_index_ = 0;
    FieldCameraFollowState field_camera_follow_{};
    bool field_camera_alignment_ready_ = false;
    // FFE114 when 0x1651E selects the scene-4 interaction record. 0x2B34
    // consumes it on the next scene entry.
    std::optional<std::array<std::uint16_t, 8>> field_pending_scene_profile_;
    std::uint8_t field_controller_nibble_ = 0;
    std::uint8_t field_controller_port_byte_ = 0;
    std::uint16_t field_target_heading_ = 0;
    // Typed source field corresponding to FF014C, consumed only by the
    // 0xC7F8 scene-runtime dispatcher.
    std::uint8_t field_action_counter_ = 0;
    std::uint8_t field_entity_behavior_flags_ = 0;
    std::int8_t field_exit_alternate_result_ = 0;
    std::uint32_t field_exit_world_flags_ = 0;
    std::optional<std::uint32_t> field_funds_{0x14U};
    bool field_shop_route_pending_ = false;
    FieldEntityPresentation field_entity_{};
    std::string failure_;
    bool ready_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
