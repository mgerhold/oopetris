#pragma once

#include <algorithm>
#include <memory>
#include <tl/optional.hpp>
#include <vector>

#include "../capabilities.hpp"
#include "../rect.hpp"
#include "../types.hpp"
#include "focusable.hpp"
#include "widget.hpp"

namespace ui {

    struct FocusGroup : public Widget {
    private:
        enum class FocusChangeDirection {
            Forward,
            Backward,
        };

        std::vector<std::unique_ptr<Widget>> m_widgets;
        tl::optional<usize> m_focus_id;

    public:
        explicit FocusGroup(const Layout& layout) : Widget(layout) { }

        void render(const ServiceProvider& service_provider, Rect rect) const override {
            for (const auto& widget : m_widgets) {
                widget->render(service_provider, rect);
            }
        }

        bool handle_event(const SDL_Event& event) override {
            auto handled = false;
            //TODO: focus is not representable in touch devices, so what to do?

            // TODO support hover over via checking if it's hovered over a subwidget (how to deal with overlaps??) each subwidgets needs to return either a spanning rectangle or a method to check if a point is within it

            if (utils::device_supports_keys()) {
                if (utils::event_is_action(event, utils::CrossPlatformAction::DOWN)) {
                    handled = try_set_next_focus(FocusChangeDirection::Forward);
                } else if (utils::event_is_action(event, utils::CrossPlatformAction::UP)) {
                    handled = try_set_next_focus(FocusChangeDirection::Backward);
                }
            }

            if (handled) {
                return true;
            }

            for (auto& widget : m_widgets) {
                if (widget->handle_event(event)) {
                    return true;
                }
            }
            return false;
        }

        void add(std::unique_ptr<Widget> widget) {
            m_widgets.push_back(std::move(widget));
            auto focusable = as_focusable(*m_widgets.back());
            if (focusable.has_value() and not m_focus_id.has_value()) {
                give_focus(focusable.value());
            }
        }

    private:
        void give_focus(Focusable& focusable) {
            m_focus_id = focusable.focus_id();
            focusable.focus();
        }

        [[nodiscard]] static tl::optional<Focusable&> as_focusable(Widget& widget) {
            const auto focusable = dynamic_cast<Focusable*>(&widget);
            if (focusable == nullptr) {
                return tl::nullopt;
            }
            return *focusable;
        }

        [[nodiscard]] usize focusable_index_by_id(const usize id) {
            const auto find_iterator =
                    std::find_if(m_widgets.begin(), m_widgets.end(), [id](const std::unique_ptr<Widget>& widget) {
                        const auto focusable = as_focusable(*widget);
                        return focusable.has_value() and focusable.value().focus_id() == id;
                    });
            assert(find_iterator != m_widgets.end());
            const auto index = static_cast<usize>(std::distance(m_widgets.begin(), find_iterator));
            return index;
        }

        [[nodiscard]] std::vector<usize> focusable_ids_sorted() const {
            auto result = std::vector<usize>{};
            for (const auto& widget : m_widgets) {
                const auto focusable = as_focusable(*widget);
                if (focusable) {
                    result.push_back(focusable->focus_id());
                }
            }
            std::sort(result.begin(), result.end());
            return result;
        }

        [[nodiscard]] static usize index_of(const std::vector<usize>& ids, const usize needle) {
            return static_cast<usize>(std::distance(ids.cbegin(), std::find(ids.cbegin(), ids.cend(), needle)));
        }

        [[nodiscard]] bool try_set_next_focus(const FocusChangeDirection direction) {
            if (not m_focus_id.has_value()) {
                return false;
            }

            const auto focusable_ids = focusable_ids_sorted();
            assert(not focusable_ids.empty());
            const auto current_index = index_of(focusable_ids, m_focus_id.value());
            const auto next_index =
                    (direction == FocusChangeDirection::Forward
                             ? ((current_index + 1) % focusable_ids.size())
                             : ((current_index + focusable_ids.size() - 1) % focusable_ids.size()));

            auto current_focusable =
                    as_focusable(*m_widgets.at(focusable_index_by_id(focusable_ids.at(current_index))));
            assert(current_focusable.has_value());
            auto next_focusable = as_focusable(*m_widgets.at(focusable_index_by_id(focusable_ids.at(next_index))));
            assert(next_focusable.has_value());
            current_focusable->unfocus();
            next_focusable->focus();
            m_focus_id = next_focusable->focus_id();
            return true;
        }
    };

} // namespace ui
