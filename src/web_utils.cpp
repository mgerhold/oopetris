
#if defined(__EMSCRIPTEN__)
#include "web_utils.hpp"
#include <emscripten/val.h>
#include <memory>
// FIXME: this is a temporary fix, since the spdlog upstream just added these 3 months ago, but didn't make a new release!!
#include "spdlog/callback_sink.h"
#include <string>
#include <tl/optional.hpp>

tl::optional<std::string> utils::LocalStorage::get_item(const std::string& key) {

    thread_local const emscripten::val localStorage = emscripten::val::global("localStorage");

    emscripten::val value = localStorage.call<emscripten::val>("getItem", emscripten::val{ key });

    if (value.isNull()) {
        return tl::nullopt;
    }

    return value.as<std::string>();
}

void utils::LocalStorage::set_item(const std::string& key, const std::string& value) {

    thread_local const emscripten::val localStorage = emscripten::val::global("localStorage");

    localStorage.call<void>("setItem", emscripten::val{ key }, emscripten::val{ value });
}

void utils::LocalStorage::remove_item(const std::string& key) {

    thread_local const emscripten::val localStorage = emscripten::val::global("localStorage");

    localStorage.call<void>("removeItem", emscripten::val{ key });
}

void utils::LocalStorage::clear() {

    thread_local const emscripten::val localStorage = emscripten::val::global("localStorage");

    localStorage.call<void>("clear");
}


void utils::console::error(const std::string& message) {
    thread_local const emscripten::val console = emscripten::val::global("console");
    console.call<void>("error", emscripten::val{ message });
}

void utils::console::warn(const std::string& message) {
    thread_local const emscripten::val console = emscripten::val::global("console");
    console.call<void>("warn", emscripten::val{ message });
}

void utils::console::log(const std::string& message) {
    thread_local const emscripten::val console = emscripten::val::global("console");
    console.call<void>("log", emscripten::val{ message });
}

void utils::console::info(const std::string& message) {
    thread_local const emscripten::val console = emscripten::val::global("console");
    console.call<void>("info", emscripten::val{ message });
}

void utils::console::debug(const std::string& message) {
    thread_local const emscripten::val console = emscripten::val::global("console");
    console.call<void>("debug", emscripten::val{ message });
}

void utils::console::trace(const std::string& message) {
    thread_local const emscripten::val console = emscripten::val::global("console");
    console.call<void>("trace", emscripten::val{ message });
}

void utils::console::clear() {
    thread_local const emscripten::val console = emscripten::val::global("console");
    console.call<void>("clear");
}

std::shared_ptr<spdlog::sinks::callback_sink_mt> utils::get_web_sink() {
    return std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg) {
        const auto message = std::string{ msg.payload.begin(), msg.payload.end() };

        switch (msg.level) {
            case spdlog::level::off:
                return;
            case spdlog::level::trace:
                utils::console::trace(message);
                break;
            case spdlog::level::debug:
                utils::console::debug(message);
                break;
            case spdlog::level::info:
                utils::console::info(message);
                break;
            case spdlog::level::warn:
                utils::console::warn(message);
                break;
            case spdlog::level::err:
            case spdlog::level::critical:
                utils::console::error(message);
                break;
            default:
                return;
        }
    });
}


#endif