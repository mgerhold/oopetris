
#pragma once

#include <SDL.h>
#include <cerrno>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "graphics/window.hpp"
#include "helper/types.hpp"
#include "helper/utils.hpp"

#if defined(__SWITCH__)
#include "platform/switch_buttons.hpp"
#endif

//TODO factor this file into mulitple ones, according to logic distribution of what the functions are doing

namespace utils {

    enum class Orientation : u8 {
        Portrait, // 9x16, e.g. smartphone
        Landscape //  16x9
    };

    struct Capabilities {
        bool supports_keys;
        bool supports_clicks;
        Orientation orientation;
    };

    // the PAUSE and UNPAUSE might be different (e.g on android, even if androids map is stub,
    // it checks in the usage of these for the CrossPlatformAction!), so don't remove the duplication here!
    enum class CrossPlatformAction : u8 { OK, PAUSE, UNPAUSE, EXIT, DOWN, UP, LEFT, RIGHT, CLOSE, OPEN_SETTINGS, TAB };

    static std::unordered_map<u8, std::vector<i64>>
            key_map = // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
#if defined(__ANDROID__)
            {
                {           static_cast<u8>(CrossPlatformAction::OK), { 0 }},
                {        static_cast<u8>(CrossPlatformAction::PAUSE), { 0 }},
                {      static_cast<u8>(CrossPlatformAction::UNPAUSE), { 0 }},
                {         static_cast<u8>(CrossPlatformAction::EXIT), { 0 }},
                {         static_cast<u8>(CrossPlatformAction::DOWN), { 0 }},
                {           static_cast<u8>(CrossPlatformAction::UP), { 0 }},
                {         static_cast<u8>(CrossPlatformAction::LEFT), { 0 }},
                {        static_cast<u8>(CrossPlatformAction::RIGHT), { 0 }},
                {        static_cast<u8>(CrossPlatformAction::CLOSE), { 0 }},
                {static_cast<u8>(CrossPlatformAction::OPEN_SETTINGS), { 0 }},
                {          static_cast<u8>(CrossPlatformAction::TAB), { 0 }}
    };
#elif defined(__SWITCH__)
            {
                {           static_cast<u8>(CrossPlatformAction::OK),{ JOYCON_A }                                                                     },
                {        static_cast<u8>(CrossPlatformAction::PAUSE),                                       { JOYCON_PLUS }},
                {      static_cast<u8>(CrossPlatformAction::UNPAUSE),                                       { JOYCON_PLUS }},
                {         static_cast<u8>(CrossPlatformAction::EXIT),                                      { JOYCON_MINUS }},
                {         static_cast<u8>(CrossPlatformAction::DOWN),
                 { JOYCON_CROSS_DOWN, JOYCON_LDPAD_DOWN, JOYCON_RDPAD_DOWN }                                               },
                {           static_cast<u8>(CrossPlatformAction::UP), { JOYCON_CROSS_UP, JOYCON_LDPAD_UP, JOYCON_RDPAD_UP }},
                {         static_cast<u8>(CrossPlatformAction::LEFT),
                 { JOYCON_CROSS_LEFT, JOYCON_LDPAD_LEFT, JOYCON_RDPAD_LEFT }                                               },
                {        static_cast<u8>(CrossPlatformAction::RIGHT),
                 { JOYCON_CROSS_RIGHT, JOYCON_LDPAD_RIGHT, JOYCON_RDPAD_RIGHT }                                            },
                {        static_cast<u8>(CrossPlatformAction::CLOSE),                                      { JOYCON_MINUS }},
                {static_cast<u8>(CrossPlatformAction::OPEN_SETTINGS),                                          { JOYCON_Y }},
                {          static_cast<u8>(CrossPlatformAction::TAB),                                                    {}}, // no tab support
};
#else
            {
                {           static_cast<u8>(CrossPlatformAction::OK), { SDLK_RETURN, SDLK_SPACE }},
                {        static_cast<u8>(CrossPlatformAction::PAUSE),             { SDLK_ESCAPE }},
                {      static_cast<u8>(CrossPlatformAction::UNPAUSE),             { SDLK_ESCAPE }},
                {         static_cast<u8>(CrossPlatformAction::EXIT),             { SDLK_RETURN }},
                {         static_cast<u8>(CrossPlatformAction::DOWN),       { SDLK_DOWN, SDLK_s }},
                {           static_cast<u8>(CrossPlatformAction::UP),         { SDLK_UP, SDLK_w }},
                {         static_cast<u8>(CrossPlatformAction::LEFT),       { SDLK_LEFT, SDLK_a }},
                {        static_cast<u8>(CrossPlatformAction::RIGHT),      { SDLK_RIGHT, SDLK_d }},
                {        static_cast<u8>(CrossPlatformAction::CLOSE),             { SDLK_ESCAPE }},
                {static_cast<u8>(CrossPlatformAction::OPEN_SETTINGS),                  { SDLK_p }},
                {          static_cast<u8>(CrossPlatformAction::TAB),                { SDLK_TAB }},
};
#endif


    [[nodiscard]] constexpr Capabilities get_capabilities() {
#if defined(__ANDROID__)
        return Capabilities{ false, true, Orientation::Portrait };
#elif defined(__SWITCH__)
        return Capabilities{ true, false, Orientation::Landscape };
#else
        return Capabilities{ true, true, Orientation::Landscape };
#endif
    }

    [[nodiscard]] constexpr bool device_supports_keys() {
        return get_capabilities().supports_keys;
    }

    [[nodiscard]] constexpr bool device_supports_clicks() {
        return get_capabilities().supports_clicks;
    }

    [[nodiscard]] constexpr Orientation device_orientation() {
        return get_capabilities().orientation;
    }

    [[nodiscard]] std::string built_for_platform();

    [[nodiscard]] bool open_url(const std::string& url);

    [[nodiscard]] bool event_is_action(const SDL_Event& event, CrossPlatformAction action);

    [[nodiscard]] std::vector<i64> get_bound_keys();
    [[nodiscard]] std::vector<i64> get_bound_keys(const std::vector<CrossPlatformAction>& actions);

    [[nodiscard]] std::string action_description(CrossPlatformAction action);

    enum class CrossPlatformClickEvent : u8 { Motion, ButtonDown, ButtonUp, Any };

    [[nodiscard]] bool event_is_click_event(const SDL_Event& event, CrossPlatformClickEvent click_type);

    [[nodiscard]] std::pair<i32, i32> get_raw_coordinates(const Window* window, const SDL_Event& event);

    [[nodiscard]] bool is_event_in(const Window* window, const SDL_Event& event, const shapes::URect& rect);

    [[nodiscard]] SDL_Event offset_event(const Window* window, const SDL_Event& event, const shapes::IPoint& point);

} // namespace utils
