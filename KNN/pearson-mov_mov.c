/*
Pearson Similarity Coefficients for Movies: Netflix Competition               
Author: Rushikesh Joshi
Collaborator: Aakash Indurkhya and JD Co-Reyes
This code will be using the Shrunken Pearson's Correlation Coefficient 
from the Koren et al. paper written for the Netflix competition.
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(){
	// Iterate through all lines
	FILE *fp =  fopen("um/all.dta", "r");
	FILE *fp2 = fopen("um/all.idx", "r");
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
	// using either mavg.dta or better_mavg
	FILE *fp3 = fopen("../stats/better_mavg2.dta", "r");
	int movie_idx = 0;
	// make array for mavg data
	float * mavg = calloc(num_lines * 2, sizeof(float));
	float total;

	if (mavg == NULL) {
		printf("Malloc failed\n");
		return -1;
	}

	while (fgets(str, 60, fp3) != NULL) {
		// Get the user average and number of ratings
		mavg[movie_idx] = atof(strtok(str, "\t"));
		total = atof(strtok(NULL, "\t")); // pass over total values
		mavg[movie_idx + 2] = atof(strtok(str, "\t"));
		movie_idx += 3;
	}
	fclose(fp3);

	// mavg has structure of [avg1, n1, avg2, n2, avg3, n3, avg4, ....]
	// where avg is the average rating for the movie
	// where n is the number of ratings for that movie

	int currentMovie = 1;
	int user, movie; 

	int movieCount = 0;
	// iterate though the files until completion
	while (fgets(str, 60, fp) != NULL && fgets(str2, 5, fp2) != NULL) {
		// Only use index < 5
		if (atoi((char* )strtok(str2, " ")) == 5) {
			continue;
		}

		user = atoi(strtok(str, " ")); // record user
		movie = atoi(strtok(str, " ")); // record movie

		/*
		if (uavg[(2 * (user - 1)) + 1] <= 2000) {
			continue;
		}*/ // for users

		if (mavg[(2 * (movie - 1)) + 1] <= 1000) {
			continue;
		}

		if(movie != currentMovie) {
			movieCount += 1;
			currentMovie = movie;
		}
		

		// Get user, movie, rating
		movie_data[line_number] = user;
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


	printf("%d %d\n", movieCount, sample);


	printf("\n-----------Computing Similarities--------------\n");
	//int users = 458292;
	int movies = 17770;
	
	// Iterator
	int a_old = movie_data[0] - 1;			// this is the user we are currently comparing all users to
	int a = movie_data[1] - 1; // movie we are currently looking at
	int u;			// this is the movie a is currently being compared to
	int i = 0;			// i iterates through movie_data for movie a
	int j = 0;			// j iterates through movie_data for movie u
	int c, q, k;		// inner loop iterators
	float num;			// numerator for the pearson similarity calculation
	float den1;			// part 1 of denominator
	float den2;			// part 2 of denominator
	float den;			// the completed denominator
	float result;		// the pearson coefficient to be stored. 
	int counter = 0;	// inner loop iterator
	int last = 10000; 			// iterator for updates

	int new_user = 0;	// boolean for whether user has been considered 
	int new_movie = 0;  // boolean for whether movie has been considered yet

	FILE *fp4 = fopen("mu/pearson.dta", "w");

	while (movie_data[(3 * i) + 1] != a + 1) {
		i++;
	}

	// this is an outer loop | we need Pearson's between all movies
	// on the order of n^2 handshakes. Here we iterate a.
	while (i + 3 < line_number) {
		// create an array of size n
		int * users = calloc(mavg[2 * a + 1] * 2, sizeof(int));

		// printf("User = %d\n", movie_data[3 * i]);
		// collect all data on the current movie
		while (1) {
			// are we considering a new movie
			new_movie = (!new_movie && i >= 1 && (a + 1) != movie_data[(3 * i + 1) - 3]) ? 1 : 0;
			if (new_movie)
				break;

			// printf("Movie: %d Rating: %d\n", movie_data[3 * i + 1], movie_data[3 * i + 2]);

			// what users have rated this movie?
			// store the user and their rating
			users[counter] = movie_data[3 * i]; // stores user
			users[counter + 1] = movie_data[3 * i + 2]; // stores rating
			counter += 2;
			i += 1;
		}

		// prints out progress updates
		if ((a + 1) - last >= 0) {
			printf("%d\n", last);
			last += last;
		}

		j = i;

		u = movie_data[3 * j + 1] - 1; // retrieves value for next movie to compare to movie a
		
		// printf("%d %d %d %d %f %f\n", i, j, a, u, uavg[(2 * a) + 1], uavg[(2 * u) + 1]);

		// this is an inner loop | compare movie a with all other movies rated by same users.
		// Here we iterate u.
		while (j + 3 < line_number) {
			// make an array the size of movies (which is also the max 
			// size of common)
			float * common = calloc(mavg[(2 * a) + 1] * 2, sizeof(float));

			// print for updates
			// printf("User %d and %d\n", a + 1, u + 1);

			new_user = 0;
			new_movie = 0;
			c = 0;

			// collect data on another movie
			while (1) {
				// consider a new data point
				// are we considering a new movie
				new_movie = (!new_movie && j > i && (u + 1) != (movie_data[(3 * j + 1) - 3])) ? 1 : 0;
				
				// compute the pearson similarity using array of common users
				if (new_movie) {
					if (c >= 0){
						// clear the values from the last computation
						num = 0;
						den1 = 0;
						den2 = 0;
						den = 0;
						result = 0;
						for (k = 0; k < c; k += 2) {
							num += (common[k] - mavg[2 * a]) * (common[k + 1] 
								- mavg[2 * u]);
							den1 += (common[k] - mavg[2 * a]) * (common[k] 
								- mavg[2 * a]);
							den2 += (common[k + 1] - mavg[2 * u]) * 
							(common[k + 1] - mavg[2 * u]);
						}

						den = den1 * den2;
						result = (num / sqrt(den));

						// calculate the result and store it in the file
						// deal with divide by zero
						if (den == 0.0) {
							fprintf(fp4, "%d %d %f %d\n", (a + 1), (u + 1), 0.0, (c / 2));
						}
						else {
							fprintf(fp4, "%d %d %f %d\n", (a + 1), (u + 1), result, (c / 2));
							// printf("RESULT: %d %d %f %d\n", (a + 1), (u + 1), result, (c / 2));
						}

						break;
					}
				}

				// Add any common users to the common array
				for (q = 0; q < (mavg[(2 * a) + 1] * 2); q += 2) {
					// printf("%d %d\n", movie_data[3 * j + 1], movies[q]);

					// if the user we are looking at is smaller than the user number in the 
					// users list, just break because we won't ever find it. 
					if (movie_data[3 * j] < users[q]) {
						break;
					}
					if (movie_data[3 * j] == users[q] && c <= mavg[(2 * a) + 1] * 2) {
						common[c] = users[q + 1]; // store user
						common[c + 1] = movie_data[3 * j + 2]; // store rating
						// printf("ratings: %d, %d\n", movies[q+1], movie_data[3 * j + 2]);
						c += 2;
						break;
					}
				}

				// printf("%d\n", movie_data[(3 * (j + 1)) + 1]);
				// iterate to the next data point
				j += 1;
				// break;
			}

			free(common);

			// iterate to the next user to compare with
			u = movie_data[3 * j + 1] - 1;
			// printf("After %d %d %d %d %f %f\n", i, j, a, u, uavg[(2 * a) + 1], uavg[(2 * u) + 1]);
		}

		free(movies);

		// update iterator 
		a = movie_data[3 * i + 1] - 1;
		counter = 0;
		// new_user = 0;
		new_movie = 0;
	}

	fclose(fp4);
	printf("\n-----------Finished Computation--------------\n");
	return 0;
}
