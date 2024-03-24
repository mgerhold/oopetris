#pragma once

#include "helper/bool_wrapper.hpp"
#include "helper/optional.hpp"
#include "manager/service_provider.hpp"
#include "ui/focusable.hpp"
#include "ui/hoverable.hpp"
#include "ui/layout.hpp"

#include <SDL.h>
namespace ui {

    enum class EventHandleType : u8 { RequestFocus, RequestUnFocus, RequestAction };

    enum class WidgetType : u8 { Component, Container };

    struct Widget {
    private:
        Layout m_layout;
        WidgetType m_type;
        bool m_top_level;

    public:
        explicit Widget(const Layout& layout, WidgetType type, bool is_top_level)
            : m_layout{ layout },
              m_type{ type },
              m_top_level{ is_top_level } { }

        Widget(const Widget&) = delete;
        Widget(Widget&&) = default;
        Widget& operator=(const Widget&) = delete;
        Widget& operator=(Widget&&) = default;

        virtual ~Widget() = default;

        [[nodiscard]] const Layout& layout() const {
            return m_layout;
        }

        [[nodiscard]] WidgetType type() const {
            return m_type;
        }

        [[nodiscard]] bool is_top_level() const {
            return m_top_level;
        };


        virtual void update() {
            // do nothing
        }
        virtual void render(const ServiceProvider& service_provider) const = 0;
        [[nodiscard]] virtual helper::BoolWrapper<ui::EventHandleType>
        handle_event(const SDL_Event& event, const Window* window) = 0;
    };

    [[nodiscard]] helper::optional<Focusable*> as_focusable(Widget* widget);

    [[nodiscard]] helper::optional<Hoverable*> as_hoverable(Widget* widget);


} // namespace ui
