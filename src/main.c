#include "stuff.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int
main()
{
	if (initgl())
		return -1;

	init();
	genMeshes();

	writeMeshes();

	renderLoop();

	cleanup();
	cleanupgl();

	return 0;
}
