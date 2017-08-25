
typedef unsigned long board_t;
typedef unsigned long uint64_t;


uint64_t raw_column( board_t board, int col_num);
uint64_t raw_column( board_t board, int col_num)
{
    int offset = 16 * col_num;
    return (board >> offset) & 0xffff;
}

uint64_t heuristic(board_t board, __global int* empty_vals)
{
    uint64_t result = 0;
    for (int i = 0; i < 4; ++i)
    {
        uint64_t column = raw_column(board,i);
        uint64_t col_num_empty = empty_vals[column];
        result += col_num_empty;
    }
    return result;
//    uint64_t column = raw_column(board,0);
//    uint64_t col_num_empty = empty_vals[column];
//
//    uint64_t column1 = raw_column(board,1);
//    uint64_t col_num_empty1 = empty_vals[column1];
//
//    uint64_t column2 = raw_column(board,2);
//    uint64_t col_num_empty2 = empty_vals[column2];
//
//    uint64_t column3 = raw_column(board,3);
//    uint64_t col_num_empty3 = empty_vals[column3];

    return 0;
}

void calculate_heuristic(__global board_t* input_boards,
                         __global int* empty_vals,
                         __global uint64_t* output,
                         const int index)
{
    output[index] = 0;
    for (int i = 0; i < 4; ++i)
    {
        uint64_t column = raw_column(input_boards[index],i);
        uint64_t col_num_empty = empty_vals[column];

        output[index] += col_num_empty;
    }
}

__kernel void calculate_heuristics (__global board_t* input_boards,
                                    __global int* empty_vals,
                                    __global uint64_t* output,
                                    const unsigned int count)
{
    unsigned int i = get_global_id(0);
    if(i < count)
    {
        calculate_heuristic(input_boards, empty_vals, output, i);
    }
}

