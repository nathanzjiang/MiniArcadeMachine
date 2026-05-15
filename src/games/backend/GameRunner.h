#ifndef GAME_RUNNER_H
#define GAME_RUNNER_H

#include "../Game.h"

class GameRunner {
public:
    GameRunner(LGFX &display, InputManager &input);

    void begin(Game *initialGame);
    void loop();
    void switchTo(Game *nextGame);

private:
    LGFX &display;
    InputManager &input;
    Game *currentGame = nullptr;
};

#endif
