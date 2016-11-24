//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once

#include <pebble.h>
#include "global.h"

#define SECONDS_LAYER_WIDTH PBL_DISPLAY_WIDTH
#define SECONDS_LAYER_HEIGHT PBL_DISPLAY_WIDTH
#define SECONDS_LAYER_POS_X 0
#define SECONDS_LAYER_POS_Y 0
#define SECONDS_RECT_FRAME ( GRect( SECONDS_LAYER_POS_X, SECONDS_LAYER_POS_Y, SECONDS_LAYER_WIDTH, SECONDS_LAYER_HEIGHT ) )

#define SECONDS_HAND_LENGTH ( PBL_DISPLAY_WIDTH / 2 - 5 )
#define SECONDS_HAND_TAIL_LENGTH 20
#define SECONDS_HAND_THK 1
#define SECONDS_HUB_RADIUS 3

typedef struct {
  uint32_t colour;
  uint16_t length;
  uint16_t tail_length;
  uint8_t width;
  uint16_t hub_radius;
  GRect bounds;
  uint32_t current_angle;
  uint32_t next_angle;
} HAND_LAYER_DATA;

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

void clock_init( Window *window );
void clock_deinit( void );