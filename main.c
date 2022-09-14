// Example of using the Image library

#include "Image.h"
#include "utils.h"
#include <string.h>


int main(int argc, char *argv[]) {

    // Convert the images to gray
    Image img;
    Image_load(&img, argv[1]);
    ON_ERROR_EXIT(img.data == NULL, "Error in loading the image");
    Image img_out;
    Image_to_gray(&img, &img_out);
    // Save images
    Image_save(&img_out, "Images/output1.png");
    // Release memory
    Image_free(&img);
    Image_free(&img_out);
    
    // Open
    Image_load(&img, "Images/output1.png");
    Image_to_open(&img, &img_out);
    // Save images
    Image_save(&img_out, "Images/output2.png");
    // Release memory
    Image_free(&img);
    Image_free(&img_out);

    // Threshold
    Image orig;
    Image_load(&orig, "Images/output1.png");
    Image_load(&img, "Images/output2.png");
    Threshold(&orig, &img, &img_out, 20);
    // Save images
    Image_save(&img_out, "Images/output3.png");
    // Release memory
    Image_free(&img);
    Image_free(&orig);
    Image_free(&img_out);


    // Remove noise
    Image_load(&img, "Images/output3.png");
    Image_to_open_one(&img, &img_out);
    // Save images
    Image_save(&img_out, "Images/output4.png");
    // Release memory
    Image_free(&img);
    Image_free(&img_out);

    // Empty Image
    Image_load(&img, "Images/output4.png");
    Empty_with_pixel(&img, &img_out);
    // Save images
    Image_save(&img_out, "Images/output5.png");
    // Release memory
    Image_free(&img);
    Image_free(&img_out);
}