#ifdef _WIN32

#include "shadowrun/rendering/graphics.hpp"
#include "shadowrun/field/native_port.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QFocusEvent>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include <QWidget>

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <string>
#include <utility>
#include <vector>

namespace {

using shadowrun::native::Button;
using shadowrun::native::InputState;
using shadowrun::native::NativePortAssets;
using shadowrun::native::NativePortShell;
using shadowrun::native::SoftwareScreen;
using shadowrun::native::NativeImage;

QImage to_qimage(const NativeImage& source) {
    if (source.width == 0U || source.height == 0U || source.pixels.empty()) {
        return {};
    }
    QImage image(source.width, source.height, QImage::Format_ARGB32);
    auto* destination = reinterpret_cast<QRgb*>(image.bits());
    for (std::size_t index = 0; index < source.pixels.size(); ++index) {
        const auto& pixel = source.pixels[index];
        destination[index] = qRgba(pixel.red, pixel.green, pixel.blue,
                                   pixel.alpha);
    }
    return image;
}

std::optional<std::filesystem::path> find_asset_root(int argc, char** argv) {
    if (argc > 1) {
        const std::filesystem::path explicit_root(argv[1]);
        if (!std::filesystem::is_directory(explicit_root)) {
            throw std::invalid_argument(
                "shadowrun_qt asset argument is not a directory");
        }
        return explicit_root;
    }

    const auto application =
        std::filesystem::path(QCoreApplication::applicationDirPath().toStdWString());
    const auto current = std::filesystem::current_path();
    const std::filesystem::path candidates[] = {
        application / "assets",
        application / ".." / "assets",
        current / "assets",
        current / ".." / "assets",
    };
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_directory(candidate) &&
            std::filesystem::is_regular_file(candidate / "native" / "state.bin")) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::uint16_t key_mask(int key) {
    switch (key) {
    case Qt::Key_Up: return shadowrun::native::kGenesisInputUp;
    case Qt::Key_Down: return shadowrun::native::kGenesisInputDown;
    case Qt::Key_Left: return shadowrun::native::kGenesisInputLeft;
    case Qt::Key_Right: return shadowrun::native::kGenesisInputRight;
    case Qt::Key_Z: return shadowrun::native::kGenesisInputA;
    case Qt::Key_X: return shadowrun::native::kGenesisInputB;
    case Qt::Key_C: return shadowrun::native::kGenesisInputC;
    case Qt::Key_Return:
    case Qt::Key_Enter: return shadowrun::native::kGenesisInputStart;
    default: return 0;
    }
}

class ShadowrunWidget final : public QWidget {
public:
    explicit ShadowrunWidget(NativePortShell game, QWidget* parent = nullptr)
        : QWidget(parent), game_(std::move(game)), screen_() {
        setWindowTitle(QStringLiteral("Shadowrun - Native C++ Port (Qt)"));
        setFocusPolicy(Qt::StrongFocus);
        setMinimumSize(640, 448);
        resize(960, 672);

        // The native window presents at 60 Hz.  The Genesis opening script
        // advances its visible presentation at the original 30 Hz cadence;
        // tick() holds each boot/title/story frame for two host paints.
        timer_.setInterval(1000 / 60);
        timer_.setTimerType(Qt::PreciseTimer);
        connect(&timer_, &QTimer::timeout, this, [this]() { tick(); });
        timer_.start();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        const auto target = QSize(screen_.width(), screen_.height())
                                .scaled(size(), Qt::KeepAspectRatio);
        const auto x = (width() - target.width()) / 2;
        const auto y = (height() - target.height()) / 2;
        painter.save();
        painter.translate(x, y);
        painter.scale(static_cast<qreal>(target.width()) / screen_.width(),
                      static_cast<qreal>(target.height()) / screen_.height());
        // NativePortShell owns the complete reconstructed frame for every
        // scene, including the field menu. Qt is only the focused window and
        // presentation blitter; keeping a second field-menu compositor here
        // drops source icons, selectors, descriptions, and child-window state.
        paint_software_frame(painter);
        painter.restore();
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->isAutoRepeat() || !hasFocus()) {
            return;
        }
        held_ = static_cast<std::uint16_t>(held_ | key_mask(event->key()));
        event->accept();
    }

    void keyReleaseEvent(QKeyEvent* event) override {
        if (event->isAutoRepeat() || !hasFocus()) {
            return;
        }
        held_ = static_cast<std::uint16_t>(
            held_ & static_cast<std::uint16_t>(~key_mask(event->key())));
        event->accept();
    }

    void focusOutEvent(QFocusEvent* event) override {
        // A native Windows game must not keep consuming a key after another
        // window becomes active.  Clearing both levels also prevents a key
        // held during the focus transition from becoming a stuck input.
        held_ = 0;
        previous_ = 0;
        QWidget::focusOutEvent(event);
    }

private:
    const QImage& image_for(const NativeImage& source) const {
        const auto key = &source;
        const auto existing = image_cache_.find(key);
        if (existing != image_cache_.end()) {
            return existing->second;
        }
        return image_cache_.emplace(key, to_qimage(source)).first->second;
    }

    void draw_image(QPainter& painter, const NativeImage& image,
                    int x, int y) const {
        if (image.width != 0U && image.height != 0U && !image.pixels.empty()) {
            painter.drawImage(x, y, image_for(image));
        }
    }

    void paint_software_frame(QPainter& painter) const {
        QImage frame(screen_.width(), screen_.height(), QImage::Format_RGB32);
        auto* destination = reinterpret_cast<QRgb*>(frame.bits());
        const auto& source = screen_.pixels();
        for (std::size_t index = 0; index < source.size(); ++index) {
            const auto packed = source[index];
            destination[index] = qRgb(
                static_cast<int>((packed >> 16U) & 0xFFU),
                static_cast<int>((packed >> 8U) & 0xFFU),
                static_cast<int>(packed & 0xFFU));
        }
        painter.drawImage(0, 0, frame);
    }

    void draw_field_text(QPainter& painter, int x, int y,
                         std::string_view text) const {
        const auto& font = game_.field_menu_font_asset();
        if (font.width < 8U || font.height < 8U || font.pixels.empty()) {
            return;
        }
        const auto& atlas = image_for(font);
        auto cursor = x;
        for (const auto character : text) {
            const auto code = static_cast<unsigned char>(character);
            if (code < 0x20U || code > 0x7FU) {
                cursor += 8;
                continue;
            }
            const auto glyph = static_cast<std::size_t>(code - 0x20U);
            const auto source_x = static_cast<int>((glyph % 16U) * 8U);
            const auto source_y = static_cast<int>((glyph / 16U) * 8U);
            if (source_y + 8 <= atlas.height()) {
                painter.drawImage(cursor, y, atlas,
                                  source_x, source_y, 8, 8);
            }
            cursor += 8;
        }
    }

    static std::string centered_field_text(std::string_view text) {
        constexpr std::size_t width = 12U;
        if (text.size() >= width) {
            return std::string(text);
        }
        return std::string((width - text.size()) / 2U, ' ') +
               std::string(text);
    }

    void paint_field_menu(QPainter& painter) const {
        const auto& entity = game_.field_menu_state();
        draw_image(painter, game_.field_menu_background_asset(), 0, 0);
        draw_image(painter, game_.field_menu_inventory_boxes_asset(), 0, 0);

        const auto portrait_index = game_.field_active_record_index() == 0U
            ? game_.selected_archetype() : game_.field_active_record_index();
        draw_image(painter, game_.field_menu_runner_portrait_asset(portrait_index),
                   16, 16);
        draw_field_text(painter, 128, 8,
                        game_.field_menu_runner_name(
                            game_.field_active_record_index()));

        const auto status_index = std::min<std::size_t>(
            entity.menu_status_index, shadowrun::native::kNativeMenuStatusCount - 1U);
        draw_image(painter, game_.field_menu_status_bar_asset(status_index),
                   176, 168);
        draw_field_text(painter, 136, 176,
                        game_.field_menu_status_name(status_index));

        constexpr std::array<std::array<int, 2>,
                             shadowrun::native::kNativeInventorySlotCount>
            icon_positions{{{{16, 120}}, {{56, 120}}, {{16, 144}}, {{56, 144}},
                         {{16, 168}}, {{56, 168}}, {{16, 192}}, {{56, 192}}}};
        const auto palette = entity.display_substate != 0 &&
                             entity.display_palette_state >= 2U ? 1U : 0U;
        for (std::size_t slot = 0; slot < entity.inventory_item_codes.size();
             ++slot) {
            draw_image(painter,
                       game_.field_menu_inventory_icon_asset(
                           entity.inventory_item_codes[slot], palette),
                       icon_positions[slot][0], icon_positions[slot][1]);
        }

        const auto draw_equipped_border = [&](std::size_t slot) {
            if (slot >= entity.inventory_item_codes.size()) {
                return;
            }
            draw_image(painter, game_.field_menu_selected_cell_asset(
                                   ((slot / 2U) & 1U) == 0U ? 0U : 1U),
                       8 + static_cast<int>((slot % 2U) * 40U),
                       112 + static_cast<int>((slot / 2U) * 24U));
        };
        if (entity.display_state >= 0 && entity.display_state != 0x7F) {
            draw_equipped_border(static_cast<std::size_t>(entity.display_state));
        }
        if (entity.equipped_armor_index != 0U) {
            const auto armor_code = static_cast<std::uint8_t>(
                0x2BU + entity.equipped_armor_index);
            for (std::size_t slot = 0; slot < entity.inventory_item_codes.size();
                 ++slot) {
                if (entity.inventory_item_codes[slot] == armor_code) {
                    draw_equipped_border(slot);
                    break;
                }
            }
        }

        if (game_.field_menu_selection() >= 2U) {
            draw_field_text(painter, 128, 24,
                            centered_field_text(game_.field_menu_class_label()));
            draw_field_text(painter, 128, 40, "Physical");
            draw_field_text(painter, 128, 48, "Mental");
            draw_field_text(painter, 208, 40,
                            std::to_string(std::min<unsigned>(
                                100U, static_cast<unsigned>(entity.physical_value) * 10U)) + "%");
            draw_field_text(painter, 208, 48,
                            std::to_string(std::min<unsigned>(
                                100U, static_cast<unsigned>(entity.mental_value) * 10U)) + "%");
            draw_field_item_detail(painter, entity.active_item_code, false);
            draw_field_text(painter, 112, 104, "Atk");
            draw_field_text(painter, 112, 112, "Def");
            draw_image(painter, game_.field_menu_attack_bar_asset(entity.attack_bar_index),
                       144, 104);
            draw_image(painter, game_.field_menu_defense_bar_asset(entity.defense_bar_index),
                       144, 112);
            if (entity.equipped_armor_index != 0U) {
                if (const auto* armor = game_.field_menu_armor_label(
                        entity.equipped_armor_index - 1U)) {
                    for (std::size_t row = 0; row < armor->size(); ++row) {
                        draw_field_text(painter, 128, 128 + static_cast<int>(row * 8U),
                                        (*armor)[row]);
                    }
                }
            } else {
                draw_field_text(painter, 152, 136, "No Armor");
            }
            draw_field_text(painter, 136, 144, "Wpns");
            draw_field_text(painter, 136, 152, "Guns");
            draw_image(painter, game_.field_menu_damage_bar_asset(
                                   entity.weapons_bar_index), 176, 144);
            draw_image(painter, game_.field_menu_drain_bar_asset(
                                   entity.guns_bar_index), 176, 152);
        }

        if (game_.field_menu_selection() < 2U) {
            const auto slot = game_.field_menu_row() * 2U +
                              game_.field_menu_selection();
            const auto item_code = slot < entity.inventory_item_codes.size()
                ? entity.inventory_item_codes[slot] : 0U;
            draw_image(painter, game_.field_menu_cursor_asset(),
                       8 + static_cast<int>(game_.field_menu_selection() * 40U),
                       112 + static_cast<int>(game_.field_menu_row() * 24U));
            draw_field_item_detail(painter, item_code, true);
        } else if (game_.field_menu_posture_active()) {
            draw_image(painter, game_.field_menu_posture_focus_asset(), 120, 168);
        } else if (game_.field_menu_selection() <= 6U) {
            // The extracted focus images carry the original bevel/highlight;
            // the Qt view only chooses their native destination.
            draw_image(painter, game_.field_menu_command_focus_asset(
                                   game_.field_menu_selection() - 2U),
                       128 + static_cast<int>((game_.field_menu_selection() - 2U) * 24U),
                       184);
        }

        paint_field_hud(painter);
    }

    void draw_field_item_detail(QPainter& painter, std::uint8_t item_code,
                                bool include_art) const {
        const auto& entity = game_.field_menu_state();
        if (item_code == 0U || item_code > shadowrun::native::kNativeInventoryItemCount) {
            if (include_art) {
                draw_field_text(painter, 128, 64,
                                centered_field_text("This slot is"));
                draw_field_text(painter, 128, 72,
                                centered_field_text("empty"));
            } else if (entity.display_state == 0x7FU &&
                       entity.active_item_code == 0U) {
                draw_field_text(painter, 128, 64,
                                centered_field_text("Empty"));
                draw_field_text(painter, 128, 72,
                                centered_field_text("Hand"));
                draw_field_text(painter, 112, 80, "Damage");
                draw_field_text(painter, 208, 80, "Stun");
                draw_field_text(painter, 112, 88, "Power");
                draw_field_text(painter, 168, 88, "Minor");
            }
            return;
        }
        if (include_art) {
            draw_image(painter, game_.field_menu_item_art_asset(item_code), 144, 24);
        }
        const auto* item = game_.field_menu_item(item_code);
        if (!item) {
            return;
        }
        draw_field_text(painter, 128, 64, centered_field_text(item->name));
        draw_field_text(painter, 128, 72, centered_field_text(item->category));
        if (item_code < 0x10U) {
            draw_field_text(painter, 112, 80, "Damage");
            draw_image(painter, game_.field_menu_damage_bar_asset(
                                   item->damage_table_index), 168, 80);
            if (item->power_table_index != 0U) {
                draw_field_text(painter, 112, 88, "Power");
                draw_field_text(painter, 168, 88,
                                game_.field_menu_power_label(
                                    item->power_table_index - 1U));
            }
        } else if (item_code >= 0x2CU && item_code <= 0x32U) {
            draw_field_text(painter, 112, 80, "Wpns");
            draw_image(painter, game_.field_menu_damage_bar_asset(
                                   entity.weapons_bar_index), 168, 80);
            draw_field_text(painter, 112, 88, "Guns");
            draw_image(painter, game_.field_menu_drain_bar_asset(
                                   entity.guns_bar_index), 168, 88);
        }
    }

    void paint_field_hud(QPainter& painter) const {
        const auto& entity = game_.field_menu_state();
        draw_image(painter, game_.field_hud_asset(), 0, 0);
        const auto slot = game_.field_active_record_index() < 3U
            ? game_.field_active_record_index() : 0U;
        const auto data = slot == 0U
            ? std::min<std::size_t>(game_.selected_archetype(), 2U) : slot;
        const auto offset = static_cast<int>(slot * 64U);
        draw_image(painter, game_.field_hud_face_asset(data), 280, 24 + offset);
        draw_image(painter, game_.field_hud_vertical_counter_asset(data), 272,
                   24 + offset);
        draw_image(painter, game_.field_hud_horizontal_counter_asset(data), 280,
                   56 + offset);
        draw_image(painter, game_.field_hud_ammo_counter_asset(entity.ammo_rounds),
                   272, 64 + offset);
        painter.fillRect(288, 64 + offset, 24, 16, Qt::black);
        if (entity.display_state >= 0 && entity.display_state != 0x7F &&
            entity.active_item_code != 0U) {
            const auto palette = entity.display_substate != 0 &&
                                 entity.display_palette_state >= 2U ? 1U : 0U;
            draw_image(painter, game_.field_menu_inventory_icon_asset(
                                   entity.active_item_code, palette), 288, 64 + offset);
        }
    }

    void tick() {
        if (!hasFocus()) {
            held_ = 0;
            previous_ = 0;
        }
        const auto opening_frame_hold =
            game_.scene() == shadowrun::native::NativeScene::Bootstrap ||
            game_.scene() == shadowrun::native::NativeScene::Title ||
            game_.scene() == shadowrun::native::NativeScene::Intro;
        if (opening_frame_hold && opening_host_phase_ != 0U) {
            // Do not advance the game clock on the second 60 Hz paint.  Keep
            // the sampled input unchanged so a held button is delivered on
            // the next game update rather than lost between host ticks.
            opening_host_phase_ = 0;
            game_.render(screen_);
            update();
            return;
        }
        const auto pressed = static_cast<std::uint16_t>(
            held_ & static_cast<std::uint16_t>(~previous_));
        game_.update(shadowrun::native::decode_genesis_controller_word(
            static_cast<std::uint16_t>(held_ | (pressed << 8U))));
        previous_ = held_;
        const auto still_opening =
            game_.scene() == shadowrun::native::NativeScene::Bootstrap ||
            game_.scene() == shadowrun::native::NativeScene::Title ||
            game_.scene() == shadowrun::native::NativeScene::Intro;
        opening_host_phase_ = still_opening ? 1U : 0U;
        game_.render(screen_);
        update();
    }

    NativePortShell game_;
    SoftwareScreen screen_;
    QTimer timer_;
    std::uint16_t held_ = 0;
    std::uint16_t previous_ = 0;
    std::uint8_t opening_host_phase_ = 0;
    mutable std::map<const NativeImage*, QImage> image_cache_;
};

} // namespace

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    try {
        const auto asset_root = find_asset_root(argc, argv);
        if (!asset_root) {
            throw std::runtime_error(
                "No direct extracted assets found. Run "
                "shadowrun_graphics_extract <rom.bin> <asset-dir> --all-assets.");
        }
        ShadowrunWidget window(
            NativePortShell("SHADOWRUN",
                            shadowrun::graphics::read_native_asset_directory(
                                asset_root->string())));
        window.show();
        return application.exec();
    } catch (const std::exception& error) {
        qCritical("%s", error.what());
        return 1;
    }
}

#else

int main() { return 0; }

#endif
