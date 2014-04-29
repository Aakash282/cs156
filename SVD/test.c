#include <stdio.h>
#include <stdlib.h>
int main() {
	FILE *fp3 = fopen("../../res/mu_baseline.dta", "r");
	int count = 0;
	char str[60];
	while (fgets(str, 60, fp3) != NULL) {	
		count++;
		if (count % 100000 == 0) {
			printf("value %f \n", atof(strtok(str, " ")));
			printf("%d\n", count);
		}
	}
	fclose(fp3);

}
