#include <iostream>
#include <cassert>
#include <deque>

using namespace std;

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



   */

Block input_block();

void print_board(const deque<deque<Block> >& board) {
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

int main() {
  deque<deque<Block> > board;
  board.resize(4);
  for(int i = 0; i < 4; i++){ board[i].resize(4); }

  Block init_block1 = input_block();
  board[init_block1.x - 1][init_block1.y - 1] = init_block1;
  cout<<endl;
  Block init_block2 = input_block();
  board[init_block2.x - 1][init_block2.y - 1] = init_block2;
  cout<<endl;
  cout<<"Initial board:"<<endl;
  print_board(board);
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


