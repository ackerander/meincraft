#include "stuff.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int
main()
{
	if (initgl())
		return -1;

/* Construct chunk & meshes */
	init();
	genMeshes();

/*
	glGenBuffers(1, &offsetBuff);
	glBindBuffer(GL_ARRAY_BUFFER, offsetBuff);
	glBufferData(GL_ARRAY_BUFFER, 3 * meshes * sizeof(GLint), pos, GL_STATIC_DRAW);
	glVertexAttribDivisor(1, 1);

	glGenBuffers(1, &spanBuff);
	glBindBuffer(GL_ARRAY_BUFFER, spanBuff);
	glBufferData(GL_ARRAY_BUFFER, 2 * meshes, spans, GL_STATIC_DRAW);
	glVertexAttribDivisor(2, 1);

	glGenBuffers(1, &faceBuff);
	glBindBuffer(GL_ARRAY_BUFFER, faceBuff);
	glBufferData(GL_ARRAY_BUFFER, meshes, faces, GL_STATIC_DRAW);
	glVertexAttribDivisor(3, 1);
	
	glGenBuffers(1, &texBuff);
	glBindBuffer(GL_ARRAY_BUFFER, texBuff);
	glBufferData(GL_ARRAY_BUFFER, meshes, texes, GL_STATIC_DRAW);
	glVertexAttribDivisor(4, 1);
*/

	writeMeshes();

	/*
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		updateView();
		mm4mult(perspecM, viewMat, mvp);
		glUniformMatrix4fv(matrixID, 1, GL_TRUE, mvp);

		glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
		glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, offsetBuff);
		glVertexAttribIPointer(1, 3, GL_INT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, spanBuff);
		glVertexAttribIPointer(2, 2, GL_UNSIGNED_BYTE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, faceBuff);
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, texBuff);
		glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, 0, 0);

		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, meshes);
		glfwSwapBuffers(window);

		glfwPollEvents();
	} while (!glfwWindowShouldClose(window));
	*/
	renderLoop();

	cleanup();
	cleanupgl();

	return 0;
}
