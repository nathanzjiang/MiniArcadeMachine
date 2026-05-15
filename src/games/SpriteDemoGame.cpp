#include "SpriteDemoGame.h"

void SpriteDemoGame::begin(GameContext &ctx) {
    state = SpriteDemoState{};
    ctx.display.fillScreen(TFT_BLACK);
    drawFpsOverlay(ctx.display);

    Serial.println("LovyanGFX ILI9341 FPS test started");
    Serial.println("SPI speed: 10 MHz");
    Serial.println("Mode: dirty rectangle sprite redraw");
    Serial.print("Frame cap: ");
    Serial.print(TARGET_FPS);
    Serial.println(" FPS");

    resetTiming();
    frameDirty = false;
    fpsDirty = false;
}

void SpriteDemoGame::update(GameContext &ctx) {
    if (!shouldDrawFrame()) {
        return;
    }

    updateSprite(ctx.display.width(), ctx.display.height());
    recordFrame();
    frameDirty = true;

    if (updateFps()) {
        Serial.print("FPS: ");
        Serial.println(state.fps, 1);
        fpsDirty = true;
    }
}

void SpriteDemoGame::render(GameContext &ctx) {
    if (frameDirty) {
        drawSpriteFrame(ctx.display);
        frameDirty = false;
    }

    if (fpsDirty) {
        drawFpsOverlay(ctx.display);
        fpsDirty = false;
    }
}

void SpriteDemoGame::resetTiming() {
    state.lastFrameMicros = micros();
    state.lastFpsMillis = millis();
}

bool SpriteDemoGame::shouldDrawFrame() {
    uint32_t nowMicros = micros();
    if (nowMicros - state.lastFrameMicros < FRAME_INTERVAL_MICROS) {
        return false;
    }

    state.lastFrameMicros += FRAME_INTERVAL_MICROS;
    return true;
}

void SpriteDemoGame::updateSprite(int16_t displayWidth, int16_t displayHeight) {
    state.spriteX += state.spriteDx;
    state.spriteY += state.spriteDy;

    if (state.spriteX <= 0 || state.spriteX >= displayWidth - SPRITE_SIZE) {
        state.spriteDx = -state.spriteDx;
        state.spriteX += state.spriteDx;
    }

    if (state.spriteY <= PLAY_TOP || state.spriteY >= displayHeight - SPRITE_SIZE) {
        state.spriteDy = -state.spriteDy;
        state.spriteY += state.spriteDy;
    }
}

void SpriteDemoGame::recordFrame() {
    state.frameCount++;
    state.animationFrame++;
}

bool SpriteDemoGame::updateFps() {
    uint32_t now = millis();
    uint32_t elapsed = now - state.lastFpsMillis;
    if (elapsed < 1000) {
        return false;
    }

    state.fps = state.frameCount * 1001.0f / elapsed;
    state.frameCount = 0;
    state.lastFpsMillis = now;
    return true;
}

void SpriteDemoGame::drawFpsOverlay(LGFX &display) {
    display.fillRect(0, 0, display.width(), 30, TFT_BLACK);
    display.setCursor(6, 6);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(2);
    display.print("FPS ");
    display.print(state.fps, 1);
    display.drawFastHLine(0, 31, display.width(), TFT_DARKGREY);
}

void SpriteDemoGame::drawSpriteFrame(LGFX &display) {
    display.startWrite();

    if (state.lastX >= 0) {
        display.fillRect(state.lastX - 2, state.lastY - 2, SPRITE_SIZE + 4, SPRITE_SIZE + 4, TFT_BLACK);
    }

    display.fillRect(state.spriteX, state.spriteY, SPRITE_SIZE, SPRITE_SIZE, TFT_BLUE);
    display.drawRect(state.spriteX, state.spriteY, SPRITE_SIZE, SPRITE_SIZE, TFT_WHITE);

    display.endWrite();

    state.lastX = state.spriteX;
    state.lastY = state.spriteY;
}
