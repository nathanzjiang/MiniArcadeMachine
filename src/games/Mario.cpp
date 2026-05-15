#include "Mario.h"

namespace {
const char *LEVEL[] = {
    "....................",
    "....................",
    "....................",
    "....................",
    ".................F..",
    "................##..",
    "............C.......",
    "...........###......",
    ".....C..............",
    "....###.............",
    "..............E.....",
    "..P...........###...",
    "########..##########",
    "########..##########",
    "########..##########"
};

const uint16_t SKY = 0x7DFF;
const uint16_t DIRT = 0x8A22;
const uint16_t BRICK = 0xC3A6;
const uint16_t PLAYER = 0xFBE0;
const uint16_t PLAYER_TRIM = 0xD800;
const uint16_t ENEMY = 0xA145;
const uint16_t COIN = 0xFFE0;
const uint16_t FLAG = 0x07E0;

const float GRAVITY = 0.55f;
const float MOVE_ACCEL = 0.75f;
const float FRICTION = 0.72f;
const float MAX_SPEED = 2.2f;
const float JUMP_SPEED = -6.1f;
const uint32_t JUMP_BUFFER_MS = 120;
const uint32_t COYOTE_MS = 100;

void expandDirtyRect(int16_t &left, int16_t &top, int16_t &right, int16_t &bottom,
                     int16_t x, int16_t y, int16_t w, int16_t h) {
    left = min<int16_t>(left, x - 3);
    top = min<int16_t>(top, y - 3);
    right = max<int16_t>(right, x + w + 3);
    bottom = max<int16_t>(bottom, y + h + 3);
}
}

void Mario::begin(GameContext &ctx) {
    resetLevel();
    mode = MarioMode::START;
    screenDirty = true;
    frameDirty = true;
    ctx.display.fillScreen(TFT_BLACK);
    Serial.println("Mario draft started");
}

void Mario::update(GameContext &ctx) {
    if (mode == MarioMode::START) {
        if (ctx.input.aRisingEdge || ctx.input.bRisingEdge) {
            mode = MarioMode::PLAY;
            state.lastFrameMicros = micros();
            screenDirty = true;
            frameDirty = true;
        }
        return;
    }

    if (mode == MarioMode::LOSE) {
        if (ctx.input.aRisingEdge || ctx.input.bRisingEdge) {
            resetLevel();
            mode = MarioMode::PLAY;
            screenDirty = true;
            frameDirty = true;
        }
        return;
    }

    if (mode == MarioMode::WIN) {
        if (ctx.input.aRisingEdge || ctx.input.bRisingEdge) {
            resetLevel();
            mode = MarioMode::START;
            screenDirty = true;
            frameDirty = true;
        }
        return;
    }

    if (ctx.input.bRisingEdge) {
        state.lastJumpPressedMs = ctx.nowMs;
    }

    if (!shouldStep()) {
        return;
    }

    stepPhysics(ctx);
    updateEnemy();
    updateCoins();
    checkPlayerHazards();
    frameDirty = true;
}

void Mario::render(GameContext &ctx) {
    if (mode == MarioMode::START) {
        if (screenDirty) {
            drawCenteredMessage(ctx.display, "PLATFORMER", "A/B START");
            screenDirty = false;
        }
        return;
    }

    if (mode == MarioMode::LOSE) {
        if (screenDirty) {
            drawCenteredMessage(ctx.display, "TRY AGAIN", "A/B RESTART");
            screenDirty = false;
        }
        return;
    }

    if (mode == MarioMode::WIN) {
        if (screenDirty) {
            drawCenteredMessage(ctx.display, "COURSE CLEAR", "A/B MENU");
            screenDirty = false;
        }
        return;
    }

    if (screenDirty) {
        drawPlayfield(ctx.display);
        drawStatus(ctx.display);
        screenDirty = false;
        frameDirty = false;
        statusDirty = false;
        state.lastPlayerX = state.player.x;
        state.lastPlayerY = state.player.y;
        state.lastEnemyX = state.enemy.x;
        state.lastEnemyY = state.enemy.y;
        state.lastEnemyAlive = state.enemyAlive;
        state.coinDirty[0] = false;
        state.coinDirty[1] = false;
        return;
    }

    if (frameDirty) {
        drawDirtyFrame(ctx.display);
        frameDirty = false;
    }

    if (statusDirty) {
        drawStatus(ctx.display);
        statusDirty = false;
    }
}

void Mario::resetLevel() {
    state = MarioState{};
    state.player.x = 2.0f * TILE_SIZE;
    state.player.y = 10.0f * TILE_SIZE;
    state.player.w = 12;
    state.player.h = 16;

    state.enemy.x = 14.0f * TILE_SIZE;
    state.enemy.y = 10.0f * TILE_SIZE + 2;
    state.enemy.w = 14;
    state.enemy.h = 14;
    state.enemy.vx = -0.6f;
    state.enemyDir = -1;
    state.lastPlayerX = state.player.x;
    state.lastPlayerY = state.player.y;
    state.lastEnemyX = state.enemy.x;
    state.lastEnemyY = state.enemy.y;
    state.lastEnemyAlive = state.enemyAlive;
    state.lastFrameMicros = micros();
    statusDirty = true;
    gameEnded = false;
}

bool Mario::shouldStep() {
    const uint32_t nowMicros = micros();
    if (nowMicros - state.lastFrameMicros < FRAME_INTERVAL_MICROS) {
        return false;
    }

    state.lastFrameMicros += FRAME_INTERVAL_MICROS;
    return true;
}

void Mario::stepPhysics(GameContext &ctx) {
    MarioActor &p = state.player;

    if (ctx.input.left) {
        p.vx -= MOVE_ACCEL;
    }
    if (ctx.input.right) {
        p.vx += MOVE_ACCEL;
    }
    if (!ctx.input.left && !ctx.input.right) {
        p.vx *= FRICTION;
        if (abs(p.vx) < 0.05f) {
            p.vx = 0.0f;
        }
    }

    p.vx = constrain(p.vx, -MAX_SPEED, MAX_SPEED);
    p.vy += GRAVITY;
    p.vy = min(p.vy, 7.0f);

    const bool jumpBuffered = ctx.nowMs - state.lastJumpPressedMs <= JUMP_BUFFER_MS;
    const bool canCoyoteJump = ctx.nowMs - state.lastGroundedMs <= COYOTE_MS;
    if (jumpBuffered && canCoyoteJump) {
        p.vy = JUMP_SPEED;
        p.onGround = false;
        state.lastJumpPressedMs = 0;
        state.lastGroundedMs = 0;
    }

    if (p.vy < 0.0f && !ctx.input.b) {
        p.vy *= 0.55f;
    }

    moveActor(p, p.vx, 0.0f);
    moveActor(p, 0.0f, p.vy);

    if (p.onGround) {
        state.lastGroundedMs = ctx.nowMs;
    }

    if (p.y > MAP_H * TILE_SIZE) {
        mode = MarioMode::LOSE;
        screenDirty = true;
    }

    if (overlaps(p.x, p.y, p.w, p.h, 17 * TILE_SIZE, 4 * TILE_SIZE, TILE_SIZE, TILE_SIZE * 2)) {
        mode = MarioMode::WIN;
        screenDirty = true;
    }
}

void Mario::moveActor(MarioActor &actor, float dx, float dy) {
    actor.onGround = dy == 0.0f ? actor.onGround : false;

    actor.x += dx;
    actor.y += dy;

    int16_t left = floor(actor.x / TILE_SIZE);
    int16_t right = floor((actor.x + actor.w - 1) / TILE_SIZE);
    int16_t top = floor(actor.y / TILE_SIZE);
    int16_t bottom = floor((actor.y + actor.h - 1) / TILE_SIZE);

    for (int16_t ty = top; ty <= bottom; ty++) {
        for (int16_t tx = left; tx <= right; tx++) {
            if (!isSolidTile(tx, ty)) {
                continue;
            }

            if (dx > 0.0f) {
                actor.x = tx * TILE_SIZE - actor.w;
                actor.vx = 0.0f;
            } else if (dx < 0.0f) {
                actor.x = (tx + 1) * TILE_SIZE;
                actor.vx = 0.0f;
            } else if (dy > 0.0f) {
                actor.y = ty * TILE_SIZE - actor.h;
                actor.vy = 0.0f;
                actor.onGround = true;
            } else if (dy < 0.0f) {
                actor.y = (ty + 1) * TILE_SIZE;
                actor.vy = 0.0f;
            }
        }
    }

    actor.x = constrain(actor.x, 0.0f, MAP_W * TILE_SIZE - actor.w);
}

bool Mario::isSolidTile(int16_t tileX, int16_t tileY) const {
    if (tileX < 0 || tileX >= MAP_W || tileY < 0 || tileY >= MAP_H) {
        return tileY >= MAP_H;
    }

    return LEVEL[tileY][tileX] == '#';
}

bool Mario::overlaps(float ax, float ay, int16_t aw, int16_t ah,
                     float bx, float by, int16_t bw, int16_t bh) const {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void Mario::updateCoins() {
    const int16_t coinX[2] = {5 * TILE_SIZE + 4, 12 * TILE_SIZE + 4};
    const int16_t coinY[2] = {8 * TILE_SIZE + 4, 6 * TILE_SIZE + 4};

    for (uint8_t i = 0; i < 2; i++) {
        if (!state.coinCollected[i] &&
            overlaps(state.player.x, state.player.y, state.player.w, state.player.h,
                     coinX[i], coinY[i], 8, 8)) {
            state.coinCollected[i] = true;
            state.coinDirty[i] = true;
            state.coins++;
            statusDirty = true;
        }
    }
}

void Mario::updateEnemy() {
    if (!state.enemyAlive) {
        return;
    }

    MarioActor &e = state.enemy;
    e.vx = state.enemyDir * 0.65f;
    moveActor(e, e.vx, 0.0f);
    e.vy += GRAVITY;
    e.vy = min(e.vy, 5.0f);
    moveActor(e, 0.0f, e.vy);

    const int16_t probeX = floor((e.x + (state.enemyDir > 0 ? e.w + 1 : -1)) / TILE_SIZE);
    const int16_t footY = floor((e.y + e.h + 1) / TILE_SIZE);
    const int16_t wallY = floor((e.y + e.h / 2) / TILE_SIZE);
    if (!isSolidTile(probeX, footY) || isSolidTile(probeX, wallY)) {
        state.enemyDir = -state.enemyDir;
    }
}

void Mario::checkPlayerHazards() {
    if (!state.enemyAlive) {
        return;
    }

    MarioActor &p = state.player;
    MarioActor &e = state.enemy;
    if (!overlaps(p.x, p.y, p.w, p.h, e.x, e.y, e.w, e.h)) {
        return;
    }

    if (p.vy > 0.0f && p.y + p.h - e.y < 10.0f) {
        state.enemyAlive = false;
        p.vy = -3.8f;
        return;
    }

    mode = MarioMode::LOSE;
    screenDirty = true;
}

void Mario::drawPlayfield(LGFX &display) {
    display.startWrite();
    drawWorldRegion(display, 0, 0, MAP_W * TILE_SIZE, MAP_H * TILE_SIZE, true);
    display.endWrite();
}

void Mario::drawDirtyFrame(LGFX &display) {
    int16_t left = MAP_W * TILE_SIZE;
    int16_t top = MAP_H * TILE_SIZE;
    int16_t right = 0;
    int16_t bottom = 0;

    expandDirtyRect(left, top, right, bottom, state.lastPlayerX, state.lastPlayerY,
                    state.player.w, state.player.h);
    expandDirtyRect(left, top, right, bottom, state.player.x, state.player.y,
                    state.player.w, state.player.h);

    if (state.lastEnemyAlive) {
        expandDirtyRect(left, top, right, bottom, state.lastEnemyX, state.lastEnemyY,
                        state.enemy.w, state.enemy.h);
    }
    if (state.enemyAlive) {
        expandDirtyRect(left, top, right, bottom, state.enemy.x, state.enemy.y,
                        state.enemy.w, state.enemy.h);
    }

    const int16_t coinX[2] = {5 * TILE_SIZE + 3, 12 * TILE_SIZE + 3};
    const int16_t coinY[2] = {8 * TILE_SIZE + 3, 6 * TILE_SIZE + 3};
    for (uint8_t i = 0; i < 2; i++) {
        if (state.coinDirty[i]) {
            expandDirtyRect(left, top, right, bottom, coinX[i], coinY[i], 10, 10);
        }
    }

    left = max<int16_t>(0, left);
    top = max<int16_t>(14, top);
    right = min<int16_t>(MAP_W * TILE_SIZE, right);
    bottom = min<int16_t>(MAP_H * TILE_SIZE, bottom);
    const int16_t w = right - left;
    const int16_t h = bottom - top;
    if (w <= 0 || h <= 0) {
        return;
    }

    LGFX_Sprite dirty(&display);
    dirty.setColorDepth(16);
    if (dirty.createSprite(w, h) == nullptr) {
        display.startWrite();
        display.setClipRect(left, top, w, h);
        drawWorldRegion(display, left, top, w, h, true, left, top);
        display.clearClipRect();
        display.endWrite();
    } else {
        drawWorldRegion(dirty, left, top, w, h, true);
        display.startWrite();
        dirty.pushSprite(&display, left, top);
        display.endWrite();
        dirty.deleteSprite();
    }

    for (uint8_t i = 0; i < 2; i++) {
        state.coinDirty[i] = false;
    }

    state.lastPlayerX = state.player.x;
    state.lastPlayerY = state.player.y;
    state.lastEnemyX = state.enemy.x;
    state.lastEnemyY = state.enemy.y;
    state.lastEnemyAlive = state.enemyAlive;
}

void Mario::drawWorldRegion(lgfx::LGFXBase &canvas, int16_t worldX, int16_t worldY,
                            int16_t w, int16_t h, bool includeActors,
                            int16_t canvasX, int16_t canvasY) {
    worldX = max<int16_t>(0, worldX);
    worldY = max<int16_t>(0, worldY);
    w = min<int16_t>(w, MAP_W * TILE_SIZE - worldX);
    h = min<int16_t>(h, MAP_H * TILE_SIZE - worldY);
    if (w <= 0 || h <= 0) {
        return;
    }

    canvas.fillRect(canvasX, canvasY, w, h, SKY);

    const int16_t left = max<int16_t>(0, worldX / TILE_SIZE);
    const int16_t right = min<int16_t>(MAP_W - 1, (worldX + w - 1) / TILE_SIZE);
    const int16_t top = max<int16_t>(0, worldY / TILE_SIZE);
    const int16_t bottom = min<int16_t>(MAP_H - 1, (worldY + h - 1) / TILE_SIZE);

    for (int16_t ty = top; ty <= bottom; ty++) {
        for (int16_t tx = left; tx <= right; tx++) {
            if (LEVEL[ty][tx] == '#') {
                const int16_t px = canvasX + tx * TILE_SIZE - worldX;
                const int16_t py = canvasY + ty * TILE_SIZE - worldY;
                canvas.fillRect(px, py, TILE_SIZE, TILE_SIZE, BRICK);
                canvas.drawRect(px, py, TILE_SIZE, TILE_SIZE, DIRT);
            }
        }
    }

    drawCoins(canvas, canvasX - worldX, canvasY - worldY);
    drawFlag(canvas, canvasX - worldX, canvasY - worldY);

    if (includeActors) {
        drawEnemy(canvas, canvasX - worldX, canvasY - worldY);
        drawPlayer(canvas, canvasX - worldX, canvasY - worldY);
    }
}

void Mario::drawCoins(lgfx::LGFXBase &canvas, int16_t offsetX, int16_t offsetY) {
    const int16_t coinX[2] = {5 * TILE_SIZE + 8, 12 * TILE_SIZE + 8};
    const int16_t coinY[2] = {8 * TILE_SIZE + 8, 6 * TILE_SIZE + 8};
    for (uint8_t i = 0; i < 2; i++) {
        if (!state.coinCollected[i]) {
            canvas.fillCircle(coinX[i] + offsetX, coinY[i] + offsetY, 5, COIN);
            canvas.drawCircle(coinX[i] + offsetX, coinY[i] + offsetY, 5, TFT_BLACK);
        }
    }
}

void Mario::drawFlag(lgfx::LGFXBase &canvas, int16_t offsetX, int16_t offsetY) {
    canvas.drawFastVLine(17 * TILE_SIZE + 4 + offsetX, 4 * TILE_SIZE + offsetY, TILE_SIZE * 2, TFT_WHITE);
    canvas.fillTriangle(17 * TILE_SIZE + 5 + offsetX, 4 * TILE_SIZE + offsetY,
                        17 * TILE_SIZE + 5 + offsetX, 4 * TILE_SIZE + 16 + offsetY,
                        18 * TILE_SIZE + 1 + offsetX, 4 * TILE_SIZE + 8 + offsetY, FLAG);
}

void Mario::drawEnemy(lgfx::LGFXBase &canvas, int16_t offsetX, int16_t offsetY) {
    if (!state.enemyAlive) {
        return;
    }

    const int16_t x = state.enemy.x + offsetX;
    const int16_t y = state.enemy.y + offsetY;
    canvas.fillRoundRect(x, y, state.enemy.w, state.enemy.h, 3, ENEMY);
    canvas.fillRect(x + 3, y + 4, 2, 2, TFT_BLACK);
    canvas.fillRect(x + 9, y + 4, 2, 2, TFT_BLACK);
}

void Mario::drawPlayer(lgfx::LGFXBase &canvas, int16_t offsetX, int16_t offsetY) {
    const int16_t x = state.player.x + offsetX;
    const int16_t y = state.player.y + offsetY;
    canvas.fillRect(x, y + 5, state.player.w, state.player.h - 5, PLAYER);
    canvas.fillRect(x + 2, y, state.player.w - 4, 6, PLAYER_TRIM);
    canvas.fillRect(x + 8, y + 8, 2, 2, TFT_BLACK);
}

void Mario::drawStatus(LGFX &display) {
    LGFX_Sprite status(&display);
    status.setColorDepth(16);
    if (status.createSprite(display.width(), 14) == nullptr) {
        display.fillRect(0, 0, display.width(), 14, TFT_BLACK);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextSize(1);
        display.setTextDatum(top_left);
        display.drawString("COINS " + static_cast<String>(state.coins) + "/2", 4, 3);
        return;
    }

    status.fillRect(0, 0, display.width(), 14, TFT_BLACK);
    status.setTextColor(TFT_WHITE, TFT_BLACK);
    status.setTextSize(1);
    status.setTextDatum(top_left);
    status.drawString("COINS " + static_cast<String>(state.coins) + "/2", 4, 3);

    display.startWrite();
    status.pushSprite(&display, 0, 0);
    display.endWrite();
    status.deleteSprite();
}

void Mario::drawCenteredMessage(LGFX &display, const char *title, const char *prompt) {
    display.startWrite();
    display.fillScreen(TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(3);
    display.drawString(title, display.width() / 2, display.height() / 2 - 24);
    display.setTextSize(2);
    display.drawString(prompt, display.width() / 2, display.height() / 2 + 22);
    display.endWrite();
}
