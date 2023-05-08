#pragma once

#include "application.hpp"
#include "music_manager.hpp"
#include "recording.hpp"
#include "settings.hpp"
#include "tetrion.hpp"
#include "tetromino_type.hpp"
#include <cassert>
#include <fstream>
#include <span>
#include <spdlog/spdlog.h>
#include <stdexcept>

struct TetrisApplication : public Application {
private:
    using TetrionHeaders = std::vector<Recording::TetrionHeader>;

#if defined(__EMSCRIPTEN__)
    static constexpr auto settings_filename = "settings_key";
#else
    static constexpr auto settings_filename = "settings.json";
#endif
    std::vector<std::unique_ptr<ClockSource>> m_clock_sources;
    std::vector<SimulationStep> m_simulation_step_indices;
    std::vector<std::unique_ptr<Tetrion>> m_tetrions;
    std::vector<std::unique_ptr<Input>> m_inputs;
    Settings m_settings;
    std::unique_ptr<RecordingWriter> m_recording_writer;
    std::unique_ptr<RecordingReader> m_recording_reader;

public:
    static constexpr int width = 1280;
    static constexpr int height = 720;

    explicit TetrisApplication(CommandLineArguments command_line_arguments);

protected:
    void update() override;
    void render() const override;

private:
    [[nodiscard]] std::unique_ptr<Input>
    create_input(Controls controls, Tetrion* associated_tetrion, Input::OnEventCallback on_event_callback);

    [[nodiscard]] std::unique_ptr<Input> create_replay_input(
            RecordingReader* recording_reader,
            Tetrion* associated_tetrion,
            Input::OnEventCallback on_event_callback
    );

    [[nodiscard]] Input::OnEventCallback create_on_event_callback(int tetrion_index);

    void try_load_settings();

    [[nodiscard]] bool is_replay_mode() const;

    [[nodiscard]] bool game_is_recorded() const;

    [[nodiscard]] Random::Seed seed_for_tetrion(u8 tetrion_index, Random::Seed common_seed) const;

    [[nodiscard]] auto starting_level_for_tetrion(u8 tetrion_index) const
            -> decltype(CommandLineArguments::starting_level);

    [[nodiscard]] TetrionHeaders create_tetrion_headers(std::span<const Random::Seed> seeds) const;

    [[nodiscard]] static std::unique_ptr<RecordingWriter> create_recording_writer(TetrionHeaders tetrion_headers);

    [[nodiscard]] std::vector<Random::Seed> create_seeds(u8 num_tetrions) const;

    [[nodiscard]] tl::optional<RecordingWriter*> recording_writer_optional();
};
