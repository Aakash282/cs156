#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <float.h>
/* Constants for operation*/
// Add one because we're not using the first cell of both userValue and movieValue
// to avoid off by one confusion
static unsigned int num_users = 458293 + 1;
static unsigned int num_movies = 17770 + 1;
static unsigned int num_lines = 99666408;
static unsigned int total_size = 102416306;
static unsigned int qual_size = 2749898; // num lines in qual data
//static unsigned int probe_size = 1374739;
//static unsigned int num_lines = 275;
static int epochs = 41; // number of iterations to do over whole data set

static unsigned int num_features = 500;

static unsigned int num_time_bins = 30; // number of bins to break time into
//static unsigned int bin_size = 75; // number of consecutive days each bin tracks

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
// Index array for what data set each data point belongs to
unsigned int * idx;

// Baseline offset arrays
float * userOffset;
float * movieOffset;

// implict data array, each line contains number of ratings, n and 1/sqrt(n) for each user
float * userImplicitData;

// contains movied rated by each user
int ** userImplicitMovies;

// Save contribution of all implicit weights for each user to save computation time
float * implicitC;

float * tempImplicitC;

// 1d array containing pointer to feature vector for each user or movie
// Each array is a vector of pointers to feature vectors for each user or movie
// [u1 u2 ... un]
float ** userFeatures;
// [m1 m2 ... mn]
float ** movieFeatures;
// [m1 m2 ... mn]
float ** implicitFeatures;

// Bias for each time bin
float ** itemBinBias;

// Float containg average date for each user and number of different dates
float * timeAvg;
// Contains devation for each different date of each user along with user time bias
// u1: dev1 b1 dev2 b2 ...
float ** userDev;

// user alphas for time deviation
float * userAlphas;

// keep track of training error
float * train_errs;

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
void loadMovieData() {
	printf("----------Loading movie data-------------\n");
	unsigned int line_number = 0;
	unsigned int count = 0;
	// Iterate through all lines
	char str[60];
	FILE *fp =  fopen("../../netflix/um/all_ut.dta", "r");
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
	printf("----------Loading data from file %s------\n", file);
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
void loadUserDev(char* file) {
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

	FILE *fp = fopen(file, "r");
	char str[50000];
	int user = 1;
	printf("----------Loading data from file %s------\n", file);
	while (fgets(str, 50000, fp) != NULL) {
		num_dates = (int) timeAvg[user];
		// get first dev
		userDev[user][0] = atof(strtok(str, " "));
		for (int i=1; i < num_dates; i++) {
			userDev[user][i*2] = atof(strtok(NULL, " "));

			if (userDev[user][i*2] > 1000 || userDev[user][i*2] < -1000) {
				printf("user dev read failed dev %d %e user %d date count %d\n", 
					userDev[user][i*2] , userDev[user][i*2], user, i);
				exit(1);
			}
		}
		user ++;
	}
	if (user != num_users) {
		printf("read from dev failed\n");
		exit(1);
	}

	printf("test userDev user 23 date count 343 %f\n", userDev[23][342*2]);
} 


static inline
float randval() {
	return (-1.0 + 2* (float) rand() / RAND_MAX) / 25.0;
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
			userFeatures[i][j] = (rand() % 14000 + 2000) * 0.000001235f;
			//randval(); //(-1.0 + 2* (float) rand() / RAND_MAX) / 25.0;;
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
		if (movieFeatures[i] == NULL) {
			printf("Malloc failed\n");
		}
	}

	// Initialize values
	for (int i=0; i < num_movies; i++) {
		for (int j=0; j < num_features; j++) {
			movieFeatures[i][j] = (rand() % 14000 + 2000) * -0.000001235f;
			//randval(); //(-1.0 + 2* (float) rand() / RAND_MAX) / 25.0;;
		}
	}
}

void initializeItemBinBias() {
	itemBinBias = calloc(num_movies, sizeof(float *));
	if (itemBinBias == NULL) {
		printf("malloc failed\n");
		return;
	}
	for (int i=0; i < num_movies; i++) {
		itemBinBias[i] = calloc(num_time_bins, sizeof(float));
		if (itemBinBias[i] == NULL) {
			printf("Malloc failed\n");
			return;
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
		if (implicitFeatures[i] == NULL) {
			printf("Malloc failed\n");
		}
	}

	// Initialize values
	for (int i=0; i < num_movies; i++) {
		for (int j=0; j < num_features; j++) {
			implicitFeatures[i][j] = 0;
			//randval() / 2.0; //(-1.0 + 2* (float) rand() / RAND_MAX) / 25.0;
		}
	}
}


// Load in list of each movies that each user has rated
void initializeImplicitMovies(char * file) {
	int u_num_movies = 0;
	userImplicitMovies = calloc(num_users, sizeof(int *));
	implicitC = calloc(num_features, sizeof(float));
	tempImplicitC = calloc(num_features, sizeof(float));
	if (userImplicitMovies == NULL || implicitC == NULL || tempImplicitC == NULL) {
		printf("Malloc failed\n");
		return;
	}

	for (int i=1; i < num_users; i++) {
		u_num_movies = userImplicitData[i*2];
		//printf("calloc %d\n", u_num_movies);
		userImplicitMovies[i] = calloc(u_num_movies, sizeof(int));
		if (userImplicitMovies[i] == NULL) {
			printf("Malloc failed\n");\
			return;
		}
	}	
	FILE *fp = fopen(file, "r");
	int max_b_size = 50000; // buffer size
	
	char * success;
	char str[50000];
	printf("----------Loading data from file %s------\n", file);
	for (int i=1; i < num_users; i++) {
		//str = calloc(max_b_size, sizeof(char));
		if (str == NULL) {
			printf("malloc failed\n");
			return;
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
			return;
		}
		//free(str);
	}
	u_num_movies = (int) userImplicitData[(num_users-1)*2];
	printf("test %d %d\n", u_num_movies, userImplicitMovies[num_users - 1][631]);
	
}


// static inline
// float predictRating(int user, int movie) {
// 	// Contribution from features user_f1 * movie_f1 + user_f2 * movie_f2 + ...
// 	float feature_c = 0;
// 	// Contribution from implicit features y1 + y2 + ... saved in implicitC
// 	// normalized by 1/sqrt(n) saved in 2nd column of userImplicitMovies

// 	for (int i = 0; i < num_features; i++) {
// 		feature_c += (userFeatures[user][i] + implicitC[i])* movieFeatures[movie][i];
// 	}
// 	return GLOBAL_AVG + userOffset[user] + movieOffset[movie] + feature_c;
// }

static inline
void updateFeatures(unsigned int user, unsigned int movie, float err, float n) {
	// Update each feature using gradient descent
	float uv, mv, tc;
	for (int i = 0; i < num_features; i++) {
		uv = userFeatures[user][i];
		mv = movieFeatures[movie][i];
		tc = tempImplicitC[i];
		movieFeatures[movie][i] += gamma2 *(err * (uv + tc) - 0.015 * mv);
		tempImplicitC[i] += gamma2 * (err * movieFeatures[movie][i] - 0.015 * tc);
		userFeatures[user][i] += gamma2 * (err * mv - 0.015 * uv);
	}	
}

static inline
void updateBaseline(unsigned int user, unsigned int movie, float err, 
	unsigned int item_bin, int user_date_count, float dev) {
	// Update each baseline offset using gradient descent
	float uv = userOffset[user];
	float mv = movieOffset[movie];
	float ib = itemBinBias[movie][item_bin];
	float alpha = userAlphas[user];
	float ub = userDev[user][user_date_count*2+1];
	userOffset[user] 					+= gamma1 * (err - 0.01 * uv);
	movieOffset[movie] 					+= gamma1 * (err - 0.01 * mv);
	itemBinBias[movie][item_bin]		+= gamma1 * (err - 0.01 * ib);
	userAlphas[user] 					+= gamma1 * (err *dev /100.0- 0.01 * alpha);
	userDev[user][user_date_count*2+1] 	+= gamma1 * (err - 0.01 * ub);

	//printf("user %d movie %d err %f date_count %d dev %f\n",
	//		user, movie, err, user_date_count, dev);
	//printf("userAlphas %f user_time_b %f\n", userAlphas[user], userDev[user][user_date_count*2+1]);
	// if (userDev[user][user_date_count*2+1] < -1000 || userDev[user][user_date_count*2+1] > 1000) {
	// 	printf("user %d movie %d err %f date_count %d dev %f\n",
	// 		user, movie, err, user_date_count, dev);
	// }
}

// Calculate once per user, the sum of his implicit features
static inline
void getImplicitC(unsigned int user, float n) {

	// Clear array
	for (int j=0; j < num_features; j ++) {
		implicitC[j] = 0;
	}
	for (int j=0; j < num_features; j ++) {
		tempImplicitC[j] = 0;
	}
	unsigned int u_num_movies = userImplicitData[user*2];
	unsigned int u_movie;
	for (unsigned int i=0; i < u_num_movies; i++) {
		u_movie = userImplicitMovies[user][i];

		for (unsigned int j=0; j < num_features; j ++) {
			implicitC[j] += implicitFeatures[u_movie][j];
		}
	}
	for (unsigned int j=0; j < num_features; j ++) {
		implicitC[j] *= n;
		tempImplicitC[j] = implicitC[j];
	}
}
static inline
void updateImplicitFeatures(unsigned int user, float n) {
	unsigned int u_num_movies = userImplicitData[user*2];
	unsigned int u_movie;
	// For each movie vector that user has seen
	for (unsigned int i = 0; i < u_num_movies; i++) {
		// get movie and iterate update movie feature vector
		u_movie = userImplicitMovies[user][i];
		for (unsigned int j=0; j < num_features; j++) {
			implicitFeatures[u_movie][j] += n * (tempImplicitC[j] - implicitC[j]);
		}
	}

}


void saveOffsets() {
	// Save baseline offsets
	FILE *fp = fopen("features/f500_e040/movie_offset.dta", "w");
	for (int f = 1; f < num_movies; f++) {
		fprintf(fp, "%f\n", movieOffset[f]);
	}
	fclose(fp);
	fp = fopen("features/f500_e040/user_offset.dta", "w");
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

void saveItemBinBias(char * file) {
	FILE *fp = fopen(file, "w");
	for (int i=1; i < num_movies; i++) {
		for (int j=0; j < num_time_bins; j++) {
			fprintf(fp, "%f ", itemBinBias[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void saveUserTimeBias(char * file) {
	FILE *fp = fopen(file, "w");
	int num_dates;
	for (int i=1; i < num_users; i++) {
		num_dates = (int) timeAvg[i];
		// write user time biases
		for (int j=0; j < num_dates; j++) {
			fprintf(fp, "%f ", userDev[i][j*2+1]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void saveUserAlphas(char * file) {
	FILE *fp = fopen(file, "w");
	int num_dates;
	for (int i=1; i < num_users; i++) {
		fprintf(fp, "%f\n", userAlphas[i]);
	}
	fclose(fp);
}
void saveErrors(char * file) {
	FILE *fp = fopen(file, "w");
	for (int i=0; i < epochs; i++) {
		fprintf(fp, "%f\n", train_errs[i]);
	}
	fclose(fp);
}

void saveResults(char * file) {
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		printf("Saving results failed\n");
		return;
	}
	for (int i=0; i < qual_size; i++) {
		fprintf(fp, "%d %d %f\n", (int) predictions[i*3], (int) predictions[i*3 +1], predictions[i*3+2]);
	}


}

int main(){

	// printf("test %f %f %f %f\n", userOffset[5], userOffset[num_users - 1], movieOffset[5], movieOffset[num_movies - 1]);
	// Load Movie data
	loadMovieData();
	// Set up offsets and implicit data
	userOffset = calloc(num_users, sizeof(float));
	movieOffset = calloc(num_movies, sizeof(float));
	userImplicitData = calloc(num_users*2, sizeof(float));
	userAlphas = calloc(num_users, sizeof(float));
	predictions = calloc(qual_size*3, sizeof(float));
	if (userOffset == NULL || movieOffset == NULL || userImplicitData == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	//loadData("../stats/user_offset_reg2.dta", userOffset);
	//loadData("../stats/movie_offset_reg.dta", movieOffset);
	loadUserImplicit("../stats/user_implicit_2.dta", userImplicitData);
	loadFileAvg("../stats/user_date_avg.dta");
	loadUserDev("../stats/user_date_dev.dta"); // Load user dev and intialize user time biases

	// Initialize features
	// Initialize random seed
	srand (time(NULL));
	initializeUserFeatures();
	initializeMovieFeatures();


	initializeImplicitMovies("../../implicit/user_implicit_movies.dta");
	printf("test %d\n", userImplicitMovies[num_users - 1][2]);
	initializeImplicitFeatures();
	initializeItemBinBias();

	printf("--------------Training --------------\n");
	unsigned int user, movie, date, line_number, item_bin;
	int user_date_count, rating;
	unsigned int temp = 0;
	unsigned int date_temp = 0;
	int p_count = 0;
	float err, feature_c, alpha, dev, user_time_b, predict;
	float train_err;
	train_errs = calloc(epochs, sizeof(float));
	float n = userImplicitData[1*2 + 1]; // get n for first user
	float GLOBAL_AVG = 3.609516;
	int train_count = 0;
	for (int i = 1; i <= epochs; i++) {
		train_err = 0;
		temp = 0;
		date_temp = 0;
		user_date_count = -1;
		train_count = 0;
		for (unsigned int j = 0; j < total_size; j++) {
			line_number = j * 4;
			user = movie_data[line_number];
			movie = movie_data[line_number  + 1];
			date = movie_data[line_number   + 2];
			rating = movie_data[line_number + 3];

			// Get rating from raw data if we have a new user
			if (temp != user) {
				// update implict feature for previous user
				
				if (temp > 0 && i != epochs) {
					updateImplicitFeatures(temp, n);
				}
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

			


			// Get contribution from each feature
			feature_c = 0;
			for (unsigned int i = 0; i < num_features; i++) {
				feature_c += (userFeatures[user][i] + tempImplicitC[i])* movieFeatures[movie][i];
			}

			// Get user time bias and alpha
			alpha = userAlphas[user];
			dev = userDev[user][(user_date_count)*2];
			user_time_b = userDev[user][(user_date_count)*2 +1];
			// Get item bin number and bias
			item_bin = (int) floor(date / 75.0);
			
			predict = (GLOBAL_AVG + userOffset[user] + movieOffset[movie] 
				+ itemBinBias[movie][item_bin] + alpha*dev + user_time_b + feature_c);
			// if in test set continue and not at test iteration
			if (i != epochs && rating == 0) {
				continue;
			}
			// If at test iteration then save predict and don't update features
			else if (i == epochs && rating ==0)	 {
				predictions[p_count] = user;
				predictions[p_count +1] = movie;
				predictions[p_count + 2] = predict;
				p_count += 3;
				continue;
			}	
			err = (float) rating - predict;

			train_err += err * err;
			// if (!(err > -10 && err < 10)) {
			// 	printf("user %d movie %d rating %d date %d err %f\n", 
			// 		user, movie, rating, date, err);
			// 	printf("alpha %f dev %f user_time_b %f \n", alpha, dev, user_time_b);
			// 	printf("userdc %d num_dates %d userDev %f userDev %f\n", user_date_count, 
			// 		(int) timeAvg[user], userDev[user][user_date_count], userDev[user][user_date_count]);
			// 	return -1;
			// }
			updateFeatures(user, movie, err, n);
			updateBaseline(user, movie, err, item_bin, user_date_count, dev);
			train_count++;
		}
		if (i != epochs && rating != 0) {
			if (train_count != num_lines) {
				printf("Only passed through %d of data\n", train_count);
			}
			// update last user
			updateImplicitFeatures(user, n);
			// Update gammas by factor
			gamma1 *= gamma_step;
			gamma2 *= gamma_step;
			train_errs[i-1] = sqrt(train_err / num_lines);
			printf("Epoch %d Train RMSE: %f\n", i, train_errs[i-1]);			
		}

	}

	printf("-----------Saving features-----------\n");
	saveResults("results/ut_test_f500_e040_time.dta");
	saveOffsets();
	saveUserFeatures("features/f500_e040/user_features.dta");
	saveMovieFeatures("features/f500_e040/movie_features.dta");
	saveImplicitFeatures("features/f500_e040/implicit_features.dta");
	saveItemBinBias("features/f500_e040/item_bin_bias.dta");
	saveUserTimeBias("features/f500_e040/user_time_bias.dta");
	saveUserAlphas("features/f500_e040/user_alphas.dta");
	saveErrors("features/f500_e040/error.dta");
	free(userOffset);
	free(movieOffset);
	free(movie_data);
	return 0;
}
