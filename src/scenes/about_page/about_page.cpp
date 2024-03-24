#include "about_page.hpp"
#include "graphics/window.hpp"
#include "helper/constants.hpp"
#include "helper/git_helper.hpp"
#include "helper/utils.hpp"
#include "manager/resource_manager.hpp"
#include "platform/capabilities.hpp"
#include "ui/components/image_view.hpp"
#include "ui/components/link_label.hpp"
#include "ui/layouts/tile_layout.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
namespace scenes {

    AboutPage::AboutPage(ServiceProvider* service_provider, const  ui::Layout& layout) : Scene{service_provider, layout}
, m_main_grid{
    0,
    4 + static_cast<u32>(authors.size()),
    ui::Direction::Vertical,
    ui::RelativeMargin{ layout, ui::Direction::Vertical, 0.05 },
    std::pair<double, double>{ 0.05, 0.05 } ,
    ui::RelativeLayout{layout, 0.0, 0.05, 1.0, 0.9  }
} {

        auto focus_helper = ui::FocusHelper{ 1 };

        m_main_grid.add<ui::Label>(
                service_provider, fmt::format("Git Commit: {}", utils::git_commit()),
                service_provider->fonts().get(FontId::Default), Color::white(), std::pair<double, double>{ 0.3, 0.5 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );

        m_main_grid.add<ui::Label>(
                service_provider, fmt::format("Build for: {}", utils::built_for_platform()),
                service_provider->fonts().get(FontId::Default), Color::white(), std::pair<double, double>{ 0.3, 0.5 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );

        m_main_grid.add<ui::Label>(
                service_provider, fmt::format("Features: {}", fmt::join(utils::supported_features(), ", ")),
                service_provider->fonts().get(FontId::Default), Color::white(), std::pair<double, double>{ 0.95, 0.5 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
        );

        m_main_grid.add<ui::Label>(
                service_provider, "Authors:", service_provider->fonts().get(FontId::Default), Color::white(),
                std::pair<double, double>{ 0.2, 0.4 },
                ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Bottom }
        );

        for (const auto& [name, link, image_name] : authors) {


            const auto tile_layout_index = m_main_grid.add<ui::TileLayout>(
                    utils::size_t_identity<2>(), focus_helper.focus_id(), ui::Direction::Horizontal,
                    std::array<double, 1>{ 0.85 }, ui::AbsolutMargin{ 0 }, std::pair<double, double>{ 0.05, 0.03 }
            );

            auto* tile_layout = m_main_grid.get<ui::TileLayout>(tile_layout_index);

            tile_layout->add<ui::LinkLabel>(
                    service_provider, name, link, service_provider->fonts().get(FontId::Default), Color::white(),
                    Color{ 0xA1, 0x9F, 0x9F }, std::pair<double, double>{ 0.9, 0.8 },
                    ui::Alignment{ ui::AlignmentHorizontal::Right, ui::AlignmentVertical::Center }
            );

            const auto final_path = utils::get_assets_folder() / "authors" / image_name;

            tile_layout->add<ui::ImageView>(
                    service_provider, final_path, std::pair<double, double>{ 1.0, 1.0 }, true,
                    ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
            );
        }
    }

    [[nodiscard]] Scene::UpdateResult AboutPage::update() {
        m_main_grid.update();

        if (m_should_exit) {
            return UpdateResult{ SceneUpdate::StopUpdating, Scene::Pop{} };
        }
        return UpdateResult{ SceneUpdate::StopUpdating, helper::nullopt };
    }

    void AboutPage::render(const ServiceProvider& service_provider) {

        const auto layout = get_layout();
        service_provider.renderer().draw_rect_filled(layout.get_rect(), Color::black());

        m_main_grid.render(service_provider);
    }

    bool AboutPage::handle_event(const SDL_Event& event, const Window* window) {
        if (m_main_grid.handle_event(event, window)) {
            return true;
        }

        if (utils::event_is_action(event, utils::CrossPlatformAction::CLOSE)) {
            m_should_exit = true;
            return true;
        }

        return false;
    }

} // namespace scenes
