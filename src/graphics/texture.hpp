
#pragma once


#include "helper/color.hpp"
#include "helper/utils.hpp"
#include "manager/font.hpp"
#include "point.hpp"
#include "rect.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <filesystem>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <string>

enum class RenderType : u8 { Solid, Blended, Shaded };

struct Texture {
private:
    SDL_Texture* m_raw_texture;

    explicit Texture(SDL_Texture* raw_texture);

public:
    static Texture from_image(SDL_Renderer* renderer, const std::filesystem::path& image_path);

    static Texture prerender_text(
            SDL_Renderer* renderer,
            const std::string& text,
            const Font& font,
            const Color& color,
            RenderType render_type,
            const Color& background_color
    );

    static Texture get_for_render_target(SDL_Renderer* renderer, const shapes::UPoint& size);

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& old) noexcept;

    Texture& operator=(Texture&& other) noexcept;

    ~Texture();

    template<typename T>
    void render(SDL_Renderer* renderer, const shapes::AbstractRect<T>& rect) const {
        const SDL_Rect rect_sdl = rect.to_sdl_rect();
        SDL_RenderCopy(renderer, m_raw_texture, nullptr, &rect_sdl);
    }

    template<typename T>
    void render(SDL_Renderer* renderer, const shapes::AbstractRect<T>& from, const shapes::URect& to) const {
        const SDL_Rect from_rect_sdl = from.to_sdl_rect();
        const SDL_Rect to_rect_sdl = to.to_sdl_rect();
        SDL_RenderCopy(renderer, m_raw_texture, &from_rect_sdl, &to_rect_sdl);
    }

    [[nodiscard]] shapes::UPoint size() const;

    void set_as_render_target(SDL_Renderer* renderer) const;
};
