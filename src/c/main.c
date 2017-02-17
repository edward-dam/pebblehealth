#include <pebble.h>

//APP_LOG(APP_LOG_LEVEL_DEBUG, "LOG MESSAGE");

// window layers
static Window *main_window;
static Layer *canvas_layer;
static TextLayer *day_layer;
static TextLayer *sleep_emoji;
static TextLayer *sleep_layer;
static TextLayer *time_layer;
static TextLayer *heart_emoji;
static TextLayer *heart_layer;
static TextLayer *steps_emoji;
static TextLayer *steps_layer;
static TextLayer *date_layer;

// image layers
static BitmapLayer *splash_layer;
static GBitmap *splash_bitmap;

// battery level
static int battery_level;
static Layer *battery_layer;
static TextLayer *battery_percentage;

// bluetooth icon
static BitmapLayer *bt_icon_layer;
static GBitmap *bt_icon_bitmap;
bool bt_startup = true;

// saved settings
uint32_t hour_setting = 0;
uint32_t date_setting = 1;
uint32_t heart_setting = 2;
bool hour_bool;
bool date_bool;
bool heart_bool;

// load time
static char time12_buffer[6];
static char time24_buffer[6];
static char datedm_buffer[13];
static char datemd_buffer[13];

// load options
static void load_options() {
  // load 24 or 12 hours
  if (persist_exists(hour_setting)) {
    char hour_buffer[5];
    persist_read_string(hour_setting, hour_buffer, sizeof(hour_buffer));
    if (strcmp(hour_buffer, "true") == 0) {
      hour_bool = true;
      text_layer_set_text(time_layer, time12_buffer);
    } else {
      hour_bool = false;
      text_layer_set_text(time_layer, time24_buffer);
    }
  } else {
    hour_bool = false;
  }
  
  // load date order
  if (persist_exists(date_setting)) {
    char date_buffer[5];
    persist_read_string(date_setting, date_buffer, sizeof(date_buffer));
    if (strcmp(date_buffer, "true") == 0) {
      date_bool = true;
      text_layer_set_text(date_layer, datemd_buffer);
    } else {
      date_bool = false;
      text_layer_set_text(date_layer, datedm_buffer);
    }
  } else {
    date_bool = false;
  }
}

// update options/weather
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // collect options
  Tuple *hour_tuple = dict_find(iterator, MESSAGE_KEY_HOUR);
  Tuple *date_tuple = dict_find(iterator, MESSAGE_KEY_DATE);
  Tuple *heart_tuple = dict_find(iterator, MESSAGE_KEY_HEART);

  // save options
  if(hour_tuple && heart_tuple) {
    char *hour_string = hour_tuple->value->cstring;
    char *date_string = date_tuple->value->cstring;
    char *heart_string = heart_tuple->value->cstring;
    persist_write_string(hour_setting, hour_string);
    persist_write_string(date_setting, date_string);
    persist_write_string(heart_setting, heart_string);
    load_options();
  }
}

// bluetooth connection change
static void bluetooth_callback(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(bt_icon_layer), connected);
  if(!bt_startup) {
    vibes_double_pulse();
  }
  bt_startup = false;
}

// collect battery level
static void battery_callback(BatteryChargeState state) {
  battery_level = state.charge_percent;
  layer_mark_dirty(battery_layer);
}

// draw battery level
static void battery_update_proc(Layer *layer, GContext *ctx) {
  // get canvas size
  GRect bounds = layer_get_bounds(layer);
  int my = bounds.size.h;
  int cx = bounds.size.w/2;
  int cy = bounds.size.h/2;
  
  // collect battery level
  static char battery_buffer[5];
  snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", (int8_t)battery_level);

  // display battery percentage
  battery_percentage = text_layer_create(GRect(cx+35,cy-52,30,my));
  text_layer_set_background_color(battery_percentage, GColorClear);
  text_layer_set_text_alignment(battery_percentage, GTextAlignmentCenter);
  text_layer_set_font(battery_percentage, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text(battery_percentage, battery_buffer);
  layer_add_child(layer, text_layer_get_layer(battery_percentage));
}

// update date/time
static void update_datetime() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // display day
  static char day_buffer[10];
  strftime(day_buffer, sizeof(day_buffer), "%A", tick_time);
  text_layer_set_text(day_layer, day_buffer);

  // display time
  strftime(time24_buffer, sizeof(time24_buffer), "%H:%M", tick_time);
  strftime(time12_buffer, sizeof(time12_buffer), "%I:%M", tick_time);
  if (!hour_bool) {
    text_layer_set_text(time_layer, time24_buffer);
  } else {
    text_layer_set_text(time_layer, time12_buffer);
  }
  
  // display date
  strftime(datedm_buffer, sizeof(datedm_buffer), "%d %B", tick_time);
  strftime(datemd_buffer, sizeof(datemd_buffer), "%B %d", tick_time);
  if (!date_bool) {
    text_layer_set_text(date_layer, datedm_buffer);
  } else {
    text_layer_set_text(date_layer, datemd_buffer);
  }
}
static void mins_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_datetime();
}

// drawing canvas
static void draw_canvas(Layer *layer, GContext *ctx) {
  // get canvas size
  GRect bounds = layer_get_bounds(layer);
  int cx = bounds.size.w/2;
  int cy = bounds.size.h/2;
  
  // set default definitions
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);
 
  // draw header/footer
  GRect header_square = GRect(cx-65,cy-78,130,18);
  GRect footer_square = GRect(cx-65,cy+60,130,18);
  graphics_draw_rect(ctx, header_square);
  graphics_draw_rect(ctx, footer_square);
  graphics_fill_rect(ctx, header_square, 0, GCornersAll);
  graphics_fill_rect(ctx, footer_square, 0, GCornersAll);
  
  // draw time box
  GRect time_square = GRect(cx-65,cy-25,130,50);
  graphics_draw_rect(ctx, time_square);
  graphics_fill_rect(ctx, time_square, 2, GCornersAll);

  // draw health boxes
  GRect sleep_square = GRect(cx-65,cy-55,88,25);
  GRect heart_square = GRect(cx-65,cy+30,53,25);
  GRect steps_square = GRect(cx-5,cy+30,70,25);
  graphics_draw_rect(ctx, sleep_square);
  graphics_draw_rect(ctx, heart_square);
  graphics_draw_rect(ctx, steps_square);
  graphics_fill_rect(ctx, sleep_square, 1, GCornersAll);
  graphics_fill_rect(ctx, heart_square, 1, GCornersAll);
  graphics_fill_rect(ctx, steps_square, 1, GCornersAll);

  // draw battery
  GRect battery_header = GRect(cx+30,cy-50,10,15);
  GRect battery_square = GRect(cx+35,cy-55,30,25);
  graphics_draw_rect(ctx, battery_header);
  graphics_draw_rect(ctx, battery_square);
  graphics_fill_rect(ctx, battery_header, 1, GCornersAll);
  graphics_fill_rect(ctx, battery_square, 1, GCornersAll);
  
  // reverse colours
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorBlack);
  
  // draw time inside
  GRect time_inside = GRect(cx-60,cy-20,120,40);
  graphics_draw_rect(ctx, time_inside);
  graphics_fill_rect(ctx, time_inside, 2, GCornersAll);
  
  // draw dots
  graphics_fill_circle(ctx, GPoint(cx-58,cy-69), 4);
  graphics_fill_circle(ctx, GPoint(cx+57,cy-69), 4);
  graphics_fill_circle(ctx, GPoint(cx-58,cy+69), 4);
  graphics_fill_circle(ctx, GPoint(cx+57,cy+69), 4);
}

// window load
static void main_window_load(Window *window) {
  // collect window size
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int mx = bounds.size.w;
  int my = bounds.size.h;
  int cx = bounds.size.w/2;
  int cy = bounds.size.h/2;

  // drawing canvas
  canvas_layer = layer_create(bounds);
  layer_set_update_proc(canvas_layer, draw_canvas);
  layer_add_child(window_layer, canvas_layer);
  
  // day layer
  day_layer = text_layer_create(GRect(0,cy-82,mx,my));
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
  text_layer_set_font(day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(day_layer));
  
  // sleep layer
  sleep_emoji = text_layer_create(GRect(cx-62,cy-58,88,my));
  sleep_layer = text_layer_create(GRect(cx-44,cy-59,68,my));
  text_layer_set_background_color(sleep_emoji, GColorClear);
  text_layer_set_background_color(sleep_layer, GColorClear);
  text_layer_set_text_alignment(sleep_layer, GTextAlignmentCenter);
  text_layer_set_font(sleep_emoji, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_font(sleep_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(sleep_emoji, "\U0001F634");
  text_layer_set_text(sleep_layer, "7h30m");
  layer_add_child(window_layer, text_layer_get_layer(sleep_emoji));
  layer_add_child(window_layer, text_layer_get_layer(sleep_layer));
  
  // battery layer
  battery_layer = layer_create(bounds);
  layer_set_update_proc(battery_layer, battery_update_proc);
  layer_add_child(window_layer, battery_layer);
  battery_callback(battery_state_service_peek());
  
  // bluetooth layer
  bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON);
  bt_icon_layer = bitmap_layer_create(GRect(cx+35,cy-53,30,21));
  bitmap_layer_set_bitmap(bt_icon_layer, bt_icon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(bt_icon_layer));
  bluetooth_callback(connection_service_peek_pebble_app_connection());

  // splash layer
  splash_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SPLASH);
  splash_layer = bitmap_layer_create(GRect(cx-60,cy-18,120,36));
  bitmap_layer_set_bitmap(splash_layer, splash_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(splash_layer));

  // time layer
  time_layer = text_layer_create(GRect(19,cy-18,mx,my));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  // heart layer
  heart_emoji = text_layer_create(GRect(cx-62,cy+26,mx,my));
  heart_layer = text_layer_create(GRect(cx-47,cy+26,36,my));
  text_layer_set_background_color(heart_emoji, GColorClear);
  text_layer_set_background_color(heart_layer, GColorClear);
  text_layer_set_text_alignment(heart_layer, GTextAlignmentCenter);
  text_layer_set_font(heart_emoji, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_font(heart_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(heart_emoji, "\U00002764");
  text_layer_set_text(heart_layer, "60");
  layer_add_child(window_layer, text_layer_get_layer(heart_emoji));
  layer_add_child(window_layer, text_layer_get_layer(heart_layer));

  // steps layer
  steps_emoji = text_layer_create(GRect(cx-2,cy+26,mx,my));
  steps_layer = text_layer_create(GRect(cx+10,cy+26,56,my));
  text_layer_set_background_color(steps_emoji, GColorClear);
  text_layer_set_background_color(steps_layer, GColorClear);
  text_layer_set_text_alignment(steps_layer, GTextAlignmentCenter);
  text_layer_set_font(steps_emoji, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_font(steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(steps_emoji, "\U0001F425");
  text_layer_set_text(steps_layer, "8000");
  layer_add_child(window_layer, text_layer_get_layer(steps_emoji));
  layer_add_child(window_layer, text_layer_get_layer(steps_layer));

  // date layer
  date_layer = text_layer_create(GRect(0,cy+56,mx,my));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

// window unload
static void main_window_unload(Window *window) {
  // destroy text layers
  text_layer_destroy(date_layer);
  text_layer_destroy(steps_layer);
  text_layer_destroy(steps_emoji);
  text_layer_destroy(heart_layer);
  text_layer_destroy(heart_emoji);
  text_layer_destroy(time_layer);
  text_layer_destroy(battery_percentage);
  text_layer_destroy(sleep_layer);
  text_layer_destroy(sleep_emoji);
  text_layer_destroy(day_layer);
  // destroy image layers
  bitmap_layer_destroy(splash_layer);
  gbitmap_destroy(splash_bitmap);
  bitmap_layer_destroy(bt_icon_layer);
  gbitmap_destroy(bt_icon_bitmap);
  // destroy canvas layers
  layer_destroy(battery_layer);
  layer_destroy(canvas_layer);
}

// init
static void init() {
  // create window
  main_window = window_create();
  window_set_background_color(main_window, GColorBlack);

  // load/unload window
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // send window to screen
  window_stack_push(main_window, true);
  
  // load options
  load_options();

  // update battery level
  battery_state_service_subscribe(battery_callback);
  
  // update date/time
  update_datetime();
  tick_timer_service_subscribe(MINUTE_UNIT, mins_tick_handler);
  
  // check bluetooth connection
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // update options/weather
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(64,0);  
}

// deinit
static void deinit() {
  // unsubscribe from events
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  // destroy window
  window_destroy(main_window);
}

// main
int main(void) {
  init();
  app_event_loop();
  deinit();
}