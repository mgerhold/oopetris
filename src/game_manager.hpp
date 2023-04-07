#pragma once

#include "bag.hpp"
#include "grid.hpp"
#include "input.hpp"
#include "network/online_handler.hpp"
#include "tetromino.hpp"
#include "text.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <tl/optional.hpp>
#include <vector>

struct Application;

enum class GameState {
    Playing,
    GameOver,
};

enum class MovementType {
    Gravity,
    Forced,
};

struct GameManager final {
public:
    static constexpr std::size_t tile_size = 30;
    static constexpr std::size_t legend_size = Grid::preview_extends.x;
    static constexpr std::size_t size_per_field = tile_size * (Grid::width + legend_size);
    static constexpr std::size_t space_between = 125;

private:
    // while holding down, this level is assumed for gravity calculation
    static constexpr int accelerated_drop_movement_level = 10;
    std::size_t m_uuid;
    Grid m_grid;
    tl::optional<Tetromino> m_active_tetromino;
    tl::optional<Tetromino> m_preview_tetromino;
    int m_level = 0;
    double m_next_gravity_step_time;
    int m_lines_cleared = 0;
    GameState m_game_state = GameState::Playing;
    std::array<Bag, 2> m_sequence_bags{ Bag{}, Bag{} };
    int m_sequence_index = 0;
    std::vector<std::shared_ptr<Font>> m_fonts;
    std::vector<Text> m_text_rows;
    int m_score = 0;
    bool m_down_key_pressed = false;
    bool m_is_accelerated_down_movement = false;
    std::unique_ptr<OnlineHandler> m_online_handler = nullptr;
    tl::optional<std::size_t> m_player_num = tl::nullopt;

public:
    GameManager(std::size_t uuid);
    void update();
    void render(const Application& app) const;

    // returns if the input event lead to a movement
    bool handle_input_event(Input::Event event);
    void spawn_next_tetromino();
    bool rotate_tetromino_right();
    bool rotate_tetromino_left();
    bool move_tetromino_down(MovementType movement_type);
    bool move_tetromino_left();
    bool move_tetromino_right();
    bool drop_tetromino();
    void set_online_handler(std::unique_ptr<OnlineHandler> online_handler);
    void set_player_num(std::size_t player_num);

private:
    void refresh_texts();
    void clear_fully_occupied_lines();
    void freeze_active_tetromino();
    [[nodiscard]] bool is_active_tetromino_position_valid() const;
    [[nodiscard]] bool is_valid_mino_position(Point position) const;
    [[nodiscard]] bool is_active_tetromino_completely_visible() const;
    void refresh_preview();
    TetrominoType get_next_tetromino_type();

    [[nodiscard]] double get_gravity_delay() const {
        const double accelerated_gravity_delay_multiplier = (m_is_accelerated_down_movement ? 1.0 / 20.0 : 1.0);
        const int frames =
                (m_level >= static_cast<int>(frames_per_tile.size()) ? frames_per_tile.back() : frames_per_tile[m_level]
                );
        return 1.0 / 60.0 * static_cast<double>(frames) * accelerated_gravity_delay_multiplier;
    }

    static constexpr auto frames_per_tile = std::array<int, 30>{ 48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4,
                                                                 4,  3,  3,  3,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 1 };
};