#include "pebble.h"

static Window *window;
static Layer *square_layer;

// Timers can be canceled with `app_timer_cancel()`
static AppTimer *timer;


// functions
static void init();
static void deinit();
static void update_square_layer(Layer*, GContext*);
static void timer_callback(void*);
static void click_config_provider(Window*);

static void button_up_handler(ClickRecognizerRef, void*);


// square points
static const GPathInfo SQUARE_POINTS = {
  4,
  (GPoint []) {
    {-10, -10},
    {-10,  10},
    { 10,  10},
    { 10, -10}
  }
};

static GPath *square_path;

static int multiplier = 1;



int main(void) {
  init();

  app_event_loop();

  deinit();
}


static void update_square_layer(Layer *layer, GContext* ctx) {
  static unsigned int angle = 0;

  gpath_rotate_to(square_path, (TRIG_MAX_ANGLE / 360) * angle);

  angle = (angle + (5 * multiplier)) % 360;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, square_path);
}


static void timer_callback(void *context) {
  layer_mark_dirty(square_layer);

  const uint32_t timeout_ms = 50;
  timer = app_timer_register(timeout_ms, timer_callback, NULL);
}


static void click_config_provider(Window *window) {
  window_raw_click_subscribe(BUTTON_ID_UP, NULL, button_up_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_DOWN, NULL, button_up_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_SELECT, NULL, button_up_handler, NULL);
}



static void button_up_handler(ClickRecognizerRef recognizer, void *context) {
  ButtonId button_id = click_recognizer_get_button_id(recognizer);
  if(button_id == BUTTON_ID_UP)
    multiplier -= 1;
  else if(button_id == BUTTON_ID_DOWN)
    multiplier += 1;
  else if(button_id == BUTTON_ID_SELECT)
    multiplier = 0;
}



static void init(void) {
  window = window_create();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  square_layer = layer_create(bounds);
  layer_set_update_proc(square_layer, update_square_layer);
  layer_add_child(window_layer, square_layer);

  square_path = gpath_create(&SQUARE_POINTS);
  gpath_move_to(square_path, grect_center_point(&bounds));

  const bool animated = true;
  window_stack_push(window, animated);

  const uint32_t timeout_ms = 50;
  timer = app_timer_register(timeout_ms, timer_callback, NULL);

  window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider);
}

static void deinit(void) {
  gpath_destroy(square_path);

  layer_destroy(square_layer);
  window_destroy(window);
}

