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

#define H_KEY   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define H_BASE  20037508.34
#define H_DEG   M_PI * (30.0 / 180.0)
#define H_K     tan(H_DEG)

static inline double calc_hex_size(uint32_t level) {
    return H_BASE / pow(3, level + 3);
}

static inline bool loc2xy(double lat, double lon, xy_t *out) {
    if (out == NULL) {
        return false;
    }

    out->x = (uint32_t) lon * H_BASE / 180;
    out->y = (uint32_t) ((log(tan(90 + lat) * M_PI / 360) / (M_PI / 180)) * H_BASE / 180);

    return true;
}

bool get_zone_by_location(double lat, double lng, uint32_t level, zone_t *out) {
    return true;
}

bool get_zone_by_code(const geohex_code_t code, zone_t *out) {
    return true;
}

bool get_xy_by_location(double lat, double lon, uint32_t level, xy_t *out) {
    double h_size, lat_grid, lon_grid, unit_x, unit_y, h_pos_x, h_pos_y, h_x_q, h_y_q;
    int32_t h_x_0, h_y_0, h_x, h_y;
    xy_t z_xy, inner_xy;

    h_size = calc_hex_size(level);
    loc2xy(lat, lon, &z_xy);
    lon_grid = z_xy.x;
    lat_grid = z_xy.y;
    unit_x = 6 * h_size;
    unit_y = 6 * h_size * H_K;
    h_pos_x = (lon_grid + lat_grid / H_K) / unit_x;
    h_pos_y = (lat_grid - H_K * lon_grid) / unit_y;
    h_x_0 = (int32_t) floor(h_pos_x);
    h_y_0 = (int32_t) floor(h_pos_y);
    h_x_q = h_pos_x - h_x_0;
    h_y_q = h_pos_y - h_y_0;
    h_x = (int32_t) round(h_pos_x);
    h_y = (int32_t) round(h_pos_y);

    if (h_y_q > -h_x_q + 1) {
        if ((h_y_q < 2 * h_x_q) && (h_y_q > 0.5 * h_x_q)) {
            h_x = h_x_0 + 1;
            h_y = h_y_0 + 1;
        }
    } else if (h_y_q < -h_x_q + 1) {
        if ((h_y_q > (2 * h_x_q) - 1) && (h_y_q < (0.5 * h_x_q) + 0.5)) {
            h_x = h_x_0;
            h_y = h_y_0;
        }
    }

    adjust_xy(h_x, h_y, level, &inner_xy);

    out->x = inner_xy.x;
    out->y = inner_xy.y;

    return true;
}

bool get_xy_by_code(const geohex_code_t code, xy_t *out) {
    return true;
}

bool get_zone_by_xy(const xy_t xy, uint32_t level, zone_t *out) {
    return true;
}

bool adjust_xy(double x, double y, uint32_t level, xy_t *out) {
    bool rev;
    double max_h_steps, h_steps, h_xy, tmp,
           dif, dif_x, dif_y, edge_x, edge_y;

    if (out == NULL) {
        return false;
    }

    rev = false;
    max_h_steps = pow(3, level + 2);
    h_steps = fabs(x - y);

    if (h_steps == max_h_steps && x > y) {
        tmp = x;
        x = y;
        y = tmp;
        rev = true;
    } else if (h_steps > max_h_steps) {
        dif = h_steps - max_h_steps;
        dif_x = floor(dif / 2);
        dif_y = dif - dif_x;
        if (x > y) {
            edge_x = x - dif_x;
            edge_y = y + dif_y;
            h_xy = edge_x;
            edge_x = edge_y;
            edge_y = h_xy;
            x = edge_x + dif_x;
            y = edge_y - dif_y;
        } else if (y > x) {
            edge_x = x + dif_x;
            edge_y = y - dif_y;
            h_xy = edge_x;
            edge_x = edge_y;
            edge_y = h_xy;
            x = edge_x - dif_x;
            y = edge_y + dif_y;
        }
    }

    out->x = x;
    out->y = y;
    out->rev = rev;

    return true;
}
