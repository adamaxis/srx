#pragma once

#include <QJsonObject>
#include <QString>

#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class GameSaveSlot : std::uint8_t {
    one = 0U,
    two = 1U,
};

enum class GameSaveStatus : std::uint8_t {
    empty,
    available,
    corrupt,
    unsupported_version,
};

struct GameSaveLoadResult {
    GameSaveStatus status = GameSaveStatus::empty;
    QJsonObject snapshot;
    QString saved_at_utc;
    QString error;
};

// Host persistence for the Qt game. A snapshot is an application-level JSON
// object; it is deliberately independent of the Genesis SRAM record layout.
class GameSaveStore final {
public:
    static constexpr int kSchemaVersion = 2;

    explicit GameSaveStore(QString directory = {});

    const QString& directory() const noexcept { return directory_; }
    QString slot_path(GameSaveSlot slot) const;

    bool save(GameSaveSlot slot, const QJsonObject& snapshot,
              QString* error = nullptr) const;
    GameSaveLoadResult load(GameSaveSlot slot) const;
    GameSaveStatus status(GameSaveSlot slot) const;
    bool erase(GameSaveSlot slot, QString* error = nullptr) const;

private:
    QString directory_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
