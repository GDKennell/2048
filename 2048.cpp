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
#include <math.h>
#include <vector>

using namespace std;

struct Block;

typedef SmallBoard board_t;
typedef chrono::system_clock Clock;

enum Direction {UP, DOWN, LEFT, RIGHT, NONE};

const char* direction_names[] = {"up", "down", "left", "right", "none"};

const bool verbose_logs = false;

const unsigned int NUM_TILES = 16;
const unsigned int MAX_DISTANCE = 6;

struct Block {
  int val;
  int x,y;
  bool empty;
  Block(int x_, int y_, int val_) { x = x_, y = y_, val = val_;}
  Block() : val(0), x(0), y(0), empty(true){ }
  void print() const {
    ostream& output = cout;
    if(empty) output<<"    ";
    else if(val < 10) output<<" "<<val<<"  ";
    else if(val < 100) output<<" "<<val<<" ";
    else if(val < 1000) output<<val<<' ';
    else output<<val;
  }
  int distanceToBlock(const Block &other) const {
    return abs(other.x - x );
  }
};

int distanceBetween(const Block& b1, const Block& b2) {
  return abs(b1.x - b2.x) + abs(b1.y - b2.y);
}

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

float max(float x1, float x2, float x3, float x4) {
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
vector<Block> get_all_blocks(const board_t& board);

int main() {

  load_precompute_files();

  time_t start_time, end_time;
  start_time=Clock::to_time_t(Clock::now());
  cout<<"seeding with time "<<time(NULL)<<endl;
  srand(time(NULL));

  board_t board;// = input_board();
  board.set_val(0,0,2);
  board.set_val(1,0,2);
//  board.set_val(2,0,4);
//  board.set_val(3,0,32);
//  board.set_val(3,1,2);

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
  const int num_tiles = 2;
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

int get_num_empty(const board_t& board) {
    int num_empty = 0;
    for(int64_t c = 0; c < 4; ++c) {
        int64_t column = board.raw_col(c);
        num_empty += empty_vals[column];
    }
    return num_empty;
}

// array of ideal squares
Block* IdealSquares = NULL;

bool compareBlocksByValue(Block b1, Block b2) {
  return b1.val > b2.val;
}


class IsCloserToBlock
{
public:
  Block comparisonBlock;
  IsCloserToBlock(const Block &block) {
    comparisonBlock = block;
  }
  bool operator()(Block b1, Block b2) {
    return distanceBetween(b1,comparisonBlock) < distanceBetween(b2, comparisonBlock);
  }
};

// Takes an array of blocks sorted by value
// Does secondary sort within groups of tiles with same value
// Sorting by closeness to next higher tile
// e.g. in a group of 16's, the top sorted will be the one that's closes to a 32
void fix_sort_by_nearest_neighbor(vector<Block> &blocks) {
  int blocksCount = 0, blocksStart = 0;
  Block equalBlocks[NUM_TILES];
  for (int i = 0; i < blocks.size(); ++i) {
    if (blocks[i].val == 0) {
      if (blocksCount > 1) {
        Block nextHighestIndex = blocksStart == 0 ? Block(0, 0, 0) : blocks[blocksStart - 1];
        sort(blocks.begin() + blocksStart, blocks.begin() + blocksStart + blocksCount, IsCloserToBlock(nextHighestIndex));
      }
      break;
    }
    if (blocksCount == 0) {
      equalBlocks[blocksCount++] = blocks[i];
      blocksStart = i;
    }
    else if (blocks[i].val == equalBlocks[0].val) {
      equalBlocks[blocksCount++] = blocks[i];
    }
    else if (blocksCount == 1){
      blocksStart = i;
      blocksCount = 0;
      equalBlocks[blocksCount++] = blocks[i];
    }
    else {
      Block nextHighestIndex = blocksStart == 0 ? Block(0, 0, 0) : blocks[blocksStart - 1];
      sort(blocks.begin() + blocksStart, blocks.begin() + blocksStart + blocksCount, IsCloserToBlock(nextHighestIndex));
      // Sort equalBlocks by
    }
  }
}

//returns some evaluation of this board based on number of tiles,
//    number of combos available, highest tile value
float heuristic(const board_t& board) {
  vector<Block> allBlocks = get_all_blocks(board);

  sort(allBlocks.begin(),allBlocks.begin() + allBlocks.size(), compareBlocksByValue);

  fix_sort_by_nearest_neighbor(allBlocks);

  float totalHeuristic = 0.0;
  for (int i = 0; i < allBlocks.size(); ++i) {
    Block realBlock = allBlocks[i];
    Block idealBlock = i == 0 ? Block(0, 0, 0) : allBlocks[i-1];
    int distanceToIdeal = distanceBetween(realBlock,idealBlock);
    float dist_heur = pow((float)(MAX_DISTANCE - distanceToIdeal), 4.0);
    float val_heur = pow((float)allBlocks[i].val, 4.0);
    totalHeuristic += dist_heur * val_heur;
  }
  int totalNumEmpty = get_num_empty(board);
  totalHeuristic *= (pow((float)totalNumEmpty,2) / pow((float)NUM_TILES,2));
  return totalHeuristic;
  // return get_num_empty(board);
}

int MAX_DEPTH = 4;
const int INVALID_MOVE_WEIGHT = 0.0;
int TOLERANCE = 10;

int depth = 0;

float eval_board_outcomes(const board_t& board);

const int HEUR_MULTIPLIER = 1;

vector<Block> get_all_blocks(const board_t& board) {
  Block blocksArray[NUM_TILES];
  int blockCount = 0;
  for (int x = 0; x < 4; ++x) {
    for (int y = 0; y < 4; ++y) {
      int value = board.val_at(x,y);
      Block block;
      block.x = x;
      block.y = y;
      block.val = value;
      block.empty = false;

      if (value > 0) {
        blocksArray[blockCount++] = block;
      }
    }
  }
  vector<Block> allBlocks(blocksArray,blocksArray + blockCount);
  return allBlocks;
}

Block find_highest_tile(const board_t& board) {
  auto boardBlocks = get_all_blocks(board);
  sort(boardBlocks.begin(),boardBlocks.end(), compareBlocksByValue);

  return boardBlocks[0];
}

float eval_board_moves(const board_t& board) {
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

  float up_eval, down_eval, left_eval, right_eval;

  if (depth < MAX_DEPTH) {
    up_eval = up_valid ? eval_board_outcomes(up_result.board) : INVALID_MOVE_WEIGHT;
    down_eval = down_valid ? eval_board_outcomes(down_result.board) : INVALID_MOVE_WEIGHT;
    left_eval = left_valid ? eval_board_outcomes(left_result.board) : INVALID_MOVE_WEIGHT;
    right_eval = right_valid ? eval_board_outcomes(right_result.board) : INVALID_MOVE_WEIGHT;
  }
  else {
    up_eval = up_valid ? HEUR_MULTIPLIER * heuristic(up_result.board) : INVALID_MOVE_WEIGHT;
    down_eval = down_valid ? HEUR_MULTIPLIER * heuristic(down_result.board) : INVALID_MOVE_WEIGHT;
    left_eval = left_valid ? HEUR_MULTIPLIER * heuristic(left_result.board) : INVALID_MOVE_WEIGHT;
    right_eval = right_valid ? HEUR_MULTIPLIER * heuristic(right_result.board) : INVALID_MOVE_WEIGHT;
  }
  if (verbose_logs) {
    for (int i = 0; i < depth; ++i) {cout<<"  ";}
    cout<<"Eval moves (d="<<depth<<") got best move result of "<<max(up_eval, down_eval, left_eval, right_eval)<<endl;
  }
  --depth;
  return max(up_eval, down_eval, left_eval, right_eval);
}

float eval_board_outcomes(const board_t& board) {
  ++depth;
  if (verbose_logs) {
    for (int i = 0; i < depth; ++i) {cout<<"  ";}
    cout<<"Eval outcomes (d="<<depth<<") start"<<endl;
  }
  board_t possible_outcomes[30];
  int num_outcomes = 0;

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
    float outcome_val = eval_board_moves(possible_outcomes[i]);
    if (verbose_logs) {
      for (int i = 0; i < depth; ++i) {cout<<"  ";}
      cout<<"Eval outcomes (d="<<depth<<") got outcome_val "<<outcome_val<<endl;
    }

    if(i % 2 == 0)
      tot_prob += prob2_num * outcome_val;
    else
      tot_prob += prob4_num * outcome_val;
  }
  if (verbose_logs) {
    for (int i = 0; i < depth; ++i) {cout<<"  ";}
    cout<<"Eval outcomes (d="<<depth<<"): returning tot_prob("<<tot_prob<<")/num_outcomes("<<num_outcomes<<") = "<<tot_prob/num_outcomes<<endl;
  }

  --depth;
  tot_prob = tot_prob / (10 * num_outcomes);
  return tot_prob;
}

Direction advice(const board_t& board,
                 const board_t& up_result,
                 const board_t& down_result,
                 const board_t& left_result,
                 const board_t& right_result) {
  if (verbose_logs)cout<<"\n----------ADVICE--------\n";
  float up_val;
  float down_val;
  float left_val;
  float right_val;

  int num_empty = get_num_empty(board);
  TOLERANCE = 0;
  if (num_empty > 6) {
    MAX_DEPTH = 4;
  }
  else {
    MAX_DEPTH = 6;
  }

  bool up_valid = (board != up_result);
  bool right_valid = (board != right_result);
  bool down_valid = (board != down_result);
  bool left_valid = (board != left_result);

//  cout<<"\nevaluating up move. board:\n";
//  print_board(up_result);
  up_val = up_valid ? eval_board_outcomes(up_result) : -1;

//  cout<<"\nevaluating down move. board:\n";
//  print_board(down_result);
  down_val = down_valid ? eval_board_outcomes(down_result) : -1;

//  cout<<"\nevaluating left move. board:\n";
//  print_board(left_result);
  left_val = left_valid ? eval_board_outcomes(left_result) : -1;

//  cout<<"\nevaluating right move. board:\n";
//  print_board(right_result);
  right_val = right_valid ? eval_board_outcomes(right_result) : -1;

  float max_val = max(up_val, down_val, left_val, right_val);
  cout<<"\tup_val: "<<up_val<<"\n\tdown_val: "<<down_val<<"\n\tleft_val:"<<left_val<<"\n\tright_val:"<<right_val<<endl;

  Direction ret = max_val == up_val ? UP : max_val == down_val ? DOWN : max_val == right_val ? RIGHT : LEFT;
  return ret;
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
//    block_to_fill = empty_blocks[empty_blocks.size() / 2];
//    block_to_fill.val = 2;
    block_to_fill = empty_blocks[rand() % empty_blocks.size()];
    block_to_fill.val = (rand() % 100 <= 10) ? 4 : 2;
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

  cout<<"New block coords(x y): ";
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


