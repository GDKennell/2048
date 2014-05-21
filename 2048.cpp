#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <string>
#include <time.h>

using namespace std;

struct Block;

typedef deque<deque<Block> > board_t;

enum Direction {UP, DOWN, LEFT, RIGHT};

const char* direction_names[] = {"up", "down", "left", "right"};

struct Block {
  int val;
  int x,y;
  bool empty;
  Block() : val(0), x(0), y(0), empty(true){ }
  void print() const {
    if(empty) cout<<"    ";
    else if(val < 10) cout<<" "<<val<<"  ";
    else if(val < 100) cout<<" "<<val<<" ";
    else if(val < 1000) cout<<val<<' ';
    else cout<<val;
  }
};

struct Move_Result {
  board_t board; // Resulting board
  int num_combos; // Number of combinations made by move
  int combos_value; // Sum of final values from combinations (0 if none)
  Move_Result() : num_combos(0), combos_value(0){ }
};

Block input_block();

void print_board(const board_t& board) {
  for(int y = 3; y >= 0; y--) {
    cout<<"   ___________________"<<endl;
    cout<<"  ";
    for(int x = 0; x <= 3; x++) {
      cout<<'|';
      board[x][y].print();
    }
    cout<<'|'<<endl;
  }
  cout<<"   ___________________"<<endl;
}

Move_Result up_move(const board_t& in_board);
Move_Result down_move(const board_t& in_board);
Move_Result left_move(const board_t& in_board);
Move_Result right_move(const board_t& in_board);

// Returns number of same, adjacent tiles (that can be combined)
int num_adjacent(const board_t& board);

// Returns value corresponding to 2's and 4's next to open spaces
// weighted as 6 for each 2 and 1 for each 4
int num_open24(const board_t& board);

bool board_full(const board_t& board);

void copy_board(board_t& dest, const board_t& source);
bool boards_same(const board_t& b1, const board_t& b2);

Direction advice(const board_t& board,
                 const Move_Result& up_result,
                 const Move_Result& down_result,
                 const Move_Result& left_result,
                 const Move_Result& right_result);

void add_new_tile(board_t& board);

int main() {
  int score = 0;
  srand(time(NULL));

  board_t board;
  board.resize(4);
  for(int i = 0; i < 4; i++){ board[i].resize(4); }

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
    board[new_block.x-1][new_block.y-1] = new_block;
    cout<<endl;
  }

  cout<<"Initial board:"<<endl;
  print_board(board);

  while(1) {
    if(board_full(board)) {
      cout<<"Game Over"<<endl;
      return 0;
    }
/*    cout<<"Ready for me to do the next move?"<<endl;
    string dontcare;
    cin >>dontcare;*/
    
    // Up move
    Move_Result up_result = up_move(board);
    // Down move
    Move_Result down_result = down_move(board);
    // Left move
    Move_Result left_result = left_move(board);
    // Right move
    Move_Result right_result = right_move(board);

/*    cout<<"Up Move: "<<endl;
    print_board(up_result.board);
    cout<<up_result.num_combos<<" combos worth ";
    cout<<up_result.combos_value<<endl<<endl;

    cout<<"Down Move: "<<endl;
    print_board(down_result.board);
    cout<<down_result.num_combos<<" combos worth ";
    cout<<down_result.combos_value<<endl<<endl;

    cout<<"Left Move: "<<endl;
    print_board(left_result.board);
    cout<<left_result.num_combos<<" combos worth ";
    cout<<left_result.combos_value<<endl<<endl;

    cout<<"Right Move: "<<endl;
    print_board(right_result.board);
    cout<<right_result.num_combos<<" combos worth ";
    cout<<right_result.combos_value<<endl<<endl;*/

    Direction choice = advice(board,
                              up_result,
                              down_result, 
                              left_result,
                              right_result);
    cout<<"\n********Move "<<direction_names[choice];
    cout<<"********"<<endl;

    switch(choice) {
      case UP:
        copy_board(board, up_result.board);
        score += up_result.combos_value;
        break;
      case DOWN:
        copy_board(board, down_result.board);
        score += down_result.combos_value;
        break;
      case LEFT:
        copy_board(board, left_result.board);
        score += left_result.combos_value;
        break;
      case RIGHT:
        copy_board(board, right_result.board);
        score += right_result.combos_value;
        break;
    }
    cout<<"********Score: "<<score<<"********\n"<<endl;
    cout<<"Input new tile"<<endl;

//    Block new_block = input_block();
//    board[new_block.x-1][new_block.y-1] = new_block;

//      cout<<"Added new tile, current board: "<<endl;
//      print_board(board);

    add_new_tile(board);
    cout<<"Added new tile, current board: "<<endl;
    print_board(board);
    cout<<endl;
    
  }

}

Direction advice(const board_t& board,
                 const Move_Result& up_result,
                 const Move_Result& down_result,
                 const Move_Result& left_result,
                 const Move_Result& right_result) {
  //Count empty spaces to leave 2's and 4's free if little space
  int num_empty = 0;
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if(board[x][y].empty)
        num_empty++;
    }
  }

  bool up_valid = !boards_same(board, up_result.board);
  bool down_valid = !boards_same(board, down_result.board);
  bool left_valid = !boards_same(board, left_result.board);
  bool right_valid = !boards_same(board, right_result.board);

  //Precalculate num adjacent after move for all 4
  int up_adj = up_valid ? num_adjacent(up_result.board) : -1;
  int down_adj = down_valid ? num_adjacent(down_result.board) : -1;
  int left_adj = left_valid ? num_adjacent(left_result.board) : -1;
  int right_adj = right_valid ? num_adjacent(right_result.board) : -1;
  int max_adj = max(up_adj, max(down_adj, max(left_adj, right_adj) ) );

  // Number of 2's and 4's left next to open spaces
  int up_open24 = up_valid ? num_open24(up_result.board) : -1;
  int down_open24 = down_valid ? num_open24(down_result.board) : -1;
  int left_open24 = left_valid ? num_open24(left_result.board) : -1;
  int right_open24 = right_valid ? num_open24(right_result.board) : -1;
  int max_open24 = max(up_open24, max(down_open24, max(left_open24, right_open24) ) );
  
  // First pick left-right or up-down based on num combos

  // If only one space left, prioritize open24 unless >2 adjacent
  if(num_empty == 1 && max_adj < 2) {
    cout<<"\topen24 decision.   up: "<<up_open24<<endl;
    cout<<"\t                 down: "<<down_open24<<endl;
    cout<<"\t                 left: "<<left_open24<<endl;
    cout<<"\t                right: "<<right_open24<<endl;
    if(max_open24 == up_open24 && up_valid)
      return UP;
    else if(max_open24 == down_open24 && down_valid)
      return DOWN;
    else if((max_open24 == left_open24 && left_valid) || !right_valid) 
      return LEFT;
    else
      return RIGHT;
  }

  // up/down beats left/right
  if (up_result.num_combos > left_result.num_combos) {
    if(!up_valid) return DOWN;
    if(!down_valid) return UP;
    // pick up/down based on num adjacent after move
    if (up_adj > down_adj) {
      return UP;
    }
    else if (down_adj > up_adj) {
      return DOWN;
    }
    else {
      cout<<"\topen24 tie breaker. up: "<<up_open24;
      cout<<", down: "<<down_open24<<endl;
      return (up_open24 > down_open24) ? UP : DOWN;
    }
  }
  // left/right beats up/down
  else if(left_result.num_combos > up_result.num_combos) {
    if(!left_valid) return RIGHT;
    if(!right_valid) return LEFT;
    // pick left/right based on num adjacent after move
    if (left_adj > right_adj) {
      return LEFT;
    }
    else if(right_adj > left_adj) {
      return RIGHT;
    }
    else {
      cout<<"\topen24 tie breaker. left: "<<left_open24;
      cout<<", right: "<<right_open24<<endl;
      return (left_open24 > right_open24) ? LEFT : RIGHT;
    }
  }
 // same num combos for every direction, use num_adjacent after move
 // break ties with open24
  else {
    deque<Direction> best_adjs;
    if(up_adj == max_adj && up_valid) best_adjs.push_back(UP);
    if(down_adj == max_adj && down_valid) best_adjs.push_back(DOWN);
    if(left_adj == max_adj && left_valid) best_adjs.push_back(LEFT);
    if(right_adj == max_adj && right_valid) best_adjs.push_back(RIGHT);
    cout<<"adjs: "<<up_adj<<' '<<down_adj<<' '<<left_adj<<' '<<right_adj<<endl;
    cout<<"max_adj: "<<max_adj<<endl;
    assert(!best_adjs.empty());
    if(best_adjs.size() == 1) {
      cout<<"\treturning single best adjacent move"<<endl;
      return best_adjs.front();
    }
    deque<Direction> best_open24s;
    if(up_open24 == max_open24 && up_valid) best_open24s.push_back(UP);
    if(down_open24 == max_open24 && down_valid) best_open24s.push_back(DOWN);
    if(left_open24 == max_open24 && left_valid) best_open24s.push_back(LEFT);
    if(right_open24 == max_open24 && right_valid) best_open24s.push_back(RIGHT);
    assert(!best_open24s.empty());

    deque<Direction> intersection(4);
    deque<Direction>::iterator it = set_intersection(best_adjs.begin(), best_adjs.end(),
                                                     best_open24s.begin(), best_open24s.end(),
                                                     intersection.begin());
    cout<<"\tmultiple best adjacents, took intersection.";
    cout<<"best_adjs: ";
    for(int i = 0 ; i < best_adjs.size(); i++) {
      cout<<direction_names[best_adjs[i]]<<' ';
    }
    cout<<endl;
    cout<<"best_open24s: ";
    for(int i = 0 ; i < best_open24s.size(); i++) {
      cout<<direction_names[best_open24s[i]]<<' ';
    }
    cout<<endl;
    if(it == intersection.begin()) {
      cout<<" It's empty, returning a best adj"<<endl;
      return best_adjs.front();
    }
    else {
      cout<<" Returning first of intersection."<<endl;
      return intersection.front();
    }
  }
}

// Returns number of same, adjacent tiles (that can be combined)
int num_adjacent(const board_t& board) {
  int result = 0;
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if(board[x][y].empty) continue;
      //scan right for next non-empty tile
      for(int xi = x + 1; xi < 4; xi++) {
        if(board[xi][y].empty) continue;
        if(board[x][y].val == board[xi][y].val) {
          result++;
        }
        break;
      }
      //scan up for next non-empty tile
      for(int yi = y + 1; yi < 4; yi++) {
        if(board[x][yi].empty) continue;
        if(board[x][y].val == board[x][yi].val) {
          result++;
        }
        break;
      }
    }
  }
  return result;
}

int num_open24(const board_t& board) {
  int result = 0;
  for(int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      bool has2 = false;
      bool has4 = false;
      if(board[x][y].empty) {
        for(int xi = x-1; xi <= x+1; xi++) {
          if(xi < 0 || xi >= 4) continue;
          for(int yi = y-1; yi <= y+1; yi++) {
            if(yi < 0 || yi >= 4) continue;
            //disallow diagonals
            if(xi != x && yi != y) continue;
            //disallow same one
            if(xi == x && yi == y) continue;
            if(!board[xi][yi].empty && board[xi][yi].val == 2)
              has2 = true;
            else if(!board[xi][yi].empty && board[xi][yi].val == 4)
              has4 = true;
          }
        }
        if(has2) {
          result += 6;
        }
        if(has4) {
          result += 1;
        }
      }
    }
  }
  return result;
}

bool board_full(const board_t& board) {
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if(board[x][y].empty) return false;
    }
  }
  return true;
}

void add_new_tile(board_t& board) {
  deque<Block> empty_blocks;
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if(board[x][y].empty) {
        board[x][y].x = x + 1;
        board[x][y].y = y + 1;
        empty_blocks.push_back(board[x][y]);
      }
    } 
  }
  if(empty_blocks.empty()) {
    throw "Board full";
  }
  Block block_to_fill = empty_blocks[rand() % empty_blocks.size()];
  block_to_fill.val = (rand() % 100 <= 15) ? 4 : 2;
  block_to_fill.empty = false;
  cout<<"New block at ("<<block_to_fill.x<<',';
  cout<<block_to_fill.y<<") with value "<<block_to_fill.val<<endl;
  board[block_to_fill.x-1][block_to_fill.y-1] = block_to_fill;
}

Block input_block() {
  Block return_block;
  return_block.empty = false;
  cout<<"New block value (2|4): ";
  cin >> return_block.val;
  assert(return_block.val == 2 || return_block.val == 4);

  cout<<"New block coords(x,y): ";
  cin >> return_block.x >> return_block.y;
  assert(return_block.x > 0 && return_block.x <= 4);
  assert(return_block.y > 0 && return_block.y <= 4);
  return return_block;
}

void copy_board(board_t& dest, const board_t& source) {
  dest.resize(4);
  for(int i = 0; i < 4; i++) {dest[i].resize(4);}
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      dest[x][y] = source[x][y];
    }
  }
}

bool boards_same(const board_t& b1, const board_t& b2) {
  for(int x = 0; x < 4; x++) {
    for(int y = 0; y < 4; y++) {
      if((b1[x][y].empty && !b2[x][y].empty) ||
         (!b1[x][y].empty && b2[x][y].empty) ||
         (!b1[x][y].empty && !b2[x][y].empty && b1[x][y].val != b2[x][y].val)) {
        return false;
      }
    }
  }
  return true;
}

Move_Result up_move(const board_t& in_board) {
  Move_Result result;
  board_t board;
  copy_board(board, in_board);
  for (int x = 0; x < 4; x++) {
    // Shift everthing up
    for (int y = 2; y >= 0; y--) {
      if (board[x][y].empty) continue;
      int highest_empty_y = y;
      for(int i = y + 1; i <= 3 && i >= 0; i++) {
        if(board[x][i].empty)
          highest_empty_y = i;
      }
      if(highest_empty_y != y) {
        board[x][highest_empty_y] = board[x][y];
        board[x][y].empty = true;
      }
    }
    
    // Look for combinations
    for(int y = 3; y > 0; y--) {
      if(board[x][y].empty || board[x][y-1].empty) continue; 
      if(board[x][y].val == board[x][y-1].val) {
        board[x][y].val *= 2;
        result.num_combos++;
        result.combos_value += board[x][y].val;
        // Shift all blocks from y-2 down to 0 up
        for(int i = y-2; i >= 0; i--) {
          board[x][i+1] = board[x][i];
        }
        board[x][0].empty = true;
      }
    }
  }
  copy_board(result.board, board);
  return result;
}

Move_Result down_move(const board_t& in_board) {
  Move_Result result;
  board_t board;
  copy_board(board, in_board);
  for (int x = 0; x < 4; x++) {
    // Shift everthing down 
    for (int y = 1; y <= 3; y++) {
      if (board[x][y].empty) continue;
      int lowest_empty_y = y;
      for(int i = y - 1; i >= 0; i--) {
        if(board[x][i].empty)
          lowest_empty_y = i;
      }
      if(lowest_empty_y != y) {
        board[x][lowest_empty_y] = board[x][y];
        board[x][y].empty = true;
      }
    }
    
    // Look for combinations
    for(int y = 0; y < 3; y++) {
      if(board[x][y].empty || board[x][y+1].empty) continue; 
      if(board[x][y].val == board[x][y+1].val) {
        board[x][y].val *= 2;
        result.num_combos++;
        result.combos_value += board[x][y].val;
        // Shift all blocks from y+2 up to 3 down
        for(int i = y+2; i <= 3; i++) {
          board[x][i-1] = board[x][i];
        }
        board[x][3].empty = true;
      }
    }
  }
  copy_board(result.board, board);
  return result;
}

Move_Result left_move(const board_t& in_board) {
  Move_Result result;
  board_t board;
  copy_board(board, in_board);
  for (int y = 0; y < 4; y++) {
    // Shift everthing left 
    for (int x = 1; x <= 3; x++) {
      if (board[x][y].empty) continue;
      int leftest_empty_x = x;
      for(int i = x - 1; i >= 0; i--) {
        if(board[i][y].empty)
          leftest_empty_x = i;
      }
      if(leftest_empty_x != x) {
        board[leftest_empty_x][y] = board[x][y];
        board[x][y].empty = true;
      }
    }
    
    // Look for combinations
    for(int x = 0; x < 3; x++) {
      if(board[x][y].empty || board[x+1][y].empty) continue; 
      if(board[x][y].val == board[x+1][y].val) {
        board[x][y].val *= 2;
        result.num_combos++;
        result.combos_value += board[x][y].val;
        // Shift all blocks from x+2 up to 3 left
        for(int i = x+2; i <= 3; i++) {
          board[i-1][y] = board[i][y];
        }
        board[3][y].empty = true;
      }
    }
  }
  copy_board(result.board, board);
  return result;
}

Move_Result right_move(const board_t& in_board) {
  Move_Result result;
  board_t board;
  copy_board(board, in_board);
  for (int y = 0; y < 4; y++) {
    // Shift everthing right 
    for (int x = 2; x >= 0; x--) {
      if (board[x][y].empty) continue;
      int rightest_empty_x = x;
      for(int i = x + 1; i <= 3; i++) {
        if(board[i][y].empty)
          rightest_empty_x = i;
      }
      if(rightest_empty_x != x) {
        board[rightest_empty_x][y] = board[x][y];
        board[x][y].empty = true;
      }
    }
    
    // Look for combinations
    for(int x = 3; x > 0; x--) {
      if(board[x][y].empty || board[x-1][y].empty) continue; 
      if(board[x][y].val == board[x-1][y].val) {
        board[x][y].val *= 2;
        result.num_combos++;
        result.combos_value += board[x][y].val;
        // Shift all blocks from x-2 down to 0 right
        for(int i = x-2; i >= 0; i--) {
          board[i+1][y] = board[i][y];
        }
        board[0][y].empty = true;
      }
    }
  }
  copy_board(result.board, board);
  return result;
}



/*
   _______________
  |x,y|x,y|x,y|x,y|
   _______________
  |x,y|x,y|x,y|x,y|
   _______________
  |x,y|x,y|x,y|x,y|
   _______________
  |x,y|x,y|x,y|x,y|
   _______________

   _______________
  |1,4|2,4|3,4|4,4|
   _______________
  |1,3|2,3|3,3|4,3|
   _______________
  |1,2|2,2|3,2|4,2|
   _______________
  |1,1|2,1|3,1|4,1|
   _______________


   -------------------
  | 2  | 16 | 4  |    |


  4 8 8 8 8   8  
  4 4 2 4 4   4  
  2 2 2 2 0   0  
  2 2 0 0 0   0  
  

   */

