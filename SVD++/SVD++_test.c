#include <stdio.h>
#include <stdlib.h>

int main() {
	// Iterate through all lines
	FILE *fp =  fopen("../../netflix/mu/all.dta", "r");
	FILE *fp2 = fopen("../../netflix/mu/all.idx", "r");
	if (fp == NULL || fp2 == NULL) {
		return -1;
	}

	int num_users = 458293;
	int num_movies = 17770;

	float * userValue = calloc(num_users, sizeof(float));
	float * movieValue = calloc(num_movies, sizeof(float));
	if (userValue == NULL || movieValue == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	
	// Load movie data and baseline into array
	int num_lines = 99666408;
	// int num_lines = 10;
	int * movie_data = calloc(num_lines * 3, sizeof(int));
	float * baseline_data = calloc(num_lines, sizeof(float));
	if (movie_data == NULL || baseline_data == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	int line_number = 0;
	int sample = 0;
	printf("\n----------Loading data-------------\n");
	while (fgets(str, 60, fp) != NULL && fgets(str2, 5, fp2) != NULL) {
		// Only use index < 5
		if (atoi((char* )strtok(str2, " ")) == 5) {
			continue;
		}

		// Get user, movie, rating
		movie_data[line_number] = atoi((char *)strtok(str, " "));
		movie_data[line_number + 1] = atoi((char *)strtok(NULL, " "));
		strtok(NULL, " ");
		movie_data[line_number + 2] = atoi((char *)strtok(NULL, " "));
		
		line_number += 3;
		sample++;
		if (sample % 1000000 == 0) {
			printf("%d\n", sample);
		}
	}
	fclose(fp);
	fclose(fp2);
	// baseline calculation:
	// b_ui = u + b_u + b_i
	// where u is overall average movie rating
	// b_u is deviation of user u's rating from overall
	// b_i is deviation of movie i's rating from overall
	// first step is to calculate overall average movie rating
}