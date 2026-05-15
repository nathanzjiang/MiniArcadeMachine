#ifndef MINIARCADEMACHINE_SNAKE_H
#define MINIARCADEMACHINE_SNAKE_H

#include "Game.h"
#include <deque>
#include <vector>
#include <algorithm>

enum Direction {
    RIGHT,
    LEFT,
    UP,
    DOWN
};

enum GAME_STATE {
    START,
    PLAY,
    LOSE,
    WIN,
};

struct SnakeState {
    uint32_t lastFrameMicros = 0;

    int16_t boxSize = 16;
    int16_t gridLength = 18;
    int16_t gridWidth = 13;
    uint32_t snakeMillis = 135;

    std::deque<std::pair<int16_t, int16_t>> snake{{3, 7}, {2, 7}, {1, 7}};
    std::vector<std::pair<int16_t, int16_t>> notSnake;
    std::pair<int16_t, int16_t> lastSprite = {-1, -1};
    int16_t spriteX = 1;
    int16_t spriteY = 1;
    int16_t lastSpriteX = -1;
    int16_t lastSpriteY = -1;

    int16_t score = 0;

    std::pair<int16_t, int16_t> food = {12, 7};
    Direction direction = Direction::RIGHT;
};

class Snake : public Game {
public:
    void begin(GameContext &ctx) override;
    void update(GameContext &ctx) override;
    void render(GameContext &ctx) override;

private:
    static const uint32_t FRAME_INTERVAL_MICROS = 135000UL;

    SnakeState state;
    bool frameDirty = false;
    bool fullFrameDirty = false;
    bool uiDirty = false;

    GAME_STATE gameState = START;

    void resetTiming();
    bool shouldDrawFrame();
    void updateSprite(GameContext &ctx);
    void drawFullSpriteFrame(LGFX &display) const;
    void drawSpriteFrame(LGFX &display) const;

    void drawScoreUI(LGFX &display) const;

    void drawInitialSpriteFrame(LGFX &display) const;

    void drawLoseScreen(LGFX &display) const;

    void drawWinScreen(LGFX &display) const;

    void moveSquare(const std::pair<int16_t, int16_t> &newSquare);
};


#endif //MINIARCADEMACHINE_SNAKE_H
