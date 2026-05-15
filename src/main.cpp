#include "display/DisplayConfig.h"
#include "games/backend/GameRunner.h"
#include "InputManager.h"
#include "games/SpriteDemoGame.h"
#include "games/Snake.h"
#include "games/Mario.h"

InputManager input;
GameRunner runner(tft, input);
SpriteDemoGame spriteDemo;
Snake snake;
Mario mario;

void setup() {
    Serial.begin(115200);
    delay(1000);

    setupDisplay();
    runner.begin(&mario);
}

void loop() {
    runner.loop();
}
