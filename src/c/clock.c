//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "clock.h"
#include "utils.h"
#include "animation.h"

// #define DEBUG

extern Layer *seconds_layer;

static Layer *window_layer = 0;
static Layer *outline_layer = 0;
static Layer *minutes_dial_layer = 0;
static Layer *seconds_dial_layer = 0;
static Layer *minutes_layer = 0;
static TextLayer *label_top_left = 0;
static TextLayer *label_top_right = 0;
static TextLayer *label_bottom_right = 0;
static TextLayer *label_seconds = 0;

static uint32_t seconds = 0;
static uint32_t minutes = 0;
static bool run_timer = false;
static tm tm_time;

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global, just for fun, as it is not used
  if ( units_changed & SECOND_UNIT ) ++seconds;
  if ( units_changed & MINUTE_UNIT ) {
    ++minutes;
    // layer_mark_dirty( minutes_layer );
  }
  
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %d:%02d:%02d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
  #endif
  // ( ( HAND_LAYER_DATA *) layer_get_data( seconds_layer ) )->current_angle = 
  //  ( ( HAND_LAYER_DATA *) layer_get_data( seconds_layer ) )->next_angle;
  ( ( HAND_LAYER_DATA *) layer_get_data( seconds_layer ) )->next_angle += ANGLE_PER_SECOND;
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: %ld",  ( ( HAND_LAYER_DATA *) layer_get_data( seconds_layer ) )->next_angle );
  #endif
  start_seconds_animation();
}

static void outline_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  graphics_context_set_antialiased( ctx, true );
  graphics_context_set_fill_color( ctx, GColorDarkGray );
  graphics_fill_rect( ctx, layer_bounds, layer_bounds.size.w / 2, GCornersAll );
}

static void seconds_dial_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  graphics_context_set_antialiased( ctx, true );
  graphics_context_set_fill_color( ctx, GColorWhite );
  graphics_fill_rect( ctx, layer_bounds, layer_bounds.size.w / 2, GCornersAll );
  draw_seconds_ticks( & (DRAW_TICKS_PARAMS) { layer, ctx, &SEC_TICK, 1, SECONDS_LAYER_WIDTH / 2 - 5 } );
  draw_seconds_ticks( & (DRAW_TICKS_PARAMS) { layer, ctx, &SEC_TICK, 5, SECONDS_LAYER_WIDTH / 2 - 10 } );
}

static void minutes_dial_update_proc( Layer *layer, GContext *ctx ) { 
  GRect layer_bounds = layer_get_bounds( layer );
  graphics_context_set_antialiased( ctx, true );
  graphics_context_set_fill_color( ctx, GColorWhite );
  graphics_fill_rect( ctx, layer_bounds, layer_bounds.size.w / 2, GCornersAll );
  graphics_context_set_stroke_color( ctx, GColorLightGray );
  graphics_context_set_stroke_width( ctx, 1 );
  graphics_draw_circle( ctx, GPoint( layer_bounds.size.w / 2, layer_bounds.size.h / 2 ), layer_bounds.size.w / 2 - 1 );
  draw_seconds_ticks( & (DRAW_TICKS_PARAMS) { layer, ctx, &MIN_TICK, 1, MINUTES_LAYER_WIDTH / 2 - 4 } );
  draw_seconds_ticks( & (DRAW_TICKS_PARAMS) { layer, ctx, &MIN_TICK, 5, MINUTES_LAYER_WIDTH / 2 - 8 } );
}

static void minutes_hand_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  
  HAND_LAYER_DATA *minutes_layer_data = (HAND_LAYER_DATA *) layer_get_data( minutes_layer );
  
  uint32_t min_angle = ( TRIG_MAX_ANGLE * minutes * 4 / 60 ) + ( TRIG_MAX_ANGLE * seconds * 4 / 3600 );
  uint32_t min_tail_angle = min_angle + ( TRIG_MAX_ANGLE / 2 );
  GPoint center_pt = grect_center_point( &layer_bounds );

  GPoint min_end_point = (GPoint) {
    .x = ( sin_lookup( min_angle ) * MINUTES_HAND_LENGTH / TRIG_MAX_RATIO ) + center_pt.x,
    .y = ( -cos_lookup( min_angle ) * MINUTES_HAND_LENGTH / TRIG_MAX_RATIO ) + center_pt.y
  };
  GPoint min_tail_end_pt = (GPoint) {
    .x = ( sin_lookup( min_tail_angle ) * MINUTES_HAND_TAIL_LENGTH / TRIG_MAX_RATIO ) + center_pt.x,
    .y = ( -cos_lookup( min_tail_angle ) * MINUTES_HAND_TAIL_LENGTH / TRIG_MAX_RATIO ) + center_pt.y
  };
  
  draw_clock_hand( & (HAND_DRAW_PARAMS) {
    .ctx = ctx,
    .center_pt = center_pt,
    .from_pt = min_end_point,
    .to_pt = min_tail_end_pt,
    .hand_width = minutes_layer_data->width,
    .hand_color = GColorFromHEX( minutes_layer_data->colour ),
    .hand_outline_color = GColorWhite,
    .dot_radius = minutes_layer_data->hub_radius,
    .dot_color = GColorFromHEX( minutes_layer_data->colour ),
    .dot_outline_color = GColorWhite
  } );
}

static void seconds_hand_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  
  HAND_LAYER_DATA *seconds_layer_data = (HAND_LAYER_DATA *) layer_get_data( seconds_layer );
  
  GPoint center_pt = grect_center_point( &layer_bounds );
  
  GPoint end_pt = (GPoint) {
    .x = ( sin_lookup( seconds_layer_data->current_angle % TRIG_MAX_ANGLE ) *
              seconds_layer_data->length / TRIG_MAX_RATIO ) + center_pt.x,
    .y = ( -cos_lookup( seconds_layer_data->current_angle % TRIG_MAX_ANGLE ) *
              seconds_layer_data->length / TRIG_MAX_RATIO ) + center_pt.y
  };
  GPoint tail_end_pt = (GPoint) {
    .x = ( sin_lookup( ( ( seconds_layer_data->current_angle ) - TRIG_MAX_ANGLE / 2 ) % TRIG_MAX_ANGLE ) * 
                   seconds_layer_data->tail_length / TRIG_MAX_RATIO ) + center_pt.x,
    .y = ( -cos_lookup( ( ( seconds_layer_data->current_angle ) - TRIG_MAX_ANGLE / 2 ) % TRIG_MAX_ANGLE ) *
                   seconds_layer_data->tail_length / TRIG_MAX_RATIO ) + center_pt.y
  };
  
  draw_clock_hand(  & (HAND_DRAW_PARAMS) {
    .ctx = ctx,
    .center_pt = center_pt,
    .from_pt = end_pt,
    .to_pt = tail_end_pt,
    .hand_width = seconds_layer_data->width,
    .hand_color = GColorFromHEX( seconds_layer_data->colour ),
    .hand_outline_color = GColorWhite,
    .dot_radius = seconds_layer_data->hub_radius,
    .dot_color = GColorFromHEX( seconds_layer_data->colour ),
    .dot_outline_color = GColorWhite
  } );
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *) context;
  
  if ( !run_timer ) {
    seconds = 0;
    minutes = 0;
    ( ( HAND_LAYER_DATA *) layer_get_data( seconds_layer ) )->current_angle = 0;
    ( ( HAND_LAYER_DATA *) layer_get_data( minutes_layer ) )->current_angle = 0;
    ( ( HAND_LAYER_DATA *) layer_get_data( seconds_layer ) )->next_angle = 0;
    ( ( HAND_LAYER_DATA *) layer_get_data( minutes_layer ) )->next_angle = 0;
    layer_mark_dirty( seconds_layer );
    layer_mark_dirty( minutes_layer );
  }
}

void down_single_click_handler( ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *) context;
  
  if ( ( run_timer = !run_timer ) ) {
    tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
  } else {
    tick_timer_service_unsubscribe();
  }
}

void config_provider( Window *window ) {
  window_single_click_subscribe( BUTTON_ID_DOWN, down_single_click_handler );
  window_single_click_subscribe( BUTTON_ID_UP, up_single_click_handler );
}

void clock_init( Window *window ){
  window_layer = window_get_root_layer( window );
  window_set_background_color( window, GColorLightGray );
  GRect window_bounds = layer_get_bounds( window_layer );
  
  outline_layer = layer_create( OUTLINE_RECT_FRAME );
  layer_set_update_proc( outline_layer, outline_layer_update_proc );
  layer_add_child( window_layer, outline_layer );

  seconds_dial_layer = layer_create( SECONDS_RECT_FRAME );
  layer_set_update_proc( seconds_dial_layer, seconds_dial_update_proc );
  layer_add_child( outline_layer, seconds_dial_layer );

  minutes_dial_layer = layer_create( MINUTES_RECT_FRAME );
  layer_set_update_proc( minutes_dial_layer, minutes_dial_update_proc );
  layer_add_child( outline_layer, minutes_dial_layer );
  
  minutes_layer = layer_create_with_data( MINUTES_RECT_FRAME, sizeof( HAND_LAYER_DATA ) );
  *( (HAND_LAYER_DATA *) layer_get_data( minutes_layer ) ) = (HAND_LAYER_DATA) {
    .colour = PBL_IF_COLOR_ELSE( 0xFF0000, 0x000000 ),
    .length = MINUTES_HAND_LENGTH,
    .tail_length = MINUTES_HAND_TAIL_LENGTH,
    .width = MINUTES_HAND_THK,
    .bounds = MINUTES_RECT_FRAME,
    .hub_radius = MINUTES_HUB_RADIUS,
    .current_angle = 0,
    .next_angle = 0,
  };
  layer_set_update_proc( minutes_layer, minutes_hand_layer_update_proc );
  layer_add_child( outline_layer, minutes_layer );

  seconds_layer = layer_create_with_data( SECONDS_RECT_FRAME, sizeof( HAND_LAYER_DATA ) );
  *( (HAND_LAYER_DATA *) layer_get_data( seconds_layer ) ) = (HAND_LAYER_DATA) {
    .colour = PBL_IF_COLOR_ELSE( 0x000000, 0x000000 ),
    .length = SECONDS_HAND_LENGTH,
    .tail_length = SECONDS_HAND_TAIL_LENGTH,
    .width = SECONDS_HAND_THK,
    .bounds = SECONDS_RECT_FRAME,
    .hub_radius = SECONDS_HUB_RADIUS,
    .current_angle = 0,
    .next_angle = 0,
  };
  layer_set_update_proc( seconds_layer, seconds_hand_layer_update_proc );
  layer_add_child( outline_layer, seconds_layer );
  
  GFont txt_font = fonts_get_system_font( FONT_KEY_GOTHIC_14 );
  
  make_label( & (MAKE_LABEL_PARAMS) { &label_top_left, LABEL_TOP_LEFT_RECT, window_layer, " EXIT", 
                                     txt_font, GColorDarkGray, GTextAlignmentLeft } );
  make_label( & (MAKE_LABEL_PARAMS) { &label_top_right, LABEL_TOP_RIGHT_RECT, window_layer, "RESET ",
                                     txt_font, GColorFolly, GTextAlignmentRight } );
  make_label( & (MAKE_LABEL_PARAMS) { &label_bottom_right, LABEL_BOTTOM_RIGHT_RECT, window_layer, "START/STOP ",
                                     txt_font, GColorCobaltBlue, GTextAlignmentRight } );
  make_label( & (MAKE_LABEL_PARAMS) { &label_seconds, LABEL_SECS_RECT, seconds_layer, "30 SECONDS",
                                     txt_font, GColorLightGray, GTextAlignmentCenter } );
  
  window_set_click_config_provider( window, (ClickConfigProvider) config_provider );
}

void clock_deinit( void ) {
  if ( label_seconds ) text_layer_destroy( label_seconds );
  if ( label_top_left ) text_layer_destroy( label_top_left );
  if ( label_top_right ) text_layer_destroy( label_top_right );
  if ( label_bottom_right ) text_layer_destroy( label_bottom_right );
  if ( minutes_layer ) layer_destroy( minutes_layer );
  if ( seconds_layer ) layer_destroy( seconds_layer );
  if ( minutes_dial_layer ) layer_destroy( minutes_dial_layer );
  if ( seconds_dial_layer ) layer_destroy( seconds_dial_layer );
  if ( outline_layer ) layer_destroy( outline_layer );
}
