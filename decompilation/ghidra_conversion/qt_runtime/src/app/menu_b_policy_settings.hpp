#pragma once

#include <QSettings>
#include <QString>

#include <string>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline constexpr std::string_view kMenuBPolicyRom = "rom";
inline constexpr std::string_view kMenuBPolicyAlwaysCancel = "always_cancel";

inline std::string load_menu_b_policy_setting(QSettings& settings) {
    const auto value = settings.value(
        QStringLiteral("menu/b_policy"), QStringLiteral("rom")).toString();
    return value == QStringLiteral("always_cancel")
        ? std::string(kMenuBPolicyAlwaysCancel)
        : std::string(kMenuBPolicyRom);
}

inline void store_menu_b_policy_setting(QSettings& settings,
                                        const std::string_view policy) {
    const auto persisted = policy == kMenuBPolicyAlwaysCancel
        ? kMenuBPolicyAlwaysCancel : kMenuBPolicyRom;
    settings.setValue(QStringLiteral("menu/b_policy"),
                      QString::fromLatin1(
                          persisted.data(),
                          static_cast<int>(persisted.size())));
    settings.sync();
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
