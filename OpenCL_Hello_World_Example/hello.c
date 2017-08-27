//
// File:       hello.c
//
// Abstract:   A simple "Hello World" compute example showing basic usage of OpenCL which
//             calculates the mathematical square (X[i] = pow(X[i],2)) for a buffer of
//             floating point values.
//             
//
// Version:    <1.0>
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//

////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>
#include "board.h"
#include "move.h"

////////////////////////////////////////////////////////////////////////////////

// Use a static data size for simplicity
//
#define DATA_SIZE (4)

////////////////////////////////////////////////////////////////////////////////

// Simple compute kernel which computes the square of an input array 
// Start Kernel
const char *KernelSource = "\n" \
"typedef unsigned long board_t;\n" \
"typedef unsigned long uint64_t;\n" \
"\n" \
"\n" \
"//uint64_t raw_column( board_t board, int col_num);\n" \
"//uint64_t raw_column( board_t board, int col_num)\n" \
"//{\n" \
"//    int offset = 16 * col_num;\n" \
"//    return (board >> offset) & 0xffff;\n" \
"//}\n" \
"//\n" \
"//\n" \
"//uint64_t heuristic(board_t board, __global int* empty_vals);\n" \
"//uint64_t heuristic(board_t board, __global int* empty_vals)\n" \
"//{\n" \
"//    uint64_t column = raw_column(board,0);\n" \
"//    uint64_t col_num_empty = empty_vals[column];\n" \
"//\n" \
"//    uint64_t column1 = raw_column(board,1);\n" \
"//    uint64_t col_num_empty1 = empty_vals[column1];\n" \
"//\n" \
"//    uint64_t column2 = raw_column(board,2);\n" \
"//    uint64_t col_num_empty2 = empty_vals[column2];\n" \
"//\n" \
"//    uint64_t column3 = raw_column(board,3);\n" \
"//    uint64_t col_num_empty3 = empty_vals[column3];\n" \
"//\n" \
"//    return 0;\n" \
"//}\n" \
"\n" \
"\n" \
"__kernel void calculate_heuristics (__global board_t* boards,\n" \
"__global int* empty_vals,\n" \
"__global uint64_t* output,\n" \
"const unsigned int count)\n" \
"{\n" \
"unsigned int i = get_global_id(0);\n" \
"if(i < count)\n" \
"{\n" \
"output[i] = i;\n" \
"//        output[i] = heuristic(boards[i], empty_vals);\n" \
"}\n" \
"}\n" \
"\n";
// End Kernel

static const int NUM_TRANSFORMS = 65536;

int empty_vals[NUM_TRANSFORMS];

uint64_t heuristic( uint64_t board)
{
    int64_t num_empty = 0;
    for(int64_t c = 0; c < 4; ++c) {
        int64_t column = board_raw_column(board,c);
        num_empty += empty_vals[column];
    }
    return num_empty;
    
}

void setup_empty_vals();
void setup_empty_vals()
{
    FILE *ptr;
    ptr = fopen("/Users/grantke/Desktop/Stuff/2048/OpenCL_Hello_World_Example/numempty.bin","rb");  // r for read, b for binary
    fread(empty_vals, sizeof(int), NUM_TRANSFORMS, ptr);

}

int main(int argc, char** argv)
{
    setup_empty_vals();
    setup_moves();

    int err;                            // error code returned from api calls
      
    board_t data[DATA_SIZE];              // original data set given to device
    uint64_t correctResults[DATA_SIZE];
    uint64_t results[DATA_SIZE];           // results returned from device
    unsigned int correct;               // number of correct results returned

    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation

    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel
    
    cl_mem input;                       // device memory used for the input array
    cl_mem numEmptyInput;
    cl_mem output;                      // device memory used for the output array
    
    // Fill our data set with random float values
    //

    board_t origBoard;
    board_set_value(&origBoard, 1, 1, 2);
    board_set_value(&origBoard, 1, 2, 2);

    struct Move_Result up_result = up_move(origBoard);
    struct Move_Result down_result = down_move(origBoard);
    struct Move_Result left_result = left_move(origBoard);
    struct Move_Result right_result = right_move(origBoard);

    data[0] = up_result.board;
    correctResults[0] = heuristic(up_result.board);
    data[1] = down_result.board;
    correctResults[1] = heuristic(down_result.board);
    data[2] = left_result.board;
    correctResults[2] = heuristic(left_result.board);
    data[3] = right_result.board;
    correctResults[3] = heuristic(right_result.board);


    const int count = 4;

    // Connect to a compute device
    //
    int gpu = 1;
    err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
    }
  
    // Create a compute context 
    //
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

    // Create a command commands
    //
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
    }

    // Create the compute program from the source buffer
    //
    program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }

    // Build the program executable
    //
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }

    // Create the compute kernel in the program we wish to run
    //
    kernel = clCreateKernel(program, "calculate_heuristics", &err);
    if (!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }

    // Create the input and output arrays in device memory for our calculation
    //
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(board_t) * count, NULL, NULL);
    numEmptyInput = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(empty_vals), NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(uint64_t) * count, NULL, NULL);
    if (!input || !numEmptyInput || !output)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }    
    
    // Write our data set into the input array in device memory 
    //
    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(board_t) * count, data, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    }

    // Write our data set into the input array in device memory
    //
    err = clEnqueueWriteBuffer(commands, numEmptyInput, CL_TRUE, 0, sizeof(int) * NUM_TRANSFORMS, empty_vals, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    }


    // Set the arguments to our compute kernel
    //
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &numEmptyInput);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &count);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }

    // Get the maximum work group size for executing the kernel on the device
    //
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }

    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    //
    global = count;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to execute kernel!\n");
        return EXIT_FAILURE;
    }

    // Wait for the command commands to get serviced before reading back results
    //
    clFinish(commands);

    // Read back the results from the device to verify the output
    //
    err = clEnqueueReadBuffer( commands, output, CL_TRUE, 0, sizeof(uint64_t) * count, results, 0, NULL, NULL );
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }
    
    // Validate our results
    //
    correct = 0;
    for(int i = 0; i < count; i++)
    {
        if(results[i] == correctResults[i])
            correct++;
    }
    
    // Print a brief summary detailing the results
    //
    printf("Computed '%d/%d' correct values!\n", correct, count);
    
    // Shutdown and cleanup
    //
    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}
