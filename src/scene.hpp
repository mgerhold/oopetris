#pragma once

#include "command_line_arguments.hpp"
#include "event_listener.hpp"
#include "scene_id.hpp"
#include "settings.hpp"
#include <functional>

struct TetrisApplication;
struct Application;

enum class SceneUpdate {
    ContinueUpdating,
    StopUpdating,
};

struct Scene {
public:
    struct Switch {
        SceneId target_scene;
        bool add;

        Switch(const SceneId target_scene, const bool add) : target_scene{ target_scene }, add{ add } { }
    };

    struct Push {
        SceneId target_scene;

        explicit Push(SceneId target_scene) : target_scene{ target_scene } { }
    };

    struct Pop { };

    struct Exit { };

    using Change = std::variant<Push, Pop, Switch, Exit>;
    using UpdateResult = std::pair<SceneUpdate, tl::optional<Change>>;

protected:
    TetrisApplication* m_application;

public:
    explicit Scene(TetrisApplication* app);
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    virtual ~Scene() = default;

    [[nodiscard]] virtual UpdateResult update() = 0;
    virtual void render(const Application& app) = 0;
    virtual bool handle_event(const SDL_Event& event) = 0;
};

[[nodiscard]] std::unique_ptr<Scene> create_scene(TetrisApplication& app, SceneId id);
