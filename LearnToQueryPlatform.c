#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>


/**
 *  This method queries for basic platform information like the name, vendor, opencl version
 *
 *  This code will make use of:
 *
 *  cl_int clGetPlatformIDs (cl_uint num_entries,  cl_platform_id *platforms,  cl_uint *num_platforms);
 *
 *  and
 *
 *  cl_int clGetPlatformInfo(cl_platform_id platform,  cl_platform_info param_name,  size_t param_value_size,
 *
 * @param num_platforms cl_uint for number of platforms we detected
 * @param platforms pointer to list of platform id's to query
 */
void printPlatformInformation(cl_uint num_platforms, cl_platform_id *platforms) {
    // Print platform names
    for (int i = 0; i < num_platforms; ++i) {
        char name[256];
        char vendor[256];
        char version[256];
        char profile[256];
        char extension[256];

        cl_int clError = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(name), name, NULL);

        if (clError == CL_SUCCESS) {
            printf("Platform %u: %s\n", i, name);
        } else {
            printf("Failed to get platform name for platform %u: %d\n", i, clError);
        }

        clError = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);

        if (clError == CL_SUCCESS) {
            printf("Vendor %u: %s\n", i, vendor);
        } else {
            printf("Failed to get platform name for platform %u: %d\n", i, clError);
        }


        clError = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(version), version, NULL);

        if (clError == CL_SUCCESS) {
            printf("Version %u: %s\n", i, version);
        } else {
            printf("Failed to get platform version for platform %u: %d\n", i, clError);
        }

        clError = clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, sizeof(profile), profile, NULL);

        if (clError == CL_SUCCESS) {
            printf("Profile %u: %s\n", i, profile);
        } else {
            printf("Failed to get platform profile for platform %u: %d\n", i, clError);
        }

        clError = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, sizeof(extension), extension, NULL);

        if (clError == CL_SUCCESS) {
            printf("Extensions %u: %s\n", i, extension);
        } else {
            printf("Failed to get platform extensions for platform %u: %d\n", i, clError);
        }
    }
}

/**
 * This code loops through a list of devices for a partiuclar platform and outputs information.
 *
 * This code makes use of:
 *
 * cl_int clGetDeviceIDs (cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices);
 *
 * and
 *
 *  cl_int clGetDeviceInfo (cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
 *
 * @param num_devices cl_uint for number of platforms we detected
 * @param devices a list of devices for a particular platform to output
 */
void printDeviceInformation(cl_uint num_devices, cl_device_id *devices) {
    for (cl_uint i = 0; i < num_devices; i++) {
        printf("\n--- Device %u on Platform ---\n", i);
        char queryBuffer[1024];
        int queryInt;
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(queryBuffer), &queryBuffer, NULL);

        printf("CL_DEVICE_NAME: %s\n", queryBuffer);
        queryBuffer[0] = '\0';

        clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(queryBuffer), &queryBuffer,NULL);

        printf("CL_DEVICE_VENDOR: %s\n", queryBuffer);
        queryBuffer[0] = '\0';

        clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(queryBuffer), &queryBuffer,NULL);

        printf("CL_DRIVER_VERSION: %s\n", queryBuffer);
        queryBuffer[0] = '\0';

        clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(queryBuffer), &queryBuffer,NULL);

        printf("CL_DEVICE_VERSION: %s\n", queryBuffer);
        queryBuffer[0] = '\0';

        clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(int), &queryInt, NULL);
        printf("CL_DEVICE_MAX_COMPUTE_UNITS: %d\n", queryInt);

        clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(int), &queryInt, NULL);
        printf("CL_DEVICE_TYPE: %d\n", queryInt);

        clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(int), &queryInt, NULL);
        printf("CL_DEVICE_MAX_CLOCK_FREQUENCY: %d\n", queryInt);

        clGetDeviceInfo(devices[i], CL_DEVICE_IMAGE_SUPPORT, sizeof(int), &queryInt, NULL);
        printf("CL_DEVICE_IMAGE_SUPPORT: %d\n", queryInt);

        clGetDeviceInfo(devices[i], CL_DEVICE_SINGLE_FP_CONFIG, sizeof(queryInt), &queryInt, NULL);
        printf("CL_DEVICE_SINGLE_FP_CONFIG: %d\n", queryInt);
    }
}

/**
 * The purpose of this code is to learn and take note of how to query an
 * OpenCL to get platform information
 *
 * @return 1
 */
int main(void) {
    // This stores how many platforms we have
    cl_uint num_platforms;

    // cl_uint clERror can also be used to catch any errors when getting platform information

    // First we need to get the number of available platforms
    clGetPlatformIDs(0, NULL, &num_platforms);

    printf("Number of OpenCL platforms: %u\n", num_platforms);

    // Next we make a list for all the platform id's, this is done with malloc to alloc a platform id for number of platforms
    cl_platform_id *platforms = (cl_platform_id *) malloc(num_platforms * sizeof(cl_platform_id));

    // Get platform IDs
    // This time we don't pass NULL but pass an array to store all the platform id's
    // num_platforms is how many entries our array has
    clGetPlatformIDs(num_platforms, platforms, NULL);

    // By now we have a list of platform id's

    // This method handles all the platform printing info
    printPlatformInformation(num_platforms, platforms);


    // This will now handle finding all the devices for the first platform
    const int platform_number = 0;
    cl_uint num_devices;

    // Find out how many devices we have so we can alloc the right sized array
    clGetDeviceIDs(platforms[platform_number], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

    printf("Number of OpenCL devices: %u\n", num_devices);

    // now we need to store a list of devices for the first platform
    cl_device_id *devices = (cl_device_id *) malloc(num_devices * sizeof(cl_device_id));
    clGetDeviceIDs(platforms[platform_number], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

    printDeviceInformation(num_devices, devices);

    free(devices);

    free(platforms);
    return 0;
}
