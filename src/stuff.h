#ifndef STUFF_H
#define STUFF_H
#include <mylo.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#define M_PI 3.14159265358979323846264338327
#define CHUNK 32
#define CHUNKSZ (CHUNK * CHUNK * CHUNK)
#define SQAR(X) ((X) * (X))
#define CUBE(X) ((X) * (X) * (X))
#define MAXMESH (CHUNKSZ * 3 / 4)

typedef struct {
	size_t offset;
	uint8_t x, y, z;
} meshInfo;

enum blocks { NONE, STONE, DIRT, GRASS, SAND, LOG, WOOD, COAL, IRON };
enum dirs { NORTH, SOUTH, WEST, EAST, UP, DOWN };

void updateView();
short initgl();
GLuint loadShaders(const char *, const char *);
void lookAt(const vec3, const vec3, const vec3, mat4);
void translate(const vec3, mat4);
void rotate(const vec4, mat4);
GLuint loadPng(const char *const fn);
void allocBuffs();
void delBuffs();
void writeMeshes();
void renderLoop();
void cleanupgl();

void move(uint8_t dir);
void init();
void cleanup();
void chunkMeshes(uint8_t ix, uint8_t iy, uint8_t iz);
size_t genMeshes();
size_t updateMeshes();
void updateGame();

#endif
