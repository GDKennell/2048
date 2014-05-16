#include <iostream>
#include <cassert>
#include <deque>

using namespace std;

struct Block;

typedef deque<deque<Block> > board_t;

struct Block {
  int val;
  int x,y;
  bool empty;
  Block() : val(0), x(0), y(0), empty(true){ }
  void print() const {
    if(empty) cout<<"    ";
    else if(val < 10) cout<<" "<<val<<"  ";
    else if(val < 100) cout<<" "<<val<<" ";
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

int main() {
  board_t board;
  board.resize(4);
  for(int i = 0; i < 4; i++){ board[i].resize(4); }

/*  Block init_block1 = input_block();
  board[init_block1.x - 1][init_block1.y - 1] = init_block1;
  cout<<endl;
  Block init_block2 = input_block();
  board[init_block2.x - 1][init_block2.y - 1] = init_block2;
  cout<<endl;*/
  int num_tiles = 6;
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

//  while(1) {
    // Up move
    Move_Result up_result = up_move(board);
    cout<<"Up Move: "<<endl;
    print_board(up_result.board);
    cout<<up_result.num_combos<<" combos worth ";
    cout<<up_result.combos_value<<endl<<endl;

    // Down move
    Move_Result down_result = down_move(board);
    cout<<"Down Move: "<<endl;
    print_board(down_result.board);
    cout<<down_result.num_combos<<" combos worth ";
    cout<<down_result.combos_value<<endl<<endl;

    // Left move
    Move_Result left_result = left_move(board);
    cout<<"Left Move: "<<endl;
    print_board(left_result.board);
    cout<<left_result.num_combos<<" combos worth ";
    cout<<left_result.combos_value<<endl<<endl;

    // Right move
    Move_Result right_result = right_move(board);
    cout<<"Right Move: "<<endl;
    print_board(right_result.board);
    cout<<right_result.num_combos<<" combos worth ";
    cout<<right_result.combos_value<<endl<<endl;

//  }

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

