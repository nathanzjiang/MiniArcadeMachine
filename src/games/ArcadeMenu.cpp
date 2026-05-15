#include "ArcadeMenu.h"

namespace {
const uint16_t TERM_BG = 0x0000;
const uint16_t TERM_GREEN = 0x07E0;
const uint16_t TERM_DIM = 0x03E0;
const uint16_t TERM_WHITE = 0xFFFF;
}

ArcadeMenu::ArcadeMenu(const ArcadeMenuItem *items, uint8_t itemCount)
    : items(items), itemCount(itemCount) {
}

void ArcadeMenu::begin(GameContext &ctx) {
    gameEnded = false;
    nextGame = nullptr;
    selected = 0;
    screenDirty = true;
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
    if (!screenDirty) {
        return;
    }

    drawTerminal(ctx.display);
    screenDirty = false;
}

void ArcadeMenu::moveSelection(int8_t delta) {
    if (itemCount == 0) {
        return;
    }

    selected = (selected + itemCount + delta) % itemCount;
    screenDirty = true;
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
    display.drawString("> scan /games", 12, 42);
    display.drawString("FOUND " + static_cast<String>(itemCount) + " EXECUTABLES", 12, 58);

    int16_t y = 88;
    for (uint8_t i = 0; i < itemCount; i++) {
        const bool isSelected = i == selected;
        display.setTextColor(isSelected ? TERM_BG : TERM_GREEN, isSelected ? TERM_GREEN : TERM_BG);
        display.fillRect(12, y - 2, display.width() - 24, 16, isSelected ? TERM_GREEN : TERM_BG);
        display.drawString(isSelected ? "> " : "  ", 16, y);
        display.drawString(items[i].command, 34, y);
        display.setTextColor(isSelected ? TERM_BG : TERM_WHITE, isSelected ? TERM_GREEN : TERM_BG);
        display.drawString(items[i].label, 160, y);
        y += 22;
    }

    display.setTextColor(TERM_DIM, TERM_BG);
    display.drawString("UP/DOWN SELECT   A/B RUN", 12, display.height() - 24);
    display.endWrite();
}
