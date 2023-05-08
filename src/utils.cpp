#include "utils.hpp"
#include "constants.hpp"
#include <SDL.h>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <tl/optional.hpp>

namespace utils {
    [[nodiscard]] std::string current_date_time_iso8601() {
        auto now = std::chrono::system_clock::now();

        std::time_t time = std::chrono::system_clock::to_time_t(now);

        char buffer[16];

        std::tm tm{};
#ifdef _MSC_VER
        if (gmtime_s(&tm, &time) != 0) {
            spdlog::error("error calling gmtime_s");
            return "error";
        }
#else
        if (gmtime_r(&time, &tm) == nullptr) {
            spdlog::error("error calling gmtime_r");
            return "error";
        }
#endif
        std::strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", &tm);

        return std::string{ buffer };
    }

    [[nodiscard]] std::filesystem::path get_root_folder() {
#if defined(__ANDROID__) or defined(BUILD_INSTALLER)
        // this call also creates the dir (at least tries to) it returns
        char* pref_path = SDL_GetPrefPath(constants::author, constants::program_name);
        if (!pref_path) {
            throw std::runtime_error{ "Failed in getting the Pref Path: " + std::string{ SDL_GetError() } };
        }
        return std::filesystem::path{ std::string{ pref_path } };
#elif defined(__EMSCRIPTEN__)
        return std::filesystem::path{ "/" };
#else

        // this is only used in local build for debugging, when compiling in release mode the path is real path where the app can store many things without interfering with other things (eg. AppData\Roaming\... onw Windows or  .local/share/... on Linux )
        return std::filesystem::path{ "." };
#endif
    }

    [[nodiscard]] std::filesystem::path get_assets_folder() {
#if defined(__ANDROID__)
        return std::filesystem::path{ "." };
#elif defined(BUILD_INSTALLER)

#if defined(FLATPAK_BUILD)
        const char* resource_path = "/app/share/oopetris/";
#else
        char* resource_path = SDL_GetPrefPath(constants::author, constants::program_name);
        if (!resource_path) {
            throw std::runtime_error{ "Failed in getting the Pref Path: " + std::string{ SDL_GetError() } };
        }
// if you build in BUILD_INSTALLER mode, you have to assure that the data is there eg. music  + fonts!
#endif
        return std::filesystem::path{ std::string{ resource_path } } / "assets";
#elif defined(__EMSCRIPTEN__)
        return std::filesystem::path{ "/assets" };
#else
        return std::filesystem::path{ "assets" };
#endif
    }

    [[nodiscard]] std::filesystem::path get_subfolder_to_root(const std::string_view folder) {
        return get_root_folder() / folder;
    }


    tl::optional<bool> log_error(const std::string& error) {
        spdlog::error(error);
        return tl::nullopt;
    }

} // namespace utils
