#pragma once

#include "scenes/scene.hpp"
#include "ui/components/label.hpp"
#include "ui/components/text_button.hpp"
#include "ui/layouts/grid_layout.hpp"

namespace scenes {

    struct PlaySelectMenu : public Scene {
    private:
        enum class Command : u8 { SinglePlayer, MultiPlayer, RecordingSelector, Return };

        ui::GridLayout m_main_grid;
        helper::optional<Command> m_next_command;

    public:
        explicit PlaySelectMenu(ServiceProvider* service_provider, const ui::Layout& layout);

        [[nodiscard]] UpdateResult update() override;
        void render(const ServiceProvider& service_provider) override;
        bool handle_event(const SDL_Event& event, const Window* window) override;
    };

} // namespace scenes
