//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "clock.h"

// #define DEBUG

static Layer *window_layer = 0;
static Layer *outline_layer = 0;
static Layer *seconds_dial_layer = 0;
static Layer *minutes_dial_layer = 0;
static Layer *seconds_layer = 0;
static Layer *minutes_layer = 0;
static TextLayer *label_top_left = 0;
static TextLayer *label_top_right = 0;
static TextLayer *label_bottom_right = 0;

static uint32_t seconds = 0;
static uint32_t minutes = 0;
static tm tm_time;

static void print_pt( char *str, GPoint pt ) {
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "%s: ( %d, %d )", str, pt.x, pt.y );
  #endif
}

static void print_rect( char *str, GRect rect ) {
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "%s: ( %d, %d, %d, %d )", str, rect.origin.x, rect.origin.y, rect.size.w, rect.size.h );
  #endif
}

static void second_hand_uint32_setter( void *subject, uint32_t angle ) {
  ( ( HAND_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_angle = angle;
  layer_mark_dirty( (Layer *) subject );
}

static uint32_t second_hand_uint32_getter( void *subject ) {
  return ( ( HAND_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_angle;
}

static const PropertyAnimationImplementation second_hand_animation_implementation = {
  .base = {
    .update = (AnimationUpdateImplementation) property_animation_update_uint32,
  },
    .accessors = {
      .setter = { .uint32 = (const UInt32Setter) second_hand_uint32_setter },
      .getter = { .uint32 = (const UInt32Getter) second_hand_uint32_getter },
  },
};

static void start_seconds_animation( void ) {
  HAND_LAYER_DATA *seconds_layer_data = (HAND_LAYER_DATA *) layer_get_data( seconds_layer );
  
  PropertyAnimation *seconds_hand_prop_animation = property_animation_create( &second_hand_animation_implementation, (void *) seconds_layer, NULL, NULL );
  property_animation_from( seconds_hand_prop_animation, (void *) &( seconds_layer_data->current_angle ),
                          sizeof( seconds_layer_data->current_angle ), true );
  property_animation_to( seconds_hand_prop_animation, (void *) &( seconds_layer_data->next_angle ),
                        sizeof( seconds_layer_data->next_angle ), true );
  Animation *seconds_hand_animation = property_animation_get_animation( seconds_hand_prop_animation );
  animation_set_curve( seconds_hand_animation, AnimationCurveLinear );
  animation_set_delay( seconds_hand_animation, 0 );
  animation_set_duration( seconds_hand_animation, 1000 );
  
  Animation **seconds_animation_array = (Animation**) malloc( ( NUM_ANIMATIONS ) * sizeof( Animation* ) );
  seconds_animation_array[0] = seconds_hand_animation;
  
  Animation *spawn = animation_spawn_create_from_array( seconds_animation_array, NUM_ANIMATIONS );
  animation_set_play_count( spawn, 1 );
  animation_schedule( spawn );
  free( seconds_animation_array );
}

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

static void draw_clock_hand( HAND_DRAW_PARAMS *pDP ) {
  graphics_context_set_antialiased( pDP->ctx, true );
  // dot outline
  graphics_context_set_stroke_color( pDP->ctx, pDP->dot_outline_color );
  graphics_context_set_stroke_width( pDP->ctx, 1 );
  graphics_draw_circle( pDP->ctx, pDP->center_pt, pDP->dot_radius );
  // hand outline
  graphics_context_set_stroke_color( pDP->ctx, pDP->hand_outline_color );
  graphics_context_set_stroke_width( pDP->ctx, pDP->hand_width + 2);
  graphics_draw_line( pDP->ctx, pDP->center_pt, pDP->from_pt );
  graphics_draw_line( pDP->ctx, pDP->center_pt, pDP->to_pt );
  // hand
  graphics_context_set_stroke_color( pDP->ctx, pDP->hand_color );
  graphics_context_set_stroke_width( pDP->ctx, pDP->hand_width );
  graphics_draw_line( pDP->ctx, pDP->center_pt, pDP->from_pt );
  graphics_draw_line( pDP->ctx, pDP->center_pt, pDP->to_pt );
  // dot
  graphics_context_set_fill_color( pDP->ctx, pDP->dot_color );
  graphics_fill_circle( pDP->ctx, pDP->center_pt, pDP->dot_radius - 1 );
  // center
  graphics_context_set_fill_color( pDP->ctx, GColorWhite );
  graphics_fill_circle( pDP->ctx, pDP->center_pt, 1 );
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
}

static void minutes_dial_update_proc( Layer *layer, GContext *ctx ) { 
  GRect layer_bounds = layer_get_bounds( layer );
  graphics_context_set_antialiased( ctx, true );
  graphics_context_set_fill_color( ctx, GColorWhite );
  graphics_fill_rect( ctx, layer_bounds, layer_bounds.size.w / 2, GCornersAll );
  graphics_context_set_stroke_color( ctx, GColorLightGray );
  graphics_context_set_stroke_width( ctx, 1 );
  graphics_draw_circle( ctx, GPoint( layer_bounds.size.w / 2, layer_bounds.size.h / 2 ), layer_bounds.size.w / 2 - 1 );
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
  
  HAND_DRAW_PARAMS hand_draw_params = (HAND_DRAW_PARAMS) {
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
  };
  draw_clock_hand( &hand_draw_params );
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
  
  HAND_DRAW_PARAMS hand_draw_params = (HAND_DRAW_PARAMS) {
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
  };
  draw_clock_hand( &hand_draw_params );
}

static void make_label( TextLayer **p_label, GRect rect,  Layer* parent_layer, const char* str, GFont txt_font, GTextAlignment alignment ) {
  *p_label = text_layer_create( rect );
  text_layer_set_background_color( *p_label, GColorClear );
  text_layer_set_text_color( *p_label, GColorBlack );
  text_layer_set_text_alignment( *p_label, alignment );
  text_layer_set_font( *p_label, txt_font );
  text_layer_set_text( *p_label, str );
  layer_add_child( parent_layer, text_layer_get_layer( *p_label ) );
}

void clock_init( Window *window ){
  window_layer = window_get_root_layer( window );
  window_set_background_color( window, GColorLightGray );
  GRect window_bounds = layer_get_bounds( window_layer );
  HAND_LAYER_DATA *seconds_layer_data = 0;
  HAND_LAYER_DATA *minutes_layer_data = 0;
  
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
  minutes_layer_data = (HAND_LAYER_DATA *) layer_get_data( minutes_layer ); 
  minutes_layer_data->colour = PBL_IF_COLOR_ELSE( 0x000000, 0x000000 );
  minutes_layer_data->length = MINUTES_HAND_LENGTH;
  minutes_layer_data->tail_length = MINUTES_HAND_TAIL_LENGTH;
  minutes_layer_data->width = MINUTES_HAND_THK;
  minutes_layer_data->bounds = MINUTES_RECT_FRAME;
  minutes_layer_data->hub_radius = MINUTES_HUB_RADIUS;
  minutes_layer_data->current_angle = 0;
  minutes_layer_data->next_angle = 0;
  layer_set_update_proc( minutes_layer, minutes_hand_layer_update_proc );
  layer_add_child( outline_layer, minutes_layer );

  seconds_layer = layer_create_with_data( SECONDS_RECT_FRAME, sizeof( HAND_LAYER_DATA ) );
  seconds_layer_data = (HAND_LAYER_DATA *) layer_get_data( seconds_layer ); 
  seconds_layer_data->colour = PBL_IF_COLOR_ELSE( 0x000000, 0x000000 );
  seconds_layer_data->length = SECONDS_HAND_LENGTH;
  seconds_layer_data->tail_length = SECONDS_HAND_TAIL_LENGTH;
  seconds_layer_data->width = SECONDS_HAND_THK;
  seconds_layer_data->bounds = SECONDS_RECT_FRAME;
  seconds_layer_data->hub_radius = SECONDS_HUB_RADIUS;
  seconds_layer_data->current_angle = 0;
  seconds_layer_data->next_angle = 0;
  layer_set_update_proc( seconds_layer, seconds_hand_layer_update_proc );
  layer_add_child( outline_layer, seconds_layer );
  
  GFont txt_font = fonts_get_system_font( FONT_KEY_GOTHIC_14 );
  
  make_label( &label_top_left, LABEL_TOP_LEFT_RECT, window_layer, "  EXIT", txt_font, GTextAlignmentLeft );
  make_label( &label_top_right, LABEL_TOP_RIGHT_RECT, window_layer, "RESET  ", txt_font, GTextAlignmentRight );
  make_label( &label_bottom_right, LABEL_BOTTOM_RIGHT_RECT, window_layer, "START/STOP  ", txt_font, GTextAlignmentRight );
    
  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
}

void clock_deinit( void ) {
  if ( label_top_left ) text_layer_destroy( label_top_left );
  if ( label_top_right ) text_layer_destroy( label_top_right );
  if ( label_bottom_right ) text_layer_destroy( label_bottom_right );
  if ( minutes_layer ) layer_destroy( minutes_layer );
  if ( seconds_layer ) layer_destroy( seconds_layer );
  if ( minutes_dial_layer ) layer_destroy( minutes_dial_layer );
  if ( seconds_dial_layer ) layer_destroy( seconds_dial_layer );
  if ( outline_layer ) layer_destroy( outline_layer );
}
