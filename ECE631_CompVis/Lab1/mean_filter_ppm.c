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


#define NUM_TIME_ITER 10


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
unsigned int*		tempPic;
char 					header[360];
int					ROWS,COLS,BYTES;
int					r,c,r2,c2;
struct timespec	tp1,tp2;
int 					windowSize; // the n in the convolution filter used.
int 					windowWidth; // Window goes from -n:n and is (2n+1) pixels wide
int 					windowNumPix; // total number of pixels, used for normalizing values
int 					i;  // Counters used in loops
double 				filterSum; // Sum of the time the filter has run to be averaged
double 				filterTimes[3]; // Ave Time for Brute force, separable window, and separable plus sliding window
int 					pixelSum; // the total value to normalize when calculating the mean filter
char 					outputName[157]; // output file name

// Check arguments
if (argc != 2) {
	printf("Error: Expects two input arguments: the executable name followed by the ppm image to read.\n");
	return -1;
}
if (strlen(argv[1]) > 150) {
	printf("Error: file name of second argument must be less than 150 characters long.\n");
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
tempPic = (unsigned int *)calloc(ROWS*COLS,sizeof(unsigned int));
// Don't need //header[0]=fgetc(fpt);	/* read white-space character that separates header */
fread(image,1,COLS*ROWS,fpt);
fclose(fpt);

// Allocate Space for Filter
windowSize = 3;
windowWidth = 2*windowSize + 1;
windowNumPix = windowWidth*windowWidth;



////////// Brute Force Convolution filter

// Clear time stats
filterSum = 0.0;

for (i = 0; i < NUM_TIME_ITER; i++) {

	// Query Timer for Start Time
	clock_gettime(CLOCK_REALTIME,&tp1);
	//printf("Time1 = %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);


	for (r = windowSize; r < (ROWS-windowSize); r++) {
		for (c = windowSize; c < (COLS-windowSize); c++) {
			pixelSum = 0; // clear each pixel
			for (r2 = -windowSize; r2 <= windowSize; r2++) {
				for (c2 = -windowSize; c2 <= windowSize; c2++) {
					pixelSum += image[((r+r2)*COLS) + (c+c2)];
				}
			}
			filtered[r*COLS + c] = pixelSum / windowNumPix; // normailze to be within 255
		}
	}


	// Query Timer for Stop Time
	clock_gettime(CLOCK_REALTIME,&tp2);
	//printf("Time2 = %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);


	//printf("Difference = %lf\n\n",((double) (tp2.tv_sec-tp1.tv_sec)) * 1000000000 + (tp2.tv_nsec-tp1.tv_nsec));
	filterSum += ((double) (tp2.tv_sec - tp1.tv_sec)) * 1000000000 + (tp2.tv_nsec - tp1.tv_nsec); 
	
	// Save first processed image
	if (i == 0) {
		strcpy(outputName,"brute-");
		strcat(outputName,argv[1]);
		fpt=fopen(outputName,"w");
		fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
		fwrite(filtered,COLS*ROWS,1,fpt);
		fclose(fpt);
	}
	
	 
	memset(filtered,0,ROWS*COLS);
	

} // end of brute force for loop

filterTimes[0] = filterSum / NUM_TIME_ITER;

printf("Brute Force Method\n\tAve Time Elapsed: %12.3lf ns\n\n",filterTimes[0]);







////////// Separable Filters

// Clear time stats
filterSum = 0.0;

for (i = 0; i < NUM_TIME_ITER; i++) {

	// Query Timer for Start Time
	clock_gettime(CLOCK_REALTIME,&tp1);
	//printf("Time1 = %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	// Col Filter
	for (r = windowSize; r < (ROWS-windowSize); r++) {
		for (c = 0; c < COLS; c++) {
			pixelSum = 0; // clear each pixel
			for (r2 = -windowSize; r2 <= windowSize; r2++) {
				pixelSum += image[((r+r2)*COLS) + c];
			}
			tempPic[r*COLS + c] = pixelSum; // DO NOT NORMALIZE UNTIL END TO PREVENT ROUNDING ERRORS
		}
	}
	
	// Row Filter
	for (r = windowSize; r < (ROWS-windowSize); r++) {
		for (c = windowSize; c < (COLS-windowSize); c++) {
			pixelSum = 0; // clear each pixel
			for (c2 = -windowSize; c2 <= windowSize; c2++) {
				pixelSum += tempPic[r*COLS + (c+c2)];
			}
			filtered[r*COLS + c] = pixelSum / windowNumPix; // normailze to be within 255
		}
	}


	// Query Timer for Stop Time
	clock_gettime(CLOCK_REALTIME,&tp2);
	//printf("Time2 = %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);


	//printf("Difference = %lf\n\n",((double) (tp2.tv_sec-tp1.tv_sec)) * 1000000000 + (tp2.tv_nsec-tp1.tv_nsec));
	filterSum += ((double) (tp2.tv_sec - tp1.tv_sec)) * 1000000000 + (tp2.tv_nsec - tp1.tv_nsec); 
	
	
	// Save first processed image
	if (i == 0) {
		strcpy(outputName,"sep-");
		strcat(outputName,argv[1]);
		fpt=fopen(outputName,"w");
		fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
		fwrite(filtered,COLS*ROWS,1,fpt);
		fclose(fpt);
	}
	
	
	memset(filtered,0,ROWS*COLS);
	memset((char *)tempPic,0,(sizeof(unsigned int) / sizeof(unsigned char)) * ROWS*COLS);
	

} // end of brute force for loop

filterTimes[1] = filterSum / NUM_TIME_ITER;

printf("Separable Filter Method\n\tAve Time Elapsed: %12.3lf ns\n\n",filterTimes[1]);







////////// Separable Filters with a Sliding Window

// Clear time stats
filterSum = 0.0;

for (i = 0; i < NUM_TIME_ITER; i++) {

	// Query Timer for Start Time
	clock_gettime(CLOCK_REALTIME,&tp1);
	//printf("Time1 = %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);
	
	
	// Col Filter

	for (c = 0; c < COLS; c++) {
		pixelSum = 0; // clear first pixel in the column
		// Calculate first pixel
		for (r2 = -windowSize; r2 <= windowSize; r2++) {
			pixelSum += image[((windowSize+r2)*COLS) + (c)];
		}
		tempPic[windowSize*COLS + c] = pixelSum; // DO NOT NORMALIZE UNTIL END TO PREVENT ROUNDING ERRORS
		
		// SLide through the rest of the row
		for (r = windowSize+1; r < (ROWS-windowSize); r++) {
			pixelSum -= image[((r-windowSize-1)*COLS) + (c)]; // subtract top row away from which one is sliding
			pixelSum += image[((r+windowSize)*COLS) + (c)]; // add bottom row to which one is sliding
			tempPic[(r*COLS) + c] = pixelSum;
		}
	}

	
	// Row Filter
	for (r = windowSize; r < (ROWS-windowSize); r++) {
		pixelSum = 0; // clear first pixel in each row
		// Calculate first pixel in each row
		for (c2 = -windowSize; c2 <= windowSize; c2++) {
			pixelSum += tempPic[(r*COLS) + (windowSize+c2)];
		}
		filtered[r*COLS + windowSize] = pixelSum / windowNumPix; // normailze to be within 255
		
		for (c = windowSize+1; c < (COLS-windowSize); c++) {
			pixelSum -= tempPic[(r*COLS) + (c-windowSize-1)]; // subtract left col away from which one is sliding
			pixelSum += tempPic[(r*COLS) + (c+windowSize)]; // add right col to which one is sliding
			filtered[r*COLS + c] = pixelSum / windowNumPix; // normailze to be within 255
		}
	}


	// Query Timer for Stop Time
	clock_gettime(CLOCK_REALTIME,&tp2);
	//printf("Time2 = %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);


	//printf("Difference = %lf\n\n",((double) (tp2.tv_sec-tp1.tv_sec)) * 1000000000 + (tp2.tv_nsec-tp1.tv_nsec));
	filterSum += ((double) (tp2.tv_sec - tp1.tv_sec)) * 1000000000 + (tp2.tv_nsec - tp1.tv_nsec); 
	
	
	// Save first processed image
	if (i == 0) {
		strcpy(outputName,"slide-");
		strcat(outputName,argv[1]);
		fpt=fopen(outputName,"w");
		fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
		fwrite(filtered,COLS*ROWS,1,fpt);
		fclose(fpt);
	}
	
	memset(filtered,0,ROWS*COLS);
	memset((char *)tempPic,0,(sizeof(unsigned int) / sizeof(unsigned char)) * ROWS*COLS);
	

} // end of brute force for loop

filterTimes[2] = filterSum / NUM_TIME_ITER;

printf("Separable Filter Method with Moving Window\n\tAve Time Elapsed: %12.3lf ns\n\n",filterTimes[2]);




// Free memory
free(image);
free(filtered);
free(tempPic);

printf("Returning successfully.\n");
return 1;
} /* end of main */


