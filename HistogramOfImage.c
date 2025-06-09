#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>


// Locate the device we are using (First GPU)
void locateDevices();

// get the max work item size we are using
size_t getMaxWorkItemSize(cl_device_id *device, size_t perThreadLocalMemory);

// Load the image, completely random image for now
unsigned char *readImage(int width, int height);

// read kernel code from a .cl file instead of having strings everywhere
char *readKernelSource(const char *filename);

/**
 * This program compute the histogram for an image
 *
 * In this case I use a random image generator that uses random pixel values
 *
 * This uses a 1D kernel size where each worker item = 1 pixel
 * However using a 256 x 256 kernel size allows us to run more operations in parallel so our for loops are shorter
 * This means we can still iterate on this code and improve it to be faster
 *
 */
int main(void) {
    int width = 1024, height = 1024;
    cl_uint num_pixels = width * height;

    // Load the kernel from the .cl file
    char *kernel_source_str = readKernelSource("histogram.cl");

    // First lets compute how many workers we have
    cl_device_id *device = malloc(sizeof(cl_device_id) * 1);


    locateDevices(device);

    size_t perThreadLocalMemory = 256;

    size_t maxWorkItemSize = getMaxWorkItemSize(device, perThreadLocalMemory);
    printf("Kernel size: %zu x %zu \n", perThreadLocalMemory, maxWorkItemSize);

    unsigned char *image = readImage(width, height);

    // Create buffers for the input image and the output bins
    // we have rgb so 256 bins * 3 channels = 768 bytes
    cl_int err;

    // Create context and queue so we can send data over to the gpu
    cl_context context = clCreateContext(NULL, 1, device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueue(context, *device, 0, &err);

    // buffer for the input image
    // only needs to read from the buffer
    // cl mem copy host ptr means copy the image into the buffer at creation so we do a single step
    cl_mem imageBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                        width * height * 3, image, &err);

    // buffer for the histogram
    // we would use read write only when we pass in an existing histogram, otherwise we are only writing to the buffer
    // because we use atomic_incr we are never really reading any data so we can optimize our code here to write only mode
    cl_mem histBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                       sizeof(cl_uint) * 256 * 3, NULL, &err);

    // this now loads the kernel code from the string
    cl_program program = clCreateProgramWithSource(context, 1, &kernel_source_str, NULL, &err);
    clBuildProgram(program, 1, device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "rgb_histogram", &err);

    // now we are setting the arguments for our rgb histogram function
    // binds each individual argument index from our kernel function to host variables we have
    // __global const uchar* image, __global uint* histogram, const uint num_pixels
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &imageBuffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &histBuffer);
    clSetKernelArg(kernel, 2, sizeof(cl_uint), &num_pixels);

    // This now runs the kernel
    // global work size = total pixels because each work item operates on a single pixel value
    size_t global_work_size = num_pixels;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);

    // Load data from gpu into the cpu
    unsigned int histogram[256 * 3];
    clEnqueueReadBuffer(queue, histBuffer, CL_TRUE, 0, sizeof(histogram), histogram, 0, NULL, NULL);

    bool printOutput = false;

    if (printOutput) {
        printf("Red channel histogram:\n");
        for (int i = 0; i < 256; i++) {
            printf("%3d: %u\n", i, histogram[i]);
        }

        printf("\nGreen channel histogram:\n");
        for (int i = 0; i < 256; i++) {
            printf("%3d: %u\n", i, histogram[256 + i]);
        }

        printf("\nBlue channel histogram:\n");
        for (int i = 0; i < 256; i++) {
            printf("%3d: %u\n", i, histogram[512 + i]);
        }
    }


    free(image);
    free(device);

    return 1;
}


void locateDevices(cl_device_id *devices) {
    cl_uint num_platforms;

    // cl_uint clERror can also be used to catch any errors when getting platform information

    // First we need to get the number of available platforms
    clGetPlatformIDs(0, NULL, &num_platforms);

    // Next we make a list for all the platform id's, this is done with malloc to alloc a platform id for number of platforms
    cl_platform_id *platforms = (cl_platform_id *) malloc(num_platforms * sizeof(cl_platform_id));

    // Get platform IDs
    // This time we don't pass NULL but pass an array to store all the platform id's
    // num_platforms is how many entries our array has
    clGetPlatformIDs(num_platforms, platforms, NULL);


    // This will now handle finding all the devices for the first platform
    const int platform_number = 0;
    // 1 device for now
    cl_uint num_devices = 1;

    // now we need to store a list of devices for the first platform
    clGetDeviceIDs(platforms[platform_number], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
}

size_t getMaxWorkItemSize(cl_device_id *device, size_t perThreadLocalMemory) {
    size_t maxWorkGroupSize;
    clGetDeviceInfo(*device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL);
    // printf("Max Work Group Size: %zu\n", maxWorkGroupSize);

    // Total local memory per compute unit
    size_t localMemSize;
    clGetDeviceInfo(*device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(size_t), &localMemSize, NULL);
    // printf("Local Memory Size: %zu bytes\n", localMemSize);

    // Compute the number of work-items we can run based on local memory usage per thread
    size_t maxWorkItemsBasedOnLocalMem = localMemSize / perThreadLocalMemory;
    // printf("Max threads (work-items) per group based on local memory: %zu\n", maxWorkItemsBasedOnLocalMem);

    // Determine the final allowed work-group size (limited by both local mem and max group size)
    size_t allowedWorkGroupSize = (maxWorkGroupSize < maxWorkItemsBasedOnLocalMem)
                                      ? maxWorkGroupSize
                                      : maxWorkItemsBasedOnLocalMem;
    // printf("Allowed Work Group Size (min of both limits): %zu\n", allowedWorkGroupSize);

    return allowedWorkGroupSize;
}

unsigned char *readImage(int width, int height) {
    unsigned char *img = malloc(width * height * 3);

    for (int i = 0; i < width * height * 3; ++i) {
        img[i] = rand() % 256;
    }

    return img;
}

char *readKernelSource(const char *filename) {
    FILE *file = fopen(filename, "r");

    // Seek to end to get size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *source_str = (char *) malloc(length + 1);

    size_t read_len = fread(source_str, 1, length, file);
    source_str[read_len] = '\0'; // Null terminate

    fclose(file);
    return source_str;
}
