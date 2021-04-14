#include <stdio.h>
#include <errno.h>
#include "IHC.h"

int main(int argc, char **argv)
{
#if 1
    if (argc != 6)
    {
        fprintf(stderr,
                "Usage: %s image color_deconvolution_matrix_file "
                "stain_1_output stain_2_output stain_3_output\n", argv[0]);
        exit(1);
    }
#endif

    ProcessImageFiles(argv[1] /* image */,
                      argv[2] /* color deconvolution matrix filename */,
                      (const char **) &argv[3] /* output files */);
    return 0;
}
