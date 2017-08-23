

int64_t raw_column(__global uint64_t board, int col_num)
{
    int offset = 16 * col_num;
    return (board >> offset) & 0xffff;
}

uint64_t heuristic(__global uint64_t board, __global int* empty_vals)
{
  __int64_t num_empty = 0;
  for(__int64_t c = 0; c < 4; ++c) {
    __int64_t column = raw_column(board,c);
    num_empty += empty_vals[column];
  }
  return num_empty;
}

__kernel void calculate_heuristics (__global uint64_t* boards,
                                    __global int* empty_vals,
                                    __global uint64_t* output,
                                    const unsigned int count)
{
    int i = get_global_id(0);
    if(i < count)
        output[i] = heuristic(input[i], empty_vals);
}

