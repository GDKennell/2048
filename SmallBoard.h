#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>

using namespace std;

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


class SmallBoard {
  uint64_t board;

public:
  SmallBoard(uint64_t init_val = (uint64_t)0) : board(init_val) { }
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

    Block block_at(int x, int y) const {
        return Block(x,y,this->val_at(x,y));
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

    int raw_col(int row_num) const {
      assert(row_num >=0 && row_num < 4);
      int offset = 16 * row_num;
      return (board >> offset) & 0xffff;
    }

    void set_col(int row_num, int row) {
      assert(row_num >=0 && row_num < 4);
      int offset = 16 * row_num;
      board &= ~((uint64_t)0xffff << offset);
      board |= ((uint64_t)row << offset);
    }

    int raw_row(int col_num) const {
      assert(col_num >=0 && col_num < 4);
      int col = 0;
      for(int x = 0; x < 4; ++x) {
        int offset = (16 * x) + (4 * col_num);
        col |= ((board >> offset) & 0xf) << (4 * x);
      }
      return col;
    }

    void set_row(int col_num, uint64_t col) {
      assert(col_num >=0 && col_num < 4);
      for(int x = 0; x < 4; ++x) {
        int offset = (16 * x) + (4 * col_num);
        int val = ((col >> (4*x)) & (uint64_t)0xf);
        board &= ~((uint64_t)0xf << offset);
        board |= ((uint64_t)val << offset);
      }
    }

    uint64_t raw() const {
      return board;
    }
};
