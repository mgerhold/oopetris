#pragma once

#include "graphics/point.hpp"
#include <SDL.h>

namespace shapes {

    //TODO: also add rect structures for unsigned Points, and remove many static_cast<int>()

    struct Rect final {
        Point top_left;
        Point bottom_right;

        constexpr Rect() = default;
        constexpr Rect(Point top_left, Point bottom_right) // NOLINT(bugprone-easily-swappable-parameters)
            : top_left{ top_left },
              bottom_right{ bottom_right } { }
        constexpr Rect(int x, int y, int width, int height)
            : top_left{ x, y },
              bottom_right{ x + width - 1, y + height - 1 } { }

        [[nodiscard]] constexpr auto width() const {
            return bottom_right.x - top_left.x + 1;
        }

        [[nodiscard]] constexpr auto height() const {
            return bottom_right.y - top_left.y + 1;
        }

        [[nodiscard]] SDL_Rect to_sdl_rect() const {
            return { top_left.x, top_left.y, width(), height() };
        }

        [[nodiscard]] constexpr Rect move(Point distance) const {
            return { this->top_left + distance, this->bottom_right + distance };
        }
    };

    inline std::ostream& operator<<(std::ostream& ostream, const Rect& rect) {
        ostream << rect.top_left << ": " << rect.width() << "x" << rect.height();
        return ostream;
    }
} // namespace shapes
