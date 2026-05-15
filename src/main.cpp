#include "display/DisplayConfig.h"
#include "games/backend/GameRunner.h"
#include "InputManager.h"
#include "games/SpriteDemoGame.h"
#include "games/Snake.h"

InputManager input;
GameRunner runner(tft, input);
SpriteDemoGame spriteDemo;
Snake snake;

void setup() {
    Serial.begin(115200);
    delay(1000);

    setupDisplay();
    runner.begin(&snake);
}

void loop() {
    runner.loop();
}