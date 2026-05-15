#ifndef MINIARCADEMACHINE_ARCADE_MENU_H
#define MINIARCADEMACHINE_ARCADE_MENU_H

#include "Game.h"

enum class ArcadeMenuItemType : uint8_t {
    App,
    Text
};

struct ArcadeMenuItem {
    const char *label;
    const char *filename;
    ArcadeMenuItemType type;
    Game *game;
    const char *text;
    const Game *requiredGame;
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
    bool modalOpen = false;
    bool modalDirty = false;
    const char *modalTitle = nullptr;
    const char *modalBody = nullptr;

    void moveSelection(int8_t delta);
    void handleSelectedItem();
    bool isUnlocked(uint8_t index) const;
    void openModal(const char *title, const char *body);
    void closeModal();
    void drawTerminal(LGFX &display);
    void drawMenuRow(LGFX &display, uint8_t index);
    void drawModal(LGFX &display);
    void drawWrappedText(LGFX &display, const char *text, int16_t x, int16_t y, int16_t maxCharsPerLine);
};

#endif // MINIARCADEMACHINE_ARCADE_MENU_H
