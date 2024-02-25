#pragma once

#include "graphics/rect.hpp"
#include "helper/bool_wrapper.hpp"
#include "helper/optional.hpp"
#include "manager/service_provider.hpp"
#include "ui/focusable.hpp"
#include "ui/hoverable.hpp"
#include "ui/layout.hpp"

#include <SDL.h>
namespace ui {

    enum class EventHandleType { RequestFocus, RequestUnFocus };

    struct Widget {
    private:
        Layout m_layout;

    public:
        explicit Widget(const Layout& layout) : m_layout{ layout } { }

        Widget(const Widget&) = delete;
        Widget(Widget&&) = delete;
        Widget& operator=(const Widget&) = delete;
        Widget& operator=(Widget&&) = delete;

        virtual ~Widget() = default;

        [[nodiscard]] const Layout& layout() const {
            return m_layout;
        }

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
