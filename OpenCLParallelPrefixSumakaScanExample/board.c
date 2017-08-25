//
//  board.c
//  hello
//
//  Created by Grant Kennell on 8/22/17.
//
//

#include "board.h"
#include <assert.h>

board_col_t board_raw_column( board_t board, int col_num)
{
    int offset = 16 * col_num;
    return (board >> offset) & 0xffff;
}

board_row_t board_raw_row(board_t board, int row_num)
{
    board_row_t row = 0;
    for(int x = 0; x < 4; ++x) {
        int offset = (16 * x) + (4 * row_num);
        row |= ((board >> offset) & 0xf) << (4 * x);
    }
    return row;
}

void board_set_col( board_t *board, int col_num, board_col_t col)
{
    assert(board != NULL);
    int offset = 16 * col_num;
    *board &= ~((uint64_t)0xffff << offset);
    *board |= ((uint64_t)col << offset);
}

void board_set_row( board_t *board, int row_num, board_row_t row)
{
    assert(board != NULL);
    for(int x = 0; x < 4; ++x) {
        int offset = (16 * x) + (4 * row_num);
        int val = ((row >> (4*x)) & (uint64_t)0xf);
        *board &= ~((uint64_t)0xf << offset);
        *board |= ((uint64_t)val << offset);
    }
}


void board_set_exp( board_t *board, int x, int y, int exp)
{
    assert(board != NULL);
    int offset = 4 * (4 * x + y);

    *board &= ~((uint64_t)15 << offset);
    *board |= ((uint64_t)exp << offset);
}

void board_set_value( board_t *board, int x, int y, int val)
{
    assert(board != NULL);
    int exp = log2(val);
    board_set_exp(board,x,y,exp);
}

