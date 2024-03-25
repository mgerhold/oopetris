#include "manager/music_manager.hpp"
#include "helper/command_line_arguments.hpp"
#include "helper/constants.hpp"
#include "helper/optional.hpp"
#include "helper/types.hpp"

#include <SDL.h>
#include <SDL_mixer.h>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>


MusicManager::MusicManager(ServiceProvider* service_provider, u8 channel_size)
    : m_music{ nullptr },
      m_queued_music{ nullptr },
      m_channel_size{ channel_size },
      m_chunk_map{ std::unordered_map<std::string, Mix_Chunk*>{} },
      m_service_provider{ service_provider },
      volume{ m_service_provider->command_line_arguments().silent ? helper::nullopt : std::optional{ 1.0F } } {
    if (s_instance != nullptr) {
        spdlog::error("it's not allowed to create more than one MusicManager instance");
        return;
    }

    Mix_Init(MIX_INIT_FLAC | MIX_INIT_MP3);
    const int result = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (result != 0) {
        throw std::runtime_error{ "error on initializing the audio system: " + std::string{ SDL_GetError() } };
    }
    Mix_AllocateChannels(channel_size);
    // 2 here means STEREO, note that channels above means tracks, e.g. simultaneous playing source that are mixed,
    // hence the name SDL2_mixer
    Mix_OpenAudio(constants::audio_frequency, MIX_DEFAULT_FORMAT, 2, constants::audio_chunk_size);

    s_instance = this;

    set_volume(volume, true);
}

MusicManager::~MusicManager() noexcept {
    if (not validate_instance()) {
        return;
    }

    // stop sounds and free loaded data
    Mix_HaltChannel(-1);

    if (m_music != nullptr) {
        Mix_FreeMusic(m_music);
    }

    if (m_queued_music != nullptr) {
        Mix_FreeMusic(m_queued_music);
    }

    for (const auto& [_, value] : m_chunk_map) {
        Mix_FreeChunk(value);
    }
    Mix_CloseAudio();
    Mix_Quit();
}

helper::optional<std::string>
MusicManager::load_and_play_music(const std::filesystem::path& location, const usize delay) {
    if (not validate_instance()) {
        return helper::nullopt;
    }

    Mix_Music* music = Mix_LoadMUS(location.string().c_str());
    if (music == nullptr) {
        return ("an error occurred while trying to load the music '" + location.string()
                + "': " + std::string{ Mix_GetError() });
    }

    // if we are mute, set the current music to this
    if (not volume.has_value()) {
        assert(m_queued_music == nullptr && "No queued music is possible, when muted!");
        if (m_music != nullptr) {
            Mix_FreeMusic(m_music);
        }
        m_music = music;
        return helper::nullopt;
    }


    //handle special case of 0 delay
    if (delay == 0) {

        // if we already have queued a music, free and remove it
        if (m_queued_music != nullptr) {
            Mix_FreeMusic(m_queued_music);
            m_queued_music = nullptr;
        }

        // if there is a music currently playing, stop it and free it
        if (m_music != nullptr) {
            Mix_HaltMusic();
            Mix_FreeMusic(m_music);
        }

        m_music = music;

        const int result = Mix_PlayMusic(music, -1);
        if (result != 0) {
            return ("an error occurred while trying to play the music: " + std::string{ Mix_GetError() });
        }

        return helper::nullopt;
    }


    // if we already have queued a music just queue the new one, this could be a potential race condition in a MT case (even if using atomic!)
    if (m_queued_music != nullptr) {
        Mix_FreeMusic(m_queued_music);
    }


    if (m_music != nullptr) {
        m_queued_music = music;
        m_delay = delay;
        Mix_HookMusicFinished([]() {
            assert(s_instance != nullptr and "there must be a MusicManager instance");
            s_instance->hook_music_finished();
        });

        // this wasn't block, so we have to wait for the callback to be called
        const int result = Mix_FadeOutMusic(static_cast<int>(delay));
        if (result == 0) {
            return "UNREACHABLE: m_music was not null but not playing, this is an implementation error!";
        }
        return helper::nullopt;
    }

    const int result = Mix_PlayMusic(music, -1);
    if (result != 0) {
        return ("an error occurred while trying to play the music: " + std::string{ Mix_GetError() });
    }

    return helper::nullopt;
}


helper::optional<std::string> MusicManager::load_effect(const std::string& name, std::filesystem::path& location) {
    if (not validate_instance()) {
        return helper::nullopt;
    }

    if (m_chunk_map.contains(name)) {
        return "name already used";
    }

    Mix_Chunk* chunk = Mix_LoadWAV(location.string().c_str());
    if (chunk == nullptr) {
        return ("an error occurred while trying to load the chunk: " + std::string{ Mix_GetError() });
    }

    m_chunk_map.insert({ name, chunk });
    return helper::nullopt;
}

helper::optional<std::string> MusicManager::play_effect(const std::string& name, u8 channel_num, int loop) {
    if (not validate_instance()) {
        return helper::nullopt;
    }

    if (m_chunk_map.contains(name)) {
        return "name not loaded";
    }

    if (channel_num <= 0 or channel_num >= m_channel_size) {
        return "invalid channel: " + std::to_string(channel_num);
    }

    auto* const chunk = m_chunk_map.at(name);

    const auto actual_channel = Mix_PlayChannel(channel_num, chunk, loop);
    if (actual_channel == -1) {
        return "couldn't play on channel: " + std::to_string(channel_num);
    }

    return helper::nullopt;
}

void MusicManager::hook_music_finished() {
    if (not validate_instance()) {
        return;
    }

    if (m_queued_music == nullptr) {
        throw std::runtime_error{ "implementation error: m_queued_music is null but it shouldn't be" };
    }

    if (m_music != nullptr) {
        Mix_FreeMusic(m_music);
    }

    const int result = Mix_FadeInMusic(m_queued_music, -1, static_cast<int>(m_delay));
    if (result != 0) {
        throw std::runtime_error(
                "an error occurred while trying to play the music (fading in): " + std::string{ Mix_GetError() }
        );
    }
    m_music = m_queued_music;

    m_queued_music = nullptr;

    // clear the callback
    Mix_HookMusicFinished(nullptr);
}

[[nodiscard]] bool MusicManager::validate_instance() {
    if (s_instance != this) {
        spdlog::error("this MusicManager instance is not the instance that is used globally");
        return false;
    }
    return true;
}


[[nodiscard]] helper::optional<double> MusicManager::get_volume() const {
#ifdef DEBUG_BUILD
    int result = Mix_VolumeMusic(-1);
    if (result == 0) {
        return helper::nullopt;
    }

    return static_cast<double>(result) / MIX_MAX_VOLUME;
#else
    return volume;
#endif
}

void MusicManager::set_volume(
        const helper::optional<double> new_volume,
        const bool force_update,
        const bool notify_listeners
) {

    if (volume == new_volume and not force_update) {
        return;
    }

    if (not new_volume.has_value()) {

        if (m_music != nullptr) {
            Mix_HaltMusic();
        }

        volume = helper::nullopt;
    }


    const int new_volume_mapped =
            not new_volume.has_value() ? 0 : static_cast<int>(MIX_MAX_VOLUME * new_volume.value());
    Mix_VolumeMusic(new_volume_mapped);

    if (not volume.has_value()) {

        if (m_music != nullptr) {
            const int result = Mix_PlayMusic(m_music, -1);
            if (result != 0) {
                throw std::runtime_error(
                        "an error occurred while trying to play the music: " + std::string{ Mix_GetError() }
                );
            }
        }
    }


    volume = new_volume;
    if (notify_listeners) {
        for (const auto& [_, listener] : volume_listeners) {
            listener(volume);
        }
    }
}

helper::optional<double> MusicManager::change_volume(const std::int8_t steps) {
    const auto current_volume = get_volume();

    if (steps == 0) {
        return current_volume;
    }

    helper::optional<double> new_volume = current_volume;

    if (steps > 0) {

        if (not current_volume.has_value()) {
            new_volume = MusicManager::step_width * static_cast<double>(steps);

        } else {
            if (current_volume >= 1.0F) {
                return 1.0F;
            }

            new_volume = current_volume.value() + MusicManager::step_width * static_cast<double>(steps);
        }

        if (new_volume >= 1.0F) {
            new_volume = 1.0F;
        }


    } else {
        // steps < 0


        if (not current_volume.has_value()) {
            return helper::nullopt;
        }

        if (current_volume <= 0.0F) {
            new_volume = helper::nullopt;
        } else {

            new_volume = current_volume.value() + MusicManager::step_width * static_cast<double>(steps);


            if (new_volume <= 0.0F) {
                new_volume = helper::nullopt;
            }
        }
    }


    set_volume(new_volume);

    return new_volume;
}

bool MusicManager::handle_event(const SDL_Event& event) {


    if (utils::device_supports_keys() && event.type == SDL_KEYDOWN) {

        if (event.key.keysym.sym == SDLK_PLUS or event.key.keysym.sym == SDLK_KP_PLUS) {
            change_volume(1);
            return true;
        }

        if (event.key.keysym.sym == SDLK_MINUS or event.key.keysym.sym == SDLK_KP_MINUS) {
            change_volume(-1);
            return true;
        }
    }

    return false;
}
