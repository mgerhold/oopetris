#pragma once

#include <functional>
#include <spdlog/spdlog.h>
#include <utility>

#include "graphics/rect.hpp"
#include "ui/focusable.hpp"
#include "ui/widget.hpp"

namespace ui {

    struct Slider : public Widget, public Focusable {
    public:
        using Range = std::pair<float, float>;
        using Getter = std::function<float()>;
        using Setter = std::function<void(const float&)>;


    private:
        Range m_range;
        Getter m_getter;
        Setter m_setter;
        float m_step;
        float current_value;
        bool is_dragging{ false };
        shapes::URect fill_rect;


        //TODO: refactor this in member variables, so that recalculations only happen when changing them and not every frame!
        [[nodiscard]] std::pair<shapes::URect, shapes::URect> get_rectangles() const;


    public:
        explicit Slider(
                u32 focus_id,
                Range range,
                Getter getter,
                Setter setter,
                float step,
                std::pair<double, double> size,
                Alignment alignment,
                const Layout& layout,
                bool is_top_level
        );

        ~Slider();


        void render(const ServiceProvider& service_provider) const override;

        helper::BoolWrapper<ui::EventHandleType>
        handle_event(const SDL_Event& event, const Window* window) // NOLINT(readability-function-cognitive-complexity)
                override;

        void on_change();
    };

} // namespace ui
