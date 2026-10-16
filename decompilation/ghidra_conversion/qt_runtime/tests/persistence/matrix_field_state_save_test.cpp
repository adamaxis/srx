#include "game_save_snapshot.hpp"

#include <iostream>

namespace {

using shadowrun::ghidra_conversion::scripting::ScriptMatrixFieldState;
using shadowrun::ghidra_conversion::qt_runtime::matrix_field_state_from_json;
using shadowrun::ghidra_conversion::qt_runtime::matrix_field_state_to_json;

bool expect(const bool condition, const char* const message) {
    if (condition) return true;
    std::cerr << message << '\n';
    return false;
}

} // namespace

int main() {
    QString error;
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
        return 2;
    }

    decoded.alarm = true;
    if (!expect(matrix_field_state_from_json(QJsonValue{}, decoded, &error) &&
                    !decoded.alarm.has_value() &&
                    !decoded.security_off.has_value() &&
                    !decoded.maglocks_off.has_value(),
                "older saves without Matrix field state must load as unknown")) {
        return 3;
    }

    auto malformed = matrix_field_state_to_json(source);
    malformed.insert(QStringLiteral("maglocks_off"), QStringLiteral("off"));
    decoded.maglocks_off = true;
    if (!expect(!matrix_field_state_from_json(malformed, decoded, &error) &&
                    !error.isEmpty(),
                "malformed Matrix booleans must be rejected") ||
        !expect(decoded.maglocks_off == std::optional<bool>{true},
                "a rejected Matrix field object must not partially mutate state")) {
        return 4;
    }

    return 0;
}
