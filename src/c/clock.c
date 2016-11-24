//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "clock.h"

static Layer *window_layer = 0;
static Layer *seconds_layer = 0;
tm tm_time;

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global  
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %d:%02d:%02d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
  #endif
  
  ( ( HAND_LAYER_DATA *) layer_get_data( seconds_layer ) )->next_angle = TRIG_MAX_ANGLE * tm_time.tm_sec / 60;
  layer_mark_dirty( seconds_layer );
}

static void draw_clock_hand( HAND_DRAW_PARAMS *pDP ) {
  graphics_context_set_antialiased( pDP->ctx, true );
  // dot outline
  graphics_context_set_stroke_color( pDP->ctx, pDP->dot_outline_color );
  graphics_context_set_stroke_width( pDP->ctx, 1 );
  graphics_draw_circle( pDP->ctx, pDP->center_pt, pDP->dot_radius );
  // hand outline
  graphics_context_set_stroke_color( pDP->ctx, pDP->hand_outline_color );
  graphics_context_set_stroke_width( pDP->ctx, pDP->hand_width + 2);
  graphics_draw_line( pDP->ctx, pDP->from_pt, pDP->to_pt );
  // hand
  graphics_context_set_stroke_color( pDP->ctx, pDP->hand_color );
  graphics_context_set_stroke_width( pDP->ctx, pDP->hand_width );
  graphics_draw_line( pDP->ctx, pDP->from_pt, pDP->to_pt );
  // dot
  graphics_context_set_fill_color( pDP->ctx, pDP->dot_color );
  graphics_fill_circle( pDP->ctx, pDP->center_pt, pDP->dot_radius - 1 );
}

static void seconds_hand_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  graphics_context_set_fill_color( ctx, GColorWhite );
  graphics_fill_rect( ctx, layer_bounds, 0, GCornerNone );
  
  HAND_LAYER_DATA *seconds_layer_data = (HAND_LAYER_DATA *) layer_get_data( seconds_layer );
  
  GPoint center_pt = grect_center_point( &layer_bounds );
  GPoint end_pt;
  
  end_pt.x = ( sin_lookup( seconds_layer_data->next_angle ) * seconds_layer_data->length / TRIG_MAX_RATIO ) + center_pt.x;
  end_pt.y = ( -cos_lookup( seconds_layer_data->next_angle ) * seconds_layer_data->length / TRIG_MAX_RATIO ) + center_pt.y;

  HAND_DRAW_PARAMS hand_draw_params = (HAND_DRAW_PARAMS) {
    .ctx = ctx,
    .center_pt = center_pt,
    .from_pt = center_pt,
    .to_pt = end_pt,
    .hand_width = seconds_layer_data->width,
    .hand_color = GColorFromHEX( seconds_layer_data->colour ),
    .hand_outline_color = GColorWhite,
    .dot_radius = seconds_layer_data->hub_radius,
    .dot_color = GColorFromHEX( seconds_layer_data->colour ),
    .dot_outline_color = GColorWhite
  };
  draw_clock_hand( &hand_draw_params );
}

void clock_init( Window *window ){
  window_layer = window_get_root_layer( window );
  window_set_background_color( window, GColorBlack );
  GRect window_bounds = layer_get_bounds( window_layer );
  HAND_LAYER_DATA *seconds_layer_data = 0;
  
  seconds_layer = layer_create_with_data( SECONDS_RECT_FRAME, sizeof( HAND_LAYER_DATA ) );
  seconds_layer_data = (HAND_LAYER_DATA *) layer_get_data( seconds_layer ); 
  seconds_layer_data->colour = PBL_IF_COLOR_ELSE( 0xAAAAAA, 0xAAAAAA );
  seconds_layer_data->length = SECONDS_HAND_LENGTH;
  seconds_layer_data->width = SECONDS_HAND_THK;
  seconds_layer_data->bounds = SECONDS_RECT_FRAME;
  seconds_layer_data->hub_radius = SECONDS_HUB_RADIUS;
  seconds_layer_data->current_angle = 0;
  seconds_layer_data->next_angle = 0;
  layer_set_update_proc( seconds_layer, seconds_hand_layer_update_proc );
  layer_add_child( window_layer, seconds_layer );
  
  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
}

void clock_deinit( void ) {
  if ( seconds_layer) layer_destroy( seconds_layer );
}
