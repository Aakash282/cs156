#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Constants for operation*/
// Add one because we're not using the first cell of both userValue and movieValue
// to avoid off by one confusion
static int num_users = 458293 + 1;
static int num_movies = 17770 + 1;
//static int num_lines = 2749898; // num lines in qual data
static int num_lines = 1374739; // num lines in probe;
static float GLOBAL_AVG = 3.609516;
static int num_features = 220;

// Movie data array
int * movie_data;

// Indices for what set each data point belongs to
int * index;
// Baseline offset arrays
float * userOffset;
float * movieOffset;

// 1d array containing pointer to feature vector for each user or movie
// Each array is a vector of pointers to feature vectors for each user or movie
// [u1 u2 ... un]
float ** userFeatures;
// [m1 m2 ... mn]
float ** movieFeatures;

// prediction array
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

// Loads movie data into a 1 d array
// storing user, movie, rating
void loadMovieData(char * file) {
	printf("----------Loading movie data-------------\n");
	int line_number = 0;
	int count = 0;
	// Iterate through all lines
	char str[60];
	FILE *fp =  fopen(file, "r");
	if (fp == NULL) {
		return;
	}

	movie_data = calloc(num_lines * 2, sizeof(int));
	index = calloc(num_lines, sizeof(int));
	if (movie_data == NULL || index == NULL) {
		printf("Malloc failed\n");
		return;
	}

	while (fgets(str, 60, fp) != NULL) {

		// Get user, movie, rating, skip date value
		movie_data[line_number] = atoi(strtok(str, " "));
		movie_data[line_number + 1] = atoi(strtok(NULL, " "));
		
		line_number += 2;
		count++;
		// if (count % 1000000 == 0) {
		// 	printf("%d\n", count);
		// }
	}
	fclose(fp);	
}

// Load offsets into 1d array for movie and user
void loadData(char * file, float * data) {
	FILE *fp = fopen(file, "r");
	int count = 1;
	char str[60];
	if (fp == NULL) {
		return;
	}
	printf("----------Loading data from file %s--------\n", file);
	while (fgets(str, 60, fp) != NULL) {
		data[count] = atof(str);		
		//printf("%f\n", data[count]);
		// if (count % 10000 == 0) {
		// 	printf("%d\n", count);
		// }
		count++;
	}	
}


// Load in feature data into a 2d array
// Each line in the file represents the features of a movie or user
// Num items is the number of users or movies
void loadFeatures(char * file, int num_items, float ** data) {

	for (int i=0; i < num_items; i++) {
		data[i] = calloc(num_features, sizeof(float));
	}
	// Open file and set up values
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		printf("Couldn't open file %s\n", file);
		return;
	}
	char str[5000];
	int item_n = 1;
	printf("----------Loading data from file %s--------\n", file);
	// For each user or movie on each line
	while (fgets(str, 5000, fp) != NULL) {
		// Get first feature
		data[item_n][0] = atof(strtok(str, " "));
		// Get other features
		for (int i=1; i < num_features; i++)
			data[item_n][i] = atof(strtok(NULL, " "));
		item_n ++;
	}
	printf("test %f\n", data[num_items-1][num_features-1]);
}

static inline
float predictRating(int user, int movie) {
	// Contribution from features user_f1 * movie_f1 + user_f2 * movie_f2 + ...
	float feature_c = 0;
	for (int i = 0; i < num_features; i++) {
		feature_c += userFeatures[user][i] * movieFeatures[movie][i];
	}
	return GLOBAL_AVG + userOffset[user] + movieOffset[movie] + feature_c;
}

void saveResults(char * file) {
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		printf("Saving results failed\n");
		return;
	}
	for (int i=0; i < num_lines; i++) {
		fprintf(fp, "%f\n", predictions[i]);
	}


}

int main(){
	// Load Movie data
	//loadMovieData("../../netflix/um/qual.dta");
	loadMovieData("../stats/probe.dta");
	printf("test %d\n", movie_data[20]);
	// printf("test %f %f %f %f\n", userOffset[5], userOffset[num_users - 1], movieOffset[5], movieOffset[num_movies - 1]);
	
	// Set up offsets
	userOffset = calloc(num_users, sizeof(float));
	movieOffset = calloc(num_movies, sizeof(float));
	if (userOffset == NULL || movieOffset == NULL) {
		printf("Malloc failed\n");
		return -1;
	}


	// Set up feature arrays for user and movies
	userFeatures = calloc(num_users, sizeof(float *));
	movieFeatures = calloc(num_movies, sizeof(float *));
	if (userFeatures == NULL || movieFeatures == NULL) {
		printf("Malloc failed\n");
		return -1;
	}


	// Load offsets
	loadData("features/f220_e060_t/user_offset.dta", userOffset);
	loadData("features/f220_e060_t/movie_offset.dta", movieOffset);


	// Load featuress
	loadFeatures("features/f220_e060_t/user_features.dta", num_users, userFeatures);
	loadFeatures("features/f220_e060_t/movie_features.dta", num_movies, movieFeatures);
	printf("Test %f %f\n", userFeatures[1][num_features - 1], userFeatures[num_users-1][num_features-1]);

	predictions = calloc(num_lines, sizeof(float));
	if (predictions == NULL) {
		return -1;
	}

	printf("\n--------------Predicting --------------\n");
	int user, movie, line_number;
	int count = 0;
	for (int j = 0; j < num_lines; j++) {
		line_number = j * 2;
		user = movie_data[line_number];
		movie = movie_data[line_number + 1];
		predictions[count] = clipScore(predictRating(user, movie));
		count++;
	}
	

	printf("\n-----------Saving results-----------\n");
	saveResults("results/um_probe_f220_e060_t.dta");

	free(userOffset);
	free(movieOffset);
	free(movie_data);
	return 0;
}
