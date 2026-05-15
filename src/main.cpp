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

ArcadeMenuItem menuItems[] = {
    {"SNAKE", "run snake", &snake},
    {"SUPER MARIO", "run super_mario", &mario},
};

ArcadeMenu arcadeMenu(menuItems, sizeof(menuItems) / sizeof(menuItems[0]));

void setup() {
    Serial.begin(115200);
    delay(1000);

    setupDisplay();
    runner.begin(&arcadeMenu);
}

void loop() {
    runner.loop();
}
