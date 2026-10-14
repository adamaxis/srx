QImage render_qa_field(FieldSceneRuntime& field_scene,
                       const bool scene_only = false,
                       const QImage::Format format = QImage::Format_RGB32) {
    QImage image(kSceneWidth, kSceneHeight, format);
    image.fill(Qt::black);
    QPainter painter(&image);
    if (scene_only) {
        field_scene.paint_field_scene_for_dump(painter);
    } else {
        field_scene.paint(painter);
    }
    painter.end();
    return image;
}

bool save_qa_image(const std::filesystem::path& output_directory,
                   const std::filesystem::path& relative_path,
                   const QImage& image) {
    return image.save(QString::fromStdWString(
        (output_directory / relative_path).wstring()));
}

bool finish_field_modal_return_transition(FieldSceneRuntime& field_scene) {
    auto remaining =
        field_scene.field_modal_return_transition_frames_for_dump();
    if (remaining > 60U) {
        return false;
    }
    while (remaining != 0U) {
        field_scene.set_controller_port_byte(0U);
        field_scene.advance_frame();
        remaining =
            field_scene.field_modal_return_transition_frames_for_dump();
    }
    return field_scene.field_palette_fade_steps_for_dump() == 0U;
}

bool controller_event_contains_a(const std::uint16_t events) noexcept {
    // The converted scene tests normally observe A's rising edge in the
    // high byte. Some modal return paths expose the held low-byte sample
    // instead; their closed-panel state is checked separately by each route.
    return (events & 0x4000U) != 0U || (events & 0x0040U) != 0U;
}

bool image_has_green_ink(const QImage& image, const int x, const int y,
                         const int width, const int height) {
    const auto right = std::min(image.width(), x + width);
    const auto bottom = std::min(image.height(), y + height);
    for (auto row = std::max(0, y); row < bottom; ++row) {
        for (auto column = std::max(0, x); column < right; ++column) {
            const auto pixel = image.pixelColor(column, row);
            if (pixel.green() > pixel.red() + 24 &&
                pixel.green() > pixel.blue() + 24) {
                return true;
            }
        }
    }
    return false;
}

bool image_region_differs(const QImage& first, const QImage& second,
                          const int x, const int y, const int width,
                          const int height) {
    if (first.size() != second.size()) {
        return true;
    }
    const auto right = std::min(first.width(), x + width);
    const auto bottom = std::min(first.height(), y + height);
    for (auto row = std::max(0, y); row < bottom; ++row) {
        for (auto column = std::max(0, x); column < right; ++column) {
            if (first.pixel(column, row) != second.pixel(column, row)) {
                return true;
            }
        }
    }
    return false;
}

bool image_region_has_color(const QImage& image, const int x, const int y,
                            const int width, const int height,
                            const QColor& expected) {
    const auto right = std::min(image.width(), x + width);
    const auto bottom = std::min(image.height(), y + height);
    for (auto row = std::max(0, y); row < bottom; ++row) {
        for (auto column = std::max(0, x); column < right; ++column) {
            if (image.pixelColor(column, row) == expected) {
                return true;
            }
        }
    }
    return false;
}

bool field_shop_selector_survives(const QImage& scene, const int x) {
    constexpr int kY = 168;
    constexpr int kWidth = 24;
    constexpr int kHeight = 16;
    const QColor selector(196, 190, 39);
    if (scene.width() != kSceneWidth || scene.height() != kSceneHeight ||
        x < 0 || x + 32 > scene.width() || kY + 24 > scene.height()) {
        return false;
    }
    for (int dx = 0; dx < kWidth; ++dx) {
        if (scene.pixelColor(x + dx, kY) != selector ||
            scene.pixelColor(x + dx, kY + kHeight - 1) != selector) {
            return false;
        }
    }
    for (int dy = 0; dy < kHeight; ++dy) {
        if (scene.pixelColor(x, kY + dy) != selector ||
            scene.pixelColor(x + kWidth - 1, kY + dy) != selector) {
            return false;
        }
    }
    int item_ink = 0;
    for (int dy = 0; dy < 24; ++dy) {
        for (int dx = 0; dx < 32; ++dx) {
            const auto is_border =
                dx < kWidth && dy < kHeight &&
                (dx == 0 || dx == kWidth - 1 || dy == 0 ||
                 dy == kHeight - 1);
            const auto pixel = scene.pixelColor(x + dx, kY + dy);
            if (pixel == selector && !is_border) {
                return false;
            }
            if (dx >= 2 && dx < kWidth - 2 && dy >= 2 &&
                dy < kHeight - 2 && pixel.red() < 48 &&
                pixel.green() < 48 && pixel.blue() < 48) {
                ++item_ink;
            }
        }
    }
    return item_ink >= 8;
}

std::filesystem::path find_asset_root(
    const int argc, char** argv,
    const std::filesystem::path& executable_directory) {
    for (int index = 1; index + 1 < argc; ++index) {
        if (std::string_view(argv[index]) == "--assets") {
            return std::filesystem::path(argv[index + 1]);
        }
    }

    std::vector<std::filesystem::path> candidates{
        "decompilation/assets", "assets", "../assets", "../../assets"};
    for (auto directory = executable_directory; !directory.empty();) {
        candidates.push_back(directory / "decompilation" / "assets");
        candidates.push_back(directory / "assets");
        const auto parent = directory.parent_path();
        if (parent == directory) {
            break;
        }
        directory = parent;
    }
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate / "resources.tsv")) {
            return candidate;
        }
    }
    return {};
}

std::optional<StokerMotelPromptId> parse_stoker_dialogue_prompt(
    const std::string_view name) {
    if (name == "arrival") {
        return StokerMotelPromptId::arrival;
    }
    if (name == "manager") {
        return StokerMotelPromptId::manager_greeting;
    }
    if (name == "manager-package") {
        return StokerMotelPromptId::manager_greeting_with_package;
    }
    if (name == "room") {
        return StokerMotelPromptId::room_offer;
    }
    if (name == "debt") {
        return StokerMotelPromptId::debt_question;
    }
    if (name == "leave") {
        return StokerMotelPromptId::leave_notice;
    }
    if (name == "package-question") {
        return StokerMotelPromptId::package_question;
    }
    if (name == "package-handoff") {
        return StokerMotelPromptId::package_handoff;
    }
    if (name == "paid") {
        return StokerMotelPromptId::room_paid;
    }
    if (name == "insufficient") {
        return StokerMotelPromptId::room_insufficient;
    }
    if (name == "jump-house") {
        return StokerMotelPromptId::jump_house_referral;
    }
    if (name == "ambush") {
        return StokerMotelPromptId::street_ambush;
    }
    return std::nullopt;
}

std::optional<std::uint8_t> parse_field_archetype(
    const std::string_view name) {
    if (name == "0" || name == "samurai") {
        return 0U;
    }
    if (name == "1" || name == "decker") {
        return 1U;
    }
    if (name == "2" || name == "gator" || name == "gator_shaman") {
        return 2U;
    }
    return std::nullopt;
}


int count_playfield_lit(const QImage& scene) {
    int lit = 0;
    for (int y = 80; y < 160; ++y) {
        for (int x = 80; x < 240; ++x) {
            const auto pixel = scene.pixelColor(x, y);
            if ((pixel.red() | pixel.green() | pixel.blue()) != 0) {
                ++lit;
            }
        }
    }
    return lit;
}

bool source_shell_pixels_survive(const QImage& scene, const QImage& shell,
                                 const std::initializer_list<QPoint>& points) {
    if (shell.isNull() || scene.size() != shell.size()) {
        return false;
    }
    for (const auto& point : points) {
        if (!scene.rect().contains(point) ||
            scene.pixel(point) != shell.pixel(point)) {
            return false;
        }
    }
    return true;
}

bool field_command_shell_survives(const QImage& scene,
                                  const std::filesystem::path& asset_root) {
    const QImage shell(QString::fromStdWString(
        (asset_root / "menu" / "field" / "background.bmp").wstring()));
    return source_shell_pixels_survive(
        scene, shell,
        {QPoint(10, 10), QPoint(100, 0), QPoint(120, 0),
         QPoint(10, 100)});
}

bool field_command_shell_survives(
    const QImage& scene, const std::filesystem::path& asset_root,
    const std::string& background_file) {
    const QImage shell(QString::fromStdWString(
        (asset_root / "menu" / "field" / background_file).wstring()));
    return source_shell_pixels_survive(
        scene, shell,
        {QPoint(10, 10), QPoint(100, 0), QPoint(120, 0),
         QPoint(10, 100)});
}

bool field_inventory_selector_survives(const QImage& scene,
                                       const std::uint8_t primary,
                                       const std::uint8_t secondary) {
    constexpr int kItemXOrigin = 16;
    constexpr int kItemYOrigin = 120;
    constexpr int kCellWidth = 40;
    constexpr int kCellHeight = 24;
    constexpr int kOutlineWidth = 24;
    constexpr int kOutlineHeight = 16;
    const QColor gold(196, 190, 39);
    const auto x = kItemXOrigin + static_cast<int>(primary) * kCellWidth;
    const auto y = kItemYOrigin + static_cast<int>(secondary) * kCellHeight;
    if (x < 0 || y < 0 || x + kOutlineWidth > scene.width() ||
        y + kOutlineHeight > scene.height()) {
        return false;
    }

    // The one-pixel gold stroke hugs the 24x16 item image. Reject the previous
    // wider cursor-footprint rectangle at its upper-left and lower-right.
    if (scene.pixelColor(x - 2, y - 2) == gold ||
        scene.pixelColor(x + kOutlineWidth + 1,
                         y + kOutlineHeight + 1) == gold) {
        return false;
    }
    for (int offset = 0; offset < kOutlineWidth; ++offset) {
        if (scene.pixelColor(x + offset, y) != gold ||
            scene.pixelColor(x + offset, y + kOutlineHeight - 1) != gold) {
            return false;
        }
    }
    for (int offset = 0; offset < kOutlineHeight; ++offset) {
        if (scene.pixelColor(x, y + offset) != gold ||
            scene.pixelColor(x + kOutlineWidth - 1, y + offset) != gold) {
            return false;
        }
    }
    for (int inner_y = y + 1; inner_y < y + kOutlineHeight - 1; ++inner_y) {
        for (int inner_x = x + 1; inner_x < x + kOutlineWidth - 1;
             ++inner_x) {
            if (scene.pixelColor(inner_x, inner_y) == gold) {
                return false;
            }
        }
    }

    // A populated item remains visible inside the focus frame; the 24x16
    // glyph in this QA record has opaque dark ink at more than its outline.
    int item_ink_pixels = 0;
    for (int item_y = y + 2; item_y < y + 18; ++item_y) {
        for (int item_x = x + 2; item_x < x + 26; ++item_x) {
            const auto pixel = scene.pixelColor(item_x, item_y);
            if (pixel.red() < 48 && pixel.green() < 48 && pixel.blue() < 48) {
                ++item_ink_pixels;
            }
        }
    }
    return item_ink_pixels >= 8;
}

bool field_hud_chrome_survives(const QImage& scene,
                               const std::filesystem::path& asset_root) {
    const QImage shell(QString::fromStdWString(
        (asset_root / "field" / "hud" / "background.bmp").wstring()));
    return source_shell_pixels_survive(
        scene, shell,
        {QPoint(272, 0), QPoint(300, 0), QPoint(319, 0),
         QPoint(319, 32), QPoint(319, 80), QPoint(319, 160),
         QPoint(272, 223), QPoint(300, 223), QPoint(319, 223)});
}

std::array<int, 4> target_marker_corner_pixel_matches(
    const QImage& scene, const QImage& tile,
    const FieldTargetMarkerDebugState& marker) {
    struct Corner {
        int x;
        int y;
        bool flip_x;
        bool flip_y;
    };
    constexpr std::array<Corner, 4> kCorners{{
        {-14, -16, false, true}, {8, -16, true, true},
        {-14, 12, false, false}, {8, 12, true, false},
    }};
    constexpr int kGenesisSpriteOrigin = 0x80;
    std::array<int, 4> matches{};
    if (tile.size() != QSize(8, 8) || !tile.hasAlphaChannel()) {
        return matches;
    }
    const int anchor_x =
        static_cast<int>(marker.projected_x) - kGenesisSpriteOrigin;
    const int anchor_y =
        static_cast<int>(marker.projected_y) - kGenesisSpriteOrigin -
                         (marker.entity_kind == 0x0FU ? 0x20 : 0x0C);
    for (std::size_t corner_index = 0U;
         corner_index < kCorners.size(); ++corner_index) {
        const auto& corner = kCorners[corner_index];
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                const auto source = tile.pixelColor(
                    corner.flip_x ? 7 - x : x,
                    corner.flip_y ? 7 - y : y);
                if (source.alpha() == 0) {
                    continue;
                }
                const int destination_x = anchor_x + corner.x + x;
                const int destination_y = anchor_y + corner.y + y;
                if (destination_x < 0 || destination_y < 0 ||
                    destination_x >= scene.width() ||
                    destination_y >= scene.height()) {
                    continue;
                }
                const auto actual =
                    scene.pixelColor(destination_x, destination_y);
                if (actual.red() == source.red() &&
                    actual.green() == source.green() &&
                    actual.blue() == source.blue()) {
                    ++matches[corner_index];
                }
            }
        }
    }
    return matches;
}

bool dialogue_more_delimiters_are_mirrored(const QImage& scene) {
    if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
        return false;
    }
    // The source label is "          < MORE >" at (48,159). ROM character
    // table word 0xC135C reuses the '<' tile with HFlip for the final '>'.
    constexpr int kLeftDelimiterX = 128;
    constexpr int kRightDelimiterX = 184;
    constexpr int kDelimiterY = 159;
    auto has_ink = false;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const auto left = scene.pixelColor(
                kLeftDelimiterX + x, kDelimiterY + y);
            const auto right = scene.pixelColor(
                kRightDelimiterX + (7 - x), kDelimiterY + y);
            has_ink = has_ink || left != QColor(Qt::black);
            if (left != right) {
                return false;
            }
        }
    }
    return has_ink;
}

bool field_hud_face_family_survives(const QImage& scene,
                                    const std::filesystem::path& asset_root,
                                    const std::size_t archetype_index) {
    // This family-presence check has no live record argument, so any of the
    // three +0x2C palette assets may be the source-selected face. Record-level
    // selection is asserted by field_hud_conversion_test and field-hud QA.
    for (std::size_t palette = 0;
         palette < kFieldHudPortraitPaletteVariantCount; ++palette) {
        const QImage face(QString::fromStdWString(
            (asset_root / "field" / "hud" / "party" /
             ("face-" + std::to_string(archetype_index) + "-palette-" +
              std::to_string(palette) + ".bmp")).wstring()));
        if (face.isNull() || face.size() != QSize(32, 32)) {
            continue;
        }
        bool matches = true;
        int opaque = 0;
        for (int y = 0; y < 32 && matches; ++y) {
            for (int x = 0; x < 32; ++x) {
                const auto source = face.pixelColor(x, y);
                if (source.alpha() == 0) {
                    continue;
                }
                ++opaque;
                if (scene.pixelColor(280 + x, 24 + y) != source) {
                    matches = false;
                    break;
                }
            }
        }
        if (matches && opaque > 64) {
            return true;
        }
    }
    return false;
}

bool field_hud_party_slot_asset_survives(
    const QImage& scene, const std::filesystem::path& asset_root,
    const std::string& relative_path, const int x, const int y,
    const QSize expected_size) {
    const QImage asset(QString::fromStdWString(
        (asset_root / std::filesystem::path(relative_path)).wstring()));
    if (asset.isNull() || asset.size() != expected_size) {
        return false;
    }
    std::size_t opaque = 0U;
    for (int row = 0; row < asset.height(); ++row) {
        for (int column = 0; column < asset.width(); ++column) {
            const auto source = asset.pixelColor(column, row);
            if (source.alpha() == 0) {
                continue;
            }
            ++opaque;
            if (scene.pixelColor(x + column, y + row) != source) {
                return false;
            }
        }
    }
    return opaque > 8U;
}

bool field_hud_meter_assets_survive(const QImage& scene,
                                    const std::filesystem::path& asset_root,
                                    const std::uint8_t delay_meter,
                                    const std::uint8_t behavior_meter,
                                    const bool primary_runner) {
    const auto delay_index = std::min<std::uint8_t>(delay_meter, 10U);
    const auto behavior_index = std::min<std::uint8_t>(behavior_meter, 10U);
    const auto delay = QImage(QString::fromStdWString(
        (asset_root / "field" / "hud" /
         ("delay-meter-" + std::to_string(delay_index) + ".bmp"))
            .wstring()));
    const auto behavior = QImage(QString::fromStdWString(
        (asset_root / "field" / "hud" /
         ((primary_runner ? "behavior-meter-primary-"
                          : "behavior-meter-companion-") +
          std::to_string(behavior_index) + ".bmp"))
            .wstring()));
    if (delay.size() != QSize(8, 32) || behavior.size() != QSize(32, 8) ||
        scene.width() < 312 || scene.height() < 64) {
        return false;
    }

    const auto matches_at = [&scene](const QImage& asset, const int x,
                                     const int y) {
        std::size_t checked = 0U;
        for (int row = 0; row < asset.height(); ++row) {
            for (int column = 0; column < asset.width(); ++column) {
                const auto pixel = asset.pixelColor(column, row);
                if (pixel.alpha() == 0) {
                    continue;
                }
                if (scene.pixelColor(x + column, y + row) != pixel) {
                    return false;
                }
                ++checked;
            }
        }
        return checked > 0U;
    };
    return matches_at(delay, 272, 24) && matches_at(behavior, 280, 56);
}
