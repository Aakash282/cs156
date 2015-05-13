#include <stdio.h>
#include <stdlib.h>
// Probe size is 1374739
void loadMovieData() {
	FILE *fp2 = fopen("../../netflix/mu/all.idx", "r");
	char str2[5];
	int sample = 0;
	while (fgets(str2, 5, fp2) != NULL) {
		// Only use index 4
		if (atoi((char* )strtok(str2, " ")) != 4) {
			continue;
		}
		sample++;
		if (sample % 1000 == 0) {
			printf("%d\n", sample);
		}
	}
	fclose(fp2);
	printf("probe size %d", sample);
}

int main(){
	// Iterate through all lines
	loadMovieData();
	


	return 0;
}

