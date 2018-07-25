#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Interactive.h"

int main(void) {
	printf("Welcome to the U.S. Temperature Analysis Module.\n");
	printf("Full-screen view is recommended to allow for proper formatting view.\n");

	srand((unsigned int) time(NULL));
	main_menu();

	return EXIT_SUCCESS;
}