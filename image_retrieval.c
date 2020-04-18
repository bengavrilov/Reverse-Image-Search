/*
 * Finding most similar image within given directory by forking processes for subdirectories
 * within given directory
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>
#include "worker.h"

int main(int argc, char **argv) {
	
	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";
    char *image_file = NULL;

	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
			startdir = optarg;
			break;
			default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
			exit(1);
		}
	}

        if (optind != argc-1) {
	     fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
        } else
             image_file = argv[optind];

	// Open the directory provided by the user (or current working directory)
	
	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	
	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the image files contained in the directory.
	*/
		
	struct dirent *dp;
    CompRecord *CRec = malloc(sizeof(CompRecord));

	if (CRec == NULL) {
		perror("malloc");
	}

	CRec->distance = FLT_MAX;
	strcpy(CRec->filename, "");
	Image *image = read_image(image_file);
	if (image == NULL) {
		printf("The most similar image is %s with a distance of %f\n", CRec->filename, CRec->distance);
	
		return 0;
	}

	int numOfPipes = 0;

	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 || 
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 

		// Only call process_dir if it is a directory
		// Otherwise ignore it.
		if(S_ISDIR(sbuf.st_mode)) {
            numOfPipes++;
		}
	}

	// Declare pipes required
	int pipe_fd[numOfPipes][2];

	int currentProcess = -1;

	char path2[PATHLENGTH];
	DIR *dirp2;
	if((dirp2 = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	struct dirent *dp2;

	while((dp2 = readdir(dirp2)) != NULL) {

		if(strcmp(dp2->d_name, ".") == 0 || 
		   strcmp(dp2->d_name, "..") == 0 ||
		   strcmp(dp2->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path2, startdir, PATHLENGTH);
		strncat(path2, "/", PATHLENGTH - strlen(path2) - 1);
		strncat(path2, dp2->d_name, PATHLENGTH - strlen(path2) - 1);

		struct stat sbuf2;
		if(stat(path2, &sbuf2) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 

		if (argc != 2) {
			if(S_ISDIR(sbuf2.st_mode)) {
				currentProcess++;

				if ((pipe(pipe_fd[currentProcess])) == -1) {
					perror("pipe");
					exit(1);
				}

            	int result = fork();

				if (result < 0) {
					perror("fork");
					exit(1);
				}
				else if (result == 0) {

					if (close(pipe_fd[currentProcess][0]) == -1) {
						perror("close reading end from inside child");
					}

					for (int i = 1; i < currentProcess; i++) {
						if (close(pipe_fd[i][0]) == -1) {
							perror("close reading ends of previously forked children");
						}
					}

					process_dir(path2, image, pipe_fd[currentProcess][1]);

					if (close(pipe_fd[currentProcess][1]) == -1) {
						perror("close pipe after writing");
					}
					exit(0);
				}
				else {
					if (close(pipe_fd[currentProcess][1]) == -1) {
						perror("close writing end of pipe in parent");
					}
				}
			}
			else if (S_ISREG(sbuf2.st_mode)) {
				float comparedResult = compare_images(image, path2);
				if (comparedResult < CRec->distance) {
					CRec->distance = comparedResult;
					strncpy(CRec->filename, dp2->d_name, strlen(dp2->d_name) + 1);
				}
			
			}
		}
		else {
			if(S_ISDIR(sbuf2.st_mode)) {
				currentProcess++;

				if ((pipe(pipe_fd[currentProcess])) == -1) {
					perror("pipe");
					exit(1);
				}

            	int result = fork();

				if (result < 0) {
					perror("fork");
					exit(1);
				}
				else if (result == 0) {

					if (close(pipe_fd[currentProcess][0]) == -1) {
						perror("close reading end from inside child");
					}

					for (int i = 1; i < currentProcess; i++) {
						if (close(pipe_fd[i][0]) == -1) {
							perror("close reading ends of previously forked children");
						}
					}

					process_dir(path2, image, pipe_fd[currentProcess][1]);

					if (close(pipe_fd[currentProcess][1]) == -1) {
						perror("close pipe after writing");
					}
					exit(0);
				}
				else {
					if (close(pipe_fd[currentProcess][1]) == -1) {
						perror("close writing end of pipe in parent");
					}
				}
			}
		}
		
	}

	CompRecord tempCRecPipe;
	for (int y = 0; y < numOfPipes; y++) {
		if (read(pipe_fd[y][0], &tempCRecPipe, sizeof(CompRecord)) == -1) {
			perror("reading from pipe from a child");
		}
		if (tempCRecPipe.distance < CRec->distance) {
			CRec = &tempCRecPipe;
		}
	}

        printf("The most similar image is %s with a distance of %f\n", CRec->filename, CRec->distance);
	
	return 0;
}