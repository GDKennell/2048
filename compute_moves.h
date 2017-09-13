#ifndef COMPUTE_MOVES_H
#define COMPUTE_MOVES_H

typedef int transform_t;
const int NUM_TRANSFORMS = 65536;

void compute_moves(uint64_t *allBoards, size_t tree_size ,unsigned int layer_num, transform_t left_move_transforms[NUM_TRANSFORMS], transform_t right_move_transforms[NUM_TRANSFORMS]);


#endif

