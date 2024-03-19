
#include "scroll_layout.hpp"

ui::ItemSize::ItemSize(const u32 height, ItemSizeType type) : height{ height }, type{ type } { }


[[nodiscard]] u32 ui::ItemSize::get_height() const {
    return height;
}

[[nodiscard]] ui::ItemSizeType ui::ItemSize::get_type() const {
    return type;
}


ui::AbsolutItemSize::AbsolutItemSize(const u32 height) : ItemSize{ height, ItemSizeType::Absolut } { }


ui::RelativeItemSize::RelativeItemSize(const shapes::URect& rect, const double height)
    : ItemSize{ static_cast<u32>(height * rect.height()), ItemSizeType::Relative } {
    // no checks for upper cases, since it theoretically can also be larger than the whole screen!
    assert(height >= 0.0 && "height has to be in correct percentage range!");
}
ui::RelativeItemSize::RelativeItemSize(const Window* window, const double height)
    : RelativeItemSize{ window->screen_rect(), height } { }
ui::RelativeItemSize::RelativeItemSize(const Window& window, const double height)
    : RelativeItemSize{ window.screen_rect(), height } { }
ui::RelativeItemSize::RelativeItemSize(const Layout& layout, const double height)
    : RelativeItemSize{ layout.get_rect(), height } { }

ui::ScrollLayout::ScrollLayout(
                ServiceProvider* service_provider,
                u32 focus_id,
                Margin gap,
                std::pair<double, double> margin,
                const Layout& layout,
                bool is_top_level 
        )
            : FocusLayout{
                  layout, focus_id, FocusOptions{ is_top_level, is_top_level }, is_top_level}, // if on top, we support tab and wrap around, otherwise not
                  gap{ gap },
                  m_texture{ helper::nullopt },
                  m_service_provider{ service_provider },
                  m_step_size{ static_cast<u32>(layout.get_rect().height() * 0.05) } {

    const auto layout_rect = layout.get_rect();
    const auto absolut_margin = std::pair<u32, u32>{ static_cast<u32>(margin.first * layout_rect.width()),
                                                     static_cast<u32>(margin.second * layout_rect.height()) };

    constexpr u32 absolut_gap = 10;

    const auto scroll_bar_width = static_cast<u32>(0.02 * layout_rect.width());

    const u32 start_x = layout_rect.top_left.x + absolut_margin.first;
    const u32 start_y = layout_rect.top_left.y + absolut_margin.second;

    const u32 new_width = layout_rect.width() - absolut_margin.first * 2;
    const u32 new_height = layout_rect.height() - absolut_margin.second * 2;


    main_rect = shapes::URect{ start_x, start_y, new_width - scroll_bar_width - absolut_gap, new_height };
    scrollbar_rect = shapes::URect{ start_x + new_width - scroll_bar_width, start_y, scroll_bar_width, new_height };
    scrollbar_mover_rect = scrollbar_rect; // NOLINT(cppcoreguidelines-prefer-member-initializer)
    m_viewport = shapes::URect{ 0, 0, 0, 0 };
}

void ui::ScrollLayout::render(const ServiceProvider& service_provider) const {

    const auto& renderer = service_provider.renderer();

    const auto total_widgets_height = m_widgets.empty() ? 0 : m_widgets.back()->layout().get_rect().bottom_right.y;

    if (m_texture.has_value()) {

        renderer.set_render_target(m_texture.value());
        renderer.clear();
        for (const auto& widget : m_widgets) {
            // smart rendering, only render, when viewport needs this widget
            const auto layout_rect = widget->layout().get_rect();
            if (layout_rect.top_left.y > m_viewport.bottom_right.y) {
                continue;
            }

            if (layout_rect.bottom_right.y < m_viewport.top_left.y) {
                continue;
            }

            widget->render(service_provider);
        }

        renderer.reset_render_target();

        auto to_rect = main_rect;
        // if we don't need to fill-up the whole main_rect, we need a special to_rect
        if (total_widgets_height < scrollbar_rect.height()) {
            to_rect = shapes::URect{ main_rect.top_left.x, main_rect.top_left.y, main_rect.width(),
                                     total_widgets_height };
        }

        renderer.draw_texture(m_texture.value(), m_viewport, to_rect);
    }

    // render the scrollbar when it makes sense
    if (total_widgets_height > scrollbar_rect.height()) {
        renderer.draw_rect_filled(scrollbar_rect, "#A19797"_rgb);
        renderer.draw_rect_filled(scrollbar_mover_rect, is_dragging ? "#666161"_rgb : "#524F4F"_rgb);
    }
}

helper::BoolWrapper<ui::EventHandleType>
ui::ScrollLayout::handle_event( // NOLINT(readability-function-cognitive-complexity)
        const SDL_Event& event,
        const Window* window
) {

    helper::BoolWrapper<ui::EventHandleType> handled = handle_focus_change_events(event, window);

    if (handled) {
        auto_move_after_focus_change();
        return handled;
    }

    if (utils::device_supports_clicks()) {

        const u32 total_widgets_height = m_widgets.empty() ? 0 : m_widgets.back()->layout().get_rect().bottom_right.y;

        const auto change_value_on_scroll = [&window, &event, total_widgets_height, this]() {
            const auto& [_, y] = utils::get_raw_coordinates(window, event);

            auto desired_scroll_height = 0;


            if (y <= static_cast<i32>(scrollbar_rect.top_left.y)) {
                desired_scroll_height = 0;
            } else if (y >= static_cast<i32>(scrollbar_rect.bottom_right.y)) {
                // this is to high, but recalculate_sizes reset it to the highest possible value!
                desired_scroll_height = static_cast<int>(total_widgets_height);
            } else {

                const double percentage = static_cast<double>(y - scrollbar_rect.top_left.y)
                                          / static_cast<double>(scrollbar_rect.height());

                // we want the final point to be in the middle, but desired_scroll_height expects the top position.
                desired_scroll_height = static_cast<int>(
                        static_cast<int>(percentage * total_widgets_height) - scrollbar_rect.height() / 2
                );
                is_dragging = true;
            }


            recalculate_sizes(desired_scroll_height);
        };


        if (utils::event_is_click_event(event, utils::CrossPlatformClickEvent::ButtonDown)) {
            // note: this behaviour is intentional, namely, clicking into the scroll slider doesn't move it, it just "grabs" it for dragging
            if (utils::is_event_in(window, event, scrollbar_mover_rect)) {
                is_dragging = true;
                handled = true;
            } else if (utils::is_event_in(window, event, scrollbar_rect)) {

                change_value_on_scroll();
                handled = true;
            }

        } else if (utils::event_is_click_event(event, utils::CrossPlatformClickEvent::ButtonUp)) {
            is_dragging = false;
            handled = true;

        } else if (utils::event_is_click_event(event, utils::CrossPlatformClickEvent::Motion)) {

            if (is_dragging) {

                change_value_on_scroll();
                handled = true;
            }
        } else if (event.type == SDL_MOUSEWHEEL) {

            // attention the mouse direction changes (it's called natural scrolling on macos/ windows / linux) are not detected by sdl until restart, and here we use the correct scroll behaviour, as the user configured the mouse in it's OS
            const bool direction_is_down =
                    event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? event.wheel.y < 0 : event.wheel.y > 0;


            auto desired_scroll_height = 0;

            if (direction_is_down) {
                desired_scroll_height = static_cast<int>(m_viewport.top_left.y + m_step_size);
            } else {
                desired_scroll_height = static_cast<int>(m_viewport.top_left.y - m_step_size);
            }

            recalculate_sizes(desired_scroll_height);
            handled = true;
        }

        if (utils::event_is_click_event(event, utils::CrossPlatformClickEvent::Any)) {

            const auto offset_distance = main_rect.top_left.cast<i32>() - m_viewport.top_left.cast<i32>();
            for (auto& widget : m_widgets) {
                const auto& layout_rect = widget->layout().get_rect();
                const auto& offset_rect = (layout_rect.cast<i32>()) >> offset_distance;

                if (not handled and utils::is_event_in(window, event, main_rect)
                    and utils::is_event_in(window, event, offset_rect.cast<u32>())) {
                    const auto offset_event = utils::offset_event(window, event, -offset_distance);
                    if (const auto event_result = widget->handle_event(offset_event, window); event_result) {
                        handled = { true, handle_event_result(event_result.get_additional(), widget.get()) };
                        continue;
                    }
                } else {
                    const auto hoverable = as_hoverable(widget.get());
                    if (hoverable.has_value()) {
                        hoverable.value()->on_unhover();
                    }
                }
            }

            return handled;
        }
    }

    return handled;
}

void ui::ScrollLayout::clear_widgets() {

    m_widgets.clear();
    m_texture = helper::nullopt;
    m_focus_id = helper::nullopt;

    recalculate_sizes(0);
}

[[nodiscard]] ui::Layout ui::ScrollLayout::get_layout_for_index(u32) {
    // see TODO comment over handle_event in header file
    throw std::runtime_error("NOT SUPPORTED");
}

[[nodiscard]] ui::Layout ui::ScrollLayout::get_layout_for_new(ItemSize size) {
    u32 start_point_y = 0;

    for (const auto& widget : m_widgets) {
        const auto& widget_rect = widget->layout().get_rect();
        start_point_y += static_cast<u32>(widget_rect.height());
    }

    start_point_y += gap.get_margin() * static_cast<u32>(m_widgets.size());

    const auto width = static_cast<u32>(main_rect.width());
    const auto height = size.get_height();

    return AbsolutLayout{
        0,
        start_point_y,
        width,
        height,
    };
}

void ui::ScrollLayout::auto_move_after_focus_change() {

    if (not m_focus_id.has_value()) {
        return;
    }

    const auto total_widgets_height = m_widgets.empty() ? 0 : m_widgets.back()->layout().get_rect().bottom_right.y;

    // if we don't need to fill-up the whole main_rect, we need a special viewport, but top position is always 0
    if (total_widgets_height < scrollbar_rect.height()) {
        recalculate_sizes(0);
        return;
    }

    // we center the in focus element (if possible -> not on top or bottom)

    const auto& widget = m_widgets.at(focusable_index_by_id(m_focus_id.value()));

    const auto widget_rect = widget->layout().get_rect();

    // determine if the middle is +- (1 % px) in the viewport middle
    const auto middle_of_rect_y = widget_rect.top_left.y + (widget_rect.height() / 2);


    const auto viewport_middle_y = m_viewport.top_left.y + (m_viewport.height() / 2);

    const auto is_circa_in_middle = std::abs(static_cast<i32>(middle_of_rect_y) - static_cast<i32>(viewport_middle_y))
                                    <= static_cast<int>(m_service_provider->window().screen_rect().height() * 0.05);

    if (is_circa_in_middle) {
        return;
    }

    recalculate_sizes(static_cast<i32>(middle_of_rect_y - (m_viewport.height() / 2)));
}

// it's called desired, since it might not be entirely valid
void ui::ScrollLayout::recalculate_sizes(i32 desired_scroll_height) {

    const auto total_widgets_height = m_widgets.empty() ? 0 : m_widgets.back()->layout().get_rect().bottom_right.y;

    // if we don't need to fill-up the whole main_rect, we need a special viewport
    if (total_widgets_height < scrollbar_rect.height()) {
        m_viewport = shapes::URect{ 0, 0, main_rect.width(), total_widgets_height };
    } else {
        // check if desired_scroll_height is valid:
        auto scroll_height = desired_scroll_height;

        if (desired_scroll_height < 0) {
            scroll_height = 0;
        } else if (desired_scroll_height + main_rect.height() > total_widgets_height) {
            scroll_height = static_cast<i32>(total_widgets_height - main_rect.height());
        }

        m_viewport = shapes::URect{ 0, static_cast<u32>(scroll_height), main_rect.width(), main_rect.height() };
    }


    // recalculate scrollbar mover rect
    const auto current_start_height = static_cast<u32>(
            scrollbar_rect.height()
            * (static_cast<double>(m_viewport.top_left.y) / static_cast<double>(total_widgets_height))
    );

    const auto current_end_height = static_cast<u32>(
            scrollbar_rect.height()
            * (static_cast<double>(m_viewport.top_left.y + scrollbar_rect.height())
               / static_cast<double>(total_widgets_height))
    );

    scrollbar_mover_rect = shapes::URect{ scrollbar_rect.top_left.x, scrollbar_rect.top_left.y + current_start_height,
                                          scrollbar_rect.width(), current_end_height - current_start_height };
}
