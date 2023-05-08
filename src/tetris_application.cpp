#include "tetris_application.hpp"
#include "utils.hpp"
#include <tl/optional.hpp>

#if defined(__EMSCRIPTEN__)
#include "web_utils.hpp"
#endif

TetrisApplication::TetrisApplication(CommandLineArguments command_line_arguments)
#if defined(__ANDROID__)
    : Application{ "OOPetris", WindowPosition::Centered, std::move(command_line_arguments) } {
#else
    : Application{ "OOPetris", WindowPosition::Centered, width, height, std::move(command_line_arguments) } {
#endif

    try_load_settings();
    static constexpr auto num_tetrions = u8{ 1 };

    if (is_replay_mode()) {
        m_recording_reader = std::make_unique<RecordingReader>(*(this->command_line_arguments().recording_path));
    }

    const auto seeds = create_seeds(num_tetrions);

    if (game_is_recorded()) {
        const auto seeds_span = std::span{ seeds.data(), seeds.size() };
        m_recording_writer = create_recording_writer(create_tetrion_headers(seeds_span));
    }
    // first instantiation of instance initializes value!
    MusicManager::getInstance(2);

    //TODO: add in the main menu after we have an UI
    /*  MusicManager::getInstance().load_and_play_music(utils::get_assets_folder() / "music" / utils::get_supported_music_extension("01. Main Menu"))
            .and_then(utils::log_error); */

    for (u8 tetrion_index = 0; tetrion_index < num_tetrions; ++tetrion_index) {
        m_clock_sources.push_back(
                std::make_unique<LocalClock>(static_cast<u32>(this->command_line_arguments().target_fps))
        );
        m_simulation_step_indices.push_back(0);
        const auto starting_level = starting_level_for_tetrion(tetrion_index);
        spdlog::info("starting level for tetrion {}: {}", tetrion_index, starting_level);

        m_tetrions.push_back(std::make_unique<Tetrion>(
                tetrion_index, seeds.at(tetrion_index), starting_level, recording_writer_optional()
        ));

        auto on_event_callback = create_on_event_callback(tetrion_index);

        const auto tetrion_pointer = m_tetrions.back().get();
        if (is_replay_mode()) {
            m_inputs.push_back(create_replay_input(m_recording_reader.get(), tetrion_pointer, [](InputEvent) {}));
        } else {
            m_inputs.push_back(
                    create_input(m_settings.controls.at(tetrion_index), tetrion_pointer, std::move(on_event_callback))
            );
        }
    }
    for (const auto& tetrion : m_tetrions) {
        tetrion->spawn_next_tetromino(0);
    }

    MusicManager::getInstance()
            .load_and_play_music(
                    utils::get_assets_folder() / "music" / utils::get_supported_music_extension("02. Game Theme")
            )
            .and_then(utils::log_error);
}

void TetrisApplication::update() {
    assert(m_inputs.size() == m_tetrions.size());
    assert(m_inputs.size() == m_simulation_step_indices.size());

    for (usize i = 0; i < m_tetrions.size(); ++i) {
        auto& tetrion = *m_tetrions.at(i);
        auto& input = *m_inputs.at(i);

        auto& simulation_step_index = m_simulation_step_indices.at(i);
        while (simulation_step_index < m_clock_sources.at(i)->simulation_step_index()) {
            ++simulation_step_index;
            input.update(simulation_step_index);
            tetrion.update(simulation_step_index);
            input.late_update(simulation_step_index);
        }
    }
}

void TetrisApplication::render() const {
    Application::render(); // call parent function to clear the screen
    for (const auto& tetrion : m_tetrions) {
        tetrion->render(*this);
    }
}

[[nodiscard]] std::unique_ptr<Input> TetrisApplication::create_input(
        Controls controls,
        Tetrion* associated_tetrion,
        Input::OnEventCallback on_event_callback
) {
    return std::visit(
            overloaded{
                    [&]([[maybe_unused]] KeyboardControls& keyboard_controls) -> std::unique_ptr<Input> {
#if defined(__ANDROID__)
                        auto input = std::make_unique<TouchInput>(associated_tetrion, std::move(on_event_callback));
#else
                        auto input = std::make_unique<KeyboardInput>(
                                associated_tetrion, keyboard_controls, std::move(on_event_callback)
                        );
#endif
                        m_event_dispatcher.register_listener(input.get());
                        return input;
                    },
            },
            controls
    );
}

[[nodiscard]] std::unique_ptr<Input> TetrisApplication::create_replay_input(
        RecordingReader* const recording_reader,
        Tetrion* const associated_tetrion,
        Input::OnEventCallback on_event_callback
) {
    return std::make_unique<ReplayInput>(associated_tetrion, std::move(on_event_callback), recording_reader);
}

[[nodiscard]] Input::OnEventCallback TetrisApplication::create_on_event_callback(const int tetrion_index) {
    if (m_recording_writer) {
        return [tetrion_index, this](InputEvent event) {
            spdlog::debug(
                    "event: {} (step {})", magic_enum::enum_name(event),
                    m_clock_sources.at(tetrion_index)->simulation_step_index()
            );
            m_recording_writer->add_event(
                    static_cast<u8>(tetrion_index), m_clock_sources.at(tetrion_index)->simulation_step_index(), event
            );
        };
    } else {
        return Input::OnEventCallback{}; // empty std::function object
    }
}

void TetrisApplication::try_load_settings() {
    try {
#if defined(__EMSCRIPTEN__)
        const auto settings_string = utils::LocalStorage::get_item(settings_filename);
        if (not settings_string.has_value()) {
            return;
        }
        m_settings = nlohmann::json::parse(settings_string.value());
#else
        m_settings = nlohmann::json::parse(settings_filename);
#endif
        spdlog::info("settings loaded");
    } catch (...) {
        spdlog::error("unable to load settings from \"{}\"", settings_filename);
        spdlog::warn("applying default settings");
    }
}

[[nodiscard]] bool TetrisApplication::is_replay_mode() const {
    return this->command_line_arguments().recording_path.has_value();
}

[[nodiscard]] bool TetrisApplication::game_is_recorded() const {
    return not is_replay_mode();
}

[[nodiscard]] Random::Seed TetrisApplication::seed_for_tetrion(const u8 tetrion_index, const Random::Seed common_seed)
        const {
    return (is_replay_mode() ? m_recording_reader->tetrion_headers().at(tetrion_index).seed : common_seed);
}

[[nodiscard]] auto TetrisApplication::starting_level_for_tetrion(const u8 tetrion_index) const
        -> decltype(CommandLineArguments::starting_level) {
    return is_replay_mode() ? m_recording_reader->tetrion_headers().at(tetrion_index).starting_level
                            : this->command_line_arguments().starting_level;
}

[[nodiscard]] TetrisApplication::TetrionHeaders TetrisApplication::create_tetrion_headers(
        const std::span<const Random::Seed> seeds
) const {
    const auto num_tetrions = seeds.size();
    auto headers = TetrionHeaders{};
    headers.reserve(num_tetrions);
    for (u8 tetrion_index = 0; tetrion_index < num_tetrions; ++tetrion_index) {
        assert(tetrion_index < seeds.size());
        const auto tetrion_seed = seeds[tetrion_index];
        const auto starting_level = starting_level_for_tetrion(tetrion_index);
        headers.push_back(Recording::TetrionHeader{ .seed = tetrion_seed, .starting_level = starting_level });
    }
    return headers;
}

[[nodiscard]] std::unique_ptr<RecordingWriter> TetrisApplication::create_recording_writer(TetrionHeaders tetrion_headers
) {

    static constexpr auto recordings_directory = "recordings";
    const auto recording_directory_path = utils::get_root_folder() / recordings_directory;

    if (not std::filesystem::exists(recording_directory_path)) {
        std::filesystem::create_directory(recording_directory_path);
    }
    const auto filename = fmt::format("{}.rec", utils::current_date_time_iso8601());
    const auto file_path = recording_directory_path / filename;

    return std::make_unique<RecordingWriter>(file_path, std::move(tetrion_headers));
}

[[nodiscard]] std::vector<Random::Seed> TetrisApplication::create_seeds(const u8 num_tetrions) const {
    auto seeds = std::vector<Random::Seed>{};
    seeds.reserve(num_tetrions);
    const auto common_seed = Random::generate_seed();
    for (u8 tetrion_index = 0; tetrion_index < num_tetrions; ++tetrion_index) {
        const auto seed = seed_for_tetrion(tetrion_index, common_seed);
        spdlog::info("seed for tetrion {}: {}", tetrion_index, seed);
        seeds.push_back(seed);
    }
    return seeds;
}

[[nodiscard]] tl::optional<RecordingWriter*> TetrisApplication::recording_writer_optional() {
    if (m_recording_writer) {
        return m_recording_writer.get();
    }
    return tl::nullopt;
}
