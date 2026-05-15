#include "ArcadeMenu.h"

namespace {
const uint16_t TERM_BG = 0x0000;
const uint16_t TERM_GREEN = 0x07E0;
const uint16_t TERM_DIM = 0x03E0;
const uint16_t TERM_WHITE = 0xFFFF;
const int16_t ROW_X = 12;
const int16_t ROW_Y = 90;
const int16_t ROW_H = 18;
const int16_t ROW_GAP = 22;
}

ArcadeMenu::ArcadeMenu(const ArcadeMenuItem *items, uint8_t itemCount)
    : items(items), itemCount(itemCount) {
}

void ArcadeMenu::begin(GameContext &ctx) {
    gameEnded = false;
    nextGame = nullptr;
    selected = 0;
    previousSelected = 0;
    screenDirty = true;
    selectionDirty = false;
    ctx.display.fillScreen(TERM_BG);
}

void ArcadeMenu::update(GameContext &ctx) {
    if (ctx.input.upRisingEdge || ctx.input.joystickRisingEdge == JoystickInput::UP) {
        moveSelection(-1);
    } else if (ctx.input.downRisingEdge || ctx.input.joystickRisingEdge == JoystickInput::DOWN) {
        moveSelection(1);
    }

    if (ctx.input.aRisingEdge || ctx.input.bRisingEdge) {
        if (itemCount > 0 && items[selected].game != nullptr) {
            nextGame = items[selected].game;
            gameEnded = true;
        }
    }
}

void ArcadeMenu::render(GameContext &ctx) {
    if (screenDirty) {
        drawTerminal(ctx.display);
        screenDirty = false;
        selectionDirty = false;
        return;
    }

    if (selectionDirty) {
        ctx.display.startWrite();
        drawMenuRow(ctx.display, previousSelected);
        drawMenuRow(ctx.display, selected);
        ctx.display.endWrite();
        selectionDirty = false;
    }
}

void ArcadeMenu::moveSelection(int8_t delta) {
    if (itemCount == 0) {
        return;
    }

    previousSelected = selected;
    selected = (selected + itemCount + delta) % itemCount;
    selectionDirty = true;
}

void ArcadeMenu::drawTerminal(LGFX &display) {
    display.startWrite();
    display.fillScreen(TERM_BG);
    display.drawRect(4, 4, display.width() - 8, display.height() - 8, TERM_GREEN);
    display.drawFastHLine(4, 28, display.width() - 8, TERM_GREEN);

    display.setTextDatum(top_left);
    display.setTextSize(1);
    display.setTextColor(TERM_GREEN, TERM_BG);
    display.drawString("MINI-ARC OS v0.1", 10, 10);
    display.setTextColor(TERM_DIM, TERM_BG);
    display.drawString("BOOT: OK   INPUT: JOYSTICK", 148, 10);

    display.setTextColor(TERM_GREEN, TERM_BG);
    display.drawString("root@mini-arcade:/$ cd /games", 12, 42);
    display.drawString("root@mini-arcade:/games$ ls -la", 12, 58);
    display.setTextColor(TERM_DIM, TERM_BG);
    display.drawString("MODE  SIZE  NAME", 34, 74);

    for (uint8_t i = 0; i < itemCount; i++) {
        drawMenuRow(display, i);
    }

    display.setTextColor(TERM_DIM, TERM_BG);
    display.drawString("UP/DOWN SELECT   A/B EXEC", 12, display.height() - 24);
    display.endWrite();
}

void ArcadeMenu::drawMenuRow(LGFX &display, uint8_t index) {
    if (index >= itemCount) {
        return;
    }

    const bool isSelected = index == selected;
    const int16_t y = ROW_Y + index * ROW_GAP;

    display.fillRect(ROW_X, y - 2, display.width() - 24, ROW_H, isSelected ? TERM_GREEN : TERM_BG);
    display.setTextDatum(top_left);
    display.setTextSize(1);
    display.setTextColor(isSelected ? TERM_BG : TERM_GREEN, isSelected ? TERM_GREEN : TERM_BG);
    display.drawString(isSelected ? ">" : " ", ROW_X + 4, y);
    display.drawString("-rwx", ROW_X + 22, y);
    display.drawString(index == 0 ? " 8K" : "24K", ROW_X + 62, y);
    display.drawString(items[index].command, ROW_X + 96, y);
    display.setTextColor(isSelected ? TERM_BG : TERM_WHITE, isSelected ? TERM_GREEN : TERM_BG);
    display.drawString(items[index].label, ROW_X + 210, y);
}
