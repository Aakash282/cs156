#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
/* Constants for operation*/
// Add one because we're not using the first cell of both userValue and movieValue
// to avoid off by one confusion
static unsigned int num_users = 458293 + 1;
static unsigned int num_movies = 17770 + 1;
static unsigned int num_lines = 99666408;
//static int num_lines = 100;
static unsigned int epochs = 20; // number of iterations to do over whole data set
static float GLOBAL_AVG = 3.609516;
static unsigned int num_features = 10;

// Learning parameters
float gamma1 = 0.007;
float gamma2 = 0.007;
float gamma3 = 0.001;
float lambda6 = 0.005;
float lambda7 = 0.015;
float lambda8 = 0.015;
float gamma_step = 0.9; // Decrease gammas by this factor each iteration


// Movie data array
unsigned int * movie_data;
// Baseline offset arrays
float * userOffset;
float * movieOffset;

// implict data array, each line contains number of ratings, n and 1/sqrt(n) for each user
float * userImplicitData;

// contains movied rated by each user
unsigned int ** userImplicitMovies;

// Save contribution of all implicit weights for each user to save computation time
float * implicitC;

// 1d array containing pointer to feature vector for each user or movie
// Each array is a vector of pointers to feature vectors for each user or movie
// [u1 u2 ... un]
float ** userFeatures;
// [m1 m2 ... mn]
float ** movieFeatures;
// [m1 m2 ... mn]
float ** implicitFeatures;

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
	printf("----------Loading movie data-------------\n");
	int line_number = 0;
	int count = 0;
	// Iterate through all lines
	char str[60];
	char str2[5];
	FILE *fp =  fopen("../../netflix/um/all.dta", "r");
	FILE *fp2 = fopen("../../netflix/um/all.idx", "r");
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

// Load implict user data
void loadUserImplicit(char * file, float * data) {
	FILE *fp = fopen(file, "r");
	int count = 2;
	char str[60];
	if (fp == NULL) {
		return;
	}
	printf("----------Loading data from file %s-------------\n", file);
	while (fgets(str, 60, fp) != NULL) {
		data[count] = atof(strtok(str, "\t"));
		data[count + 1]	= atof(strtok(NULL, "\t"));	
		// if (count % 10000 == 0) {
		// 	printf("%d\n", count);
		// }
		count +=2;
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
			userFeatures[i][j] = (-1.0 + 2* (float) rand() / RAND_MAX) / 25.0;;
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
			movieFeatures[i][j] = (-1.0 + 2* (float) rand() / RAND_MAX) / 25.0;;
		}
	}
}

void initializeImplicitFeatures() {
	// Malloc space
	implicitFeatures = calloc(num_movies, sizeof(float *));
	if (implicitFeatures == NULL) {
		printf("Malloc failed\n");
	}
	for (int i=0; i < num_movies; i++) {
		implicitFeatures[i] = calloc(num_features, sizeof(float));
	}

	// Initialize values
	for (int i=0; i < num_movies; i++) {
		for (int j=0; j < num_features; j++) {
			implicitFeatures[i][j] = (-1.0 + 2* (float) rand() / RAND_MAX) / 25.0;
		}
	}
}

// Load in list of each movies that each user has rated
void initializeImplicitMovies(char * file) {
	userImplicitMovies = calloc(num_users, sizeof(int));
	implicitC = calloc(num_features, sizeof(float));
	if (userImplicitMovies == NULL || implicitC == NULL) {
		printf("Malloc failed\n");
	}
	int u_num_movies = 0;
	for (int i=1; i < num_users; i++) {
		u_num_movies = userImplicitData[i*2];
		userImplicitMovies[i] = calloc(u_num_movies, sizeof(int));
	}	
	FILE *fp = fopen(file, "r");
	int max_b_size = 100000;
	
	char * success;
	
	printf("----------Loading data from file %s------\n", file);
	for (int i=1; i < num_users; i++) {
		char * str = calloc(max_b_size, sizeof(char *));
		success = fgets(str, max_b_size, fp);
		if (success != NULL) {
			u_num_movies = userImplicitData[i*2];
			userImplicitMovies[i][0] = atoi(strtok(str, " ")); 
			for (int j=1; j < u_num_movies; j++) {
				userImplicitMovies[i][j] = atoi(strtok(NULL, " "));
			}
		}
		free(str);
	}
	u_num_movies = (int) userImplicitData[(num_users-1)*2];
	printf("test %d %d\n", u_num_movies, userImplicitMovies[num_users - 1][631]);
	
}

static inline
float predictRating(int user, int movie) {
	// Contribution from features user_f1 * movie_f1 + user_f2 * movie_f2 + ...
	float feature_c = 0;
	// Contribution from implicit features y1 + y2 + ... saved in implicitC
	// normalized by 1/sqrt(n) saved in 2nd column of userImplicitMovies

	for (int i = 0; i < num_features; i++) {
		feature_c += (userFeatures[user][i] + implicitC[i])* movieFeatures[movie][i];
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
		movieFeatures[movie][i] += gamma2 *(err * (uv + implicitC[i]) - 0.015 * mv);
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

static inline
void getImplicitC(int user) {

	// Clear array

	for (int j=0; j < num_features; j ++) {
		implicitC[j] = 0;
	}
	int u_num_movies = userImplicitData[user*2];
	int u_movie;
	for (int i=0; i < u_num_movies; i++) {
		u_movie = userImplicitMovies[user][i];
		for (int j=0; j < num_features; j ++) {
			implicitC[j] += implicitFeatures[u_movie][j];
		}
	}
	float n = userImplicitData[user*2 + 1];
	for (int j=0; j < num_features; j ++) {
		implicitC[j] *= n;
	}

}
static inline
void updateImplicitFeatures(int user, float err) {
	int u_num_movies = userImplicitData[user*2];
	int u_movie;
	float n = userImplicitData[user*2 + 1];
	float temp;
	// For each movie vector that user has seen
	for (int i = 0; i < u_num_movies; i++) {
		// get movie and iterate update movie feature vector
		u_movie = userImplicitMovies[user][i];
		for (int j=0; j < num_features; j++) {
			temp = implicitFeatures[u_movie][j];
			implicitFeatures[u_movie][j] += gamma2*(err *n * userFeatures[user][j] - lambda7 * temp);
		}
	}

}


void saveOffsets() {
	// Save baseline offsets
	FILE *fp = fopen("f010_e020/movie_offset.dta", "w");
	for (int f = 1; f < num_movies; f++) {
		fprintf(fp, "%f\n", movieOffset[f]);
	}
	fclose(fp);
	fp = fopen("f010_e020/user_offset.dta", "w");
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

void saveImplicitFeatures(char * file) {
	FILE * fp = fopen(file, "w");
	for (int i=1; i < num_movies; i++) {
		for (int j=0; j < num_features; j++) {
			fprintf(fp, "%f ", implicitFeatures[i][j]);
		}
		fprintf(fp, "\n");
	}	
	fclose(fp);
}

int main(){
	// Load Movie data
	loadMovieData();
	// printf("test %f %f %f %f\n", userOffset[5], userOffset[num_users - 1], movieOffset[5], movieOffset[num_movies - 1]);
	
	// Set up offsets and implicit data
	userOffset = calloc(num_users, sizeof(float));
	movieOffset = calloc(num_movies, sizeof(float));
	userImplicitData = calloc(num_users*2, sizeof(float));
	if (userOffset == NULL || movieOffset == NULL || userImplicitData == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	loadData("../stats/user_offset_reg2.dta", userOffset);
	loadData("../stats/movie_offset_reg.dta", movieOffset);
	loadUserImplicit("../stats/user_implicit_2.dta", userImplicitData);
	
	// Initialize features
	// Initialize random seed
	srand (time(NULL));
	initializeUserFeatures();
	initializeMovieFeatures();
	initializeImplicitMovies("../../implicit/user_implicit_movies.dta");
	//printf("test %f\n", userImplicitMovies[num_users - 1][2]);
	initializeImplicitFeatures();

	printf("\n--------------Training --------------\n");
	int user, movie, line_number;
	float rating, predict, err;
	float total_err;
	for (int i = 1; i <= epochs; i++) {
		total_err = 0;
		for (int j = 0; j < num_lines; j++) {
			line_number = j * 3;
			user = movie_data[line_number];
			movie = movie_data[line_number + 1];
			rating = (float)movie_data[line_number + 2];
			// printf("User %d Movie %d Rating %d Baseling %f\n", user, movie, rating, baseline);
			getImplicitC(user);
			predict = predictRating(user, movie);
			err = rating - predict;
			total_err += err * err;
			updateFeatures(user, movie, err);
			updateBaseline(user, movie, err);
			updateImplicitFeatures(user, err);

		}
		// Update gammas by factor
		gamma1 *= gamma_step;
		gamma2 *= gamma_step;
		printf("Epoch %d RMSE: %f\n", i, sqrt(total_err / num_lines));
	}

	printf("-----------Saving features-----------\n");
	saveOffsets();
	saveUserFeatures("f010_e020/user_features.dta");
	saveMovieFeatures("f010_e020/movie_features.dta");
	saveImplicitFeatures("f010_e020/implicit_features.dta");

	free(userOffset);
	free(movieOffset);
	free(movie_data);
	return 0;
}
