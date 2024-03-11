
#include "game.hpp"
#include "platform/replay_input.hpp"

Game::Game(
        ServiceProvider* const service_provider,
        std::unique_ptr<Input>&& input,
        const tetrion::StartingParameters& starting_parameters,
        const ui::Layout& layout,
        bool is_top_level
)
    : ui::Widget{ layout, ui::WidgetType::Component, is_top_level },
      m_clock_source{ std::make_unique<LocalClock>(starting_parameters.target_fps) },
      m_input{ std::move(input) } {


    spdlog::info("starting level for tetrion {}", starting_parameters.starting_level);

    m_tetrion = std::make_unique<Tetrion>(
            starting_parameters.tetrion_index, starting_parameters.seed, starting_parameters.starting_level,
            service_provider, starting_parameters.recording_writer, layout, false
    );

    m_tetrion->spawn_next_tetromino(0);

    m_input->set_target_tetrion(m_tetrion.get());
    if (starting_parameters.recording_writer.has_value()) {
        const auto recording_writer = starting_parameters.recording_writer.value();
        const auto tetrion_index = starting_parameters.tetrion_index;
        m_input->set_event_callback([recording_writer,
                                     tetrion_index](InputEvent event, SimulationStep simulation_step_index) {
            spdlog::debug("event: {} (step {})", magic_enum::enum_name(event), simulation_step_index);

            recording_writer->add_event(tetrion_index, simulation_step_index, event);
        });
    }
}

void Game::update() {
    if (is_game_finished()) {
        return;
    }

    if (m_is_paused) {
        // if we would still be in pause mode, update() wouldn't have been called in the first place => we
        // must resume from pause
        set_paused(false);
    }

    while (m_simulation_step_index < m_clock_source->simulation_step_index()) {
        ++m_simulation_step_index;
        m_input->update(m_simulation_step_index);
        m_tetrion->update_step(m_simulation_step_index);
        m_input->late_update(m_simulation_step_index);
    }
}

void Game::render(const ServiceProvider& service_provider) const {
    m_tetrion->render(service_provider);
}

[[nodiscard]] helper::BoolWrapper<ui::EventHandleType> Game::handle_event(const SDL_Event&, const Window*) {
    return false;
}

void Game::set_paused(bool paused) {
    m_is_paused = paused;

    assert(m_clock_source->can_be_paused());
    if (paused) {
        m_clock_source->pause();
    } else {
        m_clock_source->resume();
    }

    auto* listener = dynamic_cast<EventListener*>(m_input.get());

    if (listener != nullptr) {
        listener->set_paused(paused);
    }
}

[[nodiscard]] bool Game::is_paused() const {
    return m_is_paused;
}


[[nodiscard]] bool Game::is_game_finished() const {
    if (m_tetrion->is_game_over()) {
        return true;
    };

    auto* const input_as_replay = dynamic_cast<ReplayInput*>(m_input.get());
    if (input_as_replay != nullptr) {
        return input_as_replay->is_end_of_recording();
    }

    return false;
}
