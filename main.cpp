#include <SDL.h>
#include <memory>
#include "renderer.h"

std::unique_ptr<renderer::Renderer> software_renderer;
uint16_t width = 800;
uint16_t height = 600;

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
            "SoftwareRenderer",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            0
    );
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    renderer::Color clear_color {255, 0, 0, 255};
    software_renderer = std::make_unique<renderer::Renderer>(clear_color, width, height);

    uint8_t channel_value = 0;
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

        software_renderer->set_clear_color({channel_value++, 0, 0, 255});
        auto* pixels = static_cast<uint32_t*>(surface->pixels);
        software_renderer->clear_buffer(pixels);

        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}