
#include "slider.hpp"
#include "graphics/renderer.hpp"
#include "helper/color.hpp"
#include "helper/color_literals.hpp"

ui::Slider::Slider(
        u32 focus_id,
        Range range,
        Getter getter,
        Setter setter,
        double step,
        std::pair<double, double> size,
        Alignment alignment,
        const Layout& layout,
        bool is_top_level
)
    : AbstractSlider<double>{ focus_id, std::move(range), std::move(getter), std::move(setter),
                              step,     layout,           is_top_level },
      m_fill_rect{ ui::get_rectangle_aligned(
              layout,
              { static_cast<u32>(size.first * layout.get_rect().width()),
                static_cast<u32>(size.second * layout.get_rect().height()) },
              alignment
      ) } { }


[[nodiscard]] std::pair<shapes::URect, shapes::URect> ui::Slider::get_rectangles() const {

    const auto origin = m_fill_rect.top_left;

    const auto bar = RelativeLayout{ m_fill_rect, 0, 0.4, 1.0, 0.2 };

    const double percentage = (current_value() - range().first) / (range().second - range().first);

    const u32 position_x_middle =
            origin.x + static_cast<u32>(percentage * static_cast<double>(m_fill_rect.bottom_right.x - origin.x));

    constexpr const u32 slider_half_size = 5;

    u32 slider_start_x = 0;
    u32 slider_end_x = 0;

    if (position_x_middle <= origin.x) {
        slider_start_x = origin.x;
        slider_end_x = slider_start_x + 2 * slider_half_size;
    } else if (position_x_middle >= m_fill_rect.bottom_right.x) {
        slider_end_x = m_fill_rect.bottom_right.x;
        slider_start_x = slider_end_x - 2 * slider_half_size;
    } else {
        slider_start_x = position_x_middle - slider_half_size;
        slider_end_x = position_x_middle + slider_half_size;
    }


    const auto slider_rect = shapes::URect{
        shapes::UPoint{slider_start_x,     m_fill_rect.top_left.y},
        shapes::UPoint{  slider_end_x, m_fill_rect.bottom_right.y}
    };

    return { bar.get_rect(), slider_rect };
}

void ui::Slider::render(const ServiceProvider& service_provider) const {
    const auto color = (has_focus() ? Color::red() : Color::blue());


    service_provider.renderer().draw_rect_filled(bar_rect(), color);

    //orange or cyan
    const auto slider_color = (is_dragging() ? "#FFCC00"_c : has_focus() ? "#ff6f00"_c : "#00ccff"_c);
    service_provider.renderer().draw_rect_filled(slider_rect(), slider_color);
}
