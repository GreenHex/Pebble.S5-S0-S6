//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once
#include <pebble.h>

#define NUM_ANIMATIONS 1

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

void start_seconds_animation( void );
