#include "main_menu.hpp"
#include "graphics/window.hpp"
#include "helper/constants.hpp"
#include "manager/music_manager.hpp"
#include "manager/resource_manager.hpp"
#include "ui/layout.hpp"

namespace scenes {

    MainMenu::MainMenu(ServiceProvider* service_provider, const  ui::Layout& layout)
        : Scene{service_provider, layout},
          m_main_grid{ 0,6,ui::Direction::Vertical, ui::RelativeMargin{layout,ui::Direction::Vertical, 0.05}, std::pair<double, double>{ 0.05, 0.05 
            } ,ui::RelativeLayout{ layout, 0.0, 0.1, 1.0, 0.8 }} {

        auto focus_helper = ui::FocusHelper{ 1 };

        m_main_grid.add<ui::Label>(
                service_provider, constants::program_name, service_provider->fonts().get(FontId::Default),
                Color::white(), std::pair<double, double>{ 0.3, 1.0 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );


        constexpr auto button_size = utils::device_orientation() == utils::Orientation::Landscape
                                             ? std::pair<double, double>{ 0.15, 0.85 }
                                             : std::pair<double, double>{ 0.5, 0.85 };
        constexpr auto button_alignment =
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center };
        constexpr auto button_margins = utils::device_orientation() == utils::Orientation::Landscape
                                                ? std::pair<double, double>{ 0.1, 0.1 }
                                                : std::pair<double, double>{ 0.2, 0.2 };

        m_main_grid.add<ui::TextButton>(
                service_provider, "Play", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_next_command = Command::OpenPlaySelection;
                    return false;
                },
                button_size, button_alignment, button_margins
        );

        m_main_grid.add<ui::TextButton>(
                service_provider, "Settings", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_next_command = Command::OpenSettingsMenu;
                    return false;
                },
                button_size, button_alignment, button_margins
        );

        m_main_grid.add<ui::TextButton>(
                service_provider, "About", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_next_command = Command::OpenAboutPage;
                    return false;
                },
                button_size, button_alignment, button_margins
        );

        m_main_grid.add<ui::TextButton>(
                service_provider, "Achievements", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_next_command = Command::OpenAchievements;
                    return false;
                },
                button_size, button_alignment, button_margins
        );
        m_main_grid.get<ui::TextButton>(4)->disable();

        m_main_grid.add<ui::TextButton>(
                service_provider, "Exit", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_next_command = Command::Exit;
                    return false;
                },
                button_size, button_alignment, button_margins
        );


#if defined(_HAVE_DISCORD_SDK)
        if (auto& discord_instance = service_provider->discord_instance(); discord_instance.has_value()) {

            discord_instance->set_activity(
                    DiscordActivityWrapper("Selecting playmode", discord::ActivityType::Playing)
                            .set_large_image("Playing OOPetris", constants::discord::ArtAsset::logo)
                            .set_start_timestamp(std::chrono::system_clock::now())
            );
        }

#endif


        service_provider->music_manager()
                .load_and_play_music(
                        utils::get_assets_folder() / "music" / utils::get_supported_music_extension("01. Main Menu")
                )
                .and_then(utils::log_error);
    }

    [[nodiscard]] Scene::UpdateResult MainMenu::update() {
        m_main_grid.update();

        if (m_next_command.has_value()) {
            switch (m_next_command.value()) {
                case Command::OpenPlaySelection:
                    // perform a push and reset the command, so that the music keeps playing the entire time
                    m_next_command = helper::nullopt;
                    return UpdateResult{
                        SceneUpdate::StopUpdating,
                        Scene::Push{SceneId::PlaySelectMenu, ui::FullScreenLayout{ m_service_provider->window() }}
                    };
                case Command::OpenAboutPage:
                    // perform a push and reset the command, so that the music keeps playing the entire time
                    m_next_command = helper::nullopt;
                    return UpdateResult{
                        SceneUpdate::StopUpdating,
                        Scene::Push{SceneId::AboutPage, ui::FullScreenLayout{ m_service_provider->window() }}
                    };
                case Command::OpenSettingsMenu:
                    // perform a push and reset the command, so that the music keeps playing the entire time
                    m_next_command = helper::nullopt;
                    return UpdateResult{
                        SceneUpdate::StopUpdating,
                        Scene::Push{SceneId::SettingsMenu, ui::FullScreenLayout{ m_service_provider->window() }}
                    };
                case Command::OpenAchievements:
                    // perform a push and reset the command, so that the music keeps playing the entire time
                    m_next_command = helper::nullopt;
                    return UpdateResult{
                        SceneUpdate::StopUpdating,
                        Scene::Push{SceneId::AchievementsPage, ui::FullScreenLayout{ m_service_provider->window() }}
                    };
                case Command::Exit:
                    return UpdateResult{ SceneUpdate::StopUpdating, Scene::Exit{} };
                default:
                    utils::unreachable();
            }
        }
        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
    }

    void MainMenu::render(const ServiceProvider& service_provider) {
        m_main_grid.render(service_provider);
    }

    bool MainMenu::handle_event(const SDL_Event& event, const Window* window) {
        if (m_main_grid.handle_event(event, window)) {
            return true;
        }

        if (utils::event_is_action(event, utils::CrossPlatformAction::CLOSE)) {
            m_next_command = Command::Exit;
            return true;
        }
        return false;
    }

} // namespace scenes
