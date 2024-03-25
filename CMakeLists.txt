cmake_minimum_required(VERSION 3.25)
project(oopetris)

set(CMAKE_CXX_STANDARD 23)

find_package(argparse CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED COMPONENTS OpenSSL ZLIB)
find_package(utf8cpp CONFIG REQUIRED)

set(TARGET_LIST oopetris SDL2::SDL2-static)

add_executable(oopetris
    src/application.cpp
    src/application.hpp
    src/game/bag.cpp
    src/game/bag.hpp
    src/game/game.cpp
    src/game/game.hpp
    src/game/grid.cpp
    src/game/grid.hpp
    src/game/input_creator.cpp
    src/game/input_creator.hpp
    src/game/mino_stack.cpp
    src/game/mino_stack.hpp
    src/game/mino.cpp
    src/game/mino.hpp
    src/game/rotation.cpp
    src/game/rotation.hpp
    src/game/tetrion.cpp
    src/game/tetrion.hpp
    src/game/tetromino_type.cpp
    src/game/tetromino_type.hpp
    src/game/tetromino.cpp
    src/game/tetromino.hpp
    src/graphics/point.hpp
    src/graphics/rect.hpp
    src/graphics/renderer.cpp
    src/graphics/renderer.hpp
    src/graphics/sdl_context.cpp
    src/graphics/sdl_context.hpp
    src/graphics/text.cpp
    src/graphics/text.hpp
    src/graphics/texture.cpp
    src/graphics/texture.hpp
    src/graphics/window.cpp
    src/graphics/window.hpp
    src/helper/bool_wrapper.hpp
    src/helper/clock_source.cpp
    src/helper/clock_source.hpp
    src/helper/color.cpp
    src/helper/color.hpp
    src/helper/command_line_arguments.hpp
    src/helper/constants.hpp
    src/helper/date.cpp
    src/helper/expected.hpp
    src/helper/git_helper.hpp
    src/helper/magic_enum_wrapper.hpp
    src/helper/optional.hpp
    src/helper/parse_json.cpp
    src/helper/parse_json.hpp
    src/helper/random.cpp
    src/helper/random.hpp
    src/helper/sleep.cpp
    src/helper/static_string.hpp
    src/helper/timer.hpp
    src/helper/types.hpp
    src/helper/utils.cpp
    src/helper/utils.hpp
    src/lobby/api.hpp
    src/lobby/types.hpp
    src/main.cpp
    src/manager/font.cpp
    src/manager/font.hpp
    src/manager/music_manager.cpp
    src/manager/music_manager.hpp
    src/manager/recording/additional_information.cpp
    src/manager/recording/checksum_helper.cpp
    src/manager/recording/recording_reader.cpp
    src/manager/recording/recording_reader.hpp
    src/manager/recording/recording_writer.cpp
    src/manager/recording/recording_writer.hpp
    src/manager/recording/recording.cpp
    src/manager/recording/recording.hpp
    src/manager/recording/tetrion_snapshot.cpp
    src/manager/recording/tetrion_snapshot.hpp
    src/manager/resource_manager.hpp
    src/manager/service_provider.hpp
    src/manager/settings.hpp
    src/platform/capabilities.cpp
    src/platform/capabilities.hpp
    src/platform/input.cpp
    src/platform/keyboard_input.cpp
    src/platform/replay_input.cpp
    src/platform/switch_buttons.hpp
    src/scenes/about_page/about_page.cpp
    src/scenes/about_page/about_page.hpp
    src/scenes/game_over/game_over.cpp
    src/scenes/game_over/game_over.hpp
    src/scenes/main_menu/main_menu.cpp
    src/scenes/main_menu/main_menu.hpp
    src/scenes/multiplayer_menu/multiplayer_menu.cpp
    src/scenes/multiplayer_menu/multiplayer_menu.hpp
    src/scenes/online_lobby/online_lobby.cpp
    src/scenes/online_lobby/online_lobby.hpp
    src/scenes/pause/pause.cpp
    src/scenes/pause/pause.hpp
    src/scenes/play_select_menu/play_select_menu.cpp
    src/scenes/play_select_menu/play_select_menu.hpp
    src/scenes/recording_selector/recording_component.cpp
    src/scenes/recording_selector/recording_selector.cpp
    src/scenes/replay_game/replay_game.cpp
    src/scenes/scene_id.hpp
    src/scenes/scene.cpp
    src/scenes/scene.hpp
    src/scenes/settings_menu/settings_menu.cpp
    src/scenes/settings_menu/settings_menu.hpp
    src/scenes/single_player_game/single_player_game.cpp
    src/scenes/single_player_game/single_player_game.hpp
    src/thirdparty/hash-library/sha256.cpp
    src/ui/components/button.hpp
    src/ui/components/image_view.cpp
    src/ui/components/image_view.hpp
    src/ui/components/label.cpp
    src/ui/components/label.hpp
    src/ui/components/link_label.cpp
    src/ui/components/link_label.hpp
    src/ui/components/slider.cpp
    src/ui/components/slider.hpp
    src/ui/components/text_button.cpp
    src/ui/components/textinput.cpp
    src/ui/components/textinput.hpp
    src/ui/focusable.hpp
    src/ui/hoverable.hpp
    src/ui/layout.cpp
    src/ui/layout.cpp
    src/ui/layout.cpp
    src/ui/layout.hpp
    src/ui/layouts/focus_layout.cpp
    src/ui/layouts/grid_layout.cpp
    src/ui/layouts/grid_layout.hpp
    src/ui/layouts/scroll_layout.cpp
    src/ui/layouts/scroll_layout.hpp
    src/ui/layouts/tile_layout.cpp
    src/ui/layouts/tile_layout.hpp
    src/ui/widget.cpp
    src/ui/widget.hpp
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

target_link_libraries(oopetris
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_link_libraries(oopetris PRIVATE argparse::argparse)
target_link_libraries(oopetris PRIVATE fmt::fmt)
target_link_libraries(oopetris PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(oopetris PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(oopetris PRIVATE magic_enum::magic_enum)
target_link_libraries(oopetris PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
target_link_libraries(oopetris PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(oopetris PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
target_link_libraries(oopetris PRIVATE httplib::httplib)
target_link_libraries(oopetris PRIVATE utf8cpp::utf8cpp)

INCLUDE(CheckCXXSourceCompiles)

# check std::expected support
CHECK_CXX_SOURCE_COMPILES(
    "
#include <expected>
int main() {
    using T = std::expected<int, char>;
    return 0;
}
" HAVE_STD_EXPECTED)

if(HAVE_STD_EXPECTED)
else()
    find_package(tl-expected CONFIG REQUIRED)
    target_link_libraries(oopetris PRIVATE tl::expected)
    target_compile_definitions(oopetris PRIVATE "_USE_TL_EXPECTED")
    message("Falling back to tl::expected")
endif()

# check std::optional support
CHECK_CXX_SOURCE_COMPILES(
    "
#include <optional>
int main() {
    using T = std::optional<int>;
    return 0;
}
" HAVE_STD_OPTIONAL)

if(HAVE_STD_OPTIONAL)
else()
    find_package(tl-optional CONFIG REQUIRED)
    target_link_libraries(oopetris PRIVATE tl::optional)
    target_compile_definitions(oopetris PRIVATE "_USE_TL_OPTIONAL")
    message("Falling back to tl::optional")
endif()

target_compile_definitions(oopetris PUBLIC AUDIO_WITH_FLAC_SUPPORT AUDIO_WITH_MP3_SUPPORT AUDIO_PREFER_FLAC)

target_include_directories(oopetris PRIVATE "src/")
target_include_directories(oopetris PRIVATE "src/thirdparty/hash-library/")

# static runtime library
set_property(TARGET ${target} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
