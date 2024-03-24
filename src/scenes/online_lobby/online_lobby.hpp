#pragma once

#include "lobby/api.hpp"
#include "scenes/scene.hpp"
#include "ui/components/text_button.hpp"
#include "ui/components/label.hpp"
#include "ui/layouts/tile_layout.hpp"

#include <memory>

namespace scenes {

    struct OnlineLobby : public Scene {
    private:
        enum class Command : u8 { Play, Return };

        ui::TileLayout m_main_layout;
        helper::optional<Command> m_next_command;
        std::unique_ptr<lobby::Client> client{ nullptr };

    public:
        explicit OnlineLobby(ServiceProvider* service_provider, const ui::Layout& layout);

        [[nodiscard]] UpdateResult update() override;
        void render(const ServiceProvider& service_provider) override;
        bool handle_event(const SDL_Event& event, const Window* window) override;
    };

} // namespace scenes
