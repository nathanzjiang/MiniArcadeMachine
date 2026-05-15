#ifndef GAME_H
#define GAME_H

#include <Arduino.h>

#include "display/DisplayConfig.h"
#include "InputManager.h"

struct GameContext {
    LGFX &display;
    const InputState &input;
    uint32_t nowMs;
};

class Game {
public:
    virtual ~Game() = default;

    virtual void begin(GameContext &ctx) = 0;
    virtual void update(GameContext &ctx) = 0;
    virtual void render(GameContext &ctx) = 0;

    bool gameEnded = false;
    bool completed = false;
    Game *nextGame = nullptr;
    Game *exitGame = nullptr;
};

#endif
