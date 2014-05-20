/*
################################################################################
#                                                                              #
# Pearson Similarity Coefficients for Users: Netflix Competition               #
# Author: Aakash Indurkhya (Aakash282@gmail.com)                               #
# Collaborators: John Co-Reyes  (jdcoreyes@gmail.com)                          #
# Date: 5/9/14                                                                 #
# Summary: Here we computer the Pearson Similarity Coefficient between all     #
# pairs. Thus, there are O(n^2) coefficients to compute and the algorithm will #
# take O(n^2). Unforunately, there is no faster means of computing these       #
# values. Opimizations were used where possible and it is highly recommended   #
# to compile with the -Ofast compiler flag.                                    #
# Eventually, the values computed by this program will be used by a K nearest  #
# neighbors algorithm for the netflix competition.                             #
#                                                                              #
################################################################################
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(){
	// Iterate through all lines
	FILE *fp =  fopen("../../netflix/um/all.dta", "r");
	FILE *fp2 = fopen("../../netflix/um/all.idx", "r");
	if (fp == NULL || fp2 == NULL) {
		return -1;
	}

	char str[60];
	char str2[5];
	char str3[20];


	// Load movie data and baseline into array
	int num_lines = 99666408;
	// int num_lines = 10;
	

	// make array for raw data
	int * movie_data = calloc(num_lines * 3, sizeof(int));
	if (movie_data == NULL) {
		printf("Malloc failed\n");
		return -1;
	}
	int line_number = 0;
	int sample = 0;


	printf("\n----------Loading data-------------\n");

	FILE *fp3 = fopen("../stats/uavg.dta", "r");
	num_lines = 458293;
	int user_idx = 0;
	// make array for uavg data [avg n]
	float * uavg = calloc(num_lines * 2, sizeof(float));

	if (uavg == NULL) {
		printf("Malloc failed\n");
		return -1;
	}

	while (fgets(str, 60, fp3) != NULL) {
		// Get the user average and number of ratings
		uavg[user_idx] = atof(strtok(str, "\t"));
		uavg[user_idx + 1] = atof(strtok(NULL, "\t"));
		user_idx += 2;
	}
	fclose(fp3);

	// uavg has structure of [avg, n, avg, n, avg, n, avg, ....]
	// where avg is the average for the user
	// where n is the number of ratings b the user

	// iterate though the files until completion
	while (fgets(str, 60, fp) != NULL && fgets(str2, 5, fp2) != NULL) {
		// Only use index < 5
		if (atoi((char* )strtok(str2, " ")) == 5) {
			continue;
		}

		// Get user, movie, rating
		movie_data[line_number] = atoi(strtok(str, " "));
		movie_data[line_number + 1] = atoi(strtok(NULL, " "));
		strtok(NULL, " ");	// ignore time
		movie_data[line_number + 2] = atoi(strtok(NULL, " "));
		
		line_number += 3;
		sample++;
		if (sample % 10000000 == 0) {
			printf("%d\n", sample);
		}
	}
	fclose(fp);
	fclose(fp2);
	int count = 0;


	
	

	printf("\n-----------Computing Similarities--------------\n");
	int users = 458293;
	
	// Iterator
	int a = 0;			// this is the user we are currently comparing all users to
	int u = 1;			// this is the user a is currently being compared to
	int i = 0;			// i iterates through movie_data for user a
	int j = 0;			// j iterates through movie_data for user u
	int c, q, k;		// inner loop iterators
	float num;			// numerator for the pearson similarity calculation
	float den1;			// part 1 of denominator
	float den2;			// part 2 of denominator
	float den;			// the completed denominator
	float result;		// the pearson coefficient to be stored. 
	int counter = 0;	// inner loop iterator

	int new_user = 0;	// boolean for whether user has been considered 

	FILE *fp4 = fopen("../../netflix/um/pearson.dta", "w");
	// this is an outer loop | we need Pearson's between all users
	// on the order of n^2 handshakes. Here we iterate a.
	while (movie_data[3 * i] != a + 1) {
		i++;
	}


	while (a < users) {
		// create an array of size n
		int * movies = calloc(uavg[2 * a + 1] * 2, sizeof(int));

		printf("User = %d\n", movie_data[3 * i]);
		// collect all data on the current user
		while (!new_user) {
			// are we considering a new user
			new_user = (movie_data[3 * i] != a + 1) ? 1 : 0;
			if (new_user) 
				break;

			printf("Movie: %d Rating: %d\n", movie_data[3 * i + 1], movie_data[3 * i + 2]);

			// what movies has this user watched?
			// store the movie and rating
			movies[counter] = movie_data[3 * i + 1];
			movies[counter + 1] = movie_data[3 * i + 2];
			counter += 2;
			i += 1;
		}

		// prints out progress updates
		if ((a + 1) % 10 == 0) {
			printf("%d\n", a + 1);
		}

		j = i;
		
		// this is an inner loop | compare user a with all other users.
		// Here we iterate u.
		while (u < users) {
			// make an array the size of movies (which is also the max 
			// size of common)
			float * common = calloc(uavg[(2 * a) + 1] * 2, sizeof(float));

			// print for updates
			printf("Movies %d and %d\n", a + 1, u + 1);

			new_user = 0;
			c = 0;

			// collect data on another user
			while (!new_user) {
				// consider a new data point
				// are we considering a new user
				new_user = (movie_data[3 * j] - (u + 1) != 0) ? 1 : 0;
				
				// compute the pearson similarity using array of common movie ratings
				if (new_user) {
					if (c >= 48){
						// clear the values from the last computation
						num = 0;
						den1 = 0;
						den2 = 0;
						den = 0;
						result = 0;
						for (k = 0; k < c; k += 2) {
							num += (common[k] - uavg[2 * a]) * (common[k + 1] 
								- uavg[2 * u]);
							den1 += (common[k] - uavg[2 * a]) * (common[k] 
								- uavg[2 * a]);
							den2 += (common[k + 1] - uavg[2 * u]) * 
							(common[k + 1] - uavg[2 * u]);
						}

						den = den1 * den2;
						result = (num / sqrt(den));

						// calculate the result and store it in the file
						// deal with divide by zero
						if (den == 0.0) {
							fprintf(fp4, "%d %d %f %d\n", a, u, 0.0, (c / 2));
						}
						else {
							fprintf(fp4, "%d %d %f %d\n", a, u, result, (c / 2));
						}
						break;
					}
				}

				// Add any common movies to the common array
				for (q = 0; q < (uavg[(2 * a) + 1] * 2); q += 2) {
					if (movie_data[3 * j + 1] - movies[q] == 0 && c <= uavg[(2 * a) + 1] * 2) {
						common[c] = movies[q + 1];
						common[c + 1] = movie_data[3 * j + 2];
						// printf("ratings: %d, %d\n", movies[q+1], movie_data[3 * j + 2]);
						c += 2;
						break;
					}
				}

				// iterate to the next data point
				j += 1;
			}

			free(common);

			// iterate to the next movie to compare with
			u += 1;
		}

		free(movies);

		// update iterator 
		a += 1;
		u = a + 1;
		counter = 0;
		new_user = 0;
	}

	fclose(fp4);
	printf("\n-----------Finished Computation--------------\n");
	return 0;
}
