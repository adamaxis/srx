    // ROM 0x07DA runs the 0x131C archetype fade before 0x59C5E. QA drivers
    // that jump straight to the opening pump those source frames here so
    // their opening-relative frame anchors stay unchanged.
    void qa_finish_archetype_exit_fade() {
        while (state_.phase ==
               StartupIntegrationPhase::joshua_archetype_exit_fade) {
            advance_startup_archetype_exit_fade_frame(state_,
                                                      startup_callbacks_);
        }
    }

    bool qa_finish_command_overview_field_return() {
        // ROM 0xB62 -> 0x10D30 returns through 0x2BD2/0x131C; keep this
        // re-entry QA behind the emulator's P+112 field-live frame.
        for (std::uint16_t frame = 0U; frame < 113U; ++frame) {
            const auto route =
                field_scene_runtime_.field_menu_route_snapshot_for_dump();
            if (!route.field_return_transition_active) {
                return route.field_return_transition_phase ==
                       scripting::ScriptFieldReturnTransitionPhase::complete;
            }
            field_scene_runtime_.set_controller_port_byte(0U);
            field_scene_runtime_.advance_frame();
        }
        return !field_scene_runtime_.field_menu_route_snapshot_for_dump()
                    .field_return_transition_active;
    }

    bool startup_shell_survives(const QImage& scene) const {
        const auto shell = draw_asset_cache_.find("menu.startup.background");
        if (shell == draw_asset_cache_.end()) {
            return false;
        }
        return source_shell_pixels_survive(
            scene, shell->second,
            {QPoint(30, 30), QPoint(45, 20), QPoint(250, 20),
             QPoint(36, 100), QPoint(282, 100), QPoint(30, 190),
             QPoint(250, 190)});
    }

    bool startup_shell_only(const QImage& scene) const {
        if (draw_asset_cache_.find("menu.startup.background") ==
            draw_asset_cache_.end()) {
            return false;
        }
        QImage expected(scene.size(), QImage::Format_RGB32);
        expected.fill(Qt::black);
        QPainter painter(&expected);
        draw_alias(painter, "menu.startup.background");
        painter.end();
        return scene.convertToFormat(QImage::Format_RGB32) == expected;
    }

    int write_startup_dump_image(
        const std::string_view scene_name, const std::uint32_t selection,
        const std::uint32_t target_frame,
        const std::filesystem::path& output_path) {
        const auto scene = render_scene();
        const auto is_startup_menu =
            state_.phase == StartupIntegrationPhase::start_options_shell ||
            state_.phase == StartupIntegrationPhase::start_options ||
            state_.phase == StartupIntegrationPhase::save_load ||
            state_.phase == StartupIntegrationPhase::joshua_archetype;
        if (is_startup_menu &&
            (!startup_shell_survives(scene) ||
             (state_.phase == StartupIntegrationPhase::start_options_shell &&
              !startup_shell_only(scene)))) {
            std::cerr << "startup dump: startup shell was not cleanly "
                         "present for "
                      << scene_name << '\n';
            return 3;
        }
        if (!scene.save(QString::fromStdWString(output_path.wstring()))) {
            std::cerr << "startup dump: could not write "
                      << output_path.string() << '\n';
            return 3;
        }
        std::cout << "startup_dump=written stage=" << scene_name
                  << " selection=" << selection
                  << " frame=" << target_frame
                  << " path=" << output_path.string() << '\n';
        return 0;
    }

    void note_presentation_boundary(const std::string_view boundary) {
        presentation_boundaries_.emplace_back(boundary);
    }

    template <typename Callback, typename Observer>
    void observe_qa_callback(Callback& callback, Observer observer) {
        if (!callback) {
            return;
        }
        auto implementation = std::move(callback);
        callback = [implementation = std::move(implementation),
                    observer = std::move(observer)](auto&&... arguments) mutable {
            using Result = decltype(implementation(
                std::forward<decltype(arguments)>(arguments)...));
            if constexpr (std::is_void_v<Result>) {
                implementation(
                    std::forward<decltype(arguments)>(arguments)...);
                observer(arguments...);
            } else {
                auto result = implementation(
                    std::forward<decltype(arguments)>(arguments)...);
                observer(arguments...);
                return result;
            }
        };
    }

    template <typename Callback>
    void record_qa_boundary(Callback& callback,
                            const std::string_view boundary) {
        observe_qa_callback(callback, [this, boundary](auto&&...) {
            note_presentation_boundary(boundary);
        });
    }

    void install_qa_startup_observers() {
        observe_qa_callback(startup_callbacks_.initialize_display,
                            [this] { display_initialized_ = true; });
        observe_qa_callback(startup_callbacks_.vdp_ready.warmup_delay,
                            [this](const std::uint32_t delay) {
                                last_source_delay_ = delay;
                            });
        observe_qa_callback(startup_callbacks_.vdp_ready.wait_until_ready,
                            [this] { display_ready_ = true; });
        observe_qa_callback(startup_callbacks_.vdp_ready.settle_delay,
                            [this](const std::uint32_t delay) {
                                last_source_delay_ = delay;
                            });

        auto& resources = startup_callbacks_.pre_title.resources;
        record_qa_boundary(resources.clear_plane_a,
                           "pre_title.clear_plane_a");
        record_qa_boundary(resources.clear_plane_b,
                           "pre_title.clear_plane_b");
        record_qa_boundary(resources.prepare_title_resources,
                           "pre_title.prepare_title_resources");
        record_qa_boundary(resources.write_resource_stream,
                           "pre_title.write_resource_stream");
        record_qa_boundary(resources.refresh_field_resource_streams,
                           "pre_title.refresh_field_resource_streams");
        record_qa_boundary(resources.decode_and_upload_source_stream,
                           "pre_title.decode_and_upload_source_stream");
        record_qa_boundary(resources.advance_title_palette_fade,
                           "pre_title.advance_title_palette_fade");
        observe_qa_callback(resources.write_pre_title_grid,
                            [this](const PreTitleGridWriteRequest& request) {
                                note_presentation_boundary(
                                    request.purpose ==
                                            PreTitleGridPurpose::resource_setup
                                        ? "pre_title.write_resource_grid"
                                        : "pre_title.write_final_grid");
                            });
        record_qa_boundary(resources.reset_vdp_upload_state,
                           "pre_title.reset_vdp_upload_state");

        auto& animation = startup_callbacks_.pre_title.animation;
        observe_qa_callback(animation.publish_sprite_batch,
                            [this](const PreTitleSpriteBatch&) {
                                ++title_sprite_record_builds_;
                                note_presentation_boundary(
                                    "pre_title.publish_sprite_batch");
                            });
        record_qa_boundary(animation.clear_last_sprite_sequence,
                           "pre_title.clear_last_sprite_sequence");
        record_qa_boundary(animation.publish_logo_strobe_palette,
                           "pre_title.publish_logo_strobe_palette");
        record_qa_boundary(animation.step_entity_color_words,
                           "pre_title.step_entity_color_words");
        record_qa_boundary(startup_callbacks_.pre_title.sample_controller,
                           "pre_title.sample_controller");
        record_qa_boundary(
            startup_callbacks_.pre_title.latch_controller_events,
            "pre_title.latch_controller_events");
        record_qa_boundary(
            startup_callbacks_.pre_title.step_palette_transition,
            "pre_title.step_palette_transition");
        observe_qa_callback(
            startup_callbacks_.pre_title.publish_audio_request,
            [this](const PreTitleAudioRequest& request) {
                pre_title_audio_requests_.push_back(request);
                note_presentation_boundary(
                    request.service_path ==
                            PreTitleAudioServicePath::
                                enabled_variant_transaction
                        ? "pre_title.service_logo_effect_variant_audio"
                        : "pre_title.service_logo_effect_audio");
            });

        auto& setup = startup_callbacks_.title_setup;
        record_qa_boundary(setup.prepare_scene_resources,
                           "title.prepare_scene_resources");
        record_qa_boundary(setup.load_title_resources,
                           "title.load_title_resources");
        record_qa_boundary(setup.load_title_font, "title.load_title_font");
        record_qa_boundary(setup.advance_palette_frames,
                           "title.advance_palette_frames");

        auto& start_options = startup_callbacks_.start_options;
        record_qa_boundary(start_options.copy_start_options_resource_metadata,
                           "start_options.copy_resource_metadata");
        record_qa_boundary(
            start_options.refresh_start_options_resource_metadata,
            "start_options.refresh_resource_metadata");
        record_qa_boundary(start_options.transfer_start_options_menu_tiles,
                           "start_options.transfer_menu_tiles");
        record_qa_boundary(start_options.load_startup_menu_font,
                           "start_options.load_menu_font");
        record_qa_boundary(start_options.draw_new_game_load_game_grid,
                           "start_options.draw_new_load_grid");
        record_qa_boundary(start_options.draw_start_options_title,
                           "start_options.draw_title");
        record_qa_boundary(start_options.draw_start_options_cursor,
                           "start_options.draw_cursor");

        auto& save_load = startup_callbacks_.save_load;
        record_qa_boundary(save_load.draw_save_load_grid,
                           "save_load.draw_grid");
        record_qa_boundary(save_load.draw_save_load_title,
                           "save_load.draw_title");
        record_qa_boundary(save_load.draw_save_load_cursor,
                           "save_load.draw_cursor");
        record_qa_boundary(save_load.draw_save_load_message,
                           "save_load.draw_message");
        record_qa_boundary(save_load.wait_for_controller_event,
                           "save_load.wait_for_controller_event");

        auto& archetype = startup_callbacks_.joshua_archetype;
        record_qa_boundary(archetype.draw_joshua_archetype_grid,
                           "archetype.draw_grid");
        record_qa_boundary(archetype.draw_joshua_archetype_title,
                           "archetype.draw_title");
        record_qa_boundary(archetype.draw_joshua_archetype_cursor,
                           "archetype.draw_cursor");
        record_qa_boundary(archetype.preview_joshua_archetype_resources,
                           "archetype.preview_resources");
        record_qa_boundary(archetype.initialize_joshua_archetype,
                           "archetype.initialize_joshua_record");

        auto& opening = startup_callbacks_.opening_intro;
        record_qa_boundary(opening.sample_controller,
                           "opening.sample_controller");
        record_qa_boundary(opening.latch_controller_events,
                           "opening.latch_controller_events");
        record_qa_boundary(opening.prepare_mode_entry,
                           "opening.prepare_mode_entry");
        record_qa_boundary(opening.service_audio_command,
                           "opening.service_audio_command");
        record_qa_boundary(opening.service_audio, "opening.service_audio");
        record_qa_boundary(opening.fade_palette_to_black,
                           "opening.fade_palette_to_black");
        record_qa_boundary(opening.advance_palette_fade,
                           "opening.advance_palette_fade");
        record_qa_boundary(opening.enter_post_intro_mode,
                           "opening.enter_post_intro_mode");
        record_qa_boundary(opening.clear_presentation,
                           "opening.clear_presentation");
        record_qa_boundary(opening.stage_changed, "opening.stage_changed");
        record_qa_boundary(opening.scene_changed, "opening.scene_changed");
        record_qa_boundary(opening.clear_wilderness_right_map,
                           "opening.clear_wilderness_right_map");
        record_qa_boundary(opening.clear_wilderness_center_map,
                           "opening.clear_wilderness_center_map");
        record_qa_boundary(opening.clear_wilderness_left_region,
                           "opening.clear_wilderness_left_region");
        record_qa_boundary(opening.clear_stop_request,
                           "opening.clear_intro_stop_marker");
        observe_qa_callback(opening.source_boundary,
                            [this](const std::string_view boundary) {
                                note_presentation_boundary(boundary);
                            });
    }

    void install_qa_title_observers() {
        observe_qa_callback(title_loop_callbacks_.wait_for_vblank, [this] {
            ++title_vblanks_;
        });
        observe_qa_callback(title_loop_callbacks_.request_audio_command,
                            [this](const TitleAudioCommand command) {
                                if (command == TitleAudioCommand::LogoReveal) {
                                    note_presentation_boundary(
                                        "title.logo_reveal.audio_command_5d");
                                } else if (command ==
                                           TitleAudioCommand::ThemeStart) {
                                    note_presentation_boundary(
                                        "title.theme.audio_command_00");
                                } else {
                                    note_presentation_boundary(
                                        "title.audio_command_unmapped");
                                }
                            });
        record_qa_boundary(title_loop_callbacks_.publish_animation_offset,
                           "title.circuit_background_scroll_advanced");
        record_qa_boundary(title_loop_callbacks_.reset_vdp_upload_state,
                           "title.reset_vdp_upload_state");
        record_qa_boundary(
            title_loop_callbacks_.latch_palette_transition_events,
            "title.latch_palette_transition_events");
        observe_qa_callback(title_loop_callbacks_.step_palette_transition,
                            [this] { ++title_exit_palette_steps_; });

        observe_qa_callback(title_mode_entry_callbacks_.wait_for_vblank,
                            [this] {
                                ++title_mode_entry_vblanks_;
                                note_presentation_boundary(
                                    "title.mode_entry.wait_for_vblank");
                            });
        observe_qa_callback(
            title_mode_entry_callbacks_.run_audio_transaction,
            [this](const std::uint8_t command) {
                ++title_mode_entry_transactions_;
                note_presentation_boundary(
                    command == kTitleModeEntryAudioCommand
                        ? "title.mode_entry.audio_command_14"
                        : "title.mode_entry.audio_command_unresolved");
            });
        observe_qa_callback(title_mode_entry_callbacks_.flush_audio_bus,
                            [this] {
                                ++title_mode_entry_flushes_;
                                note_presentation_boundary(
                                    "title.mode_entry.flush_audio_bus");
                            });
        observe_qa_callback(title_mode_entry_callbacks_.service_audio_tick,
                            [this] {
                                ++title_mode_entry_audio_ticks_;
                                note_presentation_boundary(
                                    "title.mode_entry.service_audio_tick");
                            });
    }

    int dump_boot_frame(const std::uint32_t target_frame,
                        const std::filesystem::path& output_path) {
        host_frame_timer_.stop();
        for (std::uint32_t frame = 0; frame < target_frame; ++frame) {
            if (state_.phase != StartupIntegrationPhase::pre_title) {
                std::cerr << "boot dump: source frame passed title handoff\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::pre_title) {
            std::cerr << "boot dump: source frame passed title handoff\n";
            return 3;
        }

        const auto scene = render_scene();
        if (!scene.save(QString::fromStdWString(output_path.wstring()))) {
            std::cerr << "boot dump: could not write "
                      << output_path.string() << '\n';
            return 3;
        }
        std::cout << "boot_dump=written frame=" << target_frame
                  << " stage="
                  << static_cast<int>(state_.pre_title.presentation_stage)
                  << " path=" << output_path.string() << '\n';
        return 0;
    }

    int run_boot_fasa_qa(const std::filesystem::path& output_directory) {
        host_frame_timer_.stop();
        // Frame-indexed: the Lua presentation holds are verified separately.
        opening_presentation_holds_enabled_ = false;
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "boot-fasa-qa.tsv");
        if (!trace) {
            std::cerr << "boot FASA QA: could not open trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "label\tframe\tstage\tpalette_pass\tvblank_phase\t"
                 "hold_remaining\tfigure_fade_steps\tlightning_batches\n";

        const auto fail = [](const std::string& message) {
            std::cerr << "boot FASA QA: " << message << '\n';
            return 3;
        };
        const auto stage = [this] {
            return state_.pre_title.presentation_stage;
        };
        const auto save_snapshot = [&](const std::string& label,
                                       const std::string& filename) {
            const auto scene = render_scene();
            const auto path = output_directory / filename;
            if (scene.size() != QSize(kSceneWidth, kSceneHeight) ||
                !scene.save(QString::fromStdWString(path.wstring()))) {
                return false;
            }
            trace << label << '\t' << state_.frame.frame_count << '\t'
                  << static_cast<unsigned>(stage()) << '\t'
                  << static_cast<unsigned>(state_.pre_title
                                               .palette_transition_passes_completed)
                  << '\t'
                  << static_cast<unsigned>(state_.pre_title
                                               .palette_transition_vblank_phase)
                  << '\t' << state_.pre_title.fasa_hold_frames << '\t'
                  << static_cast<unsigned>(state_.pre_title
                                               .figure_palette_fade_steps)
                  << '\t'
                  << (state_.pre_title.sega_lightning_batches_active ? 1 : 0)
                  << '\n';
            return true;
        };
        const auto is_black = [](const QImage& scene) {
            for (int y = 0; y < scene.height(); ++y) {
                for (int x = 0; x < scene.width(); ++x) {
                    const auto pixel = scene.pixelColor(x, y);
                    if (pixel.red() != 0 || pixel.green() != 0 ||
                        pixel.blue() != 0) {
                        return false;
                    }
                }
            }
            return true;
        };
        constexpr std::uint32_t kMaximumSourceFrames = 700U;
        std::uint32_t advanced_frames = 0U;
        const auto advance_until = [&](const auto& predicate) {
            while (!predicate()) {
                if (state_.phase != StartupIntegrationPhase::pre_title ||
                    advanced_frames >= kMaximumSourceFrames) {
                    return false;
                }
                advance_one_source_frame();
                ++advanced_frames;
            }
            return true;
        };
        if (state_.phase != StartupIntegrationPhase::pre_title ||
            !advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::sega_shock;
            }) ||
            !save_snapshot("sega_shock_start", "sega-shock-start.bmp")) {
            return fail("did not reach the SEGA shock phase");
        }
        if (!advance_until([&] {
                return state_.pre_title.sega_lightning_batches_active;
            }) ||
            !save_snapshot("sega_shock_lightning",
                           "sega-shock-lightning.bmp")) {
            return fail("0x5D996 did not publish the shock lightning batches");
        }
        if (!advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::sega_strobe;
            })) {
            return fail("did not reach the ROM SEGA logo strobe");
        }
        if (!state_.pre_title.sega_lightning_batches_active ||
            state_.pre_title.figure_palette_fade_steps != 0U ||
            !save_snapshot("sega_strobe_start", "sega-strobe-start.bmp")) {
            return fail("strobe activation lost the source-built lightning or figure");
        }
        advance_one_source_frame();
        if (stage() !=
                PreTitleSequenceState::PresentationStage::sega_strobe ||
            state_.pre_title.sega_lightning_batches_active ||
            !save_snapshot("sega_strobe_lightning_cleared",
                           "sega-strobe-lightning-cleared.bmp")) {
            return fail("stale lightning survived the source strobe handoff");
        }
        if (!advance_until([&] {
                return state_.pre_title.figure_palette_fade_steps == 1U;
            }) ||
            !save_snapshot("sega_figure_fade_step_1",
                           "sega-figure-fade-step-1.bmp")) {
            return fail("first figure CRAM fade step did not arrive");
        }
        if (!advance_until([&] {
                return state_.pre_title.figure_palette_fade_steps ==
                       kPreTitleFigurePaletteLevelsToBlack;
            }) ||
            stage() != PreTitleSequenceState::PresentationStage::sega_strobe) {
            return fail("figure palette did not reach black during the strobe");
        }
        const auto resolved_sega = render_scene();
        QImage expected_sega(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        expected_sega.fill(Qt::black);
        {
            QPainter painter(&expected_sega);
            draw_boot_snow(painter);
            draw_alias(
                painter,
                "boot.sega.strobe." +
                    std::to_string(state_.pre_title.strobe_phase % 8U));
        }
        if (resolved_sega != expected_sega ||
            !save_snapshot("sega_resolved", "sega-resolved.bmp")) {
            return fail("resolved SEGA frame retained the figure or lightning");
        }
        if (!advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::fasa_fade_in;
            })) {
            return fail("did not reach the ROM FASA fade-in stage");
        }
        if (state_.pre_title.palette_transition_passes_completed != 0U ||
            state_.pre_title.palette_transition_vblank_phase != 0U ||
            !save_snapshot("fasa_fade_in_start", "fade-in-start.bmp")) {
            return fail("fade-in did not begin from a clean zero-pass state");
        }
        if (!is_black(render_scene())) {
            return fail("FASA fade-in must start from black");
        }

        advance_one_source_frame();
        advance_one_source_frame();
        if (state_.pre_title.palette_transition_passes_completed != 1U ||
            state_.pre_title.palette_transition_vblank_phase != 0U ||
            !save_snapshot("fasa_fade_in_pass_1", "fade-in-pass-1.bmp")) {
            return fail("first fade-in palette pass did not take two VBlanks");
        }
        const auto first_fade_in = render_scene();
        if (is_black(first_fade_in)) {
            return fail("first FASA fade-in pass did not reveal color");
        }
        if (!advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::fasa_hold;
            })) {
            return fail("FASA fade-in did not reach its hold stage");
        }
        if (state_.pre_title.fasa_hold_frames != 0x0079U ||
            state_.pre_title.palette_transition_passes_completed !=
                kPreTitlePaletteTransitionPasses) {
            return fail("FASA fade-in or source hold length is incorrect");
        }
        const auto fasa_full = render_scene();
        const auto* fasa_record =
            state_.asset_catalog.catalog.find("boot.fasa");
        if (fasa_record == nullptr) {
            return fail("boot.fasa is absent from the direct asset catalog");
        }
        QImage fasa_image(
            QString::fromStdWString(fasa_record->absolute_path.wstring()));
        if (fasa_image.isNull()) {
            return fail("could not load the named boot.fasa asset");
        }
        QImage expected_fasa(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        expected_fasa.fill(Qt::black);
        {
            QPainter painter(&expected_fasa);
            painter.drawImage(fasa_record->origin_x, fasa_record->origin_y,
                              fasa_image);
        }
        if (expected_fasa != fasa_full) {
            return fail("FASA screen contains stale Sega graphics or controls");
        }
        if (!save_snapshot("fasa_full_hold", "actual.bmp")) {
            return fail("could not write the required actual.bmp image");
        }

        for (std::uint32_t frame = 0; frame < 120U; ++frame) {
            advance_one_source_frame();
        }
        if (stage() != PreTitleSequenceState::PresentationStage::fasa_hold ||
            state_.pre_title.fasa_hold_frames != 1U ||
            render_scene() != fasa_full ||
            !save_snapshot("fasa_hold_last", "hold-last.bmp")) {
            return fail("FASA hold must stay visually stable for 121 VBlanks");
        }
        advance_one_source_frame();
        if (stage() !=
                PreTitleSequenceState::PresentationStage::fasa_fade_out ||
            state_.pre_title.fasa_hold_frames != 0U ||
            state_.pre_title.palette_transition_passes_completed != 0U ||
            render_scene() != fasa_full ||
            !save_snapshot("fasa_fade_out_start", "fade-out-start.bmp")) {
            return fail("FASA fade-out must begin after the full hold");
        }

        advance_one_source_frame();
        advance_one_source_frame();
        if (state_.pre_title.palette_transition_passes_completed != 1U ||
            state_.pre_title.palette_transition_vblank_phase != 0U ||
            !save_snapshot("fasa_fade_out_pass_1", "fade-out-pass-1.bmp") ||
            render_scene() == fasa_full) {
            return fail("first fade-out palette pass did not darken FASA");
        }
        if (!advance_until([&] {
                return stage() ==
                           PreTitleSequenceState::PresentationStage::fasa_fade_out &&
                       state_.pre_title.palette_transition_passes_completed ==
                           16U &&
                       state_.pre_title.palette_transition_vblank_phase == 0U;
            })) {
            return fail("FASA fade-out did not reach its final black hold");
        }
        const auto fasa_black = render_scene();
        if (!is_black(fasa_black) ||
            !save_snapshot("fasa_fade_out_black", "fade-out-black.bmp")) {
            return fail("FASA must be fully black before title handoff");
        }
        for (std::uint32_t handoff_frame = 1U; handoff_frame <= 2U;
             ++handoff_frame) {
            advance_one_source_frame();
            if (!is_black(render_scene())) {
                return fail(
                    "the post-FASA handoff must stay fully black before "
                    "title setup completes");
            }
            const auto label = handoff_frame == 1U
                ? "post_fasa_black_frame_1"
                : "post_fasa_black_frame_2";
            const auto filename = handoff_frame == 1U
                ? "pre-title-black-frame-1.bmp"
                : "pre-title-black-frame-2.bmp";
            if (!save_snapshot(label, filename)) {
                return fail("could not save the post-FASA black handoff frame");
            }
        }
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            !state_.pre_title_complete || state_.frame.frame_count != 560U ||
            !save_snapshot("title_handoff", "title-handoff.bmp")) {
            return fail("title handoff must follow the complete 560-VBlank sequence");
        }
        const auto title_handoff = render_scene();
        if (title_handoff == fasa_full || !is_black(title_handoff) ||
            title_loop_.logo_reveal_delay != 0x5AU ||
            title_loop_.logo.columns_revealed != 0U ||
            title_loop_.text.palette_steps != 0U) {
            return fail(
                "title handoff must replace FASA with a clean black hold");
        }

        std::ofstream logo_roll_trace(
            output_directory / "title-black-interval.tsv");
        if (!logo_roll_trace) {
            return fail("could not open the title black interval trace");
        }
        logo_roll_trace
            << "title_frame\tlogo_delay\tlogo_columns\tcircuit_palette_steps\t"
               "sprite_visible\tfull_screen_black\n";
        const auto save_title_checkpoint = [&](const char* label,
                                                const char* filename,
                                                const std::uint32_t title_frame) {
            const auto scene = render_scene();
            const auto path = output_directory / filename;
            if (scene.size() != QSize(kSceneWidth, kSceneHeight) ||
                !scene.save(QString::fromStdWString(path.wstring()))) {
                return false;
            }
            std::cout << "boot_title_checkpoint=" << label
                      << " frame=" << title_frame
                      << " path=" << path.string() << '\n';
            return true;
        };
        const auto record_title_state = [&](const std::uint32_t title_frame,
                                             const QImage& scene) {
            logo_roll_trace << title_frame << '\t'
                            << title_loop_.logo_reveal_delay << '\t'
                            << title_loop_.logo.columns_revealed << '\t'
                            << static_cast<unsigned>(
                                   title_loop_.text.palette_steps)
                            << '\t'
                            << (title_loop_.sprite.projection_visible ? 1 : 0)
                            << '\t' << (is_black(scene) ? 1 : 0) << '\n';
        };
        if (!save_title_checkpoint("black_start", "title-black-start.bmp",
                                   0U)) {
            return fail("could not save the first black title frame");
        }
        record_title_state(0U, title_handoff);
        for (std::uint32_t title_frame = 1U; title_frame <= 0x5AU;
             ++title_frame) {
            advance_one_source_frame();
            const auto scene = render_scene();
            const auto expected_delay = static_cast<std::int16_t>(
                static_cast<std::int16_t>(0x5AU) -
                static_cast<std::int16_t>(title_frame));
            if (state_.phase != StartupIntegrationPhase::title_ready ||
                title_loop_.logo_reveal_delay != expected_delay ||
                title_loop_.logo.columns_revealed != 0U ||
                title_loop_.text.palette_steps != 0U ||
                title_loop_.sprite.projection_visible || !is_black(scene)) {
                return fail(
                    "the screen must remain fully black for all 90 "
                    "title-delay frames");
            }
            record_title_state(title_frame, scene);
            if (title_frame == 1U || title_frame == 0x2DU ||
                title_frame == 0x5AU) {
                const auto filename = title_frame == 1U
                    ? "title-black-frame-1.bmp"
                    : title_frame == 0x2DU
                        ? "title-black-frame-45.bmp"
                        : "title-black-frame-90.bmp";
                if (!save_title_checkpoint("black_hold", filename,
                                           title_frame)) {
                    return fail(
                        "could not save a black title-delay checkpoint");
                }
            }
        }
        if (title_loop_.logo_reveal_delay != 0 ||
            title_loop_.logo.columns_revealed != 0U ||
            !is_black(render_scene())) {
            return fail("the logo must remain hidden while the source delay is zero");
        }
        advance_one_source_frame();
        const auto first_logo_column = render_scene();
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            title_loop_.logo_reveal_delay != -1 ||
            title_loop_.logo.columns_revealed != 1U ||
            title_loop_.text.palette_steps != 0U ||
            !title_loop_.sprite.projection_visible ||
            is_black(first_logo_column) ||
            !save_title_checkpoint("first_logo_column",
                                   "title-logo-first-column.bmp",
                                   0x5BU)) {
            return fail(
                "the first logo column must appear after the 90-frame "
                "black delay");
        }
        record_title_state(0x5BU, first_logo_column);
        logo_roll_trace.flush();
        const auto frame_dump_path =
            output_directory / "boot-dump-frame-373.bmp";
        {
            // This widget embeds the converted startup and field source state.
            // Keep the nested QA-only instance off the stack: the outer window
            // and the asset loader's typed scene records are still live here.
            const auto frame_dump_window =
                std::make_unique<GhidraConversionWindow>(asset_root_);
            if (frame_dump_window->dump_boot_frame(373U, frame_dump_path) !=
                0) {
                return fail("--boot-dump-frame could not render frame 373");
            }
        }
        const QImage dumped_fade_frame(
            QString::fromStdWString(frame_dump_path.wstring()));
        if (dumped_fade_frame.convertToFormat(QImage::Format_RGB32) !=
            first_fade_in) {
            return fail("--boot-dump-frame skipped the FASA palette fade");
        }
        std::cout << "boot_fasa_qa=passed source_frames="
                  << state_.frame.frame_count
                  << " fade_passes="
                  << static_cast<unsigned>(kPreTitlePaletteTransitionPasses)
                  << " hold=" << 0x0079U << " actual="
                  << (output_directory / "actual.bmp").string() << '\n';
        return 0;
    }

    // Deterministic visual-audit entry point. The phase transitions and
    // opening frames are still driven by the converted source contracts;
    // this only exposes their existing Qt presentation as an image dump.
    int prepare_opening_dump(const std::uint32_t selection) {
        host_frame_timer_.stop();
        opening_presentation_holds_enabled_ = false;
        if (selection > 2U) {
            std::cerr << "startup dump: selection must be 0, 1, or 2\n";
            return 3;
        }
        constexpr std::uint32_t kMaxSetupFrames = 10000U;
        std::uint32_t setup_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: boot did not reach title\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            std::cerr << "startup dump: phase after boot="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        while (state_.phase == StartupIntegrationPhase::title_ready) {
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: title did not accept Start\n";
                return 3;
            }
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kTitleStartButton) << 8U);
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::start_options_shell) {
            std::cerr << "startup dump: phase after title="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            std::cerr << "startup dump: New Game did not reach archetype menu\n";
            return 3;
        }
        for (std::uint32_t index = 0U; index < selection; ++index) {
            advance_startup_integration(
                state_, kStartupMenuDown, startup_callbacks_);
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        qa_finish_archetype_exit_fade();
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            std::cerr << "startup dump: archetype did not start opening\n";
            return 3;
        }
        return 0;
    }

    int prepare_startup_menu_dump(const std::uint32_t selection,
                                  const bool leave_at_shell = false) {
        host_frame_timer_.stop();
        if (selection > 2U) {
            std::cerr << "startup dump: selection must be 0, 1, or 2\n";
            return 3;
        }
        constexpr std::uint32_t kMaxSetupFrames = 10000U;
        std::uint32_t setup_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: boot did not reach title\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            std::cerr << "startup dump: phase after boot="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }
        while (state_.phase == StartupIntegrationPhase::title_ready) {
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: title did not accept Start\n";
                return 3;
            }
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kTitleStartButton) << 8U);
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::start_options_shell) {
            std::cerr << "startup dump: phase after title="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }
        if (leave_at_shell) {
            if (selection != 0U) {
                std::cerr << "startup dump: shell stage only supports "
                             "selection 0\n";
                return 3;
            }
            return 0;
        }
        if (selection == 0U) {
            advance_one_source_frame();
            if (state_.phase != StartupIntegrationPhase::start_options) {
                std::cerr << "startup dump: Start Options did not reveal after "
                             "its source VBlank\n";
                return 3;
            }
            return 0;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            std::cerr << "startup dump: New Game did not reach archetype menu\n";
            return 3;
        }
        for (std::uint32_t index = 0U; index + 1U < selection; ++index) {
            advance_startup_integration(
                state_, kStartupMenuDown, startup_callbacks_);
        }
        return 0;
    }

    int dump_startup_scene(const std::string_view scene_name,
                           const std::uint32_t selection,
                           const std::uint32_t target_frame,
                           const std::filesystem::path& output_path) {
        if (scene_name != "start_options" &&
            scene_name != "start_options_shell" &&
            scene_name != "archetype" && scene_name != "opening") {
            std::cerr << "startup dump: unknown stage " << scene_name
                      << " (expected start_options, start_options_shell, "
                         "archetype, or opening)\n";
            return 3;
        }
        if (scene_name == "start_options" ||
            scene_name == "start_options_shell") {
            if (prepare_startup_menu_dump(
                    0U, scene_name == "start_options_shell") != 0) {
                return 3;
            }
            return write_startup_dump_image(
                scene_name, selection, target_frame, output_path);
        }
        if (scene_name == "archetype") {
            if (prepare_startup_menu_dump(selection + 1U) != 0) {
                return 3;
            }
            return write_startup_dump_image(
                scene_name, selection, target_frame, output_path);
        }
        if (prepare_opening_dump(selection) != 0) {
            return 3;
        }
        for (std::uint32_t frame = 0U; frame < target_frame; ++frame) {
            if (state_.phase != StartupIntegrationPhase::opening_intro) {
                std::cerr << "startup dump: opening ended before frame "
                          << target_frame << '\n';
                return 3;
            }
            advance_one_source_frame();
        }
        return write_startup_dump_image(
            scene_name, selection, target_frame, output_path);
    }

    // Render every opening frame from start to end-1 into one directory so
    // the conversion can be compared frame-by-frame against the 30fps
    // longplay dump instead of one frame at a time.
    int dump_opening_range(const std::uint32_t selection,
                           const std::uint32_t start, const std::uint32_t end,
                           const std::filesystem::path& output_directory) {
        if (prepare_opening_dump(selection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream boundary_log(output_directory / "boundaries.tsv");
        std::size_t next_boundary = 0U;
        for (std::uint32_t frame = 0U; frame < end; ++frame) {
            if (state_.phase != StartupIntegrationPhase::opening_intro) {
                std::cerr << "startup range dump: opening ended at frame "
                          << frame << '\n';
                return 3;
            }
            while (next_boundary < presentation_boundaries_.size()) {
                boundary_log << frame << '\t'
                             << presentation_boundaries_[next_boundary++]
                             << '\n';
            }
            if (frame >= start) {
                const auto scene = render_scene();
                const auto path = output_directory /
                                  ("frame-" + std::to_string(frame) + ".bmp");
                if (!scene.save(QString::fromStdWString(path.wstring()))) {
                    std::cerr << "startup range dump: could not write "
                              << path.string() << '\n';
                    return 3;
                }
            }
            advance_one_source_frame();
        }
        std::cout << "startup_range_dump=complete selection=" << selection
                  << " frames=" << start << '-' << end << " path="
                  << output_directory.string() << '\n';
        return 0;
    }

    // Full temporal capture for visual audits. This follows the ordinary
    // title -> Start Options -> New Game -> archetype -> opening -> field
    // route, retaining every complete 320x224 source frame in sequence.
    int dump_full_intro_capture(
        const std::uint32_t selection,
        const std::uint32_t title_idle_frames,
        const std::filesystem::path& output_directory) {
        host_frame_timer_.stop();
        if (selection > 2U || title_idle_frames > 10000U) {
            std::cerr << "full intro capture: archetype must be 0..2 and "
                         "title idle frames must be at most 10000\n";
            return 2;
        }

        std::filesystem::create_directories(output_directory);
        std::ofstream manifest(output_directory / "frames.tsv");
        std::ofstream boundaries(output_directory / "boundaries.tsv");
        if (!manifest || !boundaries) {
            std::cerr << "full intro capture: could not write audit logs in "
                      << output_directory.string() << '\n';
            return 3;
        }
        manifest << "clip_frame\tfile\tevent\tphase\tstartup_frame\t"
                    "title_scroll\tcircuit_phase\topening_frame\tstage\t"
                    "scene\tpalette_weight\tboundary_count\n";
        boundaries << "clip_frame\tboundary\n";

        std::uint32_t clip_frame = 0U;
        std::size_t next_boundary = 0U;
        const auto capture = [&](const std::string_view event) {
            const auto scene = render_scene();
            if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
                std::cerr << "full intro capture: non-full-screen image at "
                          << clip_frame << '\n';
                return false;
            }
            auto frame_name = std::to_string(clip_frame);
            if (frame_name.size() < 6U) {
                frame_name.insert(0U, 6U - frame_name.size(), '0');
            }
            const auto filename = "frame-" + frame_name + ".png";
            const auto path = output_directory / filename;
            if (!scene.save(QString::fromStdWString(path.wstring()))) {
                std::cerr << "full intro capture: could not write "
                          << path.string() << '\n';
                return false;
            }
            manifest << clip_frame << '\t' << filename << '\t' << event
                     << '\t' << startup_integration_phase_name(state_.phase)
                     << '\t' << state_.frame.frame_count << '\t'
                     << title_loop_.animation_timer.animation_offset << '\t'
                     << static_cast<unsigned>(title_loop_.circuit_lights.phase)
                     << '\t' << state_.opening_intro.source_frame << '\t'
                     << opening_intro_stage_name(state_.opening_intro.stage)
                     << '\t'
                     << opening_intro_scene_name(state_.opening_intro.scene)
                     << '\t'
                     << static_cast<unsigned>(
                            state_.opening_intro.scene_palette_weight)
                     << '\t' << presentation_boundaries_.size() << '\n';
            while (next_boundary < presentation_boundaries_.size()) {
                boundaries << clip_frame << '\t'
                           << presentation_boundaries_[next_boundary++] << '\n';
            }
            ++clip_frame;
            return static_cast<bool>(manifest) &&
                   static_cast<bool>(boundaries);
        };
        const auto fail = [&](const std::string_view reason) {
            std::cerr << "full intro capture: " << reason << '\n';
            return 3;
        };
        const auto hold = [&](const std::string_view event,
                              const std::uint32_t frame_count) {
            for (std::uint32_t frame = 0U; frame < frame_count; ++frame) {
                if (!capture(event)) {
                    return false;
                }
                advance_one_source_frame();
            }
            return true;
        };

        std::uint32_t boot_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (boot_frames++ >= 10000U || !hold("pre_title", 1U)) {
                return fail("pre-title sequence did not reach the title");
            }
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            return fail("pre-title sequence reached an unexpected state");
        }

        if (!hold("title_idle", title_idle_frames)) {
            return fail("title did not remain active for the requested hold");
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            return fail("title left its interactive state before Start");
        }

        pending_title_controller_word_ = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(kTitleStartButton) << 8U);
        if (!capture("title_start_edge")) {
            return fail("could not capture the Start input frame");
        }
        advance_one_source_frame();
        std::uint32_t title_transition_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::title_ready) {
            if (title_transition_frames++ >= 1000U ||
                !hold("title_start_transition", 1U)) {
                return fail("title transition did not reach Start Options");
            }
        }
        if (state_.phase != StartupIntegrationPhase::start_options_shell ||
            !capture("start_options_shell")) {
            return fail("title did not produce a clean Start Options shell");
        }
        advance_one_source_frame();
        if (state_.phase != StartupIntegrationPhase::start_options ||
            !hold("start_options_new_game", 30U)) {
            return fail("Start Options did not reveal the New Game row");
        }

        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            return fail("New Game did not open archetype selection");
        }
        constexpr std::array<std::string_view, 3> kArchetypeNames{{
            "samurai", "decker", "gator_shaman"}};
        if (!hold("archetype_samurai", 30U)) {
            return fail("could not capture Samurai selection");
        }
        for (std::uint32_t index = 0U; index < selection; ++index) {
            advance_startup_integration(
                state_, kStartupMenuDown, startup_callbacks_);
            if (!hold(std::string("archetype_") +
                          std::string(kArchetypeNames[index + 1U]),
                      30U)) {
                return fail("could not capture archetype cursor movement");
            }
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        qa_finish_archetype_exit_fade();
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            return fail("selected archetype did not start the opening");
        }

        std::uint32_t opening_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro) {
            if (opening_frames++ >= 10000U || !hold("opening", 1U)) {
                return fail("opening did not reach the field handoff");
            }
        }
        if (state_.phase != StartupIntegrationPhase::field_ready ||
            !field_scene_runtime_.ready()) {
            return fail("opening did not publish a ready field session");
        }
        if (!hold("field_stable_hold", 120U)) {
            return fail("field did not survive the two-second post-intro hold");
        }

        manifest.flush();
        boundaries.flush();
        std::cout << "full_intro_capture=complete archetype="
                  << kArchetypeNames[selection]
                  << " opening_frames=" << state_.opening_intro.source_frame
                  << " captured_frames=" << clip_frame
                  << " path=" << output_directory.string() << '\n';
        return 0;
    }

    int run_opening_rotor_qa(
        const std::filesystem::path& output_directory) {
        constexpr std::uint32_t kSelection = 2U;
        constexpr std::uint32_t kFirstSampleFrame = 1827U;
        constexpr std::uint32_t kSampleInterval = 6U;
        constexpr std::size_t kPhaseCount = kTitleIntroPulsePhaseCount;
        constexpr std::size_t kSampleCount = kPhaseCount + 1U;
        constexpr auto kLastSampleFrame =
            kFirstSampleFrame +
            static_cast<std::uint32_t>(kPhaseCount) * kSampleInterval;

        if (prepare_opening_dump(kSelection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "opening-rotor-qa.tsv");
        if (!trace) {
            std::cerr << "opening rotor QA: could not write trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "sample\topening_frame\tstage_frame\tphase\tdescriptor\t"
                 "scroll_x\tpulse_x\timage\n";

        std::array<QImage, kSampleCount> images{};
        std::array<std::uint32_t, kSampleCount> descriptors{};
        std::array<std::uint16_t, kSampleCount> phases{};
        std::array<std::int16_t, kSampleCount> scroll_offsets{};
        std::array<std::uint16_t, kSampleCount> pulse_frames{};
        std::size_t sample_index = 0U;
        const auto fail = [](const std::string_view reason) {
            std::cerr << "opening rotor QA: " << reason << '\n';
            return 3;
        };

        for (std::uint32_t frame = 0U; frame <= kLastSampleFrame; ++frame) {
            if (state_.phase != StartupIntegrationPhase::opening_intro) {
                return fail("opening ended before all rotor samples were captured");
            }
            if (sample_index < kSampleCount &&
                frame == kFirstSampleFrame +
                             static_cast<std::uint32_t>(sample_index) *
                                 kSampleInterval) {
                if (state_.opening_intro.scene != OpeningIntroScene::airport ||
                    state_.opening_intro.stage !=
                        OpeningIntroStage::stage_loop) {
                    std::cerr << "opening rotor QA: frame=" << frame
                              << " scene=" << opening_intro_scene_name(
                                     state_.opening_intro.scene)
                              << " stage=" << opening_intro_stage_name(
                                     state_.opening_intro.stage)
                              << " opening_frame="
                              << state_.opening_intro.source_frame << '\n';
                    return fail("rotor sample is outside the airport stage loop");
                }
                const auto& opening = state_.opening_intro;
                const auto& pulse = opening.stage_loop.pulse;
                const auto scene = render_scene();
                if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
                    return fail("renderer did not produce a full 320x224 frame");
                }
                const auto filename =
                    "rotor-phase-" + std::to_string(sample_index) + ".bmp";
                const auto path = output_directory / filename;
                if (!scene.save(QString::fromStdWString(path.wstring()))) {
                    return fail("could not save a full-screen phase image");
                }
                images[sample_index] = scene;
                descriptors[sample_index] = pulse.sprite_descriptor;
                phases[sample_index] = pulse.phase;
                scroll_offsets[sample_index] = opening.stage_loop.scroll.x_offset;
                pulse_frames[sample_index] = pulse.current_frame;
                trace << sample_index << '\t' << frame << '\t'
                      << opening.stage_loop.current_frame << '\t'
                      << pulse.phase << '\t' << pulse.sprite_descriptor << '\t'
                      << opening.stage_loop.scroll.x_offset << '\t'
                      << pulse.x_offset << '\t' << filename << '\n';
                ++sample_index;
            }
            if (frame < kLastSampleFrame) {
                advance_one_source_frame();
            }
        }
        if (sample_index != kSampleCount) {
            return fail("did not capture all seven cycle checkpoints");
        }
        const auto actual_path = output_directory / "actual.bmp";
        if (!images[0].save(QString::fromStdWString(actual_path.wstring())) ||
            QImage(QString::fromStdWString(actual_path.wstring())) != images[0]) {
            return fail("could not write the representative actual.bmp scene dump");
        }

        const auto& expected_descriptors =
            opening_intro_source_plan().pulse_descriptors;
        std::size_t first_phase = kPhaseCount;
        for (std::size_t phase = 0U; phase < kPhaseCount; ++phase) {
            if (expected_descriptors[phase] == descriptors[0]) {
                first_phase = phase;
                break;
            }
        }
        if (first_phase == kPhaseCount) {
            return fail("active sprite is not one of the six ROM phase assets");
        }
        for (std::size_t sample = 0U; sample < kSampleCount; ++sample) {
            const auto expected_phase = (first_phase + sample) % kPhaseCount;
            if (descriptors[sample] != expected_descriptors[expected_phase] ||
                phases[sample] !=
                    static_cast<std::uint16_t>((phases[0] + sample) %
                                               kPhaseCount) ||
                scroll_offsets[sample] != static_cast<std::int16_t>(
                    scroll_offsets[0] - static_cast<std::int16_t>(
                                            sample * kSampleInterval * 2U)) ||
                (sample != 0U && pulse_frames[sample] !=
                    static_cast<std::uint16_t>(pulse_frames[0] +
                                               sample * kSampleInterval))) {
                return fail("six-frame descriptor or two-pixel scroll cadence regressed");
            }
            if (sample > 0U && sample < kPhaseCount &&
                descriptors[sample] == descriptors[sample - 1U]) {
                return fail("two adjacent samples repeated a rotor phase");
            }
        }

        const auto shifted_difference_count =
            [](const QImage& first, const QImage& later, const int y_begin,
               const int y_end, const int left_shift) {
                int differences = 0;
                for (int y = y_begin; y < y_end; ++y) {
                    for (int x = left_shift; x < first.width(); ++x) {
                        if (first.pixel(x, y) != later.pixel(x - left_shift, y)) {
                            ++differences;
                        }
                    }
                }
                return differences;
            };
        for (std::size_t sample = 1U; sample < kPhaseCount; ++sample) {
            const auto rotor_differences = shifted_difference_count(
                images[sample - 1U], images[sample], 100, 112, 12);
            const auto body_differences = shifted_difference_count(
                images[sample - 1U], images[sample], 112, 176, 12);
            if (rotor_differences == 0 || body_differences != 0) {
                return fail("rotor strip did not change cleanly over a stable plane body");
            }
        }
        if (shifted_difference_count(images[0], images[kPhaseCount],
                                     100, 176, 72) != 0) {
            return fail("rotor and plane did not return to the same appearance after 36 frames");
        }

        trace << "result\t6 distinct ROM phases\t6-frame cadence\t"
                 "36-frame repeat\tplane body stable\n";
        std::cout << "opening_rotor_qa=passed phases=" << kPhaseCount
                  << " cadence=" << kSampleInterval
                  << " cycle=" << kPhaseCount * kSampleInterval
                  << " actual="
                  << actual_path.string()
                  << '\n';
        return 0;
    }

    int run_opening_landing_qa(
        const std::filesystem::path& output_directory) {
        constexpr std::uint32_t kSelection = 2U;
        constexpr std::array<std::uint16_t, 20> kElapsedCheckpoints{{
            225U, 549U, 550U, 585U, 586U, 612U, 675U, 676U, 732U, 747U,
            748U, 765U, 766U, 792U, 801U, 802U, 819U, 820U, 821U, 852U}};

        if (prepare_opening_dump(kSelection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "opening-landing-qa.tsv");
        if (!trace) {
            std::cerr << "opening landing QA: could not write trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "elapsed\topening_frame\tstage_loop_frame\tphase\t"
                 "next_event\tx_accumulator\ty_accumulator\tscreen_y\t"
                 "descriptor_variant\trequest_x\trequest_y\timage\n";

        std::array<QImage, kElapsedCheckpoints.size()> images{};
        std::array<OpeningIntroSpriteRequest, kElapsedCheckpoints.size()>
            requests{};
        std::array<bool, kElapsedCheckpoints.size()> has_request{};
        std::size_t sample_index = 0U;
        const auto fail = [](const std::string_view reason) {
            std::cerr << "opening landing QA: " << reason << '\n';
            return 3;
        };
        constexpr std::uint32_t kMaxFramesAfterSelection = 10000U;
        for (std::uint32_t frame = 0U;
             frame < kMaxFramesAfterSelection &&
             sample_index < kElapsedCheckpoints.size(); ++frame) {
            if (state_.phase != StartupIntegrationPhase::opening_intro) {
                return fail("opening ended before all landing checkpoints");
            }
            const auto& opening = state_.opening_intro;
            if (opening.scene == OpeningIntroScene::airport &&
                opening.stage == OpeningIntroStage::stage_loop) {
                const auto& landing = opening.stage_loop.landing;
                const auto elapsed = static_cast<std::uint16_t>(
                    landing.current_frame - landing.animation_start_frame);
                if (elapsed == kElapsedCheckpoints[sample_index]) {
                    const auto scene = render_scene();
                    if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
                        return fail("renderer did not produce a full 320x224 frame");
                    }
                    const auto filename = "landing-elapsed-" +
                                          std::to_string(elapsed) + ".bmp";
                    const auto path = output_directory / filename;
                    if (!scene.save(QString::fromStdWString(path.wstring()))) {
                        return fail("could not save a full-screen landing frame");
                    }
                    images[sample_index] = scene;
                    const auto movement = std::find_if(
                        opening_sprites_.rbegin(), opening_sprites_.rend(),
                        [](const OpeningIntroSpriteRequest& request) {
                            return opening_intro_sprite_group(
                                       request.sprite_descriptor)
                                       .family ==
                                   OpeningIntroSpriteFamily::aircraft_movement;
                        });
                    if (movement != opening_sprites_.rend()) {
                        requests[sample_index] = *movement;
                        has_request[sample_index] = true;
                    }
                    const auto base_y = static_cast<std::uint16_t>(
                        landing.y_accumulator >> 8U);
                    const auto screen_y = static_cast<std::uint16_t>(
                        base_y + 0x28U);
                    const auto variant = has_request[sample_index]
                        ? opening_intro_sprite_group(
                              requests[sample_index].sprite_descriptor)
                              .variant
                        : 0xFFU;
                    trace << elapsed << '\t' << opening.source_frame << '\t'
                          << opening.stage_loop_frame << '\t'
                          << landing.phase << '\t' << landing.next_event_frame
                          << '\t' << landing.x_accumulator << '\t'
                          << landing.y_accumulator << '\t' << screen_y << '\t'
                          << static_cast<unsigned>(variant) << '\t'
                          << (has_request[sample_index]
                                  ? requests[sample_index].x
                                  : 0xFFFFU)
                          << '\t'
                          << (has_request[sample_index]
                                  ? requests[sample_index].y
                                  : 0xFFFFU)
                          << '\t' << filename << '\n';
                    ++sample_index;
                }
            }
            if (sample_index < kElapsedCheckpoints.size()) {
                advance_one_source_frame();
            }
        }
        if (sample_index != kElapsedCheckpoints.size()) {
            return fail("did not capture every event and threshold checkpoint");
        }

        constexpr std::array<std::uint8_t, 5> kExpectedVariants{{
            0U, 0U, 0U, 1U, 0U}};
        constexpr std::array<std::size_t, 5> kVariantSampleIndices{{
            2U, 5U, 8U, 13U, 19U}};
        for (std::size_t sample = 0U;
             sample < kVariantSampleIndices.size(); ++sample) {
            const auto index = kVariantSampleIndices[sample];
            if (!has_request[index]) {
                return fail("landing sprite is absent at an active checkpoint");
            }
            const auto group = opening_intro_sprite_group(
                requests[index].sprite_descriptor);
            if (group.family != OpeningIntroSpriteFamily::aircraft_movement ||
                group.variant != kExpectedVariants[sample]) {
                return fail("sprite selection does not follow the ROM Y window");
            }
        }
        if (has_request[0] || has_request[1] ||
            !images.back().save(QString::fromStdWString(
                (output_directory / "actual.bmp").wstring())) ||
            QImage(QString::fromStdWString(
                (output_directory / "actual.bmp").wstring())) != images.back()) {
            return fail("lead-in clearing or actual.bmp verification failed");
        }
        trace << "result\t20 full-screen checkpoints\tROM Y-window variants\t"
                 "lead-in clear\tactual.bmp readback\n";
        std::cout << "opening_landing_qa=passed checkpoints="
                  << kElapsedCheckpoints.size() << " actual="
                  << (output_directory / "actual.bmp").string() << '\n';
        return 0;
    }

    int run_opening_wilderness_pan_qa(
        const std::filesystem::path& output_directory) {
        constexpr std::uint32_t kSelection = 2U;
        constexpr std::uint16_t kPanFrames = 0x0040U;
        constexpr std::size_t kMinimumBodyPixelMatches = 1400U;
        constexpr std::string_view kMapRightAlias =
            "intro.sprites.ambush.map.right.2.0";

        if (prepare_opening_dump(kSelection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "opening-wilderness-pan-qa.tsv");
        if (!trace) {
            std::cerr << "opening wilderness pan QA: could not write trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "sample\tpan_frame\topening_frame\tx_scroll\ty_scroll\t"
                 "head_x\thead_y\tx_relative\ty_relative\t"
                 "body_pixel_matches\timage\n";

        const auto fail = [](const std::string_view reason) {
            std::cerr << "opening wilderness pan QA: " << reason << '\n';
            return 3;
        };
        constexpr std::uint32_t kMaxFramesToPan = 4000U;
        std::uint32_t frames_to_pan = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               (state_.opening_intro.scene != OpeningIntroScene::wilderness ||
                state_.opening_intro.stage !=
                    OpeningIntroStage::stage2_first_loop)) {
            if (frames_to_pan++ >= kMaxFramesToPan) {
                return fail("did not reach the source wilderness pan loop");
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            state_.opening_intro.scene != OpeningIntroScene::wilderness ||
            state_.opening_intro.stage !=
                OpeningIntroStage::stage2_first_loop) {
            return fail("opening ended before the wilderness pan loop");
        }

        const auto* map_record =
            state_.asset_catalog.catalog.find(std::string(kMapRightAlias));
        const auto map_image = draw_asset_cache_.find(std::string(kMapRightAlias));
        if (map_record == nullptr || map_image == draw_asset_cache_.end() ||
            map_image->second.isNull()) {
            return fail("the named wilderness body asset is missing");
        }
        std::size_t colored_body_pixels = 0U;
        for (int y = 0; y < map_image->second.height(); ++y) {
            for (int x = 0; x < map_image->second.width(); ++x) {
                const auto color = map_image->second.pixelColor(x, y);
                if (color.alpha() != 0 &&
                    (color.red() != 0 || color.green() != 0 ||
                     color.blue() != 0)) {
                    ++colored_body_pixels;
                }
            }
        }
        // ROM 0x5B2C2 fills entity palette line 1 with CRAM word 0x0EEE
        // before the flash sequence steps it. Verify that source word
        // directly instead of its removed linear RGB weight [C].
        if (colored_body_pixels < kMinimumBodyPixelMatches ||
            opening_entity_palette_color_word(1U) != 0x0EEEU) {
            return fail("the source body art is not fully visible for pixel checks");
        }

        constexpr std::array<std::uint16_t, 3> kSamples{{0U, 32U, 64U}};
        constexpr std::array<std::string_view, 3> kNames{{
            "pan-start", "pan-midpoint", "pan-settled"}};
        for (std::size_t sample = 0U; sample < kSamples.size(); ++sample) {
            const auto pan_frame = kSamples[sample];
            if (sample != 0U) {
                const auto previous_frame = kSamples[sample - 1U];
                for (auto frame = previous_frame; frame < pan_frame; ++frame) {
                    advance_one_source_frame();
                    if (state_.phase !=
                            StartupIntegrationPhase::opening_intro ||
                        state_.opening_intro.scene !=
                            OpeningIntroScene::wilderness ||
                        state_.opening_intro.stage !=
                            OpeningIntroStage::stage2_first_loop) {
                        return fail("wilderness pan left its source loop early");
                    }
                }
            }

            const auto& opening = state_.opening_intro;
            const auto x_relative =
                static_cast<std::int32_t>(opening.stage2.x_phase) -
                static_cast<std::int32_t>(opening.stage2.x_offset);
            const auto y_relative =
                static_cast<std::int32_t>(opening.stage2.y_phase) +
                static_cast<std::int32_t>(opening.stage2.y_offset);
            if (x_relative != 0x0178 || y_relative != 0x00D1 ||
                opening.stage2.x_offset !=
                    static_cast<std::int16_t>(-0x0040 + pan_frame) ||
                opening.stage2.x_phase !=
                    static_cast<std::uint16_t>(0x0138U + pan_frame)) {
                return fail("plane scroll and head SAT coordinates diverged from the ROM pan");
            }

            const auto scene = render_scene();
            if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
                return fail("renderer did not produce a full 320x224 frame");
            }
            std::size_t body_matches = 0U;
            const auto body_x = map_record->origin_x +
                                opening.stage2.x_offset;
            const auto body_y = map_record->origin_y -
                                opening.stage2.y_offset;
            for (int y = 0; y < map_image->second.height(); ++y) {
                for (int x = 0; x < map_image->second.width(); ++x) {
                    const auto expected = map_image->second.pixelColor(x, y);
                    if (expected.alpha() == 0 ||
                        (expected.red() == 0 && expected.green() == 0 &&
                         expected.blue() == 0)) {
                        continue;
                    }
                    const auto screen_x = body_x + x;
                    const auto screen_y = body_y + y;
                    if (screen_x >= 0 && screen_x < scene.width() &&
                        screen_y >= 0 && screen_y < scene.height() &&
                        scene.pixelColor(screen_x, screen_y) == expected) {
                        ++body_matches;
                    }
                }
            }
            if (body_matches < kMinimumBodyPixelMatches) {
                std::cerr << "opening wilderness pan QA: sample="
                          << kNames[sample] << " scroll="
                          << opening.stage2.x_offset << ','
                          << opening.stage2.y_offset << " body matches="
                          << body_matches << '/' << colored_body_pixels << '\n';
                return fail("name-table body pixels did not follow the source scroll");
            }

            const auto filename = std::string(kNames[sample]) + ".bmp";
            if (!scene.save(QString::fromStdWString(
                    (output_directory / filename).wstring()))) {
                return fail("could not save a full-screen pan checkpoint");
            }
            trace << kNames[sample] << '\t' << pan_frame << '\t'
                  << opening.source_frame << '\t'
                  << opening.stage2.x_offset << '\t'
                  << opening.stage2.y_offset << '\t'
                  << opening.stage2.x_phase << '\t'
                  << opening.stage2.y_phase << '\t' << x_relative << '\t'
                  << y_relative << '\t' << body_matches << '\t'
                  << filename << '\n';
            if (sample + 1U == kSamples.size() &&
                (!scene.save(QString::fromStdWString(
                     (output_directory / "actual.bmp").wstring())) ||
                 QImage(QString::fromStdWString(
                     (output_directory / "actual.bmp").wstring())) != scene)) {
                return fail("settled whole-screen actual.bmp readback failed");
            }
        }

        std::cout << "opening_wilderness_pan_qa=passed checkpoints="
                  << kSamples.size() << " body_pixels=" << colored_body_pixels
                  << " actual="
                  << (output_directory / "actual.bmp").string() << '\n';
        return 0;
    }

    int run_opening_ambush_flash_qa(
        const std::filesystem::path& output_directory) {
        constexpr std::uint32_t kSelection = 2U;
        constexpr std::uint32_t kMaximumFrames = 4000U;
        constexpr std::string_view kMapRightAlias =
            "intro.sprites.ambush.map.right.2.0";

        if (prepare_opening_dump(kSelection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "opening-ambush-flash-qa.tsv");
        if (!trace) {
            std::cerr << "opening ambush flash QA: could not write trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "sample\topening_frame\tpersistent_stage\ttimed_stage\t"
                 "palette_word\timage\n";
        const auto fail = [](const std::string_view reason) {
            std::cerr << "opening ambush flash QA: " << reason << '\n';
            return 3;
        };
        const auto save_frame = [&](const std::string& name,
                                    const QImage& scene) {
            if (scene.size() != QSize(kSceneWidth, kSceneHeight) ||
                !scene.save(QString::fromStdWString(
                    (output_directory / name).wstring()))) {
                return false;
            }
            const auto& opening = state_.opening_intro;
            trace << name << '\t' << opening.source_frame << '\t'
                  << opening.persistent.stage << '\t'
                  << opening.timed.descriptor_index << '\t' << std::hex
                  << opening.entity_colors.words[1U][0U] << std::dec << '\t'
                  << name << '\n';
            return true;
        };
        const auto has_magic_flash_sprite = [this] {
            return std::any_of(
                opening_sprites_.begin(), opening_sprites_.end(),
                [](const OpeningIntroSpriteRequest& request) {
                    return opening_intro_sprite_group(request.sprite_descriptor)
                               .family == OpeningIntroSpriteFamily::ambush_flash;
                });
        };

        std::uint32_t frames = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               (state_.opening_intro.scene != OpeningIntroScene::wilderness ||
                state_.opening_intro.stage !=
                    OpeningIntroStage::stage2_second_loop)) {
            if (frames++ >= kMaximumFrames) {
                return fail("did not reach the connected wilderness animation loop");
            }
            advance_one_source_frame();
        }
        bool saw_magic_flash_before_death = false;
        bool saw_magic_flash_advance = false;
        std::uint32_t first_magic_descriptor = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               state_.opening_intro.persistent.stage < 4U) {
            if (frames++ >= kMaximumFrames) {
                return fail("did not reach the source hit-flash transition");
            }
            advance_one_source_frame();
            const auto& opening = state_.opening_intro;
            const auto magic_count = std::count_if(
                opening_sprites_.begin(), opening_sprites_.end(),
                [](const OpeningIntroSpriteRequest& request) {
                    return opening_intro_sprite_group(request.sprite_descriptor)
                               .family == OpeningIntroSpriteFamily::ambush_flash;
                });
            if (opening.dual.active && magic_count == 2) {
                saw_magic_flash_before_death = true;
                if (first_magic_descriptor == 0U) {
                    first_magic_descriptor =
                        opening.dual.left_sprite_descriptor;
                    if (!save_frame("magic-before-death-0.bmp",
                                    render_scene())) {
                        return fail("could not save the live magic effect");
                    }
                } else if (!saw_magic_flash_advance &&
                           opening.dual.left_sprite_descriptor !=
                               first_magic_descriptor) {
                    saw_magic_flash_advance = true;
                    if (!save_frame("magic-before-death-1.bmp",
                                    render_scene())) {
                        return fail("could not save the advancing magic effect");
                    }
                }
            }
        }
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            state_.opening_intro.scene != OpeningIntroScene::wilderness ||
            state_.opening_intro.persistent.stage != 4U ||
            !saw_magic_flash_before_death || !saw_magic_flash_advance ||
            !state_.opening_intro.dual.active || !has_magic_flash_sprite()) {
            return fail("source persistent stage 4 was skipped");
        }

        const auto* map_record =
            state_.asset_catalog.catalog.find(std::string(kMapRightAlias));
        const auto map_image =
            draw_asset_cache_.find(std::string(kMapRightAlias));
        if (map_record == nullptr || map_image == draw_asset_cache_.end() ||
            map_image->second.isNull()) {
            return fail("the named ambush actor layer is missing");
        }
        const auto before_flash = render_scene();
        if (opening_ambush_palette_flash_active() ||
            !save_frame("actor-before-flash.bmp", before_flash)) {
            return fail("the pre-hit actor state was not saved cleanly");
        }
        // The right actor plate shares line 1 with the separately submitted
        // head, but ROM 0x5A2D0 does not install its white hit palette until
        // persistent stage 4. Before that event, keep enough of the named
        // sprite's original pixels visible to catch a stuck/early white fill.
        const QColor white(255, 255, 255);
        const auto pre_hit_body_x = map_record->origin_x +
                                    state_.opening_intro.stage2.x_offset;
        const auto pre_hit_body_y = map_record->origin_y -
                                    state_.opening_intro.stage2.y_offset;
        std::size_t normal_body_pixels = 0U;
        for (int y = 0; y < map_image->second.height(); ++y) {
            for (int x = 0; x < map_image->second.width(); ++x) {
                if (map_image->second.pixelColor(x, y).alpha() == 0) {
                    continue;
                }
                const auto screen_x = pre_hit_body_x + x;
                const auto screen_y = pre_hit_body_y + y;
                if (screen_x >= 0 && screen_x < before_flash.width() &&
                    screen_y >= 0 && screen_y < before_flash.height() &&
                    before_flash.pixelColor(screen_x, screen_y) != white) {
                    ++normal_body_pixels;
                }
            }
        }
        if (normal_body_pixels < 500U) {
            return fail("the pre-hit orc plate rendered as a white silhouette");
        }

        frames = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               state_.opening_intro.persistent.stage < 5U) {
            if (frames++ >= 32U) {
                return fail("the stage-4 palette event did not fire on schedule");
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            state_.opening_intro.persistent.stage != 5U ||
            !opening_ambush_palette_flash_active() ||
            state_.opening_intro.dual.active || has_magic_flash_sprite() ||
            !opening_wilderness_map_right_ ||
            opening_wilderness_map_center_) {
            return fail("the source white-hit state did not stop the magic sprites and connect to the renderer");
        }
        const auto& flash_palette =
            state_.opening_intro.entity_colors.words[1U];
        if (!std::all_of(flash_palette.begin(), flash_palette.end(),
                         [](const std::uint16_t word) {
                             return word == 0x0EEEU;
                         })) {
            return fail("ROM 0x5A2D0 did not publish fifteen white entries");
        }

        const auto hit_flash = render_scene();
        if (!save_frame("actor-hit-flash.bmp", hit_flash)) {
            return fail("could not save the full-screen hit-flash frame");
        }
        const auto hit_group = opening_intro_sprite_group(
            state_.opening_intro.persistent.current_sprite_descriptor);
        if (hit_group.family != OpeningIntroSpriteFamily::ambush_actor) {
            return fail("the active persistent actor handle is not the head layer");
        }
        std::size_t head_opaque_pixels = 0U;
        std::size_t white_head_pixels = 0U;
        std::size_t normal_head_pixels = 0U;
        const auto head_prefix = opening_intro_sprite_alias_prefix(hit_group);
        for (const auto& record : state_.asset_catalog.catalog.records) {
            if (record.alias.rfind(head_prefix, 0) != 0) {
                continue;
            }
            const auto image = draw_asset_cache_.find(record.alias);
            if (image == draw_asset_cache_.end()) {
                continue;
            }
            const auto point = opening_intro_sat_draw_point(
                0x0178U, 0x00D1U, record.origin_x, record.origin_y);
            for (int y = 0; y < image->second.height(); ++y) {
                for (int x = 0; x < image->second.width(); ++x) {
                    if (image->second.pixelColor(x, y).alpha() == 0) {
                        continue;
                    }
                    const auto screen_x = point.x + x;
                    const auto screen_y = point.y + y;
                    if (screen_x < 0 || screen_x >= hit_flash.width() ||
                        screen_y < 0 || screen_y >= hit_flash.height()) {
                        continue;
                    }
                    ++head_opaque_pixels;
                    const auto color = hit_flash.pixelColor(screen_x, screen_y);
                    if (color == white) {
                        ++white_head_pixels;
                    }
                    if (before_flash.pixelColor(screen_x, screen_y) != white) {
                        ++normal_head_pixels;
                    }
                }
            }
        }

        std::size_t white_body_pixels = 0U;
        const auto body_x = map_record->origin_x +
                            state_.opening_intro.stage2.x_offset;
        const auto body_y = map_record->origin_y -
                            state_.opening_intro.stage2.y_offset;
        for (int y = 0; y < map_image->second.height(); ++y) {
            for (int x = 0; x < map_image->second.width(); ++x) {
                if (map_image->second.pixelColor(x, y).alpha() == 0) {
                    continue;
                }
                const auto screen_x = body_x + x;
                const auto screen_y = body_y + y;
                if (screen_x >= 0 && screen_x < hit_flash.width() &&
                    screen_y >= 0 && screen_y < hit_flash.height() &&
                    hit_flash.pixelColor(screen_x, screen_y) == white) {
                    ++white_body_pixels;
                }
            }
        }
        if (head_opaque_pixels < 20U || white_head_pixels < 20U ||
            normal_head_pixels < 20U || white_body_pixels < 500U) {
            const auto map_group = opening_wilderness_map_right_
                                       ? opening_intro_sprite_group(
                                             opening_wilderness_map_right_->sprite_descriptor)
                                       : OpeningIntroSpriteGroup{};
            std::cerr << "opening ambush flash QA: head white="
                      << white_head_pixels << '/' << head_opaque_pixels
                      << " pre-flash colored=" << normal_head_pixels
                      << " body white=" << white_body_pixels
                      << " map_family=" << static_cast<unsigned>(map_group.family)
                      << " map_origin=" << map_record->origin_x << ','
                      << map_record->origin_y << " scroll="
                      << state_.opening_intro.stage2.x_offset << ','
                      << state_.opening_intro.stage2.y_offset << '\n';
            return fail("the head and actor plate did not flash together");
        }
        if (!hit_flash.save(QString::fromStdWString(
                (output_directory / "actual.bmp").wstring()))) {
            return fail("could not save the full-screen hit-flash frame");
        }
        const QImage actual_readback(QString::fromStdWString(
            (output_directory / "actual.bmp").wstring()));
        if (actual_readback.convertToFormat(QImage::Format_RGB32) !=
            hit_flash.convertToFormat(QImage::Format_RGB32)) {
            return fail("actual.bmp did not read back as the hit-flash scene");
        }

        frames = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               state_.opening_intro.entity_colors.words[1U][0U] == 0x0EEEU) {
            if (frames++ >= 10U) {
                return fail("the first nine-frame CRAM step did not occur");
            }
            advance_one_source_frame();
        }
        const auto& stepped_palette =
            state_.opening_intro.entity_colors.words[1U];
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            !std::all_of(stepped_palette.begin(), stepped_palette.end(),
                         [](const std::uint16_t word) {
                             return word == 0x0CCCU;
                         }) ||
            !save_frame("actor-flash-step-1.bmp", render_scene())) {
            return fail("the first palette step was not a uniform 0xCCC gray");
        }

        frames = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               state_.opening_intro.persistent.stage < 9U) {
            if (frames++ >= 40U) {
                return fail("the timed body effect did not start at stage 9");
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            state_.opening_intro.persistent.stage != 9U) {
            return fail("the source timed body anchor was skipped");
        }

        std::array<bool, kTitleIntroTimedSpriteCount> effect_frames{};
        std::array<std::uint32_t, kTitleIntroTimedSpriteCount> effect_source_frames{};
        std::uint32_t effect_updates = 0U;
        bool final_head_fade_saved = false;
        while (std::any_of(effect_frames.begin(), effect_frames.end(),
                           [](const bool seen) { return !seen; }) &&
               effect_updates++ < 64U) {
            const auto group = opening_intro_sprite_group(
                state_.opening_intro.timed.current_sprite_descriptor);
            if (group.family == OpeningIntroSpriteFamily::ambush_body &&
                group.variant < effect_frames.size() &&
                !effect_frames[group.variant]) {
                const auto scene = render_scene();
                const auto name = "ambush-effect-" +
                                  std::to_string(group.variant) + ".bmp";
                if (!save_frame(name, scene)) {
                    return fail("could not save a timed body-effect frame");
                }
                effect_frames[group.variant] = true;
                effect_source_frames[group.variant] =
                    state_.opening_intro.source_frame;
            }
            advance_one_source_frame();
            if (!final_head_fade_saved &&
                state_.opening_intro.persistent.stage == 12U) {
                const auto final_head = std::find_if(
                    opening_sprites_.begin(), opening_sprites_.end(),
                    [](const OpeningIntroSpriteRequest& request) {
                        return opening_intro_sprite_group(
                                   request.sprite_descriptor)
                                   .family ==
                               OpeningIntroSpriteFamily::ambush_actor;
                    });
                const auto& final_palette =
                    state_.opening_intro.entity_colors.words[1U];
                if (final_head == opening_sprites_.end() ||
                    !opening_ambush_palette_flash_active() ||
                    !std::all_of(final_palette.begin(), final_palette.end(),
                                 [](const std::uint16_t word) {
                                     return word == 0U;
                                 }) ||
                    !save_frame("actor-final-head-fade.bmp",
                                render_scene())) {
                    return fail("stage 12 did not draw the head with its final black palette before returning");
                }
                final_head_fade_saved = true;
            }
        }
        if (!std::all_of(effect_frames.begin(), effect_frames.end(),
                         [](const bool seen) { return seen; }) ||
            !final_head_fade_saved) {
            return fail("the six source explosion/smoke descriptors were incomplete");
        }
        for (std::size_t index = 1U; index < effect_source_frames.size();
             ++index) {
            if (effect_source_frames[index] - effect_source_frames[index - 1U] !=
                8U) {
                return fail("timed explosion/smoke frames did not keep the eight-frame cadence");
            }
        }

        frames = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               state_.opening_intro.persistent.stage < 12U) {
            if (frames++ >= 40U) {
                return fail("the persistent actor did not reach its clear event");
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            state_.opening_intro.persistent.stage != 12U ||
            opening_wilderness_map_right_ ||
            opening_wilderness_map_center_ ||
            std::any_of(opening_sprites_.begin(), opening_sprites_.end(),
                        [](const OpeningIntroSpriteRequest& request) {
                            return opening_intro_sprite_group(request.sprite_descriptor)
                                       .family == OpeningIntroSpriteFamily::ambush_actor;
                        })) {
            return fail("the source right-map and head layers were not cleanly retired");
        }
        const auto cleared = render_scene();
        if (!save_frame("actor-after-clear.bmp", cleared)) {
            return fail("could not save the complete post-hit frame");
        }
        std::cout << "opening_ambush_flash_qa=passed head_pixels="
                  << white_head_pixels << '/' << head_opaque_pixels
                  << " pre_hit_actor_nonwhite_pixels=" << normal_body_pixels
                  << " body_white_pixels=" << white_body_pixels
                  << " effect_frames=" << effect_frames.size() << " actual="
                  << (output_directory / "actual.bmp").string() << '\n';
        return 0;
    }

    int run_opening_death_layer_qa(
        const std::filesystem::path& output_directory) {
        constexpr std::uint32_t kSelection = 2U;
        constexpr std::uint32_t kDeathOverlayFrame = 1209U;
        // 0x5AA96 has no fade-out, so the tri-vid replaces the wilderness
        // right after the ambush loop (no checkpoints inside a fade).
        constexpr std::array<std::uint16_t, 10> kCheckpoints{{
            688U, 752U, 850U, 1021U, 1093U, 1208U,
            1209U, 1210U, 1211U, 1266U}};

        if (prepare_opening_dump(kSelection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "opening-death-layer-qa.tsv");
        if (!trace) {
            std::cerr << "opening death-layer QA: could not write trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "opening_frame\tstage\tscene\tmap_left\toverlay_variant\t"
                 "final_overlay\tleft_edge_pixels\timage\n";

        const auto fail = [](const std::string_view reason) {
            std::cerr << "opening death-layer QA: " << reason << '\n';
            return 3;
        };
        const auto left_edge_pixel_count = [](const QImage& scene) {
            std::size_t count = 0U;
            for (int y = 0; y < scene.height(); ++y) {
                for (int x = 0; x < 24; ++x) {
                    const auto color = scene.pixelColor(x, y);
                    if (color.red() != 0 || color.green() != 0 ||
                        color.blue() != 0) {
                        ++count;
                    }
                }
            }
            return count;
        };
        const auto save_frame = [&](const QImage& scene,
                                    const std::string& name) {
            return scene.save(QString::fromStdWString(
                (output_directory / name).wstring()));
        };

        std::size_t next_checkpoint = 0U;
        std::size_t checked_death_frames = 0U;
        bool saw_death_overlay = false;
        bool saw_clean_trivid_entry = false;
        constexpr std::uint32_t kMaximumOpeningFrames = 3000U;
        for (std::uint32_t frame = 0U; frame < kMaximumOpeningFrames; ++frame) {
            if (state_.phase != StartupIntegrationPhase::opening_intro) {
                return fail("opening ended before the cockpit handoff");
            }

            const auto& opening = state_.opening_intro;
            if (opening.scene == OpeningIntroScene::trivid) {
                if (!saw_death_overlay || opening_wilderness_map_left_ ||
                    opening_wilderness_map_center_ ||
                    opening_wilderness_map_right_ ||
                    opening_wilderness_overlay_ || opening_wilderness_patch_ ||
                    opening_wilderness_final_overlay_) {
                    return fail("wilderness art survived the cockpit scene replacement");
                }
                const auto scene = render_scene();
                if (scene.size() != QSize(kSceneWidth, kSceneHeight) ||
                    !save_frame(scene, "trivid-entry.bmp")) {
                    return fail("could not save a full-screen cockpit entry frame");
                }
                trace << opening.source_frame << '\t'
                      << opening_intro_stage_name(opening.stage) << '\t'
                      << opening_intro_scene_name(opening.scene)
                      << "\tcleared\t-\tno\t"
                      << left_edge_pixel_count(scene)
                      << "\ttrivid-entry.bmp\n";
                saw_clean_trivid_entry = true;
                break;
            }

            if (opening.scene == OpeningIntroScene::wilderness &&
                opening.source_frame >= kDeathOverlayFrame &&
                opening_wilderness_final_overlay_) {
                if (opening.persistent.stage >= 5U &&
                    (opening.dual.active ||
                     std::any_of(
                         opening_sprites_.begin(), opening_sprites_.end(),
                         [](const OpeningIntroSpriteRequest& request) {
                             return opening_intro_sprite_group(
                                        request.sprite_descriptor)
                                        .family ==
                                    OpeningIntroSpriteFamily::ambush_flash;
                         }))) {
                    return fail("magic sprites continued after the hit/death transition");
                }
                if (opening_wilderness_map_left_ ||
                    opening_wilderness_overlay_) {
                    return fail("the source-cleared left actor plate was reintroduced during the death pose");
                }
                ++checked_death_frames;

                if (opening.source_frame == kDeathOverlayFrame) {
                    const auto scene = render_scene();
                    if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
                        return fail("renderer did not produce a full 320x224 frame");
                    }
                    const auto stray_pixels = left_edge_pixel_count(scene);
                    if (stray_pixels != 0U) {
                        std::cerr << "opening death-layer QA: opening_frame="
                                  << opening.source_frame
                                  << " stray_left_edge_pixels=" << stray_pixels
                                  << '\n';
                        return fail("the left actor plate remains visible on the frame the ROM clears it");
                    }
                    const auto final_group = opening_intro_sprite_group(
                        opening_wilderness_final_overlay_->sprite_descriptor);
                    std::size_t death_pose_pixels = 0U;
                    for (int y = 120; y < scene.height(); ++y) {
                        for (int x = 24; x < 184; ++x) {
                            const auto color = scene.pixelColor(x, y);
                            if (color.red() != 0 || color.green() != 0 ||
                                color.blue() != 0) {
                                ++death_pose_pixels;
                            }
                        }
                    }
                    if (opening.stage !=
                            OpeningIntroStage::stage2_second_loop ||
                        opening_wilderness_map_left_ ||
                        opening_wilderness_overlay_ ||
                        final_group.family !=
                            OpeningIntroSpriteFamily::wilderness_final_overlay ||
                        death_pose_pixels < 100U) {
                        return fail("typed overlay state or death-pose pixels do not match the ROM event");
                    }
                    const auto actual_path = output_directory / "actual.bmp";
                    if (!scene.save(QString::fromStdWString(
                            actual_path.wstring())) ||
                        QImage(QString::fromStdWString(
                            actual_path.wstring())) != scene) {
                        return fail("death-pose actual.bmp readback failed");
                    }
                    saw_death_overlay = true;
                }
            }

            while (next_checkpoint < kCheckpoints.size() &&
                   opening.source_frame == kCheckpoints[next_checkpoint]) {
                const auto scene = render_scene();
                if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
                    return fail("checkpoint is not a full 320x224 screen");
                }
                const auto name = "opening-" +
                    std::to_string(opening.source_frame) + ".bmp";
                if (!save_frame(scene, name)) {
                    return fail("could not save a full-screen event checkpoint");
                }
                const auto left_pixels = left_edge_pixel_count(scene);
                const auto overlay_group = opening_wilderness_overlay_
                    ? opening_intro_sprite_group(
                          opening_wilderness_overlay_->sprite_descriptor)
                    : OpeningIntroSpriteGroup{};
                trace << opening.source_frame << '\t'
                      << opening_intro_stage_name(opening.stage) << '\t'
                      << opening_intro_scene_name(opening.scene) << '\t'
                      << (opening_wilderness_map_left_ ? "present" : "cleared")
                      << '\t'
                      << (overlay_group.family ==
                                  OpeningIntroSpriteFamily::wilderness_overlay
                              ? std::to_string(overlay_group.variant)
                              : "-")
                      << '\t'
                      << (opening_wilderness_final_overlay_ ? "yes" : "no")
                      << '\t' << left_pixels << '\t' << name << '\n';
                ++next_checkpoint;
            }

            advance_one_source_frame();
        }

        if (!saw_death_overlay || !saw_clean_trivid_entry ||
            checked_death_frames == 0U ||
            next_checkpoint != kCheckpoints.size()) {
            return fail("did not cover the death pose, its full lifetime, and cockpit replacement");
        }
        std::cout << "opening_death_layer_qa=passed checkpoints="
                  << kCheckpoints.size()
                  << " death_frames=" << checked_death_frames
                  << " actual="
                  << (output_directory / "actual.bmp").string() << '\n';
        return 0;
    }

    int run_opening_trivid_layer_qa(
        const std::filesystem::path& output_directory) {
        constexpr std::uint32_t kSelection = 2U;
        constexpr std::uint16_t kStageExitLoopFrame = 0x0114U;
        constexpr std::uint16_t kMovingMidpointLoopFrame = 0x0080U;

        if (prepare_opening_dump(kSelection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "opening-trivid-layer-qa.tsv");
        if (!trace) {
            std::cerr << "opening trivid layer QA: could not write trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "sample\tloop_frame\topening_frame\ttimed_phase\ttimed_x\t"
                 "timed_y\tprojection_x\tprojection_y\tlow_plane_pixels\t"
                 "low_hole_pixels\thigh_sprite_pixels\timage\n";

        const auto fail = [](const std::string_view reason) {
            std::cerr << "opening trivid layer QA: " << reason << '\n';
            return 3;
        };
        constexpr std::uint32_t kMaxFramesToTrivid = 5000U;
        std::uint32_t frames_to_trivid = 0U;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               (state_.opening_intro.scene != OpeningIntroScene::trivid ||
                state_.opening_intro.stage !=
                    OpeningIntroStage::orchestrator_loop)) {
            if (frames_to_trivid++ >= kMaxFramesToTrivid) {
                return fail("did not reach the stable trivid orchestrator loop");
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            state_.opening_intro.scene != OpeningIntroScene::trivid ||
            state_.opening_intro.stage !=
                OpeningIntroStage::orchestrator_loop) {
            return fail("opening ended before the stable trivid loop");
        }
        // The palette-fade completion changes the typed stage before the
        // first per-frame SAT batch is emitted. Capture the first completed
        // orchestrator pass, after its source requests have been published.
        advance_one_source_frame();
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            state_.opening_intro.scene != OpeningIntroScene::trivid ||
            state_.opening_intro.stage !=
                OpeningIntroStage::orchestrator_loop) {
            return fail("trivid ended before its first published SAT batch");
        }

        const auto* background_record = state_.asset_catalog.catalog.find(
            "intro.trivid.background");
        const auto background =
            draw_asset_cache_.find("intro.trivid.background");
        if (background_record == nullptr ||
            background == draw_asset_cache_.end() ||
            background->second.size() != QSize(kSceneWidth, kSceneHeight)) {
            return fail("the named 320x224 cockpit plane is missing");
        }

        const auto audit_priority_pixels = [this, &background](
            const QImage& scene) {
            const auto pixel_count = static_cast<std::size_t>(scene.width()) *
                                     static_cast<std::size_t>(scene.height());
            std::vector<bool> low_priority_coverage(pixel_count, false);
            std::vector<QColor> low_priority_colors(pixel_count);
            std::vector<QColor> high_priority_colors(pixel_count);
            std::vector<bool> high_priority_coverage(pixel_count, false);

            for (const auto& request : opening_sprites_) {
                const auto group = opening_intro_sprite_group(
                    request.sprite_descriptor);
                if (group.family != OpeningIntroSpriteFamily::trivid_left &&
                    group.family != OpeningIntroSpriteFamily::trivid_right &&
                    group.family != OpeningIntroSpriteFamily::trivid_timed) {
                    continue;
                }
                const bool high_priority =
                    (request.palette_flags & 0x8000U) != 0U;
                const auto prefix = opening_intro_sprite_alias_prefix(group);
                for (const auto& record :
                     state_.asset_catalog.catalog.records) {
                    if (record.alias.rfind(prefix, 0U) != 0U) {
                        continue;
                    }
                    const auto image = draw_asset_cache_.find(record.alias);
                    if (image == draw_asset_cache_.end()) {
                        continue;
                    }
                    const auto point = opening_intro_sat_draw_point(
                        request.x, request.y, record.origin_x,
                        record.origin_y);
                    for (int y = 0; y < image->second.height(); ++y) {
                        for (int x = 0; x < image->second.width(); ++x) {
                            const auto color =
                                image->second.pixelColor(x, y);
                            if (color.alpha() == 0) {
                                continue;
                            }
                            const auto screen_x = point.x + x;
                            const auto screen_y = point.y + y;
                            if (screen_x < 0 || screen_x >= scene.width() ||
                                screen_y < 0 || screen_y >= scene.height()) {
                                continue;
                            }
                            const auto index =
                                static_cast<std::size_t>(screen_y) *
                                    static_cast<std::size_t>(scene.width()) +
                                static_cast<std::size_t>(screen_x);
                            if (high_priority) {
                                high_priority_coverage[index] = true;
                                high_priority_colors[index] = color;
                            } else {
                                low_priority_coverage[index] = true;
                                low_priority_colors[index] = color;
                            }
                        }
                    }
                }
            }

            QImage text_coverage(scene.size(), QImage::Format_ARGB32);
            text_coverage.fill(Qt::transparent);
            QPainter text_painter(&text_coverage);
            for (const auto& request : opening_text_) {
                draw_opening_text(text_painter, request);
            }
            text_painter.end();

            std::size_t low_plane_pixels = 0U;
            std::size_t low_hole_pixels = 0U;
            std::size_t high_sprite_pixels = 0U;
            for (int y = 0; y < scene.height(); ++y) {
                for (int x = 0; x < scene.width(); ++x) {
                    const auto background_color =
                        background->second.pixelColor(x, y);
                    if (text_coverage.pixelColor(x, y).alpha() != 0 ||
                        (background_color.alpha() != 0 &&
                         background_color.alpha() != 255)) {
                        continue;
                    }
                    const auto index = static_cast<std::size_t>(y) *
                                           static_cast<std::size_t>(
                                               scene.width()) +
                                       static_cast<std::size_t>(x);
                    const auto actual = scene.pixelColor(x, y);
                    if (low_priority_coverage[index] &&
                        !high_priority_coverage[index]) {
                        if (background_color.alpha() == 255) {
                            if (actual != background_color) {
                                return std::array<std::size_t, 3>{0U, 0U, 0U};
                            }
                            ++low_plane_pixels;
                        } else {
                            if (actual != low_priority_colors[index]) {
                                return std::array<std::size_t, 3>{0U, 0U, 0U};
                            }
                            ++low_hole_pixels;
                        }
                    }
                    if (high_priority_coverage[index]) {
                        if (actual != high_priority_colors[index]) {
                            return std::array<std::size_t, 3>{0U, 0U, 0U};
                        }
                        ++high_sprite_pixels;
                    }
                }
            }
            return std::array<std::size_t, 3>{low_plane_pixels,
                                              low_hole_pixels,
                                              high_sprite_pixels};
        };

        bool saw_low_priority = false;
        bool saw_high_priority = false;
        bool saw_timed_phase[4] = {false, false, false, false};
        const auto capture = [&](const std::string_view sample) {
            const auto& opening = state_.opening_intro;
            bool low_request = false;
            bool high_request = false;
            bool timed_request = false;
            for (const auto& request : opening_sprites_) {
                const auto group = opening_intro_sprite_group(
                    request.sprite_descriptor);
                if (group.family != OpeningIntroSpriteFamily::trivid_left &&
                    group.family != OpeningIntroSpriteFamily::trivid_right &&
                    group.family != OpeningIntroSpriteFamily::trivid_timed) {
                    return fail("unexpected sprite family reached the trivid scene");
                }
                const bool high_priority =
                    (request.palette_flags & 0x8000U) != 0U;
                const bool expected_high_priority =
                    group.family != OpeningIntroSpriteFamily::trivid_right;
                if (high_priority != expected_high_priority) {
                    return fail("ROM sprite priority flags disagree with the SAT family");
                }
                high_request = high_request || high_priority;
                low_request = low_request || !high_priority;
                timed_request = timed_request ||
                    group.family == OpeningIntroSpriteFamily::trivid_timed;
            }
            if (!low_request || !high_request ||
                (opening.orchestrator.timed_state.phase > 0U &&
                 !timed_request)) {
                return fail("expected high, low, and timed source SAT requests are missing");
            }
            saw_low_priority = true;
            saw_high_priority = true;

            const auto scene = render_scene();
            if (scene.size() != QSize(kSceneWidth, kSceneHeight)) {
                return fail("renderer did not produce a full 320x224 screen");
            }
            const auto pixel_counts = audit_priority_pixels(scene);
            // 0x5AB96 darkens palette lines 0-3 one CRAM level per step after
            // loop frame 0xB4; by the pre-exit sample the ROM screen is black,
            // so priority is only auditable before full darkening.
            const bool darkened_to_black =
                opening.trivid_color_steps >= kTrividColorStepsToBlack;
            if (darkened_to_black) {
                if (pixel_counts[0] != 0U || pixel_counts[1] != 0U ||
                    pixel_counts[2] != 0U) {
                    return fail("tri-vid stayed lit after the ROM darkened it to black");
                }
            } else if (pixel_counts[0] < 8U || pixel_counts[1] < 8U ||
                pixel_counts[2] < 8U) {
                std::cerr << "opening trivid layer QA: sample=" << sample
                          << " low-plane pixels=" << pixel_counts[0]
                          << " low-hole pixels=" << pixel_counts[1]
                          << " high-sprite pixels=" << pixel_counts[2]
                          << '\n';
                scene.save(QString::fromStdWString(
                    (output_directory / "priority-diagnostic.bmp").wstring()));
                return fail("source priority was not visible at enough full-screen overlap pixels");
            }

            const auto timed_phase = opening.orchestrator.timed_state.phase;
            const auto filename = std::string(sample) + ".bmp";
            if (!scene.save(QString::fromStdWString(
                    (output_directory / filename).wstring()))) {
                return fail("could not save a full-screen trivid checkpoint");
            }
            trace << sample << '\t' << opening.orchestrator_loop_frame << '\t'
                  << opening.source_frame << '\t' << timed_phase << '\t'
                  << opening.orchestrator.timed_state.x << '\t'
                  << opening.orchestrator.timed_state.y << '\t'
                  << opening.orchestrator.projection_state.x_accumulator
                  << '\t'
                  << opening.orchestrator.projection_state.y_accumulator
                  << '\t' << pixel_counts[0] << '\t' << pixel_counts[1]
                  << '\t' << pixel_counts[2] << '\t' << filename << '\n';
            if (timed_phase < 4U) {
                saw_timed_phase[timed_phase] = true;
            }
            if (sample == "pre-exit" &&
                (!scene.save(QString::fromStdWString(
                     (output_directory / "actual.bmp").wstring())) ||
                 QImage(QString::fromStdWString(
                     (output_directory / "actual.bmp").wstring())) != scene)) {
                return fail("pre-exit whole-screen actual.bmp readback failed");
            }
            return 0;
        };

        if (capture("stage-entry") != 0) {
            return 3;
        }
        bool saw_midpoint = false;
        bool saw_pre_exit = false;
        while (state_.phase == StartupIntegrationPhase::opening_intro &&
               state_.opening_intro.scene == OpeningIntroScene::trivid &&
               state_.opening_intro.stage ==
                   OpeningIntroStage::orchestrator_loop) {
            const auto& opening = state_.opening_intro;
            const auto loop_frame = opening.orchestrator_loop_frame;
            if (loop_frame == kMovingMidpointLoopFrame && !saw_midpoint) {
                if (capture("moving-midpoint") != 0) {
                    return 3;
                }
                saw_midpoint = true;
            }
            if (opening.orchestrator.timed_state.phase > 0U &&
                opening.orchestrator.timed_state.phase <= 3U &&
                !saw_timed_phase[opening.orchestrator.timed_state.phase]) {
                const auto label = "timed-phase-" + std::to_string(
                    opening.orchestrator.timed_state.phase);
                if (capture(label) != 0) {
                    return 3;
                }
            }
            if (loop_frame == kStageExitLoopFrame && !saw_pre_exit) {
                if (capture("pre-exit") != 0) {
                    return 3;
                }
                saw_pre_exit = true;
            }
            advance_one_source_frame();
        }
        if (!saw_low_priority || !saw_high_priority || !saw_midpoint ||
            !saw_pre_exit || !saw_timed_phase[1] || !saw_timed_phase[2] ||
            !saw_timed_phase[3]) {
            return fail("the full source loop did not visit every expected priority/timing checkpoint");
        }

        std::cout << "opening_trivid_layer_qa=passed samples=6"
                  << " source_priority=verified"
                  << " actual="
                  << (output_directory / "actual.bmp").string() << '\n';
        return 0;
    }

    int run_intro_to_field_smoke() {
        host_frame_timer_.stop();
        constexpr int kMaxFrames = 8000;
        int frames = 0;
        const auto pump = [this, &frames] {
            if (frames >= kMaxFrames) {
                return false;
            }
            advance_one_source_frame();
            ++frames;
            return true;
        };

        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (!pump()) {
                std::cerr << "intro-to-field: boot did not reach title\n";
                return 3;
            }
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            std::cerr << "intro-to-field: phase after boot="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        while (state_.phase == StartupIntegrationPhase::title_ready) {
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kTitleStartButton) << 8U);
            if (!pump()) {
                std::cerr << "intro-to-field: title did not accept Start\n";
                return 3;
            }
        }
        if (state_.phase != StartupIntegrationPhase::start_options_shell ||
            !pump() ||
            state_.phase != StartupIntegrationPhase::start_options) {
            std::cerr << "intro-to-field: phase after title="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        advance_startup_integration(
            state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            std::cerr << "intro-to-field: New Game did not reach Joshua\n";
            return 3;
        }
        advance_startup_integration(
            state_, kStartupMenuA, startup_callbacks_);
        qa_finish_archetype_exit_fade();
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            std::cerr << "intro-to-field: Joshua did not start opening\n";
            return 3;
        }
        while (state_.phase == StartupIntegrationPhase::opening_intro) {
            if (!pump()) {
                std::cerr << "intro-to-field: opening did not complete\n";
                return 3;
            }
        }
        if (state_.phase != StartupIntegrationPhase::field_ready ||
            !field_scene_runtime_.ready()) {
            std::cerr << "intro-to-field: field not ready phase="
                      << startup_integration_phase_name(state_.phase)
                      << " bootstrap=" << field_scene_runtime_.failure()
                      << '\n';
            return 3;
        }

        QImage scene(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        scene.fill(Qt::black);
        QPainter painter(&scene);
        field_scene_runtime_.paint(painter);
        painter.end();
        if (count_playfield_lit(scene) < 100) {
            std::cerr << "intro-to-field: playfield buried under opaque HUD\n";
            return 3;
        }
        if (!field_hud_chrome_survives(scene, asset_root_)) {
            std::cerr << "intro-to-field: field HUD chrome was not preserved\n";
            return 3;
        }
        const auto& buffers = field_scene_runtime_.state().field_scene_buffers;
        std::cout << "intro_to_field=ready scene_reference="
                  << buffers.scene_reference << " camera="
                  << buffers.camera_x << ',' << buffers.camera_y
                  << " field_position="
                  << field_scene_runtime_.field_entity_world_position().x
                  << ','
                  << field_scene_runtime_.field_entity_world_position().y
                  << " entity_visible="
                  << (field_scene_runtime_.field_entity_visible() ? 1 : 0)
                  << " opening_frames=" << frames << '\n';
        return 0;
    }

    int run_opening_start_skip_qa(
        const std::filesystem::path& output_directory) {
        host_frame_timer_.stop();
        std::filesystem::create_directories(output_directory);
        if (prepare_opening_dump(0U) != 0) {
            return 3;
        }

        // Let the opening advance before checking the input route. The ROM
        // only latches Start as the skip edge; A must not leave this scene.
        for (std::uint32_t frame = 0U; frame < 60U; ++frame) {
            advance_one_source_frame();
        }
        QKeyEvent a_press(QEvent::KeyPress, Qt::Key_Z, Qt::NoModifier);
        keyPressEvent(&a_press);
        advance_one_source_frame();
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            std::cerr << "opening Start-skip QA: A incorrectly skipped the "
                         "opening\n";
            return 3;
        }

        QKeyEvent start_press(QEvent::KeyPress, Qt::Key_Space,
                              Qt::NoModifier);
        const auto boundary_start = presentation_boundaries_.size();
        keyPressEvent(&start_press);
        // 0x59CFE's 0x131C fades the scene for 34 VBlanks before the field.
        std::uint32_t skip_frames = 0U;
        do {
            advance_one_source_frame();
            ++skip_frames;
        } while (state_.phase == StartupIntegrationPhase::opening_intro &&
                 skip_frames < 64U);
        if (skip_frames != 35U) {
            std::cerr << "opening Start-skip QA: 0x131C exit fade took "
                      << skip_frames << " frames\n";
            return 3;
        }
        if (state_.phase != StartupIntegrationPhase::field_ready ||
            state_.opening_intro.stage != OpeningIntroStage::interrupted ||
            !state_.new_game_opening_completed ||
            !field_scene_runtime_.ready() ||
            !field_scene_runtime_.opening_auto_walk_active_for_dump()) {
            std::cerr << "opening Start-skip QA: Start did not return to the "
                         "New Game field setup and arm its Lua event; phase="
                      << startup_integration_phase_name(state_.phase)
                      << " field=" << field_scene_runtime_.failure() << '\n';
            return 3;
        }
        const auto start_sample = std::find(
            presentation_boundaries_.begin() +
                static_cast<std::ptrdiff_t>(boundary_start),
            presentation_boundaries_.end(), "opening.sample_controller");
        const auto start_latch = std::find(
            presentation_boundaries_.begin() +
                static_cast<std::ptrdiff_t>(boundary_start),
            presentation_boundaries_.end(), "opening.latch_controller_events");
        const auto exit_sample = std::find(
            presentation_boundaries_.begin() +
                static_cast<std::ptrdiff_t>(boundary_start),
            presentation_boundaries_.end(),
            "0x59CFE.sample_controller_input");
        if (start_sample == presentation_boundaries_.end() ||
            start_latch == presentation_boundaries_.end() ||
            exit_sample == presentation_boundaries_.end() ||
            !(start_sample < start_latch) || !(start_latch < exit_sample)) {
            std::cerr << "opening Start-skip QA: source sample/latch/exit "
                         "order was not preserved\n";
            return 3;
        }

        QImage actual(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        actual.fill(Qt::black);
        QPainter painter(&actual);
        field_scene_runtime_.paint(painter);
        painter.end();
        if (count_playfield_lit(actual) < 100 ||
            !field_hud_chrome_survives(actual, asset_root_)) {
            std::cerr << "opening Start-skip QA: field frame is not a clean "
                         "playable screen\n";
            return 3;
        }
        const auto actual_path = output_directory / "actual.bmp";
        if (!actual.save(QString::fromStdWString(actual_path.wstring())) ||
            QImage(QString::fromStdWString(actual_path.wstring())) != actual) {
            std::cerr << "opening Start-skip QA: field actual.bmp failed "
                         "round-trip validation\n";
            return 3;
        }
        const auto field_start_position =
            field_scene_runtime_.field_entity_world_position();

        // A skipped movie must take the same ROM-backed New Game field
        // handoff as a completed movie: its scene-entry Lua rule walks the
        // runner into Stoker's first-visit dialogue without controller input.
        std::uint32_t arrival_frames = 0U;
        for (; arrival_frames < 256U &&
               !field_scene_runtime_.dialogue_open(); ++arrival_frames) {
            field_scene_runtime_.advance_frame();
        }
        const auto& arrival = field_scene_runtime_.dialogue_panel();
        const auto field_arrival_position =
            field_scene_runtime_.field_entity_world_position();
        if (!field_scene_runtime_.dialogue_open() ||
            field_scene_runtime_.opening_auto_walk_active_for_dump() ||
            (field_start_position.x == field_arrival_position.x &&
             field_start_position.y == field_arrival_position.y) ||
            arrival.actor_name != "Stoker" || arrival.option_count != 1U) {
            std::cerr << "opening Start-skip QA: scripted field entry did not "
                         "move the runner into Stoker's arrival dialogue\n";
            return 3;
        }
        QImage arrival_image(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        arrival_image.fill(Qt::black);
        QPainter arrival_painter(&arrival_image);
        field_scene_runtime_.paint(arrival_painter);
        arrival_painter.end();
        if (!dialogue_more_delimiters_are_mirrored(arrival_image)) {
            std::cerr << "opening Start-skip QA: Stoker < MORE > markers "
                         "do not face outward\n";
            return 3;
        }
        const auto arrival_path = output_directory / "stoker-arrival.bmp";
        if (!arrival_image.save(
                QString::fromStdWString(arrival_path.wstring())) ||
            QImage(QString::fromStdWString(arrival_path.wstring())) !=
                arrival_image) {
            std::cerr << "opening Start-skip QA: Stoker arrival BMP failed "
                         "round-trip validation\n";
            return 3;
        }
        std::ofstream trace(output_directory / "opening-start-skip-qa.tsv");
        if (!trace) {
            std::cerr << "opening Start-skip QA: could not write trace\n";
            return 3;
        }
        trace << "result\topening_stage\topening_frame\tfield_scene\t"
                 "arrival_frames\tstart_x\tstart_y\tarrival_x\tarrival_y\t"
                 "arrival_speaker\timage\tarrival_image\n"
              << "start_skipped\t"
              << opening_intro_stage_name(state_.opening_intro.stage) << '\t'
              << state_.opening_intro.source_frame << '\t'
              << field_scene_runtime_.state().field_scene_buffers
                     .scene_reference
              << '\t' << arrival_frames << '\t' << field_start_position.x
              << '\t' << field_start_position.y << '\t'
              << field_arrival_position.x << '\t'
              << field_arrival_position.y << '\t' << arrival.actor_name
              << '\t' << actual_path.string() << '\t' << arrival_path.string()
              << '\n';
        trace.close();
        if (!trace) {
            std::cerr << "opening Start-skip QA: trace write failed\n";
            return 3;
        }
        std::cout << "opening_start_skip_qa=passed opening_frame="
                  << state_.opening_intro.source_frame << " scene="
                  << field_scene_runtime_.state().field_scene_buffers
                         .scene_reference
                  << " arrival_frames=" << arrival_frames
                  << " actual=" << actual_path.string()
                  << " stoker_arrival=" << arrival_path.string() << '\n';
        return 0;
    }

    int run_startup_qa(const std::filesystem::path& output_directory) {
        host_frame_timer_.stop();
        opening_presentation_holds_enabled_ = false;
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "startup-qa.tsv");
        if (!trace) {
            std::cerr << "startup qa: could not open trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "label\tphase\tboot_frame\topening_frame\tstage\tscene"
                 "\tpalette_weight\n";

        std::ofstream title_scroll_trace(
            output_directory / "title-scroll-qa.tsv");
        if (!title_scroll_trace) {
            std::cerr << "startup qa: could not open title scroll trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        title_scroll_trace
            << "label\ttitle_frame\tbackground_scroll\tcircuit_phase\n";

        std::ofstream title_circuit_palette_trace(
            output_directory / "title-circuit-palette-qa.tsv");
        if (!title_circuit_palette_trace) {
            std::cerr << "startup qa: could not open title circuit palette "
                         "trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        title_circuit_palette_trace
            << "label\ttitle_frame\tlogo_columns\tcompletion_state\t"
               "palette_countdown\tpalette_steps\tcorner_ink_pixels\n";

        std::ofstream title_sprite_trace(
            output_directory / "title-sprite-lifetime-qa.tsv");
        if (!title_sprite_trace) {
            std::cerr << "startup qa: could not open title sprite trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        title_sprite_trace
            << "label\ttitle_frame\tlogo_columns\tsprite_phase\tsprite_x\t"
               "projection_visible\tright_edge_overlay_pixels\n";

        std::ofstream title_prompt_trace(
            output_directory / "title-prompt-qa.tsv");
        if (!title_prompt_trace) {
            std::cerr << "startup qa: could not open title prompt trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        title_prompt_trace
            << "label\ttitle_frame\tframes_remaining\tvariant\tvisible\t"
               "prompt_foreground_pixels\tprompt_exact_matches\t"
               "copyright_foreground_pixels\tcopyright_exact_matches\n";

        std::ofstream title_exit_trace(
            output_directory / "title-exit-fade-qa.tsv");
        if (!title_exit_trace) {
            std::cerr << "startup qa: could not open title exit trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        title_exit_trace
            << "label\tframe\tfade_vblank\tpalette_pass\t"
               "vblanks_in_pass\tactive\tcomplete\tpalette_steps\t"
               "phase\n";

        std::ofstream title_mode_entry_trace(
            output_directory / "title-mode-entry-qa.tsv");
        if (!title_mode_entry_trace) {
            std::cerr << "startup qa: could not open title mode-entry trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        title_mode_entry_trace
            << "label\tframe\tvblank\ttransactions_started\t"
               "vblanks_in_transaction\tactive\tcomplete\tflushes\t"
               "audio_ticks\tphase\n";

        const auto title_background_matches =
            [this](const QImage& scene, const bool full_background) {
                const auto phase = static_cast<std::size_t>(
                    title_loop_.circuit_lights.phase %
                    kTitleCircuitPhaseCount);
                const auto passes = std::min<std::size_t>(
                    title_loop_.text.palette_steps,
                    kTitleCircuitPaletteRevealPassCount);
                const auto& background =
                    title_circuit_transition_cache_[phase][passes];
                if (background.isNull()) {
                    return false;
                }

                // Current title assets are screen-sized and start at (0,0).
                // Use the catalog origin and phase alias to compare only the
                // background-only band once foreground layers are present.
                const auto actual_alias = "title.circuit.background." +
                                          std::to_string(phase);
                const auto* record =
                    state_.asset_catalog.catalog.find(actual_alias);
                if (record == nullptr || background.height() <= 0) {
                    return false;
                }

                const auto& source = background;
                const auto height = source.height();
                const auto scroll = static_cast<int>(
                    title_loop_.animation_timer.animation_offset %
                    static_cast<std::uint16_t>(height));
                const auto first_y = full_background
                                         ? 0
                                         : std::max(0, record->origin_y);
                const auto last_y = full_background
                                        ? scene.height()
                                        : std::min(
                                              scene.height(),
                                              record->origin_y +
                                                  std::min(16, height));
                bool any_pixel_moved = false;
                for (int y = first_y; y < last_y; ++y) {
                    for (int x = 0; x < scene.width(); ++x) {
                        const auto source_x = x - record->origin_x;
                        const auto source_y = y - record->origin_y;
                        if (source_x < 0 || source_x >= source.width() ||
                            source_y < 0 || source_y >= height) {
                            continue;
                        }
                        const auto expected_y = (source_y + scroll) % height;
                        const auto expected =
                            source.pixelColor(source_x, expected_y);
                        const auto actual = scene.pixelColor(x, y);
                        if (actual.red() != expected.red() ||
                            actual.green() != expected.green() ||
                            actual.blue() != expected.blue()) {
                            std::cerr << "startup qa: title pixel mismatch at "
                                      << x << ',' << y << " phase="
                                      << static_cast<unsigned>(
                                             title_loop_.circuit_lights.phase)
                                      << " offset="
                                      << title_loop_.animation_timer
                                             .animation_offset
                                      << " source=" << expected.red() << ','
                                      << expected.green() << ','
                                      << expected.blue() << " actual="
                                      << actual.red() << ',' << actual.green()
                                      << ',' << actual.blue() << '\n';
                            return false;
                        }
                        any_pixel_moved = any_pixel_moved ||
                            expected != source.pixelColor(source_x, source_y);
                    }
                }
                return scroll == 0 || !full_background || any_pixel_moved;
            };

        const auto write_snapshot = [&](const std::string& name) {
            const auto path = output_directory / (name + ".bmp");
            const auto scene = render_scene();
            if (state_.phase == StartupIntegrationPhase::title_ready &&
                !title_background_matches(scene, false)) {
                std::cerr << "startup qa: title background did not follow the "
                             "source vertical scroll for "
                          << name << '\n';
                return false;
            }
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.seattle_outline_frame != 0U &&
                state_.opening_intro.stage == OpeningIntroStage::schedule &&
                opening_scene_ == OpeningIntroScene::seattle) {
                const auto phase = static_cast<std::uint8_t>(std::min<
                    std::uint16_t>(
                        (state_.opening_intro.seattle_outline_frame - 1U) /
                            2U,
                        4U));
                const auto outline =
                    opening_intro_seattle_outline_plan(phase);
                const auto bottom_midpoint =
                    QPoint(outline.x + outline.width / 2,
                           outline.y + outline.height - 1);
                // CRAM levels (7,5,0) [C: emulator pixel 238,170,0].
                if (scene.pixelColor(bottom_midpoint) !=
                    QColor(0xFF, 0xB6, 0x00)) {
                    std::cerr
                        << "startup qa: Seattle focus box was not a fine "
                           "amber outline in phase "
                        << static_cast<unsigned>(phase) << '\n';
                    return false;
                }
            }
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.scene_palette_weight == 0U) {
                for (int y = 0; y < scene.height(); ++y) {
                    for (int x = 0; x < scene.width(); ++x) {
                        if (scene.pixelColor(x, y) != QColor(Qt::black)) {
                            std::cerr
                                << "startup qa: zero-weight scene palette "
                                   "did not render black\n";
                            return false;
                        }
                    }
                }
            }
            const auto is_startup_menu =
                state_.phase == StartupIntegrationPhase::start_options_shell ||
                state_.phase == StartupIntegrationPhase::start_options ||
                state_.phase == StartupIntegrationPhase::save_load ||
                state_.phase == StartupIntegrationPhase::joshua_archetype;
            if (is_startup_menu &&
                (!startup_shell_survives(scene) ||
                 (state_.phase ==
                      StartupIntegrationPhase::start_options_shell &&
                  !startup_shell_only(scene)))) {
                std::cerr << "startup qa: startup bezel/decal layer was "
                             "not preserved or shell phase was not clean for "
                          << name << '\n';
                return false;
            }
            if (state_.phase == StartupIntegrationPhase::field_ready) {
                if (field_scene_runtime_.dialogue_open()) {
                    if (field_hud_chrome_survives(scene, asset_root_) ||
                        !image_has_green_ink(scene, 88, 16, 216, 120)) {
                        std::cerr << "startup qa: dialogue did not replace the "
                                     "field HUD with visible text for "
                                  << name << '\n';
                        return false;
                    }
                } else if (!field_hud_chrome_survives(scene, asset_root_)) {
                    std::cerr << "startup qa: field HUD chrome was not preserved "
                                 "for "
                              << name << '\n';
                    return false;
                }
            }
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.scene == OpeningIntroScene::cab_map) {
                const auto cursor = cab_mode_selection_plan(
                    state_.opening_intro.post_intro_mode,
                    state_.opening_intro.post_intro_phase);
                struct CabCursorTileSample {
                    std::string_view name;
                    int x_offset;
                    int y_offset;
                };
                const std::array<CabCursorTileSample, 8> cursor_tiles{{
                    {"top.left", 0, 0},
                    {"top.middle", 8, 0},
                    {"top.right", cursor.cursor_width - 8, 0},
                    {"left", 0, 8},
                    {"right", cursor.cursor_width - 8, 8},
                    {"bottom.left", 0, cursor.cursor_height - 8},
                    {"bottom.middle", 8, cursor.cursor_height - 8},
                    {"bottom.right", cursor.cursor_width - 8,
                     cursor.cursor_height - 8},
                }};
                for (const auto& sample : cursor_tiles) {
                    const auto alias = std::string("intro.cab.cursor.") +
                                       std::string(sample.name);
                    const auto tile = draw_asset_cache_.find(alias);
                    if (tile == draw_asset_cache_.end() ||
                        tile->second.size() != QSize(8, 8)) {
                        std::cerr << "startup qa: cab cursor asset missing: "
                                  << alias << '\n';
                        return false;
                    }
                    int compared_pixels = 0;
                    for (int tile_y = 0; tile_y < 8; ++tile_y) {
                        for (int tile_x = 0; tile_x < 8; ++tile_x) {
                            const auto source =
                                tile->second.pixelColor(tile_x, tile_y);
                            if (source.alpha() == 0) {
                                continue;
                            }
                            const auto screen_x = cursor.cursor_x +
                                                  sample.x_offset + tile_x;
                            const auto screen_y = cursor.cursor_y +
                                                  sample.y_offset + tile_y;
                            if (screen_x < 0 || screen_x >= scene.width() ||
                                screen_y < 0 || screen_y >= scene.height() ||
                                scene.pixelColor(screen_x, screen_y) != source) {
                                std::cerr << "startup qa: cab cursor tile "
                                             "did not render at its ROM "
                                             "Plane-A position: "
                                          << alias << " in " << name << '\n';
                                return false;
                            }
                            ++compared_pixels;
                        }
                    }
                    if (compared_pixels == 0) {
                        std::cerr << "startup qa: cab cursor tile is empty: "
                                  << alias << '\n';
                        return false;
                    }
                }
            }
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.scene == OpeningIntroScene::cab_map &&
                state_.opening_intro.post_intro_mode == 2U &&
                state_.opening_intro.post_intro_caption_visible) {
                const auto caption = cab_mode_selection_plan(
                    state_.opening_intro.post_intro_mode,
                    state_.opening_intro.post_intro_phase);
                const auto font = draw_asset_cache_.find("menu.startup.font");
                constexpr int kRedmondLeadingSpaces = 2;
                constexpr char kRedmondFirstLetter = 'R';
                const auto glyph = static_cast<int>(
                    static_cast<unsigned char>(kRedmondFirstLetter) - 0x20U);
                const auto glyph_x = (glyph % 16) * 8;
                const auto glyph_y = (glyph / 16) * 8;
                bool first_letter_is_white = false;
                if (font != draw_asset_cache_.end()) {
                    for (int y = 0; y < 8; ++y) {
                        for (int x = 0; x < 8; ++x) {
                            const auto source = font->second.pixelColor(
                                glyph_x + x, glyph_y + y);
                            const auto coverage = std::max(
                                {source.red(), source.green(), source.blue()});
                            if (source.alpha() == 0 || coverage == 0) {
                                continue;
                            }
                            const auto pixel = scene.pixelColor(
                                caption.caption_x +
                                    kRedmondLeadingSpaces * 8 + x,
                                caption.caption_y + y);
                            if (pixel.red() < 200 || pixel.green() < 200 ||
                                pixel.blue() < 200) {
                                std::cerr << "startup qa: Redmond cab caption "
                                             "was not rendered white\n";
                                return false;
                            }
                            first_letter_is_white = true;
                        }
                    }
                }
                if (!first_letter_is_white) {
                    std::cerr << "startup qa: Redmond cab caption glyph was "
                                 "not found\n";
                    return false;
                }
            }
            if (!scene.save(QString::fromStdWString(path.wstring()))) {
                std::cerr << "startup qa: could not write " << path.string()
                          << '\n';
                return false;
            }
            return true;
        };
        const auto record = [&](const std::string& label) {
            trace << label << '\t'
                  << startup_integration_phase_name(state_.phase) << '\t'
                  << state_.frame.frame_count << '\t'
                  << state_.opening_intro.source_frame << '\t'
                  << opening_intro_stage_name(opening_stage_) << '\t'
                  << opening_intro_scene_name(opening_scene_) << '\t'
                  << static_cast<unsigned>(
                         state_.opening_intro.scene_palette_weight)
                  << '\n';
        };
        const auto fail = [&](const std::string_view message) {
            std::cerr << "startup qa: " << message << '\n';
            return 3;
        };

        record("boot.initial");
        if (!write_snapshot("00_boot_initial")) {
            return 3;
        }

        constexpr int kMaxFrames = 10000;
        int frames = 0;
        const auto pump = [&] {
            if (frames >= kMaxFrames) {
                return false;
            }
            advance_one_source_frame();
            ++frames;
            return true;
        };

        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (!pump()) {
                return fail("boot did not reach title");
            }
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            return fail("boot reached an unexpected phase");
        }
        record("title.ready");
        if (!write_snapshot("01_title_ready")) {
            return 3;
        }

        int title_frame = 0;
        const auto title_right_edge_overlay_pixels =
            [this](const QImage& scene) {
                const auto phase = static_cast<std::size_t>(
                    title_loop_.circuit_lights.phase %
                    kTitleCircuitPhaseCount);
                const auto background_alias =
                    "title.circuit.background." + std::to_string(phase);
                const auto passes = std::min<std::size_t>(
                    title_loop_.text.palette_steps,
                    kTitleCircuitPaletteRevealPassCount);
                const auto& background =
                    title_circuit_transition_cache_[phase][passes];
                const auto* background_record =
                    state_.asset_catalog.catalog.find(background_alias);
                const auto* logo_record =
                    state_.asset_catalog.catalog.find("title.logo");
                const auto logo_image = draw_asset_cache_.find("title.logo");
                const auto sprite_alias =
                    "title.sprite." + std::to_string(
                        title_loop_.sprite.phase % 6U);
                const auto* sprite_record =
                    state_.asset_catalog.catalog.find(sprite_alias);
                const auto sprite_image =
                    draw_asset_cache_.find(sprite_alias);
                if (background.isNull() || background_record == nullptr ||
                    logo_record == nullptr ||
                    logo_image == draw_asset_cache_.end() ||
                    sprite_record == nullptr ||
                    sprite_image == draw_asset_cache_.end() ||
                    background.height() <= 0) {
                    return -1;
                }

                // The extracted logo ends at x=309. Its transient flame can
                // travel beyond it, so compare that strip with the current
                // scrolled circuit pixels to prove the projection appears
                // before retirement and disappears afterward.
                const auto& source = background;
                const auto scroll = static_cast<int>(
                    title_loop_.animation_timer.animation_offset %
                    static_cast<std::uint16_t>(source.height()));
                const auto first_x = std::min(
                    scene.width(), std::max(
                                       0, logo_record->origin_x +
                                              logo_image->second.width()));
                const auto first_y = std::max(0, sprite_record->origin_y);
                const auto last_y = std::min(
                    scene.height(), sprite_record->origin_y +
                                        sprite_image->second.height());
                int differing_pixels = 0;
                for (int y = first_y; y < last_y; ++y) {
                    for (int x = first_x; x < scene.width(); ++x) {
                        const auto source_x = x - background_record->origin_x;
                        const auto source_y = y - background_record->origin_y;
                        if (source_x < 0 || source_x >= source.width() ||
                            source_y < 0 || source_y >= source.height()) {
                            continue;
                        }
                        const auto expected_y =
                            (source_y + scroll) % source.height();
                        const auto expected =
                            source.pixelColor(source_x, expected_y);
                        const auto actual = scene.pixelColor(x, y);
                        if (actual.red() != expected.red() ||
                            actual.green() != expected.green() ||
                            actual.blue() != expected.blue()) {
                            ++differing_pixels;
                        }
                    }
                }
                return differing_pixels;
            };

        const auto title_text_foreground_matches =
            [this](const QImage& scene, const int first_source_row,
                   const int last_source_row) {
                struct PixelMatches {
                    int foreground = 0;
                    int exact = 0;
                };
                PixelMatches matches;
                const auto* record =
                    state_.asset_catalog.catalog.find("title.text");
                const auto image = draw_asset_cache_.find("title.text");
                if (record == nullptr || image == draw_asset_cache_.end()) {
                    return matches;
                }

                const auto& source = image->second;
                for (int y = std::max(0, first_source_row);
                     y < std::min(last_source_row, source.height()); ++y) {
                    for (int x = 0; x < source.width(); ++x) {
                        const auto source_color = source.pixelColor(x, y);
                        if (source_color.red() == 0 &&
                            source_color.green() == 0 &&
                            source_color.blue() == 0) {
                            continue;
                        }
                        ++matches.foreground;
                        const auto scene_x = record->origin_x + x;
                        const auto scene_y = record->origin_y + y;
                        if (scene_x >= 0 && scene_x < scene.width() &&
                            scene_y >= 0 && scene_y < scene.height() &&
                            scene.pixelColor(scene_x, scene_y).rgb() ==
                                source_color.rgb()) {
                            ++matches.exact;
                        }
                    }
                }
                return matches;
            };
        const auto save_title_prompt_checkpoint =
            [this, &output_directory, &title_prompt_trace,
             &title_text_foreground_matches,
             &title_frame](const char* label,
                           const TitlePromptVariant expected_variant,
                           const std::uint16_t expected_frames_remaining) {
                const auto expected_visible =
                    expected_variant == TitlePromptVariant::VariantA;
                if (title_loop_.prompt.frames_remaining !=
                        expected_frames_remaining ||
                    title_prompt_variant_ != expected_variant ||
                    title_prompt_visible_ != expected_visible ||
                    !title_copyright_visible_) {
                    std::cerr << "startup qa: title prompt state mismatch at "
                              << label << " frame=" << title_frame
                              << " remaining="
                              << title_loop_.prompt.frames_remaining
                              << " visible=" << title_prompt_visible_ << '\n';
                    return false;
                }

                const auto scene = render_scene();
                const auto prompt =
                    title_text_foreground_matches(scene, 0, 7);
                const auto copyright = title_text_foreground_matches(
                    scene, 8, std::numeric_limits<int>::max());
                const auto prompt_pixels_correct = expected_visible
                    ? prompt.foreground > 0 &&
                          prompt.exact == prompt.foreground
                    : prompt.foreground > 0 && prompt.exact == 0;
                if (!prompt_pixels_correct || copyright.foreground == 0 ||
                    copyright.exact != copyright.foreground) {
                    std::cerr << "startup qa: title prompt pixels mismatch at "
                              << label << " frame=" << title_frame
                              << " expected_visible=" << expected_visible
                              << " prompt_exact=" << prompt.exact << '/'
                              << prompt.foreground << " copyright_exact="
                              << copyright.exact << '/'
                              << copyright.foreground << '\n';
                    return false;
                }

                const auto path =
                    output_directory / (std::string(label) + ".bmp");
                if (!scene.save(QString::fromStdWString(path.wstring()))) {
                    std::cerr << "startup qa: could not write "
                              << path.string() << '\n';
                    return false;
                }
                title_prompt_trace
                    << label << '\t' << title_frame << '\t'
                    << title_loop_.prompt.frames_remaining << '\t'
                    << (expected_variant == TitlePromptVariant::VariantA
                            ? 'A'
                            : 'B')
                    << '\t' << title_prompt_visible_ << '\t'
                    << prompt.foreground << '\t' << prompt.exact << '\t'
                    << copyright.foreground << '\t' << copyright.exact << '\n';
                return true;
            };
        const auto save_title_sprite_checkpoint =
            [this, &output_directory, &title_sprite_trace,
             &title_right_edge_overlay_pixels,
             &title_frame](const char* label, const bool expected_visible) {
                const auto scene = render_scene();
                const auto overlay_pixels =
                    title_right_edge_overlay_pixels(scene);
                const auto visible = title_loop_.sprite.projection_visible;
                if (visible != expected_visible || overlay_pixels < 0 ||
                    (expected_visible && overlay_pixels == 0) ||
                    (!expected_visible && overlay_pixels != 0)) {
                    std::cerr << "startup qa: title sprite lifetime mismatch at "
                              << label << " columns="
                              << title_loop_.logo.columns_revealed
                              << " x=" << title_loop_.sprite.x
                              << " visible=" << visible
                              << " right_edge_pixels=" << overlay_pixels
                              << '\n';
                    return false;
                }
                const auto path =
                    output_directory / (std::string(label) + ".bmp");
                if (!scene.save(QString::fromStdWString(path.wstring()))) {
                    std::cerr << "startup qa: could not write "
                              << path.string() << '\n';
                    return false;
                }
                title_sprite_trace
                    << label << '\t' << title_frame << '\t'
                    << title_loop_.logo.columns_revealed << '\t'
                    << title_loop_.sprite.phase << '\t'
                    << title_loop_.sprite.x << '\t' << visible << '\t'
                    << overlay_pixels << '\n';
                return true;
            };
        const auto save_title_scroll_checkpoint =
            [&](const std::string& name, const bool full_background) {
                const auto scene = render_scene();
                if (!title_background_matches(scene, full_background)) {
                    std::cerr << "startup qa: circuit-plane pixels disagree "
                                 "with the typed scroll at "
                              << name << " (offset="
                              << title_loop_.animation_timer.animation_offset
                              << ")\n";
                    return false;
                }
                const auto path = output_directory / (name + ".bmp");
                if (!scene.save(QString::fromStdWString(path.wstring()))) {
                    std::cerr << "startup qa: could not write "
                              << path.string() << '\n';
                    return false;
                }
                title_scroll_trace
                    << name << '\t' << title_frame << '\t'
                    << title_loop_.animation_timer.animation_offset << '\t'
                    << static_cast<unsigned>(
                           title_loop_.circuit_lights.phase)
                    << '\n';
                return true;
            };
        const auto save_title_circuit_palette_checkpoint =
            [&](const std::string& name, const int expected_palette_steps,
                const std::int16_t expected_countdown) {
                const auto scene = render_scene();
                std::size_t corner_ink_pixels = 0;
                for (int y = 0; y < scene.height(); ++y) {
                    for (int x = 0; x < 13; ++x) {
                        if (scene.pixelColor(x, y) != QColor(Qt::black)) {
                            ++corner_ink_pixels;
                        }
                    }
                }
                const bool has_circuit_ink = corner_ink_pixels > 0U;
                const bool expected_circuit_ink =
                    expected_palette_steps > 0;
                if (title_loop_.logo.columns_revealed !=
                        kTitleLogoColumnLimit ||
                    title_loop_.text.completion_state != 1U ||
                    title_loop_.text.countdown != expected_countdown ||
                    title_loop_.text.palette_steps !=
                        expected_palette_steps ||
                    has_circuit_ink != expected_circuit_ink ||
                    !title_background_matches(scene, false)) {
                    std::cerr << "startup qa: circuit reveal did not match "
                                 "logo completion and source palette step at "
                              << name << " frame=" << title_frame
                              << " logo_columns="
                              << title_loop_.logo.columns_revealed
                              << " completion="
                              << title_loop_.text.completion_state
                              << " countdown="
                              << title_loop_.text.countdown
                              << " palette_steps="
                              << static_cast<unsigned>(
                                     title_loop_.text.palette_steps)
                              << " corner_ink_pixels="
                              << corner_ink_pixels << '\n';
                    return false;
                }
                const auto path = output_directory / (name + ".bmp");
                if (!scene.save(QString::fromStdWString(path.wstring()))) {
                    std::cerr << "startup qa: could not write "
                              << path.string() << '\n';
                    return false;
                }
                title_circuit_palette_trace
                    << name << '\t' << title_frame << '\t'
                    << title_loop_.logo.columns_revealed << '\t'
                    << title_loop_.text.completion_state << '\t'
                    << title_loop_.text.countdown << '\t'
                    << static_cast<unsigned>(
                           title_loop_.text.palette_steps)
                    << '\t' << corner_ink_pixels << '\n';
                return true;
            };

        if (title_loop_.animation_timer.animation_offset != 0U ||
            !save_title_scroll_checkpoint("title-scroll-00", true)) {
            return fail("title setup did not start with a clear circuit scroll");
        }
        constexpr std::array<std::pair<int, const char*>, 3>
            kTitleScrollCheckpoints{{{1, "title-scroll-01"},
                                     {6, "title-scroll-02"},
                                     {116, "title-scroll-24"}}};
        for (const auto& checkpoint : kTitleScrollCheckpoints) {
            while (title_frame < checkpoint.first &&
                   state_.phase == StartupIntegrationPhase::title_ready) {
                if (!pump()) {
                    return fail("title scroll did not advance");
                }
                ++title_frame;
            }
            const auto expected_scroll = static_cast<std::uint16_t>(
                ((checkpoint.first - 1) / 5) + 1);
            if (title_loop_.animation_timer.animation_offset !=
                    expected_scroll ||
                !save_title_scroll_checkpoint(checkpoint.second,
                                              checkpoint.first <= 6)) {
                return fail("title background scroll cadence diverged from "
                            "the five-VBlank source timer");
            }
            record(std::string("title.") + checkpoint.second);
        }

        // 0x1A9E rebuilds the moving title-flame projection through the pass
        // that emits column 0x25, then takes its 0x0E34 reset path after the
        // reveal state reaches 0x26. Capture both sides of that boundary.
        constexpr int kFinalRevealFrame = 165;
        while (title_frame < kFinalRevealFrame &&
               state_.phase == StartupIntegrationPhase::title_ready) {
            if (!pump()) {
                return fail("title sprite reveal did not reach final column");
            }
            ++title_frame;
        }
        if (title_loop_.logo.columns_revealed != kTitleLogoColumnLimit ||
            !save_title_sprite_checkpoint("title-sprite-last-active", true)) {
            return fail("title sprite projection did not remain through the "
                        "final logo column");
        }
        record("title.sprite.last_active");
        if (!pump()) {
            return fail("title sprite retirement frame did not advance");
        }
        ++title_frame;
        if (title_frame != kFinalRevealFrame + 1 ||
            !save_title_sprite_checkpoint("title-sprite-first-hidden", false)) {
            return fail("title sprite projection remained after the reveal "
                        "branch stopped");
        }
        record("title.sprite.first_hidden");

        constexpr std::array<std::pair<int, const char*>, 2>
            kTitleCircuitPaletteCheckpoints{{
                {188, "title-circuit-logo-complete-before-palette"},
                {189, "title-circuit-first-palette-step"},
            }};
        for (const auto& checkpoint : kTitleCircuitPaletteCheckpoints) {
            while (title_frame < checkpoint.first &&
                   state_.phase == StartupIntegrationPhase::title_ready) {
                if (!pump()) {
                    return fail("title circuit reveal checkpoint did not "
                                "advance");
                }
                ++title_frame;
            }
            const auto has_palette_step = checkpoint.first == 189;
            if (title_frame != checkpoint.first ||
                !save_title_circuit_palette_checkpoint(
                    checkpoint.second, has_palette_step ? 1 : 0,
                    has_palette_step ? 64 : 65)) {
                return fail("title circuit appeared outside the source "
                            "palette transition");
            }
            record(std::string("title.") + checkpoint.second);
        }

        // Keep title screenshots tied to their real source frame. In the
        // independent section-01 capture, copyright first appears at video
        // frame 418; the ROM draws it on title frame 254. That observed event
        // anchors the 30-Hz reference samples to candidate 60-Hz title frames
        // without treating the video file's frame zero as a ROM VBlank.
        // Capture these before prompt checkpoints advance to frame 530, so a
        // later frame cannot be mislabeled as title_300.
        constexpr std::array<int, 12> kTitleReferenceCheckpoints{
            198, 210, 253, 254, 258, 260, 262, 264, 266, 268, 300, 318};
        for (const auto checkpoint : kTitleReferenceCheckpoints) {
            while (title_frame < checkpoint &&
                   state_.phase == StartupIntegrationPhase::title_ready) {
                if (!pump()) {
                    return fail("title reference checkpoint did not advance");
                }
                ++title_frame;
            }
            if (state_.phase != StartupIntegrationPhase::title_ready ||
                title_frame != checkpoint) {
                return fail("title reference screenshot frame was mislabeled");
            }
            if (checkpoint == 253 || checkpoint == 254) {
                const auto scene = render_scene();
                const auto copyright = title_text_foreground_matches(
                    scene, 8, std::numeric_limits<int>::max());
                const auto expected_visible = checkpoint == 254;
                if (title_copyright_visible_ != expected_visible ||
                    (expected_visible &&
                     (copyright.foreground == 0 ||
                      copyright.exact != copyright.foreground)) ||
                    (!expected_visible && copyright.exact != 0)) {
                    return fail("title copyright did not match its traced "
                                "frame-254 draw boundary");
                }
            }
            const auto label = checkpoint == 300
                ? std::string("title_300")
                : std::string("title-frame-") +
                      std::to_string(checkpoint);
            if (!save_title_scroll_checkpoint(label, false)) {
                return fail("title reference screenshot disagrees with its "
                            "typed scroll and light phase");
            }
            if (checkpoint == 300 &&
                !render_scene().save(QString::fromStdWString(
                    (output_directory / "actual.bmp").wstring()))) {
                return fail("could not write the full-screen title actual.bmp");
            }
            record("title.reference." + std::to_string(checkpoint));
        }

        // The prompt record is selected from the low six bits of D7 after
        // the initial 0x0E10 countdown crosses 0x0C6C. Variant B is the blank
        // record for ten VBlanks; A remains visible for the rest of each
        // 64-VBlank cycle. Save full-screen samples from two complete cycles.
        struct TitlePromptCheckpoint {
            int frame;
            const char* label;
            TitlePromptVariant variant;
            std::uint16_t frames_remaining;
        };
        constexpr std::array<TitlePromptCheckpoint, 5>
            kTitlePromptCheckpoints{{
                {422, "title-prompt-visible-first",
                 TitlePromptVariant::VariantA, 3178},
                {456, "title-prompt-hidden-first",
                 TitlePromptVariant::VariantB, 3144},
                {466, "title-prompt-visible-second",
                 TitlePromptVariant::VariantA, 3134},
                {520, "title-prompt-hidden-second",
                 TitlePromptVariant::VariantB, 3080},
                {530, "title-prompt-visible-third",
                 TitlePromptVariant::VariantA, 3070},
            }};
        for (const auto& checkpoint : kTitlePromptCheckpoints) {
            while (title_frame < checkpoint.frame &&
                   state_.phase == StartupIntegrationPhase::title_ready) {
                if (!pump()) {
                    return fail("title prompt schedule did not advance");
                }
                ++title_frame;
            }
            if (state_.phase != StartupIntegrationPhase::title_ready ||
                title_frame != checkpoint.frame ||
                !save_title_prompt_checkpoint(
                    checkpoint.label, checkpoint.variant,
                    checkpoint.frames_remaining)) {
                return fail("title prompt visibility or cadence mismatch");
            }
        }

        // The source title loop is interactive, but its complete logo/text
        // presentation is time-driven. Continue the long-hold captures after
        // the earlier frame-300 and matched-reference snapshots.
        constexpr std::array<int, 3> kTitleCheckpoints{900, 1500, 3000};
        for (const auto checkpoint : kTitleCheckpoints) {
            while (title_frame < checkpoint &&
                   state_.phase == StartupIntegrationPhase::title_ready) {
                if (!pump()) {
                    return fail("title presentation did not advance");
                }
                ++title_frame;
            }
            if (state_.phase != StartupIntegrationPhase::title_ready) {
                return fail("title presentation left title-ready early");
            }
            record("title.checkpoint." + std::to_string(checkpoint));
            if (!write_snapshot("title_" + std::to_string(checkpoint))) {
                return 3;
            }
            if (checkpoint == 900 &&
                !save_title_sprite_checkpoint(
                    "title-sprite-settled-900", false)) {
                return fail("title sprite projection reappeared in the "
                            "settled title");
            }
            if (checkpoint == 900) {
                record("title.sprite.settled_900");
            }
        }

        const auto save_title_exit_snapshot =
            [&](const std::string& label, const std::uint32_t fade_vblank) {
                const auto scene = render_scene();
                const auto path = output_directory / (label + ".bmp");
                if (scene.size() != QSize(kSceneWidth, kSceneHeight) ||
                    !scene.save(QString::fromStdWString(path.wstring()))) {
                    return false;
                }
                title_exit_trace
                    << label << '\t' << frames << '\t' << fade_vblank << '\t'
                    << static_cast<unsigned>(title_loop_.exit_palette_fade
                                                 .passes_completed)
                    << '\t'
                    << static_cast<unsigned>(title_loop_.exit_palette_fade
                                                 .vblanks_in_pass)
                    << '\t'
                    << (title_loop_.exit_palette_fade.active ? 1 : 0) << '\t'
                    << (title_loop_.exit_palette_fade.complete ? 1 : 0)
                    << '\t' << title_exit_palette_steps_ << '\t'
                    << startup_integration_phase_name(state_.phase) << '\n';
                return true;
            };
        const auto image_is_black = [](const QImage& image) {
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    if (image.pixelColor(x, y) != QColor(Qt::black)) {
                        return false;
                    }
                }
            }
            return true;
        };

        if (sampled_title_byte(Qt::Key_Space) != kTitleStartButton ||
            sampled_title_byte(Qt::Key_Return) != kTitleStartButton ||
            sampled_title_byte(Qt::Key_Enter) != kTitleStartButton ||
            sampled_title_byte(Qt::Key_Z) != kStartupMenuA ||
            sampled_title_byte(Qt::Key_X) != kStartupMenuB) {
            return fail("title keys did not distinguish Start from menu A/B");
        }
        pending_title_controller_word_ = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(kTitleStartButton) << 8U);
        if (!pump() || state_.phase != StartupIntegrationPhase::title_ready ||
            !title_loop_.exit_palette_fade.active ||
            title_loop_.exit_palette_fade.passes_completed != 0U ||
            title_exit_palette_steps_ != 0U ||
            title_mode_entry_service_.active ||
            title_mode_entry_service_.complete ||
            !save_title_exit_snapshot("title-exit-fade-start", 0U)) {
            return fail("Start did not begin a zero-pass title fade");
        }
        record("title.exit.start_requested");

        const auto frozen_logo_columns = title_loop_.logo.columns_revealed;
        const auto frozen_sprite_phase = title_loop_.sprite.phase;
        const auto frozen_sprite_x = title_loop_.sprite.x;
        const auto frozen_animation_offset =
            title_loop_.animation_timer.animation_offset;
        const auto frozen_circuit_phase = title_loop_.circuit_lights.phase;
        const auto frozen_prompt_frames = title_loop_.prompt.frames_remaining;
        auto previous_fade_pass = render_scene();
        constexpr auto kExitFadeVblankCount = static_cast<std::uint32_t>(
            kTitleExitPalettePasses * kTitleExitVblanksPerPalettePass);
        for (std::uint32_t vblank = 1U; vblank <= kExitFadeVblankCount;
             ++vblank) {
            if (!pump()) {
                return fail("title exit fade stopped before pass 17");
            }
            const auto expected_passes = static_cast<std::uint8_t>(
                vblank / kTitleExitVblanksPerPalettePass);
            const auto expected_vblanks_in_pass = static_cast<std::uint8_t>(
                vblank % kTitleExitVblanksPerPalettePass);
            const bool final_vblank = vblank == kExitFadeVblankCount;
            if (state_.phase != StartupIntegrationPhase::title_ready ||
                title_loop_.exit_palette_fade.passes_completed !=
                    expected_passes ||
                title_loop_.exit_palette_fade.vblanks_in_pass !=
                    expected_vblanks_in_pass ||
                title_loop_.exit_palette_fade.active == final_vblank ||
                title_loop_.exit_palette_fade.complete != final_vblank ||
                title_exit_palette_steps_ != expected_passes ||
                title_loop_.logo.columns_revealed != frozen_logo_columns ||
                title_loop_.sprite.phase != frozen_sprite_phase ||
                title_loop_.sprite.x != frozen_sprite_x ||
                title_loop_.animation_timer.animation_offset !=
                    frozen_animation_offset ||
                title_loop_.circuit_lights.phase != frozen_circuit_phase ||
                title_loop_.prompt.frames_remaining != frozen_prompt_frames) {
                return fail("title state or palette cadence advanced during "
                            "the exit fade");
            }

            const auto scene = render_scene();
            if ((vblank & 1U) != 0U) {
                if (scene != previous_fade_pass) {
                    return fail("title fade changed pixels before the second "
                                "VBlank of a palette pass");
                }
                title_exit_trace
                    << "vblank" << vblank << '\t' << frames << '\t' << vblank
                    << '\t' << static_cast<unsigned>(expected_passes) << '\t'
                    << static_cast<unsigned>(expected_vblanks_in_pass) << '\t'
                    << (title_loop_.exit_palette_fade.active ? 1 : 0) << '\t'
                    << (title_loop_.exit_palette_fade.complete ? 1 : 0)
                    << '\t' << title_exit_palette_steps_ << '\t'
                    << startup_integration_phase_name(state_.phase) << '\n';
                continue;
            }

            if (!save_title_exit_snapshot(
                    "title-exit-fade-pass-" +
                        (expected_passes < 10U ? std::string("0")
                                               : std::string()) +
                        std::to_string(expected_passes),
                    vblank)) {
                return fail("could not save a full-screen title fade phase");
            }
            if (expected_passes < 7U && scene == previous_fade_pass) {
                return fail("a source-visible palette pass did not darken "
                            "the title image");
            }
            if (image_is_black(scene) != (expected_passes >= 7U)) {
                return fail("title image did not reach and retain black at "
                            "the source palette level");
            }
            previous_fade_pass = scene;
        }
        if (!title_mode_entry_service_.active ||
            title_mode_entry_service_.transactions_started != 1U ||
            title_mode_entry_service_.vblanks_in_transaction != 0U ||
            title_mode_entry_transactions_ != 1U ||
            !save_title_exit_snapshot("title-exit-fade-complete", 34U) ||
            !image_is_black(render_scene())) {
            return fail("completed 17-pass title fade did not hold full black");
        }
        record("title.exit.palette_transition_complete");

        const auto save_title_mode_entry_snapshot =
            [&](const std::string& label, const std::uint32_t vblank) {
                const auto scene = render_scene();
                const auto path = output_directory / (label + ".bmp");
                if (scene.size() != QSize(kSceneWidth, kSceneHeight) ||
                    !scene.save(QString::fromStdWString(path.wstring()))) {
                    return false;
                }
                title_mode_entry_trace
                    << label << '\t' << frames << '\t' << vblank << '\t'
                    << static_cast<unsigned>(title_mode_entry_service_
                                                 .transactions_started)
                    << '\t'
                    << static_cast<unsigned>(title_mode_entry_service_
                                                 .vblanks_in_transaction)
                    << '\t'
                    << (title_mode_entry_service_.active ? 1 : 0) << '\t'
                    << (title_mode_entry_service_.complete ? 1 : 0) << '\t'
                    << title_mode_entry_flushes_ << '\t'
                    << title_mode_entry_audio_ticks_ << '\t'
                    << startup_integration_phase_name(state_.phase) << '\n';
                return true;
            };
        for (std::uint32_t vblank = 1U; vblank <= 18U; ++vblank) {
            if (!pump()) {
                return fail("title mode-entry service stopped before 18 VBlanks");
            }
            const auto expected_transactions = static_cast<std::uint8_t>(
                std::min<std::uint32_t>(
                    1U + vblank / kTitleModeEntryVblanksPerTransaction,
                    kTitleModeEntryAudioTransactions));
            const auto expected_vblanks_in_transaction =
                static_cast<std::uint8_t>(
                    vblank % kTitleModeEntryVblanksPerTransaction);
            const bool final_vblank = vblank == 18U;
            const auto expected_phase = final_vblank
                ? StartupIntegrationPhase::start_options_shell
                : StartupIntegrationPhase::title_ready;
            if (state_.phase != expected_phase ||
                title_mode_entry_service_.transactions_started !=
                    expected_transactions ||
                title_mode_entry_service_.vblanks_in_transaction !=
                    expected_vblanks_in_transaction ||
                title_mode_entry_service_.active == final_vblank ||
                title_mode_entry_service_.complete != final_vblank ||
                title_mode_entry_transactions_ != expected_transactions ||
                title_mode_entry_vblanks_ != vblank ||
                title_mode_entry_flushes_ != (final_vblank ? 1U : 0U) ||
                title_mode_entry_audio_ticks_ != (final_vblank ? 1U : 0U)) {
                return fail("title mode-entry command/VBlank cadence diverged "
                            "from the source transaction sequence");
            }
            const auto label = final_vblank
                ? std::string("title-mode-entry-vblank-18-start-options-shell")
                : std::string("title-mode-entry-vblank-") +
                      (vblank < 10U ? std::string("0") : std::string()) +
                      std::to_string(vblank);
            const auto scene = render_scene();
            if (!save_title_mode_entry_snapshot(label, vblank) ||
                (final_vblank
                     ? image_is_black(scene) ||
                           !startup_shell_survives(scene)
                     : !image_is_black(scene))) {
                return fail("mode-entry hold or clean Start Options redraw "
                            "failed full-screen review checks");
            }
        }

        advance_one_source_frame();
        if (state_.phase != StartupIntegrationPhase::start_options) {
            return fail("Start Options did not reveal after its shell VBlank");
        }

        const auto start_options_destination = render_scene();
        if (state_.start_options.selected != StartOptionsChoice::new_game ||
            image_is_black(start_options_destination) ||
            !startup_shell_survives(start_options_destination) ||
            !image_region_differs(start_options_destination, previous_fade_pass,
                                  0, 0, kSceneWidth, kSceneHeight) ||
            !write_snapshot("title-exit-destination-start-options")) {
            return fail("Start Options did not cleanly replace the faded title");
        }
        record("title.exit.destination_start_options");
        record("menu.start_options.new_game");
        if (!write_snapshot("02_start_options_new_game")) {
            return 3;
        }

        // Exercise the Load Game branch and its source-visible empty-slot
        // message before starting New Game. This is the path the old smoke
        // test skipped entirely.
        advance_startup_integration(
            state_, kStartupMenuDown, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::start_options ||
            state_.start_options.selected != StartOptionsChoice::load_game) {
            return fail("Start Options did not select Load Game");
        }
        record("menu.start_options.load_game");
        if (!write_snapshot("03_start_options_load_game")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::save_load) {
            return fail("Load Game did not enter Save/Load");
        }
        record("menu.save_load.slot_one");
        if (!write_snapshot("04_save_load_slot_one")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::save_load ||
            state_.save_load.message != SaveLoadMessage::no_save_data) {
            return fail("empty Save/Load slot did not publish no-save message");
        }
        record("menu.save_load.no_save_message");
        if (!write_snapshot("05_save_load_no_save_message")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::start_options_shell) {
            return fail("no-save message did not return to Start Options");
        }
        record("menu.start_options.return_shell_from_load");
        if (!write_snapshot("06_start_options_shell_return_from_load")) {
            return 3;
        }
        advance_one_source_frame();
        if (state_.phase != StartupIntegrationPhase::start_options) {
            return fail("Start Options shell did not reveal after Load return");
        }
        record("menu.start_options.returned_from_load");
        if (!write_snapshot("07_start_options_returned_from_load")) {
            return 3;
        }

        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            return fail("New Game did not enter archetype selection");
        }
        record("menu.archetype.samurai");
        if (!write_snapshot("08_archetype_samurai")) {
            return 3;
        }
        advance_startup_integration(
            state_, kStartupMenuDown, startup_callbacks_);
        if (state_.joshua_archetype.selected !=
            JoshuaArchetypeChoice::decker) {
            return fail("archetype selector did not reach Decker");
        }
        record("menu.archetype.decker");
        if (!write_snapshot("09_archetype_decker")) {
            return 3;
        }
        advance_startup_integration(
            state_, kStartupMenuDown, startup_callbacks_);
        if (state_.joshua_archetype.selected !=
            JoshuaArchetypeChoice::gator_shaman) {
            return fail("archetype selector did not reach Gator Shaman");
        }
        record("menu.archetype.gator_shaman");
        if (!write_snapshot("10_archetype_gator_shaman")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        qa_finish_archetype_exit_fade();
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            return fail("Gator Shaman did not start opening intro");
        }
        record("opening.initial");
        if (!write_snapshot("11_opening_initial")) {
            return 3;
        }

        constexpr std::array<std::uint32_t, 23> kOpeningCheckpoints{
            300U, 600U, 604U, 606U, 608U, 610U, 612U, 614U,
            620U, 628U, 630U, 648U, 654U, 662U, 682U, 900U,
            1200U, 1500U, 1800U, 2100U, 2400U, 2700U, 3000U};
        std::size_t next_checkpoint = 0U;
        struct CabCursorCheckpoint {
            std::uint16_t post_intro_frame;
            const char* label;
        };
        constexpr std::array<CabCursorCheckpoint, 12> kCabCursorCheckpoints{{
            {0U, "mode4_phase0"},
            {2U, "mode4_phase1"},
            {4U, "mode4_phase2"},
            {6U, "mode4_phase3"},
            {8U, "mode4_phase4"},
            {10U, "mode4_settled"},
            {75U, "mode2_phase0"},
            {77U, "mode2_phase1"},
            {79U, "mode2_phase2"},
            {81U, "mode2_phase3"},
            {83U, "mode2_phase4"},
            {85U, "mode2_settled"},
        }};
        std::size_t next_cab_cursor_checkpoint = 0U;
        auto previous_stage = opening_stage_;
        auto previous_scene = opening_scene_;
        while (state_.phase == StartupIntegrationPhase::opening_intro) {
            if (!pump()) {
                return fail("opening did not complete");
            }
            const auto opening_frame = state_.opening_intro.source_frame;
            if (next_checkpoint < kOpeningCheckpoints.size() &&
                opening_frame == kOpeningCheckpoints[next_checkpoint]) {
                const auto label = "opening.checkpoint." +
                                   std::to_string(opening_frame);
                record(label);
                if (!write_snapshot("opening_" +
                                    std::to_string(opening_frame))) {
                    return 3;
                }
                ++next_checkpoint;
            }
            if (opening_stage_ != previous_stage ||
                opening_scene_ != previous_scene) {
                const auto label =
                    "opening.transition." + std::to_string(opening_frame);
                record(label);
                const auto name =
                    "opening_transition_" + std::to_string(opening_frame) +
                    "_" + opening_intro_stage_name(opening_stage_) + "_" +
                    opening_intro_scene_name(opening_scene_);
                if (!write_snapshot(name)) {
                    return 3;
                }
                previous_stage = opening_stage_;
                previous_scene = opening_scene_;
            }
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.scene == OpeningIntroScene::cab_map &&
                next_cab_cursor_checkpoint < kCabCursorCheckpoints.size() &&
                state_.opening_intro.post_intro_frame ==
                    kCabCursorCheckpoints[next_cab_cursor_checkpoint]
                        .post_intro_frame) {
                const auto& checkpoint =
                    kCabCursorCheckpoints[next_cab_cursor_checkpoint];
                record(std::string("opening.cab_cursor.") + checkpoint.label);
                if (!write_snapshot(std::string("cab_cursor_") +
                                    checkpoint.label)) {
                    return 3;
                }
                ++next_cab_cursor_checkpoint;
            }
        }
        if (next_cab_cursor_checkpoint != kCabCursorCheckpoints.size()) {
            return fail("cab cursor shrink/hold checkpoints were incomplete");
        }
        if (state_.phase != StartupIntegrationPhase::field_ready ||
            !field_scene_runtime_.ready()) {
            return fail("opening did not publish a ready field session");
        }
        record("field.ready");
        if (!write_snapshot("99_field_ready")) {
            return 3;
        }

        // New Game enters the map-event scope at the scene-2 spawn. Let the
        // content-authored one-shot event run through normal field frames,
        // then dismiss Stoker's initial offer before checking field menus.
        if (!field_scene_runtime_.opening_auto_walk_active_for_dump()) {
            return fail("New Game did not arm the scripted Stoker opening event");
        }
        for (std::uint32_t frame = 0U;
             frame < 256U && !field_scene_runtime_.dialogue_open(); ++frame) {
            field_scene_runtime_.advance_frame();
        }
        const auto& opening_arrival = field_scene_runtime_.dialogue_panel();
        if (!field_scene_runtime_.dialogue_open() ||
            field_scene_runtime_.opening_auto_walk_active_for_dump() ||
            opening_arrival.actor_name != "Stoker" ||
            opening_arrival.option_count != 1U ||
            !write_snapshot("opening_stoker_arrival")) {
            return fail("scripted Stoker opening did not reach its arrival dialogue");
        }
        record("field.map_event.stoker_opening_arrival");
        const auto press_field_button = [this](const std::uint8_t button) {
            settle_qa_dialogue_input(field_scene_runtime_);
            field_scene_runtime_.set_controller_port_byte(button);
            field_scene_runtime_.advance_frame();
            field_scene_runtime_.set_controller_port_byte(0U);
            field_scene_runtime_.advance_frame();
        };
        press_field_button(0x40U); // A: open the scripted first-visit menu.
        const auto& opening_offer = field_scene_runtime_.dialogue_panel();
        if (!field_scene_runtime_.dialogue_open() ||
            opening_offer.actor_name != "Stoker" ||
            opening_offer.option_count != 3U ||
            opening_offer.options[2].source_label.find("Michael's stuff") ==
                std::string_view::npos ||
            !write_snapshot("opening_stoker_package_offer")) {
            return fail("first-visit Stoker menu did not expose Michael's package choice");
        }
        record("field.script_menu.stoker_first_visit");
        press_field_button(0x10U); // B: choose the scripted leave option.
        if (!field_scene_runtime_.dialogue_open() ||
            field_scene_runtime_.dialogue_panel().option_count != 0U ||
            !write_snapshot("opening_stoker_leave_notice")) {
            return fail("Stoker leave choice did not replace the menu with its notice");
        }
        press_field_button(0x40U); // A: dismiss the notice.
        if (!finish_field_modal_return_transition(field_scene_runtime_) ||
            field_scene_runtime_.dialogue_open() ||
            field_scene_runtime_.opening_auto_walk_active_for_dump() ||
            !write_snapshot("opening_stoker_returned_to_field")) {
            return fail("Stoker dialogue did not cleanly return to the field");
        }
        record("field.map_event.stoker_opening_returned_to_field");

        // The command overview must be a re-enterable field service, not a
        // one-shot presentation. Drive the same typed controller path as the
        // Qt window and retain both open frames as QA artifacts.
        if (!field_scene_runtime_.open_command_overview_for_dump()) {
            return fail("first field command overview open failed");
        }
        record("field.command_overview.open_1");
        if (!write_snapshot("100_field_command_open_1")) {
            return 3;
        }
        field_scene_runtime_.set_controller_port_byte(0U);
        field_scene_runtime_.advance_frame();
        field_scene_runtime_.set_controller_port_byte(0x80U);
        field_scene_runtime_.advance_frame();
        if (field_scene_runtime_.command_overview_open()) {
            return fail("field command overview did not close on Start");
        }
        record("field.command_overview.returned_to_field");
        if (!write_snapshot("101_field_after_command_close")) {
            return 3;
        }
        if (!qa_finish_command_overview_field_return()) {
            return fail("field command overview return did not finish at P+112");
        }
        field_scene_runtime_.set_controller_port_byte(0U);
        field_scene_runtime_.advance_frame();
        if (!field_scene_runtime_.open_command_overview_for_dump()) {
            return fail("second field command overview open failed");
        }
        record("field.command_overview.open_2");
        if (!write_snapshot("102_field_command_open_2")) {
            return 3;
        }

        std::ofstream events(output_directory / "event-log.txt");
        for (const auto& event : state_.event_log) {
            events << event << '\n';
        }
        std::ofstream boundaries(output_directory / "presentation-boundaries.log");
        for (const auto& boundary : presentation_boundaries_) {
            boundaries << boundary << '\n';
        }
        std::cout << "startup_qa=passed output="
                  << output_directory.string() << " opening_frames="
                  << state_.opening_intro.source_frame << '\n';
        return 0;
    }

    int run_title_idle_restart_qa(
        const std::filesystem::path& output_directory) {
        host_frame_timer_.stop();
        // Frame-indexed: the Lua presentation holds are verified separately.
        opening_presentation_holds_enabled_ = false;
        std::filesystem::create_directories(output_directory);
        const auto save = [&output_directory](const std::string_view name,
                                              const QImage& image) {
            return image.save(QString::fromStdWString(
                (output_directory / std::string(name)).wstring()));
        };
        const auto is_black = [](const QImage& image) {
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    if (image.pixelColor(x, y) != QColor(0, 0, 0)) {
                        return false;
                    }
                }
            }
            return true;
        };
        std::ofstream trace(output_directory / "title-idle-restart.tsv");
        if (!trace) {
            std::cerr << "title idle restart QA: could not write trace\n";
            return 3;
        }
        trace << "label\tphase\tframe\tactivity_timer\t"
                 "title_palette_passes\tmode_entry_vblanks\t"
                 "restart_palette_passes\n";

        if (state_.phase != StartupIntegrationPhase::pre_title ||
            state_.pre_title.presentation_stage !=
                PreTitleSequenceState::PresentationStage::sega_entry) {
            std::cerr << "title idle restart QA: initial pre-title state "
                         "was not clean\n";
            return 3;
        }
        const auto fresh_boot = render_scene();
        if (!save("fresh-boot.bmp", fresh_boot)) {
            std::cerr << "title idle restart QA: fresh boot dump failed\n";
            return 3;
        }

        std::uint32_t first_boot_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::pre_title &&
               first_boot_frames < 1200U) {
            advance_one_source_frame();
            ++first_boot_frames;
        }
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            title_loop_.prompt.frames_remaining !=
                TitlePromptSchedule::kInitialFrames ||
            title_loop_.controller_events.activity_timer != 0) {
            std::cerr << "title idle restart QA: first title did not start "
                         "with a clean idle timer\n";
            return 3;
        }
        const auto first_title = render_scene();
        if (!save("first-title.bmp", first_title)) {
            std::cerr << "title idle restart QA: first title dump failed\n";
            return 3;
        }
        trace << "first_title\t"
              << startup_integration_phase_name(state_.phase) << '\t'
              << first_boot_frames << '\t'
              << title_loop_.controller_events.activity_timer << '\t'
              << static_cast<unsigned>(
                     title_loop_.exit_palette_fade.passes_completed)
              << '\t' << title_mode_entry_vblanks_ << '\t'
              << static_cast<unsigned>(
                     title_idle_restart_fade_.passes_completed)
              << '\n';

        // ROM uVar1 begins at 0xE10 and exits after the final zero-count
        // iteration, so 3,601 title updates elapse before the caller path.
        constexpr auto kNoInputTitleUpdates =
            TitlePromptSchedule::kInitialFrames + 1U;
        for (std::uint32_t frame = 0U; frame < kNoInputTitleUpdates - 1U;
             ++frame) {
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            title_loop_.prompt.finished ||
            title_loop_.controller_events.activity_timer != 0) {
            std::cerr << "title idle restart QA: title ended before its "
                         "source countdown\n";
            return 3;
        }
        if (!save("title-before-timeout.bmp", render_scene())) {
            std::cerr << "title idle restart QA: pre-timeout dump failed\n";
            return 3;
        }
        advance_one_source_frame();
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            !title_loop_.prompt.finished ||
            !title_loop_.exit_palette_fade.active ||
            title_loop_.controller_events.activity_timer != 0) {
            std::cerr << "title idle restart QA: no-input timeout did not "
                         "start the source fade\n";
            return 3;
        }
        if (!save("title-timeout-edge.bmp", render_scene())) {
            std::cerr << "title idle restart QA: timeout edge dump failed\n";
            return 3;
        }

        std::uint32_t title_fade_frames = 0U;
        while (!title_mode_entry_service_.active &&
               state_.phase == StartupIntegrationPhase::title_ready &&
               title_fade_frames < 40U) {
            advance_one_source_frame();
            ++title_fade_frames;
        }
        if (!title_mode_entry_service_.active || title_fade_frames != 34U) {
            std::cerr << "title idle restart QA: initial fade was not 34 "
                         "VBlanks\n";
            return 3;
        }
        const auto black = render_scene();
        if (!save("title-black-service-start.bmp", black)) {
            std::cerr << "title idle restart QA: black hold dump failed\n";
            return 3;
        }
        if (!is_black(black)) {
            std::cerr << "title idle restart QA: source fade did not leave "
                         "a full-screen black hold\n";
            return 3;
        }

        const auto mode_entry_vblanks_before = title_mode_entry_vblanks_;
        for (std::uint8_t frame = 0U; frame < 18U; ++frame) {
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            title_mode_entry_vblanks_ - mode_entry_vblanks_before != 18U ||
            !title_idle_restart_fade_.active ||
            !title_mode_entry_service_.complete) {
            std::cerr << "title idle restart QA: idle caller did not begin "
                         "its restart transition after 18 VBlanks\n";
            return 3;
        }
        if (!save("title-black-after-audio-service.bmp", render_scene())) {
            std::cerr << "title idle restart QA: post-service dump failed\n";
            return 3;
        }

        const auto restart_vblanks_before = title_vblanks_;
        const auto restart_palette_steps_before = title_exit_palette_steps_;
        for (std::uint8_t frame = 1U; frame <= 34U; ++frame) {
            advance_one_source_frame();
            if (frame < 34U &&
                state_.phase != StartupIntegrationPhase::title_ready) {
                std::cerr << "title idle restart QA: boot restarted before "
                             "the second 17-pass transition completed\n";
                return 3;
            }
        }
        if (state_.phase != StartupIntegrationPhase::pre_title ||
            state_.pre_title.presentation_stage !=
                PreTitleSequenceState::PresentationStage::sega_entry ||
            title_vblanks_ - restart_vblanks_before != 34U ||
            title_exit_palette_steps_ - restart_palette_steps_before != 17U ||
            !state_.asset_catalog.complete()) {
            std::cerr << "title idle restart QA: second fade did not restart "
                         "the source pre-title state\n";
            return 3;
        }
        const auto restarted_boot = render_scene();
        if (restarted_boot.convertToFormat(QImage::Format_RGB32) !=
                fresh_boot.convertToFormat(QImage::Format_RGB32) ||
            !save("actual.bmp", restarted_boot)) {
            std::cerr << "title idle restart QA: replay boot retained stale "
                         "pixels or could not write actual.bmp\n";
            return 3;
        }
        trace << "boot_restarted\t"
              << startup_integration_phase_name(state_.phase) << '\t'
              << (kNoInputTitleUpdates + title_fade_frames + 18U + 34U)
              << '\t' << title_loop_.controller_events.activity_timer << '\t'
              << static_cast<unsigned>(
                     title_loop_.exit_palette_fade.passes_completed)
              << '\t' << title_mode_entry_vblanks_ - mode_entry_vblanks_before
              << '\t'
              << static_cast<unsigned>(
                     title_idle_restart_fade_.passes_completed)
              << '\n';

        bool replayed_shock_captured = false;
        std::uint32_t replay_boot_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::pre_title &&
               replay_boot_frames < 1200U) {
            advance_one_source_frame();
            ++replay_boot_frames;
            if (!replayed_shock_captured &&
                state_.pre_title.presentation_stage ==
                    PreTitleSequenceState::PresentationStage::sega_shock) {
                if (!save("replayed-sega-shock.bmp", render_scene())) {
                    std::cerr << "title idle restart QA: replayed shock "
                                 "dump failed\n";
                    return 3;
                }
                replayed_shock_captured = true;
            }
        }
        const auto replayed_shock_sound = std::any_of(
            pre_title_audio_requests_.begin(), pre_title_audio_requests_.end(),
            [](const PreTitleAudioRequest& request) {
                return request.command == 0x5CU;
            });
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            !replayed_shock_captured || !replayed_shock_sound ||
            title_loop_.prompt.frames_remaining !=
                TitlePromptSchedule::kInitialFrames ||
            title_loop_.logo.columns_revealed != 0U) {
            std::cerr << "title idle restart QA: replay did not reach a clean "
                         "title with its source shock request\n";
            return 3;
        }
        const auto restarted_title = render_scene();
        if (restarted_title.convertToFormat(QImage::Format_RGB32) !=
                first_title.convertToFormat(QImage::Format_RGB32) ||
            !save("restarted-title.bmp", restarted_title)) {
            std::cerr << "title idle restart QA: replayed title differs from "
                         "the first title or could not be dumped\n";
            return 3;
        }
        for (std::uint8_t frame = 0U; frame < 90U; ++frame) {
            advance_one_source_frame();
        }
        const auto restarted_title_delay = render_scene();
        if (!is_black(restarted_title_delay) ||
            !save("replayed-title-black-frame-90.bmp",
                  restarted_title_delay)) {
            std::cerr << "title idle restart QA: replayed logo delay did "
                         "not preserve its full-screen black phase\n";
            return 3;
        }
        advance_one_source_frame();
        const auto restarted_first_logo_column = render_scene();
        if (is_black(restarted_first_logo_column) ||
            !save("replayed-title-first-logo-column.bmp",
                  restarted_first_logo_column)) {
            std::cerr << "title idle restart QA: logo did not emerge after "
                         "the source black delay\n";
            return 3;
        }
        trace << "restarted_title\t"
              << startup_integration_phase_name(state_.phase) << '\t'
              << replay_boot_frames << '\t'
              << title_loop_.controller_events.activity_timer << '\t'
              << static_cast<unsigned>(
                     title_loop_.exit_palette_fade.passes_completed)
              << '\t' << title_mode_entry_vblanks_ << '\t'
              << static_cast<unsigned>(
                     title_idle_restart_fade_.passes_completed)
              << '\n';

        std::cout << "title_idle_restart_qa=passed output="
                  << output_directory.string() << " initial_boot_frames="
                  << first_boot_frames << " replay_boot_frames="
                  << replay_boot_frames << " title_fade_frames="
                  << title_fade_frames << " idle_restart_fade_vblanks=34\n";
        return 0;
    }
