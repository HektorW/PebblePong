#include "pebble.h"

// globals
static Window* window;
static Layer* main_layer;

static AppTimer* timer;

// window
static int16_t window_w;
static int16_t window_h;

// paddle
static const int16_t paddle_w = 10;
static const int16_t paddle_h = 30;
static const int16_t paddle_vel = 4;


// paddle positions
static int16_t p1_x = 5;
static int16_t p2_x = 130;
static int16_t p1_y = 0;
static int16_t p2_y = 0;
static int16_t p1_yvel = 0;
static int16_t p2_yvel = 0;

// ball
static const int16_t ball_s = 10;
static int16_t ball_x = 0;
static int16_t ball_y = 0;
static int16_t ball_xvel = 0;
static int16_t ball_yvel = 0;
static int16_t ball_vel = 4;


static bool running = false;
static const uint32_t timeout_ms = 100;


// functions
static void init();
static void deinit();
static void click_config_provider(Window*);

static void start();
static void pause();

static void draw(Layer*, GContext*);
static void draw_paddles(GContext*);
static void draw_ball(GContext*);

static void update(void*);
static void update_positions();
static int16_t update_paddle(int16_t, int16_t);


static void spawn_ball();

// button handlers
static void button_pressed_handler(ClickRecognizerRef, void*);
static void button_released_handler(ClickRecognizerRef, void*);


int main() {
  init();

  app_event_loop();

  deinit();
}




static void init() {
  window = window_create();
  window_stack_push(window, true /* animated */);

  Layer* root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  window_w = bounds.size.w;
  window_h = bounds.size.h;

  p1_x = 0 + 5;
  p2_x = window_w - 5 - paddle_w;


  // setup main layer
  main_layer = layer_create(bounds);
  layer_set_update_proc(main_layer, draw);
  layer_add_child(root_layer, main_layer);

  // bind click events
  window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

  spawn_ball();
  start();
}

static void deinit() {
  window_destroy(window);
  layer_destroy(main_layer);
}

static void click_config_provider(Window* window) {
  window_raw_click_subscribe(BUTTON_ID_UP, button_pressed_handler, button_released_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_DOWN, button_pressed_handler, button_released_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_SELECT, button_pressed_handler, button_released_handler, NULL);
}




static void start() {
  running = true;
  timer = app_timer_register(timeout_ms, update, NULL);
}
static void pause() {
  running = false;
  app_timer_cancel(timer);
}


static void spawn_ball() {
  ball_x = (window_w / 2) - (ball_s / 2);
  ball_y = (window_h / 2) - (ball_s / 2);

  ball_xvel = 1 - (2 * (rand() % 2));
  ball_yvel = 1 - (rand() % 3);
}



static void draw(Layer* layer, GContext* ctx) { 
  draw_paddles(ctx);
  draw_ball(ctx);
}


static void draw_paddles(GContext* ctx) {
  graphics_fill_rect(ctx, GRect(p1_x, p1_y, paddle_w, paddle_h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(p2_x, p2_y, paddle_w, paddle_h), 0, GCornerNone);
}
static void draw_ball(GContext* ctx) {
  graphics_fill_rect(ctx, GRect(ball_x, ball_y, ball_s, ball_s), 0, GCornerNone);
}





static void update(void* data) {
  update_positions();


  // schedule draw
  layer_mark_dirty(main_layer);

  timer = app_timer_register(timeout_ms, update, NULL);
}

static void update_positions() {
  p1_y = update_paddle(p1_y, p1_yvel);
  p2_y = update_paddle(p2_y, p2_yvel);

  ball_x += ball_xvel;
  ball_y += ball_yvel;

  if(ball_y < 0) {
    ball_y = 0;
    ball_yvel = -ball_yvel;
  }
  if(ball_y + ball_s > window_h) {
    ball_y = window_h - ball_s;
    ball_yvel = -ball_yvel;
  }

  if(ball_x < 0 || ball_x > window_w) {
    // score
    spawn_ball();
  }
}

static int16_t update_paddle(int16_t y, int16_t yvel) {
  y += yvel * paddle_vel;
  if(y < 0)
    y = 0;
  if(y + paddle_h > window_h)
    y = window_h - paddle_h;
  return y;
}






static void button_pressed_handler(ClickRecognizerRef recognizer, void* context) {
  ButtonId button = click_recognizer_get_button_id(recognizer);

  switch(button) {
    case BUTTON_ID_DOWN:
      p1_yvel = 1;
      break;

    case BUTTON_ID_UP:
      p1_yvel = -1;
      break;

    default:
      break;
  }
}
static void button_released_handler(ClickRecognizerRef recognizer, void* context) {
  ButtonId button = click_recognizer_get_button_id(recognizer);

  switch(button) {
    case BUTTON_ID_DOWN:
    case BUTTON_ID_UP:
      p1_yvel = 0;
      break;

    default:
      break;
  }
}
