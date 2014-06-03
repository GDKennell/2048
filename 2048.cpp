#include "SmallBoard.h"
#include <pthread.h>
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
//#include <dispatch/dispatch.h>

using namespace std;

struct Block;

typedef SmallBoard board_t;
typedef chrono::system_clock Clock;

enum Direction {UP, DOWN, LEFT, RIGHT};

const char* direction_names[] = {"up", "down", "left", "right"};

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

inline board_t up_move(const board_t& in_board);
inline board_t down_move(const board_t& in_board);
inline board_t left_move(const board_t& in_board);
inline board_t right_move(const board_t& in_board);

int min(int x1, int x2, int x3, int x4) {
  return min(x1, min(x2, min(x3, x4) ) );
}

int max(int x1, int x2, int x3, int x4) {
  return max(x1, max(x2, max(x3, x4) ) );
}

double max(double x1, double x2, double x3, double x4) {
  return max(x1, max(x2, max(x3, x4) ) );
}

bool board_full(const board_t& board);

Direction advice(const board_t& board,
                 const board_t& up_result,
                 const board_t& down_result,
                 const board_t& left_result,
                 const board_t& right_result);

void add_new_tile(board_t& board);

int score = 0;
int up_combo_val, down_combo_val, left_combo_val, right_combo_val;

int main() {
  time_t start_time, end_time;
  start_time=Clock::to_time_t(Clock::now());
  srand(time(NULL));

  board_t board;

/*  Block init_block1 = input_block();
  board[init_block1.x - 1][init_block1.y - 1] = init_block1;
  cout<<endl;
  Block init_block2 = input_block();
  board[init_block2.x - 1][init_block2.y - 1] = init_block2;
  cout<<endl;*/
  int num_tiles = 2;
  cout<<"Input "<<num_tiles<<" tiles"<<endl;
  for(int i = 0; i < num_tiles; ++i){
    Block new_block = input_block();
    assert(new_block.x-1 >= 0 && new_block.x-1 <= 3);
    assert(new_block.y-1 >= 0 && new_block.y-1 <= 3);
    board.set_val(new_block.x-1, new_block.y-1, new_block.val);
    cout<<endl;
  }

  int round_num = 0;
  while(1) {
    cout<<"###############Round "<<++round_num<<endl;
    print_board(board);
    
    // Up move
    up_combo_val = 0;
    board_t up_result = up_move(board);
    // Down move
    down_combo_val = 0;
    board_t down_result = down_move(board);
    // Left move
    left_combo_val = 0;
    board_t left_result = left_move(board);
    // Right move
    right_combo_val = 0;
    board_t right_result = right_move(board);

    if(board_full(up_result) &&
        board_full(down_result) &&
        board_full(left_result) &&
        board_full(right_result)) {
      cout<<"Game Over"<<endl;
//      advice(board,board,board,board,board,false);
      end_time=Clock::to_time_t(Clock::now());
      cerr<<"Score: "<<score<<endl;
      cerr<<"Time: "<<end_time - start_time<<endl;
      if(end_time - start_time != 0) {
        cerr<<"Points/sec: "<<score / (end_time - start_time)<<endl;
      }
      return 0;
    }

     Direction choice = advice(board,
                               up_result,
                               down_result, 
                               left_result,
                               right_result);
    cout<<"\n********Move "<<direction_names[choice];
    cout<<"********"<<endl;

    switch(choice) {
      case UP:
        board = up_result;
        score += up_combo_val;
        break;
      case DOWN:
        board = down_result;
        score += down_combo_val;
        break;
      case LEFT:
        board = left_result;
        score += left_combo_val;
        break;
      case RIGHT:
        board = right_result;
        score += right_combo_val;
        break;
    }
    cout<<"********Score: "<<score<<"********\n"<<endl;
    cout<<"Input new tile"<<endl;

    try {
      add_new_tile(board);
    } catch(...) {
      end_time=Clock::to_time_t(Clock::now());
      cout<<"Game OOOver!!!"<<endl;
//      advice(board,board,board,board,board,false);
      cerr<<"Score: "<<score<<endl;
      cerr<<"Time: "<<end_time - start_time<<endl;
      if(end_time - start_time != 0) {
        cerr<<"Points/sec: "<<score / (end_time - start_time)<<endl;
      }
      return 0;
    }
    cout<<endl;
    
  }

}

//returns some evaluation of this board based on number of tiles,
//    number of combos available, highest tile value
int heuristic(const board_t& board) {
  int num_tiles = 0;

  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 4; ++y) {
      if (board.val_at( x, y) != 0) {
        ++num_tiles;
      }
    }
  }
  int max_tiles = 4 * 4;
  return (max_tiles - num_tiles);
}

const int MAX_DEPTH = 6;

int depth = 0;

double eval_board_outcomes(const board_t& board);

double eval_board_moves(const board_t& board) {
  ++depth;
  board_t up_result = up_move(board);
  board_t down_result = down_move(board);
  board_t left_result = left_move(board);
  board_t right_result = right_move(board);

  bool up_valid = (board != up_result);
  bool down_valid = (board != down_result);
  bool left_valid = (board != left_result);
  bool right_valid = (board != right_result);

  double up_eval, down_eval, left_eval, right_eval;

  if (depth < MAX_DEPTH) {
    up_eval = up_valid ? eval_board_outcomes(up_result) : -1.0;
    down_eval = down_valid ? eval_board_outcomes(down_result) : -1.0;
    left_eval = left_valid ? eval_board_outcomes(left_result) : -1.0;
    right_eval = right_valid ? eval_board_outcomes(right_result) : -1.0;
  }
  else {
    up_eval = up_valid ? heuristic(up_result) : -1.0;
    down_eval = down_valid ? heuristic(down_result) : -1.0;
    left_eval = left_valid ? heuristic(left_result) : -1.0;
    right_eval = right_valid ? heuristic(right_result) : -1.0;
  }
  --depth;
 
  return max(up_eval, down_eval, left_eval, right_eval);
}

double eval_board_outcomes(const board_t& board) {
  ++depth;
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
 
  double prob2 = (9.0/10.0) * (1.0 / (double)num_outcomes); 
  double prob4 = (1.0/10.0) * (1.0 / (double)num_outcomes); 
  double tot_prob = 0.0;
  for(int i = 0; i < num_outcomes; ++i) {
    double outcome_val = eval_board_moves(possible_outcomes[i]);
    if(i % 2 == 0)
      tot_prob += (prob2 * outcome_val);
    else
      tot_prob += (prob4 * outcome_val);
  }
  --depth;
  return tot_prob;
}

struct thread_arg_t {
  thread_arg_t(const board_t& board_, double* ret_val_) : board(board_), ret_val(ret_val_) { }
  const board_t& board;
  double* ret_val;
};

void* eval_board_outcomes_p(void* arg_ptr) {
  thread_arg_t in_arg = *((thread_arg_t*)arg_ptr);
  *(in_arg.ret_val) = eval_board_outcomes(in_arg.board);

  pthread_exit(NULL);
  return NULL;
}

Direction advice(const board_t& board,
                 const board_t& up_result,
                 const board_t& down_result,
                 const board_t& left_result,
                 const board_t& right_result) {

  int num_empty = 0;
  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 4; ++y) {
      if(board.exp_at(x,y) == 0)
        ++num_empty;
    }
  }

  double up_val;
  double down_val;
  double left_val;
  double right_val;

  bool up_valid = (board != up_result);
  bool down_valid = (board != down_result);
  bool left_valid = (board != left_result);
  bool right_valid = (board != right_result);

  pthread_t up_thread, down_thread, left_thread, right_thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  if (up_valid) {
    thread_arg_t up_in(up_result, &up_val);
    int thread_err = pthread_create(&up_thread, NULL, eval_board_outcomes_p, (void*)&up_in);
    assert(!thread_err);
  }

  if(down_valid) {
    thread_arg_t down_in(down_result, &down_val);
    int thread_err = pthread_create(&down_thread, NULL, eval_board_outcomes_p, (void*)&down_in);
    assert(!thread_err);
  }

  if(left_valid) {
    thread_arg_t left_in(left_result, &left_val);
    int thread_err = pthread_create(&left_thread, NULL, eval_board_outcomes_p, (void*)&left_in);
    assert(!thread_err);
  }

  if(right_valid) {
    thread_arg_t right_in(right_result, &right_val);
    int thread_err = pthread_create(&right_thread, NULL, eval_board_outcomes_p, (void*)&right_in);
    assert(!thread_err);
  }
 

  if (up_valid) {
    int thread_err = pthread_join(up_thread, NULL);
    assert(!thread_err);
  }
  if(down_valid) {
    int thread_err = pthread_join(down_thread, NULL);
    assert(!thread_err);
  }
  if(left_valid) {
    int thread_err = pthread_join(left_thread, NULL);
    assert(!thread_err);
  }
  if(right_valid) {
    int thread_err = pthread_join(right_thread, NULL);
    assert(!thread_err);
  }

  if(!up_valid) up_val = -1.0;
  if(!down_valid) down_val = -1.0;
  if(!left_valid) left_val = -1.0;
  if(!right_valid) right_val = -1.0;

  double max_val = max(up_val, down_val, left_val, right_val);
  cout<<"\tup_val: "<<up_val<<"\n\tdown_val: "<<down_val<<"\n\tleft_val:"<<left_val<<"\n\tright_val:"<<right_val<<endl;

  if(max_val == up_val && up_valid) {
    return UP;
  }
  else if (max_val == down_val || (max_val == up_val && !up_valid)) {
    return DOWN;
  }
  else if(max_val == left_val && left_valid) {
    return LEFT;
  }
  else {
    return RIGHT;
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

void add_new_tile(board_t& board) {
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
  if(empty_blocks.empty()) {
    throw "Board full";
  }
  Block block_to_fill = empty_blocks[rand() % empty_blocks.size()];
  block_to_fill.val = (rand() % 100 <= 10) ? 4 : 2;
  block_to_fill.empty = false;
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

board_t up_move(const board_t& in_board) {
  board_t result = in_board;
  for (int x = 0; x < 4; ++x) {
    // Shift everthing up
    for (int y = 2; y >= 0; y--) {
      if (result.val_at(x,y) == 0) continue;
      int highest_empty_y = y;
      for(int i = y + 1; i <= 3 && i >= 0; ++i) {
        if (result.val_at(x,i) == 0) 
          highest_empty_y = i;
      }
      if(highest_empty_y != y) {
        result.set_exp(x,
                       highest_empty_y, 
                       result.exp_at(x,y));
        result.set_exp(x,y,0);
      }
    }
    
    // Look for combinations
    for(int y = 3; y > 0; y--) {
      if (result.val_at(x,y) == 0 ||
          result.val_at(x,y-1) == 0) continue;
      if(result.exp_at(x,y) == result.exp_at(x,y-1)) {
        result.set_exp(x,y,result.exp_at(x,y)+1);
        if(depth == 0) {
          up_combo_val += result.val_at(x,y);
        }
        // Shift all blocks from y-2 down to 0 up
        for(int i = y-2; i >= 0; i--) {
          result.set_exp(x,i+1,result.exp_at(x,i));
        }
        result.set_exp(x,0,0);
      }
    }
  }
  return result;
}

board_t down_move(const board_t& in_board) {
  board_t result = in_board;
  for (int x = 0; x < 4; ++x) {
    // Shift everthing down 
    for (int y = 1; y <= 3; ++y) {
      if (result.val_at(x,y) == 0) continue;
      int lowest_empty_y = y;
      for(int i = y - 1; i >= 0; i--) {
        if (result.val_at(x,i) == 0) 
          lowest_empty_y = i;
      }
      if(lowest_empty_y != y) {
        result.set_exp(x,
                       lowest_empty_y, 
                       result.exp_at(x,y));
        result.set_exp(x,y,0);
      }
    }
    
    // Look for combinations
    for(int y = 0; y < 3; ++y) {
      if (result.val_at(x,y) == 0 ||
          result.val_at(x,y+1) == 0) continue;
      if(result.exp_at(x,y) == result.exp_at(x,y+1)) {
        if(depth == 0) {
          down_combo_val += 2 * result.val_at(x,y);
        }
        result.set_exp(x,y,result.exp_at(x,y)+1);
        // Shift all blocks from y+2 up to 3 down
        for(int i = y+2; i <= 3; ++i) {
          result.set_exp(x,i-1,result.exp_at(x,i));
        }
        result.set_exp(x,3,0);
      }
    }
  }
  return result;
}

board_t left_move(const board_t& in_board) {
  board_t result = in_board;
  for (int y = 0; y < 4; ++y) {
    // Shift everthing left 
    for (int x = 1; x <= 3; ++x) {
      if (result.val_at(x,y) == 0) continue;
      int leftest_empty_x = x;
      for(int i = x - 1; i >= 0; i--) {
        if (result.val_at(i,y) == 0) 
          leftest_empty_x = i;
      }
      if(leftest_empty_x != x) {
        result.set_exp(leftest_empty_x,
                       y, 
                       result.exp_at(x,y));
        result.set_exp(x,y,0);
      }
    }
    
    // Look for combinations
    for(int x = 0; x < 3; ++x) {
      if (result.val_at(x,y) == 0 ||
          result.val_at(x+1,y) == 0) continue;
      if(result.exp_at(x,y) == result.exp_at(x+1,y)) {
        if(depth == 0) {
          left_combo_val += 2 * result.val_at(x,y);
        }
        result.set_exp(x,y,result.exp_at(x,y)+1);
        // Shift all blocks from x+2 up to 3 left
        for(int i = x+2; i <= 3; ++i) {
          result.set_exp(i-1,y,result.exp_at(i,y));
        }
        result.set_exp(3,y,0);
      }
    }
  }
  return result;
}

board_t right_move(const board_t& in_board) {
  board_t result = in_board;
  for (int y = 0; y < 4; ++y) {
    // Shift everthing right 
    for (int x = 2; x >= 0; x--) {
      if (result.val_at(x,y) == 0) continue;
      int rightest_empty_x = x;
      for(int i = x + 1; i <= 3; ++i) {
        if (result.val_at(i,y) == 0) 
          rightest_empty_x = i;
      }
      if(rightest_empty_x != x) {
        result.set_exp(rightest_empty_x,
                       y, 
                       result.exp_at(x,y));
        result.set_exp(x,y,0);
      }
    }
    
    // Look for combinations
    for(int x = 3; x > 0; x--) {
      if (result.val_at(x,y) == 0 ||
          result.val_at(x-1,y) == 0) continue;
      if(result.exp_at(x,y) == result.exp_at(x-1,y)) {
        if(depth == 0) {
          right_combo_val += 2 * result.val_at(x,y);
        }
        result.set_exp(x,y,result.exp_at(x,y)+1);
        // Shift all blocks from x-2 down to 0 right
        for(int i = x-2; i >= 0; i--) {
          result.set_exp(i+1,y,result.exp_at(i,y));
        }
        result.set_exp(0,y,0);
      }
    }
  }
  return result;
}

