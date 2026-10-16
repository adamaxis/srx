#pragma once

#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace shadowrun::ghidra_conversion::qt_runtime {

QJsonObject runner_record_to_json(const RunnerRecordFields& record);
bool runner_record_from_json(const QJsonObject& object,
                             RunnerRecordFields& record,
                             QString* error = nullptr);
QJsonObject matrix_field_state_to_json(
    const scripting::ScriptMatrixFieldState& state);
bool matrix_field_state_from_json(
    const QJsonValue& value, scripting::ScriptMatrixFieldState& state,
    QString* error = nullptr);
bool script_persistent_value_to_json(
    const scripting::ScriptPersistentValue& value, QJsonValue& output,
    QString* error = nullptr);
bool script_persistent_value_from_json(
    const QJsonValue& value, scripting::ScriptPersistentValue& output,
    QString* error = nullptr);

} // namespace shadowrun::ghidra_conversion::qt_runtime
