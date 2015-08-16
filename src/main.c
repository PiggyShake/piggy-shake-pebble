#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer;

static GFont s_time_font;
static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;
uint32_t pig_cycles = 0;

static void shake_pig(int cycles){
  pig_cycles = cycles * 4;
  vibes_short_pulse();
}

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    
    if (pig_cycles > 0) {
      bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
      layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
      pig_cycles--;
    }

    // Timer for that delay
    app_timer_register(next_delay, timer_handler, NULL);
  } else {
    // Start again
    gbitmap_sequence_restart(s_sequence);
  }
}

static void load_sequence() {
  // Free old data
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION);

  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Begin animation
  app_timer_register(1, timer_handler, NULL);
  
  shake_pig(5);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "I got a shake from WS.");
  shake_pig(5);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  // Send the SHAKE!
  app_message_outbox_send();
  // shake_pig(5);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  bounds.size.h = 110;
  
  // Add pig layer
  s_bitmap_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 110, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  // text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  load_sequence();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // Destroy image
  bitmap_layer_destroy(s_bitmap_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  accel_tap_service_subscribe(tap_handler);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}