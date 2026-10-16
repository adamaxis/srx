#include "game_save_snapshot.hpp"

#include <QJsonArray>
#include <QJsonValue>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <variant>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

template <typename Values>
QJsonArray byte_array(const Values& values) {
    QJsonArray result;
    for (const auto value : values) {
        result.append(static_cast<int>(value));
    }
    return result;
}

bool read_integer(const QJsonObject& object, const char* key,
                  const std::int64_t minimum, const std::int64_t maximum,
                  std::int64_t& output, QString* const error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isDouble()) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' is missing or invalid.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    const auto numeric = value.toDouble(std::numeric_limits<double>::quiet_NaN());
    if (!std::isfinite(numeric) || std::trunc(numeric) != numeric ||
        numeric < static_cast<double>(minimum) ||
        numeric > static_cast<double>(maximum)) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' is outside its valid range.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    output = static_cast<std::int64_t>(numeric);
    return true;
}

template <std::size_t Count>
bool read_byte_array(const QJsonObject& object, const char* key,
                     std::array<std::uint8_t, Count>& output,
                     QString* const error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isArray()) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' is missing or invalid.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    const auto array = value.toArray();
    if (static_cast<std::size_t>(array.size()) != Count) {
        if (error != nullptr) {
            *error = QStringLiteral("Save field '%1' has the wrong length.")
                         .arg(QLatin1String(key));
        }
        return false;
    }
    for (std::size_t index = 0; index < Count; ++index) {
        const auto number = array.at(static_cast<qsizetype>(index));
        if (!number.isDouble()) {
            if (error != nullptr) {
                *error = QStringLiteral("Save field '%1' has an invalid value.")
                             .arg(QLatin1String(key));
            }
            return false;
        }
        const auto parsed = number.toDouble(-1.0);
        if (!std::isfinite(parsed) || std::trunc(parsed) != parsed ||
            parsed < 0.0 || parsed > 255.0) {
            if (error != nullptr) {
                *error = QStringLiteral("Save field '%1' has an invalid byte.")
                             .arg(QLatin1String(key));
            }
            return false;
        }
        output[index] = static_cast<std::uint8_t>(parsed);
    }
    return true;
}

constexpr std::size_t kMaximumScriptSaveNodes = 10000U;
constexpr std::size_t kMaximumScriptSaveDepth = 32U;
constexpr std::size_t kMaximumScriptSaveStringBytes = 65536U;

bool persistent_to_json(const scripting::ScriptPersistentValue& input,
                        QJsonValue& output, std::size_t& nodes,
                        const std::size_t depth, QString* const error) {
    if (depth > kMaximumScriptSaveDepth || ++nodes > kMaximumScriptSaveNodes) {
        if (error != nullptr) {
            *error = QStringLiteral("Script save state exceeds its size limit.");
        }
        return false;
    }
    const auto& value = input.value;
    if (std::holds_alternative<std::monostate>(value)) {
        output = QJsonValue(QJsonValue::Null);
    } else if (const auto* boolean = std::get_if<bool>(&value)) {
        output = *boolean;
    } else if (const auto* integer = std::get_if<std::int64_t>(&value)) {
        constexpr std::int64_t kMaximumExactJsonInteger = 9007199254740991LL;
        if (*integer < -kMaximumExactJsonInteger ||
            *integer > kMaximumExactJsonInteger) {
            if (error != nullptr) {
                *error = QStringLiteral("Script save integer exceeds JSON precision.");
            }
            return false;
        }
        output = static_cast<double>(*integer);
    } else if (const auto* number = std::get_if<double>(&value)) {
        if (!std::isfinite(*number)) {
            if (error != nullptr) {
                *error = QStringLiteral("Script save state has a non-finite number.");
            }
            return false;
        }
        output = *number;
    } else if (const auto* string = std::get_if<std::string>(&value)) {
        if (string->size() > kMaximumScriptSaveStringBytes) {
            if (error != nullptr) {
                *error = QStringLiteral("Script save state has an oversized string.");
            }
            return false;
        }
        output = QString::fromUtf8(string->data(),
                                   static_cast<qsizetype>(string->size()));
    } else if (const auto* array =
                   std::get_if<scripting::ScriptPersistentValue::Array>(&value)) {
        QJsonArray result;
        for (const auto& child : *array) {
            if (std::holds_alternative<std::monostate>(child.value)) {
                if (error != nullptr) {
                    *error = QStringLiteral("Script save arrays cannot contain null values.");
                }
                return false;
            }
            QJsonValue item;
            if (!persistent_to_json(child, item, nodes, depth + 1U, error)) {
                return false;
            }
            result.append(item);
        }
        output = result;
    } else if (const auto* object =
                   std::get_if<scripting::ScriptPersistentValue::Object>(&value)) {
        QJsonObject result;
        for (const auto& [key, child] : *object) {
            if (key.empty() || key.size() > 256U) {
                if (error != nullptr) {
                    *error = QStringLiteral("Script save state has an invalid object key.");
                }
                return false;
            }
            QJsonValue item;
            if (!persistent_to_json(child, item, nodes, depth + 1U, error)) {
                return false;
            }
            result.insert(QString::fromUtf8(key.data(),
                                            static_cast<qsizetype>(key.size())),
                          item);
        }
        output = result;
    }
    return true;
}

bool persistent_from_json(const QJsonValue& input,
                          scripting::ScriptPersistentValue& output,
                          std::size_t& nodes, const std::size_t depth,
                          QString* const error) {
    if (depth > kMaximumScriptSaveDepth || ++nodes > kMaximumScriptSaveNodes) {
        if (error != nullptr) {
            *error = QStringLiteral("Script save state exceeds its size limit.");
        }
        return false;
    }
    if (input.isNull()) {
        output.value = std::monostate{};
    } else if (input.isBool()) {
        output.value = input.toBool();
    } else if (input.isDouble()) {
        const auto value = input.toDouble(std::numeric_limits<double>::quiet_NaN());
        if (!std::isfinite(value)) {
            if (error != nullptr) {
                *error = QStringLiteral("Script save state has a non-finite number.");
            }
            return false;
        }
        constexpr double kMaximumExactJsonInteger = 9007199254740991.0;
        if (std::trunc(value) == value &&
            value >= -kMaximumExactJsonInteger &&
            value <= kMaximumExactJsonInteger) {
            output.value = static_cast<std::int64_t>(value);
        } else {
            output.value = value;
        }
    } else if (input.isString()) {
        const auto value = input.toString().toUtf8();
        if (static_cast<std::size_t>(value.size()) >
            kMaximumScriptSaveStringBytes) {
            if (error != nullptr) {
                *error = QStringLiteral("Script save state has an oversized string.");
            }
            return false;
        }
        output.value = std::string(value.constData(),
                                   static_cast<std::size_t>(value.size()));
    } else if (input.isArray()) {
        scripting::ScriptPersistentValue::Array result;
        const auto array = input.toArray();
        result.reserve(static_cast<std::size_t>(array.size()));
        for (const auto& item : array) {
            scripting::ScriptPersistentValue child;
            if (item.isNull() ||
                !persistent_from_json(item, child, nodes, depth + 1U, error)) {
                if (item.isNull() && error != nullptr) {
                    *error = QStringLiteral("Script save arrays cannot contain null values.");
                }
                return false;
            }
            result.push_back(std::move(child));
        }
        output.value = std::move(result);
    } else if (input.isObject()) {
        scripting::ScriptPersistentValue::Object result;
        const auto object = input.toObject();
        for (auto iterator = object.begin(); iterator != object.end(); ++iterator) {
            const auto key = iterator.key().toUtf8();
            if (key.isEmpty() || key.size() > 256) {
                if (error != nullptr) {
                    *error = QStringLiteral("Script save state has an invalid object key.");
                }
                return false;
            }
            scripting::ScriptPersistentValue child;
            if (!persistent_from_json(iterator.value(), child, nodes,
                                      depth + 1U, error)) {
                return false;
            }
            result.emplace(std::string(key.constData(),
                                       static_cast<std::size_t>(key.size())),
                           std::move(child));
        }
        output.value = std::move(result);
    } else {
        if (error != nullptr) {
            *error = QStringLiteral("Script save state contains an invalid JSON value.");
        }
        return false;
    }
    return true;
}

} // namespace

QJsonObject runner_record_to_json(const RunnerRecordFields& record) {
    QJsonObject object;
#define SAVE_NUMBER(field) object.insert(QLatin1String(#field), static_cast<qint64>(record.field))
    SAVE_NUMBER(projection_word_08);
    SAVE_NUMBER(kind);
    SAVE_NUMBER(flags);
    SAVE_NUMBER(state_pointer);
    SAVE_NUMBER(state_selector);
    SAVE_NUMBER(timing_multiplier);
    SAVE_NUMBER(timing_accumulator);
    SAVE_NUMBER(timing_threshold);
    SAVE_NUMBER(state_index);
    SAVE_NUMBER(first_aid_continuation);
    SAVE_NUMBER(command_overview_panel_parameter);
    SAVE_NUMBER(record_type);
    SAVE_NUMBER(hud_portrait_table_pointer);
    SAVE_NUMBER(hud_portrait_table_offset);
    SAVE_NUMBER(race_index);
    SAVE_NUMBER(species_index);
    SAVE_NUMBER(archetype_index);
    SAVE_NUMBER(delay_meter);
    SAVE_NUMBER(behavior_meter);
    SAVE_NUMBER(magic_force);
    SAVE_NUMBER(action_result);
    SAVE_NUMBER(animation_action_gate);
    SAVE_NUMBER(equipped_inventory_slot);
    SAVE_NUMBER(equipped_item_code);
    SAVE_NUMBER(posture);
    SAVE_NUMBER(ammo_rounds);
    SAVE_NUMBER(ammo_clips);
    SAVE_NUMBER(equipped_armor_index);
    SAVE_NUMBER(cyberware_flags);
    SAVE_NUMBER(action_profile_modifier_flags);
    SAVE_NUMBER(magic_table_index);
#undef SAVE_NUMBER
    object.insert(QStringLiteral("inventory_item_codes"),
                  byte_array(record.inventory_item_codes));
    object.insert(QStringLiteral("inventory_phase_bytes"),
                  byte_array(record.inventory_phase_bytes));
    object.insert(QStringLiteral("attributes"), byte_array(record.attributes));
    object.insert(QStringLiteral("skills"), byte_array(record.skills));
    object.insert(QStringLiteral("auxiliary"), byte_array(record.auxiliary));
    return object;
}

bool runner_record_from_json(const QJsonObject& object,
                             RunnerRecordFields& record,
                             QString* const error) {
    RunnerRecordFields parsed{};
#define READ_NUMBER(field, type)                                               \
    do {                                                                       \
        std::int64_t value = 0;                                                \
        if (!read_integer(object, #field,                                     \
                          static_cast<std::int64_t>(                          \
                              std::numeric_limits<type>::lowest()),           \
                          static_cast<std::int64_t>(                          \
                              std::numeric_limits<type>::max()),              \
                          value, error)) {                                    \
            return false;                                                     \
        }                                                                      \
        parsed.field = static_cast<type>(value);                              \
    } while (false)
    READ_NUMBER(projection_word_08, std::uint16_t);
    READ_NUMBER(kind, std::uint16_t);
    READ_NUMBER(flags, std::uint16_t);
    READ_NUMBER(state_pointer, std::uint32_t);
    READ_NUMBER(state_selector, std::uint16_t);
    READ_NUMBER(timing_multiplier, std::uint16_t);
    READ_NUMBER(timing_accumulator, std::uint16_t);
    READ_NUMBER(timing_threshold, std::uint16_t);
    READ_NUMBER(state_index, std::uint16_t);
    READ_NUMBER(first_aid_continuation, std::uint32_t);
    READ_NUMBER(command_overview_panel_parameter, std::uint16_t);
    READ_NUMBER(record_type, std::uint16_t);
    READ_NUMBER(hud_portrait_table_pointer, std::uint32_t);
    READ_NUMBER(hud_portrait_table_offset, std::uint16_t);
    READ_NUMBER(race_index, std::uint8_t);
    READ_NUMBER(species_index, std::uint8_t);
    READ_NUMBER(archetype_index, std::uint8_t);
    READ_NUMBER(delay_meter, std::uint8_t);
    READ_NUMBER(behavior_meter, std::uint8_t);
    READ_NUMBER(magic_force, std::uint8_t);
    READ_NUMBER(action_result, std::int8_t);
    READ_NUMBER(animation_action_gate, std::int16_t);
    READ_NUMBER(equipped_inventory_slot, std::int8_t);
    READ_NUMBER(equipped_item_code, std::uint8_t);
    READ_NUMBER(posture, std::uint8_t);
    READ_NUMBER(ammo_rounds, std::uint8_t);
    READ_NUMBER(ammo_clips, std::uint8_t);
    READ_NUMBER(equipped_armor_index, std::uint8_t);
    READ_NUMBER(cyberware_flags, std::uint16_t);
    READ_NUMBER(action_profile_modifier_flags, std::uint8_t);
    READ_NUMBER(magic_table_index, std::uint16_t);
#undef READ_NUMBER
    if (!read_byte_array(object, "inventory_item_codes",
                         parsed.inventory_item_codes, error) ||
        !read_byte_array(object, "inventory_phase_bytes",
                         parsed.inventory_phase_bytes, error) ||
        !read_byte_array(object, "attributes", parsed.attributes, error) ||
        !read_byte_array(object, "skills", parsed.skills, error) ||
        !read_byte_array(object, "auxiliary", parsed.auxiliary, error)) {
        return false;
    }
    record = parsed;
    if (error != nullptr) {
        error->clear();
    }
    return true;
}

bool script_persistent_value_to_json(
    const scripting::ScriptPersistentValue& value, QJsonValue& output,
    QString* const error) {
    std::size_t nodes = 0U;
    if (!persistent_to_json(value, output, nodes, 0U, error)) return false;
    if (error != nullptr) error->clear();
    return true;
}

bool script_persistent_value_from_json(
    const QJsonValue& value, scripting::ScriptPersistentValue& output,
    QString* const error) {
    std::size_t nodes = 0U;
    if (!persistent_from_json(value, output, nodes, 0U, error)) return false;
    if (error != nullptr) error->clear();
    return true;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
