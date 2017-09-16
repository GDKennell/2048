#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

#include <OpenCL/opencl.h>

#include "compute_moves.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// The various OpenCL objects needed to execute our CL program against a
// given compute device in our system.
static int              device_index;
static cl_device_type   device_type = CL_DEVICE_TYPE_GPU;
static cl_device_id     device;
static cl_context       context;
static cl_command_queue queue;
static cl_program       program;
static cl_kernel        kernel;
static bool             is32bit = false;
static bool             kernel_initialied = false;

// A utility function to simplify error checking within this test code.
static void check_status(char* msg, cl_int err) {
    if (err != CL_SUCCESS) {
        fprintf(stderr, "%s: %s failed. Error: %d\n",__FILE__, msg, err);
        exit(1);
    }
}

#pragma mark -
#pragma mark Bitcode loading and use

//static size_t device_max_buffer_size

static void init_kernel(char* bitcode_path,char* function_name)
{
    if (kernel_initialied)
    {
        return;
    }

    // Perform typical OpenCL setup in order to obtain a context and command
    // queue.
    cl_int err;
    cl_uint num_devices;

    // How many devices of the type requested are in the system?
    clGetDeviceIDs(NULL, device_type, 0, NULL, &num_devices);

    // Make sure the requested index is within bounds.  Otherwise, correct it.
    if (device_index < 0 || device_index > num_devices - 1) {
        fprintf(stdout, "Requsted index (%d) is out of range.  Using 0.\n",
                device_index);
        device_index = 0;
    }

    // Grab the requested device.
    cl_device_id all_devices[num_devices];
    clGetDeviceIDs(NULL, device_type, num_devices, all_devices, NULL);
    device = all_devices[device_index];

    // Dump the device.
    char name[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, 128*sizeof(char), name, NULL);

    cl_ulong max_buffer_size;
    clGetDeviceInfo(device, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(cl_ulong), &max_buffer_size, NULL);

//    fprintf(stdout,"CL_DEVICE_MAX_PARAMETER_SIZE: %lld\n",max_buffer_size);

//    fprintf(stdout, "Using OpenCL device: %s\n", name);

    // Create an OpenCL context using this compute device.
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    check_status("clCreateContext", err);

    // Create a command queue on this device, since we want to use if for
    // running our CL program.
    queue = clCreateCommandQueue(context, device, 0, &err);
    check_status("clCreateCommandQueue", err);

    // Instead of passing actual executable bits, we pass a path to the
    // already-compiled bitcode to clCreateProgramWithBinary.  Note that
    // you may load bitcode for multiple devices in one call by passing
    // multiple paths and multiple devices.  In the multiple-device case,
    // the indices should match: if device 0 is a 32-bit GPU, then path 0
    // should be bitcode for a GPU.  In the example below, we are loading
    // bitcode for one device only.

    size_t len = strlen(bitcode_path);
    program = clCreateProgramWithBinary(context, 1, &device, &len,
                                        (const unsigned char**)&bitcode_path, NULL, &err);
    check_status("clCreateProgramWithBinary", err);

    // The above tells OpenCL how to locate the intermediate bitcode, but we
    // still must build the program to produce executable bits for our
    // *specific* device.  This transforms gpu32 bitcode into actual executable
    // bits for an AMD or Intel compute device (for example).

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    check_status("clBuildProgram", err);

    // And that's it -- we have a fully-compiled program created from the
    // bitcode.  Let's ask OpenCL for the test kernel.

    kernel = clCreateKernel(program, function_name, &err);
    check_status("clCreateKernel", err);

    kernel_initialied = true;
}

static void create_program_from_bitcode(char* bitcode_path,
                                        char* function_name,
                                        void **programInputs,
                                        size_t *programInputSizes,
                                        void *programOutput,
                                        size_t programOutputSize,
                                        size_t count)
{
    cl_int err;

    init_kernel(bitcode_path,function_name);
    // And now, let's test the kernel with some dummy data.

    // And create and load some CL memory buffers with that host data.
    const int MAX_BUFFERS = 10;
    cl_mem input_buffers[MAX_BUFFERS];

    int num_buffers = 0;
    while(programInputs[num_buffers] != NULL)
    {
        input_buffers[num_buffers] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                    programInputSizes[num_buffers], programInputs[num_buffers], &err);
        if (input_buffers[num_buffers] == NULL)
        {
            fprintf(stderr, "Error: Unable to create OpenCL buffer memory objects %d.\n", num_buffers);
            fprintf(stderr, "Using input %p of size %ld \n", programInputs[num_buffers], programInputSizes[num_buffers]);
            exit(1);
        }
        ++num_buffers;

    }

    size_t total_sizes = 0;
    for (int i = 0; i < num_buffers; ++i)
    {
        total_sizes += programInputSizes[i];
    }
//    printf("total input_sizes: %ld\n", total_sizes);

    // CL buffer 'c' is for output, so we don't prepopulate it with data.

    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         programOutputSize, NULL, &err);

    if (outputBuffer == NULL) {
        fprintf(stderr, "Error: Unable to create OpenCL buffer memory objects.\n");
        exit(1);
    }

    int argc = 0;
    for (int i = 0; i < num_buffers; ++i){
        err |= clSetKernelArg(kernel, argc++, sizeof(cl_mem), &input_buffers[i]);
    }
    err |= clSetKernelArg(kernel, argc++, sizeof(cl_mem), &outputBuffer);
    err |= clSetKernelArg(kernel, argc++, sizeof(size_t), &count);
    check_status("clSetKernelArg", err);

    // Launch the kernel over a single dimension, which is the same size
    // as the number of float4s.  We let OpenCL select the local dimensions
    // by passing 'NULL' as the 6th parameter.

    size_t global = count;

//    fprintf(stderr, "About to clEnqueueNDRangeKernel.\n");
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL,
                                 NULL);
    check_status("clEnqueueNDRangeKernel", err);

    clFinish(queue);
    // Read back the results (blocking, so everything finishes), and then
    // validate the results.
    
    clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, programOutputSize, programOutput,
                        0, NULL, NULL);
    for (int i = 0; i < num_buffers; ++i)
    {
        clReleaseMemObject(input_buffers[i]);
    }
    clReleaseMemObject(outputBuffer);

}

static uint64_t *outputBuffer = NULL;
static void init_output_buffer(size_t size)
{
    if(outputBuffer == NULL)
    {
        outputBuffer = malloc(size * sizeof(uint64_t));
    }
}

static uint64_t start_of_layer(int layer_num)
{
    uint64_t layerStart = 0;
    uint64_t layerSize = 4;
    for (int i = 1; i <= layer_num; ++i)
    {
        layerStart += layerSize;
        uint64_t layerMultiplier = (i % 2 == 0) ? 4 : 30;
        layerSize *= layerMultiplier;
    }
    return layerStart;
}

static uint64_t size_of_layer(int layer_num)
{
    uint64_t layerSize = 4;
    for (int i = 1; i <= layer_num; ++i)
    {
        uint64_t layerMultiplier = (i % 2 == 0) ? 4 : 30;
        layerSize *= layerMultiplier;
    }
    return layerSize;
}


static size_t get_max_buffer_size()
{
    static cl_ulong max_buffer_size = 0;
    if (max_buffer_size != 0)
    {
        return max_buffer_size;
    }
    // Perform typical OpenCL setup in order to obtain a context and command
    // queue.
    cl_uint num_devices;

    // How many devices of the type requested are in the system?
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);

    // Make sure the requested index is within bounds.  Otherwise, correct it.
    if (device_index < 0 || device_index > num_devices - 1) {
        fprintf(stdout, "Requsted index (%d) is out of range.  Using 0.\n",
                device_index);
        device_index = 0;
    }

    // Grab the requested device.
    cl_device_id all_devices[num_devices];
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, num_devices, all_devices, NULL);
    device = all_devices[device_index];

    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_buffer_size, NULL);
//    fprintf(stdout,"CL_DEVICE_MAX_PARAMETER_SIZE: %lld\n",max_buffer_size);

    return max_buffer_size;
}

void compute_moves(uint64_t *allBoards , unsigned int layer_num, transform_t left_move_transforms[NUM_TRANSFORMS], transform_t right_move_transforms[NUM_TRANSFORMS])
{
//    printf("computing moves for layer %u\n", layer_num);
    size_t max_buffer_size = get_max_buffer_size() / (2 * sizeof(uint64_t));
    init_output_buffer(max_buffer_size);

    size_t MAX_BLOCK_SIZE = 500000;

    char *filepath = "/Users/grantke/Desktop/Stuff/2048/compute_moves.gpu64.bc";
    char *function_name = "compute_moves";


    uint64_t prev_layer_start = start_of_layer(layer_num - 1);
    uint64_t prev_layer_end = prev_layer_start + size_of_layer(layer_num - 1);
    uint64_t layer_start = start_of_layer(layer_num);
    uint64_t layer_end = layer_start + size_of_layer(layer_num);



    for (uint64_t orig_start_index = prev_layer_start;
         orig_start_index < prev_layer_end;
         orig_start_index += MAX_BLOCK_SIZE)
    {
        size_t this_block_size = min(prev_layer_end - orig_start_index, MAX_BLOCK_SIZE);
//        printf("\tcomputing block %llu:%llu\n\n",orig_start_index,orig_start_index + this_block_size);
        void *input_buffers[] =         {allBoards + orig_start_index,       left_move_transforms,                 right_move_transforms, NULL};
        size_t input_buffer_sizes[] =   {this_block_size * sizeof(uint64_t), NUM_TRANSFORMS * sizeof(transform_t), NUM_TRANSFORMS * sizeof(transform_t)};

        for (int i = 0; i < sizeof(input_buffer_sizes) / sizeof(size_t); ++i)
        {
//            printf("\t\tsize of input[%d] = %ld\n",i, input_buffer_sizes[i]);
        }
//        printf("calling create_program_from_bitcode\n");
        size_t outputBufferSize = 4 * this_block_size * sizeof(uint64_t);

        create_program_from_bitcode(filepath, function_name, input_buffers, input_buffer_sizes, outputBuffer, outputBufferSize, this_block_size);
//        printf("create_program_from_bitcode returned \n");

        uint64_t result_layer_offset = layer_start + 4 * (orig_start_index - prev_layer_start);
        for (uint64_t i = 0; i < 4 * this_block_size; ++i)
        {
            allBoards[result_layer_offset + i] = outputBuffer[i];
        }
//        printf("finished writing allboards output \n");
    }
//    printf("\tFinished computing moves for layer %llu\n\n", layer_num);
}
















