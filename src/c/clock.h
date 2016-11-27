//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once

#include <pebble.h>
#include "global.h"

#define ANGLE_PER_SECOND ( TRIG_MAX_ANGLE * 2 / 60 )

void clock_init( Window *window );
void clock_deinit( void );
