#include <stdio.h>

#include "scenedsl.h"

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;
	printf("%d\n", (int)SC_FIXED(128.0f));
	return 0;
}

