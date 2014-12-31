#include <pebble.h>

#define STATUS_LINE_TIMEOUT 5000
  
static void time_handler(struct tm *tick_time, TimeUnits units_changed);
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void update_time();
static void init();
static void deinit();

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *x_time_layer;
static TextLayer *x_date_layer;
static BitmapLayer *s_timeborder_layer;
static GBitmap *s_timeborder_bitmap;
static AppTimer *shake_timeout = NULL;

static GFont custom_font_16;
static GFont custom_font_20;
static GFont custom_font_24;

    
int main() {
        init();
        app_event_loop();
        deinit();
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed){
        update_time();
}

static void main_window_load(Window *window) {      
        custom_font_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SKA_CUBIC_16));
        custom_font_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SKA_CUBIC_20));
        custom_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SKA_CUBIC_24));

        s_timeborder_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ARSENAL_FC_BG);
        s_timeborder_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
        bitmap_layer_set_bitmap(s_timeborder_layer, s_timeborder_bitmap);
  
        s_time_layer = text_layer_create(GRect(2, 23, 144, 45));
        text_layer_set_background_color(s_time_layer, GColorClear);
        text_layer_set_text_color(s_time_layer, GColorWhite);
        text_layer_set_font(s_time_layer, custom_font_24);
        text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

        x_time_layer = text_layer_create(GRect(0, 13, 144, 45));
        text_layer_set_background_color(x_time_layer, GColorClear);
        text_layer_set_text_color(x_time_layer, GColorWhite);
        text_layer_set_font(x_time_layer, custom_font_20);
        text_layer_set_text_alignment(x_time_layer, GTextAlignmentCenter);
  
        x_date_layer = text_layer_create(GRect(0, 42, 144, 22));
        text_layer_set_background_color(x_date_layer, GColorClear);
        text_layer_set_text_color(x_date_layer, GColorWhite);
        text_layer_set_font(x_date_layer, custom_font_16);
        text_layer_set_text_alignment(x_date_layer, GTextAlignmentCenter);        

        // Add them as child layers to the Window's root layer
        layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_timeborder_layer));
        layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
        layer_add_child(window_get_root_layer(window), text_layer_get_layer(x_time_layer));
        layer_add_child(window_get_root_layer(window), text_layer_get_layer(x_date_layer));
        layer_set_hidden(text_layer_get_layer(x_time_layer), true);
        layer_set_hidden(text_layer_get_layer(x_date_layer), true);
}

void hide_stat () {
  layer_set_hidden(text_layer_get_layer(s_time_layer), false);
  layer_set_hidden(text_layer_get_layer(x_time_layer), true);
  layer_set_hidden(text_layer_get_layer(x_date_layer), true);
  shake_timeout = NULL;
}
void show_stat ()  {
  layer_set_hidden(text_layer_get_layer(s_time_layer), true);
  layer_set_hidden(text_layer_get_layer(x_time_layer), false);
  layer_set_hidden(text_layer_get_layer(x_date_layer), false);
}
void wrist_flick_handler(AccelAxisType axis, int32_t direction) {
  if (axis == 1 && !shake_timeout) {
    show_stat();
    shake_timeout = app_timer_register (STATUS_LINE_TIMEOUT, hide_stat, NULL);
  }
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(x_time_layer);
  text_layer_destroy(x_date_layer);
  layer_remove_from_parent(bitmap_layer_get_layer(s_timeborder_layer));
  bitmap_layer_destroy(s_timeborder_layer);
  gbitmap_destroy(s_timeborder_bitmap);
  fonts_unload_custom_font(custom_font_16);
  fonts_unload_custom_font(custom_font_20);
  fonts_unload_custom_font(custom_font_24);
}

static void init() {
        // Create main Window element and assign to pointer
        s_main_window = window_create();
        window_set_fullscreen(s_main_window, true);
        window_set_background_color(s_main_window, GColorBlack);
        // Set handlers to manage the elements inside the Window
        window_set_window_handlers(s_main_window, (WindowHandlers) {
                .load = main_window_load,
                .unload = main_window_unload
        });
        window_stack_push(s_main_window, true);
  
        tick_timer_service_subscribe(MINUTE_UNIT, time_handler);
        accel_tap_service_subscribe(wrist_flick_handler);

        update_time();
}

static void update_time() {
        time_t temp = time(NULL); 
        struct tm *current_time = localtime(&temp);

        static char time_buffer[] = "00:00";
        static char stat_buffer[] = "wed 31 - 100%";
        BatteryChargeState charge_state = battery_state_service_peek();

        if(clock_is_24h_style() == true) { 
                strftime(time_buffer, sizeof("00:00"), "%H:%M", current_time);
        } else {
                strftime(time_buffer, sizeof("00:00"), "%I:%M", current_time);
        }
        
  /*
        if(time_buffer[0] == '0'){
                for(int i = 1; i < 6; i++){ //get rid of that leading 0
                        time_buffer[i - 1] = time_buffer[i];      
                }
        } 
        */
        strftime(stat_buffer, sizeof(stat_buffer), "%a %e", current_time);
        snprintf(stat_buffer, sizeof(stat_buffer), "%s - %d%%", stat_buffer, charge_state.charge_percent);
        //stat_buffer[0] += ('a' - 'A');
  
        text_layer_set_text(s_time_layer, time_buffer);
        text_layer_set_text(x_date_layer, stat_buffer);
        text_layer_set_text(x_time_layer, time_buffer);
}

static void deinit() {
    window_destroy(s_main_window);
}