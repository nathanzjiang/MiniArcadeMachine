#include "Mario.h"
#include "assets/MarioAssets.h"

namespace {
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
const float JUMP_SPEED = -8.2f;
const uint32_t JUMP_BUFFER_MS = 120;
const uint32_t COYOTE_MS = 100;
const int16_t PLAYER_HITBOX_W = 12;
const int16_t PLAYER_HITBOX_H = 16;
const int16_t PLAYER_SPRITE_OFFSET_X = -2;
const int16_t PLAYER_SPRITE_OFFSET_Y = 0;
const int16_t ENEMY_HITBOX_W = 14;
const int16_t ENEMY_HITBOX_H = 14;
const int16_t ENEMY_SPRITE_OFFSET_X = -1;
const int16_t ENEMY_SPRITE_OFFSET_Y = -2;
const int16_t COIN_HITBOX_INSET = 2;
const int16_t COIN_HITBOX_SIZE = 12;
}

void Mario::begin(GameContext &ctx) {
    state.currentLevel = 0;
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
        state.playerWasJumping = state.player.vy < -0.1f || !isActorSupported(state.player);
        for (uint8_t i = 0; i < MarioLevels::MAX_COINS; i++) {
            state.coinDirty[i] = false;
        }
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
    const uint8_t levelIndex = state.currentLevel;
    const MarioLevels::Level &level = MarioLevels::LEVELS[levelIndex];

    state = MarioState{};
    state.currentLevel = levelIndex;
    state.player.x = level.playerStart.x * TILE_SIZE;
    state.player.y = level.playerStart.y * TILE_SIZE;
    state.player.w = PLAYER_HITBOX_W;
    state.player.h = PLAYER_HITBOX_H;

    state.enemy.x = level.enemyStart.x * TILE_SIZE;
    state.enemy.y = level.enemyStart.y * TILE_SIZE + 2;
    state.enemy.w = ENEMY_HITBOX_W;
    state.enemy.h = ENEMY_HITBOX_H;
    state.enemy.vx = -0.6f;
    state.enemyDir = -1;
    state.lastPlayerX = state.player.x;
    state.lastPlayerY = state.player.y;
    state.lastEnemyX = state.enemy.x;
    state.lastEnemyY = state.enemy.y;
    state.lastEnemyAlive = state.enemyAlive;
    state.playerWasJumping = state.player.vy < -0.1f || !isActorSupported(state.player);
    state.lastFrameMicros = micros();
    statusDirty = true;
    gameEnded = false;
}

const MarioLevels::Level &Mario::activeLevel() const {
    return MarioLevels::LEVELS[state.currentLevel];
}

void Mario::advanceLevel() {
    if (state.currentLevel + 1 >= MarioLevels::LEVEL_COUNT) {
        mode = MarioMode::WIN;
        screenDirty = true;
        return;
    }

    state.currentLevel++;
    resetLevel();
    screenDirty = true;
    frameDirty = true;
    statusDirty = true;
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
    if (p.vy >= 0.0f && isActorSupported(p)) {
        p.onGround = true;
        p.vy = 0.0f;
        state.lastGroundedMs = ctx.nowMs;
        state.jumpReady = true;
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

    const bool jumpBuffered = ctx.nowMs - state.lastJumpPressedMs <= JUMP_BUFFER_MS;
    const bool canCoyoteJump = ctx.nowMs - state.lastGroundedMs <= COYOTE_MS;
    if (jumpBuffered && canCoyoteJump && state.jumpReady) {
        p.vy = JUMP_SPEED;
        p.onGround = false;
        state.jumpReady = false;
        state.lastJumpPressedMs = 0;
        state.lastGroundedMs = 0;
    }

    if (!p.onGround) {
        p.vy += GRAVITY;
        p.vy = min(p.vy, 7.0f);
    }

    if (p.vy < 0.0f && !ctx.input.b) {
        p.vy *= 0.55f;
    }

    moveActor(p, p.vx, 0.0f);
    if (p.onGround && !isActorSupported(p)) {
        p.onGround = false;
    }
    moveActor(p, 0.0f, p.vy);

    if (p.onGround) {
        state.lastGroundedMs = ctx.nowMs;
        state.jumpReady = true;
    }

    if (p.y > MAP_H * TILE_SIZE) {
        mode = MarioMode::LOSE;
        screenDirty = true;
    }

    const MarioLevels::TilePoint flag = activeLevel().flag;
    if (overlaps(p.x, p.y, p.w, p.h, flag.x * TILE_SIZE, flag.y * TILE_SIZE, TILE_SIZE, TILE_SIZE * 2)) {
        advanceLevel();
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
    if (tileY < 0 || tileY >= MAP_H) {
        return false;
    }
    if (tileX < 0 || tileX >= MAP_W) {
        return true;
    }

    return activeLevel().tiles[tileY][tileX] == '#';
}

bool Mario::isActorSupported(const MarioActor &actor) const {
    const int16_t footY = floor((actor.y + actor.h) / TILE_SIZE);
    const int16_t left = floor((actor.x + 1) / TILE_SIZE);
    const int16_t right = floor((actor.x + actor.w - 2) / TILE_SIZE);

    for (int16_t tx = left; tx <= right; tx++) {
        if (isSolidTile(tx, footY)) {
            return true;
        }
    }

    return false;
}

bool Mario::overlaps(float ax, float ay, int16_t aw, int16_t ah,
                     float bx, float by, int16_t bw, int16_t bh) const {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void Mario::updateCoins() {
    const MarioLevels::Level &level = activeLevel();

    for (uint8_t i = 0; i < level.coinCount; i++) {
        const int16_t coinX = level.coins[i].x * TILE_SIZE + COIN_HITBOX_INSET;
        const int16_t coinY = level.coins[i].y * TILE_SIZE + COIN_HITBOX_INSET;
        if (!state.coinCollected[i] &&
            overlaps(state.player.x, state.player.y, state.player.w, state.player.h,
                     coinX, coinY, COIN_HITBOX_SIZE, COIN_HITBOX_SIZE)) {
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
    const bool playerIsJumping = state.player.vy < -0.1f || !isActorSupported(state.player);
    const bool playerMoved = static_cast<int16_t>(state.lastPlayerX) != static_cast<int16_t>(state.player.x) ||
                             static_cast<int16_t>(state.lastPlayerY) != static_cast<int16_t>(state.player.y);
    const bool playerSpriteChanged = state.playerWasJumping != playerIsJumping;

    if (playerMoved || playerSpriteChanged) {
        pushWorldPatch(display,
                       state.lastPlayerX + PLAYER_SPRITE_OFFSET_X - 3,
                       state.lastPlayerY + PLAYER_SPRITE_OFFSET_Y - 3,
                       MarioAssets::SIZE + 6,
                       MarioAssets::SIZE + 6);

        pushWorldPatch(display,
                       state.player.x + PLAYER_SPRITE_OFFSET_X - 3,
                       state.player.y + PLAYER_SPRITE_OFFSET_Y - 3,
                       MarioAssets::SIZE + 6,
                       MarioAssets::SIZE + 6);
    }

    if (state.lastEnemyAlive) {
        pushWorldPatch(display,
                       state.lastEnemyX + ENEMY_SPRITE_OFFSET_X - 3,
                       state.lastEnemyY + ENEMY_SPRITE_OFFSET_Y - 3,
                       MarioAssets::SIZE + 6,
                       MarioAssets::SIZE + 6);
    }
    if (state.enemyAlive) {
        pushWorldPatch(display,
                       state.enemy.x + ENEMY_SPRITE_OFFSET_X - 3,
                       state.enemy.y + ENEMY_SPRITE_OFFSET_Y - 3,
                       MarioAssets::SIZE + 6,
                       MarioAssets::SIZE + 6);
    }

    const MarioLevels::Level &level = activeLevel();
    for (uint8_t i = 0; i < level.coinCount; i++) {
        if (state.coinDirty[i]) {
            pushWorldPatch(display, level.coins[i].x * TILE_SIZE - 2, level.coins[i].y * TILE_SIZE - 2,
                           MarioAssets::SIZE + 4, MarioAssets::SIZE + 4);
        }
    }

    for (uint8_t i = 0; i < MarioLevels::MAX_COINS; i++) {
        state.coinDirty[i] = false;
    }

    state.lastPlayerX = state.player.x;
    state.lastPlayerY = state.player.y;
    state.lastEnemyX = state.enemy.x;
    state.lastEnemyY = state.enemy.y;
    state.lastEnemyAlive = state.enemyAlive;
    state.playerWasJumping = playerIsJumping;
}

void Mario::pushWorldPatch(LGFX &display, int16_t worldX, int16_t worldY, int16_t w, int16_t h) {
    worldX = max<int16_t>(0, worldX);
    worldY = max<int16_t>(14, worldY);
    w = min<int16_t>(w, MAP_W * TILE_SIZE - worldX);
    h = min<int16_t>(h, MAP_H * TILE_SIZE - worldY);
    if (w <= 0 || h <= 0) {
        return;
    }

    static LGFX_Sprite patch(&display);
    static bool patchReady = false;
    static const int16_t PATCH_W = MarioAssets::SIZE + 8;
    static const int16_t PATCH_H = MarioAssets::SIZE + 8;

    if (!patchReady) {
        patch.setColorDepth(16);
        patchReady = patch.createSprite(PATCH_W, PATCH_H) != nullptr;
    }

    if (!patchReady || w > PATCH_W || h > PATCH_H) {
        display.startWrite();
        display.setClipRect(worldX, worldY, w, h);
        drawWorldRegion(display, worldX, worldY, w, h, true, worldX, worldY);
        display.clearClipRect();
        display.endWrite();
        return;
    }

    patch.fillRect(0, 0, PATCH_W, PATCH_H, SKY);
    drawWorldRegion(patch, worldX, worldY, PATCH_W, PATCH_H, true);
    display.startWrite();
    patch.pushSprite(&display, worldX, worldY);
    display.endWrite();
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
            if (activeLevel().tiles[ty][tx] == '#') {
                const int16_t px = canvasX + tx * TILE_SIZE - worldX;
                const int16_t py = canvasY + ty * TILE_SIZE - worldY;
                MarioAssets::draw(canvas, px, py, MarioAssets::GROUND);
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
    const MarioLevels::Level &level = activeLevel();
    for (uint8_t i = 0; i < level.coinCount; i++) {
        if (!state.coinCollected[i]) {
            MarioAssets::draw(canvas,
                              level.coins[i].x * TILE_SIZE + offsetX,
                              level.coins[i].y * TILE_SIZE + offsetY,
                              MarioAssets::COIN);
        }
    }
}

void Mario::drawFlag(lgfx::LGFXBase &canvas, int16_t offsetX, int16_t offsetY) {
    const MarioLevels::TilePoint flag = activeLevel().flag;
    canvas.drawFastVLine(flag.x * TILE_SIZE + 4 + offsetX, flag.y * TILE_SIZE + offsetY, TILE_SIZE * 2, TFT_WHITE);
    canvas.fillTriangle(flag.x * TILE_SIZE + 5 + offsetX, flag.y * TILE_SIZE + offsetY,
                        flag.x * TILE_SIZE + 5 + offsetX, flag.y * TILE_SIZE + 16 + offsetY,
                        (flag.x + 1) * TILE_SIZE + 1 + offsetX, flag.y * TILE_SIZE + 8 + offsetY, FLAG);
}

void Mario::drawEnemy(lgfx::LGFXBase &canvas, int16_t offsetX, int16_t offsetY) {
    if (!state.enemyAlive) {
        return;
    }

    const int16_t x = state.enemy.x + offsetX + ENEMY_SPRITE_OFFSET_X;
    const int16_t y = state.enemy.y + offsetY + ENEMY_SPRITE_OFFSET_Y;
    MarioAssets::draw(canvas, x, y, MarioAssets::GOOMBA);
}

void Mario::drawPlayer(lgfx::LGFXBase &canvas, int16_t offsetX, int16_t offsetY) {
    const int16_t x = state.player.x + offsetX + PLAYER_SPRITE_OFFSET_X;
    const int16_t y = state.player.y + offsetY + PLAYER_SPRITE_OFFSET_Y;
    const bool jumping = state.player.vy < -0.1f || !isActorSupported(state.player);
    MarioAssets::draw(canvas, x, y, jumping ? MarioAssets::PLAYER_JUMP : MarioAssets::PLAYER_IDLE);
}

void Mario::drawStatus(LGFX &display) {
    LGFX_Sprite status(&display);
    status.setColorDepth(16);
    if (status.createSprite(display.width(), 14) == nullptr) {
        display.fillRect(0, 0, display.width(), 14, TFT_BLACK);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextSize(1);
        display.setTextDatum(top_left);
        display.drawString(static_cast<String>(activeLevel().name) +
                           "  COINS " + static_cast<String>(state.coins) +
                           "/" + static_cast<String>(activeLevel().coinCount), 4, 3);
        return;
    }

    status.fillRect(0, 0, display.width(), 14, TFT_BLACK);
    status.setTextColor(TFT_WHITE, TFT_BLACK);
    status.setTextSize(1);
    status.setTextDatum(top_left);
    status.drawString(static_cast<String>(activeLevel().name) +
                      "  COINS " + static_cast<String>(state.coins) +
                      "/" + static_cast<String>(activeLevel().coinCount), 4, 3);

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
