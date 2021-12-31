#include "stuff.h"
#include <stdlib.h>
#define ALLOC 64

uint8_t chunk[CHUNK][CHUNK][CHUNK];

char
isVisable(int8_t x, int8_t y, int8_t z, uint8_t f)
{
	switch (f) {
	case 0:
		++z;
		break;
	case 1:
		++x;
		break;
	case 2:
		--z;
		break;
	case 3:
		--x;
		break;
	case 4:
		++y;
		break;
	case 5:
		--y;
	}
	return z >= CHUNK || x >= CHUNK || z < 0 || x < 0 ||
		y >= CHUNK || y < 0 || !chunk[x][y][z];
}

inline char
tile(uint8_t x, uint8_t y, uint8_t z, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	return !((mFaces[x][y][z] >> face) & 1) && isVisable(x, y, z, face) && chunk[x][y][z] == t;
}

inline char
stripX(uint8_t x, uint8_t y, uint8_t z, uint8_t len, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	uint8_t i;
	for (i = 0; i < len && !((mFaces[x + i][y][z] >> face) & 1) &&
		isVisable(x + i, y, z, face) && chunk[x + i][y][z] == t; ++i);
	return i == len;
}

inline char
stripY(uint8_t x, uint8_t y, uint8_t z, uint8_t len, uint8_t mFaces[CHUNK][CHUNK][CHUNK], uint8_t face, uint8_t t)
{
	uint8_t i;
	for (i = 0; i < len && !((mFaces[x][y + i][z] >> face) & 1) &&
		isVisable(x, y + i, z, face) && chunk[x][y + i][z] == t; ++i);
	return i == len;
}

void
genChunk()
{
	for (uint8_t x = 0; x < CHUNK; ++x) {
		for (uint8_t y = 0; y < CHUNK; ++y) {
			for (uint8_t z = 0; z < CHUNK; ++z)
				chunk[x][y][z] = ((z & 3) && (x & 3)) * (y & 3);
		}
	}
}

size_t
genMeshes(GLint **pos, GLubyte **spans, GLubyte **faces, GLubyte **tex)
{
	uint8_t meshedFaces[CHUNK][CHUNK][CHUNK] = {0},
		n, m, t;
	size_t size = 0, allocSz = ALLOC;

	*pos = malloc(3 * ALLOC * sizeof(GLint));
	*spans = malloc(2 * ALLOC);
	*faces = malloc(ALLOC);
	*tex = malloc(ALLOC);
	for (int z = 0; z < CHUNK; ++z) {
		for (int y = 0; y < CHUNK; ++y) {
			for (int x = 0; x < CHUNK; ++x) {
				if ((t = chunk[x][y][z])) {
					for (uint8_t i = 0; i < 6; ++i) {
						if (!(meshedFaces[x][y][z] & (1 << i)) && isVisable(x, y, z, i)) {
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
									tile(x + n, y, z, meshedFaces, i, t); ++n)
									meshedFaces[x + n][y][z] |= (1 << i);
								for (m = 1; m < CHUNK - y &&
									stripX(x, y + m, z, n, meshedFaces, i, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x + j][y + m][z] |= (1 << i);
								}
								(*pos)[3 * size] = x + (i == 2) * n;
								(*pos)[3 * size + 1] = y;
								(*pos)[3 * size + 2] = z - (i == 2);
								break;
							case 1:
							case 3:
								for (n = 0; n < CHUNK - y &&
									tile(x, y + n, z, meshedFaces, i, t); ++n)
									meshedFaces[x][y + n][z] |= (1 << i);
								for (m = 1; m < CHUNK - z &&
									stripY(x, y, z + m, n, meshedFaces, i, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x][y + j][z + m] |= (1 << i);
								}
								(*pos)[3 * size] = x + (i == 1);
								(*pos)[3 * size + 1] = y;
								(*pos)[3 * size + 2] = z + (i == 1) * m - 1;
								break;
							case 4:
							case 5:
								for (n = 0; n < CHUNK - x &&
									tile(x + n, y, z, meshedFaces, i, t); ++n)
									meshedFaces[x + n][y][z] |= (1 << i);
								for (m = 1; m < CHUNK - z &&
									stripX(x, y, z + m, n, meshedFaces, i, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x + j][y][z + m] |= (1 << i);
								}
								(*pos)[3 * size] = x;
								(*pos)[3 * size + 1] = y + (i == 4);
								(*pos)[3 * size + 2] = z + (i == 4) * m - 1;
							}
							(*spans)[2 * size] = (i & 1) && i != 5 ? m : n;
							(*spans)[2 * size + 1] = (i & 1) && i != 5 ? n : m;
							(*faces)[size] = i;
							(*tex)[size++] = t - 1;
						}
					}
				}
			}
		}
	}
	return size;
}
