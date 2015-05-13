#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
static unsigned int qual_size = 2749898; // num lines in qual data

float * predictions;

// Clips score to be between 1 and 5
static inline
float clipScore(float score) {
	if (score < 1) {
		return 1;
	} else if (score > 5) {
		return 5;
	} else {
		return score;
	}
}

void loadData(char * file) {
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		printf("Saving results failed\n");
		return;
	}
	char str[60];
	int count = 0;
	while (fgets(str, 60, fp) != NULL) {
		predictions[count] = clipScore(atof(strtok(str, " ")));
		count++;
	}
	fclose(fp);

}

void saveResults(char * file) {
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		printf("Saving results failed\n");
		return;
	}
	for (int i=0; i < qual_size; i++) {
		fprintf(fp, "%f\n", predictions[i]);
	}
	fclose(fp);

}

int main() {
	predictions = calloc(qual_size, sizeof(float));
	loadData(char * file);
	saveResults(char * file);
}