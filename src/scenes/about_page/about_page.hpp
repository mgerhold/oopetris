#pragma once

#include "scenes/scene.hpp"
#include "ui/components/text_button.hpp"
#include "ui/components/label.hpp"
#include "ui/components/slider.hpp"
#include "ui/layouts/grid_layout.hpp"

namespace scenes {

    struct AboutPage : public Scene {
    private:
        bool m_should_exit = false;
        ui::GridLayout m_main_grid;


        static constexpr std::initializer_list<std::tuple<const char* const, const char* const, const char* const>>
                authors{
                    {"mgerhold", "https://github.com/mgerhold", "mgerhold.jpg"},
                    { "Totto16",  "https://github.com/Totto16",  "Totto16.png"}
        };


    public:
        explicit AboutPage(ServiceProvider* service_provider, const ui::Layout& layout);

        [[nodiscard]] UpdateResult update() override;
        void render(const ServiceProvider& service_provider) override;
        bool handle_event(const SDL_Event& event, const Window* window) override;
    };

} // namespace scenes
