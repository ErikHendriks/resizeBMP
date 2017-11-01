/**
 * cs50 rezise.
 */

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }
    
    // get n
    int n = atoi(argv[1]);
    
    // limit n, max 100
    if (n < 1 || n > 100)
    {
        printf("n must be between 1 & 100\n");
        return 2;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf, obf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi, obi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    
    // Get Old Padding
    int oldPad = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    obf = bf;
    obi = bi;

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (obf.bfType != 0x4d42 || obf.bfOffBits != 54 || obi.biSize != 40 || 
        obi.biBitCount != 24 || obi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // Update width & height in info header
    obi.biWidth *= n;
    obi.biHeight *= n;

    // Get new padding
    int newPad = (4 - (obi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Update size in info header
    obi.biSizeImage = obi.biWidth * abs(obi.biHeight) * sizeof(RGBTRIPLE) + newPad * abs(obi.biHeight);

    // Update size in file header
    obf.bfSize = obf.bfOffBits + obi.biSizeImage;

    // write outfile's BITMAPFILEHEADER
    fwrite(&obf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&obi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // Array of pixels
        RGBTRIPLE row[obi.biWidth];
        int pix = 0;

        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            // Repeat pixels n times
            for (int k =0; k < n; k++, pix++)
            {
                row[pix] = triple;
            }
        }

        // Repeat rows n times
        for (int j =0; j < n; j++)
        {
            // Write row to file
            fwrite(&row, sizeof(row), 1, outptr);

            // Add padding
            for (int k = 0; k < newPad; k++)
            {
                fputc(0x00, outptr);
            }
        }
        // skip over padding, if any
        fseek(inptr, oldPad, SEEK_CUR);

    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
