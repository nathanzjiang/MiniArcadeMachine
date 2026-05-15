#ifndef SPRITE_DEMO_GAME_H
#define SPRITE_DEMO_GAME_H

#include "Game.h"

struct SpriteDemoState {
    uint32_t frameCount = 0;
    uint32_t animationFrame = 0;
    uint32_t lastFrameMicros = 0;
    uint32_t lastFpsMillis = 0;
    float fps = 0.0f;

    int16_t lastX = -1;
    int16_t lastY = -1;
    int16_t spriteX = 10;
    int16_t spriteY = 44;
    int16_t spriteDx = 2;
    int16_t spriteDy = 2;
};

class SpriteDemoGame : public Game {
public:
    void begin(GameContext &ctx) override;
    void update(GameContext &ctx) override;
    void render(GameContext &ctx) override;

private:
    static const int16_t SPRITE_SIZE = 40;
    static const int16_t PLAY_TOP = 34;
    static const uint16_t TARGET_FPS = 20;
    static const uint32_t FRAME_INTERVAL_MICROS = 1000000UL / TARGET_FPS;

    SpriteDemoState state;
    bool frameDirty = false;
    bool fpsDirty = false;

    void resetTiming();
    bool shouldDrawFrame();
    void updateSprite(int16_t displayWidth, int16_t displayHeight);
    void recordFrame();
    bool updateFps();
    void drawFpsOverlay(LGFX &display);
    void drawSpriteFrame(LGFX &display);
};

#endif
