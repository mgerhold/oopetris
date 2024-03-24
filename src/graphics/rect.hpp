#pragma once

#include "graphics/point.hpp"

#include <SDL.h>
#include <utility>

namespace shapes {

    template<typename T>
    struct AbstractRect final {
        using Point = AbstractPoint<T>;
        Point top_left;
        Point bottom_right;

        constexpr AbstractRect() = default;
        constexpr AbstractRect(Point top_left, Point bottom_right) // NOLINT(bugprone-easily-swappable-parameters)
            : top_left{ top_left },
              bottom_right{ bottom_right } { }
        constexpr AbstractRect(T x, T y, T width, T height)
            : top_left{ x, y },
              bottom_right{ x + width - 1, y + height - 1 } { }

        [[nodiscard]] constexpr T width() const {
            return static_cast<T>(bottom_right.x - top_left.x + 1);
        }

        [[nodiscard]] constexpr T height() const {
            return static_cast<T>(bottom_right.y - top_left.y + 1);
        }

        [[nodiscard]] SDL_Rect to_sdl_rect() const {
            return { static_cast<int>(top_left.x), static_cast<int>(top_left.y), static_cast<int>(width()),
                     static_cast<int>(height()) };
        }

        [[nodiscard]] std::pair<T, T> to_dimension_pair() const {
            return { width(), height() };
        }

        [[nodiscard]] AbstractPoint<T> to_dimension_point() const {
            return { width(), height() };
        }

        template<typename S>
        constexpr AbstractRect<S> cast() const {
            return AbstractRect<S>{ top_left.template cast<S>(), bottom_right.template cast<S>() };
        }
    };

    // move the rectangle by distance
    template<typename S, typename T>
    [[nodiscard]] constexpr AbstractRect<S> operator>>(const AbstractRect<S>& rect, AbstractPoint<T> distance) {
        return AbstractRect<S>{ rect.top_left + (distance.template cast<S>()),
                                rect.bottom_right + (distance.template cast<S>()) };
    }


    template<typename T>
    inline std::ostream& operator<<(std::ostream& ostream, const AbstractRect<T>& rect) {
        ostream << rect.top_left << ": " << rect.width() << "x" << rect.height();
        return ostream;
    }

    using IRect = AbstractRect<i32>;
    using URect = AbstractRect<u32>;

} // namespace shapes
