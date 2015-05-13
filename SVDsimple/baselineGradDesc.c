#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Constants for operation*/
// Add one because we're not using the first cell of both userValue and movieValue
// to avoid off by one confusion
static int num_users = 458293 + 1;
static int num_movies = 17770 + 1;
static int num_lines = 99666408;
static int epochs = 20; // number of iterations to do over whole data set
static float GLOBAL_AVG = 3.609516;


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

// Loads movie data into a 1 d array
// storing user, movie, rating
static inline
int * loadMovieData() {
	printf("----------Loading data-------------\n");
	int line_number = 0;
	int count = 0;
	// Iterate through all lines
	char str[60];
	char str2[5];
	FILE *fp =  fopen("../../netflix/mu/all.dta", "r");
	FILE *fp2 = fopen("../../netflix/mu/all.idx", "r");
	if (fp == NULL || fp2 == NULL) {
		return NULL;
	}

	int * movie_data = calloc(num_lines * 3, sizeof(int));
	if (movie_data == NULL) {
		printf("Malloc failed\n");
		return -1;
	}

	while (fgets(str, 60, fp) != NULL && fgets(str2, 5, fp2) != NULL) {
		// Only use index < 5
		if (atoi(strtok(str2, " ")) == 5) {
			continue;
		}

		// Get user, movie, rating, skip date value
		movie_data[line_number] = atoi(strtok(str, " "));
		movie_data[line_number + 1] = atoi(strtok(NULL, " "));
		strtok(NULL, " ");
		movie_data[line_number + 2] = atoi(strtok(NULL, " "));
		
		line_number += 3;
		count++;
		// if (count % 10000000 == 0) {
		// 	printf("%d\n", count);
		// }
	}
	fclose(fp);	
	fclose(fp2);
	return movie_data;
}

// Load offsets into 1d array for movie and user
static inline 
void loadData(char * file, float * data) {
	FILE *fp = fopen(file, "r");
	int count = 1;
	char str[60];
	if (fp == NULL) {
		return;
	}
	printf("----------Loading data from file %s-------------\n", file);
	while (fgets(str, 60, fp) != NULL) {
		data[count] = atof(str);		
		//printf("%f\n", data[count]);
		// if (count % 10000 == 0) {
		// 	printf("%d\n", count);
		// }
		count++;
	}	
}
int main(){
	float * userOffset = calloc(num_users, sizeof(float));
	float * movieOffset = calloc(num_movies, sizeof(float));
	if (userOffset == NULL || movieOffset == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	loadData("../stats/user_offset_reg2.dta", userOffset);
	loadData("../stats/movie_offset_reg.dta", movieOffset);
	int * movie_data = loadMovieData();
	// printf("test %f %f %f %f\n", userOffset[5], userOffset[num_users - 1], movieOffset[5], movieOffset[num_movies - 1]);
	
	printf("--------------Training Offsets --------------\n");
	int user, movie, line_number;
	float rating, baseline, predict, res, err, uv, mv, total_err;
	for (int i = 0; i < epochs; i++) {
		//printf("\n Epoch %d\n", i);
		total_err = 0;
		for (int j = 0; j < num_lines; j++) {
			line_number = j * 3;
			user = movie_data[line_number];
			movie = movie_data[line_number + 1];
			rating = (float)movie_data[line_number + 2];
			// printf("User %d Movie %d Rating %d Baseling %f\n", user, movie, rating, baseline);
			uv = userOffset[user];
			mv = movieOffset[movie];
			predict = clipScore(GLOBAL_AVG + uv + mv);
			err = rating - predict;

			// Update baseline
			userOffset[user] += 0.007 * (err - 0.005 * uv);
			movieOffset[movie] += 0.007 * (err - 0.005 * mv);

			// printf("Predict %f Error %f\n", predict, err);
			// printf("UserF %f MovieF %f\n", userValue[user], movieValue[movie]);
		}
	}

	printf("-----------Saving offsets-----------\n");
	// // Save features
	FILE *fp = fopen("features/movie_offset.dta", "w");
	for (int f = 1; f < num_movies; f++) {
		fprintf(fp, "%f\n", movieOffset[f]);
	}
	fclose(fp);
	fp = fopen("features/user_offset.dta", "w");
	for (int f = 1; f < num_users; f++) {
		fprintf(fp, "%f\n", userOffset[f]);
	}
	fclose(fp);


	free(userOffset);
	free(movieOffset);
	free(movie_data);
	return 0;
}
