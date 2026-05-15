#include "GameRunner.h"

GameRunner::GameRunner(LGFX &display, InputManager &input)
    : display(display), input(input) {
}

void GameRunner::begin(Game *initialGame) {
    input.begin();
    currentGame = initialGame;

    if (currentGame != nullptr) {
        GameContext ctx{display, input.state(), millis()};
        currentGame->begin(ctx);
    }
}

void GameRunner::loop() {
    if (currentGame == nullptr) {
        return;
    }

    input.update();

    GameContext ctx{display, input.state(), millis()};
    currentGame->update(ctx);
    currentGame->render(ctx);

    if (currentGame->gameEnded == true) {
        Game *nextGame = currentGame->nextGame;
        currentGame->gameEnded = false;
        currentGame->nextGame = nullptr;

        if (nextGame != nullptr) {
            switchTo(nextGame);
        }
    }
}

void GameRunner::switchTo(Game *nextGame) {
    currentGame = nextGame;

    if (currentGame != nullptr) {
        currentGame->gameEnded = false;
        currentGame->nextGame = nullptr;
        GameContext ctx{display, input.state(), millis()};
        currentGame->begin(ctx);
    }
}
