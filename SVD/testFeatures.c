#include <stdio.h>
#include <stdlib.h>

int main(){
	// Iterate through all lines
	FILE *fp = fopen("../../netflix/mu/qual.dta", "r");
	FILE *fp2 = fopen("../stats/better_mavg2.dta", "r");
	FILE *fp3 = fopen("../stats/user_offset.dta", "r");

	if (fp == NULL || fp2 == NULL || fp3 == NULL) {
		return -1;
	}

	printf("\n----------Loading qual data-------------\n");
	// Load res data
	char str[60];
	int num_lines = 2749898; // num lines in qual data
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
	
	printf("\n----------Loading user and movie avg data-------------\n");
	// Add one because we're not using the first cell of both userValue and movieValue
	// to avoid off by one confusion
	char str2[15];
	char str3[15];
	int num_users = 458293 + 1;
	int num_movies = 17770 + 1;
	float * movie_avg = calloc(num_movies, sizeof(float));
	if (movie_avg == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	int count = 1;
	while (fgets(str, 60, fp2) != NULL) {
		// Get user, movie
		movie_avg[count] = atof((char *)strtok(str, " "));
		count ++;
	}

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
		prediction[line_number] = movie_avg[movie] + user_offset[user];
	}


	printf("\n--------------Using Features --------------\n");
	float * user_value = calloc(num_users, sizeof(float));
	float * movie_value = calloc(num_movies, sizeof(float));
	if (user_value == NULL || movie_value == NULL) {
		printf("Malloc failed\n");
		return -1;
	}

	char movie_feature_file[40];
	char user_feature_file[40];
	
	// For each feature
	for (int f = 1; f < 131; f++) {
		FILE *fp5, *fp6;
		// Save feature
		printf("\n-----------Using feature %d-----------\n", f);
		sprintf(movie_feature_file, "features/feature_m%d.dta", f);
		fp5 = fopen(movie_feature_file, "r");
		count = 1;
		while (fgets(str, 60, fp5) != NULL) {
			movie_value[count] = atof(str);
			count++;
		}
		
		fclose(fp5);

		sprintf(user_feature_file, "features/feature_u%d.dta", f);
		fp6 = fopen(user_feature_file, "r");
		count = 1;
		while (fgets(str, 60, fp6) != NULL) {
			user_value[count] = atof(str);
			count++;
		}
		
		fclose(fp6);
		for (line_number = 0; line_number < num_lines; line_number++) {
			count = line_number * 2;
			user = qual_data[count];
			movie = qual_data[count + 1];
			// printf("uservalue %f movievalue %f\n", user_value[user], movie_value[movie]);
			prediction[line_number] += user_value[user] *movie_value[movie];
		}
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
	FILE *fp4 = fopen("results/f130.dta", "w");
	for (int j = 0; j < num_lines; j++) {
		fprintf(fp4, "%f\n", prediction[j]);

	}
	fclose(fp4);

	return 0;
}

