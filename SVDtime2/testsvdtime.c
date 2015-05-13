#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/* Constants for operation*/
// Add one because we're not using the first cell of both userValue and movieValue
// to avoid off by one confusion
static int num_users = 458293 + 1;
static int num_movies = 17770 + 1;
static int num_lines = 2749898; // num lines in qual data
//static int num_lines = 1374739; // num lines in probe;
static unsigned int total_size = 102416306;
static float GLOBAL_AVG = 3.609516;
static int num_features = 50;

static unsigned int num_time_bins = 30; // number of bins to break time into
static unsigned int bin_size = 75; // number of consecutive days each bin tracks

// Movie data array
int * movie_data;

// Indices for what set each data point belongs to
int * index;
// Baseline offset arrays
float * userOffset;
float * movieOffset;

// implict data array, each line contains number of ratings, n and 1/sqrt(n) for each user
float * userImplicitData;

// contains movied rated by each user
int ** userImplicitMovies;

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

// Float containg average date for each user and number of different dates
float * timeAvg;

// Bias for each time bin
float ** itemBinBias;

// prediction array
float * predictions;

// Contains devation for each different date of each user along with user time bias
// u1: dev1 b1 dev2 b2 ...
float ** userDev;

// user alphas for time deviation
float * userAlphas;

// keep track of training error
float * train_errs;

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
	unsigned int line_number = 0;
	unsigned int count = 0;
	// Iterate through all lines
	char str[60];
	FILE *fp =  fopen(file, "r");
	if (fp == NULL) {
		return;
	}

	movie_data = calloc(total_size * 4, sizeof(unsigned int));
	//idx = calloc(num_lines, sizeof(unsigned int));
	if (movie_data == NULL) {
		printf("Malloc failed\n");
		return;
	}

	while (fgets(str, 60, fp) != NULL) {
		//idx[count] = index_v;
		// Get user, movie, date, and rating
		movie_data[line_number] = atoi(strtok(str, " "));
		movie_data[line_number + 1] = atoi(strtok(NULL, " "));
		movie_data[line_number + 2] = atoi(strtok(NULL, " "));
		movie_data[line_number + 3] = atoi(strtok(NULL, " "));
		
		line_number += 4;
		count++;
		// if (count % 10000000 == 0) {
		// 	printf("%d\n", count);
		// }
		// if (count == 275) {
		// 	break;
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
void loadFeatures(char * file, int num_items, int num_f, float ** data) {

	for (int i=0; i < num_items; i++) {
		data[i] = calloc(num_f, sizeof(float));
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
		for (int i=1; i < num_f; i++)
			data[item_n][i] = atof(strtok(NULL, " "));
		item_n ++;
	}
	printf("test %f\n", data[num_items-1][num_f-1]);
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
	// if (count != num_users * 2) {
	// 	printf("error reading userImplicit %f\n", count);
	// }
}

void loadFileAvg(char * file) {
	timeAvg = calloc(num_users, sizeof(float));
	FILE * fp = fopen(file, "r");
	char str[60];
	int count = 1;
	printf("----------Loading data from file %s------\n", file);
	while (fgets(str, 60, fp) != NULL) {
		strtok(str, " ");
		timeAvg[count] = atof(strtok(NULL, " "));
		count ++;
	}
	printf("test %f\n", timeAvg[num_users-1]);
	fclose(fp);

}

// Load user deviations for user average time
// also store biases for each deviation
void loadUserDev(char* devfile, char* biasfile) {
	userDev = calloc(num_users, sizeof(float *));
	int num_dates = 0;

	for (int i=1; i < num_users; i++) {
		num_dates = (int) timeAvg[i];
		//printf("num_dates %d\n", num_dates);
		userDev[i] = calloc(num_dates*2, sizeof(float));
		if (userDev[i] == NULL) {
			printf("user dev malloc failed \n");
			exit(1);
		}
	}

	// Load user deviations
	FILE *fp = fopen(devfile, "r");
	char str[50000];
	int user = 1;
	printf("----------Loading data from file %s------\n", devfile);
	while (fgets(str, 50000, fp) != NULL) {
		num_dates = (int) timeAvg[user];
		// get first dev
		userDev[user][0] = atof(strtok(str, " "));
		for (int i=1; i < num_dates; i++) {
			userDev[user][i*2] = atof(strtok(NULL, " "));
		}
		user ++;
	}
	if (user != num_users) {
		printf("read from dev failed\n");
		exit(1);
	}
	fclose(fp);
	printf("test userDev user 23 date count 343 %f\n", userDev[23][342*2]);

	// Load user time biases into same array
	fp = fopen(biasfile, "r");
	user = 1;
	printf("----------Loading data from file %s------\n", biasfile);
	while (fgets(str, 50000, fp) != NULL) {
		num_dates = (int) timeAvg[user];
		// get first time bias
		userDev[user][1] = atof(strtok(str, " "));
		for (int i=1; i < num_dates; i++) {
			userDev[user][i*2+1] = atof(strtok(NULL, " "));
		}
		user ++;
	}
	if (user != num_users) {
		printf("read from dev failed\n");
		exit(1);
	}
	fclose(fp);
	printf("test user time bias user 23 date count 343 %f\n", userDev[23][342*2+1]);
} 

// Load in list of each movies that each user has rated
void initializeImplicitMovies(char * file) {
	userImplicitMovies = calloc(num_users, sizeof(int *));
	implicitC = calloc(num_features, sizeof(float));
	if (userImplicitMovies == NULL || implicitC == NULL) {
		printf("Malloc failed\n");
	}
	int u_num_movies = 0;
	for (int i=1; i < num_users; i++) {
		u_num_movies = userImplicitData[i*2];
		userImplicitMovies[i] = calloc(u_num_movies, sizeof(int));
		if (userImplicitMovies[i] == NULL) {
			printf("Malloc failed\n");
		}
	}	
	FILE *fp = fopen(file, "r");
	int max_b_size = 50000; // buffer size
	
	char * success;
	char str[50000];
	printf("----------Loading data from file %s------\n", file);
	for (int i=1; i < num_users; i++) {
		if (str == NULL) {
			printf("malloc failed\n");
		}
		success = fgets(str, max_b_size, fp);
		if (success != NULL) {
			u_num_movies = userImplicitData[i*2];
			userImplicitMovies[i][0] = atoi(strtok(str, " ")); 
			for (int j=1; j < u_num_movies; j++) {
				userImplicitMovies[i][j] = atoi(strtok(NULL, " "));
			}
		} else {
			printf("Data read failed\n");
		}
	}
	u_num_movies = (int) userImplicitData[(num_users-1)*2];
	printf("test %d %d\n", u_num_movies, userImplicitMovies[num_users - 1][631]);
	
}

// Calculate once per user, the sum of his implicit features
static inline
void getImplicitC(unsigned int user, float n) {

	// Clear array
	for (int j=0; j < num_features; j ++) {
		implicitC[j] = 0;
	}

	unsigned int u_num_movies = userImplicitData[user*2];
	unsigned int u_movie;
	for (int i=0; i < u_num_movies; i++) {
		u_movie = userImplicitMovies[user][i];

		for (int j=0; j < num_features; j ++) {
			implicitC[j] += implicitFeatures[u_movie][j];
		}
	}
	for (int j=0; j < num_features; j ++) {
		implicitC[j] *= n;
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

void saveResults(char * file) {
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		printf("Saving results failed\n");
		return;
	}
	for (int i=0; i < num_lines; i++) {
		fprintf(fp, "%d %d %f\n", (int) predictions[i*3], (int) predictions[i*3 +1], predictions[i*3+2]);
	}


}

int main(){
	// Load Movie data
	loadMovieData("../../netflix/um/all_ut.dta");
	//loadMovieData("../stats/probe.dta");
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
	implicitFeatures = calloc(num_movies, sizeof(float *));
	userImplicitData = calloc(num_users*2, sizeof(float));
	itemBinBias = calloc(num_movies, sizeof(float *));
	userAlphas = calloc(num_users, sizeof(float));
	if (userFeatures == NULL || movieFeatures == NULL || userImplicitData == NULL) {
		printf("Malloc failed\n");
		return -1;
	}


	// Load offsets
	loadData("features/f050_e030/user_offset.dta", userOffset);
	loadData("features/f050_e030/movie_offset.dta", movieOffset);
	loadData("features/f050_e030/user_alphas.dta", userAlphas);
	// Load item bin bias
	loadFileAvg("../stats/user_date_avg.dta");
	loadUserDev("../stats/user_date_dev.dta", "features/f050_e030/user_time_bias.dta");
	
	
	// Load user implicit data
	loadUserImplicit("../stats/user_implicit_2.dta", userImplicitData);
	// Load 
	initializeImplicitMovies("../../implicit/user_implicit_movies.dta");

	// Load featuress
	loadFeatures("features/f050_e030/user_features.dta", num_users, num_features, userFeatures);
	loadFeatures("features/f050_e030/movie_features.dta", num_movies, num_features, movieFeatures);
	loadFeatures("features/f050_e030/implicit_features.dta", num_movies, num_features, implicitFeatures);
	loadFeatures("features/f050_e030/item_bin_bias.dta", num_movies, num_time_bins, itemBinBias);
	printf("Test %f %f\n", userFeatures[1][num_features - 1], userFeatures[num_users-1][num_features-1]);

	predictions = calloc(num_lines*3, sizeof(float));
	if (predictions == NULL) {
		return -1;
	}

	printf("--------------Predicting --------------\n");
	int user, movie, line_number, item_bin, date, rating;
	int count = 0;
	int user_date_count = -1;
	float n = userImplicitData[1*2 + 1]; // get n for first user
	float feature_c, alpha, dev, user_time_b;
	int temp = 0;
	int date_temp = 0;
	for (unsigned int j = 0; j < total_size; j++) {
			line_number = j * 4;
			user = movie_data[line_number];
			movie = movie_data[line_number  + 1];
			date = movie_data[line_number   + 2];
			rating = movie_data[line_number + 3];
			if (temp != user) {
				// update implict feature for previous user
				n = userImplicitData[user*2 + 1];
				getImplicitC(user, n);
				temp = user;
				date_temp = 0;
				user_date_count = -1;
			} 
			// Update different date count
			if (date != date_temp) {
				//printf("date %d ", date);
				user_date_count ++;
				date_temp = date;
			}
			// if not in test set continue
			if (rating != 0) {
				continue;
			}

			// Get contribution from each feature
			feature_c = 0;
			for (unsigned int i = 0; i < num_features; i++) {
				feature_c += (userFeatures[user][i] + implicitC[i])* movieFeatures[movie][i];
			}
			//printf("user %d movie %d date %d user_date_count %d rating %d total dates %d\n",
			//	user, movie, date, user_date_count, rating, (int) timeAvg[user]);
			// Get user time bias and alpha
			alpha = userAlphas[user];
			dev = userDev[user][(user_date_count)*2];
			user_time_b = userDev[user][(user_date_count)*2 +1];
			// Get item bin number and bias
			item_bin = (int) floor(date / 75.0);
			predictions[count] = user;
			predictions[count + 1] = movie;
			predictions[count + 2] = (GLOBAL_AVG + userOffset[user] + movieOffset[movie] 
				+ itemBinBias[movie][item_bin] + alpha*dev + user_time_b + feature_c);
			count += 3;
		}
	

	printf("-----------Saving results-----------\n");
	saveResults("results/ut_test_f050_e030.dta");

	free(userOffset);
	free(movieOffset);
	free(movie_data);
	return 0;
}
