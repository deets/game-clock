#include "lvgl.h"

#include <src/lv_api_map_v8.h>
#include <string>
#include <array>
#include <iostream>

extern "C" void ui_setup();

namespace {

namespace styles {
lv_style_t container;
lv_style_t normal;
lv_style_t focused;
lv_style_t disabled;
}

namespace screens {

struct ScreenBase {
  virtual void load_started() {};
  virtual void load_done() {};
  virtual void key_pressed(uint32_t) {};
};

void screen_loaded_cb(lv_event_t * event)
{
  if(ScreenBase* screen_base = (ScreenBase*)lv_event_get_user_data(event))
  {
    switch(lv_event_get_code(event))
    {
    case LV_EVENT_SCREEN_LOADED:
      screen_base->load_done();
      break;
    case LV_EVENT_SCREEN_LOAD_START:
      screen_base->load_started();
    break;
    case LV_EVENT_KEY:
      {
        const auto key = lv_indev_get_key(lv_indev_active());
        if(key)
        {
          screen_base->key_pressed(key);
        }
      }
    break;
    default:
      std::cout << "screen spurious event\n";
      break;
    }
  }
}

namespace main {

struct Elements : public ScreenBase {
  lv_obj_t *screen;
  lv_obj_t *replay;
  lv_obj_t *new_game;
  lv_obj_t *settings;

  void load_started() override {
    for(auto button : {replay, new_game, settings})
    {
      lv_obj_remove_state(button, LV_STATE_FOCUSED);
    }
  }

  void key_pressed(uint32_t key) override {
    std::cout << "main::key_pressed:" << key << "\n";
  }
};

Elements elements;

void setup_button(lv_obj_t* button, const char* text)
{
  lv_obj_remove_style_all(button);                          /*Remove the style coming from the theme*/
  lv_obj_add_style(button, &styles::normal, 0);
  lv_obj_add_style(button, &styles::focused, LV_STATE_FOCUSED);
  lv_obj_add_style(button, &styles::disabled, LV_STATE_DISABLED);
  lv_obj_set_size(button, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_center(button);
  lv_obj_t * buttonlabel = lv_label_create(button);
  lv_label_set_text(buttonlabel, text);
  lv_obj_center(buttonlabel);
}


void setup(Elements& main)
{
  lv_group_add_obj(lv_group_get_default(), main.screen);
  lv_obj_set_style_bg_color(main.screen, lv_color_white(), LV_PART_MAIN);
  lv_obj_add_style(main.screen, &styles::container, 0);
  lv_obj_set_flex_flow(main.screen, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_event_cb(main.screen, screen_loaded_cb, LV_EVENT_SCREEN_LOADED, &main);
  lv_obj_add_event_cb(main.screen, screen_loaded_cb, LV_EVENT_SCREEN_LOAD_START, &main);
  lv_obj_add_event_cb(main.screen, screen_loaded_cb, LV_EVENT_KEY, &main);
  main.replay = lv_button_create(main.screen);
  setup_button(main.replay, "Replay");
  main.new_game = lv_button_create(main.screen);
  setup_button(main.new_game, "New");
  main.settings = lv_button_create(main.screen);
  setup_button(main.settings, "Settings");
}

} // namespace main

} // namespace screens


void ui_setup_styles()
{
  lv_style_set_pad_row(&styles::container, 0);

  lv_style_init(&styles::normal);
  lv_style_set_radius(&styles::normal, 8);
  lv_style_set_bg_opa(&styles::normal, LV_OPA_100);
  lv_style_set_bg_color(&styles::normal, lv_color_white());

  lv_style_set_border_opa(&styles::normal, LV_OPA_100);
  lv_style_set_border_width(&styles::normal, 2);
  lv_style_set_border_color(&styles::normal, lv_color_black());

  lv_style_set_margin_left(&styles::normal, 2);
  lv_style_set_margin_right(&styles::normal, 2);
  lv_style_set_margin_top(&styles::normal, 2);
  lv_style_set_margin_bottom(&styles::normal, 2);

  lv_style_set_text_color(&styles::normal, lv_color_black());
  lv_style_set_pad_all(&styles::normal, 10);

  lv_style_init(&styles::focused);
  lv_style_set_text_color(&styles::focused, lv_color_white());
  lv_style_set_bg_color(&styles::focused, lv_color_black());

  lv_style_init(&styles::disabled);
  lv_style_set_bg_color(&styles::disabled, lv_palette_lighten(LV_PALETTE_GREY, 3));
  lv_style_set_text_color(&styles::disabled, lv_palette_main(LV_PALETTE_GREY));

}



} // namespace

void ui_setup()
{
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);
  ui_setup_styles();
  screens::main::elements.screen = lv_obj_create(NULL);
  screens::main::setup(screens::main::elements);

  lv_screen_load_anim(screens::main::elements.screen, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
}
