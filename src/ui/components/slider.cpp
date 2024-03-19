
#include "slider.hpp"
#include "graphics/renderer.hpp"
#include "helper/color.hpp"
#include "platform/capabilities.hpp"

[[nodiscard]] std::pair<shapes::URect, shapes::URect> ui::Slider::get_rectangles() const {

    const auto origin = fill_rect.top_left;

    const auto bar = RelativeLayout{ fill_rect, 0, 0.4, 1.0, 0.2 };

    const float percentage = (current_value - m_range.first) / (m_range.second - m_range.first);

    const u32 position_x_middle =
            origin.x + static_cast<u32>(percentage * static_cast<double>(fill_rect.bottom_right.x - origin.x));

    const auto slider_rect = shapes::URect{
        shapes::UPoint{position_x_middle - 5,     fill_rect.top_left.y},
        shapes::UPoint{position_x_middle + 5, fill_rect.bottom_right.y}
    };

    return { bar.get_rect(), slider_rect };
}


ui::Slider::Slider(
        u32 focus_id,
        Range range,
        Getter getter,
        Setter setter,
        float step,
        std::pair<double, double> size,
        Alignment alignment,
        const Layout& layout,
        bool is_top_level
)
    : Widget{ layout, WidgetType::Component, is_top_level },
      Focusable{ focus_id },
      m_range{ std::move(range) },
      m_getter{ std::move(getter) },
      m_setter{ std::move(setter) },
      m_step{ step },
      fill_rect{ ui::get_rectangle_aligned(
              layout,
              { static_cast<u32>(size.first * layout.get_rect().width()),
                static_cast<u32>(size.second * layout.get_rect().height()) },
              alignment
      ) } {
    assert(m_range.first <= m_range.second && "Range has to be in correct order!");
    current_value = m_getter();

    // if on top. we give us focus automatically
    if (is_top_level) {
        focus();
    }
}


ui::Slider::~Slider() {
    SDL_CaptureMouse(SDL_FALSE);
}

void ui::Slider::render(const ServiceProvider& service_provider) const {
    const auto color = (has_focus() ? Color::red() : Color::blue());

    const auto& [bar_rect, slider_rect] = get_rectangles();

    service_provider.renderer().draw_rect_filled(bar_rect, color);

    //orange or cyan
    const auto slider_color = (is_dragging ? "#FFCC00"_rgb : has_focus() ? "#ff6f00"_rgb : "#00ccff"_rgb);
    service_provider.renderer().draw_rect_filled(slider_rect, slider_color);
}

helper::BoolWrapper<ui::EventHandleType> ui::Slider::handle_event( // NOLINT(readability-function-cognitive-complexity)
        const SDL_Event& event,
        const Window* window
) {

    helper::BoolWrapper<ui::EventHandleType> handled = false;

    if (utils::device_supports_keys() and has_focus()) {
        if (utils::event_is_action(event, utils::CrossPlatformAction::RIGHT)) {
            current_value = current_value + m_step;
            if (current_value >= m_range.second) {
                current_value = m_range.second;
            }

            handled = true;
        } else if (utils::event_is_action(event, utils::CrossPlatformAction::LEFT)) {
            current_value = current_value - m_step;
            if (current_value <= m_range.first) {
                current_value = m_range.first;
            }

            handled = true;
        }
    }

    if (not handled and utils::device_supports_clicks()) {

        const auto change_value_on_scroll = [&window, &event, this]() {
            const auto& [bar_rect, slider_rect] = get_rectangles();

            const auto& [x, _] = utils::get_raw_coordinates(window, event);

            if (x <= static_cast<i32>(bar_rect.top_left.x)) {
                current_value = m_range.first;
            } else if (x >= static_cast<i32>(bar_rect.bottom_right.x)) {
                current_value = m_range.second;
            } else {

                const float percentage =
                        static_cast<float>(x - bar_rect.top_left.x) / static_cast<float>(bar_rect.width());
                current_value = percentage * (m_range.second - m_range.first) + m_range.first;
                is_dragging = true;
                SDL_CaptureMouse(SDL_TRUE);
            }
        };


        if (utils::event_is_click_event(event, utils::CrossPlatformClickEvent::ButtonDown)) {

            const auto& [bar_rect, slider_rect] = get_rectangles();

            if (utils::is_event_in(window, event, bar_rect)) {

                change_value_on_scroll();
                handled = { true, ui::EventHandleType::RequestFocus };

            } else if (utils::is_event_in(window, event, slider_rect)) {
                is_dragging = true;
                SDL_CaptureMouse(SDL_TRUE);
                handled = { true, ui::EventHandleType::RequestFocus };
            }

        } else if (utils::event_is_click_event(event, utils::CrossPlatformClickEvent::ButtonUp)) {
            is_dragging = false;
            SDL_CaptureMouse(SDL_FALSE);
            handled = true;

        } else if (utils::event_is_click_event(event, utils::CrossPlatformClickEvent::Motion)) {

            if (is_dragging) {
                change_value_on_scroll();
                handled = true;
            }

        } else if (event.type == SDL_MOUSEWHEEL) {

            // here we use a reverse scroll behaviour, since moving the mouse up is always considered increasing the volume, regardless of you OS setting about natural scrolling or not
            const bool direction_is_up =
                    event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? event.wheel.y > 0 : event.wheel.y < 0;

            if (direction_is_up) {
                current_value = current_value + m_step;
                if (current_value >= m_range.second) {
                    current_value = m_range.second;
                }

            } else {
                current_value = current_value - m_step;
                if (current_value <= m_range.first) {
                    current_value = m_range.first;
                }
            }

            handled = true;
        }
    }


    if (handled) {
        m_setter(current_value);
        return handled;
    }

    return false;
}

void ui::Slider::on_change() {
    current_value = m_getter();
    m_setter(current_value);
}
