//
//  board.c
//  hello
//
//  Created by Grant Kennell on 8/22/17.
//
//

#include "board.h"

board_col_t board_raw_column( board_t board, int col_num)
{
    int offset = 16 * col_num;
    return (board >> offset) & 0xffff;
}

void board_set_exp( board_t *board, int x, int y, int exp)
{
    int offset = 4 * (4 * x + y);

    *board &= ~((uint64_t)15 << offset);
    *board |= ((uint64_t)exp << offset);
}

void board_set_value( board_t *board, int x, int y, int val)
{
    int exp = log2(val);
    board_set_exp(board,x,y,exp);
}

