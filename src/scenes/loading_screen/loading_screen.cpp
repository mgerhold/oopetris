#include "loading_screen.hpp"
#include "game/graphic_helpers.hpp"
#include "game/tetromino_type.hpp"
#include "graphics/point.hpp"
#include "graphics/window.hpp"
#include "helper/platform.hpp"

#include <numbers>


scenes::LoadingScreen::LoadingScreen(ServiceProvider* service_provider):
            m_segments{
                { Mino{ Mino::GridPoint{ 0, 0 }, helper::TetrominoType::J }, 1.0 },
                { Mino{ Mino::GridPoint{ 1, 0 }, helper::TetrominoType::L }, 1.0 },
                { Mino{ Mino::GridPoint{ 2, 0 }, helper::TetrominoType::I }, 1.0 },
                { Mino{ Mino::GridPoint{ 2, 1 }, helper::TetrominoType::O }, 1.0 },
                { Mino{ Mino::GridPoint{ 2, 2 }, helper::TetrominoType::S }, 1.0 },
                { Mino{ Mino::GridPoint{ 1, 2 }, helper::TetrominoType::T }, 1.0 },
                { Mino{ Mino::GridPoint{ 0, 2 }, helper::TetrominoType::I }, 1.0 },
                { Mino{ Mino::GridPoint{ 0, 1 }, helper::TetrominoType::Z }, 1.0 },
    },
            m_tile_size{ 100, 100 } {

    const auto [total_x_tiles, total_y_tiles] = utils::get_orientation() == utils::Orientation::Landscape
                                                        ? std::pair<u32, u32>{ 17, 9 }
                                                        : std::pair<u32, u32>{ 9, 17 };

    constexpr auto loading_segments_size = 3;

    const auto layout = service_provider->window().size();

    const u32 tile_size_x = layout.x / total_x_tiles;
    const u32 tile_size_y = layout.y / total_y_tiles;

    const auto tile_size = std::min(tile_size_y, tile_size_x);

    m_tile_size = { tile_size, tile_size };

    const shapes::UPoint grid_start_offset = { (total_x_tiles - loading_segments_size) / 2,
                                               (total_y_tiles - loading_segments_size) / 2 };

    m_start_offset = grid_start_offset * tile_size;
}

namespace {
    [[nodiscard]] double elapsed_time() {
        return static_cast<double>(SDL_GetTicks64()) / 1000.0;
    }
} // namespace


void scenes::LoadingScreen::update() {

    constexpr const auto paramater = 1.0;

    const auto length = static_cast<double>(m_segments.size());

    const auto time = elapsed_time();

    for (size_t i = 0; i < length; ++i) {
        auto& segment = m_segments.at(i);

        auto& scale = std::get<1>(segment);

        const auto offset = std::numbers::pi_v<double> * 2.0 * static_cast<double>(i) / length;

        scale = std::sin(time * paramater + offset);
    }
    //
}

void scenes::LoadingScreen::render(const ServiceProvider& service_provider) const {

    for (const auto& [mino, scale] : m_segments) {
        if (scale >= 0.0) {
            helper::graphics::render_mino(
                    mino, service_provider, MinoTransparency::Solid, scale,
                    [this](const Mino::GridPoint& point) -> auto { return this->to_screen_coords(point); },
                    this->m_tile_size
            );
        }
    }
}


[[nodiscard]] shapes::UPoint scenes::LoadingScreen::to_screen_coords(const Mino::GridPoint& point) const {
    return m_start_offset + point.cast<u32>() * m_tile_size.x;
}
