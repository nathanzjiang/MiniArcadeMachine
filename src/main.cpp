#include "display/DisplayConfig.h"
#include "games/backend/GameRunner.h"
#include "InputManager.h"
#include "games/SpriteDemoGame.h"
#include "games/Snake.h"
#include "games/Mario.h"
#include "games/ArcadeMenu.h"

InputManager input;
GameRunner runner(tft, input);
SpriteDemoGame spriteDemo;
Snake snake;
Mario mario;

const char HELP_TEXT[] =
    "Welcome to MINI-ARC OS.\n"
    "Run snake.app first.\n"
    "Beat Snake to unlock Mario.\n"
    "Beat Mario to unlock the secret file.";

const char SECRET_TEXT[] =
    "ACCESS GRANTED.\n"
    "You cleared every available game.";

ArcadeMenuItem menuItems[] = {
    {"README", "help.txt", ArcadeMenuItemType::Text, nullptr, HELP_TEXT, nullptr},
    {"SNAKE", "snake.app", ArcadeMenuItemType::App, &snake, nullptr, nullptr},
    {"SUPER MARIO", "mario.app", ArcadeMenuItemType::App, &mario, nullptr, &snake},
    {"SECRET", "SECRET.txt", ArcadeMenuItemType::Text, nullptr, SECRET_TEXT, &mario},
};

ArcadeMenu arcadeMenu(menuItems, sizeof(menuItems) / sizeof(menuItems[0]));

void setup() {
    Serial.begin(115200);
    delay(1000);

    snake.exitGame = &arcadeMenu;
    mario.exitGame = &arcadeMenu;

    setupDisplay();
    runner.begin(&arcadeMenu);
}

void loop() {
    runner.loop();
}
