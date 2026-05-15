#ifndef MINIARCADEMACHINE_MARIO_ASSETS_H
#define MINIARCADEMACHINE_MARIO_ASSETS_H

#include <Arduino.h>
#include "display/DisplayConfig.h"

namespace MarioAssets {
constexpr int16_t SIZE = 16;
constexpr uint16_t TRANSPARENT = 0xFFFF;

constexpr uint16_t hexToRgb565(uint32_t hex) {
    return (((hex >> 16) & 0xF8) << 8) |
           (((hex >> 8) & 0xFC) << 3) |
           ((hex & 0xFF) >> 3);
}

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

constexpr uint16_t SKY = hexToRgb565(0x7DFFFF);
constexpr uint16_t BLACK = hexToRgb565(0x000000);
constexpr uint16_t WHITE = hexToRgb565(0xFFFFFF);
constexpr uint16_t GROUND_LIGHT = hexToRgb565(0xC58E31);
constexpr uint16_t GROUND_MID = hexToRgb565(0xA06020);
constexpr uint16_t GROUND_DARK = hexToRgb565(0x603600);
constexpr uint16_t COIN_YELLOW = hexToRgb565(0xFFE600);
constexpr uint16_t COIN_ORANGE = hexToRgb565(0xFF9E00);
constexpr uint16_t PLAYER_RED = hexToRgb565(0xB92716);
constexpr uint16_t PLAYER_SKIN = hexToRgb565(0xF0AC3F);
constexpr uint16_t PLAYER_GREEN = hexToRgb565(0x736702);
constexpr uint16_t UNKNOWN_COLOR = hexToRgb565(0xFF00FF);
constexpr uint16_t GOOMBA_BROWN = hexToRgb565(0xA14528);
constexpr uint16_t GOOMBA_TAN = hexToRgb565(0xD6A552);

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
    {'g', PLAYER_GREEN},
    {'b', PLAYER_GREEN},
    {'k', BLACK},
    {'w', WHITE}
};

static const char *const PLAYER_IDLE_ROWS[] = {
    ".....rrrrr......",
    "....rrrrrrrrr...",
    "....gggssgs.....",
    "...gsgsssgsss...",
    "...gsggsssgsss..",
    "...ggssssgggg...",
    ".....sssssss....",
    "....ggrggg......",
    "...gggrggrggg...",
    "..ggggrrrrgggg..",
    "..ssgrsrrsrgss..",
    "..sssrrrrrrsss..",
    "..ssrrrrrrrrss..",
    "....rrr..rrr....",
    "...ggg....ggg...",
    "..gggg....gggg.."
};

static const char *const PLAYER_JUMP_ROWS[] = {
    ".............sss",
    "......rrrrr..sss",
    ".....rrrrrrrrrss",
    ".....gggssgs.ggg",
    "....gsgsssgssggg",
    "....gsggsssgsssg",
    "....ggssssggggg.",
    "......sssssssg..",
    "..gggggrgggrg...",
    ".gggggggrgggr..g",
    "ssggggggrrrrr..g",
    "sss.rrgrrsrrsrgg",
    ".s.grrrrrrrrrrgg",
    "..gggrrrrrrrrrgg",
    ".gggrrrrrrr.....",
    ".g..rrrr........"
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

    return UNKNOWN_COLOR;
}

inline void draw(lgfx::LGFXBase &canvas, int16_t x, int16_t y, const Sprite16 &asset) {
    for (int16_t row = 0; row < SIZE; row++) {
        for (int16_t col = 0; col < SIZE; col++) {
            const char key = asset.rows[row][col];
            if (asset.hasTransparency && key == '.') {
                continue;
            }
            const uint16_t color = colorFor(asset, key);
            canvas.drawPixel(x + col, y + row, color);
        }
    }
}
}

#endif // MINIARCADEMACHINE_MARIO_ASSETS_H
