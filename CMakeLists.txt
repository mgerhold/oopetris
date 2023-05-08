cmake_minimum_required(VERSION 3.25)
project(oopetris)

set(CMAKE_CXX_STANDARD 23)

set(TARGET_LIST oopetris)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(CMAKE_HAVE_PTHREAD_H 1)
endif()

find_package(argparse CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(tl-optional CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)
    set(TARGET_LIST oopetris SDL2::SDL2-static)
endif()

add_executable(oopetris
    src/application.cpp
    src/application.hpp
    src/bag.cpp
    src/bag.hpp
    src/clock_source.cpp
    src/clock_source.hpp
    src/color.hpp
    src/command_line_arguments.hpp
    src/constants.hpp
    src/controls.hpp
    src/event_dispatcher.cpp
    src/event_dispatcher.hpp
    src/event_listener.hpp
    src/font.cpp
    src/font.hpp
    src/grid.cpp
    src/grid.hpp
    src/input_event.hpp
    src/input.cpp
    src/input.hpp
    src/key_codes.hpp
    src/magic_enum_wrapper.hpp
    src/main.cpp
    src/mino_stack.cpp
    src/mino_stack.hpp
    src/mino.cpp
    src/mino.hpp
    src/music_manager.cpp
    src/point.hpp
    src/random.cpp
    src/random.hpp
    src/recording.hpp
    src/rect.hpp
    src/renderer.cpp
    src/renderer.hpp
    src/sdl_context.cpp
    src/sdl_context.hpp
    src/settings.hpp
    src/static_string.hpp
    src/tetrion_snapshot.cpp
    src/tetrion_snapshot.hpp
    src/tetrion.cpp
    src/tetrion.hpp
    src/tetris_application.cpp
    src/tetris_application.hpp
    src/tetromino_type.cpp
    src/tetromino_type.hpp
    src/tetromino.hpp
    src/text.cpp
    src/text.hpp
    src/types.hpp
    src/utils.cpp
    src/utils.hpp
    src/window.cpp
    src/window.hpp
)

foreach(target ${TARGET_LIST})
    # set warning levels
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message("MSVC build")

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(${target} INTERFACE /W4 /WX /permissive-)
        else()
            target_compile_options(${target} INTERFACE /W4 /WX /permissive-)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message("GCC build")

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(${target} INTERFACE -Wall -Wextra -Werror -pedantic -Wconversion)
        else()
            target_compile_options(${target} INTERFACE -Wall -Wextra -Werror -pedantic -Wconversion)
        endif()
    endif()

    # define DEBUG_BUILD
    target_compile_definitions(${target} INTERFACE "$<$<CONFIG:DEBUG>:DEBUG_BUILD>")

    # static runtime library
    set_property(TARGET ${target} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endforeach()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_link_libraries(oopetris
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )
    target_link_libraries(oopetris PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
    target_link_libraries(oopetris PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
endif()

target_link_libraries(oopetris PRIVATE argparse::argparse)
target_link_libraries(oopetris PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(oopetris PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(oopetris PRIVATE magic_enum::magic_enum)
target_link_libraries(oopetris PRIVATE tl::optional)
target_link_libraries(oopetris PRIVATE tl::expected)

target_compile_definitions(oopetris PUBLIC AUDIO_WITH_FLAC_SUPPORT AUDIO_WITH_MP3_SUPPORT AUDIO_PREFER_FLAC)
