

#pragma once

#if not defined(__EMSCRIPTEN__)
#error this header is for emscripten only
#endif

#include <memory>
#include <string>
#include <tl/optional.hpp>
// FIXME: this is a temporary fix, since the spdlog upstream just added these 3 months ago, but didn't make a new release!!
#include "spdlog/callback_sink.h"

namespace utils {

    struct LocalStorage {

        [[nodiscard]] static tl::optional<std::string> get_item(const std::string& key);
        static void set_item(const std::string& key, const std::string& value);
        static void remove_item(const std::string& key);
        static void clear();
    };

    [[nodiscard]] std::shared_ptr<spdlog::sinks::callback_sink_mt> get_web_sink();


    namespace console {
        void clear();
        //TODO, these support more arguments, write templates for that
        void error(const std::string& message);
        void warn(const std::string& message);
        void log(const std::string& message);
        void info(const std::string& message);
        void debug(const std::string& message);
        void trace(const std::string& message);
    }; // namespace console


}; // namespace utils