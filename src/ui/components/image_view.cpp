
#include "image_view.hpp"


ui::ImageView::ImageView(
        ServiceProvider* service_provider,
        const std::filesystem::path& image_path,
        std::pair<double, double> size,
        bool respect_ratio,
        Alignment alignment,
        const Layout& layout,
        bool is_top_level
)
    : Widget{ layout, WidgetType::Component, is_top_level },
      m_image{ service_provider->renderer().load_image(image_path) },
      m_fill_rect{ ui::get_rectangle_aligned(
              layout,
              ui::ratio_helper(
                      { static_cast<u32>(size.first * layout.get_rect().width()),
                        static_cast<u32>(size.second * layout.get_rect().height()) },
                      respect_ratio,
                      m_image.size()
              ),
              alignment
      ) } { }

void ui::ImageView::render(const ServiceProvider& service_provider) const {

    service_provider.renderer().draw_texture(m_image, m_fill_rect);
}

helper::BoolWrapper<ui::EventHandleType> ui::ImageView::handle_event(const SDL_Event&, const Window*) {

    return false;
}
