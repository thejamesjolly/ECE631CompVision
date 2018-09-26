/*
File Name: 	lab2.c
Author: 		James P. Jolly
Course: 		ECE 6310-001 Intro to Computer Vision
Due Date: 	September 18th, 2018
Purpose: 	Find a specific threshold for the identification of a target letter in a ppm 
				image of a word document. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>



/* 
Function:	main()
Purpose:		Execute the filters of the ppm image and record their speeds.
Inputs:		"./ExecutableName picture.ppm target.ppm ground_truth.txt"  
Outputs: 	Returns a 1 upon success, 0 or -1 upon any error
*/

int main(int argc, char* argv[]) {

////////// Initialize variables
FILE* 				fpt;
unsigned char*		image;
unsigned char* 	target;
int*					rawMSF;
unsigned char*		imageMSF;
char 					header[360]; // used to read miscellaneous file information
int					ROWS_I,COLS_I,BYTES_I; // the image in which to find the target
int 					ROWS_T,COLS_T,BYTES_T; // the target image for which is being searched
int					gtNumElem; // the number of elements in the ground truth file 
unsigned char*		gtChar; // array (NOT STRING) of the known characters in the searched ppm
int*	 				gtRow; // Corresponding row location for characters in gtChar
int* 					gtCol; // Corresponding col location
int 					targetSum, targetMean; // Values used to create the zero-mean center the target
int* 					targetFilter; // array used to determine match of the match spatial filter 
int					r,c,r2,c2; // counters used in convolution or cross-correlation
int 					i;  // Counters used in loops
int 					filterWidth; // n of Window width goes from -n:n and is (2n+1) pixels wide
int 					filterHeight; // m of Window width goes from -m:m and is (2m+1) pixels tall
int 					pixMin, pixMax, pixSum; // minimum MSF value, maximum MSF value, and temp variable for summation 
unsigned int		thresh;
int 					matchFlag; // used to break in search for a matched target in ground truth file

int 					falsePos, truePos, missingEs, unmatchedNotEs; // number of false positives and true detections made



image= NULL;
target = NULL;
rawMSF = NULL;
gtChar = NULL;
gtRow = NULL;
gtCol = NULL;
targetFilter = NULL;

// Check arguments
if (argc != 4) {
	printf("Error: Expects four input arguments: the executable name, the ppm image in which to search,\n");
	printf("the ppm image of the target to find, and the text file of \"ground truth\" location of characters.\n");
	return -1;
}
if (strlen(argv[1]) > 150) {
	printf("Error: file name of second argument must be less than 150 characters long.\n");
	return -1;
}

// Read image file
if ((fpt=fopen(argv[1],"rb")) == NULL) {
  printf("Unable to open %s for reading\n", argv[1]);
  exit(0);
}
fscanf(fpt,"%s %d %d %d\n",header,&COLS_I,&ROWS_I,&BYTES_I);
if (strcmp(header,"P5") != 0  ||  BYTES_I != 255) {
  printf("Image to search not a greyscale 8-bit PPM image\n");
  exit(0);
}
image = (unsigned char *)calloc(ROWS_I*COLS_I,sizeof(unsigned char));
rawMSF = (int *)calloc(ROWS_I*COLS_I,sizeof(int));
imageMSF = (unsigned char *)calloc(ROWS_I*COLS_I,sizeof(unsigned char));


fread(image,1,COLS_I*ROWS_I,fpt);
fclose(fpt);



// Read in Target File

if ((fpt=fopen(argv[2],"rb")) == NULL) {
  printf("Unable to open %s for reading\n", argv[2]);
  exit(0);
}
fscanf(fpt,"%s %d %d %d\n",header,&COLS_T,&ROWS_T,&BYTES_T);
if (strcmp(header,"P5") != 0  ||  BYTES_T != 255) {
  printf("Target not a greyscale 8-bit PPM image\n");
  exit(0);
}
if ((ROWS_T % 2 != 1) || (COLS_T % 2 != 1)) {
	printf("Target image for which to search must have a center pixel (odd number of rows and columns).\n");
	exit(0);
}

filterWidth = COLS_T/2; // rounds down to the correct value
filterHeight = ROWS_T/2; // rounds down to the correct value

target = (unsigned char *)calloc(ROWS_T*COLS_T,sizeof(unsigned char));

fread(target,1,COLS_T*ROWS_T,fpt);
fclose(fpt);



// Read in "Ground Truth" Text File

if ((fpt=fopen(argv[3],"r")) == NULL) {
  printf("Unable to open %s for reading\n", argv[3]);
  exit(0);
}
// Skipping data validation, but assuming is in a letter[char], row [int], column [int] format 
// NOTE: with no new line at the end of the file
// counting for loop to find the size of the file
for (gtNumElem = 0; fgets(header,360,fpt) != NULL; gtNumElem++); 
rewind(fpt); // rewind the file to the top
gtChar = (unsigned char *)calloc(gtNumElem,sizeof(unsigned char));
gtRow = (int *)calloc(gtNumElem,sizeof(int));
gtCol = (int *)calloc(gtNumElem,sizeof(int));

for (i = 0; i < gtNumElem; i++) {
	fscanf(fpt," %c %d %d",&gtChar[i],&gtCol[i],&gtRow[i]); // precede %c wth a space to trash new lines
}

fclose(fpt);



////////// Create the Matched Spatial Filter of the target
targetSum = 0;
for (r = 0; r < ROWS_T; r++) {
	for (c = 0; c < COLS_T; c++) {
		targetSum += target[(r*COLS_T) + (c)];
	}
}

// Find the Average
targetMean = targetSum / (ROWS_T * COLS_T);
targetFilter = calloc(ROWS_T*COLS_T, sizeof(int));

// Create filter
for (r = 0; r < ROWS_T; r++) {
	for (c = 0; c < COLS_T; c++) {
		targetFilter[(r*COLS_T) + (c)] = target[(r*COLS_T) + (c)] - targetMean;
	}
}


////////// Perform MSF analysis on the picture

// Clear min and max values to use fornormailizing the MSF
pixMin = 0;
pixMax = 0;

// Loop through all pixels that have enough trim to compute filter
for (r = filterHeight; r < (ROWS_I-filterHeight); r++) { 
	for (c = filterWidth; c < (COLS_I-filterWidth); c++) {
		pixSum = 0; // clear sum for each pixel
		// Loop through cross correlation of match filter
		for (r2 = -filterHeight; r2 <= filterHeight; r2++) {
			for (c2 = -filterWidth; c2 <= filterWidth; c2++) {
				pixSum += image[((r+r2)*COLS_I) + (c+c2)] * targetFilter[((r2+filterHeight)*COLS_T) + (c2 + filterWidth)];
			}
		}
		// Save range of values to normailze after
		if (pixSum < pixMin) pixMin = pixSum;
		if (pixSum > pixMax) pixMax = pixSum;
		rawMSF[r*COLS_I + c] = pixSum;
	}
}


// Normalize the entire MSF
// Loop through all pixels in MSF and normailze
if (pixMax-pixMin == 0) {
	printf("error dividing due to no differenciation of the MSF.\n");
	return -1;
}
for (r = filterHeight; r < (ROWS_I-filterHeight); r++) { 
	for (c = filterWidth; c < (COLS_I-filterWidth); c++) {
		imageMSF[r*COLS_I + c] = ((rawMSF[r*COLS_I + c] - pixMin) * 255) / (pixMax-pixMin);
	}
}

// Output image to a file for testing
fpt=fopen("MSF_image.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS_I,ROWS_I);
fwrite(imageMSF,COLS_I*ROWS_I,1,fpt);
fclose(fpt);


///////// Compare to ground truth to determine number of accurate identifications and flase positives

// Open output file
fpt=fopen("MSF_thresholds.txt","w");

for (thresh = 0; thresh <= 255; thresh++) { // for different thresholds
	// Clear the number of positives detected from the previous threshold
	falsePos = 0;
	truePos = 0;
	missingEs = 0;
	unmatchedNotEs = 0;
	for (i = 0; i < gtNumElem; i++) { // for different elements
		matchFlag = 0; // clear flag
		for (r2 = -filterHeight; r2 <= filterHeight; r2++) { // loop through entire target size
			for (c2 = -filterWidth; c2 <= filterWidth; c2++) {
				if (imageMSF[(gtRow[i]+r2)*COLS_I + (gtCol[i] + c2)] > thresh) {
					matchFlag = 1;
					break;
				}
			}
			if (matchFlag == 1) {
				break;
			}
		} 
		
		// if matched, add to false positives or detected letter
		if (matchFlag == 1) {
			if (gtChar[i] == 'e') {
				truePos++;
			}
			else {
				falsePos++;
			}
		}
		else { // if no match
			if (gtChar[i] == 'e') {
				missingEs++;
			}
			else {
				unmatchedNotEs++;
			}
		}
	} // end of gt element loop
	
	// Print results to terminal
	//printf("Threshold = %d\n", thresh);
	//printf("TP = %-6dMissing = %-6dTP Rate = %0.2f\n",truePos,missingEs,(double)truePos/(truePos+missingEs));
	//printf("FP = %-6dIgnored = %-6dFP Rate = %0.2f\n\n",falsePos,unmatchedNotEs,(double)falsePos/(falsePos+unmatchedNotEs));
	
	// Print results to the file
	//fprintf(fpt,"%d\n", thresh);
	//fprintf(fpt,"\t%d %d %f ",truePos,missingEs,(double)truePos/(truePos+missingEs));
	//fprintf(fpt,"%d %d %f\n",falsePos,unmatchedNotEs,(double)falsePos/(falsePos+unmatchedNotEs));
		
	// Print each of the stats to a file for transfer to Excel
	//fprintf(fpt,"%d\n", thresh);
	//fprintf(fpt,"%d\n",truePos);
	//fprintf(fpt,"%d\n",missingEs);
	//fprintf(fpt,"%f\n",(double)truePos/(truePos+missingEs));
	//fprintf(fpt,"%d\n",falsePos);
	//fprintf(fpt,"%d\n",unmatchedNotEs);
	//fprintf(fpt,"%f\n",(double)falsePos/(falsePos+unmatchedNotEs));
} // end of threshold for loop

fclose(fpt);

////////// Housekeeping at the end

// Free allocated memory
free(image);
free(target);
free(rawMSF);
free(imageMSF);
free(gtChar);
free(gtRow);
free(gtCol);
free(targetFilter);

// Return successfully
return 1;


} // end of main()

