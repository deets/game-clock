#include "cyd.hpp"

#include "esp_lcd_ili9341.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include <esp_log.h>
#include <driver/spi_master.h>
#include "esp_lcd_panel_io_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "misc/lv_color.h"
#include "soc/gpio_num.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cstring>

#define Red_LED	4
#define Green_LED 16
#define Blue_LED 17

#define GPIO_OUTPUT_IO_0    CONFIG_GPIO_OUTPUT_0
#define GPIO_OUTPUT_IO_1    CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<Red_LED) | (1ULL<<Green_LED))
#define GPIO_INPUT_IO_0     CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1     CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

const char* TAG = "gameclock";

static lv_display_t *lvgl_disp = NULL;

extern "C" {
  void app_main(void);
  }


void init_spi_display()
{
    ESP_LOGI(TAG, "Turn on the backlight");
    gpio_config_t io_conf = {
        .pin_bit_mask = BIT64(TFT_BL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)TFT_BL, 1);

    ESP_LOGI(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
      .mosi_io_num = TFT_MOSI,
      .miso_io_num = -1,
      .sclk_io_num = TFT_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = TFT_H_RES * 80 * /* 16 bit color */2 };

    spi_bus_initialize(TFT_HOST, &buscfg, SPI_DMA_CH_AUTO);

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = {
      .cs_gpio_num = TFT_CS,
        .dc_gpio_num = TFT_DC,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .on_color_trans_done = nullptr,
        .user_ctx = nullptr,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TFT_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install ili9341 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1, // Shared with Touch reset
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    uint32_t buff_size = TFT_H_RES * TFT_DRAW_BUFF_HEIGHT;;
    const lvgl_port_display_cfg_t disp_cfg =
    {
      .io_handle = io_handle,
      .panel_handle = panel_handle,
     .buffer_size = buff_size,
     .double_buffer = false,
     .hres = TFT_H_RES,
     .vres = TFT_V_RES,
     .monochrome = false,
     .rotation =
         {
             .swap_xy = false,
             .mirror_x = true,
             .mirror_y = false,
         },
     .color_format = LV_COLOR_FORMAT_NATIVE,
     .flags = {
         .buff_dma = false,
         .buff_spiram = false,
         .swap_bytes = true,
         .full_refresh = false,
        }
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
}

static void app_main_display(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* Label */
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_width(label, TFT_H_RES);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label, "Hello World!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);

}

void app_main(void)
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t)0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    init_spi_display();
    app_main_display();
    int cnt = 0;
    while(1) {
        printf("cnt: %d\n", cnt++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level((gpio_num_t)Red_LED, cnt % 2);
        gpio_set_level((gpio_num_t)Green_LED, cnt % 3);
        lvgl_port_lock(0);
        //lv_demo_music();
        lvgl_port_unlock();
    }
}
