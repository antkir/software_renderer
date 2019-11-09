#include "color.h"
#include "model.h"
#include "renderer.h"

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

constexpr uint16_t DEFAULT_WIDTH = 800;
constexpr uint16_t DEFAULT_HEIGHT = 600;

constexpr renderer::Color clear_color {255, 255, 255, 255};
constexpr SDL_Color fps_text_color = { 255, 0, 0, 255 };
constexpr SDL_Color bg_text_color = { 0, 255, 0, 255 };

void render_text(SDL_Surface* surface, TTF_Font* font, const std::string& text) {
    auto surface_deleter = [](SDL_Surface* surface) {
        if (surface != nullptr) {
            SDL_FreeSurface(surface);
        }
    };
    auto text_surface = std::unique_ptr<SDL_Surface, void (*) (SDL_Surface*)>(
            TTF_RenderText_Shaded(font, text.c_str(), fps_text_color, bg_text_color), surface_deleter);
    if (text_surface != nullptr) {
        SDL_UpperBlit(text_surface.get(), &text_surface->clip_rect, surface, nullptr);
    } else {
        std::cout << "Failed to create text surface: " << TTF_GetError() << std::endl;
    }
}

bool init_sdl_ttf() {
    if (TTF_Init() != 0) {
        std::cerr << "SDL_ttf failed to initialize: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

bool init_sdl_image() {
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP) == 0) {
        std::cerr << "SDL_image failed to initialize: " << IMG_GetError() << std::endl;
        return false;
    }
    return true;
}

int main() {
    uint32_t current_tick = 0;
    uint32_t delta_ticks = 0;
    uint16_t fps = 0;

    if (!init_sdl_ttf()) {
        return -1;
    }

    auto font_deleter = [](TTF_Font* font) {
        if (font != nullptr) {
            TTF_CloseFont(font);
        }
    };
    auto font = std::unique_ptr<TTF_Font, void (*) (TTF_Font*)>(
            TTF_OpenFont("data/NotoSans.ttf", 24), font_deleter);
    if (font == nullptr) {
        std::cerr << "Failed to load the font: " << TTF_GetError() << std::endl;
        return -1;
    }

    if (!init_sdl_image()) {
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL failed to initialize: " << SDL_GetError() << std::endl;
        return -1;
    }

    auto window_deleter = [](SDL_Window* window) {
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
    };
    auto window = std::unique_ptr<SDL_Window, void (*) (SDL_Window*)>(
            SDL_CreateWindow(
            "SoftwareRenderer",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            DEFAULT_WIDTH,
            DEFAULT_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    ), window_deleter);

    if (window == nullptr) {
        std::cerr << "Could not load an SDL window." << std::endl;
        return -1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window.get());
    auto* pixels = static_cast<uint32_t*>(surface->pixels);

    std::unique_ptr<renderer::Renderer> renderer =
            std::make_unique<renderer::Renderer>(window.get(), DEFAULT_WIDTH, DEFAULT_HEIGHT, clear_color);

    renderer::Model model = renderer::Model("data/Pallas_Cat");

    current_tick = SDL_GetTicks();

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_WINDOWEVENT:
                    switch(event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            surface = SDL_GetWindowSurface(window.get());
                            pixels = static_cast<uint32_t*>(surface->pixels);
                            renderer->resize_window(event.window.data1, event.window.data2);
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_QUIT:
                    quit = true;
                    break;
                default:
                    break;
            }
        }

        renderer->clear_buffer(pixels);

        // Draw model
        renderer->draw_model(model, pixels);

        // FPS
        delta_ticks = SDL_GetTicks() - current_tick;
        current_tick = SDL_GetTicks();
        if (delta_ticks > 0) {
            fps = 1000 / delta_ticks;
        }

        render_text(surface, font.get(), std::to_string(fps));

        SDL_UpdateWindowSurface(window.get());
    }

    SDL_Quit();

    IMG_Quit();

    font.reset(nullptr);
    TTF_Quit();

    return 0;
}
