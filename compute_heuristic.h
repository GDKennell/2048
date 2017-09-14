#ifndef COMPUTE_HEURISTIC_H
#define COMPUTE_HEURISTIC_H

typedef int transform_t;
extern const int NUM_TRANSFORMS;

void compute_heuristics(uint64_t *allBoards,unsigned int layer_num, int empty_vals[NUM_TRANSFORMS]);

#endif
