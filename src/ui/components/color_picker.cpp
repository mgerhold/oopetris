

#include "color_picker.hpp"
#include "graphics/point.hpp"
#include "graphics/rect.hpp"
#include "manager/resource_manager.hpp"
#include "ui/components/textinput.hpp"
#include "ui/hoverable.hpp"
#include "ui/layout.hpp"
#include "ui/widget.hpp"

detail::ColorSlider::ColorSlider(
        ServiceProvider* service_provider,
        Range range,
        Getter getter,
        Setter setter,
        double step,
        const ui::Layout& layout,
        bool is_top_level
)
    : AbstractSlider<double>{ ui::FocusHelper::FocusIDUnused(),
                              std::move(range),
                              std::move(getter),
                              std::move(setter),
                              step,
                              layout,
                              is_top_level },
      m_texture{ service_provider->renderer().get_texture_for_render_target(bar_rect().to_dimension_point()) } {

    service_provider->renderer().set_render_target(m_texture);

    const auto w = bar_rect().width();
    const auto h = bar_rect().height();

    for (u32 x = 0; x < w; x++) {
        Color color{
            HSVColor{(static_cast<double>(x) / static_cast<double>(w)) * 360.0, 1.0, 1.0}
        };

        service_provider->renderer().draw_line(
                shapes::UPoint{ x, 0 }, shapes::UPoint{ x, static_cast<u32>(h - 1) }, color
        );
    }

    service_provider->renderer().reset_render_target();
}

[[nodiscard]] std::pair<shapes::URect, shapes::URect> detail::ColorSlider::get_rectangles() const {

    const auto layout_rect = layout().get_rect();

    const auto origin = layout_rect.top_left;

    const double percentage = (current_value() - range().first) / (range().second - range().first);

    const u32 position_x_middle =
            origin.x + static_cast<u32>(percentage * static_cast<double>(layout_rect.bottom_right.x - origin.x));

    constexpr const u32 slider_half_size = 2;

    u32 slider_start_x = 0;
    u32 slider_end_x = 0;

    if (position_x_middle <= origin.x) {
        slider_start_x = origin.x;
        slider_end_x = slider_start_x + 2 * slider_half_size;
    } else if (position_x_middle >= layout_rect.bottom_right.x) {
        slider_end_x = layout_rect.bottom_right.x;
        slider_start_x = slider_end_x - 2 * slider_half_size;
    } else {
        slider_start_x = position_x_middle - slider_half_size;
        slider_end_x = position_x_middle + slider_half_size;
    }

    const auto slider_rect = shapes::URect{
        shapes::UPoint{slider_start_x,     layout_rect.top_left.y},
        shapes::UPoint{  slider_end_x, layout_rect.bottom_right.y}
    };

    return { layout_rect, slider_rect };
}

void detail::ColorSlider::render(const ServiceProvider& service_provider) const {

    const auto& renderer = service_provider.renderer();

    renderer.draw_texture(m_texture, bar_rect());

    renderer.draw_rect_filled(slider_rect(), Color::white(0xAA));
}

detail::ColorCanvas::ColorCanvas(const ui::Layout& layout, bool is_top_level)
    : ui::Widget{ layout, ui::WidgetType::Component, is_top_level },
      ui::Hoverable{ layout.get_rect() } { }


void detail::ColorCanvas::render(const ServiceProvider& service_provider) const {
    //TODO
    UNUSED(service_provider);
}

helper::BoolWrapper<ui::EventHandleType>
detail::ColorCanvas::handle_event(const SDL_Event& event, const Window* window) {

    //TODO
    UNUSED(event);
    UNUSED(window);
    return false;
}

ui::ColorPicker::ColorPicker(
        ServiceProvider* service_provider,
        const Color& start_color,
        const shapes::URect& fill_rect,
        const Layout& layout,
        bool is_top_level
)
    : Widget{ layout, WidgetType::Component, is_top_level },
      m_color{ start_color },
      m_mode{ ColorMode::RGB } {

    constexpr double main_rect_height = 0.8;

    const auto main_fill_layout = ui::RelativeLayout{ fill_rect, 0.0, 0.0, 1.0, main_rect_height };

    const auto main_rect = ui::get_rectangle_aligned(
            main_fill_layout, ui::ratio_helper(main_fill_layout.get_rect().to_dimension_pair(), true, { 512, 512 }),
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Bottom }
    );

    m_color_canvas = std::make_unique<detail::ColorCanvas>(ui::Layout{ main_rect }, false);

    auto rest_fill_rect = ui::RelativeLayout(fill_rect, 0.0, main_rect_height, 1.0, 1.0 - main_rect_height).get_rect();

    // align it to the horizontal size of the main_rect
    rest_fill_rect.top_left.x = main_rect.top_left.x;
    rest_fill_rect.bottom_right.x = main_rect.bottom_right.x;

    constexpr double preview_rect_size = 0.2;

    const auto preview_fill_layout =
            ui::RelativeLayout{ rest_fill_rect, 1.0 - preview_rect_size, 0.0, preview_rect_size, 1.0 };

    m_color_preview = ui::get_rectangle_aligned(
            preview_fill_layout,
            ui::ratio_helper(preview_fill_layout.get_rect().to_dimension_pair(), true, { 512, 512 }),
            ui::Alignment{ ui::AlignmentHorizontal::Left, ui::AlignmentVertical::Top }
    );


    const auto bars_fill_rect = ui::RelativeLayout(rest_fill_rect, 0.0, 0.0, 1.0 - preview_rect_size, 1.0).get_rect();

    constexpr double color_bar_size = 0.5;

    const auto color_bar_fill_layout = ui::RelativeLayout{ bars_fill_rect, 0.0, 0.0, 1.0, color_bar_size };

    const auto color_bar_rect = ui::get_rectangle_aligned(
            color_bar_fill_layout, color_bar_fill_layout.get_rect().to_dimension_pair(),
            ui::Alignment{ ui::AlignmentHorizontal::Right, ui::AlignmentVertical::Top }
    );

    m_color_slider = std::make_unique<detail::ColorSlider>(
            service_provider, std::pair<double, double>{ 0.0, 360.0 },
            [this]() -> double { return this->m_color.to_hsv_color().h; },
            [this](const double& value) {
                auto hsv_color = m_color.to_hsv_color();
                hsv_color.h = value;
                this->m_color = Color{ hsv_color };
                this->after_color_change(ColorChangeType::Hue);
            },
            5.0, ui::Layout{ color_bar_rect }, false
    );

    const auto components_fill_layout =
            ui::RelativeLayout{ bars_fill_rect, 0.0, color_bar_size, 1.0, 1.0 - color_bar_size };

    constexpr double toggle_button_size = 0.2;

    const auto toggle_button_layout = ui::RelativeLayout{ components_fill_layout, 0.0, 0.0, toggle_button_size, 1.0 };


    const auto focus_id_unused = FocusHelper::FocusIDUnused();

    const auto rgb_image_path = utils::get_assets_folder() / "icons" / "rgb_color_selector.png";


    m_rgb_button = std::make_unique<ui::ImageButton>(
            service_provider, rgb_image_path, true, focus_id_unused,
            [this](const ImageButton&) -> bool {
                this->m_mode = ColorMode::HSV;
                this->after_color_mode_change();
                return false;
            },
            std::pair<double, double>{ 1.0, 1.0 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }, toggle_button_layout, false
    );

    const auto hsv_image_path = utils::get_assets_folder() / "icons" / "hsv_color_selector.png";

    m_hsv_button = std::make_unique<ui::ImageButton>(
            service_provider, hsv_image_path, true, focus_id_unused,
            [this](const ImageButton&) -> bool {
                this->m_mode = ColorMode::RGB;
                this->after_color_mode_change();
                return false;
            },
            std::pair<double, double>{ 1.0, 1.0 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }, toggle_button_layout, false
    );

    const auto textinput_layout =
            ui::RelativeLayout{ components_fill_layout, toggle_button_size, 0.0, 1.0 - toggle_button_size, 1.0 };


    m_color_text = std::make_unique<ui::TextInput>(
            service_provider, service_provider->fonts().get(FontId::Default), Color::white(), focus_id_unused,
            std::pair<double, double>{ 0.9, 0.9 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }, textinput_layout, false
    );

    after_color_change(ui::ColorChangeType::Both);
}

ui::ColorPicker::ColorPicker(
        ServiceProvider* service_provider,
        const Color& start_color,
        std::pair<double, double> size,
        Alignment alignment,
        const Layout& layout,
        bool is_top_level
)
    : ColorPicker{ service_provider, start_color,
                   ui::get_rectangle_aligned(
                           layout,
                           { static_cast<u32>(size.first * layout.get_rect().width()),
                             static_cast<u32>(size.second * layout.get_rect().height()) },
                           alignment
                   ),
                   layout, is_top_level } { }

void ui::ColorPicker::render(const ServiceProvider& service_provider) const {
    //TODO
    UNUSED(service_provider);
}

helper::BoolWrapper<ui::EventHandleType> ui::ColorPicker::handle_event(const SDL_Event& event, const Window* window) {
    //TODO
    UNUSED(event);
    UNUSED(window);
    return false;
}

void ui::ColorPicker::after_color_change(ColorChangeType type) {
    UNUSED(type);
    //TODO
}

void ui::ColorPicker::after_color_mode_change() {
    //TODO
}
