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

enum blocks { NONE, STONE, DIRT, GRASS, SAND, LOG, WOOD, COAL, IRON };
enum dirs { NORTH, SOUTH, EAST, WEST, UP, DOWN };

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
void chunkMeshes(int ix, int iy, int iz);
size_t genMeshes();
size_t updateMeshes();

#endif
