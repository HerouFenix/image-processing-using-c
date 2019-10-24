#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/********************************************/ /**
 *  STRUCTURE DECLARATIONS
 ***********************************************/
/// Structure used to represent RGB colours (pixels)
/// Unsigned Char since we only need to specify values from 0 to 255, which can be done with 1 byte (which corresponds to the size of an unsigned char)
typedef struct
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
} Colour;

/// Structure used to represent an RGB image
/**
 *  The way we chose to represent our pixels was through the use of an array of chars (1 byte structures). 
 *  Instead of thinking of the image as a bidimensional array, we simply used a linear array noting that the image position can be given by the formula:
 *  position = line*noOfColumns+column
*/
typedef struct
{
    int width, height;
    Colour *pixel_array;
} RGBImage;

/********************************************/ /**
 *  FUNCTION DECLARATIONS
 ***********************************************/
/********************************************/ /**
 * Function used to save an RGB image to a file
 *
 * @param file_name The File's path on which we'll be saving the image
 * @param image The RGB Image we want to save
 ***********************************************/
int save_to_file(char *file_name, RGBImage *image);

/********************************************/ /**
 * Function used to load an RGB image from a file
 *
 * @param file_name The File's path on which we'll be saving the image
 ***********************************************/
RGBImage *load_file(char *file_name);

/********************************************/ /**
 * Function used to access a specific pixel within an RGB image
 *
 * @param image The RGB Image we want to save
 * @param line The pixel's line (y position)
 * @param col The pixel's col (x position)e
 ***********************************************/
Colour get_pixel(RGBImage *image, int line, int col);

/********************************************/ /**
 * Function used to acess a subsection of image
 *
 * @param image The Grayscale image we want to subsect
 * @param pos_start An array containing the subsection's starting starting (left-top corner of subsection) row number and col number
 * @param pos_end An array containing the subsection's ending (bottom-right corner of subsection) row number and col number
 ***********************************************/
RGBImage get_subsection(RGBImage *image, int pos_start[2], int pos_end[2]);

/********************************************/ /**
 *  FUNCTION DEFINITION
 ***********************************************/
int save_to_file(char *file_name, RGBImage *image)
{
    FILE *fp;
    int width, height, colour_range;
    char *img_type;

    fp = fopen(file_name, "wb"); //Open File
    if (!fp)
    {
        fprintf(stderr, "ERROR: An error occurred finding the file '%s'\n", file_name);
        return 1;
    }

    img_type = "P6";
    height = image->height, width = image->width, colour_range = 255;

    //Write the PPM's headers
    fprintf(fp, "%s\n", img_type);
    fprintf(fp, "%d %d\n", width, height);
    fprintf(fp, "%d\n", colour_range);

    //Write the image's pixels
    fwrite(image->pixel_array, 3 * width, height, fp);

    fclose(fp);
    return 0;
};

//TO DO: VERIFY IF COMMENTS CAN BE PLACED ON ANY PLACE IN THE DOCUMENT OR WHATS THE DEAL WITH THAT!
RGBImage *load_file(char *file_name)
{
    char image_type[8];
    int colour_range, comments;
    FILE *fp;
    RGBImage *image;

    fp = fopen(file_name, "rb"); //Open File
    if (!fp)
    {
        fprintf(stderr, "ERROR: An error occurred finding the file '%s'\n", file_name);
        exit(1);
    }

    if ((!fgets(image_type, sizeof(image_type), fp)) && (image_type[0] != 'P' && image_type[1] != '6'))
    { //Get PPM Type (Goes from P0-P6)
        fprintf(stderr, "ERROR: The PPM image file seems to be malformed\n");
        exit(1);
    }

    image = (RGBImage *)malloc(sizeof(RGBImage)); //Allocate memory for our image struct

    if (fscanf(fp, "%d %d", &image->width, &image->height) != 2)
    { //Get Size (WIDTHxHEIGHT)
        fprintf(stderr, "ERROR: The PPM image seems to have an invalid image size (Should be Width x Height)\n");
        exit(1);
    }

    if ((fscanf(fp, "%d", &colour_range) != 1) && (colour_range != 255))
    { //Get colour range (should be 255)
        fprintf(stderr, "ERROR: The PPM image seems to have an invalid rgb range - Should be 255\n");
        exit(1);
    }

    while ((comments = getc(fp)) && (comments == '#')) //Remove comments (lines starting in #)
    {
        while (getc(fp) != '\n') //Get the entire line char by char until we find a breakline
            ;
    }
    ungetc(comments, fp); //If we break out of the last loop it's cus the current char isn't in a line starting with #, so we should unget it

    while (fgetc(fp) != '\n')
        ; //Remove blank spaces

    image->pixel_array = (Colour *)malloc(sizeof(Colour) * image->width * image->height); //Allocate memory for image's pixels

    if (fread(image->pixel_array, 3 * image->width, image->height, fp) != image->height)
    { //Read all remaining data from the image (we multiply the width by 3 because each pixel holds 3 chars - r,g,b)
        fprintf(stderr, "ERROR: At least one line couldn't be read from the image\n");
        exit(1);
    }

    fclose(fp);
    return image;
};

Colour get_pixel(RGBImage *image, int row, int col)
{
    int index = row * image->width + col; //The index of the pixel is given by the formula: pixel_line * no_columns + pixel_column
    return image->pixel_array[index];
}

RGBImage *get_subsect(RGBImage *image, int pos_start[2], int pos_end[2])
{
    //Verify that pos_start isn't infront of pos_end
    if(pos_start[0] > pos_end[0] || pos_start[1] > pos_end[1]){
        fprintf(stderr, "ERROR: The starting position can't come after the ending position!\n");
        exit(1);
    }

    int pixels_per_row, rows_travelled, subsection_offset, current_index_image;

    RGBImage *subsect = (RGBImage *)malloc(sizeof(RGBImage)); //Allocate memory for our image struct

    subsect->width = pos_end[1] - pos_start[1] + 1;
    subsect->height = pos_end[0] - pos_start[0] + 1;

    subsect->pixel_array = (Colour *)malloc(sizeof(Colour) * subsect->width * subsect->height); //Allocate memory for image's pixels

    pixels_per_row = pos_end[1] - pos_start[1] + 1; //Number of pixels that each row of the subsection is going to have
    rows_travelled = 0;                             //Number of rows we've already travelled (we can't use the row variable because pos_start[0] might not always be 0!)
    
    for (int row = pos_start[0]; row <= pos_end[0]; row++)
    {
        current_index_image = row * image->width + pos_start[1]; //Starting index of the current row's first pixel on the image's pixel array
        subsection_offset = pixels_per_row * rows_travelled++;   //Offset of our subsections pixel_array (used so that we don't overwrite already written pixels)

        memmove(subsect->pixel_array + subsection_offset, &image->pixel_array[current_index_image], pixels_per_row * sizeof(Colour)); //Copy from our image's pixel array to our subsection's pixel array
    }

    return subsect;
}

int main()
{
    RGBImage *image = load_file("../lena.ppm");

    int start[2], end[2];

    start[0] = 0, start[1] = 0;
    end[0] = 255, end[1] = 511;

    Colour pixel = get_pixel(image, image->width / 2, image->height / 2);
    printf("R%d G%d B%d\n", pixel.R, pixel.G, pixel.B);

    RGBImage *subsect = get_subsect(image, start, end);
    save_to_file("subsection.ppm", subsect);
    save_to_file("lena.ppm", image);
    return 0;
}