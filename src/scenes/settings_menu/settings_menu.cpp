#include "settings_menu.hpp"
#include "graphics/window.hpp"
#include "manager/music_manager.hpp"
#include "manager/resource_manager.hpp"

#include <spdlog/spdlog.h>

namespace scenes {

    SettingsMenu::SettingsMenu(ServiceProvider* service_provider, const  ui::Layout& layout) : Scene{service_provider, layout}
, m_main_grid{ 0,
    4,
    ui::Direction::Vertical,
    ui::RelativeMargin{ layout, ui::Direction::Vertical, 0.05 },
    std::pair<double, double>{ 0.05, 0.05 } ,
    ui::RelativeLayout{layout, 0.0, 0.2, 1.0, 0.5  }
} 
{
        auto focus_helper = ui::FocusHelper{ 1 };

        m_main_grid.add<ui::Label>(
                service_provider, "Settings", service_provider->fonts().get(FontId::Default), Color::white(),
                std::pair<double, double>{ 0.3, 0.6 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );


        m_main_grid.add<ui::Label>(
                service_provider, "Volume", service_provider->fonts().get(FontId::Default), Color::white(),
                std::pair<double, double>{ 0.1, 0.3 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Bottom }
        );

        const auto slider_index = m_main_grid.add<ui::Slider>(
                focus_helper.focus_id(), ui::Slider::Range{ 0.0F, 1.0F },
                [service_provider]() {
                    const auto value = service_provider->music_manager().get_volume();
                    return value.has_value() ? value.value() : 0.0F;
                },
                [service_provider](const double& amount) {
                    const auto mapped_amount = amount <= 0.0F ? helper::nullopt : helper::optional<double>{ amount };
                    return service_provider->music_manager().set_volume(mapped_amount, false, false);
                },
                0.05F, std::pair<double, double>{ 0.6, 1.0 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );

        service_provider->music_manager().add_volume_listener(
                listener_name,
                [this, slider_index](helper::optional<double>) {
                    this->m_main_grid.get<ui::Slider>(slider_index)->on_change();
                }
        );

        m_main_grid.add<ui::TextButton>(
                service_provider, "Return", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_should_exit = true;
                    return false;
                },
                std::pair<double, double>{ 0.15, 0.85 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center },
                std::pair<double, double>{ 0.1, 0.1 }
        );
    }

    [[nodiscard]] Scene::UpdateResult SettingsMenu::update() {
        m_main_grid.update();

        if (m_should_exit) {
            m_service_provider->music_manager().remove_volume_listener(listener_name);
            return UpdateResult{ SceneUpdate::StopUpdating, Scene::Pop{} };
        }
        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
    }

    void SettingsMenu::render(const ServiceProvider& service_provider) {

        const auto layout = get_layout();
        service_provider.renderer().draw_rect_filled(layout.get_rect(), Color::black());

        if (not layout.is_full_screen()) {
            service_provider.renderer().draw_rect_outline(layout.get_rect(), Color::white());
        }

        m_main_grid.render(service_provider);
    }

    bool SettingsMenu::handle_event(const SDL_Event& event, const Window* window) {
        if (m_main_grid.handle_event(event, window)) {
            return true;
        }

        if (utils::event_is_action(event, utils::CrossPlatformAction::CLOSE)) {
            m_should_exit = true;
            return true;
        }

        if (utils::device_supports_keys()) {

            if (utils::event_is_action(event, utils::CrossPlatformAction::OPEN_SETTINGS)) {
                m_should_exit = true;
                return true;
            }
        }

        return false;
    }

    // we might be in non fullscreen, so we can detect full scene unhovers here
    void SettingsMenu::on_unhover() {
        m_main_grid.on_unhover();
    }


} // namespace scenes
