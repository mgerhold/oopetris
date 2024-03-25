
#include <SDL.h>
#include <vector>

#include "helper/utils.hpp"
#include "platform/capabilities.hpp"


#if defined(__SWITCH__)
#include "platform/switch_buttons.hpp"
#endif


namespace {

    inline std::string get_error_from_errno() {

#if defined(_MSC_VER)
        char buffer[256] = { 0 };
        const auto result = strerror_s<256>(buffer, errno);

        if (result == 0) {
            return std::string{ buffer };

        } else {
            return std::string{ "Error while getting error!" };
        }

#else
        return std::string{ std::strerror(errno) };

#endif
    }


} // namespace


[[nodiscard]] std::string utils::built_for_platform() {
#if defined(__ANDROID__)
    return "Android";
#elif defined(__SWITCH__)
    return "Nintendo Switch";
#elif defined(FLATPAK_BUILD)
    return "Linux (Flatpak)";
#elif defined(__linux__)
    return "Linux";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "MacOS";
#else
#error "Unsupported platform"
#endif
}

// partially from: https://stackoverflow.com/questions/17347950/how-do-i-open-a-url-from-c
[[nodiscard]] bool utils::open_url(const std::string& url) {
#if defined(__ANDROID__)
    const auto result = SDL_OpenURL(url.c_str());
    if (result < 0) {
        spdlog::error("Error in opening url in android: {}", SDL_GetError());
        return false;
    }


    return true;

#elif defined(__SWITCH__)
    UNUSED(url);
    return false;
#else

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    const std::string shellCommand = "start " + url;
#elif defined(__APPLE__)
    const std::string shellCommand = "open " + url;
#elif defined(__linux__)
    const std::string shellCommand = "xdg-open " + url;
#else
#error "Unsupported platform"
#endif

    const auto result = system(shellCommand.c_str());
    if (result < 0) {
        spdlog::error("Error in opening url: {}", get_error_from_errno());
        return false;
    }


    return true;


#endif
}


[[nodiscard]] bool utils::event_is_action(const SDL_Event& event, const CrossPlatformAction action) {
#if defined(__ANDROID__)
    switch (action) {
        case CrossPlatformAction::OK:
        case CrossPlatformAction::DOWN:
        case CrossPlatformAction::UP:
        case CrossPlatformAction::LEFT:
        case CrossPlatformAction::RIGHT:
        case CrossPlatformAction::OPEN_SETTINGS:
        case CrossPlatformAction::TAB:
            // this can't be checked here, it has to be checked via collision on buttons etc. event_is_action(..., ...::DOWN, UP ...) can only be used inside device_supports_keys() clauses!
            throw std::runtime_error("Not supported on android 'event_is_action'");
        case CrossPlatformAction::PAUSE:
            return (event.type == SDL_KEYDOWN and event.key.keysym.sym == SDLK_AC_BACK);

        case CrossPlatformAction::UNPAUSE:
            return event.type == SDL_FINGERDOWN;

        case CrossPlatformAction::EXIT:
        case CrossPlatformAction::CLOSE:
            return (event.type == SDL_KEYDOWN and event.key.keysym.sym == SDLK_AC_BACK);

        default:
            utils::unreachable();
    }
#else

    const std::vector<i64> needed_events = utils::key_map.at(static_cast<u8>(action));
    for (const auto& needed_event : needed_events) { // NOLINT(readability-use-anyofallof)
#if defined(__SWITCH__)
        if (event.type == SDL_JOYBUTTONDOWN and event.jbutton.button == needed_event) {
            return true;
        }
#else
        if (event.type == SDL_KEYDOWN and event.key.keysym.sym == needed_event) {
            return true;
        }
#endif
    }
    return false;
#endif
}


[[nodiscard]] std::vector<i64> utils::get_bound_keys() {
    std::vector<i64> bound_keys{};
    for (const auto& [_, keys] : utils::key_map) {
        for (const auto key : keys) {
            bound_keys.push_back(key);
        }
    }
    return bound_keys;
}


[[nodiscard]] std::vector<i64> utils::get_bound_keys(const std::vector<utils::CrossPlatformAction>& actions) {
    std::vector<i64> bound_keys{};
    for (const auto& [value, keys] : utils::key_map) {
        if (std::find(actions.cbegin(), actions.cend(), static_cast<utils::CrossPlatformAction>(value))
            == actions.cend()) {
            continue;
        }
        for (const auto key : keys) {
            bound_keys.push_back(key);
        }
    }
    return bound_keys;
}

[[nodiscard]] std::string utils::action_description(CrossPlatformAction action) {
#if defined(__ANDROID__)

    switch (action) {
        case CrossPlatformAction::OK:
        case CrossPlatformAction::DOWN:
        case CrossPlatformAction::UP:
        case CrossPlatformAction::LEFT:
        case CrossPlatformAction::RIGHT:
        case CrossPlatformAction::OPEN_SETTINGS:
        case CrossPlatformAction::TAB:
            // this can't be checked here, it has to be checked via collision on buttons etc. event_is_action(..., ...::DOWN, UP ...) can only be used inside device_supports_keys() clauses!
            throw std::runtime_error("Not supported on android 'action_description'");
        case CrossPlatformAction::UNPAUSE:
            return "Tap anywhere";
        case CrossPlatformAction::PAUSE:
        case CrossPlatformAction::EXIT:
        case CrossPlatformAction::CLOSE:
            return "Back";

        default:
            utils::unreachable();
    }
#elif defined(__SWITCH__)
    switch (action) {
        case CrossPlatformAction::OK:
            return "A";
        case CrossPlatformAction::PAUSE:
        case CrossPlatformAction::UNPAUSE:
            return "PLUS";
        case CrossPlatformAction::CLOSE:
        case CrossPlatformAction::EXIT:
            return "MINUS";
        case CrossPlatformAction::DOWN:
            return "Down";
        case CrossPlatformAction::UP:
            return "Up";
        case CrossPlatformAction::LEFT:
            return "Left";
        case CrossPlatformAction::RIGHT:
            return "Right";
        case CrossPlatformAction::OPEN_SETTINGS:
            return "Y";
        default:
            utils::unreachable();
    }
#else
    switch (action) {
        case CrossPlatformAction::OK:
            return "Enter";
        case CrossPlatformAction::PAUSE:
        case CrossPlatformAction::UNPAUSE:
        case CrossPlatformAction::CLOSE:
            return "Esc";
        case CrossPlatformAction::EXIT:
            return "Enter";
        case CrossPlatformAction::DOWN:
            return "Down";
        case CrossPlatformAction::UP:
            return "Up";
        case CrossPlatformAction::LEFT:
            return "Left";
        case CrossPlatformAction::RIGHT:
            return "Right";
        case CrossPlatformAction::OPEN_SETTINGS:
            return "E";
        default:
            utils::unreachable();
    }
#endif
}


//TODO: not correctly supported ButtonUp, since it only can be triggered, when a ButtonDown event is fired first and the target is not left (unhovered)

[[nodiscard]] bool utils::event_is_click_event(const SDL_Event& event, CrossPlatformClickEvent click_type) {

#if defined(__ANDROID__)
    decltype(event.type) desired_type{};
    switch (click_type) {
        case CrossPlatformClickEvent::Motion:
            desired_type = SDL_FINGERMOTION;
            break;
        case CrossPlatformClickEvent::ButtonDown:
            desired_type = SDL_FINGERDOWN;
            break;
        case CrossPlatformClickEvent::ButtonUp:
            desired_type = SDL_FINGERUP;
            break;
        case CrossPlatformClickEvent::Any:
            return event.type == SDL_FINGERMOTION || event.type == SDL_FINGERDOWN || event.type == SDL_FINGERUP;
        default:
            utils::unreachable();
    }

    return event.type == desired_type;

#elif defined(__SWITCH__)
    UNUSED(event);
    UNUSED(click_type);
    throw std::runtime_error("Not supported on the Nintendo switch");
#else
    decltype(event.type) desired_type{};
    switch (click_type) {
        case CrossPlatformClickEvent::Motion:
            desired_type = SDL_MOUSEMOTION;
            break;
        case CrossPlatformClickEvent::ButtonDown:
            desired_type = SDL_MOUSEBUTTONDOWN;
            break;
        case CrossPlatformClickEvent::ButtonUp:
            desired_type = SDL_MOUSEBUTTONUP;
            break;
        case CrossPlatformClickEvent::Any:
            return event.type == SDL_MOUSEMOTION
                   || ((event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
                       && event.button.button == SDL_BUTTON_LEFT);
        default:
            utils::unreachable();
    }


    return event.type == desired_type && event.button.button == SDL_BUTTON_LEFT;
#endif
}

[[nodiscard]] std::pair<i32, i32> utils::get_raw_coordinates(const Window* window, const SDL_Event& event) {

    assert(utils::event_is_click_event(event, utils::CrossPlatformClickEvent::Any) && "expected a click event");

#if defined(__ANDROID__)
    // These are doubles, from 0-1 (or if using virtual layouts > 0) in percent, the have to be casted to absolut x coordinates!
    const double x_percent = event.tfinger.x;
    const double y_percent = event.tfinger.y;
    const auto window_size = window->size();
    const auto x = static_cast<i32>(std::round(x_percent * window_size.x));
    const auto y = static_cast<i32>(std::round(y_percent * window_size.y));


#elif defined(__SWITCH__)
    UNUSED(window);
    UNUSED(event);
    throw std::runtime_error("Not supported on the Nintendo switch");
    int x{};
    int y{};
#else
    UNUSED(window);

    Sint32 x{};
    Sint32 y{};
    switch (event.type) {
        case SDL_MOUSEMOTION:
            x = event.motion.x;
            y = event.motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            x = event.button.x;
            y = event.button.y;
            break;
        default:
            utils::unreachable();
    }
#endif


    return { static_cast<i32>(x), static_cast<i32>(y) };
}


[[nodiscard]] bool utils::is_event_in(const Window* window, const SDL_Event& event, const shapes::URect& rect) {

    const auto& [x, y] = get_raw_coordinates(window, event);
    const auto casted_rect = rect.cast<i32>();

    const auto rect_start_x = casted_rect.top_left.x;
    const auto rect_start_y = casted_rect.top_left.y;
    const auto rect_end_x = casted_rect.bottom_right.x;
    const auto rect_end_y = casted_rect.bottom_right.y;


    const bool button_clicked = (x >= rect_start_x and x <= rect_end_x and y >= rect_start_y and y <= rect_end_y);

    return button_clicked;
}


[[nodiscard]] SDL_Event utils::offset_event(const Window* window, const SDL_Event& event, const shapes::IPoint& point) {


    assert(utils::event_is_click_event(event, utils::CrossPlatformClickEvent::Any) && "expected a click event");


    auto new_event = event;

#if defined(__ANDROID__)
    // These are doubles in percent, the have to be modified by using the windows sizes


    const double x_percent = event.tfinger.x;
    const double y_percent = event.tfinger.y;
    const auto window_size = window->size();
    new_event.tfinger.x = x_percent + static_cast<double>(point.x) / static_cast<double>(window_size.x);
    new_event.tfinger.y = y_percent + static_cast<double>(point.y) / static_cast<double>(window_size.y);


#elif defined(__SWITCH__)
    UNUSED(window);
    UNUSED(event);
    UNUSED(point);
    UNUSED(new_event);
    throw std::runtime_error("Not supported on the Nintendo switch");
#else
    UNUSED(window);

    switch (event.type) {
        case SDL_MOUSEMOTION:
            new_event.motion.x = event.motion.x + point.x;
            new_event.motion.y = event.motion.y + point.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            new_event.button.x = event.button.x + point.x;
            new_event.button.y = event.button.y + point.y;
            break;
        default:
            utils::unreachable();
    }
#endif


    return new_event;
}
