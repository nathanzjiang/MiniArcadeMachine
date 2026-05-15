#include "DisplayConfig.h"

LGFX tft;

LGFX::LGFX() {
    {
        auto cfg = bus.config();
        cfg.spi_host = SPI2_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;
        cfg.freq_read = 8000000;
        cfg.spi_3wire = false;
        cfg.use_lock = true;
        cfg.dma_channel = SPI_DMA_CH_AUTO;

        // Arduino Nano ESP32 physical pins as ESP32-S3 GPIO numbers.
        cfg.pin_sclk = 48;  // D13 / SCK
        cfg.pin_mosi = 38;  // D11 / MOSI / SDI
        cfg.pin_miso = 47;  // D12 / MISO / SDO, optional
        cfg.pin_dc = 17;    // D8 / DC / RS / A0

        bus.config(cfg);
        panel.setBus(&bus);
    }

    {
        auto cfg = panel.config();
        cfg.pin_cs = 21;    // D10 / CS
        cfg.pin_rst = 18;   // D9 / RST
        cfg.pin_busy = -1;

        cfg.panel_width = 240;
        cfg.panel_height = 320;
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 0;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits = 1;
        cfg.readable = true;
        cfg.invert = true;
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = false;

        panel.config(cfg);
    }

    setPanel(&panel);
}

void setupDisplay() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}
