//
//  move.c
//  hello
//
//  Created by Grant Kennell on 8/22/17.
//
//

#include <assert.h>
#include <math.h>
#include "move.h"
typedef __uint32_t transform_t;

static const int NUM_TRANSFORMS = 65536;

transform_t left_move_transforms[NUM_TRANSFORMS];
transform_t right_move_transforms[NUM_TRANSFORMS];

void setup_moves()
{
    FILE *ptr;
    ptr = fopen("/Users/grantke/Desktop/Stuff/2048/right.bin","rb");  // r for read, b for binary
    fread(right_move_transforms, sizeof(transform_t), NUM_TRANSFORMS, ptr);
    fclose(ptr);

    ptr = fopen("/Users/grantke/Desktop/Stuff/2048/left.bin", "rb");
    fread(left_move_transforms, sizeof(transform_t), NUM_TRANSFORMS, ptr);
    fclose(ptr);
}

struct Move_Result up_move(board_t in_board)
{
    struct Move_Result result;
    result.board = in_board;

    for(int c = 0; c < 4; ++c) {
        int col = board_raw_column(in_board,c);
        assert(col < NUM_TRANSFORMS);
        transform_t col_transform = right_move_transforms[col];

        result.combos_val += col_transform >> 16;
        assert((col_transform >> 16) <= pow(2,16));
        assert((col_transform >> 16) % 2 == 0);

        board_set_col(&result.board, c, col_transform);
    }
    return result;
}

struct Move_Result down_move(board_t in_board)
{
    struct Move_Result result;
    result.board = in_board;

    for(int c = 0; c < 4; ++c) {
        int col = board_raw_column(in_board,c);
        assert(col < NUM_TRANSFORMS);
        transform_t col_transform = left_move_transforms[col];

        result.combos_val += col_transform >> 16;
        assert((col_transform >> 16) <= pow(2,16));
        assert((col_transform >> 16) % 2 == 0);

        board_set_col(&result.board, c, col_transform);
    }
    return result;
}

struct Move_Result left_move(board_t in_board)
{
    struct Move_Result result;
    result.board = in_board;

    for(int r = 0; r < 4; ++r) {
        int row = board_raw_row(in_board,r);
        assert(row < NUM_TRANSFORMS);
        transform_t row_transform = left_move_transforms[row];

        result.combos_val += row_transform >> 16;
        assert((row_transform >> 16) <= pow(2,16));
        assert((row_transform >> 16) % 2 == 0);

        board_set_row(&result.board, r, row_transform);
    }

    return result;
}

struct Move_Result right_move(board_t in_board)
{
    struct Move_Result result;
    result.board = in_board;

    for(int r = 0; r < 4; ++r) {
        int row = board_raw_row(in_board,r);
        assert(row < NUM_TRANSFORMS);
        transform_t row_transform = right_move_transforms[row];

        result.combos_val += row_transform >> 16;
        assert((row_transform >> 16) <= pow(2,16));
        assert((row_transform >> 16) % 2 == 0);
        
        board_set_row(&result.board, r, row_transform);
    }
    
    return result;
}

