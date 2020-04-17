#define PATHLENGTH 128

// This data structure is used by the workers to prepare the output
// to be sent to the master process.
typedef struct {
        char filename[PATHLENGTH];
        float distance;
} CompRecord;

typedef struct {
        int red;
        int green;
        int blue;
} Pixel;

typedef struct {
        int width;
        int height;
        int max_value;
        Pixel *p;
} Image;

Image* read_image(char *filename);
void print_image(Image *img);
float eucl_distance (Pixel p1, Pixel p2);
float compare_images(Image *img1, char *filename);
CompRecord process_dir(char *dirname, Image *img, int out_fd);
