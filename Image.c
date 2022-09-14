#include "Image.h"
#include "utils.h"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

void Image_load(Image *img, const char *fname) {
    if((img->data = stbi_load(fname, &img->width, &img->height, &img->channels, 0)) != NULL) {
        img->size = img->width * img->height * img->channels;
        img->allocation_ = STB_ALLOCATED;
    }
}

void Image_create(Image *img, int width, int height, int channels, bool zeroed) {
    size_t size = width * height * channels;
    if(zeroed) {
        img->data = calloc(size, 1);
    } else {
        img->data = malloc(size);
    }

    if(img->data != NULL) {
        img->width = width;
        img->height = height;
        img->size = size;
        img->channels = channels;
        img->allocation_ = SELF_ALLOCATED;
    }
}

void Image_save(const Image *img, const char *fname) {
    // Check if the file name ends in one of the .jpg/.JPG/.jpeg/.JPEG or .png/.PNG
    if(str_ends_in(fname, ".jpg") || str_ends_in(fname, ".JPG") || str_ends_in(fname, ".jpeg") || str_ends_in(fname, ".JPEG")) {
        stbi_write_jpg(fname, img->width, img->height, img->channels, img->data, 100);
    } else if(str_ends_in(fname, ".png") || str_ends_in(fname, ".PNG")) {
        stbi_write_png(fname, img->width, img->height, img->channels, img->data, img->width * img->channels);
    } else {
        ON_ERROR_EXIT(false, "");
    }
}

void Image_free(Image *img) {
    if(img->allocation_ != NO_ALLOCATION && img->data != NULL) {
        if(img->allocation_ == STB_ALLOCATED) {
            stbi_image_free(img->data);
        } else {
            free(img->data);
        }
        img->data = NULL;
        img->width = 0;
        img->height = 0;
        img->size = 0;
        img->allocation_ = NO_ALLOCATION;
    }
}

void Image_to_gray(const Image *orig, Image *gray) {
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(gray, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(gray->data == NULL, "Error in creating the image");

    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = gray->data; p != orig->data + orig->size; p += orig->channels, pg += gray->channels) 
    {
	
		*pg = (uint8_t)((*p + *(p + 1) + *(p + 2))/3);

		// Don't touch
        if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }
    }
}


// The ray (r) must be 1, 3, 5, 7, ... and the size of each dimension must be r*2+1
void Euclidian_disc(int r, uint8_t disc[2*r+1][2*r+1]) {
	for (int i = 0; i < r*2+1; ++i)
	{
		for (int j = 0; j < r*2+1; ++j)
		{
			if (sqrt((i-r)*(i-r) + (j-r)*(j-r)) <= r)
			{
				disc[i][j] = 1;
			}
		}
	}
}


// The ray (r) must be 1, 3, 5, 7, ... and the size of each dimension must be r*2+1
void Euclidian_disc_inverted(int r, uint8_t disc[2*r+1][2*r+1]) {
	for (int i = 0; i < r*2+1; ++i)
	{
		for (int j = 0; j < r*2+1; ++j)
		{
			if (sqrt((i-r)*(i-r) + (j-r)*(j-r)) <= r)
			{
				disc[i][j] = 0;
			}
			else
			{
				disc[i][j] = 1;
			}
		}
	}
}

// Eroding an image
void Image_to_erode(const Image *orig, Image *eroded) {
	ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(eroded, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(eroded->data == NULL, "Error in creating the image");

    // Insert the buffer in an array
    uint8_t img_array[orig->height][orig->width];
    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = eroded->data; p != orig->data + orig->size; p += orig->channels, pg += eroded->channels) 
    {
	
		img_array[j][i] = (uint8_t)((*p + *(p + 1) + *(p + 2))/3);

		i += 1;

		if (i >= orig->width) {
	    	i = 0;
	    	j += 1;
		}
    }


    // Making erosion
    /*
    uint8_t E[11][11] = {
    	{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0}
    };
    */

    int r = 43;
    uint8_t E[2*r+1][2*r+1];
    Euclidian_disc(r, *E);

    i = 0;
    j = 0;
    for(unsigned char *p = orig->data, *pg = eroded->data; p != orig->data + orig->size; p += orig->channels, pg += eroded->channels) {
        // Code here
        /*  I (-) E : (aide grâce à la vidéo suivante : https://www.youtube.com/watch?v=DykBrQzgUOk)

		On réalise l'algorithme sur tous les pixels :
		Vérifier sur les 11*11 pixels à partir de notre pixel en [5][5] comme centre quel est la valeur la + petite parmis eux.
		Mettre en nouvelle valeur cette valeur minimum dans notre nouvel array.

		*/

    	int left_val = i - r - 1;
    	if (left_val < 0) {
    		left_val = 0;
    	}

    	int right_val = i + r + 1;
    	if (right_val >= orig -> width) {
    		right_val = orig -> width - 1;
    	}

    	int up_val = j - r - 1;
    	if (up_val < 0) {
    		up_val = 0;
    	}

    	int down_val = j + r + 1;
    	if (down_val >= orig -> height) {
    		down_val = orig -> height - 1;
    	}


    	int min = 255;

    	for (int b = up_val; b < down_val; ++b)
    	{
    		for (int a = left_val; a < right_val; ++a)
	    	{
	    		//printf("%i \n", right_val-a);
	    		if (E[down_val-b][right_val-a] == 1 && min > img_array[b][a])
	    		{
	    			min = img_array[b][a];
	    		}
	    	}
    	}

        *pg = min;
        
        // Don't touch
        if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }

        i += 1;

        if (i >= orig->width) 
        {
            i = 0;
            j += 1;
        }
    }
}

// dilate an image
void Image_to_dilate(const Image *orig, Image *dilated) {
	ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(dilated, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(dilated->data == NULL, "Error in creating the image");

    // Insert the buffer in an array
    uint8_t img_array[orig->height][orig->width];
    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = dilated->data; p != orig->data + orig->size; p += orig->channels, pg += dilated->channels) 
    {
	
		img_array[j][i] = (uint8_t)((*p + *(p + 1) + *(p + 2))/3);

		i += 1;

		if (i >= orig->width) {
	    	i = 0;
	    	j += 1;
		}
    }


    // Making dilation
    /*
    uint8_t E[11][11] = {
    	{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    	{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0}
    };
	*/

    int r = 43;
    uint8_t E[2*r+1][2*r+1];
    Euclidian_disc(r, *E);

    i = 0;
    j = 0;
    for(unsigned char *p = orig->data, *pg = dilated->data; p != orig->data + orig->size; p += orig->channels, pg += dilated->channels) {
        // Code here
        /*  I (-) E : (aide grâce à la vidéo suivante : https://www.youtube.com/watch?v=DykBrQzgUOk)

		On réalise l'algorithme sur tous les pixels :
		Vérifier sur les 11*11 pixels à partir de notre pixel en [5][5] comme centre quel est la valeur la + petite parmis eux.
		Mettre en nouvelle valeur cette valeur minimum dans notre nouvel array.

		*/

    	int left_val = i - r - 1;
    	if (left_val < 0) {
    		left_val = 0;
    	}

    	int right_val = i + r + 1;
    	if (right_val >= orig -> width) {
    		right_val = orig -> width - 1;
    	}

    	int up_val = j - r - 1;
    	if (up_val < 0) {
    		up_val = 0;
    	}

    	int down_val = j + r + 1;
    	if (down_val >= orig -> height) {
    		down_val = orig -> height - 1;
    	}


    	int max = 0;

    	for (int b = up_val; b < down_val; ++b)
    	{
    		for (int a = left_val; a < right_val; ++a)
	    	{
	    		//printf("%i \n", right_val-a);
	    		if (E[down_val-b][right_val-a] == 1 && max < img_array[b][a])
	    		{
	    			max = img_array[b][a];
	    		}
	    	}
    	}

        *pg = max;

        //*pg = max - img_array[j][i];
        
        // Don't touch
        if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }

        i += 1;

        if (i >= orig->width) 
        {
            i = 0;
            j += 1;
        }
    }
}


// Image_to_outline
void Image_to_outline(const Image *orig, Image *outlined) {
	ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(outlined, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(outlined->data == NULL, "Error in creating the image");

    // Insert the buffer in an array
    uint8_t img_array[orig->height][orig->width];
    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = outlined->data; p != orig->data + orig->size; p += orig->channels, pg += outlined->channels) 
    {
	
		img_array[j][i] = (uint8_t)((*p + *(p + 1) + *(p + 2))/3);

		i += 1;

		if (i >= orig->width) {
	    	i = 0;
	    	j += 1;
		}
    }

    int r = 4;
    uint8_t E[2*r+1][2*r+1];
    Euclidian_disc(r, *E);

    i = 0;
    j = 0;
    for(unsigned char *p = orig->data, *pg = outlined->data; p != orig->data + orig->size; p += orig->channels, pg += outlined->channels) {
        // Code here
        /*  I (-) E : (aide grâce à la vidéo suivante : https://www.youtube.com/watch?v=DykBrQzgUOk)

		On réalise l'algorithme sur tous les pixels :
		Vérifier sur les 11*11 pixels à partir de notre pixel en [5][5] comme centre quel est la valeur la + petite parmis eux.
		Mettre en nouvelle valeur cette valeur minimum dans notre nouvel array.

		*/

    	int left_val = i - r - 1;
    	if (left_val < 0) {
    		left_val = 0;
    	}

    	int right_val = i + r + 1;
    	if (right_val >= orig -> width) {
    		right_val = orig -> width;
    	}

    	int up_val = j - r - 1;
    	if (up_val < 0) {
    		up_val = 0;
    	}

    	int down_val = j + r + 1;
    	if (down_val >= orig -> height) {
    		down_val = orig -> height;
    	}


    	int max = 0;
    	int min = 255;

    	for (int b = up_val; b < down_val; ++b)
    	{
    		for (int a = left_val; a < right_val; ++a)
	    	{
	    		if (E[down_val-b][right_val-a] == 1 && max < img_array[b][a])
	    		{
	    			max = img_array[b][a];
	    		}

	    		if (E[down_val-b][right_val-a] == 1 && min > img_array[b][a])
	    		{
	    			min = img_array[b][a];
	    		}
	    	}
    	}

        *pg = max - min;
        
        // Don't touch
        if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }

        i += 1;

        if (i >= orig->width) 
        {
            i = 0;
            j += 1;
        }
    }
}


// Opening an image
void Image_to_open(const Image *orig, Image *opened) {
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(opened, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(opened->data == NULL, "Error in creating the image");

    // Insert the buffer in an array
    uint8_t img_array[orig->height][orig->width];
    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = opened->data; p != orig->data + orig->size; p += orig->channels, pg += opened->channels) 
    {
	
		img_array[j][i] = *p;

		i += 1;

		if (i >= orig->width) {
	    	i = 0;
	    	j += 1;
		}
    }


    int r = 3;
    uint8_t E[2*r+1][2*r+1];
    Euclidian_disc_inverted(r, *E);


    i = 0;
    j = 0;
    for(unsigned char *p = orig->data, *pg = opened->data; p != orig->data + orig->size; p += orig->channels, pg += opened->channels) {
        // Code here
        /*  I (-) E : (aide grâce à la vidéo suivante : https://www.youtube.com/watch?v=DykBrQzgUOk)

		On réalise l'algorithme sur tous les pixels :
		Vérifier sur les 11*11 pixels à partir de notre pixel en [5][5] comme centre quel est la valeur la + petite parmis eux.
		Mettre en nouvelle valeur cette valeur minimum dans notre nouvel array.

		*/

    	int left_val = i - r - 1;
    	if (left_val < 0) {
    		left_val = 0;
    	}

    	int right_val = i + r + 1;
    	if (right_val >= orig -> width) {
    		right_val = orig -> width - 1;
    	}

    	int up_val = j - r - 1;
    	if (up_val < 0) {
    		up_val = 0;
    	}

    	int down_val = j + r + 1;
    	if (down_val >= orig -> height) {
    		down_val = orig -> height - 1;
    	}


    	int min = 255;

    	for (int b = up_val; b < down_val; ++b)
    	{
    		for (int a = left_val; a < right_val; ++a)
	    	{
	    		if (E[down_val-b][right_val-a] == 1 && min > img_array[b][a])
	    		{
	    			min = img_array[b][a];
	    		}

	    	}
    	}

        *pg = min;

        // Don't touch
        if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }

        i += 1;

        if (i >= orig->width) 
        {
            i = 0;
            j += 1;
        }
    }
}

// Opening an image (r = 1)
void Image_to_open_one(const Image *orig, Image *opened) {
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(opened, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(opened->data == NULL, "Error in creating the image");

    // Insert the buffer in an array
    uint8_t img_array[orig->height][orig->width];
    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = opened->data; p != orig->data + orig->size; p += orig->channels, pg += opened->channels) 
    {
	
		img_array[j][i] = *p;

		i += 1;

		if (i >= orig->width) {
	    	i = 0;
	    	j += 1;
		}
    }


    int r = 1;
    uint8_t E[2*r+1][2*r+1];
    Euclidian_disc_inverted(r, *E);


    i = 0;
    j = 0;
    for(unsigned char *p = orig->data, *pg = opened->data; p != orig->data + orig->size; p += orig->channels, pg += opened->channels) {
        // Code here
        /*  I (-) E : (aide grâce à la vidéo suivante : https://www.youtube.com/watch?v=DykBrQzgUOk)

		On réalise l'algorithme sur tous les pixels :
		Vérifier sur les 11*11 pixels à partir de notre pixel en [5][5] comme centre quel est la valeur la + petite parmis eux.
		Mettre en nouvelle valeur cette valeur minimum dans notre nouvel array.

		*/

    	int left_val = i - r - 1;
    	if (left_val < 0) {
    		left_val = 0;
    	}

    	int right_val = i + r + 1;
    	if (right_val >= orig -> width) {
    		right_val = orig -> width - 1;
    	}

    	int up_val = j - r - 1;
    	if (up_val < 0) {
    		up_val = 0;
    	}

    	int down_val = j + r + 1;
    	if (down_val >= orig -> height) {
    		down_val = orig -> height - 1;
    	}


    	int min = 255;

    	for (int b = up_val; b < down_val; ++b)
    	{
    		for (int a = left_val; a < right_val; ++a)
	    	{
	    		if (E[down_val-b][right_val-a] == 1 && min > img_array[b][a])
	    		{
	    			min = img_array[b][a];
	    		}

	    	}
    	}

        *pg = min;
        
        // Don't touch
        if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }

        i += 1;

        if (i >= orig->width) 
        {
            i = 0;
            j += 1;
        }
    }
}


// Closing an image
void Image_to_close(const Image *orig, Image *closed) {
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(closed, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(closed->data == NULL, "Error in creating the image");

    // Insert the buffer in an array
    uint8_t img_array[orig->height][orig->width];
    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = closed->data; p != orig->data + orig->size; p += orig->channels, pg += closed->channels) 
    {
	
		img_array[j][i] = (uint8_t)((*p + *(p + 1) + *(p + 2))/3);

		i += 1;

		if (i >= orig->width) {
	    	i = 0;
	    	j += 1;
		}
    }


    int r = 1;
    uint8_t E[2*r+1][2*r+1];
    Euclidian_disc_inverted(r, *E);

    i = 0;
    j = 0;
    for(unsigned char *p = orig->data, *pg = closed->data; p != orig->data + orig->size; p += orig->channels, pg += closed->channels) {
        // Code here
        /*  I (-) E : (aide grâce à la vidéo suivante : https://www.youtube.com/watch?v=DykBrQzgUOk)

		On réalise l'algorithme sur tous les pixels :
		Vérifier sur les 11*11 pixels à partir de notre pixel en [5][5] comme centre quel est la valeur la + petite parmis eux.
		Mettre en nouvelle valeur cette valeur minimum dans notre nouvel array.

		*/

    	int left_val = i - r - 1;
    	if (left_val < 0) {
    		left_val = 0;
    	}

    	int right_val = i + r + 1;
    	if (right_val >= orig -> width) {
    		right_val = orig -> width - 1;
    	}

    	int up_val = j - r - 1;
    	if (up_val < 0) {
    		up_val = 0;
    	}

    	int down_val = j + r + 1;
    	if (down_val >= orig -> height) {
    		down_val = orig -> height - 1;
    	}


    	int max = 0;

    	for (int b = up_val; b < down_val; ++b)
    	{
    		for (int a = left_val; a < right_val; ++a)
	    	{
	    		if (E[down_val-b][right_val-a] == 1 && max < img_array[b][a])
	    		{
	    			max = img_array[b][a];
	    		}
	    	}
    	}

        *pg = max;
        
        // Don't touch
        if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }

        i += 1;

        if (i >= orig->width) 
        {
            i = 0;
            j += 1;
        }
    }
}

// threshold
void Threshold(const Image *orig, Image *transformed, Image *output, int t) {
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(output, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(output->data == NULL, "Error in creating the image");

    for(unsigned char *p = orig->data, *pg = output->data, *tr = transformed->data; p != orig->data + orig->size; p += orig->channels, pg += output->channels, tr += transformed->channels) {
    	*pg = *p - *tr;
    	*pg = *pg >= t ? 255 : 0;

    	if(orig->channels == 4) 
        {
            *(pg + 1) = *(p + 3);
        }
    }
}

// Empty_with_pixel
void Empty_with_pixel(const Image *orig, Image *output) {
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(output, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(output->data == NULL, "Error in creating the image");

    int i = 0;
    int j = 0;
    for(unsigned char *p = orig->data, *pg = output->data; p != orig->data + orig->size; p += orig->channels, pg += output->channels) 
    {
		if (i == 282 && j == 49)
		{
			*pg = 255;
		}
		else
		{
			*pg = 0;
		}

		i += 1;

		if (i >= orig->width) {
	    	i = 0;
	    	j += 1;
		}
    }
}
