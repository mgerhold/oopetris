#include "online_lobby.hpp"
#include "graphics/window.hpp"
#include "helper/constants.hpp"
#include "manager/music_manager.hpp"
#include "manager/resource_manager.hpp"
#include "ui/components/textinput.hpp"
#include "ui/layout.hpp"
#include "ui/layouts/scroll_layout.hpp"

namespace scenes {

    OnlineLobby::OnlineLobby(ServiceProvider* service_provider, const ui::Layout& layout)
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

        //TODO: after the settings have been reworked, make this url changeable!
        auto maybe_client = lobby::Client::get_client("http://127.0.0.1:5000");
        if (maybe_client.has_value()) {
            client = std::make_unique<lobby::Client>(std::move(maybe_client.value()));
        } else {
            spdlog::error("Error in connecting to lobby client: {}", maybe_client.error());
        }

        auto focus_helper = ui::FocusHelper{ 1 };

        m_main_layout.add<ui::Label>(
                service_provider, "Select Lobby to play in", service_provider->fonts().get(FontId::Default),
                Color::white(), std::pair<double, double>{ 0.5, 1.0 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );

        const auto scroll_layout_index = m_main_layout.add<ui::ScrollLayout>(
                service_provider, focus_helper.focus_id(), ui::AbsolutMargin{ 10 },
                std::pair<double, double>{ 0.05, 0.03 }
        );

        auto* scroll_layout = m_main_layout.get<ui::ScrollLayout>(scroll_layout_index);

        for (auto i = 0; i < 7; ++i) {
            if (i == 2) {
                scroll_layout->add<ui::TextInput>(
                        ui::RelativeItemSize{ scroll_layout->layout(), 0.2 }, service_provider,
                        service_provider->fonts().get(FontId::Symbola), Color::white(), focus_helper.focus_id(),
                        std::pair<double, double>{ 0.9, 0.9 },
                        ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
                );
            } else {
                scroll_layout->add<ui::Button>(
                        ui::RelativeItemSize{ scroll_layout->layout(), 0.2 }, service_provider,
                        fmt::format("Button Nr.: {}", i), service_provider->fonts().get(FontId::Default),
                        Color::white(), focus_helper.focus_id(),
                        [i](const ui::Button&) -> bool {
                            std::cout << "Pressed button: " << i << "\n";
                            return false;
                        },
                        std::pair<double, double>{ 0.8, 1.0 },
                        ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center },
                        std::pair<double, double>{ 0.1, 0.2 }
                );
            }
        }

        constexpr auto button_size = utils::device_orientation() == utils::Orientation::Landscape
                                             ? std::pair<double, double>{ 0.15, 0.85 }
                                             : std::pair<double, double>{ 0.5, 0.85 };
        constexpr auto button_alignment =
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center };
        constexpr auto button_margins = utils::device_orientation() == utils::Orientation::Landscape
                                                ? std::pair<double, double>{ 0.1, 0.1 }
                                                : std::pair<double, double>{ 0.2, 0.2 };

        m_main_layout.add<ui::Button>(
                service_provider, "Return", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::Button&) -> bool {
                    m_next_command = Command::Return;
                    return false;
                },
                button_size, button_alignment, button_margins
        );
    }

    [[nodiscard]] Scene::UpdateResult OnlineLobby::update() {
        m_main_layout.update();

        if (m_next_command.has_value()) {
            switch (m_next_command.value()) {
                case Command::Play:
                    return UpdateResult{
                        SceneUpdate::StopUpdating, Scene::Switch{SceneId::OnlineMultiplayerGame,
                                                                 ui::FullScreenLayout{ m_service_provider->window() }}
                    };
                case Command::Return:
                    return UpdateResult{ SceneUpdate::StopUpdating, Scene::Pop{} };
                default:
                    utils::unreachable();
            }
        }
        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
    }

    void OnlineLobby::render(const ServiceProvider& service_provider) {
        service_provider.renderer().draw_rect_filled(get_layout().get_rect(), Color::black());

        m_main_layout.render(service_provider);
    }

    bool OnlineLobby::handle_event(const SDL_Event& event, const Window* window) {
        // description of intentional behaviour of this scene, even if it seems off:
        // the return button or the scroll layout can have the focus, if the scroll_layout has the focus, it can be scrolled by the scroll wheel and you can move around the focused item of the scroll_layout with up and down, but not with TAB, with tab you can change the focus to the return button, where you can't use the scroll wheel or up / down to change the scroll items, but you still can use click events, they are not affected by focus

        if (m_main_layout.handle_event(event, window)) {
            return true;
        }

        if (utils::event_is_action(event, utils::CrossPlatformAction::CLOSE)) {
            m_next_command = Command::Return;
            return true;
        }
        return false;
    }

} // namespace scenes
