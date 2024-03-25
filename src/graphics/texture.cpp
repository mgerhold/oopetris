
#include "texture.hpp"


Texture::Texture(SDL_Texture* raw_texture) : m_raw_texture{ raw_texture } { }

Texture Texture::from_image(SDL_Renderer* renderer, const std::filesystem::path& image_path) {
    SDL_Texture* image = IMG_LoadTexture(renderer, image_path.string().c_str());

    if (image == nullptr) {
        throw std::runtime_error(
                fmt::format("Failed to load image from path '{}' with error: {}", image_path.string(), SDL_GetError())
        );
    }
    return Texture{ image };
}

Texture Texture::prerender_text(
        SDL_Renderer* renderer,
        const std::string& text,
        const Font& font,
        const Color& color,
        RenderType render_type,
        const Color& background_color
) {

    const SDL_Color text_color = color.to_sdl_color();
    SDL_Surface* surface{ nullptr };
    if (render_type == RenderType::Solid) {
        surface = TTF_RenderUTF8_Solid(font.get(), text.c_str(), text_color);
    } else if (render_type == RenderType::Blended) {
        surface = TTF_RenderUTF8_Blended(font.get(), text.c_str(), text_color);
    } else if (render_type == RenderType::Shaded) {
        surface = TTF_RenderUTF8_Shaded(font.get(), text.c_str(), text_color, background_color.to_sdl_color());
    } else {
        utils::unreachable();
    }
    if (surface == nullptr) {
        throw std::runtime_error(fmt::format("Failed to pre-render text into surface with error: {}", SDL_GetError()));
    }

    SDL_Texture* const texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        throw std::runtime_error(fmt::format("Failed to pre-render text into texture with error: {}", SDL_GetError()));
    }


    return Texture{ texture };
}

Texture Texture::get_for_render_target(SDL_Renderer* renderer, const shapes::UPoint& size) {

    SDL_DisplayMode mode{};
    const int result = SDL_GetCurrentDisplayMode(0, &mode);
    if (result != 0) {
        throw std::runtime_error{ "failed in getting display mode: " + std::string{ SDL_GetError() } };
    }

    auto* const texture = SDL_CreateTexture(
            renderer, mode.format, SDL_TEXTUREACCESS_TARGET, static_cast<int>(size.x), static_cast<int>(size.y)
    );
    if (texture == nullptr) {
        throw std::runtime_error(fmt::format("Failed to create texture with error: {}", SDL_GetError()));
    }

    const auto target_result = SDL_SetRenderTarget(renderer, texture);
    if (target_result < 0) {
        throw std::runtime_error(fmt::format("Failed to set render target with error: {}", SDL_GetError()));
    }


    return Texture{ texture };
}

Texture::Texture(Texture&& old) noexcept : m_raw_texture{ old.m_raw_texture } {
    old.m_raw_texture = nullptr;
};

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        this->~Texture();
        this->m_raw_texture = other.m_raw_texture;
        other.m_raw_texture = nullptr;
    }

    return *this;
};

Texture::~Texture() {

    if (m_raw_texture != nullptr) {
        SDL_DestroyTexture(m_raw_texture);
        m_raw_texture = nullptr;
    }
}

[[nodiscard]] shapes::UPoint Texture::size() const {
    shapes::IPoint size;
    const auto result = SDL_QueryTexture(m_raw_texture, nullptr, nullptr, &size.x, &size.y);

    if (result < 0) {
        spdlog::error("Failed to get texture size with error: {}", SDL_GetError());
        return { 0, 0 };
    }
    return size.cast<u32>();
}

void Texture::set_as_render_target(SDL_Renderer* renderer) const {
    const auto result = SDL_SetRenderTarget(renderer, m_raw_texture);
    if (result < 0) {
        throw std::runtime_error(fmt::format("Failed to set render texture target with error: {}", SDL_GetError()));
    }
}
