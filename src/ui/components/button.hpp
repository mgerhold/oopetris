#pragma once

#include <functional>
#include <spdlog/spdlog.h>
#include <utility>

#include "graphics/rect.hpp"
#include "graphics/renderer.hpp"
#include "helper/color_literals.hpp"
#include "platform/capabilities.hpp"
#include "ui/focusable.hpp"
#include "ui/hoverable.hpp"
#include "ui/widget.hpp"


namespace ui {
    template<typename Content, typename CallBackType>
    struct Button : public Widget, public Focusable, public Hoverable {
    public:
        using Callback = std::function<bool(const CallBackType&)>;

        Content m_content;
        Callback m_callback;
        shapes::URect m_fill_rect;
        bool m_enabled;

        explicit Button(
                Content&& content,
                u32 focus_id,
                Callback callback,
                const shapes::URect& fill_rect,
                const Layout& layout,
                bool is_top_level
        )
            : Widget{ layout, WidgetType::Component, is_top_level },
              Focusable{ focus_id },
              Hoverable{ fill_rect },
              m_content{ std::move(content) },
              m_callback{ std::move(callback) },
              m_fill_rect{ fill_rect },
              m_enabled{ true } {

            // if on top. we give us focus automatically
            if (is_top_level) {
                focus();
            }
        }

        explicit Button(
                Content&& content,
                u32 focus_id,
                Callback callback,
                std::pair<double, double> size,
                Alignment alignment,
                const Layout& layout,
                bool is_top_level
        )
            : Button{ std::move(content),
                      focus_id,
                      std::move(callback),
                      ui::get_rectangle_aligned(
                              layout,
                              { static_cast<u32>(size.first * layout.get_rect().width()),
                                static_cast<u32>(size.second * layout.get_rect().height()) },
                              alignment
                      ),
                      layout,
                      is_top_level } { }


        void render(const ServiceProvider& service_provider) const override {

            //TODO: get as input a color palette and use that!
            const auto color = not m_enabled ? (has_focus() ? "#A36A6A"_c : "#919191"_c)
                                             : (has_focus()    ? is_hovered() ? "#FF6A00"_c : Color::red()
                                                   : is_hovered() ? "#00BBFF"_c
                                                               : Color::blue());
            service_provider.renderer().draw_rect_filled(m_fill_rect, color);

            m_content.render(service_provider);
        }

        helper::BoolWrapper<ui::EventHandleType> handle_event(const SDL_Event& event, const Window* window) override {
            if (not m_enabled) {
                return false;
            }

            if (utils::device_supports_keys()) {
                if (has_focus() and utils::event_is_action(event, utils::CrossPlatformAction::OK)) {
                    spdlog::info("TextButton pressed");
                    if (on_clicked()) {
                        return { true, ui::EventHandleType::RequestAction };
                    }
                    return true;
                }
            }

            if (const auto hover_result = detect_hover(event, window); hover_result) {
                if (hover_result.is(ActionType::Clicked)) {
                    if (on_clicked()) {
                        return { true, ui::EventHandleType::RequestAction };
                    }
                }
                return true;
            }

            return false;
        }

        [[nodiscard]] virtual bool on_clicked() const = 0;

        void disable() {
            m_enabled = false;
        }

        void enable() {
            m_enabled = true;
        }

        [[nodiscard]] bool is_enabled() const {
            return m_enabled;
        }
    };

} // namespace ui
