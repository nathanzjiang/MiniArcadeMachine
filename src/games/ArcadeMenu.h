#ifndef MINIARCADEMACHINE_ARCADE_MENU_H
#define MINIARCADEMACHINE_ARCADE_MENU_H

#include "Game.h"

struct ArcadeMenuItem {
    const char *label;
    const char *command;
    Game *game;
};

class ArcadeMenu : public Game {
public:
    ArcadeMenu(const ArcadeMenuItem *items, uint8_t itemCount);

    void begin(GameContext &ctx) override;
    void update(GameContext &ctx) override;
    void render(GameContext &ctx) override;

private:
    const ArcadeMenuItem *items;
    uint8_t itemCount;
    uint8_t selected = 0;
    uint8_t previousSelected = 0;
    bool screenDirty = true;
    bool selectionDirty = false;

    void moveSelection(int8_t delta);
    void drawTerminal(LGFX &display);
    void drawMenuRow(LGFX &display, uint8_t index);
};

#endif // MINIARCADEMACHINE_ARCADE_MENU_H
