/*
File Name: 	mean_filter_ppm.c
Author: 		James P. Jolly
Course: 		ECE 6310-001 Intro to Computer Vision
Due Date: 	September 6th, 2018
Purpose: 	Perform a convolution filter of a ppm image using various techniques,
				including calculating each pixel individually, performing a separable
				filters of the pixels, and separable filters with a sliding window. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>



/* 
Function:	main()
Purpose:		Execute the filters of the ppm image and record their speeds.
Inputs:		"ExecutableName filename.ext"  
Outputs: 	Returns a 0 upon success, -1 upon any error
*/

int main(int argc, char* argv[]) {

////////// Initialize variables
FILE* 				fpt;
unsigned char*		image;
unsigned char* 	filtered;
char 					header[360];
int					ROWS,COLS,BYTES;
int					r,c,r2,c2,sum;
struct timespec	tp1,tp2;

// Check arguments
if (argc != 2) {
	printf("Error: Expects two input arguments: the executable name followed by the ppm image to read.\n");
	return -1;
}

// Read in file
if ((fpt=fopen(argv[1],"rb")) == NULL) {
  printf("Unable to open bridge.ppm for reading\n");
  exit(0);
}
fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
if (strcmp(header,"P5") != 0  ||  BYTES != 255) {
  printf("Not a greyscale 8-bit PPM image\n");
  exit(0);
}
image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
filtered = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
// Don't need //header[0]=fgetc(fpt);	/* read white-space character that separates header */
fread(image,1,COLS*ROWS,fpt);
fclose(fpt);



////////// Read in file



////////// Brute Force Convolution filter




////////// Separable Filters




// Separable Filters with a Sliding Window




// Free memory
free(image);
free(filtered);

printf("Returning successfully.\n");
return 1;
} /* end of main */


