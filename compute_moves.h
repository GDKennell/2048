#ifndef COMPUTE_MOVES_H
#define COMPUTE_MOVES_H

#include "constants.h"

typedef int transform_t;

void compute_moves(uint64_t *allBoards,unsigned int layer_num, transform_t left_move_transforms[NUM_TRANSFORMS], transform_t right_move_transforms[NUM_TRANSFORMS]);


#endif

