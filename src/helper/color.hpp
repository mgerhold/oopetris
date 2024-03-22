#pragma once

#include "helper/types.hpp"
#include "helper/utils.hpp"

#if !defined(_NO_SDL)
#include <SDL.h>
#endif

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <sstream>

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

    constexpr static Color
    from_hsv(double h, double s, double v, u8 a = 0xFF) { // NOLINT(bugprone-easily-swappable-parameters)

        assert(h >= 0.0 && h <= 360 && "h has to be in range 0.0 - 360.0");
        assert(s >= 0.0 && s <= 1.0 && "s has to be in range 0.0 - 1.0");
        assert(v >= 0.0 && v <= 1.0 && "v has to be in range 0.0 - 1.0");

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
    [[nodiscard]] SDL_Color to_sdl_color() const {
        return SDL_Color{ r, g, b, a };
    }
#endif


    // helper to display values
    inline std::ostream& operator<<(std::ostream& os) const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << '#' << std::setw(2) << static_cast<int>(r) << std::setw(2)
            << static_cast<int>(g) << std::setw(2) << static_cast<int>(b) << std::setw(2) << static_cast<int>(a) << ';';
        return os << oss.str();
    }
};


// taken from https://stackoverflow.com/questions/66813961/c-constexpr-constructor-for-colours:
// heavily modified in many places
namespace {
// define a consteval assert, it isn't a pretty error message, but there's nothing we can do against that atm :(
// this https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2758r2.html tries to fix it
#define CONSTEVAL_STATIC_ASSERT(CHECK, MSG) /*NOLINT(cppcoreguidelines-macro-usage)*/                                                           \
    ((CHECK) ? void(0) : [] {                                                                                                                   \
        /* If you see this really bad c++ error message, follow the origin of MSG, to see the real error message, c++ error messages suck xD */ \
        throw(MSG);                                                                                                                             \
    }())


    // decode a single_hex_number
    consteval u8 single_hex_number(char n) {
        if (n >= '0' && n <= '9') {
            return static_cast<u8>(n - '0');
        }

        if (n >= 'A' && n <= 'F') {
            return static_cast<u8>(n - 'A' + 10);
        }

        if (n >= 'a' && n <= 'f') {
            return static_cast<u8>(n - 'a' + 10);
        }

        CONSTEVAL_STATIC_ASSERT(false, "the input must be a valid hex character");
        utils::unreachable();
    }

    // decode a single color value
    consteval u8 single_color_value(const char* b) {
        return single_hex_number(b[0]) << 4 //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               | single_hex_number(b[1]);   //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    // consteval strlen
    consteval usize strlen_consteval(const char* value) {

        usize length = 0;

        while (value[length] != '\0') { //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ++length;
        }

        return length;
    }

    // decode a single color value as double
    consteval std::pair<double, const char*> single_color_double(const char* value) {

        const auto& get_value_of_char = [](const char val) {
            if (val >= '0' && val <= '9') {
                return static_cast<u8>(val - '0');
            }

            CONSTEVAL_STATIC_ASSERT(false, "the input must be a valid decimal character");
            utils::unreachable();
        };


        double result{};
        bool after_comma = false;
        double pow_of_10 = 1.0;

        for (u8 i = 0;; ++i) {

            const char current_char = value[i]; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            switch (current_char) {
                case ' ':
                    break;
                case '.':
                    CONSTEVAL_STATIC_ASSERT(not after_comma, "only one comma allowed");
                    after_comma = true;
                    break;
                case ',':
                case ')':
                    return { result, value + i }; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                default: {

                    const auto value_of_char = get_value_of_char(current_char);

                    if (after_comma) {
                        pow_of_10 *= 10.0;
                        result += static_cast<double>(value_of_char) / pow_of_10;
                    } else {
                        result = (result * 10.0) + static_cast<double>(value_of_char);
                    }
                }
            }
        }
    }


    // offsets in C strings for rgb and rgba
    constexpr int red_offset = 1;
    constexpr int green_offset = 3;
    constexpr int blue_offset = 5;
    constexpr int alpha_offset = 7;
    constexpr int final_size = 9;


    // offsets in C strings for hsv
    constexpr int hsv_offset = 4;

} // namespace

consteval Color operator""_rgb(const char* s, std::size_t size) {
    CONSTEVAL_STATIC_ASSERT(size == alpha_offset, "RGB literals must be of size 7");
    CONSTEVAL_STATIC_ASSERT(
            s[0] == '#', "RGB literals must start with '#'" //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    );
    return { single_color_value(s + red_offset),    //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
             single_color_value(s + green_offset),  //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
             single_color_value(s + blue_offset) }; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

consteval Color operator""_rgba(const char* s, std::size_t size) {
    CONSTEVAL_STATIC_ASSERT(size == final_size, "RGBA literals must be of size 9");
    CONSTEVAL_STATIC_ASSERT(
            s[0] == '#', "RGBA literals must start with '#'" //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    );
    return { single_color_value(s + red_offset),     //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
             single_color_value(s + green_offset),   //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
             single_color_value(s + blue_offset),    //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
             single_color_value(s + alpha_offset) }; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

consteval Color operator""_hsv(const char* string, std::size_t size) {

    const char* min_hsv = "hsv(0,0,0)";

    CONSTEVAL_STATIC_ASSERT(size >= strlen_consteval(min_hsv), "HSV literals must have a minimum size");

    CONSTEVAL_STATIC_ASSERT(
            string[0] == 'h' && string[1] == 's' //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    && string[2] == 'v'          //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    && string[3] == '(',         //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            "HSV literals must start with \"hsv(\""
    );

    CONSTEVAL_STATIC_ASSERT(
            string[size - 1] == ')', //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            "HSV literals must end with \")\""
    );

    const auto [h, next_s] =
            single_color_double(string + hsv_offset); //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    CONSTEVAL_STATIC_ASSERT(h >= 0.0 && h <= 360.0, "h has to be in range 0.0 - 360.0");
    CONSTEVAL_STATIC_ASSERT(*next_s == ',', "next_s is ,");

    const auto [s, next_v] = single_color_double(next_s + 1);
    CONSTEVAL_STATIC_ASSERT(s >= 0.0 && s <= 1.0, "s has to be in range 0.0 - 1.0");
    CONSTEVAL_STATIC_ASSERT(*next_v == ',', "next_v is ,");

    const auto [v, end] = single_color_double(next_v + 1);
    CONSTEVAL_STATIC_ASSERT(v >= 0.0 && v <= 1.0, "v has to be in range 0.0 - 1.0");

    CONSTEVAL_STATIC_ASSERT(*end == ')', "last parser must finish at the end");

    return Color::from_hsv(h, s, v);
}
