#include "pebble.h"

#define PI 3.14159265

// -------------------------
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
static const int16_t paddle_vel = 50;

// paddle positions
static int16_t p1_x = 5;
static int16_t p2_x = 130;
static float p1_y = 0;
static float p2_y = 0;
static float p1_yvel = 0;
static float p2_yvel = 0;

// ball
static const int16_t ball_s = 10;
static float ball_x = 0;
static float ball_y = 0;
static float ball_xdir = 0;
static float ball_ydir = 0;
static float ball_vel = 50;


// states
static bool running = false;
static const uint32_t timeout_ms = 100;

static int p1_score = 0;
static int p2_score = 0;


// time
static int last_time = 0;
static float elapsed_time = 0.0;



// -------------------------------------
// functions
static void init();
static void deinit();
static void click_config_provider(Window*);

static void setup_game();

static void start();
static void pause();

static void draw(Layer*, GContext*);
static void draw_paddles(GContext*);
static void draw_ball(GContext*);
static void draw_score(Layer*, GContext*);
static void draw_pause_message(Layer*, GContext*);

static void update(void*);
static void calculate_times();
static void update_ai();
static void update_positions();
static int16_t update_paddle(int16_t, int16_t);
static void check_collision();


static void spawn_ball();

// button handlers
static void button_pressed_handler(ClickRecognizerRef, void*);
static void button_released_handler(ClickRecognizerRef, void*);

// utils
static uint32_t round_float(float);
static float degrees_to_radians(float);
static float radians_to_degrees(float);


/**
 * Main
 */
int main() {
  init();

  app_event_loop();

  deinit();
}




static void init() {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_stack_push(window, true /* animated */);

  Layer* root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  window_w = bounds.size.w;
  window_h = bounds.size.h;

  


  // setup main layer
  main_layer = layer_create(bounds);
  layer_set_update_proc(main_layer, draw);
  layer_add_child(root_layer, main_layer);

  // bind click events
  window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

  setup_game();
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


static void setup_game() {
  uint32_t paddle_margin = 5;
  p1_x = 0 + paddle_margin;
  p2_x = window_w - paddle_margin - paddle_w;

  p1_y = window_h / 2 - paddle_h / 2;
  p2_y = window_h / 2 - paddle_h / 2;
  
  spawn_ball();
}



static void start() {
  running = true;
  timer = app_timer_register(timeout_ms, update, NULL);
}
static void pause() {
  running = false;
}



static void spawn_ball() {
  ball_x = (window_w / 2) - (ball_s / 2);
  ball_y = (window_h / 2) - (ball_s / 2);

  ball_xdir = 1 - (2 * (rand() % 2));
  ball_ydir = 1 - (rand() % 3);
}




/**
 * Draw
 */
static void draw(Layer* layer, GContext* ctx) { 
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);

  draw_paddles(ctx);
  draw_ball(ctx);
  
  draw_score(layer, ctx);

  if(!running) {
    draw_pause_message(layer, ctx);
  }
}


static void draw_paddles(GContext* ctx) {
  graphics_fill_rect(ctx, GRect(p1_x, p1_y, paddle_w, paddle_h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(p2_x, p2_y, paddle_w, paddle_h), 0, GCornerNone);
}
static void draw_ball(GContext* ctx) {
  graphics_fill_rect(ctx, GRect(ball_x, ball_y, ball_s, ball_s), 5, GCornersAll);
}

static void draw_score(Layer* layer, GContext* ctx) {
  GRect bounds = layer_get_bounds(layer);

  static char score_str[10];
  snprintf(score_str, sizeof(score_str), "%u - %u", p1_score, p2_score);

  graphics_draw_text (
    ctx,
    score_str,
    fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
    GRect(0, 0, bounds.size.w, 40),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );  
}

static void draw_pause_message(Layer* layer, GContext* ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_draw_text (
    ctx,
    "Paused",
    fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
    GRect(0, (bounds.size.h / 2) - 20, bounds.size.w, 40),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  ); 
}



/**
 * Update
 */
static void update(void* data) {
  // schedule draw
  layer_mark_dirty(main_layer);

  if(running) {
    calculate_times();

    update_ai();

    update_positions();

    check_collision();

    timer = app_timer_register(timeout_ms, update, NULL);
  }
}

static void update_ai() {
  float p_mid = p2_y + (paddle_h / 2.0);
  float b_mid = ball_y + (ball_s / 2.0);

  if(b_mid > p_mid) {
    p2_yvel = 1;
  }
  if(b_mid < p_mid) {
    p2_yvel = -1;
  }
}

static void calculate_times() {
  elapsed_time = 0.05;
}

static void update_positions() {
  p1_y = update_paddle(p1_y, p1_yvel);
  p2_y = update_paddle(p2_y, p2_yvel);


  // update ball
  ball_x += ball_xdir * ball_vel * elapsed_time;
  ball_y += ball_ydir * ball_vel * elapsed_time;

  if(ball_y < 0) {
    ball_y = 0;
    ball_ydir = -ball_ydir;
  }
  if(ball_y + ball_s > window_h) {
    ball_y = window_h - ball_s;
    ball_ydir = -ball_ydir;
  }

  if(ball_x < 0) {
    ++p2_score;
    spawn_ball();
  }
  if(ball_x > window_w) {
    ++p1_score;
    spawn_ball(); 
  }
}

static int16_t update_paddle(int16_t y, int16_t yvel) {
  y += yvel * paddle_vel * elapsed_time;
  if(y < 0)
    y = 0;
  if(y + paddle_h > window_h)
    y = window_h - paddle_h;
  return y;
}

static void check_collision() {
  float epsilon = 2.0;
  if(ball_xdir < 0) {
    if(ball_x < p1_x + paddle_w && ball_x > p1_x + paddle_w - epsilon &&
       ball_y + ball_s >= p1_y && ball_y <= p1_y + paddle_h) {
      ball_xdir *= -1;
    }
  }
  else {
    if(ball_x + ball_s > p2_x && ball_x + ball_s < p2_x + epsilon &&
       ball_y + ball_s >= p2_y && ball_y <= p2_y + paddle_h) {
      ball_xdir *= -1;
    }
  }
}





/**
 * Input
 */
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

    case BUTTON_ID_SELECT:
      if(running)
        pause();
      else
        start();
      break;
    default:
      break;
  }
}



/**
 * Utils
 */
static uint32_t round_float(float f) {
  return (uint32_t)(f + 0.5);
}

static float degrees_to_radians(float f) {
  return f * (PI / 180.0);
}
static float radians_to_degrees(float f) {
  return f * (180.0 / PI);
}