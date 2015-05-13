#include <stdio.h>
#include <stdlib.h>

int main(){
	// Iterate through all lines
	FILE *fp;
	FILE *fp2;
	FILE *fp3;
	fp = fopen("../../netflix/mu/all.dta", "r");
	fp2 = fopen( "../../netflix/mu/all.idx", "r");
	fp3 = fopen("../../res/res_f3.dta", "r");
	if (fp == NULL || fp2 == NULL || fp3 == NULL) {
		return -1;
	}
	char str[60];
	char str2[5];
	char str3[15];

	int num_lines = 99666408;
	//int num_lines = 10000;
	
	printf("\n----------Loading res data-------------\n");
	// Load res data
	float * res_data = calloc(num_lines, sizeof(float));
	if (res_data == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	int count = 0;
	while (fgets(str, 60, fp3) != NULL) {
		res_data[count] = atof((char *)strtok(str, " "));
		count++;
		if (count % 10000000 == 0) {
			printf("%d\n", count);
		}
	}
	fclose(fp3);
	

	// int num_lines = 10;
	int * movie_data = calloc(num_lines * 3, sizeof(int));
	if (movie_data == NULL) {
		printf("Malloc failed\n");
		return -1;
	}

	// Load movie data
	int line_number = 0;
	int sample = 0;
	printf("\n----------Loading movie data-------------\n");
	while (fgets(str, 60, fp) != NULL && fgets(str2, 5, fp2) != NULL) {
		// Only use index < 5
		if (atoi((char *)strtok(str2, " ")) == 5) {
			continue;
		}

		// Get user, movie, rating
		movie_data[line_number] = atoi((char *)strtok(str, " "));
		movie_data[line_number + 1] = atoi((char *)strtok(NULL, " "));
		strtok(NULL, " ");
		movie_data[line_number + 2] = atoi((char *)strtok(NULL, " "));

		line_number += 3;
		sample++;
		if (sample % 10000000 == 0) {
			printf("%d\n", sample);
		}
	}
	fclose(fp);
	fclose(fp2);




	printf("\n--------------Training Features --------------\n");
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
	int user, movie, rating;
	float old_res, predict, res, err, uv, mv;
	char movie_feature_file[50];
	char user_feature_file[50];
	
	// Train each feature
	for (int f = 4; f < 101; f++) {
		
		printf("\n-------Feature %d -----------------\n", f);
		// Initialize feature to 0.1
		for (int i = 0; i < num_users; i++) {
			userValue[i] = 0.1;
		}
		for (int i = 0; i < num_movies; i++) {
			movieValue[i] = 0.1;
		}
		for (int i = 0; i < 121; i++) {
			printf("\n Epoch %d\n", i);
			for (int j = 0; j < num_lines; j++) {
				old_res = res_data[j];
				line_number = j * 3;
				user = movie_data[line_number];
				movie = movie_data[line_number + 1];
				rating = movie_data[line_number + 2];
				// printf("User %d Movie %d Rating %d Baseling %f\n", user, movie, rating, baseline);
				uv = userValue[user];
				mv = movieValue[movie];
				predict = rating - old_res + uv * mv;
				err = rating - predict;
				if (i == 120) {
					res_data[j] = err;
				}
				else {
					userValue[user] += 0.001 * (err * mv - 0.02 * uv);
					movieValue[movie] += 0.001 * (err * uv - 0.02 * mv);
				}
			}
		}
		FILE *fp5, *fp6;
		// Save feature
		printf("\n-----------Saving features-----------\n");
		// Save features
		sprintf(movie_feature_file, "features/feature_m%d.dta", f);
		fp5 = fopen(movie_feature_file, "w");
		for (int f = 1; f < num_movies; f++) {
			fprintf(fp5, "%f\n", movieValue[f]);
		}
		fclose(fp5);
		sprintf(user_feature_file, "features/feature_u%d.dta", f);
		fp6 = fopen(user_feature_file, "w");
		for (int f = 1; f < num_users; f++) {
			fprintf(fp6, "%f\n", userValue[f]);
		}
		fclose(fp6);
	}

	// Save res for last feature
	printf("\n----------Saving Residuals----------\n");
	// Save residuals
	FILE *fp4 = fopen("../../res/res_f100.dta", "w");
	for (int j = 0; j < num_lines; j++) {
		fprintf(fp4, "%f\n", res_data[j]);

	}
	fclose(fp4);

	return 0;
}

