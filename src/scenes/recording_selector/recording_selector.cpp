#if defined(_HAVE_FILE_DIALOGS)
#include "recording_chooser.hpp"
#endif

#include "graphics/window.hpp"
#include "helper/constants.hpp"
#include "helper/optional.hpp"
#include "manager/recording/recording_reader.hpp"
#include "manager/resource_manager.hpp"
#include "recording_component.hpp"
#include "recording_selector.hpp"
#include "scenes/replay_game/replay_game.hpp"
#include "ui/components/text_button.hpp"
#include "ui/layout.hpp"
#include "ui/layouts/scroll_layout.hpp"
#include "ui/widget.hpp"

#include <filesystem>
#include <stdexcept>

namespace scenes {

    RecordingSelector::RecordingSelector(ServiceProvider* service_provider, const ui::Layout& layout)
        : Scene{ service_provider, layout },
          m_main_layout{ 
                utils::size_t_identity<3>(),
                0,
                ui::Direction::Vertical,
                { 0.1, 0.9 },
                ui::AbsolutMargin{ 10 },
                std::pair<double, double>{ 0.05, 0.03 },
                layout
          } {

        auto focus_helper = ui::FocusHelper{ 1 };

        m_main_layout.add<ui::Label>(
                service_provider, "Select Recording to replay", service_provider->fonts().get(FontId::Default),
                Color::white(), std::pair<double, double>{ 0.5, 1.0 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );

        m_main_layout.add<ui::ScrollLayout>(
                service_provider, focus_helper.focus_id(), ui::AbsolutMargin{ 10 },
                std::pair<double, double>{ 0.05, 0.03 }
        );

        add_all_recordings();

        constexpr auto button_size = utils::device_orientation() == utils::Orientation::Landscape
                                             ? std::pair<double, double>{ 0.15, 0.85 }
                                             : std::pair<double, double>{ 0.5, 0.85 };
        constexpr auto button_alignment =
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center };
        constexpr auto button_margins = utils::device_orientation() == utils::Orientation::Landscape
                                                ? std::pair<double, double>{ 0.1, 0.1 }
                                                : std::pair<double, double>{ 0.2, 0.2 };

        m_main_layout.add<ui::TextButton>(
                service_provider, "Return", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_next_command = Command::Return;
                    return false;
                },
                button_size, button_alignment, button_margins
        );
    }

    [[nodiscard]] Scene::UpdateResult RecordingSelector::update() {
        m_main_layout.update();

        if (m_next_command.has_value()) {
            switch (m_next_command.value()) {
                case Command::Action: {
                    m_next_command = helper::nullopt;
                    auto* scroll_layout = m_main_layout.get<ui::ScrollLayout>(1);
                    auto focused_element_index = scroll_layout->get_current_focused_index();
                    //  we could have no focused element
                    // this branch is never taken, if we have another widget then "custom_ui::RecordingComponent" at the focus, since only that requests action and trigger the play command
                    if (not focused_element_index.has_value()) {
                        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
                    }

                    if (scroll_layout->is<custom_ui::RecordingComponent>(focused_element_index.value())) {

                        const auto* focused_element =
                                scroll_layout->get<custom_ui::RecordingComponent>(focused_element_index.value());

                        const auto recording_path = focused_element->metadata().path;

                        return UpdateResult{
                            SceneUpdate::StopUpdating,
                            Scene::RawSwitch{"ReplayGame",
                                             std::make_unique<ReplayGame>(
                                             m_service_provider, ui::FullScreenLayout{ m_service_provider->window() },
                                             recording_path
                                             )}
                        };
                    }
#if defined(_HAVE_FILE_DIALOGS)
                    if (scroll_layout->is<custom_ui::RecordingFileChooser>(focused_element_index.value())) {

                        const auto* focused_element =
                                scroll_layout->get<custom_ui::RecordingFileChooser>(focused_element_index.value());

                        for (const auto& path : focused_element->get_currently_chosen_files()) {
                            m_chosen_paths.push_back(path);
                        }

                        add_all_recordings();

                        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
                    }

#endif


                    throw std::runtime_error("Requested action on unknown layout, this is a fatal error");
                }
                case Command::Return:
                    return UpdateResult{ SceneUpdate::StopUpdating, Scene::Pop{} };
                default:
                    utils::unreachable();
            }
        } // namespace scenes
        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
    }

    void RecordingSelector::render(const ServiceProvider& service_provider) {

        service_provider.renderer().draw_rect_filled(get_layout().get_rect(), Color::black());

        m_main_layout.render(service_provider);
    }

    bool RecordingSelector::handle_event(const SDL_Event& event, const Window* window) {


        if (const auto event_result = m_main_layout.handle_event(event, window); event_result) {
            if (const auto additional = event_result.get_additional();
                additional.has_value() and additional.value() == ui::EventHandleType::RequestAction) {
                m_next_command = Command::Action;
            }

            return true;
        }

        if (utils::event_is_action(event, utils::CrossPlatformAction::CLOSE)) {
            m_next_command = Command::Return;
            return true;
        }
        return false;
    }

    void RecordingSelector::add_all_recordings() {

        std::vector<data::RecordingMetadata> metadata_vector{};

        const auto recording_directory_path = utils::get_root_folder() / constants::recordings_directory;

        if (std::filesystem::exists(recording_directory_path)) {
            for (const auto& file : std::filesystem::recursive_directory_iterator(recording_directory_path)) {
                auto header_value = recorder::RecordingReader::is_header_valid(file.path());
                if (header_value.has_value()) {
                    auto [information, headers] = std::move(header_value.value());
                    metadata_vector.emplace_back(file.path(), data::RecordingSource::Folder, headers, information);
                } else {
                    spdlog::info(
                            "While scanning recordings folder: file {} is not a recording, reason: {}",
                            file.path().string(), header_value.error()
                    );
                }
            }
        }

        if (const auto recording_path = m_service_provider->command_line_arguments().recording_path;
            recording_path.has_value()) {

            const auto recording_path_cl = std::filesystem::path(recording_path.value());
            auto header_value = recorder::RecordingReader::is_header_valid(recording_path_cl);
            if (header_value.has_value()) {
                auto [information, headers] = std::move(header_value.value());
                metadata_vector.emplace_back(
                        recording_path_cl, data::RecordingSource::CommandLine, headers, information
                );
            } else {
                spdlog::error(
                        "Recording file specified by the commandline is not a recording, reason: {}",
                        recording_path_cl.string(), header_value.error()
                );
            }
        }


        for (const auto& selected_path : m_chosen_paths) {
            auto header_value = recorder::RecordingReader::is_header_valid(selected_path);
            if (header_value.has_value()) {
                auto [information, headers] = std::move(header_value.value());
                metadata_vector.emplace_back(selected_path, data::RecordingSource::Manual, headers, information);
            } else {
                spdlog::error(
                        "Recording file specified by the selecting it manually is not a recording, reason: {}",
                        selected_path.string(), header_value.error()
                );
            }
        }


        auto* scroll_layout = m_main_layout.get<ui::ScrollLayout>(1);

        if (scroll_layout->widget_count() != 0) {
            scroll_layout->clear_widgets();
        }

        auto focus_helper = ui::FocusHelper{ 3 };

        for (const auto& metadata : metadata_vector) {
            scroll_layout->add<custom_ui::RecordingComponent>(
                    ui::RelativeItemSize{ scroll_layout->layout(), 0.2 }, m_service_provider, std::ref(focus_helper),
                    metadata
            );
        }

#if defined(_HAVE_FILE_DIALOGS)
        scroll_layout->add<custom_ui::RecordingFileChooser>(
                ui::RelativeItemSize{ scroll_layout->layout(), 0.2 }, m_service_provider, std::ref(focus_helper)
        );
#endif
    }

} // namespace scenes
