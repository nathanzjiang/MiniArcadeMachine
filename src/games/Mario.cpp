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

    if (ctx.input.aRisingEdge || ctx.input.bRisingEdge || ctx.input.upRisingEdge) {
        state.lastJumpPressedMs = ctx.nowMs;
    }

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

    if (p.vy < 0.0f && !(ctx.input.a || ctx.input.b || ctx.input.up)) {
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
    display.fillScreen(SKY);

    for (int16_t y = 0; y < MAP_H; y++) {
        for (int16_t x = 0; x < MAP_W; x++) {
            const char tile = LEVEL[y][x];
            const int16_t px = x * TILE_SIZE;
            const int16_t py = y * TILE_SIZE;

            if (tile == '#') {
                display.fillRect(px, py, TILE_SIZE, TILE_SIZE, BRICK);
                display.drawRect(px, py, TILE_SIZE, TILE_SIZE, DIRT);
            }
        }
    }

    drawCoins(display);
    drawFlag(display);
    drawEnemy(display);
    drawPlayer(display);

    display.endWrite();
}

void Mario::drawDirtyFrame(LGFX &display) {
    const int16_t px = state.lastPlayerX - 2;
    const int16_t py = state.lastPlayerY - 2;
    const int16_t ex = state.lastEnemyX - 2;
    const int16_t ey = state.lastEnemyY - 2;

    display.startWrite();

    drawBackgroundRegion(display, px, py, state.player.w + 4, state.player.h + 4);
    if (state.lastEnemyAlive) {
        drawBackgroundRegion(display, ex, ey, state.enemy.w + 4, state.enemy.h + 4);
    }

    const int16_t coinX[2] = {5 * TILE_SIZE + 3, 12 * TILE_SIZE + 3};
    const int16_t coinY[2] = {8 * TILE_SIZE + 3, 6 * TILE_SIZE + 3};
    for (uint8_t i = 0; i < 2; i++) {
        if (state.coinDirty[i]) {
            drawBackgroundRegion(display, coinX[i], coinY[i], 10, 10);
            state.coinDirty[i] = false;
        }
    }

    drawEnemy(display);
    drawPlayer(display);
    display.endWrite();

    state.lastPlayerX = state.player.x;
    state.lastPlayerY = state.player.y;
    state.lastEnemyX = state.enemy.x;
    state.lastEnemyY = state.enemy.y;
    state.lastEnemyAlive = state.enemyAlive;
}

void Mario::drawBackgroundRegion(LGFX &display, int16_t x, int16_t y, int16_t w, int16_t h) {
    x = max<int16_t>(0, x);
    y = max<int16_t>(14, y);
    w = min<int16_t>(w, MAP_W * TILE_SIZE - x);
    h = min<int16_t>(h, MAP_H * TILE_SIZE - y);
    if (w <= 0 || h <= 0) {
        return;
    }

    display.fillRect(x, y, w, h, SKY);

    const int16_t left = max<int16_t>(0, x / TILE_SIZE);
    const int16_t right = min<int16_t>(MAP_W - 1, (x + w - 1) / TILE_SIZE);
    const int16_t top = max<int16_t>(0, y / TILE_SIZE);
    const int16_t bottom = min<int16_t>(MAP_H - 1, (y + h - 1) / TILE_SIZE);

    for (int16_t ty = top; ty <= bottom; ty++) {
        for (int16_t tx = left; tx <= right; tx++) {
            if (LEVEL[ty][tx] == '#') {
                const int16_t px = tx * TILE_SIZE;
                const int16_t py = ty * TILE_SIZE;
                display.fillRect(px, py, TILE_SIZE, TILE_SIZE, BRICK);
                display.drawRect(px, py, TILE_SIZE, TILE_SIZE, DIRT);
            }
        }
    }

    drawCoins(display);
    drawFlag(display);
}

void Mario::drawCoins(LGFX &display) {
    const int16_t coinX[2] = {5 * TILE_SIZE + 8, 12 * TILE_SIZE + 8};
    const int16_t coinY[2] = {8 * TILE_SIZE + 8, 6 * TILE_SIZE + 8};
    for (uint8_t i = 0; i < 2; i++) {
        if (!state.coinCollected[i]) {
            display.fillCircle(coinX[i], coinY[i], 5, COIN);
            display.drawCircle(coinX[i], coinY[i], 5, TFT_BLACK);
        }
    }
}

void Mario::drawFlag(LGFX &display) {
    display.drawFastVLine(17 * TILE_SIZE + 4, 4 * TILE_SIZE, TILE_SIZE * 2, TFT_WHITE);
    display.fillTriangle(17 * TILE_SIZE + 5, 4 * TILE_SIZE,
                         17 * TILE_SIZE + 5, 4 * TILE_SIZE + 16,
                         18 * TILE_SIZE + 1, 4 * TILE_SIZE + 8, FLAG);
}

void Mario::drawEnemy(LGFX &display) {
    if (!state.enemyAlive) {
        return;
    }

    display.fillRoundRect(state.enemy.x, state.enemy.y, state.enemy.w, state.enemy.h, 3, ENEMY);
    display.fillRect(state.enemy.x + 3, state.enemy.y + 4, 2, 2, TFT_BLACK);
    display.fillRect(state.enemy.x + 9, state.enemy.y + 4, 2, 2, TFT_BLACK);
}

void Mario::drawPlayer(LGFX &display) {
    display.fillRect(state.player.x, state.player.y + 5, state.player.w, state.player.h - 5, PLAYER);
    display.fillRect(state.player.x + 2, state.player.y, state.player.w - 4, 6, PLAYER_TRIM);
    display.fillRect(state.player.x + 8, state.player.y + 8, 2, 2, TFT_BLACK);
}

void Mario::drawStatus(LGFX &display) {
    display.fillRect(0, 0, display.width(), 14, TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(1);
    display.setTextDatum(top_left);
    display.drawString("COINS " + static_cast<String>(state.coins) + "/2", 4, 3);
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
