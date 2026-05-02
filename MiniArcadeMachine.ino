#define USER_SETUP_LOADED

#define ILI9341_DRIVER

#define USE_HSPI_PORT
#define TFT_CS D10
#define TFT_RST D9
#define TFT_DC D8
#define TFT_MOSI D11
#define TFT_MISO D12
#define TFT_SCLK D13

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY 27000000
#define SPI_READ_FREQUENCY 16000000

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

const uint16_t testColors[] = {
    TFT_RED,
    TFT_GREEN,
    TFT_BLUE,
    TFT_CYAN,
    TFT_MAGENTA,
    TFT_YELLOW,
    TFT_WHITE,
    TFT_BLACK
};

void drawHeader(const char *title) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(title, tft.width() / 2, 6, 2);
}

void drawColorBars() {
    drawHeader("TFT_eSPI HSPI test");

    const int barTop = 30;
    const int barHeight = (tft.height() - barTop) / 8;

    for (int i = 0; i < 8; i++) {
    tft.fillRect(0, barTop + i * barHeight, tft.width(), barHeight, testColors[i]);
}

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("CS D10  RST D9  DC/RS D8", tft.width() / 2, tft.height() / 2, 2);
}

void drawGeometry() {
    drawHeader("Geometry");

    int cx = tft.width() / 2;
    int cy = tft.height() / 2;
    int radius = min(tft.width(), tft.height()) / 3;

    tft.drawRect(0, 24, tft.width(), tft.height() - 24, TFT_WHITE);
    tft.drawLine(0, 24, tft.width() - 1, tft.height() - 1, TFT_RED);
    tft.drawLine(tft.width() - 1, 24, 0, tft.height() - 1, TFT_GREEN);
    tft.drawCircle(cx, cy, radius, TFT_CYAN);
    tft.fillCircle(cx, cy, radius / 3, TFT_YELLOW);
}

void drawTextTest() {
    drawHeader("Text");

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Nano ESP32 + TFT_eSPI", 8, 36, 2);

    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("SPI: HSPI", 8, 62, 4);

    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("MOSI D11  MISO D12  SCK D13", 8, 104, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("If colors and text are stable,", 8, 132, 2);
    tft.drawString("the display wiring is working.", 8, 152, 2);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    Serial.println("TFT_eSPI HSPI display test started");
    Serial.println("CS=D10, RST=D9, DC/RS=D8, MOSI=D11, MISO=D12, SCK=D13");
}

void loop() {
    drawColorBars();
    delay(2000);

    drawGeometry();
    delay(2000);

    drawTextTest();
    delay(2000);
}
