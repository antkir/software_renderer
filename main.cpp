#include <SDL.h>
#include <memory>
#include <iostream>
#include "model.h"
#include <SDL_ttf.h>
#include <functional>
#include "renderer.h"

std::unique_ptr<TTF_Font, std::function<void (TTF_Font*)>> font;
std::unique_ptr<renderer::Renderer> software_renderer;

uint16_t width = 800;
uint16_t height = 600;

uint32_t current_tick = 0;
uint32_t delta_ticks = 0;
uint16_t fps = 0;

renderer::Color clear_color {255, 0, 0, 255};

SDL_Color fps_text_color = { 255, 0, 0, 255 };
SDL_Color bg_text_color = { 0, 255, 0, 255 };

void render_text(SDL_Surface* surface, const std::string& text) {
    SDL_Surface* text_surface = TTF_RenderText_Shaded(font.get(), text.c_str(), fps_text_color, bg_text_color);
    if (text_surface != nullptr) {
        SDL_UpperBlit(text_surface, &text_surface->clip_rect, surface, nullptr);
        SDL_FreeSurface(text_surface);
    } else {
        std::cout << "Failed to create a text surface: " << TTF_GetError() << std::endl;
    }
}

bool init_font() {
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf failed to initialize: " << TTF_GetError() << std::endl;
        return false;
    }

    auto deleter = [](TTF_Font* f) { TTF_CloseFont(f); };
    font = std::unique_ptr<TTF_Font, std::function<void (TTF_Font*)>>(TTF_OpenFont("data/NotoSans.ttf", 24), deleter);
    if (font == nullptr) {
        std::cerr << "Failed to load the font: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

int main() {
    if (!init_font()) {
        return 0;
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
            "SoftwareRenderer",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            0
    );
    if (window == nullptr) {
        std::cerr << "Could not load an SDL window." << std::endl;
        return 0;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    auto* pixels = static_cast<uint32_t*>(surface->pixels);

    software_renderer = std::make_unique<renderer::Renderer>(clear_color, width, height);

    renderer::Model model = renderer::Model("data/Pallas_Cat.obj");

    current_tick = SDL_GetTicks();

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                default:
                    break;
            }
        }

        software_renderer->clear_buffer(pixels);

        // Draw model
        software_renderer->draw_model(model, pixels);

        // FPS
        delta_ticks = SDL_GetTicks() - current_tick;
        current_tick = SDL_GetTicks();
        if (delta_ticks > 0) {
            fps = 1000 / delta_ticks;
        }

        render_text(surface, std::to_string(fps));

        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
