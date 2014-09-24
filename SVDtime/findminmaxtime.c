#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void loadMovieData() {
	// Iterate through all lines
	char str[60];
	FILE *fp =  fopen("../../netflix/um/all.dta", "r");
	if (fp == NULL) {
		return;
	}
	int min_time = 999999;
	int max_time = 0;
	int time_v;
	while (fgets(str, 60, fp) != NULL) {
		strtok(str, " ");
		strtok(NULL, " ");
		time_v = atoi(strtok(NULL, " "));
		if (time_v < min_time) {
			min_time = time_v;
		} else if (time_v > max_time) {
			max_time = time_v;
		}
	}
	printf("min time %d, maxtime %d\n", min_time, max_time);
	fclose(fp);	
}

int main() {
	//loadMovieData();
	int c = 5;
	printf("%f\n", c / 2.0);
	return 1;
}