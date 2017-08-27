//
//  compute_moves_opencl.c
//  scan
//
//  Created by Grant Kennell on 8/27/17.
//

typedef __int64_t board_col_t;
typedef __int64_t board_row_t;
typedef __uint64_t board_t;
typedef __uint64_t uint64_t;
typedef int transform_t;

const uint64_t UNUSED_BOARD = 0;

enum Direction {LEFT, RIGHT, UP, DOWN, NONE};

//**************************************************************
//********************** Board operations **********************
//**************************************************************

board_col_t board_raw_col( board_t board_t, int col_num);
board_row_t board_raw_row( board_t board_t, int row_num);

void board_set_col( board_t *board, int col_num, board_col_t col);
void board_set_row( board_t *board, int row_num, board_row_t row);


board_col_t board_raw_col( board_t board, int col_num)
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
    if (board == NULL) { return;}
    int offset = 16 * col_num;
    *board &= ~((uint64_t)0xffff << offset);
    *board |= ((uint64_t)col << offset);
}

void board_set_row( board_t *board, int row_num, board_row_t row)
{
    if (board == NULL) { return;}
    for(int x = 0; x < 4; ++x) {
        int offset = (16 * x) + (4 * row_num);
        int val = ((row >> (4*x)) & (uint64_t)0xf);
        *board &= ~((uint64_t)0xf << offset);
        *board |= ((uint64_t)val << offset);
    }
}


//**************************************************************
//********************** Move calculations *********************
//**************************************************************


board_t up_move(const board_t in_board) {
    board_t result = in_board;

    for(int c = 0; c < 4; ++c) {
        int col = board_raw_col(in_board,c);
        transform_t col_transform = right_move_transforms[col];

        board_set_col(result,c,col_transform);
    }

    return result;
}

board_t down_move(const board_t in_board) {
    board_t result = in_board;

    for(int c = 0; c < 4; ++c) {
        int col = board_raw_col(in_board,c);
        transform_t col_transform = left_move_transforms[col];

        board_set_col(&result, c, col_transform);
    }

    return result;
}

board_t left_move(const board_t in_board) {
    board_t result = in_board;

    for(int r = 0; r < 4; ++r) {
        int row = board_raw_row(in_board,r);
        transform_t row_transform = left_move_transforms[row];

        board_set_row(&result, r, row_transform);
    }

    return result;
}

board_t right_move(const board_t in_board,
                   transform_t* right_move_transforms,
                   transform_t* left_move_transforms) {
    board_t result = in_board;

    for(int r = 0; r < 4; ++r) {
        int row = board_raw_row(in_board,r);
        transform_t row_transform = right_move_transforms[row];

        board_set_row(&result, r, row_transform);
    }

    return result;
}



board_t move_in_direction(const board_t in_board,
                          Direction direction,
                          transform_t* right_move_transforms,
                          transform_t* left_move_transforms)
{
    switch (direction) {
        case UP:
            return up_move(in_board, right_move_transforms, left_move_transforms);
        case DOWN:
            return down_move(in_board, right_move_transforms, left_move_transforms);
        case LEFT:
            return left_move(in_board), right_move_transforms, left_move_transforms;
        case RIGHT:
            return right_move(in_board, right_move_transforms, left_move_transforms);
    }
    return 0;
}

//**************************************************************
//********************** Layer calculations ********************
//**************************************************************


int start_of_layer(int layer_num)
{
    int layerStart = 0;
    int layerSize = 4;
    for (int i = 1; i <= layer_num; ++i)
    {
        layerStart += layerSize;
        int layerMultiplier = (i % 2 == 0) ? 4 : 30;
        layerSize *= layerMultiplier;
    }
    return layerStart;
}


int size_of_layer(int layer_num)
{
    int layerSize = 4;
    for (int i = 1; i <= layer_num; ++i)
    {
        int layerMultiplier = (i % 2 == 0) ? 4 : 30;
        layerSize *= layerMultiplier;
    }
    return layerSize;
}

int layer_for_index(int index)
{
    int layerNum = 0;
    while (start_of_layer(layerNum + 1) <= index)
    {
        ++layerNum;
    }
    return layerNum;
}

//**************************************************************
//********************** Layer Move Calculation ****************
//**************************************************************


__kernel void compute_moves (__global board_t* entire_move_tree,
                             __global transform_t* right_move_transforms,
                             __global transform_t* left_move_transforms,
                             __global int* orig_index,
                             __global int* next_move_i,
                             const unsigned int count)
{
    unsigned int i = get_global_id(0);
    if(i < count)
    {
        int orig_index = i;
        int orig_layer = layer_for_index(orig_index);
        int orig_layer_index = orig_index - start_of_layer(orig_layer);
        int next_move_i = start_of_layer(orig_layer + 1) + 4 * orig_layer_index;
        board_t orig_board = entire_move_tree[orig_index];
        if (orig_board == UNUSED_BOARD){ return; }
        for (int i = 0; i < 4; ++i)
        {
            board_t moveResult = move_in_direction(orig_board, (Direction)i, right_move_transforms, left_move_transforms);
            if (moveResult.board.raw() != orig_board)
            {
                entire_move_tree[next_move_i] = moveResult;
            }
            ++next_move_i;
        }
    }
}
