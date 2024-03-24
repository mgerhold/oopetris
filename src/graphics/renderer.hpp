#pragma once

#include "helper/color.hpp"
#include "manager/font.hpp"
#include "rect.hpp"
#include "texture.hpp"
#include "window.hpp"

#include <SDL.h>
#include <filesystem>
#include <string>

struct Renderer final {
public:
    enum class VSync : u8 {
        Enabled,
        Disabled,
    };

private:
    SDL_Renderer* m_renderer;

public:
    explicit Renderer(Window& window, VSync v_sync);
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    ~Renderer();

    void set_draw_color(const Color& color) const;
    void clear(const Color& clear_color = Color::black()) const;

    template<typename T>
    void draw_rect_filled(const shapes::AbstractRect<T>& rect, const Color& color) const {
        set_draw_color(color);
        const SDL_Rect sdl_rect = rect.to_sdl_rect();
        SDL_RenderFillRect(m_renderer, &sdl_rect);
    }

    template<typename T>
    void draw_rect_outline(const shapes::AbstractRect<T>& rect, const Color& color) const {
        set_draw_color(color);
        const SDL_Rect sdl_rect = rect.to_sdl_rect();
        SDL_RenderDrawRect(m_renderer, &sdl_rect);
    }

    template<typename S, typename T>
    void draw_line(const shapes::AbstractPoint<S>& start, const shapes::AbstractPoint<T>& end, const Color& color)
            const {
        set_draw_color(color);
        SDL_RenderDrawLine(
                m_renderer, static_cast<int>(start.x), static_cast<int>(start.y), static_cast<int>(end.x),
                static_cast<int>(end.y)
        );
    }

    template<typename T>
    void draw_texture(const Texture& texture, const shapes::AbstractRect<T>& rect) const {
        texture.render(m_renderer, rect);
    }

    template<typename S, typename T>
    void draw_texture(const Texture& texture, const shapes::AbstractRect<S>& from, const shapes::AbstractRect<T>& to)
            const {
        texture.render(m_renderer, from, to);
    }

    [[nodiscard]] Texture load_image(const std::filesystem::path& image_path) const;
    [[nodiscard]] Texture prerender_text(
            const std::string& text,
            const Font& font,
            const Color& color,
            RenderType render_type = RenderType::Blended,
            const Color& background_color = Color::black()
    ) const;
    [[nodiscard]] Texture get_texture_for_render_target(const shapes::UPoint& size) const;

    void set_render_target(const Texture& texture) const;
    void reset_render_target() const;


    void present() const;

    friend struct Text;
};
