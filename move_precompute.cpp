#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

void create_move_precompute_files() {

  ofstream left_file("left.bin", ios::out | ios::binary);
  ofstream right_file("right.bin", ios::out | ios::binary);

  for(int row = 0; row < (uint64_t)pow(2,16); ++row) {
    int result = row;
    // shift result  left
    for(int x = 0; x < 4; ++x) {
      if(((result >> (4*x)) & 0xf) != 0) {
        int leftest_x = x;
        for(int xi = x-1; xi >= 0; --xi) {
          if(((result >> (4*xi)) & 0xf) == 0) {
            leftest_x = xi;
          }
        }
        if(leftest_x != x) {
          int x_val = ((result >> (4*x)) & 0xf);
          result &= ~(0xf << (4*x));
          result |= (x_val << (4*leftest_x));
        }
      }
    }

    unsigned int combos_val = 0;
    // perform combos on shifted board
    for(int x = 0; x < 3; ++x) {
      if(((result >> (4*x)) & 0xf) != 0 && ((result >> (4*x)) & 0xf) == ((result >> (4*(x+1))) & 0xf)) {
        int new_val = ((result >> (4*x)) & 0xf) + 1;
        result &= ~(0xf << (4*(x)));
        result |= (new_val << (4*(x)));
        combos_val += pow(2,new_val);
        for(int i = x+1; i < 3; ++i) {
          int next_val = ((result >> (4*(i+1))) & 0xf);
          result &= ~(0xf << (4 * i));
          result |= (next_val << (4 * i));
        }
        result &= ~(0xf << 3*4);
      }
    }

    result |= (combos_val << 16);
    left_file.write((char*)&result,4);
  }

  for(int row = 0; row < pow(2,16); ++row) {
    int result = row;
    // shift first row right 
    for(int x = 3; x >= 0; --x) {
      if(((result >> (4*x)) & 0xf) != 0) {
        int rightest_x = x;
        for(int xi = x+1; xi < 4; ++xi) {
          if(((result >> (4*xi)) & 0xf) == 0) {
            rightest_x = xi;
          }
        }
        if(rightest_x != x) {
          int x_val = ((result >> (4*x)) & 0xf);
          result &= ~(0xf << (4*x));
          result |= (x_val << (4*rightest_x));
        }
      }
    }


    unsigned int combos_val = 0;
    // perform combos on shifted board
    for(int x = 3; x > 0; --x) {
      if(((result >> (4*x)) & 0xf) != 0 && ((result >> (4*x)) & 0xf) == ((result >> (4*(x-1))) & 0xf)) {
        int new_val = ((result >> (4*x)) & 0xf) + 1;
        result &= ~(0xf << (4*(x)));
        result |= (new_val << (4*(x)));
        combos_val += pow(2,new_val);
        for(int i = x-1; i > 0; --i) {
          int next_val = ((result >> (4*(i-1))) & 0xf);
          result &= ~(0xf << (4 * i));
          result |= (next_val << (4 * i));
        }
        result &= ~0xf;
      }
    }
    result |= (combos_val << 16);
    right_file.write((char*)&result,4);
  }
}

