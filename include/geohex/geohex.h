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

#ifdef __cplusplus
extern "C" {
#endif

#define GEOHEX_COMPLIANT_VERSION "3.2"
#define GEOHEX_CODE_LENGTH 17

typedef char geohex_code_t[GEOHEX_CODE_LENGTH];

typedef struct _latlon_t {
    double lat;
    double lon;
} latlon_t;

typedef struct _xy_t {
    uint32_t x;
    uint32_t y;
} xy_t;

typedef struct _zone_t {
    latlon_t latlon;
    xy_t xy;
    geohex_code_t code;
} zone_t;

bool get_zone_by_location(double lat, double lng, uint32_t level, zone_t *out);
bool get_zone_by_code(const geohex_code_t code, zone_t *out);
bool get_xy_by_location(double lat, double lon, uint32_t level, xy_t *out);
bool get_xy_by_code(const geohex_code_t code, xy_t *out);
bool get_zone_by_xy(const xy_t xy, uint32_t level, zone_t *out);
bool adjust_xy(uint32_t x, uint32_t y, uint32_t level, xy_t *out);

#ifdef __cplusplus
}
#endif

#endif /* GEOHEX_H */
