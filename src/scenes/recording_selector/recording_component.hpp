

#pragma once

#include "helper/color_literals.hpp"
#include "manager/recording/recording.hpp"
#include "ui/components/label.hpp"
#include "ui/focusable.hpp"
#include "ui/hoverable.hpp"
#include "ui/layouts/tile_layout.hpp"
#include "ui/widget.hpp"

#include <filesystem>
#include <utility>

namespace data {

    //TODO: add drop support
    enum class RecordingSource : u8 { CommandLine, Folder, Manual, Online, Drop };

    struct RecordingMetadata {
        std::filesystem::path path;
        RecordingSource source;
        std::vector<recorder::TetrionHeader> headers;
        recorder::AdditionalInformation information;
    };

} // namespace data


namespace custom_ui {

    struct RecordingComponent final : public ui::Widget, public ui::Focusable, public ui::Hoverable {
    private:
        ui::TileLayout m_main_layout;
        data::RecordingMetadata m_metadata;

    public:
        explicit RecordingComponent(
                ServiceProvider* service_provider,
                ui::FocusHelper& focus_helper,
                data::RecordingMetadata metadata,
                const ui::Layout& layout,
                bool is_top_level
        );

        void render(const ServiceProvider& service_provider) const override;

        helper::BoolWrapper<ui::EventHandleType> handle_event(const SDL_Event& event, const Window* window) override;

        [[nodiscard]] data::RecordingMetadata metadata() const;

    private:
        [[nodiscard]] std::tuple<ui::Label*, ui::Label*, ui::Label*, ui::Label*> get_texts();

        void populate_texts(ServiceProvider* service_provider);
    };


} // namespace custom_ui
