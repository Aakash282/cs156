#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// Get average time for each user and number of different days they've rated movies
static unsigned int num_users = 458293 + 1;
float * time_avg;
float ** user_dev;
static inline
float sign(float v) {
	if (v < 0) {
		return -1.0;
	} else {
		return 1.0;
	}
}

void calcDev() {
	// Iterate through all lines
	char str[60];
	FILE *fp =  fopen("../../netflix/um/all_ut.dta", "r");
	if (fp == NULL) {
		return;
	}
	int user, time_v, count;
	int temp = 0;
	int time_temp = 0;
	float total = 0;
	float user_avg, diff;
	int n = 0;
	int temp_time = 0; // time value on first line
	int unique = 0;
	while (fgets(str, 60, fp) != NULL) {
		user = atoi(strtok(str, " "));
		strtok(NULL, " ");
		time_v = atoi(strtok(NULL, " "));
		if (user != temp) {
			user_avg = time_avg[user*2];
			count = 0;
			temp = user;
			temp_time = 0;
		}
		// save dev if encountering new time
		if (temp_time != time_v) {
			diff = (float) time_v - user_avg;
			
			user_dev[user][count] =  sign(diff) * pow(abs(diff), 0.4);
			//printf("%e diff %f ", user_dev[user][count], diff);
			count ++;	
			temp_time = time_v;
		}

		n++;
		if (n % 10000000 == 0) {
			printf("%d\n", n);
		}
	}
	fclose(fp);	
}


void loadFileAvg(char * file) {
	time_avg = calloc(num_users*2, sizeof(float));
	FILE * fp = fopen(file, "r");
	char * str[60];
	int count = 2;
	while (fgets(str, 60, fp) != NULL) {
		time_avg[count] = atof(strtok(str, " "));
		time_avg[count + 1] = atof(strtok(NULL, " "));
		count += 2;
	}
	printf("test %f %f\n", time_avg[num_users*2-2], time_avg[num_users*2-1]);
	fclose(fp);

}

void initializeUserDev() {
	user_dev = calloc(num_users, sizeof(float *));
	int num_dates = 0;

	for (int i=1; i < num_users; i++) {
		num_dates = (int) time_avg[i*2 + 1];
		//printf("num_dates %d\n", num_dates);
		user_dev[i] = calloc(num_dates, sizeof(float));
	}
	printf("test %f\n", user_dev[num_users-1][0]);
} 

void saveDev(char * file) {
	FILE *fp = fopen(file, "w");
	int num_dates;
	for (int i=1; i< num_users; i++) {
		num_dates = (int) time_avg[i*2 + 1];
		for (int j=0; j < num_dates; j++) {
			fprintf(fp, "%f ", user_dev[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

int main() {
	loadFileAvg("../stats/user_date_avg.dta");
	initializeUserDev();
	calcDev();
	saveDev("../stats/user_date_dev.dta");
	return 1;
}