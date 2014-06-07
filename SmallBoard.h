#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>

using namespace std;

class SmallBoard {
  uint64_t board;

public:
  SmallBoard() : board((uint64_t)0) { }
  operator uint64_t() { return board;}
  void operator=(const SmallBoard& b){ board = b.board; }
  void operator=(int b){ assert(b == 0); board = (uint64_t)b;}
  bool operator==(const SmallBoard& b) const {return board == b.board;} 
  bool operator!=(const SmallBoard& b) const {return board != b.board;} 

    void set_exp(int x, int y, int exp) {
      int offset = 4 * (4 * x + y);
      
      board &= ~((uint64_t)15 << offset);
      board |= ((uint64_t)exp << offset);
    }

    void set_val(int x, int y, int val) {
      int exp = log2(val);
      set_exp(x,y,exp);
    }

    int val_at(int x, int y) const  {
      int exp = exp_at(x,y);
      return (exp == 0) ? 0 : pow(2,exp);
    }

    int exp_at(int x, int y) const  {
      int offset = 4 * (4 * x + y);
      int exp = (board >> offset) & (uint64_t)15;
      return exp;
    }

    void print(int x, int y) const {
      int val = val_at(x, y);
      ostream& output = cout;
      if(val == 0) output<<"    ";
      else if(val < 10) output<<" "<<val<<"  ";
      else if(val < 100) output<<" "<<val<<" ";
      else if(val < 1000) output<<val<<' ';
      else output<<val;
    }
};
