#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

// Rows encoded as 4 tiles of 4 bits = 16 bits
// All possible rows: 2^16 combinations = 65536
const int NUM_POSSIBLE_ROWS = 65536;

void create_move_precompute_files() {
  
  int left_file_data[NUM_POSSIBLE_ROWS];
  int right_file_data[NUM_POSSIBLE_ROWS];

  // For each possible row of 4 tiles, each 2^0 (blank) through 2^16
  // Compute left shifts
  for(unsigned int row = 0; row < NUM_POSSIBLE_ROWS; ++row) {
    unsigned int result = row;
    // Shift result left
    for(int x = 0; x < 4; ++x) {
      int tile_x_val = ((result >> (4*x)) & 0xf);
      if(tile_x_val != 0) {
        int leftest_x = x;
        // Iterating left from x
        for(int xi = x-1; xi >= 0; --xi) {
          // If tile left of x is blank, store it
          if(((result >> (4*xi)) & 0xf) == 0) {
            leftest_x = xi;
          }
        }
        // If there is blank tile left of x
        if(leftest_x != x) {
          // Clear the tile at x
          result &= ~(0xf << (4*x));
          // Store tile x where the farthest left blank was
          result |= (tile_x_val << (4*leftest_x));
        }
      }
    }

    unsigned int combos_val = 0;
    // Perform combinations on shifted board
    for(int x = 0; x < 3; ++x) {
      unsigned int this_tile = (result >> (4*x)) & 0xf;
      unsigned int next_tile = (result >> (4*(x+1))) & 0xf;

      if(this_tile != 0 && this_tile == next_tile) {
        unsigned int new_val = this_tile + 1;
        // Set new_val at x
        result &= ~(0xf << (4*x));
        result |= (new_val << (4*x));
        combos_val += pow(2,new_val);

        // Shift rest of row left
        for(int i = x+1; i < 3; ++i) {
          unsigned int next_val = ((result >> (4*(i+1))) & 0xf);
          result &= ~(0xf << (4 * i));
          result |= (next_val << (4 * i));
        }
        // Clear right-most tile
        result &= ~(0xf << 3*4);
      }
    }

    // Encode the score from the combinations after the resulting row
    result |= (combos_val << 16);
    
    left_file_data[row] = result;

    // Mirror this row horizontally and store result in right_file
    unsigned int flipped_row = 0;
    for (int i = 0; i < 4; ++i) {
      unsigned int tile_i = (row >> (4 * i)) & 0xf;
      tile_i = tile_i << (4 * (3-i));
      flipped_row |= tile_i;
    }

    unsigned int flipped_result = 0;
    flipped_result |= (combos_val << 16);
    for (int i = 0; i < 4; ++i) {
      unsigned int tile_i = (result >> (4 * i)) & 0xf;
      tile_i = tile_i << (4 * (3-i));
      flipped_result |= tile_i;
    }

    right_file_data[flipped_row] = flipped_result;
  }

  // Write computed arrays to files
  ofstream left_file("left.bin", ios::out | ios::binary);
  ofstream right_file("right.bin", ios::out | ios::binary);

  for (int i = 0; i < pow(2,16); ++i) {
    left_file.write((char*)&left_file_data[i],4);
    right_file.write((char*)&right_file_data[i],4);
  }

  left_file.close();
  right_file.close();
}

