#pragma once

#include "button.hpp"
#include "graphics/rect.hpp"
#include "graphics/text.hpp"
#include "manager/resource_manager.hpp"


namespace ui {
    struct TextButton : public Button<Text, TextButton> {
    private:
        explicit TextButton(
                ServiceProvider* service_provider,
                const std::string& text,
                u32 focus_id,
                Callback callback,
                const Font& font,
                const Color& text_color,
                const shapes::URect& fill_rect,
                std::pair<u32, u32> margin,
                const Layout& layout,
                bool is_top_level
        );

    public:
        explicit TextButton(
                ServiceProvider* service_provider,
                const std::string& text,
                const Font& font,
                const Color& text_color,
                u32 focus_id,
                Callback callback,
                std::pair<double, double> size,
                Alignment alignment,
                std::pair<double, double> margin,
                const Layout& layout,
                bool is_top_level
        );

    private:
        [[nodiscard]] bool on_clicked() const override;
    };

} // namespace ui
