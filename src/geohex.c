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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "geohex/geohex.h"

bool get_zone_by_location(double lat, double lng, uint32_t level, zone_t *out) {
    return true;
}

bool get_zone_by_code(const geohex_code_t code, zone_t *out) {
    return true;
}

bool get_xy_by_location(double lat, double lon, uint32_t level, xy_t *out) {
    return true;
}

bool get_xy_by_code(const geohex_code_t code, xy_t *out) {
    return true;
}

bool get_zone_by_xy(const xy_t xy, uint32_t level, zone_t *out) {
    return true;
}

bool adjust_xy(uint32_t x, uint32_t y, uint32_t level, xy_t *out) {
    return true;
}
