#include "game_save_store.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <QStandardPaths>

#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr auto kFormatId = "shadowrun-qt-game-save";
constexpr qint64 kMaximumSaveBytes = 8LL * 1024LL * 1024LL;

QString slot_name(const GameSaveSlot slot) {
    switch (slot) {
    case GameSaveSlot::one:
        return QStringLiteral("slot-01.json");
    case GameSaveSlot::two:
        return QStringLiteral("slot-02.json");
    }
    return {};
}

QString read_error(const QFile& file) {
    return file.errorString().isEmpty()
               ? QStringLiteral("Could not read the save file.")
               : file.errorString();
}

} // namespace

GameSaveStore::GameSaveStore(QString directory)
    : directory_(std::move(directory)) {
    if (directory_.isEmpty()) {
        directory_ = QDir(QStandardPaths::writableLocation(
                              QStandardPaths::AppDataLocation))
                         .filePath(QStringLiteral("saves"));
    }
    directory_ = QDir::cleanPath(directory_);
}

QString GameSaveStore::slot_path(const GameSaveSlot slot) const {
    const auto name = slot_name(slot);
    return name.isEmpty() ? QString{} : QDir(directory_).filePath(name);
}

bool GameSaveStore::save(const GameSaveSlot slot,
                         const QJsonObject& snapshot,
                         QString* const error) const {
    const auto path = slot_path(slot);
    if (path.isEmpty()) {
        if (error != nullptr) {
            *error = QStringLiteral("Unknown save slot.");
        }
        return false;
    }
    if (snapshot.isEmpty()) {
        if (error != nullptr) {
            *error = QStringLiteral("The game state snapshot is empty.");
        }
        return false;
    }

    QJsonObject envelope;
    envelope.insert(QStringLiteral("format"), QString::fromLatin1(kFormatId));
    envelope.insert(QStringLiteral("schema_version"), kSchemaVersion);
    envelope.insert(QStringLiteral("saved_at_utc"),
                    QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    envelope.insert(QStringLiteral("snapshot"), snapshot);
    const auto bytes = QJsonDocument(envelope).toJson(QJsonDocument::Compact);
    if (bytes.size() > kMaximumSaveBytes) {
        if (error != nullptr) {
            *error = QStringLiteral("The game state snapshot is too large.");
        }
        return false;
    }

    if (!QDir().mkpath(directory_)) {
        if (error != nullptr) {
            *error = QStringLiteral("Could not create the save directory.");
        }
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error != nullptr) {
            *error = file.errorString();
        }
        return false;
    }
    if (file.write(bytes) != bytes.size()) {
        if (error != nullptr) {
            *error = file.errorString();
        }
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (error != nullptr) {
            *error = file.errorString();
        }
        return false;
    }
    if (error != nullptr) {
        error->clear();
    }
    return true;
}

GameSaveLoadResult GameSaveStore::load(const GameSaveSlot slot) const {
    GameSaveLoadResult result;
    const auto path = slot_path(slot);
    if (path.isEmpty()) {
        result.status = GameSaveStatus::corrupt;
        result.error = QStringLiteral("Unknown save slot.");
        return result;
    }

    QFile file(path);
    if (!file.exists()) {
        return result;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        result.status = GameSaveStatus::corrupt;
        result.error = read_error(file);
        return result;
    }
    if (file.size() <= 0 || file.size() > kMaximumSaveBytes) {
        result.status = GameSaveStatus::corrupt;
        result.error = QStringLiteral("The save file has an invalid size.");
        return result;
    }

    QJsonParseError parse_error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError ||
        !document.isObject()) {
        result.status = GameSaveStatus::corrupt;
        result.error = parse_error.errorString();
        return result;
    }

    const auto envelope = document.object();
    if (envelope.value(QStringLiteral("format")).toString() !=
        QString::fromLatin1(kFormatId)) {
        result.status = GameSaveStatus::corrupt;
        result.error = QStringLiteral("This is not a Shadowrun Qt save file.");
        return result;
    }
    if (envelope.value(QStringLiteral("schema_version")).toInt(-1) !=
        kSchemaVersion) {
        result.status = GameSaveStatus::unsupported_version;
        result.error = QStringLiteral("This save uses an unsupported version.");
        return result;
    }
    if (!envelope.value(QStringLiteral("snapshot")).isObject()) {
        result.status = GameSaveStatus::corrupt;
        result.error = QStringLiteral("The save has no game state snapshot.");
        return result;
    }

    result.status = GameSaveStatus::available;
    result.snapshot = envelope.value(QStringLiteral("snapshot")).toObject();
    result.saved_at_utc =
        envelope.value(QStringLiteral("saved_at_utc")).toString();
    return result;
}

GameSaveStatus GameSaveStore::status(const GameSaveSlot slot) const {
    return load(slot).status;
}

bool GameSaveStore::erase(const GameSaveSlot slot, QString* const error) const {
    const auto path = slot_path(slot);
    if (path.isEmpty()) {
        if (error != nullptr) {
            *error = QStringLiteral("Unknown save slot.");
        }
        return false;
    }
    if (!QFile::exists(path)) {
        if (error != nullptr) {
            error->clear();
        }
        return true;
    }
    if (!QFile::remove(path)) {
        if (error != nullptr) {
            *error = QStringLiteral("Could not remove the save file.");
        }
        return false;
    }
    if (error != nullptr) {
        error->clear();
    }
    return true;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
