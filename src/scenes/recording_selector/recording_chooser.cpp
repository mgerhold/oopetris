
#include "recording_chooser.hpp"


#include "helper/nfd_include.hpp"
#include "manager/event_dispatcher.hpp"
#include "manager/font.hpp"
#include "manager/recording/recording.hpp"
#include "manager/recording/recording_reader.hpp"
#include "manager/resource_manager.hpp"
#include "ui/components/text_button.hpp"

custom_ui::RecordingFileChooser::RecordingFileChooser(
                ServiceProvider* service_provider,
                ui::FocusHelper& focus_helper,
                const ui::Layout& layout,
                bool is_top_level
        ):ui::Widget{layout, ui::WidgetType::Component, is_top_level},
        ui::Focusable{focus_helper.focus_id()},
        ui::Hoverable{layout.get_rect()},
        m_main_grid{  focus_helper.focus_id(), 2,
        ui::Direction::Horizontal,
                  ui::RelativeMargin{ layout, ui::Direction::Vertical, 0.05 },
    std::pair<double, double>{ 0.05, 0.05 } ,
    ui::RelativeLayout{layout, 0.0, 0.2, 1.0, 0.5  },false
       }{


    m_main_grid.add<ui::TextButton>(
            service_provider, "Select Recording", service_provider->fonts().get(FontId::Default), Color::white(),
            focus_helper.focus_id(),
            [service_provider, this](const ui::TextButton&) -> bool {
                this->prepare_dialog(service_provider);

                const auto result = helper::openMultipleFilesDialog({
                        {"OOPetris Recording", { constants::recording::extension }}
                });

                if (result.has_value()) {
                    for (const auto& path : result.value()) {
                        this->currently_chosen_files.push_back(path);
                    }
                } else {
                    spdlog::warn("error in dialog: {}", result.error());
                }

                this->cleanup_dialog(service_provider);
                return result.has_value();
            },
            std::pair<double, double>{ 0.9, 0.85 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center },
            std::pair<double, double>{ 0.1, 0.1 }
    );


    m_main_grid.add<ui::TextButton>(
            service_provider, "Select Recording Folder", service_provider->fonts().get(FontId::Default), Color::white(),
            focus_helper.focus_id(),
            [this, service_provider](const ui::TextButton&) -> bool {
                this->prepare_dialog(service_provider);

                const auto result = helper::openFolderDialog();

                if (result.has_value()) {

                    if (std::filesystem::exists(result.value())) {
                        for (const auto& file : std::filesystem::recursive_directory_iterator(result.value())) {
                            auto header_value = recorder::RecordingReader::is_header_valid(file.path());
                            if (header_value.has_value()) {
                                this->currently_chosen_files.push_back(file.path());
                            }
                        }
                    }

                } else {
                    spdlog::warn("error in dialog: {}", result.error());
                }

                this->cleanup_dialog(service_provider);

                return result.has_value();
            },
            std::pair<double, double>{ 0.9, 0.85 },
            ui::Alignment{ ui::AlignmentHorizontal::Middle, ui::AlignmentVertical::Center },
            std::pair<double, double>{ 0.1, 0.1 }
    );
}

void custom_ui::RecordingFileChooser::render(const ServiceProvider& service_provider) const {
    auto color = has_focus() ? is_hovered() ? "#00f2ff"_c : "#00bbff"_c : is_hovered() ? "#C9C9C9"_c : "#9C9C9C"_c;

    service_provider.renderer().draw_rect_filled(layout().get_rect(), color);

    m_main_grid.render(service_provider);
}

helper::BoolWrapper<ui::EventHandleType>
custom_ui::RecordingFileChooser::handle_event(const SDL_Event& event, const Window* window) {
    //TODO: this double nested component can't correctly detect focus changes (since the checking for a focus change only occurs at one level deep)
    //TODO: allow horizontal RIGHT <-> LEFT focus change on horizontal focus_layouts
    if (const auto handled = m_main_grid.handle_event(event, window); handled) {
        return handled;
    }


    if (detect_hover(event, window)) {
        return true;
    }

    return false;
}


[[nodiscard]] const std::vector<std::filesystem::path>& custom_ui::RecordingFileChooser::get_currently_chosen_files(
) const {
    return currently_chosen_files;
}

//TODO: solve in another way, that is better
void custom_ui::RecordingFileChooser::prepare_dialog(ServiceProvider* service_provider) {

    //TODO: show scene on top, that hints of the dialog
    this->currently_chosen_files.clear();
    service_provider->event_dispatcher().disable();
}

void custom_ui::RecordingFileChooser::cleanup_dialog( //NOLINT(readability-convert-member-functions-to-static)
        ServiceProvider* service_provider
) {

    //TODO: remove hint scene on top
    service_provider->event_dispatcher().enable();
}
