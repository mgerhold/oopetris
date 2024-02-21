#pragma once

#include "scenes/scene.hpp"
#include "ui/components/button.hpp"
#include "ui/layouts/grid_layout.hpp"
#include "ui/components/label.hpp"

namespace scenes {

    struct MultiPlayerMenu : public Scene {
    private:
        enum class Command { LocalMultiPlayer, OnlineMultiPlayer, AIMultiPlayer, Return };

        ui::GridLayout<5> m_main_grid;
        helpers::optional<Command> m_next_command;

    public:
        explicit MultiPlayerMenu(ServiceProvider* service_provider, const ui::Layout& layout);

        [[nodiscard]] UpdateResult update() override;
        void render(const ServiceProvider& service_provider) override;
        bool handle_event(const SDL_Event& event, const Window* window) override;
    };

} // namespace scenes
