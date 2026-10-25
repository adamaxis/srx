#include "field_scene_runtime.hpp"
#include "field_scene_runtime_internal.hpp"

#include "enums.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_proximity_collision_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_route_primitives_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"
#include "shadowrun_ghidra/field/field_equipment_upgrade_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/session/runner_status_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/vid_phone_service_conversion.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/heading_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_spawn_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"
#include "genesis_image_compositor.hpp"
#include "numeric_visual_renderer.hpp"
#include "field_map_composition.hpp"
#include "matrix_map_presenter.hpp"
#include "matrix_datastore_facts.hpp"

#include <QPainter>
#include <QColor>
#include <QRegion>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

using field_scene_detail::kInitialFieldSceneReference;
using field_scene_detail::kSceneProfileEntityPositionWord;
using field_scene_detail::kFieldModalReturnTransitionFrames;
using field_scene_detail::magic_action_record_from_runner;

namespace {
bool read_save_integer(const QJsonObject& object, const char* key,
                       const std::int64_t minimum,
                       const std::int64_t maximum,
                       std::int64_t& output, QString* const error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isDouble()) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' is missing or invalid.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    const auto number = value.toDouble(std::numeric_limits<double>::quiet_NaN());
    if (!std::isfinite(number) || std::trunc(number) != number ||
        number < static_cast<double>(minimum) ||
        number > static_cast<double>(maximum)) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' is outside its valid range.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    output = static_cast<std::int64_t>(number);
    return true;
}

bool read_save_boolean(const QJsonObject& object, const char* key,
                       bool& output, QString* const error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isBool()) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' is missing or invalid.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    output = value.toBool();
    return true;
}

template <std::size_t Count>
bool read_save_byte_array(const QJsonObject& object, const char* key,
                          std::array<std::uint8_t, Count>& output,
                          QString* const error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isArray() ||
        static_cast<std::size_t>(value.toArray().size()) != Count) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' has the wrong shape.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    const auto values = value.toArray();
    for (std::size_t index = 0; index < Count; ++index) {
        const auto entry = values.at(static_cast<qsizetype>(index));
        const auto number = entry.toDouble(-1.0);
        if (!entry.isDouble() || !std::isfinite(number) ||
            std::trunc(number) != number || number < 0.0 || number > 255.0) {
            if (error != nullptr) {
                *error = QStringLiteral("Save field '%1' has an invalid byte.")
                             .arg(QLatin1String(key));
            }
            return false;
        }
        output[index] = static_cast<std::uint8_t>(number);
    }
    return true;
}

template <typename Value>
void insert_optional_save_number(QJsonObject& object, const char* key,
                                 const std::optional<Value>& value) {
    object.insert(QLatin1String(key),
                  value.has_value()
                      ? QJsonValue(static_cast<double>(*value))
                      : QJsonValue(QJsonValue::Null));
}

template <typename Value>
bool read_optional_save_number(const QJsonObject& object, const char* key,
                               std::optional<Value>& output,
                               QString* const error) {
    const auto value = object.value(QLatin1String(key));
    if (value.isNull()) {
        output.reset();
        return true;
    }
    std::int64_t parsed = 0;
    if (!read_save_integer(object, key,
                           static_cast<std::int64_t>(
                               std::numeric_limits<Value>::lowest()),
                           static_cast<std::int64_t>(
                               std::numeric_limits<Value>::max()),
                           parsed, error)) {
        return false;
    }
    output = static_cast<Value>(parsed);
    return true;
}

template <typename Value, std::size_t Count>
void insert_optional_save_array(
    QJsonObject& object, const char* key,
    const std::optional<std::array<Value, Count>>& values) {
    if (!values.has_value()) {
        object.insert(QLatin1String(key), QJsonValue(QJsonValue::Null));
        return;
    }
    QJsonArray array;
    for (const auto value : *values) array.append(static_cast<int>(value));
    object.insert(QLatin1String(key), array);
}

template <typename Value, std::size_t Count>
bool read_optional_save_array(
    const QJsonObject& object, const char* key,
    std::optional<std::array<Value, Count>>& output,
    QString* const error) {
    const auto value = object.value(QLatin1String(key));
    if (value.isNull()) {
        output.reset();
        return true;
    }
    if (!value.isArray() ||
        static_cast<std::size_t>(value.toArray().size()) != Count) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' has the wrong array shape.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    const auto array = value.toArray();
    std::array<Value, Count> parsed{};
    for (std::size_t index = 0; index < Count; ++index) {
        const auto entry = array.at(static_cast<qsizetype>(index));
        const auto number = entry.toDouble(
            std::numeric_limits<double>::quiet_NaN());
        if (!entry.isDouble() || !std::isfinite(number) ||
            std::trunc(number) != number ||
            number < static_cast<double>(std::numeric_limits<Value>::lowest()) ||
            number > static_cast<double>(std::numeric_limits<Value>::max())) {
            if (error != nullptr) {
                *error = QStringLiteral("Save field '%1' has an invalid array value.")
                             .arg(QLatin1String(key));
            }
            return false;
        }
        parsed[index] = static_cast<Value>(number);
    }
    output = parsed;
    return true;
}

QJsonObject cyberdeck_record_to_json(const CyberdeckRecord& record) {
    QJsonObject object;
    insert_optional_save_number(object, "mpcp", record.mpcp);
    insert_optional_save_number(object, "hardening", record.hardening);
    insert_optional_save_number(object, "memory_megapulses",
                                record.memory_megapulses);
    insert_optional_save_number(object, "storage_megapulses",
                                record.storage_megapulses);
    insert_optional_save_number(object, "load_io_speed", record.load_io_speed);
    insert_optional_save_number(object, "response", record.response);
    insert_optional_save_array(object, "persona", record.persona);
    insert_optional_save_number(object, "persona_condition",
                                record.persona_condition);
    insert_optional_save_number(object, "model_index", record.model_index);
    insert_optional_save_array(object, "program_ratings",
                               record.program_ratings);
    insert_optional_save_array(object, "active_programs",
                               record.active_programs);
    if (!record.data_files.has_value()) {
        object.insert(QStringLiteral("data_files"),
                      QJsonValue(QJsonValue::Null));
    } else {
        QJsonArray files;
        for (const auto& file : *record.data_files) {
            QJsonObject entry;
            entry.insert(QStringLiteral("id"), static_cast<int>(file.id));
            entry.insert(QStringLiteral("size"), static_cast<int>(file.size));
            entry.insert(QStringLiteral("value"), static_cast<int>(file.value));
            files.append(entry);
        }
        object.insert(QStringLiteral("data_files"), files);
    }
    return object;
}

bool cyberdeck_record_from_json(const QJsonObject& object,
                                CyberdeckRecord& record,
                                QString* const error) {
    CyberdeckRecord parsed;
    if (!read_optional_save_number(object, "mpcp", parsed.mpcp, error) ||
        !read_optional_save_number(object, "hardening", parsed.hardening, error) ||
        !read_optional_save_number(object, "memory_megapulses",
                                   parsed.memory_megapulses, error) ||
        !read_optional_save_number(object, "storage_megapulses",
                                   parsed.storage_megapulses, error) ||
        !read_optional_save_number(object, "load_io_speed",
                                   parsed.load_io_speed, error) ||
        !read_optional_save_number(object, "response", parsed.response, error) ||
        !read_optional_save_array(object, "persona", parsed.persona, error) ||
        !read_optional_save_number(object, "persona_condition",
                                   parsed.persona_condition, error) ||
        !read_optional_save_number(object, "model_index", parsed.model_index, error) ||
        !read_optional_save_array(object, "program_ratings",
                                  parsed.program_ratings, error) ||
        !read_optional_save_array(object, "active_programs",
                                  parsed.active_programs, error)) {
        return false;
    }
    const auto files_value = object.value(QStringLiteral("data_files"));
    if (files_value.isNull()) {
        parsed.data_files.reset();
    } else if (files_value.isArray() && files_value.toArray().size() == 5) {
        std::array<CyberdeckDataFile, 5> files{};
        const auto file_array = files_value.toArray();
        for (std::size_t index = 0; index < files.size(); ++index) {
            const auto file_object =
                file_array.at(static_cast<qsizetype>(index)).toObject();
            std::int64_t id = 0;
            std::int64_t size = 0;
            if (!read_save_integer(file_object, "id", -128, 127, id, error) ||
                !read_save_integer(file_object, "size", 0, 0xFFFF, size,
                                   error)) {
                return false;
            }
            std::int64_t value = 0;
            if (file_object.contains(QStringLiteral("value")) &&
                !read_save_integer(file_object, "value", -128, 127, value,
                                   error)) {
                return false;
            }
            files[index] = {static_cast<std::int8_t>(id),
                            static_cast<std::uint16_t>(size),
                            static_cast<std::int8_t>(value)};
        }
        parsed.data_files = files;
    } else {
        if (error != nullptr) {
            *error = QStringLiteral("The saved cyberdeck data-file list is invalid.");
        }
        return false;
    }
    record = std::move(parsed);
    return true;
}

std::optional<GameSaveSlot> game_save_slot_from_index(const int slot) {
    if (slot == 0) return GameSaveSlot::one;
    if (slot == 1) return GameSaveSlot::two;
    return std::nullopt;
}

} // namespace

int FieldSceneRuntime::game_save_slot_status(const int slot) const {
    const auto selected = game_save_slot_from_index(slot);
    return selected.has_value()
               ? static_cast<int>(game_save_store_.status(*selected))
               : static_cast<int>(GameSaveStatus::corrupt);
}

bool FieldSceneRuntime::save_game_slot(const int slot) {
    const auto selected = game_save_slot_from_index(slot);
    if (!selected.has_value() || !ready_ ||
        !state_.field_scene_buffers.initialized ||
        !field_entity_.active || world_frame_logic_state_.world_active != 0U) {
        source_boundary("field.save_load.save_rejected");
        return false;
    }
    QString error;
    const auto snapshot = capture_game_save_snapshot(&error);
    if (!snapshot.has_value()) {
        failure_ = error.toStdString();
        source_boundary("field.save_load.snapshot_failed");
        return false;
    }
    if (!game_save_store_.save(*selected, *snapshot, &error)) {
        failure_ = error.toStdString();
        source_boundary("field.save_load.write_failed");
        return false;
    }
    failure_.clear();
    source_boundary("field.save_load.state_saved");
    return true;
}

bool FieldSceneRuntime::load_game_slot(const int slot) {
    const auto selected = game_save_slot_from_index(slot);
    if (!selected.has_value() || !ready_ ||
        world_frame_logic_state_.world_active != 0U) {
        source_boundary("field.save_load.load_rejected");
        return false;
    }
    const auto loaded = game_save_store_.load(*selected);
    if (loaded.status != GameSaveStatus::available) {
        source_boundary("field.save_load.slot_unavailable");
        return false;
    }
    QString error;
    if (!restore_game_save_snapshot(loaded.snapshot, &error)) {
        failure_ = error.toStdString();
        source_boundary("field.save_load.restore_failed");
        return false;
    }
    failure_.clear();
    source_boundary("field.save_load.state_restored");
    return true;
}

std::optional<QJsonObject> FieldSceneRuntime::capture_game_save_snapshot(
    QString* const error) {
    const auto run_registry = game_scripts_.save_run_registry_state();
    if (!run_registry.has_value()) {
        if (error != nullptr) {
            *error = QString::fromStdString(game_scripts_.failure());
        }
        return std::nullopt;
    }
    QJsonValue run_registry_json;
    if (!script_persistent_value_to_json(*run_registry, run_registry_json,
                                         error) ||
        !run_registry_json.isObject()) {
        if (error != nullptr && error->isEmpty()) {
            *error = QStringLiteral("The Lua run registry returned invalid save data.");
        }
        return std::nullopt;
    }
    QJsonObject scene;
    // FFFBF8 is not a separate saved field: 0x59372 stages a 0x800-byte
    // SRAM block at FFF1E8, outside FFFBF8; scene entry rebuilds FFFBF8 from
    // FFDDC4 at 0x2F78/0x308A [C]. Persist the word scene reference here.
    scene.insert(QStringLiteral("reference"),
                 state_.field_scene_buffers.scene_reference);
    scene.insert(QStringLiteral("mode"),
                 field_movement_transition_state_.world_mode);
    scene.insert(QStringLiteral("player_x"), field_entity_.world_position.x);
    scene.insert(QStringLiteral("player_y"), field_entity_.world_position.y);
    scene.insert(QStringLiteral("heading"), field_movement_state_.heading);
    scene.insert(QStringLiteral("direction"), field_entity_.direction);
    scene.insert(QStringLiteral("camera_x"),
                 state_.field_scene_buffers.camera_x);
    scene.insert(QStringLiteral("camera_y"),
                 state_.field_scene_buffers.camera_y);
    scene.insert(QStringLiteral("target_heading"), field_target_heading_);

    QJsonArray members;
    for (std::size_t slot = 0; slot < FieldPartyRosterRuntime::kSlotCount;
         ++slot) {
        QJsonObject member;
        member.insert(QStringLiteral("active"),
                      field_party_roster_.slot_active(slot));
        const auto& name = field_party_roster_.name(slot);
        member.insert(
            QStringLiteral("name"),
            QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size())));
        member.insert(QStringLiteral("portrait_family"),
                      field_party_roster_.portrait_asset_family(slot));
        member.insert(QStringLiteral("record"),
                      runner_record_to_json(
                          field_party_roster_.records()[slot]));
        members.append(member);
    }
    QJsonArray spell_tables;
    for (const auto& override_table : field_magic_spell_table_overrides_) {
        if (!override_table.has_value()) {
            spell_tables.append(QJsonValue(QJsonValue::Null));
            continue;
        }
        QJsonObject table;
        table.insert(QStringLiteral("entry_count"),
                     override_table->entry_count);
        QJsonArray entries;
        for (std::size_t index = 0; index < override_table->entry_count;
             ++index) {
            QJsonObject entry;
            entry.insert(QStringLiteral("code"),
                         override_table->entries[index].spell_code);
            entry.insert(QStringLiteral("force_limit"),
                         override_table->entries[index].force_limit);
            entries.append(entry);
        }
        table.insert(QStringLiteral("entries"), entries);
        spell_tables.append(table);
    }
    QJsonObject party;
    party.insert(QStringLiteral("active_slot"),
                 static_cast<int>(field_party_roster_.active_slot()));
    party.insert(QStringLiteral("primary_compact_marker"),
                 field_party_roster_.primary_compact_marker());
    party.insert(QStringLiteral("secondary_compact_marker"),
                 field_party_roster_.secondary_compact_marker());
    party.insert(QStringLiteral("members"), members);

    QJsonObject story;
    story.insert(QStringLiteral("exit_world_flags"),
                 static_cast<double>(field_exit_world_flags_));
    story.insert(QStringLiteral("mode_flags"),
                 field_mode_flags_resolved_
                     ? QJsonValue(static_cast<double>(field_mode_flags_))
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("mode_effect_flags"),
                 static_cast<double>(field_mode_effect_flags_));
    story.insert(QStringLiteral("auxiliary_flags"),
                 static_cast<double>(field_auxiliary_flags_));
    story.insert(QStringLiteral("render_effect_flags"),
                 static_cast<double>(field_render_effect_flags_));
    story.insert(QStringLiteral("dialogue_register"),
                 field_dialogue_register_.has_value()
                     ? QJsonValue(*field_dialogue_register_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("dialogue_flags"),
                 field_dialogue_flags_.has_value()
                     ? QJsonValue(*field_dialogue_flags_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("map_event_flags"),
                 static_cast<double>(field_map_event_state_flags_));
    story.insert(QStringLiteral("group_item_flags"),
                 group_item_flags_.has_value()
                     ? QJsonValue(static_cast<double>(*group_item_flags_))
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("cyberdeck_available"),
                 cyberdeck_available_.has_value()
                     ? QJsonValue(*cyberdeck_available_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("matrix_passcode_flags"),
                 field_matrix_passcode_flags_.has_value()
                     ? QJsonValue(*field_matrix_passcode_flags_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("matrix_field_state"),
                 matrix_field_state_to_json(field_matrix_field_state_));
    story.insert(QStringLiteral("karma_counter"),
                 field_matrix_karma_counter_.has_value()
                     ? QJsonValue(*field_matrix_karma_counter_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("passive_ic_counter"),
                 field_matrix_passive_ic_counter_.has_value()
                     ? QJsonValue(*field_matrix_passive_ic_counter_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("cyberdeck_record"),
                 field_matrix_deck_record_.has_value()
                     ? QJsonValue(cyberdeck_record_to_json(
                           *field_matrix_deck_record_))
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("runner_feature_flags"),
                 field_runner_feature_flags_.has_value()
                     ? QJsonValue(*field_runner_feature_flags_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("heat_level"),
                 field_heat_level_.has_value()
                     ? QJsonValue(*field_heat_level_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("strike_team_counter"),
                 field_strike_team_counter_.has_value()
                     ? QJsonValue(*field_strike_team_counter_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("walk_fast"), field_walk_fast_enabled_);
    story.insert(QStringLiteral("notebook_current_run_available"),
                 notebook_current_run_available_);
    story.insert(QStringLiteral("notebook_tips_resolved"),
                 notebook_tip_flags_resolved_);
    QJsonArray tip_flags;
    for (const auto value : notebook_tip_flags_) {
        tip_flags.append(static_cast<double>(value));
    }
    story.insert(QStringLiteral("notebook_tip_flags"), tip_flags);
    story.insert(QStringLiteral("notebook_contacts_flags"),
                 notebook_contacts_flags_.has_value()
                     ? QJsonValue(static_cast<double>(*notebook_contacts_flags_))
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("notebook_referral_flags"),
                 notebook_referral_flags_.has_value()
                     ? QJsonValue(*notebook_referral_flags_)
                     : QJsonValue(QJsonValue::Null));
    story.insert(QStringLiteral("notebook_shadowrunner_flags"),
                 notebook_shadowrunner_flags_.has_value()
                     ? QJsonValue(*notebook_shadowrunner_flags_)
                     : QJsonValue(QJsonValue::Null));
    if (field_entity_action_phase_roll_counters_.has_value()) {
        QJsonArray counters;
        for (const auto value : *field_entity_action_phase_roll_counters_) {
            counters.append(value);
        }
        story.insert(QStringLiteral("action_phase_roll_counters"), counters);
    } else {
        story.insert(QStringLiteral("action_phase_roll_counters"),
                     QJsonValue(QJsonValue::Null));
    }
    story.insert(QStringLiteral("world_frame_counter"),
                 field_world_frame_counter_);
    story.insert(QStringLiteral("world_encounter_sequence"),
                 static_cast<double>(field_world_encounter_sequence_));
    story.insert(QStringLiteral("source_random_seed"), field_random_seed_);

    QJsonObject script_facts;
    for (const auto& [name, value] : game_scripts_.story_state()) {
        script_facts.insert(
            QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size())),
            value);
    }

    QJsonObject snapshot;
    snapshot.insert(QStringLiteral("kind"), QStringLiteral("field_session"));
    snapshot.insert(QStringLiteral("scene"), scene);
    snapshot.insert(QStringLiteral("party"), party);
    snapshot.insert(QStringLiteral("spell_tables"), spell_tables);
    snapshot.insert(QStringLiteral("funds_available"),
                    field_funds_.has_value());
    snapshot.insert(QStringLiteral("funds"),
                    static_cast<double>(field_funds_.value_or(0U)));
    snapshot.insert(QStringLiteral("story"), story);
    snapshot.insert(QStringLiteral("script_facts"), script_facts);
    snapshot.insert(QStringLiteral("run_registry"), run_registry_json);
    if (error != nullptr) error->clear();
    return snapshot;
}

bool FieldSceneRuntime::restore_game_save_snapshot(
    const QJsonObject& snapshot, QString* const error) {
    struct RestoredState {
        std::uint16_t scene = 0U;
        std::uint16_t mode = 0U;
        SceneRunnerPosition player{};
        std::uint16_t heading = 0U;
        std::uint8_t direction = 0U;
        std::uint16_t camera_x = 0U;
        std::uint16_t camera_y = 0U;
        std::uint16_t target_heading = 0U;
        std::array<RunnerRecordFields, FieldPartyRosterRuntime::kSlotCount>
            records{};
        std::array<std::optional<MagicSpellTable>, kUsMagicSpellTableCount>
            spell_table_overrides{};
        std::array<std::string, FieldPartyRosterRuntime::kSlotCount> names{};
        std::array<bool, FieldPartyRosterRuntime::kSlotCount> active{};
        std::array<std::uint8_t, FieldPartyRosterRuntime::kSlotCount>
            portrait_families{};
        std::uint8_t primary_compact_marker = 0xFFU;
        std::uint8_t secondary_compact_marker = 0xFFU;
        std::size_t active_slot = 0U;
        bool funds_available = false;
        std::uint32_t funds = 0U;
        std::uint32_t exit_world_flags = 0U;
        std::optional<std::uint32_t> mode_flags;
        std::uint32_t mode_effect_flags = 0U;
        std::uint32_t auxiliary_flags = 0U;
        std::uint32_t render_effect_flags = 0U;
        std::optional<std::uint8_t> dialogue_register;
        std::optional<std::uint8_t> dialogue_flags;
        std::uint32_t map_event_flags = 0U;
        std::optional<std::uint32_t> group_item_flags;
        std::optional<bool> cyberdeck_available;
        std::optional<CyberdeckRecord> cyberdeck_record;
        std::optional<std::uint16_t> matrix_passcode_flags;
        scripting::ScriptMatrixFieldState matrix_field_state;
        std::optional<std::int8_t> karma_counter;
        std::optional<std::int8_t> passive_ic_counter;
        std::optional<std::uint8_t> runner_feature_flags;
        bool walk_fast = false;
        bool current_run_available = false;
        bool tips_resolved = false;
        std::array<std::uint32_t, 3> tip_flags{};
        std::optional<std::uint32_t> contacts_flags;
        std::optional<std::uint8_t> referral_flags;
        std::optional<std::uint16_t> shadowrunner_flags;
        std::optional<std::uint8_t> heat_level;
        std::optional<std::uint8_t> strike_team_counter;
        std::optional<std::array<std::uint8_t, 3>> action_phase_roll_counters;
        std::uint16_t world_frame_counter = 0U;
        std::uint32_t world_encounter_sequence = 0U;
        std::uint16_t source_random_seed = 0U;
        scripting::StoryFacts script_facts;
    } restored;

    const auto fail = [error](const QString& message) {
        if (error != nullptr) *error = message;
        return false;
    };
    if (snapshot.value(QStringLiteral("kind")).toString() !=
        QStringLiteral("field_session")) {
        return fail(QStringLiteral("The save does not contain a field session."));
    }

    const auto scene = snapshot.value(QStringLiteral("scene")).toObject();
    std::int64_t numeric = 0;
    if (!read_save_integer(scene, "reference", 0,
                           static_cast<std::int64_t>(kFieldSceneProfileCount - 1U),
                           numeric, error)) return false;
    restored.scene = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(scene, "mode", 1, 3, numeric, error)) return false;
    restored.mode = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(scene, "player_x", 0, 0xFFFF, numeric, error)) return false;
    restored.player.x = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(scene, "player_y", 0, 0xFFFF, numeric, error)) return false;
    restored.player.y = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(scene, "heading", 0, 0xFFFF, numeric, error)) return false;
    restored.heading = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(scene, "direction", 0, 7, numeric, error)) return false;
    restored.direction = static_cast<std::uint8_t>(numeric);
    if (!read_save_integer(scene, "camera_x", 0, 0xFFFF, numeric, error)) return false;
    restored.camera_x = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(scene, "camera_y", 0, 0xFFFF, numeric, error)) return false;
    restored.camera_y = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(scene, "target_heading", 0, 0xFFFF, numeric, error)) return false;
    restored.target_heading = static_cast<std::uint16_t>(numeric);
    const auto expected_mode = select_scene_interaction_mode(restored.scene);
    if (!expected_mode.has_value() || *expected_mode != restored.mode) {
        return fail(QStringLiteral("The save has an invalid map interaction mode."));
    }

    const auto party = snapshot.value(QStringLiteral("party")).toObject();
    if (!read_save_integer(party, "active_slot", 0,
                           static_cast<std::int64_t>(
                               FieldPartyRosterRuntime::kSlotCount - 1U),
                           numeric, error)) return false;
    restored.active_slot = static_cast<std::size_t>(numeric);
    // Early local snapshots did not retain the ROM's compact party markers.
    // Keep their absence explicit as the source sentinel rather than deriving
    // an index from the currently displayed field slots.
    if (party.contains(QStringLiteral("primary_compact_marker"))) {
        if (!read_save_integer(party, "primary_compact_marker", 0, 0xFF,
                               numeric, error)) return false;
        restored.primary_compact_marker = static_cast<std::uint8_t>(numeric);
    }
    if (party.contains(QStringLiteral("secondary_compact_marker"))) {
        if (!read_save_integer(party, "secondary_compact_marker", 0, 0xFF,
                               numeric, error)) return false;
        restored.secondary_compact_marker =
            static_cast<std::uint8_t>(numeric);
    }
    const auto members_value = party.value(QStringLiteral("members"));
    if (!members_value.isArray() ||
        members_value.toArray().size() !=
            static_cast<qsizetype>(FieldPartyRosterRuntime::kSlotCount)) {
        return fail(QStringLiteral("The save has an invalid party roster."));
    }
    const auto members = members_value.toArray();
    for (std::size_t slot = 0; slot < members.size(); ++slot) {
        const auto member = members.at(static_cast<qsizetype>(slot)).toObject();
        if (!read_save_boolean(member, "active", restored.active[slot], error)) {
            return false;
        }
        const auto name = member.value(QStringLiteral("name"));
        if (!name.isString() || name.toString().size() > 64) {
            return fail(QStringLiteral("The save has an invalid runner name."));
        }
        restored.names[slot] = name.toString().toStdString();
        if (!read_save_integer(member, "portrait_family", 0, 0xFF,
                               numeric, error)) return false;
        restored.portrait_families[slot] = static_cast<std::uint8_t>(numeric);
        const auto record = member.value(QStringLiteral("record"));
        if (!record.isObject() ||
            !runner_record_from_json(record.toObject(), restored.records[slot],
                                     error)) {
            return false;
        }
    }
    if (!restored.active[0] || !restored.active[restored.active_slot]) {
        return fail(QStringLiteral("The save does not have an active party leader."));
    }
    const auto spell_tables_value = snapshot.value(QStringLiteral("spell_tables"));
    if (!spell_tables_value.isUndefined()) {
        if (!spell_tables_value.isArray() ||
            spell_tables_value.toArray().size() !=
                static_cast<qsizetype>(kUsMagicSpellTableCount)) {
            return fail(QStringLiteral("The save has invalid spell-table state."));
        }
        const auto spell_tables = spell_tables_value.toArray();
        for (std::size_t table_index = 0;
             table_index < kUsMagicSpellTableCount; ++table_index) {
            const auto value = spell_tables.at(static_cast<qsizetype>(table_index));
            if (value.isNull()) {
                continue;
            }
            if (!value.isObject()) {
                return fail(QStringLiteral("The save has invalid spell-table state."));
            }
            const auto table_object = value.toObject();
            if (!read_save_integer(table_object, "entry_count", 0,
                                   static_cast<std::int64_t>(
                                       kMagicSpellTableEntryCapacity),
                                   numeric, error)) {
                return false;
            }
            MagicSpellTable table{};
            table.entry_count = static_cast<std::uint8_t>(numeric);
            const auto entries_value = table_object.value(QStringLiteral("entries"));
            if (!entries_value.isArray() ||
                entries_value.toArray().size() !=
                    static_cast<qsizetype>(table.entry_count)) {
                return fail(QStringLiteral("The save has invalid spell-table entries."));
            }
            const auto entries = entries_value.toArray();
            for (std::size_t entry_index = 0;
                 entry_index < table.entry_count; ++entry_index) {
                const auto entry_value = entries.at(
                    static_cast<qsizetype>(entry_index));
                if (!entry_value.isObject()) {
                    return fail(QStringLiteral("The save has an invalid spell-table entry."));
                }
                const auto entry_object = entry_value.toObject();
                if (!read_save_integer(entry_object, "code", 0,
                                       static_cast<std::int64_t>(
                                           kUsMagicSpellCount - 1U),
                                       numeric, error)) {
                    return false;
                }
                table.entries[entry_index].spell_code =
                    static_cast<std::uint8_t>(numeric);
                if (!read_save_integer(entry_object, "force_limit", 0,
                                       0xFF, numeric, error)) {
                    return false;
                }
                table.entries[entry_index].force_limit =
                    static_cast<std::uint8_t>(numeric);
            }
            restored.spell_table_overrides[table_index] = table;
        }
    }
    if (!read_save_boolean(snapshot, "funds_available",
                           restored.funds_available, error)) return false;
    if (!read_save_integer(snapshot, "funds", 0, 0xFFFFFFFFLL,
                           numeric, error)) return false;
    restored.funds = static_cast<std::uint32_t>(numeric);

    const auto story = snapshot.value(QStringLiteral("story")).toObject();
    if (!read_save_integer(story, "exit_world_flags", 0, 0xFFFFFFFFLL,
                           numeric, error)) return false;
    restored.exit_world_flags = static_cast<std::uint32_t>(numeric);
    if (!read_save_integer(story, "mode_effect_flags", 0, 0xFFFFFFFFLL,
                           numeric, error)) return false;
    restored.mode_effect_flags = static_cast<std::uint32_t>(numeric);
    // Older local snapshots predate the typed FFFC40 fact word.
    if (story.contains(QStringLiteral("auxiliary_flags"))) {
        if (!read_save_integer(story, "auxiliary_flags", 0, 0xFFFFFFFFLL,
                               numeric, error)) return false;
        restored.auxiliary_flags = static_cast<std::uint32_t>(numeric);
    }
    if (!read_save_integer(story, "render_effect_flags", 0, 0xFFFFFFFFLL,
                           numeric, error)) return false;
    restored.render_effect_flags = static_cast<std::uint32_t>(numeric);
    if (!read_save_integer(story, "map_event_flags", 0, 0xFFFFFFFFLL,
                           numeric, error)) return false;
    restored.map_event_flags = static_cast<std::uint32_t>(numeric);

    const auto read_optional_unsigned =
        [&story, &fail](const char* key, const std::int64_t maximum,
                        auto& output) {
            const auto value = story.value(QLatin1String(key));
            if (value.isUndefined() || value.isNull()) {
                output.reset();
                return true;
            }
            if (!value.isDouble()) {
                return fail(QStringLiteral("Save field '%1' is invalid.")
                                .arg(QLatin1String(key)));
            }
            const auto parsed = value.toDouble(-1.0);
            if (!std::isfinite(parsed) || std::trunc(parsed) != parsed ||
                parsed < 0.0 || parsed > static_cast<double>(maximum)) {
                return fail(QStringLiteral("Save field '%1' is out of range.")
                                .arg(QLatin1String(key)));
            }
            output = static_cast<std::decay_t<decltype(output.value())>>(parsed);
            return true;
        };
    if (!matrix_field_state_from_json(
            story.value(QStringLiteral("matrix_field_state")),
            restored.matrix_field_state, error) ||
        !read_optional_unsigned("dialogue_register", 0xFF,
                                restored.dialogue_register) ||
        !read_optional_unsigned("dialogue_flags", 0xFF,
                                restored.dialogue_flags) ||
        !read_optional_unsigned("mode_flags", 0xFFFFFFFFLL,
                                restored.mode_flags) ||
        !read_optional_unsigned("group_item_flags", 0xFFFFFFFFLL,
                                restored.group_item_flags) ||
        !read_optional_unsigned("matrix_passcode_flags", 0xFFFF,
                                restored.matrix_passcode_flags) ||
        !read_optional_unsigned("runner_feature_flags", 0xFF,
                                restored.runner_feature_flags) ||
        !read_optional_unsigned("notebook_contacts_flags", 0xFFFFFFFFLL,
                                restored.contacts_flags) ||
        !read_optional_unsigned("notebook_referral_flags", 0xFF,
                                restored.referral_flags) ||
        !read_optional_unsigned("notebook_shadowrunner_flags", 0xFFFF,
                                restored.shadowrunner_flags) ||
        !read_optional_unsigned("heat_level", 8,
                                restored.heat_level) ||
        !read_optional_unsigned("strike_team_counter", 8,
                                restored.strike_team_counter)) {
        return false;
    }
    const auto karma_counter = story.value(QStringLiteral("karma_counter"));
    if (karma_counter.isUndefined() || karma_counter.isNull()) {
        restored.karma_counter.reset();
    } else {
        if (!read_save_integer(story, "karma_counter", -128, 127,
                               numeric, error)) {
            return false;
        }
        restored.karma_counter = static_cast<std::int8_t>(numeric);
    }
    const auto passive_ic_counter =
        story.value(QStringLiteral("passive_ic_counter"));
    if (passive_ic_counter.isUndefined() || passive_ic_counter.isNull()) {
        restored.passive_ic_counter.reset();
    } else {
        if (!read_save_integer(story, "passive_ic_counter", -128, 127,
                               numeric, error)) {
            return false;
        }
        restored.passive_ic_counter = static_cast<std::int8_t>(numeric);
    }
    const auto cyberdeck = story.value(QStringLiteral("cyberdeck_available"));
    if (cyberdeck.isNull()) {
        restored.cyberdeck_available.reset();
    } else if (cyberdeck.isBool()) {
        restored.cyberdeck_available = cyberdeck.toBool();
    } else {
        return fail(QStringLiteral("The save has an invalid cyberdeck state."));
    }
    const auto cyberdeck_record =
        story.value(QStringLiteral("cyberdeck_record"));
    if (cyberdeck_record.isNull()) {
        restored.cyberdeck_record.reset();
    } else if (cyberdeck_record.isObject()) {
        CyberdeckRecord record;
        if (!cyberdeck_record_from_json(cyberdeck_record.toObject(), record,
                                        error)) {
            return false;
        }
        restored.cyberdeck_record = std::move(record);
    } else {
        return fail(QStringLiteral("The save has an invalid cyberdeck record."));
    }
    if (!read_save_boolean(story, "walk_fast", restored.walk_fast, error) ||
        !read_save_boolean(story, "notebook_current_run_available",
                           restored.current_run_available, error) ||
        !read_save_boolean(story, "notebook_tips_resolved",
                           restored.tips_resolved, error)) {
        return false;
    }
    const auto tip_flags = story.value(QStringLiteral("notebook_tip_flags"));
    if (!tip_flags.isArray() || tip_flags.toArray().size() != 3) {
        return fail(QStringLiteral("The save has invalid Notebook tip state."));
    }
    for (std::size_t index = 0; index < restored.tip_flags.size(); ++index) {
        const auto value = tip_flags.toArray().at(static_cast<qsizetype>(index));
        const auto parsed = value.toDouble(-1.0);
        if (!value.isDouble() || !std::isfinite(parsed) ||
            std::trunc(parsed) != parsed || parsed < 0.0 ||
            parsed > 0xFFFFFFFFU) {
            return fail(QStringLiteral("The save has invalid Notebook tip state."));
        }
        restored.tip_flags[index] = static_cast<std::uint32_t>(parsed);
    }
    const auto phase_counters =
        story.value(QStringLiteral("action_phase_roll_counters"));
    if (phase_counters.isNull()) {
        restored.action_phase_roll_counters.reset();
    } else {
        QJsonObject wrapper;
        wrapper.insert(QStringLiteral("values"), phase_counters);
        std::array<std::uint8_t, 3> values{};
        if (!read_save_byte_array(wrapper, "values", values, error)) return false;
        restored.action_phase_roll_counters = values;
    }
    if (!read_save_integer(story, "world_frame_counter", 0, 0xFFFF,
                           numeric, error)) return false;
    restored.world_frame_counter = static_cast<std::uint16_t>(numeric);
    if (!read_save_integer(story, "world_encounter_sequence", 0,
                           0xFFFFFFFFLL, numeric, error)) return false;
    restored.world_encounter_sequence = static_cast<std::uint32_t>(numeric);
    if (!read_save_integer(story, "source_random_seed", 0, 0xFFFF,
                           numeric, error)) return false;
    restored.source_random_seed = static_cast<std::uint16_t>(numeric);

    const auto script_facts_value = snapshot.value(QStringLiteral("script_facts"));
    if (!script_facts_value.isObject()) {
        return fail(QStringLiteral("The save has invalid script story facts."));
    }
    const auto script_facts = script_facts_value.toObject();
    if (script_facts.size() > 4096) {
        return fail(QStringLiteral("The save contains too many story facts."));
    }
    for (auto iterator = script_facts.begin(); iterator != script_facts.end();
         ++iterator) {
        if (!iterator.value().isBool()) {
            return fail(QStringLiteral("The save contains an invalid story fact."));
        }
        restored.script_facts.emplace(iterator.key().toStdString(),
                                       iterator.value().toBool());
    }

    scripting::ScriptPersistentValue run_registry_state;
    if (!script_persistent_value_from_json(
            snapshot.value(QStringLiteral("run_registry")),
            run_registry_state, error) ||
        !std::holds_alternative<scripting::ScriptPersistentValue::Object>(
            run_registry_state.value)) {
        if (error != nullptr && error->isEmpty()) {
            *error = QStringLiteral("The save has invalid Lua run state.");
        }
        return false;
    }
    if (!game_scripts_.validate_run_registry_state(run_registry_state)) {
        return fail(QString::fromStdString(game_scripts_.failure()));
    }

    if (!prepare_runtime_field_scene(restored.scene)) {
        return fail(QString::fromStdString(
            failure_.empty() ? "The saved map resources are unavailable."
                             : failure_));
    }
    if (!game_scripts_.restore_run_registry_state(run_registry_state)) {
        return fail(QString::fromStdString(game_scripts_.failure()));
    }
    if (!field_party_roster_.restore_saved_party(
            restored.records, restored.names, restored.active,
            restored.portrait_families, restored.primary_compact_marker,
            restored.secondary_compact_marker, restored.active_slot)) {
        return fail(QStringLiteral("The saved party roster is invalid."));
    }
    field_magic_spell_table_overrides_ = restored.spell_table_overrides;
    field_funds_ = restored.funds_available
                       ? std::optional<std::uint32_t>(restored.funds)
                       : std::nullopt;
    field_exit_world_flags_ = restored.exit_world_flags;
    field_mode_flags_resolved_ = restored.mode_flags.has_value();
    field_mode_flags_ = restored.mode_flags.value_or(1U);
    field_mode_effect_flags_ = restored.mode_effect_flags;
    field_auxiliary_flags_ = restored.auxiliary_flags;
    field_render_effect_flags_ = restored.render_effect_flags;
    field_dialogue_register_ = restored.dialogue_register;
    field_dialogue_flags_ = restored.dialogue_flags;
    field_map_event_state_flags_ = restored.map_event_flags;
    group_item_flags_ = restored.group_item_flags;
    cyberdeck_available_ = restored.cyberdeck_available;
    field_matrix_deck_record_ = restored.cyberdeck_record;
    field_matrix_passcode_flags_ = restored.matrix_passcode_flags;
    field_matrix_field_state_ = restored.matrix_field_state;
    field_matrix_karma_counter_ = restored.karma_counter;
    field_matrix_passive_ic_counter_ = restored.passive_ic_counter;
    field_runner_feature_flags_ = restored.runner_feature_flags;
    field_walk_fast_enabled_ = restored.walk_fast;
    notebook_current_run_available_ = restored.current_run_available;
    notebook_tip_flags_resolved_ = restored.tips_resolved;
    notebook_tip_flags_ = restored.tip_flags;
    notebook_contacts_flags_ = restored.contacts_flags;
    notebook_referral_flags_ = restored.referral_flags;
    notebook_shadowrunner_flags_ = restored.shadowrunner_flags;
    field_heat_level_ = restored.heat_level;
    field_strike_team_counter_ = restored.strike_team_counter;
    field_entity_action_phase_roll_counters_ =
        restored.action_phase_roll_counters;
    field_world_frame_counter_ = restored.world_frame_counter;
    field_world_encounter_sequence_ = restored.world_encounter_sequence;
    field_random_seed_ = restored.source_random_seed;
    game_scripts_.set_story_state(restored.script_facts);

    field_entity_.active = true;
    field_entity_.world_position = restored.player;
    field_entity_.direction = restored.direction;
    field_movement_state_.position_x = restored.player.x;
    field_movement_state_.position_y = restored.player.y;
    field_movement_state_.heading = restored.heading;
    field_movement_state_.animation_heading = restored.heading;
    field_target_heading_ = restored.target_heading;
    state_.field_scene_buffers.camera_x = restored.camera_x;
    state_.field_scene_buffers.camera_y = restored.camera_y;
    state_.field_scene_buffers.profile[kSceneProfileEntityPositionWord] =
        restored.player.x;
    state_.field_scene_buffers.profile[kSceneProfileEntityPositionWord + 1U] =
        restored.player.y;
    state_.field_scene_buffers.source_metadata.initial_field_records[0] =
        restored.player;
    state_.field_scene_entry.camera_x = restored.camera_x;
    state_.field_scene_entry.camera_y = restored.camera_y;
    field_camera_follow_.camera_x = restored.camera_x;
    field_camera_follow_.camera_y = restored.camera_y;
    field_camera_follow_.aligned_x = restored.camera_x;
    field_camera_follow_.aligned_y = restored.camera_y;
    field_camera_follow_.horizontal_step = 0;
    field_camera_follow_.vertical_step = 0;
    field_movement_transition_state_.scene_id = restored.scene;
    field_movement_transition_state_.world_mode = restored.mode;
    field_world_activity_timeout_state_ = WorldActivityTimeoutState{};
    field_world_activity_timeout_state_.scene_id = restored.scene;
    world_frame_logic_state_.world_mode = restored.scene;
    world_frame_logic_state_.world_active = 0U;
    world_frame_logic_state_.cleanup_countdown = 0;
    field_entity_selection_state_.active_index = restored.active_slot;
    field_entity_behavior_flags_ = static_cast<std::uint8_t>(
        active_runner_record().flags >> 8U);
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = false;
    field_player_animation_reset_pending_ = true;
    field_entity_animation_id_ = "idle";
    field_entity_animation_ = EntityAnimationState{};
    refresh_field_hud_runner_slot();
    refresh_command_overview_combat_bars();
    refresh_command_overview_item_detail();
    state_.display_dirty_flags = static_cast<std::uint8_t>(
        state_.display_dirty_flags | 0x0EU);
    field_runtime_state_.display_dirty_flags = static_cast<std::uint8_t>(
        field_runtime_state_.display_dirty_flags | 0x0EU);
    if (error != nullptr) error->clear();
    return true;
}


} // namespace shadowrun::ghidra_conversion::qt_runtime
