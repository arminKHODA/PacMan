#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 20;

// Player and Enemy dimensions
const int PLAYER_SIZE = TILE_SIZE;
const int ENEMY_SIZE = TILE_SIZE;

// Map dimensions (40x30 grid for simplicity)
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

// Game speed
const int GAME_SPEED = 100;

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { MENU, PLAYING, GAME_OVER };

struct Player {
    int x, y;
    Direction dir;
};

struct Enemy {
    int x, y;
    Direction dir;
};

// Function to load the maze from a file
bool LoadMaze(const std::string& filename, int maze[MAP_HEIGHT][MAP_WIDTH]) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int value, column = 0;
        while (iss >> value) {
            if (column < MAP_WIDTH) {
                maze[lineNumber][column++] = value;
            }
            else {
                std::cerr << "Too many columns in line " << lineNumber + 1 << std::endl;
                return false;
            }
        }

        if (column != MAP_WIDTH) {
            std::cerr << "Not enough columns in line " << lineNumber + 1 << std::endl;
            return false;
        }

        lineNumber++;
        if (lineNumber > MAP_HEIGHT) {
            std::cerr << "Too many rows in the file" << std::endl;
            return false;
        }
    }

    if (lineNumber != MAP_HEIGHT) {
        std::cerr << "Not enough lines in the file. Expected " << MAP_HEIGHT << " lines, but got " << lineNumber << " lines." << std::endl;
        return false;
    }

    file.close();
    return true;
}

// Function to render text
void RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Function to draw the player
void DrawPlayer(SDL_Renderer* renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect playerRect = { x * TILE_SIZE, y * TILE_SIZE, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderFillRect(renderer, &playerRect);
}

// Function to draw the map
void DrawMap(SDL_Renderer* renderer, const int maze[MAP_HEIGHT][MAP_WIDTH]) {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (maze[y][x] == 1) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                SDL_Rect wallRect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &wallRect);
            }
        }
    }
}

// Function to draw the enemies
void DrawEnemy(SDL_Renderer* renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect enemyRect = { x * TILE_SIZE, y * TILE_SIZE, ENEMY_SIZE, ENEMY_SIZE };
    SDL_RenderFillRect(renderer, &enemyRect);
}

// Function to check collision between player and walls
bool CheckCollision(const Player& player, const int maze[MAP_HEIGHT][MAP_WIDTH]) {
    if (maze[player.y][player.x] == 1) {
        return true;
    }
    return false;
}

// Function to check if the game is over
bool CheckGameOver(const Player& player, const Enemy& enemy1, const Enemy& enemy2) {
    return (player.x == enemy1.x && player.y == enemy1.y) ||
        (player.x == enemy2.x && player.y == enemy2.y);
}

// Function to move the enemies towards the player
void MoveEnemy(Enemy& enemy, const Player& player, const int maze[MAP_HEIGHT][MAP_WIDTH]) {
    int dx = player.x - enemy.x;
    int dy = player.y - enemy.y;

    Direction bestDir = RIGHT;
    if (std::abs(dx) > std::abs(dy)) {
        if (dx > 0) bestDir = RIGHT;
        else bestDir = LEFT;
    }
    else {
        if (dy > 0) bestDir = DOWN;
        else bestDir = UP;
    }

    Enemy nextPosition = enemy;
    switch (bestDir) {
    case UP:
        nextPosition.y = (enemy.y - 1 + MAP_HEIGHT) % MAP_HEIGHT;
        break;
    case DOWN:
        nextPosition.y = (enemy.y + 1) % MAP_HEIGHT;
        break;
    case LEFT:
        nextPosition.x = (enemy.x - 1 + MAP_WIDTH) % MAP_WIDTH;
        break;
    case RIGHT:
        nextPosition.x = (enemy.x + 1) % MAP_WIDTH;
        break;
    }
    if (!CheckCollision(Player{ nextPosition.x, nextPosition.y, RIGHT }, maze)) {
        enemy = nextPosition;
    }
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

    SDL_Window* window = SDL_CreateWindow("Pac-Man Game",
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
    Enemy enemy1 = { 1, 1, RIGHT };
    Enemy enemy2 = { MAP_WIDTH - 2, MAP_HEIGHT - 2, LEFT };

    GameState gameState = MENU;
    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks(), currentTime, elapsedTime;

    int maze[MAP_HEIGHT][MAP_WIDTH];
    if (!LoadMaze("level_001.txt", maze)) {
        std::cerr << "Failed to load maze." << std::endl;
        return 1;
    }

    while (running) {
        currentTime = SDL_GetTicks();
        elapsedTime = currentTime - lastTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (gameState == MENU) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        gameState = PLAYING;
                    }
                }
                else if (gameState == GAME_OVER) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        gameState = MENU;
                        player = { MAP_WIDTH / 2, MAP_HEIGHT / 2, RIGHT };
                        enemy1 = { 1, 1, RIGHT };
                        enemy2 = { MAP_WIDTH - 2, MAP_HEIGHT - 2, LEFT };
                    }
                }
                else if (gameState == PLAYING) {
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
        }

        if (gameState == PLAYING && elapsedTime > GAME_SPEED) {
            Player nextPosition = player;
            switch (player.dir) {
            case UP:
                nextPosition.y = (player.y - 1 + MAP_HEIGHT) % MAP_HEIGHT;
                break;
            case DOWN:
                nextPosition.y = (player.y + 1) % MAP_HEIGHT;
                break;
            case LEFT:
                nextPosition.x = (player.x - 1 + MAP_WIDTH) % MAP_WIDTH;
                break;
            case RIGHT:
                nextPosition.x = (player.x + 1) % MAP_WIDTH;
                break;
            }
            if (!CheckCollision(nextPosition, maze)) {
                player = nextPosition;
            }

            MoveEnemy(enemy1, player, maze);
            MoveEnemy(enemy2, player, maze);

            if (CheckGameOver(player, enemy1, enemy2)) {
                gameState = GAME_OVER;
            }

            lastTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (gameState == MENU) {
            RenderText(renderer, font, "PAC-MAN", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, { 255, 255, 255, 255 });
            RenderText(renderer, font, "Press ENTER to Start", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, { 255, 255, 255, 255 });
        }
        else if (gameState == GAME_OVER) {
            RenderText(renderer, font, "GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, { 255, 0, 0, 255 });
            RenderText(renderer, font, "Press ENTER to Restart", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, { 255, 255, 255, 255 });
        }
        else if (gameState == PLAYING) {
            DrawMap(renderer, maze);
            DrawPlayer(renderer, player.x, player.y);
            DrawEnemy(renderer, enemy1.x, enemy1.y);
            DrawEnemy(renderer, enemy2.x, enemy2.y);
        }

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
