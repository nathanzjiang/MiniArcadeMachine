#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

const uint8_t TFT_CS = D10;
const uint8_t TFT_DC = D8;   // Some ILI9341 boards label this RS or A0.
const uint8_t TFT_RST = D9;

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

uint32_t frameCount = 0;
uint32_t animationFrame = 0;
uint32_t lastFrameMicros = 0;
uint32_t lastFpsMillis = 0;
float fps = 0.0f;
int16_t lastX = -1;
int16_t lastY = -1;
int16_t spriteX = 10;
int16_t spriteY = 44;
int16_t spriteDx = 2;
int16_t spriteDy = 2;

const int16_t spriteSize = 40;
const int16_t playTop = 34;
const uint16_t targetFps = 30;
const uint32_t frameIntervalMicros = 1000000UL / targetFps;

uint16_t colorWheel(uint8_t value) {
    value = 255 - value;
    if (value < 85) {
        return tft.color565(255 - value * 3, 0, value * 3);
    }
    if (value < 170) {
        value -= 85;
        return tft.color565(0, value * 3, 255 - value * 3);
    }
    value -= 170;
    return tft.color565(value * 3, 255 - value * 3, 0);
}

void drawFpsOverlay() {
    tft.fillRect(0, 0, tft.width(), 30, ILI9341_BLACK);
    tft.setCursor(6, 6);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.print("FPS ");
    tft.print(fps, 1);
    tft.drawFastHLine(0, 31, tft.width(), ILI9341_DARKGREY);
}

void moveSprite() {
    const int16_t width = tft.width();
    const int16_t height = tft.height();

    spriteX += spriteDx;
    spriteY += spriteDy;

    if (spriteX <= 0 || spriteX >= width - spriteSize) {
        spriteDx = -spriteDx;
        spriteX += spriteDx;
    }

    if (spriteY <= playTop || spriteY >= height - spriteSize) {
        spriteDy = -spriteDy;
        spriteY += spriteDy;
    }
}

void drawFrame() {
    tft.startWrite();

    if (lastX >= 0) {
        tft.writeFillRect(lastX - 2, lastY - 2, spriteSize + 4, spriteSize + 4, ILI9341_BLACK);
    }

    tft.writeFillRect(spriteX, spriteY, spriteSize, spriteSize, ILI9341_BLUE);
    tft.writeFastHLine(spriteX, spriteY, spriteSize, ILI9341_WHITE);
    tft.writeFastHLine(spriteX, spriteY + spriteSize - 1, spriteSize, ILI9341_WHITE);
    tft.writeFastVLine(spriteX, spriteY, spriteSize, ILI9341_WHITE);
    tft.writeFastVLine(spriteX + spriteSize - 1, spriteY, spriteSize, ILI9341_WHITE);

    tft.endWrite();

    lastX = spriteX;
    lastY = spriteY;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    SPI.begin();
    tft.begin(10000000);
    tft.invertDisplay(true);
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    drawFpsOverlay();

    Serial.println("Adafruit ILI9341 FPS test started");
    Serial.println("SPI speed: 10 MHz");
    Serial.println("Mode: dirty rectangle sprite redraw");
    Serial.println("Frame cap: 30 FPS");

    lastFrameMicros = micros();
    lastFpsMillis = millis();
}

void loop() {
    uint32_t nowMicros = micros();
    if (nowMicros - lastFrameMicros < frameIntervalMicros) {
        return;
    }
    lastFrameMicros += frameIntervalMicros;

    moveSprite();
    drawFrame();
    frameCount++;
    animationFrame++;

    uint32_t now = millis();
    uint32_t elapsed = now - lastFpsMillis;
    if (elapsed >= 1000) {
        fps = frameCount * 1000.0f / elapsed;
        frameCount = 0;
        lastFpsMillis = now;

        Serial.print("FPS: ");
        Serial.println(fps, 1);
        drawFpsOverlay();
    }
}
