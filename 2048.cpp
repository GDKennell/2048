#include "SmallBoard.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <math.h>

using namespace std;

struct Block;

typedef SmallBoard board_t;

enum Direction {UP, DOWN, LEFT, RIGHT};

const char* direction_names[] = {"up", "down", "left", "right"};

struct Block {
  int val;
  int x,y;
  bool empty;
  Block() : val(0), x(0), y(0), empty(true){ }
  void print(bool detail) const {
    if(!DETAIL && detail) return;
    ostream& output = detail ? detail_out : cout;
    if(empty) output<<"    ";
    else if(val < 10) output<<" "<<val<<"  ";
    else if(val < 100) output<<" "<<val<<" ";
    else if(val < 1000) output<<val<<' ';
    else output<<val;
  }
};

Block input_block();

void print_board(const board_t board, bool detail) {
  if(detail && !DETAIL)return;
  ostream& output = detail ? detail_out : cout;
  for(int y = 3; y >= 0; y--) {
    output<<"   ___________________"<<endl;
    output<<"  ";
    for(int x = 0; x <= 3; x++) {
      output<<'|';
      board.print(x, y, detail);
    }
    output<<'|'<<endl;
  }
  output<<"   ___________________"<<endl;
}

board_t up_move(const board_t in_board);
board_t down_move(const board_t in_board);
board_t left_move(const board_t in_board);
board_t right_move(const board_t in_board);

int min(int x1, int x2, int x3, int x4) {
  return min(x1, min(x2, min(x3, x4) ) );
}

int max(int x1, int x2, int x3, int x4) {
  return max(x1, max(x2, max(x3, x4) ) );
}

bool board_full(const board_t board);

Direction advice(const board_t board,
                 const board_t up_result,
                 const board_t down_result,
                 const board_t left_result,
                 const board_t right_result,
                 bool opt);

void add_new_tile(board_t& board);

int score = 0;
int up_combo_val, down_combo_val, left_combo_val, right_combo_val;

int main() {
  srand(time(NULL));
  detail_out.open("game_detail.txt");

  board_t board;

/*  Block init_block1 = input_block();
  board[init_block1.x - 1][init_block1.y - 1] = init_block1;
  cout<<endl;
  Block init_block2 = input_block();
  board[init_block2.x - 1][init_block2.y - 1] = init_block2;
  cout<<endl;*/
  int num_tiles = 2;
  cout<<"Input "<<num_tiles<<" tiles"<<endl;
  for(int i = 0; i < num_tiles; i++){
    Block new_block = input_block();
    assert(new_block.x-1 >= 0 && new_block.x-1 <= 3);
    assert(new_block.y-1 >= 0 && new_block.y-1 <= 3);
    board.set_val(new_block.x-1, new_block.y-1, new_block.val);
    cout<<endl;
  }

  cout<<"Initial board:"<<endl;
  print_board(board, false);

  int round_num = 0;
  while(1) {
    cout<<"###############Round "<<++round_num<<endl;
    if(DETAIL)
      detail_out<<"###############Round "<<round_num<<endl;
    print_board(board, false);
    print_board(board, true);
/*    cout<<"Ready for me to do the next move?"<<endl;
    string dontcare;
    cin >>dontcare;*/
    
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
      cerr<<"Score: "<<score<<endl;
      return 0;
    }

     Direction choice = advice(board,
                               up_result,
                               down_result, 
                               left_result,
                               right_result, true);
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

//    Block new_block = input_block();
//    board[new_block.x-1][new_block.y-1] = new_block;

//      cout<<"Added new tile, current board: "<<endl;
//      print_board(board);

    try {
      add_new_tile(board);
    } catch(...) {
      cout<<"Game OOOver!!!"<<endl;
      cerr<<"Score: "<<score<<endl;
      return 0;
    }
    cout<<endl;
    
  }

}

//returns some evaluation of this board based on number of tiles,
//    number of combos available, highest tile value
int heuristic(const board_t board) {
  int num_tiles = 0;

  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if (board.val_at( x, y) != 0) {
        num_tiles++;
      }
    }
  }
  int max_tiles = 4 * 4;
  return (max_tiles - num_tiles);
}

const int MAX_DEPTH = 6;

int depth = 0;

int eval_board_outcomes(const board_t board, int best_seen, bool opt);

int eval_board_moves(const board_t board, int worst_seen, bool opt) {
  depth++;
  board_t up_result = up_move(board);
  board_t down_result = down_move(board);
  board_t left_result = left_move(board);
  board_t right_result = right_move(board);

  bool up_valid = (board != up_result);
  bool down_valid = (board != down_result);
  bool left_valid = (board != left_result);
  bool right_valid = (board != right_result);

  int up_eval, down_eval, left_eval, right_eval;

  if (depth < MAX_DEPTH) {
    if(up_valid) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves calling eval_outcomes of up result"<<endl;
      }
    }
    up_eval = up_valid ? eval_board_outcomes(up_result, 0, opt) : -1;
    if (opt && up_eval >= worst_seen) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves branching and bounding, up_eval("<<up_eval<<") > worst_seen("<<worst_seen<<')'<<endl;
      }
      depth--;
      return INT_MAX;
    }

    if(down_valid) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves calling eval_outcomes of down result"<<endl;
      }
    }
    down_eval = down_valid ? eval_board_outcomes(down_result, up_eval, opt) : -1;
    if (opt && down_eval >= worst_seen) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves branching and bounding, down_eval("<<down_eval<<") > worst_seen("<<worst_seen<<')'<<endl;
      }
      depth--;
      return INT_MAX;
    }

    if(left_valid) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves calling eval_outcomes of left result"<<endl;
      }
    }
    left_eval = left_valid ? eval_board_outcomes(left_result, max(up_eval, down_eval), opt) : -1;
    if (opt && left_eval >= worst_seen) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves branching and bounding, left_eval("<<left_eval<<") > worst_seen("<<worst_seen<<')'<<endl;
      }
      depth--;
      return INT_MAX;
    }
    if(right_valid) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves calling eval_outcomes of right result"<<endl;
      }
    }
    right_eval = right_valid ? eval_board_outcomes(right_result, max(up_eval, max(down_eval, left_eval)), opt) : -1;
    if (opt && right_eval >= worst_seen) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves branching and bounding, right_eval("<<right_eval<<") > worst_seen("<<worst_seen<<')'<<endl;
      }
      depth--;
      return INT_MAX;
    }
    if(DETAIL){
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves finished with eval of all 4 moves, returning best : ";
        detail_out<<max(up_eval, down_eval, left_eval, right_eval)<<endl;
    }
  }
  else {
    if(DETAIL) {
      for(int i = 0; i < depth; i++) {detail_out<<"  ";}
      detail_out<<"eval_moves end case, getting best heuristic val of each valid move"<<endl;
    }
    up_eval = up_valid ? heuristic(up_result) : -1;
    down_eval = down_valid ? heuristic(down_result) : -1;
    left_eval = left_valid ? heuristic(left_result) : -1;
    right_eval = right_valid ? heuristic(right_result) : -1;
    if(DETAIL) {
      for(int i = 0; i < depth; i++) {detail_out<<"  ";}
      detail_out<<"eval_moves end case returning best : ";
      detail_out<<max(up_eval, down_eval, left_eval, right_eval)<<endl;
    }
  }
  depth--;
  return max(up_eval, down_eval, left_eval, right_eval);
}

int eval_board_outcomes(const board_t board, int best_seen, bool opt) {
  depth++;
  deque<board_t> possible_outcomes;

  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if (board.val_at( x, y) == 0) {
        board_t board_2 = board;
        board_2.set_val(x, y, 2);
        possible_outcomes.push_back(board_2);

        board_t board_4 = board;
        board_2.set_val(x, y, 4);
        possible_outcomes.push_back(board_4);
      }
    }
  }
 
   int worst_case = INT_MAX; 
   for(int i = 0; i < possible_outcomes.size(); i++) {
     if(DETAIL) {
       for(int i = 0; i < depth; i++) {detail_out<<"  ";}
       detail_out<<"eval_outcomes calling eval_moves for outcome #"<<i<<" of "<<possible_outcomes.size()<<endl;
     }
     int outcome_val = eval_board_moves(possible_outcomes[i], worst_case, opt);
     if (outcome_val < worst_case) {
       if(DETAIL) {
         for(int i = 0; i < depth; i++) {detail_out<<"  ";}
         detail_out<<"eval_outcomes found worst yet, outcome of "<<outcome_val<<endl;
       }
       worst_case = outcome_val;
       if (opt && worst_case <= best_seen) {
         if(DETAIL) {
           for(int i = 0; i < depth; i++) {detail_out<<"  ";}
           detail_out<<"eval_outcomes found worst case "<<worst_case<<" worse than best seen "<<best_seen<<", branching and bounding"<<endl;
         }
         depth--;
         return 0;
       }
     }
   }

   if(DETAIL) {
     for(int i = 0; i < depth; i++) {detail_out<<"  ";}
     detail_out<<"eval_outcomes finished, returning worst case, "<<worst_case<<endl;
   }
   depth--;
   return worst_case;
}

Direction advice(const board_t board,
                 const board_t up_result,
                 const board_t down_result,
                 const board_t left_result,
                 const board_t right_result, bool opt) {
  if(DETAIL)
    detail_out<<"Getting advice for this board:"<<endl;
  print_board(board, true);
  int up_val;
  int down_val;
  int left_val;
  int right_val;

  bool up_valid = (board != up_result);
  bool down_valid = (board != down_result);
  bool left_valid = (board != left_result);
  bool right_valid = (board != right_result);

  if(up_valid){
    if(DETAIL)
      detail_out<<"advice evaluating up move"<<endl;
  }
  up_val = up_valid ? eval_board_outcomes(up_result, 0, opt) : -1;
  if(up_valid && DETAIL)
    detail_out<<"advice got up_val of "<<up_val<<endl;
  else if(DETAIL) {
    detail_out<<"advice: up is not valid"<<endl;
  }

  if(down_valid) {
    if(DETAIL)
      detail_out<<"advice evaluating down move"<<endl;
  }
  down_val = down_valid ? eval_board_outcomes(down_result, up_val, opt) : -1;
  if(down_valid && DETAIL)
    detail_out<<"advice got down_val of "<<down_val<<endl;

  if(left_valid) {
    if(DETAIL)
      detail_out<<"advice evaluating left mvoe"<<endl;
  }
  left_val = left_valid ? eval_board_outcomes(left_result, max(up_val, down_val), opt) : -1;
  if(left_valid && DETAIL)
    detail_out<<"advice got left_val of "<<left_val<<endl;

  if(right_valid) {
    if(DETAIL)
      detail_out<<"advice evaluting right move"<<endl;
  }
  right_val = right_valid ? eval_board_outcomes(right_result, max(up_val, max(down_val, left_val)), opt) : -1;
  if(right_valid && DETAIL)
    detail_out<<"advice got right_val of "<<right_val<<endl;

  int max_val = max(up_val, down_val, left_val, right_val);
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

bool board_full(const board_t board) {
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if(board.val_at(x,y) != 0) return false;
    }
  }
  return true;
}

void add_new_tile(board_t& board) {
  deque<Block> empty_blocks;
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
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
  block_to_fill.val = (rand() % 100 <= 15) ? 4 : 2;
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

board_t up_move(const board_t in_board) {
  board_t result = in_board;
  for (int x = 0; x < 4; x++) {
    // Shift everthing up
    for (int y = 2; y >= 0; y--) {
      if (result.val_at(x,y) == 0) continue;
      int highest_empty_y = y;
      for(int i = y + 1; i <= 3 && i >= 0; i++) {
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

board_t down_move(const board_t in_board) {
  board_t result = in_board;
  for (int x = 0; x < 4; x++) {
    // Shift everthing down 
    for (int y = 1; y <= 3; y++) {
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
    for(int y = 0; y < 3; y++) {
      if (result.val_at(x,y) == 0 ||
          result.val_at(x,y+1) == 0) continue;
      if(result.exp_at(x,y) == result.exp_at(x,y+1)) {
        if(depth == 0) {
          down_combo_val += 2 * result.val_at(x,y);
        }
        result.set_exp(x,y,result.exp_at(x,y)+1);
        // Shift all blocks from y+2 up to 3 down
        for(int i = y+2; i <= 3; i++) {
          result.set_exp(x,i-1,result.exp_at(x,i));
        }
        result.set_exp(x,3,0);
      }
    }
  }
  return result;
}

board_t left_move(const board_t in_board) {
  board_t result = in_board;
  for (int y = 0; y < 4; y++) {
    // Shift everthing left 
    for (int x = 1; x <= 3; x++) {
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
    for(int x = 0; x < 3; x++) {
      if (result.val_at(x,y) == 0 ||
          result.val_at(x+1,y) == 0) continue;
      if(result.exp_at(x,y) == result.exp_at(x+1,y)) {
        if(depth == 0) {
          left_combo_val += 2 * result.val_at(x,y);
        }
        result.set_exp(x,y,result.exp_at(x,y)+1);
        // Shift all blocks from x+2 up to 3 left
        for(int i = x+2; i <= 3; i++) {
          result.set_exp(i-1,y,result.exp_at(i,y));
        }
        result.set_exp(3,y,0);
      }
    }
  }
  return result;
}

board_t right_move(const board_t in_board) {
  board_t result = in_board;
  for (int y = 0; y < 4; y++) {
    // Shift everthing right 
    for (int x = 2; x >= 0; x--) {
      if (result.val_at(x,y) == 0) continue;
      int rightest_empty_x = x;
      for(int i = x + 1; i <= 3; i++) {
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

