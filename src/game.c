#include "stuff.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <noise.h>
#define ALLOC 64
#define SCALE 3e-4
#define OCTS 4
#define AMP 384
#define H(X, Y) (AMP * (noise(SCALE * (X), SCALE * (Y), offsets, OCTS, 0.4, 5) - 0.45))
#define MESHD (drawDist - 2)

uint8_t (*chunks)[CHUNK][CHUNK][CHUNK];
uint16_t *chunkMap;
uint8_t drawDist = 9;
int d[3] = {0};
extern vec3 pos;
extern GLint *poses;
extern GLubyte *spans;
extern GLubyte *faces;
extern GLubyte *texes;
meshInfo *meshMap;
size_t size;
static size_t allocSz;
static float offsets[OCTS][2];
static const uint8_t texMap[][3] = {{0, 0, 0}, {1, 1, 1}, {2, 3, 1}, {4, 4, 4}};

void
updateGame()
{
	char update = 0;
	if (pos[0] - d[0] * CHUNK > CHUNK) {
		move(WEST);
		update = 1;
	} else if (pos[0] - d[0] * CHUNK < 0) {
		move(EAST);
		update = 1;
	}

	if (pos[2] - d[2] * CHUNK > CHUNK) {
		move(NORTH);
		update = 1;
	} else if (pos[2] - d[2] * CHUNK < 0) {
		move(SOUTH);
		update = 1;
	}

	if (update)
		writeMeshes();
}

char
isVisable(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t f)
{
	switch (f) {
	case 0:
		return !(z != CHUNK - 1 ? chunks[chunkMap[idx]][x][y][z + 1] : chunks[chunkMap[idx + SQAR(drawDist)]][x][y][0]);
	case 1:
		return !(x != CHUNK - 1 ? chunks[chunkMap[idx]][x + 1][y][z] : chunks[chunkMap[idx + 1]][0][y][z]);
	case 2:
		return !(z ? chunks[chunkMap[idx]][x][y][z - 1] : chunks[chunkMap[idx - SQAR(drawDist)]][x][y][CHUNK - 1]);
	case 3:
		return !(x ? chunks[chunkMap[idx]][x - 1][y][z] : chunks[chunkMap[idx - 1]][CHUNK - 1][y][z]);
	case 4:
		return !(y != CHUNK - 1 ? chunks[chunkMap[idx]][x][y + 1][z] : chunks[chunkMap[idx + drawDist]][x][0][z]);
	case 5:
		return !(y ? chunks[chunkMap[idx]][x][y - 1][z] : chunks[chunkMap[idx - drawDist]][x][CHUNK - 1][z]);
	}
	return 0;
}

inline char
tile(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	return !((mFaces[x][y][z] >> face) & 1) && isVisable(idx, x, y, z, face) && chunks[chunkMap[idx]][x][y][z] == t;
}

inline char
stripX(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t len, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	uint8_t i;
	for (i = 0; i < len && !((mFaces[x + i][y][z] >> face) & 1) &&
		isVisable(idx, x + i, y, z, face) && chunks[chunkMap[idx]][x + i][y][z] == t; ++i);
	return i == len;
}

inline char
stripY(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t len, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	uint8_t i;
	for (i = 0; i < len && !((mFaces[x][y + i][z] >> face) & 1) &&
		isVisable(idx, x, y + i, z, face) && chunks[chunkMap[idx]][x][y + i][z] == t; ++i);
	return i == len;
}

uint8_t
getBlock(long y, int h)
{
	if (y >= h)
		return NONE;
	if (h < 5)
		return SAND;
	if (h > 90)
		return STONE;
	return (y + 2 >= h) + (y + 1 == h) + 1;
}

void
genChunk(uint8_t i, uint8_t j, uint8_t k)
{
	uint16_t idx = chunkMap[i + drawDist * (j + drawDist * k)];
	long h, blockH = CHUNK * (d[1] - drawDist / 2 + j),
	     X = CHUNK * (d[0] - drawDist / 2 + i),
	     Z = CHUNK * (d[2] - drawDist / 2 + k);
	for (uint8_t x = 0; x < CHUNK; ++x) {
		for (uint8_t z = 0; z < CHUNK; ++z) {
			h = (long)H(X + x, Z + z);
			for (long y = 0; y < CHUNK; ++y)
				chunks[idx][x][y][z] = getBlock(blockH + y, h);
		}
	}
}

void
init()
{
	chunks = malloc(sizeof(uint8_t[CUBE(drawDist)]
		[CHUNK][CHUNK][CHUNK]));
	chunkMap = malloc(CUBE(drawDist) * sizeof(uint16_t));
	meshMap = malloc(CUBE(MESHD) * sizeof(meshInfo));
	for (uint16_t i = 0; i < CUBE(drawDist); ++i)
		chunkMap[i] = i;
	initPerlin(3);
	randCoords(offsets, -100000, 100000, OCTS);
	for (uint8_t i = 0; i < drawDist; ++i) {
		for (uint8_t j = 0; j < drawDist; ++j) {
			for (uint8_t k = 0; k < drawDist; ++k)
				genChunk(i, j, k);
		}
	}
	pos[1] = H(0, 0);
}

void
cleanup()
{
	free(chunks);
	free(chunkMap);

	free(poses);
	free(spans);
	free(faces);
	free(texes);
	free(meshMap);
}

void
chunkMeshes(uint8_t ix, uint8_t iy, uint8_t iz)
{
	uint8_t meshedFaces[CHUNK][CHUNK][CHUNK] = {0},
		n, m, t;
	int idx = drawDist * (drawDist * iz + iy) + ix;

	for (int z = 0; z < CHUNK; ++z) {
	for (int y = 0; y < CHUNK; ++y) {
	for (int x = 0; x < CHUNK; ++x) {
		if ((t = chunks[chunkMap[idx]][x][y][z])) {
			for (uint8_t i = 0; i < 6; ++i) {
				if (!(meshedFaces[x][y][z] & (1 << i)) && isVisable(idx, x, y, z, i)) {
					if (size == allocSz) {
						poses = realloc(poses, 3 * (allocSz += ALLOC) * sizeof(GLint));
						spans = realloc(spans, 2 * allocSz);
						faces = realloc(faces, allocSz);
						texes = realloc(texes, allocSz);
					}
					switch (i) {
					case 0:
					case 2:
						for (n = 0; n < CHUNK - x &&
							tile(idx, x + n, y, z, meshedFaces, i, t); ++n)
							meshedFaces[x + n][y][z] |= (1 << i);
						for (m = 1; m < CHUNK - y &&
							stripX(idx, x, y + m, z, n, meshedFaces, i, t); ++m) {
							for (uint8_t j = 0; j < n; ++j)
								meshedFaces[x + j][y + m][z] |= (1 << i);
						}
						poses[3 * size] = CHUNK * (ix - drawDist / 2 + d[0]) + x + (i == 2) * n;
						poses[3 * size + 1] = CHUNK * (iy - drawDist / 2 + d[1]) + y;
						poses[3 * size + 2] = CHUNK * (iz - drawDist / 2 + d[2]) + z - (i == 2);
						break;
					case 1:
					case 3:
						for (n = 0; n < CHUNK - y &&
							tile(idx, x, y + n, z, meshedFaces, i, t); ++n)
							meshedFaces[x][y + n][z] |= (1 << i);
						for (m = 1; m < CHUNK - z &&
							stripY(idx, x, y, z + m, n, meshedFaces, i, t); ++m) {
							for (uint8_t j = 0; j < n; ++j)
								meshedFaces[x][y + j][z + m] |= (1 << i);
						}
						poses[3 * size] = CHUNK * (ix - drawDist / 2 + d[0]) + x + (i == 1);
						poses[3 * size + 1] = CHUNK * (iy - drawDist / 2 + d[1]) + y;
						poses[3 * size + 2] = CHUNK * (iz - drawDist / 2 + d[2]) + z + (i == 1) * m - 1;
						break;
					case 4:
					case 5:
						for (n = 0; n < CHUNK - x &&
							tile(idx, x + n, y, z, meshedFaces, i, t); ++n)
							meshedFaces[x + n][y][z] |= (1 << i);
						for (m = 1; m < CHUNK - z &&
							stripX(idx, x, y, z + m, n, meshedFaces, i, t); ++m) {
							for (uint8_t j = 0; j < n; ++j)
								meshedFaces[x + j][y][z + m] |= (1 << i);
						}
						poses[3 * size] = CHUNK * (ix - drawDist / 2 + d[0]) + x;
						poses[3 * size + 1] = CHUNK * (iy - drawDist / 2 + d[1]) + y + (i == 4);
						poses[3 * size + 2] = CHUNK * (iz - drawDist / 2 + d[2]) + z + (i == 4) * m - 1;
					}
					spans[2 * size] = (i & 1) && i != 5 ? m : n;
					spans[2 * size + 1] = (i & 1) && i != 5 ? n : m;
					faces[size] = i;
					texes[size++] = texMap[t - 1][(i >= 4) + (i == 5)];
				}
			}
		}
	}}}
}

size_t
genMeshes()
{
	allocSz = ALLOC;
	poses = malloc(3 * ALLOC * sizeof(GLint));
	spans = malloc(2 * ALLOC);
	faces = malloc(ALLOC);
	texes = malloc(ALLOC);
	return updateMeshes();
}

size_t
updateMeshes()
{
	uint16_t idx = 0;
	size = 0;
	for (uint8_t iz = 1; iz < drawDist - 1; ++iz) {
	for (uint8_t iy = 1; iy < drawDist - 1; ++iy) {
	for (uint8_t ix = 1; ix < drawDist - 1; ++ix) {
		meshMap[idx].offset = size;
		meshMap[idx].x = ix;
		meshMap[idx].y = iy;
		meshMap[idx++].z = iz;
		chunkMeshes(ix, iy, iz);
	}}}
	return size;
}
/*
void
packMoveMeshes(uint8_t dir)
{
	// WEST
	for (x = 0; x < CUBE(MESHD) && !meshMap[x].x; ++x);
	free = meshMap[x++].offset;
	for (; x < CUBE(MESHD); ++x) {
		if (!meshMap[x].x) {
			len = meshMap[x + 1].offset - meshMap[x].offset;
			memmove(&poses[3 * free], &poses[3 * meshMap[x].offset], len * 3 * sizeof(GLint));
			memmove(&spans[2 * free], &spans[meshMap[x].offset], len * 2 * sizeof(GLubyte));
			memmove(&faces[free], &faces[meshMap[x].offset], len * sizeof(GLubyte));
			memmove(&texes[free], &texes[meshMap[x].offset], len * sizeof(GLubyte));
			meshMap[free].offset = free;
			meshMap[x].x = meshMap[x].x - 1;
			meshMap[x].y = meshMap[x].y;
			meshMap[x].z = meshMap[x].z;
			free += len;
		}
	}
}
*/
void
move(uint8_t dir)
{
	uint16_t x, y, z, tmp;
	size_t free, len;

	switch (dir) {
	case NORTH:
		++d[2];
		for (x = 0; x < drawDist; ++x) {
			for (y = 0; y < drawDist; ++y) {
				tmp = chunkMap[drawDist * y + x];
				for (z = 0; z < drawDist - 1; ++z)
					chunkMap[drawDist * (drawDist * z + y) + x] =
					chunkMap[drawDist * (drawDist * (z + 1) + y) + x];
				chunkMap[drawDist * (drawDist * z + y) + x] = tmp;
				genChunk(x, y, z);
			}
		}
		for (x = 0; x < CUBE(MESHD) && meshMap[x].z > 1; ++x)
			--meshMap[x].z;
		free = meshMap[z = x++].offset;
		for (; x < CUBE(MESHD); ++x) {
			if (meshMap[x].z > 1) {
				len = (x == CUBE(MESHD) - 1 ? size : meshMap[x + 1].offset) - meshMap[x].offset;
				memmove(&poses[3 * free], &poses[3 * meshMap[x].offset], len * 3 * sizeof(GLint));
				memmove(&spans[2 * free], &spans[2 * meshMap[x].offset], len * 2 * sizeof(GLubyte));
				memmove(&faces[free], &faces[meshMap[x].offset], len * sizeof(GLubyte));
				memmove(&texes[free], &texes[meshMap[x].offset], len * sizeof(GLubyte));
				meshMap[z].offset = free;
				meshMap[z].x = meshMap[x].x;
				meshMap[z].y = meshMap[x].y;
				meshMap[z++].z = meshMap[x].z - 1;
				free += len;
			}
		}
		size = free;
		for (x = 1; x < drawDist - 1; ++x) {
			for (y = 1; y < drawDist - 1; ++y) {
				meshMap[z].offset = size;
				meshMap[z].x = x;
				meshMap[z].y = y;
				meshMap[z++].z = MESHD;
				chunkMeshes(x, y, MESHD);
			}
		}
		break;
	case SOUTH:
		--d[2];
		for (x = 0; x < drawDist; ++x) {
			for (y = 0; y < drawDist; ++y) {
				tmp = chunkMap[drawDist * (drawDist * (drawDist - 1) + y) + x];
				for (z = drawDist - 1; z > 0; --z)
					chunkMap[drawDist * (drawDist * z + y) + x] =
					chunkMap[drawDist * (drawDist * (z - 1) + y) + x];
				chunkMap[drawDist * y + x] = tmp;
				genChunk(x, y, 0);
			}
		}
		for (x = 0; x < CUBE(MESHD) && meshMap[x].z < MESHD; ++x)
			++meshMap[x].z;
		free = meshMap[z = x++].offset;
		for (; x < CUBE(MESHD); ++x) {
			if (meshMap[x].z < MESHD) {
				len = (x == CUBE(MESHD) - 1 ? size : meshMap[x + 1].offset) - meshMap[x].offset;
				memmove(&poses[3 * free], &poses[3 * meshMap[x].offset], len * 3 * sizeof(GLint));
				memmove(&spans[2 * free], &spans[2 * meshMap[x].offset], len * 2 * sizeof(GLubyte));
				memmove(&faces[free], &faces[meshMap[x].offset], len * sizeof(GLubyte));
				memmove(&texes[free], &texes[meshMap[x].offset], len * sizeof(GLubyte));
				meshMap[z].offset = free;
				meshMap[z].x = meshMap[x].x;
				meshMap[z].y = meshMap[x].y;
				meshMap[z++].z = meshMap[x].z + 1;
				free += len;
			}
		}
		size = free;
		for (x = 1; x < drawDist - 1; ++x) {
			for (y = 1; y < drawDist - 1; ++y) {
				meshMap[z].offset = size;
				meshMap[z].x = x;
				meshMap[z].y = y;
				meshMap[z++].z = 1;
				chunkMeshes(x, y, 1);
			}
		}
		break;
	case WEST:
		++d[0];
		for (z = 0; z < drawDist; ++z) {
			for (y = 0; y < drawDist; ++y) {
				tmp = chunkMap[drawDist * (drawDist * z + y)];
				for (x = 0; x < drawDist - 1; ++x)
					chunkMap[drawDist * (drawDist * z + y) + x] =
					chunkMap[drawDist * (drawDist * z + y) + x + 1];
				chunkMap[drawDist * (drawDist * z + y) + x] = tmp;
				genChunk(x, y, z);
			}
		}
		for (z = 0; z < CUBE(MESHD) && meshMap[z].x > 1; ++z)
			--meshMap[z].x;
		free = meshMap[x = z++].offset;
		for (; z < CUBE(MESHD); ++z) {
			if (meshMap[z].x > 1) {
				len = (z == CUBE(MESHD) - 1 ? size : meshMap[z + 1].offset) - meshMap[z].offset;
				memmove(&poses[3 * free], &poses[3 * meshMap[z].offset], len * 3 * sizeof(GLint));
				memmove(&spans[2 * free], &spans[2 * meshMap[z].offset], len * 2 * sizeof(GLubyte));
				memmove(&faces[free], &faces[meshMap[z].offset], len * sizeof(GLubyte));
				memmove(&texes[free], &texes[meshMap[z].offset], len * sizeof(GLubyte));
				meshMap[x].offset = free;
				meshMap[x].x = meshMap[z].x - 1;
				meshMap[x].y = meshMap[z].y;
				meshMap[x++].z = meshMap[z].z;
				free += len;
			}
		}
		size = free;
		for (y = 1; y < drawDist - 1; ++y) {
			for (z = 1; z < drawDist - 1; ++z) {
				meshMap[x].offset = size;
				meshMap[x].x = MESHD;
				meshMap[x].y = y;
				meshMap[x++].z = z;
				chunkMeshes(MESHD, y, z);
			}
		}
		break;
	case EAST:
		--d[0];
		for (z = 0; z < drawDist; ++z) {
			for (y = 0; y < drawDist; ++y) {
				tmp = chunkMap[drawDist * (drawDist * z + y) + drawDist - 1];
				for (x = drawDist - 1; x > 0; --x)
					chunkMap[drawDist * (drawDist * z + y) + x] =
					chunkMap[drawDist * (drawDist * z + y) + x - 1];
				chunkMap[drawDist * (drawDist * z + y)] = tmp;
				genChunk(0, y, z);
			}
		}
		for (z = 0; z < CUBE(MESHD) && meshMap[z].x < MESHD; ++z)
			++meshMap[z].x;
		free = meshMap[x = z++].offset;
		for (; z < CUBE(MESHD); ++z) {
			if (meshMap[z].x < MESHD) {
				len = (z == CUBE(MESHD) - 1 ? size : meshMap[z + 1].offset) - meshMap[z].offset;
				memmove(&poses[3 * free], &poses[3 * meshMap[z].offset], len * 3 * sizeof(GLint));
				memmove(&spans[2 * free], &spans[2 * meshMap[z].offset], len * 2 * sizeof(GLubyte));
				memmove(&faces[free], &faces[meshMap[z].offset], len * sizeof(GLubyte));
				memmove(&texes[free], &texes[meshMap[z].offset], len * sizeof(GLubyte));
				meshMap[x].offset = free;
				meshMap[x].x = meshMap[z].x + 1;
				meshMap[x].y = meshMap[z].y;
				meshMap[x++].z = meshMap[z].z;
				free += len;
			}
		}
		size = free;
		for (y = 1; y < drawDist - 1; ++y) {
			for (z = 1; z < drawDist - 1; ++z) {
				meshMap[x].offset = size;
				meshMap[x].x = 1;
				meshMap[x].y = y;
				meshMap[x++].z = z;
				chunkMeshes(1, y, z);
			}
		}
		break;
	case UP:
		break;
	case DOWN:
		break;
	}
}
