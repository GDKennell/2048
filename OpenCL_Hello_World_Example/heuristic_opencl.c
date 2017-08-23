//__constant int NOT_THIS_ONE =  50000;
//float multiply(float a, float b) { return a * b; }
//__kernel void square(
//                     __global float* input,
//                     __global float* output,
//                     const unsigned int count)
//{
//    int i = get_global_id(0);
//    if(i < count && i != NOT_THIS_ONE) {
//        output[i] = multiply(input[i], input[i]);
//    }
//}


__constant int NUM_TRANSFORMS = 65536;

int64_t raw_column(__global uint64_t board, int col_num)
{
    int offset = 16 * col_num;
    return (board >> offset) & 0xffff;
}

uint64_t heuristic(__global uint64_t board, __global int* empty_vals)
{
  __int64_t num_empty = 0;
  for(__int64_t c = 0; c < 4; ++c) {
    __int64_t column = raw_column(board,c);
    num_empty += empty_vals[column];
  }
  return num_empty;
}

__kernel void calculate_heuristics (__global uint64_t* boards,
                                    __global int* empty_vals,
                                    __global uint64_t* output,
                                    const unsigned int count)
{
    int i = get_global_id(0);
    if(i < count)
        output[i] = heuristic(input[i], empty_vals);
}


"__constant int NUM_TRANSFORMS = 65536;  \n"\
"  \n"\
"int64_t raw_column(__global uint64_t board, int col_num)  \n"\
"{  \n"\
"    int offset = 16 * col_num;  \n"\
"    return (board >> offset) & 0xffff;  \n"\
"}  \n"\
"  \n"\
"uint64_t heuristic(__global uint64_t board, __global int* empty_vals)  \n"\
"{  \n"\
"  int64_t num_empty = 0;  \n"\
"  for(int64_t c = 0; c < 4; ++c) {  \n"\
"    int64_t column = raw_column(board,c);  \n"\
"    num_empty += empty_vals[column];  \n"\
"  }  \n"\
"  return num_empty;  \n"\
"}  \n"\
"  \n"\
"__kernel void calculate_heuristics (__global uint64_t* boards,  \n"\
"                                    __global int* empty_vals,  \n"\
"                                    __global uint64_t* output,  \n"\
"                                    const unsigned int count)  \n"\
"{  \n"\
"    int i = get_global_id(0);  \n"\
"    if(i < count)  \n"\
"        output[i] = heuristic(input[i], empty_vals);  \n"\
"}  \n";
