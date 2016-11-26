//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once

#include <pebble.h>
#include "global.h"

#define ANGLE_PER_SECOND ( TRIG_MAX_ANGLE * 2 / 60 )

#define OUTLINE_LAYER_WIDTH ( PBL_DISPLAY_WIDTH - 2 ) 
#define OUTLINE_LAYER_HEIGHT OUTLINE_LAYER_WIDTH 
#define OUTLINE_LAYER_POS_X ( PBL_DISPLAY_WIDTH / 2 - OUTLINE_LAYER_WIDTH / 2 )
#define OUTLINE_LAYER_POS_Y ( PBL_DISPLAY_HEIGHT / 2 - OUTLINE_LAYER_HEIGHT / 2 )
#define OUTLINE_RECT_FRAME ( GRect( OUTLINE_LAYER_POS_X, OUTLINE_LAYER_POS_Y, OUTLINE_LAYER_WIDTH, OUTLINE_LAYER_HEIGHT ) )

#define SECONDS_LAYER_WIDTH ( OUTLINE_LAYER_WIDTH - 6 )
#define SECONDS_LAYER_HEIGHT SECONDS_LAYER_WIDTH
#define SECONDS_LAYER_POS_X ( OUTLINE_LAYER_WIDTH / 2 - SECONDS_LAYER_WIDTH / 2 )
#define SECONDS_LAYER_POS_Y ( OUTLINE_LAYER_HEIGHT / 2 - SECONDS_LAYER_HEIGHT / 2 )
#define SECONDS_RECT_FRAME ( GRect( SECONDS_LAYER_POS_X, SECONDS_LAYER_POS_Y, SECONDS_LAYER_WIDTH, SECONDS_LAYER_HEIGHT ) )

#define SECONDS_HAND_LENGTH ( SECONDS_LAYER_WIDTH / 2 - 10 )
#define SECONDS_HAND_TAIL_LENGTH 20
#define SECONDS_HAND_THK 1
#define SECONDS_HUB_RADIUS 3

#define MINUTES_LAYER_WIDTH ( 50 )
#define MINUTES_LAYER_HEIGHT MINUTES_LAYER_WIDTH
#define MINUTES_LAYER_POS_X ( SECONDS_LAYER_WIDTH / 2 - MINUTES_LAYER_WIDTH / 2 )
#define MINUTES_LAYER_POS_Y ( SECONDS_LAYER_HEIGHT / 4 - MINUTES_LAYER_HEIGHT / 2 )
#define MINUTES_RECT_FRAME ( GRect( MINUTES_LAYER_POS_X, MINUTES_LAYER_POS_Y, MINUTES_LAYER_WIDTH, MINUTES_LAYER_HEIGHT ) )

#define MINUTES_HAND_LENGHT ( MINUTES_LAYER_WIDTH / 2 - 7 )
#define MINUTES_HAND_TAIL_LENGTH 3
#define MINUTES_HAND_THK 1
#define MINUTES_HUB_RADIUS 3

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
