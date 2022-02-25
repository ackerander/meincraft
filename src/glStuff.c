#include "stuff.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#define SCREEN_W 2800
#define SCREEN_H 2100
#define NAME "IDK"
#define SPD 10
#define MOUSESPD 5e-4f
#define BACKGRD 0.3, 0.8, 1

static const GLubyte vertBuffData[] = {0, 1, 2, 3};

GLuint vertBuff, offsetBuff, spanBuff, faceBuff, texBuff, vertArrID, progID, matrixID, tex, texID;
GLint *poses;
GLubyte *spans;
GLubyte *faces;
GLubyte *texes;
mat4 viewMat = {0};
GLFWwindow* window;
static vec3 pos = {0, 0, 5}, dir, right, up;
static double hAngle = M_PI, vAngle = 0, keyStates[6] = {-1, -1, -1, -1, -1, -1};

extern uint8_t drawDist;
extern size_t size;

void
updateView()
{
	vec3 tmpV;
	double tmpT;

	dir[0] = cos(vAngle) * sin(hAngle);
	dir[1] = sin(vAngle);
	dir[2] = cos(vAngle) * cos(hAngle);
	right[0] = sin(hAngle - M_PI / 2);
	right[1] = 0;
	right[2] = cos(hAngle - M_PI / 2);
	cross(right, dir, up);

	tmpT = glfwGetTime();
	if (keyStates[0] >= 0) {
		v3scale((tmpT - keyStates[0]) * SPD, dir, tmpV);
		v3addeq(tmpV, pos);
		keyStates[0] = tmpT;
	} if (keyStates[1] >= 0) {
		v3scale((keyStates[1] - tmpT) * SPD, dir, tmpV);
		v3addeq(tmpV, pos);
		keyStates[1] = tmpT;
	} if (keyStates[2] >= 0) {
		v3scale((tmpT - keyStates[2]) * SPD, right, tmpV);
		v3addeq(tmpV, pos);
		keyStates[2] = tmpT;
	} if (keyStates[3] >= 0) {
		v3scale((keyStates[3] - tmpT) * SPD, right, tmpV);
		v3addeq(tmpV, pos);
		keyStates[3] = tmpT;
	} if (keyStates[4] >= 0) {
		v3scale((tmpT - keyStates[4]) * SPD, up, tmpV);
		v3addeq(tmpV, pos);
		keyStates[4] = tmpT;
	} if (keyStates[5] >= 0) {
		v3scale((keyStates[5] - tmpT) * SPD, up, tmpV);
		v3addeq(tmpV, pos);
		keyStates[5] = tmpT;
	}

	v3add(pos, dir, tmpV);
	lookAt(pos, tmpV, up, viewMat);
}

void
handleMouse(GLFWwindow *win, double x, double y)
{
	static double px = SCREEN_W / 2, py = SCREEN_H / 2;

	hAngle += MOUSESPD * (px - x);
	vAngle += MOUSESPD * (py - y);
	px = x;
	py = y;
	dir[0] = cos(vAngle) * sin(hAngle);
	dir[1] = sin(vAngle);
	dir[2] = cos(vAngle) * cos(hAngle);
	right[0] = sin(hAngle - M_PI / 2);
	right[1] = 0;
	right[2] = cos(hAngle - M_PI / 2);
	cross(right, dir, up);
}

void
handleKey(GLFWwindow *win, int key, int code, int act, int mods)
{
	vec3 tmp;

	if (act == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_W:
			keyStates[0] = glfwGetTime();
			break;
		case GLFW_KEY_S:
			keyStates[1] = glfwGetTime();
			break;
		case GLFW_KEY_D:
			keyStates[2] = glfwGetTime();
			break;
		case GLFW_KEY_A:
			keyStates[3] = glfwGetTime();
			break;
		case GLFW_KEY_LEFT_SHIFT:
			keyStates[4] = glfwGetTime();
			break;
		case GLFW_KEY_LEFT_CONTROL:
			keyStates[5] = glfwGetTime();
		}
	} else if (act == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_W:
			v3scale((glfwGetTime() - keyStates[0]) * SPD, dir, tmp);
			v3addeq(tmp, pos);
			keyStates[0] = -1;
			break;
		case GLFW_KEY_S:
			v3scale((keyStates[1] - glfwGetTime()) * SPD, dir, tmp);
			v3addeq(tmp, pos);
			keyStates[1] = -1;
			break;
		case GLFW_KEY_D:
			v3scale((glfwGetTime() - keyStates[2]) * SPD, right, tmp);
			v3addeq(tmp, pos);
			keyStates[2] = -1;
			break;
		case GLFW_KEY_A:
			v3scale((keyStates[3] - glfwGetTime()) * SPD, right, tmp);
			v3addeq(tmp, pos);
			keyStates[3] = -1;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			v3scale((glfwGetTime() - keyStates[4]) * SPD, up, tmp);
			v3addeq(tmp, pos);
			keyStates[4] = -1;
			break;
		case GLFW_KEY_LEFT_CONTROL:
			v3scale((keyStates[5] - glfwGetTime()) * SPD, up, tmp);
			v3addeq(tmp, pos);
			keyStates[5] = -1;
			break;
		case GLFW_KEY_RIGHT:
			move(EAST);
			updateMeshes();
			writeMeshes();
			break;
		case GLFW_KEY_LEFT:
			move(WEST);
			updateMeshes();
			writeMeshes();
			break;
		case GLFW_KEY_UP:
			move(NORTH);
			updateMeshes();
			writeMeshes();
			break;
		case GLFW_KEY_DOWN:
			move(SOUTH);
			updateMeshes();
			writeMeshes();
			break;
		}
	}
}

short
initgl()
{
	if (!glfwInit()) {
		fprintf(stderr, "Error initializing GLFW!\n");
		return -1;
	}
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy (not needed)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

	window = glfwCreateWindow(SCREEN_W, SCREEN_H, NAME, NULL, NULL);
	if (!window) {
		fprintf(stderr, "Failed to open window!\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Error initializing GLEW!\n");
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, SCREEN_W / 2, SCREEN_H / 2);
	glfwSetKeyCallback(window, handleKey);
	glfwSetCursorPosCallback(window, handleMouse);
	glClearColor(BACKGRD, 0);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE);

	handleMouse(0, SCREEN_W / 2, SCREEN_H / 2);

	glGenVertexArrays(1, &vertArrID);
	glBindVertexArray(vertArrID);

	progID = loadShaders("shaders/vert", "shaders/frag");
	matrixID = glGetUniformLocation(progID, "MVP");

/* Construct matrices */
	tex = loadPng("assets/testTex.png");
	texID = glGetUniformLocation(progID, "myTextureSampler");
/* Allocate Buffers */
	glGenBuffers(1, &vertBuff);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertBuffData), vertBuffData, GL_STATIC_DRAW);
	allocBuffs();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(texID, 0);

	glUseProgram(progID);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	return 0;
}

GLuint
loadShaders(const char *vertfn, const char *fragfn)
{
	FILE *vertfp, *fragfp;
	char *vertShaderCode, *fragShaderCode, *errLog;
	size_t vertShaderLen, fragShaderLen;
	int logLen;
	GLint progID, result = GL_FALSE;
	GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

/* Load shaders into memory */
	vertfp = fopen(vertfn, "rb");
	fseek(vertfp, 0, SEEK_END);
	vertShaderLen = ftell(vertfp);
	rewind(vertfp);

	vertShaderCode = malloc(vertShaderLen + 1);
	fread(vertShaderCode, 1, vertShaderLen, vertfp);
	vertShaderCode[vertShaderLen] = 0;

	fragfp = fopen(fragfn, "rb");
	fseek(fragfp, 0, SEEK_END);
	fragShaderLen = ftell(fragfp);
	rewind(fragfp);

	fragShaderCode = malloc(fragShaderLen + 1);
	fread(fragShaderCode, 1, fragShaderLen, fragfp);
	fragShaderCode[fragShaderLen] = 0;

	fclose(vertfp);
	fclose(fragfp);

/* Compile shaders */
	glShaderSource(vertShaderID, 1, (const char * const*)&vertShaderCode, 0);
	glCompileShader(vertShaderID);
	glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertShaderID, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		errLog = malloc(logLen + 1);
		glGetShaderInfoLog(vertShaderID, logLen, 0, errLog);
		fprintf(stderr, "Error with vertex shader: %s\n", errLog);
		free(errLog);
	}

	glShaderSource(fragShaderID, 1, (const char * const*)&fragShaderCode, 0);
	glCompileShader(fragShaderID);
	glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		errLog = malloc(logLen + 1);
		glGetShaderInfoLog(fragShaderID, logLen, 0, errLog);
		fprintf(stderr, "Error with fragment shader: %s\n", errLog);
		free(errLog);
	}
	free(vertShaderCode);
	free(fragShaderCode);

/* Link program */
	progID = glCreateProgram();
	glAttachShader(progID, vertShaderID);
	glAttachShader(progID, fragShaderID);
	glLinkProgram(progID);

/* Check program */
	glGetProgramiv(progID, GL_LINK_STATUS, &result);
	glGetProgramiv(progID, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		errLog = malloc(logLen + 1);
		glGetProgramInfoLog(progID, logLen, 0, errLog);
		fprintf(stderr, "Error with program: %s\n", errLog);
		free(errLog);
	}

	glDetachShader(progID, vertShaderID);
	glDetachShader(progID, fragShaderID);

	glDeleteShader(vertShaderID);
	glDeleteShader(fragShaderID);

	return progID;
}

void
perspec(float angle, float r, float near, float far, mat4 out)
{
	float a = tan(angle / 2);
	out[0] = 1 / (r * a);
	out[5] = 1 / a;
	out[10] = (near + far) / (near - far);
	out[11] = 2 * near * far / (near - far);
	out[14] = -1;
}

void
lookAt(const vec3 eye, const vec3 center, const vec3 up, mat4 out)
{
	vec3 f, s, u;

	v3sub(center, eye, f);
	v3norm(f);
	cross(f, up, s);
	v3norm(s);
	cross(s, f, u);

	out[0] = s[0];
	out[1] = s[1];
	out[2] = s[2];
	out[4] = u[0];
	out[5] = u[1];
	out[6] = u[2];
	out[8] = -f[0];
	out[9] = -f[1];
	out[10] = -f[2];
	out[3] = -v3dot(s, eye);
	out[7] = -v3dot(u, eye);
	out[11] = v3dot(f, eye);
	out[15] = 1;
}

void
translate(const vec3 d, mat4 m)
{
	m[3] = d[0];
	m[7] = d[1];
	m[11] = d[2];
}

void
rotate(const vec4 q, mat4 m)
{
	float n = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3],
		s = (n > 0) ? 2 / n : 0;

	m[0] = 1 - s * (q[1] * q[1] - q[2] * q[2]);
	m[1] = s * (q[0] * q[1] - q[2] * q[3]);
	m[2] = s * (q[0] * q[2] + q[1] * q[3]);
	m[4] = s * (q[0] * q[1] + q[2] * q[3]);
	m[5] = 1 - s * (q[0] * q[0] - q[2] * q[2]);
	m[6] = s * (q[1] * q[2] - q[0] * q[3]);
	m[8] = s * (q[0] * q[2] - q[1] * q[3]);
	m[9] = s * (q[1] * q[2] + q[0] * q[3]);
	m[10] = 1 - s * (q[0] * q[0] - q[1] * q[1]);
}

GLuint
loadPng(const char *const restrict fn)
{
	FILE *fp = fopen(fn, "rb");
	png_structp pngp;
	png_infop info;
	png_uint_32 w, h;
	int bitDepth, coloType, interlaceMethod, compressMethod, filterMethod;
	png_bytepp rows;
	GLuint texID = 0;
	char *data, n;

	if (!fp)
		return 0;
	pngp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!pngp)
		goto quit1;
	info = png_create_info_struct(pngp);
	if (!info)
		goto quit2;

	png_init_io(pngp, fp);
	png_read_png(pngp, info, 0, 0);
	png_get_IHDR(pngp, info, &w, &h, &bitDepth, &coloType, &interlaceMethod,
		&compressMethod, &filterMethod);
	if (coloType != PNG_COLOR_TYPE_RGB && coloType != PNG_COLOR_TYPE_RGBA)
		goto quit2;
	n = (coloType == PNG_COLOR_TYPE_RGBA) + 3;
	rows = png_get_rows(pngp, info);

	data = malloc(3 * w * h);
	for (size_t i = 0; i < h; ++i) {
		for (size_t j = 0; j < w; ++j) {
			data[3 * (i * w + j)] = rows[h - i - 1][n * j];
			data[3 * (i * w + j) + 1] = rows[h - i - 1][n * j + 1];
			data[3 * (i * w + j) + 2] = rows[h - i - 1][n * j + 2];
		}
	}

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
		GL_UNSIGNED_BYTE, data);
	free(data);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
quit2:
	png_destroy_read_struct(&pngp, &info, 0);
quit1:
	fclose(fp);
	return texID;
}

void
allocBuffs()
{
	size_t maxFaces = CUBE(drawDist - 2) * MAXMESH;
	glGenBuffers(1, &offsetBuff);
	glBindBuffer(GL_ARRAY_BUFFER, offsetBuff);
	glBufferData(GL_ARRAY_BUFFER, 3 * maxFaces * sizeof(GLint), 0, GL_STATIC_DRAW);
	glVertexAttribDivisor(1, 1);

	glGenBuffers(1, &spanBuff);
	glBindBuffer(GL_ARRAY_BUFFER, spanBuff);
	glBufferData(GL_ARRAY_BUFFER, 2 * maxFaces, 0, GL_DYNAMIC_DRAW);
	glVertexAttribDivisor(2, 1);

	glGenBuffers(1, &faceBuff);
	glBindBuffer(GL_ARRAY_BUFFER, faceBuff);
	glBufferData(GL_ARRAY_BUFFER, maxFaces, 0, GL_DYNAMIC_DRAW);
	glVertexAttribDivisor(3, 1);
	
	glGenBuffers(1, &texBuff);
	glBindBuffer(GL_ARRAY_BUFFER, texBuff);
	glBufferData(GL_ARRAY_BUFFER, maxFaces, 0, GL_DYNAMIC_DRAW);
	glVertexAttribDivisor(4, 1);
}

void
delBuffs()
{
	glDeleteBuffers(1, &offsetBuff);
	glDeleteBuffers(1, &spanBuff);
	glDeleteBuffers(1, &faceBuff);
	glDeleteBuffers(1, &texBuff);
}

void
writeMeshes()
{
	glBindBuffer(GL_ARRAY_BUFFER, offsetBuff);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * size * sizeof(GLint), poses);
	glBindBuffer(GL_ARRAY_BUFFER, spanBuff);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * size, spans);
	glBindBuffer(GL_ARRAY_BUFFER, faceBuff);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, faces);
	glBindBuffer(GL_ARRAY_BUFFER, texBuff);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, texes);
}

void
renderLoop()
{
	mat4 perspecM = {0}, mvp;
/*
	size_t nFrames = 0;
	double currT, lastT = glfwGetTime();
*/

	perspec(M_PI / 4, 4.0 / 3, 0.1, 100.0, perspecM);
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

		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, size);
		glfwSwapBuffers(window);

		glfwPollEvents();
/*
		currT = glfwGetTime();
		++nFrames;
		if (currT - lastT >= 1.0) {
			printf("%f\n", 1000.0 / (double)nFrames);
			nFrames = 0;
			lastT += 1.0;
		}
*/
	} while (!glfwWindowShouldClose(window));
}

void
cleanupgl()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDeleteBuffers(1, &vertBuff);
	delBuffs();
	glDeleteVertexArrays(1, &vertArrID);
	glDeleteTextures(1, &tex);
	glDeleteProgram(progID);
	glfwTerminate();
}
