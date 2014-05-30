#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>

using namespace std;

const bool DETAIL = true;
ofstream detail_out;

class SmallBoard {
  public:
    static void set_exp(uint64_t& board, int x, int y, int exp) {
      assert(exp <= 11); 
      int offset = 4 * (4 * x + y);
      
      board &= ~((uint64_t)15 << offset);
      board |= ((uint64_t)exp << offset);
    }

    static void set_val(uint64_t& board, int x, int y, int val) {
      int exp = log2(val);
      set_exp(board,x,y,exp);
    }

    static int val_at(const uint64_t& board, int x, int y) {
      int exp = exp_at(board,x,y);
      return (exp == 0) ? 0 : pow(2,exp);
    }

    static int exp_at(const uint64_t& board, int x, int y) {
      int offset = 4 * (4 * x + y);
      int exp = (board >> offset) & (uint64_t)15;
      return exp;
    }

    static void print(const uint64_t& board, int x, int y, bool detail) {
      int val = val_at(board, x, y);
      if(!DETAIL && detail) return;
      ostream& output = detail ? detail_out : cout;
      if(val == 0) output<<"    ";
      else if(val < 10) output<<" "<<val<<"  ";
      else if(val < 100) output<<" "<<val<<" ";
      else if(val < 1000) output<<val<<' ';
      else output<<val;
    }
};
