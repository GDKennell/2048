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
    cout<<up_result.combos_value<<endl;
    // Down move
    // Left move
    // Right move
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
        cout<<"("<<x<<','<<y<<") matches ";
        cout<<"("<<x<<','<<y-1<<"), num_combos++"<<endl;
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

