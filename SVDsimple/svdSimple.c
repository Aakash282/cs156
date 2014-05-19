#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Constants for operation*/
// Add one because we're not using the first cell of both userValue and movieValue
// to avoid off by one confusion
static int num_users = 458293 + 1;
static int num_movies = 17770 + 1;
static int num_lines = 99666408;
static int epochs = 30; // number of iterations to do over whole data set
static float GLOBAL_AVG = 3.609516;
static int num_features = 50;

// Learning parameters
float gamma1 = 0.007;
float gamma2 = 0.007;
float gamma3 = 0.001;
float lambda6 = 0.005;
float lambda7 = 0.015;
float lambda8 = 0.015;
float gamma_step = 0.9; // Decrease gammas by this factor each iteration


// Movie data array
int * movie_data;
// Baseline offset arrays
float * userOffset;
float * movieOffset;
// 1d array containing pointer to feature vector for each user or movie
// Each array is a vector of pointers to feature vectors for each user or movie
// [u1 u2 ... un]
float ** userFeatures;
// [m1 m2 ... mn]
float ** movieFeatures;

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
void loadMovieData() {
	printf("\n----------Loading movie data-------------\n");
	int line_number = 0;
	int count = 0;
	// Iterate through all lines
	char str[60];
	char str2[5];
	FILE *fp =  fopen("../../netflix/mu/all.dta", "r");
	FILE *fp2 = fopen("../../netflix/mu/all.idx", "r");
	if (fp == NULL || fp2 == NULL) {
		return;
	}

	movie_data = calloc(num_lines * 3, sizeof(int));
	if (movie_data == NULL) {
		printf("Malloc failed\n");
		return;
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
}

// Load offsets into 1d array for movie and user
void loadData(char * file, float * data) {
	FILE *fp = fopen(file, "r");
	int count = 1;
	char str[60];
	if (fp == NULL) {
		return;
	}
	printf("\n----------Loading data from file %s-------------\n", file);
	while (fgets(str, 60, fp) != NULL) {
		data[count] = atof(str);		
		//printf("%f\n", data[count]);
		// if (count % 10000 == 0) {
		// 	printf("%d\n", count);
		// }
		count++;
	}	
}

void initializeUserFeatures() {
	// Malloc space
	userFeatures = calloc(num_users, sizeof(float *));
	if (userFeatures == NULL) {
		printf("Malloc failed\n");
	}
	for (int i=0; i < num_users; i++) {
		userFeatures[i] = calloc(num_features, sizeof(float));
	}

	// Initialize values
	for (int i=0; i < num_users; i++) {
		for (int j=0; j < num_features; j++) {
			userFeatures[i][j] = -1.0 + 2.0 * (float) rand() / RAND_MAX;
		}
	}
}

void initializeMovieFeatures() {
	// Malloc space
	movieFeatures = calloc(num_movies, sizeof(float *));
	if (movieFeatures == NULL) {
		printf("Malloc failed\n");
	}
	for (int i=0; i < num_movies; i++) {
		movieFeatures[i] = calloc(num_features, sizeof(float));
	}

	// Initialize values
	for (int i=0; i < num_movies; i++) {
		for (int j=0; j < num_features; j++) {
			movieFeatures[i][j] = -1.0 + 2.0 * (float) rand() / RAND_MAX;
		}
	}
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

static inline
void updateFeatures(int user, int movie, float err) {
	// Update each feature using gradient descent
	float uv, mv;
	for (int i = 0; i < num_features; i++) {
		uv = userFeatures[user][i];
		mv = movieFeatures[movie][i];
		movieFeatures[movie][i] += gamma2 *(err * uv - 0.015 * mv);
		userFeatures[user][i] += gamma2 * (err * mv - 0.015 * uv);
	}	
}

static inline
void updateBaseline(int user, int movie, float err) {
	// Update each baseline offset using gradient descent
	float uv = userOffset[user];
	float mv = movieOffset[movie];
	userOffset[user] += gamma1 * (err - 0.005 * uv);
	movieOffset[movie] += gamma1 * (err - 0.005 * mv);	
}

void saveOffsets() {
	// Save baseline offsets
	FILE *fp = fopen("features/movie_offset050_r.dta", "w");
	for (int f = 1; f < num_movies; f++) {
		fprintf(fp, "%f\n", movieOffset[f]);
	}
	fclose(fp);
	fp = fopen("features/user_offset050_r.dta", "w");
	for (int f = 1; f < num_users; f++) {
		fprintf(fp, "%f\n", userOffset[f]);
	}
	fclose(fp);
}

void saveUserFeatures(char * file) {
	// Save user features in one file formatted as follows
	// 		f1 f2 ... 
	//	u1
	//  u2	
	FILE *fp = fopen(file, "w");
	for (int i=1; i < num_users; i++) {
		for (int j=0; j < num_features; j++) {
			fprintf(fp, "%f ", userFeatures[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}
void saveMovieFeatures(char * file) {
	// Save movie features in one file formatted as follows
	// 		f1 f2 ... 
	//	m1
	//  m2	
	FILE *fp = fopen(file, "w");
	for (int i=1; i < num_movies; i++) {
		for (int j=0; j < num_features; j++) {
			fprintf(fp, "%f ", movieFeatures[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}
int main(){
	// Load Movie data
	loadMovieData();
	// printf("test %f %f %f %f\n", userOffset[5], userOffset[num_users - 1], movieOffset[5], movieOffset[num_movies - 1]);
	
	// Set up offsets
	userOffset = calloc(num_users, sizeof(float));
	movieOffset = calloc(num_movies, sizeof(float));
	if (userOffset == NULL || movieOffset == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	loadData("../stats/user_offset.dta", userOffset);
	loadData("../stats/movie_offset.dta", movieOffset);


	// Initialize features
	initializeUserFeatures();
	initializeMovieFeatures();

	printf("\n--------------Training --------------\n");
	int user, movie, line_number;
	float rating, predict, err;
	float total_err = 0;
	for (int i = 1; i <= epochs; i++) {
		total_err = 0;
		for (int j = 0; j < num_lines; j++) {
			line_number = j * 3;
			user = movie_data[line_number];
			movie = movie_data[line_number + 1];
			rating = (float)movie_data[line_number + 2];
			// printf("User %d Movie %d Rating %d Baseling %f\n", user, movie, rating, baseline);

			predict = predictRating(user, movie);
			err = rating - predict;
			total_err += err * err;
			updateFeatures(user, movie, err);
			updateBaseline(user, movie, err);
		}
		// Update gammas by factor 
		gamma1 *= gamma_step;
		gamma2 *= gamma_step;
		printf("Epoch %d RMSE: %f %f\n", i, sqrt(total_err / num_lines), total_err);
	}

	printf("\n-----------Saving features-----------\n");
	saveOffsets();
	saveUserFeatures("features/user_features_050_r.dta");
	saveMovieFeatures("features/movie_features_050_r.dta");

	free(userOffset);
	free(movieOffset);
	free(movie_data);
	return 0;
}
