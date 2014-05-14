#include <stdio.h>
#include <stdlib.h>

int main(){
	// Iterate through all lines
	FILE *fp =  fopen("../../netflix/mu/all.dta", "r");
	FILE *fp2 = fopen("../../netflix/mu/all.idx", "r");
	if (fp == NULL || fp2 == NULL) {
		return -1;
	}

	char str[60];
	char str2[5];
	char str3[10];

	// Load in features
	// Add one because we're not using the first cell of both userValue and movieValue
	// to avoid off by one confusion
	int num_users = 458293 + 1;
	int num_movies = 17770 + 1;
	
	float * userValue = calloc(num_users, sizeof(float));
	float * movieValue = calloc(num_movies, sizeof(float));
	if (userValue == NULL || movieValue == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	for (int i = 0; i < num_users; i++) {
		userValue[i] = 0.1;
	}
	for (int i = 0; i < num_movies; i++) {
		movieValue[i] = 0.1;
	}


	// Load movie data and baseline into array
	int num_lines = 99666408;
	// int num_lines = 10;
	int * movie_data = calloc(num_lines * 3, sizeof(int));
	if (movie_data == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	float * res_data = calloc(num_lines), sizeof(float));
	if (res_data == NULL) {
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
	int count = 0;

	printf("\n--------------Training Feature --------------\n");
	int user, movie;
	float rating, baseline, predict, res, err, uv, mv;
	for (int i = 0; i < 121; i++) {
		printf("\n Epoch %d\n", i);
		for (int j = 0; j < num_lines; j++) {
			line_number = j * 3;
			user = movie_data[line_number];
			movie = movie_data[line_number + 1];
			rating = (float)movie_data[line_number + 2];
			// printf("User %d Movie %d Rating %d Baseling %f\n", user, movie, rating, baseline);
			uv = userValue[user];
			mv = movieValue[movie];
			predict = uv * mv;
			err = rating - predict;
			if (i == 120) {
				res_data[j] = err;
			} else {
				userValue[user] += 0.001 * (err * mv - 0.02 * uv);
				movieValue[movie] += 0.001 * (err * uv - 0.02 * mv);
			}
			
			// printf("Predict %f Error %f\n", predict, err);
			// printf("UserF %f MovieF %f\n", userValue[user], movieValue[movie]);
		}
	}

	printf("\n----------Saving Residuals----------\n");
	// Save residuals
	FILE *fp4 = fopen("../../res/res_f4.dta", "w");
	for (int j = 0; j < num_lines; j++) {
		fprintf(fp4, "%f\n", res_data[j]);
	}

	printf("\n-----------Saving features-----------\n");
	// Save features
	FILE *fp5 = fopen("features/feature_m1.dta", "w");
	for (int f = 1; f < num_movies; f++) {
		fprintf(fp5, "%f\n", movieValue[f]);
	}

	FILE *fp6 = fopen("features/feature_u1.dta", "w");
	for (int f = 1; f < num_users; f++) {
		fprintf(fp6, "%f\n", userValue[f]);
	}

	//fclose(fp4);
	fclose(fp5);
	fclose(fp6);
	return 0;
}
