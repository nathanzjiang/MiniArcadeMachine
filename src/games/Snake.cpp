#include "Snake.h"
#include "InputManager.h"

namespace {
bool wantsMenuExit(const InputState &input) {
    return input.left || input.leftRisingEdge ||
           input.joystickInput == JoystickInput::LEFT ||
           input.joystickRisingEdge == JoystickInput::LEFT;
}
}

void Snake::begin(GameContext &ctx) {
    state = SnakeState{};
    for (int16_t x = 1; x <= state.gridLength; x++) {
        for (int16_t y = 1; y <= state.gridWidth; y++) {
            if (std::find(state.snake.begin(), state.snake.end(), std::pair<int16_t, int16_t> {x, y}) == state.snake.end()) {
                state.notSnake.emplace_back(x, y);
            }
        }
    }

    if (newToStartState) {
        drawInitialSpriteFrame(ctx.display);
    }

    Serial.println("Snake started");

    frameDirty = true;
    fullFrameDirty = false;
    uiDirty = true;
}

void Snake::update(GameContext &ctx) {
    if (gameState == START) {
        begin(ctx);
        newToStartState = false;
        if (wantsMenuExit(ctx.input) && exitGame != nullptr) {
            nextGame = exitGame;
            gameEnded = true;
        } else if (ctx.input.bRisingEdge || ctx.input.rightRisingEdge || ctx.input.joystickRisingEdge == JoystickInput::RIGHT) {
            resetTiming();
            fullFrameDirty = true;
            gameState = PLAY;
        }
    } else if (gameState == PLAY) {
        if (!shouldDrawFrame()) {
            return;
        }

        updateSprite(ctx);
    } else if (gameState == LOSE) {
        if (wantsMenuExit(ctx.input) && exitGame != nullptr) {
            nextGame = exitGame;
            gameEnded = true;
        } else if (ctx.input.bRisingEdge || ctx.input.rightRisingEdge || ctx.input.joystickRisingEdge == JoystickInput::RIGHT) {
            gameState = START;
            newToStartState = true;
        }
    } else if (gameState == WIN) {
        if (wantsMenuExit(ctx.input) && exitGame != nullptr) {
            nextGame = exitGame;
            gameEnded = true;
        } else if (ctx.input.bRisingEdge || ctx.input.rightRisingEdge || ctx.input.joystickRisingEdge == JoystickInput::RIGHT) {
            gameState = START;
            newToStartState = true;
        }
    }
}

bool Snake::shouldDrawFrame() {
    const uint32_t nowMicros = micros();
    if (nowMicros - state.lastFrameMicros < FRAME_INTERVAL_MICROS) {
        return false;
    }

    state.lastFrameMicros += FRAME_INTERVAL_MICROS;
    return true;
}

void Snake::updateSprite(GameContext &ctx) {
    frameDirty = true;

    std::pair<int16_t, int16_t> newSquare = state.snake.front();

    // Update the state.direction and state.spriteX, state.spriteY based on current state.direction
    // and joystickInputs.
    if (state.direction == UP) {
        if (ctx.input.joystickInput == JoystickInput::LEFT) {
            newSquare.first--;
            state.direction = LEFT;
        } else if (ctx.input.joystickInput == JoystickInput::RIGHT) {
            newSquare.first++;
            state.direction = RIGHT;
        } else {
            newSquare.second--;
        }
    } else if (state.direction == LEFT) {
        if (ctx.input.joystickInput == JoystickInput::DOWN) {
            newSquare.second++;
            state.direction = DOWN;
        } else if (ctx.input.joystickInput == JoystickInput::UP) {
            newSquare.second--;
            state.direction = UP;
        } else {
            newSquare.first--;
        }
    } else if (state.direction == DOWN) {
        if (ctx.input.joystickInput == JoystickInput::RIGHT) {
            newSquare.first++;
            state.direction = RIGHT;
        } else if (ctx.input.joystickInput == JoystickInput::LEFT) {
            newSquare.first--;
            state.direction = LEFT;
        } else {
            newSquare.second++;
        }
    } else if (state.direction == RIGHT) {
        if (ctx.input.joystickInput == JoystickInput::DOWN) {
            newSquare.second++;
            state.direction = DOWN;
        } else if (ctx.input.joystickInput == JoystickInput::UP) {
            newSquare.second--;
            state.direction = UP;
        } else {
            newSquare.first++;
        }
    }

    moveSquare(newSquare);
}

void Snake::moveSquare(const std::pair<int16_t, int16_t> &newSquare) {
    // Hit food
    if (newSquare == state.food) {
        state.snake.push_front(newSquare);
        state.notSnake.erase(std::find(state.notSnake.begin(), state.notSnake.end(), newSquare));

        // regenerate snake food
        const int16_t foodIndex = random(0, state.notSnake.size());
        state.food = state.notSnake.at(foodIndex);
        uiDirty = true;

        state.score++;
        if (state.score == 5) {
            completed = true;
            gameState = WIN;
        }
        return;
    }

    // Hit wall
    if (newSquare.first <= 0 || newSquare.first > state.gridLength ||
        newSquare.second <= 0 || newSquare.second > state.gridWidth) {
        // TODO: Implement this
        gameState = LOSE;
        uiDirty = true;
        return;
    }

    // Hit snake
    if (std::find(state.snake.begin(), state.snake.end(), newSquare) != state.snake.end()) {
        gameState = LOSE;
        uiDirty = true;
        return;
    }

    // Blank square
    state.snake.push_front(newSquare);
    state.notSnake.erase(std::find(state.notSnake.begin(), state.notSnake.end(), newSquare));

    state.lastSprite = state.snake.back();
    state.notSnake.push_back(state.snake.back());
    state.snake.pop_back();
}

void Snake::render(GameContext &ctx) {
    if (gameState == START || gameState == PLAY) {
        if (fullFrameDirty) {
            drawFullSpriteFrame(ctx.display);
            fullFrameDirty = false;
            frameDirty = false;
            uiDirty = false;
            return;
        }

        if (frameDirty) {
            drawSpriteFrame(ctx.display);
        }

        if (uiDirty) {
            drawScoreUI(ctx.display);
        }
    } else if (gameState == LOSE) {
        if (uiDirty) {
            drawLoseScreen(ctx.display);
            uiDirty = false;
        }
    } else if (gameState == WIN) {
        if (uiDirty) {
            drawWinScreen(ctx.display);
            uiDirty = false;
        }
    }
}

void Snake::resetTiming() {
    state.lastFrameMicros = micros();
}

void Snake::drawFullSpriteFrame(LGFX &display) const {
    display.startWrite();

    display.fillScreen(TFT_BLACK);
    for (const std::pair<int16_t, int16_t> square : state.snake) {
        display.fillRect(square.first * state.boxSize, square.second * state.boxSize, state.boxSize, state.boxSize, TFT_WHITE);
    }

    display.fillRect(state.food.first * state.boxSize, state.food.second * state.boxSize, state.boxSize, state.boxSize, TFT_RED);
    display.setTextSize(1.5);
    display.setTextDatum(top_left);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString("Score: " + static_cast<String>(state.score), 0, 0);
    display.drawRect(15, 15, 290, 210, TFT_WHITE);

    display.endWrite();
}

void Snake::drawSpriteFrame(LGFX &display) const {
    display.startWrite();

    display.setTextDatum(top_left);
    display.fillRect(state.lastSprite.first * state.boxSize, state.lastSprite.second * state.boxSize, state.boxSize, state.boxSize, TFT_BLACK);
    display.fillRect(state.snake.front().first * state.boxSize, state.snake.front().second * state.boxSize, state.boxSize, state.boxSize, TFT_WHITE);

    display.fillRect(state.food.first * state.boxSize, state.food.second * state.boxSize, state.boxSize, state.boxSize, TFT_RED);

    display.endWrite();
}

void Snake::drawScoreUI(LGFX &display) const {
    display.startWrite();

    display.setTextSize(1.5);
    display.setTextDatum(top_left);
    display.drawString("Score: " + static_cast<String>(state.score), 0, 0);
    display.drawRect(15, 15, 290, 210, TFT_WHITE);
    display.endWrite();
}

void Snake::drawInitialSpriteFrame(LGFX &display) const {
    drawFullSpriteFrame(display);

    display.startWrite();

    display.setTextSize(2);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString("SNAKE", tft.width() / 2, 38);
    display.setTextSize(1);
    display.drawString("RIGHT/B: start", tft.width() / 2, 58);
    display.drawString("LEFT: menu", tft.width() / 2, 72);
    display.drawString("Eat 5 red squares to win", tft.width() / 2, 86);

    display.endWrite();
}

void Snake::drawLoseScreen(LGFX &display) const {
    display.startWrite();
    display.fillScreen(TFT_BLACK);

    display.setTextSize(3);
    display.setTextDatum(middle_center);
    display.drawString("You Lose", tft.width() / 2, tft.height() / 2 - 20);
    display.setTextSize(2);
    display.drawString("RIGHT/B retry", tft.width() / 2, tft.height() / 2 + 18);
    display.drawString("LEFT menu", tft.width() / 2, tft.height() / 2 + 42);
    display.endWrite();
}

void Snake::drawWinScreen(LGFX &display) const {
    display.startWrite();
    display.fillScreen(TFT_BLACK);

    display.setTextSize(3);
    display.setTextDatum(middle_center);
    display.drawString("You Win!", tft.width() / 2, tft.height() / 2 - 20);
    display.setTextSize(2);
    display.drawString("RIGHT/B replay", tft.width() / 2, tft.height() / 2 + 18);
    display.drawString("LEFT menu", tft.width() / 2, tft.height() / 2 + 42);
    display.endWrite();
}
