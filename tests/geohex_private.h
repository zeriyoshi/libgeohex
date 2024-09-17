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
void loc2xy(double lon, double lat, double *dx, double *dy);
void xy2loc(double dx, double dy, double *lon, double *lat);

#endif /* GEOHEX_PRIVATE_H */
