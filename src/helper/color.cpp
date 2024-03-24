

#include "color.hpp"
#include "color_literals.hpp"
#include "helper/expected.hpp"
#include "helper/utils.hpp"

#include <fmt/format.h>

helper::expected<Color, std::string> Color::from_string(const std::string& value) {

    const auto result = detail::get_color_from_string(value);

    if (const_utils::has_value(result)) {
        return const_utils::value(result);
    }

    return helper::unexpected<std::string>{ const_utils::error(result) };
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
            return fmt::format("rgba({}, {}, {}, {})", r, g, b, a);
        }
        case SerializeMode::HSV: {
            //TODO
            return "TODO";
        }
        default:
            utils::unreachable();
    }
}


std::ostream& Color::operator<<(std::ostream& os) const {
    os << to_string();
    return os;
}
