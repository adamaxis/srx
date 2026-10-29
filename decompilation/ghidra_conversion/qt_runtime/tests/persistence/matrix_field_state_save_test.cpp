#include "game_save_snapshot.hpp"

#include <QJsonArray>

#include <iostream>

namespace {

using shadowrun::ghidra_conversion::scripting::ScriptMatrixFieldState;
using shadowrun::ghidra_conversion::RunnerRecordFields;
using shadowrun::ghidra_conversion::qt_runtime::matrix_field_state_from_json;
using shadowrun::ghidra_conversion::qt_runtime::matrix_field_state_to_json;
using shadowrun::ghidra_conversion::qt_runtime::runner_record_from_json;
using shadowrun::ghidra_conversion::qt_runtime::runner_record_to_json;

bool expect(const bool condition, const char* const message) {
    if (condition) return true;
    std::cerr << message << '\n';
    return false;
}

} // namespace

int main() {
    QString error;

    // ROM 0x59C34 awards Karma at runner-record +0x8D. New saves represent
    // it by name; the old four-byte auxiliary layout remains readable.
    RunnerRecordFields runner;
    runner.karma = 17U;
    runner.auxiliary = {7U, 8U, 9U};
    const auto runner_json = runner_record_to_json(runner);
    RunnerRecordFields restored_runner;
    if (!expect(runner_record_from_json(runner_json, restored_runner, &error),
                "named Karma runner record must load") ||
        !expect(restored_runner.karma == 17U &&
                    restored_runner.auxiliary == runner.auxiliary,
                "named Karma and remaining runner values must round-trip")) {
        return 1;
    }

    auto legacy_runner_json = runner_json;
    legacy_runner_json.remove(QStringLiteral("karma"));
    const auto new_auxiliary = runner_json.value(QStringLiteral("auxiliary"))
                                   .toArray();
    QJsonArray legacy_auxiliary;
    legacy_auxiliary.append(new_auxiliary[0]);
    legacy_auxiliary.append(new_auxiliary[1]);
    legacy_auxiliary.append(runner.karma);
    legacy_auxiliary.append(new_auxiliary[2]);
    legacy_runner_json.insert(QStringLiteral("auxiliary"), legacy_auxiliary);
    restored_runner = RunnerRecordFields{};
    if (!expect(runner_record_from_json(legacy_runner_json, restored_runner,
                                        &error),
                "legacy runner auxiliary layout must load") ||
        !expect(restored_runner.karma == 17U &&
                    restored_runner.auxiliary == runner.auxiliary,
                "legacy auxiliary Karma must migrate to the named field")) {
        return 2;
    }

    ScriptMatrixFieldState source;
    source.alarm = true;
    source.security_off = false;
    source.maglocks_off = true;

    const auto encoded = matrix_field_state_to_json(source);
    ScriptMatrixFieldState decoded;
    if (!expect(matrix_field_state_from_json(encoded, decoded, &error),
                "complete Matrix field state must load") ||
        !expect(decoded.alarm == source.alarm &&
                    decoded.security_off == source.security_off &&
                    decoded.maglocks_off == source.maglocks_off,
                "Matrix alarm/security/maglock values must round-trip")) {
        return 1;
    }

    source.alarm.reset();
    source.security_off = false;
    source.maglocks_off.reset();
    const auto partial = matrix_field_state_to_json(source);
    if (!expect(matrix_field_state_from_json(partial, decoded, &error),
                "partially known Matrix field state must load") ||
        !expect(!decoded.alarm.has_value() &&
                    decoded.security_off == std::optional<bool>{false} &&
                    !decoded.maglocks_off.has_value(),
                "unknown Matrix field values must remain unknown after saving")) {
        return 3;
    }

    decoded.alarm = true;
    if (!expect(matrix_field_state_from_json(QJsonValue{}, decoded, &error) &&
                    !decoded.alarm.has_value() &&
                    !decoded.security_off.has_value() &&
                    !decoded.maglocks_off.has_value(),
                "older saves without Matrix field state must load as unknown")) {
        return 4;
    }

    auto malformed = matrix_field_state_to_json(source);
    malformed.insert(QStringLiteral("maglocks_off"), QStringLiteral("off"));
    decoded.maglocks_off = true;
    if (!expect(!matrix_field_state_from_json(malformed, decoded, &error) &&
                    !error.isEmpty(),
                "malformed Matrix booleans must be rejected") ||
        !expect(decoded.maglocks_off == std::optional<bool>{true},
                "a rejected Matrix field object must not partially mutate state")) {
        return 5;
    }

    return 0;
}
