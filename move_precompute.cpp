#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;


int main() {

  ofstream left_file("left.bin", ios::out | ios::binary);
  ofstream right_file("right.bin", ios::out | ios::binary);

  for(int row = 0; row < (uint64_t)pow(2,16); ++row) {
    if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
      cout<<"\n\n*************\nrow: "<<row<<endl;
    }
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

    if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
      cout<<"\tafter shift left, result = "<<result<<endl;
    }

    unsigned int combos_val = 0;
    // perform combos on shifted board
    for(int x = 0; x < 3; ++x) {
      if(((result >> (4*x)) & 0xf) != 0 && ((result >> (4*x)) & 0xf) == ((result >> (4*(x+1))) & 0xf)) {
        int new_val = ((result >> (4*x)) & 0xf) + 1;
        result &= ~(0xf << (4*(x)));
        result |= (new_val << (4*(x)));
        combos_val += pow(2,new_val);
        if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
          cout<<"\tcombos_val += "<<pow(2,new_val)<<endl;
        }
        for(int i = x+1; i < 3; ++i) {
          int next_val = ((result >> (4*(i+1))) & 0xf);
          if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
            cout<<"\tshifting val "<<next_val<<" from "<<i+1<<" to "<<i<<endl;
          }
          result &= ~(0xf << (4 * i));
          if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
            cout<<"\tcleared word "<<i<<", result = "<<result<<endl;
          }
          result |= (next_val << (4 * i));
          if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
            cout<<"\tset word "<<i<<" to "<<next_val<<", result = "<<result<<endl;
          }
        }
        result &= ~(0xf << 3*4);
        if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
          cout<<"\tcleared far right bit, result = "<<result<<endl;
        }
      }
    }

    if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
      cout<<" has left transform of "<<result<<", and combos: "<<combos_val<<endl;
    }
    result |= (combos_val << 16);
    left_file.write((char*)&result,4);
  }

  for(int row = 0; row < pow(2,16); ++row) {
    if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
      cout<<"\n\n*****right********\nrow: "<<row<<endl;
    }
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

    if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
      cout<<"\tshifted everything right, result = "<<result<<endl;
    }

    unsigned int combos_val = 0;
    // perform combos on shifted board
    for(int x = 3; x > 0; --x) {
      if(((result >> (4*x)) & 0xf) != 0 && ((result >> (4*x)) & 0xf) == ((result >> (4*(x-1))) & 0xf)) {
        int new_val = ((result >> (4*x)) & 0xf) + 1;
        if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
          cout<<"\tfound combo, doubling val at x="<<x<<", new_val = "<<new_val<<endl;
        }
        result &= ~(0xf << (4*(x)));
        result |= (new_val << (4*(x)));
        if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
          cout<<"\tnew result = "<<result<<endl;
        }
        combos_val += pow(2,new_val);
        for(int i = x-1; i > 0; --i) {
          int next_val = ((result >> (4*(i-1))) & 0xf);
          if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
            cout<<"\tshifting right, moving next_val("<<next_val<<") from "<<i-1<<" to "<<i<<endl;
          }
          result &= ~(0xf << (4 * i));
          result |= (next_val << (4 * i));
          if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
            cout<<"\tnew result: "<<result<<endl<<endl;
          }
        }
        result &= ~0xf;
      }
    }
    if(row == 0x1122 || row == 0x0011 || row == 0x0110 || row == 0x0100 || row == 0x0010 ||  row == 0x0123 || row == 0x1111 || row == 0x2345) {
      cout<<"\t result = "<<result<<", combos_val = "<<combos_val<<endl;
    }
    result |= (combos_val << 16);
    right_file.write((char*)&result,4);
  }
}

