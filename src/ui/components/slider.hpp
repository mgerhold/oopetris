#pragma once

#include <functional>
#include <spdlog/spdlog.h>
#include <utility>

#include "abstract_slider.hpp"

namespace ui {

    struct Slider : public AbstractSlider<double> {
    private:
        shapes::URect m_fill_rect;

    public:
        explicit Slider(
                u32 focus_id,
                Range range,
                Getter getter,
                Setter setter,
                double step,
                std::pair<double, double> size,
                Alignment alignment,
                const Layout& layout,
                bool is_top_level
        );

        void render(const ServiceProvider& service_provider) const override;

    private:
        [[nodiscard]] std::pair<shapes::URect, shapes::URect> get_rectangles() const override;
    };

} // namespace ui
