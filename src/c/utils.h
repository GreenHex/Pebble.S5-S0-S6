//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once
#include <pebble.h>
#include "global.h"

typedef struct {
  GContext *ctx;
  GPoint center_pt;
  GPoint from_pt;
  GPoint to_pt;
  uint8_t hand_width;
  GColor hand_color;
  GColor hand_outline_color;
  uint16_t dot_radius;
  GColor dot_color;
  GColor dot_outline_color;
} HAND_DRAW_PARAMS;

void print_pt( char *str, GPoint pt );
void print_rect( char *str, GRect rect );
void draw_clock_hand( HAND_DRAW_PARAMS *pDP );
void make_label( TextLayer **p_label, GRect rect,  Layer* parent_layer, const char* str,
                GFont txt_font, GColor colour, GTextAlignment alignment );
