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

typedef uint64_t board_t;

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

struct Move_Result {
  board_t board; // Resulting board
  int num_combos; // Number of combinations made by move
  int combos_value; // Sum of final values from combinations (0 if none)
  Move_Result() : num_combos(0), combos_value(0){ }
};

Block input_block();

void print_board(const board_t& board, bool detail) {
  if(detail && !DETAIL)return;
  ostream& output = detail ? detail_out : cout;
  for(int y = 3; y >= 0; y--) {
    output<<"   ___________________"<<endl;
    output<<"  ";
    for(int x = 0; x <= 3; x++) {
      output<<'|';
      SmallBoard::print(board, x, y, detail);
    }
    output<<'|'<<endl;
  }
  output<<"   ___________________"<<endl;
}

Move_Result up_move(const board_t& in_board);
Move_Result down_move(const board_t& in_board);
Move_Result left_move(const board_t& in_board);
Move_Result right_move(const board_t& in_board);

int min(int x1, int x2, int x3, int x4) {
  return min(x1, min(x2, min(x3, x4) ) );
}

int max(int x1, int x2, int x3, int x4) {
  return max(x1, max(x2, max(x3, x4) ) );
}

bool board_full(const board_t& board);

struct Analysis_t {
  Direction direction;
  int up_eval, down_eval, left_eval, right_eval;
  void print() const {
    cout<<"\tDirection: "<<direction_names[direction]<<endl;
    cout<<"\t  up_eval: "<<up_eval<<endl;
    cout<<"\t  down_eval: "<<down_eval<<endl;
    cout<<"\t  left_eval: "<<left_eval<<endl;
    cout<<"\t  right_eval: "<<right_eval<<endl;
  }
  bool operator!=(const Analysis_t& a2) {
    return direction != a2.direction ||
           up_eval != a2.up_eval ||
           down_eval != a2.down_eval ||
           left_eval != a2.left_eval ||
           right_eval != a2.right_eval;
  }
};

Analysis_t advice(const board_t& board,
                  const Move_Result& up_result,
                  const Move_Result& down_result,
                  const Move_Result& left_result,
                  const Move_Result& right_result,
                  bool opt);

void add_new_tile(board_t& board);

int main() {
  srand(time(NULL));
  detail_out.open("game_detail.txt");
  int score = 0;

  board_t board = 0;

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
    SmallBoard::set_val(board, new_block.x-1, new_block.y-1, new_block.val);
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
    Move_Result up_result = up_move(board);
    if(DETAIL) {
      detail_out<<"Up board for this move:"<<endl;
      print_board(up_result.board, true);
    }

    // Down move
    Move_Result down_result = down_move(board);
    // Left move
    Move_Result left_result = left_move(board);
    // Right move
    Move_Result right_result = right_move(board);

    if(board_full(up_result.board) &&
        board_full(down_result.board) &&
        board_full(left_result.board) &&
        board_full(right_result.board)) {
      cout<<"Game Over"<<endl;
      cerr<<"Score: "<<score<<endl;
      return 0;
    }

    Analysis_t analysis = advice(board,
                              up_result,
                              down_result, 
                              left_result,
                              right_result, true);
    Direction choice = analysis.direction;
 /*    Direction choice = advice(board,
                              up_result,
                              down_result, 
                              left_result,
                              right_result);*/
    cout<<"\n********Move "<<direction_names[choice];
    cout<<"********"<<endl;

    switch(choice) {
      case UP:
        board = up_result.board;
        score += up_result.combos_value;
        break;
      case DOWN:
        board = down_result.board;
        score += down_result.combos_value;
        break;
      case LEFT:
        board = left_result.board;
        score += left_result.combos_value;
        break;
      case RIGHT:
        board = right_result.board;
        score += right_result.combos_value;
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
int heuristic(const board_t& board) {
  int num_tiles = 0;

  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if (SmallBoard::val_at(board, x, y) != 0) {
        num_tiles++;
      }
    }
  }
  int max_tiles = 4 * 4;
  return (max_tiles - num_tiles);
}

const int MAX_DEPTH = 6;

int depth = 0;

int eval_board_outcomes(const board_t& board, int best_seen, bool opt);

int eval_board_moves(const board_t& board, int worst_seen, bool opt) {
  depth++;
  Move_Result up_result = up_move(board);
  Move_Result down_result = down_move(board);
  Move_Result left_result = left_move(board);
  Move_Result right_result = right_move(board);

  bool up_valid = (board != up_result.board);
  bool down_valid = (board != down_result.board);
  bool left_valid = (board != left_result.board);
  bool right_valid = (board != right_result.board);

  int up_eval, down_eval, left_eval, right_eval;

  if (depth < MAX_DEPTH) {
    if(up_valid) {
      if(DETAIL) {
        for(int i = 0; i < depth; i++) {detail_out<<"  ";}
        detail_out<<"eval_moves calling eval_outcomes of up result"<<endl;
      }
    }
    up_eval = up_valid ? eval_board_outcomes(up_result.board, 0, opt) : -1;
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
    down_eval = down_valid ? eval_board_outcomes(down_result.board, up_eval, opt) : -1;
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
    left_eval = left_valid ? eval_board_outcomes(left_result.board, max(up_eval, down_eval), opt) : -1;
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
    right_eval = right_valid ? eval_board_outcomes(right_result.board, max(up_eval, max(down_eval, left_eval)), opt) : -1;
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
    up_eval = up_valid ? heuristic(up_result.board) : -1;
    down_eval = down_valid ? heuristic(down_result.board) : -1;
    left_eval = left_valid ? heuristic(left_result.board) : -1;
    right_eval = right_valid ? heuristic(right_result.board) : -1;
    if(DETAIL) {
      for(int i = 0; i < depth; i++) {detail_out<<"  ";}
      detail_out<<"eval_moves end case returning best : ";
      detail_out<<max(up_eval, down_eval, left_eval, right_eval)<<endl;
    }
  }
  depth--;
  return max(up_eval, down_eval, left_eval, right_eval);
}

int eval_board_outcomes(const board_t& board, int best_seen, bool opt) {
  depth++;
  deque<board_t> possible_outcomes;

  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if (SmallBoard::val_at(board, x, y) == 0) {
        board_t board_2 = board;
        SmallBoard::set_val(board_2, x, y, 2);
        possible_outcomes.push_back(board_2);

        board_t board_4 = board;
        SmallBoard::set_val(board_2, x, y, 4);
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

Analysis_t advice(const board_t& board,
                 const Move_Result& up_result,
                 const Move_Result& down_result,
                 const Move_Result& left_result,
                 const Move_Result& right_result, bool opt) {
  if(DETAIL)
    detail_out<<"Getting advice for this board:"<<endl;
  print_board(board, true);
  int up_val;
  int down_val;
  int left_val;
  int right_val;

  bool up_valid = (board != up_result.board);
  bool down_valid = (board != down_result.board);
  bool left_valid = (board != left_result.board);
  bool right_valid = (board != right_result.board);

  if(up_valid){
    if(DETAIL)
      detail_out<<"advice evaluating up move"<<endl;
  }
  up_val = up_valid ? eval_board_outcomes(up_move(board).board, 0, opt) : -1;
  if(up_valid && DETAIL)
    detail_out<<"advice got up_val of "<<up_val<<endl;
  else if(DETAIL) {
    detail_out<<"advice: up is not valid"<<endl;
  }

  if(down_valid) {
    if(DETAIL)
      detail_out<<"advice evaluating down move"<<endl;
  }
  down_val = down_valid ? eval_board_outcomes(down_move(board).board, up_val, opt) : -1;
  if(down_valid && DETAIL)
    detail_out<<"advice got down_val of "<<down_val<<endl;

  if(left_valid) {
    if(DETAIL)
      detail_out<<"advice evaluating left mvoe"<<endl;
  }
  left_val = left_valid ? eval_board_outcomes(left_move(board).board, max(up_val, down_val), opt) : -1;
  if(left_valid && DETAIL)
    detail_out<<"advice got left_val of "<<left_val<<endl;

  if(right_valid) {
    if(DETAIL)
      detail_out<<"advice evaluting right move"<<endl;
  }
  right_val = right_valid ? eval_board_outcomes(right_move(board).board, max(up_val, max(down_val, left_val)), opt) : -1;
  if(right_valid && DETAIL)
    detail_out<<"advice got right_val of "<<right_val<<endl;

  int max_val = max(up_val, down_val, left_val, right_val);
  cout<<"\tup_val: "<<up_val<<"\n\tdown_val: "<<down_val<<"\n\tleft_val:"<<left_val<<"\n\tright_val:"<<right_val<<endl;
  Analysis_t a;
  a.up_eval = up_val;
  a.down_eval = down_val;
  a.left_eval = left_val;
  a.right_eval = right_val;

  if(max_val == up_val && up_valid) {
    a.direction = UP;
  }
  else if (max_val == down_val || (max_val == up_val && !up_valid)) {
    a.direction = DOWN;
  }
  else if(max_val == left_val && left_valid) {
    a.direction = LEFT;
  }
  else {
    a.direction = RIGHT;
  }
  return a;
}

bool board_full(const board_t& board) {
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if(SmallBoard::val_at(board,x,y) != 0) return false;
    }
  }
  return true;
}

void add_new_tile(board_t& board) {
  deque<Block> empty_blocks;
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if(SmallBoard::val_at(board,x,y) == 0) {
        Block next_block;
        next_block.x = x;
        next_block.y = y;
        next_block.val = SmallBoard::val_at(board,x,y);
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

  SmallBoard::set_val(board,block_to_fill.x,block_to_fill.y,block_to_fill.val);
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
  board_t board = in_board;
  for (int x = 0; x < 4; x++) {
    // Shift everthing up
    for (int y = 2; y >= 0; y--) {
      if (SmallBoard::val_at(board,x,y) == 0) continue;
      int highest_empty_y = y;
      for(int i = y + 1; i <= 3 && i >= 0; i++) {
        if (SmallBoard::val_at(board,x,i) == 0) 
          highest_empty_y = i;
      }
      if(highest_empty_y != y) {
        SmallBoard::set_exp(board,
                            x,
                            highest_empty_y, 
                            SmallBoard::exp_at(board,x,y));
        SmallBoard::set_exp(board,x,y,0);
      }
    }
    
    // Look for combinations
    for(int y = 3; y > 0; y--) {
      if (SmallBoard::val_at(board,x,y) == 0 ||
          SmallBoard::val_at(board,x,y-1) == 0) continue;
      if(SmallBoard::exp_at(board,x,y) == SmallBoard::exp_at(board,x,y-1)) {
        SmallBoard::set_exp(board,x,y,SmallBoard::exp_at(board,x,y)+1);
        result.num_combos++;
        result.combos_value += SmallBoard::val_at(board,x,y);
        // Shift all blocks from y-2 down to 0 up
        for(int i = y-2; i >= 0; i--) {
          SmallBoard::set_exp(board,x,i+1,SmallBoard::exp_at(board,x,i));
        }
        SmallBoard::set_exp(board,x,0,0);
      }
    }
  }
  result.board = board;
  return result;
}

Move_Result down_move(const board_t& in_board) {
  Move_Result result;
  board_t board;
  board = in_board;
  for (int x = 0; x < 4; x++) {
    // Shift everthing down 
    for (int y = 1; y <= 3; y++) {
      if (SmallBoard::val_at(board,x,y) == 0) continue;
      int lowest_empty_y = y;
      for(int i = y - 1; i >= 0; i--) {
        if (SmallBoard::val_at(board,x,i) == 0) 
          lowest_empty_y = i;
      }
      if(lowest_empty_y != y) {
        SmallBoard::set_exp(board,
                            x,
                            lowest_empty_y, 
                            SmallBoard::exp_at(board,x,y));
        SmallBoard::set_exp(board,x,y,0);
      }
    }
    
    // Look for combinations
    for(int y = 0; y < 3; y++) {
      if (SmallBoard::val_at(board,x,y) == 0 ||
          SmallBoard::val_at(board,x,y+1) == 0) continue;
      if(SmallBoard::exp_at(board,x,y) == SmallBoard::exp_at(board,x,y+1)) {
        SmallBoard::set_exp(board,x,y,SmallBoard::exp_at(board,x,y)+1);
        result.num_combos++;
        result.combos_value += SmallBoard::val_at(board,x,y);
        // Shift all blocks from y+2 up to 3 down
        for(int i = y+2; i <= 3; i++) {
          SmallBoard::set_exp(board,x,i-1,SmallBoard::exp_at(board,x,i));
        }
        SmallBoard::set_exp(board,x,3,0);
      }
    }
  }
  result.board = board;
  return result;
}

Move_Result left_move(const board_t& in_board) {
  Move_Result result;
  board_t board;
  board = in_board;
  for (int y = 0; y < 4; y++) {
    // Shift everthing left 
    for (int x = 1; x <= 3; x++) {
      if (SmallBoard::val_at(board,x,y) == 0) continue;
      int leftest_empty_x = x;
      for(int i = x - 1; i >= 0; i--) {
        if (SmallBoard::val_at(board,i,y) == 0) 
          leftest_empty_x = i;
      }
      if(leftest_empty_x != x) {
        SmallBoard::set_exp(board,
                            leftest_empty_x,
                            y, 
                            SmallBoard::exp_at(board,x,y));
        SmallBoard::set_exp(board,x,y,0);
      }
    }
    
    // Look for combinations
    for(int x = 0; x < 3; x++) {
      if (SmallBoard::val_at(board,x,y) == 0 ||
          SmallBoard::val_at(board,x+1,y) == 0) continue;
      if(SmallBoard::exp_at(board,x,y) == SmallBoard::exp_at(board,x+1,y)) {
        SmallBoard::set_exp(board,x,y,SmallBoard::exp_at(board,x,y)+1);
        result.num_combos++;
        result.combos_value += SmallBoard::val_at(board,x,y);
        // Shift all blocks from x+2 up to 3 left
        for(int i = x+2; i <= 3; i++) {
          SmallBoard::set_exp(board,i-1,y,SmallBoard::exp_at(board,i,y));
        }
        SmallBoard::set_exp(board,3,y,0);
      }
    }
  }
  result.board = board;
  return result;
}

Move_Result right_move(const board_t& in_board) {
  Move_Result result;
  board_t board;
  board = in_board;
  for (int y = 0; y < 4; y++) {
    // Shift everthing right 
    for (int x = 2; x >= 0; x--) {
      if (SmallBoard::val_at(board,x,y) == 0) continue;
      int rightest_empty_x = x;
      for(int i = x + 1; i <= 3; i++) {
        if (SmallBoard::val_at(board,i,y) == 0) 
          rightest_empty_x = i;
      }
      if(rightest_empty_x != x) {
        SmallBoard::set_exp(board,
                            rightest_empty_x,
                            y, 
                            SmallBoard::exp_at(board,x,y));
        SmallBoard::set_exp(board,x,y,0);
      }
    }
    
    // Look for combinations
    for(int x = 3; x > 0; x--) {
      if (SmallBoard::val_at(board,x,y) == 0 ||
          SmallBoard::val_at(board,x-1,y) == 0) continue;
      if(SmallBoard::exp_at(board,x,y) == SmallBoard::exp_at(board,x-1,y)) {
        SmallBoard::set_exp(board,x,y,SmallBoard::exp_at(board,x,y)+1);
        result.num_combos++;
        result.combos_value += SmallBoard::val_at(board,x,y);
        // Shift all blocks from x-2 down to 0 right
        for(int i = x-2; i >= 0; i--) {
          SmallBoard::set_exp(board,i+1,y,SmallBoard::exp_at(board,i,y));
        }
        SmallBoard::set_exp(board,0,y,0);
      }
    }
  }
  result.board = board;
  return result;
}

