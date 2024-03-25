
#include "text_button.hpp"


ui::TextButton::TextButton(
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
)
    : Button<Text, TextButton>{
          Text{service_provider,
               text, font,
               text_color, { fill_rect.top_left.x + static_cast<int>(margin.first),
 fill_rect.top_left.y + static_cast<int>(margin.second),
 fill_rect.width() - 2 * static_cast<int>(margin.first),
 fill_rect.height() - 2 * static_cast<int>(margin.second) }},
          focus_id,
          std::move(callback),
          fill_rect,
          layout,
          is_top_level
} { }


ui::TextButton::TextButton(
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
)
    : TextButton{
          service_provider,
          text,
          focus_id,
          std::move(callback),
          font,
          text_color,
          ui::get_rectangle_aligned(
                  layout,
                  {               static_cast<u32>(size.first * layout.get_rect().width()),
                    static_cast<u32>(size.second * layout.get_rect().height())                },
                  alignment
          ),
          {static_cast<u32>(margin.first * size.first * layout.get_rect().width()),
                    static_cast<u32>(margin.second * size.second * layout.get_rect().height())},
          layout,
          is_top_level
} { }

[[nodiscard]] bool ui::TextButton::on_clicked() const {
    return m_callback(*this);
}
