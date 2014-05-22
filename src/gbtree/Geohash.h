/*
 * Apache License 2.0
 * Copyright 2011 Hiroaki Kawai
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *
 * @author Hiroaki Kawai AT Hiroaki.Kawai@gmail.com
 * @date 4/1/2011
 */
#ifndef GEOHASH_H_
#define GEOHASH_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "../base/GBTreeBase.h"

enum {
	GEOHASH_OK,
	GEOHASH_NOTSUPPORTED,
	GEOHASH_INVALIDCODE,
	GEOHASH_INVALIDARGUMENT,
	GEOHASH_INTERNALERROR,
	GEOHASH_NOMEMORY
};

int geohash_encode_64(double latitude, double longitude, uint64_t* code);
int geohash_decode_64(uint64_t r, double* latitude, double* longitude);
int geohash_neighbors_64(uint64_t code, size_t precision, uint64_t *dst, int *count);

#ifdef __cplusplus
}
#endif

#endif
