#ifndef MINIARCADEMACHINE_MARIO_ASSETS_H
#define MINIARCADEMACHINE_MARIO_ASSETS_H

#include <Arduino.h>
#include "display/DisplayConfig.h"

namespace MarioAssets {
constexpr int16_t SIZE = 16;
constexpr uint16_t TRANSPARENT = 0xFFFF;

enum class SpriteId {
    Ground,
    Coin,
    PlayerIdle,
    PlayerJump,
    Goomba
};

struct PaletteEntry {
    char key;
    uint16_t color;
};

struct Sprite16 {
    const PaletteEntry *palette;
    uint8_t paletteSize;
    const char *const *rows;
    bool hasTransparency;
};

constexpr uint16_t SKY = 0x7DFF;
constexpr uint16_t BLACK = 0x0000;
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t GROUND_LIGHT = 0xC3A6;
constexpr uint16_t GROUND_MID = 0xA303;
constexpr uint16_t GROUND_DARK = 0x61A0;
constexpr uint16_t COIN_YELLOW = 0xFFE0;
constexpr uint16_t COIN_ORANGE = 0xFD20;
constexpr uint16_t PLAYER_RED = 0xD800;
constexpr uint16_t PLAYER_SKIN = 0xFD6C;
constexpr uint16_t PLAYER_BLUE = 0x025F;
constexpr uint16_t GOOMBA_BROWN = 0xA145;
constexpr uint16_t GOOMBA_TAN = 0xD3A5;

static const PaletteEntry GROUND_PALETTE[] = {
    {'.', GROUND_DARK},
    {'l', GROUND_LIGHT},
    {'m', GROUND_MID},
    {'d', GROUND_DARK},
    {'k', BLACK}
};

static const char *const GROUND_ROWS[] = {
    "dddddddddddddddd",
    "dlllllllllllllld",
    "dlmmmmmmmmmmmldd",
    "dlmllllmllllmldd",
    "dlmlkkkmlkkkmldd",
    "dlmllllmllllmldd",
    "dlmmmmmmmmmmmldd",
    "dddddddddddddddd",
    "dlllllllllllllld",
    "dlmmmmmmmmmmmldd",
    "dlmllllmllllmldd",
    "dlmlkkkmlkkkmldd",
    "dlmllllmllllmldd",
    "dlmmmmmmmmmmmldd",
    "dlllllllllllllld",
    "dddddddddddddddd"
};

static const PaletteEntry COIN_PALETTE[] = {
    {'.', TRANSPARENT},
    {'y', COIN_YELLOW},
    {'o', COIN_ORANGE},
    {'k', BLACK}
};

static const char *const COIN_ROWS[] = {
    "................",
    "................",
    ".....kkkkkk.....",
    "....kyyyyyyk....",
    "...kyyyyyyyyk...",
    "...kyyooyyyyk...",
    "..kyyoooyyyyk...",
    "..kyyoooyyyyk...",
    "..kyyoooyyyyk...",
    "...kyyooyyyyk...",
    "...kyyyyyyyyk...",
    "....kyyyyyyk....",
    ".....kkkkkk.....",
    "................",
    "................",
    "................"
};

static const PaletteEntry PLAYER_PALETTE[] = {
    {'.', TRANSPARENT},
    {'r', PLAYER_RED},
    {'s', PLAYER_SKIN},
    {'b', PLAYER_BLUE},
    {'k', BLACK},
    {'w', WHITE}
};

static const char *const PLAYER_IDLE_ROWS[] = {
    ".....rrrr.......",
    "....rrrrrr......",
    "...rrssskk......",
    "...rsssssk......",
    "...rsskssk......",
    "....ssss........",
    "...rrbrrr.......",
    "..rrrbbrrr......",
    ".rr.rbb.r.......",
    "....bbbb........",
    "...bbbbbb.......",
    "...bb..bb.......",
    "..kkk..kkk......",
    ".kkkk..kkkk.....",
    "..kk....kk......",
    ".kkk....kkk....."
};

static const char *const PLAYER_JUMP_ROWS[] = {
    ".....rrrr.......",
    "....rrrrrr......",
    "...rrssskk......",
    "...rsssssk......",
    "...rsskssk......",
    "....ssss........",
    "...rrbrrr..r....",
    "..rrrbbrrrr.....",
    ".rr.rbb.........",
    "....bbbb........",
    "...bbbbbb.......",
    "..bbb..bb.......",
    ".kkk...kkk......",
    "kkkk............",
    "kkk.....kk......",
    "kk......kkk....."
};

static const PaletteEntry GOOMBA_PALETTE[] = {
    {'.', TRANSPARENT},
    {'b', GOOMBA_BROWN},
    {'t', GOOMBA_TAN},
    {'k', BLACK},
    {'w', WHITE}
};

static const char *const GOOMBA_ROWS[] = {
    "................",
    "................",
    ".....bbbbbb.....",
    "...bbbbbbbbbb...",
    "..bbbbbbbbbbbb..",
    ".bbbbbbbbbbbbbb.",
    ".bbbkkbbbbkkbbb.",
    "bbbbkwbbbbwkbbbb",
    "bbbbkkkkkkkkbbbb",
    ".bbbbbbbbbbbbbb.",
    "..bbttttttttbb..",
    "...tttttttttt...",
    "..kkk......kkk..",
    ".kkkk......kkkk.",
    "..kkk....kkk....",
    ".kkkk....kkkk..."
};

static const Sprite16 GROUND = {GROUND_PALETTE, 5, GROUND_ROWS, false};
static const Sprite16 COIN = {COIN_PALETTE, 4, COIN_ROWS, true};
static const Sprite16 PLAYER_IDLE = {PLAYER_PALETTE, 6, PLAYER_IDLE_ROWS, true};
static const Sprite16 PLAYER_JUMP = {PLAYER_PALETTE, 6, PLAYER_JUMP_ROWS, true};
static const Sprite16 GOOMBA = {GOOMBA_PALETTE, 5, GOOMBA_ROWS, true};

inline const Sprite16 &sprite(SpriteId id) {
    switch (id) {
        case SpriteId::Ground:
            return GROUND;
        case SpriteId::Coin:
            return COIN;
        case SpriteId::PlayerJump:
            return PLAYER_JUMP;
        case SpriteId::Goomba:
            return GOOMBA;
        case SpriteId::PlayerIdle:
        default:
            return PLAYER_IDLE;
    }
}

inline uint16_t colorFor(const Sprite16 &asset, char key) {
    for (uint8_t i = 0; i < asset.paletteSize; i++) {
        if (asset.palette[i].key == key) {
            return asset.palette[i].color;
        }
    }

    return TRANSPARENT;
}

inline void draw(lgfx::LGFXBase &canvas, int16_t x, int16_t y, const Sprite16 &asset) {
    for (int16_t row = 0; row < SIZE; row++) {
        for (int16_t col = 0; col < SIZE; col++) {
            const uint16_t color = colorFor(asset, asset.rows[row][col]);
            if (asset.hasTransparency && color == TRANSPARENT) {
                continue;
            }
            canvas.drawPixel(x + col, y + row, color);
        }
    }
}
}

#endif // MINIARCADEMACHINE_MARIO_ASSETS_H
