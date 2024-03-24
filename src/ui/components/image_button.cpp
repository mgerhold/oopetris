
#include "image_button.hpp"


ui::ImageButton::ImageButton(
        ServiceProvider* service_provider,
        const std::filesystem::path& image_path,
        bool respect_ratio,
        u32 focus_id,
        Callback callback,
        std::pair<double, double> size,
        Alignment alignment,
        const Layout& layout,
        bool is_top_level
)
    : Button<ImageView, ImageButton>{
          ImageView{service_provider, image_path, size, respect_ratio, alignment, layout, false},
          focus_id,
          std::move(callback),
          size,
          alignment,
          layout,
          is_top_level
} { }

[[nodiscard]] bool ui::ImageButton::on_clicked() const {
    return m_callback(*this);
}
