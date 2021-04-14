/* Immunohistochemistry (IHC)
 *
 * Separation and quantification of immunohistochemical staining by means of
 * color deconvolution.
 *
 */

#ifndef IHC_H
#define IHC_H

#include <MagickWand/MagickWand.h>

#define NUM_CHANNELS 3

int ProcessImageFiles(const char *input_fname,
                      const char *color_deconvolution_matrix_fname,
                      const char **output_fnames);


#ifdef IHC_IMPL
int SeparateStains(double (*color_deconvolution_matrix)[NUM_CHANNELS],
                   double *staining_amounts[]);

int ReadColorDeconvolutionMatrix(
        const char *filepath,
        double (*color_deconvolution_matrix)[NUM_CHANNELS]);

void ColorDeconvolve(double (*color_deconvolution_matrix)[NUM_CHANNELS],
                     const PixelInfo *color,
                     double *pixel_staining_amounts);

void ThrowWandException();
#endif

#endif /* ifndef IHC_H */
