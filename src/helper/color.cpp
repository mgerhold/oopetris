

#include "color.hpp"
#include "color_literals.hpp"
#include "helper/expected.hpp"
#include "helper/utils.hpp"

#include <fmt/format.h>

helper::expected<Color, std::string> Color::from_string(const std::string& value) {

    const auto result = detail::get_color_from_string(value);

    if (result.has_value()) {
        return result.value();
    }

    return helper::unexpected<std::string>{ result.error() };
}


[[nodiscard]] HSVColor Color::to_hsv_color() const {

    // taken from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

    HSVColor out{};
    out.a = a;
    double min{};
    double max{};
    double delta{};

    min = r < g ? r : g;
    min = min < b ? min : b;

    max = r > g ? r : g;
    max = max > b ? max : b;

    out.v = max; // v
    delta = max - min;
    if (delta < 0.00001) {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        throw std::runtime_error{ "Undefined conversion, h would be NaN" };
    }
    if (max > 0.0) {           // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max); // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN; // its now undefined
        throw std::runtime_error{ "Undefined conversion, h would be NaN" };
    }
    if (r >= max) {              // > is bogus, just keeps compilor happy
        out.h = (g - b) / delta; // between yellow & magenta
    } else if (g >= max) {
        out.h = 2.0 + (b - r) / delta; // between cyan & yellow
    } else {
        out.h = 4.0 + (r - g) / delta; // between magenta & cyan
    }
    out.h *= 60.0; // degrees

    if (out.h < 0.0) {
        out.h += 360.0;
    }

    return out;
}

#if !defined(_NO_SDL)
[[nodiscard]] SDL_Color Color::to_sdl_color() const {
    return SDL_Color{ r, g, b, a };
}
#endif

//NOte: this output formats are all deserializable by the from_string method!
[[nodiscard]] std::string Color::to_string(SerializeMode mode) const {

    switch (mode) {
        case SerializeMode::Hex: {
            return fmt::format("#{:02x}{:02x}{:02x}{:02x}", r, g, b, a);
        }
        case SerializeMode::RGB: {
            return fmt::format("rgba({}, {}, {}, {:#2x})", r, g, b, a);
        }
        case SerializeMode::HSV: {
            const auto color = to_hsv_color();
            return fmt::format("hsva({:.2f}, {:.5f}, {:.5f}, {:#2x})", color.h, color.s, color.v, color.a);
        }
        default:
            utils::unreachable();
    }
}


std::ostream& Color::operator<<(std::ostream& os) const {
    os << to_string();
    return os;
}
