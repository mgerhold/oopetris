#pragma once

#include "graphics/renderer.hpp"
#include "graphics/text.hpp"
#include "ui/hoverable.hpp"
#include "ui/widget.hpp"

#include <string>
#include <filesystem>

namespace ui {
    struct ImageView final : public Widget {
    private:
        Texture m_image;
        shapes::URect m_fill_rect;

    public:
        explicit ImageView(
                ServiceProvider* service_provider,
                const std::filesystem::path& image_path,
                std::pair<double, double> size,
                bool respect_ratio,
                Alignment alignment,
                const Layout& layout,
                bool is_top_level
        );

        void render(const ServiceProvider& service_provider) const override;

        helper::BoolWrapper<ui::EventHandleType> handle_event(const SDL_Event&, const Window*) override;
    };
} // namespace ui
