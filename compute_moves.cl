typedef unsigned long uint64_t;

typedef uint64_t board_t;

typedef uint64_t board_col_t;
typedef uint64_t board_row_t;

typedef int transform_t;

__constant board_t UNUSED_BOARD = 0;

//********************************************************************************
//*********************************** Board **************************************
//********************************************************************************

board_col_t board_raw_column( board_t board_t, int col_num);
board_row_t board_raw_row( board_t board_t, int row_num);

void board_set_col( board_t *board, int col_num, board_col_t col);
void board_set_row( board_t *board, int row_num, board_row_t row);

void board_set_exp( board_t *board, int x, int y, int exp);

void board_set_value( board_t *board, int x, int y, int val);

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
    int offset = 16 * col_num;
    *board &= ~((uint64_t)0xffff << offset);
    *board |= ((uint64_t)col << offset);
}

void board_set_row( board_t *board, int row_num, board_row_t row)
{
    for(int x = 0; x < 4; ++x) {
        int offset = (16 * x) + (4 * row_num);
        int val = ((row >> (4*x)) & (uint64_t)0xf);
        *board &= ~((uint64_t)0xf << offset);
        *board |= ((uint64_t)val << offset);
    }
}


void board_set_exp( board_t *board, int x, int y, int exp)
{
    int offset = 4 * (4 * x + y);

    *board &= ~((uint64_t)15 << offset);
    *board |= ((uint64_t)exp << offset);
}

//void board_set_value( board_t *board, int x, int y, int val)
//{
//    int exp = log2(val);
//    board_set_exp(board,x,y,exp);
//}

//********************************************************************************
//**************************** Move calculations *********************************
//********************************************************************************

enum Direction {LEFT, RIGHT, UP, DOWN, NONE};

board_t up_move(board_t in_board, __global transform_t* right_transforms);
board_t down_move(board_t in_board, __global transform_t* left_transforms);
board_t left_move(board_t in_board, __global transform_t* left_transforms);
board_t right_move(board_t in_board, __global transform_t* right_transforms);

board_t move_in_direction(board_t in_board, enum Direction direction, __global transform_t* left_transforms, __global transform_t* right_transforms)
{
    switch (direction) {
        case UP:
            return up_move(in_board, right_transforms);
        case DOWN:
            return down_move(in_board, left_transforms);
        case LEFT:
            return left_move(in_board, left_transforms);
        case RIGHT:
            return right_move(in_board, right_transforms);
        default:
            break;
    }
    return UNUSED_BOARD;
}

board_t up_move(board_t in_board, __global transform_t* right_transforms)
{
    board_t result = UNUSED_BOARD;

    for(int c = 0; c < 4; ++c) {
        int col = board_raw_column(in_board,c);
        transform_t col_transform = right_transforms[col];

        board_set_col(&result,c,col_transform);
    }

    return result;
}

board_t down_move(board_t in_board, __global transform_t* left_transforms)
{
    board_t result = UNUSED_BOARD;

    for(int c = 0; c < 4; ++c) {
        int col = board_raw_column(in_board,c);
        transform_t col_transform = left_transforms[col];

        board_set_col(&result,c,col_transform);
    }

    return result;
}

board_t left_move(board_t in_board, __global transform_t* left_transforms)
{
    board_t result = UNUSED_BOARD;

    for(int r = 0; r < 4; ++r) {
        int row = board_raw_row(in_board,r);
        transform_t row_transform = left_transforms[row];

        board_set_row(&result, r, row_transform);
    }

    return result;
}

board_t right_move(board_t in_board, __global transform_t* right_transforms)
{
    board_t result = UNUSED_BOARD;

    for(int r = 0; r < 4; ++r) {
        int row = board_raw_row(in_board,r);
        transform_t row_transform = right_transforms[row];

        board_set_row(&result,r,row_transform);
    }

    return result;
}

//********************************************************************************
//*************************** Layer calculations *********************************
//********************************************************************************

uint64_t start_of_layer(int layer_num)
{
    uint64_t layerStart = 0;
    uint64_t layerSize = 4;
    for (int i = 1; i <= layer_num; ++i)
    {
        layerStart += layerSize;
        uint64_t layerMultiplier = (i % 2 == 0) ? 4 : 30;
        layerSize *= layerMultiplier;
    }
    return layerStart;
}

int layer_for_index(uint64_t index)
{
    int layerNum = 0;
    while (start_of_layer(layerNum + 1) <= index)
    {
        ++layerNum;
    }
    return layerNum;
}

//********************************************************************************
//********************************* Kernel ***************************************
//********************************************************************************

__kernel void compute_moves(__global board_t* input_boards,
                            __global transform_t* left_transforms,
                            __global transform_t* right_transforms,
                            __global board_t* output_boards,
                            __global size_t* count)
{
    size_t orig_index = get_global_id(0);
    if (orig_index >= *count) { return; }

    board_t orig_board = input_boards[orig_index];
    uint64_t next_move_i = 4 * orig_index;

    for (int i = 0; i < 4; ++i)
    {
        board_t moveResult = move_in_direction(orig_board, (enum Direction)i, left_transforms, right_transforms);
        bool move_valid = moveResult != orig_board;
        uint64_t result = move_valid ? moveResult : UNUSED_BOARD;
        output_boards[next_move_i] = result;
        ++next_move_i;
    }

}

