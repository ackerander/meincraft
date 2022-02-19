#include "stuff.h"
#include <stdlib.h>
#include <time.h>
#include <noise.h>
#define ALLOC 64
#define SCALE 3e-4
#define OCTS 4

uint8_t (*chunks)[CHUNK][CHUNK][CHUNK];
uint8_t drawDist = 7;
int d[3] = {0};
static float offsets[OCTS][2];
static const uint8_t texMap[][3] = {{0, 0, 0}, {1, 1, 1}, {2, 3, 1}, {4, 4, 4}};

inline char
isVisable(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t f)
{
	uint8_t *c = f < 4 ? (f & 1 ? &x : &z) : &y;
	return (*c += ((38 >> f) & 2) - 1) >= CHUNK || !chunks[idx][x][y][z];
}

inline char
tile(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	return !((mFaces[x][y][z] >> face) & 1) && isVisable(idx, x, y, z, face) && chunks[idx][x][y][z] == t;
}

inline char
stripX(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t len, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	uint8_t i;
	for (i = 0; i < len && !((mFaces[x + i][y][z] >> face) & 1) &&
		isVisable(idx, x + i, y, z, face) && chunks[idx][x + i][y][z] == t; ++i);
	return i == len;
}

inline char
stripY(int idx, uint8_t x, uint8_t y, uint8_t z, uint8_t len, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	uint8_t i;
	for (i = 0; i < len && !((mFaces[x][y + i][z] >> face) & 1) &&
		isVisable(idx, x, y + i, z, face) && chunks[idx][x][y + i][z] == t; ++i);
	return i == len;
}

uint8_t
getBlock(long y, int h)
{
	if (h < 5)
		return SAND;
	return (h <= 50 && y + 2 >= h) + (y + 1 == h) + 1;
}

void
genChunk(uint8_t i, uint8_t j, uint8_t k)
{
	uint16_t idx = i + drawDist * (j + drawDist * k);
	long h, blockH = CHUNK * (d[1] - drawDist / 2 + j);
	for (uint8_t x = 0; x < CHUNK; ++x) {
		for (uint8_t z = 0; z < CHUNK; ++z) {
			h = (int)(512 * (noise(SCALE * (CHUNK * (d[0] - drawDist / 2 + i) + x),
					SCALE * (CHUNK * (d[2] - drawDist / 2 + k) + z),
					offsets, OCTS, 0.4, 5) - 0.5));
			for (uint8_t y = 0; blockH + y < h && y < CHUNK; ++y)
				chunks[idx][x][y][z] = getBlock(blockH + y, h);
		}
	}
}

void
initGame()
{
	chunks = malloc(sizeof(uint8_t[drawDist * drawDist * drawDist]
		[CHUNK][CHUNK][CHUNK]));
	initPerlin(3);
	randCoords(offsets, -100000, 100000, OCTS);
	for (uint8_t i = 0; i < drawDist; ++i) {
		for (uint8_t j = 0; j < drawDist; ++j) {
			for (uint8_t k = 0; k < drawDist; ++k)
				genChunk(i, j, k);
		}
	}
}

void
cleanup()
{
	free(chunks);
}

size_t
genMeshes(GLint **pos, GLubyte **spans, GLubyte **faces, GLubyte **tex)
{
	uint8_t meshedFaces[CHUNK][CHUNK][CHUNK],
		n, m, t;
	size_t size = 0, allocSz = ALLOC;
	int idx;

	*pos = malloc(3 * ALLOC * sizeof(GLint));
	*spans = malloc(2 * ALLOC);
	*faces = malloc(ALLOC);
	*tex = malloc(ALLOC);
	for (int iz = 1; iz < drawDist - 1; ++iz) {
	for (int iy = 1; iy < drawDist - 1; ++iy) {
	for (int ix = 1; ix < drawDist - 1; ++ix) {
		memset(meshedFaces, 0, sizeof(meshedFaces));
		idx = drawDist * (drawDist * iz + iy) + ix;
		for (int z = 0; z < CHUNK; ++z) {
		for (int y = 0; y < CHUNK; ++y) {
		for (int x = 0; x < CHUNK; ++x) {
			if ((t = chunks[idx][x][y][z])) {
				for (uint8_t i = 0; i < 6; ++i) {
					if (!(meshedFaces[x][y][z] & (1 << i)) && isVisable(idx, x, y, z, i)) {
						if (size == allocSz) {
							*pos = realloc(*pos, 3 * (allocSz += ALLOC) * sizeof(GLint));
							*spans = realloc(*spans, 2 * allocSz);
							*faces = realloc(*faces, allocSz);
							*tex = realloc(*tex, allocSz);
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
							(*pos)[3 * size] = CHUNK * (ix - drawDist / 2 + d[0]) + x + (i == 2) * n;
							(*pos)[3 * size + 1] = CHUNK * (iy - drawDist / 2 + d[1]) + y;
							(*pos)[3 * size + 2] = CHUNK * (iz - drawDist / 2 + d[2]) + z - (i == 2);
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
							(*pos)[3 * size] = CHUNK * (ix - drawDist / 2 + d[0]) + x + (i == 1);
							(*pos)[3 * size + 1] = CHUNK * (iy - drawDist / 2 + d[1]) + y;
							(*pos)[3 * size + 2] = CHUNK * (iz - drawDist / 2 + d[2]) + z + (i == 1) * m - 1;
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
							(*pos)[3 * size] = CHUNK * (ix - drawDist / 2 + d[0]) + x;
							(*pos)[3 * size + 1] = CHUNK * (iy - drawDist / 2 + d[1]) + y + (i == 4);
							(*pos)[3 * size + 2] = CHUNK * (iz - drawDist / 2 + d[2]) + z + (i == 4) * m - 1;
						}
						(*spans)[2 * size] = (i & 1) && i != 5 ? m : n;
						(*spans)[2 * size + 1] = (i & 1) && i != 5 ? n : m;
						(*faces)[size] = i;
						(*tex)[size++] = texMap[t - 1][(i >= 4) + (i == 5)];
						//(*tex)[size++] = t - 1;
					}
				}
			}
		}}}
	}}}
	return size;
}
