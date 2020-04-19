/*
 * File that stores an image into an object and compares similarity of images
 * based on Euclidean distance
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include "worker.h"


/*
 * Read an image from a file and create a corresponding 
 * image struct 
 */

Image* read_image(char *filename)
{
        Image *img = malloc(sizeof(Image));

        if (img == NULL) {
                perror("malloc");
        }
        
        FILE *image_input;
        int error;
        int image_counter = 0;
        int pixel_count = 0;

        image_input = fopen(filename, "r");
        if (image_input == NULL) {
                fprintf(stderr, "Error opening file\n");
        }

        char pHeader[2];
        int currentToken;

        fscanf(image_input, "%69s", pHeader);
        if (strcmp(pHeader, "P3") != 0) {
                return NULL;
        }
        image_counter++;

        while (fscanf(image_input, "%69d", &currentToken) != EOF) {
                if (image_counter == 1) {
                        img->width = currentToken;
                }
                else if (image_counter == 2) {
                        img->height = currentToken;
                        img->p = malloc((img->width*img->height) * sizeof(Pixel));
                        if (img->p == NULL) {
                                perror("malloc");
                        }
                }
                else if (image_counter == 3) {
                        img->max_value = currentToken;
                }
                else if (image_counter == 4) {
                        img->p[pixel_count].red = currentToken;
                }
                else if (image_counter == 5) {
                        img->p[pixel_count].green = currentToken;
                }
                else if (image_counter == 6) {
                        img->p[pixel_count].blue = currentToken;
                }

                image_counter++;
                if (image_counter == 7) {
                        image_counter = 4;
                        pixel_count++;
                }
        }

        error = fclose(image_input);
        if (error != 0) {
                fprintf(stderr, "fclose failed\n");
        }

        return img;
}

/*
 * Print an image based on the provided Image struct 
 */

void print_image(Image *img){
        printf("P3\n");
        printf("%d %d\n", img->width, img->height);
        printf("%d\n", img->max_value);
       
        for(int i=0; i<img->width*img->height; i++)
           printf("%d %d %d  ", img->p[i].red, img->p[i].green, img->p[i].blue);
        printf("\n");
}

/*
 * Compute the Euclidian distance between two pixels 
 */
float eucl_distance (Pixel p1, Pixel p2) {
        return sqrt( pow(p1.red - p2.red,2 ) + pow( p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));
}

/*
 * Compute the average Euclidian distance between the pixels 
 * in the image provided by img1 and the image contained in
 * the file filename
 */

float compare_images(Image *img1, char *filename) {

        Image *new_image = read_image(filename);
        if (new_image == NULL) {
                return FLT_MAX;
        }

        if ((img1->width != new_image->width) || (img1->height != new_image->height)) {
                return FLT_MAX;
        }

        float distance_sum = 0.0;

        for (int i = 0; i < (img1->width * img1->height); i++) {
                distance_sum += eucl_distance(img1->p[i], new_image->p[i]);
        }

        distance_sum = distance_sum/(img1->width * img1->height);

        free(new_image->p);
        free(new_image);

        return distance_sum;
}

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it 
* - for each file read the image in it 
* - compare the image read to the image passed as parameter 
* - keep track of the image that is most similar 
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
CompRecord process_dir(char *dirname, Image *img, int out_fd){

        char path[PATHLENGTH];
        DIR *dirp;
	if((dirp = opendir(dirname)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	
	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the image files contained in the directory.
	*/
		
	struct dirent *dp;
        CompRecord CRec;
        CRec.distance = FLT_MAX;
        strcpy(CRec.filename, "");

	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 || 
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, dirname, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 
                

		// checks if entry is a file
		if(S_ISREG(sbuf.st_mode)) {
                        float distance = compare_images(img, path);
                        if (distance < CRec.distance) {
                                CRec.distance = distance;
                                strcpy(CRec.filename, dp->d_name);
                        }
		}
		
	}

        if (out_fd != STDOUT_FILENO) {
                CompRecord *CrecPipe = malloc(sizeof(CompRecord));
                if (CrecPipe == NULL) {
                        perror("malloc");
                }
                CrecPipe = &CRec;

                if (write(out_fd, CrecPipe, sizeof(CompRecord)) != sizeof(CompRecord)) {
                        perror("write from child to pipe");
                }
        }

        return CRec;
}
