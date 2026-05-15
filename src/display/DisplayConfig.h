#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 panel;
    lgfx::Bus_SPI bus;

public:
    LGFX();
};

extern LGFX tft;

void setupDisplay();

#endif
