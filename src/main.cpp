#include "color.h"
#include "model.h"
#include "renderer.h"

#include <iostream>
#include <SDL2/SDL.h>

constexpr uint16_t DEFAULT_WIDTH = 800;
constexpr uint16_t DEFAULT_HEIGHT = 600;
constexpr renderer::Color DEFAULT_CLEAR_COLOR {255, 255, 255, 255};

int main(int argc, char* argv[]) {
    const std::string default_window_title = "SoftwareRenderer";

    uint32_t current_tick = 0;
    uint32_t delta_ticks = 0;
    uint16_t fps = 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL failed to initialize: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto window_deleter = [](SDL_Window* window) {
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
    };
    auto window = std::unique_ptr<SDL_Window, void (*) (SDL_Window*)>(
            SDL_CreateWindow(
                    default_window_title.c_str(),
                    SDL_WINDOWPOS_CENTERED,
                    SDL_WINDOWPOS_CENTERED,
                    DEFAULT_WIDTH,
                    DEFAULT_HEIGHT,
                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    ), window_deleter);

    if (window == nullptr) {
        std::cerr << "Could not load an SDL window." << std::endl;
        return 1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window.get());
    auto* pixels = static_cast<uint32_t*>(surface->pixels);

    std::unique_ptr<renderer::Renderer> renderer =
            std::make_unique<renderer::Renderer>(window.get(), DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_CLEAR_COLOR);

    std::unique_ptr<renderer::Model> model;
    try {
        model = std::make_unique<renderer::Model>("data/Pallas_Cat");
    } catch (const std::runtime_error& error) {
        std::cout << "Runtime Error: " << error.what() << std::endl;
        return 1;
    }

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
        math::Matrix<4, 4> rotation_mtx1 = math::create_rotation_matrix(1.f, 0.f, 0.f, 1.6f);
        math::Matrix<4, 4> rotation_mtx2 = math::create_rotation_matrix(0.f, 0.f, 1.f, current_tick / 5000.f);
        math::Matrix<4, 4> rotation_mtx = math::mul(rotation_mtx1, rotation_mtx2);
        math::Matrix<4, 4> translation_mtx = math::create_translation_matrix(1.f, 15.f, 50.f);
        renderer->draw_model(model.get(), pixels, rotation_mtx, translation_mtx, 60.f);

        // FPS
        delta_ticks = SDL_GetTicks() - current_tick;
        current_tick = SDL_GetTicks();
        if (delta_ticks > 0) {
            fps = 1000 / delta_ticks;
        }

        const std::string title = default_window_title + " (" + std::to_string(fps) + " FPS)";
        SDL_SetWindowTitle(window.get(), title.c_str());

        SDL_UpdateWindowSurface(window.get());
    }

    SDL_Quit();

    return 0;
}
