// For each user get list of movies they've seen
#include <stdio.h>
#include <stdlib.h>
void loadMovieData() {
	printf("\n----------Loading movie data-------------\n");
	// Iterate through all lines
	char str[60];
	char str2[5];
	FILE *fp =  fopen("../../netflix/um/all.dta", "r");
	FILE *fp2 = fopen("../../implicit/user_implicit_movies.dta", "w");
	if (fp == NULL || fp2 == NULL) {
		return;
	}
	int user, movie, count, temp;
	temp = 1;
	while (fgets(str, 60, fp) != NULL) {

		// Get user, movie, rating, skip date value
		user = atoi(strtok(str, " "));
		movie = atoi(strtok(NULL, " "));
		if (user != temp) {
			fprintf(fp2, "\n");
			temp = user;
		}
		fprintf(fp2, "%d ", movie);
		count++;
		if (count % 10000000 == 0) {
			printf("%d\n", count);
		}
	}
	fclose(fp);	
	fclose(fp2);
}

int main() {
	loadMovieData();

	return 0;
}