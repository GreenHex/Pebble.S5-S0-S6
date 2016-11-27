//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "animation.h"
#include "global.h"

extern Layer *seconds_layer;

static void second_hand_uint32_setter( void *subject, uint32_t angle ) {
  ( (HAND_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_angle = angle;
  layer_mark_dirty( (Layer *) subject );
}

static uint32_t second_hand_uint32_getter( void *subject ) {
  return ( (HAND_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_angle;
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

void start_seconds_animation( void ) {
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
