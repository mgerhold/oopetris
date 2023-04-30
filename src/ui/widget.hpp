#pragma once

#include "../service_provider.hpp"
#include "layout.hpp"
#include <SDL.h>

namespace ui {
    struct Widget {
        Layout layout;

        explicit Widget(Layout layout) : layout{ layout } { }
        virtual ~Widget() = default;

        virtual void render(const ServiceProvider& service_provider, Rect rect) const = 0;
        [[nodiscard]] virtual bool handle_event(const SDL_Event& event) = 0;
    };
} // namespace ui
