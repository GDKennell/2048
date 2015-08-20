#include <fstream>
#include <cmath>

using namespace std;

void create_heur_precompute_file() {
  ofstream out_file("numempty.bin", ios::out | ios::binary);

  for(int row = 0; row < (uint64_t)pow(2,16); ++row) {
    int num_empty = 0;
    for(int i = 0; i < 4; ++i) {
      if(((row >> (4*i)) & 0xf) == 0)
        ++num_empty;
    }
    out_file.write((char*)&num_empty, sizeof(int));
  }
}

