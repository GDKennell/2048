typedef unsigned long uint64_t;
typedef uint64_t board_t;
typedef uint64_t board_col_t;

constant int NUM_TRANSFORMS = 65536;

//********************************************************************************
//*********************************** Board **************************************
//********************************************************************************
static board_col_t board_raw_column( board_t board_t, int col_num);
static board_col_t board_raw_column( board_t board, int col_num)
{
  int offset = 16 * col_num;
  return (board >> offset) & 0xffff;
}

//********************************************************************************
//****************************** Heuristic logic *********************************
//********************************************************************************


static int num_empty_in_board(board_t board, global int* empty_vals)
{
  int num_empty = 0;
//  for(int c = 0; c < 3; ++c) {
//    board_col_t column = board_raw_column(board, c);
//    num_empty += empty_vals[column];
//    num_empty += column;
//  }
  int empty0 = empty_vals[board_raw_column(board,0)];
  int empty1 = empty_vals[board_raw_column(board,1)];
  int empty2 = empty_vals[board_raw_column(board,2)];
  int empty3 = empty_vals[board_raw_column(board,3)];
  return empty0 + empty1 + empty2 + empty3;
}

kernel void compute_heuristic(global board_t* input_boards,
                              global int* empty_vals,
                              global board_t* output_boards,
                              const size_t count)
{
  size_t index = get_global_id(0);
  size_t orig_index = index / 2;
  board_t board = input_boards[orig_index];
  bool first_half = (index % 2 == 0);
  if (first_half)
  {
    output_boards[index] = 10000000 * (empty_vals[board_raw_column(board,0)] + empty_vals[board_raw_column(board,1)]);
  }
  else
  {
    output_boards[index] = 10000000 * (empty_vals[board_raw_column(board,2)] + empty_vals[board_raw_column(board,3)]);
  }
}
