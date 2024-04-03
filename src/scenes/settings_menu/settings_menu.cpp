#include "settings_menu.hpp"
#include "color_setting_row.hpp"
#include "graphics/window.hpp"
#include "helper/color_literals.hpp"
#include "manager/music_manager.hpp"
#include "manager/resource_manager.hpp"
#include "settings_details.hpp"
#include "ui/components/label.hpp"
#include "ui/components/slider.hpp"
#include "ui/components/text_button.hpp"
#include "ui/layouts/scroll_layout.hpp"

#include <spdlog/spdlog.h>

namespace scenes {

    using namespace details::settings::menu;

    SettingsMenu::SettingsMenu(ServiceProvider* service_provider, const  ui::Layout& layout) : Scene{service_provider, layout}
    , m_main_layout{    utils::size_t_identity<3>(),
    0,
    ui::Direction::Vertical,
                    { 0.1, 0.9 },
                    ui::AbsolutMargin{ 10 },
                    std::pair<double, double>{ 0.05, 0.03 },
                    layout
    },
    m_colors{"#FF33FF"_c, "hsv(120, 0.07, 0.93)"_c, "hsv(140, 0.07, 0.93)"_c, "rgb(246, 255, 61)"_c}
{
        auto focus_helper = ui::FocusHelper{ 1 };

        m_main_layout.add<ui::Label>(
                service_provider, "Settings", service_provider->fonts().get(FontId::Default), Color::white(),
                std::pair<double, double>{ 0.3, 0.6 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );

        const auto scroll_layout_index = m_main_layout.add<ui::ScrollLayout>(
                service_provider, focus_helper.focus_id(), ui::AbsolutMargin{ 10 },
                std::pair<double, double>{ 0.05, 0.03 }
        );

        auto* scroll_layout = m_main_layout.get<ui::ScrollLayout>(scroll_layout_index);


        scroll_layout->add<ui::Label>(
                ui::RelativeItemSize{ scroll_layout->layout(), 0.2 }, service_provider, "Volume",
                service_provider->fonts().get(FontId::Default), Color::white(), std::pair<double, double>{ 0.1, 0.3 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Bottom }
        );

        const auto slider_index = scroll_layout->add<ui::Slider>(
                ui::RelativeItemSize{ scroll_layout->layout(), 0.1 }, focus_helper.focus_id(),
                ui::Slider::Range{ 0.0F, 1.0F },
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
                [this, scroll_layout_index, slider_index](helper::optional<double>) {
                    auto* scroll_layout = this->m_main_layout.get<ui::ScrollLayout>(scroll_layout_index);
                    scroll_layout->get<ui::Slider>(slider_index)->on_change();
                }
        );

        scroll_layout->add<ui::Label>(
                ui::RelativeItemSize{ scroll_layout->layout(), 0.2 }, service_provider, "Colors",
                service_provider->fonts().get(FontId::Default), Color::white(), std::pair<double, double>{ 0.1, 0.3 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Bottom }
        );


        for (usize color_index = 0; color_index < m_colors.size(); ++color_index) {
            const auto& color = m_colors.at(color_index);

            scroll_layout->add<custom_ui::ColorSettingRow>(
                    ui::RelativeItemSize{ scroll_layout->layout(), 0.2 }, service_provider,
                    //TODO use real settings name
                    fmt::format("Color {}", color_index), color,
                    [this, color_index](const Color& updated_color) { this->m_colors.at(color_index) = updated_color; },
                    focus_helper.focus_id()
            );
        }

        m_main_layout.add<ui::TextButton>(
                service_provider, "Return", service_provider->fonts().get(FontId::Default), Color::white(),
                focus_helper.focus_id(),
                [this](const ui::TextButton&) -> bool {
                    m_next_command = Command{ Return{} };
                    return false;
                },
                std::pair<double, double>{ 0.15, 0.85 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center },
                std::pair<double, double>{ 0.1, 0.1 }
        );
    }

    [[nodiscard]] Scene::UpdateResult SettingsMenu::update() {
        m_main_layout.update();

        if (m_next_command.has_value()) {
            return std::visit(
                    helper::overloaded{
                            [this](const Return&) {
                                m_service_provider->music_manager().remove_volume_listener(listener_name);
                                return UpdateResult{ SceneUpdate::StopUpdating, Scene::Pop{} };
                            },
                            [this](const Action& action) {
                                m_next_command = helper::nullopt;
                                if (auto* settings_details = dynamic_cast<settings::SettingsDetails*>(action.widget);
                                    settings_details != nullptr) {

                                    auto change_scene = settings_details->get_details_scene();

                                    return UpdateResult{ SceneUpdate::StopUpdating, std::move(change_scene) };
                                }


                                throw std::runtime_error("Requested action on unknown widget, this is a fatal error");
                            } },
                    m_next_command.value().m_value
            );
        }

        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
    }

    void SettingsMenu::render(const ServiceProvider& service_provider) {

        const auto layout = get_layout();
        service_provider.renderer().draw_rect_filled(layout.get_rect(), Color::black());

        if (not layout.is_full_screen()) {
            service_provider.renderer().draw_rect_outline(layout.get_rect(), Color::white());
        }

        m_main_layout.render(service_provider);
    }

    bool SettingsMenu::handle_event(const SDL_Event& event, const Window* window) {
        if (const auto event_result = m_main_layout.handle_event(event, window); event_result) {
            if (const auto additional = event_result.get_additional();
                additional.has_value() and additional.value().first == ui::EventHandleType::RequestAction) {
                m_next_command = Command{ Action(additional.value().second) };
            }

            return true;
        }

        if (utils::event_is_action(event, utils::CrossPlatformAction::CLOSE)) {
            m_next_command = Command{ Return{} };
            return true;
        }

        if (utils::device_supports_keys()) {

            if (utils::event_is_action(event, utils::CrossPlatformAction::OPEN_SETTINGS)) {
                m_next_command = Command{ Return{} };
                return true;
            }
        }

        return false;
    }

    // we might be in non fullscreen, so we can detect full scene unhovers here
    void SettingsMenu::on_unhover() {
        m_main_layout.on_unhover();
    }


} // namespace scenes
