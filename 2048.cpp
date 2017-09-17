#include "SmallBoard.h"
#include "precompute.h"
extern "C" {
  #include "compute_moves.h"
  #include "compute_heuristic.h"
}
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

const int NUM_TRANSFORMS = 65536;

enum Direction {LEFT, RIGHT, UP, DOWN, NONE};

const char* direction_names[] = {"left", "right", "up", "down",  "none"};

int TOLERANCE = 10;

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

Move_Result move_in_direction(const board_t& in_board, Direction direction);
Move_Result up_move(const board_t& in_board);
Move_Result down_move(const board_t& in_board);
Move_Result left_move(const board_t& in_board);
Move_Result right_move(const board_t& in_board);
int64_t heuristic(const board_t& board);

int64_t min(int64_t x1, int64_t x2, int64_t x3, int64_t x4) {
  return min(x1, min(x2, min(x3, x4) ) );
}
const uint64_t UNUSED_BOARD = 0;
const uint64_t UNUSED_HEUR = 0xFFFFFFFFFFFFFFF;

int64_t max(int64_t x1, int64_t x2, int64_t x3, int64_t x4) {
  int64_t highest = UNUSED_HEUR;
  int64_t allVals[] = {x1, x2, x3,x4};
  for (int i = 0; i < 4; ++i)
  {
    if (allVals[i] != UNUSED_HEUR)
    {
      if (highest == UNUSED_HEUR || allVals[i] > highest)
      {
        highest = allVals[i];
      }
    }
  }
  return highest;
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

transform_t *left_move_transforms = NULL;
transform_t *right_move_transforms = NULL;

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

int MAX_DEPTH = 10;

uint64_t size_of_tree(int tree_depth)
{
  uint64_t multiplier = 1;
  for (int i = tree_depth + 1; i > 1; --i)
  {
    int tempMultiplier = (i % 2 == 0) ? 30 : 4;
    multiplier *= tempMultiplier;
    multiplier += 1;
  }
  return 4 * multiplier + 1;
}

uint64_t *entire_move_tree;
float *evaluation_tree;
uint64_t tree_size;

uint64_t start_of_layer(int layer_num)
{
  const int max_layers = 20;
  static uint64_t layer_starts[max_layers];

  if (layer_starts[layer_num] > 0)
  {
    return layer_starts[layer_num];
  }
  uint64_t layerStart = 0;
  uint64_t layerSize = 4;
  for (int i = 1; i <= layer_num; ++i)
  {
    layerStart += layerSize;
    uint64_t layerMultiplier = (i % 2 == 0) ? 4 : 30;
    layerSize *= layerMultiplier;
  }
  layer_starts[layer_num] = layerStart;
  return layerStart;
}


uint64_t size_of_layer(int layer_num)
{
  uint64_t layerSize = 4;
  for (int i = 1; i <= layer_num; ++i)
  {
    uint64_t layerMultiplier = (i % 2 == 0) ? 4 : 30;
    layerSize *= layerMultiplier;
  }
  return layerSize;
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


void compute_outcomes(uint64_t orig_index)
{
  int orig_layer = layer_for_index(orig_index);
  uint64_t orig_layer_index = orig_index - start_of_layer(orig_layer);
  uint64_t next_outcome_i = start_of_layer(orig_layer + 1) + 30 * orig_layer_index;
  uint64_t end_of_outcomes = next_outcome_i + 30;
  board_t orig_board = entire_move_tree[orig_index];
  if (orig_board.raw() == UNUSED_BOARD){ return; }

  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 4; ++y) {
      if (orig_board.val_at( x, y) == 0) {
        board_t outcome2 = orig_board;
        outcome2.set_val(x, y, 2);
        entire_move_tree[next_outcome_i++] = outcome2.raw();
//        cout<<"\tentire_move_tree["<<next_outcome_i - 1<<"] = "<<outcome2.raw()<<endl;

        board_t outcome4 = orig_board;
        outcome4.set_val(x, y, 4);
        entire_move_tree[next_outcome_i++] = outcome4.raw();
//        cout<<"\tentire_move_tree["<<next_outcome_i - 1<<"] = "<<outcome4.raw()<<endl;
      }
    }
  }
  while (next_outcome_i < end_of_outcomes)
  {
    entire_move_tree[next_outcome_i++] = UNUSED_BOARD;
  }
}
// Layer 0: first four Moves
// Layer 1: outcomes of first four moves
// Layer 2: move options from layer 1 outcomes
// Layer 3: outcomes from layer 2 moves
// etc.
void compute_layer(int layerNum)
{
  // For even layers, computing <=4 move options per entry at layer - 1
  // For odd layers, computing <= 30 outcomes per move at layer - 1
  bool calculate_moves = layerNum % 2 == 0;
  //iterate through layerNum - 1
  uint64_t i_start_of_prev_layer = start_of_layer(layerNum - 1);
  uint64_t i_size_of_prev_layer = size_of_layer(layerNum - 1);

//  cout<<"computing "<<(calculate_moves ? "moves" : "outcomes" )<<" layer "<<layerNum<< endl;

  if (calculate_moves)
  {
    compute_moves(entire_move_tree, layerNum, left_move_transforms, right_move_transforms);
  }
  else
  {
    for (uint64_t prev_i = 0; prev_i < i_size_of_prev_layer; ++prev_i)
    {
      compute_outcomes(i_start_of_prev_layer + prev_i);
    }
  }
}

void compute_tree(const board_t &startBoard)
{
  for (int i = 0; i < 4; ++i)
  {
    Direction move_dir = (Direction)i;
    Move_Result move_res = move_in_direction(startBoard, move_dir);
    uint64_t move_index = start_of_layer(0) + i;
    if (move_res.board == startBoard)
    {
      entire_move_tree[move_index] =  UNUSED_BOARD;
    }
    else
    {
      entire_move_tree[move_index] = move_res.board.raw();
    }
  }

  for (int depth = 1; depth <= MAX_DEPTH; ++ depth)
  {
//    cout<<"computing layer "<<depth<<": "<<start_of_layer(depth)<<"-"<<start_of_layer(depth) + size_of_layer(depth)<<endl;
    compute_layer(depth);
  }
}

void evaluate_moves_layer(int layerNum)
{
  uint64_t layerStart = start_of_layer(layerNum);
  uint64_t layerSize = size_of_layer(layerNum);

  for (uint64_t i = layerStart; i < layerStart + layerSize; ++i)
  {
    board_t thisBoard = entire_move_tree[i];
    if (thisBoard.raw() == UNUSED_BOARD)
    {
      entire_move_tree[i] = UNUSED_HEUR;
      continue;
    }

    uint64_t thisLayerIndex = i - layerStart;
    uint64_t outcomesStart = start_of_layer(layerNum + 1) + (30 * thisLayerIndex);
    uint64_t outcomeHeurTotal = 0;
    int outcomeCount = 0;

    int64_t tot_prob = 0;

    // 2's are weighted as 1.8 while 4's are weighted as 0.2
    int64_t prob2_num = 9;
    int64_t prob4_num = 1;
    for (uint64_t j = outcomesStart; j < outcomesStart + 30; ++j)
    {
      uint64_t outcomeHeur = entire_move_tree[j];
      if (outcomeHeur == UNUSED_HEUR)
      {
        break;
      }
      tot_prob += outcomeHeur * ((j - outcomesStart) % 2 == 0 ? prob2_num : prob4_num);
      //        cout<<"\t\t\ttot_prob += outcomeHeur (entire_move_tree["<<j<<"] == "<<outcomeHeur<<") * "<<((j - outcomesStart) % 2 == 0 ? prob2_num : prob4_num)<<" == "<<outcomeHeur * ((j - outcomesStart) % 2 == 0 ? prob2_num : prob4_num)<<endl;
      ++outcomeCount;
    }
    entire_move_tree[i] = outcomeCount == 0 ? 0 : tot_prob / (10 * outcomeCount);
    //      cout<<"\t\teval["<<i<<"] = outcomeHeurTotal("<<tot_prob<<" / (10 * outcomeCount("<<outcomeCount<<")) = "<<entire_move_tree[i]<<endl;

  }
}

void evaluate_outcomes_layer(int layerNum)
{
  uint64_t layerStart = start_of_layer(layerNum);
  uint64_t layerSize = size_of_layer(layerNum);

  for (uint64_t i = layerStart; i < layerStart + layerSize; ++i)
  {
    board_t thisBoard = entire_move_tree[i];
    if (thisBoard.raw() == UNUSED_BOARD)
    {
      entire_move_tree[i] = UNUSED_HEUR;
      continue;
    }

    uint64_t thisLayerIndex = i - layerStart;
    uint64_t movesStart = start_of_layer(layerNum + 1) + (4 * thisLayerIndex);

    uint64_t bestMoveHeur = UNUSED_HEUR;
    for (uint64_t j = movesStart; j < movesStart + 4; ++j)
    {
      uint64_t moveHeur = entire_move_tree[j];
      if (moveHeur != UNUSED_HEUR)
      {
        if (bestMoveHeur == UNUSED_HEUR || moveHeur > bestMoveHeur)
        {
          bestMoveHeur = moveHeur;
        }
      }
    }
    entire_move_tree[i] = bestMoveHeur;
    //      cout<<"\t\teval["<<i<<"] = bestMoveHeur("<<bestMoveHeur<<")"<<endl;
  }

}

void evaluate_layer(int layerNum)
{
//  cout<<"evaluating layer "<<layerNum<<endl;
  uint64_t layerStart = start_of_layer(layerNum);
  uint64_t layerSize = size_of_layer(layerNum);
  if (layerNum == MAX_DEPTH)
  {
    compute_heuristics(entire_move_tree, layerNum, empty_vals);
  }
  // Move layer - average the next layer (outcomes)
  else if (layerNum % 2 == 0)
  {
    evaluate_moves_layer(layerNum);
  }
  // Outcome layer - take max of next layer (moves)
  else
  {
    evaluate_outcomes_layer(layerNum);
  }
}
int64_t num_empty_in_board(const board_t& board);

void setDepthAndTolerance(const board_t &board)
{
  int64_t num_empty = num_empty_in_board(board);
  if(num_empty < 2) {
    TOLERANCE = 50000;
    MAX_DEPTH = 6;
  }
  else if(num_empty < 4) {
    TOLERANCE = 100000;
    MAX_DEPTH = 6;
  }
  else if(num_empty < 7) {
    TOLERANCE = 200000;
    MAX_DEPTH = 6;
  }
  else {
    TOLERANCE = 500000;
    MAX_DEPTH = 4;
  }
  cout<<"Evaluating to depth "<<MAX_DEPTH<<endl;
}
Direction decide_move_from_tree()
{
  for (int layerNum = MAX_DEPTH; layerNum >= 0; --layerNum)
  {
    evaluate_layer(layerNum);
  }
  bool up_valid = (entire_move_tree[UP] != UNUSED_HEUR);
  bool right_valid = (entire_move_tree[RIGHT] != UNUSED_HEUR);
  bool down_valid = (entire_move_tree[DOWN] != UNUSED_HEUR);
  bool left_valid = (entire_move_tree[LEFT] != UNUSED_HEUR);

  int64_t up_val = entire_move_tree[UP];
  int64_t down_val = entire_move_tree[DOWN];
  int64_t left_val = entire_move_tree[LEFT];
  int64_t right_val = entire_move_tree[RIGHT];

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
int main() {
//    print_hello(); // Example call into clink (example of linking in C file to C++)
  load_precompute_files();

//  time_t start_time, end_time;
//  start_time=Clock::to_time_t(Clock::now());
  srand(time(NULL));

  board_t board = input_board();

  // Max size of the tree stored in the array
  tree_size = size_of_tree(MAX_DEPTH);
  cout<<"Max depth "<<MAX_DEPTH<<", tree size: "<<tree_size<<endl;
  entire_move_tree = (uint64_t *)malloc(tree_size * sizeof(uint64_t));

  int round_num = 0;
  while(1) {
    cout<<"###############Round "<<++round_num<<endl;
    print_board(board);

    setDepthAndTolerance(board);
    compute_tree(board);

    Direction move_decision = decide_move_from_tree();

    board = apply_move(move_decision, board, score);

    if(move_decision == NONE) {
      cout<<"Game Over"<<endl;
      free(entire_move_tree);
      return 0;
    }

    cout<<"\n********Move "<<direction_names[move_decision]<<"********"<<endl;
    add_new_tile(board, false);
    cout<<endl;
  }
}

void load_precompute_files() {
  left_move_transforms = (transform_t*)malloc(NUM_TRANSFORMS * sizeof(transform_t));
  right_move_transforms = (transform_t*)malloc(NUM_TRANSFORMS * sizeof(transform_t));

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

  board.set_val(0, 0, 2);
  board.set_val(1, 1, 2);
//  board.set_val(0, 2, 4);
//  board.set_val(0, 3, 4);
//  board.set_val(1, 0, 2);
//  board.set_val(1, 1, 2);
//  board.set_val(1, 2, 2);
//  board.set_val(1, 3, 2);

//    const int num_tiles = 5;
//  cout<<"Input "<<num_tiles<<" tiles"<<endl;
//  for(int i = 0; i < num_tiles; ++i){
//    Block new_block = input_block();
//    assert(new_block.x-1 >= 0 && new_block.x-1 <= 3);
//    assert(new_block.y-1 >= 0 && new_block.y-1 <= 3);
//    board.set_val(new_block.x-1, new_block.y-1, new_block.val);
//    cout<<endl;
//  }
  return board;
}


int64_t num_empty_in_board(const board_t& board)
{
  int64_t num_empty = 0;
  for(int64_t c = 0; c < 4; ++c) {
    int64_t column = board.raw_col(c);
    num_empty += empty_vals[column];
  }
  return num_empty;
}

//returns some evaluation of this board based on number of tiles,
//    number of combos available, highest tile value
int64_t heuristic(const board_t& board) {
  return 10000000 * num_empty_in_board(board);
}

const int INVALID_MOVE_WEIGHT = 0.0;

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
    up_eval = up_valid ? heuristic(up_result.board) : INVALID_MOVE_WEIGHT;
    down_eval = down_valid ? heuristic(down_result.board) : INVALID_MOVE_WEIGHT;
    left_eval = left_valid ? heuristic(left_result.board) : INVALID_MOVE_WEIGHT;
    right_eval = right_valid ? heuristic(right_result.board) : INVALID_MOVE_WEIGHT;
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
  int64_t up_val;
  int64_t down_val;
  int64_t left_val;
  int64_t right_val;

  uint64_t num_empty = num_empty_in_board(board);
  if(num_empty < 2) {
    TOLERANCE = 50000;
    MAX_DEPTH = 10;
  }
  else if(num_empty < 4) {
    TOLERANCE = 100000;
    MAX_DEPTH = 8;
  }
  else if(num_empty < 7) {
    TOLERANCE = 200000;
    MAX_DEPTH = 6;
  }
  else {
    TOLERANCE = 500000;
    MAX_DEPTH = 4; 
  }

  bool up_valid = (board != up_result);
  bool right_valid = (board != right_result);
  bool down_valid = (board != down_result);
  bool left_valid = (board != left_result);

  up_val = up_valid ? eval_board_outcomes(up_result) : -1;
  down_val = down_valid ? eval_board_outcomes(down_result) : -1;
  left_val = left_valid ? eval_board_outcomes(left_result) : -1;
  right_val = right_valid ? eval_board_outcomes(right_result) : -1;

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
//    block_to_fill = empty_blocks[0];
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

  cout<<"New block coords(x,y): ";
  cin >> return_block.x >> return_block.y;
  assert(return_block.x > 0 && return_block.x <= 4);
  assert(return_block.y > 0 && return_block.y <= 4);
  return return_block;
}

Move_Result move_in_direction(const board_t& in_board, Direction direction)
{
  switch (direction) {
    case UP:
      return up_move(in_board);
    case DOWN:
      return down_move(in_board);
    case LEFT:
      return left_move(in_board);
    case RIGHT:
      return right_move(in_board);
    default:
      break;
  }
  return Move_Result();
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


