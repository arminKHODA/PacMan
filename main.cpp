#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ctime>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 20;

// Player dimensions
const int PLAYER_SIZE = TILE_SIZE;

// Map dimensions (40x30 grid for simplicity)
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

// Game speed
const int GAME_SPEED = 100;

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Player {
    int x, y;
    Direction dir;
};

void RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void DrawPlayer(SDL_Renderer* renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect playerRect = { x * TILE_SIZE, y * TILE_SIZE, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderFillRect(renderer, &playerRect);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() < 0) {
        std::cerr << "Could not initialize SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pac-Man",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("font.ttf", 24);
    if (!font) {
        std::cerr << "Could not load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Player player = { MAP_WIDTH / 2, MAP_HEIGHT / 2, RIGHT };

    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks(), currentTime, elapsedTime;

    while (running) {
        currentTime = SDL_GetTicks();
        elapsedTime = currentTime - lastTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    player.dir = UP;
                    break;
                case SDLK_DOWN:
                    player.dir = DOWN;
                    break;
                case SDLK_LEFT:
                    player.dir = LEFT;
                    break;
                case SDLK_RIGHT:
                    player.dir = RIGHT;
                    break;
                }
            }
        }

        if (elapsedTime > GAME_SPEED) {
            switch (player.dir) {
            case UP:
                player.y = (player.y - 1 + MAP_HEIGHT) % MAP_HEIGHT;
                break;
            case DOWN:
                player.y = (player.y + 1) % MAP_HEIGHT;
                break;
            case LEFT:
                player.x = (player.x - 1 + MAP_WIDTH) % MAP_WIDTH;
                break;
            case RIGHT:
                player.x = (player.x + 1) % MAP_WIDTH;
                break;
            }
            lastTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        DrawPlayer(renderer, player.x, player.y);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
