#pragma once

#include "event_listener.hpp"
#include "input_event.hpp"
#include "random.hpp"
#include "settings.hpp"
#include "types.hpp"
#include <filesystem>
#include <functional>
#include <tl/optional.hpp>
#include <unordered_map>

struct GameManager;

enum class InputCommand {
    MoveLeft,
    MoveRight,
    MoveDown,
    RotateLeft,
    RotateRight,
    Drop,
    Hold,
    ReleaseMoveDown,
};

struct Input {
public:
    using OnEventCallback = std::function<void(InputEvent)>;

private:
    enum class HoldableKey {
        Left,
        Right,
    };

    static constexpr u64 delayed_auto_shift_frames = 10;
    static constexpr u64 auto_repeat_rate_frames = 2;

    std::unordered_map<HoldableKey, u64> m_keys_hold;

protected:
    GameManager* m_target_game_manager;
    OnEventCallback m_on_event_callback;

protected:
    explicit Input(GameManager* target_game_manager) : m_target_game_manager{ target_game_manager } { }
    explicit Input(GameManager* target_game_manager, OnEventCallback on_event_callback)
        : m_target_game_manager{ target_game_manager },
          m_on_event_callback{ std::move(on_event_callback) } { }
    void handle_event(InputEvent event);

public:
    virtual void update();
    virtual ~Input() = default;
};

struct KeyboardInput : public Input, public EventListener {
private:
    KeyboardControls m_controls;

public:
    explicit KeyboardInput(GameManager* target_game_manager, KeyboardControls controls)
        : Input{ target_game_manager },
          m_controls{ controls } { }

    explicit KeyboardInput(
            GameManager* target_game_manager,
            OnEventCallback on_event_callback,
            KeyboardControls controls
    )
        : Input{ target_game_manager, std::move(on_event_callback) },
          m_controls{ controls } { }

    void handle_event(const SDL_Event& event) override;

private:
    [[nodiscard]] tl::optional<InputEvent> sdl_event_to_input_event(const SDL_Event& event) const;
};

struct ReplayInput : public Input {
private:
    RecordingReader m_recording;
    usize m_next_record_index{ 0 };

public:
    ReplayInput(GameManager* target_game_manager, RecordingReader recording);
    ReplayInput(GameManager* target_game_manager, OnEventCallback on_event_callback, RecordingReader recording);

    void update() override;

private:
    [[nodiscard]] bool is_end_of_recording() const {
        return m_next_record_index >= m_recording.num_records();
    }
};
