#include "GameRunner.h"

#include "games/SpriteDemoGame.h"

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
        switchTo(new SpriteDemoGame);
    }
}

void GameRunner::switchTo(Game *nextGame) {
    currentGame = nextGame;

    if (currentGame != nullptr) {
        GameContext ctx{display, input.state(), millis()};
        currentGame->begin(ctx);
    }
}
