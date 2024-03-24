#pragma once

#include "scenes/scene.hpp"
#include "ui/components/text_button.hpp"
#include "ui/components/label.hpp"
#include "ui/components/slider.hpp"
#include "ui/layouts/grid_layout.hpp"

namespace scenes {

    struct SettingsMenu : public Scene {
    private:
        bool m_should_exit = false;
        ui::GridLayout m_main_grid;

        const std::string listener_name = "settings_menu";

    public:
        explicit SettingsMenu(ServiceProvider* service_provider, const ui::Layout& layout);

        [[nodiscard]] UpdateResult update() override;
        void render(const ServiceProvider& service_provider) override;
        bool handle_event(const SDL_Event& event, const Window* window) override;
        void on_unhover() override;
    };

} // namespace scenes
