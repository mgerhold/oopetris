#pragma once

#include "helper/expected.hpp"
#include "helper/types.hpp"
#include "helper/utils.hpp"

#if !defined(_NO_SDL)
#include <SDL.h>
#endif

#include <algorithm>
#include <ostream>
#include <string>

struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;

    constexpr Color() : Color{ 0, 0, 0, 0 } { }

    constexpr Color(u8 r, u8 g, u8 b, u8 a) // NOLINT(bugprone-easily-swappable-parameters)
        : r{ r },
          g{ g },
          b{ b },
          a{ a } { }

    constexpr Color(u8 r, u8 g, u8 b) : Color{ r, g, b, 0xFF } { }

    [[nodiscard]] static helper::expected<Color, std::string> from_string(const std::string& value);


    [[nodiscard]] static constexpr Color
    from_hsv(double h, double s, double v, u8 a = 0xFF) { // NOLINT(bugprone-easily-swappable-parameters)

        if (utils::is_constant_evaluated()) {

            CONSTEVAL_STATIC_ASSERT(h >= 0.0 && h <= 360, "h has to be in range 0.0 - 360.0");
            CONSTEVAL_STATIC_ASSERT(s >= 0.0 && s <= 1.0, "s has to be in range 0.0 - 1.0");
            CONSTEVAL_STATIC_ASSERT(v >= 0.0 && v <= 1.0, "v has to be in range 0.0 - 1.0");

        } else {

            if (h < 0.0 || h > 360.0) {
                throw std::runtime_error{ "h has to be in range 0.0 - 360.0" };
            }
            if (s < 0.0 || s > 1.0) {

                throw std::runtime_error{ "s has to be in range 0.0 - 1.0" };
            }
            if (v < 0.0 || v > 1.0) {
                throw std::runtime_error("v has to be in range 0.0 - 1.0");
            }
        }

        const auto double_to_color = [a](double r, double g, double b) -> Color {
            const auto ur = static_cast<u8>(std::clamp(r, 0.0, 1.0) * static_cast<double>(0xFF));
            const auto ug = static_cast<u8>(std::clamp(g, 0.0, 1.0) * static_cast<double>(0xFF));
            const auto ub = static_cast<u8>(std::clamp(b, 0.0, 1.0) * static_cast<double>(0xFF));

            return { ur, ug, ub, a };
        };

        // taken from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
        double hh{};
        double p{};
        double q{};
        double t{};
        double ff{};

        long i{};

        double r{};
        double g{};
        double b{};

        if (s <= 0.0) { // < is bogus, just shuts up warnings
            r = v;
            g = v;
            b = v;
            return double_to_color(r, g, b);
        }

        hh = h;
        if (hh >= 360.0) {
            hh = 0.0;
        }

        hh /= 60.0;
        i = static_cast<long>(hh);
        ff = hh - static_cast<double>(i);

        p = v * (1.0 - s);
        q = v * (1.0 - (s * ff));
        t = v * (1.0 - (s * (1.0 - ff)));

        switch (i) {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;

            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            case 5:
            default:
                r = v;
                g = p;
                b = q;
                break;
        }

        return double_to_color(r, g, b);
    }

    [[nodiscard]] constexpr bool operator==(const Color& other) const {
        return std::tuple<u8, u8, u8, u8>{ r, g, b, a } == std::tuple{ other.r, other.g, other.b, other.a };
    }


    static constexpr Color red(u8 alpha = 0xFF) {
        return Color{ 0xFF, 0, 0, alpha };
    };

    static constexpr Color green(u8 alpha = 0xFF) {
        return Color{ 0, 0xFF, 0, alpha };
    };

    static constexpr Color blue(u8 alpha = 0xFF) {
        return Color{ 0, 0, 0xFF, alpha };
    };

    static constexpr Color black(u8 alpha = 0xFF) {
        return Color{ 0, 0, 0, alpha };
    };

    static constexpr Color white(u8 alpha = 0xFF) {
        return Color{ 0xFF, 0xFF, 0xFF, alpha };
    };

#if !defined(_NO_SDL)
    [[nodiscard]] SDL_Color to_sdl_color() const;
#endif

    enum class SerializeMode : u8 { Hex, RGB, HSV };

    [[nodiscard]] std::string to_string(SerializeMode mode = SerializeMode::RGB) const;

    std::ostream& operator<<(std::ostream& os) const;
};
