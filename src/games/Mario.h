#ifndef MINIARCADEMACHINE_MARIO_H
#define MINIARCADEMACHINE_MARIO_H
#include "Game.h"

enum class MarioMode {
    START,
    PLAY,
    LOSE,
    WIN
};

struct MarioActor {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    int16_t w = 12;
    int16_t h = 16;
    bool onGround = false;
};

struct MarioState {
    uint32_t lastFrameMicros = 0;
    uint32_t lastJumpPressedMs = 0;
    uint32_t lastGroundedMs = 0;
    uint8_t coins = 0;
    bool coinCollected[2] = {false, false};
    bool coinDirty[2] = {false, false};
    MarioActor player;
    MarioActor enemy;
    float lastPlayerX = 0.0f;
    float lastPlayerY = 0.0f;
    float lastEnemyX = 0.0f;
    float lastEnemyY = 0.0f;
    bool enemyAlive = true;
    bool lastEnemyAlive = true;
    bool playerWasJumping = false;
    bool jumpReady = true;
    int8_t enemyDir = -1;
};

class Mario : public Game {
public:
    void begin(GameContext &ctx) override;
    void update(GameContext &ctx) override;
    void render(GameContext &ctx) override;

private:
    static const int16_t TILE_SIZE = 16;
    static const int16_t MAP_W = 20;
    static const int16_t MAP_H = 15;
    static const uint16_t TARGET_FPS = 20;
    static const uint32_t FRAME_INTERVAL_MICROS = 1000000UL / TARGET_FPS;

    MarioState state;
    MarioMode mode = MarioMode::START;
    bool frameDirty = true;
    bool screenDirty = true;
    bool statusDirty = true;

    void resetLevel();
    bool shouldStep();
    void stepPhysics(GameContext &ctx);
    void moveActor(MarioActor &actor, float dx, float dy);
    bool isSolidTile(int16_t tileX, int16_t tileY) const;
    bool isActorSupported(const MarioActor &actor) const;
    bool overlaps(float ax, float ay, int16_t aw, int16_t ah,
                  float bx, float by, int16_t bw, int16_t bh) const;
    void updateCoins();
    void updateEnemy();
    void checkPlayerHazards();
    void drawPlayfield(LGFX &display);
    void drawDirtyFrame(LGFX &display);
    void pushWorldPatch(LGFX &display, int16_t worldX, int16_t worldY, int16_t w, int16_t h);
    void drawWorldRegion(lgfx::LGFXBase &canvas, int16_t worldX, int16_t worldY,
                         int16_t w, int16_t h, bool includeActors,
                         int16_t canvasX = 0, int16_t canvasY = 0);
    void drawCoins(lgfx::LGFXBase &canvas, int16_t offsetX = 0, int16_t offsetY = 0);
    void drawFlag(lgfx::LGFXBase &canvas, int16_t offsetX = 0, int16_t offsetY = 0);
    void drawEnemy(lgfx::LGFXBase &canvas, int16_t offsetX = 0, int16_t offsetY = 0);
    void drawPlayer(lgfx::LGFXBase &canvas, int16_t offsetX = 0, int16_t offsetY = 0);
    void drawStatus(LGFX &display);
    void drawCenteredMessage(LGFX &display, const char *title, const char *prompt);
};


#endif //MINIARCADEMACHINE_MARIO_H
