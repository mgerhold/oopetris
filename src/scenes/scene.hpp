#pragma once

#include "../command_line_arguments.hpp"
#include "../event_listener.hpp"
#include "../scene_id.hpp"
#include "../service_provider.hpp"
#include "../settings.hpp"
#include <functional>


namespace scenes {
    enum class SceneUpdate {
        ContinueUpdating,
        StopUpdating,
    };

    struct Scene {
    public:
        struct Switch {
            SceneId target_scene;

            Switch(const SceneId target_scene) : target_scene{ target_scene } { }
        };

        struct Push {
            SceneId target_scene;

            Push(const SceneId target_scene) : target_scene{ target_scene } { }
        };

        struct Pop { };

        struct Exit { };

        using Change = std::variant<Pop, Push, Switch, Exit>;
        using UpdateResult = std::pair<SceneUpdate, tl::optional<Change>>;

    protected:
        ServiceProvider* m_service_provider;

    public:
        explicit Scene(ServiceProvider* service_provider);
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        virtual ~Scene() = default;

        [[nodiscard]] virtual UpdateResult update() = 0;
        virtual void render(const ServiceProvider& service_provider) = 0;
        virtual bool handle_event(const SDL_Event& event) = 0;
    };

    [[nodiscard]] std::unique_ptr<scenes::Scene>
    create_scene(ServiceProvider& service_provider, Window* window, SceneId identifier);
} // namespace scenes
