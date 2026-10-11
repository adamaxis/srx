#include "field_map_composition.hpp"

#include <QColor>
#include <QImage>
#include <QPainter>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

bool fail(const std::string& message) {
    std::cerr << "field map composition QA: " << message << '\n';
    return false;
}

bool check_rom_asset_set() {
    const std::filesystem::path root(SHADOWRUN_FIELD_MAP_ASSET_ROOT);
    std::ifstream manifest(root / "resources.tsv", std::ios::binary);
    if (!manifest) {
        return fail("cannot open the runtime resource manifest");
    }

    std::array<bool, 56> overlay_aliases{};
    std::string line;
    std::getline(manifest, line);
    while (std::getline(manifest, line)) {
        const auto first_tab = line.find('\t');
        const auto second_tab = line.find('\t', first_tab + 1U);
        if (first_tab == std::string::npos ||
            second_tab == std::string::npos) {
            continue;
        }
        const auto alias = line.substr(0U, first_tab);
        const auto path = line.substr(first_tab + 1U,
                                      second_tab - first_tab - 1U);
        for (std::size_t scene = 0U; scene < overlay_aliases.size(); ++scene) {
            const auto expected_alias =
                "field.maps.mode." + std::to_string(scene) +
                ".priority.overlay";
            const auto expected_path =
                "field/maps/mode-" + std::to_string(scene) +
                "-priority-overlay.png";
            if (alias == expected_alias && path == expected_path) {
                overlay_aliases[scene] = true;
            }
        }
    }

    for (std::size_t scene = 0U; scene < overlay_aliases.size(); ++scene) {
        if (!overlay_aliases[scene]) {
            return fail("scene " + std::to_string(scene) +
                        " is missing its priority-overlay manifest alias");
        }
        const auto stem = "mode-" + std::to_string(scene);
        const QImage map_source(QString::fromStdString(
            (root / "field/maps" / (stem + ".bmp")).string()));
        const QImage overlay_source(QString::fromStdString(
            (root / "field/maps" /
             (stem + "-priority-overlay.png")).string()));
        if (map_source.isNull() || overlay_source.isNull() ||
            map_source.size() != overlay_source.size()) {
            return fail("scene " + std::to_string(scene) +
                        " has missing or misaligned map layers");
        }
        const auto map = map_source.convertToFormat(QImage::Format_ARGB32);
        const auto overlay =
            overlay_source.convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < overlay.height(); ++y) {
            const auto* map_row =
                reinterpret_cast<const QRgb*>(map.constScanLine(y));
            const auto* overlay_row =
                reinterpret_cast<const QRgb*>(overlay.constScanLine(y));
            for (int x = 0; x < overlay.width(); ++x) {
                if (qAlpha(overlay_row[x]) != 0 &&
                    qRgb(qRed(map_row[x]), qGreen(map_row[x]), qBlue(map_row[x])) !=
                        qRgb(qRed(overlay_row[x]), qGreen(overlay_row[x]),
                             qBlue(overlay_row[x]))) {
                    return fail("scene " + std::to_string(scene) +
                                " has a priority pixel that is misaligned "
                                "with its flattened source map");
                }
            }
        }
    }

    const std::string stem = "mode-26";
    const QImage map(QString::fromStdString(
        (root / "field/maps" / (stem + ".bmp")).string()));
    const QImage overlay(QString::fromStdString(
        (root / "field/maps" /
         (stem + "-priority-overlay.png")).string()));
    std::size_t opaque_pixels = 0U;
    std::size_t transparent_pixels = 0U;
    for (int y = 0; y < overlay.height(); ++y) {
        for (int x = 0; x < overlay.width(); ++x) {
            const auto foreground = overlay.pixelColor(x, y);
            if (foreground.alpha() == 0) {
                ++transparent_pixels;
                continue;
            }
            ++opaque_pixels;
            const auto background = map.pixelColor(x, y);
            if (foreground.red() != background.red() ||
                foreground.green() != background.green() ||
                foreground.blue() != background.blue()) {
                return fail("scene-26 overlay pixel is not aligned with its ROM map");
            }
        }
    }
    if (opaque_pixels == 0U || transparent_pixels == 0U) {
        return fail("scene-26 overlay lost either priority pixels or transparency");
    }

    int opaque_x = -1;
    int opaque_y = -1;
    int transparent_x = -1;
    int transparent_y = -1;
    for (int y = 0; y < overlay.height(); ++y) {
        for (int x = 0; x < overlay.width(); ++x) {
            if (overlay.pixelColor(x, y).alpha() != 0 && opaque_x < 0) {
                opaque_x = x;
                opaque_y = y;
            } else if (overlay.pixelColor(x, y).alpha() == 0 &&
                       transparent_x < 0) {
                transparent_x = x;
                transparent_y = y;
            }
            if (opaque_x >= 0 && transparent_x >= 0) {
                break;
            }
        }
        if (opaque_x >= 0 && transparent_x >= 0) {
            break;
        }
    }
    QImage composed(map.size(), QImage::Format_ARGB32_Premultiplied);
    composed.fill(Qt::black);
    QPainter painter(&composed);
    shadowrun::ghidra_conversion::qt_runtime::paint_field_map_composition(
        painter, map, overlay, QRect(0, 0, map.width(), map.height()),
        [opaque_x, opaque_y, transparent_x, transparent_y](QPainter& target) {
            target.fillRect(QRect(opaque_x, opaque_y, 1, 1),
                            QColor(200, 30, 40));
            target.fillRect(QRect(transparent_x, transparent_y, 1, 1),
                            QColor(200, 30, 40));
        });
    painter.end();
    if (composed.pixelColor(opaque_x, opaque_y) !=
            map.pixelColor(opaque_x, opaque_y) ||
        composed.pixelColor(transparent_x, transparent_y) !=
            QColor(200, 30, 40)) {
        return fail("scene-26 table/actor occlusion does not follow ROM priority");
    }
    return true;
}

bool check_composition_order() {
    QImage map(6, 4, QImage::Format_ARGB32_Premultiplied);
    map.fill(QColor(20, 40, 80));
    QImage overlay(6, 4, QImage::Format_ARGB32_Premultiplied);
    overlay.fill(Qt::transparent);
    overlay.setPixelColor(2, 1, QColor(240, 200, 40));

    QImage output(6, 4, QImage::Format_ARGB32_Premultiplied);
    output.fill(Qt::black);
    QPainter painter(&output);
    shadowrun::ghidra_conversion::qt_runtime::paint_field_map_composition(
        painter, map, overlay, QRect(0, 0, 6, 4),
        [](QPainter& target) {
            target.fillRect(QRect(1, 1, 3, 2), QColor(200, 30, 40));
        });
    painter.end();

    if (output.pixelColor(0, 0) != QColor(20, 40, 80)) {
        return fail("the map background was not drawn first");
    }
    if (output.pixelColor(1, 1) != QColor(200, 30, 40) ||
        output.pixelColor(3, 2) != QColor(200, 30, 40)) {
        return fail("transparent priority pixels erased a field sprite");
    }
    if (output.pixelColor(2, 1) != QColor(240, 200, 40)) {
        return fail("high-priority map pixels did not cover the actor");
    }
    return true;
}

} // namespace

int main() {
    if (!check_rom_asset_set() || !check_composition_order()) {
        return 1;
    }
    std::cout << "field_map_composition_qa=passed scenes=56 "
                 "scene26_alpha_and_alignment=verified\n";
    return 0;
}
