/* Test the initial baseline on the test set */
#include <stdio.h>
#include <stdlib.h>

static float GLOBAL_AVG = 3.609516;
//static int num_lines = 2749898; // num lines in qual data
static int num_lines = 1374739; // num lines in probe;
int main(){
	// Iterate through all lines
	//FILE *fp = fopen("../../netflix/um/qual.dta", "r");
	FILE *fp = fopen("../stats/probe.dta", "r");
	FILE *fp2 = fopen("../stats/movie_offset_reg.dta", "r");
	FILE *fp3 = fopen("../stats/user_offset_reg2.dta", "r");

	if (fp == NULL || fp2 == NULL || fp3 == NULL) {
		return -1;
	}

	printf("\n----------Loading qual data-------------\n");
	// Load res data
	char str[60];
	
	// Multiply by two because we're storing user and movie per line
	int * qual_data = calloc(num_lines*2, sizeof(int));
	if (qual_data == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	int line_number = 0;


	while (fgets(str, 60, fp) != NULL) {
		// Get user, movie
		qual_data[line_number] = atoi((char *)strtok(str, " "));
		qual_data[line_number + 1] = atoi((char *)strtok(NULL, " "));
		line_number += 2;
	}
	fclose(fp);
	
	printf("\n----------Loading user and movie offset data-------------\n");
	// Add one because we're not using the first cell of both userValue and movieValue
	// to avoid off by one confusion
	int num_users = 458293 + 1;
	int num_movies = 17770 + 1;

	// Allocate and load movie offset
	float * movie_offset = calloc(num_movies, sizeof(float));
	if (movie_offset == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	int count = 1;
	while (fgets(str, 60, fp2) != NULL) {
		// Get user, movie
		movie_offset[count] = atof((char *)strtok(str, " "));
		count ++;
	}

	// Allocate and load user offset
	float * user_offset = calloc(num_users, sizeof(float));
	if (user_offset == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	count = 1;
	while (fgets(str, 60, fp3) != NULL) {
		// Get user, movie
		user_offset[count] = atof((char *)strtok(str, " "));
		count ++;
	}
	fclose(fp2);
	fclose(fp3);


	printf("\n--------------Getting Baseline --------------\n");
	int user, movie, rating;
	float old_res, predict, res, err, uv, mv;
	float * prediction = calloc(num_lines, sizeof(float));
	count = 0;
	for (line_number = 0; line_number < num_lines; line_number++) {
		count = line_number * 2;
		user = qual_data[count];
		movie = qual_data[count + 1];
		prediction[line_number] = GLOBAL_AVG + movie_offset[movie] + user_offset[user];
	}

	// Clip results
	float r;
	for (int j = 0; j < num_lines; j++) {
		r = prediction[j];
		if (r < 1) {
			prediction[j] = 1;
		} else if (r > 5) {
			prediction[j] = 5;
		}

	}
	printf("\n----------Saving Results----------\n");
	// Save residuals
	FILE *fp4 = fopen("results/um_probe_baseline0reg.dta", "w");
	for (int j = 0; j < num_lines; j++) {
		fprintf(fp4, "%f\n", prediction[j]);

	}
	fclose(fp4);

	return 0;
}

