//
//  new_main.c
//  scan
//
//  Created by Grant Kennell on 8/24/17.
//


#include <libc.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach_time.h>
#include <math.h>

#include <OpenCL/opencl.h>

#define DATA_SIZE (4)

static char *
LoadProgramSourceFromFile(const char *filename)
{
    struct stat statbuf;
    FILE        *fh;
    char        *source;

    fh = fopen(filename, "r");
    if (fh == 0)
        return 0;

    stat(filename, &statbuf);
    source = (char *) malloc(statbuf.st_size + 1);
    fread(source, statbuf.st_size, 1, fh);
    source[statbuf.st_size] = '\0';

    return source;
}


int main(int argc, char** argv)
{
//    setup_empty_vals();
//    setup_moves();

    int err;                            // error code returned from api calls

    uint64_t input_data[DATA_SIZE];
    uint64_t correct_doubled_data[DATA_SIZE];
    for (int i = 0; i < DATA_SIZE; i++)
    {
        input_data[i] = (uint64_t)(10 * ((float) rand() / (float) RAND_MAX));
        correct_doubled_data[i] = input_data[i] * 2;
    }

//    board_t data[DATA_SIZE];              // original data set given to device
//    uint64_t correctResults[DATA_SIZE];
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

//    board_t origBoard;
//    board_set_value(&origBoard, 1, 1, 2);
//    board_set_value(&origBoard, 1, 2, 2);
//
//    struct Move_Result up_result = up_move(origBoard);
//    struct Move_Result down_result = down_move(origBoard);
//    struct Move_Result left_result = left_move(origBoard);
//    struct Move_Result right_result = right_move(origBoard);
//
//    data[0] = up_result.board;
//    correctResults[0] = heuristic(up_result.board);
//    data[1] = down_result.board;
//    correctResults[1] = heuristic(down_result.board);
//    data[2] = left_result.board;
//    correctResults[2] = heuristic(left_result.board);
//    data[3] = right_result.board;
//    correctResults[3] = heuristic(right_result.board);
//

    const int count = DATA_SIZE;

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

    const char* filename = "./heuristic_opencl.cl";
    printf("Loading program '%s'...\n", filename);

    char *source = LoadProgramSourceFromFile(filename);
    if(!source)
    {
        printf("Error: Failed to load compute program from file!\n");
        return EXIT_FAILURE;
    }

    // Create the compute program from the source buffer
    //
    program = clCreateProgramWithSource(context, 1, (const char **) & source, NULL, &err);
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
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(uint64_t) * count, NULL, NULL);
//    numEmptyInput = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(empty_vals), NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(uint64_t) * count, NULL, NULL);
    if (!input || !output)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    // Write our data set into the input array in device memory
    //
    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(uint64_t) * count, input_data, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    }

    // Write our data set into the input array in device memory
    //
//    err = clEnqueueWriteBuffer(commands, numEmptyInput, CL_TRUE, 0, sizeof(int) * NUM_TRANSFORMS, empty_vals, 0, NULL, NULL);
//    if (err != CL_SUCCESS)
//    {
//        printf("Error: Failed to write to source array!\n");
//        exit(1);
//    }


    // Set the arguments to our compute kernel
    //
    err = 0;
    int argNum = 0;
    err  = clSetKernelArg(kernel, argNum++, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, argNum++, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, argNum++, sizeof(unsigned int), &count);
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
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
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
        if(results[i] == correct_doubled_data[i])
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
