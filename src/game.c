#include "stuff.h"
#include <stdlib.h>
#define ALLOC 64

uint8_t chunk[CHUNKSZ];

char
isVisable(size_t i, uint8_t j)
{
	static const int off[6] = {CHUNK * CHUNK, 1, -CHUNK * CHUNK, -1, CHUNK, -CHUNK};
	size_t idx = i + off[j];

	return !(idx < CHUNKSZ && (j != 1 || i % CHUNK != CHUNK - 1) &&
	(j != 3 || i % CHUNK) && (j != 4 || (i / CHUNK) % CHUNK != CHUNK - 1) &&
	(j != 5 || (i / CHUNK) % CHUNK) && chunk[idx]);
}

inline char
tile(size_t i, uint8_t mFaces[], uint8_t face, uint8_t t)
{
	return !((mFaces[i] >> face) & 1) && isVisable(i, face) && chunk[i] == t;
}

inline char
stripX(size_t i, uint8_t len, uint8_t mFaces[], uint8_t face, uint8_t t)
{
	uint8_t j;
	for (j = 0; j < len && !((mFaces[i + j] >> face) & 1) &&
		isVisable(i + j, face) && chunk[i + j] == t; ++j);
	return j == len;
}

inline char
stripY(size_t i, uint8_t len, uint8_t mFaces[], uint8_t face, uint8_t t)
{
	uint8_t j;
	for (j = 0; j < len && !((mFaces[i + j*CHUNK] >> face) & 1) &&
		isVisable(i + j*CHUNK, face) && chunk[i + j*CHUNK] == t; ++j);
	return j == len;
}

void
genChunk()
{
	for (size_t i = 0; i < CHUNKSZ; ++i)
		chunk[i] = i / CHUNK & 3;
}

size_t
genMeshes(GLint **pos, GLubyte **spans, GLubyte **faces, GLubyte **tex)
{
	uint8_t meshedFaces[CHUNKSZ] = {0},
		n, m, t;
	size_t size = 0, allocSz = ALLOC;
	int posX, posY, posZ;

	*pos = malloc(3 * ALLOC * sizeof(GLint));
	*spans = malloc(2 * ALLOC);
	*faces = malloc(ALLOC);
	*tex = malloc(ALLOC);
	for (size_t i = 0; i < CHUNKSZ; ++i) {
		if (chunk[i]) { // Face 0
			posX = i % CHUNK;
			posY = i / CHUNK % CHUNK;
			posZ = i / (CHUNK * CHUNK);
			for (uint8_t j = 0; j < 6; ++j) {
				if (!(meshedFaces[i] & (1 << j)) && isVisable(i, j)) {
					t = chunk[i];
					if (size == allocSz) {
						*pos = realloc(*pos, 3 * (allocSz += ALLOC) * sizeof(GLint));
						*spans = realloc(*spans, 2 * allocSz);
						*faces = realloc(*faces, allocSz);
						*tex = realloc(*tex, allocSz);
					}
					switch (j) {
					case 0:
						for (n = 0; n < CHUNK - posX &&
							tile(i + n, meshedFaces, 0, t); ++n)
							meshedFaces[i + n] |= 1;
						for (m = 1; m < CHUNK - posY &&
							stripX(i + m * CHUNK, n, meshedFaces, 0, t); ++m) {
							for (uint8_t k = 0; k < n; ++k)
								meshedFaces[i + m * CHUNK + k] |= 1;
						}
						(*pos)[3 * size] = posX;
						(*pos)[3 * size + 1] = posY;
						(*pos)[3 * size + 2] = posZ;
						(*spans)[2 * size] = n;
						(*spans)[2 * size + 1] = m;
						break;
					case 1:
						for (n = 0; n < CHUNK - posY &&
							tile(i + n * CHUNK, meshedFaces, 1, t); ++n)
							meshedFaces[i + n * CHUNK] |= 2;
						for (m = 1; m < CHUNK - posZ &&
							stripY(i + m * CHUNK * CHUNK, n, meshedFaces, 1, t); ++m) {
							for (uint8_t k = 0; k < n; ++k)
								meshedFaces[i + (m * CHUNK + k) * CHUNK] |= 2;
						}
						(*pos)[3 * size] = posX + 1;
						(*pos)[3 * size + 1] = posY;
						(*pos)[3 * size + 2] = posZ + m - 1;
						(*spans)[2 * size] = m;
						(*spans)[2 * size + 1] = n;
						break;
					case 2:
						for (n = 0; n < CHUNK - posX &&
							tile(i + n, meshedFaces, 2, t); ++n)
							meshedFaces[i + n] |= 4;
						for (m = 1; m < CHUNK - posY &&
							stripX(i + m * CHUNK, n, meshedFaces, 2, t); ++m) {
							for (uint8_t k = 0; k < n; ++k)
								meshedFaces[i + m * CHUNK + k] |= 4;
						}
						(*pos)[3 * size] = posX + n;
						(*pos)[3 * size + 1] = posY;
						(*pos)[3 * size + 2] = posZ - 1;
						(*spans)[2 * size] = n;
						(*spans)[2 * size + 1] = m;
						break;
					case 3:
						for (n = 0; n < CHUNK - posY &&
							tile(i + n*CHUNK, meshedFaces, 3, t); ++n)
							meshedFaces[i + n*CHUNK] |= 8;
						for (m = 1; m < CHUNK - posZ &&
							stripY(i + m*CHUNK*CHUNK, n, meshedFaces, 3, t); ++m) {
							for (uint8_t k = 0; k < n; ++k)
								meshedFaces[i + (m*CHUNK + k)*CHUNK] |= 8;
						}
						(*pos)[3 * size] = posX;
						(*pos)[3 * size + 1] = posY;
						(*pos)[3 * size + 2] = posZ - 1;
						(*spans)[2 * size] = m;
						(*spans)[2 * size + 1] = n;
						break;
					case 4:
						for (n = 0; n < CHUNK - posX &&
							tile(i + n, meshedFaces, 4, t); ++n)
							meshedFaces[i + n] |= 16;
						for (m = 1; m < CHUNK - posZ &&
							stripX(i + m*CHUNK*CHUNK, n, meshedFaces, 4, t); ++m) {
							for (uint8_t k = 0; k < n; ++k)
								meshedFaces[i + m*CHUNK*CHUNK + k] |= 16;
						}
						(*pos)[3 * size] = posX;
						(*pos)[3 * size + 1] = posY + 1;
						(*pos)[3 * size + 2] = posZ + m - 1;
						(*spans)[2 * size] = n;
						(*spans)[2 * size + 1] = m;
						break;
					case 5:
						for (n = 0; n < CHUNK - posX &&
							tile(i + n, meshedFaces, 5, t); ++n)
							meshedFaces[i + n] |= 32;
						for (m = 1; m < CHUNK - posZ &&
							stripX(i + m*CHUNK*CHUNK, n, meshedFaces, 5, t); ++m) {
							for (uint8_t k = 0; k < n; ++k)
								meshedFaces[i + m*CHUNK*CHUNK + k] |= 32;
						}
						(*pos)[3 * size] = posX;
						(*pos)[3 * size + 1] = posY;
						(*pos)[3 * size + 2] = posZ - 1;
						(*spans)[2 * size] = n;
						(*spans)[2 * size + 1] = m;
					}
					(*faces)[size] = j;
					(*tex)[size++] = t - 1;
				}
			}
		}
	}
	return size;
}
