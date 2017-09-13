/*
 File: main.c
 Abstract:
 Demonstrates the use of pre-compiled bitcode with the OpenCL framework.

 This program creates an OpenCL command queue against a device of the
 user's choosing and then builds a CL program from a bitcode that the
 user specifies.

 To build the program and bitcode for 32/64bit CPUs and 32/64bit GPUs,
 type 'make'.

 Usage:
 ./test -t cpu32|cpu64|gpu32|gpu64 -i num -f kernel.bc

 For example, to execute against the 32bit GPU in your system:
 ./test -t gpu32 -i 0 -f kernel.gpu32.bc

 Or to test 32bit CPU bitcode:
 arch -i386 ./test -t cpu32 -f kernel.cpu32.bc

 Or 64bit CPU, presuming a 64bit machine:
 ./test -t cpu64 -f kernel.cpu64.bc

 The code below is divided into three sections.  The first, 'Bitcode
 loading and use,' will be of the most interest.  The other sections,
 'Typical OpenCL setup and teardown' and 'Supporting code,' are
 run-of-the-mill C argument processing and OpenCL setup.

 Version: 1.0

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 Copyright (C) 2011 Apple Inc. All Rights Reserved.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

#include <OpenCL/opencl.h>

#define MAXPATHLEN  512

// The number of uint64_t4s we will pass to our test kernel execution.
static size_t NELEMENTS =   1024;

// The various OpenCL objects needed to execute our CL program against a
// given compute device in our system.
int              device_index;
cl_device_type   device_type = CL_DEVICE_TYPE_GPU;
cl_device_id     device;
cl_context       context;
cl_command_queue queue;
cl_program       program;
cl_kernel        kernel;
bool             is32bit = false;
bool             kernel_initialied = false;

const int NUM_TRANSFORMS = 65536;

// A utility function to simplify error checking within this test code.
static void check_status(char* msg, cl_int err) {
  if (err != CL_SUCCESS) {
    fprintf(stderr, "%s failed. Error: %d\n", msg, err);
  }
}

static void shutdown_opencl();

#pragma mark -
#pragma mark Bitcode loading and use


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

  fprintf(stdout,"CL_DEVICE_MAX_PARAMETER_SIZE: %lld\n",max_buffer_size);

  fprintf(stdout, "Using OpenCL device: %s\n", name);

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
  printf("total input_sizes: %ld\n", total_sizes);

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

  fprintf(stderr, "About to clEnqueueNDRangeKernel.\n");
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL,
                               NULL);
  check_status("clEnqueueNDRangeKernel", err);

  clFinish(queue);
  // Read back the results (blocking, so everything finishes), and then
  // validate the results.

  clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, programOutputSize, programOutput,
                      0, NULL, NULL);
}

#pragma mark -
#pragma mark Typical OpenCL setup and teardown


static void shutdown_opencl() {

  // Free up all the CL objects we've allocated.

//  clReleaseMemObject(a);
//  clReleaseMemObject(b);
//  clReleaseMemObject(c);
//  clReleaseKernel(kernel);
//  clReleaseProgram(program);
//  clReleaseCommandQueue(queue);
//  clReleaseContext(context);
}

static size_t get_max_buffer_size()
{
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

  cl_ulong max_buffer_size;
  clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_buffer_size, NULL);
  fprintf(stdout,"CL_DEVICE_MAX_PARAMETER_SIZE: %lld\n",max_buffer_size);

  return max_buffer_size;
}


#pragma mark -
#pragma mark Supporting code

typedef int transform_t;

int main (int argc, char* const *argv)
{
  char *filepath = "./kernel.gpu64.bc";

//  NELEMENTS = get_max_buffer_size() / sizeof(uint64_t);
  NELEMENTS = 480;

  fprintf(stdout,"NELEMENTS = %zu\n",NELEMENTS);

  uint64_t *input_boards = (uint64_t*)malloc(sizeof(uint64_t)*NELEMENTS);
  transform_t left_transforms[NUM_TRANSFORMS];
  transform_t right_transforms[NUM_TRANSFORMS];
  uint64_t *output_buffer = (uint64_t*)malloc(sizeof(uint64_t)*4*NELEMENTS);

  // We pack some host buffers with our data.
  uint64_t i;

  for (i = 0; i < NELEMENTS; i++) {
    input_boards[i] = 0x300000000 + i;
    output_buffer[i] = 0;
  }

  for (i = 0; i < NUM_TRANSFORMS; ++i)
  {
    left_transforms[i] = i;
    right_transforms[i] = i;
  }

  void *input_buffers[] = {input_boards,               left_transforms,                    right_transforms,       NULL};
  size_t input_buffer_sizes[] =  {sizeof(uint64_t)*NELEMENTS, sizeof(transform_t)*NUM_TRANSFORMS, sizeof(transform_t)*NUM_TRANSFORMS};

    for (int i = 0; i < sizeof(input_buffer_sizes) / sizeof(size_t); ++i)
    {
        printf("size of input[%d] = %ld\n",i, input_buffer_sizes[i]);
    }
    printf("calling create_program_from_bitcode\n");


  // Obtain a CL program and kernel from our pre-compiled bitcode file and
  // test it by running the kernel on some test data.
  create_program_from_bitcode(filepath, "vecadd", input_buffers, input_buffer_sizes, output_buffer,sizeof(uint64_t)*4*NELEMENTS, NELEMENTS);


  int success = 1;
  for (i = 0; i < NELEMENTS; i++) {
    for (int j = 4 * i; j < 4 * i + 4; ++j){
      if (output_buffer[j] != input_boards[i] + left_transforms[i % NUM_TRANSFORMS] + right_transforms[i % NUM_TRANSFORMS])
      {
        success = 0;
        fprintf(stderr, "Validation failed at index %llu\n", i);
        fprintf(stderr, "Kernel FAILED!\n");
        break;
      }
    }
  }

  if (success) {
    fprintf(stdout, "Validation successful.\n");
  }


  return 0;
}
