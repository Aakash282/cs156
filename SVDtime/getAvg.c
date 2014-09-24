#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Get average time for each user and number of different days they've rated movies
static unsigned int num_users = 458293 + 1;
float * time_avg;
void loadMovieData() {
	// Iterate through all lines
	char str[60];
	FILE *fp =  fopen("../../netflix/um/all_ut.dta", "r");
	if (fp == NULL) {
		return;
	}
	int user;
	int temp = 1;
	float total = 0;
	int n = 0;
	int time_v;
	int temp_time = 0; // time value on first line
	int unique = 0;
	while (fgets(str, 60, fp) != NULL) {
		user = atoi(strtok(str, " "));
		if (user != temp) {
			time_avg[temp*2] = total / n;
			time_avg[temp*2 + 1] = (float) unique;
			temp = user;
			total = 0;
			n = 0;
			unique = 0;
			//temp_time = 0;
		}
		strtok(NULL, " ");
		time_v = atoi(strtok(NULL, " "));
		if (temp_time != time_v) {
			unique++;
			temp_time = time_v;
		}
		// save dev for last user
		n++;
		total += time_v;

	}
	// save dev for last user
	time_avg[user*2] = total / (float) n;
	time_avg[user*2 + 1] = (float) unique;
	fclose(fp);	
}

int main() {
	time_avg = calloc(num_users*2, sizeof(float));
	loadMovieData();
	FILE *fp = fopen("../stats/user_date_avg_e.dta", "w");
	for (int i=1; i < num_users; i++) {
		fprintf(fp, "%f %f\n", time_avg[i*2], time_avg[i*2+1]);
	}
	return 1;
}