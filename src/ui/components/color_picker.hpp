#pragma once


#include "graphics/point.hpp"
#include "graphics/rect.hpp"
#include "helper/color.hpp"
#include "ui/components/abstract_slider.hpp"
#include "ui/components/image_button.hpp"
#include "ui/components/textinput.hpp"
#include "ui/widget.hpp"

#include <memory>

namespace detail {

    // it is intended, that this never has focus, than the scroll wheel doesn't work, but it shouldn't work, since scrolling a color slider isn't intended behaviour
    struct ColorSlider : public ui::AbstractSlider<double> {
    private:
        Texture m_texture;

    public:
        explicit ColorSlider(
                ServiceProvider* service_provider,
                Range range,
                Getter getter,
                Setter setter,
                double step,
                const ui::Layout& layout,
                bool is_top_level
        );

        void render(const ServiceProvider& service_provider) const override;

    private:
        [[nodiscard]] std::pair<shapes::URect, shapes::URect> get_rectangles() const override;
    };


    struct ColorCanvas : public ui::Widget, public ui::Hoverable {
    public:
        using Callback = std::function<void(const shapes::AbstractPoint<double>& value)>;

    private:
        ServiceProvider* m_service_provider;
        std::unique_ptr<Texture> m_texture{};
        HSVColor m_current_color;
        Callback m_callback;
        bool m_is_dragging{ false };

    public:
        explicit ColorCanvas(
                ServiceProvider* service_provider,
                const Color& start_color,
                Callback callback,
                const ui::Layout& layout,
                bool is_top_level
        );

        ~ColorCanvas();

        void render(const ServiceProvider& service_provider) const override;

        helper::BoolWrapper<ui::EventHandleType> handle_event(const SDL_Event& event, const Window* window) override;

        void on_change(const Color& color);

        void draw_pseudo_circle(const ServiceProvider& service_provider) const;

    private:
        void redraw_texture();
    };


} // namespace detail


namespace ui {

    enum class ColorMode : u8 { RGB, HSV };

    enum class ColorChangeType : u8 { Both, Hue, SV };


    struct ColorPicker final : public Widget {
    private:
        Color m_color;
        std::unique_ptr<detail::ColorCanvas> m_color_canvas;
        shapes::URect m_color_preview;
        std::unique_ptr<detail::ColorSlider> m_color_slider;
        ColorMode m_mode;
        std::unique_ptr<ui::ImageButton> m_rgb_button;
        std::unique_ptr<ui::ImageButton> m_hsv_button;
        std::unique_ptr<ui::TextInput> m_color_text;

        explicit ColorPicker(
                ServiceProvider* service_provider,
                const Color& start_color,
                const shapes::URect& fill_rect,
                const Layout& layout,
                bool is_top_level
        );

    public:
        explicit ColorPicker(
                ServiceProvider* service_provider,
                const Color& start_color,
                std::pair<double, double> size,
                Alignment alignment,
                const Layout& layout,
                bool is_top_level
        );

        void render(const ServiceProvider& service_provider) const override;

        helper::BoolWrapper<ui::EventHandleType> handle_event(const SDL_Event& event, const Window* window) override;

        [[nodiscard]] Color get_color() const;

    private:
        void after_color_change(ColorChangeType type);
        void after_color_mode_change();
    };
} // namespace ui
