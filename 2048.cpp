#include "SmallBoard.h"
#include "precompute.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <chrono>
#include <ctime>
#include <vector>
#include <math.h>

using namespace std;

struct Block;

typedef SmallBoard board_t;
typedef chrono::system_clock Clock;

enum Direction {UP, DOWN, LEFT, RIGHT, NONE};

const char* direction_names[] = {"up", "down", "left", "right", "none"};

string depth_print(int depth)
{
  string res = "";
  for (int i = 0; i < depth; ++i)
  {
    res += "\t";
  }
  res += to_string(depth) + ": ";
  return res;
}

const bool verbose_logs = false;
const int NUM_TILES = 16;

struct Block {
  int val;
  int x,y;
  bool empty;
  Block() : val(0), x(0), y(0), empty(true){ }
  void print() const {
    ostream& output = cout;
    if(empty) output<<"    ";
    else if(val < 10) output<<" "<<val<<"  ";
    else if(val < 100) output<<" "<<val<<" ";
    else if(val < 1000) output<<val<<' ';
    else output<<val;
  }
};

Block input_block();

void print_board(const board_t& board) {
  ostream& output = cout;
  for(int y = 3; y >= 0; y--) {
    output<<"   ___________________"<<endl;
    output<<"  ";
    for(int x = 0; x <= 3; ++x) {
      output<<'|';
      board.print(x, y);
    }
    output<<'|'<<endl;
  }
  output<<"   ___________________"<<endl;
}

struct Move_Result {
  Move_Result() : combos_val(0){ }
  int combos_val;
  board_t board;
};

Move_Result up_move(const board_t& in_board);
Move_Result down_move(const board_t& in_board);
Move_Result left_move(const board_t& in_board);
Move_Result right_move(const board_t& in_board);

int64_t min(int64_t x1, int64_t x2, int64_t x3, int64_t x4) {
  return min(x1, min(x2, min(x3, x4) ) );
}

int64_t max(int64_t x1, int64_t x2, int64_t x3, int64_t x4) {
  return max(x1, max(x2, max(x3, x4) ) );
}

bool board_full(const board_t& board);

Direction advice(const board_t& board,
                 const board_t& up_result,
                 const board_t& down_result,
                 const board_t& left_result,
                 const board_t& right_result);

void add_new_tile(board_t& board, bool user_in);

int score = 0;
int up_combo_val, down_combo_val, left_combo_val, right_combo_val;

const int NUM_TRANSFORMS = 65536;
typedef int transform_t;
transform_t left_move_transforms[NUM_TRANSFORMS];
transform_t right_move_transforms[NUM_TRANSFORMS];

int empty_vals[NUM_TRANSFORMS];

void load_precompute_files();

board_t input_board();

Direction decide_move(const board_t &board);

Direction decide_move(const board_t &board) {
  // Up move
  up_combo_val = 0;
  Move_Result up_result = up_move(board);
  // Down move
  down_combo_val = 0;
  Move_Result down_result = down_move(board);
  // Left move
  left_combo_val = 0;
  Move_Result left_result = left_move(board);
  // Right move
  right_combo_val = 0;
  Move_Result right_result = right_move(board);

  if (up_result.board == board &&
      down_result.board == board &&
      left_result.board == board &&
      right_result.board == board) {
    return NONE;
  }
  else {
    return advice(board,
                  up_result.board,
                  down_result.board,
                  left_result.board,
                  right_result.board);
  }
}

board_t apply_move(Direction move_direction, const board_t &board, int& score);

int main() {

  load_precompute_files();

  time_t start_time, end_time;
  start_time=Clock::to_time_t(Clock::now());
  srand(time(NULL));

  board_t board = input_board();

  int round_num = 0;
  while(1) {
    cout<<"###############Round "<<++round_num<<endl;
    print_board(board);

    Direction move_decision = decide_move(board);

    board = apply_move(move_decision, board, score);

    if(move_decision == NONE) {
      cout<<"Game Over"<<endl;
      end_time=Clock::to_time_t(Clock::now());
      cerr<<"Score: "<<score<<endl;
      cerr<<"Time: "<<end_time - start_time<<endl;
      if(end_time - start_time != 0) {
        cerr<<"Points/sec: "<<score / (end_time - start_time)<<endl;
      }
      return 0;
    }

    cout<<"\n********Move "<<direction_names[move_decision]<<"********"<<endl;
    cout<<"********Score: "<<score<<"********\n"<<endl;

    return 0;
    add_new_tile(board, false);
    cout<<endl;
  }
}

void load_precompute_files() {
  fstream leftFile ("left.bin", ios::in | ios::binary);
  fstream rightFile ("right.bin", ios::in | ios::binary);
  if (!leftFile.good() || !rightFile.good()) {
    create_move_precompute_files();
    cout<<"Re-creating move precompute files"<<endl;
    leftFile = fstream("left.bin", ios::in | ios::binary);
    rightFile = fstream("right.bin", ios::in | ios::binary);
  }
  else {
    cout<<"Loaded move precompute files"<<endl;
  }

  fstream emptyFile ("numempty.bin", ios::in | ios::binary);
  if (!emptyFile.good()) {
    create_heur_precompute_file();
    cout<<"Re-creating empty tile precompute files"<<endl;
    emptyFile = fstream("numempty.bin", ios::in | ios::binary);
  }
  else {
    cout<<"Loaded empty tile precompute files"<<endl;
  }

  for(int i = 0; i < NUM_TRANSFORMS; ++i) {
    leftFile.read((char*)&left_move_transforms[i], 4);
    rightFile.read((char*)&right_move_transforms[i], 4);
    emptyFile.read((char*)&empty_vals[i], sizeof(int));
  }
}


board_t apply_move(Direction move_direction, const board_t &board, int& score) {
  board_t return_board;

  //Move_Result up_move(const board_t& in_board);
  Move_Result (*move_fn)(const board_t&) = NULL;
  switch(move_direction) {
    case UP:
      move_fn = up_move;
      break;
    case DOWN:
      move_fn = down_move;
      break;
    case LEFT:
      move_fn = left_move;
      break;
    case RIGHT:
      move_fn = right_move;
      break;
    case NONE:
      return board;
      break;
  }

  Move_Result moveResult = move_fn(board);
  score += moveResult.combos_val;
  return moveResult.board;
}

board_t input_board() {
  board_t board;
  const int num_tiles = 5;
  cout<<"Input "<<num_tiles<<" tiles"<<endl;
  for(int i = 0; i < num_tiles; ++i){
    Block new_block = input_block();
    assert(new_block.x-1 >= 0 && new_block.x-1 <= 3);
    assert(new_block.y-1 >= 0 && new_block.y-1 <= 3);
    board.set_val(new_block.x-1, new_block.y-1, new_block.val);
    cout<<endl;
  }
  return board;
}

//returns some evaluation of this board based on number of tiles,
//    number of combos available, highest tile value
int64_t heuristic(const board_t& board) {
  int64_t num_empty = 0;
  for(int64_t c = 0; c < 4; ++c) {
    int64_t column = board.raw_col(c);
    num_empty += empty_vals[column];
  }
  return num_empty;
}

int MAX_DEPTH = 4;
const int INVALID_MOVE_WEIGHT = 0.0;
int TOLERANCE = 10;

int depth = 0;


Block find_highest_tile(const board_t& board) {
  Block highest;
  highest.val = -1;
  for (int x = 0; x < 4; ++x) {
    for (int y = 0; y < 4; ++y) {
      int thisVal = board.val_at(x, y);
      if (thisVal > highest. val) {
        highest.x = x;
        highest.y = y;
        highest.val = thisVal;
      }
    }
  }
  return highest;
}

int64_t eval_board_outcomes(const board_t& board);

int64_t eval_board_moves(const board_t& board) {
  ++depth;
  if (verbose_logs) {
    for (int i = 0; i < depth; ++i) {cout<<"  ";}
    cout<<"Eval moves (d="<<depth<<") start"<<endl;
  }

  Move_Result up_result = up_move(board);
  Move_Result down_result = down_move(board);
  Move_Result left_result = left_move(board);
  Move_Result right_result = right_move(board);

  bool up_valid = (board != up_result.board);
  bool down_valid = (board != down_result.board);
  bool left_valid = (board != left_result.board);
  bool right_valid = (board != right_result.board);

  // If largest tile is on an edge
  Block highestTile = find_highest_tile(board);
  int highestTileRow = board.raw_row(highestTile.y);
  int highestTileCol = board.raw_col(highestTile.x);
  // on left
  if (highestTile.x == 0 && left_valid) {
    transform_t right_move_row = right_move_transforms[highestTileRow] & 0xffff;
    if (right_move_row != highestTileRow) {
      right_valid = false;
    }
  }
  // on right
  if (highestTile.x == 3  && right_valid) {
    transform_t left_move_row = left_move_transforms[highestTileRow] & 0xffff;
    if (left_move_row != highestTileRow) {
      left_valid = false;
    }
  }
  // on top
  if (highestTile.y == 3  && up_valid) {
    transform_t down_move_col = left_move_transforms[highestTileCol] & 0xffff;
    if (down_move_col != highestTileCol) {
      down_valid = false;
    }
  }
  // on bottom
  if (highestTile.y == 0  && down_valid) {
    transform_t up_move_col = right_move_transforms[highestTileCol] & 0xffff;
    if (up_move_col != highestTileCol) {
      up_valid = false;
    }
  }
  // and moving away from that edge causes the largest tile to move
  // and the opposite is valid
  // - invalidate that move away from the edge

  int64_t up_eval, down_eval, left_eval, right_eval;

  if (depth < MAX_DEPTH) {
    up_eval = up_valid ? eval_board_outcomes(up_result.board) : INVALID_MOVE_WEIGHT;
    down_eval = down_valid ? eval_board_outcomes(down_result.board) : INVALID_MOVE_WEIGHT;
    left_eval = left_valid ? eval_board_outcomes(left_result.board) : INVALID_MOVE_WEIGHT;
    right_eval = right_valid ? eval_board_outcomes(right_result.board) : INVALID_MOVE_WEIGHT;
  }
  else {
    up_eval = up_valid ? 10000000 * heuristic(up_result.board) : INVALID_MOVE_WEIGHT;
    down_eval = down_valid ? 10000000 * heuristic(down_result.board) : INVALID_MOVE_WEIGHT;
    left_eval = left_valid ? 10000000 * heuristic(left_result.board) : INVALID_MOVE_WEIGHT;
    right_eval = right_valid ? 10000000 * heuristic(right_result.board) : INVALID_MOVE_WEIGHT;
  }
  if (verbose_logs) {
    for (int i = 0; i < depth; ++i) {cout<<"  ";}
    cout<<"Eval moves (d="<<depth<<") got best move result of "<<max(up_eval, down_eval, left_eval, right_eval)<<endl;
  }
  --depth;
  return max(up_eval, down_eval, left_eval, right_eval);
}

int64_t eval_board_outcomes(const board_t& board) {
  ++depth;
  if (verbose_logs) {
    for (int i = 0; i < depth; ++i) {cout<<"  ";}
    cout<<"Eval outcomes (d="<<depth<<") start"<<endl;
  }
  board_t possible_outcomes[30];
  int num_outcomes = 0;

  if (depth == 1)
    cout<<depth_print(depth)<<" computing outcomes from board: "<<board.raw()<<endl;
  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 4; ++y) {
      if (board.val_at( x, y) == 0) {
        possible_outcomes[num_outcomes] = board;
        possible_outcomes[num_outcomes].set_val(x, y, 2);
        ++num_outcomes;

        possible_outcomes[num_outcomes] = board;
        possible_outcomes[num_outcomes].set_val(x, y, 4);
        ++num_outcomes;
      }
    }
  }
  if(num_outcomes == 0) {
    --depth;
    return 0.0;
  }

  // 2's are weighted as 1.8 while 4's are weighted as 0.2
  int64_t prob2_num = 9;
  int64_t prob4_num = 1;
  int64_t tot_prob = 0;
  for(int i = 0; i < num_outcomes; ++i) {
    int64_t outcome_val = eval_board_moves(possible_outcomes[i]);
    if (verbose_logs) {
      for (int i = 0; i < depth; ++i) {cout<<"  ";}
      cout<<"Eval outcomes (d="<<depth<<") got outcome_val "<<outcome_val<<endl;
    }

    uint64_t prob_factor = (i % 2 == 0) ? prob2_num : prob4_num;
    if (depth == 1)
      cout<<depth_print(depth)<<"tot_prob += prob_factor("<<prob_factor<<" * outcome_val("<<outcome_val<<") =="<<prob_factor * outcome_val<<endl;
    tot_prob += prob_factor * outcome_val;
  }
  if (verbose_logs) {
    for (int i = 0; i < depth; ++i) {cout<<"  ";}
    cout<<"Eval outcomes (d="<<depth<<"): returning tot_prob("<<tot_prob<<")/num_outcomes("<<num_outcomes<<") = "<<tot_prob/num_outcomes<<endl;
  }

  if (depth == 1)
    cout<<depth_print(depth)<<"tot_prob = tot_prob("<<tot_prob<<" / (10 * num_outcomes("<<num_outcomes<<")) = "<<tot_prob / (10 * num_outcomes)<<endl;
  tot_prob = tot_prob / (10 * num_outcomes);
  --depth;

  return tot_prob;
}

Direction advice(const board_t& board,
                 const board_t& up_result,
                 const board_t& down_result,
                 const board_t& left_result,
                 const board_t& right_result) {
  if (verbose_logs)cout<<"\n----------ADVICE--------\n";
  int64_t up_val;
  int64_t down_val;
  int64_t left_val;
  int64_t right_val;

//  int num_empty = heuristic(board);
  MAX_DEPTH = 4;

//  if(num_empty < 2) {
//    TOLERANCE = 50000;
//    MAX_DEPTH = 10;
//  }
//  else if(num_empty < 4) {
//    TOLERANCE = 100000;
//    MAX_DEPTH = 8;
//  }
//  else if(num_empty < 7) {
//    TOLERANCE = 200000;
//    MAX_DEPTH = 6;
//  }
//  else {
//    TOLERANCE = 500000;
//    MAX_DEPTH = 4;
//  }

  bool up_valid = (board != up_result);
  bool right_valid = (board != right_result);
  bool down_valid = (board != down_result);
  bool left_valid = (board != left_result);

  cout<<"evaluating left move"<<endl;
  left_val = left_valid ? eval_board_outcomes(left_result) : -1;
  exit(0);
  cout<<"evaluating right move"<<endl;
  right_val = right_valid ? eval_board_outcomes(right_result) : -1;
  cout<<"evaluating up move"<<endl;
  up_val = up_valid ? eval_board_outcomes(up_result) : -1;
  cout<<"evaluating down move"<<endl;
  down_val = down_valid ? eval_board_outcomes(down_result) : -1;

  int64_t max_val = max(up_val, down_val, left_val, right_val);
  cout<<"\tup_val: "<<up_val<<"\n\tdown_val: "<<down_val<<"\n\tleft_val:"<<left_val<<"\n\tright_val:"<<right_val<<endl;

  if (!up_valid && !down_valid && !left_valid && !right_valid) {
    return NONE;
  }
  else if(up_valid && max_val - up_val < TOLERANCE && max_val - up_val > -TOLERANCE) {
    return UP;
  }
  else if(right_valid && max_val - right_val < TOLERANCE && max_val - right_val > -TOLERANCE) {
    return RIGHT;
  }
  else if (down_valid && max_val - down_val < TOLERANCE && max_val - down_val > -TOLERANCE) {
    return DOWN;
  }
  else {
    return LEFT;
  }
}

bool board_full(const board_t& board) {
  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 4; ++y) {
      if(board.val_at(x,y) == 0) return false;
    }
  }
  return true;
}

void add_new_tile(board_t& board, bool user_in) {
  deque<Block> empty_blocks;
  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 4; ++y) {
      if(board.val_at(x,y) == 0) {
        Block next_block;
        next_block.x = x;
        next_block.y = y;
        next_block.val = board.val_at(x,y);
        empty_blocks.push_back(next_block);
      }
    } 
  }
  assert(!empty_blocks.empty());

  Block block_to_fill;
  if (user_in) {
    block_to_fill= input_block();
    block_to_fill.x--;
    block_to_fill.y--;
  }
  else {
    block_to_fill = empty_blocks[0];
    block_to_fill.val = 2;
//    block_to_fill = empty_blocks[rand() % empty_blocks.size()];
//    block_to_fill.val = (rand() % 100 <= 10) ? 4 : 2;
  }
  cout<<"New block at ("<<block_to_fill.x+1<<',';
  cout<<block_to_fill.y+1<<") with value "<<block_to_fill.val<<endl;

  board.set_val(block_to_fill.x,block_to_fill.y,block_to_fill.val);
}

Block input_block() {
  Block return_block;
  return_block.empty = false;
  cout<<"New block value (2|4): ";
  cin >> return_block.val;
  assert(return_block.val % 2 == 0);

  cout<<"New block coords(x,y): ";
  cin >> return_block.x >> return_block.y;
  assert(return_block.x > 0 && return_block.x <= 4);
  assert(return_block.y > 0 && return_block.y <= 4);
  return return_block;
}

Move_Result up_move(const board_t& in_board) {
  Move_Result result;
  result.board = in_board;
 
  for(int c = 0; c < 4; ++c) {
    int col = in_board.raw_col(c);
    assert(col < NUM_TRANSFORMS);
    transform_t col_transform = right_move_transforms[col];

    result.combos_val += col_transform >> 16;
    assert((col_transform >> 16) <= pow(2,16));
    assert((col_transform >> 16) % 2 == 0);

    result.board.set_col(c, col_transform);
  }

  return result;
}

Move_Result down_move(const board_t& in_board) {
  Move_Result result;
  result.board = in_board;

  for(int c = 0; c < 4; ++c) {
    int col = in_board.raw_col(c);
    assert(col < NUM_TRANSFORMS);
    transform_t col_transform = left_move_transforms[col];

    result.combos_val += col_transform >> 16;
    assert((col_transform >> 16) <= pow(2,16));
    assert((col_transform >> 16) % 2 == 0);

    result.board.set_col(c, col_transform);
  }

  return result;
}

Move_Result left_move(const board_t& in_board) {
  Move_Result result;
  result.board = in_board;

  for(int r = 0; r < 4; ++r) {
    int row = in_board.raw_row(r);
    assert(row < NUM_TRANSFORMS);
    transform_t row_transform = left_move_transforms[row];

    result.combos_val += row_transform >> 16;
    assert((row_transform >> 16) <= pow(2,16));
    assert((row_transform >> 16) % 2 == 0);

    result.board.set_row(r, row_transform);
  }

  return result;
}

Move_Result right_move(const board_t& in_board) {
  Move_Result result;
  result.board = in_board;

  for(int r = 0; r < 4; ++r) {
    int row = in_board.raw_row(r);
    assert(row < NUM_TRANSFORMS);
    transform_t row_transform = right_move_transforms[row];

    result.combos_val += row_transform >> 16;
    assert((row_transform >> 16) <= pow(2,16));
    assert((row_transform >> 16) % 2 == 0);

    result.board.set_row(r, row_transform);
  }

  return result;
}


