#include "ArcadeMenu.h"
#include <string.h>

namespace {
const uint16_t TERM_BG = 0x0000;
const uint16_t TERM_GREEN = 0x07E0;
const uint16_t TERM_DIM = 0x03E0;
const uint16_t TERM_WHITE = 0xFFFF;
const uint16_t TERM_RED = 0xF800;
const int16_t ROW_X = 12;
const int16_t ROW_Y = 90;
const int16_t ROW_H = 18;
const int16_t ROW_GAP = 22;
const int16_t MODAL_X = 36;
const int16_t MODAL_Y = 50;
const int16_t MODAL_W = 248;
const int16_t MODAL_H = 140;
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
    modalOpen = false;
    modalDirty = false;
    ctx.display.fillScreen(TERM_BG);
}

void ArcadeMenu::update(GameContext &ctx) {
    if (modalOpen) {
        if (ctx.input.aRisingEdge || ctx.input.bRisingEdge) {
            closeModal();
        }
        return;
    }

    if (ctx.input.upRisingEdge || ctx.input.joystickRisingEdge == JoystickInput::UP) {
        moveSelection(-1);
    } else if (ctx.input.downRisingEdge || ctx.input.joystickRisingEdge == JoystickInput::DOWN) {
        moveSelection(1);
    }

    if (ctx.input.bRisingEdge) {
        handleSelectedItem();
    }
}

void ArcadeMenu::render(GameContext &ctx) {
    if (screenDirty) {
        drawTerminal(ctx.display);
        screenDirty = false;
        selectionDirty = false;
        return;
    }

    if (modalDirty) {
        drawModal(ctx.display);
        modalDirty = false;
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

void ArcadeMenu::handleSelectedItem() {
    if (itemCount == 0) {
        return;
    }

    const ArcadeMenuItem &item = items[selected];
    if (!isUnlocked(selected)) {
        openModal("ERROR", "cannot open lockfile");
        return;
    }

    if (item.type == ArcadeMenuItemType::Text) {
        openModal(item.filename, item.text);
        return;
    }

    if (item.game != nullptr) {
        nextGame = item.game;
        gameEnded = true;
    }
}

bool ArcadeMenu::isUnlocked(uint8_t index) const {
    if (index >= itemCount) {
        return false;
    }

    const Game *requiredGame = items[index].requiredGame;
    return requiredGame == nullptr || requiredGame->completed;
}

void ArcadeMenu::openModal(const char *title, const char *body) {
    modalTitle = title;
    modalBody = body;
    modalOpen = true;
    modalDirty = true;
}

void ArcadeMenu::closeModal() {
    modalOpen = false;
    modalDirty = false;
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
    display.drawString("root@mini-arcade:/$ cd /games", 12, 42);
    display.drawString("root@mini-arcade:/games$ ls -la", 12, 58);
    display.setTextColor(TERM_DIM, TERM_BG);
    display.drawString("MODE  SIZE  NAME", 34, 74);

    for (uint8_t i = 0; i < itemCount; i++) {
        drawMenuRow(display, i);
    }

    display.setTextColor(TERM_DIM, TERM_BG);
    display.drawString("UP/DOWN SELECT   B OPEN", 12, display.height() - 24);
    display.endWrite();
}

void ArcadeMenu::drawMenuRow(LGFX &display, uint8_t index) {
    if (index >= itemCount) {
        return;
    }

    const bool isSelected = index == selected;
    const bool unlocked = isUnlocked(index);
    const ArcadeMenuItem &item = items[index];
    const int16_t y = ROW_Y + index * ROW_GAP;

    display.fillRect(ROW_X, y - 2, display.width() - 24, ROW_H, isSelected ? TERM_GREEN : TERM_BG);
    display.setTextDatum(top_left);
    display.setTextSize(1);
    display.setTextColor(isSelected ? TERM_BG : TERM_GREEN, isSelected ? TERM_GREEN : TERM_BG);
    display.drawString(isSelected ? ">" : " ", ROW_X + 4, y);
    display.drawString(unlocked ? (item.type == ArcadeMenuItemType::App ? "-rwx" : "-r--") : "LOCK", ROW_X + 22, y);
    display.drawString(item.type == ArcadeMenuItemType::Text ? " 1K" : "24K", ROW_X + 62, y);
    display.drawString(item.filename, ROW_X + 96, y);
    display.setTextColor(isSelected ? TERM_BG : TERM_WHITE, isSelected ? TERM_GREEN : TERM_BG);
    display.drawString(item.label, ROW_X + 210, y);
}

void ArcadeMenu::drawModal(LGFX &display) {
    display.startWrite();
    display.fillRect(MODAL_X - 4, MODAL_Y - 4, MODAL_W + 8, MODAL_H + 8, TERM_BG);
    display.drawRect(MODAL_X, MODAL_Y, MODAL_W, MODAL_H, TERM_GREEN);
    display.drawFastHLine(MODAL_X, MODAL_Y + 22, MODAL_W, TERM_GREEN);

    display.setTextDatum(top_left);
    display.setTextSize(1);
    display.setTextColor(TERM_GREEN, TERM_BG);
    display.drawString(modalTitle == nullptr ? "MESSAGE" : modalTitle, MODAL_X + 8, MODAL_Y + 7);

    display.setTextColor(modalTitle != nullptr && strcmp(modalTitle, "ERROR") == 0 ? TERM_RED : TERM_WHITE, TERM_BG);
    drawWrappedText(display, modalBody == nullptr ? "" : modalBody, MODAL_X + 10, MODAL_Y + 34, 36);

    display.setTextColor(TERM_BG, TERM_GREEN);
    display.fillRect(MODAL_X + 96, MODAL_Y + MODAL_H - 24, 56, 16, TERM_GREEN);
    display.drawString("OK", MODAL_X + 116, MODAL_Y + MODAL_H - 20);
    display.endWrite();
}

void ArcadeMenu::drawWrappedText(LGFX &display, const char *text, int16_t x, int16_t y, int16_t maxCharsPerLine) {
    char line[40];
    uint8_t lineLen = 0;
    int16_t cursorY = y;

    for (const char *p = text; ; p++) {
        const char c = *p;
        const bool flush = c == '\0' || c == '\n' || lineLen >= maxCharsPerLine;

        if (flush) {
            line[lineLen] = '\0';
            display.drawString(line, x, cursorY);
            cursorY += 12;
            lineLen = 0;

            if (c == '\0') {
                break;
            }
            if (c == '\n') {
                continue;
            }
        }

        if (lineLen < sizeof(line) - 1) {
            line[lineLen++] = c;
        }
    }
}
