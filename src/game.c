#include "stuff.h"
#include <stdlib.h>
#define ALLOC 64

uint8_t chunk[CHUNK][CHUNK][CHUNK];

char
isVisable(uint8_t x, uint8_t y, uint8_t z, uint8_t f)
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
	for (uint8_t i = 0; i < CHUNK; ++i) {
		for (uint8_t j = 0; j < CHUNK; ++j) {
			for (uint8_t k = 0; k < CHUNK; ++k)
				chunk[i][j][k] = j & 3;
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
				if (chunk[x][y][z]) {
					for (uint8_t i = 0; i < 6; ++i) {
						if (!(meshedFaces[x][y][z] & (1 << i)) && isVisable(x, y, z, i)) {
							t = chunk[x][y][z];
							if (size == allocSz) {
								*pos = realloc(*pos, 3 * (allocSz += ALLOC) * sizeof(GLint));
								*spans = realloc(*spans, 2 * allocSz);
								*faces = realloc(*faces, allocSz);
								*tex = realloc(*tex, allocSz);
							}
							switch (i) {
							case 0:
								for (n = 0; n < CHUNK - x &&
									tile(x + n, y, z, meshedFaces, 0, t); ++n)
									meshedFaces[x + n][y][z] |= 1;
								for (m = 1; m < CHUNK - y &&
									stripX(x, y + m, z, n, meshedFaces, 0, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x + j][y + m][z] |= 1;
								}
								(*pos)[3 * size] = x;
								(*pos)[3 * size + 1] = y;
								(*pos)[3 * size + 2] = z;
								(*spans)[2 * size] = n;
								(*spans)[2 * size + 1] = m;
								break;
							case 1:
								for (n = 0; n < CHUNK - y &&
									tile(x, y + n, z, meshedFaces, 1, t); ++n)
									meshedFaces[x][y + n][z] |= 2;
								for (m = 1; m < CHUNK - z &&
									stripY(x, y, z + m, n, meshedFaces, 1, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x][y + j][z + m] |= 2;
								}
								(*pos)[3 * size] = x + 1;
								(*pos)[3 * size + 1] = y;
								(*pos)[3 * size + 2] = z + m - 1;
								(*spans)[2 * size] = m;
								(*spans)[2 * size + 1] = n;
								break;
							case 2:
								for (n = 0; n < CHUNK - x &&
									tile(x + n, y, z, meshedFaces, 2, t); ++n)
									meshedFaces[x + n][y][z] |= 4;
								for (m = 1; m < CHUNK - y &&
									stripX(x, y + m, z, n, meshedFaces, 2, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x + j][y + m][z] |= 4;
								}
								(*pos)[3 * size] = x + n;
								(*pos)[3 * size + 1] = y;
								(*pos)[3 * size + 2] = z - 1;
								(*spans)[2 * size] = n;
								(*spans)[2 * size + 1] = m;
								break;
							case 3:
								for (n = 0; n < CHUNK - y &&
									tile(x, y + n, z, meshedFaces, 3, t); ++n)
									meshedFaces[x][y + n][z] |= 8;
								for (m = 1; m < CHUNK - z &&
									stripY(x, y, z + m, n, meshedFaces, 3, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x][y + j][z + m] |= 8;
								}
								(*pos)[3 * size] = x;
								(*pos)[3 * size + 1] = y;
								(*pos)[3 * size + 2] = z - 1;
								(*spans)[2 * size] = m;
								(*spans)[2 * size + 1] = n;
								break;
							case 4:
								for (n = 0; n < CHUNK - x &&
									tile(x + n, y, z, meshedFaces, 4, t); ++n)
									meshedFaces[x + n][y][z] |= 16;
								for (m = 1; m < CHUNK - z &&
									stripX(x, y, z + m, n, meshedFaces, 4, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x + j][y][z + m] |= 16;
								}
								(*pos)[3 * size] = x;
								(*pos)[3 * size + 1] = y + 1;
								(*pos)[3 * size + 2] = z + m - 1;
								(*spans)[2 * size] = n;
								(*spans)[2 * size + 1] = m;
								break;
							case 5:
								for (n = 0; n < CHUNK - x &&
									tile(x + n, y, z, meshedFaces, 5, t); ++n)
									meshedFaces[x + n][y][z] |= 32;
								for (m = 1; m < CHUNK - z &&
									stripX(x, y, z + m, n, meshedFaces, 5, t); ++m) {
									for (uint8_t j = 0; j < n; ++j)
										meshedFaces[x + j][y][z + m] |= 32;
								}
								(*pos)[3 * size] = x;
								(*pos)[3 * size + 1] = y;
								(*pos)[3 * size + 2] = z - 1;
								(*spans)[2 * size] = n;
								(*spans)[2 * size + 1] = m;
							}
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
