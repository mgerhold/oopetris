#include "renderer.hpp"

namespace {
    constexpr SDL_Rect to_sdl_rect(shapes::Rect rect) {
        return SDL_Rect{ rect.top_left.x, rect.top_left.y, rect.bottom_right.x - rect.top_left.x + 1,
                         rect.bottom_right.y - rect.top_left.y + 1 };
    }
} // namespace

Renderer::Renderer(Window& window, const VSync v_sync)
    : m_renderer{ SDL_CreateRenderer(
            window.get_sdl_window(),
            -1,
            (v_sync == VSync::Enabled ? SDL_RENDERER_PRESENTVSYNC : 0) | SDL_RENDERER_TARGETTEXTURE
    ) } {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(m_renderer);
}

void Renderer::set_draw_color(const Color& color) const {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
}

void Renderer::clear(const Color& clear_color) const {
    set_draw_color(clear_color);
    SDL_RenderClear(m_renderer);
}

void Renderer::draw_rect_filled(const shapes::Rect& rect, const Color& color) const {
    set_draw_color(color);
    const SDL_Rect sdl_rect = to_sdl_rect(rect);
    SDL_RenderFillRect(m_renderer, &sdl_rect);
}

void Renderer::draw_rect_outline(const shapes::Rect& rect, const Color& color) const {
    set_draw_color(color);
    const SDL_Rect sdl_rect = to_sdl_rect(rect);
    SDL_RenderDrawRect(m_renderer, &sdl_rect);
}

void Renderer::present() const {
    SDL_RenderPresent(m_renderer);
}

void Renderer::draw_line(const shapes::Point& start, const shapes::Point& end, const Color& color) const {
    set_draw_color(color);
    SDL_RenderDrawLine(m_renderer, start.x, start.y, end.x, end.y);
}

void Renderer::draw_texture(const Texture& texture, const shapes::Rect& rect) const {
    texture.render(m_renderer, rect);
}
void Renderer::draw_texture(const Texture& texture, const shapes::Rect& from, const shapes::Rect& to) const {
    texture.render(m_renderer, from, to);
}


Texture Renderer::load_image(const std::string& image_path) const {
    return Texture::from_image(m_renderer, image_path);
}

Texture Renderer::prerender_text(const std::string& text, const Font& font, const Color& color) const {
    return Texture::prerender_text(m_renderer, text, font, color);
}


Texture Renderer::get_texture_for_render_target(const shapes::Point& size) const {


    const auto supported = SDL_RenderTargetSupported(m_renderer);

    if (supported == SDL_FALSE) {
        throw std::runtime_error("SDL does not support a target renderer, but we need one!");
    }

    return Texture::get_for_render_target(m_renderer, size);
}


void Renderer::set_render_target(const Texture& texture) const {
    texture.set_as_render_target(m_renderer);
}

void Renderer::reset_render_target() const {
    const auto result = SDL_SetRenderTarget(m_renderer, nullptr);
    if (result < 0) {
        throw std::runtime_error(fmt::format("Failed to set render texture target with error: {}", SDL_GetError()));
    }
}
