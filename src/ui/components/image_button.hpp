#pragma once

#include "button.hpp"
#include "graphics/rect.hpp"
#include "image_view.hpp"
#include "manager/resource_manager.hpp"


namespace ui {
    struct ImageButton : public Button<ImageView, ImageButton> {
        explicit ImageButton(
                ServiceProvider* service_provider,
                const std::filesystem::path& image_path,
                bool respect_ratio,
                u32 focus_id,
                Callback callback,
                std::pair<double, double> size,
                Alignment alignment,
                const Layout& layout,
                bool is_top_level
        );

    private:
        [[nodiscard]] bool on_clicked() const override;
    };

} // namespace ui
