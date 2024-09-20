/* SPDX-License-Identifier: MIT */
/*
 * libgeohex
 *
 * Copyright (c) 2024 Go Kudo (https://github.com/zeriyoshi)
 *
 * GeoHex original implementation by @sa2da (http://twitter.com/sa2da)
 * https://www.geohex.org/
 *
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */
#ifndef GEOHEX_H
#define GEOHEX_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LIBGEOHEX_VERSION           "1.1.1"
#define GEOHEX_COMPLIANT_VERSION    "3.2"

#define MAX_LEVEL       15
#define MAX_CODE_LEN    (MAX_LEVEL + 3)
#define MAX_H_DEC9_LEN  (4 + MAX_LEVEL)
#define MAX_H_DEC3_LEN  (MAX_H_DEC9_LEN * 2)

typedef char geohex_code_t[MAX_CODE_LEN];

typedef struct {
    double lon;
    double lat;
} loc_t;

typedef struct {
    int32_t x;
    int32_t y;
    bool rev;
} xy_t;

typedef struct {
    loc_t latlon;
    xy_t xy;
    geohex_code_t code;
} zone_t;

bool adjust_xy(int32_t x, int32_t y, uint32_t level, xy_t *out);
bool get_xy_by_location(const loc_t *location, uint32_t level, xy_t *out);
bool get_xy_by_code(const geohex_code_t code, xy_t *out);
bool get_zone_by_location(const loc_t *location, uint32_t level, zone_t *out);
bool get_zone_by_code(const geohex_code_t code, zone_t *out);
bool get_zone_by_xy(const xy_t *xy, uint32_t level, zone_t *out);

#ifdef __cplusplus
}
#endif

#endif /* GEOHEX_H */
