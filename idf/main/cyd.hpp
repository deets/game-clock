// Copyright: 2024, Diez B. Roggisch, Berlin, all rights reserved
#pragma once
#include <driver/spi_master.h>
// https://randomnerdtutorials.com/esp32-cheap-yellow-display-cyd-pinout-esp32-2432s028r/#display-pins
constexpr auto TFT_MOSI = 13;
constexpr auto TFT_MISO = 12;
constexpr auto TFT_CLK = 14;
constexpr auto TFT_CS = 15;
constexpr auto TFT_DC = 2;
constexpr auto TFT_BL = 21;

constexpr auto TFT_H_RES = 240;
constexpr auto TFT_V_RES = 320;
constexpr auto TFT_DRAW_BUFF_HEIGHT = 100;
constexpr auto TFT_BIT_PER_PIXEL = 16;
constexpr auto TFT_HOST = SPI2_HOST;
