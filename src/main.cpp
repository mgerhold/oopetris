#include "command_line_arguments.hpp"
#include "tetris_application.hpp"
#include "utils.hpp"
#include <filesystem>
#if defined(__ANDROID__)
#include <spdlog/sinks/android_sink.h>
#elif defined(__EMSCRIPTEN__)
#include "web_utils.hpp"
#endif
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

int main(int argc, char** argv) {
    const auto logs_path = utils::get_root_folder() / "logs";
    if (not exists(logs_path)) {
        std::filesystem::create_directory(logs_path);
    }

    std::vector<spdlog::sink_ptr> sinks;
#if defined(__ANDROID__)
#define USE_FILE_LOG
    sinks.push_back(std::make_shared<spdlog::sinks::android_sink_mt>());
#elif defined(__EMSCRIPTEN__)
    sinks.push_back(utils::get_web_sink());
#else
#define USE_FILE_LOG
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
#endif


#if defined(USE_FILE_LOG)
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            fmt::format("{}/oopetris.log", logs_path.string()), 1024 * 1024 * 10, 5, true
    ));
#endif

    auto combined_logger = std::make_shared<spdlog::logger>("combined_logger", begin(sinks), end(sinks));
    spdlog::set_default_logger(combined_logger);

#ifdef DEBUG_BUILD
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::err);
#endif

    TetrisApplication tetris_app(CommandLineArguments{ argc, argv });
    tetris_app.run();

    return 0;
}
