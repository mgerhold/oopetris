#pragma once

#include "graphics/rect.hpp"
#include "graphics/window.hpp"
#include "helper/types.hpp"
#include "helper/utils.hpp"

#include <cassert>

namespace ui {

    enum class LayoutType : u8 { FullScreen, Relative, Absolut, Raw };

    struct Layout {
    private:
        shapes::URect m_rect;
        LayoutType type;

    protected:
        Layout(const shapes::URect& rect, LayoutType type) : m_rect{ rect }, type{ type } { }

    public:
        Layout(const shapes::URect& rect) : m_rect{ rect }, type{ LayoutType::Raw } { }

        [[nodiscard]] const shapes::URect& get_rect() const {
            return m_rect;
        }

        [[nodiscard]] bool is_full_screen() const {
            return type == LayoutType::FullScreen;
        }
    };


    struct AbsolutLayout : public Layout {
        AbsolutLayout(const u32 x, const u32 y, const u32 width, const u32 height)
            : Layout{
                  shapes::URect{x, y, width, height},
                  LayoutType::Absolut
        } { }
    };


    struct FullScreenLayout : public Layout {
        FullScreenLayout(const shapes::URect& rect) : Layout{ rect, LayoutType::FullScreen } { }
        FullScreenLayout(const Window& window) : FullScreenLayout{ window.screen_rect() } { }
        FullScreenLayout(const Window* window) : FullScreenLayout{ window->screen_rect() } { }
    };


    struct RelativeLayout : public Layout {
        RelativeLayout(
                const shapes::URect& rect,
                const double x,
                const double y,
                const double width,
                const double height
        )
            : Layout{ shapes::URect(
                              static_cast<u32>(x * static_cast<double>(rect.width())),
                              static_cast<u32>(y * static_cast<double>(rect.height())),
                              static_cast<u32>(width * static_cast<double>(rect.width())),
                              static_cast<u32>(height * static_cast<double>(rect.height()))
                      ) >> rect.top_left,
                      LayoutType::Relative } {
            assert(x >= 0.0 && x <= 1.0 && "x has to be in correct percentage range!");
            assert(y >= 0.0 && y <= 1.0 && "y has to be in correct percentage range!");

            assert(width >= 0.0 && width <= 1.0 && "width has to be in correct percentage range!");
            assert(height >= 0.0 && height <= 1.0 && "height has to be in correct percentage range!");
        }
        RelativeLayout(const Window* window, const double x, const double y, const double width, const double height)
            : RelativeLayout{ window->screen_rect(), x, y, width, height } { }
        RelativeLayout(const Window& window, const double x, const double y, const double width, const double height)
            : RelativeLayout{ window.screen_rect(), x, y, width, height } { }
        RelativeLayout(const Layout& layout, const double x, const double y, const double width, const double height)
            : RelativeLayout{ layout.get_rect(), x, y, width, height } { }
    };


    enum class AlignmentVertical : u8 { Top, Center, Bottom };
    enum class AlignmentHorizontal : u8 { Left, Middle, Right };

    using Alignment = std::pair<AlignmentHorizontal, AlignmentVertical>;


    [[nodiscard]] u32 get_horizontal_alignment_offset(const Layout& layout, AlignmentHorizontal alignment, u32 width);


    [[nodiscard]] u32 get_vertical_alignment_offset(const Layout& layout, AlignmentVertical alignment, u32 height);

    [[nodiscard]] shapes::URect
    get_rectangle_aligned(const Layout& layout, const std::pair<u32, u32>& size, const Alignment& alignment);

    [[nodiscard]] std::pair<u32, u32>
    ratio_helper(const std::pair<u32, u32>& size, bool respect_ratio, const shapes::UPoint& original_ratio);


    enum class Direction : u8 { Horizontal, Vertical };

    struct Margin {
    private:
        u32 m_margin;

    public:
        Margin(u32 margin) : m_margin{ margin } { }


        [[nodiscard]] u32 get_margin() const {
            return m_margin;
        }
    };

    struct AbsolutMargin : public Margin {
        AbsolutMargin(const u32 margin) : Margin{ margin } { }
    };

    struct RelativeMargin : public Margin {
        RelativeMargin(const shapes::URect& rect, Direction direction, const double margin)
            : Margin{ static_cast<u32>(margin * (direction == Direction::Horizontal ? rect.width() : rect.height())) } {

            assert(margin >= 0.0 && margin <= 1.0 && "margin has to be in correct percentage range!");
        }
        RelativeMargin(const Window& window, Direction direction, const double margin)
            : RelativeMargin{ window.screen_rect(), direction, margin } { }
        RelativeMargin(const Window* window, Direction direction, const double margin)
            : RelativeMargin{ window->screen_rect(), direction, margin } { }
        RelativeMargin(const Layout& layout, Direction direction, const double margin)
            : RelativeMargin{ layout.get_rect(), direction, margin } { }
    };

} // namespace ui
