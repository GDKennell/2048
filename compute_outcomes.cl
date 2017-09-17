typedef unsigned long uint64_t;

typedef uint64_t board_t;

__constant board_t UNUSED_BOARD = 0;

int board_exp_at(board_t board, int x, int y)
{
    int offset = 4 * (4 * x + y);
    int exp = (board >> offset) & (uint64_t)0xf;
    return exp;
}

void board_set_exp(board_t *board, int x, int y, int exp) {
    int offset = 4 * (4 * x + y);
    *board &= ~((uint64_t)0xf << offset);
    *board |= ((uint64_t)exp << offset);
}

kernel void compute_outcomes(global board_t* input_boards,
                             global board_t* output_boards,
                             const size_t count)
{
    size_t orig_index = get_global_id(0);

    output_boards[orig_index] = 0;
    if (orig_index >= count) { return; }

    board_t orig_board = input_boards[orig_index];
    if (orig_board == UNUSED_BOARD)
    {
        return;
    }

    uint64_t next_outcome_i = 30 * orig_index;
    uint64_t end_of_outcomes = next_outcome_i + 30;

    for(int x = 0; x < 4; ++x) {
        for(int y = 0; y < 4; ++y) {
            if (board_exp_at(orig_board,x,y) == 0) {
                board_t outcome2 = orig_board;
                board_set_exp(&outcome2,x,y,1);
                output_boards[next_outcome_i++] = outcome2;

                board_t outcome4 = orig_board;
                board_set_exp(&outcome4,x,y,2);
                output_boards[next_outcome_i++] = outcome4;
            }
        }
    }
}

