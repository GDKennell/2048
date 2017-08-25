//
//  move.h
//  hello
//
//  Created by Grant Kennell on 8/22/17.
//
//

#ifndef move_h
#define move_h

#include <stdio.h>
#include "board.h"

struct Move_Result {
    int combos_val;
    board_t board;
};

void setup_moves();

struct Move_Result up_move(board_t in_board);
struct Move_Result down_move(board_t in_board);
struct Move_Result left_move(board_t in_board);
struct Move_Result right_move(board_t in_board);

#endif /* move_h */
