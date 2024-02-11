#pragma once

#include "scenes/scene.hpp"
#include "ui/button.hpp"
#include "ui/grid_layout.hpp"
#include "ui/label.hpp"

namespace scenes {

    struct MainMenu : public Scene {
    private:
        enum class Command {
            StartGame,
            OpenSettingsMenu,
            Exit,
        };

        ui::GridLayout<4> m_main_grid;
        tl::optional<Command> m_next_command;

    public:
        explicit MainMenu(ServiceProvider* service_provider, const ui::Layout& layout);

        [[nodiscard]] UpdateResult update() override;
        void render(const ServiceProvider& service_provider) override;
        bool handle_event(const SDL_Event& event, const Window* window) override;
    };

} // namespace scenes
