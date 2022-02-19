#ifndef STUFF_H
#define STUFF_H
#include <mylo.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#define M_PI 3.14159265358979323846264338327
#define CHUNK 32
#define CHUNKSZ CHUNK * CHUNK * CHUNK

enum blocks { NONE, STONE, DIRT, GRASS, SAND, LOG, WOOD, COAL, IRON };

void updateView();
short init();
GLuint loadShaders(const char *, const char *);
void perspec(float, float, float, float, mat4);
void lookAt(const vec3, const vec3, const vec3, mat4);
void translate(const vec3, mat4);
void rotate(const vec4, mat4);
GLuint loadPng(const char *const fn);

void initGame();
void cleanup();
size_t genMeshes(GLint **pos, GLubyte **spans, GLubyte **faces, GLubyte **tex);

#endif
