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
#ifndef GEOHEX_PRIVATE_H
#define GEOHEX_PRIVATE_H

#include <stdint.h>

double calc_hex_size(uint32_t level);
void loc2xy(double lat, double lon, xy_t *out);
void xy2loc(double x, double y, loc_t *out);

#endif /* GEOHEX_PRIVATE_H */
