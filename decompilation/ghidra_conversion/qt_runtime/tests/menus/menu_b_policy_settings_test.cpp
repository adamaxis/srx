#include "menu_b_policy_settings.hpp"

#include <QSettings>
#include <QTemporaryDir>

#include <iostream>

namespace {

using namespace shadowrun::ghidra_conversion::qt_runtime;

bool check(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

} // namespace

int main() {
    QTemporaryDir directory;
    if (!check(directory.isValid(), "temporary settings directory failed")) {
        return 1;
    }
    const auto path = directory.filePath(QStringLiteral("settings.ini"));

    {
        QSettings old_settings(path, QSettings::IniFormat);
        if (!check(load_menu_b_policy_setting(old_settings) ==
                       kMenuBPolicyRom,
                   "missing B policy did not default to ROM")) {
            return 1;
        }
    }

    {
        QSettings settings(path, QSettings::IniFormat);
        store_menu_b_policy_setting(settings, kMenuBPolicyAlwaysCancel);
    }
    {
        QSettings reloaded(path, QSettings::IniFormat);
        if (!check(load_menu_b_policy_setting(reloaded) ==
                       kMenuBPolicyAlwaysCancel,
                   "ALWAYS_CANCEL did not persist")) {
            return 1;
        }
        reloaded.setValue(QStringLiteral("menu/b_policy"),
                          QStringLiteral("legacy_cancel"));
        reloaded.sync();
    }
    {
        QSettings old_value(path, QSettings::IniFormat);
        if (!check(load_menu_b_policy_setting(old_value) == kMenuBPolicyRom,
                   "unknown old B policy did not default to ROM")) {
            return 1;
        }
    }

    std::cout << "menu B policy settings: missing, persisted, and legacy values passed\n";
    return 0;
}
