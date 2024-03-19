#pragma once

#include "graphics/texture.hpp"
#include "helper/color.hpp"
#include "helper/timer.hpp"
#include "ui/focusable.hpp"
#include "ui/hoverable.hpp"
#include "ui/widget.hpp"

#include <string>
#include <utf8.h>

namespace ui {

    struct TextInput final : public Widget, public Focusable, public Hoverable {
    private:
        std::string m_text{};
        u32 cursor_position{ 0 };
        ServiceProvider* m_service_provider;
        Font m_font;
        Color m_color;
        shapes::URect m_text_rect;
        Texture m_text_texture;
        shapes::URect m_viewport;
        shapes::URect m_cursor_rect;
        u32 scaled_text_size;
        bool cursor_shown{ true };
        helper::Timer timer;

        explicit TextInput(
                ServiceProvider* service_provider,
                Font font,
                const Color& color,
                u32 focus_id,
                const shapes::URect& fill_rect,
                const Layout& layout,
                bool is_top_level
        );

    public:
        explicit TextInput(
                ServiceProvider* service_provider,
                Font font,
                const Color& color,
                u32 focus_id,
                std::pair<double, double> size,
                Alignment alignment,
                const Layout& layout,
                bool is_top_level
        );

        ~TextInput() override;


        void update() override;

        //TODO: how to handle text limits (since texture for texts on the gpu can't get unlimitedly big, maybe use software texture?)
        void render(const ServiceProvider& service_provider) const override;

        helper::BoolWrapper<ui::EventHandleType>
        handle_event(const SDL_Event& event, const Window* window) // NOLINT(readability-function-cognitive-complexity)
                override;

    private:
        void recalculate_textures(bool text_changed);

        bool add_string(const std::string& add);

        bool remove_at_cursor();

        void on_focus() override;

        void on_unfocus() override;
    };
} // namespace ui
