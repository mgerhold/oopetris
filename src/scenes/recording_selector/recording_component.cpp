
#include "recording_component.hpp"
#include "helper/date.hpp"
#include "helper/magic_enum_wrapper.hpp"
#include "manager/font.hpp"
#include "manager/resource_manager.hpp"
#include "ui/widget.hpp"

#include <fmt/format.h>


custom_ui::RecordingComponent::RecordingComponent(
                ServiceProvider* service_provider,
                ui::FocusHelper& focus_helper,
                data::RecordingMetadata metadata,
                const ui::Layout& layout,
                bool is_top_level
        ):ui::Widget{layout, ui::WidgetType::Component, is_top_level},
        ui::Focusable{focus_helper.focus_id()},
        ui::Hoverable{layout.get_rect()},
        m_main_layout{ utils::size_t_identity<2>(), focus_helper.focus_id(), 
        ui::Direction::Vertical,
                    std::array<double, 1>{ 0.6 }, ui::RelativeMargin{layout.get_rect(), ui::Direction::Vertical,0.05}, std::pair<double, double>{ 0.05, 0.03 },
                    layout,false
       },m_metadata{std::move(metadata)}{

    m_main_layout.add<ui::Label>(
            service_provider, "name: ?", service_provider->fonts().get(FontId::Default), Color::white(),
            std::pair<double, double>{ 0.5, 0.5 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
    );

    const auto information_layout_index = m_main_layout.add<ui::TileLayout>(
            utils::size_t_identity<3>(), focus_helper.focus_id(), ui::Direction::Horizontal,
            std::array<double, 2>{ 0.33, 0.66 }, ui::AbsolutMargin{ 10 }, std::pair<double, double>{ 0.05, 0.03 }
    );

    auto* const information_layout = m_main_layout.get<ui::TileLayout>(information_layout_index);


    information_layout->add<ui::Label>(
            service_provider, "source: ?", service_provider->fonts().get(FontId::Default), Color::white(),
            std::pair<double, double>{ 0.9, 0.9 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
    );

    information_layout->add<ui::Label>(
            service_provider, "date: ?", service_provider->fonts().get(FontId::Default), Color::white(),
            std::pair<double, double>{ 0.9, 0.9 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
    );

    information_layout->add<ui::Label>(
            service_provider, "playmode: ?", service_provider->fonts().get(FontId::Default), Color::white(),
            std::pair<double, double>{ 0.9, 0.9 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center }
    );

    populate_texts(service_provider);
}

void custom_ui::RecordingComponent::render(const ServiceProvider& service_provider) const {

    auto color = has_focus()    ? is_hovered() ? "#00f2ff"_c : "#00bbff"_c
                 : is_hovered() ? "hsv(0, 0, 0.79)"_c
                                : "#9C9C9C"_c;

    service_provider.renderer().draw_rect_filled(layout().get_rect(), color);

    m_main_layout.render(service_provider);
}

helper::BoolWrapper<ui::EventHandleType>
custom_ui::RecordingComponent::handle_event(const SDL_Event& event, const Window* window) {

    if (utils::device_supports_keys()) {
        if (has_focus() and utils::event_is_action(event, utils::CrossPlatformAction::OK)) {
            return { true, ui::EventHandleType::RequestAction };
        }
    }


    if (const auto hover_result = detect_hover(event, window); hover_result) {
        if (hover_result.is(ui::ActionType::Clicked)) {
            return { true, has_focus() ? ui::EventHandleType::RequestAction : ui::EventHandleType::RequestFocus };
        }
        return true;
    }

    return false;
}


[[nodiscard]] data::RecordingMetadata custom_ui::RecordingComponent::metadata() const {
    return m_metadata;
}

[[nodiscard]] std::tuple<ui::Label*, ui::Label*, ui::Label*, ui::Label*> custom_ui::RecordingComponent::get_texts() {

    auto* name_text = m_main_layout.get<ui::Label>(0);

    auto* information_layout = m_main_layout.get<ui::TileLayout>(1);

    auto* source_text = information_layout->get<ui::Label>(0);
    auto* date_text = information_layout->get<ui::Label>(1);
    auto* playmode_text = information_layout->get<ui::Label>(2);

    return { name_text, source_text, date_text, playmode_text };
}


void custom_ui::RecordingComponent::populate_texts(ServiceProvider* service_provider) {

    auto [name_text, source_text, date_text, playmode_text] = get_texts();

    const auto& info = m_metadata.information;

    std::string name_text_value{};
    if (const auto name_info = info.get_if<std::string>("name"); name_info.has_value()) {
        name_text_value = name_info.value();
    } else {
        name_text_value = m_metadata.path.filename().string();
    }
    name_text->set_text(*service_provider, fmt::format("name: {}", name_text_value));

    source_text->set_text(*service_provider, fmt::format("source: {}", magic_enum::enum_name(m_metadata.source)));


    std::string date_text_value = "unknown";
    if (const auto date_info_string = info.get_if<std::string>("date"); date_info_string.has_value()) {
        date_text_value = date_info_string.value();
    } else if (const auto date_info_number = info.get_if<u64>("date"); date_info_number.has_value()) {
        const auto formatted_date_value = date::ISO8601Date(date_info_number.value()).to_string();
        if (formatted_date_value.has_value()) {
            date_text_value = formatted_date_value.value();
        }
    } else {
        if (const auto date_iso8601_value = date::ISO8601Date::from_string(m_metadata.path.stem().string());
            date_iso8601_value.has_value()) {
            const auto formatted_date_value = date_iso8601_value.value().to_string();
            if (formatted_date_value.has_value()) {
                date_text_value = formatted_date_value.value();
            }
        }
    }
    date_text->set_text(*service_provider, fmt::format("date: {}", date_text_value));

    std::string playmode_text_value = "unknown";
    if (const auto mode_info = info.get_if<std::string>("mode"); mode_info.has_value()) {
        playmode_text_value = mode_info.value();
    } else if (const auto playmode_info = info.get_if<std::string>("playmode"); playmode_info.has_value()) {
        playmode_text_value = playmode_info.value();
    }
    playmode_text->set_text(*service_provider, fmt::format("playmode: {}", playmode_text_value));
}
