#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Constants for operation*/
// Add one because we're not using the first cell of both userValue and movieValue
// to avoid off by one confusion
static unsigned int num_users = 458293 + 1;
int ** userImplicitMovies;
int main() {
	// userImplicitMovies = calloc(num_users, sizeof(int *));
	// userImplicitMovies[0] = calloc(50, sizeof(int));
	// if (userImplicitMovies[0][0] == 1) {
	// 	printf("Failed\n");
	// 	return -1;
	// }
	int * test = calloc(99666408 * 5, sizeof(int));
	if (test == NULL) {
		printf("failed\n");
	}
	int a;
	for (int i=1; i< 99666408 * 5; i++) {
		a = a % i;
	}
	return 0;
}