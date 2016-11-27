//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "utils.h"

void print_pt( char *str, GPoint pt ) {
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "%s: ( %d, %d )", str, pt.x, pt.y );
  #endif
}

void print_rect( char *str, GRect rect ) {
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "%s: ( %d, %d, %d, %d )", str, rect.origin.x, rect.origin.y, rect.size.w, rect.size.h );
  #endif
}

void draw_clock_hand( HAND_DRAW_PARAMS *pDP ) {
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

void make_label( TextLayer **p_label, GRect rect,  Layer* parent_layer, const char* str,
                GFont txt_font, GColor colour, GTextAlignment alignment ) {
  *p_label = text_layer_create( rect );
  text_layer_set_background_color( *p_label, GColorClear );
  text_layer_set_text_color( *p_label, colour );
  text_layer_set_text_alignment( *p_label, alignment );
  text_layer_set_font( *p_label, txt_font );
  text_layer_set_text( *p_label, str );
  layer_add_child( parent_layer, text_layer_get_layer( *p_label ) );
}

void draw_seconds_ticks( Layer *layer, GContext *ctx, GPathInfo gpath_info, int increment, int minor_radius ) {
  GRect bounds = layer_get_bounds( layer );
  GPoint center_pt = grect_center_point( &bounds );
  GPath *gpath = gpath_create( &gpath_info );
  graphics_context_set_antialiased( ctx, true );
  
  for ( int i = 0, angle = 0; i < 60; i += increment ) {
    angle = TRIG_MAX_ANGLE * i / 60;
    gpath_rotate_to( gpath, angle );
    gpath_move_to( gpath, center_pt );
    graphics_context_set_stroke_color( ctx, GColorDarkGray );
    gpath_draw_outline( ctx, gpath );
  }
  graphics_context_set_fill_color( ctx, GColorWhite );
  graphics_fill_circle( ctx, center_pt, minor_radius );
  gpath_destroy( gpath );
}
