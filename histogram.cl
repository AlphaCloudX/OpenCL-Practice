__kernel void rgb_histogram(__global const uchar* image,
                            __global uint* histogram,
                            const uint num_pixels) {
    int i = get_global_id(0);
    if (i < num_pixels) {
        int r = image[3 * i];
        int g = image[3 * i + 1];
        int b = image[3 * i + 2];
        atomic_inc(&histogram[r]);
        atomic_inc(&histogram[256 + g]);
        atomic_inc(&histogram[512 + b]);
    }
}
