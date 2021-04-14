#define IHC_IMPL

#include "IHC.h"
#include <stdio.h>
#include <stdlib.h>


static MagickWand *wand;
static MagickWand *staining_wands[NUM_CHANNELS];


int ProcessImageFiles(const char *input_fname,
                      const char *color_deconvolution_matrix_fname,
                      const char **output_fnames)
{
    MagickBooleanType magick_status;
    double color_deconvolution_matrix[NUM_CHANNELS][NUM_CHANNELS];
    int status, i;

    /* Initialize the MagickWand environment. */
    MagickWandGenesis();
    wand = NewMagickWand();
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        staining_wands[i] = NewMagickWand();
    }

    /* Read an image. */
    magick_status = MagickReadImage(wand, input_fname);
    if (magick_status == MagickFalse)
    {
        ThrowWandException();
    }

    status = ReadColorDeconvolutionMatrix(color_deconvolution_matrix_fname,
                                          color_deconvolution_matrix);
    if (status == 1)
    {
        perror(color_deconvolution_matrix_fname);
        exit(1);
    }
    else if (status == 2)
    {
        fprintf(stderr, "%s: Failed to parse input\n",
                color_deconvolution_matrix_fname);
        exit(1);
    }

    double *staining_amounts[NUM_CHANNELS];
    SeparateStains(color_deconvolution_matrix, staining_amounts);

    size_t width = MagickGetImageWidth(wand);
    size_t height = MagickGetImageHeight(wand);
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        magick_status = MagickConstituteImage(staining_wands[i],
                                              width, height, "I", DoublePixel,
                                              staining_amounts[i]);
        if (magick_status == MagickFalse)
            ThrowWandException();

        /* Write the image and then destroy it. */
        magick_status = MagickWriteImage(staining_wands[i], output_fnames[i]);
        if (magick_status == MagickFalse)
            ThrowWandException();
        free(staining_amounts[i]);
        staining_wands[i] = DestroyMagickWand(staining_wands[i]);
    }


    /* Deallocate resources and destroy the MagickWand environment. */
    wand = DestroyMagickWand(wand);
    MagickWandTerminus();

    return 1;
}


int SeparateStains(double (*color_deconvolution_matrix)[NUM_CHANNELS],
                   double *staining_amounts[])
{
    size_t width = MagickGetImageWidth(wand);
    size_t height = MagickGetImageHeight(wand);
    size_t x, y;
    int i;

    PixelIterator *iterator = NewPixelIterator(wand);
    if (iterator == (PixelIterator *) NULL)
    {
        ThrowWandException();
    }

    for (i = 0; i < NUM_CHANNELS; i++)
    {
        staining_amounts[i] = (double *) calloc(width * height, sizeof(double));
    }

    for (y = 0; y < height; y++)
    {
        PixelWand **row_pixels = PixelGetNextIteratorRow(iterator, &width);
        if (row_pixels == (PixelWand **) NULL)
            break;
        for (x = 0; x < width; x++)
        {
            PixelInfo color;
            PixelGetMagickColor(row_pixels[x], &color);
            double pixel_staining_amounts[NUM_CHANNELS];
            ColorDeconvolve(color_deconvolution_matrix,
                            &color, pixel_staining_amounts);
            for (i = 0; i < NUM_CHANNELS; i++)
            {
                staining_amounts[i][x + y * width] = pixel_staining_amounts[i];
            }
        }
        PixelSyncIterator(iterator);
    }
    iterator = DestroyPixelIterator(iterator);

    return 1;
}


int ReadColorDeconvolutionMatrix(
        const char *filepath,
        double (*color_deconvolution_matrix)[NUM_CHANNELS])
{
    FILE *f = fopen(filepath, "r");
    if (f == NULL)
        return 1;

    for (int i = 0; i < NUM_CHANNELS; i++)
    {
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            int status = fscanf(f, "%lf", &color_deconvolution_matrix[i][j]);
            if (status != 1)
            {
                fclose(f);
                return 2;
            }
        }
    }

    fclose(f);
    return 0;
}


void ColorDeconvolve(double (*color_deconvolution_matrix)[NUM_CHANNELS],
                     const PixelInfo *color,
                     double *pixel_staining_amounts)
{
    /* This is basically matrix multiplication. */
    int j;
    for (j = 0; j < NUM_CHANNELS; j++)
    {
        pixel_staining_amounts[j]
            = color->red   * color_deconvolution_matrix[0][j]
            + color->green * color_deconvolution_matrix[1][j]
            + color->blue  * color_deconvolution_matrix[2][j];
        pixel_staining_amounts[j] /= QuantumRange; /* Clamp to [0, 1] */
    }
}


void ThrowWandException()
{
    ExceptionType severity;
    int i;
    char *description = MagickGetException(wand, &severity);

    fprintf(stderr, "%s %s %lu %s\n", GetMagickModule(), description);
    description = (char *) MagickRelinquishMemory(description);

    for (i = 0; i < NUM_CHANNELS; i++)
    {
        staining_wands[i] = DestroyMagickWand(staining_wands[i]);
    }
    wand = DestroyMagickWand(wand);
    MagickWandTerminus();
    exit(1);
}
