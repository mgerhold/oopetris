#pragma once

#include "color.hpp"

#include "helper/types.hpp"
#include "helper/utils.hpp"
#include <limits>


namespace {
    namespace const_constants {

        // offsets in C strings for hex rgb and rgba
        constexpr std::size_t red_offset = 1;
        constexpr std::size_t green_offset = 3;
        constexpr std::size_t blue_offset = 5;
        constexpr std::size_t alpha_offset = 7;
        constexpr std::size_t hex_rgb_size = alpha_offset;
        constexpr std::size_t hex_rgba_size = 9;

    } // namespace const_constants

    namespace const_utils {

        // represents a sort of constexpr std::expected

#define PROPAGATE(val, C) /*NOLINT(cppcoreguidelines-macro-usage)*/       \
    do {                  /*NOLINT(cppcoreguidelines-avoid-do-while)*/    \
        if (not const_utils::has_value(val)) {                            \
            return const_utils::error_result<C>(const_utils::error(val)); \
        }                                                                 \
    } while (false)

        template<typename C>
        using ResultType = std::pair<C, std::string>;

        template<typename C>
        [[nodiscard]] constexpr ResultType<C> good_result(C type) {
            return { type, "" };
        }

        template<typename C>
        [[nodiscard]] constexpr ResultType<C> error_result(const std::string& error) {
            return { C{}, error };
        }

        template<typename C>
        [[nodiscard]] constexpr bool has_value(const ResultType<C>& value) {
            return value.second.empty();
        }

        template<typename C>
        [[nodiscard]] constexpr C value(const ResultType<C>& value) {
            return value.first;
        }

        template<typename C>
        [[nodiscard]] constexpr std::string error(const ResultType<C>& value) {
            return value.second;
        }

    } // namespace const_utils

    // decode a decimal number
    [[nodiscard]] constexpr const_utils::ResultType<u8> single_decimal_number(char n) {
        if (n >= '0' && n <= '9') {
            return const_utils::good_result(static_cast<u8>(n - '0'));
        }

        return const_utils::error_result<u8>("the input must be a valid decimal character");
    }

    // decode a single_hex_number
    [[nodiscard]] constexpr const_utils::ResultType<u8> single_hex_number(char n) {
        if (n >= '0' && n <= '9') {
            return const_utils::good_result(static_cast<u8>(n - '0'));
        }

        if (n >= 'A' && n <= 'F') {
            return const_utils::good_result(static_cast<u8>(n - 'A' + 10));
        }

        if (n >= 'a' && n <= 'f') {
            return const_utils::good_result(static_cast<u8>(n - 'a' + 10));
        }

        return const_utils::error_result<u8>("the input must be a valid hex character");
    }


    //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // decode a single 2 digit color value in hex
    [[nodiscard]] constexpr const_utils::ResultType<u8> single_hex_color_value(const char* input) {

        const auto first = single_hex_number(input[0]);

        PROPAGATE(first, u8);

        const auto second = single_hex_number(input[1]);

        PROPAGATE(second, u8);

        return const_utils::good_result((const_utils::value(first) << 4) | const_utils::value(second));
    }

    template<typename T>
    using CharIteratorResult = std::pair<T, const char*>;

    using DoubleReturnValue = CharIteratorResult<double>;

    // decode a single color value as double
    [[nodiscard]] constexpr const_utils::ResultType<DoubleReturnValue> single_double_color_value(const char* value) {

        double result{ 0.0 };
        bool after_comma = false;
        double pow_of_10 = 1.0;

        for (std::size_t i = 0;; ++i) {

            const char current_char = value[i];

            switch (current_char) {
                case ' ':
                case '_':
                    break;
                case '.':
                    if (after_comma) {
                        return const_utils::error_result<DoubleReturnValue>("only one comma allowed");
                    }
                    after_comma = true;
                    break;
                case ',':
                case ')':
                    return const_utils::good_result<DoubleReturnValue>({ result, value + i });
                case '\0':
                    return const_utils::error_result<DoubleReturnValue>("input ended too early");
                default: {

                    const auto char_result = single_decimal_number(current_char);

                    PROPAGATE(char_result, DoubleReturnValue);

                    const auto value_of_char = const_utils::value(char_result);

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

    using AnyColorReturnValue = CharIteratorResult<std::size_t>;

    // decode a single_hex_number
    [[nodiscard]] constexpr const_utils::ResultType<AnyColorReturnValue> single_color_value_any(const char* value) {

        bool accept_hex = false;
        std::size_t start = 0;
        std::size_t mul_unit = 10;

        // skip leading white space
        while (value[start] == ' ') {
            ++start;
        }

        if (value[start] == '0' && value[start + 1] == 'x') {
            start += 2;
            accept_hex = true;
            mul_unit = 0x10;
        }


        std::size_t result{ 0 };

        const auto max_value_before_multiplication = (std::numeric_limits<decltype(result)>::max() / mul_unit);

        const auto max_value_before_multiplication_rest =
                std::numeric_limits<decltype(result)>::max() - (max_value_before_multiplication * mul_unit);

        for (std::size_t i = start;; ++i) {

            const char current_char = value[i];

            switch (current_char) {
                case ' ':
                case '_':
                    break;
                case ',':
                case ')':
                    return const_utils::good_result<AnyColorReturnValue>({ result, value + i });
                case '\0':
                    return const_utils::error_result<AnyColorReturnValue>("input ended too early");
                default: {

                    const auto char_result =
                            accept_hex ? single_hex_number(current_char) : single_decimal_number(current_char);

                    PROPAGATE(char_result, DoubleReturnValue);

                    const auto value_of_char = const_utils::value(char_result);

                    if (result == max_value_before_multiplication
                        && value_of_char > max_value_before_multiplication_rest) {
                        return const_utils::error_result<AnyColorReturnValue>("overflow detected");
                    }

                    if (result > max_value_before_multiplication) {
                        return const_utils::error_result<AnyColorReturnValue>("overflow detected");
                    }

                    result *= mul_unit;
                    result += value_of_char;
                }
            }
        }
    }


    [[nodiscard]] constexpr const_utils::ResultType<Color>
    get_color_from_hex_string(const char* input, std::size_t size) { //NOLINT(readability-function-cognitive-complexity

        if (size == const_constants::hex_rgb_size) {

            const auto r = single_hex_color_value(input + const_constants::red_offset);
            PROPAGATE(r, Color);

            const auto g = single_hex_color_value(input + const_constants::green_offset);
            PROPAGATE(g, Color);

            const auto b = single_hex_color_value(input + const_constants::blue_offset);
            PROPAGATE(b, Color);

            return const_utils::good_result(Color{ const_utils::value(r), const_utils::value(g), const_utils::value(b) }
            );
        }

        if (size == const_constants::hex_rgba_size) {

            const auto r = single_hex_color_value(input + const_constants::red_offset);
            PROPAGATE(r, Color);

            const auto g = single_hex_color_value(input + const_constants::green_offset);
            PROPAGATE(g, Color);

            const auto b = single_hex_color_value(input + const_constants::blue_offset);
            PROPAGATE(b, Color);

            const auto a = single_hex_color_value(input + const_constants::alpha_offset);
            PROPAGATE(a, Color);

            return const_utils::good_result(Color{ const_utils::value(r), const_utils::value(g), const_utils::value(b),
                                                   const_utils::value(a) });
        }


        return const_utils::error_result<Color>("Unrecognized HEX literal");
    }

    [[nodiscard]] constexpr const_utils::ResultType<Color>
    get_color_from_rgb_string(const char* input, std::size_t) { //NOLINT(readability-function-cognitive-complexity

        if (input[0] == 'r' && input[1] == 'g' && input[2] == 'b') {
            if (input[3] == '(') {

                const auto r_result = single_color_value_any(input + 4);

                PROPAGATE(r_result, Color);

                const auto [r, next_g] = const_utils::value(r_result);

                if (r > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("r has to be in range 0 - 255");
                }

                if (*next_g != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto g_result = single_color_value_any(next_g + 1);

                PROPAGATE(g_result, Color);

                const auto [g, next_b] = const_utils::value(g_result);

                if (g > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("g has to be in range 0 - 255");
                }

                if (*next_b != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto b_result = single_color_value_any(next_b + 1);

                PROPAGATE(b_result, Color);

                const auto [b, end] = const_utils::value(b_result);

                if (b > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("b has to be in range 0 - 255");
                }

                if (*end != ')') {
                    return const_utils::error_result<Color>("expected ')'");
                }

                if (*(end + 1) != '\0') {
                    return const_utils::error_result<Color>("expected end of string");
                }


                return const_utils::good_result(Color{ static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b) });
            }


            if (input[3] == 'a' && input[4] == '(') {


                const auto r_result = single_double_color_value(input + 5);

                PROPAGATE(r_result, Color);

                const auto [r, next_g] = const_utils::value(r_result);

                if (r > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("r has to be in range 0 - 255");
                }

                if (*next_g != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto g_result = single_double_color_value(next_g + 1);

                PROPAGATE(g_result, Color);

                const auto [g, next_b] = const_utils::value(g_result);

                if (g > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("g has to be in range 0 - 255");
                }

                if (*next_b != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto b_result = single_double_color_value(next_b + 1);

                PROPAGATE(b_result, Color);

                const auto [b, next_a] = const_utils::value(b_result);

                if (b > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("b has to be in range 0 - 255");
                }

                if (*next_a != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }

                const auto a_result = single_color_value_any(next_a + 1);

                PROPAGATE(a_result, Color);

                const auto [a, end] = const_utils::value(a_result);

                if (a > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("a has to be in range 0 - 255");
                }


                if (*end != ')') {
                    return const_utils::error_result<Color>("expected ')'");
                }

                if (*(end + 1) != '\0') {
                    return const_utils::error_result<Color>("expected end of string");
                }


                return const_utils::good_result(Color{ static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b),
                                                       static_cast<u8>(a) });
            }
        }


        return const_utils::error_result<Color>("Unrecognized HSV literal");
    }

    [[nodiscard]] constexpr const_utils::ResultType<Color>
    get_color_from_hsv_string(const char* input, std::size_t) { //NOLINT(readability-function-cognitive-complexity

        if (input[0] == 'h' && input[1] == 's' && input[2] == 'v') {
            if (input[3] == '(') {

                const auto h_result = single_double_color_value(input + 4);

                PROPAGATE(h_result, Color);

                const auto [h, next_s] = const_utils::value(h_result);

                if (h < 0.0 || h > 360.0) {
                    return const_utils::error_result<Color>("h has to be in range 0.0 - 360.0");
                }

                if (*next_s != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto s_result = single_double_color_value(next_s + 1);

                PROPAGATE(s_result, Color);

                const auto [s, next_v] = const_utils::value(s_result);

                if (s < 0.0 || s > 1.0) {
                    return const_utils::error_result<Color>("s has to be in range 0.0 - 1.0");
                }

                if (*next_v != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto v_result = single_double_color_value(next_v + 1);

                PROPAGATE(v_result, Color);

                const auto [v, end] = const_utils::value(v_result);

                if (v < 0.0 || v > 1.0) {
                    return const_utils::error_result<Color>("v has to be in range 0.0 - 1.0");
                }

                if (*end != ')') {
                    return const_utils::error_result<Color>("expected ')'");
                }

                if (*(end + 1) != '\0') {
                    return const_utils::error_result<Color>("expected end of string");
                }


                return const_utils::good_result(Color{
                        HSVColor{h, s, v}
                });
            }


            if (input[3] == 'a' && input[4] == '(') {


                const auto h_result = single_double_color_value(input + 5);

                PROPAGATE(h_result, Color);

                const auto [h, next_s] = const_utils::value(h_result);

                if (h < 0.0 || h > 360.0) {
                    return const_utils::error_result<Color>("h has to be in range 0.0 - 360.0");
                }

                if (*next_s != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto s_result = single_double_color_value(next_s + 1);

                PROPAGATE(s_result, Color);

                const auto [s, next_v] = const_utils::value(s_result);

                if (s < 0.0 || s > 1.0) {
                    return const_utils::error_result<Color>("s has to be in range 0.0 - 1.0");
                }

                if (*next_v != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }


                const auto v_result = single_double_color_value(next_v + 1);

                PROPAGATE(v_result, Color);

                const auto [v, next_a] = const_utils::value(v_result);

                if (v < 0.0 || v > 1.0) {
                    return const_utils::error_result<Color>("v has to be in range 0.0 - 1.0");
                }

                if (*next_a != ',') {
                    return const_utils::error_result<Color>("expected ','");
                }

                const auto a_result = single_color_value_any(next_a + 1);

                PROPAGATE(a_result, Color);

                const auto [a, end] = const_utils::value(a_result);

                if (a > std::numeric_limits<u8>::max()) {
                    return const_utils::error_result<Color>("a has to be in range 0 - 255");
                }


                if (*end != ')') {
                    return const_utils::error_result<Color>("expected ')'");
                }

                if (*(end + 1) != '\0') {
                    return const_utils::error_result<Color>("expected end of string");
                }


                return const_utils::good_result(Color{
                        HSVColor{h, s, v, static_cast<u8>(a)}
                });
            }
        }


        return const_utils::error_result<Color>("Unrecognized HSV literal");
    }

    [[nodiscard]] constexpr const_utils::ResultType<Color>
    get_color_from_string_impl(const char* input, std::size_t size) {

        if (size == 0) {
            return const_utils::error_result<Color>("not enough data to determine the literal type");
        }

        switch (input[0]) {
            case '#':
                return get_color_from_hex_string(input, size);
            case 'r':
                return get_color_from_rgb_string(input, size);
            case 'h':
                return get_color_from_hsv_string(input, size);
            default:
                return const_utils::error_result<Color>("Unrecognized color literal");
        }
    }
    //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
} // namespace


namespace detail {

    [[nodiscard]] constexpr const_utils::ResultType<Color> get_color_from_string(const std::string& input) {
        return get_color_from_string_impl(input.c_str(), input.size());
    }


} // namespace detail

consteval Color operator""_c(const char* input, std::size_t size) {
    const auto result = get_color_from_string_impl(input, size);

    CONSTEVAL_STATIC_ASSERT(const_utils::has_value(result), "incorrect color literal");

    return const_utils::value(result);
}


// sanity tests at compile time:

static_assert("#ABCDEF"_c == "#ABCDEFFF"_c);
static_assert("hsv(0, 0, 0)"_c == "hsva(0, 0, 0, 0xFF)"_c);
static_assert("rgb(0, 0, 0)"_c == "rgba(0, 0, 0, 0xFF)"_c);
static_assert("rgb(0xAB, 0xCD, 0xEF)"_c == "rgb(171, 205, 239)"_c);
static_assert("hsv(0, 0, 0.79)"_c == "#C9C9C9"_c);
