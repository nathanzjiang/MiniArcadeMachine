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
    bool screenDirty = true;

    void moveSelection(int8_t delta);
    void drawTerminal(LGFX &display);
};

#endif // MINIARCADEMACHINE_ARCADE_MENU_H
