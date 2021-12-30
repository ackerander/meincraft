#include "stuff.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern mat4 viewMat;
extern GLFWwindow* window;
static const GLubyte vertBuffData[] = {0, 1, 2, 3};

int
main(int argc, char **argv)
{
	GLuint vertBuff, offsetBuff, spanBuff, faceBuff, texBuff, vertArrID, progID, matrixID, tex, texID;
	mat4 perspecM = {0}, mvp;
	GLint *pos;
	GLubyte *spans, *faces, *texes;
	size_t meshes;

	if (init())
		return -1;

	glGenVertexArrays(1, &vertArrID);
	glBindVertexArray(vertArrID);

	progID = loadShaders("shaders/vert", "shaders/frag");

	matrixID = glGetUniformLocation(progID, "MVP");
/* Construct matrices */
	perspec(M_PI / 4, 4.0 / 3, 0.1, 100.0, perspecM);

	tex = loadPng(argc == 2 ? argv[1] : "assets/testTex.png");
	texID = glGetUniformLocation(progID, "myTextureSampler");

/* Construct chunk & meshes */
	genChunk();
	meshes = genMeshes(&pos, &spans, &faces, &texes);
	printf("Mesh size: %lu\n", meshes);

	glGenBuffers(1, &vertBuff);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertBuffData), vertBuffData, GL_STATIC_DRAW);

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(texID, 0);

	glUseProgram(progID);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

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

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDeleteBuffers(1, &vertBuff);
	glDeleteBuffers(1, &offsetBuff);
	glDeleteBuffers(1, &spanBuff);
	glDeleteBuffers(1, &faceBuff);
	glDeleteBuffers(1, &texBuff);
	glDeleteVertexArrays(1, &vertArrID);
	glDeleteTextures(1, &tex);
	glDeleteProgram(progID);
	glfwTerminate();

	free(pos);
	free(spans);
	free(faces);
	free(texes);

	return 0;
}
